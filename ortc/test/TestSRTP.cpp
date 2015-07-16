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

#include <ortc/IDTLSTransport.h>
#include <ortc/ISRTPSDESTransport.h>
#include <ortc/ISettings.h>

#include <ortc/internal/ortc_ISRTPTransport.h>
#include <ortc/internal/ortc_SRTPTransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_DTLSTransport.h>

#include <openpeer/services/IHelper.h>

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
using ortc::internal::ISecureTransportForSRTPTransport;
using ortc::internal::ISRTPTransportForSecureTransport;
using ortc::internal::ISRTPTransportDelegate;
using ortc::internal::ISRTPTransportDelegatePtr;


ZS_DECLARE_USING_PTR(ortc::internal, ISRTPTransport)
ZS_DECLARE_USING_PTR(openpeer::services, SecureByteBlock)

ZS_DECLARE_TYPEDEF_PTR(ortc::ISettings, UseSettings)
ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportForSRTPTransport, UseSecureTransport)
ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISRTPTransportForSecureTransport, UseSRTPTransport)

ZS_DECLARE_TYPEDEF_PTR(ortc::ISRTPSDESTransport::CryptoParameters, CryptoParameters)


namespace ortc
{
  namespace test
  {
    namespace dtls
    {
      ZS_DECLARE_INTERACTION_PROXY(IFakeSecureTransportAsyncDelegate)

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IFakeSecureTransportAsyncDelegate
      #pragma mark

      interaction IFakeSecureTransportAsyncDelegate
      {
        virtual void onPacketFromLinkedFakedTransport(
                                                      IICETypes::Components sendOverICETransport,
                                                      IICETypes::Components packetType,
                                                      SecureByteBlockPtr buffer
                                                      ) = 0;
      };

    }
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::test::dtls::IFakeSecureTransportAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(openpeer::services::SecureByteBlockPtr, SecureByteBlockPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IICETypes::Components, Components)
ZS_DECLARE_PROXY_METHOD_3(onPacketFromLinkedFakedTransport, Components, Components, SecureByteBlockPtr)
ZS_DECLARE_PROXY_END()

namespace ortc
{
  namespace test
  {
    namespace dtls
    {
      ZS_DECLARE_CLASS_PTR(FakeSecureTransport)
      ZS_DECLARE_INTERACTION_PTR(ISRTPTester)
      ZS_DECLARE_CLASS_PTR(SRTPTester)

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //---------------------------------------------------------------------
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark ISRTPTester
      #pragma mark

