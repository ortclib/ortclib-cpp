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


#include "TestRTPChannel.h"

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
    namespace rtpchannel
    {
      ZS_DECLARE_CLASS_PTR(FakeICETransport)
      ZS_DECLARE_CLASS_PTR(FakeSecureTransport)
      ZS_DECLARE_CLASS_PTR(RTPChannelTester)

      ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportTypes, ISecureTransportTypes)
      ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransport, ISecureTransport)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiver)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiverChannel)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiverChannelAudio)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiverChannelVideo)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSender)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSenderChannel)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSenderChannelAudio)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSenderChannelVideo)
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

        ElementPtr resultEl = Element::create("ortc::test::rtpchannel::FakeICETransport");

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
            ZS_LOG_WARNING(Detail, log("no rtpchannel transport attached (thus cannot forward received packet)") + ZS_PARAM("buffer", (PTRNUMBER)(buffer->BytePtr())) + ZS_PARAM("buffer size", buffer->SizeInBytes()))
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
            ZS_LOG_WARNING(Detail, log("no rtpchannel transport attached (thus cannot forward delayed packets)"))
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
        ElementPtr objectEl = Element::create("ortc::test::rtpchannel::FakeICETransport");
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

        ElementPtr resultEl = Element::create("ortc::test::rtpchannel::FakeICETransport");

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
      #pragma mark FakeSecureTransport => ISecureTransportForRTPReceiver
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
      #pragma mark FakeSecureTransport => ISecureTransportForRTPReceiver
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
        ElementPtr objectEl = Element::create("ortc::test::rtpchannel::FakeSecureTransport");
        UseServicesHelper::debugAppend(objectEl, "id", DTLSTransport::getID());
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::cancel()
      {
        setState(IDTLSTransportTypes::State_Closed);

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

        ElementPtr result = Element::create("ortc::test::rtpchannel::FakeListener");

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());

        UseServicesHelper::debugAppend(result, "subscriptions", mSubscriptions.size());
        UseServicesHelper::debugAppend(result, "default subscription", (bool)mDefaultSubscription);

        UseServicesHelper::debugAppend(result, "clean timer", mCleanBuffersTimer ? mCleanBuffersTimer->getID() : 0);

        UseServicesHelper::debugAppend(result, "buffered rtp packets", mBufferedRTPPackets.size());
        UseServicesHelper::debugAppend(result, "buffered rtcp packets", mBufferedRTCPPackets.size());

        auto receiver = mReceiver.lock();
        UseServicesHelper::debugAppend(result, "receiver", receiver ? receiver->getID() : 0);

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

        for (auto iter = inParams.mHeaderExtensions.begin(); iter != inParams.mHeaderExtensions.end(); ++iter)
        {
          auto &ext = (*iter);
          switch (IRTPTypes::toHeaderExtensionURI(ext.mURI)) {
            case IRTPTypes::HeaderExtensionURI_MID: {
              mMuxIDHeaderExtension = ext.mID;
              break;
            }
            default: {
              break;
            }
          }
        }

        // MuxID == MID - FYI

        if (inParams.mMuxID.hasData()) {
          mMuxIDToReceivers[inParams.mMuxID] = inReceiver;
        }

        if (inParams.mEncodings.size() > 0) {
          auto &encoding = inParams.mEncodings.front();
          if (encoding.mCodecPayloadType.hasValue()) {
            mPayloadTypesToReceivers[encoding.mCodecPayloadType] = inReceiver;
          }
        }

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

        {
          auto receiver = mReceiver.lock();
          if (receiver) {
            if (receiver->getID() == inReceiver.getID()) {
              mReceiver.reset();
            }
          } else {
            mReceiver.reset();
          }
        }

        for (auto iter_doNotUse = mMuxIDToReceivers.begin(); iter_doNotUse != mMuxIDToReceivers.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto receiver = (*current).second.lock();
          if (!receiver) {
            mMuxIDToReceivers.erase(current);
            continue;
          }

          if (receiver->getID() != inReceiver.getID()) continue;

          mMuxIDToReceivers.erase(current);
        }

        for (auto iter_doNotUse = mPayloadTypesToReceivers.begin(); iter_doNotUse != mPayloadTypesToReceivers.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto receiver = (*current).second.lock();
          if (!receiver) {
            mPayloadTypesToReceivers.erase(current);
            continue;
          }

          if (receiver->getID() != inReceiver.getID()) continue;

          mPayloadTypesToReceivers.erase(current);
        }
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
        // ignored - not called
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

          if (rtpPacket) {
            // step 1 - check if RTP packet matches by MuxID
            if (!receiver) {
              if (mMuxIDHeaderExtension.hasValue()) {
                size_t total = rtpPacket->headerExtensionSize();

                for (size_t index = 0; index < total; ++index) {
                  auto ext = rtpPacket->getHeaderExtensionAtIndex(index);
                  TESTING_CHECK(ext)

                  if (ext->mID != mMuxIDHeaderExtension.value()) continue;

                  RTPPacket::StringHeaderExtension strExt(*ext);
                  String muxID(strExt.str());

                  auto found = mMuxIDToReceivers.find(muxID);
                  if (found == mMuxIDToReceivers.end()) break;

                  receiver = (*found).second.lock();
                  break;
                }
              }
            }

            // step 2 - check for payload type match
            if (!receiver) {
              IRTPTypes::PayloadType payloadType = rtpPacket->pt();

              auto found = mPayloadTypesToReceivers.find(payloadType);
              if (found != mPayloadTypesToReceivers.end()) {
                receiver = (*found).second.lock();
              }
            }

            // step 3 - check if any receiver is attached
            if (!receiver) {
              receiver = mReceiver.lock();
            }

            if (!receiver) {
              // buffer the packet for later
              mBufferedRTPPackets.push_back(TimeRTPPacketPair(zsLib::now(), rtpPacket));
            }
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
      #pragma mark FakeListener => (friend RTPChannelTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeListener::setTransport(RTPChannelTesterPtr tester)
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
        ElementPtr objectEl = Element::create("ortc::test::rtpchannel::FakeListener");
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

        mSenders.clear();
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
      #pragma mark FakeMediaStreamTrack => IMediaStreamTrackForRTPReceiverChannel
      #pragma mark

#define MOSA_THESE_METHODS_ARE_CALLED_BY_YOUR_REAL_RECEIVER_CHANNEL_CLASS_TO_FAKE_MEDIA_TRACK 1
#define MOSA_THESE_METHODS_ARE_CALLED_BY_YOUR_REAL_RECEIVER_CHANNEL_CLASS_TO_FAKE_MEDIA_TRACK 2
#if 0
      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::renderVideoFrame(const webrtc::VideoFrame& videoFrame)
      {
      }
#endif
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => IMediaStreamTrackForRTPSenderChannel
      #pragma mark

#define MOSA_THESE_METHODS_ARE_CALLED_BY_YOUR_REAL_SENDER_CHANNEL_CLASS_TO_FAKE_MEDIA_TRACK 1
#define MOSA_THESE_METHODS_ARE_CALLED_BY_YOUR_REAL_SENDER_CHANNEL_CLASS_TO_FAKE_MEDIA_TRACK 2

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => (friend RTPChannelTester)
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
      void FakeMediaStreamTrack::setTransport(RTPChannelTesterPtr tester)
      {
        AutoRecursiveLock lock(*this);
        mTester = tester;
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
        ElementPtr objectEl = Element::create("ortc::test::rtpchannel::FakeMediaStreamTrack");
        UseServicesHelper::debugAppend(objectEl, "id", MediaStreamTrack::getID());
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      ElementPtr FakeMediaStreamTrack::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::rtpchannel::FakeMediaStreamTrack");

        UseServicesHelper::debugAppend(result, "kind", IMediaStreamTrackTypes::toString(mKind));

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());

        return result;
      }
      

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSender
      #pragma mark

      //-----------------------------------------------------------------------
      FakeSender::FakeSender(IMessageQueuePtr queue) :
        RTPSender(Noop(true), queue)
      {
      }

      //-----------------------------------------------------------------------
      FakeSender::~FakeSender()
      {
        mThisWeak.reset();
      }

      //-----------------------------------------------------------------------
      FakeSenderPtr FakeSender::create(IMessageQueuePtr queue)
      {
        FakeSenderPtr pThis(make_shared<FakeSender>(queue));
        pThis->mThisWeak = pThis;
        return pThis;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSender => IRTPSenderForRTPListener
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeSender::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::rtpchannel::FakeSender");

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());
        UseServicesHelper::debugAppend(result, mParameters ? mParameters->toDebug() : ElementPtr());
        UseServicesHelper::debugAppend(result, "expect buffers", mExpectBuffers.size());
        UseServicesHelper::debugAppend(result, "listener", mListener ? mListener->getID() : 0);
        UseServicesHelper::debugAppend(result, "secure transport", mSecureTransport ? mSecureTransport->getID() : 0);

        return result;
      }

      //-----------------------------------------------------------------------
      bool FakeSender::handlePacket(
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
      #pragma mark FakeSender => IRTPSenderForRTPListener
      #pragma mark

      //-----------------------------------------------------------------------
      bool FakeSender::sendPacket(RTPPacketPtr packet)
      {
        sendPacket(packet->buffer());
        return true;
      }

      //-----------------------------------------------------------------------
      bool FakeSender::sendPacket(RTCPPacketPtr packet)
      {
        sendPacket(packet->buffer());
        return true;
      }

      //-----------------------------------------------------------------------
      void FakeSender::notifyConflict(
                                      UseChannelPtr channel,
                                      IRTPTypes::SSRCType ssrc,
                                      bool selfDestruct
                                      )
      {
        RTPChannelTesterPtr tester;
        {
          AutoRecursiveLock lock(*this);
          tester = mTester.lock();
          TESTING_CHECK(tester)
        }
        tester->notifySenderChannelConflict();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSender => (friend RTPChannelTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeSender::setTransport(RTPChannelTesterPtr tester)
      {
        auto secureTransport = (((bool)tester) ? tester->getFakeSecureTransport() : FakeSecureTransportPtr());
        TESTING_CHECK(secureTransport)


        AutoRecursiveLock lock(*this);

        mTester = tester;

        if (mListener) {
          ZS_LOG_BASIC(log("unregistering receiver") + ZS_PARAM("listener", mListener->getID()))
          mListener->unregisterSender(*this);
        }

        if (secureTransport) {
          mListener = secureTransport->getListener();

          if (mParameters) {
            ZS_LOG_BASIC(log("registering receiver") + ZS_PARAM("listener", mListener->getID()) + mParameters->toDebug())

            RTCPPacketList packetList;
            mListener->registerSender(mThisWeak.lock(), *mParameters, packetList);

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
      void FakeSender::linkChannel(RTPSenderChannelPtr channel)
      {
        AutoRecursiveLock lock(*this);
        mSenderChannel = channel;
      }

      //-----------------------------------------------------------------------
      PromisePtr FakeSender::send(const Parameters &parameters)
      {
        AutoRecursiveLock lock(*this);

        mParameters = make_shared<Parameters>(parameters);

        if (mListener) {
          RTCPPacketList packetList;
          mListener->registerSender(mThisWeak.lock(), *mParameters, packetList);

          for (auto iter = packetList.begin(); iter != packetList.end(); ++iter) {
            // fake this as if it was a received packet
            handlePacket(IICETypes::Component_RTP, *iter);
          }
        }
        return Promise::createResolved(getAssociatedMessageQueue());
      }

      //-----------------------------------------------------------------------
      void FakeSender::stop()
      {
        AutoRecursiveLock lock(*this);

        if (mListener) {
          mListener->unregisterSender(*this);
        }
      }

      //-----------------------------------------------------------------------
      void FakeSender::expectData(SecureByteBlockPtr data)
      {
        TESTING_CHECK((bool)data)

        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("expecting buffer") + ZS_PARAM("buffer size", data->SizeInBytes()))

        mExpectBuffers.push_back(data);
      }

      //-----------------------------------------------------------------------
      void FakeSender::sendPacket(SecureByteBlockPtr buffer)
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
      #pragma mark FakeSender => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params FakeSender::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpchannel::FakeSender");
        UseServicesHelper::debugAppend(objectEl, "id", FakeSender::getID());
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
      #pragma mark FakeReceiver => IRTPSenderForRTPListener
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

        UseReceiverChannelPtr channel;

        {
          AutoRecursiveLock lock(*this);

          channel = mReceiverChannel;
          if (channel) goto skip_check;

          TESTING_CHECK(mExpectBuffers.size() > 0)

          TESTING_CHECK(0 == UseServicesHelper::compare(*(mExpectBuffers.front()), *(packet->mBuffer)))

          mExpectBuffers.pop_front();

          auto tester = mTester.lock();

          if (tester) tester->notifyReceivedPacket();
        }

      skip_check:
        if (channel) {
          channel->handlePacket(packet);
        }

        return true;
      }

      //-----------------------------------------------------------------------
      bool FakeReceiver::handlePacket(
                                      IICETypes::Components viaTransport,
                                      RTCPPacketPtr packet
                                      )
      {
        ZS_LOG_BASIC(log("received RTCP packet") + packet->toDebug())

        UseReceiverChannelPtr channel;

        {
          AutoRecursiveLock lock(*this);

          channel = mReceiverChannel;
          if (channel) goto skip_check;

          TESTING_CHECK(mExpectBuffers.size() > 0)

          TESTING_CHECK(0 == UseServicesHelper::compare(*(mExpectBuffers.front()), *(packet->mBuffer)))

          mExpectBuffers.pop_front();

          auto tester = mTester.lock();

          if (tester) tester->notifyReceivedPacket();
        }

      skip_check:
        if (channel) {
          channel->handlePacket(packet);
        }

        return true;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver => IRTPReceiverForRTPReceiverChannel
      #pragma mark

      //-----------------------------------------------------------------------
      bool FakeReceiver::sendPacket(RTCPPacketPtr packet)
      {
        sendPacket(packet->buffer());
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
      void FakeReceiver::setTransport(RTPChannelTesterPtr tester)
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
      void FakeReceiver::linkChannel(RTPReceiverChannelPtr channel)
      {
        AutoRecursiveLock lock(*this);
        mReceiverChannel = channel;
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
        ElementPtr objectEl = Element::create("ortc::test::rtpchannel::FakeReceiver");
        UseServicesHelper::debugAppend(objectEl, "id", FakeReceiver::getID());
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannelAudio
      #pragma mark

      //-----------------------------------------------------------------------
      FakeReceiverChannelAudio::FakeReceiverChannelAudio(
                                                         IMessageQueuePtr queue,
                                                         const char *mediaID,
                                                         const Parameters &params
                                                         ) :
        RTPReceiverChannelAudio(Noop(true), queue),
        mMediaID(mediaID),
        mParameters(make_shared<Parameters>(params))
      {
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelAudio::~FakeReceiverChannelAudio()
      {
        mThisWeak.reset();
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelAudioPtr FakeReceiverChannelAudio::create(
                                                                   IMessageQueuePtr queue,
                                                                   const char *mediaID,
                                                                   const Parameters &params
                                                                   )
      {
        FakeReceiverChannelAudioPtr pThis(make_shared<FakeReceiverChannelAudio>(queue, mediaID, params));
        pThis->mThisWeak = pThis;
        return pThis;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannelAudio => IRTPReceiverChannelMediaBaseForRTPReceiverChannel
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeReceiverChannelAudio::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::rtpsender::FakeReceiverChannelAudio");

        UseServicesHelper::debugAppend(result, "media id", mMediaID);
        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());
        UseServicesHelper::debugAppend(result, mParameters ? mParameters->toDebug() : ElementPtr());
        UseServicesHelper::debugAppend(result, "expect buffers", mExpectBuffers.size());
        auto receiverChannel = mReceiverChannel.lock();
        UseServicesHelper::debugAppend(result, "receiver channel", receiverChannel ? receiverChannel->getID() : 0);

        return result;
      }

      //-----------------------------------------------------------------------
      bool FakeReceiverChannelAudio::handlePacket(RTPPacketPtr packet)
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
      bool FakeReceiverChannelAudio::handlePacket(RTCPPacketPtr packet)
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
      #pragma mark FakeReceiverChannelAudio => IRTPReceiverChannelAudioForRTPReceiverChannel
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannelAudio => (friend RTPChannelTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeReceiverChannelAudio::setTransport(RTPChannelTesterPtr tester)
      {
        AutoRecursiveLock lock(*this);

        mTester = tester;
      }

      //-----------------------------------------------------------------------
      void FakeReceiverChannelAudio::create(
                                            RTPReceiverChannelPtr inReceiverChannel,
                                            const Parameters &params
                                            )
      {
        UseChannelPtr receiverChannel = inReceiverChannel;

        ZS_LOG_BASIC(log("create called") + ZS_PARAM("receiver channel", receiverChannel->getID()) + params.toDebug())

        AutoRecursiveLock lock(*this);
        mReceiverChannel = receiverChannel;

        TESTING_EQUAL(params.hash(), mParameters->hash())
      }

      //-----------------------------------------------------------------------
      void FakeReceiverChannelAudio::expectData(SecureByteBlockPtr data)
      {
        TESTING_CHECK((bool)data)

        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("expecting buffer") + ZS_PARAM("buffer size", data->SizeInBytes()))

        mExpectBuffers.push_back(data);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannelAudio => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params FakeReceiverChannelAudio::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpchannel::FakeReceiverChannelAudio");
        UseServicesHelper::debugAppend(objectEl, "id", FakeReceiverChannelAudio::getID());
        return Log::Params(message, objectEl);
      }



      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannelVideo
      #pragma mark

      //-----------------------------------------------------------------------
      FakeReceiverChannelVideo::FakeReceiverChannelVideo(
                                                         IMessageQueuePtr queue,
                                                         const char *mediaID,
                                                         const Parameters &params
                                                         ) :
        RTPReceiverChannelVideo(Noop(true), queue),
        mMediaID(mediaID),
        mParameters(make_shared<Parameters>(params))
      {
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelVideo::~FakeReceiverChannelVideo()
      {
        mThisWeak.reset();
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelVideoPtr FakeReceiverChannelVideo::create(
                                                                   IMessageQueuePtr queue,
                                                                   const char *mediaID,
                                                                   const Parameters &params
                                                                   )
      {
        FakeReceiverChannelVideoPtr pThis(make_shared<FakeReceiverChannelVideo>(queue, mediaID, params));
        pThis->mThisWeak = pThis;
        return pThis;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannelVideo => IRTPReceiverChannelMediaBaseForRTPReceiverChannel
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeReceiverChannelVideo::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::rtpsender::FakeReceiverChannelVideo");

        UseServicesHelper::debugAppend(result, "media id", mMediaID);
        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());
        UseServicesHelper::debugAppend(result, mParameters ? mParameters->toDebug() : ElementPtr());
        UseServicesHelper::debugAppend(result, "expect buffers", mExpectBuffers.size());
        auto receiverChannel = mReceiverChannel.lock();
        UseServicesHelper::debugAppend(result, "receiver channel", receiverChannel ? receiverChannel->getID() : 0);

        return result;
      }

      //-----------------------------------------------------------------------
      bool FakeReceiverChannelVideo::handlePacket(RTPPacketPtr packet)
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
      bool FakeReceiverChannelVideo::handlePacket(RTCPPacketPtr packet)
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
      #pragma mark FakeReceiverChannelVideo => IRTPReceiverChannelVideoForRTPReceiverChannel
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannelVideo => (friend RTPChannelTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeReceiverChannelVideo::setTransport(RTPChannelTesterPtr tester)
      {
        AutoRecursiveLock lock(*this);

        mTester = tester;
      }

      //-----------------------------------------------------------------------
      void FakeReceiverChannelVideo::create(
                                            RTPReceiverChannelPtr inReceiverChannel,
                                            const Parameters &params
                                            )
      {
        UseChannelPtr receiverChannel = inReceiverChannel;

        ZS_LOG_BASIC(log("create called") + ZS_PARAM("receiver channel", receiverChannel->getID()) + params.toDebug())

        AutoRecursiveLock lock(*this);
        mReceiverChannel = receiverChannel;

        TESTING_EQUAL(params.hash(), mParameters->hash())
      }

      //-----------------------------------------------------------------------
      void FakeReceiverChannelVideo::expectData(SecureByteBlockPtr data)
      {
        TESTING_CHECK((bool)data)

        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("expecting buffer") + ZS_PARAM("buffer size", data->SizeInBytes()))

        mExpectBuffers.push_back(data);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannelVideo => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params FakeReceiverChannelVideo::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpchannel::FakeReceiverChannelVideo");
        UseServicesHelper::debugAppend(objectEl, "id", FakeReceiverChannelVideo::getID());
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannelAudio
      #pragma mark

      //-----------------------------------------------------------------------
      FakeSenderChannelAudio::FakeSenderChannelAudio(
                                                     IMessageQueuePtr queue,
                                                     const char *mediaID,
                                                     const Parameters &params
                                                     ) :
        RTPSenderChannelAudio(Noop(true), queue),
        mMediaID(mediaID),
        mParameters(make_shared<Parameters>(params))
      {
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelAudio::~FakeSenderChannelAudio()
      {
        mThisWeak.reset();
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelAudioPtr FakeSenderChannelAudio::create(
                                                               IMessageQueuePtr queue,
                                                               const char *mediaID,
                                                               const Parameters &params
                                                               )
      {
        FakeSenderChannelAudioPtr pThis(make_shared<FakeSenderChannelAudio>(queue, mediaID, params));
        pThis->mThisWeak = pThis;
        return pThis;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannelAudio => IRTPSenderChannelMediaBaseForRTPSenderChannel
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeSenderChannelAudio::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::rtpsender::FakeSenderChannelAudio");

        UseServicesHelper::debugAppend(result, "media id", mMediaID);
        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());
        UseServicesHelper::debugAppend(result, mParameters ? mParameters->toDebug() : ElementPtr());
        UseServicesHelper::debugAppend(result, "expect buffers", mExpectBuffers.size());
        auto senderChannel = mSenderChannel.lock();
        UseServicesHelper::debugAppend(result, "sender channel", senderChannel ? senderChannel->getID() : 0);

        return result;
      }

      //-----------------------------------------------------------------------
      bool FakeSenderChannelAudio::handlePacket(RTCPPacketPtr packet)
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
      #pragma mark FakeSenderChannelAudio => IRTPSenderChannelAudioForRTPSenderChannel
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannelAudio => (friend RTPChannelTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeSenderChannelAudio::setTransport(RTPChannelTesterPtr tester)
      {
        AutoRecursiveLock lock(*this);

        mTester = tester;
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannelAudio::create(
                                          RTPSenderChannelPtr inSenderChannel,
                                          const Parameters &params
                                          )
      {
        UseChannelPtr senderChannel = inSenderChannel;

        ZS_LOG_BASIC(log("create called") + ZS_PARAM("sender channel", senderChannel->getID()) + params.toDebug())

        AutoRecursiveLock lock(*this);
        mSenderChannel = senderChannel;

        TESTING_EQUAL(params.hash(), mParameters->hash())
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannelAudio::expectData(SecureByteBlockPtr data)
      {
        TESTING_CHECK((bool)data)

        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("expecting buffer") + ZS_PARAM("buffer size", data->SizeInBytes()))

        mExpectBuffers.push_back(data);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannelAudio => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params FakeSenderChannelAudio::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpchannel::FakeSenderChannelAudio");
        UseServicesHelper::debugAppend(objectEl, "id", FakeSenderChannelAudio::getID());
        return Log::Params(message, objectEl);
      }



      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannelVideo
      #pragma mark

      //-----------------------------------------------------------------------
      FakeSenderChannelVideo::FakeSenderChannelVideo(
                                                     IMessageQueuePtr queue,
                                                     const char *mediaID,
                                                     const Parameters &params
                                                     ) :
        RTPSenderChannelVideo(Noop(true), queue),
        mMediaID(mediaID),
        mParameters(make_shared<Parameters>(params))
      {
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelVideo::~FakeSenderChannelVideo()
      {
        mThisWeak.reset();
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelVideoPtr FakeSenderChannelVideo::create(
                                                               IMessageQueuePtr queue,
                                                               const char *mediaID,
                                                               const Parameters &params
                                                               )
      {
        FakeSenderChannelVideoPtr pThis(make_shared<FakeSenderChannelVideo>(queue, mediaID, params));
        pThis->mThisWeak = pThis;
        return pThis;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannelVideo => IRTPSenderChannelMediaBaseForRTPSenderChannel
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeSenderChannelVideo::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::rtpsender::FakeSenderChannelVideo");

        UseServicesHelper::debugAppend(result, "media id", mMediaID);
        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());
        UseServicesHelper::debugAppend(result, mParameters ? mParameters->toDebug() : ElementPtr());
        UseServicesHelper::debugAppend(result, "expect buffers", mExpectBuffers.size());
        auto senderChannel = mSenderChannel.lock();
        UseServicesHelper::debugAppend(result, "sender channel", senderChannel ? senderChannel->getID() : 0);

        return result;
      }

      //-----------------------------------------------------------------------
      bool FakeSenderChannelVideo::handlePacket(RTCPPacketPtr packet)
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
      #pragma mark FakeSenderChannelVideo => IRTPSenderChannelVideoForRTPSenderChannel
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannelVideo => (friend RTPChannelTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeSenderChannelVideo::setTransport(RTPChannelTesterPtr tester)
      {
        AutoRecursiveLock lock(*this);

        mTester = tester;
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannelVideo::create(
                                          RTPSenderChannelPtr inSenderChannel,
                                          const Parameters &params
                                          )
      {
        UseChannelPtr senderChannel = inSenderChannel;

        ZS_LOG_BASIC(log("create called") + ZS_PARAM("sender channel", senderChannel->getID()) + params.toDebug())

        AutoRecursiveLock lock(*this);
        mSenderChannel = senderChannel;

        TESTING_EQUAL(params.hash(), mParameters->hash())
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannelVideo::expectData(SecureByteBlockPtr data)
      {
        TESTING_CHECK((bool)data)

        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("expecting buffer") + ZS_PARAM("buffer size", data->SizeInBytes()))

        mExpectBuffers.push_back(data);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannelVideo => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params FakeSenderChannelVideo::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpchannel::FakeSenderChannelVideo");
        UseServicesHelper::debugAppend(objectEl, "id", FakeSenderChannelVideo::getID());
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelTester::Expectations
      #pragma mark

      //-----------------------------------------------------------------------
      bool RTPChannelTester::Expectations::operator==(const Expectations &op2) const
      {
        return (mReceivedPackets == op2.mReceivedPackets) &&
               (mSenderChannelConflict == op2.mSenderChannelConflict);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelTester::OverrideReceiverChannelAudioFactory
      #pragma mark

      //-----------------------------------------------------------------------
      RTPChannelTester::OverrideReceiverChannelAudioFactoryPtr RTPChannelTester::OverrideReceiverChannelAudioFactory::create(RTPChannelTesterPtr tester)
      {
        OverrideReceiverChannelAudioFactoryPtr pThis(make_shared<OverrideReceiverChannelAudioFactory>());
        pThis->mTester = tester;
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPReceiverChannelAudioPtr RTPChannelTester::OverrideReceiverChannelAudioFactory::create(
                                                                                               RTPReceiverChannelPtr receiverChannel,
                                                                                               MediaStreamTrackPtr track,
                                                                                               const Parameters &params
                                                                                               )
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)

        return tester->createReceiverChannelAudio(receiverChannel, params);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelTester::OverrideReceiverChannelVideoFactory
      #pragma mark

      //-----------------------------------------------------------------------
      RTPChannelTester::OverrideReceiverChannelVideoFactoryPtr RTPChannelTester::OverrideReceiverChannelVideoFactory::create(RTPChannelTesterPtr tester)
      {
        OverrideReceiverChannelVideoFactoryPtr pThis(make_shared<OverrideReceiverChannelVideoFactory>());
        pThis->mTester = tester;
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPReceiverChannelVideoPtr RTPChannelTester::OverrideReceiverChannelVideoFactory::create(
                                                                                               RTPReceiverChannelPtr receiverChannel,
                                                                                               MediaStreamTrackPtr track,
                                                                                               const Parameters &params
                                                                                               )
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)

        return tester->createReceiverChannelVideo(receiverChannel, params);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelTester::OverrideSenderChannelAudioFactory
      #pragma mark

      //-----------------------------------------------------------------------
      RTPChannelTester::OverrideSenderChannelAudioFactoryPtr RTPChannelTester::OverrideSenderChannelAudioFactory::create(RTPChannelTesterPtr tester)
      {
        OverrideSenderChannelAudioFactoryPtr pThis(make_shared<OverrideSenderChannelAudioFactory>());
        pThis->mTester = tester;
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPSenderChannelAudioPtr RTPChannelTester::OverrideSenderChannelAudioFactory::create(
                                                                                           RTPSenderChannelPtr senderChannel,
                                                                                           MediaStreamTrackPtr track,
                                                                                           const Parameters &params
                                                                                           )
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)

        return tester->createSenderChannelAudio(senderChannel, params);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelTester::OverrideSenderChannelVideoFactory
      #pragma mark

      //-----------------------------------------------------------------------
      RTPChannelTester::OverrideSenderChannelVideoFactoryPtr RTPChannelTester::OverrideSenderChannelVideoFactory::create(RTPChannelTesterPtr tester)
      {
        OverrideSenderChannelVideoFactoryPtr pThis(make_shared<OverrideSenderChannelVideoFactory>());
        pThis->mTester = tester;
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPSenderChannelVideoPtr RTPChannelTester::OverrideSenderChannelVideoFactory::create(
                                                                                           RTPSenderChannelPtr senderChannel,
                                                                                           MediaStreamTrackPtr track,
                                                                                           const Parameters &params
                                                                                           )
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)

        return tester->createSenderChannelVideo(senderChannel, params);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelTester
      #pragma mark

      //-----------------------------------------------------------------------
      RTPChannelTesterPtr RTPChannelTester::create(
                                                   IMessageQueuePtr queue,
                                                   IMediaStreamTrackTypes::Kinds kind,
                                                   bool overrideFactories,
                                                   Milliseconds packetDelay
                                                   )
      {
        RTPChannelTesterPtr pThis(new RTPChannelTester(queue, overrideFactories));
        pThis->mThisWeak = pThis;
        pThis->init(kind, packetDelay);
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPChannelTester::RTPChannelTester(
                                         IMessageQueuePtr queue,
                                         bool overrideFactories
                                         ) :
        SharedRecursiveLock(SharedRecursiveLock::create()),
        MessageQueueAssociator(queue),
        mOverrideFactories(overrideFactories)
      {
        ZS_LOG_BASIC(log("rtpchannel tester"))
      }

      //-----------------------------------------------------------------------
      RTPChannelTester::~RTPChannelTester()
      {
        ZS_LOG_BASIC(log("rtpchannel tester"))

        if (mOverrideFactories) {
          RTPReceiverChannelAudioFactory::override(RTPReceiverChannelAudioFactoryPtr());
          RTPReceiverChannelVideoFactory::override(RTPReceiverChannelVideoFactoryPtr());
          RTPSenderChannelAudioFactory::override(RTPSenderChannelAudioFactoryPtr());
          RTPSenderChannelVideoFactory::override(RTPSenderChannelVideoFactoryPtr());
        }
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::init(
                                  IMediaStreamTrackTypes::Kinds kind,
                                  Milliseconds packetDelay
                                  )
      {
        AutoRecursiveLock lock(*this);

        if (mOverrideFactories) {
          RTPReceiverChannelAudioFactory::override(OverrideReceiverChannelAudioFactory::create(mThisWeak.lock()));
          RTPReceiverChannelVideoFactory::override(OverrideReceiverChannelVideoFactory::create(mThisWeak.lock()));
          RTPSenderChannelAudioFactory::override(OverrideSenderChannelAudioFactory::create(mThisWeak.lock()));
          RTPSenderChannelVideoFactory::override(OverrideSenderChannelVideoFactory::create(mThisWeak.lock()));
        }

        mICETransport = FakeICETransport::create(getAssociatedMessageQueue(), packetDelay);
        mDTLSTransport = FakeSecureTransport::create(getAssociatedMessageQueue(), mICETransport);

        auto listener = mDTLSTransport->getListener();
        TESTING_CHECK(listener)

        mListenerSubscription = FakeListener::convert(listener)->subscribe(mThisWeak.lock());
        TESTING_CHECK(mListenerSubscription)

        mMediaStreamTrack = FakeMediaStreamTrack::create(getAssociatedMessageQueue(), kind);
        mMediaStreamTrack->setTransport(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      bool RTPChannelTester::matches()
      {
        AutoRecursiveLock lock(*this);
        return mExpectationsFound == mExpecting;
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::close()
      {
        AutoRecursiveLock lock(*this);

        for (auto iter = mReceiverChannels.begin(); iter != mReceiverChannels.end(); ++iter)
        {
          UseReceiverChannelForReceiverPtr receiverChannel = (*iter).second;
          receiverChannel->notifyTransportState(ISecureTransportTypes::State_Closed);
        }

        mReceiverChannels.clear();

        for (auto iter = mAttached.begin(); iter != mAttached.end(); ++iter) {
          auto &receiver = (*iter).second.first;
          auto &sender = (*iter).second.second;

          if (receiver) {
            receiver->stop();
          }
          if (sender) {
            sender->stop();
          }
        }

        mAttached.clear();
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::closeByReset()
      {
        AutoRecursiveLock lock(*this);
        mICETransport.reset();
        mDTLSTransport.reset();

        mReceiverChannels.clear();
      }

      //-----------------------------------------------------------------------
      RTPChannelTester::Expectations RTPChannelTester::getExpectations() const
      {
        return mExpectationsFound;
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::state(IICETransport::States newState)
      {
        FakeICETransportPtr transport;
        {
          AutoRecursiveLock lock(*this);
          transport = mICETransport;
        }
        transport->state(newState);
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::state(IDTLSTransport::States newState)
      {
        FakeSecureTransportPtr transport;
        {
          AutoRecursiveLock lock(*this);
          transport = mDTLSTransport;
        }
        transport->state(newState);
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::setClientRole(bool clientRole)
      {
        FakeSecureTransportPtr transport;
        {
          AutoRecursiveLock lock(*this);
          transport = mDTLSTransport;
        }
        transport->setClientRole(clientRole);
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::connect(RTPChannelTesterPtr remote)
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
      void RTPChannelTester::createReceiver(const char *receiverID)
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
      void RTPChannelTester::createSender(const char *senderID)
      {
        FakeSenderPtr sender = getSender(senderID);

        if (!sender) {
          sender = FakeSender::create(getAssociatedMessageQueue());
          attach(senderID, sender);
        }

        TESTING_CHECK(sender)
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::createReceiverChannelAudio(
                                                         const char *receiverChannelAudioID,
                                                         const char *parametersID
                                                         )
      {
        FakeReceiverChannelAudioPtr receiverChannelAudio = getReceiverChannelAudio(receiverChannelAudioID);

        if (!receiverChannelAudio) {
          auto params = getParameters(parametersID);
          TESTING_CHECK(params)
          receiverChannelAudio = FakeReceiverChannelAudio::create(getAssociatedMessageQueue(), receiverChannelAudioID, *params);
          attach(receiverChannelAudioID, receiverChannelAudio);
        }

        TESTING_CHECK(receiverChannelAudio)
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::createReceiverChannelVideo(
                                                        const char *receiverChannelVideoID,
                                                        const char *parametersID
                                                        )
      {
        FakeReceiverChannelVideoPtr receiverChannelVideo = getReceiverChannelVideo(receiverChannelVideoID);

        if (!receiverChannelVideo) {
          auto params = getParameters(parametersID);
          TESTING_CHECK(params)
          receiverChannelVideo = FakeReceiverChannelVideo::create(getAssociatedMessageQueue(), receiverChannelVideoID, *params);
          attach(receiverChannelVideoID, receiverChannelVideo);
        }

        TESTING_CHECK(receiverChannelVideo)
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::createSenderChannelAudio(
                                                      const char *senderChannelAudioID,
                                                      const char *parametersID
                                                      )
      {
        FakeSenderChannelAudioPtr senderChannelAudio = getSenderChannelAudio(senderChannelAudioID);

        if (!senderChannelAudio) {
          auto params = getParameters(parametersID);
          TESTING_CHECK(params)
          senderChannelAudio = FakeSenderChannelAudio::create(getAssociatedMessageQueue(), senderChannelAudioID, *params);
          attach(senderChannelAudioID, senderChannelAudio);
        }

        TESTING_CHECK(senderChannelAudio)
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::createSenderChannelVideo(
                                                      const char *senderChannelVideoID,
                                                      const char *parametersID
                                                      )
      {
        FakeSenderChannelVideoPtr senderChannelVideo = getSenderChannelVideo(senderChannelVideoID);

        if (!senderChannelVideo) {
          auto params = getParameters(parametersID);
          TESTING_CHECK(params)
          senderChannelVideo = FakeSenderChannelVideo::create(getAssociatedMessageQueue(), senderChannelVideoID, *params);
          attach(senderChannelVideoID, senderChannelVideo);
        }

        TESTING_CHECK(senderChannelVideo)
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::send(
                                  const char *senderID,
                                  const char *parametersID
                                  )
      {
        FakeSenderPtr sender = getSender(senderID);

        if (!sender) {
          sender = FakeSender::create(getAssociatedMessageQueue());
          attach(senderID, sender);
        }

        TESTING_CHECK(sender)

        auto params = getParameters(parametersID);
        TESTING_CHECK(params)

        sender->send(*params);
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::receive(
                                     const char *receiverID,
                                     const char *parametersID
                                     )
      {
        FakeReceiverPtr receiver = getReceiver(receiverID);

        if (!receiver) {
          IMediaStreamTrackTypes::Kinds kind {};

          {
            AutoRecursiveLock lock(*this);
            TESTING_CHECK((bool)mMediaStreamTrack)
            kind = mMediaStreamTrack->kind();
          }

          receiver = FakeReceiver::create(getAssociatedMessageQueue(), kind);
          attach(receiverID, receiver);
        }

        TESTING_CHECK(receiver)

        auto params = getParameters(parametersID);
        TESTING_CHECK(params)

        receiver->receive(*params);
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelTester::stop(const char *senderOrReceiverID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mAttached.find(String(senderOrReceiverID));
        TESTING_CHECK(found != mAttached.end())

        auto receiver = (*found).second.first;
        auto sender = (*found).second.second;

        if (receiver) {
          receiver->stop();
        }
        if (sender) {
          sender->stop();
        }

        mAttached.erase(found);
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::attach(
                                    const char *receiverID,
                                    FakeReceiverPtr receiverChannel
                                    )
      {
        TESTING_CHECK(receiverChannel)

        String receiverIDStr(receiverID);

        AutoRecursiveLock lock(*this);

        FakeReceiverPtr oldChannel;

        auto found = mAttached.find(receiverIDStr);

        if (found != mAttached.end()) {
          auto &previousReceiver = (*found).second.first;
          if (previousReceiver) {
            previousReceiver->stop();
          }

          previousReceiver = receiverChannel;
          receiverChannel->setTransport(mThisWeak.lock());
          return;
        }

        mAttached[receiverIDStr] = FakeReceiverFakeSenderPair(receiverChannel, FakeSenderPtr());
        receiverChannel->setTransport(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::attach(
                                    const char *senderID,
                                    FakeSenderPtr sender
                                    )
      {
        AutoRecursiveLock lock(*this);

        auto found = mAttached.find(String(senderID));

        if (found != mAttached.end()) {
          auto &previousSender = (*found).second.second;
          if (previousSender) {
            previousSender->stop();
          }

          previousSender = sender;
          sender->setTransport(mThisWeak.lock());
          return;
        }

        mAttached[String(senderID)] = FakeReceiverFakeSenderPair(FakeReceiverPtr(), sender);
        sender->setTransport(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::attach(
                                    const char *receiverChannelID,
                                    RTPReceiverChannelPtr receiverChannel
                                    )
      {
        TESTING_CHECK(receiverChannel)

        String receiverChannelIDStr(receiverChannelID);

        AutoRecursiveLock lock(*this);

        RTPReceiverChannelPtr oldChannel;

        auto found = mReceiverChannels.find(receiverChannelIDStr);

        if (found != mReceiverChannels.end()) {
          auto &previousReceiverChannel = (*found).second;
          if (previousReceiverChannel) {
            UseReceiverChannelForReceiverPtr(previousReceiverChannel)->notifyTransportState(ISecureTransportTypes::State_Closed);
          }

          previousReceiverChannel = receiverChannel;
          return;
        }

        mReceiverChannels[receiverChannelIDStr] = receiverChannel;
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::attach(
                                    const char *senderChannelID,
                                    RTPSenderChannelPtr senderChannel
                                    )
      {
        TESTING_CHECK(senderChannel)

        String senderChannelIDStr(senderChannelID);

        AutoRecursiveLock lock(*this);

        RTPReceiverChannelPtr oldChannel;

        auto found = mSenderChannels.find(senderChannelIDStr);

        if (found != mSenderChannels.end()) {
          auto &previousSenderChannel = (*found).second;
          if (previousSenderChannel) {
            UseSenderChannelForSenderPtr(previousSenderChannel)->notifyTransportState(ISecureTransportTypes::State_Closed);
          }

          previousSenderChannel = senderChannel;
          return;
        }

        mSenderChannels[senderChannelIDStr] = senderChannel;
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::attach(
                                    const char *receiverChannelAudioID,
                                    FakeReceiverChannelAudioPtr receiverChannelAudio
                                    )
      {
        TESTING_CHECK(receiverChannelAudio)

        String mediaIDStr(receiverChannelAudioID);

        AutoRecursiveLock lock(*this);

        FakeReceiverChannelAudioPtr oldMedia;

        for (auto iter_doNotUse = mFakeReceiverChannelAudioCreationList.begin(); iter_doNotUse != mFakeReceiverChannelAudioCreationList.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &mediaID = (*current).first;

          if (mediaID != mediaIDStr) continue;

          oldMedia = (*current).second;

          mFakeReceiverChannelAudioCreationList.erase(current);
        }

        mFakeReceiverChannelAudioCreationList.push_back(FakeReceiverChannelAudioPair(mediaIDStr, receiverChannelAudio));

        mAttachedReceiverChannelAudio[mediaIDStr] = receiverChannelAudio;

        receiverChannelAudio->setTransport(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::attach(
                                    const char *receiverChannelVideoID,
                                    FakeReceiverChannelVideoPtr receiverChannelVideo
                                    )
      {
        TESTING_CHECK(receiverChannelVideo)

        String mediaIDStr(receiverChannelVideoID);

        AutoRecursiveLock lock(*this);

        FakeReceiverChannelVideoPtr oldMedia;

        for (auto iter_doNotUse = mFakeReceiverChannelVideoCreationList.begin(); iter_doNotUse != mFakeReceiverChannelVideoCreationList.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &mediaID = (*current).first;

          if (mediaID != mediaIDStr) continue;

          oldMedia = (*current).second;

          mFakeReceiverChannelVideoCreationList.erase(current);
        }

        mFakeReceiverChannelVideoCreationList.push_back(FakeReceiverChannelVideoPair(mediaIDStr, receiverChannelVideo));

        mAttachedReceiverChannelVideo[mediaIDStr] = receiverChannelVideo;

        receiverChannelVideo->setTransport(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::attach(
                                    const char *senderChannelAudioID,
                                    FakeSenderChannelAudioPtr senderChannelAudio
                                    )
      {
        TESTING_CHECK(senderChannelAudio)

        String mediaIDStr(senderChannelAudioID);

        AutoRecursiveLock lock(*this);

        FakeSenderChannelAudioPtr oldMedia;

        for (auto iter_doNotUse = mFakeSenderChannelAudioCreationList.begin(); iter_doNotUse != mFakeSenderChannelAudioCreationList.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &mediaID = (*current).first;

          if (mediaID != mediaIDStr) continue;

          oldMedia = (*current).second;

          mFakeSenderChannelAudioCreationList.erase(current);
        }

        mFakeSenderChannelAudioCreationList.push_back(FakeSenderChannelAudioPair(mediaIDStr, senderChannelAudio));

        mAttachedSenderChannelAudio[mediaIDStr] = senderChannelAudio;

        senderChannelAudio->setTransport(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::attach(
                                    const char *senderChannelVideoID,
                                    FakeSenderChannelVideoPtr senderChannelVideo
                                    )
      {
        TESTING_CHECK(senderChannelVideo)

        String mediaIDStr(senderChannelVideoID);

        AutoRecursiveLock lock(*this);

        FakeSenderChannelVideoPtr oldMedia;

        for (auto iter_doNotUse = mFakeSenderChannelVideoCreationList.begin(); iter_doNotUse != mFakeSenderChannelVideoCreationList.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &mediaID = (*current).first;

          if (mediaID != mediaIDStr) continue;

          oldMedia = (*current).second;

          mFakeSenderChannelVideoCreationList.erase(current);
        }

        mFakeSenderChannelVideoCreationList.push_back(FakeSenderChannelVideoPair(mediaIDStr, senderChannelVideo));

        mAttachedSenderChannelVideo[mediaIDStr] = senderChannelVideo;

        senderChannelVideo->setTransport(mThisWeak.lock());
      }

      //-----------------------------------------------------------------------
      FakeReceiverPtr RTPChannelTester::detachReceiver(const char *receiverID)
      {
        String receiverIDStr(receiverID);

        AutoRecursiveLock lock(*this);

        auto found = mAttached.find(receiverIDStr);
        TESTING_CHECK(found != mAttached.end())

        auto &currentReceiver = (*found).second.first;
        auto &currentSender = (*found).second.second;

        FakeReceiverPtr receiver = currentReceiver;

        if (currentReceiver) {
          currentReceiver->setTransport(RTPChannelTesterPtr());
        }

        currentReceiver.reset();

        if (!currentSender) mAttached.erase(found);

        return receiver;
      }
      
      //-----------------------------------------------------------------------
      FakeSenderPtr RTPChannelTester::detachSender(const char *senderID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mAttached.find(String(senderID));
        TESTING_CHECK(found != mAttached.end())

        auto &currentReceiver = (*found).second.first;
        auto &currentSender = (*found).second.second;

        FakeSenderPtr sender = currentSender;

        if (sender) {
          sender->setTransport(RTPChannelTesterPtr());
        }

        currentSender.reset();
        if (!currentReceiver) mAttached.erase(found);
        
        return sender;
      }

      //-----------------------------------------------------------------------
      RTPReceiverChannelPtr RTPChannelTester::detachReceiverChannel(const char *receiverChannelID)
      {
        String receiverChannelIDStr(receiverChannelID);

        AutoRecursiveLock lock(*this);

        auto found = mReceiverChannels.find(receiverChannelIDStr);
        TESTING_CHECK(found != mReceiverChannels.end())

        RTPReceiverChannelPtr receiverChannel = (*found).second;

        mReceiverChannels.erase(found);

        return receiverChannel;
      }

      //-----------------------------------------------------------------------
      RTPSenderChannelPtr RTPChannelTester::detachSenderChannel(const char *senderChannelID)
      {
        String senderChannelIDStr(senderChannelID);

        AutoRecursiveLock lock(*this);

        auto found = mSenderChannels.find(senderChannelIDStr);
        TESTING_CHECK(found != mSenderChannels.end())

        RTPSenderChannelPtr senderChannel = (*found).second;

        mSenderChannels.erase(found);

        return senderChannel;
      }
      
      //-----------------------------------------------------------------------
      FakeReceiverChannelAudioPtr RTPChannelTester::detachReceiverChannelAudio(const char *receiverChannelAudioID)
      {
        String mediaIDStr(receiverChannelAudioID);

        AutoRecursiveLock lock(*this);

        auto found = mAttachedReceiverChannelAudio.find(mediaIDStr);
        TESTING_CHECK(found != mAttachedReceiverChannelAudio.end())

        auto &currentMediaWeak = (*found).second;
        auto currentMedia = (*found).second.lock();

        if (currentMedia) {
          currentMedia->setTransport(RTPChannelTesterPtr());
        }

        currentMediaWeak.reset();

        mAttachedReceiverChannelAudio.erase(found);

        for (auto iter_doNotUse = mFakeReceiverChannelAudioCreationList.begin(); iter_doNotUse != mFakeReceiverChannelAudioCreationList.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &mediaID = (*current).first;

          if (mediaID != mediaIDStr) continue;

          mFakeReceiverChannelAudioCreationList.erase(current);
        }

        return currentMedia;
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelVideoPtr RTPChannelTester::detachReceiverChannelVideo(const char *receiverChannelVideoID)
      {
        String mediaIDStr(receiverChannelVideoID);

        AutoRecursiveLock lock(*this);

        auto found = mAttachedReceiverChannelVideo.find(mediaIDStr);
        TESTING_CHECK(found != mAttachedReceiverChannelVideo.end())

        auto &currentMediaWeak = (*found).second;
        auto currentMedia = (*found).second.lock();

        if (currentMedia) {
          currentMedia->setTransport(RTPChannelTesterPtr());
        }

        currentMediaWeak.reset();

        mAttachedReceiverChannelVideo.erase(found);

        for (auto iter_doNotUse = mFakeReceiverChannelVideoCreationList.begin(); iter_doNotUse != mFakeReceiverChannelVideoCreationList.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &mediaID = (*current).first;

          if (mediaID != mediaIDStr) continue;

          mFakeReceiverChannelVideoCreationList.erase(current);
        }

        return currentMedia;
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelAudioPtr RTPChannelTester::detachSenderChannelAudio(const char *senderChannelAudioID)
      {
        String mediaIDStr(senderChannelAudioID);

        AutoRecursiveLock lock(*this);

        auto found = mAttachedSenderChannelAudio.find(mediaIDStr);
        TESTING_CHECK(found != mAttachedSenderChannelAudio.end())

        auto &currentMediaWeak = (*found).second;
        auto currentMedia = (*found).second.lock();

        if (currentMedia) {
          currentMedia->setTransport(RTPChannelTesterPtr());
        }

        currentMediaWeak.reset();

        mAttachedSenderChannelAudio.erase(found);

        for (auto iter_doNotUse = mFakeSenderChannelAudioCreationList.begin(); iter_doNotUse != mFakeSenderChannelAudioCreationList.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &mediaID = (*current).first;

          if (mediaID != mediaIDStr) continue;

          mFakeSenderChannelAudioCreationList.erase(current);
        }

        return currentMedia;
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelVideoPtr RTPChannelTester::detachSenderChannelVideo(const char *senderChannelVideoID)
      {
        String mediaIDStr(senderChannelVideoID);

        AutoRecursiveLock lock(*this);

        auto found = mAttachedSenderChannelVideo.find(mediaIDStr);
        TESTING_CHECK(found != mAttachedSenderChannelVideo.end())

        auto &currentMediaWeak = (*found).second;
        auto currentMedia = (*found).second.lock();

        if (currentMedia) {
          currentMedia->setTransport(RTPChannelTesterPtr());
        }

        currentMediaWeak.reset();

        mAttachedSenderChannelVideo.erase(found);

        for (auto iter_doNotUse = mFakeSenderChannelVideoCreationList.begin(); iter_doNotUse != mFakeSenderChannelVideoCreationList.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &mediaID = (*current).first;
          
          if (mediaID != mediaIDStr) continue;
          
          mFakeSenderChannelVideoCreationList.erase(current);
        }
        
        return currentMedia;
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::store(
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
      void RTPChannelTester::store(
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
      void RTPChannelTester::store(
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
      RTPPacketPtr RTPChannelTester::getRTPPacket(const char *packetID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mPackets.find(String(packetID));
        if (found == mPackets.end()) return RTPPacketPtr();
        return (*found).second.first;
      }

      //-----------------------------------------------------------------------
      RTCPPacketPtr RTPChannelTester::getRTCPPacket(const char *packetID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mPackets.find(String(packetID));
        if (found == mPackets.end()) return RTCPPacketPtr();
        return (*found).second.second;
      }

      //-----------------------------------------------------------------------
      RTPChannelTester::ParametersPtr RTPChannelTester::getParameters(const char *parametersID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mParameters.find(String(parametersID));
        if (found == mParameters.end()) return ParametersPtr();
        return make_shared<Parameters>(*((*found).second));
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::sendPacket(
                                        const char *senderOrReceiverID,
                                        const char *packetID
                                        )
      {
        RTPPacketPtr rtp;
        RTCPPacketPtr rtcp;

        getPackets(packetID, rtp, rtcp);

        if (rtp) {
          sendData(senderOrReceiverID, rtp->buffer());
        }
        if (rtcp) {
          sendData(senderOrReceiverID, rtcp->buffer());
        }
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::expectPacket(
                                          const char *senderOrReceiverID,
                                          const char *packetID
                                          )
      {
        RTPPacketPtr rtp;
        RTCPPacketPtr rtcp;

        getPackets(packetID, rtp, rtcp);

        {
          AutoRecursiveLock lock(*this);
          ++mExpecting.mReceivedPackets;
        }

        if (rtp) {
          expectData(senderOrReceiverID, rtp->buffer());
        }
        if (rtcp) {
          expectData(senderOrReceiverID, rtcp->buffer());
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelTester => IRTPReceiverChannelForRTPReceiver
      #pragma mark

      //-----------------------------------------------------------------------
      void RTPChannelTester::createReceiverChannel(
                                                   const char *receiverChannelID,
                                                   const char *linkReceiverID,
                                                   const char *parametersID,
                                                   const PacketIDList &inPackets
                                                   )
      {
        typedef std::list<RTCPPacketPtr> PacketList;

        PacketList packets;

        for (auto iter = inPackets.begin(); iter != inPackets.end(); ++iter) {
          auto packetID = (*iter);

          RTPPacketPtr rtp;
          RTCPPacketPtr rtcp;

          getPackets(packetID, rtp, rtcp);
          TESTING_CHECK(rtcp)

          packets.push_back(rtcp);
        }

        auto receiver = getReceiver(linkReceiverID);
        TESTING_CHECK(receiver)

        auto params = getParameters(parametersID);
        TESTING_CHECK(params)

        auto receiverChannel = UseReceiverChannelForReceiver::create(receiver, MediaStreamTrackPtr(), *params, packets);

        receiver->linkChannel(receiverChannel);

        attach(receiverChannelID, receiverChannel);
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::notifyTransportState(
                                                  const char *receiverChannelOrSenderChannelID,
                                                  ISecureTransportTypes::States state
                                                  )
      {
        {
          auto receiverChannel = getReceiverChannel(receiverChannelOrSenderChannelID);
          if (receiverChannel) {
            UseReceiverChannelForReceiverPtr(receiverChannel)->notifyTransportState(state);
          }
        }
        {
          auto senderChannel = getSenderChannel(receiverChannelOrSenderChannelID);
          if (senderChannel) {
            UseSenderChannelForSenderPtr(senderChannel)->notifyTransportState(state);
          }
        }
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::notifyPacket(
                                          const char *receiverChannelID,
                                          const char *packetID
                                          )
      {
        RTPPacketPtr rtp;
        RTCPPacketPtr rtcp;

        getPackets(packetID, rtp, rtcp);
        TESTING_CHECK(rtp)

        {
          auto receiverChannel = getReceiverChannel(receiverChannelID);
          TESTING_CHECK(receiverChannel)
          UseReceiverChannelForReceiverPtr(receiverChannel)->notifyPacket(rtp);
        }
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::notifyPackets(
                                           const char *receiverChannelOrSenderChannelID,
                                           const PacketIDList &inPackets
                                           )
      {
        ZS_DECLARE_TYPEDEF_PTR(std::list<RTCPPacketPtr>, PacketList);

        PacketListPtr packets(make_shared<PacketList>());

        for (auto iter = inPackets.begin(); iter != inPackets.end(); ++iter) {
          auto packetID = (*iter);

          RTPPacketPtr rtp;
          RTCPPacketPtr rtcp;

          getPackets(packetID, rtp, rtcp);
          TESTING_CHECK(rtcp)

          packets->push_back(rtcp);
        }

        {
          auto receiverChannel = getReceiverChannel(receiverChannelOrSenderChannelID);
          if (receiverChannel) {
            UseReceiverChannelForReceiverPtr(receiverChannel)->notifyPackets(packets);
          }
        }
        {
          auto senderChannel = getSenderChannel(receiverChannelOrSenderChannelID);
          if (senderChannel) {
            UseSenderChannelForSenderPtr(senderChannel)->notifyPackets(packets);
          }
        }
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::notifyUpdate(
                                          const char *receiverChannelOrSenderChannelID,
                                          const char *parametersID
                                          )
      {
        auto params = getParameters(parametersID);
        TESTING_CHECK(params)

        {
          auto receiverChannel = getReceiverChannel(receiverChannelOrSenderChannelID);
          if (receiverChannel) {
            UseReceiverChannelForReceiverPtr(receiverChannel)->notifyUpdate(*params);
          }
        }
        {
          auto senderChannel = getSenderChannel(receiverChannelOrSenderChannelID);
          if (senderChannel) {
            UseSenderChannelForSenderPtr(senderChannel)->notifyUpdate(*params);
          }
        }
      }
      
      //-----------------------------------------------------------------------
      bool RTPChannelTester::handlePacket(
                                          const char *receiverChannelOrSenderChannelID,
                                          const char *packetID
                                          )
      {
        auto receiverChannel = getReceiverChannel(receiverChannelOrSenderChannelID);
        auto senderChannel = getSenderChannel(receiverChannelOrSenderChannelID);

        RTPPacketPtr rtp;
        RTCPPacketPtr rtcp;

        getPackets(packetID, rtp, rtcp);

        bool result = false;

        bool called = false;

        if (rtp) {
          TESTING_CHECK(receiverChannel)
          called = true;
          result = UseReceiverChannelForReceiverPtr(receiverChannel)->handlePacket(rtp);
        }
        if (rtcp) {
          if (receiverChannel) {
            called = true;
            result = UseReceiverChannelForReceiverPtr(receiverChannel)->handlePacket(rtcp);
          }
          if (senderChannel) {
            called = true;
            result = UseSenderChannelForSenderPtr(senderChannel)->handlePacket(rtcp);
          }
        }

        TESTING_CHECK(called)

        return result;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelTester => IRTPReceiverChannelForRTPReceiver
      #pragma mark

      //-----------------------------------------------------------------------
      void RTPChannelTester::createSenderChannel(
                                                 const char *senderChannelID,
                                                 const char *linkSenderID,
                                                 const char *parametersID
                                                 )
      {
        auto sender = getSender(linkSenderID);
        TESTING_CHECK(sender)

        auto params = getParameters(parametersID);
        TESTING_CHECK(params)

          auto senderChannel = UseSenderChannelForSender::create(sender, MediaStreamTrackPtr(), *params);

        sender->linkChannel(senderChannel);

        attach(senderChannelID, senderChannel);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelTester::IRTPReceiverDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void RTPChannelTester::onRTPListenerUnhandledRTP(
                                                       IRTPListenerPtr receiver,
                                                       SSRCType ssrc,
                                                       PayloadType payloadType,
                                                       const char *mid,
                                                       const char *rid
                                                       )
      {
        // ignored - not called
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelTester::IRTPReceiverDelegate
      #pragma mark


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelTester => (friend fake sender/receiver)
      #pragma mark

      //-----------------------------------------------------------------------
      FakeSecureTransportPtr RTPChannelTester::getFakeSecureTransport() const
      {
        return mDTLSTransport;
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::notifyReceivedPacket()
      {
        ZS_LOG_BASIC(log("notified received packet"))

        AutoRecursiveLock lock(*this);
        ++mExpectationsFound.mReceivedPackets;
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::notifySenderChannelConflict()
      {
        ZS_LOG_BASIC(log("notified sender channel conflict"))

        AutoRecursiveLock lock(*this);
        ++mExpectationsFound.mSenderChannelConflict;
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelAudioPtr RTPChannelTester::createReceiverChannelAudio(
                                                                               RTPReceiverChannelPtr receiverChannel,
                                                                               const Parameters &params
                                                                               )
      {
        AutoRecursiveLock lock(*this);

        TESTING_CHECK(mFakeReceiverChannelAudioCreationList.size() > 0)

        FakeReceiverChannelAudioPtr channelMedia = mFakeReceiverChannelAudioCreationList.front().second;

        TESTING_CHECK(channelMedia)

        channelMedia->create(receiverChannel, params);

        mFakeReceiverChannelAudioCreationList.pop_front();

        return channelMedia;
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelVideoPtr RTPChannelTester::createReceiverChannelVideo(
                                                                               RTPReceiverChannelPtr receiverChannel,
                                                                               const Parameters &params
                                                                               )
      {
        AutoRecursiveLock lock(*this);

        TESTING_CHECK(mFakeReceiverChannelVideoCreationList.size() > 0)

        FakeReceiverChannelVideoPtr channelMedia = mFakeReceiverChannelVideoCreationList.front().second;

        TESTING_CHECK(channelMedia)

        channelMedia->create(receiverChannel, params);

        mFakeReceiverChannelVideoCreationList.pop_front();

        return channelMedia;
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelAudioPtr RTPChannelTester::createSenderChannelAudio(
                                                                           RTPSenderChannelPtr senderChannel,
                                                                           const Parameters &params
                                                                           )
      {
        AutoRecursiveLock lock(*this);

        TESTING_CHECK(mFakeSenderChannelAudioCreationList.size() > 0)

        FakeSenderChannelAudioPtr channelMedia = mFakeSenderChannelAudioCreationList.front().second;

        TESTING_CHECK(channelMedia)

        channelMedia->create(senderChannel, params);

        mFakeSenderChannelAudioCreationList.pop_front();

        return channelMedia;
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelVideoPtr RTPChannelTester::createSenderChannelVideo(
                                                                           RTPSenderChannelPtr senderChannel,
                                                                           const Parameters &params
                                                                           )
      {
        AutoRecursiveLock lock(*this);

        TESTING_CHECK(mFakeSenderChannelVideoCreationList.size() > 0)

        FakeSenderChannelVideoPtr channelMedia = mFakeSenderChannelVideoCreationList.front().second;

        TESTING_CHECK(channelMedia)

        channelMedia->create(senderChannel, params);

        mFakeSenderChannelVideoCreationList.pop_front();

        return channelMedia;
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelTester => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params RTPChannelTester::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpchannel::RTPChannelTester");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      FakeICETransportPtr RTPChannelTester::getICETransport() const
      {
        AutoRecursiveLock lock(*this);
        return mICETransport;
      }

      //-----------------------------------------------------------------------
      FakeReceiverPtr RTPChannelTester::getReceiver(const char *receiverID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mAttached.find(String(receiverID));
        if (mAttached.end() == found) return FakeReceiverPtr();
        return (*found).second.first;
      }

      //-----------------------------------------------------------------------
      FakeSenderPtr RTPChannelTester::getSender(const char *senderID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mAttached.find(String(senderID));
        if (mAttached.end() == found) return FakeSenderPtr();
        return (*found).second.second;
      }

      //-----------------------------------------------------------------------
      RTPReceiverChannelPtr RTPChannelTester::getReceiverChannel(const char *receiverChannelID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mReceiverChannels.find(String(receiverChannelID));
        if (mReceiverChannels.end() == found) return RTPReceiverChannelPtr();
        return (*found).second;
      }

      //-----------------------------------------------------------------------
      RTPSenderChannelPtr RTPChannelTester::getSenderChannel(const char *senderChannelID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mSenderChannels.find(String(senderChannelID));
        if (mSenderChannels.end() == found) return RTPSenderChannelPtr();
        return (*found).second;
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelAudioPtr RTPChannelTester::getReceiverChannelAudio(const char *receiverChannelAudioID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mAttachedReceiverChannelAudio.find(String(receiverChannelAudioID));
        if (mAttachedReceiverChannelAudio.end() == found) return FakeReceiverChannelAudioPtr();
        return (*found).second.lock();
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelVideoPtr RTPChannelTester::getReceiverChannelVideo(const char *receiverChannelVideoID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mAttachedReceiverChannelVideo.find(String(receiverChannelVideoID));
        if (mAttachedReceiverChannelVideo.end() == found) return FakeReceiverChannelVideoPtr();
        return (*found).second.lock();
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelAudioPtr RTPChannelTester::getSenderChannelAudio(const char *senderChannelAudioID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mAttachedSenderChannelAudio.find(String(senderChannelAudioID));
        if (mAttachedSenderChannelAudio.end() == found) return FakeSenderChannelAudioPtr();
        return (*found).second.lock();
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelVideoPtr RTPChannelTester::getSenderChannelVideo(const char *senderChannelVideoID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mAttachedSenderChannelVideo.find(String(senderChannelVideoID));
        if (mAttachedSenderChannelVideo.end() == found) return FakeSenderChannelVideoPtr();
        return (*found).second.lock();
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::expectData(
                                        const char *senderOrReceiverID,
                                        SecureByteBlockPtr secureBuffer
                                        )
      {
        {
          FakeSenderPtr sender = getSender(senderOrReceiverID);
          if (sender) {
            sender->expectData(secureBuffer);
          }
        }

        {
          FakeReceiverPtr receiver = getReceiver(senderOrReceiverID);
          if (receiver) {
            receiver->expectData(secureBuffer);
          }
        }
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::sendData(
                                      const char *senderOrReceiverID,
                                      SecureByteBlockPtr secureBuffer
                                      )
      {
        {
          FakeSenderPtr sender = getSender(senderOrReceiverID);
          if (sender) {
            sender->sendPacket(secureBuffer);
          }
        }
        {
          FakeReceiverPtr receiver = getReceiver(senderOrReceiverID);
          if (receiver) {
            receiver->sendPacket(secureBuffer);
          }
        }
      }

      //-----------------------------------------------------------------------
      void RTPChannelTester::getPackets(
                                        const char *packetID,
                                        RTPPacketPtr &outRTP,
                                        RTCPPacketPtr &outRTCP
                                        )
      {
        AutoRecursiveLock lock(*this);
        auto found = mPackets.find(String(packetID));
        TESTING_CHECK(found != mPackets.end())

        outRTP = (*found).second.first;
        outRTCP = (*found).second.second;
      }

    }
  }
}

ZS_DECLARE_USING_PTR(ortc::test::rtpchannel, FakeICETransport)
ZS_DECLARE_USING_PTR(ortc::test::rtpchannel, RTPChannelTester)
ZS_DECLARE_USING_PTR(ortc, IICETransport)
using ortc::IDTLSTransportTypes;
ZS_DECLARE_USING_PTR(ortc, IDTLSTransport)
ZS_DECLARE_USING_PTR(ortc, IDataChannel)
ZS_DECLARE_USING_PTR(ortc, IRTPReceiver)
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
#define TEST_ADVANCED_ROUTING 1

static void bogusSleep()
{
  for (int loop = 0; loop < 100; ++loop)
  {
    TESTING_SLEEP(100)
  }
}

void doTestRTPChannel()
{
  if (!ORTC_TEST_DO_RTP_CHANNEL_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  UseSettings::applyDefaults();

  auto thread(zsLib::IMessageQueueThread::createBasic());

  RTPChannelTesterPtr testObject1;
  RTPChannelTesterPtr testObject2;

  TESTING_STDOUT() << "WAITING:      Waiting for RTPReceiver testing to complete (max wait is 180 seconds).\n";

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

            testObject1 = RTPChannelTester::create(thread, IMediaStreamTrackTypes::Kind_Audio, true);
            testObject2 = RTPChannelTester::create(thread, IMediaStreamTrackTypes::Kind_Audio, false);

            TESTING_CHECK(testObject1)
            TESTING_CHECK(testObject2)

            testObject1->setClientRole(true);
            testObject2->setClientRole(false);
          }
          break;
        }
        case TEST_ADVANCED_ROUTING: {
          {
            testObject1 = RTPChannelTester::create(thread, IMediaStreamTrackTypes::Kind_Audio, true);
            testObject2 = RTPChannelTester::create(thread, IMediaStreamTrackTypes::Kind_Audio, false);

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
                if (testObject1) testObject1->state(IDTLSTransportTypes::State_Connected);
                if (testObject2) testObject2->state(IDTLSTransportTypes::State_Connected);
          //    bogusSleep();
                break;
              }
              case 5: {
          //    bogusSleep();
                break;
              }
              case 6: {
          //    bogusSleep();
                break;
              }
              case 7: {
          //    bogusSleep();
                break;
              }
              case 8: {
          //    bogusSleep();
                break;
              }
              case 9: {
          //    bogusSleep();
                break;
              }
              case 10: {
          //    bogusSleep();
                break;
              }
              case 11: {
          //    bogusSleep();
                break;
              }
              case 12: {
          //    bogusSleep();
                break;
              }
              case 14: {
          //    bogusSleep();
                break;
              }
              case 15: {
          //    bogusSleep();
                break;
              }
              case 20: {
                //bogusSleep();
                break;
              }
              case 21: {
                if (testObject1) testObject1->state(IDTLSTransportTypes::State_Closed);
                if (testObject2) testObject2->state(IDTLSTransportTypes::State_Closed);
                //bogusSleep();
                break;
              }
              case 22: {
                if (testObject1) testObject1->state(IICETransport::State_Disconnected);
                if (testObject2) testObject2->state(IICETransport::State_Disconnected);
                //bogusSleep();
                break;
              }
              case 23: {
                if (testObject1) testObject1->state(IICETransport::State_Closed);
                if (testObject2) testObject2->state(IICETransport::State_Closed);
                //bogusSleep();
                break;
              }
              case 24: {
                if (testObject1) testObject1->closeByReset();
                if (testObject2) testObject2->closeByReset();
                //bogusSleep();
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
          case TEST_ADVANCED_ROUTING: {
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
          //    bogusSleep();
                break;
              }
              case 5:
              {
          //    bogusSleep();
                break;
              }
              case 6: {
                if (testObject2) testObject2->state(IDTLSTransportTypes::State_Connected);
                if (testObject1) testObject1->state(IDTLSTransportTypes::State_Connected);
          //    bogusSleep();
                break;
              }
              case 10: {
          //    bogusSleep();
                break;
              }
              case 11: {
          //    bogusSleep();
                break;
              }
              case 12: {
          //    bogusSleep();
                break;
              }
              case 13: {
          //    bogusSleep();
                break;
              }
              case 14: {
          //    bogusSleep();
                break;
              }
              case 15: {
          //    bogusSleep();
                break;
              }
              case 16: {
          //    bogusSleep();
                break;
              }
              case 17: {
          //    bogusSleep();
                break;
              }
              case 18: {
          //    bogusSleep();
                break;
              }
              case 20: {
          //    bogusSleep();
                break;
              }
              case 21: {
          //    bogusSleep();
                break;
              }
              case 22: {
          //    bogusSleep();
                break;
              }
              case 23: {
          //    bogusSleep();
                break;
              }
              case 24: {
          //    bogusSleep();
                break;
              }
              case 25: {
          //    bogusSleep();
                break;
              }
              case 26: {
          //    bogusSleep();
                break;
              }
              case 27: {
          //    bogusSleep();
                break;
              }
              case 28: {
          //    bogusSleep();
                break;
              }
              case 29: {
          //    bogusSleep();
                break;
              }
              case 30: {
          //    bogusSleep();
                break;
              }
              case 35: {
                if (testObject1) testObject1->close();
                if (testObject2) testObject1->close();
          //    bogusSleep();
                break;
              }
              case 36: {
                if (testObject1) testObject1->state(IDTLSTransportTypes::State_Closed);
                if (testObject2) testObject2->state(IDTLSTransportTypes::State_Closed);
                //bogusSleep();
                break;
              }
              case 37: {
                if (testObject1) testObject1->state(IICETransport::State_Disconnected);
                if (testObject2) testObject2->state(IICETransport::State_Disconnected);
                //bogusSleep();
                break;
              }
              case 38: {
                if (testObject1) testObject1->state(IICETransport::State_Closed);
                if (testObject2) testObject2->state(IICETransport::State_Closed);
                //bogusSleep();
                break;
              }
              case 39: {
                if (testObject1) testObject1->closeByReset();
                if (testObject2) testObject2->closeByReset();
                //bogusSleep();
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

      switch (testNumber) {
        default:
        {
          if (testObject1) {TESTING_CHECK(testObject1->matches())}
          if (testObject2) {TESTING_CHECK(testObject2->matches())}
          break;
        }
      }

      testObject1.reset();
      testObject2.reset();

      ++testNumber;
    } while (true);
  }

  TESTING_STDOUT() << "WAITING:      All channels have finished. Waiting for 'bogus' events to process (1 second wait).\n";
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

