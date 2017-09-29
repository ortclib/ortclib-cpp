/*

 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/RTCPPacket.h>
#include <ortc/internal/ortc.events.h>

#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_RTPUtils.h>
#include <ortc/internal/platform.h>

#include <ortc/services/IHelper.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cstddef>

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG

#define RTCP_IS_FLAG_SET(xByte, xBitPos) (0 != ((xByte) & (1 << xBitPos)))
#define RTCP_GET_BITS(xByte, xBitPattern, xLowestBit) (((xByte) >> (xLowestBit)) & (xBitPattern))
#define RTCP_PACK_BITS(xByte, xBitPattern, xLowestBit) (((xByte) & (xBitPattern)) << (xLowestBit))

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_rtp_rtcp_packet) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper);
  ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper);
  ZS_DECLARE_TYPEDEF_PTR(ortc::internal::RTPUtils, UseRTPUtils);

  using zsLib::PTRNUMBER;

//  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP)
//
//  typedef ortc::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    static const size_t kMinRtcpPacketLen = 4;
    static const BYTE kRtpVersion = 2;

    //-------------------------------------------------------------------------
    static Log::Params packet_slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::RTCPPacket");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    static size_t alignedSize(size_t size)
    {
      size_t modulas = size % alignof(std::max_align_t);

      return size + (0 == modulas ? 0 : (alignof(std::max_align_t) - modulas));
    }

    //-------------------------------------------------------------------------
    static void advancePos(BYTE * &ioPos, size_t &ioRemaining, size_t length = 1)
    {
      ASSERT(ioRemaining >= length)

      ioPos += length;
      ioRemaining -= length;
    }

    //-------------------------------------------------------------------------
    static void advancePos(const BYTE * &ioPos, size_t &ioRemaining, size_t length = 1)
    {
      ASSERT(ioRemaining >= length)

      ioPos += length;
      ioRemaining -= length;
    }
    
    //-------------------------------------------------------------------------
    static void *allocateBuffer(BYTE * &ioAllocationBuffer, size_t &ioRemaining, size_t size)
    {
      size = alignedSize(size);

      void *result = ioAllocationBuffer;

      ioAllocationBuffer += size;
      ASSERT(ioRemaining >= size)
      ioRemaining -= size;

      return result;
    }

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

    //-------------------------------------------------------------------------
    static bool throwIfGreaterThanBitsAllow(
                                            size_t value,
                                            size_t maxBits
                                            )
    {
      ASSERT(value <= static_cast<size_t>(((1 << maxBits)-1)))
      ORTC_THROW_INVALID_PARAMETERS_IF(value > static_cast<size_t>(((1 << maxBits)-1)))
      return true;
    }

    //-------------------------------------------------------------------------
    static bool throwIfLessThan(
                                size_t size,
                                size_t min
                                )
    {
      ASSERT(size >= min)
      ORTC_THROW_INVALID_PARAMETERS_IF(size < min)
      return true;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCPPacket (writing sizing routines)
    #pragma mark

    //-------------------------------------------------------------------------
    static size_t getPacketSizeSenderReport(const RTCPPacket::SenderReport *report)
    {
      auto rc = report->rc();
      throwIfGreaterThanBitsAllow(rc, 5);
      return (sizeof(DWORD)*2) + (sizeof(DWORD)*5) + ((sizeof(DWORD)*6)*rc) + boundarySize(report->extensionSize());
    }

    //-------------------------------------------------------------------------
    static size_t getPacketSizeReceiverReport(const RTCPPacket::ReceiverReport *report)
    {
      auto rc = report->rc();
      throwIfGreaterThanBitsAllow(rc, 5);
      return (sizeof(DWORD)*2) + ((sizeof(DWORD)*6)*rc) + boundarySize(report->extensionSize());
    }

    //-------------------------------------------------------------------------
    static size_t getPacketSizeSDES(const RTCPPacket::SDES *report)
    {
      typedef RTCPPacket::SDES::Chunk Chunk;

      size_t result = (sizeof(DWORD));

      size_t chunkCount = 0;

      for (Chunk *chunk = report->firstChunk(); NULL != chunk; chunk = chunk->next(), ++chunkCount)
      {
        size_t chunkSize = sizeof(DWORD);

        for (auto *item = chunk->firstCName(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
          ZS_LOG_INSANE(packet_slog("get packet SDES CName size") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstName(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
          ZS_LOG_INSANE(packet_slog("get packet SDES Name size") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstEmail(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
          ZS_LOG_INSANE(packet_slog("get packet SDES Email size") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstPhone(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
          ZS_LOG_INSANE(packet_slog("get packet SDES Phone size") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstLoc(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
          ZS_LOG_INSANE(packet_slog("get packet SDES Loc size") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstTool(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
          ZS_LOG_INSANE(packet_slog("get packet SDES Tool size") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstNote(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
          ZS_LOG_INSANE(packet_slog("get packet SDES Note size") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstPriv(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len1 = (NULL != item->mValue ? strlen(item->mValue) : 0);
          size_t len2 = (NULL != item->mPrefix ? strlen(item->mPrefix) : 0);

          size_t totalLen = len1 + len2;
          if (0 != totalLen) ++totalLen;

          throwIfGreaterThanBitsAllow(totalLen, 8);
          chunkSize += ((sizeof(BYTE))*totalLen);
          ZS_LOG_INSANE(packet_slog("get packet SDES Priv size") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("prefix len", len2) + ZS_PARAM("prefix", (NULL != item->mPrefix ? item->mPrefix : NULL)) + ZS_PARAM("len", len1) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstMid(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
          ZS_LOG_INSANE(packet_slog("get packet SDES Mid size") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstRid(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
          ZS_LOG_INSANE(packet_slog("get packet SDES Rid size") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }
        for (auto *item = chunk->firstUnknown(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
          ZS_LOG_INSANE(packet_slog("get packet SDES Unknown size") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        size_t modulas = chunkSize % sizeof(DWORD);

        if ((chunkSize == sizeof(DWORD)) ||
            (0 == modulas)) {
          chunkSize += sizeof(DWORD);
        }

        result += boundarySize(chunkSize);
      }

      throwIfGreaterThanBitsAllow(chunkCount, 5);
      return result;
    }

    //-------------------------------------------------------------------------
    static size_t getPacketSizeBye(const RTCPPacket::Bye *report)
    {
      auto sc = report->sc();
      throwIfGreaterThanBitsAllow(sc, 5);

      size_t result = (sizeof(DWORD)) + (sizeof(DWORD)*sc);

      if (NULL != report->reasonForLeaving()) {
        size_t len = strlen(report->reasonForLeaving());
        if (len > 0) {
          throwIfGreaterThanBitsAllow(len, 8);
          result += sizeof(BYTE)+(sizeof(BYTE)*len);
        }
      }

      return boundarySize(result);
    }

    //-------------------------------------------------------------------------
    static size_t getPacketSizeApp(const RTCPPacket::App *report)
    {
      size_t result = (sizeof(DWORD)*3);

      auto dataSize = report->dataSize();
      if (0 != dataSize) {
        ORTC_THROW_INVALID_PARAMETERS_IF(NULL == report->mData)
        result += dataSize;
      }

      return boundarySize(result);
    }
    
    //-------------------------------------------------------------------------
    static size_t getPacketSizeTransportLayerFeedbackMessage(const RTCPPacket::TransportLayerFeedbackMessage *fm)
    {
      //typedef RTCPPacket::TransportLayerFeedbackMessage TransportLayerFeedbackMessage;
      typedef RTCPPacket::TransportLayerFeedbackMessage::GenericNACK GenericNACK;
      typedef RTCPPacket::TransportLayerFeedbackMessage::TMMBR TMMBR;
      typedef RTCPPacket::TransportLayerFeedbackMessage::TMMBN TMMBN;

      size_t result = (sizeof(DWORD)*3);

      switch (fm->fmt()) {
        case GenericNACK::kFmt:
        {
          auto count = fm->genericNACKCount();
          throwIfLessThan(count, 1);
          result += ((sizeof(DWORD))*(count));
          break;
        }
        case TMMBR::kFmt:
        {
          auto count = fm->tmmbrCount();
          throwIfLessThan(count, 1);
          result += ((sizeof(DWORD)*2)*(count));
          break;
        }
        case TMMBN::kFmt:
        {
          auto count = fm->tmmbnCount();
          result += ((sizeof(DWORD)*2)*(count));
          break;
        }
        default:
        {
          auto fciSize = fm->fciSize();
          if (0 != fciSize) {
            ORTC_THROW_INVALID_PARAMETERS_IF(NULL == fm->fci())
            result += fciSize;
          }
          break;
        }
      }

      ZS_LOG_INSANE(packet_slog("getting transport layer feedback message packet size") + ZS_PARAM("fmt", fm->fmtToString()) + ZS_PARAM("fmt (number)", fm->reportSpecific()) + ZS_PARAM("size", result))

      return boundarySize(result);
    }

    //-------------------------------------------------------------------------
    static size_t getPacketSizePayloadSpecificFeedbackMessage(const RTCPPacket::PayloadSpecificFeedbackMessage *fm)
    {
      //typedef RTCPPacket::PayloadSpecificFeedbackMessage PayloadSpecificFeedbackMessage;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::PLI PLI;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::SLI SLI;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::RPSI RPSI;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::FIR FIR;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::TSTR TSTR;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::TSTN TSTN;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::VBCM VBCM;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::AFB AFB;
      //typedef RTCPPacket::PayloadSpecificFeedbackMessage::REMB REMB;

      size_t result = (sizeof(DWORD)*3);

      switch (fm->fmt()) {
        case PLI::kFmt:
        {
          break;
        }
        case SLI::kFmt:
        {
          auto count = fm->sliCount();
          throwIfLessThan(count, 1);
          result += (sizeof(DWORD)*(count));
          break;
        }
        case RPSI::kFmt:
        {
          auto rpsi = fm->rpsi();
          ORTC_THROW_INVALID_PARAMETERS_IF(NULL == rpsi)
          result += sizeof(WORD) + (rpsi->nativeRPSIBitStringSizeInBits()/8);
          if (0 != (rpsi->nativeRPSIBitStringSizeInBits()%8)) ++result;
          break;
        }
        case FIR::kFmt:
        {
          auto count = fm->firCount();
          throwIfLessThan(count, 1);
          result += ((sizeof(DWORD)*2)*(count));
          break;
        }
        case TSTR::kFmt:
        {
          auto count = fm->tstrCount();
          throwIfLessThan(count, 1);
          result += ((sizeof(DWORD)*2)*(count));
          break;
        }
        case TSTN::kFmt:
        {
          auto count = fm->tstnCount();
          throwIfLessThan(count, 1);
          result += ((sizeof(DWORD)*2)*(count));
          break;
        }
        case VBCM::kFmt:
        {
          auto count = fm->vbcmCount();
          throwIfLessThan(count, 1);
          for (size_t index = 0; index < count; ++index)
          {
            auto vbcm = fm->vbcmAtIndex(index);
            ORTC_THROW_INVALID_PARAMETERS_IF(NULL == vbcm)

            size_t size = (sizeof(DWORD)*2) + (vbcm->vbcmOctetStringSize()*sizeof(BYTE));
            result += boundarySize(size);
          }
          break;
        }
        case AFB::kFmt:
        {
          auto remb = fm->remb();
          if (NULL == remb) {
            auto afb = fm->afb();
            ORTC_THROW_INVALID_PARAMETERS_IF(NULL == afb)

            result += afb->dataSize();
          } else {
            result += (sizeof(DWORD)*2);
            size_t numSSRCs = remb->numSSRC();
            throwIfGreaterThanBitsAllow(numSSRCs, 8);
            throwIfLessThan(numSSRCs, 1);
            result += ((sizeof(DWORD))*numSSRCs);
          }
          break;
        }
        default:
        {
          auto fciSize = fm->fciSize();
          if (0 != fciSize) {
            ORTC_THROW_INVALID_PARAMETERS_IF(NULL == fm->fci())
            result += fciSize;
          }
          break;
        }
      }

      ZS_LOG_INSANE(packet_slog("getting payload specific feedback message packet size") + ZS_PARAM("fmt", fm->fmtToString()) + ZS_PARAM("fmt (number)", fm->reportSpecific()) + ZS_PARAM("size", result))

      return boundarySize(result);
    }

    //-------------------------------------------------------------------------
    static size_t getPacketSizeXR(const RTCPPacket::XR *report)
    {
      //typedef RTCPPacket::XR XR;
      typedef RTCPPacket::XR::ReportBlock ReportBlock;
      typedef RTCPPacket::XR::LossRLEReportBlock LossRLEReportBlock;
      typedef RTCPPacket::XR::DuplicateRLEReportBlock DuplicateRLEReportBlock;
      typedef RTCPPacket::XR::PacketReceiptTimesReportBlock PacketReceiptTimesReportBlock;
      typedef RTCPPacket::XR::ReceiverReferenceTimeReportBlock ReceiverReferenceTimeReportBlock;
      typedef RTCPPacket::XR::DLRRReportBlock DLRRReportBlock;
      typedef RTCPPacket::XR::StatisticsSummaryReportBlock StatisticsSummaryReportBlock;
      typedef RTCPPacket::XR::VoIPMetricsReportBlock VoIPMetricsReportBlock;
      //typedef RTCPPacket::XR::UnknownReportBlock UnknownReportBlock;

      size_t result = (sizeof(DWORD)*2);

      for (const ReportBlock *reportBlock = report->firstReportBlock(); NULL != reportBlock; reportBlock = reportBlock->next())
      {
        size_t preSize = result;

        switch (reportBlock->mBlockType) {
          case LossRLEReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const LossRLEReportBlock *>(reportBlock);
            result += (sizeof(DWORD)*3);
            result += boundarySize(sizeof(WORD)*(block->chunkCount()));
            break;
          }
          case DuplicateRLEReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const DuplicateRLEReportBlock *>(reportBlock);
            result += (sizeof(DWORD)*3);
            result += boundarySize(sizeof(WORD)*(block->chunkCount()));
            break;
          }
          case PacketReceiptTimesReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const PacketReceiptTimesReportBlock *>(reportBlock);
            result += (sizeof(DWORD)*3);
            result += (sizeof(DWORD)*(block->receiptTimeCount()));
            break;
          }
          case ReceiverReferenceTimeReportBlock::kBlockType:
          {
            result += (sizeof(DWORD)*3);
            break;
          }
          case DLRRReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const DLRRReportBlock *>(reportBlock);
            result += (sizeof(DWORD));
            result += ((sizeof(DWORD)*3)*(block->subBlockCount()));
            break;
          }
          case StatisticsSummaryReportBlock::kBlockType:
          {
            result += (sizeof(DWORD)*10);
            break;
          }
          case VoIPMetricsReportBlock::kBlockType:
          {
            result += (sizeof(DWORD)*9);
            break;
          }
          default:
          {
            result += (sizeof(DWORD)) + boundarySize(reportBlock->mTypeSpecificContentSize);
          }
        }

        ZS_LOG_INSANE(packet_slog("XR block packet size") + ZS_PARAM("block type", reportBlock->blockTypeToString()) + ZS_PARAM("block type (number)", reportBlock->blockType()) + ZS_PARAM("size", result - preSize))
      }

      return boundarySize(result);
    }

    //-------------------------------------------------------------------------
    static void writePacketUnknown(const RTCPPacket::UnknownReport *report, BYTE * &pos, size_t &remaining)
    {
      //typedef RTCPPacket::UnknownReport UnknownReport;
      pos[1] = report->pt();

      advancePos(pos, remaining, sizeof(DWORD));

      auto size = report->size();
      if (0 == size) return;

      ASSERT(NULL != report->ptr());

      memcpy(pos, report->ptr(), size);

      advancePos(pos, remaining, size);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCPPacket (packet writing routines)
    #pragma mark

    //-------------------------------------------------------------------------
    static void writePacketHeader(const RTCPPacket::Report *report, BYTE * &pos, size_t &remaining)
    {
      ASSERT(remaining >= sizeof(DWORD))
      ASSERT(2 == report->version())

      auto padding = report->padding();
      if (0 != padding) {
        ASSERT(NULL == report->next())
        ASSERT(throwIfGreaterThanBitsAllow(padding, 8))
      }

      ASSERT(throwIfGreaterThanBitsAllow(report->reportSpecific(), 5))

      pos[0] = RTCP_PACK_BITS(report->version(), 0x3, 6) |
                ((0 != padding) ? RTCP_PACK_BITS(1, 0x1, 5) : 0) |
                RTCP_PACK_BITS(report->reportSpecific(), 0x1F, 0);
      pos[1] = report->pt();
    }

    //-------------------------------------------------------------------------
    static void writePacketSenderReceiverCommonReport(const RTCPPacket::SenderReceiverCommonReport *report, BYTE * &pos, size_t &remaining)
    {
      //typedef RTCPPacket::SenderReceiverCommonReport SenderReceiverCommonReport;
      typedef RTCPPacket::SenderReceiverCommonReport::ReportBlock ReportBlock;

      size_t count = 0;
      for (const ReportBlock *block = report->firstReportBlock(); NULL != block; block = block->next(), ++count)
      {
        ASSERT(count < report->rc());

        UseRTPUtils::setBE32(&(pos[0]), block->ssrc());
        ASSERT(throwIfGreaterThanBitsAllow(block->cumulativeNumberOfPacketsLost(), 24));
        UseRTPUtils::setBE32(&(pos[4]), block->cumulativeNumberOfPacketsLost());
        pos[4] = block->fractionLost();
        UseRTPUtils::setBE32(&(pos[8]), block->extendedHighestSequenceNumberReceived());
        UseRTPUtils::setBE32(&(pos[12]), block->interarrivalJitter());
        UseRTPUtils::setBE32(&(pos[16]), block->lsr());
        UseRTPUtils::setBE32(&(pos[20]), block->dlsr());

        advancePos(pos, remaining, sizeof(DWORD)*6);
      }

      ASSERT(count == report->rc())

      if (report->extensionSize() > 0) {
        ASSERT(NULL != report->extension())
        memcpy(pos, report->extension(), report->extensionSize());
        advancePos(pos, remaining, boundarySize(report->extensionSize()));
      }
    }
    
    //-------------------------------------------------------------------------
    static void writePacketSenderReport(const RTCPPacket::SenderReport *report, BYTE * &pos, size_t &remaining)
    {
      typedef RTCPPacket::SenderReport SenderReport;
      pos[1] = SenderReport::kPayloadType;

      UseRTPUtils::setBE32(&(pos[4]), report->ssrcOfSender());
      UseRTPUtils::setBE32(&(pos[8]), report->ntpTimestampMS());
      UseRTPUtils::setBE32(&(pos[12]), report->ntpTimestampLS());
      UseRTPUtils::setBE32(&(pos[16]), report->rtpTimestamp());
      UseRTPUtils::setBE32(&(pos[20]), report->senderPacketCount());
      UseRTPUtils::setBE32(&(pos[24]), report->senderOctetCount());

      advancePos(pos, remaining, sizeof(DWORD)*7);

      writePacketSenderReceiverCommonReport(report, pos, remaining);
    }

    //-------------------------------------------------------------------------
    static void writePacketReceiverReport(const RTCPPacket::ReceiverReport *report, BYTE * &pos, size_t &remaining)
    {
      typedef RTCPPacket::ReceiverReport ReceiverReport;
      pos[1] = ReceiverReport::kPayloadType;

      UseRTPUtils::setBE32(&(pos[4]), report->ssrcOfPacketSender());

      advancePos(pos, remaining, sizeof(DWORD)*2);

      writePacketSenderReceiverCommonReport(report, pos, remaining);
    }
    
    //-------------------------------------------------------------------------
    static void writePacketSDES(const RTCPPacket::SDES *report, BYTE * &pos, size_t &remaining)
    {
      typedef RTCPPacket::SDES SDES;
      typedef RTCPPacket::SDES::Chunk Chunk;

      pos[1] = SDES::kPayloadType;

      advancePos(pos, remaining, sizeof(DWORD));

      size_t chunkCount = 0;

      for (Chunk *chunk = report->firstChunk(); NULL != chunk; chunk = chunk->next(), ++chunkCount)
      {
        UseRTPUtils::setBE32(pos, chunk->ssrc());
        advancePos(pos, remaining, sizeof(DWORD));

        BYTE *startPos = pos;

        for (auto *item = chunk->firstCName(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::CName::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = static_cast<BYTE>(len);
          ASSERT(throwIfGreaterThanBitsAllow(len, 8))

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
          ZS_LOG_INSANE(packet_slog("writing SDES CName") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstName(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Name::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = static_cast<BYTE>(len);
          ASSERT(throwIfGreaterThanBitsAllow(len, 8))

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
          ZS_LOG_INSANE(packet_slog("writing SDES Name") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstEmail(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Email::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = static_cast<BYTE>(len);
          ASSERT(throwIfGreaterThanBitsAllow(len, 8));

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
          ZS_LOG_INSANE(packet_slog("writing SDES Email") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstPhone(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Phone::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = static_cast<BYTE>(len);
          ASSERT(throwIfGreaterThanBitsAllow(len, 8));

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
          ZS_LOG_INSANE(packet_slog("writing SDES Phone") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstLoc(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Loc::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = static_cast<BYTE>(len);
          ASSERT(throwIfGreaterThanBitsAllow(len, 8));

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
          ZS_LOG_INSANE(packet_slog("writing SDES Loc") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstTool(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Tool::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = static_cast<BYTE>(len);
          ASSERT(throwIfGreaterThanBitsAllow(len, 8));

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
          ZS_LOG_INSANE(packet_slog("writing SDES Tool") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstNote(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Note::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = static_cast<BYTE>(len);
          ASSERT(throwIfGreaterThanBitsAllow(len, 8));

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
          ZS_LOG_INSANE(packet_slog("writing SDES Note") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstPriv(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Priv::kItemType;
          size_t len1 = (NULL != item->mPrefix ? strlen(item->mPrefix) : 0);
          size_t len2 = (NULL != item->mValue ? strlen(item->mValue) : 0);
          size_t len = len1 + len2;
          if (len > 0) ++len;
          pos[1] = static_cast<BYTE>(len);
          ASSERT(throwIfGreaterThanBitsAllow(len, 8));

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            pos[0] = static_cast<BYTE>(len1);
            advancePos(pos, remaining);
            if (len1 > 0) {
              memcpy(pos, item->prefix(), len1*sizeof(BYTE));
              advancePos(pos, remaining, len1*sizeof(BYTE));
            }
            if (len2 > 0) {
              memcpy(pos, item->value(), len2*sizeof(BYTE));
              advancePos(pos, remaining, len2*sizeof(BYTE));
            }
          }
          ZS_LOG_INSANE(packet_slog("writing SDES Priv") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("prefix len", len1) + ZS_PARAM("prefix", (NULL != item->mPrefix ? item->mPrefix : NULL)) + ZS_PARAM("len", len2) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstMid(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Mid::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = static_cast<BYTE>(len);
          ASSERT(throwIfGreaterThanBitsAllow(len, 8));

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
          ZS_LOG_INSANE(packet_slog("writing SDES Mid report") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstRid(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Rid::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = static_cast<BYTE>(len);
          ASSERT(throwIfGreaterThanBitsAllow(len, 8));

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
          ZS_LOG_INSANE(packet_slog("writing SDES Rid report") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        for (auto *item = chunk->firstUnknown(); NULL != item; item = item->next())
        {
          pos[0] = item->type();
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = static_cast<BYTE>(len);
          ASSERT(throwIfGreaterThanBitsAllow(len, 8));

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
          ZS_LOG_INSANE(packet_slog("writing SDES Unknown") + ZS_PARAM("chunk", chunkCount) + ZS_PARAM("len", len) + ZS_PARAM("type", item->mType) + ZS_PARAM("value", (NULL != item->mValue ? item->mValue : NULL)))
        }

        BYTE *endPos = pos;

        PTRNUMBER diff = reinterpret_cast<PTRNUMBER>(endPos) - reinterpret_cast<PTRNUMBER>(startPos);
        auto modulas = (diff % sizeof(DWORD));
        if (0 != modulas) {
          advancePos(pos, remaining, sizeof(DWORD)-modulas);
        }

        if ((0 == diff) ||
            (0 == modulas)) {
          // write "empty" chunk
          UseRTPUtils::setBE32(pos, 0);
          advancePos(pos, remaining, sizeof(DWORD));
        }
      }

      throwIfGreaterThanBitsAllow(chunkCount, 5);
    }
    
    //-------------------------------------------------------------------------
    static void writePacketBye(const RTCPPacket::Bye *report, BYTE * &pos, size_t &remaining)
    {
      typedef RTCPPacket::Bye Bye;
      pos[1] = Bye::kPayloadType;

      advancePos(pos, remaining, sizeof(DWORD));

      for (size_t index = 0; index < report->sc(); ++index)
      {
        UseRTPUtils::setBE32(pos, report->ssrc(index));
        advancePos(pos, remaining, sizeof(DWORD));
      }

      if (NULL != report->reasonForLeaving()) {
        size_t len = strlen(report->reasonForLeaving());
        if (len > 0) {
          pos[0] = static_cast<BYTE>(len);
          memcpy(&(pos[1]), report->reasonForLeaving(), len*sizeof(BYTE));
          advancePos(pos, remaining, (len*sizeof(BYTE))+sizeof(BYTE));
        }
      }
    }

    //-------------------------------------------------------------------------
    static void writePacketApp(const RTCPPacket::App *report, BYTE * &pos, size_t &remaining)
    {
      typedef RTCPPacket::App App;
      pos[1] = App::kPayloadType;

      UseRTPUtils::setBE32(&(pos[4]), report->ssrc());
      memcpy(&(pos[8]), report->name(), sizeof(DWORD));

      advancePos(pos, remaining, sizeof(DWORD)*3);

      size_t dataSize = report->dataSize();
      if (0 != dataSize) {
        memcpy(pos, report->data(), dataSize);
        advancePos(pos, remaining, dataSize);
      }
    }

    //-------------------------------------------------------------------------
    static void writePacketTransportLayerFeedbackMessage(const RTCPPacket::TransportLayerFeedbackMessage *report, BYTE * &pos, size_t &remaining)
    {
      typedef RTCPPacket::TransportLayerFeedbackMessage TransportLayerFeedbackMessage;
      typedef RTCPPacket::TransportLayerFeedbackMessage::GenericNACK GenericNACK;
      typedef RTCPPacket::TransportLayerFeedbackMessage::TMMBR TMMBR;
      typedef RTCPPacket::TransportLayerFeedbackMessage::TMMBN TMMBN;

      pos[1] = TransportLayerFeedbackMessage::kPayloadType;

      UseRTPUtils::setBE32(&(pos[4]), report->ssrcOfPacketSender());
      UseRTPUtils::setBE32(&(pos[8]), report->ssrcOfMediaSource());

      advancePos(pos, remaining, sizeof(DWORD)*3);

      switch (report->fmt()) {
        case GenericNACK::kFmt:
        {
          auto count = report->genericNACKCount();

          for (size_t index = 0; index < count; ++index)
          {
            auto item = report->genericNACKAtIndex(index);
            UseRTPUtils::setBE16(&(pos[0]), item->pid());
            UseRTPUtils::setBE16(&(pos[2]), item->blp());
            advancePos(pos, remaining, sizeof(DWORD));
          }
          break;
        }
        case TMMBR::kFmt:
        {
          auto count = report->tmmbrCount();

          for (size_t index = 0; index < count; ++index)
          {
            auto item = report->tmmbrAtIndex(index);
            UseRTPUtils::setBE32(&(pos[0]), item->ssrc());

            DWORD merged = RTCP_PACK_BITS(static_cast<DWORD>(item->mxTBRExp()), 0x3F, 26) |
                            RTCP_PACK_BITS(static_cast<DWORD>(item->mxTBRMantissa()), 0x1FFFF, 9) |
                            RTCP_PACK_BITS(static_cast<DWORD>(item->measuredOverhead()), 0x1FF, 0);

            UseRTPUtils::setBE32(&(pos[4]), merged);

            advancePos(pos, remaining, sizeof(DWORD)*2);
          }
          break;
        }
        case TMMBN::kFmt:
        {
          auto count = report->tmmbnCount();

          for (size_t index = 0; index < count; ++index)
          {
            auto item = report->tmmbnAtIndex(index);
            UseRTPUtils::setBE32(&(pos[0]), item->ssrc());

            DWORD merged = RTCP_PACK_BITS(static_cast<DWORD>(item->mxTBRExp()), 0x3F, 26) |
                            RTCP_PACK_BITS(static_cast<DWORD>(item->mxTBRMantissa()), 0x1FFFF, 9) |
                            RTCP_PACK_BITS(static_cast<DWORD>(item->measuredOverhead()), 0x1FF, 0);

            UseRTPUtils::setBE32(&(pos[4]), merged);

            advancePos(pos, remaining, sizeof(DWORD)*2);
          }
          break;
        }
        default:
        {
          auto fciSize = report->fciSize();
          if (0 != fciSize) {
            memcpy(pos, report->fci(), report->fciSize());
            advancePos(pos, remaining, report->fciSize());
          }
          break;
        }
      }

      ZS_LOG_INSANE(packet_slog("writing transport layer feedback message") + ZS_PARAM("fmt", report->fmtToString()) + ZS_PARAM("fmt (number)", report->reportSpecific()))
    }
    
    //-------------------------------------------------------------------------
    static void writePacketPayloadSpecificFeedbackMessage(const RTCPPacket::PayloadSpecificFeedbackMessage *report, BYTE * &pos, size_t &remaining)
    {
      typedef RTCPPacket::PayloadSpecificFeedbackMessage PayloadSpecificFeedbackMessage;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::PLI PLI;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::SLI SLI;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::RPSI RPSI;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::FIR FIR;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::TSTR TSTR;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::TSTN TSTN;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::VBCM VBCM;
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::AFB AFB;
      //typedef RTCPPacket::PayloadSpecificFeedbackMessage::REMB REMB;

      pos[1] = PayloadSpecificFeedbackMessage::kPayloadType;

      UseRTPUtils::setBE32(&(pos[4]), report->ssrcOfPacketSender());
      UseRTPUtils::setBE32(&(pos[8]), report->ssrcOfMediaSource());

      advancePos(pos, remaining, sizeof(DWORD)*3);

      switch (report->fmt()) {
        case PLI::kFmt:
        {
          break;
        }
        case SLI::kFmt:
        {
          auto count = report->sliCount();

          for (size_t index = 0; index < count; ++index)
          {
            auto item = report->sliAtIndex(index);

            DWORD merged = RTCP_PACK_BITS(static_cast<DWORD>(item->first()), 0x1FFF, 19) |
                            RTCP_PACK_BITS(static_cast<DWORD>(item->number()), 0x1FFF, 6) |
                            RTCP_PACK_BITS(static_cast<DWORD>(item->pictureID()), 0x3F, 0);

            UseRTPUtils::setBE32(&(pos[0]), merged);
            advancePos(pos, remaining, sizeof(DWORD));
          }
          break;
        }
        case RPSI::kFmt:
        {
          auto rpsi = report->rpsi();

          pos[1] = RTCP_PACK_BITS(static_cast<BYTE>(rpsi->zeroBit()), 0x1, 7) |
                    RTCP_PACK_BITS(static_cast<BYTE>(rpsi->payloadType()), 0x7F, 0);

          auto totalBits = static_cast<size_t>(rpsi->nativeRPSIBitStringSizeInBits());

          size_t byteModulas = (totalBits%8);

          size_t totalBytes = (totalBits/8) + ((0 != byteModulas) ? 1 : 0);
          if (0 != totalBytes) {
            memcpy(&(pos[2]), rpsi->nativeRPSIBitString(), totalBytes);
            if (0 != byteModulas) {
              BYTE &by = (pos[2+totalBytes-1]);
              by = by & static_cast<BYTE>(((1 << byteModulas)-1) << (8-byteModulas));
            }
          }

          size_t totalSizeInBits = (sizeof(WORD)*8)+(totalBits);
          auto boundaryModulas = totalSizeInBits % (sizeof(DWORD)*8);

          if (0 != boundaryModulas) {
            pos[0] = static_cast<BYTE>((sizeof(DWORD)*8)-boundaryModulas);
          } else {
            pos[0] = 0;
          }

          advancePos(pos, remaining, sizeof(WORD)+(sizeof(BYTE)*totalBytes));
          break;
        }
        case FIR::kFmt:
        {
          auto count = report->firCount();

          for (size_t index = 0; index < count; ++index)
          {
            auto item = report->firAtIndex(index);
            UseRTPUtils::setBE32(&(pos[0]), item->ssrc());
            UseRTPUtils::setBE32(&(pos[4]), item->reserved());
            pos[4] = item->seqNr();
            advancePos(pos, remaining, sizeof(DWORD)*2);
          }
          break;
        }
        case TSTR::kFmt:
        {
          auto count = report->tstrCount();

          for (size_t index = 0; index < count; ++index)
          {
            auto item = report->tstrAtIndex(index);
            UseRTPUtils::setBE32(&(pos[0]), item->ssrc());

            DWORD merged = RTCP_PACK_BITS(static_cast<DWORD>(item->seqNr()), 0xFF, 24) |
                            RTCP_PACK_BITS(static_cast<DWORD>(item->reserved()), 0x7FFFF, 5) |
                            RTCP_PACK_BITS(static_cast<DWORD>(item->index()), 0x1F, 0);

            UseRTPUtils::setBE32(&(pos[4]), merged);
            advancePos(pos, remaining, sizeof(DWORD)*2);
          }
          break;
        }
        case TSTN::kFmt:
        {
          auto count = report->tstnCount();

          for (size_t index = 0; index < count; ++index)
          {
            auto item = report->tstnAtIndex(index);
            UseRTPUtils::setBE32(&(pos[0]), item->ssrc());

            DWORD merged = RTCP_PACK_BITS(static_cast<DWORD>(item->seqNr()), 0xFF, 24) |
                            RTCP_PACK_BITS(static_cast<DWORD>(item->reserved()), 0x7FFFF, 5) |
                            RTCP_PACK_BITS(static_cast<DWORD>(item->index()), 0x1F, 0);

            UseRTPUtils::setBE32(&(pos[4]), merged);
            advancePos(pos, remaining, sizeof(DWORD)*2);
          }
          break;
        }
        case VBCM::kFmt:
        {
          auto count = report->vbcmCount();

          for (size_t index = 0; index < count; ++index)
          {
            auto item = report->vbcmAtIndex(index);

            UseRTPUtils::setBE32(&(pos[0]), item->ssrc());

            pos[4] = item->seqNr();
            pos[5] = RTCP_PACK_BITS(static_cast<BYTE>(item->zeroBit()), 0x1, 7) |
                      RTCP_PACK_BITS(static_cast<BYTE>(item->payloadType()), 0x7F, 0);

            auto size = item->vbcmOctetStringSize();
            if (0 != size) {
              UseRTPUtils::setBE16(&(pos[6]), static_cast<WORD>(size));
              memcpy(&(pos[8]), item->vbcmOctetString(), size*sizeof(BYTE));
            }

            advancePos(pos, remaining, boundarySize((sizeof(DWORD)*2)+(size*sizeof(BYTE))));
          }
          break;
        }
        case AFB::kFmt:
        {

          auto remb = report->remb();
          if (NULL == remb) {
            auto afb = report->afb();

            size_t size = afb->dataSize();
            if (0 != size) {
              memcpy(pos, afb->data(), size*sizeof(BYTE));
              advancePos(pos, remaining, boundarySize(size*sizeof(BYTE)));
            }
          } else {
            memcpy(pos, "REMB", sizeof(DWORD));

            DWORD merged = RTCP_PACK_BITS(static_cast<DWORD>(remb->brExp()), 0x3F, 18) |
                            RTCP_PACK_BITS(static_cast<DWORD>(remb->brMantissa()), 0x3FFFF, 0);
            UseRTPUtils::setBE32(&(pos[4]), merged);
            pos[4] = static_cast<BYTE>(remb->numSSRC());
            advancePos(pos, remaining, sizeof(DWORD)*2);

            auto count = remb->numSSRC();
            for (size_t index = 0; index < count; ++index)
            {
              auto ssrc = remb->ssrcAtIndex(index);
              UseRTPUtils::setBE32(pos, ssrc);
              advancePos(pos, remaining, sizeof(DWORD));
            }
          }
          break;
        }
        default:
        {
          auto fciSize = report->fciSize();
          if (0 != fciSize) {
            memcpy(pos, report->fci(), fciSize);
            advancePos(pos, remaining, fciSize);
          }
          break;
        }
      }
      ZS_LOG_INSANE(packet_slog("writing payload specific feedback message") + ZS_PARAM("fmt", report->fmtToString()) + ZS_PARAM("fmt (number)", report->reportSpecific()))
    }

    //-------------------------------------------------------------------------
    static void writePacketXR(const RTCPPacket::XR *report, BYTE * &pos, size_t &remaining)
    {
      typedef RTCPPacket::XR XR;
      typedef RTCPPacket::XR::ReportBlock ReportBlock;
      typedef RTCPPacket::XR::LossRLEReportBlock LossRLEReportBlock;
      typedef RTCPPacket::XR::DuplicateRLEReportBlock DuplicateRLEReportBlock;
      typedef RTCPPacket::XR::PacketReceiptTimesReportBlock PacketReceiptTimesReportBlock;
      typedef RTCPPacket::XR::ReceiverReferenceTimeReportBlock ReceiverReferenceTimeReportBlock;
      typedef RTCPPacket::XR::DLRRReportBlock DLRRReportBlock;
      typedef RTCPPacket::XR::StatisticsSummaryReportBlock StatisticsSummaryReportBlock;
      typedef RTCPPacket::XR::VoIPMetricsReportBlock VoIPMetricsReportBlock;
      typedef RTCPPacket::XR::UnknownReportBlock UnknownReportBlock;
      typedef RTCPPacket::XR::RLEChunk RLEChunk;

      pos[1] = XR::kPayloadType;
      UseRTPUtils::setBE32(&(pos[4]), report->mSSRC);

      advancePos(pos, remaining, sizeof(DWORD)*2);

      for (const ReportBlock *reportBlock = report->firstReportBlock(); NULL != reportBlock; reportBlock = reportBlock->next())
      {
        BYTE *blockStart = pos;

        pos[0] = reportBlock->blockType();
        pos[1] = reportBlock->typeSpecific();

        advancePos(pos, remaining, sizeof(DWORD));

        switch (reportBlock->mBlockType) {
          case LossRLEReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const LossRLEReportBlock *>(reportBlock);

            UseRTPUtils::setBE32(&(pos[0]), block->ssrcOfSource());
            UseRTPUtils::setBE16(&(pos[4]), block->beginSeq());
            UseRTPUtils::setBE16(&(pos[6]), block->endSeq());

            advancePos(pos, remaining, sizeof(DWORD)*2);

            size_t chunkCount = block->chunkCount();
            for (size_t index = 0; index < chunkCount; ++index)
            {
              RLEChunk chunk = block->chunkAtIndex(index);
              UseRTPUtils::setBE16(pos, chunk);
              advancePos(pos, remaining, sizeof(WORD));
            }
            break;
          }
          case DuplicateRLEReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const DuplicateRLEReportBlock *>(reportBlock);

            UseRTPUtils::setBE32(&(pos[0]), block->ssrcOfSource());
            UseRTPUtils::setBE16(&(pos[4]), block->beginSeq());
            UseRTPUtils::setBE16(&(pos[6]), block->endSeq());

            advancePos(pos, remaining, sizeof(DWORD)*2);

            size_t count = block->chunkCount();
            for (size_t index = 0; index < count; ++index)
            {
              RLEChunk chunk = block->chunkAtIndex(index);
              UseRTPUtils::setBE16(pos, chunk);
              advancePos(pos, remaining, sizeof(WORD));
            }
            break;
          }
          case PacketReceiptTimesReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const PacketReceiptTimesReportBlock *>(reportBlock);

            UseRTPUtils::setBE32(&(pos[0]), block->ssrcOfSource());
            UseRTPUtils::setBE16(&(pos[4]), block->beginSeq());
            UseRTPUtils::setBE16(&(pos[6]), block->endSeq());

            advancePos(pos, remaining, sizeof(DWORD)*2);

            size_t count = block->receiptTimeCount();
            for (size_t index = 0; index < count; ++index)
            {
              DWORD receiptTime = block->receiptTimeAtIndex(index);
              UseRTPUtils::setBE32(pos, receiptTime);
              advancePos(pos, remaining, sizeof(DWORD));
            }
            break;
          }
          case ReceiverReferenceTimeReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const ReceiverReferenceTimeReportBlock *>(reportBlock);
            UseRTPUtils::setBE32(&(pos[0]), block->ntpTimestampMS());
            UseRTPUtils::setBE32(&(pos[4]), block->ntpTimestampLS());
            advancePos(pos, remaining, sizeof(DWORD)*2);
            break;
          }
          case DLRRReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const DLRRReportBlock *>(reportBlock);

            size_t count = block->subBlockCount();
            for (size_t index = 0; index < count; ++index)
            {
              DLRRReportBlock::SubBlock *subBlock = block->subBlockAtIndex(index);
              UseRTPUtils::setBE32(&(pos[0]), subBlock->ssrc());
              UseRTPUtils::setBE32(&(pos[4]), subBlock->lrr());
              UseRTPUtils::setBE32(&(pos[8]), subBlock->dlrr());
              advancePos(pos, remaining, sizeof(DWORD)*3);
            }
            break;
          }
          case StatisticsSummaryReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const StatisticsSummaryReportBlock *>(reportBlock);
            UseRTPUtils::setBE32(&(pos[0]), block->ssrcOfSource());
            UseRTPUtils::setBE16(&(pos[4]), block->beginSeq());
            UseRTPUtils::setBE16(&(pos[6]), block->endSeq());
            UseRTPUtils::setBE32(&(pos[8]), block->lostPackets());
            UseRTPUtils::setBE32(&(pos[12]), block->dupPackets());
            UseRTPUtils::setBE32(&(pos[16]), block->minJitter());
            UseRTPUtils::setBE32(&(pos[20]), block->maxJitter());
            UseRTPUtils::setBE32(&(pos[24]), block->meanJitter());
            UseRTPUtils::setBE32(&(pos[28]), block->devJitter());
            pos[32] = block->mMinTTLOrHL;
            pos[33] = block->mMaxTTLOrHL;
            pos[34] = block->mMeanTTLOrHL;
            pos[35] = block->mDevTTLOrHL;
            advancePos(pos, remaining, sizeof(DWORD)*9);
            break;
          }
          case VoIPMetricsReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const VoIPMetricsReportBlock *>(reportBlock);
            UseRTPUtils::setBE32(&(pos[0]), block->ssrcOfSource());
            pos[4] = block->lossRate();
            pos[5] = block->discardRate();
            pos[6] = block->burstDensity();
            pos[7] = block->gapDensity();
            UseRTPUtils::setBE16(&(pos[8]), block->burstDuration());
            UseRTPUtils::setBE16(&(pos[10]), block->gapDuration());
            UseRTPUtils::setBE16(&(pos[12]), block->roundTripDelay());
            UseRTPUtils::setBE16(&(pos[14]), block->endSystemDelay());
            pos[16] = block->signalLevel();
            pos[17] = block->noiseLevel();
            pos[18] = block->rerl();
            pos[19] = block->Gmin();

            pos[20] = block->rFactor();
            pos[21] = block->extRFactor();
            pos[22] = block->mosLQ();
            pos[23] = block->mosCQ();

            pos[24] = block->rxConfig();
            pos[25] = block->mReservedVoIP;
            UseRTPUtils::setBE16(&(pos[26]), block->jbNominal());
            UseRTPUtils::setBE16(&(pos[28]), block->jbMaximum());
            UseRTPUtils::setBE16(&(pos[30]), block->jbAbsMax());
            advancePos(pos, remaining, sizeof(DWORD)*8);
            break;
          }
          default:
          {
            auto block = reinterpret_cast<const UnknownReportBlock *>(reportBlock);

            auto size = block->typeSpecificContentSize();
            if (0 != size) {
              memcpy(pos, block->typeSpecificContents(), size*sizeof(BYTE));
              advancePos(pos, remaining, boundarySize(size*sizeof(BYTE)));
            }
          }
        }

        BYTE *blockEnd = pos;

        size_t diff = static_cast<size_t>(reinterpret_cast<PTRNUMBER>(blockEnd) - reinterpret_cast<PTRNUMBER>(blockStart));
        auto modulas = diff % sizeof(DWORD);
        if (0 != modulas) {
          advancePos(pos, remaining, sizeof(DWORD)-modulas);
        }

        diff = boundarySize(diff);
        ZS_LOG_INSANE(packet_slog("writing XR block") + ZS_PARAM("block type", reportBlock->blockTypeToString()) + ZS_PARAM("block type (number)", reportBlock->blockType()) + ZS_PARAM("block size", diff))

        UseRTPUtils::setBE16(&(blockStart[2]), static_cast<WORD>((diff/sizeof(DWORD))-1));
      }
    }

    //-------------------------------------------------------------------------
    static void traceReport(
                            IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                            RTCPPacket::Report *common
                            )
    {
      ZS_EVENTING_8(x, i, Insane, RTCPPacketTraceReport, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        bool, next, NULL != common->next(),
        buffer, buffer, common->ptr(),
        size, size, common->size(),
        byte, version, common->version(),
        size_t, padding, common->padding(),
        byte, reportSpecific, common->reportSpecific(),
        byte, payloadType, common->pt()
      );
    }

    //-------------------------------------------------------------------------
    static void traceReportBlocks(
                                  IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                  RTCPPacket::SenderReceiverCommonReport *common
                                  )
    {
      RTCPPacket::SenderReceiverCommonReport::ReportBlock *block = common->firstReportBlock();
      if (NULL == block) return;

      for (; NULL != block; block = block->next()) {
        ZS_EVENTING_8(x, i, Insane, RTCPPacketTraceSenderReceiverCommonReportBlock, ol, RtcpPacket, Info,
          puid, mediaChannelID, mediaChannelID,
          dword, ssrc, block->ssrc(),
          byte, fractionLost, block->fractionLost(),
          dword, cumulativeNumberOfPacketsLost, block->cumulativeNumberOfPacketsLost(),
          dword, extendedHighestSequenceNumberReceived, block->extendedHighestSequenceNumberReceived(),
          dword, interarrivalJitter, block->interarrivalJitter(),
          dword, lsr, block->lsr(),
          dword, dlsr, block->dlsr()
        );
      }
    }
    
    //-------------------------------------------------------------------------
    static void traceSenderReceiverCommonReport(
                                                IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                                RTCPPacket::SenderReceiverCommonReport *common
                                                )
    {
      traceReport(mediaChannelID, common);
      ZS_EVENTING_5(x, i, Insane, RTCPPacketTraceSenderReceiverCommonReport, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        string, ssrcType, (RTCPPacket::ReceiverReport::kPayloadType != common->pt() ? "ssrc of sender" : "ssrc of packet sender"),
        dword, ssrc, common->ssrcOfSender(),
        buffer, extension, common->extension(),
        size, extensionSize, common->extensionSize()
      );
      traceReportBlocks(mediaChannelID, common);
    }

    //-------------------------------------------------------------------------
    static void traceStringItem(
                                IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                RTCPPacket::SDES::Chunk::StringItem *common
                                )
  {
      auto type = common->type();

      if (RTCPPacket::SDES::Chunk::Priv::kItemType == type) {
        RTCPPacket::SDES::Chunk::Priv *priv = reinterpret_cast<RTCPPacket::SDES::Chunk::Priv *>(common);
        ZS_EVENTING_6(x, i, Insane, RTCPPacketTraceChunkStringItemPriv, ol, RtcpPacket, Info,
          puid, mediaChannelID, mediaChannelID,
          byte, type, common->type(),
          size_t, prefixLength, priv->prefixLength(),
          string, prefix, priv->prefix(),
          size_t, length, common->length(),
          string, value, common->value()
        );
      } else {
        ZS_EVENTING_4(x, i, Insane, RTCPPacketTraceChunkStringItem, ol, RtcpPacket, Parse,
          puid, mediaChannelID, mediaChannelID,
          byte, type, common->type(),
          size_t, length, common->length(),
          string, value, common->value()
        );
      }
    }

    //-------------------------------------------------------------------------
    static void traceStringItemList(
                                    IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                    RTCPPacket::SDES::Chunk::StringItem *first
                                    )
    {
      if (NULL == first) return;

      RTCPPacket::SDES::Chunk::StringItem *item = first;
      for (; NULL != item; item = item->mNext) {
        traceStringItem(mediaChannelID, item);
      }
    }

    //-------------------------------------------------------------------------
    static void traceDWORDs(
                            IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                            const char *type,
                            DWORD *values,
                            size_t count
                            )
    {
      if (NULL == values) return;

      for (size_t index = 0; index < count; ++index) {
        ZS_EVENTING_4(x, i, Insane, RTCPPacketTraceDWORDs, ol, RtcpPacket, Info,
          puid, mediaChannelID, mediaChannelID,
          string, type, type,
          size_t, index, index,
          dword, value, values[index]
        );
      }
    }
    
    //-------------------------------------------------------------------------
    static void traceFeedbackMessage(
                                     IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                     RTCPPacket::FeedbackMessage *common
                                     )
    {
      traceReport(mediaChannelID, common);
      ZS_EVENTING_6(x, i, Insane, RTCPPacketTraceFeedbackMessage, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        byte, fmt, common->fmt(),
        dword, ssrcOfPacketSender, common->ssrcOfPacketSender(),
        dword, ssrcOfMediaSource, common->ssrcOfMediaSource(),
        buffer, fci, common->fci(),
        size, fciSize, common->fciSize()
      );
    }

    //-------------------------------------------------------------------------
    static void traceTMMBRCommon(
                                 IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                 BYTE fmt,
                                 RTCPPacket::TransportLayerFeedbackMessage::TMMBRCommon *common
                                 )
    {
      ZS_EVENTING_6(x, i, Insane, RTCPPacketTraceFeedbackMessageTMMBRCommon, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        byte, fmt, fmt,
        dword, ssrc, common->ssrc(),
        byte, mxTBRExp, common->mxTBRExp(),
        dword, mxTBRMantissa, common->mxTBRMantissa(),
        word, measuredOverhead, common->measuredOverhead()
      );
    }
    
    //-------------------------------------------------------------------------
    static void traceCodecControlCommon(
                                        IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                        BYTE fmt,
                                        RTCPPacket::PayloadSpecificFeedbackMessage::CodecControlCommon *common
                                        )
    {
      ZS_EVENTING_5(x, i, Insane, RTCPPacketTraceFeedbackMessageCodecControlCommon, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        byte, fmt, fmt,
        dword, ssrc, common->ssrc(),
        byte, seqNr, common->seqNr(),
        dword, reserved, common->reserved()
      );
    }

    //-------------------------------------------------------------------------
    static void traceXRReportBlock(
                                   IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                   RTCPPacket::XR::ReportBlock *common
                                   )
    {
      ZS_EVENTING_6(x, i, Insane, RTCPPacketTraceXRReportBlock, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        bool, next, (NULL != common->next()),
        byte, blockType, common->blockType(),
        byte, typeSpecific, common->typeSpecific(),
        buffer, typeSpecificContents, common->typeSpecificContents(),
        size, typeSpecificContentSize, common->typeSpecificContentSize()
      );
    }

    //-------------------------------------------------------------------------
    static void traceReportBlockRange(
                                      IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                      RTCPPacket::XR::ReportBlockRange *common
                                      )
    {
      traceXRReportBlock(mediaChannelID, common);
      ZS_EVENTING_6(x, i, Insane, RTCPPacketTraceXRReportBlockRange, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        byte, reserved, common->reserved(),
        byte, thinning, common->thinning(),
        dword, ssrcOfSource, common->ssrcOfSource(),
        word, beginSeq, common->beginSeq(),
        word, endSeq, common->endSeq()
      );
    }

    //-------------------------------------------------------------------------
    static void traceRLEReportBlock(
                                    IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                    RTCPPacket::XR::RLEReportBlock *common
                                    )
    {
      traceReportBlockRange(mediaChannelID, common);
      auto count = common->chunkCount();

      if (count < 1) return;

      ElementPtr outerEl = Element::create("chunks");

      for (size_t index = 0; index < count; ++index) {
        RTCPPacket::XR::RLEChunk chunk = common->chunkAtIndex(index);

        ElementPtr chunkEl;

        if (RTCPPacket::XR::isRunLengthChunk(chunk)) {
          RTCPPacket::XR::RunLength rl(chunk);
          ZS_EVENTING_3(x, i, Insane, RTCPPacketTraceXRRLEReportBlockChunkRunLength, ol, RtcpPacket, Info,
            puid, mediaChannelID, mediaChannelID,
            byte, runType, rl.runType(),
            size_t, runLength, rl.runLength()
          );
        } else if (RTCPPacket::XR::isBitVectorChunk(chunk)) {
          RTCPPacket::XR::BitVector bv(chunk);
          ZS_EVENTING_2(x, i, Insane, RTCPPacketTraceXRRLEReportBlockChunkBitVector, ol, RtcpPacket, Info,
            puid, mediaChannelID, mediaChannelID,
            word, bitVector, bv.bitVector()
          );
        } else {
          ZS_EVENTING_2(x, i, Insane, RTCPPacketTraceXRRLEReportBlockChunkUnknown, ol, RtcpPacket, Info,
            puid, mediaChannelID, mediaChannelID,
            word, chunk, chunk
          );
        }
      }
    }

    //-------------------------------------------------------------------------
    static void traceSenderReport(
                                  IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                  RTCPPacket::SenderReport *sr
                                  )
    {
      traceSenderReceiverCommonReport(mediaChannelID, sr);
      ZS_EVENTING_7(x, i, Insane, RTCPPacketTraceSenderReport, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        dword, ntpTimestampMS, sr->ntpTimestampMS(),
        dword, ntpTimestampLS, sr->ntpTimestampLS(),
        string, ntpTimestamp, string(sr->ntpTimestamp()),
        dword, senderPacketCount, sr->senderPacketCount(),
        dword, senderOctetCount, sr->senderOctetCount(),
        bool, nextSenderReport, (NULL != sr->nextSenderReport())
      );
    }

    //-------------------------------------------------------------------------
    static void traceReceiverReport(
                                    IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                    RTCPPacket::ReceiverReport *rr
                                    )
    {
      ElementPtr subEl = Element::create("ReceiverReport");
      traceSenderReceiverCommonReport(mediaChannelID, rr);
      ZS_EVENTING_2(x, i, Insane, RTCPPacketTraceReceiverReport, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        bool, nextReceiverReport, (NULL != rr->nextReceiverReport())
      );
    }

    //-------------------------------------------------------------------------
    static void traceSDES(
                          IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                          RTCPPacket::SDES *sdes
                          )
    {
      traceReport(mediaChannelID, sdes);

      ZS_EVENTING_3(x, i, Insane, RTCPPacketTraceSDES, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        size_t, sc, sdes->sc(),
        bool, nextSDES, (NULL != sdes->nextSDES())
      );

      RTCPPacket::SDES::Chunk *chunk = sdes->firstChunk();
      if (NULL != chunk) {
        for (; NULL != chunk; chunk = chunk->next()) {
          ZS_EVENTING_14(x, i, Insane, RTCPPacketTraceSDESChunk, ol, RtcpPacket, Info,
            puid, mediaChannelID, mediaChannelID,
            dword, ssrc, chunk->ssrc(),
            size_t, count, chunk->count(),
            size_t, cNameCount, chunk->cNameCount(),
            size_t, nameCount, chunk->nameCount(),
            size_t, emailCount, chunk->emailCount(),
            size_t, phoneCount, chunk->phoneCount(),
            size_t, locCount, chunk->locCount(),
            size_t, toolCount, chunk->toolCount(),
            size_t, noteCount, chunk->noteCount(),
            size_t, privCount, chunk->privCount(),
            size_t, midCount, chunk->midCount(),
            size_t, ridCount, chunk->ridCount(),
            size_t, unknownCount, chunk->unknownCount()
          );

          traceStringItemList(mediaChannelID, chunk->firstCName());
          traceStringItemList(mediaChannelID, chunk->firstName());
          traceStringItemList(mediaChannelID, chunk->firstEmail());
          traceStringItemList(mediaChannelID, chunk->firstPhone());
          traceStringItemList(mediaChannelID, chunk->firstLoc());
          traceStringItemList(mediaChannelID, chunk->firstTool());
          traceStringItemList(mediaChannelID, chunk->firstNote());
          traceStringItemList(mediaChannelID, chunk->firstPriv());
          traceStringItemList(mediaChannelID, chunk->firstMid());
          traceStringItemList(mediaChannelID, chunk->firstRid());
          traceStringItemList(mediaChannelID, chunk->firstUnknown());
        }
      }
    }

    //-------------------------------------------------------------------------
    static void traceBye(
                         IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                         RTCPPacket::Bye *bye
                         )
    {
      traceReport(mediaChannelID, bye);
      ZS_EVENTING_3(x, i, Insane, RTCPPacketTraceBye, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        string, reasonForLeaving, bye->reasonForLeaving(),
        bool, nextBye, (NULL != bye->nextBye())
      );
      traceDWORDs(mediaChannelID, "ssrc", bye->mSSRCs, bye->sc());
    }

    //-------------------------------------------------------------------------
    static void traceApp(
                         IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                         RTCPPacket::App *app
                         )
    {
      traceReport(mediaChannelID, app);
      ZS_EVENTING_7(x, i, Insane, RTCPPacketTraceApp, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        byte, subtype, app->subtype(),
        dword, ssrc, app->ssrc(),
        string, name, app->name(),
        buffer, data, app->data(),
        size, dataSize, app->dataSize(),
        bool, nextApp, (NULL != app->nextApp())
      );
    }

    //-------------------------------------------------------------------------
    static void traceTransportLayerFeedbackMessage(
                                                   IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                                   RTCPPacket::TransportLayerFeedbackMessage *fm
                                                   )
    {
      traceFeedbackMessage(mediaChannelID, fm);

      ZS_EVENTING_3(x, i, Insane, RTCPPacketTraceTransportLayerFeedbackMessage, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        bool, unknown, (NULL != fm->unknown()),
        bool, nextTransportLayerFeedbackMessage, (NULL != fm->nextTransportLayerFeedbackMessage())
      );

      {
        auto count = fm->genericNACKCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = fm->genericNACKAtIndex(index);

            ZS_EVENTING_3(x, i, Insane, RTCPPacketTransportLayerFeedbackMessageGenericNACK, ol, RtcpPacket, Info,
              puid, mediaChannelID, mediaChannelID,
              word, pid, format->pid(),
              word, blp, format->blp()
            );
          }
        }
      }

      {
        auto count = fm->tmmbrCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("TMMBRs");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("TMMBR");

            auto format = fm->tmmbrAtIndex(index);
            traceTMMBRCommon(mediaChannelID, RTCPPacket::TransportLayerFeedbackMessage::TMMBR::kFmt, format);
          }
        }
      }

      {
        auto count = fm->tmmbrCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = fm->tmmbnAtIndex(index);
            traceTMMBRCommon(mediaChannelID, RTCPPacket::TransportLayerFeedbackMessage::TMMBN::kFmt, format);
          }
        }
      }
    }

    //-------------------------------------------------------------------------
    static void tracePayloadSpecificFeedbackMessage(
                                                    IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                                                    RTCPPacket::PayloadSpecificFeedbackMessage *fm
                                                    )
    {
      traceFeedbackMessage(mediaChannelID, fm);

      {
        auto format = fm->pli();
        if (NULL != format) {
          ZS_EVENTING_1(x, i, Insane, RTCPPacketTracePayloadSpecificFeedbackMessagePLI, ol, RtcpPacket, Info, puid, mediaChannelID, mediaChannelID);
        }
      }

      {
        auto count = fm->sliCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = fm->sliAtIndex(index);

            ZS_EVENTING_5(x, i, Insane, RTCPPacketTracePayloadSpecificFeedbackMessageSLI, ol, RtcpPacket, Info,
              puid, mediaChannelID, mediaChannelID,
              size_t, index, index,
              word, first, format->first(),
              word, number, format->number(),
              byte, pictureID, format->pictureID()
            );
          }
        }
      }

      {
        auto format = fm->rpsi();
        if (NULL != format) {
          ZS_EVENTING_5(x, i, Insane, RTCPPacketTracePayloadSpecificFeedbackMessageRPSI, ol, RtcpPacket, Info,
            puid, mediaChannelID, mediaChannelID,
            byte, zeroBit, format->zeroBit(),
            byte, payloadType, format->payloadType(),
            buffer, nativeRPSIBitString, format->nativeRPSIBitString(),
            size, nativeRPSIBitStringSizeInBits, (format->nativeRPSIBitStringSizeInBits()/8)+(0 == format->nativeRPSIBitStringSizeInBits()%8 ? 0 : 1)
          );
        }
      }

      {
        auto count = fm->firCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = fm->firAtIndex(index);
            traceCodecControlCommon(mediaChannelID, RTCPPacket::PayloadSpecificFeedbackMessage::FIR::kFmt, format);
          }
        }
      }

      {
        auto count = fm->tstrCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = fm->tstrAtIndex(index);
            traceCodecControlCommon(mediaChannelID, RTCPPacket::PayloadSpecificFeedbackMessage::TSTR::kFmt, format);

            ZS_EVENTING_4(x, i, Insane, RTCPPacketTracePayloadSpecificFeedbackMessageTSTx, ol, RtcpPacket, Info,
              puid, mediaChannelID, mediaChannelID,
              string, type, "TSTR",
              size_t, index, index,
              byte, formatIndex, format->index()
            );
          }
        }
      }

      {
        auto count = fm->tstnCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = fm->tstnAtIndex(index);
            traceCodecControlCommon(mediaChannelID, RTCPPacket::PayloadSpecificFeedbackMessage::TSTN::kFmt, format);

            ZS_EVENTING_4(x, i, Insane, RTCPPacketTracePayloadSpecificFeedbackMessageTSTx, ol, RtcpPacket, Info,
              puid, mediaChannelID, mediaChannelID,
              string, type, "TSTN",
              size_t, index, index,
              byte, formatIndex, format->index()
            );
          }
        }
      }

      {
        auto count = fm->vbcmCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = fm->vbcmAtIndex(index);
            traceCodecControlCommon(mediaChannelID, RTCPPacket::PayloadSpecificFeedbackMessage::TSTN::kFmt, format);

            ZS_EVENTING_6(x, i, Insane, RTCPPacketTracePayloadSpecificFeedbackMessageVBCM, ol, RtcpPacket, Info,
              puid, mediaChannelID, mediaChannelID,
              size_t, index, index,
              byte, zeroBit, format->zeroBit(),
              byte, payloadType, format->payloadType(),
              buffer, vbcmOctetString, format->vbcmOctetString(),
              size, vbcmOctetStringSize, format->vbcmOctetStringSize()
            );
          }
        }
      }

      {
        auto format = fm->afb();
        auto remb = fm->remb();

        if ((NULL != format) &&
            (NULL == remb)) {
          ZS_EVENTING_3(x, i, Insane, RTCPPacketTracePayloadSpecificFeedbackMessageAFB, ol, RtcpPacket, Info,
            puid, mediaChannelID, mediaChannelID,
            buffer, data, format->data(),
            size, dataSize, format->dataSize()
          );
        }
      }

      {
        auto format = fm->remb();
        if (NULL != format) {
          ZS_EVENTING_4(x, i, Insane, RTCPPacketTracePayloadSpecificFeedbackMessageREMB, ol, RtcpPacket, Info,
            puid, mediaChannelID, mediaChannelID,
            size_t, numSSRC, format->numSSRC(),
            byte, brExp, format->brExp(),
            dword, brMantissa, format->brMantissa()
          );
          traceDWORDs(mediaChannelID, "ssrc", format->mSSRCs, format->numSSRC());
        }
      }
    }

    //-------------------------------------------------------------------------
    static void traceXR(
                        IMediaStreamTrackTypes::MediaChannelID mediaChannelID,
                        RTCPPacket::XR *xr
                        )
    {
      traceReport(mediaChannelID, xr);

      ZS_EVENTING_5(x, i, Insane, RTCPPacketTraceXR, ol, RtcpPacket, Info,
        puid, mediaChannelID, mediaChannelID,
        byte, numSSRC, xr->reserved(),
        dword, ssrc, xr->ssrc(),
        size_t, reportBlockCount, xr->reportBlockCount(),
        bool, next, xr->nextXR()
      );

      {
        auto count = xr->lossRLEReportBlockCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = xr->lossRLEReportBlockAtIndex(index);
            ZS_EVENTING_4(x, i, Insane, RTCPPacketTraceXRTypeRLEReportBlock, ol, RtcpPacket, Info,
              puid, mediaChannelID, mediaChannelID,
              string, type, "LossRLE", 
              size_t, index, index,
              bool, next, (NULL != format->nextLossRLE())
            );
            traceRLEReportBlock(mediaChannelID, format);
          }
        }
      }

      {
        auto count = xr->duplicateRLEReportBlockCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = xr->duplicateRLEReportBlockAtIndex(index);
            ZS_EVENTING_4(x, i, Insane, RTCPPacketTraceXRTypeRLEReportBlock, ol, RtcpPacket, Info,
              puid, mediaChannelID, mediaChannelID,
              string, type, "DuplicateRLE",
              size_t, index, index,
              bool, next, (NULL != format->nextDuplicateRLE())
            );
            traceRLEReportBlock(mediaChannelID, format);
          }
        }
      }

      {
        auto count = xr->packetReceiptTimesReportBlockCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = xr->packetReceiptTimesReportBlockAtIndex(index);
            ZS_EVENTING_4(x, i, Insane, RTCPPacketTraceXRTypeRLEReportBlock, ol, RtcpPacket, Info,
              puid, mediaChannelID, mediaChannelID,
              string, type, "PacketReceiptTimes",
              size_t, index, index,
              bool, next, (NULL != format->nextPacketReceiptTimesReportBlock())
            );
            traceDWORDs(mediaChannelID, "receipt time", format->mReceiptTimes, format->receiptTimeCount());
            traceReportBlockRange(mediaChannelID, format);
          }
        }
      }

      {
        auto count = xr->receiverReferenceTimeReportBlockCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = xr->receiverReferenceTimeReportBlockAtIndex(index);
            ZS_EVENTING_6(x, i, Insane, RTCPPacketTraceXRReceiverReferenceTimeReportBlock, ol, RtcpPacket, Info,
              puid, mediaChannelID, mediaChannelID,
              size_t, index, index,
              dword, ntpTimestampMS, format->ntpTimestampMS(),
              dword, ntpTimestampLS, format->ntpTimestampLS(),
              string, ntpTimestamp, string(format->ntpTimestamp()),
              bool, next, (NULL != format->nextReceiverReferenceTimeReportBlock())
            );
            traceXRReportBlock(mediaChannelID, format);
          }
        }
      }

      {
        auto count = xr->dlrrReportBlockCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = xr->dlrrReportBlockAtIndex(index);
            size_t subBlockCount = format->subBlockCount();

            ZS_EVENTING_4(x, i, Insane, RTCPPacketTraceXRDLRRReportBlock, ol, RtcpPacket, Info,
              puid, mediaChannelID, mediaChannelID,
              size_t, index, index,
              size_t, subBlockCount, subBlockCount,
              bool, next, (NULL != format->nextDLRRReportBlock())
            );

            if (subBlockCount > 0) {
              for (size_t indexSubBlock = 0; indexSubBlock < subBlockCount; ++indexSubBlock)
              {
                auto subBlock = format->subBlockAtIndex(indexSubBlock);
                ZS_EVENTING_6(x, i, Insane, RTCPPacketTraceXRDLRRReportBlockSubBlock, ol, RtcpPacket, Info,
                  puid, mediaChannelID, mediaChannelID,
                  size_t, index, indexSubBlock,
                  size_t, subBlockCount, subBlockCount,
                  dword, ssrc, subBlock->ssrc(),
                  dword, lrr, subBlock->lrr(),
                  dword, dlrr, subBlock->dlrr()
                );
              }
            }
            traceXRReportBlock(mediaChannelID, format);
          }
        }
      }

      {
        auto count = xr->statisticsSummaryReportBlockCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = xr->statisticsSummaryReportBlockAtIndex(index);
            ZS_EVENTING_22(x, i, Insane, RTCPPacketTraceXRStatisticsSummary, ol, RtcpPacket, Info,
              puid, mediaChannelID, mediaChannelID,
              size_t, index, index,
              bool, lossReportFlag, format->lossReportFlag(),
              bool, lrr, format->duplicateReportFlag(),
              bool, jitterFlag, format->jitterFlag(),
              bool, ttlFlag, format->ttlFlag(),
              bool, hopLimitFlag, format->hopLimitFlag(),
              dword, lostPackets, format->lossReportFlag() ? format->lostPackets() : 0,
              dword, dupPackets, format->duplicateReportFlag() ? format->dupPackets() : 0,
              dword, minJitter, format->jitterFlag() ? format->minJitter() : 0,
              dword, maxJitter, format->jitterFlag() ? format->maxJitter() : 0,
              dword, meanJitter, format->jitterFlag() ? format->meanJitter() : 0,
              dword, devJitter, format->jitterFlag() ? format->devJitter() : 0,
              byte, minTTL, format->ttlFlag() ? format->minTTL() : static_cast<BYTE>(0),
              byte, maxTTL, format->ttlFlag() ? format->maxTTL() : static_cast<BYTE>(0),
              byte, meanTTL, format->ttlFlag() ? format->meanTTL() : static_cast<BYTE>(0),
              byte, devTTL, format->ttlFlag() ? format->devTTL() : static_cast<BYTE>(0),
              byte, minHopLimit, format->hopLimitFlag() ? format->minHopLimit() : static_cast<BYTE>(0),
              byte, maxHopLimit, format->hopLimitFlag() ? format->maxHopLimit() : static_cast<BYTE>(0),
              byte, meanHopLimit, format->hopLimitFlag() ? format->meanHopLimit() : static_cast<BYTE>(0),
              byte, devHopLimit, format->hopLimitFlag() ? format->devHopLimit() : static_cast<BYTE>(0),
              bool, next, (NULL != format->nextStatisticsSummaryReportBlock())
            );
            traceReportBlockRange(mediaChannelID, format);
          }
        }
      }

      {
        auto count = xr->voIPMetricsReportBlockCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = xr->voIPMetricsReportBlockAtIndex(index);
            ZS_EVENTING_27(x, i, Insane, RTCPPacketTraceXRVoIPMetricsReportBlock, ol, RtcpPacket, Info,
              puid, mediaChannelID, mediaChannelID,
              size_t, index, index,
              dword, ssrcOfSource, format->ssrcOfSource(),
              byte, lossRate, format->lossRate(),
              byte, discardRate, format->discardRate(),
              byte, burstDensity, format->burstDensity(),
              byte, gapDensity, format->gapDensity(),
              word, burstDuration, format->burstDuration(),
              word, gapDuration, format->gapDuration(),
              word, roundTripDelay, format->roundTripDelay(),
              word, endSystemDelay, format->endSystemDelay(),
              byte, signalLevel, format->signalLevel(),
              byte, noiseLevel, format->noiseLevel(),
              byte, rerl, format->rerl(),
              byte, Gmin, format->Gmin(),
              byte, rFactor, format->rFactor(),
              byte, extRFactor, format->extRFactor(),
              byte, mosLQ, format->mosLQ(),
              byte, mosCQ, format->mosCQ(),
              byte, rxConfig, format->rxConfig(),
              byte, plc, format->plc(),
              byte, jba, format->jba(),
              byte, jbRate, format->jbRate(),
              word, jbNominal, format->jbNominal(),
              word, jbMaximum, format->jbMaximum(),
              word, jbAbsMax, format->jbAbsMax(),
              bool, next, (NULL != format->nextVoIPMetricsReportBlock())
            );

            traceXRReportBlock(mediaChannelID, format);
          }
        }
      }

      {
        auto count = xr->unknownReportBlockCount();
        if (count > 0) {
          for (size_t index = 0; index < count; ++index) {
            auto format = xr->unknownReportBlockAtIndex(index);
            ZS_EVENTING_4(x, i, Insane, RTCPPacketTraceXRTypeRLEReportBlock, ol, RtcpPacket, Info,
              puid, mediaChannelID, mediaChannelID,
              string, type, "UnknownReportBlock",
              size_t, index, index,
              bool, next, (NULL != format->nextUnknownReportBlock())
            );

            traceXRReportBlock(mediaChannelID, format);
          }
        }
      }
    }

  } // namespace internal

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::SenderReport
  #pragma mark

  //-------------------------------------------------------------------------
  const char *RTCPPacket::Report::ptToString(BYTE pt)
  {
    switch (pt) {
      case SenderReport::kPayloadType:                    return "SenderReport";
      case ReceiverReport::kPayloadType:                  return "ReceiverReport";
      case SDES::kPayloadType:                            return "SDES";
      case Bye::kPayloadType:                             return "Bye";
      case App::kPayloadType:                             return "App";
      case TransportLayerFeedbackMessage::kPayloadType:   return "TransportLayerFeedbackMessage";
      case PayloadSpecificFeedbackMessage::kPayloadType:  return "PayloadSpecificFeedbackMessage";
      case XR::kPayloadType:                              return "XR";
      default:  {
      }
    }
    return "Uknown";
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::SenderReport
  #pragma mark

  //-------------------------------------------------------------------------
  Time RTCPPacket::SenderReport::ntpTimestamp() const
  {
    return UseRTPUtils::ntpToTime(mNTPTimestampMS, mNTPTimestampLS);
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::SDES::Chunk::StringItem
  #pragma mark

  //-------------------------------------------------------------------------
  const char *RTCPPacket::SDES::Chunk::StringItem::typeToString(BYTE type)
  {
    switch (type) {
      case CName::kItemType:  return "CName";
      case Name::kItemType:   return "Name";
      case Email::kItemType:  return "Email";
      case Phone::kItemType:  return "Phone";
      case Loc::kItemType:    return "Loc";
      case Tool::kItemType:   return "Tool";
      case Note::kItemType:   return "Note";
      case Priv::kItemType:   return "Priv";
      case Mid::kItemType:    return "Mid";
      case Rid::kItemType:    return "Rid";
      default: {
        break;
      }
    }
    return "Unknown";
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::SDES::Chunk
  #pragma mark

  //-------------------------------------------------------------------------
  RTCPPacket::SDES::Chunk::CName *RTCPPacket::SDES::Chunk::cNameAtIndex(size_t index) const
  {
    ASSERT(index < mCNameCount)
    return &(mFirstCName[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::SDES::Chunk::Name *RTCPPacket::SDES::Chunk::nameAtIndex(size_t index) const
  {
    ASSERT(index < mNameCount)
    return &(mFirstName[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::SDES::Chunk::Email *RTCPPacket::SDES::Chunk::emailAtIndex(size_t index) const
  {
    ASSERT(index < mEmailCount)
    return &(mFirstEmail[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::SDES::Chunk::Phone *RTCPPacket::SDES::Chunk::phoneAtIndex(size_t index) const
  {
    ASSERT(index < mPhoneCount)
    return &(mFirstPhone[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::SDES::Chunk::Loc *RTCPPacket::SDES::Chunk::locAtIndex(size_t index) const
  {
    ASSERT(index < mLocCount)
    return &(mFirstLoc[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::SDES::Chunk::Tool *RTCPPacket::SDES::Chunk::toolAtIndex(size_t index) const
  {
    ASSERT(index < mToolCount)
    return &(mFirstTool[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::SDES::Chunk::Note *RTCPPacket::SDES::Chunk::noteAtIndex(size_t index) const
  {
    ASSERT(index < mNoteCount)
    return &(mFirstNote[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::SDES::Chunk::Priv *RTCPPacket::SDES::Chunk::privAtIndex(size_t index) const
  {
    ASSERT(index < mPrivCount)
    return &(mFirstPriv[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::SDES::Chunk::Mid *RTCPPacket::SDES::Chunk::midAtIndex(size_t index) const
  {
    ASSERT(index < mMidCount)
    return &(mFirstMid[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::SDES::Chunk::Rid *RTCPPacket::SDES::Chunk::ridAtIndex(size_t index) const
  {
    ASSERT(index < mRidCount)
    return &(mFirstRid[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::SDES::Chunk::Unknown *RTCPPacket::SDES::Chunk::unknownAtIndex(size_t index) const
  {
    ASSERT(index < mUnknownCount)
    return &(mFirstUnknown[index]);
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::Bye
  #pragma mark

  //-------------------------------------------------------------------------
  DWORD RTCPPacket::Bye::ssrc(size_t index) const
  {
    ASSERT(index < sc())
    return mSSRCs[index];
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::FeedbackMessage
  #pragma mark

  //-------------------------------------------------------------------------
  const char *RTCPPacket::FeedbackMessage::fmtToString(BYTE pt, BYTE fmt, DWORD subFmt)
  {
    switch (pt) {
      case TransportLayerFeedbackMessage::kPayloadType:     {
        switch (fmt) {
          case TransportLayerFeedbackMessage::GenericNACK::kFmt:  return "GenericNACK";
          case TransportLayerFeedbackMessage::TMMBR::kFmt:        return "TMMBR";
          case TransportLayerFeedbackMessage::TMMBN::kFmt:        return "TMMBN";
          default:                                                break;
        }
        break;
      }
      case PayloadSpecificFeedbackMessage::kPayloadType:    {
        switch (fmt) {
          case PayloadSpecificFeedbackMessage::PLI::kFmt:         return "PLI";
          case PayloadSpecificFeedbackMessage::SLI::kFmt:         return "SLI";
          case PayloadSpecificFeedbackMessage::RPSI::kFmt:        return "RPSI";
          case PayloadSpecificFeedbackMessage::FIR::kFmt:         return "FIR";
          case PayloadSpecificFeedbackMessage::TSTR::kFmt:        return "TSTR";
          case PayloadSpecificFeedbackMessage::TSTN::kFmt:        return "TSTN";
          case PayloadSpecificFeedbackMessage::VBCM::kFmt:        return "VBCM";
          case PayloadSpecificFeedbackMessage::AFB::kFmt:         {
            const char *tmp = "REMB";
            if (subFmt == *(reinterpret_cast<const DWORD *>(tmp))) return "REMB";
            return "AFB";
          }
          default:                                                break;
        }
        break;
      }
      default:                                              {
        break;
      }
    }
    return "Unknown";
  }

  //-------------------------------------------------------------------------
  const char *RTCPPacket::FeedbackMessage::fmtToString() const
  {
    if (PayloadSpecificFeedbackMessage::kPayloadType == mPT) {
      if (PayloadSpecificFeedbackMessage::AFB::kFmt == mReportSpecific) {
        auto result = reinterpret_cast<const PayloadSpecificFeedbackMessage *>(this);
        const char tmp1[sizeof(DWORD)] {};
        const char *tmp2 = "REMB";
        const char *usingTmp = (result->mHasREMB ? tmp2 : (&(tmp1[0])));

        return fmtToString(mPT, mReportSpecific, *(reinterpret_cast<const DWORD *>(usingTmp)));
      }
    }
    return fmtToString(mPT, mReportSpecific);
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::TransportLayerFeedbackMessage
  #pragma mark

  //-------------------------------------------------------------------------
  RTCPPacket::TransportLayerFeedbackMessage::GenericNACK *RTCPPacket::TransportLayerFeedbackMessage::genericNACKAtIndex(size_t index) const
  {
    ASSERT(index < mGenericNACKCount)
    return &(mFirstGenericNACK[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::TransportLayerFeedbackMessage::TMMBR *RTCPPacket::TransportLayerFeedbackMessage::tmmbrAtIndex(size_t index) const
  {
    ASSERT(index < mTMMBRCount)
    return &(mFirstTMMBR[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::TransportLayerFeedbackMessage::TMMBN *RTCPPacket::TransportLayerFeedbackMessage::tmmbnAtIndex(size_t index) const
  {
    ASSERT(index < mTMMBNCount)
    return &(mFirstTMMBN[index]);
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage::VBCM
  #pragma mark

  //-------------------------------------------------------------------------
  BYTE RTCPPacket::PayloadSpecificFeedbackMessage::VBCM::zeroBit() const
  {
    return RTCP_GET_BITS(mControlSpecific, 0x1, 23);
  }

  //-------------------------------------------------------------------------
  BYTE RTCPPacket::PayloadSpecificFeedbackMessage::VBCM::payloadType() const
  {
    return RTCP_GET_BITS(mControlSpecific, 0x7F, 16);
  }

  //-------------------------------------------------------------------------
  size_t RTCPPacket::PayloadSpecificFeedbackMessage::VBCM::vbcmOctetStringSize() const
  {
    return RTCP_GET_BITS(mControlSpecific, 0xFFFF, 0);
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage::REMB
  #pragma mark

  //-------------------------------------------------------------------------
  DWORD RTCPPacket::PayloadSpecificFeedbackMessage::REMB::ssrcAtIndex(size_t index) const
  {
    ASSERT(index < numSSRC())
    return mSSRCs[index];
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage
  #pragma mark

  //-------------------------------------------------------------------------
  RTCPPacket::PayloadSpecificFeedbackMessage::PLI *RTCPPacket::PayloadSpecificFeedbackMessage::pli() const
  {
    if (PLI::kFmt != fmt()) return NULL;
    return const_cast<PLI *>(&mPLI);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::PayloadSpecificFeedbackMessage::SLI *RTCPPacket::PayloadSpecificFeedbackMessage::sliAtIndex(size_t index) const
  {
    ASSERT(index < mSLICount)
    return &(mFirstSLI[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::PayloadSpecificFeedbackMessage::FIR *RTCPPacket::PayloadSpecificFeedbackMessage::firAtIndex(size_t index) const
  {
    ASSERT(index < mFIRCount)
    return &(mFirstFIR[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::PayloadSpecificFeedbackMessage::TSTR *RTCPPacket::PayloadSpecificFeedbackMessage::tstrAtIndex(size_t index) const
  {
    ASSERT(index < mTSTRCount)
    return &(mFirstTSTR[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::PayloadSpecificFeedbackMessage::TSTN *RTCPPacket::PayloadSpecificFeedbackMessage::tstnAtIndex(size_t index) const
  {
    ASSERT(index < mTSTNCount)
    return &(mFirstTSTN[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::PayloadSpecificFeedbackMessage::VBCM *RTCPPacket::PayloadSpecificFeedbackMessage::vbcmAtIndex(size_t index) const
  {
    ASSERT(index < mVBCMCount)
    return &(mFirstVBCM[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::PayloadSpecificFeedbackMessage::RPSI *RTCPPacket::PayloadSpecificFeedbackMessage::rpsi() const
  {
    if (RPSI::kFmt != fmt()) return NULL;
    return const_cast<RPSI *>(&mRPSI);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::PayloadSpecificFeedbackMessage::AFB *RTCPPacket::PayloadSpecificFeedbackMessage::afb() const
  {
    if (AFB::kFmt != fmt()) return NULL;
    if (mHasREMB) return NULL;
    return const_cast<AFB *>(&mAFB);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::PayloadSpecificFeedbackMessage::REMB *RTCPPacket::PayloadSpecificFeedbackMessage::remb() const
  {
    if (REMB::kFmt != fmt()) return NULL;
    if (!mHasREMB) return NULL;
    return const_cast<REMB *>(&mREMB);
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::XR::ReportBlock
  #pragma mark

  //-------------------------------------------------------------------------
  const char *RTCPPacket::XR::ReportBlock::blockTypeToString(BYTE blockType)
  {
    switch (blockType) {
      case LossRLEReportBlock::kBlockType:                return "LossRLEReportBlock";
      case DuplicateRLEReportBlock::kBlockType:           return "DuplicateRLEReportBlock";
      case PacketReceiptTimesReportBlock::kBlockType:     return "PacketReceiptTimesReportBlock";
      case ReceiverReferenceTimeReportBlock::kBlockType:  return "ReceiverReferenceTimeReportBlock";
      case DLRRReportBlock::kBlockType:                   return "DLRRReportBlock";
      case StatisticsSummaryReportBlock::kBlockType:      return "StatisticsSummaryReportBlock";
      case VoIPMetricsReportBlock::kBlockType:            return "VoIPMetricsReportBlock";
      default: {
        break;
      }
    }

    return "UnknownReportBlock";
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::XR::ReportBlockRange
  #pragma mark

  //-------------------------------------------------------------------------
  BYTE RTCPPacket::XR::ReportBlockRange::reserved() const
  {
    return RTCP_GET_BITS(mTypeSpecific, 0xF, 4);
  }

  //-------------------------------------------------------------------------
  BYTE RTCPPacket::XR::ReportBlockRange::thinning() const
  {
    return RTCP_GET_BITS(mTypeSpecific, 0xF, 0);
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::XR::RLEReportBlock
  #pragma mark

  //-------------------------------------------------------------------------
  RTCPPacket::XR::RLEChunk RTCPPacket::XR::RLEReportBlock::chunkAtIndex(size_t index) const
  {
    ASSERT(index < chunkCount())
    return mChunks[index];
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::XR::PacketReceiptTimesReportBlock
  #pragma mark

  //-------------------------------------------------------------------------
  DWORD RTCPPacket::XR::PacketReceiptTimesReportBlock::receiptTimeAtIndex(size_t index) const
  {
    ASSERT(index < receiptTimeCount())
    return mReceiptTimes[index];
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::XR::ReceiverReferenceTimeReportBlock
  #pragma mark

  //-------------------------------------------------------------------------
  Time RTCPPacket::XR::ReceiverReferenceTimeReportBlock::ntpTimestamp() const
  {
    return UseRTPUtils::ntpToTime(mNTPTimestampMS, mNTPTimestampLS);
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::XR::DLRRReportBlock
  #pragma mark

  //-------------------------------------------------------------------------
  RTCPPacket::XR::DLRRReportBlock::SubBlock *RTCPPacket::XR::DLRRReportBlock::subBlockAtIndex(size_t index) const
  {
    ASSERT(index < subBlockCount())
    return &(mSubBlocks[index]);
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::XR::StatisticsSummaryReportBlock
  #pragma mark

  //-------------------------------------------------------------------------
  bool RTCPPacket::XR::StatisticsSummaryReportBlock::lossReportFlag() const
  {
    return RTCP_IS_FLAG_SET(mTypeSpecific, 7);
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::XR::StatisticsSummaryReportBlock::duplicateReportFlag() const
  {
    return RTCP_IS_FLAG_SET(mTypeSpecific, 6);
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::XR::StatisticsSummaryReportBlock::jitterFlag() const
  {
    return RTCP_IS_FLAG_SET(mTypeSpecific, 5);
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::XR::StatisticsSummaryReportBlock::ttlFlag() const
  {
    return (1 == RTCP_GET_BITS(mTypeSpecific, 0x3, 3));
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::XR::StatisticsSummaryReportBlock::hopLimitFlag() const
  {
    return (2 == RTCP_GET_BITS(mTypeSpecific, 0x3, 3));
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::XR::VoIPMetricsReportBlock
  #pragma mark

  //-------------------------------------------------------------------------
  BYTE RTCPPacket::XR::VoIPMetricsReportBlock::plc() const
  {
    return RTCP_GET_BITS(mRXConfig, 0x3, 6);
  }

  //-------------------------------------------------------------------------
  BYTE RTCPPacket::XR::VoIPMetricsReportBlock::jba() const
  {
    return RTCP_GET_BITS(mRXConfig, 0x3, 4);
  }

  //-------------------------------------------------------------------------
  BYTE RTCPPacket::XR::VoIPMetricsReportBlock::jbRate() const
  {
    return RTCP_GET_BITS(mRXConfig, 0xF, 0);
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::XR::RunLength
  #pragma mark

  //-------------------------------------------------------------------------
  RTCPPacket::XR::RunLength::RunLength(RLEChunk chunk) :
    mRunType(RTCP_GET_BITS(chunk, 0x1, 14)),
    mRunLength(RTCP_GET_BITS(chunk, 0x3FFF, 0))
  {
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::XR::BitVector
  #pragma mark

  //-------------------------------------------------------------------------
  RTCPPacket::XR::BitVector::BitVector(RLEChunk chunk) :
    mBitVector(RTCP_GET_BITS(chunk, 0x7FFF, 0))
  {
  }

  //-------------------------------------------------------------------------
  BYTE RTCPPacket::XR::BitVector::bitAtIndex(size_t index) const
  {
    ASSERT(index < (sizeof(WORD)*8))
    return (mBitVector >> ((sizeof(WORD)*8)-index-1)) & 0x1;
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket::XR
  #pragma mark

  //-------------------------------------------------------------------------
  RTCPPacket::XR::LossRLEReportBlock *RTCPPacket::XR::lossRLEReportBlockAtIndex(size_t index) const
  {
    ASSERT(index < mLossRLEReportBlockCount)
    return &(mFirstLossRLEReportBlock[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::XR::DuplicateRLEReportBlock *RTCPPacket::XR::duplicateRLEReportBlockAtIndex(size_t index) const
  {
    ASSERT(index < mDuplicateRLEReportBlockCount)
    return &(mFirstDuplicateRLEReportBlock[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::XR::PacketReceiptTimesReportBlock *RTCPPacket::XR::packetReceiptTimesReportBlockAtIndex(size_t index) const
  {
    ASSERT(index < mPacketReceiptTimesReportBlockCount)
    return &(mFirstPacketReceiptTimesReportBlock[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::XR::ReceiverReferenceTimeReportBlock *RTCPPacket::XR::receiverReferenceTimeReportBlockAtIndex(size_t index) const
  {
    ASSERT(index < mReceiverReferenceTimeReportBlockCount)
    return &(mFirstReceiverReferenceTimeReportBlock[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::XR::DLRRReportBlock *RTCPPacket::XR::dlrrReportBlockAtIndex(size_t index) const
  {
    ASSERT(index < mDLRRReportBlockCount)
    return &(mFirstDLRRReportBlock[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::XR::StatisticsSummaryReportBlock *RTCPPacket::XR::statisticsSummaryReportBlockAtIndex(size_t index) const
  {
    ASSERT(index < mStatisticsSummaryReportBlockCount)
    return &(mFirstStatisticsSummaryReportBlock[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::XR::VoIPMetricsReportBlock *RTCPPacket::XR::voIPMetricsReportBlockAtIndex(size_t index) const
  {
    ASSERT(index < mVoIPMetricsReportBlockCount)
    return &(mFirstVoIPMetricsReportBlock[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::XR::UnknownReportBlock *RTCPPacket::XR::unknownReportBlockAtIndex(size_t index) const
  {
    ASSERT(index < mUnknownReportBlockCount)
    return &(mFirstUnknownReportBlock[index]);
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::XR::isRunLengthChunk(RLEChunk chunk)
  {
    return !RTCP_IS_FLAG_SET(chunk, 15);
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::XR::isBitVectorChunk(RLEChunk chunk)
  {
    return RTCP_IS_FLAG_SET(chunk, 15);
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket (public)
  #pragma mark

  //-------------------------------------------------------------------------
  RTCPPacket::RTCPPacket(
                         const make_private &,
                         MediaChannelID mediaChannelID
                         ) :
    mMediaChannelID(mediaChannelID)
  {
  }

  //-------------------------------------------------------------------------
  RTCPPacket::~RTCPPacket()
  {
  }

  //-------------------------------------------------------------------------
  RTCPPacketPtr RTCPPacket::create(
                                   const BYTE *buffer,
                                   size_t bufferLengthInBytes,
                                   MediaChannelID mediaChannelID
                                   )
  {
    ORTC_THROW_INVALID_PARAMETERS_IF(!buffer)
    ORTC_THROW_INVALID_PARAMETERS_IF(0 == bufferLengthInBytes)
    return RTCPPacket::create(UseServicesHelper::convertToBuffer(buffer, bufferLengthInBytes), mediaChannelID);
  }

  //-------------------------------------------------------------------------
  RTCPPacketPtr RTCPPacket::create(
                                   const SecureByteBlock &buffer,
                                   MediaChannelID mediaChannelID
                                   )
  {
    return RTCPPacket::create(buffer.BytePtr(), buffer.SizeInBytes(), mediaChannelID);
  }

  //-------------------------------------------------------------------------
  RTCPPacketPtr RTCPPacket::create(
                                   SecureByteBlockPtr buffer,
                                   MediaChannelID mediaChannelID
                                   )
  {
    RTCPPacketPtr pThis(make_shared<RTCPPacket>(make_private{}, mediaChannelID));
    pThis->mBuffer = buffer;
    if (!pThis->parse()) {
      ZS_EVENTING_4(x, w, Debug, RTCPPacketParseWarningNotParsed, ol, RtcpPacket, Parse,
        string, message, "packet could not be parsed",
        puid, mediaChannelID, mediaChannelID,
        buffer, buffer, ((bool)buffer) ? buffer->BytePtr() : NULL,
        size, size, ((bool)buffer) ? buffer->SizeInBytes() : 0
      );
      return RTCPPacketPtr();
    }
    return pThis;
  }

  //-------------------------------------------------------------------------
  RTCPPacketPtr RTCPPacket::create(
                                   const Report *first,
                                   MediaChannelID mediaChannelID
                                   )
  {
    size_t allocationSize = getPacketSize(first);
    SecureByteBlockPtr temp(make_shared<SecureByteBlock>(allocationSize));

    BYTE *buffer = temp->BytePtr();
    BYTE *pos = buffer;
    writePacket(first, pos, allocationSize);

    return create(temp, mediaChannelID);
  }

  //-------------------------------------------------------------------------
  SecureByteBlockPtr RTCPPacket::generateFrom(const Report *first)
  {
    size_t allocationSize = getPacketSize(first);
    SecureByteBlockPtr temp(make_shared<SecureByteBlock>(allocationSize));

    BYTE *buffer = temp->BytePtr();
    BYTE *pos = buffer;
    writePacket(first, pos, allocationSize);

    return temp;
  }

  //-------------------------------------------------------------------------
  const BYTE *RTCPPacket::ptr() const
  {
    return mBuffer->BytePtr();
  }

  //-------------------------------------------------------------------------
  size_t RTCPPacket::size() const
  {
    return mBuffer->SizeInBytes();
  }

  //-------------------------------------------------------------------------
  SecureByteBlockPtr RTCPPacket::buffer() const
  {
    return mBuffer;
  }

  //-------------------------------------------------------------------------
  RTCPPacket::SenderReport *RTCPPacket::senderReportAtIndex(size_t index) const
  {
    ASSERT(index < mSenderReportCount)
    return &(mFirstSenderReport[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::ReceiverReport *RTCPPacket::receiverReportAtIndex(size_t index) const
  {
    ASSERT(index < mReceiverReportCount)
    return &(mFirstReceiverReport[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::SDES *RTCPPacket::sdesAtIndex(size_t index) const
  {
    ASSERT(index < mSDESCount)
    return &(mFirstSDES[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::Bye *RTCPPacket::byeAtIndex(size_t index) const
  {
    ASSERT(index < mByeCount)
    return &(mFirstBye[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::App *RTCPPacket::appAtIndex(size_t index) const
  {
    ASSERT(index < mAppCount)
    return &(mFirstApp[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::TransportLayerFeedbackMessage *RTCPPacket::transportLayerFeedbackReportAtIndex(size_t index) const
  {
    ASSERT(index < mTransportLayerFeedbackMessageCount)
    return &(mFirstTransportLayerFeedbackMessage[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::PayloadSpecificFeedbackMessage *RTCPPacket::payloadSpecificFeedbackReportAtIndex(size_t index) const
  {
    ASSERT(index < mPayloadSpecificFeedbackMessageCount)
    return &(mFirstPayloadSpecificFeedbackMessage[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::XR *RTCPPacket::xrAtIndex(size_t index) const
  {
    ASSERT(index < mXRCount)
    return &(mFirstXR[index]);
  }

  //-------------------------------------------------------------------------
  RTCPPacket::UnknownReport *RTCPPacket::unknownAtIndex(size_t index) const
  {
    ASSERT(index < mUnknownReportCount)
    return &(mFirstUnknownReport[index]);
  }

  //-------------------------------------------------------------------------
  void RTCPPacket::trace(const char *message) const
  {
    if (!ZS_EVENTING_IS_LOGGING(Insane)) return;

    ZS_EVENTING_7(x, i, Insane, RTCPPacketTrace, ol, RtcpPacket, Info,
      puid, mediaChannelID, mMediaChannelID,
      string, message, message,
      buffer, buffer, ((bool)mBuffer) ? mBuffer->BytePtr() : NULL,
      size, size, ((bool)mBuffer) ? mBuffer->SizeInBytes() : 0,
      buffer, allocationBuffer, ((bool)mAllocationBuffer) ? mAllocationBuffer->BytePtr() : NULL,
      size, allocationBuffersize, ((bool)mAllocationBuffer) ? mAllocationBuffer->SizeInBytes() : 0,
      size_t, allocationPos, (NULL != mAllocationPos ? (mAllocationBuffer ? (reinterpret_cast<PTRNUMBER>(mAllocationPos) - reinterpret_cast<PTRNUMBER>(mAllocationBuffer->BytePtr())) : reinterpret_cast<PTRNUMBER>(mAllocationPos)) : 0)
    );

    for (Report *report = mFirst; NULL != report; report = report->next())
    {
      switch (report->pt()) {
        case SenderReport::kPayloadType:
        {
          SenderReport *sr = static_cast<SenderReport *>(report);
          internal::traceSenderReport(mMediaChannelID, sr);
          break;
        }
        case ReceiverReport::kPayloadType:
        {
          ReceiverReport *rr = static_cast<ReceiverReport *>(report);
          internal::traceReceiverReport(mMediaChannelID, rr);
          break;
        }
        case SDES::kPayloadType:
        {
          SDES *sdes = static_cast<SDES *>(report);
          internal::traceSDES(mMediaChannelID, sdes);
          break;
        }
        case Bye::kPayloadType:
        {
          Bye *bye = static_cast<Bye *>(report);
          internal::traceBye(mMediaChannelID, bye);
          break;
        }
        case App::kPayloadType:
        {
          App *app = static_cast<App *>(report);
          internal::traceApp(mMediaChannelID, app);
          break;
        }
        case TransportLayerFeedbackMessage::kPayloadType:
        {
          TransportLayerFeedbackMessage *fm = static_cast<TransportLayerFeedbackMessage *>(report);
          internal::traceTransportLayerFeedbackMessage(mMediaChannelID, fm);
          break;
        }
        case PayloadSpecificFeedbackMessage::kPayloadType:
        {
          PayloadSpecificFeedbackMessage *fm = static_cast<PayloadSpecificFeedbackMessage *>(report);
          internal::tracePayloadSpecificFeedbackMessage(mMediaChannelID, fm);
          break;
        }
        case XR::kPayloadType:
        {
          XR *xr = static_cast<XR *>(report);
          internal::traceXR(mMediaChannelID, xr);
          break;
        }
        default:
        {
          UnknownReport *unknown = static_cast<UnknownReport *>(report);
          ZS_EVENTING_2(x, i, Insane, RTCPPacketTraceUnknownReport, ol, RtcpPacket, Info,
            puid, mediaChannelID, mMediaChannelID,
            bool, next, (NULL != unknown->nextUnknown())
          );
          break;
        }
      }
    }
  }


  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket (internal)
  #pragma mark

  //-------------------------------------------------------------------------
  bool RTCPPacket::parse()
  {
    const BYTE *buffer = mBuffer->BytePtr();
    size_t size = mBuffer->SizeInBytes();

    if (size < internal::kMinRtcpPacketLen) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "packet length is too short",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, 0,
        size_t, length, size
      );
      trace("packet length is too short");
      return false;
    }

    bool foundPaddingBit = false;

    // scope: calculate total memory allocation size needed to parse entire RTCP packet
    {
      size_t remaining = size;
      const BYTE *pos = buffer;

      while (remaining >= internal::kMinRtcpPacketLen) {
        BYTE version = RTCP_GET_BITS(*pos, 0x3, 6);
        if (internal::kRtpVersion != version) {
          ZS_EVENTING_3(x, w, Trace, RTCPPacketParseWarningIllegalVersion, ol, RtcpPacket, Parse,
            string, message, "illegal version found",
            puid, mediaChannelID, mMediaChannelID,
            byte, version, version
          );
          trace("illegal version found");
          return false;
        }

        size_t length = sizeof(DWORD) + (static_cast<size_t>(UseRTPUtils::getBE16(&(pos[2]))) * sizeof(DWORD));

        size_t padding = 0;

        if (RTCP_IS_FLAG_SET(*pos, 5)) {
          if (foundPaddingBit) {
            ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
              string, message, "found illegal second padding bit set in compound RTCP block",
              puid, mediaChannelID, mMediaChannelID,
              size_t, remaining, remaining,
              size_t, length, length
            );
            trace("found illegal second padding bit set in compound RTCP block");
            return false;
          }

          padding = buffer[size-1];
        }

        if ((sizeof(DWORD) + padding) > length) {
          ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
            string, message, "malformed padding size in RTCP packet",
            puid, mediaChannelID, mMediaChannelID,
            size_t, remaining, remaining,
            size_t, length, length
          );
          trace("malformed padding size in RTCP packet");
          return false;
        }

        length -= padding;

        if (remaining < length) {
          ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
            string, message, "insufficient length remaining for RTCP block",
            puid, mediaChannelID, mMediaChannelID,
            size_t, remaining, remaining,
            size_t, length, length
          );
          trace("insufficient length remaining for RTCP block");
          return false;
        }

        BYTE reportSpecific = RTCP_GET_BITS(*pos, 0x1F, 0);

        const BYTE *prePos = pos;
        BYTE pt = pos[1];

        internal::advancePos(pos, remaining, sizeof(DWORD));

        size_t preAllocationSize = mAllocationSize;

        if (!getAllocationSize(static_cast<BYTE>(version), static_cast<BYTE>(padding), reportSpecific, pt, pos, static_cast<size_t>(length - sizeof(DWORD)))) return false;

        internal::advancePos(pos, remaining, length - sizeof(DWORD));

        if (0 != padding) {
          internal::advancePos(pos, remaining, padding);
        }

        ZS_EVENTING_6(x, i, Insane, RTCPPacketParseAllocationSize, ol, RtcpPacket, Parse,
          puid, mediaChannelID, mMediaChannelID,
          string, pt, Report::ptToString(pt),
          size_t, consumed, static_cast<size_t>(reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(prePos)),
          size_t, allocationSize, mAllocationSize - preAllocationSize,
          size_t, remaining, remaining,
          size_t, length, length
        );
        ++mCount;
      }
    }

    if (0 == mAllocationSize) {
      trace("no RTCP packets were processed");
      return true;
    }

    mAllocationBuffer = make_shared<SecureByteBlock>(internal::alignedSize(mAllocationSize));

    mAllocationPos = mAllocationBuffer->BytePtr();

    // scope: allocation size is now established; begin parsing all reports contained in RTCP packet
    {
      size_t remaining = size;
      const BYTE *pos = buffer;


      if (0 != mSenderReportCount) {
        mFirstSenderReport = new (allocateBuffer(internal::alignedSize(sizeof(SenderReport)) * mSenderReportCount)) SenderReport[mSenderReportCount];
      }
      if (0 != mReceiverReportCount) {
        mFirstReceiverReport = new (allocateBuffer(internal::alignedSize(sizeof(ReceiverReport)) * mReceiverReportCount)) ReceiverReport[mReceiverReportCount];
      }
      if (0 != mSDESCount) {
        mFirstSDES = new (allocateBuffer(internal::alignedSize(sizeof(SDES)) * mSDESCount)) SDES[mSDESCount];
      }
      if (0 != mByeCount) {
        mFirstBye = new (allocateBuffer(internal::alignedSize(sizeof(Bye)) * mByeCount)) Bye[mByeCount];
      }
      if (0 != mAppCount) {
        mFirstApp = new (allocateBuffer(internal::alignedSize(sizeof(App)) * mAppCount)) App[mAppCount];
      }
      if (0 != mTransportLayerFeedbackMessageCount) {
        mFirstTransportLayerFeedbackMessage = new (allocateBuffer(internal::alignedSize(sizeof(TransportLayerFeedbackMessage)) * mTransportLayerFeedbackMessageCount)) TransportLayerFeedbackMessage[mTransportLayerFeedbackMessageCount];
      }
      if (0 != mPayloadSpecificFeedbackMessageCount) {
        mFirstPayloadSpecificFeedbackMessage = new (allocateBuffer(internal::alignedSize(sizeof(PayloadSpecificFeedbackMessage)) * mPayloadSpecificFeedbackMessageCount)) PayloadSpecificFeedbackMessage[mPayloadSpecificFeedbackMessageCount];
      }
      if (0 != mXRCount) {
        mFirstXR = new (allocateBuffer(internal::alignedSize(sizeof(XR)) * mXRCount)) XR[mXRCount];
      }
      if (0 != mUnknownReportCount) {
        mFirstUnknownReport = new (allocateBuffer(internal::alignedSize(sizeof(UnknownReport)) * mUnknownReportCount)) UnknownReport[mUnknownReportCount];
      }

      mSenderReportCount = 0;
      mReceiverReportCount = 0;
      mSDESCount = 0;
      mByeCount = 0;
      mAppCount = 0;
      mTransportLayerFeedbackMessageCount = 0;
      mPayloadSpecificFeedbackMessageCount = 0;
      mXRCount = 0;
      mUnknownReportCount = 0;

      Report *lastReport = NULL;
      size_t count = 0;

      while (remaining >= internal::kMinRtcpPacketLen) {
        BYTE version = RTCP_GET_BITS(*pos, 0x3, 6);
        size_t length = sizeof(DWORD) + (static_cast<size_t>(UseRTPUtils::getBE16(&(pos[2]))) * sizeof(DWORD));

        size_t padding = 0;

        if (RTCP_IS_FLAG_SET(*pos, 5)) {
          padding = buffer[size-1];
          length -= padding; // already protected during pre-parse against malformed padding length
        }

        BYTE reportSpecific = RTCP_GET_BITS(*pos, 0x1F, 0);

        const BYTE *prePos = pos;
        BYTE pt = pos[1];

        internal::advancePos(pos, remaining, sizeof(DWORD));

        size_t preAllocationSize = mAllocationSize;

        if (!parse(lastReport, version, static_cast<BYTE>(padding), reportSpecific, pt, pos, length - sizeof(DWORD))) return false;

        if (NULL == mFirst) {
          mFirst = lastReport;
        }

        internal::advancePos(pos, remaining, length - sizeof(DWORD));

        if (0 != padding) {
          internal::advancePos(pos, remaining, padding);
        }

        ZS_EVENTING_5(x, i, Insane, RTCPPacketParsedReport, ol, RtcpPacket, Parse,
          puid, mediaChannelID, mMediaChannelID,
          byte, ptValue, pt,
          string, pt, Report::ptToString(pt),
          size_t, consumed, static_cast<size_t>(reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(prePos)),
          size_t, consumedAllocation, preAllocationSize - mAllocationSize
        );

        ++count;
        ASSERT(count <= mCount);
      }
    }

    trace("parsed");
    return true;
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket (parse allocation sizing routines)
  #pragma mark

  //-------------------------------------------------------------------------
  bool RTCPPacket::getAllocationSize(
                                      BYTE version,
                                      BYTE padding,
                                      BYTE reportSpecific,
                                      BYTE pt,
                                      const BYTE *contents,
                                      size_t contentSize
                                      )
  {
    bool result = false;

    switch (pt) {
      case SenderReport::kPayloadType:                    result = getSenderReportAllocationSize(version, padding, reportSpecific, contents, contentSize); break;
      case ReceiverReport::kPayloadType:                  result = getReceiverReportAllocationSize(version, padding, reportSpecific, contents, contentSize); break;
      case SDES::kPayloadType:                            result = getSDESAllocationSize(version, padding, reportSpecific, contents, contentSize); break;
      case Bye::kPayloadType:                             result = getByeAllocationSize(version, padding, reportSpecific, contents, contentSize); break;
      case App::kPayloadType:                             result = getAppAllocationSize(version, padding, reportSpecific, contents, contentSize); break;
      case TransportLayerFeedbackMessage::kPayloadType:   result = getTransportLayerFeedbackMessageAllocationSize(version, padding, reportSpecific, contents, contentSize); break;
      case PayloadSpecificFeedbackMessage::kPayloadType:  result = getPayloadSpecificFeedbackMessageAllocationSize(version, padding, reportSpecific, contents, contentSize); break;
      case XR::kPayloadType:                              result = getXRAllocationSize(version, padding, reportSpecific, contents, contentSize); break;
      default:
      {
        result = getUnknownReportAllocationSize(version, padding, reportSpecific, contents, contentSize);
        break;
      }
    }
    return result;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getSenderReportAllocationSize(
                                                  BYTE version,
                                                  BYTE padding,
                                                  BYTE reportSpecific,
                                                  const BYTE *contents,
                                                  size_t contentSize
                                                  )
  {
    ++mSenderReportCount;

    mAllocationSize += internal::alignedSize(sizeof(SenderReport)) + (internal::alignedSize(sizeof(SenderReport::ReportBlock)) * static_cast<size_t>(reportSpecific));
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getReceiverReportAllocationSize(
                                                    BYTE version,
                                                    BYTE padding,
                                                    BYTE reportSpecific,
                                                    const BYTE *contents,
                                                    size_t contentSize
                                                    )
  {
    ++mReceiverReportCount;

    mAllocationSize += internal::alignedSize(sizeof(ReceiverReport)) + (internal::alignedSize(sizeof(ReceiverReport::ReportBlock)) * static_cast<size_t>(reportSpecific));
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getSDESAllocationSize(
                                          BYTE version,
                                          BYTE padding,
                                          BYTE reportSpecific,
                                          const BYTE *contents,
                                          size_t contentSize
                                          )
  {
    ++mSDESCount;

    size_t chunksCount = static_cast<size_t>(reportSpecific);

    mAllocationSize += internal::alignedSize(sizeof(SDES)) + (internal::alignedSize(sizeof(SDES::Chunk)) * chunksCount);

    size_t remaining = contentSize;

    const BYTE *pos = contents;

    while ((remaining > sizeof(DWORD)) &&
            (chunksCount > 0))
    {
      internal::advancePos(pos, remaining, sizeof(DWORD));

      ZS_EVENTING_3(x, i, Insane, RTCPPacketGetSDESAllocationSizeChunks, ol, RtcpPacket, Parse,
        string, message, "getting SDES chunk allocation size",
        puid, mediaChannelID, mMediaChannelID,
        size_t, chunkNumber, static_cast<size_t>(reportSpecific) - chunksCount
      );

      --chunksCount;

      while (remaining >= sizeof(BYTE)) {

        size_t preAllocationSize = mAllocationSize;

        BYTE type = *pos;
        internal::advancePos(pos, remaining);

        if (SDES::Chunk::kEndOfItemsType == type) {
          // skip NUL item (no length octet is present)

          // skip to next DWORD alignment
          auto diff = reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(contents);
          while ((0 != (diff % sizeof(DWORD))) &&
                  (remaining > 0))
          {
            // only NUL chunks are allowed
            if (SDES::Chunk::kEndOfItemsType != (*pos)) {
              ZS_EVENTING_3(x, w, Insane, RTCPPacketGetSDESAllocationSizeSDESTypeNotUnderstood, ol, RtcpPacket, Parse,
                string, message, "SDES item type is not understood",
                puid, mediaChannelID, mMediaChannelID,
                byte, type, *pos
              );
              return false;
            }

            internal::advancePos(pos, remaining);
            ++diff;
          }
          break;
        }

        if (remaining < sizeof(BYTE)) {
          ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
            string, message, "no length of SDES entry present",
            puid, mediaChannelID, mMediaChannelID,
            size_t, remaining, remaining,
            size_t, length, 0
          );
          return false;
        }

        size_t prefixLength = 0;
        size_t length = static_cast<size_t>(*pos);
        internal::advancePos(pos, remaining);

        if (remaining < length) {
          ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
            string, message, "malformed SDES length found",
            puid, mediaChannelID, mMediaChannelID,
            size_t, remaining, remaining,
            size_t, length, length
          );
          return false;
        }

        switch (type) {
          case SDES::Chunk::CName::kItemType: mAllocationSize += internal::alignedSize(sizeof(SDES::Chunk::CName)); break;
          case SDES::Chunk::Name::kItemType:  mAllocationSize += internal::alignedSize(sizeof(SDES::Chunk::Name)); break;
          case SDES::Chunk::Email::kItemType: mAllocationSize += internal::alignedSize(sizeof(SDES::Chunk::Email)); break;
          case SDES::Chunk::Phone::kItemType: mAllocationSize += internal::alignedSize(sizeof(SDES::Chunk::Phone)); break;
          case SDES::Chunk::Loc::kItemType:   mAllocationSize += internal::alignedSize(sizeof(SDES::Chunk::Loc)); break;
          case SDES::Chunk::Tool::kItemType:  mAllocationSize += internal::alignedSize(sizeof(SDES::Chunk::Tool)); break;
          case SDES::Chunk::Note::kItemType:  mAllocationSize += internal::alignedSize(sizeof(SDES::Chunk::Note)); break;
          case SDES::Chunk::Priv::kItemType:
          {
            {
              mAllocationSize += internal::alignedSize(sizeof(SDES::Chunk::Priv));

              if (length > 0) {
                prefixLength = static_cast<size_t>(*pos);

                if (prefixLength > (length-1)) goto illegal_priv_prefix;

                if (0 != prefixLength) {
                  mAllocationSize += internal::alignedSize(sizeof(char)*(prefixLength+1));
                }
              }
              break;
            }
          illegal_priv_prefix:
            {
              ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
                string, message, "malformed SDES Priv prefix found",
                puid, mediaChannelID, mMediaChannelID,
                size_t, remaining, remaining,
                size_t, length, length
              );
              trace("malformed SDES Priv prefix found");
              return false;
            }
            break;
          }
          case SDES::Chunk::Mid::kItemType:  mAllocationSize += internal::alignedSize(sizeof(SDES::Chunk::Mid)); break;
          case SDES::Chunk::Rid::kItemType:  mAllocationSize += internal::alignedSize(sizeof(SDES::Chunk::Rid)); break;
          default:
          {
            mAllocationSize += internal::alignedSize(sizeof(SDES::Chunk::Unknown));
            ZS_EVENTING_3(x, w, Insane, RTCPPacketGetSDESAllocationSizeSDESTypeNotUnderstood, ol, RtcpPacket, Parse,
              string, message, "SDES item type is not understood",
              puid, mediaChannelID, mMediaChannelID,
              byte, type, type
            );
            trace("SDES item type is not understood");
            break;
          }
        }

        if (0 != length) {
          mAllocationSize += internal::alignedSize((sizeof(char)*length)+sizeof(char));
        }

        ZS_EVENTING_7(x, i, Insane, RTCPPacketGetSDESAllocationSizeSDES, ol, RtcpPacket, Parse,
          string, message, "get SDES item allocation size",
          puid, mediaChannelID, mMediaChannelID,
          string, type, SDES::Chunk::StringItem::typeToString(type),
          byte, typeValue, type,
          size_t, prefixLength, prefixLength,
          size_t, length, length,
          size_t, allocationSize, (mAllocationSize - preAllocationSize)
        );
        internal::advancePos(pos, remaining, length);
      }

    }

    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getByeAllocationSize(
                                        BYTE version,
                                        BYTE padding,
                                        BYTE reportSpecific,
                                        const BYTE *contents,
                                        size_t contentSize
                                        )
  {
    ++mByeCount;

    size_t ssrcCount = static_cast<size_t>(reportSpecific);

    mAllocationSize += internal::alignedSize(sizeof(Bye)) + (internal::alignedSize(sizeof(DWORD)) * ssrcCount);

    const BYTE *pos = contents;
    size_t remaining = contentSize;

    if (remaining < (ssrcCount * sizeof(DWORD))) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed BYE SSRC size",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    internal::advancePos(pos, remaining, sizeof(DWORD) * ssrcCount);

    if (remaining < sizeof(BYTE)) return true;

    size_t length = static_cast<size_t>(*pos);

    if (length < 1) return true;

    if (length > remaining) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed BYE reason length",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, length
      );
      return false;
    }

    mAllocationSize += internal::alignedSize((sizeof(char)*(length+1)));
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getAppAllocationSize(
                                          BYTE version,
                                          BYTE padding,
                                          BYTE reportSpecific,
                                          const BYTE *contents,
                                          size_t contentSize
                                          )
  {
    ++mAppCount;

    mAllocationSize += internal::alignedSize(sizeof(App));

    size_t remaining = contentSize;

    if (remaining < (sizeof(DWORD)*2)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed APP length",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getTransportLayerFeedbackMessageAllocationSize(
                                                                  BYTE version,
                                                                  BYTE padding,
                                                                  BYTE reportSpecific,
                                                                  const BYTE *contents,
                                                                  size_t contentSize
                                                                  )
  {
    ++mTransportLayerFeedbackMessageCount;

    mAllocationSize += internal::alignedSize(sizeof(TransportLayerFeedbackMessage));

    const BYTE *pos = contents;
    size_t remaining = contentSize;

    if (remaining < (sizeof(DWORD)*2)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed transport layer feedback message length",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    internal::advancePos(pos, remaining, sizeof(DWORD)*2);

    bool result = false;

    switch (reportSpecific) {
      case TransportLayerFeedbackMessage::GenericNACK::kFmt:  result = getTransportLayerFeedbackMessageGenericNACKAllocationSize(reportSpecific, pos, remaining); break;
      case TransportLayerFeedbackMessage::TMMBR::kFmt:        result = getTransportLayerFeedbackMessageTMMBRAllocationSize(reportSpecific, pos, remaining); break;
      case TransportLayerFeedbackMessage::TMMBN::kFmt:        result = getTransportLayerFeedbackMessageTMMBNAllocationSize(reportSpecific, pos, remaining); break;
      default: {
        break;
      }
    }

    ZS_EVENTING_3(x, i, Insane, RTCPPacketGetTransportLayerFeedbackMessageAllocationSize, ol, RtcpPacket, Parse,
      puid, mediaChannelID, mMediaChannelID,
      string, fmt, FeedbackMessage::fmtToString(TransportLayerFeedbackMessage::kPayloadType, reportSpecific),
      byte, fmtNumber, reportSpecific
    );
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getPayloadSpecificFeedbackMessageAllocationSize(
                                                                    BYTE version,
                                                                    BYTE padding,
                                                                    BYTE reportSpecific,
                                                                    const BYTE *contents,
                                                                    size_t contentSize
                                                                    )
  {
    ++mPayloadSpecificFeedbackMessageCount;

    mAllocationSize += internal::alignedSize(sizeof(PayloadSpecificFeedbackMessage));

    const BYTE *pos = contents;
    size_t remaining = contentSize;

    if (remaining < (sizeof(DWORD)*2)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed transport layer feedback message length",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    internal::advancePos(pos, remaining, sizeof(DWORD)*2);

    bool result = false;

    char bogus[sizeof(DWORD)] {};
    const DWORD *usingSubType = reinterpret_cast<const DWORD *>(&(bogus[0]));

    switch (reportSpecific) {
      case PayloadSpecificFeedbackMessage::PLI::kFmt:   result = getPayloadSpecificFeedbackMessagePLIAllocationSize(reportSpecific, pos, remaining); break;
      case PayloadSpecificFeedbackMessage::SLI::kFmt:   result = getPayloadSpecificFeedbackMessageSLIAllocationSize(reportSpecific, pos, remaining); break;
      case PayloadSpecificFeedbackMessage::RPSI::kFmt:  result = getPayloadSpecificFeedbackMessageRPSIAllocationSize(reportSpecific, pos, remaining); break;
      case PayloadSpecificFeedbackMessage::FIR::kFmt:   result = getPayloadSpecificFeedbackMessageFIRAllocationSize(reportSpecific, pos, remaining); break;
      case PayloadSpecificFeedbackMessage::TSTR::kFmt:  result = getPayloadSpecificFeedbackMessageTSTRAllocationSize(reportSpecific, pos, remaining); break;
      case PayloadSpecificFeedbackMessage::TSTN::kFmt:  result = getPayloadSpecificFeedbackMessageTSTNAllocationSize(reportSpecific, pos, remaining); break;
      case PayloadSpecificFeedbackMessage::VBCM::kFmt:  result = getPayloadSpecificFeedbackMessageVBCMAllocationSize(reportSpecific, pos, remaining); break;
      case PayloadSpecificFeedbackMessage::AFB::kFmt:   {
        {
          if (remaining < sizeof(DWORD)) goto generic_afb;
          if (0 != memcmp(pos, reinterpret_cast<const BYTE *>("REMB"), sizeof(DWORD))) goto generic_afb;

          usingSubType = reinterpret_cast<const DWORD *>(pos);

          result = getPayloadSpecificFeedbackMessageREMBAllocationSize(reportSpecific, pos, remaining);
          break;
        }
      generic_afb:
        {
          result = getPayloadSpecificFeedbackMessageAFBAllocationSize(reportSpecific, pos, remaining);
          break;
        }
      }
      default: {
        break;
      }
    }

    ZS_EVENTING_3(x, i, Insane, RTCPPacketGetPayloadSpecificFeedbackMessageAllocationSize, ol, RtcpPacket, Parse,
      puid, mediaChannelID, mMediaChannelID,
      string, fmt, FeedbackMessage::fmtToString(TransportLayerFeedbackMessage::kPayloadType, reportSpecific, *usingSubType),
      byte, fmtNumber, reportSpecific
    );
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getXRAllocationSize(
                                        BYTE version,
                                        BYTE padding,
                                        BYTE reportSpecific,
                                        const BYTE *contents,
                                        size_t contentSize
                                        )
  {
    ++mXRCount;

    mAllocationSize += internal::alignedSize(sizeof(XR));

    const BYTE *pos = contents;
    size_t remaining = contentSize;

    if (remaining < sizeof(DWORD)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "XR is not a valid length",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    internal::advancePos(pos, remaining, sizeof(DWORD));

    while (remaining >= sizeof(DWORD)) {
      const BYTE *prePos = pos;
      size_t preAllocationSize = mAllocationSize;

      BYTE bt = pos[0];
      BYTE typeSpecific = pos[1];
      size_t blockLength = static_cast<size_t>(UseRTPUtils::getBE16(&(pos[2]))) * sizeof(DWORD);

      internal::advancePos(pos, remaining, sizeof(DWORD));

      if (remaining < blockLength) {
        ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
          string, message, "malformed XR block length found",
          puid, mediaChannelID, mMediaChannelID,
          size_t, remaining, remaining,
          size_t, length, blockLength
        );
        return false;
      }

      bool result = false;

      switch (bt) {
        case XR::LossRLEReportBlock::kBlockType:                result = getXRLossRLEReportBlockAllocationSize(typeSpecific, pos, blockLength); break;
        case XR::DuplicateRLEReportBlock::kBlockType:           result = getXRDuplicateRLEReportBlockAllocationSize(typeSpecific, pos, blockLength); break;
        case XR::PacketReceiptTimesReportBlock::kBlockType:     result = getXRPacketReceiptTimesReportBlockAllocationSize(typeSpecific, pos, blockLength); break;
        case XR::ReceiverReferenceTimeReportBlock::kBlockType:  result = getXRReceiverReferenceTimeReportBlockAllocationSize(typeSpecific, pos, blockLength); break;
        case XR::DLRRReportBlock::kBlockType:                   result = getXRDLRRReportBlockAllocationSize(typeSpecific, pos, blockLength); break;
        case XR::StatisticsSummaryReportBlock::kBlockType:      result = getXRStatisticsSummaryReportBlockAllocationSize(typeSpecific, pos, blockLength); break;
        case XR::VoIPMetricsReportBlock::kBlockType:            result = getXRVoIPMetricsReportBlockAllocationSize(typeSpecific, pos, blockLength); break;
        default:
        {
          result = getXRUnknownReportBlockAllocationSize(typeSpecific, pos, blockLength); break;
          break;
        }
      }

      if (!result) return false;

      internal::advancePos(pos, remaining, blockLength);

      ZS_EVENTING_5(x, i, Insane, RTCPPacketGetXRAllocationSize, ol, RtcpPacket, Parse,
        puid, mediaChannelID, mMediaChannelID,
        string, blockType, XR::ReportBlock::blockTypeToString(bt),
        size_t, blockLength, blockLength,
        uint64, consumed, (reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(prePos)),
        size_t, allocationSize, mAllocationSize - preAllocationSize
      );
    }

    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getUnknownReportAllocationSize(
                                                  BYTE version,
                                                  BYTE padding,
                                                  BYTE reportSpecific,
                                                  const BYTE *contents,
                                                  size_t contentSize
                                                  )
  {
    ++mUnknownReportCount;

    mAllocationSize += internal::alignedSize(sizeof(UnknownReport));
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getTransportLayerFeedbackMessageGenericNACKAllocationSize(
                                                                              BYTE fmt,
                                                                              const BYTE *contents,
                                                                              size_t contentSize
                                                                              )
  {
    size_t remaining = contentSize;

    if (remaining < sizeof(DWORD)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed generic NACK transport layer feedback message",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    size_t possibleNACKs = remaining / sizeof(DWORD);

    mAllocationSize += (internal::alignedSize(sizeof(TransportLayerFeedbackMessage::GenericNACK)) * possibleNACKs);
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getTransportLayerFeedbackMessageTMMBRAllocationSize(
                                                                        BYTE fmt,
                                                                        const BYTE *contents,
                                                                        size_t contentSize
                                                                        )
  {
    size_t remaining = contentSize;

    if (remaining < (sizeof(DWORD)*2)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed TMMBR transport layer feedback message",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    size_t possibleTMMBRs = remaining / (sizeof(DWORD)*2);

    mAllocationSize += (internal::alignedSize(sizeof(TransportLayerFeedbackMessage::TMMBR)) * possibleTMMBRs);
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getTransportLayerFeedbackMessageTMMBNAllocationSize(
                                                                        BYTE fmt,
                                                                        const BYTE *contents,
                                                                        size_t contentSize
                                                                        )
  {
    size_t remaining = contentSize;

    if (remaining < (sizeof(DWORD)*2)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed TMMBN transport layer feedback message",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    size_t possibleTMMBNs = remaining / (sizeof(DWORD)*2);

    mAllocationSize += (internal::alignedSize(sizeof(TransportLayerFeedbackMessage::TMMBN)) * possibleTMMBNs);
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getPayloadSpecificFeedbackMessagePLIAllocationSize(
                                                                      BYTE fmt,
                                                                      const BYTE *contents,
                                                                      size_t contentSize
                                                                      )
  {
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getPayloadSpecificFeedbackMessageSLIAllocationSize(
                                                                      BYTE fmt,
                                                                      const BYTE *contents,
                                                                      size_t contentSize
                                                                      )
  {
    size_t remaining = contentSize;

    if (remaining < sizeof(DWORD)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed SLI payload specific feedback message",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    size_t possibleSLIs = remaining / sizeof(DWORD);

    mAllocationSize += (internal::alignedSize(sizeof(PayloadSpecificFeedbackMessage::SLI)) * possibleSLIs);
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getPayloadSpecificFeedbackMessageRPSIAllocationSize(
                                                                        BYTE fmt,
                                                                        const BYTE *contents,
                                                                        size_t contentSize
                                                                        )
  {
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getPayloadSpecificFeedbackMessageFIRAllocationSize(
                                                                        BYTE fmt,
                                                                        const BYTE *contents,
                                                                        size_t contentSize
                                                                        )
  {
    size_t remaining = contentSize;

    if (remaining < (sizeof(DWORD)*2)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed FIR payload specific feedback message",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    size_t possibleFIRs = remaining / (sizeof(DWORD)*2);

    mAllocationSize += (internal::alignedSize(sizeof(PayloadSpecificFeedbackMessage::FIR)) * possibleFIRs);
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getPayloadSpecificFeedbackMessageTSTRAllocationSize(
                                                                        BYTE fmt,
                                                                        const BYTE *contents,
                                                                        size_t contentSize
                                                                        )
  {
    size_t remaining = contentSize;

    if (remaining < (sizeof(DWORD)*2)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed TSTR payload specific feedback message",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    size_t possibleTSTRs = remaining / (sizeof(DWORD)*2);

    mAllocationSize += (internal::alignedSize(sizeof(PayloadSpecificFeedbackMessage::TSTR)) * possibleTSTRs);
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getPayloadSpecificFeedbackMessageTSTNAllocationSize(
                                                                        BYTE fmt,
                                                                        const BYTE *contents,
                                                                        size_t contentSize
                                                                        )
  {
    size_t remaining = contentSize;

    if (remaining < (sizeof(DWORD)*2)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed TSTN payload specific feedback message",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    size_t possibleTSTNs = remaining / (sizeof(DWORD)*2);

    mAllocationSize += (internal::alignedSize(sizeof(PayloadSpecificFeedbackMessage::TSTN)) * possibleTSTNs);
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getPayloadSpecificFeedbackMessageVBCMAllocationSize(
                                                                        BYTE fmt,
                                                                        const BYTE *contents,
                                                                        size_t contentSize
                                                                        )
  {
    const BYTE *pos = contents;
    size_t remaining = contentSize;

    size_t possibleVBCMs = 0;

    while (remaining >= (sizeof(DWORD)*2)) {
      mAllocationSize += internal::alignedSize(sizeof(PayloadSpecificFeedbackMessage::VBCM));
      ++possibleVBCMs;

      size_t length = UseRTPUtils::getBE16(&(pos[6]));
      size_t modulas = length % sizeof(DWORD);
      size_t padding = ((0 != modulas) ? (sizeof(DWORD)-modulas) : 0);

      internal::advancePos(pos, remaining, sizeof(DWORD)*2);

      if (remaining < length) {
        ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
          string, message, "malformed VBCM payload specific feedback message",
          puid, mediaChannelID, mMediaChannelID,
          size_t, remaining, remaining,
          size_t, length, length
        );
        return false;
      }

      size_t skipLength = (((length + padding) > remaining) ? remaining : (length + padding));

      internal::advancePos(pos, remaining, skipLength);
    }

    if (possibleVBCMs < 1) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed VBCM payload specific feedback message (possible VBCMs too small)",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, possibleVBCMs
      );
      return false;
    }

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getPayloadSpecificFeedbackMessageAFBAllocationSize(
                                                                        BYTE fmt,
                                                                        const BYTE *contents,
                                                                        size_t contentSize
                                                                        )
  {
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getPayloadSpecificFeedbackMessageREMBAllocationSize(
                                                                      BYTE fmt,
                                                                      const BYTE *contents,
                                                                      size_t contentSize
                                                                      )
  {
    size_t remaining = contentSize;

    if (remaining < (sizeof(DWORD)*3)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed REMB payload specific feedback message",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    size_t possibleSSRCs = (remaining - (sizeof(DWORD)*2)) / sizeof(DWORD);

    mAllocationSize += (internal::alignedSize(sizeof(DWORD)) * possibleSSRCs);
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getXRLossRLEReportBlockAllocationSize(
                                                          BYTE typeSpecific,
                                                          const BYTE *contents,
                                                          size_t contentSize
                                                          )
  {
    mAllocationSize += internal::alignedSize(sizeof(XR::LossRLEReportBlock));

    const BYTE *pos = contents;
    size_t remaining = contentSize;

    if (remaining < (sizeof(DWORD)*2)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed loss RLE report block",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    internal::advancePos(pos, remaining, sizeof(DWORD)*2);

    size_t possibleChunks = remaining  / sizeof(WORD);

    if (0 != possibleChunks) {
      mAllocationSize += internal::alignedSize(sizeof(XR::RLEChunk)) * possibleChunks;
    }

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getXRDuplicateRLEReportBlockAllocationSize(
                                                              BYTE typeSpecific,
                                                              const BYTE *contents,
                                                              size_t contentSize
                                                              )
  {
    mAllocationSize += internal::alignedSize(sizeof(XR::DuplicateRLEReportBlock));

    const BYTE *pos = contents;
    size_t remaining = contentSize;

    if (remaining < (sizeof(DWORD)*2)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed duplicate RLE report block",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    internal::advancePos(pos, remaining, sizeof(DWORD)*2);

    size_t possibleChunks = remaining  / sizeof(WORD);

    if (0 != possibleChunks) {
      mAllocationSize += internal::alignedSize(sizeof(XR::RLEChunk)) * possibleChunks;
    }

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getXRPacketReceiptTimesReportBlockAllocationSize(
                                                                    BYTE typeSpecific,
                                                                    const BYTE *contents,
                                                                    size_t contentSize
                                                                    )
  {
    mAllocationSize += internal::alignedSize(sizeof(XR::PacketReceiptTimesReportBlock));

    const BYTE *pos = contents;
    size_t remaining = contentSize;

    if (remaining < (sizeof(DWORD)*2)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed packet receipt times report block",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    internal::advancePos(pos, remaining, sizeof(DWORD)*2);

    size_t possibleReceiptTimes = remaining  / sizeof(DWORD);

    if (0 != possibleReceiptTimes) {
      mAllocationSize += internal::alignedSize(sizeof(DWORD)) * possibleReceiptTimes;
    }

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getXRReceiverReferenceTimeReportBlockAllocationSize(
                                                                        BYTE typeSpecific,
                                                                        const BYTE *contents,
                                                                        size_t contentSize
                                                                        )
  {
    mAllocationSize += internal::alignedSize(sizeof(XR::ReceiverReferenceTimeReportBlock));
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::getXRDLRRReportBlockAllocationSize(
                                                      BYTE typeSpecific,
                                                      const BYTE *contents,
                                                      size_t contentSize
                                                      )
  {
    mAllocationSize += internal::alignedSize(sizeof(XR::DLRRReportBlock));

    size_t possibleSubBlocks = contentSize / (sizeof(DWORD) + sizeof(DWORD) + sizeof(DWORD));
    if (0 != possibleSubBlocks) {
      mAllocationSize += internal::alignedSize(sizeof(XR::DLRRReportBlock::SubBlock)) * possibleSubBlocks;
    }
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getXRStatisticsSummaryReportBlockAllocationSize(
                                                                    BYTE typeSpecific,
                                                                    const BYTE *contents,
                                                                    size_t contentSize
                                                                    )
  {
    mAllocationSize += internal::alignedSize(sizeof(XR::StatisticsSummaryReportBlock));
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getXRVoIPMetricsReportBlockAllocationSize(
                                                              BYTE typeSpecific,
                                                              const BYTE *contents,
                                                              size_t contentSize
                                                              )
  {
    mAllocationSize += internal::alignedSize(sizeof(XR::VoIPMetricsReportBlock));
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::getXRUnknownReportBlockAllocationSize(
                                                          BYTE typeSpecific,
                                                          const BYTE *contents,
                                                          size_t contentSize
                                                          )
  {
    mAllocationSize += internal::alignedSize(sizeof(XR::UnknownReportBlock));
    return true;
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCPPacket (parsing routines)
  #pragma mark

  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(
                          Report * &ioLastReport,
                          BYTE version,
                          BYTE padding,
                          BYTE reportSpecific,
                          BYTE pt,
                          const BYTE *contents,
                          size_t contentSize
                          )
  {
    Report *usingReport = NULL;

    switch (pt) {
      case SenderReport::kPayloadType:                                {
        auto temp = &(mFirstSenderReport[mSenderReportCount]);;
        fill(temp, version, padding, reportSpecific, pt, contents, contentSize);
        if (!parse(temp)) return false;
        usingReport = temp;
        break;
      }
      case ReceiverReport::kPayloadType:                              {
        auto temp = &(mFirstReceiverReport[mReceiverReportCount]);
        fill(temp, version, padding, reportSpecific, pt, contents, contentSize);
        if (!parse(temp)) return false;
        usingReport = temp;
        break;
      }
      case SDES::kPayloadType:                                        {
        auto temp = &(mFirstSDES[mSDESCount]);
        fill(temp, version, padding, reportSpecific, pt, contents, contentSize);
        if (!parse(temp)) return false;
        usingReport = temp;
        break;
      }
      case Bye::kPayloadType:                                         {
        auto temp = &(mFirstBye[mByeCount]);
        fill(temp, version, padding, reportSpecific, pt, contents, contentSize);
        if (!parse(temp)) return false;
        usingReport = temp;
        break;
      }
      case App::kPayloadType:                                         {
        auto temp = &(mFirstApp[mAppCount]);
        fill(temp, version, padding, reportSpecific, pt, contents, contentSize);
        if (!parse(temp)) return false;
        usingReport = temp;
        break;
      }
      case TransportLayerFeedbackMessage::kPayloadType:               {
        auto temp = &(mFirstTransportLayerFeedbackMessage[mTransportLayerFeedbackMessageCount]);
        fill(temp, version, padding, reportSpecific, pt, contents, contentSize);
        if (!parse(temp)) return false;
        usingReport = temp;
        break;
      }
      case PayloadSpecificFeedbackMessage::kPayloadType:              {
        auto temp = &(mFirstPayloadSpecificFeedbackMessage[mPayloadSpecificFeedbackMessageCount]);
        fill(temp, version, padding, reportSpecific, pt, contents, contentSize);
        if (!parse(temp)) return false;
        usingReport = temp;
        break;
      }
      case XR::kPayloadType:                                          {
        auto temp = &(mFirstXR[mXRCount]);
        fill(temp, version, padding, reportSpecific, pt, contents, contentSize);
        if (!parse(temp)) return false;
        usingReport = temp;
        break;
      }
      default:
      {
        auto temp = &(mFirstUnknownReport[mUnknownReportCount]);
        fill(temp, version, padding, reportSpecific, pt, contents, contentSize);
        if (!parse(temp)) return false;
        usingReport = temp;
        break;
      }
    }

    if (NULL != ioLastReport) {
      ioLastReport->mNext = usingReport;
    }

    ioLastReport = usingReport;
    return true;
  }

  //-------------------------------------------------------------------------
  void RTCPPacket::fill(
                        Report *report,
                        BYTE version,
                        BYTE padding,
                        BYTE reportSpecific,
                        BYTE pt,
                        const BYTE *contents,
                        size_t contentSize
                        )
  {
    if (contentSize > 0) {
      report->mPtr = contents;
      report->mSize = contentSize;
    }
    report->mVersion = version;
    report->mPadding = padding;
    report->mReportSpecific = reportSpecific;
    report->mPT = pt;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parseCommon(
                                SenderReceiverCommonReport *report,
                                size_t detailedHeaderSize
                                )
  {
    const BYTE *pos = report->ptr();
    size_t remaining = report->size();

    {
      if (remaining < sizeof(DWORD)) goto illegal_size;

      report->mSSRCOfSender = UseRTPUtils::getBE32(pos);

      internal::advancePos(pos, remaining, sizeof(DWORD));

      if (remaining < detailedHeaderSize) goto illegal_size;

      internal::advancePos(pos, remaining, detailedHeaderSize);

      size_t count = 0;

      if (report->rc() > 0) {
        report->mFirstReportBlock = new (allocateBuffer(internal::alignedSize(sizeof(SenderReceiverCommonReport::ReportBlock))*report->rc())) SenderReceiverCommonReport::ReportBlock[report->rc()];

        while ((remaining >= (sizeof(DWORD)*6)) &&
                (count < report->rc()))
        {
          SenderReceiverCommonReport::ReportBlock *block = &(report->mFirstReportBlock[count]);
          if (0 != count) {
            report->mFirstReportBlock[count-1].mNext = block;
          }

          block->mSSRC = UseRTPUtils::getBE32(&(pos[0]));
          block->mFractionLost = pos[4];
          block->mCumulativeNumberOfPacketsLost = UseRTPUtils::getBE32(&(pos[4]));
          block->mCumulativeNumberOfPacketsLost = block->mCumulativeNumberOfPacketsLost & 0x00FFFFFF;
          block->mExtendedHighestSequenceNumberReceived = UseRTPUtils::getBE32(&(pos[8]));
          block->mInterarrivalJitter = UseRTPUtils::getBE32(&(pos[12]));
          block->mLSR = UseRTPUtils::getBE32(&(pos[16]));
          block->mDLSR = UseRTPUtils::getBE32(&(pos[20]));

          internal::advancePos(pos, remaining, sizeof(DWORD)*6);

          ++count;
        }
      }

      if (count != report->rc()) goto illegal_size;

      if (0 != remaining) {
        report->mExtension = pos;
        report->mExtensionSize = remaining;
      }

      return true;
    }

  illegal_size:
    {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "unable to parse sender/receiver report (malformed packet)",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, detailedHeaderSize
      );
    }

    return false;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(SenderReport *report)
  {
    if (0 != mSenderReportCount) {
      (&(mFirstSenderReport[mSenderReportCount-1]))->mNextSenderReport = report;
    }
    ++mSenderReportCount;

    if (!parseCommon(report, sizeof(DWORD)*5)) return false;

    const BYTE *pos = report->ptr();

    report->mNTPTimestampMS = UseRTPUtils::getBE32(&(pos[4]));
    report->mNTPTimestampLS = UseRTPUtils::getBE32(&(pos[8]));
    report->mRTPTimestamp = UseRTPUtils::getBE32(&(pos[12]));
    report->mSenderPacketCount = UseRTPUtils::getBE32(&(pos[16]));
    report->mSenderOctetCount = UseRTPUtils::getBE32(&(pos[20]));

    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(ReceiverReport *report)
  {
    if (0 != mReceiverReportCount) {
      (&(mFirstReceiverReport[mReceiverReportCount-1]))->mNextReceiverReport = report;
    }
    ++mReceiverReportCount;

    if (!parseCommon(report, 0)) return false;

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(SDES *report)
  {
    if (0 != mSDESCount) {
      (&(mFirstSDES[mSDESCount-1]))->mNextSDES = report;
    }
    ++mSDESCount;

    if (0 == report->sc()) return true;

    size_t chunkCount = 0;

    const BYTE *pos = report->ptr();
    size_t remaining = report->size();

    report->mFirstChunk = new (allocateBuffer(internal::alignedSize(sizeof(SDES::Chunk))*(report->sc()))) SDES::Chunk[report->sc()];

    while ((remaining > sizeof(DWORD)) &&
            ((chunkCount < report->sc())))
    {
      SDES::Chunk *chunk = &(report->mFirstChunk[chunkCount]);
      if (0 != chunkCount) {
        report->mFirstChunk[chunkCount-1].mNext = chunk;
      }
      ++chunkCount;

      ZS_EVENTING_2(x, i, Insane, RTCPPacketParseSDES, ol, RtcpPacket, Parse,
        puid, mediaChannelID, mMediaChannelID,
        size_t, chunkCount, chunkCount - 1
      );

      chunk->mSSRC = UseRTPUtils::getBE32(pos);
      internal::advancePos(pos, remaining, sizeof(DWORD));

      const BYTE *startOfItems = pos;
      size_t remainingAtStartOfItems = remaining;

      // first do an items count
      while (remaining >= sizeof(BYTE)) {

        BYTE type = *pos;
        internal::advancePos(pos, remaining);

        if (SDES::Chunk::kEndOfItemsType == type) {
          // stop now (going to retry parsing again anyway)
          break;
        }

        ASSERT(remaining >= sizeof(BYTE))

        size_t length = static_cast<size_t>(*pos);
        internal::advancePos(pos, remaining);

        ASSERT(remaining >= length)

        switch (type) {
          case SDES::Chunk::CName::kItemType: ++(chunk->mCNameCount); break;
          case SDES::Chunk::Name::kItemType:  ++(chunk->mNameCount); break;
          case SDES::Chunk::Email::kItemType: ++(chunk->mEmailCount); break;
          case SDES::Chunk::Phone::kItemType: ++(chunk->mPhoneCount); break;
          case SDES::Chunk::Loc::kItemType:   ++(chunk->mLocCount); break;
          case SDES::Chunk::Tool::kItemType:  ++(chunk->mToolCount); break;
          case SDES::Chunk::Note::kItemType:  ++(chunk->mNoteCount); break;
          case SDES::Chunk::Priv::kItemType:  ++(chunk->mPrivCount); break;
          case SDES::Chunk::Mid::kItemType:   ++(chunk->mMidCount); break;
          case SDES::Chunk::Rid::kItemType:   ++(chunk->mRidCount); break;
          default:
          {
            ++(chunk->mUnknownCount);
            ZS_EVENTING_2(x, w, Insane, RTCPPacketParseWarningSDESItemNotUnderstood, ol, RtcpPacket, Parse,
              puid, mediaChannelID, mMediaChannelID,
              byte, type, type
            );
            break;
          }
        }

        internal::advancePos(pos, remaining, length);
      }

      // allocate space for items

      if (0 != chunk->mCNameCount) {
        chunk->mFirstCName = new (allocateBuffer(internal::alignedSize(sizeof(SDES::Chunk::CName))*(chunk->mCNameCount))) SDES::Chunk::CName[chunk->mCNameCount];
      }
      if (0 != chunk->mNameCount) {
        chunk->mFirstName = new (allocateBuffer(internal::alignedSize(sizeof(SDES::Chunk::Name))*(chunk->mNameCount))) SDES::Chunk::Name[chunk->mNameCount];
      }
      if (0 != chunk->mEmailCount) {
        chunk->mFirstEmail = new (allocateBuffer(internal::alignedSize(sizeof(SDES::Chunk::Email))*(chunk->mEmailCount))) SDES::Chunk::Email[chunk->mEmailCount];
      }
      if (0 != chunk->mPhoneCount) {
        chunk->mFirstPhone = new (allocateBuffer(internal::alignedSize(sizeof(SDES::Chunk::Phone))*(chunk->mPhoneCount))) SDES::Chunk::Phone[chunk->mPhoneCount];
      }
      if (0 != chunk->mLocCount) {
        chunk->mFirstLoc = new (allocateBuffer(internal::alignedSize(sizeof(SDES::Chunk::Loc))*(chunk->mLocCount))) SDES::Chunk::Loc[chunk->mLocCount];
      }
      if (0 != chunk->mToolCount) {
        chunk->mFirstTool = new (allocateBuffer(internal::alignedSize(sizeof(SDES::Chunk::Tool))*(chunk->mToolCount))) SDES::Chunk::Tool[chunk->mToolCount];
      }
      if (0 != chunk->mNoteCount) {
        chunk->mFirstNote = new (allocateBuffer(internal::alignedSize(sizeof(SDES::Chunk::Note))*(chunk->mNoteCount))) SDES::Chunk::Note[chunk->mNoteCount];
      }
      if (0 != chunk->mPrivCount) {
        chunk->mFirstPriv = new (allocateBuffer(internal::alignedSize(sizeof(SDES::Chunk::Priv))*(chunk->mPrivCount))) SDES::Chunk::Priv[chunk->mPrivCount];
      }
      if (0 != chunk->mMidCount) {
        chunk->mFirstMid = new (allocateBuffer(internal::alignedSize(sizeof(SDES::Chunk::Mid))*(chunk->mMidCount))) SDES::Chunk::Mid[chunk->mMidCount];
      }
      if (0 != chunk->mRidCount) {
        chunk->mFirstRid = new (allocateBuffer(internal::alignedSize(sizeof(SDES::Chunk::Rid))*(chunk->mRidCount))) SDES::Chunk::Rid[chunk->mRidCount];
      }
      if (0 != chunk->mUnknownCount) {
        chunk->mFirstUnknown = new (allocateBuffer(internal::alignedSize(sizeof(SDES::Chunk::Unknown))*(chunk->mUnknownCount))) SDES::Chunk::Unknown[chunk->mUnknownCount];
      }

      chunk->mCNameCount = 0;
      chunk->mNameCount = 0;
      chunk->mEmailCount = 0;
      chunk->mPhoneCount = 0;
      chunk->mLocCount = 0;
      chunk->mToolCount = 0;
      chunk->mNoteCount = 0;
      chunk->mPrivCount = 0;
      chunk->mMidCount = 0;
      chunk->mRidCount = 0;
      chunk->mUnknownCount = 0;

      // start over and now parse
      pos = startOfItems;
      remaining = remainingAtStartOfItems;

      while (remaining >= sizeof(BYTE)) {

        BYTE type = *pos;
        internal::advancePos(pos, remaining);

        if (SDES::Chunk::kEndOfItemsType == type) {
          // skip NUL item (no length octet is present)

          // skip to next DWORD alignment
          auto diff = reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(ptr());
          while ((0 != (diff % sizeof(DWORD))) &&
                  (remaining > 0))
          {
            // only NUL chunks are allowed
            ASSERT(SDES::Chunk::kEndOfItemsType == (*pos));
            internal::advancePos(pos, remaining);
            ++diff;
          }
          break;
        }

        ASSERT(remaining >= sizeof(BYTE))

        size_t length = static_cast<size_t>(*pos);
        internal::advancePos(pos, remaining);

        ASSERT(remaining >= length)

        SDES::Chunk::StringItem *item {};

        const char *prefixStr = NULL;
        size_t prefixLen = 0;

        switch (type) {
          case SDES::Chunk::CName::kItemType: {
            item = &(chunk->mFirstCName[chunk->mCNameCount]);
            if (0 != chunk->mCNameCount) {
              (&(chunk->mFirstCName[chunk->mCNameCount-1]))->mNext = item;
            }
            ++(chunk->mCNameCount);
            break;
          }
          case SDES::Chunk::Name::kItemType:  {
            item = &(chunk->mFirstName[chunk->mNameCount]);
            if (0 != chunk->mNameCount) {
              (&(chunk->mFirstName[chunk->mNameCount-1]))->mNext = item;
            }
            ++(chunk->mNameCount);
            break;
          }
          case SDES::Chunk::Email::kItemType: {
            item = &(chunk->mFirstEmail[chunk->mEmailCount]);
            if (0 != chunk->mEmailCount) {
              (&(chunk->mFirstEmail[chunk->mEmailCount-1]))->mNext = item;
            }
            ++(chunk->mEmailCount);
            break;
          }
          case SDES::Chunk::Phone::kItemType: {
            item = &(chunk->mFirstPhone[chunk->mPhoneCount]);
            if (0 != chunk->mPhoneCount) {
              (&(chunk->mFirstPhone[chunk->mPhoneCount-1]))->mNext = item;
            }
            ++(chunk->mPhoneCount);
            break;
          }
          case SDES::Chunk::Loc::kItemType:   {
            item = &(chunk->mFirstLoc[chunk->mLocCount]);
            if (0 != chunk->mLocCount) {
              (&(chunk->mFirstLoc[chunk->mLocCount-1]))->mNext = item;
            }
            ++(chunk->mLocCount);
            break;
          }
          case SDES::Chunk::Tool::kItemType:  {
            item = &(chunk->mFirstTool[chunk->mToolCount]);
            if (0 != chunk->mToolCount) {
              (&(chunk->mFirstTool[chunk->mToolCount-1]))->mNext = item;
            }
            ++(chunk->mToolCount);
            break;
          }
          case SDES::Chunk::Note::kItemType:  {
            item = &(chunk->mFirstNote[chunk->mNoteCount]);
            if (0 != chunk->mNoteCount) {
              (&(chunk->mFirstNote[chunk->mNoteCount-1]))->mNext = item;
            }
            ++(chunk->mNoteCount);
            break;
          }
          case SDES::Chunk::Priv::kItemType:  {
            SDES::Chunk::Priv *priv = &(chunk->mFirstPriv[chunk->mPrivCount]);
            if (0 != chunk->mPrivCount) {
              (&(chunk->mFirstPriv[chunk->mPrivCount-1]))->mNext = priv;
            }

            if (length > 0) {
              prefixLen = static_cast<size_t>(*pos);
              internal::advancePos(pos, remaining);
              --length;
              if (0 != prefixLen) {
                priv->mPrefix = new (allocateBuffer(sizeof(char)*(prefixLen+1))) char [prefixLen+1];
                priv->mPrefixLength = prefixLen;
                memcpy(const_cast<char *>(priv->mPrefix), pos, prefixLen);

                internal::advancePos(pos, remaining, prefixLen);
                ASSERT(length >= prefixLen)

                length -= prefixLen;
              }
            }

            item = priv;

            ++(chunk->mPrivCount);
            break;
          }
          case SDES::Chunk::Mid::kItemType:  {
            item = &(chunk->mFirstMid[chunk->mMidCount]);
            if (0 != chunk->mMidCount) {
              (&(chunk->mFirstMid[chunk->mMidCount-1]))->mNext = item;
            }
            ++(chunk->mMidCount);
            break;
          }
          case SDES::Chunk::Rid::kItemType:  {
            item = &(chunk->mFirstRid[chunk->mRidCount]);
            if (0 != chunk->mRidCount) {
              (&(chunk->mFirstRid[chunk->mRidCount-1]))->mNext = item;
            }
            ++(chunk->mRidCount);
            break;
          }
          default:
          {
            item = &(chunk->mFirstUnknown[chunk->mUnknownCount]);
            if (0 != chunk->mUnknownCount) {
              (&(chunk->mFirstUnknown[chunk->mUnknownCount-1]))->mNext = item;
            }
            ++(chunk->mUnknownCount);
            ZS_EVENTING_2(x, w, Insane, RTCPPacketParseSDESItemTypeNotUnderstood, ol, RtcpPacket, Parse,
              puid, mediaChannelID, mMediaChannelID,
              byte, type, type
            );
            break;
          }
        }

        if (NULL != item) {
          item->mType = type;
          if (length > 0) {
            item->mValue = new (allocateBuffer(length+1)) char [length+1];
            item->mLength = length;
            memcpy(const_cast<char *>(item->mValue), pos, length);
          }
          if (NULL != prefixStr) {
            ZS_EVENTING_7(x, i, Insane, RTCPPacketParseSDESPrefixItemPrefix, ol, RtcpPacket, Parse,
              puid, mediaChannelID, mMediaChannelID,
              string, type, item->typeToString(),
              size_t, chunkCount, chunkCount,
              size_t, prefixLen, prefixLen,
              string, prefixStr, prefixStr,
              size_t, length, length,
              string, value, (NULL != item->mValue ? item->mValue : NULL)
            );
          } else {
            ZS_EVENTING_5(x, i, Insane, RTCPPacketParseSDESItemPrefix, ol, RtcpPacket, Parse,
              puid, mediaChannelID, mMediaChannelID,
              string, type, item->typeToString(),
              size_t, chunkCount, chunkCount,
              size_t, length, length,
              string, value, (NULL != item->mValue ? item->mValue : NULL)
            );
          }
        }

        ++(chunk->mCount);

        internal::advancePos(pos, remaining, length);
      }

    }

    if (chunkCount != report->sc()) {
      ZS_EVENTING_3(x, w, Trace, RTCPPacketParseSDESChunkCountAndPacketCountMismatch, ol, RtcpPacket, Parse,
        puid, mediaChannelID, mMediaChannelID,
        size_t, chunkCount, chunkCount,
        size_t, sc, report->sc()
      );
      return false;
    }

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(Bye *report)
  {
    if (0 != mByeCount) {
      (&(mFirstBye[mByeCount-1]))->mNextBye = report;
    }
    ++mByeCount;

    const BYTE *pos = report->ptr();
    size_t remaining = report->size();

    if (NULL == pos) {
      if (0 != report->sc()) {
        ZS_EVENTING_2(x, w, Trace, RTCPPacketParseByeReportCountMissingSSRCs, ol, RtcpPacket, Parse,
          puid, mediaChannelID, mMediaChannelID,
          size_t, sc, report->sc()
        );
        trace("BYE report count > 0 but does not contain SSRCs");
        return false;
      }

      return true;
    }

    if (0 != report->sc()) {
      report->mSSRCs = new (allocateBuffer(internal::alignedSize(sizeof(DWORD))*(report->sc()))) DWORD[report->sc()];
    }

    {
      size_t index = 0;

      while (index < report->sc()) {
        if (remaining < sizeof(DWORD)) goto illegal_remaining;

        report->mSSRCs[index] = UseRTPUtils::getBE32(pos);
        internal::advancePos(pos, remaining, sizeof(DWORD));
        ++index;
      }

      if (remaining > sizeof(BYTE)) {
        size_t length = static_cast<size_t>(*pos);
        internal::advancePos(pos, remaining);

        if (remaining < length) goto illegal_remaining;

        if (length > 0) {
          report->mReasonForLeaving = new (allocateBuffer(internal::alignedSize((sizeof(char)*length)+sizeof(char)))) char[length+1];
          memcpy(const_cast<char *>(report->mReasonForLeaving), pos, length);
        }
      }

      return true;
    }

  illegal_remaining:
    {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed BYE",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, (reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->ptr()))
      );
    }
    return false;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(App *report)
  {
    if (0 != mAppCount) {
      (&(mFirstApp[mAppCount-1]))->mNextApp = report;
    }
    ++mAppCount;

    const BYTE *pos = report->ptr();
    size_t remaining = report->size();

    {
      if (remaining < sizeof(DWORD)) goto illegal_remaining;

      report->mSSRC = UseRTPUtils::getBE32(pos);
      internal::advancePos(pos, remaining, sizeof(DWORD));

      if (remaining < sizeof(DWORD)) goto illegal_remaining;

      for (size_t index = 0; index < sizeof(DWORD); ++index) {
        report->mName[index] = static_cast<char>(pos[index]);
      }
      internal::advancePos(pos, remaining, sizeof(DWORD));

      if (0 != remaining) {
        report->mData = pos;
        report->mDataSize = remaining;
      }

      return true;
    }

  illegal_remaining:
    {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed APP",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, (reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->ptr()))
      );
    }
    return false;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(TransportLayerFeedbackMessage *report)
  {
    if (0 != mTransportLayerFeedbackMessageCount) {
      (&(mFirstTransportLayerFeedbackMessage[mTransportLayerFeedbackMessageCount-1]))->mNextTransportLayerFeedbackMessage = report;
    }
    ++mTransportLayerFeedbackMessageCount;

    const BYTE *pos = report->ptr();
    size_t remaining = report->size();

    {
      if (remaining < (sizeof(DWORD)*2)) goto illegal_remaining;

      fill(report, pos, remaining);

      bool result = false;

      switch (report->fmt()) {
        case TransportLayerFeedbackMessage::GenericNACK::kFmt:  result = parseGenericNACK(report); break;
        case TransportLayerFeedbackMessage::TMMBR::kFmt:        result = parseTMMBR(report); break;
        case TransportLayerFeedbackMessage::TMMBN::kFmt:        result = parseTMMBN(report); break;
        default: {
          result = parseUnknown(report);
          break;
        }
      }

      ZS_EVENTING_3(x, i, Insane, RTCPPacketParseTransportLayerFeedbackMessage, ol, RtcpPacket, Parse,
        puid, mediaChannelID, mMediaChannelID,
        string, fmt, report->fmtToString(),
        byte, fmtNumber, report->reportSpecific()
      );
      return true;
    }

  illegal_remaining:
    {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed transport layer feedback message",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, (reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->ptr()))
      );
    }
    return false;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(PayloadSpecificFeedbackMessage *report)
  {
    if (0 != mPayloadSpecificFeedbackMessageCount) {
      (&(mFirstPayloadSpecificFeedbackMessage[mPayloadSpecificFeedbackMessageCount-1]))->mNextPayloadSpecificFeedbackMessage = report;
    }
    ++mPayloadSpecificFeedbackMessageCount;

    const BYTE *pos = report->ptr();
    size_t remaining = report->size();

    {
      if (remaining < (sizeof(DWORD)*2)) goto illegal_remaining;

      fill(report, pos, remaining);
      internal::advancePos(pos, remaining, sizeof(DWORD)*2);

      bool result = false;

      switch (report->fmt()) {
        case PayloadSpecificFeedbackMessage::PLI::kFmt:   result = parsePLI(report); break;
        case PayloadSpecificFeedbackMessage::SLI::kFmt:   result = parseSLI(report); break;
        case PayloadSpecificFeedbackMessage::RPSI::kFmt:  result = parseRPSI(report); break;
        case PayloadSpecificFeedbackMessage::FIR::kFmt:   result = parseFIR(report); break;
        case PayloadSpecificFeedbackMessage::TSTR::kFmt:  result = parseTSTR(report); break;
        case PayloadSpecificFeedbackMessage::TSTN::kFmt:  result = parseTSTN(report); break;
        case PayloadSpecificFeedbackMessage::VBCM::kFmt:  result = parseVBCM(report); break;
        case PayloadSpecificFeedbackMessage::AFB::kFmt:   {
          {
            if (remaining < sizeof(DWORD)) goto generic_afb;
            if (0 != memcmp(pos, reinterpret_cast<const BYTE *>("REMB"), sizeof(DWORD))) goto generic_afb;

            result = parseREMB(report);
            break;
          }
        generic_afb:
          {
            result = parseAFB(report);
            break;
          }
        }
        default: {
          result = parseUnknown(report);
          break;
        }
      }

      ZS_EVENTING_3(x, i, Insane, RTCPPacketParsePayloadSpecificFeedbackMessage, ol, RtcpPacket, Parse,
        puid, mediaChannelID, mMediaChannelID,
        string, fmt, report->fmtToString(),
        byte, fmtNumber, report->reportSpecific()
      );
      return true;
    }

  illegal_remaining:
    {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed payload specific feedback message",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, (reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->ptr()))
      );
    }
    return false;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(XR *report)
  {
    if (0 != mXRCount) {
      (&(mFirstXR[mXRCount-1]))->mNextXR = report;
    }
    ++mXRCount;

    const BYTE *pos = report->ptr();
    size_t remaining = report->size();

    {
      if (remaining < sizeof(DWORD)) goto illegal_remaining;

      report->mSSRC = UseRTPUtils::getBE32(pos);
      internal::advancePos(pos, remaining, sizeof(DWORD));

      // first count the totals for each XR block type
      while (remaining >= sizeof(DWORD)) {

        BYTE bt = pos[0];
        size_t blockLength = static_cast<size_t>(UseRTPUtils::getBE16(&(pos[2]))) * sizeof(DWORD);

        internal::advancePos(pos, remaining, sizeof(DWORD));

        ++report->mReportBlockCount;

        if (remaining < blockLength) {
          ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
            string, message, "illegal XR block length found",
            puid, mediaChannelID, mMediaChannelID,
            size_t, remaining, remaining,
            size_t, length, blockLength
          );
          return false;
        }

        switch (bt) {
          case XR::LossRLEReportBlock::kBlockType:                ++(report->mLossRLEReportBlockCount); break;
          case XR::DuplicateRLEReportBlock::kBlockType:           ++(report->mDuplicateRLEReportBlockCount); break;
          case XR::PacketReceiptTimesReportBlock::kBlockType:     ++(report->mPacketReceiptTimesReportBlockCount); break;
          case XR::ReceiverReferenceTimeReportBlock::kBlockType:  ++(report->mReceiverReferenceTimeReportBlockCount); break;
          case XR::DLRRReportBlock::kBlockType:                   ++(report->mDLRRReportBlockCount); break;
          case XR::StatisticsSummaryReportBlock::kBlockType:      ++(report->mStatisticsSummaryReportBlockCount); break;
          case XR::VoIPMetricsReportBlock::kBlockType:            ++(report->mVoIPMetricsReportBlockCount); break;
          default:
          {
            ++(report->mUnknownReportBlockCount); break;
            break;
          }
        }
          
        internal::advancePos(pos, remaining, blockLength);
      }

      if (0 != report->mLossRLEReportBlockCount) {
        report->mFirstLossRLEReportBlock = new (allocateBuffer(internal::alignedSize(sizeof(XR::LossRLEReportBlock))*(report->mLossRLEReportBlockCount))) XR::LossRLEReportBlock[report->mLossRLEReportBlockCount];
      }
      if (0 != report->mDuplicateRLEReportBlockCount) {
        report->mFirstDuplicateRLEReportBlock = new (allocateBuffer(internal::alignedSize(sizeof(XR::DuplicateRLEReportBlock))*(report->mDuplicateRLEReportBlockCount))) XR::DuplicateRLEReportBlock[report->mDuplicateRLEReportBlockCount];
      }
      if (0 != report->mPacketReceiptTimesReportBlockCount) {
        report->mFirstPacketReceiptTimesReportBlock = new (allocateBuffer(internal::alignedSize(sizeof(XR::PacketReceiptTimesReportBlock))*(report->mPacketReceiptTimesReportBlockCount))) XR::PacketReceiptTimesReportBlock[report->mPacketReceiptTimesReportBlockCount];
      }
      if (0 != report->mReceiverReferenceTimeReportBlockCount) {
        report->mFirstReceiverReferenceTimeReportBlock = new (allocateBuffer(internal::alignedSize(sizeof(XR::ReceiverReferenceTimeReportBlock))*(report->mReceiverReferenceTimeReportBlockCount))) XR::ReceiverReferenceTimeReportBlock[report->mReceiverReferenceTimeReportBlockCount];
      }
      if (0 != report->mDLRRReportBlockCount) {
        report->mFirstDLRRReportBlock = new (allocateBuffer(internal::alignedSize(sizeof(XR::DLRRReportBlock))*(report->mDLRRReportBlockCount))) XR::DLRRReportBlock[report->mDLRRReportBlockCount];
      }
      if (0 != report->mStatisticsSummaryReportBlockCount) {
        report->mFirstStatisticsSummaryReportBlock = new (allocateBuffer(internal::alignedSize(sizeof(XR::StatisticsSummaryReportBlock))*(report->mStatisticsSummaryReportBlockCount))) XR::StatisticsSummaryReportBlock[report->mStatisticsSummaryReportBlockCount];
      }
      if (0 != report->mVoIPMetricsReportBlockCount) {
        report->mFirstVoIPMetricsReportBlock = new (allocateBuffer(internal::alignedSize(sizeof(XR::VoIPMetricsReportBlock))*(report->mVoIPMetricsReportBlockCount))) XR::VoIPMetricsReportBlock[report->mVoIPMetricsReportBlockCount];
      }
      if (0 != report->mUnknownReportBlockCount) {
        report->mFirstUnknownReportBlock = new (allocateBuffer(internal::alignedSize(sizeof(XR::UnknownReportBlock))*(report->mUnknownReportBlockCount))) XR::UnknownReportBlock[report->mUnknownReportBlockCount];
      }

      report->mLossRLEReportBlockCount = 0;
      report->mDuplicateRLEReportBlockCount = 0;
      report->mPacketReceiptTimesReportBlockCount = 0;
      report->mReceiverReferenceTimeReportBlockCount = 0;
      report->mDLRRReportBlockCount = 0;
      report->mStatisticsSummaryReportBlockCount = 0;
      report->mVoIPMetricsReportBlockCount = 0;
      report->mUnknownReportBlockCount = 0;

      XR::ReportBlock *previousReportBlock = NULL;

      // reset to start of buffer
      pos = report->ptr();
      remaining = report->size();

      internal::advancePos(pos, remaining, sizeof(DWORD));

      // parse each XR report block
      while (remaining >= sizeof(DWORD)) {
        const BYTE *prePos = pos;
        size_t preAllocationSize = mAllocationSize;

        BYTE bt = pos[0];
        BYTE typeSpecific = pos[1];
        size_t blockLength = static_cast<size_t>(UseRTPUtils::getBE16(&(pos[2]))) * sizeof(DWORD);

        internal::advancePos(pos, remaining, sizeof(DWORD));

        if (remaining < blockLength) {
          ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
            string, message, "malformed XR block length found",
            puid, mediaChannelID, mMediaChannelID,
            size_t, remaining, remaining,
            size_t, length, blockLength
          );
          return false;
        }

        XR::ReportBlock *usingBlock = NULL;

        switch (bt) {
          case XR::LossRLEReportBlock::kBlockType:                {
            auto reportBlock = &(report->mFirstLossRLEReportBlock[report->mLossRLEReportBlockCount]);
            usingBlock = reportBlock;
            if (0 != report->mLossRLEReportBlockCount) {
              (&(report->mFirstLossRLEReportBlock[report->mLossRLEReportBlockCount-1]))->mNextLossRLE = reportBlock;
            }
            ++(report->mLossRLEReportBlockCount);
            fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
            if (!parse(report, reportBlock)) return false;
            break;
          }
          case XR::DuplicateRLEReportBlock::kBlockType:           {
            auto reportBlock = &(report->mFirstDuplicateRLEReportBlock[report->mDuplicateRLEReportBlockCount]);
            usingBlock = reportBlock;
            if (0 != report->mDuplicateRLEReportBlockCount) {
              (&(report->mFirstDuplicateRLEReportBlock[report->mDuplicateRLEReportBlockCount-1]))->mNextDuplicateRLE = reportBlock;
            }
            ++(report->mDuplicateRLEReportBlockCount);
            fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
            if (!parse(report, reportBlock)) return false;
            break;
          }
          case XR::PacketReceiptTimesReportBlock::kBlockType:     {
            auto reportBlock = &(report->mFirstPacketReceiptTimesReportBlock[report->mPacketReceiptTimesReportBlockCount]);
            usingBlock = reportBlock;
            if (0 != report->mPacketReceiptTimesReportBlockCount) {
              (&(report->mFirstPacketReceiptTimesReportBlock[report->mPacketReceiptTimesReportBlockCount-1]))->mNextPacketReceiptTimesReportBlock = reportBlock;
            }
            ++(report->mPacketReceiptTimesReportBlockCount);
            fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
            if (!parse(report, reportBlock)) return false;
            break;
          }
          case XR::ReceiverReferenceTimeReportBlock::kBlockType:  {
            auto reportBlock = &(report->mFirstReceiverReferenceTimeReportBlock[report->mReceiverReferenceTimeReportBlockCount]);
            usingBlock = reportBlock;
            if (0 != report->mReceiverReferenceTimeReportBlockCount) {
              (&(report->mFirstReceiverReferenceTimeReportBlock[report->mReceiverReferenceTimeReportBlockCount-1]))->mNextReceiverReferenceTimeReportBlock = reportBlock;
            }
            ++(report->mReceiverReferenceTimeReportBlockCount);
            fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
            if (!parse(report, reportBlock)) return false;
            break;
          }
          case XR::DLRRReportBlock::kBlockType:                   {
            auto reportBlock = &(report->mFirstDLRRReportBlock[report->mDLRRReportBlockCount]);
            usingBlock = reportBlock;
            if (0 != report->mDLRRReportBlockCount) {
              (&(report->mFirstDLRRReportBlock[report->mDLRRReportBlockCount-1]))->mNextDLRRReportBlock = reportBlock;
            }
            ++(report->mDLRRReportBlockCount);
            fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
            if (!parse(report, reportBlock)) return false;
            break;
          }
          case XR::StatisticsSummaryReportBlock::kBlockType:      {
            auto reportBlock = &(report->mFirstStatisticsSummaryReportBlock[report->mStatisticsSummaryReportBlockCount]);
            usingBlock = reportBlock;
            if (0 != report->mStatisticsSummaryReportBlockCount) {
              (&(report->mFirstStatisticsSummaryReportBlock[report->mStatisticsSummaryReportBlockCount-1]))->mNextStatisticsSummaryReportBlock = reportBlock;
            }
            ++(report->mStatisticsSummaryReportBlockCount);
            fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
            if (!parse(report, reportBlock)) return false;
            break;
          }
          case XR::VoIPMetricsReportBlock::kBlockType:            {
            auto reportBlock = &(report->mFirstVoIPMetricsReportBlock[report->mVoIPMetricsReportBlockCount]);
            usingBlock = reportBlock;
            if (0 != report->mVoIPMetricsReportBlockCount) {
              (&(report->mFirstVoIPMetricsReportBlock[report->mVoIPMetricsReportBlockCount-1]))->mNextVoIPMetricsReportBlock = reportBlock;
            }
            ++(report->mVoIPMetricsReportBlockCount);
            fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
            if (!parse(report, reportBlock)) return false;
            break;
          }
          default:                                                {
            auto reportBlock = &(report->mFirstUnknownReportBlock[report->mUnknownReportBlockCount]);
            usingBlock = reportBlock;
            if (0 != report->mUnknownReportBlockCount) {
              (&(report->mFirstUnknownReportBlock[report->mUnknownReportBlockCount-1]))->mNextUnknownReportBlock = reportBlock;
            }
            ++(report->mUnknownReportBlockCount);
            fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
            if (!parse(report, reportBlock)) return false;
            break;
          }
        }

        internal::advancePos(pos, remaining, blockLength);

        ZS_EVENTING_5(x, i, Insane, RTCPPacketParseXR, ol, RtcpPacket, Parse,
          puid, mediaChannelID, mMediaChannelID,
          string, blockType, usingBlock->blockTypeToString(),
          size_t, blockLength, blockLength,
          size_t, consumed, (reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(prePos)),
          size_t, allocationConsumed, preAllocationSize - mAllocationSize
        );
      }

      return true;
    }

  illegal_remaining:
    {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed XR when parsing",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, (reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->ptr()))
      );
    }
    return false;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(UnknownReport *report)
  {
    if (0 != mUnknownReportCount) {
      (&(mFirstUnknownReport[mUnknownReportCount-1]))->mNextUnknown = report;
    }
    ++mUnknownReportCount;

    return true;
  }
    
  //-------------------------------------------------------------------------
  void RTCPPacket::fill(FeedbackMessage *report, const BYTE *contents, size_t contentSize)
  {
    const BYTE *pos = contents;
    size_t remaining = contentSize;

    report->mSSRCOfPacketSender = UseRTPUtils::getBE32(&(pos[0]));
    report->mSSRCOfMediaSource = UseRTPUtils::getBE32(&(pos[4]));

    internal::advancePos(pos, remaining, sizeof(DWORD)*2);

    if (remaining > sizeof(BYTE)) {
      report->mFCI = pos;
      report->mFCISize = remaining;
    }
  }

  //-------------------------------------------------------------------------
  void RTCPPacket::fill(
                        XR *report,
                        XR::ReportBlock *reportBlock,
                        XR::ReportBlock * &ioPreviousReportBlock,
                        BYTE blockType,
                        BYTE typeSpecific,
                        const BYTE *contents,
                        size_t contentSize
                        )
  {
    reportBlock->mBlockType = blockType;
    reportBlock->mTypeSpecific = typeSpecific;
    if (0 != contentSize) {
      reportBlock->mTypeSpecificContents = contents;
      reportBlock->mTypeSpecificContentSize = contentSize;
    }

    if (NULL == report->mFirstReportBlock) {
      report->mFirstReportBlock = reportBlock;
    }
    if (NULL != ioPreviousReportBlock) {
      ioPreviousReportBlock->mNext = reportBlock;
    }
    ioPreviousReportBlock = reportBlock;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parseGenericNACK(TransportLayerFeedbackMessage *report)
  {
    typedef TransportLayerFeedbackMessage::GenericNACK GenericNACK;
    const BYTE *pos = report->fci();
    size_t remaining = report->fciSize();

    size_t possibleNACKs = remaining / sizeof(DWORD);

    ASSERT(0 != possibleNACKs)

    report->mFirstGenericNACK = new (allocateBuffer(internal::alignedSize(sizeof(GenericNACK))*possibleNACKs)) GenericNACK[possibleNACKs];

    while (remaining >= sizeof(DWORD)) {
      GenericNACK *nack = &(report->mFirstGenericNACK[report->mGenericNACKCount]);

      nack->mPID = UseRTPUtils::getBE16(&(pos[0]));
      nack->mBLP = UseRTPUtils::getBE16(&(pos[2]));

      internal::advancePos(pos, remaining, sizeof(DWORD));

      ++(report->mGenericNACKCount);
    }

    ASSERT(possibleNACKs == report->mGenericNACKCount)

    return true;
  }
    
  //-------------------------------------------------------------------------
  void RTCPPacket::fillTMMBRCommon(
                                    TransportLayerFeedbackMessage *report,
                                    TransportLayerFeedbackMessage::TMMBRCommon *common,
                                    const BYTE *pos
                                    )
  {
    //typedef TransportLayerFeedbackMessage::TMMBRCommon TMMBRCommon;

    common->mSSRC = UseRTPUtils::getBE32(&(pos[0]));
    common->mMxTBRExp = RTCP_GET_BITS(pos[4], 0x3F, 2);
    common->mMxTBRMantissa = RTCP_GET_BITS(UseRTPUtils::getBE32(&(pos[4])), 0x1FFFF, 9);
    common->mMeasuredOverhead = RTCP_GET_BITS(UseRTPUtils::getBE32(&(pos[4])), 0x1FF, 0);
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parseTMMBR(TransportLayerFeedbackMessage *report)
  {
    typedef TransportLayerFeedbackMessage::TMMBR TMMBR;

    const BYTE *pos = report->fci();
    size_t remaining = report->fciSize();

    size_t possibleTMMBRs = remaining / (sizeof(DWORD)*2);

    ASSERT(0 != possibleTMMBRs)

    report->mFirstTMMBR = new (allocateBuffer(internal::alignedSize(sizeof(TMMBR))*possibleTMMBRs)) TMMBR[possibleTMMBRs];

    while (remaining >= (sizeof(DWORD)*2)) {
      TMMBR *tmmbr = &(report->mFirstTMMBR[report->mTMMBRCount]);

      fillTMMBRCommon(report, tmmbr, pos);

      internal::advancePos(pos, remaining, sizeof(DWORD)*2);

      ++(report->mTMMBRCount);
    }

    ASSERT(possibleTMMBRs == report->mTMMBRCount)

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parseTMMBN(TransportLayerFeedbackMessage *report)
  {
    typedef TransportLayerFeedbackMessage::TMMBN TMMBN;

    const BYTE *pos = report->fci();
    size_t remaining = report->fciSize();

    size_t possibleTMMBNs = remaining / (sizeof(DWORD)*2);

    if (possibleTMMBNs < 1) {
      report->mFirstTMMBN = NULL;
      report->mTMMBNCount = 0;
      return true;
    }

    report->mFirstTMMBN = new (allocateBuffer(internal::alignedSize(sizeof(TMMBN))*possibleTMMBNs)) TMMBN[possibleTMMBNs];

    while (remaining >= (sizeof(DWORD)*2)) {
      TMMBN *tmmbr = &(report->mFirstTMMBN[report->mTMMBNCount]);

      fillTMMBRCommon(report, tmmbr, pos);

      internal::advancePos(pos, remaining, sizeof(DWORD)*2);

      ++(report->mTMMBNCount);
    }

    ASSERT(possibleTMMBNs == report->mTMMBNCount)

    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parseUnknown(TransportLayerFeedbackMessage *report)
  {
    report->mUnknown = report;
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parsePLI(PayloadSpecificFeedbackMessage *report)
  {
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parseSLI(PayloadSpecificFeedbackMessage *report)
  {
    typedef PayloadSpecificFeedbackMessage::SLI SLI;

    const BYTE *pos = report->fci();
    size_t remaining = report->fciSize();

    size_t possibleSLIs = remaining / (sizeof(DWORD));

    ASSERT(0 != possibleSLIs)

    report->mFirstSLI = new (allocateBuffer(internal::alignedSize(sizeof(SLI))*possibleSLIs)) SLI[possibleSLIs];

    while (remaining >= (sizeof(DWORD))) {
      SLI *sli = &(report->mFirstSLI[report->mSLICount]);

      sli->mFirst = RTCP_GET_BITS(UseRTPUtils::getBE16(&(pos[0])), 0x1FFF, 3);
      sli->mNumber = static_cast<WORD>(RTCP_GET_BITS(UseRTPUtils::getBE32(&(pos[0])), 0x1FFF, 6));
      sli->mPictureID = RTCP_GET_BITS(pos[3], 0x3F, 0);

      internal::advancePos(pos, remaining, sizeof(DWORD));

      ++(report->mSLICount);
    }

    ASSERT(possibleSLIs == report->mSLICount)

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parseRPSI(PayloadSpecificFeedbackMessage *report)
  {
    const BYTE *pos = report->fci();
    size_t remaining = report->fciSize();

    {
      if (remaining < sizeof(WORD)) goto illegal_remaining;

      BYTE pb = pos[0];
      report->mRPSI.mZeroBit = RTCP_GET_BITS(pos[1], 0x1, 7);
      report->mRPSI.mPayloadType = RTCP_GET_BITS(pos[1], 0x7F, 0);

      internal::advancePos(pos, remaining, sizeof(WORD));

      if (remaining > 0) {
        report->mRPSI.mNativeRPSIBitString = pos;
        report->mRPSI.mNativeRPSIBitStringSizeInBits = (remaining*8);
        if (pb > report->mRPSI.mNativeRPSIBitStringSizeInBits) goto illegal_remaining;

        report->mRPSI.mNativeRPSIBitStringSizeInBits -= static_cast<size_t>(pb);
        if (0 == report->mRPSI.mNativeRPSIBitStringSizeInBits) {
          report->mRPSI.mNativeRPSIBitString = NULL;
        }
      }

      return true;
    }

  illegal_remaining:
    {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed RPSI payload specific feedback message",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, (reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->fci()))
      );
    }
    return false;
  }
    
  //-------------------------------------------------------------------------
  void RTCPPacket::fillCodecControlCommon(
                                          PayloadSpecificFeedbackMessage *report,
                                          PayloadSpecificFeedbackMessage::CodecControlCommon *common,
                                          const BYTE *pos
                                          )
  {
    common->mSSRC = UseRTPUtils::getBE32(&(pos[0]));
    common->mSeqNr = pos[4];
    common->mReserved = RTCP_GET_BITS(UseRTPUtils::getBE32(&(pos[4])), 0xFFFFFF, 0);
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parseFIR(PayloadSpecificFeedbackMessage *report)
  {
    typedef PayloadSpecificFeedbackMessage::FIR FIR;

    const BYTE *pos = report->fci();
    size_t remaining = report->fciSize();

    size_t possibleFIRs = remaining / (sizeof(DWORD)*2);

    ASSERT(0 != possibleFIRs)

    report->mFirstFIR = new (allocateBuffer(internal::alignedSize(sizeof(FIR))*possibleFIRs)) FIR[possibleFIRs];

    while (remaining >= (sizeof(DWORD)*2)) {
      FIR *fir = &(report->mFirstFIR[report->mFIRCount]);
      fillCodecControlCommon(report, fir, pos);

      internal::advancePos(pos, remaining, sizeof(DWORD)*2);

      ++(report->mFIRCount);
    }

    ASSERT(possibleFIRs == report->mFIRCount)

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parseTSTR(PayloadSpecificFeedbackMessage *report)
  {
    typedef PayloadSpecificFeedbackMessage::TSTR TSTR;

    const BYTE *pos = report->fci();
    size_t remaining = report->fciSize();

    size_t possibleTSTRs = remaining / (sizeof(DWORD)*2);

    ASSERT(0 != possibleTSTRs)

    report->mFirstTSTR = new (allocateBuffer(internal::alignedSize(sizeof(TSTR))*possibleTSTRs)) TSTR[possibleTSTRs];

    while (remaining >= (sizeof(DWORD)*2)) {
      TSTR *tstr = &(report->mFirstTSTR[report->mTSTRCount]);
      fillCodecControlCommon(report, tstr, pos);

      tstr->mControlSpecific = RTCP_GET_BITS(tstr->mReserved, 0x1F, 0);
      tstr->mReserved = RTCP_GET_BITS(tstr->mReserved, 0x7FFFF, 5);

      internal::advancePos(pos, remaining, sizeof(DWORD)*2);

      ++(report->mTSTRCount);
    }

    ASSERT(possibleTSTRs == report->mTSTRCount)
      
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parseTSTN(PayloadSpecificFeedbackMessage *report)
  {
    typedef PayloadSpecificFeedbackMessage::TSTN TSTN;

    const BYTE *pos = report->fci();
    size_t remaining = report->fciSize();

    size_t possibleTSTNs = remaining / (sizeof(DWORD)*2);

    ASSERT(0 != possibleTSTNs)

    report->mFirstTSTN = new (allocateBuffer(internal::alignedSize(sizeof(TSTN))*possibleTSTNs)) TSTN[possibleTSTNs];

    while (remaining >= (sizeof(DWORD)*2)) {
      TSTN *tstn = &(report->mFirstTSTN[report->mTSTNCount]);
      fillCodecControlCommon(report, tstn, pos);

      tstn->mControlSpecific = RTCP_GET_BITS(tstn->mReserved, 0x1F, 0);
      tstn->mReserved = RTCP_GET_BITS(tstn->mReserved, 0x7FFFF, 5);

      internal::advancePos(pos, remaining, sizeof(DWORD)*2);

      ++(report->mTSTNCount);
    }

    ASSERT(possibleTSTNs == report->mTSTNCount)
      
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parseVBCM(PayloadSpecificFeedbackMessage *report)
  {
    typedef PayloadSpecificFeedbackMessage::VBCM VBCM;

    const BYTE *pos = report->fci();
    size_t remaining = report->fciSize();

    size_t possibleVBCMs = 0;

    // first count the VBCM blocks
    {
      while (remaining >= (sizeof(DWORD)*2)) {
        ++possibleVBCMs;

        size_t length = UseRTPUtils::getBE16(&(pos[6]));
        size_t modulas = length % sizeof(DWORD);
        size_t padding = ((0 != modulas) ? (sizeof(DWORD)-modulas) : 0);

        internal::advancePos(pos, remaining, sizeof(DWORD)*2);

        ASSERT(remaining >= length)

        size_t skipLength = (((length + padding) > remaining) ? remaining : (length + padding));

        internal::advancePos(pos, remaining, skipLength);
      }
      ASSERT(0 != possibleVBCMs);
    }

    pos = report->fci();
    remaining = report->fciSize();

    {
      ASSERT(0 != possibleVBCMs);

      report->mFirstVBCM = new (allocateBuffer(internal::alignedSize(sizeof(VBCM))*possibleVBCMs)) VBCM[possibleVBCMs];

      while (remaining >= (sizeof(DWORD)*2)) {
        VBCM *vcbm = &(report->mFirstVBCM[report->mVBCMCount]);
        fillCodecControlCommon(report, vcbm, pos);

        // move reserved to control specific
        vcbm->mControlSpecific = vcbm->mReserved;
        vcbm->mReserved = 0;

        size_t length = UseRTPUtils::getBE16(&(pos[6]));
        size_t modulas = length % sizeof(DWORD);
        size_t padding = ((0 != modulas) ? (sizeof(DWORD)-modulas) : 0);

        internal::advancePos(pos, remaining, sizeof(DWORD)*2);

        ASSERT(remaining >= length)

        if (length > 0) {
          vcbm->mVBCMOctetString = pos;
        }

        size_t skipLength = (((length + padding) > remaining) ? remaining : (length + padding));

        internal::advancePos(pos, remaining, skipLength);

        ++(report->mVBCMCount);
      }

      ASSERT(possibleVBCMs == report->mVBCMCount);
    }
    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parseAFB(PayloadSpecificFeedbackMessage *report)
  {
    //typedef PayloadSpecificFeedbackMessage::AFB AFB;

    const BYTE *pos = report->fci();
    size_t remaining = report->fciSize();

    if (remaining > 0) {
      report->mAFB.mData = pos;
      report->mAFB.mDataSize = remaining;
    }
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parseREMB(PayloadSpecificFeedbackMessage *report)
  {
    //typedef PayloadSpecificFeedbackMessage::REMB REMB;

    if (!parseAFB(report)) return false;

    const BYTE *pos = report->fci();
    size_t remaining = report->fciSize();

    report->mHasREMB = true;

    {
      ASSERT(remaining >= sizeof(DWORD) * 3);

      report->mREMB.mNumSSRC = pos[4];
      report->mREMB.mBRExp = RTCP_GET_BITS(pos[5], 0x3F, 2);
      report->mREMB.mBRMantissa = RTCP_GET_BITS(UseRTPUtils::getBE32(&(pos[4])), 0x3FFFF, 0);

      size_t possibleSSRCs = (remaining - sizeof(DWORD)*2) / (sizeof(DWORD));

      internal::advancePos(pos, remaining, sizeof(DWORD)*2);

      report->mREMB.mSSRCs = new (allocateBuffer(internal::alignedSize(sizeof(DWORD))*possibleSSRCs)) DWORD[possibleSSRCs];

      size_t count = 0;
      while ((possibleSSRCs > 0) &&
              (count < report->mREMB.numSSRC()) &&
              (remaining >= sizeof(DWORD)))
      {
        report->mREMB.mSSRCs[count] = UseRTPUtils::getBE32(pos);
        internal::advancePos(pos, remaining, sizeof(DWORD));
        --possibleSSRCs;
        ++count;
      }

      if (count != report->mREMB.numSSRC()) goto illegal_remaining;
      return true;
    }

  illegal_remaining:
    {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed REMB payload specific feedback message",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, (reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->fci()))
      );
    }
    return false;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parseUnknown(PayloadSpecificFeedbackMessage *report)
  {
    report->mUnknown = report;
    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parseCommonRange(
                                    XR *xr,
                                    XR::ReportBlockRange *reportBlock
                                    )
  {
    const BYTE *pos = reportBlock->typeSpecificContents();
    size_t remaining = reportBlock->typeSpecificContentSize();

    {
      if (remaining < (sizeof(DWORD)*2)) goto illegal_remaining;

      reportBlock->mSSRCOfSource = UseRTPUtils::getBE32(&(pos[0]));
      reportBlock->mBeginSeq = UseRTPUtils::getBE16(&(pos[4]));
      reportBlock->mEndSeq = UseRTPUtils::getBE16(&(pos[6]));

      return true;
    }

  illegal_remaining:
    {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed report block range",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, (reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(reportBlock->typeSpecificContents()))
      );
    }
    return false;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parseCommonRLE(
                                  XR *xr,
                                  XR::RLEReportBlock *reportBlock
                                  )
  {
    const BYTE *pos = reportBlock->typeSpecificContents();
    size_t remaining = reportBlock->typeSpecificContentSize();

    if (!parseCommonRange(xr, reportBlock)) return false;

    internal::advancePos(pos, remaining, sizeof(DWORD)*2);

    size_t possibleRLEsCount = (remaining / sizeof(WORD));

    if (0 == possibleRLEsCount) return true;

    reportBlock->mChunks = new (allocateBuffer(internal::alignedSize(sizeof(XR::RLEChunk)*possibleRLEsCount))) XR::RLEChunk[possibleRLEsCount];

    while (remaining >= sizeof(WORD))
    {
      XR::RLEChunk value = UseRTPUtils::getBE16(pos);
      internal::advancePos(pos, remaining, sizeof(WORD));

      if (0 == value) break;
      reportBlock->mChunks[reportBlock->mChunkCount] = value;
      ++(reportBlock->mChunkCount);
    }

    if (0 == reportBlock->mChunkCount) {
      reportBlock->mChunks = NULL;
    }

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(
                          XR *xr,
                          XR::LossRLEReportBlock *reportBlock
                          )
  {
    return parseCommonRLE(xr, reportBlock);
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(
                          XR *xr,
                          XR::DuplicateRLEReportBlock *reportBlock
                          )
  {
    return parseCommonRLE(xr, reportBlock);
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(
                          XR *xr,
                          XR::PacketReceiptTimesReportBlock *reportBlock
                          )
  {
    if (!parseCommonRange(xr, reportBlock)) return false;

    const BYTE *pos = reportBlock->typeSpecificContents();
    size_t remaining = reportBlock->typeSpecificContentSize();

    internal::advancePos(pos, remaining, sizeof(DWORD)*2);

    size_t possibleTimes = remaining / sizeof(DWORD);

    if (0 == possibleTimes) return true;

    reportBlock->mReceiptTimes = new (allocateBuffer(internal::alignedSize(sizeof(DWORD))*possibleTimes)) DWORD[possibleTimes];

    while (remaining >= sizeof(DWORD)) {
      reportBlock->mReceiptTimes[reportBlock->mReceiptTimeCount] = UseRTPUtils::getBE32(pos);
      internal::advancePos(pos, remaining, sizeof(DWORD));
      ++(reportBlock->mReceiptTimeCount);
    }

    return true;
  }

  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(
                          XR *xr,
                          XR::ReceiverReferenceTimeReportBlock *reportBlock
                          )
  {
    const BYTE *pos = reportBlock->typeSpecificContents();
    size_t remaining = reportBlock->typeSpecificContentSize();

    if (remaining < (sizeof(DWORD)*2)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed receiver reference time report block",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    reportBlock->mNTPTimestampMS = UseRTPUtils::getBE32(&(pos[0]));
    reportBlock->mNTPTimestampLS = UseRTPUtils::getBE32(&(pos[4]));

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(
                          XR *xr,
                          XR::DLRRReportBlock *reportBlock
                          )
  {
    const BYTE *pos = reportBlock->typeSpecificContents();
    size_t remaining = reportBlock->typeSpecificContentSize();

    size_t possibleSubBlockCount = remaining / (sizeof(DWORD)*3);

    if (0 == possibleSubBlockCount) return true;

    if (remaining < (sizeof(DWORD)*2)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed dlr report block",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    reportBlock->mSubBlocks = new (allocateBuffer(internal::alignedSize(sizeof(XR::DLRRReportBlock::SubBlock))*possibleSubBlockCount)) XR::DLRRReportBlock::SubBlock[possibleSubBlockCount];

    while (remaining >= (sizeof(DWORD)*3))
    {
      auto subBlock = &(reportBlock->mSubBlocks[reportBlock->mSubBlockCount]);

      subBlock->mSSRC = UseRTPUtils::getBE32(&(pos[0]));
      subBlock->mLRR = UseRTPUtils::getBE32(&(pos[4]));
      subBlock->mDLRR = UseRTPUtils::getBE32(&(pos[8]));

      internal::advancePos(pos, remaining, sizeof(DWORD)*3);

      ++(reportBlock->mSubBlockCount);
    }

    ASSERT(reportBlock->mSubBlockCount == possibleSubBlockCount)

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(
                          XR *xr,
                          XR::StatisticsSummaryReportBlock *reportBlock
                          )
  {
    const BYTE *pos = reportBlock->typeSpecificContents();
    size_t remaining = reportBlock->typeSpecificContentSize();

    parseCommonRange(xr, reportBlock);

    if (remaining < (sizeof(DWORD)*9)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed statistics summary report block",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    internal::advancePos(pos, remaining, sizeof(DWORD)*2);

    reportBlock->mLostPackets = UseRTPUtils::getBE32(&(pos[0]));
    reportBlock->mDupPackets = UseRTPUtils::getBE32(&(pos[4]));
    reportBlock->mMinJitter = UseRTPUtils::getBE32(&(pos[8]));
    reportBlock->mMaxJitter = UseRTPUtils::getBE32(&(pos[12]));
    reportBlock->mMeanJitter = UseRTPUtils::getBE32(&(pos[16]));
    reportBlock->mDevJitter = UseRTPUtils::getBE32(&(pos[20]));

    reportBlock->mMinTTLOrHL = pos[24];
    reportBlock->mMaxTTLOrHL = pos[25];
    reportBlock->mMeanTTLOrHL = pos[26];
    reportBlock->mDevTTLOrHL = pos[27];

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(
                          XR *xr,
                          XR::VoIPMetricsReportBlock *reportBlock
                          )
  {
    const BYTE *pos = reportBlock->typeSpecificContents();
    size_t remaining = reportBlock->typeSpecificContentSize();

    if (remaining < (sizeof(DWORD)*8)) {
      ZS_EVENTING_4(x, w, Trace, RTCPPacketParseWarning, ol, RtcpPacket, Parse,
        string, message, "malformed voip metrics report block",
        puid, mediaChannelID, mMediaChannelID,
        size_t, remaining, remaining,
        size_t, length, 0
      );
      return false;
    }

    reportBlock->mSSRCOfSource = UseRTPUtils::getBE32(&(pos[0]));
    reportBlock->mLossRate = pos[4];
    reportBlock->mDiscardRate = pos[5];
    reportBlock->mBurstDensity = pos[6];
    reportBlock->mGapDensity = pos[7];
    reportBlock->mBurstDuration = UseRTPUtils::getBE16(&(pos[8]));
    reportBlock->mGapDuration = UseRTPUtils::getBE16(&(pos[10]));
    reportBlock->mRoundTripDelay = UseRTPUtils::getBE16(&(pos[12]));
    reportBlock->mEndSystemDelay = UseRTPUtils::getBE16(&(pos[14]));
    reportBlock->mSignalLevel = pos[16];
    reportBlock->mNoiseLevel = pos[17];
    reportBlock->mRERL = pos[18];
    reportBlock->mGmin = pos[19];
    reportBlock->mRFactor = pos[20];
    reportBlock->mExtRFactor = pos[21];
    reportBlock->mMOSLQ = pos[22];
    reportBlock->mMOSCQ = pos[23];
    reportBlock->mRXConfig = pos[24];
    reportBlock->mReservedVoIP = pos[25];
    reportBlock->mJBNominal = UseRTPUtils::getBE16(&(pos[26]));
    reportBlock->mJBMaximum = UseRTPUtils::getBE16(&(pos[28]));
    reportBlock->mJBAbsMax = UseRTPUtils::getBE16(&(pos[30]));

    return true;
  }
    
  //-------------------------------------------------------------------------
  bool RTCPPacket::parse(
                          XR *xr,
                          XR::UnknownReportBlock *reportBlock
                          )
  {
    return true;
  }

  //-------------------------------------------------------------------------
  void *RTCPPacket::allocateBuffer(size_t size)
  {
    return internal::allocateBuffer(mAllocationPos, mAllocationSize, size);
  }

  //-------------------------------------------------------------------------
  size_t RTCPPacket::getPacketSize(const Report *first)
  {
    size_t result = 0;

    const Report *final = NULL;

    for (const Report *report = first; NULL != report; report = report->next())
    {
      final = report;

      size_t beforeSize = result;

      switch (report->pt()) {
        case SenderReport::kPayloadType:
        {
          const SenderReport *sr = static_cast<const SenderReport *>(report);
          result += internal::getPacketSizeSenderReport(sr);
          break;
        }
        case ReceiverReport::kPayloadType:
        {
          const ReceiverReport *rr = static_cast<const ReceiverReport *>(report);
          result += internal::getPacketSizeReceiverReport(rr);
          break;
        }
        case SDES::kPayloadType:
        {
          const SDES *sdes = static_cast<const SDES *>(report);
          result += internal::getPacketSizeSDES(sdes);
          break;
        }
        case Bye::kPayloadType:
        {
          const Bye *bye = static_cast<const Bye *>(report);
          result += internal::getPacketSizeBye(bye);
          break;
        }
        case App::kPayloadType:
        {
          const App *app = static_cast<const App *>(report);
          result += internal::getPacketSizeApp(app);
          break;
        }
        case TransportLayerFeedbackMessage::kPayloadType:
        {
          const TransportLayerFeedbackMessage *fm = static_cast<const TransportLayerFeedbackMessage *>(report);
          result += internal::getPacketSizeTransportLayerFeedbackMessage(fm);
          break;
        }
        case PayloadSpecificFeedbackMessage::kPayloadType:
        {
          const PayloadSpecificFeedbackMessage *fm = static_cast<const PayloadSpecificFeedbackMessage *>(report);
          result += internal::getPacketSizePayloadSpecificFeedbackMessage(fm);
          break;
        }
        case XR::kPayloadType:
        {
          const XR *xr = static_cast<const XR *>(report);
          result += internal::getPacketSizeXR(xr);
          break;
        }
        default:
        {
          result += sizeof(DWORD) + internal::boundarySize(report->size());
          break;
        }
      }

      size_t afterSize = result;

      ZS_EVENTING_3(x, i, Insane, RTCPPacketGettingReportPacketSize, ol, RtcpPacket, Info,
        string, payloadType , report->ptToString(),
        byte, payloadTypeNumber, report->pt(),
        size_t, size, afterSize - beforeSize
      );
    }

    if (NULL != final) {
      auto padding = final->padding();
      if (0 != padding) {
        result += padding;
      }
    }

    return internal::boundarySize(result);
  }

  //-------------------------------------------------------------------------
  void RTCPPacket::writePacket(const Report *first, BYTE * &pos, size_t &remaining)
  {
    ASSERT(sizeof(char) == sizeof(BYTE))
    ASSERT(NULL != first)

    for (const Report *report = first; NULL != report; report = report->next())
    {
      BYTE *startOfReport = pos;

      internal::writePacketHeader(report, pos, remaining);

      switch (report->pt()) {
        case SenderReport::kPayloadType:
        {
          const SenderReport *sr = static_cast<const SenderReport *>(report);
          internal::writePacketSenderReport(sr, pos, remaining);
          break;
        }
        case ReceiverReport::kPayloadType:
        {
          const ReceiverReport *rr = static_cast<const ReceiverReport *>(report);
          internal::writePacketReceiverReport(rr, pos, remaining);
          break;
        }
        case SDES::kPayloadType:
        {
          const SDES *sdes = static_cast<const SDES *>(report);
          internal::writePacketSDES(sdes, pos, remaining);
          break;
        }
        case Bye::kPayloadType:
        {
          const Bye *bye = static_cast<const Bye *>(report);
          internal::writePacketBye(bye, pos, remaining);
          break;
        }
        case App::kPayloadType:
        {
          const App *app = static_cast<const App *>(report);
          internal::writePacketApp(app, pos, remaining);
          break;
        }
        case TransportLayerFeedbackMessage::kPayloadType:
        {
          const TransportLayerFeedbackMessage *fm = static_cast<const TransportLayerFeedbackMessage *>(report);
          internal::writePacketTransportLayerFeedbackMessage(fm, pos, remaining);
          break;
        }
        case PayloadSpecificFeedbackMessage::kPayloadType:
        {
          const PayloadSpecificFeedbackMessage *fm = static_cast<const PayloadSpecificFeedbackMessage *>(report);
          internal::writePacketPayloadSpecificFeedbackMessage(fm, pos, remaining);
          break;
        }
        case XR::kPayloadType:
        {
          const XR *xr = static_cast<const XR *>(report);
          internal::writePacketXR(xr, pos, remaining);
          break;
        }
        default:
        {
          const UnknownReport *unknown = static_cast<const UnknownReport *>(report);
          internal::writePacketUnknown(unknown, pos, remaining);
          break;
        }
      }

      BYTE *endOfReport = pos;
      size_t diff = static_cast<size_t>(reinterpret_cast<PTRNUMBER>(endOfReport) - reinterpret_cast<PTRNUMBER>(startOfReport));

      size_t modulas = diff % sizeof(DWORD);
      if (0 != modulas) {
        internal::advancePos(pos, remaining, sizeof(DWORD)-modulas);
        diff += (sizeof(DWORD)-modulas);
      }

      ZS_EVENTING_3(x, i, Insane, RTCPPacketWriteReport, ol, RtcpPacket, Info,
        string, payloadType, report->ptToString(),
        byte, payloadTypeNumber, report->pt(),
        size_t, size, diff
      );

      size_t padding = 0;

      if (NULL == report->next()) {
        padding = internal::boundarySize(static_cast<size_t>(report->padding()));
        if (0 != padding) {
          if (padding > 1) {
            internal::advancePos(pos, remaining, (padding-1)*sizeof(BYTE));
          }
          pos[0] = static_cast<BYTE>(padding);
          internal::advancePos(pos, remaining);
        }
      }

      size_t headerSize = ((diff+padding)/sizeof(DWORD))-1;
      ASSERT(internal::throwIfGreaterThanBitsAllow(headerSize, 16));

      UseRTPUtils::setBE16(&(startOfReport[2]), static_cast<WORD>(headerSize));
    }

    ASSERT(0 == remaining);
  }

}