      interaction ISRTPTester
      {
        virtual PUID getID() const = 0;

        virtual bool notifyFakeReceivedPacket(
                                              IICETypes::Components viaTransport,
                                              IICETypes::Components packetType,
                                              const BYTE *buffer,
                                              size_t bufferLengthInBytes
                                              ) = 0;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSecureTransport
      #pragma mark

      //-----------------------------------------------------------------------
      class FakeSecureTransport : public ortc::internal::DTLSTransport,
                                  public IFakeSecureTransportAsyncDelegate
      {
      protected:
        struct make_private {};

      public:

      public:
        //---------------------------------------------------------------------
        FakeSecureTransport(
                            const make_private &,
                            IMessageQueuePtr queue
                            ) :
          DTLSTransport(zsLib::Noop(true), queue)
        {
          ZS_LOG_BASIC(log("created"))
        }

      protected:
        //---------------------------------------------------------------------
        void init(
                  const CryptoParameters &encryptParameters,
                  const CryptoParameters &decryptParameters
                  )
        {
          AutoRecursiveLock lock(*this);
          mSRTPTransport = UseSRTPTransport::create(mThisWeak.lock(), mThisWeak.lock(), encryptParameters, decryptParameters);
        }

      public:
        //---------------------------------------------------------------------
        ~FakeSecureTransport()
        {
          mThisWeak.reset();

          ZS_LOG_BASIC(log("destroyed"))
        }

        //---------------------------------------------------------------------
        static FakeSecureTransportPtr create(
                                             IMessageQueuePtr queue,
                                             const CryptoParameters &encryptParameters,
                                             const CryptoParameters &decryptParameters
                                             )
        {
          FakeSecureTransportPtr pThis(make_shared<FakeSecureTransport>(make_private{}, queue));
          pThis->mThisWeak = pThis;
          pThis->init(encryptParameters, decryptParameters);
          return pThis;
        }

        //---------------------------------------------------------------------
        void linkTransport(
                           ISRTPTesterPtr tester,
                           FakeSecureTransportPtr remoteTransport
                           )
        {
          AutoRecursiveLock lock(*this);
          mOuterTester = tester;
          mLinkedTransport = remoteTransport;

          if (remoteTransport) {
            ZS_LOG_BASIC(log("transport linked") + ZS_PARAM("outer", tester->getID()) + ZS_PARAM("linked transport", remoteTransport->getID()))
          } else {
            ZS_LOG_BASIC(log("transport unlinked"))
          }
        }

        //---------------------------------------------------------------------
        bool fakeSendPacket(
                            IICETypes::Components sendOverICETransport,
                            IICETypes::Components packetType,
                            const BYTE *buffer,
                            size_t bufferLengthInBytes
                            )
        {
          UseSRTPTransportPtr transport;

          {
            AutoRecursiveLock lock(*this);
            transport = mSRTPTransport;
            if (!transport) {
              ZS_LOG_WARNING(Basic, log("no srtp transport available"))
              TESTING_CHECK(false)
              return false;
            }
          }

          return transport->sendPacket(sendOverICETransport, packetType, buffer, bufferLengthInBytes);
        }

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => IICETransport
        #pragma mark

        //---------------------------------------------------------------------
        virtual ElementPtr toDebug() const
        {
          AutoRecursiveLock lock(*this);

          ElementPtr resultEl = Element::create("ortc::test::dtls::FakeSecureTransport");

          UseServicesHelper::debugAppend(resultEl, "id", getID());

          UseServicesHelper::debugAppend(resultEl, "secure transport", mSRTPTransport ? mSRTPTransport->getID() : 0);

          UseServicesHelper::debugAppend(resultEl, "linked transport", (bool)(mLinkedTransport.lock()));

          UseServicesHelper::debugAppend(resultEl, "subscriptions", mSubscriptions.size());
          UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

          return resultEl;
        }

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => ISecureTransportForSRTPTransport
        #pragma mark

        //---------------------------------------------------------------------
        virtual PUID getID() const override {return DTLSTransport::getID();}

        //---------------------------------------------------------------------
        virtual bool sendEncryptedPacket(
                                         IICETypes::Components sendOverICETransport,
                                         IICETypes::Components packetType,
                                         const BYTE *buffer,
                                         size_t bufferLengthInBytes
                                         ) override
        {
          FakeSecureTransportPtr transport;

          {
            AutoRecursiveLock lock(*this);
            transport = mLinkedTransport.lock();
            if (!transport) {
              ZS_LOG_WARNING(Detail, log("not linked to another fake transport") + ZS_PARAM("buffer", (PTRNUMBER)(buffer)) + ZS_PARAM("buffer size", bufferLengthInBytes))
              TESTING_CHECK(false);
              return false;
            }
          }

          ZS_LOG_DEBUG(log("sending packet to linked fake transport") + ZS_PARAM("buffer", (PTRNUMBER)(buffer)) + ZS_PARAM("buffer size", bufferLengthInBytes))

          SecureByteBlockPtr sendBuffer(make_shared<SecureByteBlock>(buffer, bufferLengthInBytes));

          IFakeSecureTransportAsyncDelegateProxy::create(transport)->onPacketFromLinkedFakedTransport(sendOverICETransport, packetType, sendBuffer);
          return true;
        }

        //---------------------------------------------------------------------
        virtual bool handleReceivedDecryptedPacket(
                                                   IICETypes::Components viaTransport,
                                                   IICETypes::Components packetType,
                                                   const BYTE *buffer,
                                                   size_t bufferLengthInBytes
                                                   ) override
        {
          ZS_LOG_DEBUG(log("handling decrypted packet from SRTP") + ZS_PARAM("via", IICETypes::toString(viaTransport)) + ZS_PARAM("packet type", IICETypes::toString(packetType)) + ZS_PARAM("buffer", (PTRNUMBER)(buffer)) + ZS_PARAM("buffer size", bufferLengthInBytes))

          ISRTPTesterPtr tester;

          {
            AutoRecursiveLock lock(*this);

            tester = mOuterTester.lock();
            if (!tester) {
              ZS_LOG_WARNING(Basic, log("tester not found"))
              TESTING_CHECK(false);
              return false;
            }
          }

          return tester->notifyFakeReceivedPacket(viaTransport, packetType, buffer, bufferLengthInBytes);
        }


        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => ISRTPTransportDelegate
        #pragma mark

        virtual void onSRTPTransportLifetimeRemaining(
                                                      ISRTPTransportPtr transport,
                                                      ULONG leastLifetimeRemainingPercentageForAllKeys,
                                                      ULONG overallLifetimeRemainingPercentage
                                                      ) override
        {
          ZS_LOG_DEBUG(log("lifetime remaining") + ZS_PARAM("transport", transport->getID()) + ZS_PARAM("least", leastLifetimeRemainingPercentageForAllKeys) + ZS_PARAM("overall", overallLifetimeRemainingPercentage))
        }

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => IFakeSecureTransportAsyncDelegate
        #pragma mark

        //---------------------------------------------------------------------
        virtual void onPacketFromLinkedFakedTransport(
                                                      IICETypes::Components sendOverICETransport,
                                                      IICETypes::Components packetType,
                                                      SecureByteBlockPtr buffer
                                                      ) override
        {
          UseSRTPTransportPtr transport;

          {
            AutoRecursiveLock lock(*this);
            transport = mSRTPTransport;
            if (!transport) {
              ZS_LOG_WARNING(Detail, log("no dtls transport attached (thus cannot forward received packet)") + ZS_PARAM("buffer", (PTRNUMBER)(buffer->BytePtr())) + ZS_PARAM("buffer size", buffer->SizeInBytes()))
              TESTING_CHECK(false);
              return;
            }
          }

          TESTING_CHECK(buffer)

          ZS_LOG_DEBUG(log("packet received") + ZS_PARAM("buffer", (PTRNUMBER)(buffer->BytePtr())) + ZS_PARAM("buffer size", buffer->SizeInBytes()))

          transport->handleReceivedPacket(sendOverICETransport, buffer->BytePtr(), buffer->SizeInBytes());
        }

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => (internal)
        #pragma mark

        //---------------------------------------------------------------------
        Log::Params log(const char *message) const
        {
          ElementPtr objectEl = Element::create("ortc::test::dtls::FakeSecureTransport");
          UseServicesHelper::debugAppend(objectEl, "id", DTLSTransport::getID());
          return Log::Params(message, objectEl);
        }

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark FakeSecureTransport => (data)
        #pragma mark

        FakeSecureTransportWeakPtr mThisWeak;

        ISRTPTesterWeakPtr mOuterTester;

        UseSRTPTransportPtr mSRTPTransport;

        FakeSecureTransportWeakPtr mLinkedTransport;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPTester
      #pragma mark

      //-----------------------------------------------------------------------
      class SRTPTester : public SharedRecursiveLock,
                         public zsLib::MessageQueueAssociator,
                         public ISRTPTester
      {
      protected:
        struct make_private {};

        struct ExpectingPacket
        {
          IICETypes::Components mVia {IICETypes::Component_RTP};
          IICETypes::Components mPacketType {IICETypes::Component_RTP};
          SecureByteBlockPtr mBuffer;
        };

        typedef std::queue<ExpectingPacket> BufferQueue;

      public:
        struct Expectations {
          ULONG mSentPackets {0};
          ULONG mReceivedPackets {0};

          //-------------------------------------------------------------------
          bool operator==(const Expectations &op2) const
          {
            return (mSentPackets == op2.mSentPackets) &&
                   (mReceivedPackets == op2.mReceivedPackets)
            ;
          }
        };

      public:
        //---------------------------------------------------------------------
        static SRTPTesterPtr create(
                                    IMessageQueuePtr queue,
                                    FakeSecureTransportPtr dtlsTransport
                                    )
        {
          SRTPTesterPtr pThis(new SRTPTester(queue));
          pThis->mThisWeak = pThis;
          pThis->init(dtlsTransport);
          return pThis;
        }

        //---------------------------------------------------------------------
        SRTPTester(IMessageQueuePtr queue) :
          SharedRecursiveLock(SharedRecursiveLock::create()),
          MessageQueueAssociator(queue)
        {
          ZS_LOG_BASIC(log("dtls tester"))
        }

        //---------------------------------------------------------------------
        ~SRTPTester()
        {
          ZS_LOG_BASIC(log("dtls tester"))
        }

        //---------------------------------------------------------------------
        void init(FakeSecureTransportPtr dtlsTransport)
        {
          AutoRecursiveLock lock(*this);
          mDTLS = dtlsTransport;
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
          mDTLS.reset();
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
        void expectingIncomingPacket(
                                     IICETypes::Components viaTransport,
                                     IICETypes::Components packetType,
                                     const BYTE *buffer,
                                     size_t bufferLengthInBytes
                                     )
        {
          AutoRecursiveLock lock(*this);

          ExpectingPacket packetInfo;
          packetInfo.mBuffer = SecureByteBlockPtr(make_shared<SecureByteBlock>(buffer, bufferLengthInBytes));
          packetInfo.mVia = viaTransport;
          packetInfo.mPacketType = packetType;

          mExpectingPackets.push(packetInfo);
        }

        //---------------------------------------------------------------------
        bool sendPacket(
                        IICETypes::Components viaTransport,
                        IICETypes::Components packetType,
                        const BYTE *buffer,
                        size_t bufferLengthInBytes
                        )
        {
          FakeSecureTransportPtr transport;

          {
            AutoRecursiveLock lock(*this);
            transport = mDTLS;
            ++mExpectations.mSentPackets;
          }

          return transport->fakeSendPacket(viaTransport, packetType, buffer, bufferLengthInBytes);
        }

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark ISRTPTester
        #pragma mark

        //---------------------------------------------------------------------
        virtual PUID getID() const {return mID;}

        //---------------------------------------------------------------------
        virtual bool notifyFakeReceivedPacket(
                                              IICETypes::Components viaTransport,
                                              IICETypes::Components packetType,
                                              const BYTE *buffer,
                                              size_t bufferLengthInBytes
                                              ) override
        {
          AutoRecursiveLock lock(*this);

          TESTING_CHECK(mExpectingPackets.size() > 0)

          ExpectingPacket expecting = mExpectingPackets.front();
          mExpectingPackets.pop();

          TESTING_EQUAL(expecting.mVia, viaTransport)
          TESTING_EQUAL(expecting.mPacketType, packetType)
          TESTING_CHECK(((bool)expecting.mBuffer))

          SecureByteBlockPtr tempBuffer(make_shared<SecureByteBlock>(buffer, bufferLengthInBytes));

          TESTING_CHECK(0 == UseServicesHelper::compare(*tempBuffer, *expecting.mBuffer))

          ++mExpectations.mReceivedPackets;

          return true;
        }

      protected:
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark SRTPTester => (internal)
        #pragma mark

        //---------------------------------------------------------------------
        Log::Params log(const char *message) const
        {
          ElementPtr objectEl = Element::create("ortc::test::dtls::SRTPTester");
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
        #pragma mark SRTPTester => (data)
        #pragma mark

        AutoPUID mID;
        SRTPTesterWeakPtr mThisWeak;

        FakeSecureTransportPtr mDTLS;

        Expectations mExpectations;

        BufferQueue mExpectingPackets;
      };
    }
  }
}

ZS_DECLARE_USING_PTR(ortc::test::dtls, FakeSecureTransport)
ZS_DECLARE_USING_PTR(ortc::test::dtls, SRTPTester)
ZS_DECLARE_USING_PTR(ortc, IICETransport)
using ortc::IICETypes;

#define TEST_BASIC_CONNECTIVITY 0


void doTestSRTP()
{
  if (!ORTC_TEST_DO_SRTP_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  ortc::ISettings::applyDefaults();

  zsLib::MessageQueueThreadPtr thread(zsLib::MessageQueueThread::createBasic());

  FakeSecureTransportPtr fakeDTLSObject1;
  FakeSecureTransportPtr fakeDTLSObject2;
  SRTPTesterPtr testSRTPObject1;
  SRTPTesterPtr testSRTPObject2;

  TESTING_STDOUT() << "WAITING:      Waiting for ICE testing to complete (max wait is 180 seconds).\n";

  // check to see if all DNS routines have resolved
  {
    ULONG testNumber = 0;
    ULONG maxSteps = 80;

    do
    {
      TESTING_STDOUT() << "TESTING       ---------->>>>>>>>>> " << testNumber << " <<<<<<<<<<----------\n";

      bool quit = false;
      ULONG expecting = 0;

      SRTPTester::Expectations expectationsDTLS1;
      SRTPTester::Expectations expectationsDTLS2;

      expectationsDTLS1.mSentPackets = 0;
      expectationsDTLS1.mReceivedPackets = 0;

      expectationsDTLS2 = expectationsDTLS1;

      switch (testNumber) {
        case TEST_BASIC_CONNECTIVITY: {
          {
            CryptoParameters encrypt1;
            CryptoParameters decrypt1;

            CryptoParameters encrypt2;
            CryptoParameters decrypt2;

            // setup for test 0
            fakeDTLSObject1 = FakeSecureTransport::create(thread, encrypt1, decrypt1);
            fakeDTLSObject2 = FakeSecureTransport::create(thread, encrypt2, decrypt2);

            TESTING_CHECK(fakeDTLSObject1)
            TESTING_CHECK(fakeDTLSObject2)

            testSRTPObject1 = SRTPTester::create(thread, fakeDTLSObject1);
            testSRTPObject2 = SRTPTester::create(thread, fakeDTLSObject2);

            TESTING_CHECK(testSRTPObject1)
            TESTING_CHECK(testSRTPObject2)
          }
          break;
        }
        default:  quit = true; break;
      }
      if (quit) break;

      expecting = 0;
      expecting += (testSRTPObject1 ? 1 : 0);
      expecting += (testSRTPObject2 ? 1 : 0);

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
                if (fakeDTLSObject1) fakeDTLSObject1->linkTransport(testSRTPObject1, fakeDTLSObject2);
                if (fakeDTLSObject2) fakeDTLSObject2->linkTransport(testSRTPObject2, fakeDTLSObject1);
                break;
              }
              case 10: {
                SecureByteBlockPtr buffer(std::make_shared<SecureByteBlock>(40));  // allocate a buffer of 40 bytes
                if (testSRTPObject1) testSRTPObject1->expectingIncomingPacket(IICETypes::Component_RTP, IICETypes::Component_RTP, *buffer, buffer->SizeInBytes());
                if (testSRTPObject2) testSRTPObject2->sendPacket(IICETypes::Component_RTP, IICETypes::Component_RTP, *buffer, buffer->SizeInBytes());
                break;
              }
              case 20: {
                break;
              }
              case 25: {
                break;
              }
              case 30: {
                break;
              }
              case 35: {
                if (testSRTPObject1) testSRTPObject1->close();
                if (testSRTPObject2) testSRTPObject2->close();
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
          found += (testSRTPObject1 ? (testSRTPObject1->matches(expectationsDTLS1) ? 1 : 0) : 0);
          found += (testSRTPObject2 ? (testSRTPObject2->matches(expectationsDTLS2) ? 1 : 0) : 0);
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
          if (testSRTPObject1) {TESTING_CHECK(testSRTPObject1->matches(expectationsDTLS1))}
          if (testSRTPObject2) {TESTING_CHECK(testSRTPObject2->matches(expectationsDTLS2))}
          break;
        }
      }

      testSRTPObject1.reset();
      testSRTPObject2.reset();

      fakeDTLSObject1.reset();
      fakeDTLSObject2.reset();

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
