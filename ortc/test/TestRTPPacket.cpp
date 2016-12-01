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



#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTPUtils.h>
#include <ortc/internal/ortc_Helper.h>

#include <ortc/services/IHelper.h>

#include <zsLib/IMessageQueueThread.h>
#include <zsLib/ISettings.h>
//
//#include <zsLib/XML.h>

#include "config.h"
#include "testing.h"

namespace ortc { namespace test { ZS_DECLARE_SUBSYSTEM(ortc_test) } }

using std::make_shared;

using zsLib::BYTE;
//using zsLib::String;
using zsLib::ULONG;
using zsLib::WORD;
using zsLib::DWORD;
//using zsLib::PTRNUMBER;
using zsLib::IMessageQueue;
//using zsLib::Log;
using zsLib::AutoPUID;
using zsLib::AutoRecursiveLock;
//using namespace zsLib::XML;
//
//
ZS_DECLARE_TYPEDEF_PTR(zsLib::ISettings, UseSettings)
ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper)
ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper)
ZS_DECLARE_TYPEDEF_PTR(ortc::internal::RTPUtils, UseRTPUtils)


namespace ortc
{
  namespace test
  {
    namespace rtppacket
    {
      ZS_DECLARE_CLASS_PTR(Tester)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPPacket)

      class Tester : public SharedRecursiveLock
      {
      public:
        //---------------------------------------------------------------------
        Tester() :
          SharedRecursiveLock(SharedRecursiveLock::create())
        {
        }

        //---------------------------------------------------------------------
        ~Tester()
        {
        }

        //---------------------------------------------------------------------
        void init()
        {
        }

        //---------------------------------------------------------------------
        static TesterPtr create()
        {
          TesterPtr pThis(make_shared<Tester>());
          pThis->mThisWeak = pThis;
          return pThis;
        }

        //---------------------------------------------------------------------
        static SecureByteBlockPtr createPacket(
                                               BYTE version,
                                               BYTE padding,
                                               BYTE cc, // 0x0 -> 0xF
                                               bool m,
                                               BYTE pt, // 0x00 -> 0x7F
                                               WORD sequenceNumber,
                                               DWORD timeStamp,
                                               DWORD ssrc,
                                               DWORD *ccrcs,
                                               BYTE *headerExtensionData,
                                               size_t headerExtensionSizeInBytes,
                                               const char *payloadData
                                               )
        {
          size_t size = 12 + (sizeof(DWORD)*cc) + headerExtensionSizeInBytes + (NULL != payloadData ? strlen(payloadData) : 0) + static_cast<size_t>(padding);

          auto result = make_shared<SecureByteBlock>(size);

          BYTE *buffer = result->BytePtr();

          BYTE *pos = buffer;
          bool x = (NULL != headerExtensionData);
          bool p = (0 != padding);

          pos[0] = (version << 6) | ((p ? 1 : 0) << 5) | ((x ? 1 :0) << 4) | (cc & 0xF);
          pos[1] = ((m ? 1: 0) << 7) | (pt & 0x7F);

          UseRTPUtils::setBE16(&(pos[2]), sequenceNumber);
          UseRTPUtils::setBE32(&(pos[4]), timeStamp);
          UseRTPUtils::setBE32(&(pos[8]), ssrc);

          pos += (sizeof(DWORD)*3);
          for (size_t index = 0; index < (static_cast<size_t>(cc) & 0xF); ++index, pos += sizeof(DWORD))
          {
            UseRTPUtils::setBE32(&(pos[0]), ccrcs[index]);
          }

          if (NULL != headerExtensionData) {
            memcpy(pos, headerExtensionData, headerExtensionSizeInBytes);
          }

          pos += headerExtensionSizeInBytes;

          if (NULL != payloadData) {
            memcpy(pos, payloadData, strlen(payloadData));
            pos += strlen(payloadData);
          }

          if (0 != padding) {
            pos += (padding-1);
            pos[0] = padding;
          }
          return result;
        }
        
        //---------------------------------------------------------------------
        void load(
                  const SecureByteBlock &buffer,
                  bool willSucceed = true
                  )
        {
          AutoRecursiveLock lock(*this);
          mPacket = RTPPacket::create(buffer);
          if (willSucceed) {
            TESTING_CHECK((bool)mPacket)
          } else {
            TESTING_CHECK(!(bool)mPacket)
          }
        }

        //---------------------------------------------------------------------
        void compare(const SecureByteBlock &buffer)
        {
          AutoRecursiveLock lock(*this);
          TESTING_CHECK((bool)mPacket)

          TESTING_EQUAL(0, UseServicesHelper::compare(buffer, *(mPacket->buffer())))
        }

