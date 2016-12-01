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


#include <ortc/IDTLSTransport.h>

#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>

#include <ortc/services/IHelper.h>

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
using zsLib::IPromiseSettledDelegate;
using namespace zsLib::XML;

ZS_DECLARE_TYPEDEF_PTR(zsLib::ISettings, UseSettings)
ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper)

namespace ortc
{
  namespace test
  {
    namespace dtls
    {
      ZS_DECLARE_INTERACTION_PROXY(IFakeICETransportAsyncDelegate)

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark IFakeICETransportAsyncDelegate
      #pragma mark

      interaction IFakeICETransportAsyncDelegate
      {
        virtual void onPacketFromLinkedFakedTransport(SecureByteBlockPtr buffer) = 0;
      };

    }
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::test::dtls::IFakeICETransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::services::SecureByteBlockPtr, SecureByteBlockPtr)
ZS_DECLARE_PROXY_METHOD_1(onPacketFromLinkedFakedTransport, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

namespace ortc
{
  namespace test
  {
    namespace dtls
    {
      ZS_DECLARE_CLASS_PTR(FakeICETransport)
      ZS_DECLARE_CLASS_PTR(DTLSTester)

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeICETransport
      #pragma mark

      //---------------------------------------------------------------------
      class FakeICETransport : public ortc::internal::ICETransport,
                               public IFakeICETransportAsyncDelegate
      {
      protected:
        struct make_private {};

      public:
        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IICETransportForSecureTransport::UseSecureTransport, UseSecureTransport)

      public:
        //---------------------------------------------------------------------
        FakeICETransport(
                         const make_private &,
                         IMessageQueuePtr queue
                         ) :
          ICETransport(zsLib::Noop(true), queue)
        {
          ZS_LOG_BASIC(log("created"))
        }

      protected:
        //---------------------------------------------------------------------
        void init()
        {
        }

      public:
        //---------------------------------------------------------------------
        ~FakeICETransport()
        {
          mThisWeak.reset();

          ZS_LOG_BASIC(log("destroyed"))
        }

        //---------------------------------------------------------------------
        static FakeICETransportPtr create(IMessageQueuePtr queue)
        {
          FakeICETransportPtr pThis(make_shared<FakeICETransport>(make_private{}, queue));
          pThis->mThisWeak = pThis;
          pThis->init();
          return pThis;
        }

        //---------------------------------------------------------------------
        void linkTransport(FakeICETransportPtr transport)
        {
          AutoRecursiveLock lock(*this);
          mLinkedTransport = transport;

          if (transport) {
            ZS_LOG_BASIC(log("transport linked") + ZS_PARAM("linked transport", transport->getID()))
          } else {
            ZS_LOG_BASIC(log("transport unlinked"))
          }
        }

        //---------------------------------------------------------------------
        void state(IICETransport::States newState)
        {
          AutoRecursiveLock lock(*this);
          setState(newState);
        }

        //---------------------------------------------------------------------
        void role(IICETypes::Roles role)
        {
          AutoRecursiveLock lock(*this);
          ZS_LOG_BASIC(log("setting role") + ZS_PARAM("role", IICETypes::toString(role)))
          mRole = role;
        }

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => IICETransport
        #pragma mark

        //---------------------------------------------------------------------
        virtual ElementPtr toDebug() const override
        {
          AutoRecursiveLock lock(*this);

          ElementPtr resultEl = Element::create("ortc::test::dtls::FakeICETransport");

          UseServicesHelper::debugAppend(resultEl, "id", getID());

          UseServicesHelper::debugAppend(resultEl, "state", IICETransport::toString(mCurrentState));

          UseServicesHelper::debugAppend(resultEl, "secure transport id", mSecureTransportID);
          UseServicesHelper::debugAppend(resultEl, "secure transport", (bool)(mSecureTransport.lock()));

          UseServicesHelper::debugAppend(resultEl, "linked transport", (bool)(mLinkedTransport.lock()));

          UseServicesHelper::debugAppend(resultEl, "subscriptions", mSubscriptions.size());
          UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

          return resultEl;
        }

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => IICETransportForSecureTransport
        #pragma mark

        //---------------------------------------------------------------------
        virtual PUID getID() const override {return ICETransport::getID();}

        //---------------------------------------------------------------------
        virtual void notifyAttached(
                                    PUID secureTransportID,
                                    UseSecureTransportPtr transport
                                    ) override
        {
          AutoRecursiveLock lock(*this);

          mSecureTransportID = secureTransportID;
          mSecureTransport = transport;

          ZS_LOG_BASIC(log("transport attached") + ZS_PARAMIZE(secureTransportID))
        }

        //---------------------------------------------------------------------
        virtual void notifyDetached(PUID secureTransportID) override
        {
          AutoRecursiveLock lock(*this);
          if (mSecureTransportID != secureTransportID) {
            ZS_LOG_WARNING(Detail, log("attempting to detach what was never attached") + ZS_PARAMIZE(secureTransportID) + ZS_PARAMIZE(mSecureTransportID))
            return;
          }

          ZS_LOG_BASIC(log("transport detached") + ZS_PARAMIZE(secureTransportID))

          mSecureTransportID = 0;
          mSecureTransport.reset();
        }

        //---------------------------------------------------------------------
        virtual IICETypes::Components component() const override
        {
          return mComponent;
        }

        //---------------------------------------------------------------------
        virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr originalDelegate) override
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

        //---------------------------------------------------------------------
        virtual IICETransport::States state() const override
        {
          AutoRecursiveLock lock(*this);
          return mCurrentState;
        }

        //---------------------------------------------------------------------
        virtual IICETypes::Roles getRole() const override
        {
          AutoRecursiveLock lock(*this);
          return mRole;
        }

        //---------------------------------------------------------------------
        virtual bool sendPacket(
                                const BYTE *buffer,
                                size_t bufferSizeInBytes
                                ) override
        {
          FakeICETransportPtr transport;

          {
            AutoRecursiveLock lock(*this);
            transport = mLinkedTransport.lock();
            if (!transport) {
              ZS_LOG_WARNING(Detail, log("not linked to another fake transport") + ZS_PARAM("buffer", (PTRNUMBER)(buffer)) + ZS_PARAM("buffer size", bufferSizeInBytes))
              return false;
            }
          }

          ZS_LOG_DEBUG(log("sending packet to linked fake transport") + ZS_PARAM("buffer", (PTRNUMBER)(buffer)) + ZS_PARAM("buffer size", bufferSizeInBytes))

          SecureByteBlockPtr sendBuffer(make_shared<SecureByteBlock>(buffer, bufferSizeInBytes));

          IFakeICETransportAsyncDelegateProxy::create(transport)->onPacketFromLinkedFakedTransport(sendBuffer);
          return true;
        }

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => IFakeICETransportAsyncDelegate
        #pragma mark

        //---------------------------------------------------------------------
        virtual void onPacketFromLinkedFakedTransport(SecureByteBlockPtr buffer) override
        {
          UseSecureTransportPtr transport;

          {
            AutoRecursiveLock lock(*this);
            transport = mSecureTransport.lock();
            if (!transport) {
              ZS_LOG_WARNING(Detail, log("no dtls transport attached (thus cannot forward received packet)") + ZS_PARAM("buffer", (PTRNUMBER)(buffer->BytePtr())) + ZS_PARAM("buffer size", buffer->SizeInBytes()))
              return;
            }
          }

          TESTING_CHECK(buffer)

          ZS_LOG_DEBUG(log("packet received") + ZS_PARAM("buffer", (PTRNUMBER)(buffer->BytePtr())) + ZS_PARAM("buffer size", buffer->SizeInBytes()))

          transport->handleReceivedPacket(mComponent, buffer->BytePtr(), buffer->SizeInBytes());
        }

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => (internal)
        #pragma mark

        //---------------------------------------------------------------------
        void setState(IICETransportTypes::States state)
        {
          if (state == mCurrentState) return;

          ZS_LOG_DETAIL(log("state changed") + ZS_PARAM("new state", IICETransport::toString(state)) + ZS_PARAM("old state", IICETransport::toString(mCurrentState)))

          mCurrentState = state;

          auto pThis = mThisWeak.lock();
          if (pThis) {
            mSubscriptions.delegate()->onICETransportStateChange(pThis, mCurrentState);
          }
        }

        //---------------------------------------------------------------------
        bool isShutdown()
        {
          return IICETransport::State_Closed == mCurrentState;
        }

        //---------------------------------------------------------------------
        Log::Params log(const char *message) const
        {
          ElementPtr objectEl = Element::create("ortc::test::dtls::FakeICETransport");
          UseServicesHelper::debugAppend(objectEl, "id", ICETransport::getID());
          return Log::Params(message, objectEl);
        }

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeICETransport => (data)
        #pragma mark

        FakeICETransportWeakPtr mThisWeak;

        IICETypes::Components mComponent {IICETypes::Component_RTP};
        IICETypes::Roles mRole {IICETypes::Role_Controlling};

        IICETransportTypes::States mCurrentState {IICETransportTypes::State_New};

        PUID mSecureTransportID {0};
        UseSecureTransportWeakPtr mSecureTransport;

        FakeICETransportWeakPtr mLinkedTransport;

        IICETransportDelegateSubscriptions mSubscriptions;
        IICETransportSubscriptionPtr mDefaultSubscription;

      };

      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTester
      #pragma mark

      //---------------------------------------------------------------------
      class DTLSTester : public SharedRecursiveLock,
                         public zsLib::MessageQueueAssociator,
                         public IDTLSTransportDelegate,
                         public IPromiseSettledDelegate
      {
      protected:
        struct make_private {};

      public:
        struct Expectations {
          ULONG mStateNew {0};
          ULONG mStateConnecting {0};
          ULONG mStateConnected {0};
          ULONG mStateClosed {0};
          ULONG mStateFailed {0};

          ULONG mError {0};

          //-------------------------------------------------------------------
          bool operator==(const Expectations &op2) const
          {
            return (mStateNew == op2.mStateNew) &&
                   (mStateConnecting == op2.mStateConnecting) &&
                   (mStateConnected == op2.mStateConnected) &&
                   (mStateFailed == op2.mStateFailed) &&
                   (mStateClosed == op2.mStateClosed) &&

                   (mError == op2.mError);
          }
        };

      public:
        //---------------------------------------------------------------------
        static DTLSTesterPtr create(
                                    IMessageQueuePtr queue,
                                    IICETransportPtr iceTransport
                                    )
        {
          DTLSTesterPtr pThis(new DTLSTester(queue));
          pThis->mThisWeak = pThis;
          pThis->init(iceTransport);
          return pThis;
        }

        //---------------------------------------------------------------------
        DTLSTester(IMessageQueuePtr queue) :
          SharedRecursiveLock(SharedRecursiveLock::create()),
          MessageQueueAssociator(queue)
        {
          ZS_LOG_BASIC(log("dtls tester"))
        }

        //---------------------------------------------------------------------
        ~DTLSTester()
        {
          ZS_LOG_BASIC(log("dtls tester"))
        }

        //---------------------------------------------------------------------
        void init(IICETransportPtr iceTransport)
        {
          AutoRecursiveLock lock(*this);
          mICETransport = iceTransport;
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
          mDTLS->stop();
        }

        //---------------------------------------------------------------------
        void closeByReset()
        {
          AutoRecursiveLock lock(*this);
          mDTLS.reset();
        }

        //---------------------------------------------------------------------
        Expectations getExpectations() const {return mExpectations;}

        //---------------------------------------------------------------------
        void generateCertificate()
        {
          AutoRecursiveLock lock(*this);
          mCertificatePromise = ICertificate::generateCertificate();
          mCertificatePromise->then(mThisWeak.lock());
        }

        //---------------------------------------------------------------------
        void start(DTLSTesterPtr remote)
        {
          AutoRecursiveLock lock(*this);
          TESTING_CHECK(remote)

          IDTLSTransportPtr localDTLS = getTransport();
          IDTLSTransportPtr remoteDTLS = remote->getTransport();

          TESTING_CHECK(localDTLS)
          TESTING_CHECK(remoteDTLS)

          auto remoteParams = remoteDTLS->getLocalParameters();

          TESTING_CHECK(remoteParams)

          localDTLS->start(*remoteParams);
        }

      protected:

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark DTLSTester::IDTLSTransportDelegate
        #pragma mark

        //---------------------------------------------------------------------
        virtual void onDTLSTransportStateChange(
                                                 IDTLSTransportPtr transport,
                                                 IDTLSTransport::States state
                                                 ) override
        {
          TESTING_CHECK(transport)

          AutoRecursiveLock lock(*this);

          ZS_LOG_BASIC(log("dtls transport state changed") + ZS_PARAM("transport", transport->getID()) + ZS_PARAM("state", IDTLSTransport::toString(state)))

          switch (state) {
            case IDTLSTransportTypes::State_New:         ++mExpectations.mStateNew; break;
            case IDTLSTransportTypes::State_Connecting:  ++mExpectations.mStateConnecting; break;
            case IDTLSTransportTypes::State_Connected:   ++mExpectations.mStateConnected; break;
            case IDTLSTransportTypes::State_Closed:      ++mExpectations.mStateClosed; break;
            case IDTLSTransportTypes::State_Failed:      ++mExpectations.mStateFailed; break;
          }
        }

        //---------------------------------------------------------------------
        virtual void onDTLSTransportError(
                                          IDTLSTransportPtr transport,
                                          ErrorAnyPtr error
                                          ) override
        {
          TESTING_CHECK(transport)

          AutoRecursiveLock lock(*this);

          ++mExpectations.mError;
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark DTLSTester::IPromiseSettledDelegate
        #pragma mark

        //---------------------------------------------------------------------
        virtual void onPromiseSettled(PromisePtr promise) override
        {
          AutoRecursiveLock lock(*this);
          TESTING_CHECK(promise->isResolved())

          TESTING_CHECK(mCertificatePromise)

          auto certificate = mCertificatePromise->value();

          TESTING_CHECK(certificate)

          ZS_LOG_BASIC(log("certificate was generated") + ICertificate::toDebug(certificate))

          std::list<ICertificatePtr> certificates;
          certificates.push_back(certificate);

          mDTLS = IDTLSTransport::create(mThisWeak.lock(), mICETransport, certificates);
          mICETransport.reset();
        }

      protected:
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark DTLSTester => (internal)
        #pragma mark

        //---------------------------------------------------------------------
        Log::Params log(const char *message) const
        {
          ElementPtr objectEl = Element::create("ortc::test::dtls::DTLSTester");
          UseServicesHelper::debugAppend(objectEl, "id", mID);
          return Log::Params(message, objectEl);
        }

        //---------------------------------------------------------------------
        IDTLSTransportPtr getTransport() const
        {
          AutoRecursiveLock lock(*this);
          return mDTLS;
        }

      public:
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark DTLSTester => (data)
        #pragma mark

        AutoPUID mID;
        DTLSTesterWeakPtr mThisWeak;

        IICETransportPtr mICETransport;
        IDTLSTransportPtr mDTLS;

        ICertificateTypes::PromiseWithCertificatePtr mCertificatePromise;

        Expectations mExpectations;
      };
    }
  }
}

ZS_DECLARE_USING_PTR(ortc::test::dtls, FakeICETransport)
ZS_DECLARE_USING_PTR(ortc::test::dtls, DTLSTester)
ZS_DECLARE_USING_PTR(ortc, IICETransport)
using ortc::IICETypes;

#define TEST_BASIC_CONNECTIVITY 0


void doTestDTLS()
{
  if (!ORTC_TEST_DO_DTLS_TRANSPORT_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  UseSettings::applyDefaults();

  auto thread(zsLib::IMessageQueueThread::createBasic());

  FakeICETransportPtr fakeIceObject1;
  FakeICETransportPtr fakeIceObject2;
  DTLSTesterPtr testDTLSObject1;
  DTLSTesterPtr testDTLSObject2;

  TESTING_STDOUT() << "WAITING:      Waiting for DTLS testing to complete (max wait is 180 seconds).\n";

  // check to see if all DNS routines have resolved
  {
    ULONG testNumber = 0;
    ULONG maxSteps = 80;

    do
    {
      TESTING_STDOUT() << "TESTING       ---------->>>>>>>>>> " << testNumber << " <<<<<<<<<<----------\n";

      bool quit = false;
      ULONG expecting = 0;

      DTLSTester::Expectations expectationsDTLS1;
      DTLSTester::Expectations expectationsDTLS2;

      expectationsDTLS1.mStateNew = 0;
      expectationsDTLS1.mStateConnecting = 1;
      expectationsDTLS1.mStateConnected = 1;
      expectationsDTLS1.mStateClosed = 1;
      expectationsDTLS1.mStateFailed = 0;

      expectationsDTLS2 = expectationsDTLS1;

      switch (testNumber) {
        case TEST_BASIC_CONNECTIVITY: {
          {
            // setup for test 0
            fakeIceObject1 = FakeICETransport::create(thread);
            fakeIceObject2 = FakeICETransport::create(thread);

            TESTING_CHECK(fakeIceObject1)
            TESTING_CHECK(fakeIceObject2)

            testDTLSObject1 = DTLSTester::create(thread, fakeIceObject1);
            testDTLSObject2 = DTLSTester::create(thread, fakeIceObject2);

            TESTING_CHECK(testDTLSObject1)
            TESTING_CHECK(testDTLSObject2)
          }
          break;
        }
        default:  quit = true; break;
      }
      if (quit) break;

      expecting = 0;
      expecting += (testDTLSObject1 ? 1 : 0);
      expecting += (testDTLSObject2 ? 1 : 0);

      ULONG found = 0;
      ULONG lastFound = 0;
      ULONG step = 0;

      while (found < expecting)
      {
        TESTING_SLEEP(1000)
        ++step;
        if (step >= maxSteps)
          break;

        found = 0;

        switch (testNumber) {
          case TEST_BASIC_CONNECTIVITY: {
            switch (step) {
              case 2: {
                if (fakeIceObject1) fakeIceObject1->state(IICETransport::State_Checking);
                if (fakeIceObject2) fakeIceObject2->state(IICETransport::State_Checking);
                break;
              }
              case 3: {
                if (testDTLSObject1) testDTLSObject1->generateCertificate();
                if (testDTLSObject2) testDTLSObject2->generateCertificate();
                break;
              }
              case 6: {
                if (fakeIceObject1) fakeIceObject1->role(IICETypes::Role_Controlling);
                if (fakeIceObject2) fakeIceObject1->role(IICETypes::Role_Controlled);
                break;
              }
              case 7: {
                if (fakeIceObject1) fakeIceObject1->linkTransport(fakeIceObject2);
                if (fakeIceObject2) fakeIceObject2->linkTransport(fakeIceObject1);

                if (fakeIceObject1) fakeIceObject1->state(IICETransport::State_Connected);
                if (fakeIceObject2) fakeIceObject2->state(IICETransport::State_Connected);
                break;
              }
              case 10: {
                if (testDTLSObject1) testDTLSObject1->start(testDTLSObject2);
                if (testDTLSObject2) testDTLSObject2->start(testDTLSObject1);
                break;
              }
              case 20: {
                if (fakeIceObject1) fakeIceObject1->state(IICETransport::State_Completed);
                if (fakeIceObject2) fakeIceObject2->state(IICETransport::State_Completed);
                break;
              }
              case 25: {
                if (fakeIceObject1) fakeIceObject1->state(IICETransport::State_Disconnected);
                if (fakeIceObject2) fakeIceObject2->state(IICETransport::State_Disconnected);
                break;
              }
              case 30: {
                if (fakeIceObject1) fakeIceObject1->state(IICETransport::State_Closed);
                if (fakeIceObject2) fakeIceObject2->state(IICETransport::State_Closed);
                break;
              }
              case 35: {
                if (testDTLSObject1) testDTLSObject1->close();
                if (testDTLSObject2) testDTLSObject2->close();
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
          found += (testDTLSObject1 ? (testDTLSObject1->matches(expectationsDTLS1) ? 1 : 0) : 0);
          found += (testDTLSObject2 ? (testDTLSObject2->matches(expectationsDTLS2) ? 1 : 0) : 0);
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
          if (testDTLSObject1) {TESTING_CHECK(testDTLSObject1->matches(expectationsDTLS1))}
          if (testDTLSObject2) {TESTING_CHECK(testDTLSObject2->matches(expectationsDTLS2))}
          break;
        }
      }

      testDTLSObject1.reset();
      testDTLSObject2.reset();

      fakeIceObject1.reset();
      fakeIceObject2.reset();

      ++testNumber;
    } while (true);
  }

  TESTING_STDOUT() << "WAITING:      All DTLS transports have finished. Waiting for 'bogus' events to process (10 second wait).\n";
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
