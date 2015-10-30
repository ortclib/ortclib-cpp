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

#include "TestMediaStreamTrack.h"

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
using namespace zsLib::XML;

ZS_DECLARE_TYPEDEF_PTR(ortc::ISettings, UseSettings)
ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)

namespace ortc
{
  namespace test
  {
    namespace mediastreamtrack
    {
      ZS_DECLARE_CLASS_PTR(FakeReceiver)
      ZS_DECLARE_CLASS_PTR(FakeReceiverChannel)
      ZS_DECLARE_CLASS_PTR(MediaStreamTrackTester)

      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiver)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiverChannel)
      ZS_DECLARE_USING_PTR(ortc::internal, MediaStreamTrack)

      using zsLib::AutoRecursiveLock;

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark (helpers)
      #pragma mark


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver
      #pragma mark

      //-----------------------------------------------------------------------
      FakeReceiver::FakeReceiver(
                                 const make_private &,
                                 IMessageQueuePtr queue
                                 ) :
        RTPReceiver(zsLib::Noop(true))
      {
        ZS_LOG_BASIC(log("created"))
      }

      //-----------------------------------------------------------------------
      void FakeReceiver::init()
      {
        AutoRecursiveLock lock(*this);
      }

      //-----------------------------------------------------------------------
      FakeReceiver::~FakeReceiver()
      {
        mThisWeak.reset();

        ZS_LOG_BASIC(log("destroyed"))
      }