        //---------------------------------------------------------------------
        RTPPacketPtr getPacket()
        {
          AutoRecursiveLock lock(*this);
          return mPacket;
        }

        //---------------------------------------------------------------------
        void compareExtensionHeader(
                                    const BYTE *headerExtension,
                                    size_t headerExtensionSizeInBytes
                                    )
        {
          AutoRecursiveLock lock(*this);
          TESTING_CHECK((bool)mPacket)

          const BYTE *buffer = mPacket->ptr();
          TESTING_CHECK(buffer)

          const BYTE *pos = buffer + mPacket->headerSize();

          if (0 == mPacket->headerExtensionSize()) {
            TESTING_CHECK(NULL == headerExtension)
            TESTING_CHECK(0 == headerExtensionSizeInBytes)
          }

          SecureByteBlockPtr op1 = UseServicesHelper::convertToBuffer(pos, mPacket->headerExtensionSize());
          SecureByteBlockPtr op2 = UseServicesHelper::convertToBuffer(headerExtension, headerExtensionSizeInBytes);

          TESTING_EQUAL(0, UseServicesHelper::compare(*op1, *op2))
        }

      protected:
        AutoPUID mID;
        TesterWeakPtr mThisWeak;

        RTPPacketPtr mPacket;
      };

    }
  }
}

static void bogusSleep()
{
  for (int loop = 0; loop < 100; ++loop)
  {
    TESTING_SLEEP(100)
  }
}

#define TEST_BASIC_RTP 0

ZS_DECLARE_USING_PTR(ortc::test::rtppacket, Tester)
ZS_DECLARE_USING_PTR(ortc::internal, RTPPacket)

static BYTE gHeader1[] =
{
  0x10, 0x00, 0x00, 0x03,
  0x04, 0x00, 0x05, 0x01,
  0x55, 0x00, 0x06, 0x04,
  0x41, 0x42, 0x43, 0x44
};

static BYTE gHeader2[] =
{
  0xBE, 0xDE, 0x00, 0x03,
  0xE0, 0x77, 0xD1, 0x88,
  0x99, 0x00, 0x00, 0xC3,
  0x99, 0xAA, 0xBB, 0xCC
};

static BYTE gHeader3[] =
{
  0xBE, 0xDE, 0x00, 0x05,
  0x00, 0x00, 0x00, 0x00,
  0xE0, 0x77, 0xD1, 0x88,
  0x99, 0x00, 0x00, 0xC3,
  0x99, 0xAA, 0xBB, 0xCC,
  0x00, 0xFD, 0xAB, 0xCD
};

static BYTE gHeader4[] =
{
  0x10, 0x02, 0x00, 0x04,
  0x00, 0x04, 0x00, 0x05,
  0x01, 0x55, 0x00, 0x06,
  0x04, 0x41, 0x42, 0x43,
  0x44, 0x00, 0x00, 0x00
};

static BYTE gHeader5[] =
{
  0x10, 0x05, 0x00, 0x00
};

