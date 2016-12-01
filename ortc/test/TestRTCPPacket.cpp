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


#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_Helper.h>

#include <ortc/services/IHelper.h>

#include <zsLib/ISettings.h>
#include <zsLib/IMessageQueueThread.h>
#include <zsLib/XML.h>

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
//ZS_DECLARE_USING_PTR(ortc::services, SecureByteBlock)
//
ZS_DECLARE_TYPEDEF_PTR(zsLib::ISettings, UseSettings)
ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper)
ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper)


namespace ortc
{
  namespace test
  {
    namespace rtcppacket
    {
      //-------------------------------------------------------------------------
      static size_t boundarySize(
                                 size_t size,
                                 size_t alignment = sizeof(DWORD)
                                 )
      {
        size_t modulus = size % alignment;
        if (0 == modulus) return size;
        return size + (alignment - modulus);
      }

      //-----------------------------------------------------------------------
      static size_t random(size_t min, size_t max)
      {
        ZS_THROW_INVALID_ARGUMENT_IF(min > max)
        if (min == max) return min;

        size_t result = 0;
        BYTE *pos = reinterpret_cast<BYTE *>(&result);
        for (size_t index = 0; index < sizeof(result); ++index) {
          *pos = static_cast<BYTE>(rand()%0xFF);
        }
        size_t range = (max - min)+1;

        result = min + (result % range);
        return result;
      }

      //-----------------------------------------------------------------------
      static void randomizeBuffer(BYTE *pos, size_t size)
      {
        for (; size > 0; --size, ++pos)
        {
          *pos = static_cast<BYTE>(random(0, 0xFF));
        }
      }

      //-----------------------------------------------------------------------
      static void randomizeBufferSequencial(BYTE *pos, size_t size)
      {
        BYTE current = static_cast<BYTE>(random(0, 0xFF));
        for (; size > 0; --size, ++pos)
        {
          *pos = current;
          ++current;
        }
      }

      //-----------------------------------------------------------------------
      static size_t randomSize(size_t minSize, size_t maxSize)
      {
        if (maxSize < minSize) maxSize = minSize;
        if (minSize > maxSize) minSize = maxSize;
        return random(minSize, maxSize);
      }

      //-----------------------------------------------------------------------
      static size_t randomSize(size_t maxSize)
      {
        return random(0, maxSize);
      }
      
      //-----------------------------------------------------------------------
      static DWORD randomDWORD(size_t maxBits = 0)
      {
        DWORD result = 0;
        randomizeBuffer(reinterpret_cast<BYTE *>(&result), sizeof(result));

        if ((maxBits * 8) == sizeof(result)) {
          maxBits = 0;
        }

        if (0 != maxBits) {
          DWORD maxValue = ((1 << static_cast<DWORD>(maxBits)) - 1);
          result = result & maxValue;
        }
        return result;
      }

      //-----------------------------------------------------------------------
      static WORD randomWORD(size_t maxBits = 0)
      {
        WORD result = 0;
        randomizeBuffer(reinterpret_cast<BYTE *>(&result), sizeof(result));

        if ((maxBits * 8) == sizeof(result)) {
          maxBits = 0;
        }

        if (0 != maxBits) {
          WORD maxValue = ((1 << static_cast<WORD>(maxBits)) - 1);
          result = result & maxValue;
        }
        return result;
      }

      //-----------------------------------------------------------------------
      static BYTE randomBYTE(size_t maxBits = 0)
      {
        BYTE result = 0;
        randomizeBuffer(reinterpret_cast<BYTE *>(&result), sizeof(result));

        if ((maxBits * 8) == sizeof(result)) {
          maxBits = 0;
        }

        if (0 != maxBits) {
          BYTE maxValue = ((1 << static_cast<BYTE>(maxBits)) - 1);
          result = result & maxValue;
        }
        return result;
      }

      //-----------------------------------------------------------------------
      static String randomString(size_t length)
      {
        static const char *pool = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890abcdefghijklmnopqrstuvwxyzAbCdEfGhIjKlMnOpQrStUvWxYzAaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwZzYyZz0987654321";
        static const size_t poolLen = strlen(pool);

        if (0 == length) return String();

        char *buf = new char[length+1] {};
        char *pos = buf;

        size_t remaining = length;

        size_t startPos = randomSize(poolLen-1);
        size_t poolRemaining = poolLen - startPos;
        const char *poolPos = &(pool[startPos]);

        while (0 != remaining) {
          size_t consume = (poolRemaining > remaining ? remaining : poolRemaining);
          memcpy(pos, poolPos, consume);

          pos += consume;
          remaining -= consume;

          poolPos = pool;
          poolRemaining = poolLen;
        }

        String result(buf);

        delete [] buf;
        buf = NULL;

        return result;
      }

      //-----------------------------------------------------------------------
      bool shouldPerform(size_t percentageProbability)
      {
        if (0 == percentageProbability) return false;
        auto result = random(0, 100);
        if (result <= percentageProbability) return true;
        return false;
      }

      //-----------------------------------------------------------------------
      void checkEqual(
                      const BYTE *buffer1,
                      size_t size1,
                      const BYTE *buffer2,
                      size_t size2
                      )
      {
        TESTING_EQUAL(size1, size2)
        if (0 == size1) {
          TESTING_CHECK(NULL == buffer1)
          TESTING_CHECK(NULL == buffer2)
          return;
        }

        TESTING_CHECK(NULL != buffer1)
        TESTING_CHECK(NULL != buffer2)

        SecureByteBlock b1(buffer1, size1);
        SecureByteBlock b2(buffer2, size2);

        TESTING_EQUAL(0, UseServicesHelper::compare(b1, b2))
      }

      //-----------------------------------------------------------------------
      void checkMaxBits(DWORD value, size_t maxBits)
      {
        if ((sizeof(DWORD)*8) == maxBits) return;

        DWORD max = (1 << static_cast<DWORD>(maxBits))-1;
        TESTING_CHECK(value <= max)
      }

      ZS_DECLARE_CLASS_PTR(Tester)
      ZS_DECLARE_USING_PTR(ortc::internal, RTCPPacket)

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Tester
      #pragma mark

      class Tester : public SharedRecursiveLock
      {
      public:
        typedef zsLib::Log Log;
        typedef zsLib::XML::Element Element;

        typedef RTCPPacket::Report Report;
        typedef RTCPPacket::SenderReceiverCommonReport SenderReceiverCommonReport;
        typedef RTCPPacket::SenderReport SenderReport;
        typedef RTCPPacket::ReceiverReport ReceiverReport;
        typedef RTCPPacket::SDES SDES;
        typedef RTCPPacket::Bye Bye;
        typedef RTCPPacket::App App;
        typedef RTCPPacket::FeedbackMessage FeedbackMessage;
        typedef RTCPPacket::TransportLayerFeedbackMessage TransportLayerFeedbackMessage;
        typedef RTCPPacket::PayloadSpecificFeedbackMessage PayloadSpecificFeedbackMessage;
        typedef RTCPPacket::XR XR;
        typedef RTCPPacket::UnknownReport UnknownReport;

        typedef RTCPPacket::TransportLayerFeedbackMessage::GenericNACK GenericNACK;
        typedef RTCPPacket::TransportLayerFeedbackMessage::TMMBRCommon TMMBRCommon;
        typedef RTCPPacket::TransportLayerFeedbackMessage::TMMBR TMMBR;
        typedef RTCPPacket::TransportLayerFeedbackMessage::TMMBN TMMBN;

        typedef RTCPPacket::PayloadSpecificFeedbackMessage::PLI PLI;
        typedef RTCPPacket::PayloadSpecificFeedbackMessage::SLI SLI;
        typedef RTCPPacket::PayloadSpecificFeedbackMessage::RPSI RPSI;
        typedef RTCPPacket::PayloadSpecificFeedbackMessage::CodecControlCommon CodecControlCommon;
        typedef RTCPPacket::PayloadSpecificFeedbackMessage::FIR FIR;
        typedef RTCPPacket::PayloadSpecificFeedbackMessage::TSTR TSTR;
        typedef RTCPPacket::PayloadSpecificFeedbackMessage::TSTN TSTN;
        typedef RTCPPacket::PayloadSpecificFeedbackMessage::VBCM VBCM;
        typedef RTCPPacket::PayloadSpecificFeedbackMessage::AFB AFB;
        typedef RTCPPacket::PayloadSpecificFeedbackMessage::REMB REMB;

        typedef RTCPPacket::XR::RLEChunk RLEChunk;
        typedef RTCPPacket::XR::ReportBlock ReportBlock;
        typedef RTCPPacket::XR::ReportBlockRange ReportBlockRange;
        typedef RTCPPacket::XR::RLEReportBlock RLEReportBlock;
        typedef RTCPPacket::XR::LossRLEReportBlock LossRLEReportBlock;
        typedef RTCPPacket::XR::DuplicateRLEReportBlock DuplicateRLEReportBlock;
        typedef RTCPPacket::XR::PacketReceiptTimesReportBlock PacketReceiptTimesReportBlock;
        typedef RTCPPacket::XR::ReceiverReferenceTimeReportBlock ReceiverReferenceTimeReportBlock;
        typedef RTCPPacket::XR::DLRRReportBlock DLRRReportBlock;
        typedef RTCPPacket::XR::StatisticsSummaryReportBlock StatisticsSummaryReportBlock;
        typedef RTCPPacket::XR::VoIPMetricsReportBlock VoIPMetricsReportBlock;
        typedef RTCPPacket::XR::UnknownReportBlock UnknownReportBlock;
        typedef RTCPPacket::XR::RunLength RunLength;
        typedef RTCPPacket::XR::BitVector BitVector;

        class CheckRTCPPcaket : public RTCPPacket
        {
        public:
          static size_t checkOutputSize(const Report *first)
          {
            return RTCPPacket::getPacketSize(first);
          }
        };


      public:
        //---------------------------------------------------------------------
        Tester() :
          SharedRecursiveLock(SharedRecursiveLock::create())
        {
        }

        //---------------------------------------------------------------------
        ~Tester()
        {
          if (NULL != mGeneratedFirst) {
            clean(mGeneratedFirst);
            mGeneratedFirst = NULL;
          }
        }

        //---------------------------------------------------------------------
        void init()
        {
        }

        //-------------------------------------------------------------------------
        static Log::Params slog(const char *message)
        {
          ElementPtr objectEl = Element::create("ortc::RTCPPacket");
          return Log::Params(message, objectEl);
        }
        
        //-------------------------------------------------------------------------
        Log::Params log(const char *message)
        {
          ElementPtr objectEl = Element::create("ortc::RTCPPacket");
          UseServicesHelper::debugAppend(objectEl, "id", mID);
          return Log::Params(message, objectEl);
        }


        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Tester => (clean routines)
        #pragma mark

        //---------------------------------------------------------------------
        static void cleanSenderReceiverCommonReport(SenderReceiverCommonReport *common)
        {
          if (NULL != common->mExtension) {
            BYTE *buffer = const_cast<BYTE *>(common->mExtension);
            delete [] buffer;
            common->mExtension = NULL;
          }
          if (NULL != common->mFirstReportBlock) {
            delete [] common->mFirstReportBlock;
            common->mFirstReportBlock = NULL;
          }
        }

        //---------------------------------------------------------------------
        static void cleanSenderReport(SenderReport *report)
        {
          cleanSenderReceiverCommonReport(report);
          delete report;
        }

        //---------------------------------------------------------------------
        static void cleanReceiverReport(ReceiverReport *report)
        {
          cleanSenderReceiverCommonReport(report);
          delete report;
        }

        //---------------------------------------------------------------------
        static void cleanSDES(SDES::Chunk::StringItem *first, size_t length)
        {
          for (size_t index = 0; index < length; ++index)
          {
            auto current = (&(first[index]));
            if (NULL != current->mValue) {
              char *value = const_cast<char *>(current->mValue);
              delete [] value;
              current->mValue = NULL;
            }
          }
        }

        //---------------------------------------------------------------------
        static void cleanSDESPriv(SDES::Chunk::Priv *first, size_t length)
        {
          for (size_t index = 0; index < length; ++index)
          {
            auto current = (&(first[index]));
            if (NULL != current->mValue) {
              char *value = const_cast<char *>(current->mValue);
              delete [] value;
              current->mValue = NULL;
            }

            if (NULL != current->mPrefix) {
              char *value = const_cast<char *>(current->mPrefix);
              delete [] value;
              current->mPrefix = NULL;
            }
          }
        }
        
        //---------------------------------------------------------------------
        static void cleanSDES(SDES::Chunk *chunk)
        {
          if (NULL != chunk->mFirstCName) {
            cleanSDES(chunk->mFirstCName, chunk->mCNameCount);
            delete [] chunk->mFirstCName;
            chunk->mFirstCName = NULL;
          }
          if (NULL != chunk->mFirstName) {
            cleanSDES(chunk->mFirstName, chunk->mNameCount);
            delete [] chunk->mFirstName;
            chunk->mFirstName = NULL;
          }
          if (NULL != chunk->mFirstEmail) {
            cleanSDES(chunk->mFirstEmail, chunk->mEmailCount);
            delete [] chunk->mFirstEmail;
            chunk->mFirstEmail = NULL;
          }
          if (NULL != chunk->mFirstPhone) {
            cleanSDES(chunk->mFirstPhone, chunk->mPhoneCount);
            delete [] chunk->mFirstPhone;
            chunk->mFirstPhone = NULL;
          }
          if (NULL != chunk->mFirstLoc) {
            cleanSDES(chunk->mFirstLoc, chunk->mLocCount);
            delete [] chunk->mFirstLoc;
            chunk->mFirstLoc = NULL;
          }
          if (NULL != chunk->mFirstTool) {
            cleanSDES(chunk->mFirstTool, chunk->mToolCount);
            delete [] chunk->mFirstTool;
            chunk->mFirstTool = NULL;
          }
          if (NULL != chunk->mFirstNote) {
            cleanSDES(chunk->mFirstNote, chunk->mNoteCount);
            delete [] chunk->mFirstNote;
            chunk->mFirstNote = NULL;
          }
          if (NULL != chunk->mFirstPriv) {
            cleanSDESPriv(chunk->mFirstPriv, chunk->mPrivCount);
            delete [] chunk->mFirstPriv;
            chunk->mFirstPriv = NULL;
          }
          if (NULL != chunk->mFirstMid) {
            cleanSDES(chunk->mFirstMid, chunk->mMidCount);
            delete [] chunk->mFirstMid;
            chunk->mFirstMid = NULL;
          }
          if (NULL != chunk->mFirstUnknown) {
            cleanSDES(chunk->mFirstUnknown, chunk->mUnknownCount);
            delete [] chunk->mFirstUnknown;
            chunk->mFirstUnknown = NULL;
          }
        }

        //---------------------------------------------------------------------
        static void cleanSDES(SDES *report)
        {
          if (NULL == report->mFirstChunk) return;

          typedef SDES::Chunk Chunk;

          for (Chunk *current = report->mFirstChunk; NULL != current; current = current->next())
          {
            cleanSDES(current);
          }

          delete [] report->mFirstChunk;
          report->mFirstChunk = NULL;

          delete report;
        }

        //---------------------------------------------------------------------
        static void cleanBye(Bye *report)
        {
          if (NULL != report->mSSRCs) {
            delete [] report->mSSRCs;
            report->mSSRCs = NULL;
          }
          if (NULL != report->mReasonForLeaving) {
            char *reason = const_cast<char *>(report->mReasonForLeaving);
            delete [] reason;
            report->mReasonForLeaving = NULL;
          }
          delete report;
        }

        //---------------------------------------------------------------------
        static void cleanApp(App *report)
        {
          if (NULL != report->mData) {
            delete [] report->mData;
            report->mData = NULL;
          }
          delete report;
        }

        //---------------------------------------------------------------------
        static void cleanTransportLayerFeedbackMessage(TransportLayerFeedbackMessage *report)
        {
          if (NULL != report->mFirstGenericNACK) {
            delete [] report->mFirstGenericNACK;
            report->mFirstGenericNACK = NULL;
          }
          if (NULL != report->mFirstTMMBR) {
            delete [] report->mFirstTMMBR;
            report->mFirstTMMBR = NULL;
          }
          if (NULL != report->mFirstTMMBN) {
            delete [] report->mFirstTMMBN;
            report->mFirstTMMBN = NULL;
          }
          if (NULL != report->mFCI) {
            delete [] report->mFCI;
            report->mFCI = NULL;
          }
          delete report;
        }

        //---------------------------------------------------------------------
        static void cleanPayloadSpecificFeedbackMessage(PayloadSpecificFeedbackMessage *report)
        {
          if (NULL != report->mFirstSLI) {
            delete [] report->mFirstSLI;
            report->mFirstSLI = NULL;
          }
          if (NULL != report->mRPSI.mNativeRPSIBitString) {
            BYTE *buffer = const_cast<BYTE *>(report->mRPSI.mNativeRPSIBitString);
            delete [] buffer;
            report->mRPSI.mNativeRPSIBitString = NULL;
          }
          if (NULL != report->mFirstFIR) {
            delete [] report->mFirstFIR;
            report->mFirstFIR = NULL;
          }
          if (NULL != report->mFirstTSTR) {
            delete [] report->mFirstTSTR;
            report->mFirstTSTR = NULL;
          }
          if (NULL != report->mFirstTSTN) {
            delete [] report->mFirstTSTN;
            report->mFirstTSTN = NULL;
          }
          if (NULL != report->mFirstVBCM) {
            for (size_t index = 0; index < report->vbcmCount(); ++index)
            {
              auto item = report->vbcmAtIndex(index);
              if (NULL != item->mVBCMOctetString) {
                BYTE *buffer = const_cast<BYTE *>(item->mVBCMOctetString);
                delete [] buffer;
                item->mVBCMOctetString = NULL;
              }
            }

            delete [] report->mFirstVBCM;
            report->mFirstVBCM = NULL;
          }
          if (NULL != report->mAFB.mData) {
            BYTE *buffer = const_cast<BYTE *>(report->mAFB.mData);
            delete [] buffer;
            report->mAFB.mData = NULL;
          }
          if (NULL != report->mREMB.mSSRCs) {
            delete [] report->mREMB.mSSRCs;
            report->mREMB.mSSRCs = NULL;
          }
          delete report;
        }

