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


#include <ortc/IICEGatherer.h>

#include <ortc/services/IHelper.h>

#include <zsLib/IMessageQueueThread.h>
#include <zsLib/ISettings.h>
#include <zsLib/XML.h>

#include "config.h"
#include "testing.h"

namespace ortc { namespace test { ZS_DECLARE_SUBSYSTEM(ortc_test) } }

using zsLib::String;
using zsLib::ULONG;
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
    namespace gatherer
    {
      ZS_DECLARE_CLASS_PTR(ICEGathererTester)

      class ICEGathererTester : public SharedRecursiveLock,
                                public zsLib::MessageQueueAssociator,
                                public IICEGathererDelegate
      {
      public:
        typedef String Hash;
        typedef std::map<Hash, CandidatePtr> CandidateMap;

        struct Expectations {
          ULONG mStateNew {0};
          ULONG mStateGathering {0};
          ULONG mStateComplete {0};
          ULONG mStateClosed {0};

          ULONG mCandidatesUDPHost {0};
          ULONG mCandidatesTCPHostPassive {0};
          ULONG mCandidatesTCPHostActive {0};
          ULONG mCandidatesTCPHostSO {0};

          ULONG mCandidatesPrflx {0};
          ULONG mCandidatesSrflex {0};
          ULONG mCandidatesRelay {0};

          ULONG mCandidateComplete {0};
          ULONG mCandidateGone {0};

          ULONG mError {0};

          //-------------------------------------------------------------------
          bool operator==(const Expectations &op2) const
          {
            return (mStateNew == op2.mStateNew) &&
                   (mStateGathering == op2.mStateGathering) &&
                   (mStateComplete == op2.mStateComplete) &&
                   (mStateClosed == op2.mStateClosed) &&

                   (mCandidatesUDPHost == op2.mCandidatesUDPHost) &&
                   (mCandidatesTCPHostPassive == op2.mCandidatesTCPHostPassive) &&
                   (mCandidatesTCPHostActive == op2.mCandidatesTCPHostActive) &&

                   (mCandidatesPrflx == op2.mCandidatesPrflx) &&
                   (mCandidatesSrflex == op2.mCandidatesSrflex) &&
                   (mCandidatesRelay == op2.mCandidatesRelay) &&

                   (mCandidateComplete == op2.mCandidateComplete) &&
                   (mCandidateGone == op2.mCandidateGone);
          }
        };

      public:
        //---------------------------------------------------------------------
        static ICEGathererTesterPtr create(IMessageQueuePtr queue)
        {
          ICEGathererTesterPtr pThis(new ICEGathererTester(queue));
          pThis->mThisWeak = pThis;
          pThis->init();
          return pThis;
        }

        //---------------------------------------------------------------------
        static ICEGathererTesterPtr create(
                                           IMessageQueuePtr queue,
                                           const IICEGathererTypes::Options &options
                                           )
        {
          ICEGathererTesterPtr pThis(new ICEGathererTester(queue));
          pThis->mThisWeak = pThis;
          pThis->init(options);
          return pThis;
        }

        //---------------------------------------------------------------------
        ICEGathererTester(IMessageQueuePtr queue) :
          SharedRecursiveLock(SharedRecursiveLock::create()),
          MessageQueueAssociator(queue)
        {
          ZS_LOG_BASIC(log("ice tester"))
        }

        //---------------------------------------------------------------------
        ~ICEGathererTester()
        {
          ZS_LOG_BASIC(log("ice tester"))
        }

        //---------------------------------------------------------------------
        void init()
        {
          AutoRecursiveLock lock(*this);
          IICEGathererTypes::Options options;
          mGatherer = IICEGatherer::create(mThisWeak.lock(), options);
        }

        //---------------------------------------------------------------------
        void init(const IICEGathererTypes::Options &options)
        {
          AutoRecursiveLock lock(*this);
          mGatherer = IICEGatherer::create(mThisWeak.lock(), options);
        }

        //---------------------------------------------------------------------
        bool matches(const Expectations &op2)
        {
          AutoRecursiveLock lock(*this);
          return mExpectations == op2;
        }

        //---------------------------------------------------------------------
        void close()
        {
          AutoRecursiveLock lock(*this);
          mGatherer->close();
        }

        //---------------------------------------------------------------------
        void closeByReset()
        {
          AutoRecursiveLock lock(*this);
          mGatherer.reset();
        }

        //---------------------------------------------------------------------
        Log::Params log(const char *message) const
        {
          ElementPtr objectEl = Element::create("ortc::test::ICEGathererTester");
          UseServicesHelper::debugAppend(objectEl, "id", mID);
          return Log::Params(message, objectEl);
        }

        //---------------------------------------------------------------------
        Expectations getExpectations() const {return mExpectations;}

        //---------------------------------------------------------------------
        virtual void onICEGathererStateChange(
                                              IICEGathererPtr gatherer,
                                              IICEGatherer::States state
                                              ) override
        {
          ZS_LOG_BASIC(log("gatherer state changed") + IICEGatherer::toDebug(gatherer) + ZS_PARAM("state", IICEGatherer::toString(state)))

          AutoRecursiveLock lock(*this);

          switch (state) {
            case IICEGatherer::State_New:       ++mExpectations.mStateNew;        break;
            case IICEGatherer::State_Gathering: ++mExpectations.mStateGathering;  break;
            case IICEGatherer::State_Complete:  ++mExpectations.mStateComplete;   break;
            case IICEGatherer::State_Closed:    ++mExpectations.mStateClosed;     break;
          }
        }

        //---------------------------------------------------------------------
        virtual void onICEGathererLocalCandidate(
                                                 IICEGathererPtr gatherer,
                                                 CandidatePtr candidate
                                                 ) override
        {
          ZS_LOG_BASIC(log("gatherer local candidate") + IICEGatherer::toDebug(gatherer) + (candidate ? candidate->toDebug() : ElementPtr()))

          TESTING_CHECK(candidate)

          AutoRecursiveLock lock(*this);

          auto hash = candidate->hash();

          TESTING_CHECK(mCandidates.end() == mCandidates.find(hash))

          mCandidates[hash] = candidate;

          switch (candidate->mCandidateType) {
            case IICETypes::CandidateType_Host:
            {
              switch (candidate->mProtocol) {
                case IICETypes::Protocol_UDP: ++mExpectations.mCandidatesUDPHost; break;
                case IICETypes::Protocol_TCP: {
                  switch (candidate->mTCPType) {
                    case IICETypes::TCPCandidateType_Active:    ++mExpectations.mCandidatesTCPHostActive; break;
                    case IICETypes::TCPCandidateType_Passive:   ++mExpectations.mCandidatesTCPHostPassive; break;
                    case IICETypes::TCPCandidateType_SO:        ++mExpectations.mCandidatesTCPHostSO; break;
                  }
                  break;
                }
              }
              break;
            }
            case IICETypes::CandidateType_Prflx:  ++mExpectations.mCandidatesPrflx; break;
            case IICETypes::CandidateType_Srflex: ++mExpectations.mCandidatesSrflex; break;
            case IICETypes::CandidateType_Relay:  ++mExpectations.mCandidatesRelay; break;
          }
        }

        //---------------------------------------------------------------------
        virtual void onICEGathererLocalCandidateComplete(
                                                         IICEGathererPtr gatherer,
                                                         CandidateCompletePtr candidate
                                                         ) override
        {
          ZS_LOG_BASIC(log("gatherer candidate complete") + IICEGatherer::toDebug(gatherer) + (candidate ? candidate->toDebug() : ElementPtr()))

          TESTING_CHECK(candidate)

          AutoRecursiveLock lock(*this);

          ++mExpectations.mCandidateComplete;
        }

        //---------------------------------------------------------------------
        virtual void onICEGathererLocalCandidateGone(
                                                     IICEGathererPtr gatherer,
                                                     CandidatePtr candidate
                                                     ) override
        {
          ZS_LOG_BASIC(log("gatherer candidate gone") + IICEGatherer::toDebug(gatherer) + (candidate ? candidate->toDebug() : ElementPtr()))

          TESTING_CHECK(candidate)

          auto hash = candidate->hash();

          AutoRecursiveLock lock(*this);

          auto found = mCandidates.find(hash);
          TESTING_CHECK(found != mCandidates.end())

          mCandidates.erase(found);

          ++mExpectations.mCandidateGone;
        }

        //---------------------------------------------------------------------
        virtual void onICEGathererError(
                                        IICEGathererPtr gatherer,
                                        ErrorEventPtr errorEvent
                                        ) override
        {
          ZS_LOG_ERROR(Basic, log("gatherer error") + IICEGatherer::toDebug(gatherer) + errorEvent->toDebug())

          AutoRecursiveLock lock(*this);

          ++mExpectations.mError;
        }

      public:
        AutoPUID mID;
        ICEGathererTesterWeakPtr mThisWeak;

        IICEGathererPtr mGatherer;

        Expectations mExpectations;

        CandidateMap mCandidates;
      };

    }
  }
}

