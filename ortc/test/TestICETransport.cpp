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
#include <ortc/IICETransport.h>

#include <ortc/services/IHelper.h>

#include <zsLib/ISettings.h>
#include <zsLib/IMessageQueueThread.h>
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
    namespace transport
    {
      ZS_DECLARE_CLASS_PTR(ICEGathererTester)
      ZS_DECLARE_CLASS_PTR(ICETransportTester)

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
          ZS_LOG_BASIC(log("ice gather tester"))
        }

        //---------------------------------------------------------------------
        ~ICEGathererTester()
        {
          ZS_LOG_BASIC(log("ice gather tester"))
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
        IICEGathererPtr getGatherer() const
        {
          AutoRecursiveLock lock(*this);
          return mGatherer;
        }

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

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransportTester
      #pragma mark

      class ICETransportTester : public SharedRecursiveLock,
                                 public zsLib::MessageQueueAssociator,
                                 public IICETransportDelegate,
                                 public IICEGathererDelegate
      {
      public:
        typedef String Hash;
        typedef std::map<Hash, CandidatePairPtr> CandidateMap;

        struct Expectations {
          ULONG mStateNew {0};
          ULONG mStateChecking {0};
          ULONG mStateConnected {0};
          ULONG mStateCompleted {0};
          ULONG mStateDisconnected {0};
          ULONG mStateFailed {0};
          ULONG mStateClosed {0};

          //-------------------------------------------------------------------
          bool operator==(const Expectations &op2) const
          {
            return (mStateNew == op2.mStateNew) &&
                   (mStateChecking == op2.mStateChecking) &&
                   (mStateConnected == op2.mStateConnected) &&
                   (mStateCompleted == op2.mStateCompleted) &&
                   (mStateDisconnected == op2.mStateDisconnected) &&
                   (mStateFailed == op2.mStateFailed) &&
                   (mStateClosed == op2.mStateClosed);
          }
        };

      public:

        //-----------------------------------------------------------------------
        static ICETransportTesterPtr create(IMessageQueuePtr queue)
        {
          ICETransportTesterPtr pThis(new ICETransportTester(queue));
          pThis->mThisWeak = pThis;
          pThis->init();
          return pThis;
        }

        //-----------------------------------------------------------------------
        static ICETransportTesterPtr create(
                                            IMessageQueuePtr queue,
                                            ICEGathererTesterPtr gathererTester
                                            )
        {
          ICETransportTesterPtr pThis(new ICETransportTester(queue));
          pThis->mThisWeak = pThis;
          pThis->mGathererTester = gathererTester;
          pThis->init();
          return pThis;
        }

        //-----------------------------------------------------------------------
        ICETransportTester(IMessageQueuePtr queue) :
          SharedRecursiveLock(SharedRecursiveLock::create()),
          MessageQueueAssociator(queue)
        {
          ZS_LOG_BASIC(log("ice tester"))
        }

        //-----------------------------------------------------------------------
        ~ICETransportTester()
        {
          ZS_LOG_BASIC(log("ice tester"))
        }

        //-----------------------------------------------------------------------
        void init()
        {
          AutoRecursiveLock lock(*this);
          mTransport = IICETransport::create(mThisWeak.lock(), mGathererTester ? mGathererTester->getGatherer() : IICEGathererPtr());
        }

        //-----------------------------------------------------------------------
        bool matches(const Expectations &op2)
        {
          AutoRecursiveLock lock(*this);
          return mExpectations == op2;
        }

        //-----------------------------------------------------------------------
        void close()
        {
          AutoRecursiveLock lock(*this);
          mTransport->stop();
          mStopCalled = true;

          mRemoteGathererSubscription->cancel();
          mRemoteGathererSubscription.reset();
        }

        //-----------------------------------------------------------------------
        void closeByReset()
        {
          AutoRecursiveLock lock(*this);
          mTransport.reset();

          mRemoteGathererSubscription.reset();
        }

        //-----------------------------------------------------------------------
        Log::Params log(const char *message) const
        {
          ElementPtr objectEl = Element::create("ortc::test::ICETransportTester");
          UseServicesHelper::debugAppend(objectEl, "id", mID);
          return Log::Params(message, objectEl);
        }

        //-----------------------------------------------------------------------
        Expectations getExpectations() const {return mExpectations;}

        //-----------------------------------------------------------------------
        void setRemote(ICEGathererTesterPtr remoteGathererTester)
        {
          AutoRecursiveLock lock(*this);
          mRemoteGathererTester = remoteGathererTester;

          TESTING_CHECK(mGathererTester)
          TESTING_CHECK(remoteGathererTester)

          auto localGatherer = mGathererTester->getGatherer();
          auto remoteGatherer = remoteGathererTester->getGatherer();

          TESTING_CHECK(localGatherer)
          TESTING_CHECK(remoteGatherer)

          mRemoteGathererSubscription = remoteGatherer->subscribe(mThisWeak.lock());

          TESTING_CHECK(mRemoteGathererSubscription)
        }

        //-----------------------------------------------------------------------
        void start(const IICETransport::Options &options)
        {
          AutoRecursiveLock lock(*this);

          TESTING_CHECK(mGathererTester)
          TESTING_CHECK(mRemoteGathererTester)

          auto localGatherer = mGathererTester->getGatherer();
          auto remoteGatherer = mRemoteGathererTester->getGatherer();

          TESTING_CHECK(localGatherer)
          TESTING_CHECK(remoteGatherer)

          if ((mTransport) &&
              (localGatherer) &&
              (remoteGatherer)) {
            IICEGathererTypes::ParametersPtr params = remoteGatherer->getLocalParameters();
            TESTING_CHECK(params)
            mTransport->start(localGatherer, *params, options);
          }
        }

      protected:

        //-----------------------------------------------------------------------
        virtual void onICETransportStateChange(
                                               IICETransportPtr transport,
                                               IICETransport::States state
                                               ) override
        {
          ZS_LOG_BASIC(log("transport state changed") + ZS_PARAM("state", IICETransport::toString(state)) + IICETransport::toDebug(transport))

          AutoRecursiveLock lock(*this);

          switch (state) {
            case IICETransport::State_New:          ++mExpectations.mStateNew; break;
            case IICETransport::State_Checking:     ++mExpectations.mStateChecking;  break;
            case IICETransport::State_Connected:    ++mExpectations.mStateConnected; break;
            case IICETransport::State_Completed:    ++mExpectations.mStateCompleted; break;
            case IICETransport::State_Disconnected: ++mExpectations.mStateDisconnected; break;
            case IICETransport::State_Failed:       ++mExpectations.mStateFailed; break;
            case IICETransport::State_Closed:       ++mExpectations.mStateClosed; break;
          }
        }

        //-----------------------------------------------------------------------
        virtual void onICETransportCandidatePairAvailable(
                                                          IICETransportPtr transport,
                                                          CandidatePairPtr candidatePair
                                                          ) override
        {
          ZS_LOG_BASIC(log("transport pair available") + IICETransport::toDebug(transport) + (candidatePair ? candidatePair->toDebug() : ElementPtr()))
        }

        //-----------------------------------------------------------------------
        virtual void onICETransportCandidatePairGone(
                                                     IICETransportPtr transport,
                                                     CandidatePairPtr candidatePair
                                                     ) override
        {
          ZS_LOG_BASIC(log("transport pair gone") + IICETransport::toDebug(transport) + (candidatePair ? candidatePair->toDebug() : ElementPtr()))
        }

        //-----------------------------------------------------------------------
        virtual void onICETransportCandidatePairChanged(
                                                        IICETransportPtr transport,
                                                        CandidatePairPtr candidatePair
                                                        ) override
        {
          ZS_LOG_BASIC(log("transport pair changed") + IICETransport::toDebug(transport) + (candidatePair ? candidatePair->toDebug() : ElementPtr()))
        }

      protected:
        //---------------------------------------------------------------------
        virtual void onICEGathererStateChange(
                                              IICEGathererPtr gatherer,
                                              IICEGatherer::States state
                                              ) override
        {
          ZS_LOG_BASIC(log("remote gatherer state changed") + IICEGatherer::toDebug(gatherer) + ZS_PARAM("state", IICEGatherer::toString(state)))
        }

        //---------------------------------------------------------------------
        virtual void onICEGathererLocalCandidate(
                                                 IICEGathererPtr gatherer,
                                                 CandidatePtr candidate
                                                 ) override
        {
          ZS_LOG_BASIC(log("remote gatherer notified candidate") + IICEGatherer::toDebug(gatherer) + (candidate ? candidate->toDebug() : ElementPtr()))

          AutoRecursiveLock lock(*this);

          if (!mTransport) return;
          if (mStopCalled) return;

          mTransport->addRemoteCandidate(*candidate);
        }

        //---------------------------------------------------------------------
        virtual void onICEGathererLocalCandidateComplete(
                                                         IICEGathererPtr gatherer,
                                                         CandidateCompletePtr candidate
                                                         ) override
        {
          ZS_LOG_BASIC(log("remote gatherer notified candidate complete") + IICEGatherer::toDebug(gatherer) + (candidate ? candidate->toDebug() : ElementPtr()))

          TESTING_CHECK(candidate)

          AutoRecursiveLock lock(*this);

          if (!mTransport) return;
          if (mStopCalled) return;

          mTransport->addRemoteCandidate(*candidate);
        }

        //---------------------------------------------------------------------
        virtual void onICEGathererLocalCandidateGone(
                                                     IICEGathererPtr gatherer,
                                                     CandidatePtr candidate
                                                     ) override
        {
          ZS_LOG_BASIC(log("remote gatherer notified candidate gone") + IICEGatherer::toDebug(gatherer) + (candidate ? candidate->toDebug() : ElementPtr()))

          TESTING_CHECK(candidate)

          AutoRecursiveLock lock(*this);

          if (!mTransport) return;
          if (mStopCalled) return;

          mTransport->removeRemoteCandidate(*candidate);
        }

        //---------------------------------------------------------------------
        virtual void onICEGathererError(
                                        IICEGathererPtr gatherer,
                                        ErrorEventPtr errorEvent
                                        ) override
        {
          ZS_LOG_ERROR(Basic, log("remote gatherer error") + IICEGatherer::toDebug(gatherer) + errorEvent->toDebug())
        }

      public:
        AutoPUID mID;
        ICETransportTesterWeakPtr mThisWeak;

        ICEGathererTesterPtr mGathererTester;
        IICETransportPtr mTransport;
        bool mStopCalled {false};
        
        Expectations mExpectations;
        
        CandidateMap mCandidates;

        ICEGathererTesterPtr mRemoteGathererTester;

        IICEGathererSubscriptionPtr mRemoteGathererSubscription;
      };
    }
  }
}

