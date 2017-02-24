/*
 
 Copyright (c) 2015, Hookflash Inc.
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 
 */


#include "TestRTPSender.h"

#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/IRTPTypes.h>

#include <zsLib/IMessageQueueThread.h>
#include <zsLib/ISettings.h>
#include <zsLib/XML.h>

#include "config.h"
#include "testing.h"

namespace ortc { namespace test { ZS_DECLARE_SUBSYSTEM(ortc_test) } }

using zsLib::String;
using zsLib::ULONG;
using zsLib::PTRNUMBER;
using zsLib::IMessageQueue;
using zsLib::Log;
using zsLib::AutoPUID;
using zsLib::AutoRecursiveLock;
using namespace zsLib::XML;

ZS_DECLARE_TYPEDEF_PTR(zsLib::ISettings, UseSettings)
ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper)

namespace ortc
{
  namespace test
  {
    namespace rtpsender
    {
      ZS_DECLARE_CLASS_PTR(FakeICETransport)
      ZS_DECLARE_CLASS_PTR(FakeSecureTransport)
      ZS_DECLARE_CLASS_PTR(RTPSenderTester)

      ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportTypes, ISecureTransportTypes)
      ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransport, ISecureTransport)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSender)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSenderChannel)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSender)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPListener)
      ZS_DECLARE_USING_PTR(ortc::internal, MediaStreamTrack)

      using zsLib::AutoRecursiveLock;

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark (helpers)
      #pragma mark

      //-------------------------------------------------------------------------
      static bool isRTCPPacketType(const BYTE *data, size_t len)
      {
        if (len < 2) {
          return false;
        }
        BYTE pt = (data[1] & 0x7F);
        return (63 < pt) && (pt < 96);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeICETransport
      #pragma mark

      //-----------------------------------------------------------------------
      FakeICETransport::FakeICETransport(
                                         const make_private &,
                                         IMessageQueuePtr queue,
                                         Milliseconds packetDelay
                                         ) :
        ICETransport(zsLib::Noop(true), queue),
        mPacketDelay(packetDelay)
      {
        ZS_LOG_BASIC(log("created"))
      }

      //-----------------------------------------------------------------------
      void FakeICETransport::init()
      {
        AutoRecursiveLock lock(*this);
        if (Milliseconds() != mPacketDelay) {
          mTimer = ITimer::create(mThisWeak.lock(), mPacketDelay);
        }
      }

      //-----------------------------------------------------------------------
      FakeICETransport::~FakeICETransport()
      {
        mThisWeak.reset();

        ZS_LOG_BASIC(log("destroyed"))
      }

      //-----------------------------------------------------------------------
      FakeICETransportPtr FakeICETransport::create(
                                                   IMessageQueuePtr queue,
                                                   Milliseconds packetDelay
                                                   )
      {
        FakeICETransportPtr pThis(make_shared<FakeICETransport>(make_private{}, queue, packetDelay));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      void FakeICETransport::reliability(ULONG percentage)
      {
        AutoRecursiveLock lock(*this);
        mReliability = percentage;
      }

      //-----------------------------------------------------------------------
      void FakeICETransport::linkTransport(FakeICETransportPtr transport)
      {
        AutoRecursiveLock lock(*this);
        mLinkedTransport = transport;

        if (transport) {
          ZS_LOG_BASIC(log("transport linked") + ZS_PARAM("linked transport", transport->getID()))
        } else {
          ZS_LOG_BASIC(log("transport unlinked"))
        }
      }

      //-----------------------------------------------------------------------
      void FakeICETransport::state(IICETransport::States newState)
      {
        AutoRecursiveLock lock(*this);
        setState(newState);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeICETransport => IICETransport
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeICETransport::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr resultEl = Element::create("ortc::test::rtpsender::FakeICETransport");

        UseServicesHelper::debugAppend(resultEl, "id", getID());

        UseServicesHelper::debugAppend(resultEl, "state", IICETransport::toString(mCurrentState));

        UseServicesHelper::debugAppend(resultEl, "secure transport id", mSecureTransportID);
        UseServicesHelper::debugAppend(resultEl, "secure transport", (bool)(mSecureTransport.lock()));

        UseServicesHelper::debugAppend(resultEl, "linked transport", (bool)(mLinkedTransport.lock()));

        UseServicesHelper::debugAppend(resultEl, "subscriptions", mSubscriptions.size());
        UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeICETransport => IFakeICETransportAsyncDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeICETransport::onPacketFromLinkedFakedTransport(SecureByteBlockPtr buffer)
      {
        FakeSecureTransportPtr transport;

        {
          AutoRecursiveLock lock(*this);
          transport = mSecureTransport.lock();
          if (!transport) {
            ZS_LOG_WARNING(Detail, log("no rtpsender transport attached (thus cannot forward received packet)") + ZS_PARAM("buffer", (PTRNUMBER)(buffer->BytePtr())) + ZS_PARAM("buffer size", buffer->SizeInBytes()))
            return;
          }

          switch (state()) {
            case IICETransport::State_New:
            case IICETransport::State_Checking:
            case IICETransport::State_Disconnected:
            case IICETransport::State_Failed:
            case IICETransport::State_Closed:
            {
              ZS_LOG_WARNING(Detail, log("dropping incoming packet to simulate non-connected state"))
              return;
            }
            case IICETransport::State_Connected:
            case IICETransport::State_Completed:
            {
              break;
            }
          }
        }

        TESTING_CHECK(buffer)

        if (Milliseconds() != mPacketDelay) {
          ZS_LOG_DEBUG(log("delaying packet arrival") + ZS_PARAM("buffer", (PTRNUMBER)(buffer->BytePtr())) + ZS_PARAM("buffer size", buffer->SizeInBytes()))
          mDelayedBuffers.push_back(DelayedBufferPair(zsLib::now() + mPacketDelay, buffer));
          return;
        }

        ZS_LOG_DEBUG(log("packet received") + ZS_PARAM("buffer", (PTRNUMBER)(buffer->BytePtr())) + ZS_PARAM("buffer size", buffer->SizeInBytes()))

        transport->handleReceivedPacket(mComponent, buffer->BytePtr(), buffer->SizeInBytes());
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeICETransport => ITimerDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeICETransport::onTimer(ITimerPtr timer)
      {
        FakeSecureTransportPtr transport;

        DelayedBufferList delayedPackets;

        auto tick = zsLib::now();

        {
          AutoRecursiveLock lock(*this);
          transport = mSecureTransport.lock();
          if (!transport) {
            ZS_LOG_WARNING(Detail, log("no rtpsender transport attached (thus cannot forward delayed packets)"))
            return;
          }

          while (mDelayedBuffers.size() > 0) {
            Time &delayTime = mDelayedBuffers.front().first;
            SecureByteBlockPtr buffer = mDelayedBuffers.front().second;

            if (delayTime > tick) {
              ZS_LOG_INSANE(log("delaying packet until tick") + ZS_PARAM("delay", delayTime) + ZS_PARAM("tick", tick))
              break;
            }

            switch (state()) {
              case IICETransport::State_New:
              case IICETransport::State_Checking:
              case IICETransport::State_Disconnected:
              case IICETransport::State_Failed:
              case IICETransport::State_Closed:
              {
                ZS_LOG_WARNING(Detail, log("dropping incoming packet to simulate non-connected state"))
                return;
              }
              case IICETransport::State_Connected:
              case IICETransport::State_Completed:
              {
                delayedPackets.push_back(DelayedBufferPair(delayTime, buffer));
                break;
              }
            }

            mDelayedBuffers.pop_front();
          }
        }

        for (auto iter = delayedPackets.begin(); iter != delayedPackets.end(); ++iter)
        {
          auto buffer = (*iter).second;

          ZS_LOG_DEBUG(log("packet received (after delay)") + ZS_PARAM("buffer", (PTRNUMBER)(buffer->BytePtr())) + ZS_PARAM("buffer size", buffer->SizeInBytes()))

          transport->handleReceivedPacket(mComponent, buffer->BytePtr(), buffer->SizeInBytes());
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeICETransport => friend FakeSecureTransport
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeICETransport::attachSecure(FakeSecureTransportPtr transport)
      {
        AutoRecursiveLock lock(*this);
        TESTING_CHECK(!((bool)mSecureTransport.lock()))
        mSecureTransportID = transport->getID();
        mSecureTransport = transport;
      }

      //-----------------------------------------------------------------------
      void FakeICETransport::detachSecure(FakeSecureTransport &transport)
      {
        AutoRecursiveLock lock(*this);
        if (transport.getID() != mSecureTransportID) return;
        mSecureTransportID = 0;
        mSecureTransport.reset();
      }

      //---------------------------------------------------------------------
      bool FakeICETransport::sendPacket(
                                        const BYTE *buffer,
                                        size_t bufferSizeInBytes
                                        )
      {
        FakeICETransportPtr transport;

        {
          AutoRecursiveLock lock(*this);
          transport = mLinkedTransport.lock();
          if (!transport) {
            ZS_LOG_WARNING(Detail, log("not linked to another fake transport") + ZS_PARAM("buffer", (PTRNUMBER)(buffer)) + ZS_PARAM("buffer size", bufferSizeInBytes))
            return false;
          }

          switch (state()) {
            case IICETransport::State_New:
            case IICETransport::State_Checking:
            case IICETransport::State_Disconnected:
            case IICETransport::State_Failed:
            case IICETransport::State_Closed:
            {
              ZS_LOG_WARNING(Detail, log("dropping outgoing packet to simulate non-connected state") + ZS_PARAM("buffer size", bufferSizeInBytes))
              return false;
            }
            case IICETransport::State_Connected:
            case IICETransport::State_Completed:
            {
              break;
            }
          }

          if (100 != mReliability) {
            auto value = UseServicesHelper::random(0, 100);
            if (value >= mReliability) {
              ZS_LOG_WARNING(Trace, log("intentionally dropping packet due to connectivity failure") + ZS_PARAM("buffer size", bufferSizeInBytes))
              return true;
            }
          }
        }

        ZS_LOG_DEBUG(log("sending packet to linked fake transport") + ZS_PARAM("other transport", transport->getID()) + ZS_PARAM("buffer", (PTRNUMBER)(buffer)) + ZS_PARAM("buffer size", bufferSizeInBytes))

        SecureByteBlockPtr sendBuffer(make_shared<SecureByteBlock>(buffer, bufferSizeInBytes));

        IFakeICETransportAsyncDelegateProxy::create(transport)->onPacketFromLinkedFakedTransport(sendBuffer);
        return true;
      }

      //-----------------------------------------------------------------------
      IICETransportTypes::States FakeICETransport::state() const
      {
        return mCurrentState;
      }

      //-------------------------------------------------------------------------
      IICETransportSubscriptionPtr FakeICETransport::subscribe(IICETransportDelegatePtr originalDelegate)
      {
        ZS_LOG_DETAIL(log("subscribing to transport state"))

        AutoRecursiveLock lock(*this);
        if (!originalDelegate) return mDefaultSubscription;

        IICETransportSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, getAssociatedMessageQueue());

        IICETransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

        if (delegate) {
          FakeICETransportPtr pThis = mThisWeak.lock();

          if (IICETransportTypes::State_New != mCurrentState) {
            delegate->onICETransportStateChange(pThis, mCurrentState);
          }
        }

        return subscription;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeICETransport => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeICETransport::setState(IICETransportTypes::States state)
      {
        if (state == mCurrentState) return;

        ZS_LOG_DETAIL(log("state changed") + ZS_PARAM("new state", IICETransport::toString(state)) + ZS_PARAM("old state", IICETransport::toString(mCurrentState)))

        mCurrentState = state;

        auto pThis = mThisWeak.lock();
        if (pThis) {
          mSubscriptions.delegate()->onICETransportStateChange(pThis, mCurrentState);
        }
      }

      //-----------------------------------------------------------------------
      bool FakeICETransport::isShutdown()
      {
        return IICETransport::State_Closed == mCurrentState;
      }

      //-----------------------------------------------------------------------
      Log::Params FakeICETransport::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpsender::FakeICETransport");
        UseServicesHelper::debugAppend(objectEl, "id", ICETransport::getID());
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSecureTransport
      #pragma mark

      //-----------------------------------------------------------------------
      FakeSecureTransport::FakeSecureTransport(
                                               const make_private &,
                                               IMessageQueuePtr queue,
                                               FakeICETransportPtr iceTransport
                                               ) :
        DTLSTransport(zsLib::Noop(true), queue),
        mICETransport(iceTransport)
      {
        ZS_LOG_BASIC(log("created"))
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::init()
      {
        AutoRecursiveLock lock(*this);
        mICETransport->attachSecure(mThisWeak.lock());

        mICESubscription = mICETransport->subscribe(mThisWeak.lock());
        TESTING_CHECK(mICESubscription)

        mListener = FakeListener::create(getAssociatedMessageQueue());
        TESTING_CHECK(mListener)
      }

      //-----------------------------------------------------------------------
      FakeSecureTransport::~FakeSecureTransport()
      {
        mThisWeak.reset();

        ZS_LOG_BASIC(log("destroyed"))

        cancel();
      }

      //-----------------------------------------------------------------------
      FakeSecureTransportPtr FakeSecureTransport::create(
                                                         IMessageQueuePtr queue,
                                                         FakeICETransportPtr iceTransport
                                                         )
      {
        FakeSecureTransportPtr pThis(make_shared<FakeSecureTransport>(make_private{}, queue, iceTransport));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::state(IDTLSTransport::States newState)
      {
        AutoRecursiveLock lock(*this);
        setState(newState);
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::setClientRole(bool clientRole)
      {
        AutoRecursiveLock lock(*this);
        mClientRole = clientRole;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSecureTransport => IICETransport
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeSecureTransport::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr resultEl = Element::create("ortc::test::rtpsender::FakeICETransport");

        UseServicesHelper::debugAppend(resultEl, "id", getID());

        UseServicesHelper::debugAppend(resultEl, "state", IDTLSTransport::toString(mCurrentState));

        UseServicesHelper::debugAppend(resultEl, "ice transport", mICETransport ? mICETransport->getID() : 0);

        UseServicesHelper::debugAppend(resultEl, "subscriptions", mSubscriptions.size());
        UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSecureTransport => ISecureTransportForRTPSender
      #pragma mark

      //-----------------------------------------------------------------------
      RTPListenerPtr FakeSecureTransport::getListener() const
      {
        return RTPListener::convert(mListener);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSecureTransport => ISecureTransportForRTPSender
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSecureTransport => ISecureTransportForRTPSender
      #pragma mark

      //-----------------------------------------------------------------------
      PUID FakeSecureTransport::getID() const
      {
        return DTLSTransport::getID();
      }

      //-----------------------------------------------------------------------
      FakeSecureTransport::ISecureTransportSubscriptionPtr FakeSecureTransport::subscribe(ISecureTransportDelegatePtr originalDelegate)
      {
        ZS_LOG_DETAIL(log("subscribing to transport state"))

        AutoRecursiveLock lock(*this);
        if (!originalDelegate) return mDefaultSubscription;

        ISecureTransportSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, getAssociatedMessageQueue());

        ISecureTransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

        if (delegate) {
          FakeSecureTransportPtr pThis = mThisWeak.lock();

          if (ISecureTransportTypes::State_Pending != mLastReportedState) {
            delegate->onSecureTransportStateChanged(pThis, mLastReportedState);
          }
        }
        
        return subscription;
      }

      //-----------------------------------------------------------------------
      ISecureTransportTypes::States FakeSecureTransport::state(ISecureTransportTypes::States ignored) const
      {
        return getSecureState();
      }

      //-----------------------------------------------------------------------
      bool FakeSecureTransport::sendPacket(
                                           IICETypes::Components sendOverICETransport,
                                           IICETypes::Components packetType,
                                           const BYTE *buffer,
                                           size_t bufferLengthInBytes
                                           )
      {
        FakeICETransportPtr iceTransport;

        {
          AutoRecursiveLock lock(*this);
          if (IDTLSTransportTypes::State_Connected != mCurrentState) {
            ZS_LOG_WARNING(Detail, log("cannot send packet when not in validated state"))
            return false;
          }

          iceTransport = mICETransport;
        }

        return iceTransport->sendPacket(buffer, bufferLengthInBytes);
      }

      //-----------------------------------------------------------------------
      IICETransportPtr FakeSecureTransport::getICETransport() const
      {
        return mICETransport;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSecureTransport => IICETransportDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeSecureTransport::onICETransportStateChange(
                                                          IICETransportPtr transport,
                                                          IICETransport::States state
                                                          )
      {
        ZS_LOG_BASIC(log("ice transport state changed") + ZS_PARAM("transport", transport->getID()) + ZS_PARAM("state", IICETransportTypes::toString(state)))

        AutoRecursiveLock lock(*this);
        notifySecureState();
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::onICETransportCandidatePairAvailable(
                                                                     IICETransportPtr transport,
                                                                     CandidatePairPtr candidatePair
                                                                     )
      {
        // ignored
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::onICETransportCandidatePairGone(
                                                                IICETransportPtr transport,
                                                                CandidatePairPtr candidatePair
                                                                )
      {
        // ignored
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::onICETransportCandidatePairChanged(
                                                                   IICETransportPtr transport,
                                                                   CandidatePairPtr candidatePair
                                                                   )
      {
        // ignored
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSecureTransport => IFakeSecureTransportAsyncDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeICETransport => friend FakeICETransport
      #pragma mark

      //-----------------------------------------------------------------------
      bool FakeSecureTransport::handleReceivedPacket(
                                                     IICETypes::Components component,
                                                     const BYTE *buffer,
                                                     size_t bufferSizeInBytes
                                                     )
      {
        UseListenerPtr listener;

        {
          AutoRecursiveLock lock(*this);

          if (IDTLSTransportTypes::State_Connected != mCurrentState) {
            ZS_LOG_WARNING(Detail, log("dropping incoming packet to simulate non validated state"))
            return false;
          }

          listener = mListener;
        }

        if (!listener) {
          ZS_LOG_WARNING(Detail, log("dropping incoming packet (as data channel is gone)"))
          return false;
        }

        return listener->handleRTPPacket(component, isRTCPPacketType(buffer, bufferSizeInBytes) ? IICETypes::Component_RTCP : IICETypes::Component_RTP, buffer, bufferSizeInBytes);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSecureTransport => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeSecureTransport::setState(IDTLSTransportTypes::States state)
      {
        if (state == mCurrentState) return;

        if (isShutdown()) {
          ZS_LOG_DETAIL(log("already shutdown"))
          return;
        }
        ZS_LOG_DETAIL(log("state changed") + ZS_PARAM("new state", IDTLSTransport::toString(state)) + ZS_PARAM("old state", IDTLSTransport::toString(mCurrentState)))

        mCurrentState = state;
        notifySecureState();
      }

      //-----------------------------------------------------------------------
      bool FakeSecureTransport::isShutdown()
      {
        return ((IDTLSTransportTypes::State_Closed == mCurrentState) ||
                (IDTLSTransportTypes::State_Failed == mCurrentState));
      }

      //-----------------------------------------------------------------------
      Log::Params FakeSecureTransport::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpsender::FakeSecureTransport");
        UseServicesHelper::debugAppend(objectEl, "id", DTLSTransport::getID());
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::cancel()
      {
        setState(IDTLSTransport::State_Closed);

        mICETransport->detachSecure(*this);
      }

      //-----------------------------------------------------------------------
      IDTLSTransportTypes::States FakeSecureTransport::state() const
      {
        return mCurrentState;
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::notifySecureState()
      {
        auto state = getSecureState();
        if (state == mLastReportedState) return;

        ZS_LOG_BASIC(log("notify state change") + ZS_PARAM("state", ISecureTransportTypes::toString(state)))

        auto pThis = mThisWeak.lock();
        if (pThis) {
          mSubscriptions.delegate()->onSecureTransportStateChanged(mThisWeak.lock(), state);
        }
        mLastReportedState = state;
      }

      //-----------------------------------------------------------------------
      ISecureTransportTypes::States FakeSecureTransport::getSecureState() const
      {
        if (!mICETransport) return ISecureTransportTypes::State_Closed;

        auto iceState = mICETransport->state();
        switch (iceState) {
          case IICETransportTypes::State_New:           return ISecureTransportTypes::State_Pending;
          case IICETransportTypes::State_Checking:      return ISecureTransportTypes::State_Pending;
          case IICETransportTypes::State_Connected:     break;
          case IICETransportTypes::State_Completed:     break;
          case IICETransportTypes::State_Disconnected:
          case IICETransportTypes::State_Failed:        {
            switch (state()) {
              case IDTLSTransportTypes::State_New:        return ISecureTransportTypes::State_Pending;
              case IDTLSTransportTypes::State_Connecting: return ISecureTransportTypes::State_Pending;
              case IDTLSTransportTypes::State_Connected:  return ISecureTransportTypes::State_Disconnected;
              case IDTLSTransportTypes::State_Closed:     return ISecureTransportTypes::State_Closed;
              case IDTLSTransportTypes::State_Failed:     return ISecureTransportTypes::State_Closed;
            }
            break;
          }
          case IICETransportTypes::State_Closed:        return ISecureTransportTypes::State_Closed;
        }

        switch (state()) {
          case IDTLSTransportTypes::State_New:        return ISecureTransportTypes::State_Pending;
          case IDTLSTransportTypes::State_Connecting: return ISecureTransportTypes::State_Pending;
          case IDTLSTransportTypes::State_Connected:  return ISecureTransportTypes::State_Connected;
          case IDTLSTransportTypes::State_Closed:     return ISecureTransportTypes::State_Closed;
          case IDTLSTransportTypes::State_Failed:     return ISecureTransportTypes::State_Closed;
        }

        return ISecureTransportTypes::State_Closed;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeListener
      #pragma mark

      //-----------------------------------------------------------------------
      FakeListener::FakeListener(IMessageQueuePtr queue) :
        RTPListener(Noop(true), queue)
      {
      }

      //-----------------------------------------------------------------------
      FakeListener::~FakeListener()
      {
        mThisWeak.reset();
        cancel();
      }

      //-----------------------------------------------------------------------
      FakeListenerPtr FakeListener::create(IMessageQueuePtr queue)
      {
        FakeListenerPtr pThis(make_shared<FakeListener>(queue));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      FakeListenerPtr FakeListener::convert(RTPListenerPtr listener)
      {
        return ZS_DYNAMIC_PTR_CAST(FakeListener, listener);
      }

      //-----------------------------------------------------------------------
      void FakeListener::init()
      {
        AutoRecursiveLock lock(*this);

        mCleanBuffersTimer = ITimer::create(mThisWeak.lock(), Seconds(5));
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeListener => IRTPReceiverForRTPReceiver
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeListener::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::rtpsender::FakeListener");

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());

        UseServicesHelper::debugAppend(result, "subscriptions", mSubscriptions.size());
        UseServicesHelper::debugAppend(result, "default subscription", (bool)mDefaultSubscription);

        UseServicesHelper::debugAppend(result, "clean timer", mCleanBuffersTimer ? mCleanBuffersTimer->getID() : 0);

        UseServicesHelper::debugAppend(result, "buffered rtp packets", mBufferedRTPPackets.size());
        UseServicesHelper::debugAppend(result, "buffered rtcp packets", mBufferedRTCPPackets.size());

        auto receiver = mReceiver.lock();
        UseServicesHelper::debugAppend(result, "receiver", receiver ? receiver->getID() : 0);

        UseServicesHelper::debugAppend(result, "unhandled", mUnhandled.size());

        return result;
      }

      //-----------------------------------------------------------------------
      void FakeListener::registerReceiver(
                                          Optional<IMediaStreamTrackTypes::Kinds> kind,
                                          UseReceiverPtr inReceiver,
                                          const Parameters &inParams,
                                          RTCPPacketList *outPacketList
                                          )
      {
        ZS_LOG_BASIC(log("register receiver") + ZS_PARAM("receiver", inReceiver->getID()) + inParams.toDebug())

        AutoRecursiveLock lock(*this);
        mReceiver = inReceiver;

        if (outPacketList) {
          for (auto iter = mBufferedRTCPPackets.begin(); iter != mBufferedRTCPPackets.end(); ++iter) {
            auto &rtcpPacket = (*iter).second;
            outPacketList->push_back(rtcpPacket);
          }
        }

        for (auto iter = mBufferedRTPPackets.begin(); iter != mBufferedRTPPackets.end(); ++iter) {
          auto &rtpPacket = (*iter).second;
          IFakeListenerAsyncDelegateProxy::create(mThisWeak.lock())->onForwardBufferedPacket(rtpPacket);
        }

        mBufferedRTPPackets.clear();
      }

      //-----------------------------------------------------------------------
      void FakeListener::unregisterReceiver(UseReceiver &inReceiver)
      {
        ZS_LOG_BASIC(log("unregister receiver") + ZS_PARAM("receiver", inReceiver.getID()))

        AutoRecursiveLock lock(*this);

        auto receiver = mReceiver.lock();
        if (!receiver) {
          mReceiver.reset();
          return;
        }

        if (receiver->getID() != inReceiver.getID()) return;

        mReceiver.reset();
      }

      //-----------------------------------------------------------------------
      void FakeListener::getPackets(RTCPPacketList &outPacketList)
      {
        outPacketList.clear();

        AutoRecursiveLock lock(*this);
        for (auto iter = mBufferedRTCPPackets.begin(); iter != mBufferedRTCPPackets.end(); ++iter) {
          auto &rtcpPacket = (*iter).second;
          outPacketList.push_back(rtcpPacket);
        }
      }

      //-----------------------------------------------------------------------
      void FakeListener::notifyUnhandled(
                                         const String &muxID,
                                         const String &rid,
                                         IRTPTypes::SSRCType ssrc,
                                         IRTPTypes::PayloadType payloadType
                                         )
      {
        AutoRecursiveLock lock(*this);

        for (auto iter = mUnhandled.begin(); iter != mUnhandled.end(); ++iter)
        {
          auto &data = (*iter);

          if (data.mMuxID != muxID) continue;
          if (data.mRID != rid) continue;
          if (data.mSSRC != ssrc) continue;
          if (data.mPayloadType != payloadType) continue;

          ZS_LOG_TRACE(log("notified unhandled already processed") + ZS_PARAM("mux id", muxID) + ZS_PARAM("rid", rid) + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("payload type", payloadType))
          return;
        }

        UnhandledData data;
        data.mMuxID = muxID;
        data.mRID = rid;
        data.mSSRC = ssrc;
        data.mPayloadType = payloadType;

        mUnhandled.push_back(data);

        ZS_LOG_BASIC(log("notified unhandled") + ZS_PARAM("mux id", muxID) + ZS_PARAM("rid", rid) + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("payload type", payloadType))
        mSubscriptions.delegate()->onRTPListenerUnhandledRTP(mThisWeak.lock(), ssrc, payloadType, muxID, rid);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeListener => IRTPListenerForRTPSender
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeListener::registerSender(
                                        UseSenderPtr inSender,
                                        const Parameters &inParams,
                                        RTCPPacketList &outPacketList
                                        )
      {
        AutoRecursiveLock lock(*this);

        for (auto iter = mSenders.begin(); iter != mSenders.end(); ++iter) {
          auto sender = (*iter).lock();
          if (sender->getID() == inSender->getID()) return;
        }

        mSenders.push_back(inSender);

        for (auto iter = mBufferedRTCPPackets.begin(); iter != mBufferedRTCPPackets.end(); ++iter) {
          auto &rtcpPacket = (*iter).second;
          outPacketList.push_back(rtcpPacket);
        }
      }

      //-----------------------------------------------------------------------
      void FakeListener::unregisterSender(UseSender &inSender)
      {
        AutoRecursiveLock lock(*this);

        for (auto iter_doNotUse = mSenders.begin(); iter_doNotUse != mSenders.end(); ) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto sender = (*current).lock();
          if (!sender) {
            mSenders.erase(current);
            continue;
          }
          if (sender->getID() != inSender.getID()) continue;
          mSenders.erase(current);
          return;
        }
      }
      

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeListener => IRTPListenerForSecureTransport
      #pragma mark

      //-----------------------------------------------------------------------
      bool FakeListener::handleRTPPacket(
                                         IICETypes::Components viaComponent,
                                         IICETypes::Components packetType,
                                         const BYTE *buffer,
                                         size_t bufferLengthInBytes
                                         )
      {
        RTPPacketPtr rtpPacket;
        RTCPPacketPtr rtcpPacket;

        UseReceiverPtr receiver;
        SenderList senders;

        ZS_LOG_BASIC(log("received RTC packet") + ZS_PARAM("via", IICETypes::toString(viaComponent)) + ZS_PARAM("packet type", IICETypes::toString(packetType)) + ZS_PARAM("buffer length", bufferLengthInBytes))

        {
          AutoRecursiveLock lock(*this);

          switch (packetType) {
            case IICETypes::Component_RTP: {
              rtpPacket = RTPPacket::create(buffer, bufferLengthInBytes);
              break;
            }
            case IICETypes::Component_RTCP: {
              rtcpPacket = RTCPPacket::create(buffer, bufferLengthInBytes);
              break;
            }
          }

          if ((!rtpPacket) &&
              (!rtcpPacket)) {
            ZS_LOG_WARNING(Basic, log("failed to parse packet"))
            return false;
          }

          if (rtcpPacket) {
            mBufferedRTCPPackets.push_back(TimeRTCPPacketPair(zsLib::now(), rtcpPacket));
          }

          receiver = mReceiver.lock();
          if ((!receiver) &&
              (rtpPacket)) {
            mBufferedRTPPackets.push_back(TimeRTPPacketPair(zsLib::now(), rtpPacket));
          }

          for (auto iter_doNotUse = mSenders.begin(); iter_doNotUse != mSenders.end(); ) {
            auto current = iter_doNotUse;
            ++iter_doNotUse;

            auto sender = (*current).lock();
            if (!sender) {
              mSenders.erase(current);
              continue;
            }

            senders.push_back(sender);
          }
        }

        if (rtcpPacket) {
          for (auto iter = senders.begin(); iter != senders.end(); ++iter) {
            auto &sender = (*iter);
            sender->handlePacket(viaComponent, rtcpPacket);
          }
          senders.clear();
        }

        if (!receiver) return true;

        if (rtpPacket) {
          return receiver->handlePacket(viaComponent, rtpPacket);
        }
        return receiver->handlePacket(viaComponent, rtcpPacket);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeListener => ITimerDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeListener::onTimer(ITimerPtr timer)
      {
        AutoRecursiveLock lock(*this);

        if (timer == mCleanBuffersTimer) {

          auto tick = zsLib::now();

          while (mBufferedRTPPackets.size() > 0) {
            auto &bufferTime = mBufferedRTPPackets.front().first;

            if (bufferTime + Seconds(60) > tick) break;

            mBufferedRTPPackets.erase(mBufferedRTPPackets.begin());
          }

          while (mBufferedRTCPPackets.size() > 0) {
            auto &bufferTime = mBufferedRTCPPackets.front().first;

            if (bufferTime + Seconds(60) > tick) break;

            mBufferedRTCPPackets.erase(mBufferedRTCPPackets.begin());
          }
          return;
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeListener => IFakeListenerAsyncDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeListener::onForwardBufferedPacket(RTPPacketPtr packet)
      {
        UseReceiverPtr receiver;

        {
          AutoRecursiveLock lock(*this);
          receiver = mReceiver.lock();
        }

        if (!receiver) {
          ZS_LOG_WARNING(Basic, log("dropping packet as receiver is gone") + packet->toDebug())
          return;
        }

        ZS_LOG_DEBUG(log("forwarding packet to receiver") + ZS_PARAM("receiver", receiver->getID()) + packet->toDebug())

        receiver->handlePacket(IICETypes::Component_RTP, packet);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeListener => (friend RTPReceiverTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeListener::setTransport(RTPSenderTesterPtr tester)
      {
        AutoRecursiveLock lock(*this);

        mTester = tester;
      }

      //-----------------------------------------------------------------------
      IRTPListenerSubscriptionPtr FakeListener::subscribe(IRTPListenerDelegatePtr originalDelegate)
      {
        ZS_LOG_DETAIL(log("subscribing to listener"))

        AutoRecursiveLock lock(*this);
        if (!originalDelegate) return mDefaultSubscription;

        IRTPListenerSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, getAssociatedMessageQueue());

        IRTPListenerDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

        if (delegate) {
          FakeListenerPtr pThis = mThisWeak.lock();

          for (auto iter = mUnhandled.begin(); iter != mUnhandled.end(); ++iter)
          {
            auto &data = (*iter);
            delegate->onRTPListenerUnhandledRTP(pThis, data.mSSRC, data.mPayloadType, data.mMuxID, data.mRID);
          }
        }

        return subscription;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeListener => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params FakeListener::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpsender::FakeListener");
        UseServicesHelper::debugAppend(objectEl, "id", FakeListener::getID());
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      void FakeListener::cancel()
      {
        if (mDefaultSubscription) {
          mDefaultSubscription->cancel();
          mDefaultSubscription.reset();
        }

        mReceiver.reset();

        mBufferedRTPPackets.clear();
        mBufferedRTCPPackets.clear();

        mUnhandled.clear();

        mSenders.clear();
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel
      #pragma mark

      //-----------------------------------------------------------------------
      FakeSenderChannel::FakeSenderChannel(
                                           IMessageQueuePtr queue,
                                           const char *receiverChannelID
                                           ) :
        RTPSenderChannel(Noop(true), queue),
        mSenderChannelID(receiverChannelID)
      {
      }

      //-----------------------------------------------------------------------
      FakeSenderChannel::~FakeSenderChannel()
      {
        mThisWeak.reset();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel => IRTPSenderChannelForRTPSender
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeSenderChannel::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::rtpsender::FakeSenderChannel");

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());

        UseServicesHelper::debugAppend(result, "parameters", mParameters ? mParameters->toDebug() : ElementPtr());

        UseServicesHelper::debugAppend(result, "expect buffers", mExpectBuffers.size());
        UseServicesHelper::debugAppend(result, "expect parameters", mExpectParameters.size());
        UseServicesHelper::debugAppend(result, "expect states", mExpectStates.size());

        auto sender = mSender.lock();
        UseServicesHelper::debugAppend(result, "sender", sender ? sender->getID() : 0);

        return result;
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannel::create(
                                     RTPSenderPtr inSender,
                                     MediaStreamTrackPtr track,
                                     const Parameters &params
                                     )
      {
        UseSenderPtr sender(inSender);

        ZS_LOG_BASIC(log("create called") + ZS_PARAM("receiver", sender->getID()) + params.toDebug())

        {
          AutoRecursiveLock lock(*this);

          mSender = sender;

          TESTING_EQUAL(params.hash(), mParameters->hash())
        }
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannel::notifyTransportState(ISecureTransport::States state)
      {
        IFakeSenderChannelAsyncDelegateProxy::create(mThisWeak.lock())->onState(state);
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannel::notifyPackets(RTCPPacketListPtr packets)
      {
        IFakeSenderChannelAsyncDelegateProxy::create(mThisWeak.lock())->onRTCPPackets(packets);
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannel::notifyUpdate(const Parameters &params)
      {
        IFakeSenderChannelAsyncDelegateProxy::create(mThisWeak.lock())->onUpdate(make_shared<Parameters>(params));
      }

      //-----------------------------------------------------------------------
      bool FakeSenderChannel::handlePacket(RTCPPacketPtr packet)
      {
        ZS_LOG_BASIC(log("handle packet") + packet->toDebug())

        RTPSenderTesterPtr tester;

        {
          AutoRecursiveLock lock(*this);

          tester = mTester.lock();
          TESTING_CHECK(tester)

          TESTING_CHECK(mExpectBuffers.size() > 0)

          TESTING_CHECK(0 == UseServicesHelper::compare(*(mExpectBuffers.front()), *(packet->mBuffer)))

          mExpectBuffers.pop_front();
        }

        tester->notifyReceivedPacket();

        return true;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel => IFakeSenderChannelAsyncDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeSenderChannel::onState(ISecureTransportTypes::States state)
      {
        ZS_LOG_BASIC(log("on state") + ZS_PARAM("state", ISecureTransportTypes::toString(state)))

        RTPSenderTesterPtr tester;

        {
          AutoRecursiveLock lock(*this);

          tester = mTester.lock();
          TESTING_CHECK(tester)

          TESTING_CHECK(mExpectStates.size() > 0)

          TESTING_EQUAL(state, mExpectStates.front())

          mExpectStates.pop_front();
        }

        tester->notifySenderChannelOfSecureTransportState();
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannel::onRTCPPackets(RTCPPacketListPtr packets)
      {
        ZS_LOG_BASIC(log("notified packets") + ZS_PARAM("packets", packets->size()))

        for (auto iter = packets->begin(); iter != packets->end(); ++iter) {
          handlePacket(*iter);
        }
      }
      
      //-----------------------------------------------------------------------
      void FakeSenderChannel::onUpdate(ParametersPtr params)
      {
        ZS_LOG_BASIC(log("on updatae") + params->toDebug())

        RTPSenderTesterPtr tester;

        {
          AutoRecursiveLock lock(*this);

          tester = mTester.lock();
          TESTING_CHECK(tester)

          TESTING_CHECK(mExpectParameters.size() > 0)

          TESTING_EQUAL(params->hash(), mExpectParameters.front()->hash())

          mExpectParameters.pop_front();
        }
        
        tester->notifyReceivedChannelUpdate();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel => (friend RTPSenderTester)
      #pragma mark

      //-----------------------------------------------------------------------
      FakeSenderChannelPtr FakeSenderChannel::create(
                                                     IMessageQueuePtr queue,
                                                     const char *receiverChannelID,
                                                     const Parameters &expectedParams
                                                     )
      {
        FakeSenderChannelPtr pThis(make_shared<FakeSenderChannel>(queue, receiverChannelID));
        pThis->mThisWeak = pThis;
        pThis->mParameters = make_shared<Parameters>(expectedParams);
        return pThis;
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannel::setTransport(RTPSenderTesterPtr tester)
      {
        AutoRecursiveLock lock(*this);
        mTester = tester;
      }

      //-----------------------------------------------------------------------
      bool FakeSenderChannel::sendPacket(RTPPacketPtr packet)
      {
        UseSenderPtr sender;

        {
          AutoRecursiveLock lock(*this);
          sender = mSender.lock();

          TESTING_CHECK(sender)
        }

        sender->sendPacket(packet);

        return true;
      }
      
      //-----------------------------------------------------------------------
      bool FakeSenderChannel::sendPacket(RTCPPacketPtr packet)
      {
        UseSenderPtr sender;

        {
          AutoRecursiveLock lock(*this);
          sender = mSender.lock();

          TESTING_CHECK(sender)
        }

        sender->sendPacket(packet);

        return true;
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannel::expectState(ISecureTransport::States state)
      {
        AutoRecursiveLock lock(*this);
        mExpectStates.push_back(state);
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannel::expectUpdate(const Parameters &params)
      {
        AutoRecursiveLock lock(*this);
        mExpectParameters.push_back(make_shared<Parameters>(params));
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannel::expectData(SecureByteBlockPtr data)
      {
        AutoRecursiveLock lock(*this);
        mExpectBuffers.push_back(data);
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannel::stop()
      {
        AutoRecursiveLock lock(*this);

        mSender.reset();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params FakeSenderChannel::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpsender::FakeSenderChannel");
        UseServicesHelper::debugAppend(objectEl, "id", RTPSenderChannel::getID());
        UseServicesHelper::debugAppend(objectEl, "channel id", mSenderChannelID);
        return Log::Params(message, objectEl);
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack
      #pragma mark

      //-----------------------------------------------------------------------
      FakeMediaStreamTrack::FakeMediaStreamTrack(IMessageQueuePtr queue) :
        MediaStreamTrack(Noop(true), queue)
      {
      }

      //-----------------------------------------------------------------------
      FakeMediaStreamTrack::~FakeMediaStreamTrack()
      {
        mThisWeak.reset();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => IMediaStreamTrackForRTPSender
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeMediaStreamTrack::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::rtpsender::FakeMediaStreamTrack");

        UseServicesHelper::debugAppend(result, "kind", IMediaStreamTrackTypes::toString(mKind));

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());

        auto receiver = mSender.lock();
        UseServicesHelper::debugAppend(result, "receiver", receiver ? receiver->getID() : 0);

        UseServicesHelper::debugAppend(result, "expect attach channel IDs", mExpectAttachChannelIDs.size());
        UseServicesHelper::debugAppend(result, "expect detach channel IDs", mExpectDetachChannelIDs.size());

        return result;
      }

      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::create(IMediaStreamTrackTypes::Kinds kind)
      {
        ZS_LOG_BASIC(log("create called") + ZS_PARAM("kind", IMediaStreamTrackTypes::toString(kind)))

        AutoRecursiveLock lock(*this);
        TESTING_CHECK(mKind == kind)
      }

      //-----------------------------------------------------------------------
      IMediaStreamTrackTypes::Kinds FakeMediaStreamTrack::kind() const
      {
        return mKind;
      }

      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::notifyAttachSenderChannel(RTPSenderChannelPtr inChannel)
      {
        UseSenderChannelPtr channel(inChannel);

        ZS_LOG_BASIC(log("notify active receiver channel") + ZS_PARAM("channel", channel ? channel->getID() : 0))

        RTPSenderTesterPtr tester;

        {
          AutoRecursiveLock lock(*this);

          tester = mTester.lock();
          TESTING_CHECK(tester)

          TESTING_CHECK(mExpectAttachChannelIDs.size() > 0)

          auto channelID = (channel ? channel->getID() : 0);

          TESTING_EQUAL(channelID, mExpectAttachChannelIDs.front())

          mExpectAttachChannelIDs.pop_front();
        }
        
        tester->notifyAttachSenderChannel();
      }

      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::notifyDetachSenderChannel(RTPSenderChannelPtr inChannel)
      {
        UseSenderChannelPtr channel(inChannel);

        ZS_LOG_BASIC(log("notify active receiver channel") + ZS_PARAM("channel", channel ? channel->getID() : 0))

        RTPSenderTesterPtr tester;

        {
          AutoRecursiveLock lock(*this);

          tester = mTester.lock();
          TESTING_CHECK(tester)

          TESTING_CHECK(mExpectDetachChannelIDs.size() > 0)

          auto channelID = (channel ? channel->getID() : 0);

          bool found = false;

          for (auto iter = mExpectDetachChannelIDs.begin(); iter != mExpectDetachChannelIDs.end(); ++iter) {

            auto id = (*iter);

            if (channelID != id) continue;

            TESTING_EQUAL(channelID, id)

            mExpectDetachChannelIDs.erase(iter);
            found = true;
            break;
          }

          TESTING_CHECK(found)
        }

        tester->notifyDetachSenderChannel();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => (friend RTPSenderTester)
      #pragma mark

      //-----------------------------------------------------------------------
      FakeMediaStreamTrackPtr FakeMediaStreamTrack::create(
                                                           IMessageQueuePtr queue,
                                                           IMediaStreamTrackTypes::Kinds kind
                                                           )
      {
        FakeMediaStreamTrackPtr pThis(make_shared<FakeMediaStreamTrack>(queue));
        pThis->mThisWeak = pThis;
        return pThis;
      }

      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::setTransport(RTPSenderTesterPtr tester)
      {
        AutoRecursiveLock lock(*this);
        mTester = tester;
      }

      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::expectAttachChannel(SenderChannelID channelID)
      {
        ZS_LOG_BASIC(log("expect attach channel id") + ZS_PARAM("channel id", channelID))

        AutoRecursiveLock lock(*this);
        mExpectAttachChannelIDs.push_back(channelID);
      }

      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::expectDetachChannel(SenderChannelID channelID)
      {
        ZS_LOG_BASIC(log("expect detach channel id") + ZS_PARAM("channel id", channelID))

        AutoRecursiveLock lock(*this);
        mExpectDetachChannelIDs.push_back(channelID);
      }
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params FakeMediaStreamTrack::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpsender::FakeMediaStreamTrack");
        UseServicesHelper::debugAppend(objectEl, "id", MediaStreamTrack::getID());
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver
      #pragma mark

      //-----------------------------------------------------------------------
      FakeReceiver::FakeReceiver(
                                 IMessageQueuePtr queue,
                                 IMediaStreamTrackTypes::Kinds kind
                                 ) :
        RTPReceiver(Noop(true), queue),
        mKind(kind)
      {
      }

      //-----------------------------------------------------------------------
      FakeReceiver::~FakeReceiver()
      {
        mThisWeak.reset();
      }

      //-----------------------------------------------------------------------
      FakeReceiverPtr FakeReceiver::create(
                                           IMessageQueuePtr queue,
                                           IMediaStreamTrackTypes::Kinds kind
                                           )
      {
        FakeReceiverPtr pThis(make_shared<FakeReceiver>(queue, kind));
        pThis->mThisWeak = pThis;
        return pThis;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver => IRTPSenderForRTPSender
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeReceiver::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::rtpsender::FakeReceiver");

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());
        UseServicesHelper::debugAppend(result, mParameters ? mParameters->toDebug() : ElementPtr());
        UseServicesHelper::debugAppend(result, "expect buffers", mExpectBuffers.size());
        UseServicesHelper::debugAppend(result, "listener", mListener ? mListener->getID() : 0);
        UseServicesHelper::debugAppend(result, "secure transport", mSecureTransport ? mSecureTransport->getID() : 0);

        return result;
      }

      //-----------------------------------------------------------------------
      bool FakeReceiver::handlePacket(
                                      IICETypes::Components viaTransport,
                                      RTPPacketPtr packet
                                      )
      {
        ZS_LOG_BASIC(log("received RTCP packet") + packet->toDebug())

        AutoRecursiveLock lock(*this);

        TESTING_CHECK(mExpectBuffers.size() > 0)

        TESTING_CHECK(0 == UseServicesHelper::compare(*(mExpectBuffers.front()), *(packet->mBuffer)))

        mExpectBuffers.pop_front();

        auto tester = mTester.lock();

        if (tester) tester->notifyReceivedPacket();
        
        return true;
      }

      //-----------------------------------------------------------------------
      bool FakeReceiver::handlePacket(
                                      IICETypes::Components viaTransport,
                                      RTCPPacketPtr packet
                                      )
      {
        ZS_LOG_BASIC(log("received RTCP packet") + packet->toDebug())

        AutoRecursiveLock lock(*this);

        TESTING_CHECK(mExpectBuffers.size() > 0)

        TESTING_CHECK(0 == UseServicesHelper::compare(*(mExpectBuffers.front()), *(packet->mBuffer)))

        mExpectBuffers.pop_front();

        auto tester = mTester.lock();

        if (tester) tester->notifyReceivedPacket();

        return true;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver => (friend RTPSenderTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeReceiver::setTransport(RTPSenderTesterPtr tester)
      {
        auto secureTransport = (((bool)tester) ? tester->getFakeSecureTransport() : FakeSecureTransportPtr());
        TESTING_CHECK(secureTransport)


        AutoRecursiveLock lock(*this);

        mTester = tester;

        if (mListener) {
          ZS_LOG_BASIC(log("unregistering receiver") + ZS_PARAM("listener", mListener->getID()))
          mListener->unregisterReceiver(*this);
        }

        if (secureTransport) {
          mListener = secureTransport->getListener();

          if (mParameters) {
            ZS_LOG_BASIC(log("registering receiver") + ZS_PARAM("listener", mListener->getID()) + mParameters->toDebug())

            RTCPPacketList packetList;
            mListener->registerReceiver(mKind, mThisWeak.lock(), *mParameters, &packetList);

            for (auto iter = packetList.begin(); iter != packetList.end(); ++iter) {
              // fake this as if it was a received packet
              handlePacket(IICETypes::Component_RTP, *iter);
            }
          }
          
        } else {
          mListener.reset();
        }
      }

      //-----------------------------------------------------------------------
      PromisePtr FakeReceiver::receive(const Parameters &parameters)
      {
        AutoRecursiveLock lock(*this);

        mParameters = make_shared<Parameters>(parameters);

        if (mListener) {
          RTCPPacketList packetList;
          mListener->registerReceiver(mKind, mThisWeak.lock(), *mParameters, &packetList);

          for (auto iter = packetList.begin(); iter != packetList.end(); ++iter) {
            // fake this as if it was a received packet
            handlePacket(IICETypes::Component_RTP, *iter);
          }
        }
        return Promise::createResolved(getAssociatedMessageQueue());
      }

      //-----------------------------------------------------------------------
      void FakeReceiver::stop()
      {
        AutoRecursiveLock lock(*this);

        if (mListener) {
          mListener->unregisterReceiver(*this);
        }
      }

      //-----------------------------------------------------------------------
      void FakeReceiver::expectData(SecureByteBlockPtr data)
      {
        TESTING_CHECK((bool)data)

        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("expecting buffer") + ZS_PARAM("buffer size", data->SizeInBytes()))

        mExpectBuffers.push_back(data);
      }

      //-----------------------------------------------------------------------
      void FakeReceiver::sendPacket(SecureByteBlockPtr buffer)
      {
        UseSecureTransportPtr transport;

        {
          AutoRecursiveLock lock(*this);

          auto tester = mTester.lock();
          TESTING_CHECK(tester)

          transport = tester->getFakeSecureTransport();
          TESTING_CHECK(transport)
        }

        transport->sendPacket(IICETypes::Component_RTP, IICETypes::Component_RTP, buffer->BytePtr(), buffer->SizeInBytes());
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params FakeReceiver::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpsender::FakeReceiver");
        UseServicesHelper::debugAppend(objectEl, "id", FakeReceiver::getID());
        return Log::Params(message, objectEl);
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderTester::UnhandledEventData
      #pragma mark

      //-----------------------------------------------------------------------
      RTPSenderTester::UnhandledEventData::UnhandledEventData(
                                                                DWORD ssrc,
                                                                BYTE pt,
                                                                const char *mid,
                                                                const char *rid
                                                                ) :
        mSSRC(ssrc),
        mPT(pt),
        mMID(mid),
        mRID(rid)
      {
      }

      //-----------------------------------------------------------------------
      bool RTPSenderTester::UnhandledEventData::operator==(const UnhandledEventData &op2) const
      {
        return ((mSSRC == op2.mSSRC) &&
                (mPT == op2.mPT) &&
                (mMID == op2.mMID) &&
                (mRID == op2.mRID));
      }
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderTester::Expectations
      #pragma mark

      //-----------------------------------------------------------------------
      bool RTPSenderTester::Expectations::operator==(const Expectations &op2) const
      {
        return (mUnhandled == op2.mUnhandled) &&

               (mReceivedPackets == op2.mReceivedPackets) &&

               (mError == op2.mError) &&
               (mSSRCconflict == op2.mSSRCconflict) &&
               (mChannelUpdate == op2.mChannelUpdate) &&
               (mSenderChannelOfSecureTransportState == op2.mSenderChannelOfSecureTransportState) &&

               (mAttachSenderChannel == op2.mAttachSenderChannel) &&
               (mDetachSenderChannel == op2.mDetachSenderChannel) &&

               (mKind == op2.mKind);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderTester::OverrideSenderChannelFactory
      #pragma mark

      //-----------------------------------------------------------------------
      RTPSenderTester::OverrideSenderChannelFactoryPtr RTPSenderTester::OverrideSenderChannelFactory::create(RTPSenderTesterPtr tester)
      {
        OverrideSenderChannelFactoryPtr pThis(make_shared<OverrideSenderChannelFactory>());
        pThis->mTester = tester;
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPSenderChannelPtr RTPSenderTester::OverrideSenderChannelFactory::create(
                                                                                RTPSenderPtr receiver,
                                                                                MediaStreamTrackPtr track,
                                                                                const Parameters &params
                                                                                )
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)

        return tester->create(receiver, params);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderTester::OverrideSenderChannelFactory
      #pragma mark

      //-----------------------------------------------------------------------
      RTPSenderTester::OverrideMediaStreamTrackFactoryPtr RTPSenderTester::OverrideMediaStreamTrackFactory::create(RTPSenderTesterPtr tester)
      {
        OverrideMediaStreamTrackFactoryPtr pThis(make_shared<OverrideMediaStreamTrackFactory>());
        pThis->mTester = tester;
        return pThis;
      }

      //-----------------------------------------------------------------------
      MediaStreamTrackPtr RTPSenderTester::OverrideMediaStreamTrackFactory::create(
                                                                                     IMediaStreamTrackTypes::Kinds kind,
                                                                                     bool remote,
                                                                                     TrackConstraintsPtr constraints
                                                                                     )
      {
        TESTING_CHECK(false)
        return MediaStreamTrackPtr();
      }

      //-----------------------------------------------------------------------
      MediaStreamTrackPtr RTPSenderTester::OverrideMediaStreamTrackFactory::create(IMediaStreamTrackTypes::Kinds kind)
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)

        return tester->create(kind);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderTester
      #pragma mark

      //-----------------------------------------------------------------------
      RTPSenderTesterPtr RTPSenderTester::create(
                                                 IMessageQueuePtr queue,
                                                 IMediaStreamTrackTypes::Kinds kind,
                                                 bool overrideFactories,
                                                 Milliseconds packetDelay
                                                 )
      {
        RTPSenderTesterPtr pThis(new RTPSenderTester(queue, overrideFactories));
        pThis->mThisWeak = pThis;
        pThis->init(kind, packetDelay);
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPSenderTester::RTPSenderTester(
                                       IMessageQueuePtr queue,
                                       bool overrideFactories
                                       ) :
        SharedRecursiveLock(SharedRecursiveLock::create()),
        MessageQueueAssociator(queue),
        mOverrideFactories(overrideFactories)
      {
        ZS_LOG_BASIC(log("rtpsender tester"))
      }

      //-----------------------------------------------------------------------
      RTPSenderTester::~RTPSenderTester()
      {
        ZS_LOG_BASIC(log("rtpsender tester"))

        if (mOverrideFactories) {
          RTPSenderChannelFactory::override(RTPSenderChannelFactoryPtr());
          MediaStreamTrackFactory::override(MediaStreamTrackFactoryPtr());
        }
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::init(
                                 IMediaStreamTrackTypes::Kinds kind,
                                 Milliseconds packetDelay
                                 )
      {
        AutoRecursiveLock lock(*this);

        if (mOverrideFactories) {
          RTPSenderChannelFactory::override(OverrideSenderChannelFactory::create(mThisWeak.lock()));
          MediaStreamTrackFactory::override(OverrideMediaStreamTrackFactory::create(mThisWeak.lock()));
        }

        mICETransport = FakeICETransport::create(getAssociatedMessageQueue(), packetDelay);
        mDTLSTransport = FakeSecureTransport::create(getAssociatedMessageQueue(), mICETransport);

        auto listener = mDTLSTransport->getListener();
        TESTING_CHECK(listener)

        mListenerSubscription = FakeListener::convert(listener)->subscribe(mThisWeak.lock());
        TESTING_CHECK(mListenerSubscription)

        mMediaStreamTrack = FakeMediaStreamTrack::create(getAssociatedMessageQueue(), kind);
        mMediaStreamTrack->setTransport(mThisWeak.lock());

        mSender = IRTPSender::create(mThisWeak.lock(), mMediaStreamTrack, mDTLSTransport);
        TESTING_CHECK(mSender)
      }

      //-----------------------------------------------------------------------
      bool RTPSenderTester::matches()
      {
        AutoRecursiveLock lock(*this);
        return mExpectationsFound == mExpecting;
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::close()
      {
        AutoRecursiveLock lock(*this);

        mSender->stop();

        for (auto iter = mAttached.begin(); iter != mAttached.end(); ++iter) {
          auto receiverChannel = (*iter).second.first.lock();
          auto &sender = (*iter).second.second;

          if (receiverChannel) {
            receiverChannel->stop();
          }
          if (sender) {
            sender->stop();
          }
        }

        mAttached.clear();
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::closeByReset()
      {
        AutoRecursiveLock lock(*this);
        mICETransport.reset();
        mDTLSTransport.reset();

        mSender.reset();
      }

      //-----------------------------------------------------------------------
      RTPSenderTester::Expectations RTPSenderTester::getExpectations() const
      {
        return mExpectationsFound;
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::state(IICETransport::States newState)
      {
        FakeICETransportPtr transport;
        {
          AutoRecursiveLock lock(*this);
          transport = mICETransport;
        }
        transport->state(newState);
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::state(IDTLSTransport::States newState)
      {
        FakeSecureTransportPtr transport;
        {
          AutoRecursiveLock lock(*this);
          transport = mDTLSTransport;
        }
        transport->state(newState);
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::setClientRole(bool clientRole)
      {
        FakeSecureTransportPtr transport;
        {
          AutoRecursiveLock lock(*this);
          transport = mDTLSTransport;
        }
        transport->setClientRole(clientRole);
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::connect(RTPSenderTesterPtr remote)
      {
        AutoRecursiveLock lock(*this);
        AutoRecursiveLock lock2(*remote);

        TESTING_CHECK(remote)

        auto localTransport = getICETransport();
        auto remoteTransport = remote->getICETransport();

        TESTING_CHECK((bool)localTransport)
        TESTING_CHECK((bool)remoteTransport)

        localTransport->linkTransport(remoteTransport);
        remoteTransport->linkTransport(localTransport);

        mConnectedTester = remote;
        remote->mConnectedTester = mThisWeak.lock();
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::createSenderChannel(
                                                const char *senderChannelID,
                                                const char *parametersID
                                                )
      {
        FakeSenderChannelPtr senderChannel = getSenderChannel(senderChannelID);

        if (!senderChannel) {
          auto params = getParameters(parametersID);
          TESTING_CHECK(params)
          senderChannel = FakeSenderChannel::create(getAssociatedMessageQueue(), senderChannelID, *params);
          attach(senderChannelID, senderChannel);
        }

        TESTING_CHECK(senderChannel)
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::createReceiver(const char *receiverID)
      {
        FakeReceiverPtr receiver = getReceiver(receiverID);

        IMediaStreamTrackTypes::Kinds kind {};

        {
          AutoRecursiveLock lock(*this);
          TESTING_CHECK((bool)mMediaStreamTrack)
          kind = mMediaStreamTrack->kind();
        }

        if (!receiver) {
          receiver = FakeReceiver::create(getAssociatedMessageQueue(), kind);
          attach(receiverID, receiver);
        }

        TESTING_CHECK(receiver)
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::receive(
                                    const char *receiverID,
                                    const char *parametersID
                                    )
      {
        FakeReceiverPtr receiver = getReceiver(receiverID);

        IMediaStreamTrackTypes::Kinds kind {};

        {
          AutoRecursiveLock lock(*this);
          TESTING_CHECK((bool)mMediaStreamTrack)
          kind = mMediaStreamTrack->kind();
        }

        if (!receiver) {
          receiver = FakeReceiver::create(getAssociatedMessageQueue(), kind);
          attach(receiverID, receiver);
        }

        TESTING_CHECK(receiver)

        auto params = getParameters(parametersID);
        TESTING_CHECK(params)

        receiver->receive(*params);
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::send(const char *parametersID)
      {
        TESTING_CHECK(mSender)

        auto params = getParameters(parametersID);
        TESTING_CHECK(params)

        mSender->send(*params);
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::stop(const char *senderOrSenderID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mAttached.find(String(senderOrSenderID));
        TESTING_CHECK(found != mAttached.end())

        auto receiverChannel = (*found).second.first.lock();
        auto sender = (*found).second.second;

        if (receiverChannel) {
          receiverChannel->stop();
        }
        if (sender) {
          sender->stop();
        }

        mAttached.erase(found);
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::attach(
                                   const char *receiverChannelID,
                                   FakeSenderChannelPtr receiverChannel
                                   )
      {
        TESTING_CHECK(receiverChannel)

        String receiverChannelIDStr(receiverChannelID);

        AutoRecursiveLock lock(*this);

        FakeSenderChannelPtr oldChannel;

        for (auto iter_doNotUse = mFakeSenderChannelCreationList.begin(); iter_doNotUse != mFakeSenderChannelCreationList.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &channelID = (*current).first;

          if (channelID != receiverChannelIDStr) continue;

          oldChannel = (*current).second;

          mFakeSenderChannelCreationList.erase(current);
        }

        mFakeSenderChannelCreationList.push_back(FakeSenderChannelPair(receiverChannelIDStr, receiverChannel));

        auto found = mAttached.find(receiverChannelIDStr);

        if (found != mAttached.end()) {
          auto &previousSenderWeak = (*found).second.first;
          auto previousSender = previousSenderWeak.lock();
          if (previousSender) {
            previousSender->stop();
          }

          previousSenderWeak = receiverChannel;
          receiverChannel->setTransport(mThisWeak.lock());
          return;
        }

        mAttached[receiverChannelIDStr] = FakeSenderChannelFakeReceiverPair(receiverChannel, FakeReceiverPtr());
        receiverChannel->setTransport(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::attach(
                                   const char *receiverID,
                                   FakeReceiverPtr receiver
                                   )
      {
        AutoRecursiveLock lock(*this);

        auto found = mAttached.find(String(receiverID));

        if (found != mAttached.end()) {
          auto &previousReceiver = (*found).second.second;
          if (previousReceiver) {
            previousReceiver->stop();
          }

          previousReceiver = receiver;
          receiver->setTransport(mThisWeak.lock());
          return;
        }

        mAttached[String(receiverID)] = FakeSenderChannelFakeReceiverPair(FakeSenderChannelPtr(), receiver);
        receiver->setTransport(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelPtr RTPSenderTester::detachSenderChannel(const char *receiverChannelID)
      {
        String receiverChannelIDStr(receiverChannelID);

        AutoRecursiveLock lock(*this);

        auto found = mAttached.find(receiverChannelIDStr);
        TESTING_CHECK(found != mAttached.end())

        auto &currentSenderWeak = (*found).second.first;
        auto &currentSender = (*found).second.second;

        FakeSenderChannelPtr receiverChannel = currentSenderWeak.lock();

        if (receiverChannel) {
          receiverChannel->setTransport(RTPSenderTesterPtr());
        }

        currentSenderWeak.reset();

        if (!currentSender) mAttached.erase(found);

        if (receiverChannel) {
          for (auto iter_doNotUse = mFakeSenderChannelCreationList.begin(); iter_doNotUse != mFakeSenderChannelCreationList.end(); )
          {
            auto current = iter_doNotUse;
            ++iter_doNotUse;

            auto &channelID = (*current).first;

            if (channelID != receiverChannelIDStr) continue;

            mFakeSenderChannelCreationList.erase(current);
          }
        }

        return receiverChannel;
      }
      
      //-----------------------------------------------------------------------
      FakeReceiverPtr RTPSenderTester::detachReceiver(const char *receiverID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mAttached.find(String(receiverID));
        TESTING_CHECK(found != mAttached.end())

        auto &currentSender = (*found).second.first;
        auto &currentReceiver = (*found).second.second;

        FakeReceiverPtr receiver = currentReceiver;

        if (receiver) {
          receiver->setTransport(RTPSenderTesterPtr());
        }

        currentReceiver.reset();
        if (!currentSender.lock()) mAttached.erase(found);

        return receiver;
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::expectKind(IMediaStreamTrackTypes::Kinds kind)
      {
        AutoRecursiveLock lock(*this);
        mExpecting.mKind = kind;
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::expectingUnhandled(
                                               IRTPTypes::SSRCType ssrc,
                                               PayloadType payloadType,
                                               const char *mid,
                                               const char *rid
                                               )
      {
        AutoRecursiveLock lock(*this);

        ++mExpecting.mUnhandled;
        mExpectingUnhandled.push_back(UnhandledEventData(ssrc, payloadType, mid, rid));
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::expectReceiveChannelUpdate(
                                                       const char *senderChannelID,
                                                       const char *parametersID
                                                       )
      {
        FakeSenderChannelPtr senderChannel = getSenderChannel(senderChannelID);

        ParametersPtr params;

        {
          AutoRecursiveLock lock(*this);
          ++mExpecting.mChannelUpdate;

          auto found = mParameters.find(String(parametersID));
          TESTING_CHECK(found != mParameters.end())

          params = make_shared<Parameters>(*((*found).second));
          TESTING_CHECK(params)
        }

        TESTING_CHECK(senderChannel)

        senderChannel->expectUpdate(*params);
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::expectState(
                                        const char *receiverChannelID,
                                        ISecureTransportTypes::States state
                                        )
      {
        {
          AutoRecursiveLock lock(*this);
          ++mExpecting.mSenderChannelOfSecureTransportState;
        }
        FakeSenderChannelPtr receiverChannel = getSenderChannel(receiverChannelID);

        TESTING_CHECK(receiverChannel)

        receiverChannel->expectState(state);
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::expectAttachChannel(const char *senderChannelID)
      {
        FakeMediaStreamTrackPtr track;
        FakeSenderChannelPtr senderChannel;

        {
          AutoRecursiveLock lock(*this);
          ++mExpecting.mAttachSenderChannel;

          TESTING_CHECK(mMediaStreamTrack)

          if (NULL != senderChannelID) {
            senderChannel = getSenderChannel(senderChannelID);

            TESTING_CHECK(senderChannel)
          }

          track = mMediaStreamTrack;
        }

        track->expectAttachChannel(senderChannel ? senderChannel->getID() : 0);
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::expectDetachChannel(const char *senderChannelID)
      {
        FakeMediaStreamTrackPtr track;
        FakeSenderChannelPtr senderChannel;

        {
          AutoRecursiveLock lock(*this);
          ++mExpecting.mDetachSenderChannel;

          TESTING_CHECK(mMediaStreamTrack)

          if (NULL != senderChannelID) {
            senderChannel = getSenderChannel(senderChannelID);

            TESTING_CHECK(senderChannel)
          }

          track = mMediaStreamTrack;
        }

        track->expectDetachChannel(senderChannel ? senderChannel->getID() : 0);
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::store(
                                  const char *packetID,
                                  RTPPacketPtr packet
                                  )
      {
        TESTING_CHECK(packet)
        AutoRecursiveLock lock(*this);

        auto found = mPackets.find(String(packetID));
        if (found != mPackets.end()) {
          (*found).second.first = packet;
          return;
        }

        mPackets[String(packetID)] = PacketPair(packet, RTCPPacketPtr());
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::store(
                                  const char *packetID,
                                  RTCPPacketPtr packet
                                  )
      {
        TESTING_CHECK(packet)
        AutoRecursiveLock lock(*this);

        auto found = mPackets.find(String(packetID));
        if (found != mPackets.end()) {
          (*found).second.second = packet;
          return;
        }

        mPackets[String(packetID)] = PacketPair(RTPPacketPtr(), packet);
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::store(
                                  const char *parametersID,
                                  const Parameters &params
                                  )
      {
        AutoRecursiveLock lock(*this);

        auto found = mParameters.find(String(parametersID));
        if (found != mParameters.end()) {
          (*found).second = make_shared<Parameters>(params);
          return;
        }

        mParameters[String(parametersID)] = make_shared<Parameters>(params);
      }

      //-----------------------------------------------------------------------
      RTPPacketPtr RTPSenderTester::getRTPPacket(const char *packetID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mPackets.find(String(packetID));
        if (found == mPackets.end()) return RTPPacketPtr();
        return (*found).second.first;
      }

      //-----------------------------------------------------------------------
      RTCPPacketPtr RTPSenderTester::getRTCPPacket(const char *packetID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mPackets.find(String(packetID));
        if (found == mPackets.end()) return RTCPPacketPtr();
        return (*found).second.second;
      }

      //-----------------------------------------------------------------------
      RTPSenderTester::ParametersPtr RTPSenderTester::getParameters(const char *parametersID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mParameters.find(String(parametersID));
        if (found == mParameters.end()) return ParametersPtr();
        return make_shared<Parameters>(*((*found).second));
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::sendPacket(
                                       const char *receiverOrSenderChannelID,
                                       const char *packetID
                                       )
      {
        RTPPacketPtr rtp;
        RTCPPacketPtr rtcp;

        {
          AutoRecursiveLock lock(*this);
          auto found = mPackets.find(String(packetID));
          TESTING_CHECK(found != mPackets.end())

          rtp = (*found).second.first;
          rtcp = (*found).second.second;
        }

        {
          FakeSenderChannelPtr senderChannel = getSenderChannel(receiverOrSenderChannelID);
          if (senderChannel) {
            if (rtp) {
              senderChannel->sendPacket(rtp);
            }
            if (rtcp) {
              senderChannel->sendPacket(rtcp);
            }
            return;
          }
        }

        if (rtp) {
          sendData(receiverOrSenderChannelID, rtp->buffer());
        }
        if (rtcp) {
          sendData(receiverOrSenderChannelID, rtcp->buffer());
        }
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::expectPacket(
                                         const char *receiverOrSenderChannelID,
                                         const char *packetID
                                         )
      {
        RTPPacketPtr rtp;
        RTCPPacketPtr rtcp;

        {
          AutoRecursiveLock lock(*this);
          ++mExpecting.mReceivedPackets;

          auto found = mPackets.find(String(packetID));
          TESTING_CHECK(found != mPackets.end())

          rtp = (*found).second.first;
          rtcp = (*found).second.second;
        }

        if (rtp) {
          expectData(receiverOrSenderChannelID, rtp->buffer());
        }
        if (rtcp) {
          expectData(receiverOrSenderChannelID, rtcp->buffer());
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderTester::IRTPSenderDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void RTPSenderTester::onRTPListenerUnhandledRTP(
                                                      IRTPListenerPtr receiver,
                                                      IRTPTypes::SSRCType ssrc,
                                                      PayloadType payloadType,
                                                      const char *mid,
                                                      const char *rid
                                                      )
      {
        ZS_LOG_BASIC(log("rtp listener unhandled rtp event") + ZS_PARAM("receiver id", receiver->getID()) + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("payload type", payloadType) + ZS_PARAM("mid", mid) + ZS_PARAM("rid", rid))

        AutoRecursiveLock lock(*this);
        ++mExpectationsFound.mUnhandled;

        TESTING_CHECK(mExpectingUnhandled.size() > 0)

        auto &expecting = mExpectingUnhandled.front();

        TESTING_CHECK(expecting == UnhandledEventData(ssrc, payloadType, mid, rid))

        mExpectingUnhandled.pop_front();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderTester::IRTPSenderDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void RTPSenderTester::onRTPSenderSSRCConflict(
                                                    IRTPSenderPtr sender,
                                                    IRTPTypes::SSRCType ssrc
                                                    )
      {
        ZS_LOG_BASIC(log("rtp receiver ssrc conflict") + ZS_PARAM("sender", sender->getID()) + ZS_PARAM("ssrc", ssrc))

        AutoRecursiveLock lock(*this);
        ++mExpectationsFound.mSSRCconflict;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderTester => (friend fake sender/receiver)
      #pragma mark

      //-----------------------------------------------------------------------
      FakeSecureTransportPtr RTPSenderTester::getFakeSecureTransport() const
      {
        return mDTLSTransport;
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::notifyReceivedPacket()
      {
        ZS_LOG_BASIC(log("notified received packet"))

        AutoRecursiveLock lock(*this);
        ++mExpectationsFound.mReceivedPackets;
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::notifyReceivedChannelUpdate()
      {
        ZS_LOG_BASIC(log("notified received channel update"))

        AutoRecursiveLock lock(*this);
        ++mExpectationsFound.mChannelUpdate;
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::notifyAttachSenderChannel()
      {
        ZS_LOG_BASIC(log("notified active channel"))

        AutoRecursiveLock lock(*this);
        ++mExpectationsFound.mAttachSenderChannel;
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::notifyDetachSenderChannel()
      {
        ZS_LOG_BASIC(log("notified active channel"))

        AutoRecursiveLock lock(*this);
        ++mExpectationsFound.mDetachSenderChannel;
      }
      
      //-----------------------------------------------------------------------
      void RTPSenderTester::notifySenderChannelOfSecureTransportState()
      {
        ZS_LOG_BASIC(log("notified receiver channel of secure transport state"))

        AutoRecursiveLock lock(*this);
        ++mExpectationsFound.mSenderChannelOfSecureTransportState;
      }

      //-----------------------------------------------------------------------
      RTPSenderChannelPtr RTPSenderTester::create(
                                                  RTPSenderPtr sender,
                                                  const Parameters &params
                                                  )
      {
        AutoRecursiveLock lock(*this);

        TESTING_CHECK(mFakeSenderChannelCreationList.size() > 0)

        FakeSenderChannelPtr receiverChannel = mFakeSenderChannelCreationList.front().second;

        TESTING_CHECK(receiverChannel)

        receiverChannel->create(sender, MediaStreamTrackPtr(), params);

        mFakeSenderChannelCreationList.pop_front();

        return receiverChannel;
      }

      //-----------------------------------------------------------------------
      MediaStreamTrackPtr RTPSenderTester::create(IMediaStreamTrackTypes::Kinds kind)
      {
        FakeMediaStreamTrackPtr track = FakeMediaStreamTrack::create(getAssociatedMessageQueue(), kind);
        track->setTransport(mThisWeak.lock());

        {
          AutoRecursiveLock lock(*this);

          TESTING_CHECK(!mMediaStreamTrack)

          mExpectationsFound.mKind = kind;
          mMediaStreamTrack = track;
        }

        TESTING_CHECK(track)

        track->create(kind);
        return track;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderTester => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params RTPSenderTester::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpsender::RTPSenderTester");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      FakeICETransportPtr RTPSenderTester::getICETransport() const
      {
        AutoRecursiveLock lock(*this);
        return mICETransport;
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelPtr RTPSenderTester::getSenderChannel(const char *senderChannelID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mAttached.find(String(senderChannelID));
        if (mAttached.end() == found) return FakeSenderChannelPtr();
        return (*found).second.first.lock();
      }

      //-----------------------------------------------------------------------
      FakeReceiverPtr RTPSenderTester::getReceiver(const char *receiverID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mAttached.find(String(receiverID));
        if (mAttached.end() == found) return FakeReceiverPtr();
        return (*found).second.second;
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::expectData(
                                       const char *receiverOrSenderChannelID,
                                       SecureByteBlockPtr secureBuffer
                                       )
      {
        {
          FakeReceiverPtr receiver = getReceiver(receiverOrSenderChannelID);

          if (receiver) {
            receiver->expectData(secureBuffer);
          }
        }

        {
          FakeSenderChannelPtr senderChannel = getSenderChannel(receiverOrSenderChannelID);

          if (senderChannel) {
            senderChannel->expectData(secureBuffer);
          }
        }
      }

      //-----------------------------------------------------------------------
      void RTPSenderTester::sendData(
                                     const char *receiverOrSenderChannelID,
                                     SecureByteBlockPtr secureBuffer
                                     )
      {
        FakeReceiverPtr receiver = getReceiver(receiverOrSenderChannelID);
        TESTING_CHECK(receiver)

        receiver->sendPacket(secureBuffer);
      }
    }
  }
}

ZS_DECLARE_USING_PTR(ortc::test::rtpsender, FakeICETransport)
ZS_DECLARE_USING_PTR(ortc::test::rtpsender, RTPSenderTester)
ZS_DECLARE_USING_PTR(ortc, IICETransport)
ZS_DECLARE_USING_PTR(ortc, IDTLSTransport)
using ortc::IDTLSTransportTypes;
ZS_DECLARE_USING_PTR(ortc, IDataChannel)
ZS_DECLARE_USING_PTR(ortc, IRTPSender)
ZS_DECLARE_USING_PTR(ortc, IRTPTypes)

ZS_DECLARE_USING_PTR(ortc::internal, RTPPacket)
ZS_DECLARE_USING_PTR(ortc::internal, RTCPPacket)
ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportTypes, ISecureTransportTypes)

ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::Parameters, Parameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::CodecParameters, CodecParameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::EncodingParameters, EncodingParameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::RTXParameters, RTXParameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::FECParameters, FECParameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IMediaStreamTrackTypes, IMediaStreamTrackTypes)

using ortc::IICETypes;
using zsLib::Optional;
using zsLib::WORD;
using zsLib::DWORD;
using zsLib::BYTE;
using zsLib::Milliseconds;
using ortc::SecureByteBlock;
using ortc::SecureByteBlockPtr;

#define TEST_BASIC_ROUTING 0
#define TEST_SSRC_ROUTING 1

static void bogusSleep()
{
  for (int loop = 0; loop < 100; ++loop)
  {
    TESTING_SLEEP(100)
  }
}

void doTestRTPSender()
{
  if (!ORTC_TEST_DO_RTP_SENDER_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  UseSettings::applyDefaults();

  auto thread(zsLib::IMessageQueueThread::createBasic());

  RTPSenderTesterPtr testObject1;
  RTPSenderTesterPtr testObject2;

  TESTING_STDOUT() << "WAITING:      Waiting for RTPSender testing to complete (max wait is 180 seconds).\n";

  // check to see if all DNS routines have resolved
  {
    ULONG testNumber = 0;
    ULONG maxSteps = 80;

    do
    {
      TESTING_STDOUT() << "TESTING       ---------->>>>>>>>>> " << testNumber << " <<<<<<<<<<----------\n";

      bool quit = false;
      ULONG expecting = 0;

      switch (testNumber) {
        case TEST_BASIC_ROUTING: {
          {
            UseSettings::setUInt("ortc/rtp-receiver/max-age-rtp-packets-in-seconds", 60);
            UseSettings::setUInt("ortc/rtp-receiver/max-age-rtcp-packets-in-seconds", 60);

            testObject1 = RTPSenderTester::create(thread, IMediaStreamTrackTypes::Kind_Audio, true);
            testObject2 = RTPSenderTester::create(thread, IMediaStreamTrackTypes::Kind_Audio, false);

            TESTING_CHECK(testObject1)
            TESTING_CHECK(testObject2)

            testObject1->setClientRole(true);
            testObject2->setClientRole(false);
          }
          break;
        }
        case TEST_SSRC_ROUTING: {
          {
            testObject1 = RTPSenderTester::create(thread, IMediaStreamTrackTypes::Kind_Audio, true);
            testObject2 = RTPSenderTester::create(thread, IMediaStreamTrackTypes::Kind_Audio, false);

            TESTING_CHECK(testObject1)
            TESTING_CHECK(testObject2)

            testObject1->setClientRole(true);
            testObject2->setClientRole(false);
          }
          break;
        }
        default:  quit = true; break;
      }
      if (quit) break;

      expecting = 0;
      expecting += (testObject1 ? 1 : 0);
      expecting += (testObject2 ? 1 : 0);

      ULONG found = 0;
      ULONG lastFound = 0;
      ULONG step = 0;

      bool lastStepReached = false;

      while ((found < expecting) ||
             (!lastStepReached))
      {
        TESTING_SLEEP(1000)
        ++step;
        if (step >= maxSteps) {
          TESTING_CHECK(false)
          break;
        }

        found = 0;

        switch (testNumber) {
          case TEST_BASIC_ROUTING: {
            switch (step) {
              case 2: {
                if (testObject1) testObject1->connect(testObject2);
            //  bogusSleep();
                break;
              }
              case 3: {
                if (testObject1) testObject1->state(IICETransport::State_Completed);
                if (testObject2) testObject2->state(IICETransport::State_Completed);
            //  bogusSleep();
                break;
              }
              case 4: {
                if (testObject1) testObject1->state(IDTLSTransportTypes::State_Connected);
                if (testObject2) testObject2->state(IDTLSTransportTypes::State_Connected);
            //  bogusSleep();
                break;
              }
              case 5: {
                testObject1->expectKind(IMediaStreamTrackTypes::Kind_Audio);
                {
                  Parameters params;
                  testObject1->store("params-empty", params);
                  testObject2->store("params-empty", params);
                }
                {
                  Parameters params;
                  params.mMuxID = "r1";

                  CodecParameters codec;
                  codec.mName = "opus";
                  codec.mClockRate = 48000;
                  codec.mPayloadType = 96;

                  params.mCodecs.push_back(codec);

                  IRTPTypes::HeaderExtensionParameters headerParams;
                  headerParams.mID = 1;
                  headerParams.mURI = IRTPTypes::toString(IRTPTypes::HeaderExtensionURI_MuxID);
                  params.mHeaderExtensions.push_back(headerParams);
                  headerParams.mID = 2;
                  headerParams.mURI = IRTPTypes::toString(IRTPTypes::HeaderExtensionURI_RID);
                  params.mHeaderExtensions.push_back(headerParams);
                  headerParams.mID = 3;
                  headerParams.mURI = IRTPTypes::toString(IRTPTypes::HeaderExtensionURI_ClienttoMixerAudioLevelIndication);
                  params.mHeaderExtensions.push_back(headerParams);
                  headerParams.mID = 4;
                  headerParams.mURI = IRTPTypes::toString(IRTPTypes::HeaderExtensionURI_MixertoClientAudioLevelIndication);
                  params.mHeaderExtensions.push_back(headerParams);

                  testObject1->store("params1", params);
                }
          //    bogusSleep();
                break;
              }
              case 6: {
                testObject1->createSenderChannel("c1", "params1");
                testObject1->expectState("c1", ISecureTransportTypes::State_Connected);
                testObject1->expectState("c1", ISecureTransportTypes::State_Closed);
                testObject1->expectAttachChannel("c1");
                testObject1->send("params1");
          //    bogusSleep();
                break;
              }
              case 7: {
                Parameters params;
                testObject2->receive("r1", "params-empty");
          //    bogusSleep();
                break;
              }
              case 8:
              {
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 96;
                  params.mSequenceNumber = 1;
                  params.mTimestamp = 10000;
                  params.mSSRC = 5;
                  DWORD csrcs[] = {77,88};
                  params.mCSRCList = csrcs;
                  params.mCC = sizeof(csrcs)/sizeof(DWORD);
                  const char *payload = "sparksthesoftware";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacket::MidHeaderExtension mid1(1, "r1");
                  RTPPacket::MidHeaderExtension rid1(2, "c1");
                  RTPPacket::ClientToMixerExtension ctm1(3, true, 2);
                  BYTE levels[] = {4,15};
                  RTPPacket::MixerToClientExtension mtc1(4, levels, sizeof(levels)/sizeof(BYTE));

                  params.mFirstHeaderExtension = &mid1;
                  mid1.mNext = &rid1;
                  rid1.mNext = &ctm1;
                  ctm1.mNext = &mtc1;

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p1", packet);
                  testObject2->store("p1", packet);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 96;
                  params.mSequenceNumber = 2;
                  params.mTimestamp = 10001;
                  params.mSSRC = 5;
                  const char *payload = "sparksthesoftware2";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p2", packet);
                  testObject2->store("p2", packet);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 97;
                  params.mSequenceNumber = 3;
                  params.mTimestamp = 10002;
                  params.mSSRC = 6;
                  const char *payload = "sparksthesoftware3";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacket::MidHeaderExtension mid1(1, "r2");
                  RTPPacket::MidHeaderExtension rid1(2, "c2");

                  params.mFirstHeaderExtension = &mid1;
                  mid1.mNext = &rid1;

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p3", packet);
                  testObject2->store("p3", packet);
                }
          //    bogusSleep();
                break;
              }
              case 9: {
          //    bogusSleep();
                break;
              }
              case 10: {
                testObject2->expectPacket("r1","p1");
                testObject2->expectPacket("r1","p1");
                testObject2->expectPacket("r1","p2");
          //    bogusSleep();
                break;
              }
              case 11: {
                testObject1->sendPacket("c1", "p1");
                testObject1->sendPacket("c1", "p1");
                testObject1->sendPacket("c1", "p2");
          //    bogusSleep();
                break;
              }
              case 12: {
                testObject2->expectPacket("r1","p3");
                testObject1->sendPacket("c1", "p3");
          //    bogusSleep();
                break;
              }
              case 14: {
                auto params = testObject1->getParameters("params1");

                CodecParameters codec;
                codec.mName = "isac";
                codec.mClockRate = 32000;
                codec.mPayloadType = 97;

                params->mCodecs.push_back(codec);

                EncodingParameters encoding;
                encoding.mCodecPayloadType = 96;
                params->mEncodings.push_back(encoding);

                encoding.mCodecPayloadType = 97;
                params->mEncodings.push_back(encoding);

                testObject1->store("params2", *params);

                auto oldEncodingsList = params->mEncodings;

                {
                  params->mEncodings = oldEncodingsList;
                  params->mEncodings.pop_back();
                  testObject1->store("params2-c2", *params);
                }
                {
                  params->mEncodings = oldEncodingsList;
                  params->mEncodings.pop_front();
                  testObject1->store("params2-c3", *params);
                }

          //    bogusSleep();
                break;
              }
              case 15: {
                testObject1->createSenderChannel("c2", "params2-c2");
                testObject1->expectState("c2", ISecureTransportTypes::State_Connected);
                testObject1->expectState("c2", ISecureTransportTypes::State_Closed);
                testObject1->createSenderChannel("c3", "params2-c3");
                testObject1->expectState("c3", ISecureTransportTypes::State_Connected);
                testObject1->expectState("c3", ISecureTransportTypes::State_Closed);
                testObject1->expectDetachChannel("c1");
                testObject1->expectAttachChannel("c2");
                testObject1->expectAttachChannel("c3");

                testObject1->send("params2");

          //    bogusSleep();
                break;
              }
              case 20: {
                testObject1->expectDetachChannel("c2");
                testObject1->expectDetachChannel("c3");

                if (testObject1) testObject1->close();
                if (testObject2) testObject1->close();
          //    bogusSleep();
                break;
              }
              case 21: {
                if (testObject1) testObject1->state(IDTLSTransportTypes::State_Closed);
                if (testObject2) testObject2->state(IDTLSTransportTypes::State_Closed);
          //    bogusSleep();
                break;
              }
              case 22: {
                if (testObject1) testObject1->state(IICETransport::State_Disconnected);
                if (testObject2) testObject2->state(IICETransport::State_Disconnected);
          //    bogusSleep();
                break;
              }
              case 23: {
                if (testObject1) testObject1->state(IICETransport::State_Closed);
                if (testObject2) testObject2->state(IICETransport::State_Closed);
          //    bogusSleep();
                break;
              }
              case 24: {
                if (testObject1) testObject1->closeByReset();
                if (testObject2) testObject2->closeByReset();
          //    bogusSleep();
                break;
              }
              case 25: {
                lastStepReached = true;
                break;
              }
              default: {
                // nothing happening in this step
                break;
              }
            }
            break;
          }
          case TEST_SSRC_ROUTING: {
            switch (step) {
              case 2: {
                if (testObject1) testObject1->connect(testObject2);
          //    bogusSleep();
                break;
              }
              case 3: {
                if (testObject1) testObject1->state(IICETransport::State_Completed);
                if (testObject2) testObject2->state(IICETransport::State_Completed);
          //    bogusSleep();
                break;
              }
              case 4: {
                testObject1->expectKind(IMediaStreamTrackTypes::Kind_Audio);
                {
                  Parameters params;
                  testObject1->store("params-empty", params);
                  testObject2->store("params-empty", params);
                }
                {
                  Parameters params;

                  CodecParameters codec;
                  codec.mName = IRTPTypes::toString(IRTPTypes::SupportedCodec_Opus);
                  codec.mClockRate = 48000;
                  codec.mPayloadType = 96;
                  params.mCodecs.push_back(codec);

                  codec.mName = IRTPTypes::toString(IRTPTypes::SupportedCodec_ILBC);
                  codec.mClockRate = 48000;
                  codec.mPayloadType = 97;
                  params.mCodecs.push_back(codec);

                  IRTPTypes::RTXCodecParameters rtxCodecParams;
                  rtxCodecParams.mRTXTime = Milliseconds(100);

                  codec.mName = IRTPTypes::toString(IRTPTypes::SupportedCodec_RTX);
                  codec.mClockRate = 90000;
                  codec.mPayloadType = 120;
                  rtxCodecParams.mApt = 97;
                  codec.mParameters = IRTPTypes::RTXCodecParameters::create(rtxCodecParams);
                  params.mCodecs.push_back(codec);  // not used

                  codec.mName = IRTPTypes::toString(IRTPTypes::SupportedCodec_RTX);
                  codec.mClockRate = 48000;
                  codec.mPayloadType = 121;
                  rtxCodecParams.mApt = 96;
                  codec.mParameters = IRTPTypes::RTXCodecParameters::create(rtxCodecParams);
                  params.mCodecs.push_back(codec);

                  codec.mName = IRTPTypes::toString(IRTPTypes::SupportedCodec_ULPFEC);
                  codec.mClockRate = 90000;
                  codec.mPayloadType = 122;
                  params.mCodecs.push_back(codec);  // not used

                  codec.mName = IRTPTypes::toString(IRTPTypes::SupportedCodec_ULPFEC);
                  codec.mClockRate = 48000;
                  codec.mPayloadType = 123;
                  params.mCodecs.push_back(codec);

                  IRTPTypes::REDCodecParameters redCodecParams1;
                  redCodecParams1.mPayloadTypes.push_back(122);
                  IRTPTypes::REDCodecParameters redCodecParams2;
                  redCodecParams2.mPayloadTypes.push_back(123);

                  codec.mName = IRTPTypes::toString(IRTPTypes::SupportedCodec_RED);
                  codec.mClockRate = 90000;
                  codec.mPayloadType = 124;
                  codec.mParameters = IRTPTypes::REDCodecParameters::create(redCodecParams1);
                  params.mCodecs.push_back(codec);  // red herring -- get it?? get it??? *sigh*

                  codec.mName = IRTPTypes::toString(IRTPTypes::SupportedCodec_RED);
                  codec.mClockRate = 48000;
                  codec.mPayloadType = 125;
                  codec.mParameters = IRTPTypes::REDCodecParameters::create(redCodecParams2);
                  params.mCodecs.push_back(codec);

                  EncodingParameters encoding;
                  encoding.mSSRC = 19;
                  encoding.mCodecPayloadType = 96;

                  RTXParameters rtx;
                  rtx.mSSRC = 23;
                  encoding.mRTX = rtx;

                  FECParameters fec;
                  fec.mSSRC = 31;
                  fec.mMechanism = IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC);
                  encoding.mFEC = fec;

                  params.mEncodings.push_back(encoding);

                  testObject1->store("params1", params);
                }
          //    bogusSleep();
                break;
              }
              case 5:
              {
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 96;
                  params.mSequenceNumber = 1;
                  params.mTimestamp = 10000;
                  params.mSSRC = 19;
                  const char *payload = "makemyday";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p1", packet);
                  testObject2->store("p1", packet);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 121;
                  params.mSequenceNumber = 2;
                  params.mTimestamp = 10001;
                  params.mSSRC = 23;
                  const char *payload = "Hasta la vista Baby!";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p2", packet);
                  testObject2->store("p2", packet);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 125;
                  params.mSequenceNumber = 3;
                  params.mTimestamp = 10002;
                  params.mSSRC = 31;
                  const char *payload = "Monster high";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p3", packet);
                  testObject2->store("p3", packet);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 125;
                  params.mSequenceNumber = 3;
                  params.mTimestamp = 10002;
                  params.mSSRC = 37;
                  const char *payload = "Totally radical fec";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p4", packet);
                  testObject2->store("p4", packet);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 96;
                  params.mSequenceNumber = 100;
                  params.mTimestamp = 10003;
                  params.mSSRC = 11;
                  const char *payload = "Totally radical opus";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p5", packet);
                  testObject2->store("p5", packet);
                }
          //    bogusSleep();
                break;
              }
              case 6: {
                if (testObject2) testObject2->state(IDTLSTransportTypes::State_Connected);
                if (testObject1) testObject1->state(IDTLSTransportTypes::State_Connected);
                testObject2->createReceiver("r1");
                testObject2->receive("r1", "params-empty");
          //    bogusSleep();
                break;
              }
              case 10: {
                testObject1->createSenderChannel("c1", "params1");
                testObject1->expectState("c1", ISecureTransportTypes::State_Connected);
                testObject1->expectAttachChannel("c1");
                testObject1->send("params1");
                testObject2->expectPacket("r1", "p1");
                testObject1->sendPacket("c1", "p1");
          //    bogusSleep();
                break;
              }
              case 11: {
                testObject2->expectPacket("r1", "p2");
                testObject1->sendPacket("c1", "p2");
          //    bogusSleep();
                break;
              }
              case 12: {
                testObject2->expectPacket("r1", "p4");
                testObject2->expectPacket("r1", "p5");
                testObject2->expectPacket("r1", "p4");
                testObject2->expectPacket("r1", "p5");
                testObject1->sendPacket("c1", "p4");
                testObject1->sendPacket("c1", "p5");
                testObject1->sendPacket("c1", "p4");
                testObject1->sendPacket("c1", "p5");
          //    bogusSleep();
                break;
              }
              case 13: {
                testObject2->expectPacket("r1", "p3");
                testObject1->sendPacket("c1", "p3");
          //    bogusSleep();
                break;
              }
              case 14: {
                {
                  auto params = testObject1->getParameters("params1");
                  EncodingParameters encoding;
                  encoding.mCodecPayloadType = 96;

                  RTXParameters rtx;
                  encoding.mRTX = rtx;

                  FECParameters fec;
                  fec.mMechanism = IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC);
                  encoding.mFEC = fec;

                  params->mEncodings.push_back(encoding);

                  testObject1->store("params2", *params);
                }
                {
                  auto params = testObject1->getParameters("params2");
                  params->mEncodings.pop_front();
                  testObject1->store("params2-c2", *params);  // only a single encoding will go to this new channel
                }
          //    bogusSleep();
                break;
              }
              case 15: {
                testObject1->createSenderChannel("c2", "params2-c2");
                testObject1->expectState("c2", ISecureTransportTypes::State_Connected);
                testObject1->expectAttachChannel("c2");
                testObject1->send("params2");
          //    bogusSleep();
                break;
              }
              case 16: {
                {
                  auto params = testObject1->getParameters("params2");
                  auto &encoding = (*(params->mEncodings.begin()));
                  encoding.mSSRC = 61;
                  testObject1->store("params3", *params);
                }
                {
                  auto params = testObject1->getParameters("params3");
                  params->mEncodings.pop_back();
                  testObject1->store("params3-c3", *params);  // only a single encoding will go to this new channel
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 96;
                  params.mSequenceNumber = 1;
                  params.mTimestamp = 10000;
                  params.mSSRC = 61;
                  const char *payload = "flash the message";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p6", packet);
                  testObject2->store("p6", packet);
                }
                testObject1->createSenderChannel("c3", "params3-c3");
                testObject1->expectState("c1", ISecureTransportTypes::State_Closed);
                testObject1->expectState("c3", ISecureTransportTypes::State_Connected);
                testObject1->expectDetachChannel("c1");
                testObject1->expectAttachChannel("c3");
                testObject1->send("params3");
          //    bogusSleep();
                break;
              }
              case 17: {
                testObject2->expectPacket("r1", "p6");
                // testObject1->expectActiveChannel("c3");  // no - will not become active because switch lockout
                testObject1->sendPacket("c3", "p6");
          //    bogusSleep();
                break;
              }
              case 18: {
                {
                  auto params = testObject1->getParameters("params3");

                  EncodingParameters encoding;
                  encoding.mEncodingID = "enc1";
                  encoding.mCodecPayloadType = 96;
                  encoding.mSSRC = 79;

                  RTXParameters rtx;
                  rtx.mSSRC = 151;
                  encoding.mRTX = rtx;

                  FECParameters fec;
                  fec.mMechanism = IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC);
                  fec.mSSRC = 149;
                  encoding.mFEC = fec;

                  params->mEncodings.push_back(encoding);

                  testObject1->store("params4", *params);
                }
                {
                  auto params = testObject1->getParameters("params4");
                  while (params->mEncodings.size() > 1) {
                    params->mEncodings.pop_front();
                  }
                  testObject1->store("params4-c4", *params);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 96;
                  params.mSequenceNumber = 1;
                  params.mTimestamp = 10000;
                  params.mSSRC = 79;
                  const char *payload = "somethings out there";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p7", packet);
                  testObject2->store("p7", packet);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 125;
                  params.mSequenceNumber = 2;
                  params.mTimestamp = 10001;
                  params.mSSRC = 149;
                  const char *payload = "captain kirk";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p8", packet);
                  testObject2->store("p8", packet);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 121;
                  params.mSequenceNumber = 3;
                  params.mTimestamp = 10002;
                  params.mSSRC = 151;
                  const char *payload = "captain picard";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p9", packet);
                  testObject2->store("p9", packet);
                }
                testObject1->createSenderChannel("c4", "params4-c4");
                testObject1->expectAttachChannel("c4");
                testObject1->expectState("c4", ISecureTransportTypes::State_Connected);
                testObject1->send("params4");
          //    bogusSleep();
                break;
              }
              case 20: {
                testObject2->expectPacket("r1", "p9");
                testObject2->expectPacket("r1", "p8");
                testObject2->expectPacket("r1", "p7");
                testObject1->sendPacket("c4", "p9");
                testObject1->sendPacket("c4", "p8");
                testObject1->sendPacket("c4", "p7");
          //    bogusSleep();
                break;
              }
              case 21: {
                {
                  auto params = testObject1->getParameters("params4");

                  auto &encoding = params->mEncodings.back();

                  encoding.mRTX.value().mSSRC = 131;
                  encoding.mFEC.value().mSSRC = 137;

                  testObject1->store("params5", *params);
                }
                {
                  auto params = testObject1->getParameters("params5");
                  while (params->mEncodings.size() > 1) {
                    params->mEncodings.pop_front();
                  }
                  testObject1->store("params5-c4", *params);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 125;
                  params.mSequenceNumber = 2;
                  params.mTimestamp = 10001;
                  params.mSSRC = 131;
                  const char *payload = "five ten";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p10", packet);
                  testObject2->store("p10", packet);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 121;
                  params.mSequenceNumber = 3;
                  params.mTimestamp = 10002;
                  params.mSSRC = 137;
                  const char *payload = "fifteen twenty";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p11", packet);
                  testObject2->store("p11", packet);
                }
                testObject1->expectReceiveChannelUpdate("c4", "params5-c4");
                testObject1->send("params5");
          //    bogusSleep();
                break;
              }
              case 22: {
                testObject2->expectPacket("r1", "p11");
                testObject2->expectPacket("r1", "p10");
                testObject2->expectPacket("r1", "p7");
                testObject1->sendPacket("c4", "p11");
                testObject1->sendPacket("c4", "p10");
                testObject1->sendPacket("c4", "p7");
          //    bogusSleep();
                break;
              }
              case 23: {
                {
                  auto params = testObject1->getParameters("params5");
                  auto &encoding = params->mEncodings.back();
                  encoding.mCodecPayloadType = 97;
                  testObject1->store("params6", *params);
                }
                {
                  auto params = testObject1->getParameters("params6");
                  while (params->mEncodings.size() > 1) {
                    params->mEncodings.pop_front();
                  }
                  testObject1->store("params6-c5", *params);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 97;
                  params.mSequenceNumber = 4;
                  params.mTimestamp = 10003;
                  params.mSSRC = 79;
                  const char *payload = "one two three four five";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p12", packet);
                  testObject2->store("p12", packet);
                }
                testObject1->expectState("c4", ISecureTransportTypes::State_Closed);
                testObject1->expectDetachChannel("c4");
                testObject1->createSenderChannel("c5", "params6-c5");
                testObject1->expectState("c5", ISecureTransportTypes::State_Connected);
                testObject1->expectAttachChannel("c5");
                testObject1->send("params6");
          //    bogusSleep();
                break;
              }
              case 24: {
                testObject2->expectPacket("r1", "p11");
                testObject2->expectPacket("r1", "p10");
                testObject2->expectPacket("r1", "p12");
                testObject1->sendPacket("c5", "p11");
                testObject1->sendPacket("c5", "p10");
                testObject1->sendPacket("c5", "p12");
          //    bogusSleep();
                break;
              }
              case 25: {
                {
                  auto params = testObject1->getParameters("params6");
                  params->mRTCP.mSSRC = 271;
                  testObject1->store("params7", *params);
                }
                {
                  auto params = testObject1->getParameters("params7");
                  params->mEncodings.pop_front();
                  params->mEncodings.pop_back();
                  testObject1->store("params7-c2", *params);
                }
                {
                  auto params = testObject1->getParameters("params7");
                  while (params->mEncodings.size() > 1) {
                    params->mEncodings.pop_back();
                  }
                  testObject1->store("params7-c3", *params);
                }
                {
                  auto params = testObject1->getParameters("params7");
                  while (params->mEncodings.size() > 1) {
                    params->mEncodings.pop_front();
                  }
                  testObject1->store("params7-c5", *params);
                }
                testObject1->expectReceiveChannelUpdate("c5", "params7-c5");
                testObject1->expectReceiveChannelUpdate("c2", "params7-c2");
                testObject1->expectReceiveChannelUpdate("c3", "params7-c3");
                testObject1->send("params7");
          //    bogusSleep();
                break;
              }
              case 26: {
                testObject2->expectPacket("r1", "p11");
                testObject2->expectPacket("r1", "p10");
                testObject2->expectPacket("r1", "p12");
                testObject1->sendPacket("c5", "p11");
                testObject1->sendPacket("c5", "p10");
                testObject1->sendPacket("c5", "p12");
          //    bogusSleep();
                break;
              }
              case 27: {
                {
                  auto params = testObject1->getParameters("params7");

                  EncodingParameters encoding;
                  encoding.mEncodingID = "enc2";
                  encoding.mCodecPayloadType = 96;

                  RTXParameters rtx;

                  FECParameters fec;
                  fec.mMechanism = IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC);

                  encoding.mRTX = rtx;
                  encoding.mFEC = fec;

                  params->mEncodings.push_back(encoding);

                  testObject1->store("params8", *params);
                }
                {
                  auto params = testObject1->getParameters("params8");
                  while (params->mEncodings.size() > 1) {
                    params->mEncodings.pop_front();
                  }
                  testObject1->store("params8-c6", *params);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 96;
                  params.mSequenceNumber = 6;
                  params.mTimestamp = 9001;
                  params.mSSRC = 173;
                  const char *payload = "milk";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p13", packet);
                  testObject2->store("p13", packet);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 125;
                  params.mSequenceNumber = 7;
                  params.mTimestamp = 9002;
                  params.mSSRC = 179;
                  const char *payload = "butter";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p14", packet);
                  testObject2->store("p14", packet);
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 121;
                  params.mSequenceNumber = 8;
                  params.mTimestamp = 9003;
                  params.mSSRC = 199;
                  const char *payload = "eggs";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  RTPPacketPtr packet = RTPPacket::create(params);
                  testObject1->store("p15", packet);
                  testObject2->store("p15", packet);
                }
                testObject1->createSenderChannel("c6", "params8-c6");
                testObject1->expectState("c6", ISecureTransportTypes::State_Connected);
                testObject1->expectAttachChannel("c6");
                testObject1->send("params8");
          //    bogusSleep();
                break;
              }
              case 28: {
                testObject2->expectPacket("r1", "p13");
                testObject2->expectPacket("r1", "p14");
                testObject2->expectPacket("r1", "p15");
                testObject1->sendPacket("c6", "p13");
                testObject1->sendPacket("c6", "p14");
                testObject1->sendPacket("c6", "p15");
          //    bogusSleep();
                break;
              }
              case 29: {
                {
                  auto params = testObject1->getParameters("params8");

                  EncodingParameters encoding;
                  encoding.mEncodingID = "enc3";
                  encoding.mCodecPayloadType = 96;
                  encoding.mSSRC = 229;

                  RTXParameters rtx;
                  rtx.mSSRC = 233;

                  FECParameters fec;
                  fec.mMechanism = IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC);
                  fec.mSSRC = 239;

                  encoding.mRTX = rtx;
                  encoding.mFEC = fec;

                  params->mEncodings.push_back(encoding);

                  IRTPTypes::HeaderExtensionParameters headerParams;
                  headerParams.mID = 1;
                  headerParams.mURI = IRTPTypes::toString(IRTPTypes::HeaderExtensionURI_MuxID);
                  params->mHeaderExtensions.push_back(headerParams);
                  headerParams.mID = 2;
                  headerParams.mURI = IRTPTypes::toString(IRTPTypes::HeaderExtensionURI_RID);
                  params->mHeaderExtensions.push_back(headerParams);
                  headerParams.mID = 3;
                  headerParams.mURI = IRTPTypes::toString(IRTPTypes::HeaderExtensionURI_ClienttoMixerAudioLevelIndication);
                  params->mHeaderExtensions.push_back(headerParams);
                  headerParams.mID = 4;
                  headerParams.mURI = IRTPTypes::toString(IRTPTypes::HeaderExtensionURI_MixertoClientAudioLevelIndication);
                  params->mHeaderExtensions.push_back(headerParams);

                  testObject1->store("params9", *params);
                }
                {
                  auto params = testObject1->getParameters("params9");
                  while (params->mEncodings.size() > 1) {
                    params->mEncodings.pop_front();
                  }
                  testObject1->store("params9-c7", *params);
                }
                {
                  auto sourceParams = testObject1->getParameters("params9");
                  {
                    auto params = testObject1->getParameters("params7-c2");
                    params->mHeaderExtensions = sourceParams->mHeaderExtensions;
                    testObject1->store("params9-c2", *params);
                  }
                  {
                    auto params = testObject1->getParameters("params7-c3");
                    params->mHeaderExtensions = sourceParams->mHeaderExtensions;
                    testObject1->store("params9-c3", *params);
                  }
                  {
                    auto params = testObject1->getParameters("params7-c5");
                    params->mHeaderExtensions = sourceParams->mHeaderExtensions;
                    testObject1->store("params9-c5", *params);
                  }
                  {
                    auto params = testObject1->getParameters("params8-c6");
                    params->mHeaderExtensions = sourceParams->mHeaderExtensions;
                    testObject1->store("params9-c6", *params);
                  }
                }
                {
                  RTPPacket::CreationParams params;
                  params.mPT = 96;
                  params.mSequenceNumber = 6;
                  params.mTimestamp = 9001;
                  params.mSSRC = 229;
                  const char *payload = "wetlands";
                  params.mPayload = reinterpret_cast<const BYTE *>(payload);
                  params.mPayloadSize = strlen(payload);

                  {
                    RTPPacket::RidHeaderExtension rid(2, "enc3");
                    params.mFirstHeaderExtension = &rid;

                    RTPPacketPtr packet = RTPPacket::create(params);
                    testObject1->store("p16", packet);
                    testObject2->store("p16", packet);
                  }
                  {
                    params.mPT = 121;
                    params.mSSRC = 233;

                    RTPPacket::RidHeaderExtension rid(2, "enc3-no");
                    params.mFirstHeaderExtension = &rid;

                    RTPPacketPtr packet = RTPPacket::create(params);
                    testObject1->store("p17", packet);
                    testObject2->store("p17", packet);
                  }
                }
                testObject1->expectReceiveChannelUpdate("c2", "params9-c2");
                testObject1->expectReceiveChannelUpdate("c3", "params9-c3");
                testObject1->expectReceiveChannelUpdate("c5", "params9-c5");
                testObject1->expectReceiveChannelUpdate("c6", "params9-c6");
                testObject1->createSenderChannel("c7", "params9-c7");
                testObject1->expectState("c7", ISecureTransportTypes::State_Connected);
                testObject1->expectAttachChannel("c7");
                testObject1->send("params9");
          //    bogusSleep();
                break;
              }
              case 30: {
                testObject2->expectPacket("r1", "p16");
                testObject1->sendPacket("c7", "p16");
                testObject2->expectPacket("r1", "p17"); // will latch the "c2" which does not have RTX filled in and "c7" rid does not match (nor does "c2" have an encoding ID)
                testObject1->sendPacket("c2", "p17");
          //    bogusSleep();
                break;
              }
              case 35: {
                testObject1->expectDetachChannel("c3");
                testObject1->expectDetachChannel("c2");
                testObject1->expectDetachChannel("c5");
                testObject1->expectDetachChannel("c6");
                testObject1->expectDetachChannel("c7");
                testObject1->expectState("c2", ISecureTransportTypes::State_Closed);
                testObject1->expectState("c3", ISecureTransportTypes::State_Closed);
                testObject1->expectState("c5", ISecureTransportTypes::State_Closed);
                testObject1->expectState("c6", ISecureTransportTypes::State_Closed);
                testObject1->expectState("c7", ISecureTransportTypes::State_Closed);

                if (testObject1) testObject1->close();
                if (testObject2) testObject1->close();
          //    bogusSleep();
                break;
              }
              case 36: {
                if (testObject1) testObject1->state(IDTLSTransportTypes::State_Closed);
                if (testObject2) testObject2->state(IDTLSTransportTypes::State_Closed);
          //    bogusSleep();
                break;
              }
              case 37: {
                if (testObject1) testObject1->state(IICETransport::State_Disconnected);
                if (testObject2) testObject2->state(IICETransport::State_Disconnected);
          //    bogusSleep();
                break;
              }
              case 38: {
                if (testObject1) testObject1->state(IICETransport::State_Closed);
                if (testObject2) testObject2->state(IICETransport::State_Closed);
          //    bogusSleep();
                break;
              }
              case 39: {
                if (testObject1) testObject1->closeByReset();
                if (testObject2) testObject2->closeByReset();
          //    bogusSleep();
                break;
              }
              case 40: {
                lastStepReached = true;
                break;
              }
              default: {
                // nothing happening in this step
                break;
              }
            }
            break;
          }
          default: {
            // none defined
            break;
          }
        }

        if (0 == found) {
          found += (testObject1 ? (testObject1->matches() ? 1 : 0) : 0);
          found += (testObject2 ? (testObject2->matches() ? 1 : 0) : 0);
        }

        if (lastFound != found) {
          lastFound = found;
          TESTING_STDOUT() << "FOUND:        [" << found << "].\n";
        }
      }

      TESTING_EQUAL(found, expecting)

      TESTING_SLEEP(2000)

      //switch (testNumber) {
      //  default:
        {
          if (testObject1) {TESTING_CHECK(testObject1->matches())}
          if (testObject2) {TESTING_CHECK(testObject2->matches())}
      //    break;
        }
      //}

      testObject1.reset();
      testObject2.reset();

      ++testNumber;
    } while (true);
  }

  TESTING_STDOUT() << "WAITING:      All RTPSender transports have finished. Waiting for 'bogus' events to process (1 second wait).\n";
  TESTING_SLEEP(1000)

  // wait for shutdown
  {
    IMessageQueue::size_type count = 0;
    do
    {
      count = thread->getTotalUnprocessedMessages();
      if (0 != count)
        std::this_thread::yield();
    } while (count > 0);

    thread->waitForShutdown();
  }
  TESTING_UNINSTALL_LOGGER();
  zsLib::proxyDump();
  TESTING_EQUAL(zsLib::proxyGetTotalConstructed(), 0);
}

