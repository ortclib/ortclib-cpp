/*
 
 Copyright (c) 2013, SMB Phone Inc.
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
//#include <zsLib/Exception.h>
//#include <zsLib/Socket.h>
//#include <zsLib/Timer.h>
#include <ortc/IICEGatherer.h>
#include <ortc/ISettings.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/XML.h>

#include "config.h"
#include "testing.h"

//#include <list>
//#include <iostream>
//#include <algorithm>
//#include <fstream>
//#include <cstdio>
//#include <cstring>

namespace ortc { namespace test { ZS_DECLARE_SUBSYSTEM(ortc_test) } }

using zsLib::ULONG;
using zsLib::IMessageQueue;
using zsLib::Log;
using zsLib::AutoPUID;
using zsLib::AutoRecursiveLock;
using namespace zsLib::XML;

ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)

//using zsLib::BYTE;
//using zsLib::WORD;
//using zsLib::Socket;
//using zsLib::SocketPtr;
//using zsLib::IPAddress;
//using zsLib::String;
//using openpeer::services::IDNS;
//using openpeer::services::IDNSQuery;
//using openpeer::services::ITURNSocket;
//using openpeer::services::ITURNSocketPtr;
//using openpeer::services::ITURNSocketDelegate;
//using openpeer::services::IICESocket;
//using openpeer::services::IICESocketPtr;
//using openpeer::services::IICESocketSessionPtr;

namespace ortc
{
  namespace test
  {
    ZS_DECLARE_CLASS_PTR(ICEGathererTester)

    class ICEGathererTester : public SharedRecursiveLock,
                              public zsLib::MessageQueueAssociator,
                              public IICEGathererDelegate
    {
    public:
      //-----------------------------------------------------------------------
      static ICEGathererTesterPtr create(IMessageQueuePtr queue)
      {
        ICEGathererTesterPtr pThis(new ICEGathererTester(queue));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      ICEGathererTester(IMessageQueuePtr queue) :
        SharedRecursiveLock(SharedRecursiveLock::create()),
        MessageQueueAssociator(queue)
      {
        ZS_LOG_BASIC(log("ice tester"))
      }

      //-----------------------------------------------------------------------
      ~ICEGathererTester()
      {
        ZS_LOG_BASIC(log("ice tester"))
      }

      //-----------------------------------------------------------------------
      void init()
      {
        AutoRecursiveLock lock(*this);
        IICEGathererTypes::Options options;
        mGatherer = IICEGatherer::create(mThisWeak.lock(), options);
      }

      //-----------------------------------------------------------------------
      Log::Params log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::ICEGathererTester");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      virtual void onICEGathererStateChanged(
                                             IICEGathererPtr gatherer,
                                             IICEGatherer::States state
                                             ) override
      {
        ZS_LOG_BASIC(log("gatherer state changed") + IICEGatherer::toDebug(gatherer) + ZS_PARAM("state", IICEGatherer::toString(state)))
      }

      //-----------------------------------------------------------------------
      virtual void onICEGathererLocalCandidate(
                                               IICEGathererPtr gatherer,
                                               CandidatePtr candidate
                                               ) override
      {
        ZS_LOG_BASIC(log("gatherer local candidate") + IICEGatherer::toDebug(gatherer) + (candidate ? candidate->toDebug() : ElementPtr()))
      }

      //-----------------------------------------------------------------------
      virtual void onICEGathererLocalCandidateComplete(
                                                       IICEGathererPtr gatherer,
                                                       CandidateCompletePtr candidate
                                                       ) override
      {
        ZS_LOG_BASIC(log("gatherer candidate complete") + IICEGatherer::toDebug(gatherer) + (candidate ? candidate->toDebug() : ElementPtr()))
      }

      //-----------------------------------------------------------------------
      virtual void onICEGathererLocalCandidateGone(
                                                   IICEGathererPtr gatherer,
                                                   CandidatePtr candidate
                                                   ) override
      {
        ZS_LOG_BASIC(log("gatherer candidate gone") + IICEGatherer::toDebug(gatherer) + (candidate ? candidate->toDebug() : ElementPtr()))
      }

      //-----------------------------------------------------------------------
      virtual void onICEGathererError(
                                      IICEGathererPtr gatherer,
                                      ErrorCode errorCode,
                                      String errorReason
                                      ) override
      {
        ZS_LOG_BASIC(log("gatherer error") + IICEGatherer::toDebug(gatherer) + ZS_PARAM("error", errorCode) + ZS_PARAM("reason", errorReason))
      }

    public:
      AutoPUID mID;
      ICEGathererTesterWeakPtr mThisWeak;

      IICEGathererPtr mGatherer;
    };

  }
}

ZS_DECLARE_USING_PTR(ortc::test, ICEGathererTester)

//using openpeer::services::test::TestICESocketCallback;
//using openpeer::services::test::TestICESocketCallbackPtr;

void doTestICEGatherer()
{
  if (!ORTC_TEST_DO_ICE_GATHERER_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  ortc::ISettings::applyDefaults();

  zsLib::MessageQueueThreadPtr thread(zsLib::MessageQueueThread::createBasic());

  ICEGathererTesterPtr testObject1;
//  TestICESocketCallbackPtr testObject2;
//  TestICESocketCallbackPtr testObject3;
//  TestICESocketCallbackPtr testObject4;

  TESTING_STDOUT() << "WAITING:      Waiting for ICE testing to complete (max wait is 180 seconds).\n";

  // check to see if all DNS routines have resolved
  {
    ULONG step = 0;

    do
    {
      TESTING_STDOUT() << "STEP:         ---------->>>>>>>>>> " << step << " <<<<<<<<<<----------\n";

      bool quit = false;
      ULONG expecting = 0;
      ULONG maxWait = 60;
      switch (step) {
        case 0: {
          testObject1 = ICEGathererTester::create(thread);
//          testObject1 = TestICESocketCallback::create(thread, 5000 + (rand() % (65525 - 5000)), ORTC_TEST_TURN_SERVER_DOMAIN, ORTC_TEST_STUN_SERVER);
//          testObject2 = TestICESocketCallback::create(thread, 5000 + (rand() % (65525 - 5000)), ORTC_TEST_TURN_SERVER_DOMAIN, ORTC_TEST_STUN_SERVER);
//
//          testObject1->setRemote(testObject2);
//          testObject2->setRemote(testObject1);
          break;
        }
        case 1: {
//          testObject1 = TestICESocketCallback::create(thread, 5000 + (rand() % (65525 - 5000)), ORTC_TEST_TURN_SERVER_DOMAIN, ORTC_TEST_STUN_SERVER, true, false, false, true, false);
//          testObject2 = TestICESocketCallback::create(thread, 5000 + (rand() % (65525 - 5000)), ORTC_TEST_TURN_SERVER_DOMAIN, ORTC_TEST_STUN_SERVER, true, false, false, true, false);
//
//          testObject1->setRemote(testObject2);
//          testObject2->setRemote(testObject1);
          break;
        }
        default:  quit = true; break;
      }
      if (quit) break;

      expecting = 0;
      expecting += (testObject1 ? 1 : 0);
//      expecting += (testObject2 ? 1 : 0);
//      expecting += (testObject3 ? 1 : 0);
//      expecting += (testObject4 ? 1 : 0);

      ULONG found = 0;
      ULONG lastFound = 0;
      ULONG totalWait = 0;

      while (found < expecting)
      {
        TESTING_SLEEP(1000)
        ++totalWait;
        if (totalWait >= maxWait)
          break;

        found = 0;

        switch (step) {
          case 0: {
            if (1 == totalWait) {
//              testObject1->createSessionFromRemoteCandidates(IICESocket::ICEControl_Controlling);
//              testObject2->createSessionFromRemoteCandidates(IICESocket::ICEControl_Controlled);
            }

            if (20 == totalWait) {
//              testObject1->shutdown();
//              testObject2->shutdown();
            }
            break;
          }
          case 1: {
            if (10 == totalWait) {
//              testObject1->createSessionFromRemoteCandidates(IICESocket::ICEControl_Controlling);
//              testObject2->createSessionFromRemoteCandidates(IICESocket::ICEControl_Controlling);
            }

            if (39 == totalWait) {
              found = 2;
//              testObject1->shutdown();
//              testObject2->shutdown();
            }

            break;
          }
        }

        if (0 == found) {
//          found += (testObject1 ? (testObject1->isComplete() ? 1 : 0) : 0);
//          found += (testObject2 ? (testObject2->isComplete() ? 1 : 0) : 0);
//          found += (testObject3 ? (testObject3->isComplete() ? 1 : 0) : 0);
//          found += (testObject4 ? (testObject4->isComplete() ? 1 : 0) : 0);
        }

        if (lastFound != found) {
          lastFound = found;
          TESTING_STDOUT() << "FOUND:        [" << found << "].\n";
        }
      }
      TESTING_EQUAL(found, expecting);

      switch (step) {
        case 0: {
//          if (testObject1) testObject1->expectationsOkay();
//          if (testObject2) testObject2->expectationsOkay();

          break;
        }
        case 1: {
//          if (testObject1) testObject1->expectationsOkay();
//          if (testObject2) testObject2->expectationsOkay();
          break;
        }
      }
      testObject1.reset();
//      testObject2.reset();
//      testObject3.reset();
//      testObject4.reset();

      ++step;
    } while (true);
  }

  TESTING_STDOUT() << "WAITING:      All ICE gethers have finished. Waiting for 'bogus' events to process (10 second wait).\n";
  TESTING_SLEEP(50*10000)

  // wait for shutdown
  {
    IMessageQueue::size_type count = 0;
    do
    {
      count = thread->getTotalUnprocessedMessages();
      //    count += mThreadNeverCalled->getTotalUnprocessedMessages();
      if (0 != count)
        std::this_thread::yield();
    } while (count > 0);

    thread->waitForShutdown();
  }
  TESTING_UNINSTALL_LOGGER();
  zsLib::proxyDump();
  TESTING_EQUAL(zsLib::proxyGetTotalConstructed(), 0);
}