        //---------------------------------------------------------------------
        static void cleanXR(XR *report)
        {
          if (NULL != report->mFirstLossRLEReportBlock) {
            for (size_t index = 0; index < report->lossRLEReportBlockCount(); ++index) {
              auto item = report->lossRLEReportBlockAtIndex(index);

              if (NULL != item->mChunks) {
                delete [] item->mChunks;
                item->mChunks = NULL;
              }
            }
            delete [] report->mFirstLossRLEReportBlock;
            report->mFirstLossRLEReportBlock = NULL;
          }
          if (NULL != report->mFirstDuplicateRLEReportBlock) {
            for (size_t index = 0; index < report->duplicateRLEReportBlockCount(); ++index) {
              auto item = report->duplicateRLEReportBlockAtIndex(index);

              if (NULL != item->mChunks) {
                delete [] item->mChunks;
                item->mChunks = NULL;
              }
            }

            delete [] report->mFirstDuplicateRLEReportBlock;
            report->mFirstDuplicateRLEReportBlock = NULL;
          }
          if (NULL != report->mFirstPacketReceiptTimesReportBlock) {
            for (size_t index = 0; index < report->packetReceiptTimesReportBlockCount(); ++index) {
              auto item = report->packetReceiptTimesReportBlockAtIndex(index);

              if (NULL != item->mReceiptTimes) {
                delete [] item->mReceiptTimes;
                item->mReceiptTimes = NULL;
              }
            }
            delete [] report->mFirstPacketReceiptTimesReportBlock;
            report->mFirstPacketReceiptTimesReportBlock = NULL;
          }
          if (NULL != report->mFirstReceiverReferenceTimeReportBlock) {
            delete [] report->mFirstReceiverReferenceTimeReportBlock;
            report->mFirstReceiverReferenceTimeReportBlock = NULL;
          }
          if (NULL != report->mFirstDLRRReportBlock) {
            for (size_t index = 0; index < report->dlrrReportBlockCount(); ++index) {
              auto item = report->dlrrReportBlockAtIndex(index);

              if (NULL != item->mSubBlocks) {
                delete [] item->mSubBlocks;
                item->mSubBlocks = NULL;
              }
            }
            delete [] report->mFirstDLRRReportBlock;
            report->mFirstDLRRReportBlock = NULL;
          }
          if (NULL != report->mFirstStatisticsSummaryReportBlock) {
            delete [] report->mFirstStatisticsSummaryReportBlock;
            report->mFirstStatisticsSummaryReportBlock = NULL;
          }
          if (NULL != report->mFirstVoIPMetricsReportBlock) {
            delete [] report->mFirstVoIPMetricsReportBlock;
            report->mFirstVoIPMetricsReportBlock = NULL;
          }
          if (NULL != report->mFirstUnknownReportBlock) {
            for (size_t index = 0; index < report->unknownReportBlockCount(); ++index) {
              auto item = report->unknownReportBlockAtIndex(index);

              if (NULL != item->mTypeSpecificContents) {
                delete [] item->mTypeSpecificContents;
                item->mTypeSpecificContents = NULL;
              }
            }
            delete [] report->mFirstUnknownReportBlock;
            report->mFirstUnknownReportBlock = NULL;
          }
          delete report;
        }

        //---------------------------------------------------------------------
        static void cleanUnknownReport(UnknownReport *report)
        {
          delete report;
        }