      //-----------------------------------------------------------------------
      FakeReceiverPtr FakeReceiver::create(
                                           IMessageQueuePtr queue
                                           )
      {
        FakeReceiverPtr pThis(make_shared<FakeReceiver>(make_private{}, queue));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver => IRTPReceiver
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeReceiver::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr resultEl = Element::create("ortc::test::mediastreamtrack::FakeReceiver");

        UseServicesHelper::debugAppend(resultEl, "id", getID());

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver => IFakeReceiverAsyncDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      bool FakeReceiver::isShutdown()
      {
        return false;
      }

      //-----------------------------------------------------------------------
      Log::Params FakeReceiver::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::mediastreamtrack::FakeReceiver");
        UseServicesHelper::debugAppend(objectEl, "id", RTPReceiver::getID());
        return Log::Params(message, objectEl);
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel
      #pragma mark

      //-----------------------------------------------------------------------
      FakeReceiverChannel::FakeReceiverChannel(IMessageQueuePtr queue) :
        RTPReceiverChannel(Noop(true), queue)
      {
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannel::~FakeReceiverChannel()
      {
        mThisWeak.reset();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel => IRTPReceiverChannelForMediaStreamTrack
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeReceiverChannel::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::mediastreamtrack::FakeReceiverChannel");

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());

        auto receiver = mReceiver.lock();
        UseServicesHelper::debugAppend(result, "receiver", receiver ? receiver->getID() : 0);

        return result;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel => IFakeReceiverChannelAsyncDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel => (friend MediaStreamTrackTester)
      #pragma mark

      //-----------------------------------------------------------------------
      FakeReceiverChannelPtr FakeReceiverChannel::create(
                                                         IMessageQueuePtr queue
                                                         )
      {
        FakeReceiverChannelPtr pThis(make_shared<FakeReceiverChannel>(queue));
        pThis->mThisWeak = pThis;
        return pThis;
      }

      //-----------------------------------------------------------------------
      void FakeReceiverChannel::stop()
      {
        AutoRecursiveLock lock(*this);

        mReceiver.reset();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params FakeReceiverChannel::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::mediastreamtrack::FakeReceiverChannel");
        UseServicesHelper::debugAppend(objectEl, "id", RTPReceiverChannel::getID());
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
      #pragma mark FakeSender => IRTPSenderForMediaStreamTrack
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeSender::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::mediastreamtrack::FakeSender");

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());

        return result;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSender => (friend MediaStreamTrackTester)
      #pragma mark

      //-----------------------------------------------------------------------
      PromisePtr FakeSender::setTrack(IMediaStreamTrackPtr track)
      {
        return PromisePtr();
      }

      //-----------------------------------------------------------------------
      void FakeSender::send(const Parameters &parameters)
      {
        AutoRecursiveLock lock(*this);
      }

      //-----------------------------------------------------------------------
      void FakeSender::stop()
      {
        AutoRecursiveLock lock(*this);
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
        ElementPtr objectEl = Element::create("ortc::test::mediastreamtrack::FakeSender");
        UseServicesHelper::debugAppend(objectEl, "id", FakeSender::getID());
        return Log::Params(message, objectEl);
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester::Expectations
      #pragma mark

      //-----------------------------------------------------------------------
      bool MediaStreamTrackTester::Expectations::operator==(const Expectations &op2) const
      {
        return  (mError == op2.mError) &&
          (mKind == op2.mKind);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester::OverrideReceiverFactory
      #pragma mark

      //-----------------------------------------------------------------------
      MediaStreamTrackTester::OverrideReceiverFactoryPtr MediaStreamTrackTester::OverrideReceiverFactory::create(MediaStreamTrackTesterPtr tester)
      {
        OverrideReceiverFactoryPtr pThis(make_shared<OverrideReceiverFactory>());
        pThis->mTester = tester;
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPReceiverPtr MediaStreamTrackTester::OverrideReceiverFactory::create(
                                                                             IRTPReceiverDelegatePtr delegate,
                                                                             IRTPTransportPtr transport,
                                                                             IRTCPTransportPtr rtcpTransport
                                                                             )
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)

        return tester->create(delegate, transport, rtcpTransport);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester::OverrideReceiverChannelFactory
      #pragma mark

      //-----------------------------------------------------------------------
      MediaStreamTrackTester::OverrideReceiverChannelFactoryPtr MediaStreamTrackTester::OverrideReceiverChannelFactory::create(MediaStreamTrackTesterPtr tester)
      {
        OverrideReceiverChannelFactoryPtr pThis(make_shared<OverrideReceiverChannelFactory>());
        pThis->mTester = tester;
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPReceiverChannelPtr MediaStreamTrackTester::OverrideReceiverChannelFactory::create(
                                                                                           RTPReceiverPtr receiver,
                                                                                           const Parameters &params,
                                                                                           const RTCPPacketList &packets
                                                                                           )
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)

        return tester->create(receiver, params, packets);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester
      #pragma mark

      //-----------------------------------------------------------------------
      MediaStreamTrackTesterPtr MediaStreamTrackTester::create(
                                                               IMessageQueuePtr queue,
                                                               bool overrideFactories
                                                               )
      {
        MediaStreamTrackTesterPtr pThis(new MediaStreamTrackTester(queue, overrideFactories));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      MediaStreamTrackTester::MediaStreamTrackTester(
                                                     IMessageQueuePtr queue,
                                                     bool overrideFactories
                                                     ) :
        SharedRecursiveLock(SharedRecursiveLock::create()),
        MessageQueueAssociator(queue),
        mOverrideFactories(overrideFactories)
      {
        ZS_LOG_BASIC(log("mediastreamtrack tester"))
      }

      //-----------------------------------------------------------------------
      MediaStreamTrackTester::~MediaStreamTrackTester()
      {
        ZS_LOG_BASIC(log("mediastreamtrack tester"))

        if (mOverrideFactories) {
          RTPReceiverFactory::override(RTPReceiverFactoryPtr());
          RTPReceiverChannelFactory::override(RTPReceiverChannelFactoryPtr());
        }
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::init()
      {
        AutoRecursiveLock lock(*this);

        if (mOverrideFactories) {
          RTPReceiverFactory::override(OverrideReceiverFactory::create(mThisWeak.lock()));
          RTPReceiverChannelFactory::override(OverrideReceiverChannelFactory::create(mThisWeak.lock()));
        }
      }

      //-----------------------------------------------------------------------
      bool MediaStreamTrackTester::matches(const Expectations &op2)
      {
        AutoRecursiveLock lock(*this);
        return mExpectations == op2;
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::close()
      {
        AutoRecursiveLock lock(*this);
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::closeByReset()
      {
        AutoRecursiveLock lock(*this);
      }

      //-----------------------------------------------------------------------
      MediaStreamTrackTester::Expectations MediaStreamTrackTester::getExpectations() const
      {
        return mExpectations;
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::createReceiverChannel(
                                                         const char *receiverID,
                                                         const char *parametersID
                                                         )
      {
      }
\
      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::stop(const char *senderOrReceiverID)
      {
        AutoRecursiveLock lock(*this);
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::attach(
                                          const char *receiverChannelID,
                                          FakeReceiverChannelPtr receiverChannel
                                          )
      {
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelPtr MediaStreamTrackTester::detachReceiverChannel(const char *receiverChannelID)
      {
        String receiverChannelIDStr(receiverChannelID);

        AutoRecursiveLock lock(*this);

        return FakeReceiverChannelPtr();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester::IMediaStreamTrackDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::onMediaStreamTrackMute(
                                                          IMediaStreamTrackPtr track,
                                                          bool isMuted
                                                          )
      {

      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::onMediaStreamTrackEnded(IMediaStreamTrackPtr track)
      {

      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::onMediaStreamTrackOverConstrained(IMediaStreamTrackPtr track)
      {

      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester => (friend fake receiver)
      #pragma mark

      RTPReceiverPtr MediaStreamTrackTester::create(
                                                    IRTPReceiverDelegatePtr delegate,
                                                    IRTPTransportPtr transport,
                                                    IRTCPTransportPtr rtcpTransport
                                                    )
      {
        return RTPReceiverPtr();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester => (friend fake receiver channel)
      #pragma mark

      RTPReceiverChannelPtr MediaStreamTrackTester::create(
                                                           RTPReceiverPtr receiver,
                                                           const Parameters &params,
                                                           const RTCPPacketList &packets
                                                           )
      {
        return RTPReceiverChannelPtr();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params MediaStreamTrackTester::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::mediastreamtrack::MediaStreamTrackTester");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      FakeReceiverPtr MediaStreamTrackTester::getReceiver(const char *receiverID)
      {
        AutoRecursiveLock lock(*this);
        return FakeReceiverPtr();
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelPtr MediaStreamTrackTester::getReceiverChannel(const char *receiverID)
      {
        AutoRecursiveLock lock(*this);
        return FakeReceiverChannelPtr();
      }
    }
  }
}

ZS_DECLARE_USING_PTR(ortc::test::mediastreamtrack, FakeReceiver)
ZS_DECLARE_USING_PTR(ortc::test::mediastreamtrack, MediaStreamTrackTester)
ZS_DECLARE_USING_PTR(ortc, IRTPReceiver)

ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::Parameters, Parameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::CodecParameters, CodecParameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::EncodingParameters, EncodingParameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IMediaStreamTrackTypes, IMediaStreamTrackTypes)

using zsLib::Optional;
using zsLib::WORD;
using zsLib::BYTE;
using zsLib::Milliseconds;

#define TEST_BASIC_ROUTING 0

static void bogusSleep()
{
  for (int loop = 0; loop < 100; ++loop)
  {
    TESTING_SLEEP(100)
  }
}

void doTestMediaStreamTrack()
{
  if (!ORTC_TEST_DO_RTP_RECEIVER_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  ortc::ISettings::applyDefaults();

  zsLib::MessageQueueThreadPtr thread(zsLib::MessageQueueThread::createBasic());

  MediaStreamTrackTesterPtr testObject1;
  MediaStreamTrackTesterPtr testObject2;

  TESTING_STDOUT() << "WAITING:      Waiting for MediaStreamTrack testing to complete (max wait is 180 seconds).\n";

  {
    ULONG testNumber = 0;
    ULONG maxSteps = 80;

    do
    {
      TESTING_STDOUT() << "TESTING       ---------->>>>>>>>>> " << testNumber << " <<<<<<<<<<----------\n";

      bool quit = false;
      ULONG expecting = 0;

      MediaStreamTrackTester::Expectations expectations1;
      MediaStreamTrackTester::Expectations expectations2;

      expectations2 = expectations1;

      switch (testNumber) {
      case TEST_BASIC_ROUTING: {
        {
          testObject1 = MediaStreamTrackTester::create(thread, true);
          testObject2 = MediaStreamTrackTester::create(thread, false);

          TESTING_CHECK(testObject1)
          TESTING_CHECK(testObject2)

          expectations1.mKind = IMediaStreamTrackTypes::Kind_Audio;
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
            //  bogusSleep();
            break;
          }
          case 3: {
            break;
          }
          case 4: {
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
          found += (testObject1 ? (testObject1->matches(expectations1) ? 1 : 0) : 0);
          found += (testObject2 ? (testObject2->matches(expectations2) ? 1 : 0) : 0);
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
          if (testObject1) { TESTING_CHECK(testObject1->matches(expectations1)) }
          if (testObject2) { TESTING_CHECK(testObject2->matches(expectations2)) }
          break;
        }
      }

      testObject1.reset();
      testObject2.reset();

      ++testNumber;
    } while (true);
  }

  TESTING_STDOUT() << "WAITING:      All MediaStreamTrack tests have finished. Waiting for 'bogus' events to process (1 second wait).\n";
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