ZS_DECLARE_USING_PTR(ortc::test::gatherer, ICEGathererTester)


void doTestICEGatherer()
{
  if (!ORTC_TEST_DO_ICE_GATHERER_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  UseSettings::applyDefaults();

  auto thread(zsLib::IMessageQueueThread::createBasic());

  size_t totalHostIPs = UseSettings::getUInt("tester/total-host-ips");

  ICEGathererTesterPtr testObject1;

  TESTING_STDOUT() << "WAITING:      Waiting for ICE testing to complete (max wait is 180 seconds).\n";

  // check to see if all DNS routines have resolved
  {
    ULONG step = 0;
    ULONG maxWait = 80;

    do
    {
      TESTING_STDOUT() << "STEP:         ---------->>>>>>>>>> " << step << " <<<<<<<<<<----------\n";

      bool quit = false;
      ULONG expecting = 0;

      ICEGathererTester::Expectations expectations1;

      expectations1.mStateNew = 0;
      expectations1.mStateGathering = 1;
      expectations1.mStateComplete = 1;
      expectations1.mStateClosed = 1;
      expectations1.mCandidatesUDPHost = totalHostIPs;
      expectations1.mCandidatesTCPHostActive = totalHostIPs;
      expectations1.mCandidatesTCPHostPassive = totalHostIPs;
      expectations1.mCandidateGone = expectations1.mCandidatesUDPHost + expectations1.mCandidatesTCPHostActive + expectations1.mCandidatesTCPHostPassive;
      expectations1.mCandidateComplete = 1;

      switch (step) {
        case 0: {
          testObject1 = ICEGathererTester::create(thread);
          break;
        }
        case 1: {
          UseSettings::setBool("ortc/gatherer/gather-passive-tcp-candidates", false);

          expectations1.mCandidatesTCPHostPassive = 0;
          expectations1.mCandidateGone = expectations1.mCandidatesUDPHost + expectations1.mCandidatesTCPHostActive + expectations1.mCandidatesTCPHostPassive;

          testObject1 = ICEGathererTester::create(thread);
          break;
        }
        case 2: {
          UseSettings::setBool("ortc/gatherer/gather-passive-tcp-candidates", true);

          expectations1.mCandidateGone = 0;
          expectations1.mStateClosed = 0;

          testObject1 = ICEGathererTester::create(thread);
          break;
        }
        case 3: {
          expectations1.mCandidateGone = 0;
          expectations1.mStateClosed = 0;

          testObject1 = ICEGathererTester::create(thread);
          break;
        }
        case 4: {
          expectations1.mCandidatesSrflex = ORTC_TEST_REFLEXIVE_UDP_IPS;
          expectations1.mCandidateGone += expectations1.mCandidatesSrflex + expectations1.mCandidatesRelay;

          String url = String("stun:") + ORTC_TEST_STUN_SERVER;

          ortc::IICEGatherer::Server server;
          server.mURLs.push_back(url);
          ortc::IICEGatherer::Options options;
          options.mICEServers.push_back(server);

          testObject1 = ICEGathererTester::create(thread, options);
          break;
        }
        case 5: {
          expectations1.mCandidatesSrflex = ORTC_TEST_REFLEXIVE_UDP_IPS;
          expectations1.mCandidatesRelay = ORTC_TEST_RELAY_UDP_IPS;
          expectations1.mCandidateGone += expectations1.mCandidatesSrflex + expectations1.mCandidatesRelay;

          String url = String("turn:") + ORTC_SERVICE_TEST_TURN_SERVER_DOMAIN_VIA_A_RECORD_1;

          ortc::IICEGatherer::Server server;
          server.mURLs.push_back(url);
          server.mUserName = ORTC_SERVICE_TEST_TURN_USERNAME;
          server.mCredential = ORTC_SERVICE_TEST_TURN_PASSWORD;

          ortc::IICEGatherer::Options options;
          options.mICEServers.push_back(server);

          testObject1 = ICEGathererTester::create(thread, options);
          break;
        }
        default:  quit = true; break;
      }
      if (quit) break;

      expecting = 0;
      expecting += (testObject1 ? 1 : 0);

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
            if (7 == totalWait) {
              testObject1->close();
            }
            break;
          }
          case 1: {
            if (7 == totalWait) {
              testObject1->close();
            }

            break;
          }
          case 2: {
            if (7 == totalWait) {
              TESTING_CHECK(testObject1->matches(expectations1))
              testObject1.reset();
            }

            break;
          }
          case 3: {
            if (7 == totalWait) {
              testObject1->closeByReset();
            }

            break;
          }
          case 4: {
            if (65 == totalWait) {
              testObject1->close();
            }
            break;
          }
          case 5: {
            if (65 == totalWait) {
              testObject1->close();
            }
            break;
          }
        }

        if (0 == found) {
          found += (testObject1 ? (testObject1->matches(expectations1) ? 1 : 0) : 0);
        }

        if (lastFound != found) {
          lastFound = found;
          TESTING_STDOUT() << "FOUND:        [" << found << "].\n";
        }
      }

      TESTING_EQUAL(found, expecting)
      TESTING_SLEEP(2000)

      switch (step) {
        case 0: {
          TESTING_CHECK(testObject1->matches(expectations1))
          break;
        }
        case 1: {
          TESTING_CHECK(testObject1->matches(expectations1))
          break;
        }
        case 2: {
          break;
        }
        case 3: {
          TESTING_CHECK(testObject1->matches(expectations1))
          break;
        }
        case 4: {
          TESTING_CHECK(testObject1->matches(expectations1))
          break;
        }
        case 5: {
          TESTING_CHECK(testObject1->matches(expectations1))
          break;
        }
      }
      testObject1.reset();

      ++step;
    } while (true);
  }

  TESTING_STDOUT() << "WAITING:      All ICE gatherers have finished. Waiting for 'bogus' events to process (10 second wait).\n";
  TESTING_SLEEP(10000)

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