ZS_DECLARE_USING_PTR(ortc::test::transport, ICEGathererTester)
ZS_DECLARE_USING_PTR(ortc::test::transport, ICETransportTester)


void doTestICETransport()
{
  if (!ORTC_TEST_DO_ICE_TRANSPORT_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  UseSettings::applyDefaults();

  auto thread(zsLib::IMessageQueueThread::createBasic());

  size_t totalHostIPs = UseSettings::getUInt("tester/total-host-ips");

  ICEGathererTesterPtr testGathererObject1;
  ICETransportTesterPtr testTransportObject1;

  ICEGathererTesterPtr testGathererObject2;
  ICETransportTesterPtr testTransportObject2;

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

      ICEGathererTester::Expectations expectationsGatherer1;
      ICEGathererTester::Expectations expectationsGatherer2;

      expectationsGatherer1.mStateNew = 0;
      expectationsGatherer1.mStateGathering = 1;
      expectationsGatherer1.mStateComplete = 1;
      expectationsGatherer1.mStateClosed = 1;
      expectationsGatherer1.mCandidatesUDPHost = totalHostIPs;
      expectationsGatherer1.mCandidatesTCPHostActive = totalHostIPs;
      expectationsGatherer1.mCandidatesTCPHostPassive = totalHostIPs;
      expectationsGatherer1.mCandidateGone = expectationsGatherer1.mCandidatesUDPHost + expectationsGatherer1.mCandidatesTCPHostActive + expectationsGatherer1.mCandidatesTCPHostPassive;
      expectationsGatherer1.mCandidateComplete = 1;

      expectationsGatherer2 = expectationsGatherer1;

      ICETransportTester::Expectations expectationsTransport1;
      ICETransportTester::Expectations expectationsTransport2;

      expectationsTransport1.mStateChecking = 1;
      expectationsTransport1.mStateConnected = 1;
      expectationsTransport1.mStateCompleted = 1;
      expectationsTransport1.mStateClosed = 1;

      expectationsTransport2 = expectationsTransport1;

      switch (step) {
/*        case 0: {
          testTransportObject1 = ICETransportTester::create(thread);
          break;
        }
        case 1: {
          testGathererObject1 = ICEGathererTester::create(thread);
          testTransportObject1 = ICETransportTester::create(thread, testGathererObject1);
          break;
        }*/
        case 0: {
          {
            expectationsGatherer1.mCandidatesSrflex = ORTC_TEST_REFLEXIVE_UDP_IPS;
            expectationsGatherer1.mCandidatesRelay = ORTC_TEST_RELAY_UDP_IPS;
            expectationsGatherer1.mCandidateGone += expectationsGatherer1.mCandidatesSrflex + expectationsGatherer1.mCandidatesRelay;

            String url = String("turn:") + ORTC_SERVICE_TEST_TURN_SERVER_DOMAIN_VIA_A_RECORD_1;

            ortc::IICEGatherer::Server server;
            server.mURLs.push_back(url);
            server.mUserName = ORTC_SERVICE_TEST_TURN_USERNAME;
            server.mCredential = ORTC_SERVICE_TEST_TURN_PASSWORD;

            ortc::IICEGatherer::Options options;
            options.mICEServers.push_back(server);

            testGathererObject1 = ICEGathererTester::create(thread, options);
            testTransportObject1 = ICETransportTester::create(thread, testGathererObject1);
          }
          {
            expectationsGatherer2.mCandidatesSrflex = ORTC_TEST_REFLEXIVE_UDP_IPS;
            expectationsGatherer2.mCandidateGone += expectationsGatherer2.mCandidatesSrflex + expectationsGatherer2.mCandidatesRelay;

            String url = String("stun:") + ORTC_TEST_STUN_SERVER;

            ortc::IICEGatherer::Server server;
            server.mURLs.push_back(url);
            ortc::IICEGatherer::Options options;
            options.mICEServers.push_back(server);

            testGathererObject2 = ICEGathererTester::create(thread, options);
            testTransportObject2 = ICETransportTester::create(thread, testGathererObject2);
          }

          testTransportObject1->setRemote(testGathererObject2);
          testTransportObject2->setRemote(testGathererObject1);
          break;
        }
/*        case 2: {
          UseSettings::setBool("ortc/gatherer/gather-passive-tcp-candidates", true);

          expectations1.mCandidateGone = 0;
          expectations1.mStateClosed = 0;

          testGathererObject1 = ICEGathererTester::create(thread);
          break;
        }
        case 3: {
          expectations1.mCandidateGone = 0;
          expectations1.mStateClosed = 0;

          testGathererObject1 = ICEGathererTester::create(thread);
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

          testGathererObject1 = ICEGathererTester::create(thread, options);
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

          testGathererObject1 = ICEGathererTester::create(thread, options);
          break;
        }
*/
        default:  quit = true; break;
      }
      if (quit) break;

      expecting = 0;
      expecting += (testGathererObject1 ? 1 : 0);
      expecting += (testGathererObject2 ? 1 : 0);
      expecting += (testTransportObject1 ? 1 : 0);
      expecting += (testTransportObject2 ? 1 : 0);

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
            if (5 == totalWait) {
              ortc::IICETransportTypes::Options options1;
              ortc::IICETransportTypes::Options options2;

              options1.mRole = ortc::IICETypes::Role_Controlling;
              options2.mRole = ortc::IICETypes::Role_Controlled;
              
              testTransportObject1->start(options1);
              testTransportObject2->start(options2);
            }
            if (70 == totalWait) {
              if (testTransportObject1) testTransportObject1->close();
              if (testTransportObject2) testTransportObject2->close();
              if (testGathererObject1) testGathererObject1->close();
              if (testGathererObject2) testGathererObject2->close();
            }
            break;
          }
          case 1: {
            if (7 == totalWait) {
              if (testGathererObject1) testGathererObject1->close();
              if (testGathererObject2) testGathererObject2->close();
              if (testTransportObject1) testTransportObject1->close();
              if (testTransportObject2) testTransportObject2->close();
            }

            break;
          }
          case 2: {
            if (7 == totalWait) {
              TESTING_CHECK(testGathererObject1->matches(expectationsGatherer1))
              testGathererObject1.reset();
            }

            break;
          }
          case 3: {
            if (7 == totalWait) {
              testGathererObject1->closeByReset();
            }

            break;
          }
          case 4: {
            if (65 == totalWait) {
              testGathererObject1->close();
            }
            break;
          }
          case 5: {
            if (65 == totalWait) {
              testGathererObject1->close();
            }
            break;
          }
        }

        if (0 == found) {
          found += (testGathererObject1 ? (testGathererObject1->matches(expectationsGatherer1) ? 1 : 0) : 0);
          found += (testGathererObject2 ? (testGathererObject2->matches(expectationsGatherer2) ? 1 : 0) : 0);
          found += (testTransportObject1 ? (testTransportObject1->matches(expectationsTransport1) ? 1 : 0) : 0);
          found += (testTransportObject2 ? (testTransportObject2->matches(expectationsTransport2) ? 1 : 0) : 0);
        }

        if (lastFound != found) {
          lastFound = found;
          TESTING_STDOUT() << "FOUND:        [" << found << "].\n";
        }
      }

      TESTING_EQUAL(found, expecting)

      TESTING_SLEEP(2000)

      //switch (step) {
      //  default:
        {
          if (testGathererObject1) {TESTING_CHECK(testGathererObject1->matches(expectationsGatherer1))}
          if (testGathererObject2) {TESTING_CHECK(testGathererObject2->matches(expectationsGatherer2))}
          if (testTransportObject1) {TESTING_CHECK(testTransportObject1->matches(expectationsTransport1))}
          if (testTransportObject2) {TESTING_CHECK(testTransportObject2->matches(expectationsTransport2))}
      //    break;
        }
      //}
      testGathererObject1.reset();
      testGathererObject2.reset();
      testTransportObject1.reset();
      testTransportObject2.reset();

      ++step;
    } while (true);
  }

  TESTING_STDOUT() << "WAITING:      All ICE transports have finished. Waiting for 'bogus' events to process (10 second wait).\n";
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