void doTestRTPPacket()
{
  if (!ORTC_TEST_DO_RTP_PACKET_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  UseSettings::applyDefaults();

  auto thread(zsLib::IMessageQueueThread::createBasic());

  TesterPtr testObject1;
  TesterPtr testObject2;

  TESTING_STDOUT() << "WAITING:      Waiting for RTP packet testing to complete (max wait is 180 seconds).\n";

  // check to see if all DNS routines have resolved
  {
    ULONG testNumber = 0;
    ULONG maxSteps = 40;

    do
    {
      TESTING_STDOUT() << "TESTING       ---------->>>>>>>>>> " << testNumber << " <<<<<<<<<<----------\n";

      bool quit = false;
      ULONG expecting = 0;

      switch (testNumber) {
        case TEST_BASIC_RTP: {
          {
            testObject1 = Tester::create();

            TESTING_CHECK(testObject1)
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

      bool reachedFinalStep = false;

      while ((found < expecting) ||
             (!reachedFinalStep))
      {
        TESTING_SLEEP(1000)
        ++step;
        if (step >= maxSteps)
          break;

        found = 0;

        switch (testNumber) {
          case TEST_BASIC_RTP: {
            switch (step) {
              case 1: {
                const char *payload = "ABCDEFG";
                testObject1->load(*Tester::createPacket(2, 0, 0, false, 96, 1, 1024, 5, NULL, NULL, 0, payload));

                auto packet = testObject1->getPacket();
                TESTING_CHECK(packet)

                TESTING_CHECK(NULL != packet->ptr())
                TESTING_EQUAL(12 + strlen(payload), packet->size())

                TESTING_EQUAL(2, packet->version())
                TESTING_EQUAL(0, packet->padding())
                TESTING_EQUAL(0, packet->cc())
                TESTING_CHECK(!packet->m())
                TESTING_EQUAL(96, packet->pt())
                TESTING_EQUAL(1, packet->sequenceNumber())
                TESTING_EQUAL(1024, packet->timestamp())
                TESTING_EQUAL(5, packet->ssrc())

                RTPPacket::HeaderExtension temp[3];

                temp[0].mID = 0x04;
                temp[0].mDataSizeInBytes = 0;
                temp[0].mNext = &(temp[1]);

                temp[1].mID = 0x05;
                temp[1].mData = new BYTE[1] {0x55};
                temp[1].mDataSizeInBytes = 1;
                temp[1].mPostPaddingSize = 1;
                temp[1].mNext = &(temp[2]);

                temp[2].mID = 0x06;
                temp[2].mData = new BYTE[4] {0x41, 0x42, 0x43, 0x44};
                temp[2].mDataSizeInBytes = 4;
                temp[2].mNext = NULL;

                packet->changeHeaderExtensions(&temp[0]);

                TESTING_EQUAL(3, packet->totalHeaderExtensions())

                {
                  RTPPacket::HeaderExtension *current = packet->firstHeaderExtension();
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[0].mID, current->mID)
                  TESTING_EQUAL(temp[0].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[0].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(1), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(1);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[1].mID, current->mID)
                  TESTING_EQUAL(temp[1].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[1].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_CHECK(current->mData)
                  TESTING_EQUAL(0x55, current->mData[0])
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(2), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(2);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[2].mID, current->mID)
                  TESTING_EQUAL(temp[2].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[2].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_CHECK(current->mData)
                  TESTING_EQUAL(0x41, current->mData[0])
                  TESTING_EQUAL(0x42, current->mData[1])
                  TESTING_EQUAL(0x43, current->mData[2])
                  TESTING_EQUAL(0x44, current->mData[3])
                  TESTING_EQUAL(NULL, current->mNext)
                }

                testObject1->compareExtensionHeader(&gHeader1[0], sizeof(gHeader1));

                delete [] (temp[1].mData);
                delete [] (temp[2].mData);

                auto tempPacket = Tester::createPacket(2, 0, 0, false, 96, 1, 1024, 5, NULL, &gHeader1[0], sizeof(gHeader1), payload);

                testObject1->compare(*tempPacket);
                break;
              }
              case 2: {
                const char *payload = "QWERTYUIOPASDFGHJKLZXCVBNM";

                DWORD *csrs = new DWORD[5] {19,20,21,22,23};
                auto tempPacket1 = Tester::createPacket(2, 3, 5, true, 97, 2, 2048, 6, csrs, &gHeader1[0], sizeof(gHeader1), payload);
                testObject1->load(*tempPacket1);

                auto packet = testObject1->getPacket();
                TESTING_CHECK(packet)

                TESTING_CHECK(NULL != packet->ptr())
                TESTING_EQUAL(12 + 20 + sizeof(gHeader1) + strlen(payload) + 3, packet->size())

                TESTING_EQUAL(2, packet->version())
                TESTING_EQUAL(3, packet->padding())
                TESTING_EQUAL(5, packet->cc())
                TESTING_CHECK(packet->m())
                TESTING_EQUAL(97, packet->pt())
                TESTING_EQUAL(2, packet->sequenceNumber())
                TESTING_EQUAL(2048, packet->timestamp())
                TESTING_EQUAL(6, packet->ssrc())
                TESTING_EQUAL(csrs[0], packet->getCSRC(0))
                TESTING_EQUAL(csrs[1], packet->getCSRC(1))
                TESTING_EQUAL(csrs[2], packet->getCSRC(2))
                TESTING_EQUAL(csrs[3], packet->getCSRC(3))
                TESTING_EQUAL(csrs[4], packet->getCSRC(4))

                TESTING_EQUAL(3, packet->totalHeaderExtensions())

                {
                  RTPPacket::HeaderExtension *current = packet->firstHeaderExtension();
                  TESTING_CHECK(current)
                  TESTING_EQUAL(4, current->mID)
                  TESTING_EQUAL(0, current->mDataSizeInBytes)
                  TESTING_EQUAL(0, current->mPostPaddingSize)
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(1), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(1);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(5, current->mID)
                  TESTING_CHECK(current->mData)
                  TESTING_EQUAL(0x55, current->mData[0])
                  TESTING_EQUAL(1, current->mDataSizeInBytes)
                  TESTING_EQUAL(1, current->mPostPaddingSize)
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(2), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(2);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(6, current->mID)
                  TESTING_CHECK(current->mData)
                  TESTING_EQUAL(0x41, current->mData[0])
                  TESTING_EQUAL(0x42, current->mData[1])
                  TESTING_EQUAL(0x43, current->mData[2])
                  TESTING_EQUAL(0x44, current->mData[3])
                  TESTING_EQUAL(4, current->mDataSizeInBytes)
                  TESTING_EQUAL(0, current->mPostPaddingSize)
                  TESTING_EQUAL(NULL, current->mNext)
                }

                RTPPacket::HeaderExtension temp[3];

                temp[0].mID = 0x0E;
                temp[0].mDataSizeInBytes = 1;
                temp[0].mData = new BYTE[1] {0x77};
                temp[0].mNext = &(temp[1]);

                temp[1].mID = 0x0D;
                temp[1].mData = new BYTE[2] {0x88, 0x99};
                temp[1].mDataSizeInBytes = 2;
                temp[1].mPostPaddingSize = 2;
                temp[1].mNext = &(temp[2]);

                temp[2].mID = 0x0C;
                temp[2].mData = new BYTE[4] {0x99, 0xAA, 0xBB, 0xCC};
                temp[2].mDataSizeInBytes = 4;
                temp[2].mNext = NULL;

                packet->changeHeaderExtensions(&temp[0]);

                TESTING_EQUAL(3, packet->totalHeaderExtensions())

                {
                  RTPPacket::HeaderExtension *current = packet->firstHeaderExtension();
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[0].mID, current->mID)
                  TESTING_EQUAL(temp[0].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[0].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_EQUAL(0x77, current->mData[0])
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(1), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(1);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[1].mID, current->mID)
                  TESTING_EQUAL(temp[1].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[1].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_EQUAL(0x88, current->mData[0])
                  TESTING_EQUAL(0x99, current->mData[1])
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(2), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(2);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[2].mID, current->mID)
                  TESTING_EQUAL(temp[2].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[2].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_EQUAL(0x99, current->mData[0])
                  TESTING_EQUAL(0xAA, current->mData[1])
                  TESTING_EQUAL(0xBB, current->mData[2])
                  TESTING_EQUAL(0xCC, current->mData[3])
                  TESTING_EQUAL(NULL, current->mNext)
                }

                testObject1->compareExtensionHeader(&gHeader2[0], sizeof(gHeader2));
                
                delete [] (temp[0].mData);
                delete [] (temp[1].mData);
                delete [] (temp[2].mData);

                auto tempPacket2 = Tester::createPacket(2, 3, 5, true, 97, 2, 2048, 6, csrs, &gHeader2[0], sizeof(gHeader2), payload);
                testObject1->compare(*tempPacket2);

                delete [] csrs;
                csrs = NULL;

                break;
              }
              case 3: {
                const char *payload = "QWERTYUIOPASDFGHJKLZXCVBNM";

//                BYTE version,
//                BYTE padding,
//                BYTE cc, // 0x0 -> 0xF
//                bool m,
//                BYTE pt, // 0x00 -> 0x7F
//                WORD sequenceNumber,
//                DWORD timeStamp,
//                DWORD ssrc,
//                DWORD *ccrcs,
//                BYTE *headerExtensionData,
//                size_t headerExtensionSizeInBytes,
//                const char *payloadData

                DWORD *csrs = new DWORD[1] {23};
                auto tempPacket1 = Tester::createPacket(2, 1, 1, false, 98, 3, 4096, 7, csrs, &gHeader2[0], sizeof(gHeader2), payload);
                testObject1->load(*tempPacket1);

                auto packet = testObject1->getPacket();
                TESTING_CHECK(packet)

                TESTING_CHECK(NULL != packet->ptr())
                TESTING_EQUAL(12 + 4 + sizeof(gHeader2) + strlen(payload) + 1, packet->size())

                TESTING_EQUAL(2, packet->version())
                TESTING_EQUAL(1, packet->padding())
                TESTING_EQUAL(1, packet->cc())
                TESTING_CHECK(!packet->m())
                TESTING_EQUAL(98, packet->pt())
                TESTING_EQUAL(3, packet->sequenceNumber())
                TESTING_EQUAL(4096, packet->timestamp())
                TESTING_EQUAL(7, packet->ssrc())
                TESTING_EQUAL(csrs[0], packet->getCSRC(0))

                TESTING_EQUAL(3, packet->totalHeaderExtensions())

                TESTING_EQUAL(sizeof(DWORD) + (3 * sizeof(DWORD)), packet->headerExtensionSize())

                {
                  RTPPacket::HeaderExtension *current = packet->firstHeaderExtension();
                  TESTING_CHECK(current)
                  TESTING_EQUAL(0xE, current->mID)
                  TESTING_EQUAL(1, current->mDataSizeInBytes)
                  TESTING_EQUAL(0, current->mPostPaddingSize)
                  TESTING_CHECK(current->mData)
                  TESTING_EQUAL(0x77, current->mData[0])
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(1), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(1);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(0xD, current->mID)
                  TESTING_EQUAL(2, current->mDataSizeInBytes)
                  TESTING_EQUAL(2, current->mPostPaddingSize)
                  TESTING_CHECK(current->mData)
                  TESTING_EQUAL(0x88, current->mData[0])
                  TESTING_EQUAL(0x99, current->mData[1])
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(2), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(2);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(0xC, current->mID)
                  TESTING_EQUAL(4, current->mDataSizeInBytes)
                  TESTING_EQUAL(0, current->mPostPaddingSize)
                  TESTING_CHECK(current->mData)
                  TESTING_EQUAL(0x99, current->mData[0])
                  TESTING_EQUAL(0xAA, current->mData[1])
                  TESTING_EQUAL(0xBB, current->mData[2])
                  TESTING_EQUAL(0xCC, current->mData[3])
                  TESTING_EQUAL(NULL, current->mNext)
                }

                RTPPacket::HeaderExtension temp[3];

                temp[0].mID = 0x0E;
                temp[0].mDataSizeInBytes = 1;
                temp[0].mData = new BYTE[1] {0x77};
                temp[0].mNext = &(temp[1]);

                temp[1].mID = 0x0D;
                temp[1].mData = new BYTE[2] {0x88, 0x99};
                temp[1].mDataSizeInBytes = 2;
                temp[1].mPostPaddingSize = 2;
                temp[1].mNext = &(temp[2]);

                temp[2].mID = 0x0C;
                temp[2].mData = new BYTE[4] {0x99, 0xAA, 0xBB, 0xCC};
                temp[2].mDataSizeInBytes = 4;
                temp[2].mPostPaddingSize = 1;
                temp[2].mNext = NULL;

                BYTE *unparsedBuffer = new BYTE[3] {0xFD, 0xAB, 0xCD};
                packet->mHeaderExtensionPrepaddedSize = 4;
                packet->mHeaderExtensionParseStoppedPos = unparsedBuffer;
                packet->mHeaderExtensionParseStoppedSize = 3;
                packet->changeHeaderExtensions(&temp[0]);

                TESTING_EQUAL(4, packet->headerExtensionPrepaddedSize())
                TESTING_EQUAL(0, packet->headerExtensionAppBits())
                TESTING_EQUAL(3, packet->headerExtensionParseStoppedSize())
                TESTING_CHECK(packet->headerExtensionParseStopped())
                TESTING_EQUAL(0xFD, packet->headerExtensionParseStopped()[0])
                TESTING_EQUAL(0xAB, packet->headerExtensionParseStopped()[1])
                TESTING_EQUAL(0xCD, packet->headerExtensionParseStopped()[2])

                TESTING_EQUAL(3, packet->totalHeaderExtensions())

                {
                  RTPPacket::HeaderExtension *current = packet->firstHeaderExtension();
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[0].mID, current->mID)
                  TESTING_EQUAL(temp[0].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[0].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_EQUAL(0x77, current->mData[0])
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(1), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(1);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[1].mID, current->mID)
                  TESTING_EQUAL(temp[1].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[1].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_EQUAL(0x88, current->mData[0])
                  TESTING_EQUAL(0x99, current->mData[1])
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(2), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(2);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[2].mID, current->mID)
                  TESTING_EQUAL(temp[2].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[2].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_EQUAL(0x99, current->mData[0])
                  TESTING_EQUAL(0xAA, current->mData[1])
                  TESTING_EQUAL(0xBB, current->mData[2])
                  TESTING_EQUAL(0xCC, current->mData[3])
                  TESTING_EQUAL(NULL, current->mNext)
                }

                testObject1->compareExtensionHeader(&gHeader3[0], sizeof(gHeader3));

                delete [] (temp[0].mData);
                delete [] (temp[1].mData);
                delete [] (temp[2].mData);

                auto tempPacket2 = Tester::createPacket(2, 1, 1, false, 98, 3, 4096, 7, csrs, &gHeader3[0], sizeof(gHeader3), payload);
                testObject1->compare(*tempPacket2);

                delete [] unparsedBuffer;
                unparsedBuffer = NULL;

                delete [] csrs;
                csrs = NULL;
                break;
              }
              case 4: {
                const char *payload = "QWERTYUIOPASDFGHJKLZXCVBNM";

                //                BYTE version,
                //                BYTE padding,
                //                BYTE cc, // 0x0 -> 0xF
                //                bool m,
                //                BYTE pt, // 0x00 -> 0x7F
                //                WORD sequenceNumber,
                //                DWORD timeStamp,
                //                DWORD ssrc,
                //                DWORD *ccrcs,
                //                BYTE *headerExtensionData,
                //                size_t headerExtensionSizeInBytes,
                //                const char *payloadData

                DWORD *csrs = new DWORD[1] {23};
                auto tempPacket1 = Tester::createPacket(2, 1, 1, false, 98, 3, 4096, 7, csrs, &gHeader3[0], sizeof(gHeader3), payload);
                testObject1->load(*tempPacket1);

                auto packet = testObject1->getPacket();
                TESTING_CHECK(packet)

                TESTING_CHECK(NULL != packet->ptr())
                TESTING_EQUAL(12 + 4 + sizeof(gHeader3) + strlen(payload) + 1, packet->size())

                TESTING_EQUAL(2, packet->version())
                TESTING_EQUAL(1, packet->padding())
                TESTING_EQUAL(1, packet->cc())
                TESTING_CHECK(!packet->m())
                TESTING_EQUAL(98, packet->pt())
                TESTING_EQUAL(3, packet->sequenceNumber())
                TESTING_EQUAL(4096, packet->timestamp())
                TESTING_EQUAL(7, packet->ssrc())
                TESTING_EQUAL(csrs[0], packet->getCSRC(0))

                TESTING_EQUAL(3, packet->totalHeaderExtensions())

                TESTING_EQUAL(sizeof(DWORD) + (5 * sizeof(DWORD)), packet->headerExtensionSize())

                TESTING_EQUAL(4, packet->headerExtensionPrepaddedSize())
                TESTING_EQUAL(0, packet->headerExtensionAppBits())
                TESTING_EQUAL(3, packet->headerExtensionParseStoppedSize())
                TESTING_CHECK(packet->headerExtensionParseStopped())
                TESTING_EQUAL(0xFD, packet->headerExtensionParseStopped()[0])
                TESTING_EQUAL(0xAB, packet->headerExtensionParseStopped()[1])
                TESTING_EQUAL(0xCD, packet->headerExtensionParseStopped()[2])

                {
                  RTPPacket::HeaderExtension *current = packet->firstHeaderExtension();
                  TESTING_CHECK(current)
                  TESTING_EQUAL(0xE, current->mID)
                  TESTING_EQUAL(1, current->mDataSizeInBytes)
                  TESTING_EQUAL(0, current->mPostPaddingSize)
                  TESTING_CHECK(current->mData)
                  TESTING_EQUAL(0x77, current->mData[0])
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(1), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(1);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(0xD, current->mID)
                  TESTING_EQUAL(2, current->mDataSizeInBytes)
                  TESTING_EQUAL(2, current->mPostPaddingSize)
                  TESTING_CHECK(current->mData)
                  TESTING_EQUAL(0x88, current->mData[0])
                  TESTING_EQUAL(0x99, current->mData[1])
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(2), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(2);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(0xC, current->mID)
                  TESTING_EQUAL(4, current->mDataSizeInBytes)
                  TESTING_EQUAL(1, current->mPostPaddingSize)
                  TESTING_CHECK(current->mData)
                  TESTING_EQUAL(0x99, current->mData[0])
                  TESTING_EQUAL(0xAA, current->mData[1])
                  TESTING_EQUAL(0xBB, current->mData[2])
                  TESTING_EQUAL(0xCC, current->mData[3])
                  TESTING_EQUAL(NULL, current->mNext)
                }

                RTPPacket::HeaderExtension temp[3];

                temp[0].mID = 0x0E;
                temp[0].mDataSizeInBytes = 1;
                temp[0].mData = new BYTE[1] {0x77};
                temp[0].mNext = &(temp[1]);

                temp[1].mID = 0x0D;
                temp[1].mData = new BYTE[2] {0x88, 0x99};
                temp[1].mDataSizeInBytes = 2;
                temp[1].mPostPaddingSize = 2;
                temp[1].mNext = &(temp[2]);

                temp[2].mID = 0x0C;
                temp[2].mData = new BYTE[4] {0x99, 0xAA, 0xBB, 0xCC};
                temp[2].mDataSizeInBytes = 4;
                temp[2].mPostPaddingSize = 1;
                temp[2].mNext = NULL;

                BYTE *unparsedBuffer = new BYTE[3] {0xFD, 0xAB, 0xCD};
                packet->mHeaderExtensionPrepaddedSize = 4;
                packet->mHeaderExtensionParseStoppedPos = unparsedBuffer;
                packet->mHeaderExtensionParseStoppedSize = 3;
                packet->changeHeaderExtensions(&temp[0]);

                TESTING_EQUAL(4, packet->headerExtensionPrepaddedSize())
                TESTING_EQUAL(0, packet->headerExtensionAppBits())
                TESTING_EQUAL(3, packet->headerExtensionParseStoppedSize())
                TESTING_CHECK(packet->headerExtensionParseStopped())
                TESTING_EQUAL(0xFD, packet->headerExtensionParseStopped()[0])
                TESTING_EQUAL(0xAB, packet->headerExtensionParseStopped()[1])
                TESTING_EQUAL(0xCD, packet->headerExtensionParseStopped()[2])

                TESTING_EQUAL(3, packet->totalHeaderExtensions())

                {
                  RTPPacket::HeaderExtension *current = packet->firstHeaderExtension();
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[0].mID, current->mID)
                  TESTING_EQUAL(temp[0].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[0].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_EQUAL(0x77, current->mData[0])
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(1), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(1);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[1].mID, current->mID)
                  TESTING_EQUAL(temp[1].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[1].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_EQUAL(0x88, current->mData[0])
                  TESTING_EQUAL(0x99, current->mData[1])
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(2), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(2);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[2].mID, current->mID)
                  TESTING_EQUAL(temp[2].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[2].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_EQUAL(0x99, current->mData[0])
                  TESTING_EQUAL(0xAA, current->mData[1])
                  TESTING_EQUAL(0xBB, current->mData[2])
                  TESTING_EQUAL(0xCC, current->mData[3])
                  TESTING_EQUAL(NULL, current->mNext)
                }

                testObject1->compareExtensionHeader(&gHeader3[0], sizeof(gHeader3));
                
                delete [] (temp[0].mData);
                delete [] (temp[1].mData);
                delete [] (temp[2].mData);
                
                auto tempPacket2 = Tester::createPacket(2, 1, 1, false, 98, 3, 4096, 7, csrs, &gHeader3[0], sizeof(gHeader3), payload);
                testObject1->compare(*tempPacket2);
                
                delete [] unparsedBuffer;
                unparsedBuffer = NULL;
                
                delete [] csrs;
                csrs = NULL;
                break;
              }
              case 5: {
                const char *payload = "ABCDEFG";
                testObject1->load(*Tester::createPacket(2, 0, 0, false, 96, 1, 1024, 5, NULL, NULL, 0, payload));

                auto packet = testObject1->getPacket();
                TESTING_CHECK(packet)

                TESTING_CHECK(NULL != packet->ptr())
                TESTING_EQUAL(12 + strlen(payload), packet->size())

                TESTING_EQUAL(2, packet->version())
                TESTING_EQUAL(0, packet->padding())
                TESTING_EQUAL(0, packet->cc())
                TESTING_CHECK(!packet->m())
                TESTING_EQUAL(96, packet->pt())
                TESTING_EQUAL(1, packet->sequenceNumber())
                TESTING_EQUAL(1024, packet->timestamp())
                TESTING_EQUAL(5, packet->ssrc())

                RTPPacket::HeaderExtension temp[3];

                temp[0].mID = 0x04;
                temp[0].mDataSizeInBytes = 0;
                temp[0].mNext = &(temp[1]);

                temp[1].mID = 0x05;
                temp[1].mData = new BYTE[1] {0x55};
                temp[1].mDataSizeInBytes = 1;
                temp[1].mPostPaddingSize = 1;
                temp[1].mNext = &(temp[2]);

                temp[2].mID = 0x06;
                temp[2].mData = new BYTE[4] {0x41, 0x42, 0x43, 0x44};
                temp[2].mDataSizeInBytes = 4;
                temp[2].mNext = NULL;

                packet->mHeaderExtensionAppBits = 2;
                packet->mHeaderExtensionPrepaddedSize = 1;
                packet->changeHeaderExtensions(&temp[0]);

                TESTING_EQUAL(3, packet->totalHeaderExtensions())
                TESTING_EQUAL(sizeof(DWORD) + (sizeof(DWORD)*4), packet->headerExtensionSize())

                {
                  RTPPacket::HeaderExtension *current = packet->firstHeaderExtension();
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[0].mID, current->mID)
                  TESTING_EQUAL(temp[0].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[0].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(1), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(1);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[1].mID, current->mID)
                  TESTING_EQUAL(temp[1].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[1].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_CHECK(current->mData)
                  TESTING_EQUAL(0x55, current->mData[0])
                  TESTING_EQUAL(packet->getHeaderExtensionAtIndex(2), current->mNext)
                }

                {
                  RTPPacket::HeaderExtension *current = packet->getHeaderExtensionAtIndex(2);
                  TESTING_CHECK(current)
                  TESTING_EQUAL(temp[2].mID, current->mID)
                  TESTING_EQUAL(temp[2].mDataSizeInBytes, current->mDataSizeInBytes)
                  TESTING_EQUAL(temp[2].mPostPaddingSize, current->mPostPaddingSize)
                  TESTING_CHECK(current->mData)
                  TESTING_EQUAL(0x41, current->mData[0])
                  TESTING_EQUAL(0x42, current->mData[1])
                  TESTING_EQUAL(0x43, current->mData[2])
                  TESTING_EQUAL(0x44, current->mData[3])
                  TESTING_EQUAL(NULL, current->mNext)
                }

                testObject1->compareExtensionHeader(&gHeader4[0], sizeof(gHeader4));

                delete [] (temp[1].mData);
                delete [] (temp[2].mData);
                
                auto tempPacket = Tester::createPacket(2, 0, 0, false, 96, 1, 1024, 5, NULL, &gHeader4[0], sizeof(gHeader4), payload);
                
                testObject1->compare(*tempPacket);
                break;
              }
              case 6: {
                const char *payload = "FOOBAR";
                testObject1->load(*Tester::createPacket(2, 0, 0, false, 96, 1, 1024, 5, NULL, &gHeader5[0], sizeof(gHeader5), payload));

                auto packet = testObject1->getPacket();
                TESTING_CHECK(packet)

                TESTING_CHECK(NULL != packet->ptr())
                TESTING_EQUAL(12 + sizeof(gHeader5) + strlen(payload), packet->size())

                TESTING_EQUAL(2, packet->version())
                TESTING_EQUAL(0, packet->padding())
                TESTING_EQUAL(0, packet->cc())
                TESTING_CHECK(!packet->m())
                TESTING_EQUAL(96, packet->pt())
                TESTING_EQUAL(1, packet->sequenceNumber())
                TESTING_EQUAL(1024, packet->timestamp())
                TESTING_EQUAL(5, packet->ssrc())

                TESTING_EQUAL(5, packet->headerExtensionAppBits())
                TESTING_EQUAL(0, packet->headerExtensionPrepaddedSize())
                TESTING_EQUAL(sizeof(DWORD), packet->headerExtensionSize())
                TESTING_EQUAL(0, packet->totalHeaderExtensions())

                packet->changeHeaderExtensions(NULL);

                TESTING_EQUAL(0, packet->totalHeaderExtensions())
                TESTING_EQUAL(sizeof(DWORD), packet->headerExtensionSize())
                TESTING_CHECK(NULL == packet->headerExtensionParseStopped())
                TESTING_EQUAL(0, packet->headerExtensionParseStoppedSize())
                TESTING_EQUAL(NULL, packet->firstHeaderExtension())

                testObject1->compareExtensionHeader(&gHeader5[0], sizeof(gHeader5));

                auto tempPacket = Tester::createPacket(2, 0, 0, false, 96, 1, 1024, 5, NULL, &gHeader5[0], sizeof(gHeader5), payload);
                
                testObject1->compare(*tempPacket);
                break;
              }
              case 7: {
                reachedFinalStep = true;
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
          found += (testObject1 ? 1 : 0);
          found += (testObject2 ? 1 : 0);
        }

        if (lastFound != found) {
          lastFound = found;
          TESTING_STDOUT() << "FOUND:        [" << found << "].\n";
        }
      }

      TESTING_EQUAL(found, expecting)

      TESTING_SLEEP(2000)

      testObject1.reset();
      testObject2.reset();

      ++testNumber;
    } while (true);
  }

  TESTING_STDOUT() << "WAITING:      All RTP packet tests have finished. Waiting for 'bogus' events to process (1 second wait).\n";
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