        //---------------------------------------------------------------------
        static void clean(Report *report)
        {
          Report *next = NULL;
          for (Report *current = report; NULL != current; current = next)
          {
            next = current->next();

            if (NULL != current->mPtr) {
              BYTE *buffer = const_cast<BYTE *>(current->mPtr);
              delete [] buffer;
              current->mPtr = NULL;
            }

            switch (current->pt()) {
              case SenderReport::kPayloadType:                    cleanSenderReport(reinterpret_cast<SenderReport *>(current)); break;
              case ReceiverReport::kPayloadType:                  cleanReceiverReport(reinterpret_cast<ReceiverReport *>(current)); break;
              case SDES::kPayloadType:                            cleanSDES(reinterpret_cast<SDES *>(current)); break;
              case Bye::kPayloadType:                             cleanBye(reinterpret_cast<Bye *>(current)); break;
              case App::kPayloadType:                             cleanApp(reinterpret_cast<App *>(current)); break;
              case TransportLayerFeedbackMessage::kPayloadType:   cleanTransportLayerFeedbackMessage(reinterpret_cast<TransportLayerFeedbackMessage *>(current)); break;
              case PayloadSpecificFeedbackMessage::kPayloadType:  cleanPayloadSpecificFeedbackMessage(reinterpret_cast<PayloadSpecificFeedbackMessage *>(current)); break;
              case XR::kPayloadType:                              cleanXR(reinterpret_cast<XR *>(current)); break;
              default:
              {
                cleanUnknownReport(reinterpret_cast<UnknownReport *>(current));
                break;
              }
            }
          }
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Tester => (compare routines)
        #pragma mark

        //---------------------------------------------------------------------
        static void compareReport(Report *report1, Report *report2)
        {
          TESTING_EQUAL(report1->version(), report2->version())
          TESTING_EQUAL(boundarySize(report1->padding()), boundarySize(report2->padding())) // padding is aligned to boundary size
          TESTING_EQUAL(report1->reportSpecific(), report2->reportSpecific())
          TESTING_EQUAL(report1->pt(), report2->pt())

          TESTING_EQUAL(report1->version(), report1->mVersion)
          TESTING_EQUAL(report1->padding(), report1->mPadding)
          TESTING_EQUAL(report1->reportSpecific(), report1->mReportSpecific)
          TESTING_EQUAL(report1->pt(), report1->mPT)
        }

        //---------------------------------------------------------------------
        static void compareSenderReceiverCommonReport(SenderReceiverCommonReport *report1, SenderReceiverCommonReport *report2)
        {
          typedef SenderReceiverCommonReport::ReportBlock CommonReportBlock;

          TESTING_EQUAL(report1->rc(), report2->rc())
          TESTING_EQUAL(report1->ssrcOfSender(), report2->ssrcOfSender())
          TESTING_EQUAL(boundarySize(report1->extensionSize()), boundarySize(report2->extensionSize())) // padding can cause a slight difference in size

          TESTING_EQUAL(report1->ssrcOfSender(), report1->mSSRCOfSender)
          TESTING_EQUAL(report1->extensionSize(), report1->mExtensionSize)

          // padding can cause a slight difference in size
          auto size1 = report1->extensionSize();
          auto size2 = report2->extensionSize();
          if (size1 < size2) size2 = size1;
          if (size2 < size1) size1 = size2;

          checkEqual(report1->extension(), size1, report2->extension(), size2);

          if (0 == report1->rc()) {
            TESTING_CHECK(NULL == report1->mFirstReportBlock)
            TESTING_CHECK(NULL == report2->mFirstReportBlock)
          }

          for (size_t index = 0; index < report1->rc(); ++index)
          {
            TESTING_CHECK(NULL != report1->mFirstReportBlock)
            TESTING_CHECK(NULL != report2->mFirstReportBlock)

            CommonReportBlock *block1 = &(report1->mFirstReportBlock[index]);
            CommonReportBlock *block2 = &(report2->mFirstReportBlock[index]);

            TESTING_EQUAL(block1->ssrc(), block2->ssrc())
            TESTING_EQUAL(block1->fractionLost(), block2->fractionLost())
            TESTING_EQUAL(block1->cumulativeNumberOfPacketsLost(), block2->cumulativeNumberOfPacketsLost())
            checkMaxBits(block1->cumulativeNumberOfPacketsLost(), 24);
            TESTING_EQUAL(block1->extendedHighestSequenceNumberReceived(), block2->extendedHighestSequenceNumberReceived())
            TESTING_EQUAL(block1->interarrivalJitter(), block2->interarrivalJitter())
            TESTING_EQUAL(block1->lsr(), block2->lsr())
            TESTING_EQUAL(block1->dlsr(), block2->dlsr())

            TESTING_EQUAL(block1->ssrc(), block1->mSSRC)
            TESTING_EQUAL(block1->fractionLost(), block1->mFractionLost)
            TESTING_EQUAL(block1->cumulativeNumberOfPacketsLost(), block1->mCumulativeNumberOfPacketsLost)
            TESTING_EQUAL(block1->extendedHighestSequenceNumberReceived(), block1->mExtendedHighestSequenceNumberReceived)
            TESTING_EQUAL(block1->interarrivalJitter(), block1->mInterarrivalJitter)
            TESTING_EQUAL(block1->lsr(), block1->mLSR)
            TESTING_EQUAL(block1->dlsr(), block1->mDLSR)
          }
        }

        //---------------------------------------------------------------------
        static void compareSenderReport(SenderReport *report1, SenderReport *report2)
        {
          TESTING_EQUAL(report1->pt(), SenderReport::kPayloadType)

          compareSenderReceiverCommonReport(report1, report2);
          TESTING_EQUAL(report1->ntpTimestampMS(), report2->ntpTimestampMS())
          TESTING_EQUAL(report1->ntpTimestampLS(), report2->ntpTimestampLS())
          TESTING_CHECK(report1->ntpTimestamp() == report2->ntpTimestamp())
          TESTING_EQUAL(report1->rtpTimestamp(), report2->rtpTimestamp())
          TESTING_EQUAL(report1->senderPacketCount(), report2->senderPacketCount())
          TESTING_EQUAL(report1->senderOctetCount(), report2->senderOctetCount())

          TESTING_EQUAL(report1->ntpTimestampMS(), report1->mNTPTimestampMS)
          TESTING_EQUAL(report1->ntpTimestampLS(), report1->mNTPTimestampLS)
          TESTING_EQUAL(report1->rtpTimestamp(), report1->mRTPTimestamp)
          TESTING_EQUAL(report1->senderPacketCount(), report1->mSenderPacketCount)
          TESTING_EQUAL(report1->senderOctetCount(), report1->mSenderOctetCount)
        }

        //---------------------------------------------------------------------
        static void compareReceiverReport(ReceiverReport *report1, ReceiverReport *report2)
        {
          TESTING_EQUAL(report1->pt(), ReceiverReport::kPayloadType)

          compareSenderReceiverCommonReport(report1, report2);
          TESTING_EQUAL(report1->ssrcOfPacketSender(), report2->ssrcOfPacketSender())
          TESTING_EQUAL(report1->ssrcOfPacketSender(), report1->mSSRCOfSender)
        }

        //---------------------------------------------------------------------
        static void compareSDES(SDES::Chunk::StringItem *item1, SDES::Chunk::StringItem *item2)
        {
          TESTING_CHECK(NULL != item1)
          TESTING_CHECK(NULL != item2)

          TESTING_EQUAL(item1->length(), item2->length())
          TESTING_EQUAL(item1->length(), item1->mLength)

          TESTING_CHECK(item1->value() == item1->mValue)
          TESTING_CHECK(item2->value() == item2->mValue)

          if (0 != item1->length()) {
            TESTING_CHECK(NULL != item1->value())
            TESTING_CHECK(NULL != item2->value())

            TESTING_EQUAL(strlen(item1->value()), strlen(item2->value()))
            TESTING_EQUAL(strlen(item1->value()), item1->length())
          } else {
            TESTING_CHECK(NULL == item1->value())
            TESTING_CHECK(NULL == item2->value())
          }
        }

        //---------------------------------------------------------------------
        static void compareSDES(SDES::Chunk::StringItem *first1, SDES::Chunk::StringItem *first2, size_t count, BYTE type)
        {
          if (NULL == first1) {
            TESTING_EQUAL(0, count)
            TESTING_CHECK(NULL == first1)
            TESTING_CHECK(NULL == first2)
            return;
          }
          TESTING_CHECK(0 != count)
          TESTING_CHECK(NULL != first1)
          TESTING_CHECK(NULL != first2)

          TESTING_EQUAL(first1->type(), first2->type())
          TESTING_EQUAL(first1->mType, first1->type())
          if (0 != type) {
            TESTING_EQUAL(first1->type(), type)
          } else {
            TESTING_CHECK(0 != first1->type())
          }

          if (0 == count) {
            TESTING_CHECK(NULL == first1)
            TESTING_CHECK(NULL == first2)
            return;
          }

          TESTING_CHECK(NULL != first1)
          TESTING_CHECK(NULL != first2)

          for (size_t index = 0; index < count; ++index)
          {
            auto item1 = &(first1[index]);
            auto item2 = &(first2[index]);

            compareSDES(item1, item2);
          }
        }

        //---------------------------------------------------------------------
        static void compareSDESPriv(SDES::Chunk::Priv *item1, SDES::Chunk::Priv *item2)
        {
          TESTING_CHECK(NULL != item1)
          TESTING_CHECK(NULL != item2)

          TESTING_EQUAL(item1->prefixLength(), item2->prefixLength())
          TESTING_EQUAL(item1->prefixLength(), item1->mPrefixLength)

          TESTING_CHECK(item1->prefix() == item1->mPrefix)
          TESTING_CHECK(item2->prefix() == item2->mPrefix)

          if (0 != item1->prefixLength()) {
            TESTING_CHECK(NULL != item1->prefix())
            TESTING_CHECK(NULL != item2->prefix())

            TESTING_EQUAL(strlen(item1->prefix()), strlen(item2->prefix()))
            TESTING_EQUAL(strlen(item1->prefix()), item1->prefixLength())
          } else {
            TESTING_CHECK(NULL == item1->prefix())
            TESTING_CHECK(NULL == item2->prefix())
          }
        }

        //---------------------------------------------------------------------
        static void compareSDESPriv(SDES::Chunk::Priv *first1, SDES::Chunk::Priv *first2, size_t count)
        {
          if (0 == count) {
            TESTING_CHECK(NULL == first1)
            TESTING_CHECK(NULL == first2)
            return;
          }

          TESTING_CHECK(NULL != first1)
          TESTING_CHECK(NULL != first2)

          for (size_t index = 0; index < count; ++index)
          {
            auto item1 = &(first1[index]);
            auto item2 = &(first2[index]);

            compareSDESPriv(item1, item2);
          }
        }
        
        //---------------------------------------------------------------------
        static void compareSDES(SDES *report1, SDES *report2)
        {
          typedef SDES::Chunk Chunk;

          TESTING_EQUAL(report1->pt(), SDES::kPayloadType)

          TESTING_EQUAL(report1->sc(), report2->sc())
          TESTING_EQUAL(report1->sc(), report1->mReportSpecific)

          checkMaxBits(report1->mReportSpecific, 5);

          size_t count = 0;

          Chunk *chunk1 = report1->firstChunk();
          Chunk *chunk2 = report2->firstChunk();

          if (0 != report1->sc()) {
            TESTING_CHECK(NULL != chunk1)
            TESTING_CHECK(NULL != chunk2)
          }

          Chunk *next1 = NULL;
          Chunk *next2 = NULL;
          for (; (NULL != chunk1) && (NULL != chunk2); chunk1 = next1, chunk2 = next2, ++count)
          {
            TESTING_CHECK(count < report1->sc())

            next1 = chunk1->next();
            next2 = chunk2->next();

            TESTING_EQUAL(chunk1->ssrc(), chunk2->ssrc())
            TESTING_EQUAL(chunk1->ssrc(), chunk1->mSSRC)

            TESTING_EQUAL(chunk1->count(), chunk2->count())
            TESTING_EQUAL(chunk1->count(), chunk1->mCount)

            TESTING_EQUAL(chunk1->cNameCount(), chunk2->cNameCount())
            TESTING_EQUAL(chunk1->nameCount(), chunk2->nameCount())
            TESTING_EQUAL(chunk1->emailCount(), chunk2->emailCount())
            TESTING_EQUAL(chunk1->phoneCount(), chunk2->phoneCount())
            TESTING_EQUAL(chunk1->locCount(), chunk2->locCount())
            TESTING_EQUAL(chunk1->toolCount(), chunk2->toolCount())
            TESTING_EQUAL(chunk1->noteCount(), chunk2->noteCount())
            TESTING_EQUAL(chunk1->privCount(), chunk2->privCount())
            TESTING_EQUAL(chunk1->midCount(), chunk2->midCount())
            TESTING_EQUAL(chunk1->unknownCount(), chunk2->unknownCount())

            TESTING_EQUAL(chunk1->cNameCount(), chunk1->mCNameCount)
            TESTING_EQUAL(chunk1->nameCount(), chunk1->mNameCount)
            TESTING_EQUAL(chunk1->emailCount(), chunk1->mEmailCount)
            TESTING_EQUAL(chunk1->phoneCount(), chunk1->mPhoneCount)
            TESTING_EQUAL(chunk1->locCount(), chunk1->mLocCount)
            TESTING_EQUAL(chunk1->toolCount(), chunk1->mToolCount)
            TESTING_EQUAL(chunk1->noteCount(), chunk1->mNoteCount)
            TESTING_EQUAL(chunk1->privCount(), chunk1->mPrivCount)
            TESTING_EQUAL(chunk1->midCount(), chunk1->mMidCount)
            TESTING_EQUAL(chunk1->unknownCount(), chunk1->mUnknownCount)

            size_t total = chunk1->mCNameCount +
                           chunk1->mNameCount +
                           chunk1->mEmailCount +
                           chunk1->mPhoneCount +
                           chunk1->mLocCount +
                           chunk1->mToolCount +
                           chunk1->mNoteCount +
                           chunk1->mPrivCount +
                           chunk1->mMidCount +
                           chunk1->mUnknownCount;

            TESTING_EQUAL(chunk1->count(), total)

            if (0 != chunk1->cNameCount()) {
              TESTING_CHECK(NULL != chunk1->firstCName())
              TESTING_CHECK(NULL != chunk2->firstCName())
              for (size_t index = 0; index < chunk1->cNameCount(); ++index) {
                TESTING_CHECK(chunk1->cNameAtIndex(index) == &(chunk1->mFirstCName[index]))
              }
            } else {
              TESTING_CHECK(NULL == chunk1->firstCName())
              TESTING_CHECK(NULL == chunk2->firstCName())
            }
            if (0 != chunk1->nameCount()) {
              TESTING_CHECK(NULL != chunk1->firstName())
              TESTING_CHECK(NULL != chunk2->firstName())
              for (size_t index = 0; index < chunk1->nameCount(); ++index) {
                TESTING_CHECK(chunk1->nameAtIndex(index) == &(chunk1->mFirstName[index]))
              }
            } else {
              TESTING_CHECK(NULL == chunk1->firstName())
              TESTING_CHECK(NULL == chunk2->firstName())
            }
            if (0 != chunk1->emailCount()) {
              TESTING_CHECK(NULL != chunk1->firstEmail())
              TESTING_CHECK(NULL != chunk2->firstEmail())
              for (size_t index = 0; index < chunk1->emailCount(); ++index) {
                TESTING_CHECK(chunk1->emailAtIndex(index) == &(chunk1->mFirstEmail[index]))
              }
            } else {
              TESTING_CHECK(NULL == chunk1->firstEmail())
              TESTING_CHECK(NULL == chunk2->firstEmail())
            }
            if (0 != chunk1->phoneCount()) {
              TESTING_CHECK(NULL != chunk1->firstPhone())
              TESTING_CHECK(NULL != chunk2->firstPhone())
              for (size_t index = 0; index < chunk1->phoneCount(); ++index) {
                TESTING_CHECK(chunk1->phoneAtIndex(index) == &(chunk1->mFirstPhone[index]))
              }
            } else {
              TESTING_CHECK(NULL == chunk1->firstPhone())
              TESTING_CHECK(NULL == chunk2->firstPhone())
            }
            if (0 != chunk1->locCount()) {
              TESTING_CHECK(NULL != chunk1->firstLoc())
              TESTING_CHECK(NULL != chunk2->firstLoc())
              for (size_t index = 0; index < chunk1->locCount(); ++index) {
                TESTING_CHECK(chunk1->locAtIndex(index) == &(chunk1->mFirstLoc[index]))
              }
            } else {
              TESTING_CHECK(NULL == chunk1->firstLoc())
              TESTING_CHECK(NULL == chunk2->firstLoc())
            }
            if (0 != chunk1->toolCount()) {
              TESTING_CHECK(NULL != chunk1->firstTool())
              TESTING_CHECK(NULL != chunk2->firstTool())
              for (size_t index = 0; index < chunk1->toolCount(); ++index) {
                TESTING_CHECK(chunk1->toolAtIndex(index) == &(chunk1->mFirstTool[index]))
              }
            } else {
              TESTING_CHECK(NULL == chunk1->firstTool())
              TESTING_CHECK(NULL == chunk2->firstTool())
            }
            if (0 != chunk1->noteCount()) {
              TESTING_CHECK(NULL != chunk1->firstNote())
              TESTING_CHECK(NULL != chunk2->firstNote())
              for (size_t index = 0; index < chunk1->noteCount(); ++index) {
                TESTING_CHECK(chunk1->noteAtIndex(index) == &(chunk1->mFirstNote[index]))
              }
            } else {
              TESTING_CHECK(NULL == chunk1->firstNote())
              TESTING_CHECK(NULL == chunk2->firstNote())
            }
            if (0 != chunk1->privCount()) {
              TESTING_CHECK(NULL != chunk1->firstPriv())
              TESTING_CHECK(NULL != chunk2->firstPriv())
              for (size_t index = 0; index < chunk1->privCount(); ++index) {
                TESTING_CHECK(chunk1->privAtIndex(index) == &(chunk1->mFirstPriv[index]))
              }
            } else {
              TESTING_CHECK(NULL == chunk1->firstPriv())
              TESTING_CHECK(NULL == chunk2->firstPriv())
            }
            if (0 != chunk1->midCount()) {
              TESTING_CHECK(NULL != chunk1->firstMid())
              TESTING_CHECK(NULL != chunk2->firstMid())
              for (size_t index = 0; index < chunk1->midCount(); ++index) {
                TESTING_CHECK(chunk1->midAtIndex(index) == &(chunk1->mFirstMid[index]))
              }
            } else {
              TESTING_CHECK(NULL == chunk1->firstMid())
              TESTING_CHECK(NULL == chunk2->firstMid())
            }
            if (0 != chunk1->unknownCount()) {
              TESTING_CHECK(NULL != chunk1->firstUnknown())
              TESTING_CHECK(NULL != chunk2->firstUnknown())
              for (size_t index = 0; index < chunk1->unknownCount(); ++index) {
                TESTING_CHECK(chunk1->unknownAtIndex(index) == &(chunk1->mFirstUnknown[index]))
              }
            } else {
              TESTING_CHECK(NULL == chunk1->firstUnknown())
              TESTING_CHECK(NULL == chunk2->firstUnknown())
            }

            TESTING_CHECK(chunk1->firstCName() == chunk1->mFirstCName)
            TESTING_CHECK(chunk1->firstName() == chunk1->mFirstName)
            TESTING_CHECK(chunk1->firstEmail() == chunk1->mFirstEmail)
            TESTING_CHECK(chunk1->firstPhone() == chunk1->mFirstPhone)
            TESTING_CHECK(chunk1->firstLoc() == chunk1->mFirstLoc)
            TESTING_CHECK(chunk1->firstTool() == chunk1->mFirstTool)
            TESTING_CHECK(chunk1->firstNote() == chunk1->mFirstNote)
            TESTING_CHECK(chunk1->firstPriv() == chunk1->mFirstPriv)
            TESTING_CHECK(chunk1->firstMid() == chunk1->mFirstMid)
            TESTING_CHECK(chunk1->firstUnknown() == chunk1->mFirstUnknown)

            TESTING_CHECK(chunk2->firstCName() == chunk2->mFirstCName)
            TESTING_CHECK(chunk2->firstName() == chunk2->mFirstName)
            TESTING_CHECK(chunk2->firstEmail() == chunk2->mFirstEmail)
            TESTING_CHECK(chunk2->firstPhone() == chunk2->mFirstPhone)
            TESTING_CHECK(chunk2->firstLoc() == chunk2->mFirstLoc)
            TESTING_CHECK(chunk2->firstTool() == chunk2->mFirstTool)
            TESTING_CHECK(chunk2->firstNote() == chunk2->mFirstNote)
            TESTING_CHECK(chunk2->firstPriv() == chunk2->mFirstPriv)
            TESTING_CHECK(chunk2->firstMid() == chunk2->mFirstMid)
            TESTING_CHECK(chunk2->firstUnknown() == chunk2->mFirstUnknown)

            compareSDES(chunk1->mFirstCName, chunk2->mFirstCName, chunk1->mCNameCount, Chunk::CName::kItemType);
            compareSDES(chunk1->mFirstName, chunk2->mFirstName, chunk1->mNameCount, Chunk::Name::kItemType);
            compareSDES(chunk1->mFirstEmail, chunk2->mFirstEmail, chunk1->mEmailCount, Chunk::Email::kItemType);
            compareSDES(chunk1->mFirstPhone, chunk2->mFirstPhone, chunk1->mPhoneCount, Chunk::Phone::kItemType);
            compareSDES(chunk1->mFirstLoc, chunk2->mFirstLoc, chunk1->mLocCount, Chunk::Loc::kItemType);
            compareSDES(chunk1->mFirstTool, chunk2->mFirstTool, chunk1->mToolCount, Chunk::Tool::kItemType);
            compareSDES(chunk1->mFirstNote, chunk2->mFirstNote, chunk1->mNoteCount, Chunk::Note::kItemType);
            compareSDESPriv(chunk1->mFirstPriv, chunk2->mFirstPriv, chunk1->mPrivCount);
            compareSDES(chunk1->mFirstMid, chunk2->mFirstMid, chunk1->mMidCount, Chunk::Mid::kItemType);
            compareSDES(chunk1->mFirstUnknown, chunk2->mFirstUnknown, chunk1->mUnknownCount, 0);

            compareSDESPriv(chunk1->mFirstPriv, chunk2->mFirstPriv, chunk1->mPrivCount);
          }

          TESTING_CHECK(NULL == chunk1)
          TESTING_CHECK(NULL == chunk2)

          TESTING_EQUAL(count, report1->sc())
        }

        //---------------------------------------------------------------------
        static void compareBye(Bye *report1, Bye *report2)
        {
          TESTING_EQUAL(report1->pt(), Bye::kPayloadType)

          TESTING_EQUAL(report1->sc(), report2->sc())
          TESTING_EQUAL(report1->sc(), report1->mReportSpecific)

          checkMaxBits(report1->mReportSpecific, 5);

          if (0 != report1->sc()) {
            TESTING_CHECK(NULL != report1->mSSRCs)
            TESTING_CHECK(NULL != report2->mSSRCs)

            for (size_t index = 0; index < report1->sc(); ++index)
            {
              TESTING_EQUAL(report1->ssrc(index), report2->ssrc(index))
              TESTING_EQUAL(report1->ssrc(index), report1->mSSRCs[index])
              TESTING_EQUAL(report2->ssrc(index), report2->mSSRCs[index])
            }
          } else {
            TESTING_CHECK(NULL == report1->mSSRCs)
            TESTING_CHECK(NULL == report2->mSSRCs)
          }

          TESTING_CHECK(report1->reasonForLeaving() == report1->mReasonForLeaving)

          if (NULL != report1->reasonForLeaving()) {
            TESTING_CHECK(NULL != report1->reasonForLeaving())
            TESTING_CHECK(NULL != report2->reasonForLeaving())
            TESTING_EQUAL(strlen(report1->reasonForLeaving()), strlen(report2->reasonForLeaving()))
            TESTING_CHECK(0 == strcmp(report1->reasonForLeaving(), report2->reasonForLeaving()))
          } else {
            TESTING_CHECK(NULL == report1->reasonForLeaving())
            TESTING_CHECK(NULL == report2->reasonForLeaving())
          }
        }

        //---------------------------------------------------------------------
        static void compareApp(App *report1, App *report2)
        {
          TESTING_EQUAL(report1->pt(), App::kPayloadType)

          TESTING_EQUAL(report1->subtype(), report2->subtype())
          TESTING_EQUAL(report1->ssrc(), report2->ssrc())
          TESTING_EQUAL(boundarySize(report1->dataSize()), boundarySize(report2->dataSize()))
          TESTING_EQUAL(report1->mName[0], report2->mName[0])
          TESTING_EQUAL(report1->mName[1], report2->mName[1])
          TESTING_EQUAL(report1->mName[2], report2->mName[2])
          TESTING_EQUAL(report1->mName[3], report2->mName[3])
          TESTING_EQUAL(report1->mName[4], report2->mName[4])
          TESTING_EQUAL(report1->mName[4], 0)

          checkMaxBits(report1->subtype(), 5);

          TESTING_EQUAL(report1->subtype(), report1->mReportSpecific)
          TESTING_EQUAL(report1->ssrc(), report1->mSSRC)
          TESTING_EQUAL(report1->dataSize(), report1->mDataSize)
          TESTING_EQUAL(report1->name(), &(report1->mName[0]))

          auto size1 = report1->mDataSize;
          auto size2 = report2->mDataSize;

          // make sure boundary padded app data doesn't cause a failure in the comparison
          if (size1 < size2) size2 = size1;
          if (size2 < size1) size1 = size2;

          checkEqual(report1->mData, size1, report2->mData, size2);
        }

        //---------------------------------------------------------------------
        static void compareTransportLayerFeedbackMessageTMMBRCommon(TransportLayerFeedbackMessage::TMMBRCommon *common1, TransportLayerFeedbackMessage::TMMBRCommon *common2)
        {
          TESTING_EQUAL(common1->ssrc(), common2->ssrc())
          TESTING_EQUAL(common1->mxTBRExp(), common2->mxTBRExp())
          TESTING_EQUAL(common1->mxTBRMantissa(), common2->mxTBRMantissa())
          TESTING_EQUAL(common1->measuredOverhead(), common2->measuredOverhead())

          TESTING_EQUAL(common1->ssrc(), common1->mSSRC)
          TESTING_EQUAL(common1->mxTBRExp(), common1->mMxTBRExp)
          TESTING_EQUAL(common1->mxTBRMantissa(), common1->mMxTBRMantissa)
          TESTING_EQUAL(common1->measuredOverhead(), common1->measuredOverhead())

          checkMaxBits(common1->mMxTBRExp, 6);
          checkMaxBits(common1->mMxTBRMantissa, 17);
          checkMaxBits(common1->mMeasuredOverhead, 9);
        }

        //---------------------------------------------------------------------
        static void compareTransportLayerFeedbackMessageTMMBRCommon(TransportLayerFeedbackMessage::TMMBRCommon *first1, TransportLayerFeedbackMessage::TMMBRCommon *first2, size_t count)
        {
          typedef TransportLayerFeedbackMessage::TMMBRCommon TMMBRCommon;

          if (0 == count) {
            TESTING_CHECK(NULL == first1)
            TESTING_CHECK(NULL == first2)
            return;
          }
          TESTING_CHECK(NULL != first1)
          TESTING_CHECK(NULL != first2)

          for (size_t index = 0; index < count; ++index) {
            TMMBRCommon *common1 = &(first1[index]);
            TMMBRCommon *common2 = &(first2[index]);
            compareTransportLayerFeedbackMessageTMMBRCommon(common1, common2);
          }
        }

        //---------------------------------------------------------------------
        static void compareTransportLayerFeedbackMessageGenericNACK(TransportLayerFeedbackMessage::GenericNACK *item1, TransportLayerFeedbackMessage::GenericNACK *item2)
        {
          TESTING_EQUAL(item1->pid(), item2->pid())
          TESTING_EQUAL(item1->blp(), item2->blp())

          TESTING_EQUAL(item1->pid(), item1->mPID)
          TESTING_EQUAL(item1->blp(), item1->mBLP)
        }

        //---------------------------------------------------------------------
        static void compareTransportLayerFeedbackMessageGenericNACK(TransportLayerFeedbackMessage::GenericNACK *first1, TransportLayerFeedbackMessage::GenericNACK *first2, size_t count)
        {
          typedef TransportLayerFeedbackMessage::GenericNACK GenericNACK;

          if (0 == count) {
            TESTING_CHECK(NULL == first1)
            TESTING_CHECK(NULL == first2)
            return;
          }
          TESTING_CHECK(NULL != first1)
          TESTING_CHECK(NULL != first2)

          for (size_t index = 0; index < count; ++index) {
            auto *item1 = &(first1[index]);
            auto *item2 = &(first2[index]);
            compareTransportLayerFeedbackMessageGenericNACK(item1, item2);
          }
        }

        //---------------------------------------------------------------------
        static void compareTransportLayerFeedbackMessage(TransportLayerFeedbackMessage *report1, TransportLayerFeedbackMessage *report2)
        {
          TESTING_EQUAL(report1->pt(), TransportLayerFeedbackMessage::kPayloadType)

          TESTING_EQUAL(report1->fmt(), report2->fmt())

          TESTING_EQUAL(report1->ssrcOfPacketSender(), report2->ssrcOfPacketSender())
          TESTING_EQUAL(report1->ssrcOfMediaSource(), report2->ssrcOfMediaSource())

          TESTING_EQUAL(report1->fmt(), report1->mReportSpecific)
          checkMaxBits(report1->mReportSpecific, 5);

          TESTING_CHECK(report1->mFCI == report1->fci())
          TESTING_EQUAL(report1->mFCISize, report1->fciSize())
          TESTING_CHECK(report2->mFCI == report2->fci())
          TESTING_EQUAL(report2->mFCISize, report2->fciSize())

          switch (report1->fmt()) {
            case GenericNACK::kFmt: {
              TESTING_EQUAL(report1->genericNACKCount(), report2->genericNACKCount())
              TESTING_EQUAL(report1->genericNACKCount(), report1->mGenericNACKCount)
              compareTransportLayerFeedbackMessageGenericNACK(report1->mFirstGenericNACK, report2->mFirstGenericNACK, report1->genericNACKCount());
              for (size_t index = 0; index < report1->genericNACKCount(); ++index) {
                TESTING_CHECK(&(report1->mFirstGenericNACK[index]) == report1->genericNACKAtIndex(index))
                TESTING_CHECK(&(report2->mFirstGenericNACK[index]) == report2->genericNACKAtIndex(index))
              }
              break;
            }
            case TMMBR::kFmt:       {
              TESTING_EQUAL(report1->tmmbrCount(), report2->tmmbrCount())
              TESTING_EQUAL(report1->tmmbrCount(), report1->mTMMBRCount)

              compareTransportLayerFeedbackMessageTMMBRCommon(report1->mFirstTMMBR, report2->mFirstTMMBR, report1->tmmbrCount());
              for (size_t index = 0; index < report1->tmmbrCount(); ++index) {
                TESTING_CHECK(&(report1->mFirstTMMBR[index]) == report1->tmmbrAtIndex(index))
                TESTING_CHECK(&(report2->mFirstTMMBR[index]) == report2->tmmbrAtIndex(index))
              }
              break;
            }
            case TMMBN::kFmt:       {
              TESTING_EQUAL(report1->tmmbnCount(), report2->tmmbnCount())
              TESTING_EQUAL(report1->tmmbnCount(), report1->mTMMBNCount)

              compareTransportLayerFeedbackMessageTMMBRCommon(report1->mFirstTMMBN, report2->mFirstTMMBN, report1->tmmbnCount());
              for (size_t index = 0; index < report1->tmmbnCount(); ++index) {
                TESTING_CHECK(&(report1->mFirstTMMBN[index]) == report1->tmmbnAtIndex(index))
                TESTING_CHECK(&(report2->mFirstTMMBN[index]) == report2->tmmbnAtIndex(index))
              }
              break;
            }
            default:
            {
              TESTING_CHECK(NULL != report1->mUnknown)
              TESTING_CHECK(NULL != report2->mUnknown)

              TESTING_CHECK(report1->unknown() == report1->mUnknown)
              TESTING_CHECK(report2->unknown() == report2->mUnknown)

              // padding can cause FCI size to be padded to next DWORD boundary
              TESTING_EQUAL(boundarySize(report1->mFCISize), boundarySize(report2->mFCISize))

              auto size1 = report1->mFCISize;
              auto size2 = report2->mFCISize;

              if (size1 < size2) size2 = size1;
              if (size2 < size1) size1 = size2;

              checkEqual(report1->mFCI, size1, report2->mFCI, size2);
            }
          }
        }

        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessageSLI(PayloadSpecificFeedbackMessage::SLI *item1, PayloadSpecificFeedbackMessage::SLI *item2)
        {
          TESTING_EQUAL(item1->first(), item2->first())
          TESTING_EQUAL(item1->number(), item2->number())
          TESTING_EQUAL(item1->pictureID(), item2->pictureID())

          TESTING_EQUAL(item1->first(), item1->mFirst)
          TESTING_EQUAL(item1->number(), item1->mNumber)
          TESTING_EQUAL(item1->pictureID(), item1->mPictureID)

          checkMaxBits(item1->mFirst, 13);
          checkMaxBits(item1->mNumber, 13);
          checkMaxBits(item1->mPictureID, 6);
        }

        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessageSLI(PayloadSpecificFeedbackMessage::SLI *first1, PayloadSpecificFeedbackMessage::SLI *first2, size_t count)
        {
          if (0 == count) {
            TESTING_CHECK(NULL == first1)
            TESTING_CHECK(NULL == first2)
            return;
          }
          TESTING_CHECK(NULL != first1)
          TESTING_CHECK(NULL != first2)

          for (size_t index = 0; index < count; ++index) {
            auto *item1 = &(first1[index]);
            auto *item2 = &(first2[index]);
            comparePayloadSpecificFeedbackMessageSLI(item1, item2);
          }
        }

        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessageCodecControlCommon(PayloadSpecificFeedbackMessage::CodecControlCommon *item1, PayloadSpecificFeedbackMessage::CodecControlCommon *item2)
        {
          TESTING_EQUAL(item1->ssrc(), item2->ssrc())
          TESTING_EQUAL(item1->seqNr(), item2->seqNr())
          TESTING_EQUAL(item1->reserved(), item2->reserved())
          TESTING_EQUAL(item1->mControlSpecific, item2->mControlSpecific)

          TESTING_EQUAL(item1->ssrc(), item1->mSSRC)
          TESTING_EQUAL(item1->seqNr(), item1->mSeqNr)
          TESTING_EQUAL(item1->reserved(), item1->mReserved)

          checkMaxBits(item1->mReserved, 24);
        }
        
        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessageFIR(PayloadSpecificFeedbackMessage::FIR *first1, PayloadSpecificFeedbackMessage::FIR *first2, size_t count)
        {
          if (0 == count) {
            TESTING_CHECK(NULL == first1)
            TESTING_CHECK(NULL == first2)
            return;
          }
          TESTING_CHECK(NULL != first1)
          TESTING_CHECK(NULL != first2)

          for (size_t index = 0; index < count; ++index) {
            auto *item1 = &(first1[index]);
            auto *item2 = &(first2[index]);
            comparePayloadSpecificFeedbackMessageCodecControlCommon(item1, item2);
          }
        }

        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessageTSTR(PayloadSpecificFeedbackMessage::TSTR *first1, PayloadSpecificFeedbackMessage::TSTR *first2, size_t count)
        {
          if (0 == count) {
            TESTING_CHECK(NULL == first1)
            TESTING_CHECK(NULL == first2)
            return;
          }
          TESTING_CHECK(NULL != first1)
          TESTING_CHECK(NULL != first2)

          for (size_t index = 0; index < count; ++index) {
            auto *item1 = &(first1[index]);
            auto *item2 = &(first2[index]);
            comparePayloadSpecificFeedbackMessageCodecControlCommon(item1, item2);

            TESTING_EQUAL(item1->index(), item2->index());
            TESTING_EQUAL(item1->index(), item1->mControlSpecific);

            checkMaxBits(item1->mReserved, 19);
            checkMaxBits(item1->index(), 5);
          }
        }
        
        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessageTSTN(PayloadSpecificFeedbackMessage::TSTN *first1, PayloadSpecificFeedbackMessage::TSTN *first2, size_t count)
        {
          if (0 == count) {
            TESTING_CHECK(NULL == first1)
            TESTING_CHECK(NULL == first2)
            return;
          }
          TESTING_CHECK(NULL != first1)
          TESTING_CHECK(NULL != first2)

          for (size_t index = 0; index < count; ++index) {
            auto *item1 = &(first1[index]);
            auto *item2 = &(first2[index]);
            comparePayloadSpecificFeedbackMessageCodecControlCommon(item1, item2);

            TESTING_EQUAL(item1->index(), item2->index());
            TESTING_EQUAL(item1->index(), item1->mControlSpecific);

            checkMaxBits(item1->mReserved, 19);
            checkMaxBits(item1->index(), 5);
          }
        }
        
        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessageVBCM(PayloadSpecificFeedbackMessage::VBCM *item1, PayloadSpecificFeedbackMessage::VBCM *item2)
        {
          comparePayloadSpecificFeedbackMessageCodecControlCommon(item1, item2);

          TESTING_EQUAL(item1->zeroBit(), item2->zeroBit())
          TESTING_EQUAL(item1->payloadType(), item2->payloadType())
          TESTING_EQUAL(item1->vbcmOctetStringSize(), item2->vbcmOctetStringSize())

          checkMaxBits(item1->zeroBit(), 1);
          checkMaxBits(item1->payloadType(), 7);

          checkEqual(item1->vbcmOctetString(), item1->vbcmOctetStringSize(), item2->vbcmOctetString(), item2->vbcmOctetStringSize());
        }
        
        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessageVBCM(PayloadSpecificFeedbackMessage::VBCM *first1, PayloadSpecificFeedbackMessage::VBCM *first2, size_t count)
        {
          if (0 == count) {
            TESTING_CHECK(NULL == first1)
            TESTING_CHECK(NULL == first2)
            return;
          }
          TESTING_CHECK(NULL != first1)
          TESTING_CHECK(NULL != first2)

          for (size_t index = 0; index < count; ++index) {
            auto *item1 = &(first1[index]);
            auto *item2 = &(first2[index]);
            comparePayloadSpecificFeedbackMessageVBCM(item1, item2);
          }
        }
        
        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessage(PayloadSpecificFeedbackMessage *report1, PayloadSpecificFeedbackMessage *report2)
        {
          TESTING_EQUAL(report1->pt(), PayloadSpecificFeedbackMessage::kPayloadType)
          TESTING_EQUAL(report1->fmt(), report2->fmt())

          TESTING_EQUAL(report1->ssrcOfPacketSender(), report2->ssrcOfPacketSender())
          TESTING_EQUAL(report1->ssrcOfMediaSource(), report2->ssrcOfMediaSource())

          TESTING_EQUAL(report1->fmt(), report1->mReportSpecific)
          checkMaxBits(report1->mReportSpecific, 5);

          switch (report1->fmt()) {
            case PLI::kFmt:       {
              TESTING_CHECK(NULL != report1->pli())
              TESTING_CHECK(NULL != report2->pli())
              break;
            }
            case SLI::kFmt:       {
              TESTING_EQUAL(report1->sliCount(), report2->sliCount())
              TESTING_EQUAL(report1->sliCount(), report1->mSLICount)
              comparePayloadSpecificFeedbackMessageSLI(report1->mFirstSLI, report2->mFirstSLI, report1->sliCount());
              for (size_t index = 0; index < report1->sliCount(); ++index) {
                TESTING_CHECK(&(report1->mFirstSLI[index]) == report1->sliAtIndex(index))
                TESTING_CHECK(&(report2->mFirstSLI[index]) == report2->sliAtIndex(index))
              }
              break;
            }
            case RPSI::kFmt:      {
              TESTING_CHECK(NULL != report1->rpsi())
              TESTING_CHECK(NULL != report2->rpsi())

              auto item1 = report1->rpsi();
              auto item2 = report1->rpsi();

              TESTING_EQUAL(item1->zeroBit(), item2->zeroBit())
              TESTING_EQUAL(item1->payloadType(), item2->payloadType())
              TESTING_EQUAL(item1->nativeRPSIBitStringSizeInBits(), item2->nativeRPSIBitStringSizeInBits())

              TESTING_EQUAL(item1->zeroBit(), item1->mZeroBit)
              TESTING_EQUAL(item1->payloadType(), item1->mPayloadType)
              TESTING_EQUAL(item1->nativeRPSIBitStringSizeInBits(), item1->mNativeRPSIBitStringSizeInBits)

              TESTING_CHECK(item1->nativeRPSIBitString() == item1->mNativeRPSIBitString)
              TESTING_CHECK(item2->nativeRPSIBitString() == item2->mNativeRPSIBitString)

              checkMaxBits(item1->mPayloadType, 7);
              checkMaxBits(item1->mZeroBit, 1);

              if (0 != item1->nativeRPSIBitStringSizeInBits()) {
                if (0 != (item1->nativeRPSIBitStringSizeInBits()/8)) {
                  checkEqual(item1->nativeRPSIBitString(), item1->nativeRPSIBitStringSizeInBits()/8, item2->nativeRPSIBitString(), item2->nativeRPSIBitStringSizeInBits()/8);
                }

                auto modulas = item1->nativeRPSIBitStringSizeInBits() % 8;
                if (0 != modulas) {
                  BYTE by1 = item1->nativeRPSIBitString()[item1->nativeRPSIBitStringSizeInBits()/8];
                  BYTE by2 = item1->nativeRPSIBitString()[item1->nativeRPSIBitStringSizeInBits()/8];

                  by1 = (by1 >> static_cast<BYTE>(8-modulas)) & ((1 << static_cast<BYTE>(modulas))-1);
                  by2 = (by2 >> static_cast<BYTE>(8-modulas)) & ((1 << static_cast<BYTE>(modulas))-1);
                  TESTING_EQUAL(by1, by2)
                }
              }
              break;
            }
            case FIR::kFmt:       {
              TESTING_EQUAL(report1->firCount(), report2->firCount())
              TESTING_EQUAL(report1->firCount(), report1->mFIRCount)
              comparePayloadSpecificFeedbackMessageFIR(report1->mFirstFIR, report2->mFirstFIR, report1->firCount());
              for (size_t index = 0; index < report1->firCount(); ++index) {
                TESTING_CHECK(&(report1->mFirstFIR[index]) == report1->firAtIndex(index))
                TESTING_CHECK(&(report2->mFirstFIR[index]) == report2->firAtIndex(index))
              }
              break;
            }
            case TSTR::kFmt:      {
              TESTING_EQUAL(report1->tstrCount(), report2->tstrCount())
              TESTING_EQUAL(report1->tstrCount(), report1->mTSTRCount)
              comparePayloadSpecificFeedbackMessageTSTR(report1->mFirstTSTR, report2->mFirstTSTR, report1->tstrCount());
              for (size_t index = 0; index < report1->tstrCount(); ++index) {
                TESTING_CHECK(&(report1->mFirstTSTR[index]) == report1->tstrAtIndex(index))
                TESTING_CHECK(&(report2->mFirstTSTR[index]) == report2->tstrAtIndex(index))
              }
              break;
            }
            case TSTN::kFmt:      {
              TESTING_EQUAL(report1->tstnCount(), report2->tstnCount())
              TESTING_EQUAL(report1->tstnCount(), report1->mTSTNCount)
              comparePayloadSpecificFeedbackMessageTSTN(report1->mFirstTSTN, report2->mFirstTSTN, report1->tstnCount());
              for (size_t index = 0; index < report1->tstnCount(); ++index) {
                TESTING_CHECK(&(report1->mFirstTSTN[index]) == report1->tstnAtIndex(index))
                TESTING_CHECK(&(report2->mFirstTSTN[index]) == report2->tstnAtIndex(index))
              }
              break;
            }
            case VBCM::kFmt:      {
              TESTING_EQUAL(report1->vbcmCount(), report2->vbcmCount())
              TESTING_EQUAL(report1->vbcmCount(), report1->mVBCMCount)
              comparePayloadSpecificFeedbackMessageVBCM(report1->mFirstVBCM, report2->mFirstVBCM, report1->vbcmCount());
              for (size_t index = 0; index < report1->vbcmCount(); ++index) {
                TESTING_CHECK(&(report1->mFirstVBCM[index]) == report1->vbcmAtIndex(index))
                TESTING_CHECK(&(report2->mFirstVBCM[index]) == report2->vbcmAtIndex(index))
              }
              break;
            }
            case AFB::kFmt:       {
              auto remb1 = report1->remb();
              auto remb2 = report2->remb();

              if (NULL == remb1) {
                TESTING_CHECK(NULL == remb1)
                TESTING_CHECK(NULL == remb2)

                TESTING_CHECK(NULL != report1->afb())
                TESTING_CHECK(NULL != report2->afb())

                auto item1 = report1->afb();
                auto item2 = report1->afb();

                checkEqual(item1->data(), item1->dataSize(), item2->data(), item2->dataSize());
              } else {
                TESTING_CHECK(NULL != remb1)
                TESTING_CHECK(NULL != remb2)

                TESTING_CHECK(NULL == report1->afb())
                TESTING_CHECK(NULL == report2->afb())

                TESTING_EQUAL(remb1->numSSRC(), remb2->numSSRC())
                TESTING_EQUAL(remb1->brExp(), remb2->brExp())
                TESTING_EQUAL(remb1->brMantissa(), remb2->brMantissa())

                TESTING_EQUAL(remb1->numSSRC(), remb1->mNumSSRC)
                TESTING_EQUAL(remb1->brExp(), remb1->mBRExp)
                TESTING_EQUAL(remb1->brMantissa(), remb1->mBRMantissa)

                checkMaxBits(remb1->mBRExp, 6);
                checkMaxBits(remb1->mBRMantissa, 18);

                if (0 == remb1->mNumSSRC) {
                  TESTING_CHECK(NULL == remb1->mSSRCs)
                  TESTING_CHECK(NULL == remb2->mSSRCs)
                }

                for (size_t index = 0; index < remb1->mNumSSRC; ++index)
                {
                  TESTING_EQUAL(remb1->ssrcAtIndex(index), remb2->ssrcAtIndex(index))
                  TESTING_EQUAL(remb1->ssrcAtIndex(index), remb1->mSSRCs[index])
                }
              }

              break;
            }
            default:
            {
              TESTING_CHECK(NULL != report1->mUnknown)
              TESTING_CHECK(NULL != report2->mUnknown)

              TESTING_CHECK(report1->unknown() == report1->mUnknown)
              TESTING_CHECK(report2->unknown() == report2->mUnknown)
            }
          }
        }

        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessageReportBlockRange(ReportBlockRange *item1, ReportBlockRange *item2)
        {
          TESTING_EQUAL(item1->reserved(), item2->reserved())
          TESTING_EQUAL(item1->thinning(), item2->thinning())
          TESTING_EQUAL(item1->ssrcOfSource(), item2->ssrcOfSource())
          TESTING_EQUAL(item1->beginSeq(), item2->beginSeq())
          TESTING_EQUAL(item1->endSeq(), item2->endSeq())

          TESTING_EQUAL(item1->reserved(), ((item1->mTypeSpecific >> 4) & 0xF))
          TESTING_EQUAL(item1->thinning(), ((item1->mTypeSpecific >> 0) & 0xF))
          TESTING_EQUAL(item1->ssrcOfSource(), item1->mSSRCOfSource)
          TESTING_EQUAL(item1->beginSeq(), item1->mBeginSeq)
          TESTING_EQUAL(item1->endSeq(), item1->mEndSeq)
        }

        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessageRLEReportBlock(RLEReportBlock *item1, RLEReportBlock *item2)
        {
          comparePayloadSpecificFeedbackMessageReportBlockRange(item1, item2);

          TESTING_EQUAL(item1->chunkCount(), item2->chunkCount())

          TESTING_EQUAL(item1->chunkCount(), item1->mChunkCount)

          if (0 != item1->chunkCount()) {
            TESTING_CHECK(NULL != item1->mChunks)
            TESTING_CHECK(NULL != item2->mChunks)
            for (size_t index = 0; index < item1->chunkCount(); ++index) {
              TESTING_EQUAL(item1->chunkAtIndex(index), item2->chunkAtIndex(index))
              TESTING_EQUAL(item1->mChunks[index], item1->chunkAtIndex(index))
              TESTING_EQUAL(item2->mChunks[index], item2->chunkAtIndex(index))

              auto chunk1 = item1->chunkAtIndex(index);
              auto chunk2 = item2->chunkAtIndex(index);
              if (XR::isRunLengthChunk(chunk1)) {
                TESTING_CHECK(XR::isRunLengthChunk(chunk2))
                RunLength rle1(chunk1);
                RunLength rle2(chunk2);
                TESTING_EQUAL(rle1.mRunType, rle2.mRunType)
                TESTING_EQUAL(rle1.mRunLength, rle2.mRunLength)

                TESTING_EQUAL(0, ((chunk1 >> 15) & 0x01))
                TESTING_EQUAL(rle1.mRunType, ((chunk1 >> 14) & 0x01))

              } else {
                TESTING_CHECK(XR::isBitVectorChunk(chunk1))
                TESTING_CHECK(XR::isBitVectorChunk(chunk2))

                BitVector bv1(chunk1);
                BitVector bv2(chunk2);

                TESTING_EQUAL(bv1.mBitVector, bv2.mBitVector)

                TESTING_EQUAL(1, ((chunk1 >> 15) & 0x01))
                TESTING_EQUAL(bv1.mBitVector, chunk1 & 0x7FFF)
              }
            }
          } else {
            TESTING_CHECK(NULL == item1->mChunks)
            TESTING_CHECK(NULL == item2->mChunks)
          }
        }

        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessagePacketReceiptTimesReportBlock(PacketReceiptTimesReportBlock *item1, PacketReceiptTimesReportBlock *item2)
        {
          comparePayloadSpecificFeedbackMessageReportBlockRange(item1, item2);

          TESTING_EQUAL(item1->receiptTimeCount(), item2->receiptTimeCount())

          TESTING_EQUAL(item1->receiptTimeCount(), item1->mReceiptTimeCount)

          if (0 != item1->receiptTimeCount()) {
            TESTING_CHECK(NULL != item1->mReceiptTimes)
            TESTING_CHECK(NULL != item2->mReceiptTimes)
            for (size_t index = 0; index < item1->receiptTimeCount(); ++index) {
              TESTING_EQUAL(item1->receiptTimeAtIndex(index), item2->receiptTimeAtIndex(index))
              TESTING_EQUAL(item1->mReceiptTimes[index], item1->receiptTimeAtIndex(index))
              TESTING_EQUAL(item2->mReceiptTimes[index], item2->receiptTimeAtIndex(index))
            }
          } else {
            TESTING_CHECK(NULL == item1->mReceiptTimes)
            TESTING_CHECK(NULL == item2->mReceiptTimes)
          }
        }

        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessageReceiverReferenceTimeReportBlock(ReceiverReferenceTimeReportBlock *item1, ReceiverReferenceTimeReportBlock *item2)
        {
          TESTING_EQUAL(item1->ntpTimestampMS(), item2->ntpTimestampMS())
          TESTING_EQUAL(item1->ntpTimestampLS(), item2->ntpTimestampLS())
          TESTING_CHECK(item1->ntpTimestamp() == item2->ntpTimestamp())

          TESTING_EQUAL(item1->ntpTimestampMS(), item1->mNTPTimestampMS)
          TESTING_EQUAL(item1->ntpTimestampLS(), item1->mNTPTimestampLS)
        }
        
        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessagePacketDLRRReportBlock(DLRRReportBlock *item1, DLRRReportBlock *item2)
        {
          TESTING_EQUAL(item1->subBlockCount(), item2->subBlockCount())

          TESTING_EQUAL(item1->subBlockCount(), item1->mSubBlockCount)

          if (0 != item1->subBlockCount()) {
            TESTING_CHECK(NULL != item1->mSubBlocks)
            TESTING_CHECK(NULL != item2->mSubBlocks)
            for (size_t index = 0; index < item1->subBlockCount(); ++index) {
              TESTING_EQUAL(&(item1->mSubBlocks[index]), item1->subBlockAtIndex(index))
              TESTING_EQUAL(&(item2->mSubBlocks[index]), item2->subBlockAtIndex(index))

              DLRRReportBlock::SubBlock *sub1 = item1->subBlockAtIndex(index);
              DLRRReportBlock::SubBlock *sub2 = item2->subBlockAtIndex(index);

              TESTING_CHECK(NULL != sub1)
              TESTING_CHECK(NULL != sub2)

              TESTING_EQUAL(sub1->ssrc(), sub2->ssrc())
              TESTING_EQUAL(sub1->lrr(), sub2->lrr())
              TESTING_EQUAL(sub1->dlrr(), sub2->dlrr())

              TESTING_EQUAL(sub1->ssrc(), sub1->mSSRC)
              TESTING_EQUAL(sub1->lrr(), sub1->mLRR)
              TESTING_EQUAL(sub1->dlrr(), sub1->mDLRR)
            }
          } else {
            TESTING_CHECK(NULL == item1->mSubBlocks)
            TESTING_CHECK(NULL == item2->mSubBlocks)
          }
        }
        
        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessageStatisticsSummaryReportBlock(StatisticsSummaryReportBlock *item1, StatisticsSummaryReportBlock *item2)
        {
          TESTING_EQUAL(item1->lossReportFlag(), item2->lossReportFlag())
          TESTING_EQUAL(item1->duplicateReportFlag(), item2->duplicateReportFlag())
          TESTING_EQUAL(item1->jitterFlag(), item2->jitterFlag())
          TESTING_EQUAL(item1->ttlFlag(), item2->ttlFlag())
          TESTING_EQUAL(item1->hopLimitFlag(), item2->hopLimitFlag())

          TESTING_EQUAL(item1->lostPackets(), item2->lostPackets())
          TESTING_EQUAL(item1->dupPackets(), item2->dupPackets())
          TESTING_EQUAL(item1->minJitter(), item2->minJitter())
          TESTING_EQUAL(item1->maxJitter(), item2->maxJitter())
          TESTING_EQUAL(item1->meanJitter(), item2->meanJitter())
          TESTING_EQUAL(item1->devJitter(), item2->devJitter())

          TESTING_EQUAL(item1->minTTL(), item2->minTTL())
          TESTING_EQUAL(item1->maxTTL(), item2->maxTTL())
          TESTING_EQUAL(item1->meanTTL(), item2->meanTTL())
          TESTING_EQUAL(item1->devTTL(), item2->devTTL())

          TESTING_EQUAL(item1->minHopLimit(), item2->minHopLimit())
          TESTING_EQUAL(item1->maxHopLimit(), item2->maxHopLimit())
          TESTING_EQUAL(item1->meanHopLimit(), item2->meanHopLimit())
          TESTING_EQUAL(item1->devHopLimit(), item2->devHopLimit())

          TESTING_EQUAL(item1->lostPackets(), item1->mLostPackets)
          TESTING_EQUAL(item1->dupPackets(), item1->mDupPackets)
          TESTING_EQUAL(item1->minJitter(), item1->mMinJitter)
          TESTING_EQUAL(item1->maxJitter(), item1->mMaxJitter)
          TESTING_EQUAL(item1->meanJitter(), item1->mMeanJitter)
          TESTING_EQUAL(item1->devJitter(), item1->mDevJitter)

          TESTING_EQUAL(item1->minTTL(), item1->mMinTTLOrHL)
          TESTING_EQUAL(item1->maxTTL(), item1->mMaxTTLOrHL)
          TESTING_EQUAL(item1->meanTTL(), item1->mMeanTTLOrHL)
          TESTING_EQUAL(item1->devTTL(), item1->mDevTTLOrHL)

          TESTING_EQUAL(item1->minHopLimit(), item1->mMinTTLOrHL)
          TESTING_EQUAL(item1->maxHopLimit(), item1->mMaxTTLOrHL)
          TESTING_EQUAL(item1->meanHopLimit(), item1->mMeanTTLOrHL)
          TESTING_EQUAL(item1->devHopLimit(), item1->mDevTTLOrHL)
        }

        //---------------------------------------------------------------------
        static void comparePayloadSpecificFeedbackMessageVoIPMetricsReportBlock(VoIPMetricsReportBlock *item1, VoIPMetricsReportBlock *item2)
        {
          TESTING_EQUAL(item1->ssrcOfSource(), item2->ssrcOfSource())
          TESTING_EQUAL(item1->lossRate(), item2->lossRate())
          TESTING_EQUAL(item1->discardRate(), item2->discardRate())
          TESTING_EQUAL(item1->burstDensity(), item2->burstDensity())
          TESTING_EQUAL(item1->gapDensity(), item2->gapDensity())

          TESTING_EQUAL(item1->burstDuration(), item2->burstDuration())
          TESTING_EQUAL(item1->gapDuration(), item2->gapDuration())

          TESTING_EQUAL(item1->roundTripDelay(), item2->roundTripDelay())
          TESTING_EQUAL(item1->endSystemDelay(), item2->endSystemDelay())

          TESTING_EQUAL(item1->signalLevel(), item2->signalLevel())
          TESTING_EQUAL(item1->noiseLevel(), item2->noiseLevel())
          TESTING_EQUAL(item1->rerl(), item2->rerl())
          TESTING_EQUAL(item1->Gmin(), item2->Gmin())

          TESTING_EQUAL(item1->rFactor(), item2->rFactor())
          TESTING_EQUAL(item1->extRFactor(), item2->extRFactor())
          TESTING_EQUAL(item1->mosLQ(), item2->mosLQ())
          TESTING_EQUAL(item1->mosCQ(), item2->mosCQ())

          TESTING_EQUAL(item1->rxConfig(), item2->rxConfig())
          TESTING_EQUAL(item1->plc(), item2->plc())
          TESTING_EQUAL(item1->jba(), item2->jba())
          TESTING_EQUAL(item1->jbRate(), item2->jbRate())

          TESTING_EQUAL(item1->jbNominal(), item2->jbNominal())
          TESTING_EQUAL(item1->jbMaximum(), item2->jbMaximum())
          TESTING_EQUAL(item1->jbAbsMax(), item2->jbAbsMax())

          TESTING_EQUAL(item1->mReservedVoIP, item2->mReservedVoIP)


          TESTING_EQUAL(item1->ssrcOfSource(), item1->mSSRCOfSource)
          TESTING_EQUAL(item1->lossRate(), item1->mLossRate)
          TESTING_EQUAL(item1->discardRate(), item1->mDiscardRate)
          TESTING_EQUAL(item1->burstDensity(), item1->mBurstDensity)
          TESTING_EQUAL(item1->gapDensity(), item1->mGapDensity)

          TESTING_EQUAL(item1->burstDuration(), item1->mBurstDuration)
          TESTING_EQUAL(item1->gapDuration(), item1->mGapDuration)

          TESTING_EQUAL(item1->roundTripDelay(), item1->mRoundTripDelay)
          TESTING_EQUAL(item1->endSystemDelay(), item1->mEndSystemDelay)

          TESTING_EQUAL(item1->signalLevel(), item1->mSignalLevel)
          TESTING_EQUAL(item1->noiseLevel(), item1->mNoiseLevel)
          TESTING_EQUAL(item1->rerl(), item1->mRERL)
          TESTING_EQUAL(item1->Gmin(), item1->mGmin)

          TESTING_EQUAL(item1->rFactor(), item1->mRFactor)
          TESTING_EQUAL(item1->extRFactor(), item1->mExtRFactor)
          TESTING_EQUAL(item1->mosLQ(), item1->mMOSLQ)
          TESTING_EQUAL(item1->mosCQ(), item1->mMOSCQ)

          TESTING_EQUAL(item1->rxConfig(), item1->mRXConfig)

          TESTING_EQUAL(item1->jbNominal(), item1->mJBNominal)
          TESTING_EQUAL(item1->jbMaximum(), item1->mJBMaximum)
          TESTING_EQUAL(item1->jbAbsMax(), item1->mJBAbsMax)
        }
        
        //---------------------------------------------------------------------
        static void compareXR(XR *report1, XR *report2)
        {
          TESTING_EQUAL(report1->pt(), XR::kPayloadType)

          TESTING_EQUAL(report1->ssrc(), report2->ssrc())
          TESTING_EQUAL(report1->reportBlockCount(), report2->reportBlockCount())
          TESTING_EQUAL(report1->lossRLEReportBlockCount(), report2->lossRLEReportBlockCount())
          TESTING_EQUAL(report1->duplicateRLEReportBlockCount(), report2->duplicateRLEReportBlockCount())
          TESTING_EQUAL(report1->packetReceiptTimesReportBlockCount(), report2->packetReceiptTimesReportBlockCount())
          TESTING_EQUAL(report1->receiverReferenceTimeReportBlockCount(), report2->receiverReferenceTimeReportBlockCount())
          TESTING_EQUAL(report1->dlrrReportBlockCount(), report2->dlrrReportBlockCount())
          TESTING_EQUAL(report1->statisticsSummaryReportBlockCount(), report2->statisticsSummaryReportBlockCount())
          TESTING_EQUAL(report1->voIPMetricsReportBlockCount(), report2->voIPMetricsReportBlockCount())
          TESTING_EQUAL(report1->unknownReportBlockCount(), report2->unknownReportBlockCount())

          TESTING_EQUAL(report1->ssrc(), report1->mSSRC)
          TESTING_EQUAL(report1->reportBlockCount(), report1->mReportBlockCount)
          TESTING_EQUAL(report1->lossRLEReportBlockCount(), report1->mLossRLEReportBlockCount)
          TESTING_EQUAL(report1->duplicateRLEReportBlockCount(), report1->mDuplicateRLEReportBlockCount)
          TESTING_EQUAL(report1->packetReceiptTimesReportBlockCount(), report1->mPacketReceiptTimesReportBlockCount)
          TESTING_EQUAL(report1->receiverReferenceTimeReportBlockCount(), report1->mReceiverReferenceTimeReportBlockCount)
          TESTING_EQUAL(report1->dlrrReportBlockCount(), report1->mDLRRReportBlockCount)
          TESTING_EQUAL(report1->statisticsSummaryReportBlockCount(), report1->mStatisticsSummaryReportBlockCount)
          TESTING_EQUAL(report1->voIPMetricsReportBlockCount(), report1->mVoIPMetricsReportBlockCount)
          TESTING_EQUAL(report1->unknownReportBlockCount(), report1->mUnknownReportBlockCount)

          TESTING_CHECK(report1->firstLossRLEReportBlock() == report1->mFirstLossRLEReportBlock)
          TESTING_CHECK(report2->firstLossRLEReportBlock() == report2->mFirstLossRLEReportBlock)
          TESTING_CHECK(report1->firstDuplicateRLEReportBlock() == report1->mFirstDuplicateRLEReportBlock)
          TESTING_CHECK(report2->firstDuplicateRLEReportBlock() == report2->mFirstDuplicateRLEReportBlock)
          TESTING_CHECK(report1->firstPacketReceiptTimesReportBlock() == report1->mFirstPacketReceiptTimesReportBlock)
          TESTING_CHECK(report2->firstPacketReceiptTimesReportBlock() == report2->mFirstPacketReceiptTimesReportBlock)
          TESTING_CHECK(report1->firstReceiverReferenceTimeReportBlock() == report1->mFirstReceiverReferenceTimeReportBlock)
          TESTING_CHECK(report2->firstReceiverReferenceTimeReportBlock() == report2->mFirstReceiverReferenceTimeReportBlock)
          TESTING_CHECK(report1->firstDLRRReportBlock() == report1->mFirstDLRRReportBlock)
          TESTING_CHECK(report2->firstDLRRReportBlock() == report2->mFirstDLRRReportBlock)
          TESTING_CHECK(report1->firstStatisticsSummaryReportBlock() == report1->mFirstStatisticsSummaryReportBlock)
          TESTING_CHECK(report2->firstStatisticsSummaryReportBlock() == report2->mFirstStatisticsSummaryReportBlock)
          TESTING_CHECK(report1->firstVoIPMetricsReportBlock() == report1->mFirstVoIPMetricsReportBlock)
          TESTING_CHECK(report2->firstVoIPMetricsReportBlock() == report2->mFirstVoIPMetricsReportBlock)
          TESTING_CHECK(report1->firstUnknownReportBlock() == report1->mFirstUnknownReportBlock)
          TESTING_CHECK(report2->firstUnknownReportBlock() == report2->mFirstUnknownReportBlock)

          size_t totalReports = report1->mLossRLEReportBlockCount +
                                report1->mDuplicateRLEReportBlockCount +
                                report1->mPacketReceiptTimesReportBlockCount +
                                report1->mReceiverReferenceTimeReportBlockCount +
                                report1->mDLRRReportBlockCount +
                                report1->mStatisticsSummaryReportBlockCount +
                                report1->mVoIPMetricsReportBlockCount +
                                report1->mUnknownReportBlockCount;

          TESTING_EQUAL(report1->reportBlockCount(), totalReports)

          ReportBlock *current1 = report1->firstReportBlock();
          ReportBlock *current2 = report2->firstReportBlock();

          ReportBlock *next1 = NULL;
          ReportBlock *next2 = NULL;

          if (0 == totalReports) {
            TESTING_CHECK(NULL == current1)
            TESTING_CHECK(NULL == current2)
          }

          for (; (NULL != current1) && (NULL != current2); current1 = next1, current2 = next2)
          {
            next1 = current1->next();
            next2 = current1->next();

            TESTING_EQUAL(current1->blockType(), current2->blockType())
            TESTING_EQUAL(current1->typeSpecific(), current2->typeSpecific())

            TESTING_EQUAL(current1->blockType(), current1->mBlockType)
            TESTING_EQUAL(current1->typeSpecific(), current1->mTypeSpecific)
            TESTING_CHECK(current1->typeSpecificContents() == current1->mTypeSpecificContents)
            TESTING_CHECK(current2->typeSpecificContents() == current2->mTypeSpecificContents)
            TESTING_EQUAL(current1->typeSpecificContentSize(), current1->mTypeSpecificContentSize)
            TESTING_EQUAL(current2->typeSpecificContentSize(), current2->mTypeSpecificContentSize)

            switch (current1->blockType()) {
              case LossRLEReportBlock::kBlockType:                    {
                comparePayloadSpecificFeedbackMessageRLEReportBlock(reinterpret_cast<LossRLEReportBlock *>(current1), reinterpret_cast<LossRLEReportBlock *>(current2));
                break;
              }
              case DuplicateRLEReportBlock::kBlockType:               {
                comparePayloadSpecificFeedbackMessageRLEReportBlock(reinterpret_cast<DuplicateRLEReportBlock *>(current1), reinterpret_cast<DuplicateRLEReportBlock *>(current2));
                break;
              }
              case PacketReceiptTimesReportBlock::kBlockType:         {
                comparePayloadSpecificFeedbackMessagePacketReceiptTimesReportBlock(reinterpret_cast<PacketReceiptTimesReportBlock *>(current1), reinterpret_cast<PacketReceiptTimesReportBlock *>(current2));
                break;
              }
              case ReceiverReferenceTimeReportBlock::kBlockType:      {
                comparePayloadSpecificFeedbackMessageReceiverReferenceTimeReportBlock(reinterpret_cast<ReceiverReferenceTimeReportBlock *>(current1), reinterpret_cast<ReceiverReferenceTimeReportBlock *>(current2));
                break;
              }
              case DLRRReportBlock::kBlockType:                       {
                comparePayloadSpecificFeedbackMessagePacketDLRRReportBlock(reinterpret_cast<DLRRReportBlock *>(current1), reinterpret_cast<DLRRReportBlock *>(current2));
                break;
              }
              case StatisticsSummaryReportBlock::kBlockType:          {
                comparePayloadSpecificFeedbackMessageStatisticsSummaryReportBlock(reinterpret_cast<StatisticsSummaryReportBlock *>(current1), reinterpret_cast<StatisticsSummaryReportBlock *>(current2));
                break;
              }
              case VoIPMetricsReportBlock::kBlockType:                {
                comparePayloadSpecificFeedbackMessageVoIPMetricsReportBlock(reinterpret_cast<VoIPMetricsReportBlock *>(current1), reinterpret_cast<VoIPMetricsReportBlock *>(current2));
                break;
              }
              default:                                                {
                // padding could cause the buffer size to be increased to padding size
                TESTING_EQUAL(boundarySize(current1->typeSpecificContentSize()), boundarySize(current2->typeSpecificContentSize()))
                
                // check within boundary size
                auto size1 = current1->typeSpecificContentSize();
                auto size2 = current2->typeSpecificContentSize();

                if (size1 < size2) size2 = size1;
                if (size2 < size1) size1 = size2;

                checkEqual(current1->typeSpecificContents(), size1, current2->typeSpecificContents(), size2);
                break;
              }
            }
          }

          if (0 != report1->lossRLEReportBlockCount()) {
            TESTING_CHECK(NULL != report1->mFirstLossRLEReportBlock)
            TESTING_CHECK(NULL != report2->mFirstLossRLEReportBlock)
            for (size_t index = 0; index < report1->lossRLEReportBlockCount(); ++index) {
              TESTING_CHECK(&(report1->mFirstLossRLEReportBlock[index]) == report1->lossRLEReportBlockAtIndex(index))
              TESTING_CHECK(&(report2->mFirstLossRLEReportBlock[index]) == report2->lossRLEReportBlockAtIndex(index))
            }
          } else {
            TESTING_CHECK(NULL == report1->mFirstLossRLEReportBlock)
            TESTING_CHECK(NULL == report2->mFirstLossRLEReportBlock)
          }

          TESTING_CHECK(NULL == current1)
          TESTING_CHECK(NULL == current2)
        }

        //---------------------------------------------------------------------
        static void compareUnknownReport(UnknownReport *report1, UnknownReport *report2)
        {
          auto size1 = report1->size();
          auto size2 = report2->size();
          if (size1 != size2) {
            if (boundarySize(size1) == boundarySize(size2)) {
              if (size2 < size1) size1 = size2;
              if (size1 < size2) size2 = size1;
            }
          }
          checkEqual(report1->ptr(), size1, report2->ptr(), size2);
        }

        //---------------------------------------------------------------------
        static void compare(Report *report1, Report *report2)
        {
          Report *current1 = report1;
          Report *current2 = report2;
          Report *next1 = NULL;
          Report *next2 = NULL;

          for (; (NULL != current1) && (NULL != current2); current1 = next1, current2 = next2)
          {
            next1 = current1->next();
            next2 = current2->next();

            TESTING_CHECK(current1->pt() == current2->pt())

            compareReport(current1, current2);

            switch (current1->pt()) {
              case SenderReport::kPayloadType:                    compareSenderReport(reinterpret_cast<SenderReport *>(current1), reinterpret_cast<SenderReport *>(current2)); break;
              case ReceiverReport::kPayloadType:                  compareReceiverReport(reinterpret_cast<ReceiverReport *>(current1),reinterpret_cast<ReceiverReport *>(current2)); break;
              case SDES::kPayloadType:                            compareSDES(reinterpret_cast<SDES *>(current1),reinterpret_cast<SDES *>(current2)); break;
              case Bye::kPayloadType:                             compareBye(reinterpret_cast<Bye *>(current1),reinterpret_cast<Bye *>(current2)); break;
              case App::kPayloadType:                             compareApp(reinterpret_cast<App *>(current1),reinterpret_cast<App *>(current2)); break;
              case TransportLayerFeedbackMessage::kPayloadType:   compareTransportLayerFeedbackMessage(reinterpret_cast<TransportLayerFeedbackMessage *>(current1),reinterpret_cast<TransportLayerFeedbackMessage *>(current2)); break;
              case PayloadSpecificFeedbackMessage::kPayloadType:  comparePayloadSpecificFeedbackMessage(reinterpret_cast<PayloadSpecificFeedbackMessage *>(current1),reinterpret_cast<PayloadSpecificFeedbackMessage *>(current2)); break;
              case XR::kPayloadType:                              compareXR(reinterpret_cast<XR *>(current1),reinterpret_cast<XR *>(current2)); break;
              default:
              {
                compareUnknownReport(reinterpret_cast<UnknownReport *>(current1),reinterpret_cast<UnknownReport *>(current2));
                break;
              }
            }
          }

          TESTING_CHECK(NULL == current1)
          TESTING_CHECK(NULL == current2)
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Tester => (sanity routines)
        #pragma mark
        //---------------------------------------------------------------------
        static void checkSanity(const RTCPPacket &packet)
        {
          size_t total = 0;

          for (Report *current = packet.mFirst; NULL != current; current = current->next(), ++total)
          {
          }

          size_t index = 0;

          index = 0;
          for (auto *current = packet.mFirstSenderReport; NULL != current; current = current->nextSenderReport(), --total, ++index)
          {
            TESTING_CHECK(0 != total)

            TESTING_CHECK(packet.senderReportAtIndex(index) == current)

            size_t count = 0;
            for (auto block = current->mFirstReportBlock; NULL != block; block = block->next(), ++count) {
            }
            TESTING_EQUAL(count, current->rc())
          }

          index = 0;
          for (auto *current = packet.mFirstReceiverReport; NULL != current; current = current->nextReceiverReport(), --total, ++index)
          {
            TESTING_CHECK(0 != total)

            TESTING_CHECK(packet.receiverReportAtIndex(index) == current)

            size_t count = 0;
            for (auto block = current->mFirstReportBlock; NULL != block; block = block->next(), ++count) {
            }
            TESTING_EQUAL(count, current->rc())
          }

          index = 0;
          for (auto *current = packet.mFirstSDES; NULL != current; current = current->nextSDES(), --total, ++index)
          {
            TESTING_CHECK(0 != total)

            TESTING_CHECK(packet.sdesAtIndex(index) == current)

            size_t count = 0;
            for (auto block = current->mFirstChunk; NULL != block; block = block->next(), ++count) {
              size_t itemCount = 0;

              size_t itemTotal = block->mCount;

              itemCount = block->mCNameCount;
              for (auto item = block->mFirstCName; NULL != item; item = item->next(), --itemCount, --itemTotal) {
              }
              TESTING_EQUAL(itemCount, 0)

              itemCount = block->mNameCount;
              for (auto item = block->mFirstName; NULL != item; item = item->next(), --itemCount, --itemTotal) {
              }
              TESTING_EQUAL(itemCount, 0)

              itemCount = block->mEmailCount;
              for (auto item = block->mFirstEmail; NULL != item; item = item->next(), --itemCount, --itemTotal) {
              }
              TESTING_EQUAL(itemCount, 0)

              itemCount = block->mPhoneCount;
              for (auto item = block->mFirstPhone; NULL != item; item = item->next(), --itemCount, --itemTotal) {
              }
              TESTING_EQUAL(itemCount, 0)

              itemCount = block->mLocCount;
              for (auto item = block->mFirstLoc; NULL != item; item = item->next(), --itemCount, --itemTotal) {
              }
              TESTING_EQUAL(itemCount, 0)

              itemCount = block->mToolCount;
              for (auto item = block->mFirstTool; NULL != item; item = item->next(), --itemCount, --itemTotal) {
              }
              TESTING_EQUAL(itemCount, 0)

              itemCount = block->mNoteCount;
              for (auto item = block->mFirstNote; NULL != item; item = item->next(), --itemCount, --itemTotal) {
              }
              TESTING_EQUAL(itemCount, 0)

              itemCount = block->mPrivCount;
              for (auto item = block->mFirstPriv; NULL != item; item = item->next(), --itemCount, --itemTotal) {
              }
              TESTING_EQUAL(itemCount, 0)

              itemCount = block->mMidCount;
              for (auto item = block->mFirstMid; NULL != item; item = item->next(), --itemCount, --itemTotal) {
              }
              TESTING_EQUAL(itemCount, 0)

              itemCount = block->mUnknownCount;
              for (auto item = block->mFirstUnknown; NULL != item; item = item->next(), --itemCount, --itemTotal) {
              }
              TESTING_EQUAL(itemCount, 0)

              TESTING_EQUAL(itemTotal, 0)
            }
            TESTING_EQUAL(count, current->sc())
          }

          index = 0;
          for (auto *current = packet.mFirstBye; NULL != current; current = current->nextBye(), --total, ++index)
          {
            TESTING_CHECK(0 != total)

            TESTING_CHECK(packet.byeAtIndex(index) == current)
          }

          index = 0;
          for (auto *current = packet.mFirstApp; NULL != current; current = current->nextApp(), --total, ++index)
          {
            TESTING_CHECK(0 != total)

            TESTING_CHECK(packet.appAtIndex(index) == current)
          }

          index = 0;
          for (auto *current = packet.mFirstTransportLayerFeedbackMessage; NULL != current; current = current->nextTransportLayerFeedbackMessage(), --total, ++index)
          {
            TESTING_CHECK(0 != total)

            TESTING_CHECK(packet.transportLayerFeedbackReportAtIndex(index) == current)
          }

          index = 0;
          for (auto *current = packet.mFirstPayloadSpecificFeedbackMessage; NULL != current; current = current->nextPayloadSpecificFeedbackMessage(), --total, ++index)
          {
            TESTING_CHECK(0 != total)

            TESTING_CHECK(packet.payloadSpecificFeedbackReportAtIndex(index) == current)
          }

          index = 0;
          for (auto *current = packet.mFirstXR; NULL != current; current = current->nextXR(), --total, ++index)
          {
            TESTING_CHECK(0 != total)

            TESTING_CHECK(packet.xrAtIndex(index) == current)

            size_t count = 0;
            for (auto block = current->mFirstReportBlock; NULL != block; block = block->next(), ++count) {
            }

            TESTING_EQUAL(count, current->mReportBlockCount)

            size_t itemCount = 0;

            itemCount = current->mLossRLEReportBlockCount;
            for (auto item = current->mFirstLossRLEReportBlock; NULL != item; item = item->nextLossRLE(), --itemCount, --count) {
            }
            TESTING_EQUAL(itemCount, 0)

            itemCount = current->mDuplicateRLEReportBlockCount;
            for (auto item = current->mFirstDuplicateRLEReportBlock; NULL != item; item = item->nextDuplicateRLE(), --itemCount, --count) {
            }
            TESTING_EQUAL(itemCount, 0)

            itemCount = current->mPacketReceiptTimesReportBlockCount;
            for (auto item = current->mFirstPacketReceiptTimesReportBlock; NULL != item; item = item->nextPacketReceiptTimesReportBlock(), --itemCount, --count) {
            }
            TESTING_EQUAL(itemCount, 0)

            itemCount = current->mReceiverReferenceTimeReportBlockCount;
            for (auto item = current->mFirstReceiverReferenceTimeReportBlock; NULL != item; item = item->nextReceiverReferenceTimeReportBlock(), --itemCount, --count) {
            }
            TESTING_EQUAL(itemCount, 0)

            itemCount = current->mDLRRReportBlockCount;
            for (auto item = current->mFirstDLRRReportBlock; NULL != item; item = item->nextDLRRReportBlock(), --itemCount, --count) {
            }
            TESTING_EQUAL(itemCount, 0)

            itemCount = current->mStatisticsSummaryReportBlockCount;
            for (auto item = current->mFirstStatisticsSummaryReportBlock; NULL != item; item = item->nextStatisticsSummaryReportBlock(), --itemCount, --count) {
            }
            TESTING_EQUAL(itemCount, 0)

            itemCount = current->mVoIPMetricsReportBlockCount;
            for (auto item = current->mFirstVoIPMetricsReportBlock; NULL != item; item = item->nextVoIPMetricsReportBlock(), --itemCount, --count) {
            }
            TESTING_EQUAL(itemCount, 0)

            itemCount = current->mUnknownReportBlockCount;
            for (auto item = current->mFirstUnknownReportBlock; NULL != item; item = item->nextUnknownReportBlock(), --itemCount, --count) {
            }
            TESTING_EQUAL(itemCount, 0)


            TESTING_EQUAL(count, 0)
          }

          index = 0;
          for (auto *current = packet.mFirstUnknownReport; NULL != current; current = current->nextUnknown(), --total, ++index)
          {
            TESTING_CHECK(0 != total)

            TESTING_CHECK(packet.unknownAtIndex(index) == current)
          }

          TESTING_EQUAL(total, 0)
        }

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark Tester => (create routines)
        #pragma mark

        //---------------------------------------------------------------------
        static void fillReport(Report *common)
        {
          common->mVersion = 2;
        }

        //---------------------------------------------------------------------
        static void fillSenderReceiverCommonReport(SenderReceiverCommonReport *common)
        {
          typedef SenderReceiverCommonReport::ReportBlock CommonReportBlock;

          fillReport(common);

          common->mSSRCOfSender = randomDWORD();

          if (shouldPerform(30)) {
            common->mExtensionSize = randomSize(1, 1000);
            common->mExtension = new BYTE[common->mExtensionSize];
            randomizeBufferSequencial(const_cast<BYTE *>(common->mExtension), common->mExtensionSize);
          }

          if (shouldPerform(70)) {
            common->mReportSpecific = static_cast<BYTE>(randomSize(1, randomBYTE(5)));

            common->mFirstReportBlock = new CommonReportBlock[common->rc()];

            TESTING_CHECK(common->rc() > 0)

            for (size_t count = 0; count < common->rc(); ++count) {
              CommonReportBlock *block = &(common->mFirstReportBlock[count]);
              if (0 != count) {
                (&(common->mFirstReportBlock[count-1]))->mNext = block;
              }

              block->mSSRC = randomDWORD();
              block->mFractionLost = randomBYTE();
              block->mCumulativeNumberOfPacketsLost = randomDWORD(24);
              block->mExtendedHighestSequenceNumberReceived = randomDWORD();
              block->mLSR = randomDWORD();
              block->mDLSR = randomDWORD();
            }
          }
        }

        //---------------------------------------------------------------------
        static SenderReport *createSenderReport()
        {
          SenderReport *result = new SenderReport;
          result->mPT = SenderReport::kPayloadType;
          fillSenderReceiverCommonReport(result);
          result->mNTPTimestampMS = randomDWORD();
          result->mNTPTimestampLS = randomDWORD();
          result->mRTPTimestamp = randomDWORD();
          result->mSenderPacketCount = randomDWORD();
          result->mSenderOctetCount = randomDWORD();
          return result;
        }

        //---------------------------------------------------------------------
        static ReceiverReport *createReceiverReport()
        {
          ReceiverReport *result = new ReceiverReport;
          result->mPT = ReceiverReport::kPayloadType;
          fillSenderReceiverCommonReport(result);
          return result;
        }

        //---------------------------------------------------------------------
        static void fillSenderReceiverPriv(SDES::Chunk::Priv *common)
        {
          if (!shouldPerform(80)) {
            common->mPrefixLength = 0;
            common->mLength = 0;
            return;
          }

          if (shouldPerform(80)) {
            common->mPrefixLength = random(1, 20);
            if (shouldPerform(5)) {
              common->mPrefixLength = 0xFE;   // force to maximum length
            } else if (shouldPerform(5)) {
              common->mPrefixLength = 0xFD;   // force to maximum length-1
            } else if (shouldPerform(5)) {
              common->mPrefixLength = 1;      // force to minimum length possible
            }
            common->mPrefix = new char[common->mPrefixLength+1];
            memset(const_cast<char *>(common->mPrefix), 0, (common->mPrefixLength+1)*sizeof(char));

            String str = randomString(common->mPrefixLength);
            memcpy(const_cast<char *>(common->mPrefix), str.c_str(), common->mPrefixLength);
          }

          if (shouldPerform(95)) {
            if (common->mPrefixLength < 0xFE) {
              common->mLength = random(1, 0xFF-(common->mPrefixLength)-1);
              if (shouldPerform(5)) {
                common->mLength = 0xFF-(common->mPrefixLength)-1;   // force to maximum length possible
              } else if (shouldPerform(5)) {
                if (common->mPrefixLength < 0xFD) {
                  common->mLength = 0xFF-(common->mPrefixLength)-2; // force to maximum length possible-1
                }
              } else if (shouldPerform(5)) {
                common->mLength = 1;                                // force to minimum length possible
              }
              common->mValue = new char[common->mLength+1];
              memset(const_cast<char *>(common->mValue), 0, (common->mLength+1)*sizeof(char));

              String str = randomString(common->mLength);
              memcpy(const_cast<char *>(common->mValue), str.c_str(), common->mLength);
            }
          }
        }

        //---------------------------------------------------------------------
        static void fillSenderReceiverStringItem(BYTE type, SDES::Chunk::StringItem *common)
        {
          common->mType = type;
          if (shouldPerform(95)) {
            common->mLength = random(1, 0xFF);
            if (shouldPerform(5)) {
              common->mLength = 0xFF; // force to maximum length
            } else if (shouldPerform(5)) {
              common->mLength = 0xFF-1; // force to maximum length-1
            } else if (shouldPerform(5)) {
              common->mLength = 1;      // force to minimum length possible
            }
            common->mValue = new char[common->mLength+1];
            memset(const_cast<char *>(common->mValue), 0, (common->mLength+1)*sizeof(char));

            String str = randomString(common->mLength);
            memcpy(const_cast<char *>(common->mValue), str.c_str(), common->mLength);

            ZS_LOG_TRACE(slog("generated SDES item") + ZS_PARAM("type", common->typeToString()) + ZS_PARAM("type (number)", common->type()) + ZS_PARAM("length", common->mLength) + ZS_PARAM("value", (NULL != common->mValue ? common->mValue : NULL)))
          }
        }

        //---------------------------------------------------------------------
        static void fillSenderReceiverStringItem(BYTE type, SDES::Chunk::StringItem *first, size_t count, size_t &ioTotalItems)
        {
          for (size_t index = 0; index < count; ++index, ++ioTotalItems) {
            auto current = &(first[index]);
            if (0 != index) {
              (&(first[index-1]))->mNext = current;
            }
            current->mType = type;
            fillSenderReceiverStringItem(type, current);
          }
        }

        //---------------------------------------------------------------------
        static void fillSenderReceiverPriv(BYTE type, SDES::Chunk::Priv *first, size_t count, size_t &ioTotalItems)
        {
          for (size_t index = 0; index < count; ++index, ++ioTotalItems) {
            auto current = &(first[index]);
            if (0 != index) {
              (&(first[index-1]))->mNext = current;
            }
            current->mType = type;
            fillSenderReceiverPriv(current);
            ZS_LOG_TRACE(slog("generated SDES item") + ZS_PARAM("type", current->typeToString()) + ZS_PARAM("prefix len", current->prefixLength()) + ZS_PARAM("prefix", (NULL != current->mPrefix ? current->mPrefix : NULL)) + ZS_PARAM("length", current->mLength) + ZS_PARAM("value", (NULL != current->mValue ? current->mValue : NULL)))
          }
        }

        //---------------------------------------------------------------------
        static SDES *createSDES()
        {
          typedef SDES::Chunk Chunk;

          SDES *result = new SDES;
          result->mPT = SDES::kPayloadType;
          fillReport(result);

          size_t count = randomSize(15);

          if (0 != count) {
            result->mReportSpecific = static_cast<decltype(result->mReportSpecific)>(count);
            result->mFirstChunk = new SDES::Chunk[count];

            for (size_t index = 0; index < count; ++index)
            {
              auto chunk = &(result->mFirstChunk[index]);
              chunk->mSSRC = randomDWORD();

              size_t &totalItems = chunk->mCount;

              if (0 != index) {
                (&(result->mFirstChunk[index-1]))->mNext = chunk;
              }

              if (shouldPerform(20)) {
                chunk->mCNameCount = randomSize(1, shouldPerform(80) ? 1 : randomSize(2, 5));
                chunk->mFirstCName = new Chunk::CName[chunk->mCNameCount];
                fillSenderReceiverStringItem(Chunk::CName::kItemType, chunk->mFirstCName, chunk->mCNameCount, totalItems);
              }
              if (shouldPerform(20)) {
                chunk->mNameCount = randomSize(1, shouldPerform(80) ? 1 : randomSize(2, 5));
                chunk->mFirstName = new Chunk::Name[chunk->mNameCount];
                fillSenderReceiverStringItem(Chunk::Name::kItemType, chunk->mFirstName, chunk->mNameCount, totalItems);
              }
              if (shouldPerform(20)) {
                chunk->mEmailCount = randomSize(1, shouldPerform(80) ? 1 : randomSize(2, 5));
                chunk->mFirstEmail = new Chunk::Email[chunk->mEmailCount];
                fillSenderReceiverStringItem(Chunk::Email::kItemType, chunk->mFirstEmail, chunk->mEmailCount, totalItems);
              }
              if (shouldPerform(20)) {
                chunk->mPhoneCount = randomSize(1, shouldPerform(80) ? 1 : randomSize(2, 5));
                chunk->mFirstPhone = new Chunk::Phone[chunk->mPhoneCount];
                fillSenderReceiverStringItem(Chunk::Phone::kItemType, chunk->mFirstPhone, chunk->mPhoneCount, totalItems);
              }
              if (shouldPerform(20)) {
                chunk->mLocCount = randomSize(1, shouldPerform(80) ? 1 : randomSize(2, 5));
                chunk->mFirstLoc = new Chunk::Loc[chunk->mLocCount];
                fillSenderReceiverStringItem(Chunk::Loc::kItemType, chunk->mFirstLoc, chunk->mLocCount, totalItems);
              }
              if (shouldPerform(20)) {
                chunk->mToolCount = randomSize(1, shouldPerform(80) ? 1 : randomSize(2, 5));
                chunk->mFirstTool = new Chunk::Tool[chunk->mToolCount];
                fillSenderReceiverStringItem(Chunk::Tool::kItemType, chunk->mFirstTool, chunk->mToolCount, totalItems);
              }
              if (shouldPerform(20)) {
                chunk->mNoteCount = randomSize(1, shouldPerform(80) ? 1 : randomSize(2, 5));
                chunk->mFirstNote = new Chunk::Note[chunk->mNoteCount];
                fillSenderReceiverStringItem(Chunk::Note::kItemType, chunk->mFirstNote, chunk->mNoteCount, totalItems);
              }
              if (shouldPerform(20)) {
                chunk->mPrivCount = randomSize(1, shouldPerform(80) ? 1 : randomSize(2, 5));
                chunk->mFirstPriv = new Chunk::Priv[chunk->mPrivCount];
                fillSenderReceiverPriv(Chunk::Priv::kItemType, chunk->mFirstPriv, chunk->mPrivCount, totalItems);
              }
              if (shouldPerform(20)) {
                chunk->mMidCount = randomSize(1, shouldPerform(80) ? 1 : randomSize(2, 5));
                chunk->mFirstMid = new Chunk::Mid[chunk->mMidCount];
                fillSenderReceiverStringItem(Chunk::Mid::kItemType, chunk->mFirstMid, chunk->mMidCount, totalItems);
              }
              if (shouldPerform(20)) {
                chunk->mUnknownCount = randomSize(1, shouldPerform(80) ? 1 : randomSize(2, 5));
                chunk->mFirstUnknown = new Chunk::Unknown[chunk->mUnknownCount];
                fillSenderReceiverStringItem(static_cast<BYTE>(random(Chunk::Mid::kItemType+1, 0xFF)), chunk->mFirstUnknown, chunk->mUnknownCount, totalItems);
              }

              ZS_LOG_TRACE(slog("generated SDES chunk complete") + ZS_PARAM("chunk", index))
            }
          }

          return result;
        }

        //---------------------------------------------------------------------
        static Bye *createBye()
        {
          Bye *result = new Bye;
          result->mPT = Bye::kPayloadType;
          fillReport(result);

          result->mReportSpecific = static_cast<decltype(result->mReportSpecific)>(randomSize(15));

          if (0 != result->sc()) {
            result->mSSRCs = new DWORD[result->sc()];

            for (size_t index = 0; index < result->sc(); ++index)
            {
              result->mSSRCs[index] = randomDWORD();
            }
          }

          if (shouldPerform(40)) {
            String str = randomString(random(1,0xFF));

            result->mReasonForLeaving = new char[str.length()+1];
            memset(const_cast<char *>(result->mReasonForLeaving), 0, sizeof(char)*(str.length()+1));
            memcpy(const_cast<char *>(result->mReasonForLeaving), str.c_str(), str.length()*sizeof(char));
          }

          return result;
        }

        //---------------------------------------------------------------------
        static App *createApp()
        {
          App *result = new App;
          result->mPT = App::kPayloadType;
          fillReport(result);

          result->mReportSpecific = static_cast<decltype(result->mReportSpecific)>(random(0, 15));
          result->mSSRC = randomDWORD();

          String tmp = randomString(sizeof(DWORD));

          memcpy(&(result->mName[0]), tmp.c_str(), sizeof(DWORD));

          if (shouldPerform(80)) {
            result->mDataSize = randomSize(1, 150);
            result->mData = new BYTE[result->mDataSize];

            randomizeBufferSequencial(const_cast<BYTE *>(result->mData), result->mDataSize);
          }
          return result;
        }

        //---------------------------------------------------------------------
        static TransportLayerFeedbackMessage *createTransportLayerFeedbackMessage()
        {
          TransportLayerFeedbackMessage *result = new TransportLayerFeedbackMessage;
          result->mPT = TransportLayerFeedbackMessage::kPayloadType;
          fillReport(result);

          result->mSSRCOfPacketSender = randomDWORD();
          result->mSSRCOfMediaSource = randomDWORD();

          switch (randomSize(3)) {
            case 0: result->mReportSpecific = GenericNACK::kFmt; break;
            case 1: result->mReportSpecific = TMMBR::kFmt; break;
            case 2: result->mReportSpecific = TMMBN::kFmt; break;
            case 3: {
              result->mReportSpecific = static_cast<decltype(result->mReportSpecific)>(randomSize(TMMBN::kFmt+1, 0x1F));
              break;
            }
            default:
            {
              TESTING_CHECK(false)
              break;
            }
          }

          switch (result->mReportSpecific) {
            case GenericNACK::kFmt:  {
              result->mGenericNACKCount = randomSize(1, 25);
              result->mFirstGenericNACK = new GenericNACK[result->mGenericNACKCount];

              for (size_t index = 0; index < result->mGenericNACKCount; ++index) {
                auto current = (&(result->mFirstGenericNACK[index]));

                current->mPID = randomWORD();
                current->mBLP = randomWORD();
              }
              break;
            }
            case TMMBR::kFmt:        {
              result->mTMMBRCount = randomSize(1, 25);
              result->mFirstTMMBR = new TMMBR[result->mTMMBRCount];

              for (size_t index = 0; index < result->mTMMBRCount; ++index) {
                auto current = (&(result->mFirstTMMBR[index]));

                current->mSSRC = randomDWORD();
                current->mMxTBRExp = randomBYTE(6);
                current->mMxTBRMantissa = randomDWORD(17);
                current->mMeasuredOverhead = randomWORD(9);
              }
              break;
            }
            case TMMBN::kFmt:        {
              result->mTMMBNCount = randomSize(1, 25);
              result->mFirstTMMBN = new TMMBN[result->mTMMBNCount];

              for (size_t index = 0; index < result->mTMMBNCount; ++index) {
                auto current = (&(result->mFirstTMMBN[index]));

                current->mSSRC = randomDWORD();
                current->mMxTBRExp = randomBYTE(6);
                current->mMxTBRMantissa = randomDWORD(17);
                current->mMeasuredOverhead = randomWORD(9);
              }
              break;
            }
            default: {
              result->mUnknown = result;
              if (shouldPerform(90)) {
                result->mFCISize = randomSize(1,100);
                result->mFCI = new BYTE[result->mFCISize];
                randomizeBufferSequencial(const_cast<BYTE *>(result->mFCI), result->mFCISize);
              }
              break;
            }
          }

          ZS_LOG_TRACE(slog("generated transport specific report") + ZS_PARAM("fmt", result->fmtToString()) + ZS_PARAM("fmt (number)", result->fmt()))

          return result;
        }
        
        //---------------------------------------------------------------------
        static PayloadSpecificFeedbackMessage *createPayloadSpecificFeedbackMessage()
        {
          PayloadSpecificFeedbackMessage *result = new PayloadSpecificFeedbackMessage;
          result->mPT = PayloadSpecificFeedbackMessage::kPayloadType;
          fillReport(result);

          result->mSSRCOfPacketSender = randomDWORD();
          result->mSSRCOfMediaSource = randomDWORD();

          switch (randomSize(9)) {
            case 0: result->mReportSpecific = PLI::kFmt; break;
            case 1: result->mReportSpecific = SLI::kFmt; break;
            case 2: result->mReportSpecific = RPSI::kFmt; break;
            case 3: result->mReportSpecific = FIR::kFmt; break;
            case 4: result->mReportSpecific = TSTR::kFmt; break;
            case 5: result->mReportSpecific = TSTN::kFmt; break;
            case 6: result->mReportSpecific = VBCM::kFmt; break;
            case 7: result->mReportSpecific = AFB::kFmt; break;
            case 8: {
              result->mReportSpecific = PayloadSpecificFeedbackMessage::AFB::kFmt;
              result->mHasREMB = true;
              break;
            }
            case 9: {
              result->mReportSpecific = static_cast<decltype(result->mReportSpecific)>(randomSize(AFB::kFmt+1, 0x1F));
              break;
            }
            default:
            {
              TESTING_CHECK(false)
            }
          }

          switch (result->mReportSpecific) {
            case PLI::kFmt:  {
              break;
            }
            case SLI::kFmt:        {
              result->mSLICount = randomSize(1, 25);
              result->mFirstSLI = new SLI[result->mSLICount];

              for (size_t index = 0; index < result->mSLICount; ++index) {
                auto current = (&(result->mFirstSLI[index]));

                current->mFirst = randomWORD(13);
                current->mNumber = randomWORD(13);
                current->mPictureID = randomBYTE(6);
              }
              break;
            }
            case RPSI::kFmt:        {
              result->mRPSI.mZeroBit = shouldPerform(90) ? 0 : 1;
              result->mRPSI.mPayloadType = randomBYTE(7);

              if (shouldPerform(80)) {
                result->mRPSI.mNativeRPSIBitStringSizeInBits = (random(1, 50*8));
                if (shouldPerform(5)) {
                  result->mRPSI.mNativeRPSIBitStringSizeInBits = (0xFFFC*sizeof(DWORD))*8;      // force to max length
                } else if (shouldPerform(5)) {
                  result->mRPSI.mNativeRPSIBitStringSizeInBits = ((0xFFFC*sizeof(DWORD))*8)-1;  // force to max length-1
                } else if (shouldPerform(5)) {
                  result->mRPSI.mNativeRPSIBitStringSizeInBits = 1;           // force to minimu length possible
                }
                result->mRPSI.mNativeRPSIBitString = new BYTE[((result->mRPSI.mNativeRPSIBitStringSizeInBits)/8)+1];

                randomizeBufferSequencial(const_cast<BYTE *>(result->mRPSI.mNativeRPSIBitString), (result->mRPSI.mNativeRPSIBitStringSizeInBits/8)+1);
              }
              break;
            }
            case FIR::kFmt:         {
              result->mFIRCount = randomSize(1, 25);
              result->mFirstFIR = new FIR[result->mFIRCount];

              for (size_t index = 0; index < result->mFIRCount; ++index) {
                auto current = (&(result->mFirstFIR[index]));

                current->mSSRC = randomDWORD();
                current->mSeqNr = randomBYTE();
                if (shouldPerform(10)) {
                  current->mReserved = randomDWORD(24);
                }
              }
              break;
            }
            case TSTR::kFmt:        {
              result->mTSTRCount = randomSize(1, 25);
              result->mFirstTSTR = new TSTR[result->mTSTRCount];

              for (size_t index = 0; index < result->mTSTRCount; ++index) {
                auto current = (&(result->mFirstTSTR[index]));

                current->mSSRC = randomDWORD();
                current->mSeqNr = randomBYTE();
                if (shouldPerform(10)) {
                  current->mReserved = randomDWORD(19);
                }
                current->mControlSpecific = randomBYTE(5);
              }
              break;
            }
            case TSTN::kFmt:        {
              result->mTSTNCount = randomSize(1, 25);
              result->mFirstTSTN = new TSTN[result->mTSTNCount];

              for (size_t index = 0; index < result->mTSTNCount; ++index) {
                auto current = (&(result->mFirstTSTN[index]));

                current->mSSRC = randomDWORD();
                current->mSeqNr = randomBYTE();
                if (shouldPerform(10)) {
                  current->mReserved = randomDWORD(19);
                }
                current->mControlSpecific = randomBYTE(5);
              }
              break;
            }
            case VBCM::kFmt:        {
              size_t totalSize = 0;

              result->mVBCMCount = 0;
              result->mFirstVBCM = NULL;

              do {

                for (size_t index = 0; index < result->mVBCMCount; ++index) {
                  auto current = (&(result->mFirstVBCM[index]));

                  if (current->mVBCMOctetString) {
                    delete [] const_cast<BYTE *>(current->mVBCMOctetString);
                    current->mVBCMOctetString = NULL;
                  }
                }
                if (NULL != result->mFirstVBCM) {
                  delete [] result->mFirstVBCM;
                  result->mFirstVBCM = NULL;
                }

                result->mVBCMCount = randomSize(1, 25);
                result->mFirstVBCM = new VBCM[result->mVBCMCount];

                totalSize = (sizeof(DWORD)*2);

                for (size_t index = 0; index < result->mVBCMCount; ++index) {
                  auto current = (&(result->mFirstVBCM[index]));

                  current->mSSRC = randomDWORD();
                  current->mControlSpecific = static_cast<DWORD>(randomBYTE(7)) << 16;
                  if (shouldPerform(10)) {
                    current->mControlSpecific = current->mControlSpecific | (1 << 23);
                  }
                  current->mSeqNr = randomBYTE();
                  current->mReserved = 0;

                  totalSize += sizeof(DWORD)*2;

                  if (shouldPerform(90)) {
                    WORD size = randomWORD();
                    if (shouldPerform(50)) {
                      size = size % 1000; // keep small to prevent massively large RTCP report blocks
                    }
                    if (shouldPerform(5)) {
                      size = 0xFFFF;                // fill to capacity
                    } else if (shouldPerform(5)) {
                      size = (0xFFFF-1);            // fill to capacity-1
                    } else if (shouldPerform(5)) {
                      size = 1;                     // fill to minum capacity possible
                    }
                    if (0 == size) ++size;

                    totalSize += boundarySize(size);
                    current->mControlSpecific = current->mControlSpecific | static_cast<DWORD>(size);
                    current->mVBCMOctetString = new BYTE[static_cast<size_t>(size)];

                    randomizeBufferSequencial(const_cast<BYTE *>(current->mVBCMOctetString), static_cast<size_t>(size));
                  }
                }
              } while ((totalSize / sizeof(DWORD)) > 0xFFFF);
              break;
            }
            case AFB::kFmt:         {
              auto remb = result->remb();
              if (NULL != remb) {
                // REMB feedback message
                remb->mNumSSRC = randomBYTE();
                if (0 == remb->mNumSSRC) ++(remb->mNumSSRC);
                remb->mSSRCs = new DWORD[remb->numSSRC()];
                for (size_t index = 0; index < remb->numSSRC(); ++index) {
                  remb->mSSRCs[index] = randomDWORD();
                }
              } else {
                // application feedback message
                if (shouldPerform(90)) {
                  result->mAFB.mDataSize = randomSize(1, 100);
                  result->mAFB.mData = new BYTE[result->mAFB.mDataSize];
                  randomizeBufferSequencial(const_cast<BYTE *>(result->mAFB.mData), result->mAFB.mDataSize);
                }
              }
              break;
            }
            default: {
              result->mUnknown = result;
              if (shouldPerform(90)) {
                result->mFCISize = randomSize(1, 100);
                result->mFCI = new BYTE[result->mFCISize];
                randomizeBufferSequencial(const_cast<BYTE *>(result->mFCI), result->mFCISize);
              }
              break;
            }
          }

          ZS_LOG_TRACE(slog("generated payload specific report") + ZS_PARAM("fmt", result->fmtToString()) + ZS_PARAM("fmt (number)", result->fmt()))

          return result;
        }

        //---------------------------------------------------------------------
        static void fillReportBlockRange(ReportBlockRange *range)
        {
          if (shouldPerform(5)) {
            range->mTypeSpecific = (randomBYTE(4)) << 4;
          } else {
            range->mTypeSpecific = 0;
          }
          range->mTypeSpecific = range->mTypeSpecific | randomBYTE(4);

          range->mSSRCOfSource = randomDWORD();
          range->mBeginSeq = randomWORD();
          range->mEndSeq = randomWORD();
        }

        //---------------------------------------------------------------------
        static void fillRLEReportBlock(RLEReportBlock *block)
        {
          fillReportBlockRange(block);

          if (!shouldPerform(80)) return;

          block->mChunkCount = randomSize(1, 12);
          block->mChunks = new RLEChunk[block->mChunkCount];

          for (size_t index = 0; index < block->mChunkCount; ++index) {
            RLEChunk &value = block->mChunks[index];
            if (shouldPerform(50)) {
              value = (0 << 15) | (((shouldPerform(50)) ? 1 : 0) << 14);
              WORD length = randomWORD(13);
              if (0 == length) ++length;
              value = value | length;
            } else {
              value = (1 << 15) | randomWORD(14);
            }
          }
        }
        
        //---------------------------------------------------------------------
        static XR *createXR()
        {
          XR *result = new XR;
          result->mPT = XR::kPayloadType;
          fillReport(result);

          result->mSSRC = randomDWORD();

          if (!shouldPerform(95)) return result;

          result->mReportBlockCount = randomSize(1,10);

          BYTE *randomBuffer = new BYTE[result->mReportBlockCount+1];
          randomizeBuffer(randomBuffer, result->mReportBlockCount);

          for (size_t loop = 0; loop < result->mReportBlockCount; ++loop) {
            BYTE type = 0;

            switch (randomBuffer[loop]%8) { // using the fixed seed
              case 0: type = static_cast<decltype(type)>(++(result->mLossRLEReportBlockCount)); break;
              case 1: type = static_cast<decltype(type)>(++(result->mDuplicateRLEReportBlockCount)); break;
              case 2: type = static_cast<decltype(type)>(++(result->mPacketReceiptTimesReportBlockCount)); break;
              case 3: type = static_cast<decltype(type)>(++(result->mReceiverReferenceTimeReportBlockCount)); break;
              case 4: type = static_cast<decltype(type)>(++(result->mDLRRReportBlockCount)); break;
              case 5: type = static_cast<decltype(type)>(++(result->mStatisticsSummaryReportBlockCount)); break;
              case 6: type = static_cast<decltype(type)>(++(result->mVoIPMetricsReportBlockCount)); break;
              case 7: {
                type = static_cast<decltype(type)>(++(result->mUnknownReportBlockCount));
                break;
              }
              default:
              {
                TESTING_CHECK(false)
              }
            }
          }

          if (0 != result->mLossRLEReportBlockCount) {
            result->mFirstLossRLEReportBlock = new LossRLEReportBlock[result->mLossRLEReportBlockCount];
            result->mLossRLEReportBlockCount = 0;
          }
          if (0 != result->mDuplicateRLEReportBlockCount) {
            result->mFirstDuplicateRLEReportBlock = new DuplicateRLEReportBlock[result->mDuplicateRLEReportBlockCount];
            result->mDuplicateRLEReportBlockCount = 0;
          }
          if (0 != result->mPacketReceiptTimesReportBlockCount) {
            result->mFirstPacketReceiptTimesReportBlock = new PacketReceiptTimesReportBlock[result->mPacketReceiptTimesReportBlockCount];
            result->mPacketReceiptTimesReportBlockCount = 0;
          }
          if (0 != result->mReceiverReferenceTimeReportBlockCount) {
            result->mFirstReceiverReferenceTimeReportBlock = new ReceiverReferenceTimeReportBlock[result->mReceiverReferenceTimeReportBlockCount];
            result->mReceiverReferenceTimeReportBlockCount = 0;
          }
          if (0 != result->mDLRRReportBlockCount) {
            result->mFirstDLRRReportBlock = new DLRRReportBlock[result->mDLRRReportBlockCount];
            result->mDLRRReportBlockCount = 0;
          }
          if (0 != result->mStatisticsSummaryReportBlockCount) {
            result->mFirstStatisticsSummaryReportBlock = new StatisticsSummaryReportBlock[result->mStatisticsSummaryReportBlockCount];
            result->mStatisticsSummaryReportBlockCount = 0;
          }
          if (0 != result->mVoIPMetricsReportBlockCount) {
            result->mFirstVoIPMetricsReportBlock = new VoIPMetricsReportBlock[result->mVoIPMetricsReportBlockCount];
            result->mVoIPMetricsReportBlockCount = 0;
          }
          if (0 != result->mUnknownReportBlockCount) {
            result->mFirstUnknownReportBlock = new UnknownReportBlock[result->mUnknownReportBlockCount];
            result->mUnknownReportBlockCount = 0;
          }

          ReportBlock *lastBlock = NULL;

          for (size_t loop = 0; loop < result->mReportBlockCount; ++loop) {
            BYTE type = 0;

            ReportBlock *useBlock = NULL;

            switch (randomBuffer[loop]%8) { // using the fixed seed
              case 0: type = LossRLEReportBlock::kBlockType; break;
              case 1: type = DuplicateRLEReportBlock::kBlockType; break;
              case 2: type = PacketReceiptTimesReportBlock::kBlockType; break;
              case 3: type = ReceiverReferenceTimeReportBlock::kBlockType; break;
              case 4: type = DLRRReportBlock::kBlockType; break;
              case 5: type = StatisticsSummaryReportBlock::kBlockType; break;
              case 6: type = VoIPMetricsReportBlock::kBlockType; break;
              case 7: {
                type = static_cast<BYTE>(randomSize(VoIPMetricsReportBlock::kBlockType+1, 0xFF));
                break;
              }
              default:
              {
                TESTING_CHECK(false)
              }
            }

            switch (type) {
              case LossRLEReportBlock::kBlockType:                  {
                size_t &index = result->mLossRLEReportBlockCount;
                LossRLEReportBlock *first = result->mFirstLossRLEReportBlock;

                auto current = (&(first[index]));
                useBlock = current;
                if (0 != index) {
                  (&(first[index-1]))->mNextLossRLE = current;
                }
                ++(index);

                fillRLEReportBlock(current);
                break;
              }
              case DuplicateRLEReportBlock::kBlockType:             {
                size_t &index = result->mDuplicateRLEReportBlockCount;
                DuplicateRLEReportBlock *first = result->mFirstDuplicateRLEReportBlock;

                auto current = (&(first[index]));
                useBlock = current;
                if (0 != index) {
                  (&(first[index-1]))->mNextDuplicateRLE = current;
                }
                ++(index);

                fillRLEReportBlock(current);
                break;
              }
              case PacketReceiptTimesReportBlock::kBlockType:       {
                size_t &index = result->mPacketReceiptTimesReportBlockCount;
                PacketReceiptTimesReportBlock *first = result->mFirstPacketReceiptTimesReportBlock;

                auto current = (&(first[index]));
                useBlock = current;
                if (0 != index) {
                  (&(first[index-1]))->mNextPacketReceiptTimesReportBlock = current;
                }
                ++(index);

                fillReportBlockRange(current);

                if (shouldPerform(80)) {
                  current->mReceiptTimeCount = random(1, 20);
                  current->mReceiptTimes = new DWORD[current->mReceiptTimeCount];
                  for (size_t innerCount = 0; innerCount < current->mReceiptTimeCount; ++innerCount) {
                    current->mReceiptTimes[innerCount] = randomDWORD();
                  }
                }
                break;
              }
              case ReceiverReferenceTimeReportBlock::kBlockType:    {
                size_t &index = result->mReceiverReferenceTimeReportBlockCount;
                ReceiverReferenceTimeReportBlock *first = result->mFirstReceiverReferenceTimeReportBlock;

                auto current = (&(first[index]));
                useBlock = current;
                if (0 != index) {
                  (&(first[index-1]))->mNextReceiverReferenceTimeReportBlock = current;
                }
                ++(index);

                current->mNTPTimestampMS = randomDWORD();
                current->mNTPTimestampLS = randomDWORD();
                break;
              }
              case DLRRReportBlock::kBlockType:                     {
                size_t &index = result->mDLRRReportBlockCount;
                DLRRReportBlock *first = result->mFirstDLRRReportBlock;

                auto current = (&(first[index]));
                useBlock = current;
                if (0 != index) {
                  (&(first[index-1]))->mNextDLRRReportBlock = current;
                }
                ++(index);

                if (shouldPerform(90)) {
                  current->mSubBlockCount = random(1, 20);
                  current->mSubBlocks = new DLRRReportBlock::SubBlock[current->mSubBlockCount];
                  for (size_t innerCount = 0; innerCount < current->mSubBlockCount; ++innerCount) {
                    auto sub = (&(current->mSubBlocks[innerCount]));
                    sub->mSSRC = randomDWORD();
                    sub->mLRR = randomDWORD();
                    sub->mDLRR = randomDWORD();
                  }
                }
                break;
              }
              case StatisticsSummaryReportBlock::kBlockType:      {
                size_t &index = result->mStatisticsSummaryReportBlockCount;
                StatisticsSummaryReportBlock *first = result->mFirstStatisticsSummaryReportBlock;

                auto current = (&(first[index]));
                useBlock = current;
                if (0 != index) {
                  (&(first[index-1]))->mNextStatisticsSummaryReportBlock = current;
                }
                ++(index);

                fillReportBlockRange(current);

                current->mTypeSpecific = (((shouldPerform(50)) ? 1 : 0) << 7) |
                                         (((shouldPerform(50)) ? 1 : 0) << 6) |
                                         (((shouldPerform(50)) ? 1 : 0) << 5) |
                                         (randomBYTE(2) << 3);  // note: could produce illegal "3" value at times

                if (current->lossReportFlag()) {
                  current->mLostPackets = randomDWORD();
                  current->mDupPackets = randomDWORD();
                }

                if (current->jitterFlag()) {
                  current->mMinJitter = randomDWORD();
                  current->mMaxJitter = randomDWORD();
                  current->mMeanJitter = randomDWORD();
                  current->mDevJitter = randomDWORD();
                }

                if ((current->ttlFlag()) ||
                    (current->hopLimitFlag())) {
                  current->mMinTTLOrHL = randomBYTE();
                  current->mMaxTTLOrHL = randomBYTE();
                  current->mMeanTTLOrHL = randomBYTE();
                  current->mDevTTLOrHL = randomBYTE();
                }
                break;
              }
              case VoIPMetricsReportBlock::kBlockType:          {
                size_t &index = result->mVoIPMetricsReportBlockCount;
                VoIPMetricsReportBlock *first = result->mFirstVoIPMetricsReportBlock;

                auto current = (&(first[index]));
                useBlock = current;
                if (0 != index) {
                  (&(first[index-1]))->mNextVoIPMetricsReportBlock = current;
                }
                ++(index);

                current->mSSRCOfSource = randomDWORD();

                current->mLossRate = randomBYTE();
                current->mDiscardRate = randomBYTE();
                current->mBurstDensity = randomBYTE();
                current->mGapDensity = randomBYTE();

                current->mBurstDuration = randomWORD();
                current->mGapDuration = randomWORD();
                current->mRoundTripDelay = randomWORD();
                current->mEndSystemDelay = randomWORD();

                current->mSignalLevel = randomBYTE();
                current->mNoiseLevel = randomBYTE();
                current->mRERL = randomBYTE();
                current->mGmin = randomBYTE();

                current->mRFactor = randomBYTE();
                current->mExtRFactor = randomBYTE();
                current->mMOSLQ = randomBYTE();
                current->mMOSCQ = randomBYTE();

                current->mRXConfig = randomBYTE();
                if (shouldPerform(10)) {
                  current->mReservedVoIP = randomBYTE();
                }
                current->mJBNominal = randomWORD();
                current->mJBMaximum = randomWORD();
                current->mJBAbsMax = randomWORD();
                break;
              }
              default:
              {
                size_t &index = result->mUnknownReportBlockCount;
                UnknownReportBlock *first = result->mFirstUnknownReportBlock;

                auto current = (&(first[index]));
                useBlock = current;
                if (0 != index) {
                  (&(first[index-1]))->mNextUnknownReportBlock = current;
                }
                ++(index);

                if (shouldPerform(90)) {
                  current->mTypeSpecificContentSize = randomSize(1, 100);
                  current->mTypeSpecificContents = new BYTE[current->mTypeSpecificContentSize];

                  randomizeBufferSequencial(const_cast<BYTE *>(current->mTypeSpecificContents), current->mTypeSpecificContentSize);
                }
                break;
              }
            }

            useBlock->mBlockType = type;
            if (NULL != lastBlock) {
              lastBlock->mNext = useBlock;
            }
            lastBlock = useBlock;

            if (NULL == result->mFirstReportBlock) {
              result->mFirstReportBlock = useBlock;
            }

            ZS_LOG_TRACE(slog("generated XR report block") + ZS_PARAM("type", useBlock->blockTypeToString()) + ZS_PARAM("type (number)", useBlock->blockType()))
          }

          delete [] randomBuffer;
          randomBuffer = NULL;

          return result;
        }

        //---------------------------------------------------------------------
        static UnknownReport *createUnknownReport()
        {
          UnknownReport *result = new UnknownReport;
          result->mPT = static_cast<BYTE>(randomSize(XR::kPayloadType+1, 0xFF));
          fillReport(result);

          if (shouldPerform(90)) {
            result->mSize = randomSize(1, 100);
            result->mPtr = new BYTE[result->mSize];

            randomizeBufferSequencial(const_cast<BYTE *>(result->mPtr), result->mSize);
          }
          return result;
        }

        //---------------------------------------------------------------------
        static TesterPtr create()
        {
          TesterPtr pThis(make_shared<Tester>());
          pThis->mThisWeak = pThis;
          return pThis;
        }

        //---------------------------------------------------------------------
        void load(
                  const SecureByteBlock &buffer,
                  bool willSucceed = true
                  )
        {
          AutoRecursiveLock lock(*this);
          mPacket = RTCPPacket::create(buffer);
          if (willSucceed) {
            TESTING_CHECK((bool)mPacket)
          } else {
            TESTING_CHECK(!(bool)mPacket)
          }
        }

        //---------------------------------------------------------------------
        void randomReports()
        {
          if (NULL != mGeneratedFirst) {
            clean(mGeneratedFirst);
            mGeneratedFirst = NULL;
          }
          size_t count = randomSize(1, 10);

          Report *first = NULL;

          Report *lastReport = NULL;
          for (size_t index = 0; index < count; ++index) {
            Report *current = NULL;

            switch (randomSize(8)) {
              case 0: current = createSenderReport(); break;
              case 1: current = createReceiverReport(); break;
              case 2: current = createSDES(); break;
              case 3: current = createBye(); break;
              case 4: current = createApp(); break;
              case 5: current = createTransportLayerFeedbackMessage(); break;
              case 6: current = createPayloadSpecificFeedbackMessage(); break;
              case 7: current = createXR(); break;
              case 8: current = createUnknownReport(); break;
              default:
              {
                TESTING_CHECK(false)
                break;
              }
            }

            ZS_LOG_TRACE(log("generated report") + ZS_PARAM("pt", current->ptToString()) + ZS_PARAM("pt (number)", current->pt()))

            if (NULL == first) {
              first = current;
            }
            if (NULL != lastReport) {
              lastReport->mNext = current;
            }
            lastReport = current;
          }

          if (shouldPerform(10)) {
            lastReport->mPadding = static_cast<decltype(lastReport->mPadding)>(randomSize(1, 50));
            size_t outputSize = CheckRTCPPcaket::checkOutputSize(lastReport);
            outputSize += static_cast<size_t>(lastReport->mPadding);
            if ((outputSize / sizeof(DWORD)) > 0xFFFF) {
              // disable padding if packet is already too big
              lastReport->mPadding = 0;
            }
          }

          mGeneratedFirst = first;
        }

        //---------------------------------------------------------------------
        void generateBuffer()
        {
          if (NULL == mGeneratedFirst) randomReports();
          TESTING_CHECK(NULL != mGeneratedFirst)
          mGeneratedBuffer = RTCPPacket::generateFrom(mGeneratedFirst);
        }

        //---------------------------------------------------------------------
        void generatePacket()
        {
          if (!mGeneratedBuffer) generateBuffer();
          TESTING_CHECK(mGeneratedBuffer)
          mPacket = RTCPPacket::create(*mGeneratedBuffer, mGeneratedBuffer->SizeInBytes());
        }

        //---------------------------------------------------------------------
        void compare(const SecureByteBlock &buffer)
        {
          AutoRecursiveLock lock(*this);
          TESTING_CHECK((bool)mPacket)

          TESTING_EQUAL(0, UseServicesHelper::compare(buffer, *(mPacket->buffer())))
        }

        //---------------------------------------------------------------------
        void compare()
        {
          if (!mPacket) generatePacket();

          TESTING_CHECK((bool)mPacket)

          if (!mGeneratedBuffer) {
            generateBuffer();
          }

          TESTING_CHECK((bool)mGeneratedBuffer)
          compare(*mGeneratedBuffer);

          TESTING_CHECK(NULL != mGeneratedFirst)

          checkSanity(*mPacket);
          compare(mPacket->first(), mGeneratedFirst);
        }

        //---------------------------------------------------------------------
        RTCPPacketPtr getPacket()
        {
          AutoRecursiveLock lock(*this);
          return mPacket;
        }

      protected:
        AutoPUID mID;
        TesterWeakPtr mThisWeak;

        Report *mGeneratedFirst {};
        SecureByteBlockPtr mGeneratedBuffer;
        RTCPPacketPtr mPacket;
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

#define TEST_BASIC_RTCP 0

ZS_DECLARE_USING_PTR(ortc::test::rtcppacket, Tester)
ZS_DECLARE_USING_PTR(ortc::internal, RTCPPacket)

static signed gSeeds[] = {2286, 2075, -57479, 1000, 1001, -17, 89, -97, 523, 104297, -1269287, 0};

using namespace ortc::test;

void doTestRTCPPacket()
{
  if (!ORTC_TEST_DO_RTCP_PACKET_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  UseSettings::applyDefaults();

  auto thread(zsLib::IMessageQueueThread::createBasic());

  TesterPtr testObject1;
  TesterPtr testObject2;

  TESTING_STDOUT() << "WAITING:      Waiting for RTCP packet testing to complete (max wait is 180 seconds).\n";

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
        case TEST_BASIC_RTCP: {
          {
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
          case TEST_BASIC_RTCP: {
            switch (step) {
              case 1: {
                for (size_t index = 0; 0 != gSeeds[index]; ++index) {
                  TESTING_STDOUT() << "\n\nTESTING SEED: [" << gSeeds[index] << "].\n\n";
                  ZS_LOG_BASIC(Tester::slog("testing seed") + ZS_PARAM("seed", gSeeds[index]))

                  srand(static_cast<unsigned>(gSeeds[index]));  // want fixed random seeds that guarentee API coverage

                  testObject1 = Tester::create();
                  testObject2 = Tester::create();

                  TESTING_CHECK(testObject1)

                  testObject1->compare();
                  testObject2->compare();

                  testObject1.reset();
                  testObject2.reset();
                }
                break;
              }
              case 2: {
                for (size_t index = 0; index < 500; ++index) {
                  TESTING_STDOUT() << "\n\nTESTING SEED: [" << index << "].\n\n";
                  ZS_LOG_BASIC(Tester::slog("testing seed (part 2)") + ZS_PARAM("seed", index))

                  srand(static_cast<unsigned>(index));  // want fixed random seeds that guarentee API coverage

                  testObject1 = Tester::create();
                  testObject2 = Tester::create();

                  TESTING_CHECK(testObject1)

                  testObject1->compare();
                  testObject2->compare();

                  testObject1.reset();
                  testObject2.reset();
                }
                break;
              }
              case 3: {
                reachedFinalStep = true;
                break;
              }
              case 4: {
                break;
              }
              case 5: {
                break;
              }
              case 7: {
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

  TESTING_STDOUT() << "WAITING:      All RTCP packet tests have finished. Waiting for 'bogus' events to process (1 second wait).\n";
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

  srand(static_cast<unsigned>(time(NULL))); // put seed back to current time
}
