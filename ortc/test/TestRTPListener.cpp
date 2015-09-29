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


#include <zsLib/MessageQueueThread.h>

#include "TestRTPListener.h"

#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/ISettings.h>

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
using zsLib::IPromiseSettledDelegate;
using namespace zsLib::XML;

ZS_DECLARE_TYPEDEF_PTR(ortc::ISettings, UseSettings)
ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)

namespace ortc
{
  namespace test
  {
    namespace rtplistener
    {
      ZS_DECLARE_CLASS_PTR(FakeICETransport)
      ZS_DECLARE_CLASS_PTR(FakeSecureTransport)
      ZS_DECLARE_CLASS_PTR(RTPListenerTester)

      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiver)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSender)

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
          mTimer = Timer::create(mThisWeak.lock(), mPacketDelay);
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

        ElementPtr resultEl = Element::create("ortc::test::rtplistener::FakeICETransport");

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
            ZS_LOG_WARNING(Detail, log("no rtplistener transport attached (thus cannot forward received packet)") + ZS_PARAM("buffer", (PTRNUMBER)(buffer->BytePtr())) + ZS_PARAM("buffer size", buffer->SizeInBytes()))
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
      void FakeICETransport::onTimer(TimerPtr timer)
      {
        FakeSecureTransportPtr transport;

        DelayedBufferList delayedPackets;

        auto tick = zsLib::now();

        {
          AutoRecursiveLock lock(*this);
          transport = mSecureTransport.lock();
          if (!transport) {
            ZS_LOG_WARNING(Detail, log("no rtplistener transport attached (thus cannot forward delayed packets)"))
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
          mSubscriptions.delegate()->onICETransportStateChanged(pThis, mCurrentState);
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
        ElementPtr objectEl = Element::create("ortc::test::rtplistener::FakeICETransport");
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

        mListener = UseListener::create(mThisWeak.lock());
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

        ElementPtr resultEl = Element::create("ortc::test::rtplistener::FakeICETransport");

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
      #pragma mark FakeSecureTransport => ISecureTransportForRTPListener
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
      PUID FakeSecureTransport::getID() const
      {
        return DTLSTransport::getID();
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
          if (State_Validated != mCurrentState) {
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

          if (State_Validated != mCurrentState) {
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
      #pragma mark FakeSecureTransport => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeSecureTransport::setState(IDTLSTransportTypes::States state)
      {
        if (state == mCurrentState) return;

        ZS_LOG_DETAIL(log("state changed") + ZS_PARAM("new state", IDTLSTransport::toString(state)) + ZS_PARAM("old state", IDTLSTransport::toString(mCurrentState)))

        mCurrentState = state;
      }

      //-----------------------------------------------------------------------
      bool FakeSecureTransport::isShutdown()
      {
        return IDTLSTransport::State_Closed == mCurrentState;
      }

      //-----------------------------------------------------------------------
      Log::Params FakeSecureTransport::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtplistener::FakeSecureTransport");
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
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver
      #pragma mark

      //-----------------------------------------------------------------------
      FakeReceiver::FakeReceiver() :
        RTPReceiver(Noop(true))
      {
      }

      //-----------------------------------------------------------------------
      FakeReceiver::~FakeReceiver()
      {
        mThisWeak.reset();
        stop();
      }

      //-----------------------------------------------------------------------
      FakeReceiverPtr FakeReceiver::create()
      {
        FakeReceiverPtr pThis(make_shared<FakeReceiver>());
        pThis->mThisWeak = pThis;
        return pThis;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver => IRTPReceiverForRTPListener
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeReceiver::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::rtplistener::FakeReceiver");

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());
        UseServicesHelper::debugAppend(result, mParameters ? mParameters->toDebug() : ElementPtr());
        UseServicesHelper::debugAppend(result, "buffers", mBuffers.size());
        UseServicesHelper::debugAppend(result, "listener", mListener ? mListener->getID() : 0);

        return result;
      }

      //-----------------------------------------------------------------------
      bool FakeReceiver::handlePacket(
                                      IICETypes::Components viaTransport,
                                      RTPPacketPtr packet
                                      )
      {
        ZS_LOG_BASIC(log("received RTC packet") + packet->toDebug())

        AutoRecursiveLock lock(*this);

        TESTING_CHECK(mBuffers.size() > 0)

        TESTING_CHECK(0 == UseServicesHelper::compare(*(mBuffers.front()), *(packet->mBuffer)))

        mBuffers.pop_front();

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

        TESTING_CHECK(mBuffers.size() > 0)

        TESTING_CHECK(0 == UseServicesHelper::compare(*(mBuffers.front()), *(packet->mBuffer)))

        mBuffers.pop_front();

        auto tester = mTester.lock();

        if (tester) tester->notifyReceivedPacket();
        
        return true;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver => (friend RTPListenerTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeReceiver::setTransport(RTPListenerTesterPtr tester)
      {
        auto secureTransport = (((bool)tester) ? tester->getFakeSecureTransport() : FakeSecureTransportPtr());
        TESTING_CHECK(secureTransport)


        AutoRecursiveLock lock(*this);

        mTester = tester;

        if (mListener) {
          ZS_LOG_BASIC(log("unregistering listener") + ZS_PARAM("listener", mListener->getID()))
          mListener->unregisterReceiver(*this);
        }

        if (secureTransport) {
          mListener = secureTransport->getListener();

          if (mParameters) {
            ZS_LOG_BASIC(log("registering listener") + ZS_PARAM("listener", mListener->getID()) + mParameters->toDebug())

            RTCPPacketList packetList;
            mListener->registerReceiver(mThisWeak.lock(), *mParameters, packetList);

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
      void FakeReceiver::expectData(SecureByteBlockPtr data)
      {
        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("expecting buffer") + ZS_PARAM("buffer size", data->SizeInBytes()))

        mBuffers.push_back(data);
      }

      //-----------------------------------------------------------------------
      void FakeReceiver::receive(const Parameters &parameters)
      {
        AutoRecursiveLock lock(*this);

        mParameters = ParametersPtr(make_shared<Parameters>(parameters));

        if (mListener) {
          RTCPPacketList packetList;
          mListener->registerReceiver(mThisWeak.lock(), *mParameters, packetList);

          for (auto iter = packetList.begin(); iter != packetList.end(); ++iter) {
            // fake this as if it was a received packet
            handlePacket(IICETypes::Component_RTP, *iter);
          }
        }
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
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params FakeReceiver::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtplistener::FakeReceiver");
        UseServicesHelper::debugAppend(objectEl, "id", RTPReceiver::getID());
        return Log::Params(message, objectEl);
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSender
      #pragma mark

      //-----------------------------------------------------------------------
      FakeSender::FakeSender() :
        RTPSender(Noop(true))
      {
      }

      //-----------------------------------------------------------------------
      FakeSender::~FakeSender()
      {
        mThisWeak.reset();
      }

      //-----------------------------------------------------------------------
      FakeSenderPtr FakeSender::create()
      {
        FakeSenderPtr pThis(make_shared<FakeSender>());
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

        ElementPtr result = Element::create("ortc::test::rtplistener::FakeSender");

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());
        UseServicesHelper::debugAppend(result, mParameters ? mParameters->toDebug() : ElementPtr());
        UseServicesHelper::debugAppend(result, "buffers", mBuffers.size());
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

        TESTING_CHECK(mBuffers.size() > 0)

        TESTING_CHECK(0 == UseServicesHelper::compare(*(mBuffers.front()), *(packet->mBuffer)))

        mBuffers.pop_front();

        auto tester = mTester.lock();

        if (tester) tester->notifyReceivedPacket();

        return true;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSender => (friend RTPListenerTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeSender::setTransport(RTPListenerTesterPtr tester)
      {
        auto secureTransport = (((bool)tester) ? tester->getFakeSecureTransport() : FakeSecureTransportPtr());
        TESTING_CHECK(secureTransport)


        AutoRecursiveLock lock(*this);

        mTester = tester;

        if (mListener) {
          ZS_LOG_BASIC(log("unregistering listener") + ZS_PARAM("listener", mListener->getID()))
          mListener->unregisterSender(*this);
        }

        if (secureTransport) {
          mListener = secureTransport->getListener();

          if (mParameters) {
            ZS_LOG_BASIC(log("registering listener") + ZS_PARAM("listener", mListener->getID()) + mParameters->toDebug())

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
      void FakeSender::send(const Parameters &parameters)
      {
        AutoRecursiveLock lock(*this);

        mParameters = ParametersPtr(make_shared<Parameters>(parameters));

        if (mListener) {
          RTCPPacketList packetList;
          mListener->registerSender(mThisWeak.lock(), *mParameters, packetList);

          for (auto iter = packetList.begin(); iter != packetList.end(); ++iter) {
            // fake this as if it was a received packet
            handlePacket(IICETypes::Component_RTP, *iter);
          }
        }
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
        AutoRecursiveLock lock(*this);

        ZS_LOG_TRACE(log("expecting buffer") + ZS_PARAM("buffer size", data->SizeInBytes()))

        mBuffers.push_back(data);
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
        ElementPtr objectEl = Element::create("ortc::test::rtplistener::FakeSender");
        UseServicesHelper::debugAppend(objectEl, "id", FakeSender::getID());
        return Log::Params(message, objectEl);
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListenerTester
      #pragma mark

      //-----------------------------------------------------------------------
      RTPListenerTester::UnhandledEventData::UnhandledEventData(
                                                                DWORD ssrc,
                                                                BYTE pt,
                                                                const char *mid
                                                                ) :
        mSSRC(ssrc),
        mPT(pt),
        mMID(mid)
      {
      }

      //-----------------------------------------------------------------------
      bool RTPListenerTester::UnhandledEventData::operator==(const UnhandledEventData &op2) const
      {
        return ((mSSRC == op2.mSSRC) &&
                (mPT == op2.mPT) &&
                (mMID == op2.mMID));
      }
      
      //-----------------------------------------------------------------------
      bool RTPListenerTester::Expectations::operator==(const Expectations &op2) const
      {
        return (mStateConnecting == op2.mStateConnecting) &&
               (mStateOpen == op2.mStateOpen) &&
               (mStateClosing == op2.mStateClosing) &&
               (mStateClosed == op2.mStateClosed) &&

               (mUnhandled == op2.mUnhandled) &&
               (mReceivedPackets == op2.mReceivedPackets) &&

               (mError == op2.mError);
      }

      //-----------------------------------------------------------------------
      RTPListenerTesterPtr RTPListenerTester::create(
                                                     IMessageQueuePtr queue,
                                                     Milliseconds packetDelay
                                                     )
      {
        RTPListenerTesterPtr pThis(new RTPListenerTester(queue));
        pThis->mThisWeak = pThis;
        pThis->init(packetDelay);
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPListenerTester::RTPListenerTester(IMessageQueuePtr queue) :
        SharedRecursiveLock(SharedRecursiveLock::create()),
        MessageQueueAssociator(queue)
      {
        ZS_LOG_BASIC(log("rtplistener tester"))
      }

      //-----------------------------------------------------------------------
      RTPListenerTester::~RTPListenerTester()
      {
        ZS_LOG_BASIC(log("rtplistener tester"))
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::init(Milliseconds packetDelay)
      {
        AutoRecursiveLock lock(*this);
        mICETransport = FakeICETransport::create(getAssociatedMessageQueue(), packetDelay);
        mDTLSTransport = FakeSecureTransport::create(getAssociatedMessageQueue(), mICETransport);
      }

      //-----------------------------------------------------------------------
      bool RTPListenerTester::matches(const Expectations &op2)
      {
        AutoRecursiveLock lock(*this);
        return mExpectations == op2;
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::close()
      {
        AutoRecursiveLock lock(*this);
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::closeByReset()
      {
        AutoRecursiveLock lock(*this);
        mICETransport.reset();
        mDTLSTransport.reset();
      }

      //-----------------------------------------------------------------------
      RTPListenerTester::Expectations RTPListenerTester::getExpectations() const
      {
        return mExpectations;
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::state(IICETransport::States newState)
      {
        FakeICETransportPtr transport;
        {
          AutoRecursiveLock lock(*this);
          transport = mICETransport;
        }
        transport->state(newState);
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::state(IDTLSTransport::States newState)
      {
        FakeSecureTransportPtr transport;
        {
          AutoRecursiveLock lock(*this);
          transport = mDTLSTransport;
        }
        transport->state(newState);
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::setClientRole(bool clientRole)
      {
        FakeSecureTransportPtr transport;
        {
          AutoRecursiveLock lock(*this);
          transport = mDTLSTransport;
        }
        transport->setClientRole(clientRole);
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::connect(RTPListenerTesterPtr remote)
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
      void RTPListenerTester::send(
                                   const char *senderID,
                                   const Parameters &params
                                   )
      {
        FakeSenderPtr sender = getSender(senderID);

        if (!sender) {
          sender = FakeSender::create();
          attach(senderID, sender);
        }

        TESTING_CHECK(sender)

        sender->send(params);
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::receive(
                                      const char *receiverID,
                                      const Parameters &params
                                      )
      {
        FakeReceiverPtr receiver = getReceiver(receiverID);

        if (!receiver) {
          receiver = FakeReceiver::create();
          attach(receiverID, receiver);
        }

        TESTING_CHECK(receiver)

        receiver->receive(params);
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::stop(const char *senderOrReceiverID)
      {
        AutoRecursiveLock lock(*this);

        {
          auto found = mReceivers.find(String(senderOrReceiverID));
          if (mReceivers.end() != found) {
            (*found).second->stop();
            mReceivers.erase(found);
            return;
          }
        }

        {
          auto found = mSenders.find(String(senderOrReceiverID));
          TESTING_CHECK(mSenders.end() != found)
          (*found).second->stop();
          mSenders.erase(found);
        }
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::expectData(
                                         const char *senderOrReceiverID,
                                         SecureByteBlockPtr secureBuffer
                                         )
      {
        {
          FakeSenderPtr sender = getSender(senderOrReceiverID);

          if (sender) {
            sender->expectData(secureBuffer);
            return;
          }
        }

        {
          FakeReceiverPtr receiver = getReceiver(senderOrReceiverID);
          TESTING_CHECK(receiver)

          receiver->expectData(secureBuffer);
        }
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::sendData(
                                       const char *senderID,
                                       SecureByteBlockPtr secureBuffer
                                       )
      {
        FakeSenderPtr sender = getSender(senderID);
        TESTING_CHECK(sender)

        sender->sendPacket(secureBuffer);
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::attach(
                                     const char *receiverID,
                                     FakeReceiverPtr receiver
                                     )
      {
        AutoRecursiveLock lock(*this);

        auto previousReceiver = getReceiver(receiverID);
        TESTING_CHECK(!previousReceiver)

        mReceivers[String(receiverID)] = receiver;
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::attach(
                                     const char *senderID,
                                     FakeSenderPtr sender
                                     )
      {
        AutoRecursiveLock lock(*this);

        auto previousSender = getSender(senderID);
        TESTING_CHECK(!previousSender)

        mSenders[String(senderID)] = sender;
      }

      //-----------------------------------------------------------------------
      FakeReceiverPtr RTPListenerTester::detachReceiver(const char *receiverID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mReceivers.find(String(receiverID));
        TESTING_CHECK(found != mReceivers.end())

        FakeReceiverPtr result = (*found).second;

        mReceivers.erase(found);

        return result;
      }
      
      //-----------------------------------------------------------------------
      FakeSenderPtr RTPListenerTester::detachSender(const char *senderID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mSenders.find(String(senderID));
        TESTING_CHECK(found != mSenders.end())

        FakeSenderPtr result = (*found).second;

        mSenders.erase(found);
        
        return result;
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::expectingUnhandled(
                                                 SSRCType ssrc,
                                                 PayloadType payloadType,
                                                 const char *mid
                                                 )
      {
        AutoRecursiveLock lock(*this);

        mExpectingUnhandled.push_back(UnhandledEventData(ssrc, payloadType, mid));
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListenerTester::IRTPListenerDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void RTPListenerTester::onRTPListenerUnhandledRTP(
                                                        IRTPListenerPtr listener,
                                                        SSRCType ssrc,
                                                        PayloadType payloadType,
                                                        const char *mid
                                                        )
      {
        ZS_LOG_BASIC(log("rtp listener unhandled rtp event") + ZS_PARAM("listener id", listener->getID()) + ZS_PARAM("ssrc", ssrc) + ZS_PARAM("payload type", payloadType) + ZS_PARAM("mid", mid))

        AutoRecursiveLock lock(*this);
        ++mExpectations.mUnhandled;

        TESTING_CHECK(mExpectingUnhandled.size() > 0)

        auto &expecting = mExpectingUnhandled.front();

        TESTING_CHECK(expecting == UnhandledEventData(ssrc, payloadType, mid))

        mExpectingUnhandled.pop_front();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListenerTester => (friend fake sender/receiver)
      #pragma mark

      //-----------------------------------------------------------------------
      FakeSecureTransportPtr RTPListenerTester::getFakeSecureTransport() const
      {
        return mDTLSTransport;
      }

      //-----------------------------------------------------------------------
      void RTPListenerTester::notifyReceivedPacket()
      {
        ZS_LOG_BASIC(log("notified received packet"))

        AutoRecursiveLock lock(*this);
        ++mExpectations.mReceivedPackets;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPListenerTester => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params RTPListenerTester::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtplistener::RTPListenerTester");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      FakeICETransportPtr RTPListenerTester::getICETransport() const
      {
        AutoRecursiveLock lock(*this);
        return mICETransport;
      }

      //-----------------------------------------------------------------------
      FakeReceiverPtr RTPListenerTester::getReceiver(const char *receiverID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mReceivers.find(String(receiverID));
        if (mReceivers.end() == found) return FakeReceiverPtr();
        return (*found).second;
      }

      //-----------------------------------------------------------------------
      FakeSenderPtr RTPListenerTester::getSender(const char *senderID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mSenders.find(String(senderID));
        if (mSenders.end() == found) return FakeSenderPtr();
        return (*found).second;
      }

    }
  }
}

ZS_DECLARE_USING_PTR(ortc::test::rtplistener, FakeICETransport)
ZS_DECLARE_USING_PTR(ortc::test::rtplistener, RTPListenerTester)
ZS_DECLARE_USING_PTR(ortc, IICETransport)
ZS_DECLARE_USING_PTR(ortc, IDTLSTransport)
ZS_DECLARE_USING_PTR(ortc, IDataChannel)
ZS_DECLARE_USING_PTR(ortc, IRTPListener)

ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::Parameters, Parameters)

using ortc::IICETypes;
using zsLib::Optional;
using zsLib::WORD;
using zsLib::BYTE;
using zsLib::Milliseconds;
using ortc::SecureByteBlock;
using ortc::SecureByteBlockPtr;

#define TEST_BASIC_CONNECTIVITY 0
#define TEST_INCOMING_RTPListener 1
#define TEST_INCOMING_DELAYED_RTPListener 2

static void bogusSleep()
{
  for (int loop = 0; loop < 100; ++loop)
  {
    TESTING_SLEEP(100)
  }
}

void doTestRTPListener()
{
  if (!ORTC_TEST_DO_RTP_LISTENER_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  ortc::ISettings::applyDefaults();

  zsLib::MessageQueueThreadPtr thread(zsLib::MessageQueueThread::createBasic());

  RTPListenerTesterPtr testRTPListenerObject1;
  RTPListenerTesterPtr testRTPListenerObject2;

  TESTING_STDOUT() << "WAITING:      Waiting for RTPListener testing to complete (max wait is 180 seconds).\n";

  // check to see if all DNS routines have resolved
  {
    ULONG testNumber = 0;
    ULONG maxSteps = 80;

    do
    {
      TESTING_STDOUT() << "TESTING       ---------->>>>>>>>>> " << testNumber << " <<<<<<<<<<----------\n";

      bool quit = false;
      ULONG expecting = 0;

      RTPListenerTester::Expectations expectationsRTPListener1;
      RTPListenerTester::Expectations expectationsRTPListener2;

      expectationsRTPListener1.mStateConnecting = 0;
      expectationsRTPListener1.mStateOpen = 1;
      expectationsRTPListener1.mStateClosing = 1;
      expectationsRTPListener1.mStateClosed = 1;

      expectationsRTPListener2 = expectationsRTPListener1;

      switch (testNumber) {
        case TEST_BASIC_CONNECTIVITY: {
          {
            testRTPListenerObject1 = RTPListenerTester::create(thread);
            testRTPListenerObject2 = RTPListenerTester::create(thread);

            TESTING_CHECK(testRTPListenerObject1)
            TESTING_CHECK(testRTPListenerObject2)

            testRTPListenerObject1->setClientRole(true);
            testRTPListenerObject2->setClientRole(false);
          }
          break;
        }
        case TEST_INCOMING_RTPListener:
        {
          testRTPListenerObject1 = RTPListenerTester::create(thread);
          testRTPListenerObject2 = RTPListenerTester::create(thread);

          TESTING_CHECK(testRTPListenerObject1)
          TESTING_CHECK(testRTPListenerObject2)

          testRTPListenerObject1->setClientRole(true);
          testRTPListenerObject2->setClientRole(false);
          break;
        }
        case TEST_INCOMING_DELAYED_RTPListener:
        {
          testRTPListenerObject1 = RTPListenerTester::create(thread, Milliseconds(300));
          testRTPListenerObject2 = RTPListenerTester::create(thread);

          TESTING_CHECK(testRTPListenerObject1)
          TESTING_CHECK(testRTPListenerObject2)

          testRTPListenerObject1->setClientRole(true);
          testRTPListenerObject2->setClientRole(false);
          break;
        }
        default:  quit = true; break;
      }
      if (quit) break;

      expecting = 0;
      expecting += (testRTPListenerObject1 ? 1 : 0);
      expecting += (testRTPListenerObject2 ? 1 : 0);

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
                if (testRTPListenerObject1) testRTPListenerObject1->connect(testRTPListenerObject2);
                //bogusSleep();
                break;
              }
              case 3: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IICETransport::State_Completed);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IICETransport::State_Completed);
                //bogusSleep();
                break;
              }
              case 4: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IDTLSTransport::State_Validated);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IDTLSTransport::State_Validated);
                //bogusSleep();
                break;
              }
              case 5: {
                Parameters params;
                params.mMuxID = "r1";
                testRTPListenerObject1->receive("r1", params);
                bogusSleep();
                break;
              }
              case 6: {
                Parameters params;
                params.mMuxID = "s1";
                testRTPListenerObject1->send("s1", params);
                bogusSleep();
                break;
              }
              case 25: {
                //bogusSleep();
                break;
              }
              case 40: {
                //bogusSleep();
                break;
              }
              case 44: {
                if (testRTPListenerObject1) testRTPListenerObject1->close();
                if (testRTPListenerObject2) testRTPListenerObject1->close();
                //bogusSleep();
                break;
              }
              case 46: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IDTLSTransport::State_Closed);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IDTLSTransport::State_Closed);
                //bogusSleep();
                break;
              }
              case 47: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IICETransport::State_Disconnected);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IICETransport::State_Disconnected);
                //bogusSleep();
                break;
              }
              case 49: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IICETransport::State_Closed);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IICETransport::State_Closed);
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
          case TEST_INCOMING_RTPListener: {
            switch (step) {
              case 1: {
                //bogusSleep();
                break;
              }
              case 2: {
                if (testRTPListenerObject1) testRTPListenerObject1->connect(testRTPListenerObject2);
                //bogusSleep();
                break;
              }
              case 3: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IICETransport::State_Checking);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IICETransport::State_Checking);
                //bogusSleep();
                break;
              }
              case 4: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IICETransport::State_Connected);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IICETransport::State_Connected);
                //bogusSleep();
                break;
              }
              case 5: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IDTLSTransport::State_Connecting);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IDTLSTransport::State_Connecting);
                //bogusSleep();
                break;
              }
              case 6: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IDTLSTransport::State_Connected);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IDTLSTransport::State_Connected);
                //bogusSleep();
                break;
              }
              case 7: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IDTLSTransport::State_Validated);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IDTLSTransport::State_Validated);
                //bogusSleep();
                break;
              }
              case 8: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IICETransport::State_Completed);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IICETransport::State_Completed);
                //bogusSleep();
                break;
              }
              case 15: {
                //bogusSleep();
                break;
              }
              case 25: {
                //bogusSleep();
                break;
              }
              case 40: {
                //bogusSleep();
                break;
              }
              case 44: {
                if (testRTPListenerObject1) testRTPListenerObject1->close();
                if (testRTPListenerObject2) testRTPListenerObject1->close();
                //bogusSleep();
                break;
              }
              case 46: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IDTLSTransport::State_Closed);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IDTLSTransport::State_Closed);
                //bogusSleep();
                break;
              }
              case 47: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IICETransport::State_Disconnected);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IICETransport::State_Disconnected);
                //bogusSleep();
                break;
              }
              case 49: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IICETransport::State_Closed);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IICETransport::State_Closed);
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
          case TEST_INCOMING_DELAYED_RTPListener: {
            switch (step) {
              case 1: {
                //bogusSleep();
                break;
              }
              case 2: {
                if (testRTPListenerObject1) testRTPListenerObject1->connect(testRTPListenerObject2);
                //bogusSleep();
                break;
              }
              case 3: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IICETransport::State_Checking);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IICETransport::State_Checking);
                //bogusSleep();
                break;
              }
              case 4: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IICETransport::State_Connected);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IICETransport::State_Connected);
                //bogusSleep();
                break;
              }
              case 5: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IDTLSTransport::State_Connecting);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IDTLSTransport::State_Connecting);
                //bogusSleep();
                break;
              }
              case 6: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IDTLSTransport::State_Connected);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IDTLSTransport::State_Connected);
                //bogusSleep();
                break;
              }
              case 7: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IDTLSTransport::State_Validated);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IDTLSTransport::State_Validated);
                //bogusSleep();
                break;
              }
              case 8: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IICETransport::State_Completed);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IICETransport::State_Completed);
                //bogusSleep();
                break;
              }
              case 25: {
                //bogusSleep();
                break;
              }
              case 33: {
                //bogusSleep();
                break;
              }
              case 39: {
                //bogusSleep();
                break;
              }
              case 40: {
                // if (testRTPListenerObject1) testRTPListenerObject1->closeChannel("foo1");  // DO NOT CLOSE - ERROR SHOULD CLOSE IT
                //bogusSleep();
                break;
              }
              case 44: {
                if (testRTPListenerObject1) testRTPListenerObject1->close();
                if (testRTPListenerObject2) testRTPListenerObject1->close();
                //bogusSleep();
                break;
              }
              case 46: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IDTLSTransport::State_Closed);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IDTLSTransport::State_Closed);
                //bogusSleep();
                break;
              }
              case 47: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IICETransport::State_Disconnected);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IICETransport::State_Disconnected);
                //bogusSleep();
                break;
              }
              case 49: {
                if (testRTPListenerObject1) testRTPListenerObject1->state(IICETransport::State_Closed);
                if (testRTPListenerObject2) testRTPListenerObject2->state(IICETransport::State_Closed);
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
          found += (testRTPListenerObject1 ? (testRTPListenerObject1->matches(expectationsRTPListener1) ? 1 : 0) : 0);
          found += (testRTPListenerObject2 ? (testRTPListenerObject2->matches(expectationsRTPListener2) ? 1 : 0) : 0);
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
          if (testRTPListenerObject1) {TESTING_CHECK(testRTPListenerObject1->matches(expectationsRTPListener1))}
          if (testRTPListenerObject2) {TESTING_CHECK(testRTPListenerObject2->matches(expectationsRTPListener2))}
          break;
        }
      }

      testRTPListenerObject1.reset();
      testRTPListenerObject2.reset();

      ++testNumber;
    } while (true);
  }

  TESTING_STDOUT() << "WAITING:      All RTPListener transports have finished. Waiting for 'bogus' events to process (10 second wait).\n";
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

