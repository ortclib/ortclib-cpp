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



#include "TestSCTP.h"
#include <ortc/ISCTPTransport.h>

#include <zsLib/IMessageQueueThread.h>
#include <zsLib/ISettings.h>
#include <zsLib/XML.h>

#include "config.h"
#include "testing.h"
#include <zsLib/date.h>

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
    namespace sctp
    {
      ZS_DECLARE_CLASS_PTR(FakeICETransport)
      ZS_DECLARE_CLASS_PTR(FakeSecureTransport)
      ZS_DECLARE_CLASS_PTR(SCTPTester)

      ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportTypes, ISecureTransportTypes)
      ZS_DECLARE_TYPEDEF_PROXY(ortc::internal::ISecureTransportDelegate, ISecureTransportDelegate)

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

        ElementPtr resultEl = Element::create("ortc::test::sctp::FakeICETransport");

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
      #pragma mark FakeICETransport => IICETransportForDataTransport
      #pragma mark

      //-----------------------------------------------------------------------
      PUID FakeICETransport::getID() const
      {
        return ICETransport::getID();
      }

      //-----------------------------------------------------------------------
      IICETransportSubscriptionPtr FakeICETransport::subscribe(IICETransportDelegatePtr originalDelegate)
      {
        ZS_LOG_DETAIL(log("subscribing to transport state"))

        AutoRecursiveLock lock(*this);
        if (!originalDelegate) return mDefaultSubscription;

        IICETransportSubscriptionPtr subscription = mSubscriptions.subscribe(IICETransportDelegateProxy::create(getAssociatedMessageQueue(), originalDelegate));

        IICETransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

        if (delegate) {
          FakeICETransportPtr pThis = mThisWeak.lock();

          if (IICETransportTypes::State_New != mCurrentState) {
            delegate->onICETransportStateChange(pThis, mCurrentState);
          }
        }

        if (isShutdown()) {
          mSubscriptions.clear();
        }

        return subscription;
      }

      //-----------------------------------------------------------------------
      IICETransport::States FakeICETransport::state() const
      {
        AutoRecursiveLock lock(*this);
        return mCurrentState;
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

          switch (mCurrentState) {
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
            ZS_LOG_WARNING(Detail, log("no sctp transport attached (thus cannot forward received packet)") + ZS_PARAM("buffer", (PTRNUMBER)(buffer->BytePtr())) + ZS_PARAM("buffer size", buffer->SizeInBytes()))
            return;
          }

          switch (mCurrentState) {
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
            ZS_LOG_WARNING(Detail, log("no sctp transport attached (thus cannot forward delayed packets)"))
            return;
          }

          while (mDelayedBuffers.size() > 0) {
            Time &delayTime = mDelayedBuffers.front().first;
            SecureByteBlockPtr buffer = mDelayedBuffers.front().second;

            if (delayTime > tick) {
              ZS_LOG_INSANE(log("delaying packet until tick") + ZS_PARAM("delay", delayTime) + ZS_PARAM("tick", tick))
              break;
            }

            switch (mCurrentState) {
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
        ElementPtr objectEl = Element::create("ortc::test::sctp::FakeICETransport");
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
        mICETransportSubscription = mICETransport->subscribe(mThisWeak.lock());

        mDataTransport = UseDataTransport::create(mThisWeak.lock());
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

        ElementPtr resultEl = Element::create("ortc::test::sctp::FakeICETransport");

        UseServicesHelper::debugAppend(resultEl, "id", getID());

        UseServicesHelper::debugAppend(resultEl, "state", IDTLSTransport::toString(mCurrentState));

        UseServicesHelper::debugAppend(resultEl, "ice transport", mICETransport ? mICETransport->getID() : 0);
        UseServicesHelper::debugAppend(resultEl, "ice transport state", IICETransportTypes::toString(mICETransportState));
        UseServicesHelper::debugAppend(resultEl, "ice transport subscription", (bool)mICETransportSubscription);

        UseServicesHelper::debugAppend(resultEl, "subscriptions", mSubscriptions.size());
        UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

        UseServicesHelper::debugAppend(resultEl, "secure transport subscriptions", mSecureTransportSubscriptions.size());
        UseServicesHelper::debugAppend(resultEl, "secure transport state", ISecureTransportTypes::toString(mSecureTransportState));

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSecureTransport => IICETransportForDataTransport
      #pragma mark

      //-----------------------------------------------------------------------
      PUID FakeSecureTransport::getID() const
      {
        return DTLSTransport::getID();
      }

      //-----------------------------------------------------------------------
      IDTLSTransportSubscriptionPtr FakeSecureTransport::subscribe(IDTLSTransportDelegatePtr originalDelegate)
      {
        ZS_LOG_DETAIL(log("subscribing to transport state"))

        AutoRecursiveLock lock(*this);
        if (!originalDelegate) return mDefaultSubscription;

        IDTLSTransportSubscriptionPtr subscription = mSubscriptions.subscribe(IDTLSTransportDelegateProxy::create(getAssociatedMessageQueue(), originalDelegate));

        IDTLSTransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

        if (delegate) {
          FakeSecureTransportPtr pThis = mThisWeak.lock();

          if (IDTLSTransportTypes::State_New != mCurrentState) {
            delegate->onDTLSTransportStateChange(pThis, mCurrentState);
          }
        }

        if (isShutdown()) {
          mSubscriptions.clear();
        }

        return subscription;
      }

      //-----------------------------------------------------------------------
      IDTLSTransport::States FakeSecureTransport::state() const
      {
        AutoRecursiveLock lock(*this);
        return mCurrentState;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSecureTransport => ISecureTransportForDataTransport
      #pragma mark

      //-----------------------------------------------------------------------
      FakeSecureTransport::ISecureTransportSubscriptionPtr FakeSecureTransport::subscribe(ISecureTransportDelegatePtr originalDelegate)
      {
        ZS_LOG_DETAIL(log("subscribing to secure transport state"))

        AutoRecursiveLock lock(*this);

        ISecureTransportSubscriptionPtr subscription = mSecureTransportSubscriptions.subscribe(ISecureTransportDelegateProxy::create(getAssociatedMessageQueue(), originalDelegate));

        ISecureTransportDelegatePtr delegate = mSecureTransportSubscriptions.delegate(subscription, true);

        if (delegate) {
          FakeSecureTransportPtr pThis = mThisWeak.lock();

          if (ISecureTransportTypes::State_Pending != mSecureTransportState) {
            delegate->onSecureTransportStateChanged(pThis, mSecureTransportState);
          }
        }

        if (isShutdown()) {
          mSubscriptions.clear();
        }
        
        return subscription;
      }

      //-----------------------------------------------------------------------
      ISecureTransportTypes::States FakeSecureTransport::state(ISecureTransportTypes::States ignored) const
      {
        return mSecureTransportState;
      }

      //-----------------------------------------------------------------------
      bool FakeSecureTransport::isClientRole() const
      {
        AutoRecursiveLock lock(*this);
        return mClientRole;
      }

      //-----------------------------------------------------------------------
      IICETransportPtr FakeSecureTransport::getICETransport() const
      {
        return mICETransport;
      }

      //-----------------------------------------------------------------------
      FakeSecureTransport::UseDataTransportPtr FakeSecureTransport::getDataTransport() const
      {
        return mDataTransport;
      }

      //-----------------------------------------------------------------------
      bool FakeSecureTransport::sendDataPacket(
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
        UseDataTransportPtr dataTransport;

        {
          AutoRecursiveLock lock(*this);

          if (IDTLSTransportTypes::State_Connected != mCurrentState) {
            ZS_LOG_WARNING(Detail, log("dropping incoming packet to simulate non validated state"))
            return false;
          }

          dataTransport = mDataTransport;
        }

        if (!dataTransport) {
          ZS_LOG_WARNING(Detail, log("dropping incoming packet (as data channel is gone)"))
          return false;
        }

        return dataTransport->handleDataPacket(buffer, bufferSizeInBytes);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeICETransport => friend IICETransportDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeSecureTransport::onICETransportStateChange(
                                                          IICETransportPtr transport,
                                                          IICETransport::States state
                                                          )
      {
        ZS_LOG_BASIC(log("ice transport state changed") + ZS_PARAM("transport", transport->getID()) + ZS_PARAM("state", IICETransportTypes::toString(state)))

        AutoRecursiveLock lock(*this);
        mICETransportState = state;
        fixState();

        if (IICETransportTypes::State_Closed == state) {
          cancel();
        }
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::onICETransportCandidatePairAvailable(
                                                                     IICETransportPtr transport,
                                                                     CandidatePairPtr candidatePair
                                                                     )
      {
        // IGNORED
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::onICETransportCandidatePairGone(
                                                                IICETransportPtr transport,
                                                                CandidatePairPtr candidatePair
                                                                )
      {
        // IGNORED
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::onICETransportCandidatePairChanged(
                                                                   IICETransportPtr transport,
                                                                   CandidatePairPtr candidatePair
                                                                   )
      {
        // IGNORED
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

        ZS_LOG_DETAIL(log("state changed") + ZS_PARAM("new state", IDTLSTransport::toString(state)) + ZS_PARAM("old state", IDTLSTransport::toString(mCurrentState)))

        mCurrentState = state;

        auto pThis = mThisWeak.lock();
        if (pThis) {
          mSubscriptions.delegate()->onDTLSTransportStateChange(pThis, mCurrentState);
        }
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::setState(ISecureTransportTypes::States state)
      {
        if (state == mSecureTransportState) return;

        if (isShutdown()) {
          ZS_LOG_TRACE(log("already closed"))
          return;
        }

        ZS_LOG_DETAIL(log("secure state changed") + ZS_PARAM("new state", ISecureTransport::toString(state)) + ZS_PARAM("old state", ISecureTransport::toString(mSecureTransportState)))

        mSecureTransportState = state;

        auto pThis = mThisWeak.lock();
        if (pThis) {
          mSecureTransportSubscriptions.delegate()->onSecureTransportStateChanged(pThis, mSecureTransportState);
        }

        fixState();
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
        ElementPtr objectEl = Element::create("ortc::test::sctp::FakeSecureTransport");
        UseServicesHelper::debugAppend(objectEl, "id", DTLSTransport::getID());
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::fixState()
      {
        switch (mCurrentState) {
          case IDTLSTransportTypes::State_New:
          case IDTLSTransportTypes::State_Connecting:
          {
            ZS_LOG_TRACE(log("transport not ready yet"))
            break;
          }
          case IDTLSTransportTypes::State_Connected:
          {
            switch (mICETransportState) {
              case IICETransportTypes::State_Connected:
              case IICETransportTypes::State_Completed:
              {
                setState(ISecureTransportTypes::State_Connected);
                break;
              }
              case IICETransportTypes::State_New:
              case IICETransportTypes::State_Checking:
              case IICETransportTypes::State_Disconnected:
              case IICETransportTypes::State_Failed:
              case IICETransportTypes::State_Closed:
              {
                setState(ISecureTransportTypes::State_Disconnected);
                break;
              }
            }
            break;
          }
          case IDTLSTransportTypes::State_Closed:
          case IDTLSTransportTypes::State_Failed:
          {
            ZS_LOG_TRACE(log("transport closed"))
            setState(ISecureTransportTypes::State_Closed);
            break;
          }
        }
      }

      //-----------------------------------------------------------------------
      void FakeSecureTransport::cancel()
      {
        setState(IDTLSTransportTypes::State_Closed);
        setState(ISecureTransportTypes::State_Closed);

        if (mICETransportSubscription) {
          mICETransportSubscription->cancel();
          mICETransportSubscription.reset();
        }

        mICETransport->detachSecure(*this);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTester
      #pragma mark

      //-----------------------------------------------------------------------
      bool SCTPTester::Expectations::operator==(const Expectations &op2) const
      {
        return (mStateConnecting == op2.mStateConnecting) &&
               (mStateOpen == op2.mStateOpen) &&
               (mStateClosing == op2.mStateClosing) &&
               (mStateClosed == op2.mStateClosed) &&

               (mReceivedBinary == op2.mReceivedBinary) &&
               (mReceivedText == op2.mReceivedText) &&

               (mError == op2.mError) &&

               (mTransportIncoming == op2.mTransportIncoming) &&

               (mTransportStateNew == op2.mTransportStateNew) &&
               (mTransportStateConnecting == op2.mTransportStateConnecting) &&
               (mTransportStateConnected == op2.mTransportStateConnected) &&
               (mTransportStateClosed == op2.mTransportStateClosed);
      }

      //-----------------------------------------------------------------------
      SCTPTesterPtr SCTPTester::create(
                                       IMessageQueuePtr queue,
                                       bool createSCTPNow,
                                       Optional<WORD> localPort,
                                       Optional<WORD> removePort,
                                       Milliseconds packetDelay
                                       )
      {
        SCTPTesterPtr pThis(new SCTPTester(queue));
        pThis->mThisWeak = pThis;
        pThis->init(createSCTPNow, localPort, removePort, packetDelay);
        return pThis;
      }

      //-----------------------------------------------------------------------
      SCTPTester::SCTPTester(IMessageQueuePtr queue) :
        SharedRecursiveLock(SharedRecursiveLock::create()),
        MessageQueueAssociator(queue)
      {
        ZS_LOG_BASIC(log("sctp tester"))
      }

      //-----------------------------------------------------------------------
      SCTPTester::~SCTPTester()
      {
        ZS_LOG_BASIC(log("sctp tester"))
      }

      //-----------------------------------------------------------------------
      void SCTPTester::init(
                            bool createSCTPNow,
                            Optional<WORD> localPort,
                            Optional<WORD> remotePort,
                            Milliseconds packetDelay
                            )
      {
        AutoRecursiveLock lock(*this);
        mICETransport = FakeICETransport::create(getAssociatedMessageQueue(), packetDelay);
        mDTLSTransport = FakeSecureTransport::create(getAssociatedMessageQueue(), mICETransport);
        mRemotePort = remotePort;

        if (createSCTPNow) {
          mSCTP = ISCTPTransport::create(mThisWeak.lock(), mDTLSTransport, localPort);
        }
      }

      //-----------------------------------------------------------------------
      bool SCTPTester::matches(const Expectations &op2)
      {
        AutoRecursiveLock lock(*this);
        return mExpectations == op2;
      }

      //-----------------------------------------------------------------------
      void SCTPTester::close()
      {
        AutoRecursiveLock lock(*this);
        mSCTP->stop();
      }

      //-----------------------------------------------------------------------
      void SCTPTester::closeByReset()
      {
        AutoRecursiveLock lock(*this);
        mICETransport.reset();
        mDTLSTransport.reset();
        mSCTP.reset();
      }

      //-----------------------------------------------------------------------
      SCTPTester::Expectations SCTPTester::getExpectations() const
      {
        return mExpectations;
      }

      //-----------------------------------------------------------------------
      void SCTPTester::state(IICETransport::States newState)
      {
        FakeICETransportPtr transport;
        {
          AutoRecursiveLock lock(*this);
          transport = mICETransport;
        }
        transport->state(newState);
      }

      //-----------------------------------------------------------------------
      void SCTPTester::state(IDTLSTransport::States newState)
      {
        FakeSecureTransportPtr transport;
        {
          AutoRecursiveLock lock(*this);
          transport = mDTLSTransport;
        }
        transport->state(newState);
      }

      //-----------------------------------------------------------------------
      void SCTPTester::setClientRole(bool clientRole)
      {
        FakeSecureTransportPtr transport;
        {
          AutoRecursiveLock lock(*this);
          transport = mDTLSTransport;
        }
        transport->setClientRole(clientRole);
      }

      //-----------------------------------------------------------------------
      void SCTPTester::listen()
      {
        AutoRecursiveLock lock(*this);
        auto remoteCaps = ISCTPTransport::getCapabilities();
        mListenerSubscription = ISCTPTransport::listen(mThisWeak.lock(), mDTLSTransport, *remoteCaps);
      }

      //-----------------------------------------------------------------------
      void SCTPTester::start(SCTPTesterPtr remote)
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

        auto localCaps = ISCTPTransport::getCapabilities();
        auto remoteCaps = ISCTPTransport::getCapabilities();

        if (mSCTP) {
          mSCTP->start(*remoteCaps, mRemotePort.hasValue() ? mRemotePort.value() : 0);
        }
        if (remote->mSCTP) {
          remote->mSCTP->start(*localCaps, remote->mRemotePort.hasValue() ? remote->mRemotePort.value() : 0);
        }
      }

      //-----------------------------------------------------------------------
      void SCTPTester::createChannel(const IDataChannel::Parameters &params)
      {
        AutoRecursiveLock lock(*this);

        ZS_LOG_BASIC(log("creating data channel") + params.toDebug())

        auto dataChannel = IDataChannel::create(mThisWeak.lock(), mSCTP, params);

        mDataChannels[params.mLabel] = dataChannel;
      }

      //-----------------------------------------------------------------------
      void SCTPTester::sendData(
                                const char *channelID,
                                SecureByteBlockPtr buffer
                                )
      {
        {
          auto remote = mConnectedTester.lock();
          TESTING_CHECK((bool)remote)

          AutoRecursiveLock lock(*remote);
          remote->expectData(channelID, buffer);
        }

        IDataChannelPtr channel;

        {
          AutoRecursiveLock lock(*this);
          TESTING_CHECK((bool)mSCTP)

          auto found = mDataChannels.find(String(channelID));
          TESTING_CHECK(found != mDataChannels.end())

          channel = (*found).second;

          TESTING_CHECK((bool)channel)
        }

        channel->send(*buffer);
      }

      //-----------------------------------------------------------------------
      void SCTPTester::sendData(
                                const char *channelID,
                                const String &message
                                )
      {
        {
          auto remote = mConnectedTester.lock();
          TESTING_CHECK((bool)remote)

          AutoRecursiveLock lock(*remote);
          remote->expectData(channelID, message);
        }

        IDataChannelPtr channel;

        {
          AutoRecursiveLock lock(*this);
          TESTING_CHECK((bool)mSCTP)

          auto found = mDataChannels.find(String(channelID));
          TESTING_CHECK(found != mDataChannels.end())

          channel = (*found).second;

          TESTING_CHECK((bool)channel)
        }

        channel->send(message);
      }

      //-----------------------------------------------------------------------
      void SCTPTester::closeChannel(const char *channelID)
      {
        IDataChannelPtr channel;

        {
          AutoRecursiveLock lock(*this);

          auto found = mDataChannels.find(String(channelID));
          TESTING_CHECK(found != mDataChannels.end())

          channel = (*found).second;
        }

        channel->close();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTester::ISCTPTransportDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void SCTPTester::onSCTPTransportDataChannel(
                                                  ISCTPTransportPtr transport,
                                                  IDataChannelPtr channel
                                                  )
      {
        auto params = channel->parameters();
        ZS_LOG_BASIC(log("on datachannel") + ZS_PARAM("channel", channel->getID()) + params->toDebug())

        AutoRecursiveLock lock(*this);
        mDataChannels[params->mLabel] = channel;

        auto subscription = channel->subscribe(mThisWeak.lock());
        TESTING_CHECK(subscription)

        subscription->background(); // auto clean subscription when data channel closes

        ++mExpectations.mIncoming;
      }

      //-----------------------------------------------------------------------
      void SCTPTester::onSCTPTransportStateChange(
                                                  ISCTPTransportPtr transport,
                                                  ISCTPTransportTypes::States state
                                                  )
      {
        ZS_LOG_BASIC(log("on sctptransport change") + ZS_PARAM("state", ISCTPTransportTypes::toString(state)))

        AutoRecursiveLock lock(*this);
        switch (state)
        {
          case ISCTPTransportTypes::State_New:          ++mExpectations.mTransportStateNew;
          case ISCTPTransportTypes::State_Connecting:   ++mExpectations.mTransportStateConnecting;
          case ISCTPTransportTypes::State_Connected:    ++mExpectations.mTransportStateConnected;
          case ISCTPTransportTypes::State_Closed:       ++mExpectations.mTransportStateClosed;
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTester::ISCTPTransportListenerDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void SCTPTester::onSCTPTransport(ISCTPTransportPtr transport)
      {
        ZS_LOG_BASIC(log("on SCTP transport") + ZS_PARAM("transport id", transport->getID()))

        AutoRecursiveLock lock(*this);

        TESTING_CHECK(!((bool)mSCTP))
        mSCTP = transport;
        auto subscription = mSCTP->subscribe(mThisWeak.lock());
        TESTING_CHECK(subscription)

        subscription->background();

        ++mExpectations.mTransportIncoming;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTester::IDataChannelDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void SCTPTester::onDataChannelStateChange(
                                                IDataChannelPtr channel,
                                                IDataChannelTypes::States state
                                                )
      {
        AutoRecursiveLock lock(*this);

        auto params = channel->parameters();

        String channelID = params->mLabel;

        ZS_LOG_BASIC(log("data channel state changed") + ZS_PARAM("channel id", channel->getID()) + ZS_PARAM("label", channelID) + ZS_PARAM("state", IDataChannel::toString(state)))

        auto found = mDataChannels.find(channelID);
        TESTING_CHECK(found != mDataChannels.end())

        switch (state) {
          case IDataChannel::State_Connecting:  ++mExpectations.mStateConnecting; break;
          case IDataChannel::State_Open:        ++mExpectations.mStateOpen; break;
          case IDataChannel::State_Closing:     ++mExpectations.mStateClosing; break;
          case IDataChannel::State_Closed:      {
            mDataChannels.erase(found);
            ++mExpectations.mStateClosed;
            break;
          }
        }
      }

      //-----------------------------------------------------------------------
      void SCTPTester::onDataChannelError(
                                          IDataChannelPtr channel,
                                          ErrorAnyPtr error
                                          )
      {
        ZS_LOG_ERROR(Basic, log("data channel error") + ZS_PARAM("channel", channel->getID()) + ZS_PARAM("error code", error->mErrorCode) + ZS_PARAM("reason", error->mReason))
        AutoRecursiveLock lock(*this);
        ++mExpectations.mError;
      }

      //-----------------------------------------------------------------------
      void SCTPTester::onDataChannelBufferedAmountLow(IDataChannelPtr channel)
      {
        ZS_LOG_BASIC(log("data channel buffer low event") + ZS_PARAM("channel id", channel->getID()))
#define TODO_TEST_TO_VERIFY_BUFFER_LOW_EVENT 1
#define TODO_TEST_TO_VERIFY_BUFFER_LOW_EVENT 2
      }

      //-----------------------------------------------------------------------
      void SCTPTester::onDataChannelMessage(
                                            IDataChannelPtr channel,
                                            MessageEventDataPtr data
                                            )
      {

        AutoRecursiveLock lock(*this);

        auto params = channel->parameters();

        if (data->mBinary) {
          ZS_LOG_DETAIL(log("data channel binary message") + ZS_PARAM("channel id", channel->getID()) + ZS_PARAM("data", data->mBinary->SizeInBytes()))

          auto found = mBuffers.find(params->mLabel);
          TESTING_CHECK(found != mBuffers.end())

          BufferList &bufferList = (*found).second;

          TESTING_CHECK(bufferList.size() > 0)

          TESTING_CHECK(0 == UseServicesHelper::compare(*(bufferList.front()), *(data->mBinary)))

          ++mExpectations.mReceivedBinary;

          bufferList.pop_front();
        } else {
          ZS_LOG_DETAIL(log("data channel text message") + ZS_PARAM("channel id", channel->getID()) + ZS_PARAM("data", data->mText))

          auto found = mStrings.find(params->mLabel);
          TESTING_CHECK(found != mStrings.end())

          StringList &stringList = (*found).second;

          TESTING_CHECK(stringList.size() > 0)

          TESTING_EQUAL(stringList.front(), data->mText)

          ++mExpectations.mReceivedText;

          stringList.pop_front();
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTester => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params SCTPTester::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::sctp::SCTPTester");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      FakeICETransportPtr SCTPTester::getICETransport() const
      {
        AutoRecursiveLock lock(*this);
        return mICETransport;
      }

      //-----------------------------------------------------------------------
      void SCTPTester::expectData(
                                  const char *inChannelID,
                                  SecureByteBlockPtr data
                                  )
      {
        String channelID(inChannelID);

        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("expecting buffer") + ZS_PARAM("channel id", channelID) + ZS_PARAM("buffer size", data->SizeInBytes()))

        auto found = mBuffers.find(channelID);
        if (found == mBuffers.end()) {
          BufferList buffers;
          buffers.push_back(data);
          mBuffers[channelID] = buffers;
          return;
        }

        BufferList &bufferList = (*found).second;

        bufferList.push_back(data);
      }

      //-----------------------------------------------------------------------
      void SCTPTester::expectData(
                                  const char *inChannelID,
                                  const String &message
                                  )
      {
        String channelID(inChannelID);

        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("expecting buffer") + ZS_PARAM("channel id", channelID) + ZS_PARAM("message", message))

        auto found = mStrings.find(channelID);
        if (found == mStrings.end()) {
          StringList stringList;
          stringList.push_back(message);
          mStrings[channelID] = stringList;
          return;
        }

        StringList &stringList = (*found).second;

        stringList.push_back(message);
      }

    }
  }
}

ZS_DECLARE_USING_PTR(ortc::test::sctp, FakeICETransport)
ZS_DECLARE_USING_PTR(ortc::test::sctp, SCTPTester)
ZS_DECLARE_USING_PTR(ortc, IICETransport)
ZS_DECLARE_USING_PTR(ortc, IDTLSTransport)
using ortc::IDTLSTransportTypes;
ZS_DECLARE_USING_PTR(ortc, IDataChannel)
ZS_DECLARE_USING_PTR(ortc, ISCTPTransport)

using ortc::IICETypes;
using zsLib::Optional;
using zsLib::WORD;
using zsLib::BYTE;
using zsLib::Milliseconds;
using ortc::SecureByteBlock;
using ortc::SecureByteBlockPtr;

#define TEST_BASIC_CONNECTIVITY 0
#define TEST_INCOMING_SCTP 1
#define TEST_INCOMING_DELAYED_SCTP 2

static void bogusSleep()
{
  for (int loop = 0; loop < 100; ++loop)
  {
    TESTING_SLEEP(100)
  }
}

void doTestSCTP()
{
  if (!ORTC_TEST_DO_SCTP_TRANSPORT_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  UseSettings::applyDefaults();

  auto thread(zsLib::IMessageQueueThread::createBasic());

  SCTPTesterPtr testSCTPObject1;
  SCTPTesterPtr testSCTPObject2;

  TESTING_STDOUT() << "WAITING:      Waiting for SCTP testing to complete (max wait is 180 seconds).\n";

  // check to see if all DNS routines have resolved
  {
    ULONG testNumber = 0;
    ULONG maxSteps = 80;

    do
    {
      TESTING_STDOUT() << "TESTING       ---------->>>>>>>>>> " << testNumber << " <<<<<<<<<<----------\n";

      bool quit = false;
      ULONG expecting = 0;

      SCTPTester::Expectations expectationsSCTP1;
      SCTPTester::Expectations expectationsSCTP2;

      expectationsSCTP1.mStateConnecting = 0;
      expectationsSCTP1.mStateOpen = 1;
      expectationsSCTP1.mStateClosing = 1;
      expectationsSCTP1.mStateClosed = 1;

      expectationsSCTP2 = expectationsSCTP1;

      switch (testNumber) {
        case TEST_BASIC_CONNECTIVITY: {
          {
            testSCTPObject1 = SCTPTester::create(thread);
            testSCTPObject2 = SCTPTester::create(thread);

            TESTING_CHECK(testSCTPObject1)
            TESTING_CHECK(testSCTPObject2)

            testSCTPObject1->setClientRole(true);
            testSCTPObject2->setClientRole(false);

            expectationsSCTP2.mIncoming = 1;

            expectationsSCTP2.mReceivedBinary = 0;
            expectationsSCTP2.mReceivedText = 1;
          }
          break;
        }
        case TEST_INCOMING_SCTP:
        {
          testSCTPObject1 = SCTPTester::create(thread, true, 7000, 9000);
          testSCTPObject2 = SCTPTester::create(thread, false);

          TESTING_CHECK(testSCTPObject1)
          TESTING_CHECK(testSCTPObject2)

          testSCTPObject1->setClientRole(true);
          testSCTPObject2->setClientRole(false);

          expectationsSCTP2.mReceivedBinary = 3;
          expectationsSCTP2.mReceivedText = 0;

          expectationsSCTP2.mTransportIncoming = 1;
          break;
        }
        case TEST_INCOMING_DELAYED_SCTP:
        {
          testSCTPObject1 = SCTPTester::create(thread, true, 5000, Optional<WORD>(), Milliseconds(300));
          testSCTPObject2 = SCTPTester::create(thread, false);

          TESTING_CHECK(testSCTPObject1)
          TESTING_CHECK(testSCTPObject2)

          testSCTPObject1->setClientRole(true);
          testSCTPObject2->setClientRole(false);

          expectationsSCTP2.mIncoming = 1;

          expectationsSCTP2.mReceivedBinary = 2;
          expectationsSCTP1.mReceivedText = 3;

          expectationsSCTP2.mTransportIncoming = 1;
          expectationsSCTP1.mError = 1;
          break;
        }
        default:  quit = true; break;
      }
      if (quit) break;

      expecting = 0;
      expecting += (testSCTPObject1 ? 1 : 0);
      expecting += (testSCTPObject2 ? 1 : 0);

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
          case TEST_BASIC_CONNECTIVITY: {
            switch (step) {
              case 2: {
                if (testSCTPObject1) testSCTPObject1->start(testSCTPObject2);
                //bogusSleep();
                break;
              }
              case 3: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Checking);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Checking);
                //bogusSleep();
                break;
              }
              case 5: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Connected);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Connected);
                //bogusSleep();
                break;
              }
              case 7: {
                if (testSCTPObject1) testSCTPObject1->state(IDTLSTransportTypes::State_Connecting);
                if (testSCTPObject2) testSCTPObject2->state(IDTLSTransportTypes::State_Connecting);
                //bogusSleep();
                break;
              }
              case 10: {
                //bogusSleep();
                break;
              }
              case 11: {
                if (testSCTPObject1) testSCTPObject1->state(IDTLSTransportTypes::State_Connected);
                if (testSCTPObject2) testSCTPObject2->state(IDTLSTransportTypes::State_Connected);
                //bogusSleep();
                break;
              }
              case 12: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Completed);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Completed);
                //bogusSleep();
                break;
              }
              case 15: {
                IDataChannel::Parameters params;
                params.mLabel = "foo1";
                if (testSCTPObject1) testSCTPObject1->createChannel(params);
                //bogusSleep();
                break;
              }
              case 25: {
                if (testSCTPObject1) testSCTPObject1->sendData("foo1", UseServicesHelper::randomString(10));
                //bogusSleep();
                break;
              }
              case 40: {
                if (testSCTPObject1) testSCTPObject1->closeChannel("foo1");
                //bogusSleep();
                break;
              }
              case 44: {
                if (testSCTPObject1) testSCTPObject1->close();
                if (testSCTPObject2) testSCTPObject1->close();
                //bogusSleep();
                break;
              }
              case 46: {
                if (testSCTPObject1) testSCTPObject1->state(IDTLSTransportTypes::State_Closed);
                if (testSCTPObject2) testSCTPObject2->state(IDTLSTransportTypes::State_Closed);
                //bogusSleep();
                break;
              }
              case 47: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Disconnected);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Disconnected);
                //bogusSleep();
                break;
              }
              case 49: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Closed);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Closed);
                //bogusSleep();
                break;
              }
              case 50: {
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
          case TEST_INCOMING_SCTP: {
            switch (step) {
              case 1: {
                if (testSCTPObject2) testSCTPObject2->listen();
                //bogusSleep();
                break;
              }
              case 2: {
                if (testSCTPObject1) testSCTPObject1->start(testSCTPObject2);
                //bogusSleep();
                break;
              }
              case 3: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Checking);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Checking);
                //bogusSleep();
                break;
              }
              case 5: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Connected);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Connected);
                //bogusSleep();
                break;
              }
              case 7: {
                if (testSCTPObject1) testSCTPObject1->state(IDTLSTransportTypes::State_Connecting);
                if (testSCTPObject2) testSCTPObject2->state(IDTLSTransportTypes::State_Connecting);
                //bogusSleep();
                break;
              }
              case 10: {
                //bogusSleep();
                break;
              }
              case 11: {
                if (testSCTPObject1) testSCTPObject1->state(IDTLSTransportTypes::State_Connected);
                if (testSCTPObject2) testSCTPObject2->state(IDTLSTransportTypes::State_Connected);
                //bogusSleep();
                break;
              }
              case 12: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Completed);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Completed);
                //bogusSleep();
                break;
              }
              case 15: {
                IDataChannel::Parameters params;
                params.mLabel = "foo1";
                params.mID = 0;
                params.mNegotiated = true;
                if (testSCTPObject1) testSCTPObject1->createChannel(params);
                if (testSCTPObject2) testSCTPObject2->createChannel(params);
                //bogusSleep();
                break;
              }
              case 25: {
                if (testSCTPObject1) testSCTPObject1->sendData("foo1", UseServicesHelper::random(20));
                if (testSCTPObject1) testSCTPObject1->sendData("foo1", UseServicesHelper::random(20));
                if (testSCTPObject1) testSCTPObject1->sendData("foo1", UseServicesHelper::random(20));
                //bogusSleep();
                break;
              }
              case 40: {
                if (testSCTPObject1) testSCTPObject1->closeChannel("foo1");
                //bogusSleep();
                break;
              }
              case 44: {
                if (testSCTPObject1) testSCTPObject1->close();
                if (testSCTPObject2) testSCTPObject1->close();
                //bogusSleep();
                break;
              }
              case 46: {
                if (testSCTPObject1) testSCTPObject1->state(IDTLSTransportTypes::State_Closed);
                if (testSCTPObject2) testSCTPObject2->state(IDTLSTransportTypes::State_Closed);
                //bogusSleep();
                break;
              }
              case 47: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Disconnected);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Disconnected);
                //bogusSleep();
                break;
              }
              case 49: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Closed);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Closed);
                //bogusSleep();
                break;
              }
              case 50: {
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
          case TEST_INCOMING_DELAYED_SCTP: {
            switch (step) {
              case 1: {
                if (testSCTPObject2) testSCTPObject2->listen();
                //bogusSleep();
                break;
              }
              case 2: {
                if (testSCTPObject1) testSCTPObject1->start(testSCTPObject2);
                IDataChannel::Parameters params;
                params.mLabel = "foo1";
                params.mID = 0;
                if (testSCTPObject1) testSCTPObject1->createChannel(params);
                const char *bufferedStr = "buffing-this-string-until-connected";
                SecureByteBlockPtr buffer(std::make_shared<SecureByteBlock>((const BYTE *)bufferedStr, strlen(bufferedStr)));
                if (testSCTPObject1) testSCTPObject1->sendData("foo1", buffer);
                //bogusSleep();
                break;
              }
              case 3: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Checking);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Checking);
                //bogusSleep();
                break;
              }
              case 5: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Connected);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Connected);
                //bogusSleep();
                break;
              }
              case 7: {
                if (testSCTPObject1) testSCTPObject1->state(IDTLSTransportTypes::State_Connecting);
                if (testSCTPObject2) testSCTPObject2->state(IDTLSTransportTypes::State_Connecting);
                //bogusSleep();
                break;
              }
              case 10: {
                //bogusSleep();
                break;
              }
              case 11: {
                if (testSCTPObject1) testSCTPObject1->state(IDTLSTransportTypes::State_Connected);
                if (testSCTPObject2) testSCTPObject2->state(IDTLSTransportTypes::State_Connected);
                //bogusSleep();
                break;
              }
              case 12: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Completed);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Completed);
                //bogusSleep();
                break;
              }
              case 25: {
                if (testSCTPObject2) testSCTPObject2->sendData("foo1", UseServicesHelper::randomString(1024));
                if (testSCTPObject2) testSCTPObject2->sendData("foo1", UseServicesHelper::randomString(4098));
                if (testSCTPObject2) testSCTPObject2->sendData("foo1", UseServicesHelper::randomString(8192));
                //bogusSleep();
                break;
              }
              case 33: {
                auto remoteCaps = ISCTPTransport::getCapabilities();

                if (testSCTPObject2) testSCTPObject1->sendData("foo1", UseServicesHelper::random(remoteCaps->mMaxMessageSize));
                //bogusSleep();
                break;
              }
              case 39: {
                auto remoteCaps = ISCTPTransport::getCapabilities();
                if (testSCTPObject2) testSCTPObject1->sendData("foo1", UseServicesHelper::random(remoteCaps->mMaxMessageSize+1));
                //bogusSleep();
                break;
              }
              case 40: {
                // if (testSCTPObject1) testSCTPObject1->closeChannel("foo1");  // DO NOT CLOSE - ERROR SHOULD CLOSE IT
                //bogusSleep();
                break;
              }
              case 44: {
                if (testSCTPObject1) testSCTPObject1->close();
                if (testSCTPObject2) testSCTPObject1->close();
                //bogusSleep();
                break;
              }
              case 46: {
                if (testSCTPObject1) testSCTPObject1->state(IDTLSTransportTypes::State_Closed);
                if (testSCTPObject2) testSCTPObject2->state(IDTLSTransportTypes::State_Closed);
                //bogusSleep();
                break;
              }
              case 47: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Disconnected);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Disconnected);
                //bogusSleep();
                break;
              }
              case 49: {
                if (testSCTPObject1) testSCTPObject1->state(IICETransport::State_Closed);
                if (testSCTPObject2) testSCTPObject2->state(IICETransport::State_Closed);
                //bogusSleep();
                break;
              }
              case 50: {
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
          found += (testSCTPObject1 ? (testSCTPObject1->matches(expectationsSCTP1) ? 1 : 0) : 0);
          found += (testSCTPObject2 ? (testSCTPObject2->matches(expectationsSCTP2) ? 1 : 0) : 0);
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
          if (testSCTPObject1) {TESTING_CHECK(testSCTPObject1->matches(expectationsSCTP1))}
          if (testSCTPObject2) {TESTING_CHECK(testSCTPObject2->matches(expectationsSCTP2))}
      //    break;
        }
      //}

      testSCTPObject1.reset();
      testSCTPObject2.reset();

      ++testNumber;
    } while (true);
  }

  TESTING_STDOUT() << "WAITING:      All SCTP transports have finished. Waiting for 'bogus' events to process (10 second wait).\n";
  TESTING_SLEEP(10000)

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
