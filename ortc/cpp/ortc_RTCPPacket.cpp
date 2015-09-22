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

#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/IHelper.h>

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

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
//  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)
//
//  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
    ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    static const size_t kMinRtcpPacketLen = 12;
    static const BYTE kRtpVersion = 2;

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
      ASSERT(ioRemaining > size)
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
      ASSERT(value <= ((1 << maxBits)-1))
      ORTC_THROW_INVALID_PARAMETERS_IF(value > ((1 << maxBits)-1))
      return true;
    }

    //-------------------------------------------------------------------------
    static bool throwIfGreaterThan(
                                   size_t size,
                                   size_t max
                                   )
    {
      ASSERT(size <= max)
      ORTC_THROW_INVALID_PARAMETERS_IF(size > max)
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
    #pragma mark RTCPPacket::SenderReport
    #pragma mark

    //-------------------------------------------------------------------------
    Time RTCPPacket::SenderReport::ntpTimestamp() const
    {
      return UseHelper::ntpToTime(mNTPTimestampMS, mNTPTimestampLS);
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
      return UseHelper::ntpToTime(mNTPTimestampMS, mNTPTimestampLS);
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
    RTCPPacket::RTCPPacket(const make_private &)
    {
    }

    //-------------------------------------------------------------------------
    RTCPPacket::~RTCPPacket()
    {
    }

    //-------------------------------------------------------------------------
    RTCPPacketPtr RTCPPacket::create(const BYTE *buffer, size_t bufferLengthInBytes)
    {
      ORTC_THROW_INVALID_PARAMETERS_IF(!buffer)
      ORTC_THROW_INVALID_PARAMETERS_IF(0 == bufferLengthInBytes)
      return RTCPPacket::create(UseServicesHelper::convertToBuffer(buffer, bufferLengthInBytes));
    }

    //-------------------------------------------------------------------------
    RTCPPacketPtr RTCPPacket::create(const SecureByteBlock &buffer)
    {
      return RTCPPacket::create(buffer.BytePtr(), buffer.SizeInBytes());
    }

    //-------------------------------------------------------------------------
    RTCPPacketPtr RTCPPacket::create(SecureByteBlockPtr buffer)
    {
      RTCPPacketPtr pThis(make_shared<RTCPPacket>(make_private{}));
      pThis->mBuffer = buffer;
      if (!pThis->parse()) {
        ZS_LOG_WARNING(Debug, pThis->log("packet could not be parsed"))
        return RTCPPacketPtr();
      }
      return pThis;
    }

    //-------------------------------------------------------------------------
    RTCPPacketPtr RTCPPacket::create(const Report *first)
    {
      size_t allocationSize = getPacketSize(first);
      SecureByteBlockPtr temp(make_shared<SecureByteBlock>(allocationSize));

      BYTE *buffer = temp->BytePtr();
      writePacket(first, buffer, allocationSize);

      return create(temp);
    }

    //-------------------------------------------------------------------------
    SecureByteBlockPtr RTCPPacket::generateFrom(const Report *first)
    {
      size_t allocationSize = getPacketSize(first);
      SecureByteBlockPtr temp(make_shared<SecureByteBlock>(allocationSize));

      BYTE *buffer = temp->BytePtr();
      writePacket(first, buffer, allocationSize);

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
    static void toDebug(
                        ElementPtr &subEl,
                        RTCPPacket::Report *common
                        )
    {
      UseServicesHelper::debugAppend(subEl, "next", NULL != common->next());
      UseServicesHelper::debugAppend(subEl, "ptr", NULL != common->ptr());
      UseServicesHelper::debugAppend(subEl, "size", common->size());
      UseServicesHelper::debugAppend(subEl, "version", common->version());
      UseServicesHelper::debugAppend(subEl, "padding", common->padding());
      UseServicesHelper::debugAppend(subEl, "report specific", common->reportSpecific());
      UseServicesHelper::debugAppend(subEl, "pt", common->pt());
    }

    //-------------------------------------------------------------------------
    static void toDebugSenderReceiverCommonReport(
                                                  ElementPtr &subEl,
                                                  RTCPPacket::SenderReceiverCommonReport *common
                                                  )
    {
      internal::toDebug(subEl, common);
      UseServicesHelper::debugAppend(subEl, "rc", common->rc());
      UseServicesHelper::debugAppend(subEl, (RTCPPacket::ReceiverReport::kPayloadType != common->pt() ? "ssrc of sender" : "ssrc of packet sender"), common->ssrcOfSender());
      UseServicesHelper::debugAppend(subEl, "extension", (NULL != common->extension()));
      UseServicesHelper::debugAppend(subEl, "extension size", common->extensionSize());
    }

    //-------------------------------------------------------------------------
    static ElementPtr toDebug(RTCPPacket::SenderReceiverCommonReport *common)
    {
      RTCPPacket::SenderReceiverCommonReport::ReportBlock *block = common->firstReportBlock();
      if (NULL == block) return ElementPtr();

      ElementPtr outerEl = Element::create("ReportBlocks");

      for (; NULL != block; block = block->next()) {
        ElementPtr subEl = Element::create("ReportBlocks");
        UseServicesHelper::debugAppend(subEl, "ssrc", block->ssrc());
        UseServicesHelper::debugAppend(subEl, "fraction lost", block->fractionLost());
        UseServicesHelper::debugAppend(subEl, "cumulative number of packets lost", block->cumulativeNumberOfPacketsLost());
        UseServicesHelper::debugAppend(subEl, "extended highest sequence number received", block->extendedHighestSequenceNumberReceived());
        UseServicesHelper::debugAppend(subEl, "interarrival jitter", block->interarrivalJitter());
        UseServicesHelper::debugAppend(subEl, "lsr", block->lsr());
        UseServicesHelper::debugAppend(subEl, "dlsr", block->dlsr());
        UseServicesHelper::debugAppend(outerEl, subEl);
      }
      return outerEl;
    }

    //-------------------------------------------------------------------------
    static void toDebug(
                        ElementPtr &subEl,
                        RTCPPacket::SDES::Chunk::StringItem *common
                        )
    {
      auto type = common->type();
      UseServicesHelper::debugAppend(subEl, "type", type);
      if (RTCPPacket::SDES::Chunk::Priv::kItemType == type) {
        RTCPPacket::SDES::Chunk::Priv *priv = reinterpret_cast<RTCPPacket::SDES::Chunk::Priv *>(common);
        UseServicesHelper::debugAppend(subEl, "prefix length", priv->prefixLength());
        UseServicesHelper::debugAppend(subEl, "prefix", priv->prefix());
      }
      UseServicesHelper::debugAppend(subEl, "length", common->length());
      UseServicesHelper::debugAppend(subEl, "value", common->value());
    }

    //-------------------------------------------------------------------------
    static ElementPtr toDebug(
                              const char *typeNames,
                              const char *typeName,
                              RTCPPacket::SDES::Chunk::StringItem *first
                              )
    {
      if (NULL == first) return ElementPtr();

      RTCPPacket::SDES::Chunk::StringItem *item = first;
      bool hasMoreThanOne = (NULL != item ? (NULL != item->mNext) : false);

      ElementPtr outerEl;

      if (hasMoreThanOne) {
        outerEl = Element::create(typeNames);
      }

      for (; NULL != item; item = item->mNext) {
        ElementPtr subEl = Element::create(typeName);
        internal::toDebug(subEl, item);

        if (!hasMoreThanOne) {
          return subEl;
        }
        UseServicesHelper::debugAppend(outerEl, subEl);
      }

      return outerEl;
    }

    //-------------------------------------------------------------------------
    static ElementPtr toDebug(
                              const char *typeNamse,
                              const char *typeName,
                              DWORD *ssrcs,
                              size_t count
                              )
    {
      if (0 == count) return ElementPtr();

      ElementPtr outerEl = Element::create(typeNamse);

      for (size_t index = 0; index < count; ++index) {
        UseServicesHelper::debugAppend(outerEl, typeName, ssrcs[index]);
      }

      return outerEl;
    }
    
    //-------------------------------------------------------------------------
    static void toDebugFeedbackMessage(
                                       ElementPtr &subEl,
                                       RTCPPacket::FeedbackMessage *common
                                       )
    {
      internal::toDebug(subEl, common);

      UseServicesHelper::debugAppend(subEl, "fmt", common->fmt());
      UseServicesHelper::debugAppend(subEl, "ssrc of packet sender", common->ssrcOfPacketSender());
      UseServicesHelper::debugAppend(subEl, "ssrc of media source", common->ssrcOfMediaSource());
      UseServicesHelper::debugAppend(subEl, "fci", (NULL != common->fci()));
      UseServicesHelper::debugAppend(subEl, "fci size", common->fciSize());
    }

    //-------------------------------------------------------------------------
    static void toDebug(
                        ElementPtr &subEl,
                        RTCPPacket::TransportLayerFeedbackMessage::TMMBRCommon *common
                        )
    {
      UseServicesHelper::debugAppend(subEl, "ssrc", common->ssrc());
      UseServicesHelper::debugAppend(subEl, "mx tbr exp", common->mxTBRExp());
      UseServicesHelper::debugAppend(subEl, "mx tbr mantissa", common->mxTBRMantissa());
      UseServicesHelper::debugAppend(subEl, "measured overhead", common->measuredOverhead());
    }
    
    //-------------------------------------------------------------------------
    static void toDebug(
                        ElementPtr &subEl,
                        RTCPPacket::PayloadSpecificFeedbackMessage::CodecControlCommon *common
                        )
    {
      UseServicesHelper::debugAppend(subEl, "ssrc", common->ssrc());
      UseServicesHelper::debugAppend(subEl, "seq nr", common->seqNr());
      UseServicesHelper::debugAppend(subEl, "reserved", common->reserved());
    }

    //-------------------------------------------------------------------------
    static void toDebugReportBlock(
                                   ElementPtr &subEl,
                                   RTCPPacket::XR::ReportBlock *common
                                   )
    {
      UseServicesHelper::debugAppend(subEl, "next", (NULL != common->next()));
      UseServicesHelper::debugAppend(subEl, "block type", common->blockType());
      UseServicesHelper::debugAppend(subEl, "type specific", common->typeSpecific());
      UseServicesHelper::debugAppend(subEl, "type specific contents", (NULL != common->typeSpecificContents()));
      UseServicesHelper::debugAppend(subEl, "type specific contents size", common->typeSpecificContentSize());
    }

    //-------------------------------------------------------------------------
    static void toDebugReportBlockRange(
                                        ElementPtr &subEl,
                                        RTCPPacket::XR::ReportBlockRange *common
                                        )
    {
      internal::toDebugReportBlock(subEl, common);
      UseServicesHelper::debugAppend(subEl, "reserved", common->reserved());
      UseServicesHelper::debugAppend(subEl, "thinning", common->thinning());
      UseServicesHelper::debugAppend(subEl, "ssrc", common->ssrcOfSource());
      UseServicesHelper::debugAppend(subEl, "begin seq", common->beginSeq());
      UseServicesHelper::debugAppend(subEl, "end seq", common->endSeq());
    }

    //-------------------------------------------------------------------------
    static void toDebugRLEReportBlock(
                                      ElementPtr &subEl,
                                      RTCPPacket::XR::RLEReportBlock *common
                                      )
    {
      internal::toDebugReportBlockRange(subEl, common);
      auto count = common->chunkCount();

      if (count < 1) return;

      ElementPtr outerEl = Element::create("chunks");

      for (size_t index = 0; index < count; ++index) {
        RTCPPacket::XR::RLEChunk chunk = common->chunkAtIndex(index);

        ElementPtr chunkEl;

        if (RTCPPacket::XR::isRunLengthChunk(chunk)) {
          chunkEl = Element::create("run length");

          RTCPPacket::XR::RunLength rl(chunk);
          UseServicesHelper::debugAppend(chunkEl, "run type", string(rl.runType()));
          UseServicesHelper::debugAppend(chunkEl, "run length", rl.runLength());

        } else if (RTCPPacket::XR::isBitVectorChunk(chunk)) {
          chunkEl = Element::create("bit vector");

          String bits;
          RTCPPacket::XR::BitVector bv(chunk);
          for (int bitPos = 0; bitPos < sizeof(RTCPPacket::XR::RLEChunk); ++bitPos) {
            bits += string(bv.bitAtIndex(bitPos));
          }
          UseServicesHelper::debugAppend(chunkEl, "bit vector", bits);
        } else {
          chunkEl = Element::create("unknown");
        }

        UseServicesHelper::debugAppend(outerEl, chunkEl);
      }
      UseServicesHelper::debugAppend(subEl, outerEl);

    }

    //-------------------------------------------------------------------------
    static ElementPtr toDebugSenderReport(RTCPPacket::SenderReport *sr)
    {
      ElementPtr subEl = Element::create("SenderReport");

      internal::toDebugSenderReceiverCommonReport(subEl, sr);
      UseServicesHelper::debugAppend(subEl, "ntp timestamp ms", sr->ntpTimestampMS());
      UseServicesHelper::debugAppend(subEl, "ntp timestamp ls", sr->ntpTimestampLS());
      UseServicesHelper::debugAppend(subEl, "ntp timestamp", sr->ntpTimestamp());
      UseServicesHelper::debugAppend(subEl, "sender packet count", sr->senderPacketCount());
      UseServicesHelper::debugAppend(subEl, "sender octet count", sr->senderOctetCount());

      UseServicesHelper::debugAppend(subEl, internal::toDebug(sr));

      UseServicesHelper::debugAppend(subEl, "next sender report", NULL != sr->nextSenderReport());
      return subEl;
    }

    //-------------------------------------------------------------------------
    static ElementPtr toDebugReceiverReport(RTCPPacket::ReceiverReport *rr)
    {
      ElementPtr subEl = Element::create("ReceiverReport");
      internal::toDebugSenderReceiverCommonReport(subEl, rr);
      UseServicesHelper::debugAppend(subEl, internal::toDebug(rr));
      UseServicesHelper::debugAppend(subEl, "next receiver report", NULL != rr->nextReceiverReport());
      return subEl;
    }

    //-------------------------------------------------------------------------
    static ElementPtr toDebugSDES(RTCPPacket::SDES *sdes)
    {
      ElementPtr subEl = Element::create("SDES");
      internal::toDebug(subEl, sdes);
      UseServicesHelper::debugAppend(subEl, "sc", sdes->sc());

      RTCPPacket::SDES::Chunk *chunk = sdes->firstChunk();
      if (NULL != chunk) {
        ElementPtr chunksEl = Element::create("Chunks");

        for (; NULL != chunk; chunk = chunk->next()) {
          ElementPtr chunkEl = Element::create("Chunk");
          UseServicesHelper::debugAppend(chunkEl, "ssrc", chunk->ssrc());
          UseServicesHelper::debugAppend(chunkEl, "count", chunk->count());
          UseServicesHelper::debugAppend(chunkEl, "cname count", chunk->cNameCount());
          UseServicesHelper::debugAppend(chunkEl, "name count", chunk->nameCount());
          UseServicesHelper::debugAppend(chunkEl, "email count", chunk->emailCount());
          UseServicesHelper::debugAppend(chunkEl, "phone count", chunk->phoneCount());
          UseServicesHelper::debugAppend(chunkEl, "loc count", chunk->locCount());
          UseServicesHelper::debugAppend(chunkEl, "tool count", chunk->toolCount());
          UseServicesHelper::debugAppend(chunkEl, "note count", chunk->noteCount());
          UseServicesHelper::debugAppend(chunkEl, "priv count", chunk->privCount());
          UseServicesHelper::debugAppend(chunkEl, "unknown count", chunk->unknownCount());

          UseServicesHelper::debugAppend(chunkEl, internal::toDebug("cnames", "cname", chunk->firstCName()));
          UseServicesHelper::debugAppend(chunkEl, internal::toDebug("names", "name", chunk->firstName()));
          UseServicesHelper::debugAppend(chunkEl, internal::toDebug("emails", "email", chunk->firstEmail()));
          UseServicesHelper::debugAppend(chunkEl, internal::toDebug("phones", "phone", chunk->firstPhone()));
          UseServicesHelper::debugAppend(chunkEl, internal::toDebug("locs", "loc", chunk->firstLoc()));
          UseServicesHelper::debugAppend(chunkEl, internal::toDebug("tools", "tool", chunk->firstTool()));
          UseServicesHelper::debugAppend(chunkEl, internal::toDebug("notes", "note", chunk->firstNote()));
          UseServicesHelper::debugAppend(chunkEl, internal::toDebug("privs", "priv", chunk->firstPriv()));
          UseServicesHelper::debugAppend(chunkEl, internal::toDebug("mid", "mid", chunk->firstMid()));
          UseServicesHelper::debugAppend(chunkEl, internal::toDebug("unknowns", "unknown", chunk->firstUnknown()));

          UseServicesHelper::debugAppend(chunksEl, chunkEl);
        }

        UseServicesHelper::debugAppend(subEl, chunksEl);
      }

      UseServicesHelper::debugAppend(subEl, "next sdes", NULL != sdes->nextSDES());
      return subEl;
    }

    //-------------------------------------------------------------------------
    static ElementPtr toDebugBye(RTCPPacket::Bye *bye)
    {
      ElementPtr subEl = Element::create("Bye");
      internal::toDebug(subEl, bye);

      UseServicesHelper::debugAppend(subEl, internal::toDebug("ssrcs", "ssrc", bye->mSSRCs, bye->sc()));

      UseServicesHelper::debugAppend(subEl, "reason for leaving", bye->reasonForLeaving());
      UseServicesHelper::debugAppend(subEl, "next bye", NULL != bye->nextBye());
      return subEl;
    }

    //-------------------------------------------------------------------------
    static ElementPtr toDebugApp(RTCPPacket::App *app)
    {
      ElementPtr subEl = Element::create("App");
      internal::toDebug(subEl, app);

      UseServicesHelper::debugAppend(subEl, "subtype", app->subtype());
      UseServicesHelper::debugAppend(subEl, "ssrc", app->ssrc());
      UseServicesHelper::debugAppend(subEl, "name", app->name());

      UseServicesHelper::debugAppend(subEl, "data", (NULL != app->data()));
      UseServicesHelper::debugAppend(subEl, "name size", app->dataSize());

      UseServicesHelper::debugAppend(subEl, "next app", NULL != app->nextApp());
      return subEl;
    }

    //-------------------------------------------------------------------------
    static ElementPtr toDebugTransportLayerFeedbackMessage(RTCPPacket::TransportLayerFeedbackMessage *fm)
    {
      ElementPtr subEl = Element::create("TransportLayerFeedbackMessage");
      internal::toDebugFeedbackMessage(subEl, fm);

      {
        auto count = fm->genericNACKCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("GenericNACKs");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("GenericNACK");

            auto format = fm->genericNACKAtIndex(index);

            UseServicesHelper::debugAppend(formatEl, "pid", format->pid());
            UseServicesHelper::debugAppend(formatEl, "blp", format->blp());

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto count = fm->tmmbrCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("TMMBRs");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("TMMBR");

            auto format = fm->tmmbrAtIndex(index);
            internal::toDebug(formatEl, format);

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto count = fm->tmmbrCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("TMMBNs");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("TMMBN");

            auto format = fm->tmmbrAtIndex(index);
            internal::toDebug(formatEl, format);

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      UseServicesHelper::debugAppend(subEl, "unknown", NULL != fm->unknown());

      UseServicesHelper::debugAppend(subEl, "next transport layer feedback message", (NULL != fm->nextTransportLayerFeedbackMessage()));

      return subEl;
    }

    //-------------------------------------------------------------------------
    static ElementPtr toDebugPayloadSpecificFeedbackMessage(RTCPPacket::PayloadSpecificFeedbackMessage *fm)
    {
      ElementPtr subEl = Element::create("PayloadSpecificFeedbackMessage");
      internal::toDebugFeedbackMessage(subEl, fm);

      {
        auto format = fm->pli();
        if (NULL != format) {
          ElementPtr formatEl = Element::create("PLI");

          //UseServicesHelper::debugAppend(formatEl, "pid", format->pid());

          UseServicesHelper::debugAppend(subEl, formatEl);
        }
      }

      {
        auto count = fm->sliCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("SLIs");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("SLI");

            auto format = fm->sliAtIndex(index);

            UseServicesHelper::debugAppend(formatEl, "first", format->first());
            UseServicesHelper::debugAppend(formatEl, "number", format->number());
            UseServicesHelper::debugAppend(formatEl, "picture id", format->pictureID());

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto format = fm->rpsi();
        if (NULL != format) {
          ElementPtr formatEl = Element::create("RPSI");

          UseServicesHelper::debugAppend(formatEl, "pb", format->pb());
          UseServicesHelper::debugAppend(formatEl, "zero bit", format->zeroBit());
          UseServicesHelper::debugAppend(formatEl, "payload type", format->payloadType());

          UseServicesHelper::debugAppend(formatEl, "native rpsi bit string", (NULL != format->nativeRPSIBitString()));
          UseServicesHelper::debugAppend(formatEl, "native rpsi bit string size", format->nativeRPSIBitStringSizeInBits());

          UseServicesHelper::debugAppend(subEl, formatEl);
        }
      }

      {
        auto count = fm->firCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("FIRs");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("FIR");

            auto format = fm->firAtIndex(index);
            internal::toDebug(formatEl, format);

            //                  UseServicesHelper::debugAppend(formatEl, "first", format->first());

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto count = fm->tstrCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("TSTRs");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("TSTR");

            auto format = fm->tstrAtIndex(index);
            internal::toDebug(formatEl, format);

            UseServicesHelper::debugAppend(formatEl, "index", format->index());

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto count = fm->tstnCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("TSTNs");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("TSTN");

            auto format = fm->tstnAtIndex(index);
            internal::toDebug(formatEl, format);

            UseServicesHelper::debugAppend(formatEl, "index", format->index());

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto count = fm->vbcmCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("VBCMs");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("VBCM");

            auto format = fm->vbcmAtIndex(index);
            internal::toDebug(formatEl, format);

            UseServicesHelper::debugAppend(formatEl, "zero bit", format->zeroBit());
            UseServicesHelper::debugAppend(formatEl, "payload type", format->payloadType());

            UseServicesHelper::debugAppend(formatEl, "vbcm octet string", (NULL != format->vbcmOctetString()));
            UseServicesHelper::debugAppend(formatEl, "vbcm octet string size", format->vbcmOctetStringSize());

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto format = fm->afb();
        auto remb = fm->remb();

        if ((NULL != format) &&
            (NULL == remb)) {
          ElementPtr formatEl = Element::create("AFB");

          UseServicesHelper::debugAppend(formatEl, "data", (NULL != format->data()));
          UseServicesHelper::debugAppend(formatEl, "data size", format->dataSize());

          UseServicesHelper::debugAppend(subEl, formatEl);
        }
      }

      {
        auto format = fm->remb();
        if (NULL != format) {
          ElementPtr formatEl = Element::create("REMB");

          UseServicesHelper::debugAppend(formatEl, "num ssrc", format->numSSRC());
          UseServicesHelper::debugAppend(formatEl, "br exp", format->brExp());
          UseServicesHelper::debugAppend(formatEl, "br mantissa", format->brMantissa());

          UseServicesHelper::debugAppend(formatEl, internal::toDebug("ssrcs", "ssrc", format->mSSRCs, format->numSSRC()));

          UseServicesHelper::debugAppend(subEl, formatEl);
        }
      }

      UseServicesHelper::debugAppend(subEl, "unknown", NULL != fm->unknown());

      UseServicesHelper::debugAppend(subEl, "next payload specific feedback message", (NULL != fm->nextPayloadSpecificFeedbackMessage()));
      return subEl;
    }

    //-------------------------------------------------------------------------
    static ElementPtr toDebugXR(RTCPPacket::XR *xr)
    {
      ElementPtr subEl = Element::create("XR");
      internal::toDebug(subEl, xr);

      UseServicesHelper::debugAppend(subEl, "reserved", xr->reserved());
      UseServicesHelper::debugAppend(subEl, "ssrc", xr->ssrc());
      UseServicesHelper::debugAppend(subEl, "report block count", xr->reportBlockCount());

      {
        auto count = xr->lossRLEReportBlockCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("LossRLEReportBlocks");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("LossRLEReportBlock");

            auto format = xr->lossRLEReportBlockAtIndex(index);

            internal::toDebugRLEReportBlock(formatEl, format);

            UseServicesHelper::debugAppend(formatEl, "next loss rle report block", (NULL != format->nextLossRLE()));

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto count = xr->duplicateRLEReportBlockCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("DuplicateRLEReportBlocks");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("DuplicateRLEReportBlock");

            auto format = xr->duplicateRLEReportBlockAtIndex(index);

            internal::toDebugRLEReportBlock(formatEl, format);

            UseServicesHelper::debugAppend(formatEl, "next duplicate rle report block", (NULL != format->nextDuplicateRLE()));

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto count = xr->packetReceiptTimesReportBlockCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("PacketReceiptTimesReportBlocks");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("PacketReceiptTimesReportBlock");

            auto format = xr->packetReceiptTimesReportBlockAtIndex(index);

            internal::toDebugReportBlockRange(formatEl, format);

            UseServicesHelper::debugAppend(formatEl, internal::toDebug("receipt times", "receipt time", format->mReceiptTimes, format->receiptTimeCount()));
            UseServicesHelper::debugAppend(formatEl, "next packet receipt times report block", (NULL != format->nextPacketReceiptTimesReportBlock()));

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto count = xr->receiverReferenceTimeReportBlockCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("ReceiverReferenceTimeReportBlocks");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("ReceiverReferenceTimeReportBlock");

            auto format = xr->receiverReferenceTimeReportBlockAtIndex(index);

            internal::toDebugReportBlock(formatEl, format);

            UseServicesHelper::debugAppend(formatEl, "ntp timestamp ms", format->ntpTimestampMS());
            UseServicesHelper::debugAppend(formatEl, "ntp timestamp ls", format->ntpTimestampLS());
            UseServicesHelper::debugAppend(formatEl, "ntp timestamp", format->ntpTimestamp());
            UseServicesHelper::debugAppend(formatEl, "next receiver reference time report block", (NULL != format->nextReceiverReferenceTimeReportBlock()));

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto count = xr->dlrrReportBlockCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("DLRRReportBlocks");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("DLRRReportBlock");

            auto format = xr->dlrrReportBlockAtIndex(index);

            internal::toDebugReportBlock(formatEl, format);

            UseServicesHelper::debugAppend(formatEl, "subblock count", format->subBlockCount());

            size_t subBlockCount = format->subBlockCount();
            if (subBlockCount > 0) {
              ElementPtr subblocksEl = Element::create("SubBlocks");
              for (size_t indexSubBlock = 0; indexSubBlock < subBlockCount; ++indexSubBlock)
              {
                ElementPtr subblockEl = Element::create("SubBlock");
                auto subBlock = format->subBlockAtIndex(indexSubBlock);
                UseServicesHelper::debugAppend(formatEl, "ssrc", subBlock->ssrc());
                UseServicesHelper::debugAppend(formatEl, "lrr", subBlock->lrr());
                UseServicesHelper::debugAppend(formatEl, "dlrr", subBlock->dlrr());
                UseServicesHelper::debugAppend(subblocksEl, subblockEl);
              }
              UseServicesHelper::debugAppend(formatEl, subblocksEl);
            }

            UseServicesHelper::debugAppend(formatEl, "next dlrr report block", (NULL != format->nextDLRRReportBlock()));

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto count = xr->statisticsSummaryReportBlockCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("StatisticsSummaryReportBlocks");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("StatisticsSummaryReportBlock");

            auto format = xr->statisticsSummaryReportBlockAtIndex(index);

            internal::toDebugReportBlockRange(formatEl, format);

            UseServicesHelper::debugAppend(formatEl, "loss report flag", format->lossReportFlag());
            UseServicesHelper::debugAppend(formatEl, "duplicate report flag", format->duplicateReportFlag());
            UseServicesHelper::debugAppend(formatEl, "jitter report flag", format->jitterFlag());
            UseServicesHelper::debugAppend(formatEl, "ttl flag", format->ttlFlag());
            UseServicesHelper::debugAppend(formatEl, "hop limit flag", format->hopLimitFlag());

            if (format->lossReportFlag()) {
              UseServicesHelper::debugAppend(formatEl, "lost packets", format->lostPackets());
            }
            if (format->duplicateReportFlag()) {
              UseServicesHelper::debugAppend(formatEl, "dup packets", format->dupPackets());
            }

            if (format->jitterFlag()) {
              UseServicesHelper::debugAppend(formatEl, "min jitter", format->minJitter());
              UseServicesHelper::debugAppend(formatEl, "max jitter", format->maxJitter());
              UseServicesHelper::debugAppend(formatEl, "mean jitter", format->meanJitter());
              UseServicesHelper::debugAppend(formatEl, "dev jitter", format->devJitter());
            }

            if (format->ttlFlag()) {
              UseServicesHelper::debugAppend(formatEl, "min ttl", format->minTTL());
              UseServicesHelper::debugAppend(formatEl, "max ttl", format->maxTTL());
              UseServicesHelper::debugAppend(formatEl, "mean ttl", format->meanTTL());
              UseServicesHelper::debugAppend(formatEl, "dev ttl", format->devTTL());
            }

            if (format->hopLimitFlag()) {
              UseServicesHelper::debugAppend(formatEl, "min hop limit", format->minHopLimit());
              UseServicesHelper::debugAppend(formatEl, "max hop limit", format->maxHopLimit());
              UseServicesHelper::debugAppend(formatEl, "mean hop limit", format->meanHopLimit());
              UseServicesHelper::debugAppend(formatEl, "dev hop limit", format->devHopLimit());
            }

            UseServicesHelper::debugAppend(formatEl, "next statistics summary report block", (NULL != format->nextStatisticsSummaryReportBlock()));

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto count = xr->voIPMetricsReportBlockCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("VoIPMetricsReportBlocks");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("VoIPMetricsReportBlock");

            auto format = xr->voIPMetricsReportBlockAtIndex(index);

            internal::toDebugReportBlock(formatEl, format);

            UseServicesHelper::debugAppend(formatEl, "ssrc of source", format->ssrcOfSource());

            UseServicesHelper::debugAppend(formatEl, "loss rate", format->lossRate());
            UseServicesHelper::debugAppend(formatEl, "discard rate", format->discardRate());
            UseServicesHelper::debugAppend(formatEl, "burst density", format->burstDensity());
            UseServicesHelper::debugAppend(formatEl, "gap densitity", format->gapDensity());

            UseServicesHelper::debugAppend(formatEl, "burst duration", format->burstDuration());
            UseServicesHelper::debugAppend(formatEl, "gap duration", format->gapDuration());

            UseServicesHelper::debugAppend(formatEl, "round trip delay", format->roundTripDelay());
            UseServicesHelper::debugAppend(formatEl, "end system delay", format->endSystemDelay());

            UseServicesHelper::debugAppend(formatEl, "signal level", format->signalLevel());
            UseServicesHelper::debugAppend(formatEl, "noise level", format->noiseLevel());
            UseServicesHelper::debugAppend(formatEl, "rerl", format->rerl());
            UseServicesHelper::debugAppend(formatEl, "Gmin", format->Gmin());

            UseServicesHelper::debugAppend(formatEl, "r factor", format->rFactor());
            UseServicesHelper::debugAppend(formatEl, "ext r factor", format->extRFactor());
            UseServicesHelper::debugAppend(formatEl, "moslq", format->mosLQ());
            UseServicesHelper::debugAppend(formatEl, "moscq", format->mosCQ());

            UseServicesHelper::debugAppend(formatEl, "rx config", format->rxConfig());
            UseServicesHelper::debugAppend(formatEl, "plc", format->plc());
            UseServicesHelper::debugAppend(formatEl, "jba", format->jba());
            UseServicesHelper::debugAppend(formatEl, "jb rate", format->jbRate());

            UseServicesHelper::debugAppend(formatEl, "jb nominal", format->jbNominal());
            UseServicesHelper::debugAppend(formatEl, "jb maximum", format->jbMaximum());
            UseServicesHelper::debugAppend(formatEl, "jb abs max", format->jbAbsMax());

            UseServicesHelper::debugAppend(formatEl, "next voip metrics report block", (NULL != format->nextVoIPMetricsReportBlock()));

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      {
        auto count = xr->unknownReportBlockCount();
        if (count > 0) {
          ElementPtr formatsEl = Element::create("UnknownReportBlocks");

          for (size_t index = 0; index < count; ++index) {
            ElementPtr formatEl = Element::create("UnknownReportBlock");

            auto format = xr->unknownReportBlockAtIndex(index);

            internal::toDebugReportBlock(formatEl, format);

            UseServicesHelper::debugAppend(formatEl, "next unknown report block", (NULL != format->nextUnknownReportBlock()));

            UseServicesHelper::debugAppend(formatsEl, formatEl);
          }
          UseServicesHelper::debugAppend(subEl, formatsEl);
        }
      }

      UseServicesHelper::debugAppend(subEl, "next xr", (NULL != xr->nextXR()));
      return subEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr RTCPPacket::toDebug() const
    {
      ElementPtr objectEl = Element::create("ortc::RTCPPacket");

      UseServicesHelper::debugAppend(objectEl, "buffer", mBuffer ? mBuffer->SizeInBytes() : 0);
      UseServicesHelper::debugAppend(objectEl, "allocate buffer", mAllocationBuffer ? mAllocationBuffer->SizeInBytes() : 0);

      UseServicesHelper::debugAppend(objectEl, "allocation pos", (NULL != mAllocationPos ? (mAllocationBuffer ? (reinterpret_cast<PTRNUMBER>(mAllocationPos) - reinterpret_cast<PTRNUMBER>(mAllocationBuffer->BytePtr())) : reinterpret_cast<PTRNUMBER>(mAllocationPos)) : 0));
      UseServicesHelper::debugAppend(objectEl, "buffer", mAllocationSize);

      for (Report *report = mFirst; NULL != report; report = report->next())
      {
        switch (report->pt()) {
          case SenderReport::kPayloadType:
          {
            SenderReport *sr = static_cast<SenderReport *>(report);
            UseServicesHelper::debugAppend(objectEl, toDebugSenderReport(sr));
            break;
          }
          case ReceiverReport::kPayloadType:
          {
            ReceiverReport *rr = static_cast<ReceiverReport *>(report);
            UseServicesHelper::debugAppend(objectEl, toDebugReceiverReport(rr));
            break;
          }
          case SDES::kPayloadType:
          {
            SDES *sdes = static_cast<SDES *>(report);
            UseServicesHelper::debugAppend(objectEl, toDebugSDES(sdes));
            break;
          }
          case Bye::kPayloadType:
          {
            Bye *bye = static_cast<Bye *>(report);
            UseServicesHelper::debugAppend(objectEl, toDebugBye(bye));
            break;
          }
          case App::kPayloadType:
          {
            App *app = static_cast<App *>(report);
            UseServicesHelper::debugAppend(objectEl, toDebugApp(app));
            break;
          }
          case TransportLayerFeedbackMessage::kPayloadType:
          {
            TransportLayerFeedbackMessage *fm = static_cast<TransportLayerFeedbackMessage *>(report);
            UseServicesHelper::debugAppend(objectEl, toDebugTransportLayerFeedbackMessage(fm));
            break;
          }
          case PayloadSpecificFeedbackMessage::kPayloadType:
          {
            PayloadSpecificFeedbackMessage *fm = static_cast<PayloadSpecificFeedbackMessage *>(report);
            UseServicesHelper::debugAppend(objectEl, toDebugPayloadSpecificFeedbackMessage(fm));
            break;
          }
          case XR::kPayloadType:
          {
            XR *xr = static_cast<XR *>(report);
            UseServicesHelper::debugAppend(objectEl, toDebugXR(xr));
            break;
          }
          default:
          {
            UnknownReport *unknown = static_cast<UnknownReport *>(report);

            ElementPtr subEl = Element::create("UnknownReport");
            internal::toDebug(subEl, unknown);

            UseServicesHelper::debugAppend(subEl, "next unknown", (NULL != unknown->nextUnknown()));

            UseServicesHelper::debugAppend(objectEl, subEl);
            break;
          }
        }
      }

      return objectEl;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCPPacket (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTCPPacket::slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::RTCPPacket");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTCPPacket::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTCPPacket");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTCPPacket::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    bool RTCPPacket::parse()
    {
      const BYTE *buffer = mBuffer->BytePtr();
      size_t size = mBuffer->SizeInBytes();

      if (size < kMinRtcpPacketLen) {
        ZS_LOG_WARNING(Trace, log("packet length is too short") + ZS_PARAM("length", size))
        return false;
      }

      bool foundPaddingBit = false;

      // scope: calculate total memory allocation size needed to parse entire RTCP packet
      {
        size_t remaining = size;
        const BYTE *pos = buffer;

        while (remaining >= kMinRtcpPacketLen) {
          auto version = RTCP_GET_BITS(*pos, 0x3, 6);
          if (kRtpVersion != version) {
            ZS_LOG_WARNING(Trace, debug("illegal version found") + ZS_PARAM("version", version))
            return false;
          }

          size_t length = sizeof(DWORD) + (static_cast<size_t>(UseHelper::getBE16(&(pos[2]))) * sizeof(DWORD));

          BYTE padding = 0;

          if (RTCP_IS_FLAG_SET(*pos, 5)) {
            if (foundPaddingBit) {
              ZS_LOG_WARNING(Trace, debug("found illegal second padding bit set in compound RTCP block"))
              return false;
            }

            padding = buffer[size-1];
          }

          if (remaining < length + static_cast<size_t>(padding)) {
            ZS_LOG_WARNING(Trace, debug("insufficient length remaining for RTCP block") + ZS_PARAM("length", length) + ZS_PARAM("remaining", remaining))
            return false;
          }

          BYTE reportSpecific = RTCP_GET_BITS(*pos, 0x1F, 0);

          BYTE pt = pos[1];

          advancePos(pos, remaining, sizeof(DWORD));

          if (!getAllocationSize(version, padding, reportSpecific, pt, pos, length - sizeof(DWORD))) return false;

          advancePos(pos, remaining, length - sizeof(DWORD));

          ++mCount;
        }
      }

      if (0 == mAllocationSize) {
        ZS_LOG_TRACE(debug("no RTCP packets were processed"))
        return true;
      }

      mAllocationBuffer = SecureByteBlockPtr(make_shared<SecureByteBlock>(alignedSize(mAllocationSize)));

      mAllocationPos = mAllocationBuffer->BytePtr();

      // scope: allocation size is now established; begin parsing all reports contained in RTCP packet
      {
        size_t remaining = size;
        const BYTE *pos = buffer;


        if (0 != mSenderReportCount) {
          mFirstSenderReport = new (allocateBuffer(alignedSize(sizeof(SenderReport)) * mSenderReportCount)) SenderReport[mSenderReportCount];
        }
        if (0 != mReceiverReportCount) {
          mFirstReceiverReport = new (allocateBuffer(alignedSize(sizeof(ReceiverReport)) * mReceiverReportCount)) ReceiverReport[mReceiverReportCount];
        }
        if (0 != mSDESCount) {
          mFirstSDES = new (allocateBuffer(alignedSize(sizeof(SDES)) * mSDESCount)) SDES[mSDESCount];
        }
        if (0 != mByeCount) {
          mFirstBye = new (allocateBuffer(alignedSize(sizeof(Bye)) * mByeCount)) Bye[mByeCount];
        }
        if (0 != mAppCount) {
          mFirstApp = new (allocateBuffer(alignedSize(sizeof(App)) * mAppCount)) App[mAppCount];
        }
        if (0 != mTransportLayerFeedbackMessageCount) {
          mFirstTransportLayerFeedbackMessage = new (allocateBuffer(alignedSize(sizeof(TransportLayerFeedbackMessage)) * mTransportLayerFeedbackMessageCount)) TransportLayerFeedbackMessage[mTransportLayerFeedbackMessageCount];
        }
        if (0 != mPayloadSpecificFeedbackMessageCount) {
          mFirstPayloadSpecificFeedbackMessage = new (allocateBuffer(alignedSize(sizeof(PayloadSpecificFeedbackMessage)) * mPayloadSpecificFeedbackMessageCount)) PayloadSpecificFeedbackMessage[mPayloadSpecificFeedbackMessageCount];
        }
        if (0 != mXRCount) {
          mFirstXR = new (allocateBuffer(alignedSize(sizeof(XR)) * mXRCount)) XR[mXRCount];
        }
        if (0 != mUnknownReportCount) {
          mFirstUnknownReport = new (allocateBuffer(alignedSize(sizeof(UnknownReport)) * mUnknownReportCount)) UnknownReport[mUnknownReportCount];
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

        while (remaining >= kMinRtcpPacketLen) {
          auto version = RTCP_GET_BITS(*pos, 0x3, 6);
          size_t length = sizeof(DWORD) + (static_cast<size_t>(UseHelper::getBE16(&(pos[2]))) * sizeof(DWORD));

          BYTE padding = 0;

          if (RTCP_IS_FLAG_SET(*pos, 5)) {
            padding = buffer[size-1];
          }

          BYTE reportSpecific = RTCP_GET_BITS(*pos, 0x1F, 0);

          BYTE pt = pos[1];

          advancePos(pos, remaining, sizeof(DWORD));

          if (!parse(lastReport, version, padding, reportSpecific, pt, pos, length - sizeof(DWORD))) return false;

          if (NULL == mFirst) {
            mFirst = lastReport;
          }

          advancePos(pos, remaining, length - sizeof(DWORD));

          ++count;
          ASSERT(count <= mCount)
        }
      }

      return true;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCPPacket (sizing routines)
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

      mAllocationSize += alignedSize(sizeof(SenderReport)) + (alignedSize(sizeof(SenderReport::ReportBlock)) * static_cast<size_t>(reportSpecific));
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

      mAllocationSize += alignedSize(sizeof(ReceiverReport)) + (alignedSize(sizeof(ReceiverReport::ReportBlock)) * static_cast<size_t>(reportSpecific));
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

      mAllocationSize += alignedSize(sizeof(SDES)) + (alignedSize(sizeof(SDES::Chunk)) * chunksCount);

      size_t remaining = contentSize;

      const BYTE *pos = contents;

      while ((remaining > sizeof(DWORD)) &&
             (chunksCount > 0))
      {
        advancePos(pos, remaining, sizeof(DWORD));

        --chunksCount;

        while (remaining >= sizeof(BYTE)) {

          BYTE type = *pos;
          advancePos(pos, remaining);

          if (SDES::Chunk::kEndOfItemsType == type) {
            // skip NUL item (no length octet is present)

            // skip to next DWORD alignment
            auto diff = reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(contents);
            while ((0 != (diff % sizeof(DWORD))) &&
                   (remaining > 0))
            {
              // only NUL chunks are allowed
              if (SDES::Chunk::kEndOfItemsType != (*pos)) {
                ZS_LOG_WARNING(Insane, log("SDES item type is not understood") + ZS_PARAM("type", *pos))
                return false;
              }

              advancePos(pos, remaining);
              ++diff;
            }
            break;
          }

          if (remaining < sizeof(BYTE)) {
            ZS_LOG_WARNING(Trace, log("no length of SDES entry present") + ZS_PARAM("remaining", remaining))
            return false;
          }

          size_t length = static_cast<size_t>(*pos);
          advancePos(pos, remaining);

          if (remaining < length) {
            ZS_LOG_WARNING(Trace, debug("malformed SDES length found") + ZS_PARAM("length", length) + ZS_PARAM("remaining", remaining))
            return false;
          }

          switch (type) {
            case SDES::Chunk::CName::kItemType: mAllocationSize += alignedSize(sizeof(SDES::Chunk::CName)); break;
            case SDES::Chunk::Name::kItemType:  mAllocationSize += alignedSize(sizeof(SDES::Chunk::Name)); break;
            case SDES::Chunk::Email::kItemType: mAllocationSize += alignedSize(sizeof(SDES::Chunk::Email)); break;
            case SDES::Chunk::Phone::kItemType: mAllocationSize += alignedSize(sizeof(SDES::Chunk::Phone)); break;
            case SDES::Chunk::Loc::kItemType:   mAllocationSize += alignedSize(sizeof(SDES::Chunk::Loc)); break;
            case SDES::Chunk::Tool::kItemType:  mAllocationSize += alignedSize(sizeof(SDES::Chunk::Tool)); break;
            case SDES::Chunk::Note::kItemType:  mAllocationSize += alignedSize(sizeof(SDES::Chunk::Note)); break;
            case SDES::Chunk::Priv::kItemType:
            {
              {
                mAllocationSize += alignedSize(sizeof(SDES::Chunk::Priv));

                if (length < sizeof(BYTE)) goto illegal_priv_prefix;
                size_t prefixLength = static_cast<size_t>(*pos);

                if (prefixLength >= length) goto illegal_priv_prefix;

                if (0 != prefixLength) {
                  mAllocationSize += alignedSize(sizeof(char)*(prefixLength+1));
                }
                break;
              }
            illegal_priv_prefix:
              {
                ZS_LOG_WARNING(Trace, debug("malformed SDES Priv prefix found"))
                return false;
              }
              break;
            }
            case SDES::Chunk::Mid::kItemType:  mAllocationSize += alignedSize(sizeof(SDES::Chunk::Mid)); break;
            default:
            {
              mAllocationSize += alignedSize(sizeof(SDES::Chunk::Unknown));
              ZS_LOG_WARNING(Insane, log("SDES item type is not understood") + ZS_PARAM("type", type))
              break;
            }
          }

          if (0 != length) {
            mAllocationSize += alignedSize((sizeof(char)*length)+sizeof(char));
          }

          advancePos(pos, remaining, length);
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

      mAllocationSize += alignedSize(sizeof(Bye)) + (alignedSize(sizeof(DWORD)) * ssrcCount);

      const BYTE *pos = contents;
      size_t remaining = contentSize;

      if (remaining < (ssrcCount * sizeof(DWORD))) {
        ZS_LOG_WARNING(Trace, debug("malformed BYE SSRC size"))
        return false;
      }

      advancePos(pos, remaining, sizeof(DWORD) * ssrcCount);

      if (remaining < sizeof(BYTE)) return true;

      size_t length = static_cast<size_t>(*pos);

      if (length < 1) return true;

      if (length > remaining) {
        ZS_LOG_WARNING(Trace, debug("malformed BYE reason length") + ZS_PARAM("length", length) + ZS_PARAM("remaining", remaining))
        return false;
      }

      mAllocationSize += alignedSize((sizeof(char)*(length+1)));
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

      mAllocationSize += alignedSize(sizeof(App));

      size_t remaining = contentSize;

      if (remaining < (sizeof(DWORD)*2)) {
        ZS_LOG_WARNING(Detail, log("malformed APP length") + ZS_PARAM("remaining", remaining))
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

      mAllocationSize += alignedSize(sizeof(TransportLayerFeedbackMessage));

      const BYTE *pos = contents;
      size_t remaining = contentSize;

      if (remaining < (sizeof(DWORD)*2)) {
        ZS_LOG_WARNING(Detail, log("malformed transport layer feedback message length") + ZS_PARAM("remaining", remaining))
        return false;
      }

      advancePos(pos, remaining, sizeof(DWORD)*2);

      bool result = false;

      switch (reportSpecific) {
        case TransportLayerFeedbackMessage::GenericNACK::kFmt:  result = getTransportLayerFeedbackMessageGenericNACKAllocationSize(reportSpecific, pos, remaining); break;
        case TransportLayerFeedbackMessage::TMMBR::kFmt:        result = getTransportLayerFeedbackMessageTMMBRAllocationSize(reportSpecific, pos, remaining); break;
        case TransportLayerFeedbackMessage::TMMBN::kFmt:        result = getTransportLayerFeedbackMessageTMMBNAllocationSize(reportSpecific, pos, remaining); break;
        default: {
          break;
        }
      }

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

      mAllocationSize += alignedSize(sizeof(PayloadSpecificFeedbackMessage));

      const BYTE *pos = contents;
      size_t remaining = contentSize;

      if (remaining < (sizeof(DWORD)*2)) {
        ZS_LOG_WARNING(Detail, log("malformed transport layer feedback message length") + ZS_PARAM("remaining", remaining))
        return false;
      }

      advancePos(pos, remaining, sizeof(DWORD)*2);

      bool result = false;

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

      mAllocationSize += alignedSize(sizeof(XR));

      const BYTE *pos = contents;
      size_t remaining = contentSize;

      if (remaining < sizeof(DWORD)) {
        ZS_LOG_WARNING(Trace, debug("XR is not a valid length") + ZS_PARAM("remaining", remaining))
        return false;
      }

      advancePos(pos, remaining, sizeof(DWORD));

      while (remaining >= sizeof(DWORD)) {
        BYTE bt = pos[0];
        BYTE typeSpecific = pos[1];
        size_t blockLength = static_cast<size_t>(UseHelper::getBE16(&(pos[2]))) * sizeof(DWORD);

        advancePos(pos, remaining, sizeof(DWORD));

        if (remaining < blockLength) {
          ZS_LOG_WARNING(Trace, debug("malformed XR block length found") + ZS_PARAM("block length", blockLength) + ZS_PARAM("remaining", remaining))
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

        advancePos(pos, remaining, blockLength);
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

      mAllocationSize += alignedSize(sizeof(UnknownReport));
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
        ZS_LOG_WARNING(Trace, debug("malformed generic NACK transport layer feedback message") + ZS_PARAM("remaining", remaining))
        return false;
      }

      size_t possibleNACKs = remaining / sizeof(DWORD);

      mAllocationSize += (alignedSize(sizeof(TransportLayerFeedbackMessage::GenericNACK)) * possibleNACKs);
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
        ZS_LOG_WARNING(Trace, debug("malformed TMMBR transport layer feedback message") + ZS_PARAM("remaining", remaining))
        return false;
      }

      size_t possibleTMMBRs = remaining / (sizeof(DWORD)*2);

      mAllocationSize += (alignedSize(sizeof(TransportLayerFeedbackMessage::TMMBR)) * possibleTMMBRs);
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
        ZS_LOG_WARNING(Trace, debug("malformed TMMBN transport layer feedback message") + ZS_PARAM("remaining", remaining))
        return false;
      }

      size_t possibleTMMBNs = remaining / (sizeof(DWORD)*2);

      mAllocationSize += (alignedSize(sizeof(TransportLayerFeedbackMessage::TMMBN)) * possibleTMMBNs);
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
        ZS_LOG_WARNING(Trace, debug("malformed SLI payload specific feedback message") + ZS_PARAM("remaining", remaining))
        return false;
      }

      size_t possibleSLIs = remaining / sizeof(DWORD);

      mAllocationSize += (alignedSize(sizeof(PayloadSpecificFeedbackMessage::SLI)) * possibleSLIs);
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
        ZS_LOG_WARNING(Trace, debug("malformed FIR payload specific feedback message") + ZS_PARAM("remaining", remaining))
        return false;
      }

      size_t possibleFIRs = remaining / (sizeof(DWORD)*2);

      mAllocationSize += (alignedSize(sizeof(PayloadSpecificFeedbackMessage::FIR)) * possibleFIRs);
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
        ZS_LOG_WARNING(Trace, debug("malformed TSTR payload specific feedback message") + ZS_PARAM("remaining", remaining))
        return false;
      }

      size_t possibleTSTRs = remaining / (sizeof(DWORD)*2);

      mAllocationSize += (alignedSize(sizeof(PayloadSpecificFeedbackMessage::TSTR)) * possibleTSTRs);
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
        ZS_LOG_WARNING(Trace, debug("malformed TSTN payload specific feedback message") + ZS_PARAM("remaining", remaining))
        return false;
      }

      size_t possibleTSTNs = remaining / (sizeof(DWORD)*2);

      mAllocationSize += (alignedSize(sizeof(PayloadSpecificFeedbackMessage::TSTN)) * possibleTSTNs);
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
        mAllocationSize += alignedSize(sizeof(PayloadSpecificFeedbackMessage::VBCM));

        size_t length = UseHelper::getBE16(&(pos[6]));
        size_t modulas = length % sizeof(DWORD);
        size_t padding = ((0 != modulas) ? (sizeof(DWORD)-modulas) : 0);

        advancePos(pos, remaining, sizeof(DWORD)*2);

        if (remaining < length) {
          ZS_LOG_WARNING(Trace, debug("malformed VBCM payload specific feedback message") + ZS_PARAM("remaining", remaining) + ZS_PARAM("length", length))
          return false;
        }

        size_t skipLength = (((length + padding) > remaining) ? remaining : (length + padding));

        advancePos(pos, remaining, skipLength);
      }

      if (possibleVBCMs < 1) {
        ZS_LOG_WARNING(Trace, debug("malformed VBCM payload specific feedback message") + ZS_PARAM("remaining", remaining) + ZS_PARAM("possible", possibleVBCMs))
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
        ZS_LOG_WARNING(Trace, debug("malformed REMB payload specific feedback message") + ZS_PARAM("remaining", remaining))
        return false;
      }

      size_t possibleSSRCs = (remaining - (sizeof(DWORD)*2)) / sizeof(DWORD);

      mAllocationSize += (alignedSize(sizeof(DWORD)) * possibleSSRCs);
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool RTCPPacket::getXRLossRLEReportBlockAllocationSize(
                                                           BYTE typeSpecific,
                                                           const BYTE *contents,
                                                           size_t contentSize
                                                           )
    {
      mAllocationSize += alignedSize(sizeof(XR::LossRLEReportBlock));

      const BYTE *pos = contents;
      size_t remaining = contentSize;

      if (remaining < (sizeof(DWORD)*2)) {
        ZS_LOG_WARNING(Trace, debug("malformed loss RLE report block"))
        return false;
      }

      advancePos(pos, remaining, sizeof(DWORD)*2);

      size_t possibleChunks = remaining  / sizeof(WORD);

      if (0 != possibleChunks) {
        mAllocationSize = alignedSize(sizeof(XR::RLEChunk)) * possibleChunks;
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
      mAllocationSize += alignedSize(sizeof(XR::DuplicateRLEReportBlock));

      const BYTE *pos = contents;
      size_t remaining = contentSize;

      if (remaining < (sizeof(DWORD)*2)) {
        ZS_LOG_WARNING(Trace, debug("malformed duplicate RLE report block"))
        return false;
      }

      advancePos(pos, remaining, sizeof(DWORD)*2);

      size_t possibleChunks = remaining  / sizeof(WORD);

      if (0 != possibleChunks) {
        mAllocationSize = alignedSize(sizeof(XR::RLEChunk)) * possibleChunks;
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
      mAllocationSize += alignedSize(sizeof(XR::PacketReceiptTimesReportBlock));

      const BYTE *pos = contents;
      size_t remaining = contentSize;

      if (remaining < (sizeof(DWORD)*2)) {
        ZS_LOG_WARNING(Trace, debug("malformed packet receipt times report block"))
        return false;
      }

      advancePos(pos, remaining, sizeof(DWORD)*2);

      size_t possibleReceiptTimes = remaining  / sizeof(DWORD);

      if (0 != possibleReceiptTimes) {
        mAllocationSize = alignedSize(sizeof(DWORD)) * possibleReceiptTimes;
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
      mAllocationSize += alignedSize(sizeof(XR::ReceiverReferenceTimeReportBlock));
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool RTCPPacket::getXRDLRRReportBlockAllocationSize(
                                                        BYTE typeSpecific,
                                                        const BYTE *contents,
                                                        size_t contentSize
                                                        )
    {
      mAllocationSize += alignedSize(sizeof(XR::DLRRReportBlock));

      size_t possibleSubBlocks = contentSize / (sizeof(DWORD) + sizeof(DWORD) + sizeof(DWORD));
      if (0 != possibleSubBlocks) {
        mAllocationSize = alignedSize(sizeof(XR::DLRRReportBlock::SubBlock)) * possibleSubBlocks;
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
      mAllocationSize += alignedSize(sizeof(XR::StatisticsSummaryReportBlock));
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTCPPacket::getXRVoIPMetricsReportBlockAllocationSize(
                                                               BYTE typeSpecific,
                                                               const BYTE *contents,
                                                               size_t contentSize
                                                               )
    {
      mAllocationSize += alignedSize(sizeof(XR::VoIPMetricsReportBlock));
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTCPPacket::getXRUnknownReportBlockAllocationSize(
                                                           BYTE typeSpecific,
                                                           const BYTE *contents,
                                                           size_t contentSize
                                                           )
    {
      mAllocationSize += alignedSize(sizeof(XR::UnknownReportBlock));
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
      bool result = false;

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
          auto temp = &(mFirstXR[mXRCount]); break;
          fill(temp, version, padding, reportSpecific, pt, contents, contentSize);
          if (!parse(temp)) return false;
          usingReport = temp;
        }
        default:
        {
          auto temp = &(mFirstUnknownReport[mUnknownReportCount]);
          fill(temp, version, padding, reportSpecific, pt, contents, contentSize);
          usingReport = temp;
          if (!parse(temp)) return false;
          break;
        }
      }

      if (ioLastReport) {
        ioLastReport->mNext = usingReport;
      }

      ioLastReport = usingReport;
      return result;
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

        report->mSSRCOfSender = UseHelper::getBE32(pos);

        advancePos(pos, remaining, sizeof(DWORD));

        if (remaining < detailedHeaderSize) goto illegal_size;

        advancePos(pos, remaining, detailedHeaderSize);

        size_t count = 0;

        if (report->rc() > 0) {
          report->mFirstReportBlock = new (allocateBuffer(alignedSize(sizeof(SenderReceiverCommonReport::ReportBlock))*report->rc())) SenderReceiverCommonReport::ReportBlock[report->rc()];

          while ((remaining >= (sizeof(DWORD)*6)) &&
                 (count < report->rc()))
          {
            SenderReceiverCommonReport::ReportBlock *block = &(report->mFirstReportBlock[count]);
            if (0 != count) {
              report->mFirstReportBlock[count-1].mNext = block;
            }

            block->mSSRC = UseHelper::getBE32(&(pos[0]));
            block->mFractionLost = pos[4];
            block->mCumulativeNumberOfPacketsLost = UseHelper::getBE32(&(pos[4]));
            block->mCumulativeNumberOfPacketsLost = block->mCumulativeNumberOfPacketsLost & 0x00FFFFFF;
            block->mExtendedHighestSequenceNumberReceived = UseHelper::getBE32(&(pos[8]));
            block->mInterarrivalJitter = UseHelper::getBE32(&(pos[12]));
            block->mLSR = UseHelper::getBE32(&(pos[16]));
            block->mDLSR = UseHelper::getBE32(&(pos[20]));

            advancePos(pos, remaining, sizeof(DWORD)*6);

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
        ZS_LOG_WARNING(Trace, debug("unable to parse sender/receiver report (malformed packet)") + ZS_PARAM("remaining", remaining) + ZS_PARAM("detailed header size", detailedHeaderSize))
      }

      return false;
    }

    //-------------------------------------------------------------------------
    bool RTCPPacket::parse(SenderReport *report)
    {
      if (0 != mSenderReportCount) {
        auto temp = &(mFirstSenderReport[mSenderReportCount-1]);;
        temp->mNextSenderReport = report;
      }
      ++mSenderReportCount;

      if (!parseCommon(report, sizeof(DWORD)*5)) return false;

      const BYTE *pos = report->ptr();

      report->mNTPTimestampMS = UseHelper::getBE32(&(pos[4]));
      report->mNTPTimestampLS = UseHelper::getBE32(&(pos[8]));
      report->mRTPTimestamp = UseHelper::getBE32(&(pos[12]));
      report->mSenderPacketCount = UseHelper::getBE32(&(pos[16]));
      report->mSenderOctetCount = UseHelper::getBE32(&(pos[20]));

      return true;
    }

    //-------------------------------------------------------------------------
    bool RTCPPacket::parse(ReceiverReport *report)
    {
      if (0 != mReceiverReportCount) {
        auto temp = &(mFirstReceiverReport[mReceiverReportCount-1]);;
        temp->mNextReceiverReport = report;
      }
      ++mReceiverReportCount;

      if (!parseCommon(report, 0)) return false;

      return true;
    }
    
    //-------------------------------------------------------------------------
    bool RTCPPacket::parse(SDES *report)
    {
      if (0 != mSDESCount) {
        auto temp = &(mFirstSDES[mSDESCount-1]);;
        temp->mNextSDES = report;
      }
      ++mSDESCount;

      if (0 == report->sc()) return true;

      size_t chunkCount = 0;

      const BYTE *pos = report->ptr();
      size_t remaining = report->size();

      while ((remaining > sizeof(DWORD)) &&
             ((chunkCount < report->sc())))
      {
        SDES::Chunk *chunk = &(report->mFirstChunk[chunkCount]);
        if (0 != chunkCount) {
          report->mFirstChunk[chunkCount-1].mNext = chunk;
        }
        ++chunkCount;

        chunk->mSSRC = UseHelper::getBE32(pos);
        advancePos(pos, remaining, sizeof(DWORD));

        const BYTE *startOfItems = pos;
        size_t remainingAtStartOfItems = remaining;

        // first do an items count
        while (remaining >= sizeof(BYTE)) {

          BYTE type = *pos;
          advancePos(pos, remaining);

          if (SDES::Chunk::kEndOfItemsType == type) {
            // stop now (going to retry parsing again anyway)
            break;
          }

          ASSERT(remaining >= sizeof(BYTE))

          size_t length = static_cast<size_t>(*pos);
          advancePos(pos, remaining);

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
            default:
            {
              ++(chunk->mUnknownCount);
              ZS_LOG_WARNING(Insane, log("SDES item type is not understood") + ZS_PARAM("type", type))
              break;
            }
          }

          advancePos(pos, remaining, length);
        }

        // allocate space for items

        if (0 != chunk->mCNameCount) {
          chunk->mFirstCName = new (allocateBuffer(alignedSize(sizeof(SDES::Chunk::CName))*(chunk->mCNameCount))) SDES::Chunk::CName[chunk->mCNameCount];
        }
        if (0 != chunk->mNameCount) {
          chunk->mFirstName = new (allocateBuffer(alignedSize(sizeof(SDES::Chunk::Name))*(chunk->mNameCount))) SDES::Chunk::Name[chunk->mNameCount];
        }
        if (0 != chunk->mEmailCount) {
          chunk->mFirstEmail = new (allocateBuffer(alignedSize(sizeof(SDES::Chunk::Email))*(chunk->mEmailCount))) SDES::Chunk::Email[chunk->mEmailCount];
        }
        if (0 != chunk->mPhoneCount) {
          chunk->mFirstPhone = new (allocateBuffer(alignedSize(sizeof(SDES::Chunk::Phone))*(chunk->mPhoneCount))) SDES::Chunk::Phone[chunk->mPhoneCount];
        }
        if (0 != chunk->mLocCount) {
          chunk->mFirstLoc = new (allocateBuffer(alignedSize(sizeof(SDES::Chunk::Loc))*(chunk->mLocCount))) SDES::Chunk::Loc[chunk->mLocCount];
        }
        if (0 != chunk->mToolCount) {
          chunk->mFirstTool = new (allocateBuffer(alignedSize(sizeof(SDES::Chunk::Tool))*(chunk->mToolCount))) SDES::Chunk::Tool[chunk->mToolCount];
        }
        if (0 != chunk->mNoteCount) {
          chunk->mFirstNote = new (allocateBuffer(alignedSize(sizeof(SDES::Chunk::Note))*(chunk->mNoteCount))) SDES::Chunk::Note[chunk->mNoteCount];
        }
        if (0 != chunk->mPrivCount) {
          chunk->mFirstPriv = new (allocateBuffer(alignedSize(sizeof(SDES::Chunk::Priv))*(chunk->mPrivCount))) SDES::Chunk::Priv[chunk->mPrivCount];
        }
        if (0 != chunk->mMidCount) {
          chunk->mFirstMid = new (allocateBuffer(alignedSize(sizeof(SDES::Chunk::Mid))*(chunk->mMidCount))) SDES::Chunk::Mid[chunk->mMidCount];
        }
        if (0 != chunk->mUnknownCount) {
          chunk->mFirstUnknown = new (allocateBuffer(alignedSize(sizeof(SDES::Chunk::Unknown))*(chunk->mUnknownCount))) SDES::Chunk::Unknown[chunk->mUnknownCount];
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
        chunk->mUnknownCount = 0;

        // start over and now parse
        pos = startOfItems;
        remaining = remainingAtStartOfItems;

        while (remaining >= sizeof(BYTE)) {

          BYTE type = *pos;
          advancePos(pos, remaining);

          if (SDES::Chunk::kEndOfItemsType == type) {
            // skip NUL item (no length octet is present)

            // skip to next DWORD alignment
            auto diff = reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(ptr());
            while ((0 != (diff % sizeof(DWORD))) &&
                   (remaining > 0))
            {
              // only NUL chunks are allowed
              ASSERT(SDES::Chunk::kEndOfItemsType == (*pos))
              advancePos(pos, remaining);
              ++diff;
            }
            break;
          }

          ASSERT(remaining >= sizeof(BYTE))

          size_t length = static_cast<size_t>(*pos);
          advancePos(pos, remaining);

          ASSERT(remaining >= length)

          SDES::Chunk::StringItem *item {};

          switch (type) {
            case SDES::Chunk::CName::kItemType: {
              item = &(chunk->mFirstCName[chunk->mCNameCount]);
              if (0 != chunk->mCNameCount) {
                item->mNext = &(chunk->mFirstCName[chunk->mCNameCount-1]);
              }
              ++(chunk->mCNameCount);
              break;
            }
            case SDES::Chunk::Name::kItemType:  {
              item = &(chunk->mFirstName[chunk->mNameCount]);
              if (0 != chunk->mNameCount) {
                item->mNext = &(chunk->mFirstName[chunk->mNameCount-1]);
              }
              ++(chunk->mNameCount);
              break;
            }
            case SDES::Chunk::Email::kItemType: {
              item = &(chunk->mFirstEmail[chunk->mEmailCount]);
              if (0 != chunk->mEmailCount) {
                item->mNext = &(chunk->mFirstEmail[chunk->mEmailCount-1]);
              }
              ++(chunk->mEmailCount);
              break;
            }
            case SDES::Chunk::Phone::kItemType: {
              item = &(chunk->mFirstPhone[chunk->mPhoneCount]);
              if (0 != chunk->mPhoneCount) {
                item->mNext = &(chunk->mFirstPhone[chunk->mPhoneCount-1]);
              }
              ++(chunk->mPhoneCount);
              break;
            }
            case SDES::Chunk::Loc::kItemType:   {
              item = &(chunk->mFirstLoc[chunk->mLocCount]);
              if (0 != chunk->mLocCount) {
                item->mNext = &(chunk->mFirstLoc[chunk->mLocCount-1]);
              }
              ++(chunk->mLocCount);
              break;
            }
            case SDES::Chunk::Tool::kItemType:  {
              item = &(chunk->mFirstTool[chunk->mToolCount]);
              if (0 != chunk->mToolCount) {
                item->mNext = &(chunk->mFirstTool[chunk->mToolCount-1]);
              }
              ++(chunk->mToolCount);
              break;
            }
            case SDES::Chunk::Note::kItemType:  {
              item = &(chunk->mFirstNote[chunk->mNoteCount]);
              if (0 != chunk->mNoteCount) {
                item->mNext = &(chunk->mFirstNote[chunk->mNoteCount-1]);
              }
              ++(chunk->mNoteCount);
              break;
            }
            case SDES::Chunk::Priv::kItemType:  {
              SDES::Chunk::Priv *priv = &(chunk->mFirstPriv[chunk->mPrivCount]);
              if (0 != chunk->mPrivCount) {
                priv->mNext = &(chunk->mFirstPriv[chunk->mPrivCount-1]);
              }

              if (length > 0) {
                size_t privLength = static_cast<size_t>(*pos);
                if (0 != privLength) {
                  advancePos(pos, remaining);

                  priv->mPrefix = new (allocateBuffer(sizeof(char)*(privLength+1))) char [privLength+1];
                  memcpy(const_cast<char *>(priv->mValue), pos, privLength);

                  advancePos(pos, remaining, privLength);
                  ASSERT(length >= privLength)

                  length -= privLength;
                }
              }

              item = priv;

              ++(chunk->mPrivCount);
              break;
            }
            case SDES::Chunk::Mid::kItemType:  {
              item = &(chunk->mFirstMid[chunk->mMidCount]);
              if (0 != chunk->mMidCount) {
                item->mNext = &(chunk->mFirstMid[chunk->mMidCount-1]);
              }
              ++(chunk->mMidCount);
              break;
            }
            default:
            {
              item = &(chunk->mFirstUnknown[chunk->mUnknownCount]);
              if (0 != chunk->mUnknownCount) {
                item->mNext = &(chunk->mFirstUnknown[chunk->mUnknownCount-1]);
              }
              ++(chunk->mUnknownCount);
              ZS_LOG_WARNING(Insane, log("SDES item type is not understood") + ZS_PARAM("type", type))
              break;
            }
          }

          if (NULL != item) {
            item->mType = type;
            if (length > 0) {
              item->mValue = new (allocateBuffer(length+1)) char [length+1];
              memcpy(const_cast<char *>(item->mValue), pos, length);
            }
          }

          advancePos(pos, remaining, length);
        }

      }

      if (chunkCount != report->sc()) {
        ZS_LOG_WARNING(Trace, debug("chunk count in SDES does not match packet count") + ZS_PARAM("chunk count", chunkCount) + ZS_PARAM("packet count", report->sc()))
        return false;
      }

      return true;
    }
    
    //-------------------------------------------------------------------------
    bool RTCPPacket::parse(Bye *report)
    {
      if (0 != mByeCount) {
        auto temp = &(mFirstBye[mByeCount-1]);;
        temp->mNextBye = report;
      }
      ++mByeCount;

      report->mSSRCs = new (allocateBuffer(alignedSize(sizeof(DWORD))*(report->sc()))) DWORD[report->sc()];

      const BYTE *pos = report->ptr();
      size_t remaining = report->size();

      if (NULL == pos) {
        if (0 != report->sc()) {
          ZS_LOG_WARNING(Trace, debug("BYE report count > 0 but does not contain SSRCs") + ZS_PARAM("sc", report->sc()))
          return false;
        }

        return true;
      }

      {
        size_t index = 0;

        while (index < report->sc()) {
          if (remaining < sizeof(DWORD)) goto illegal_remaining;

          report->mSSRCs[index] = UseHelper::getBE32(pos);
          advancePos(pos, remaining, sizeof(DWORD));
          ++index;
        }

        if (remaining > sizeof(BYTE)) {
          size_t length = static_cast<size_t>(*pos);
          advancePos(pos, remaining);

          if (remaining < length) goto illegal_remaining;

          if (length > 0) {
            report->mReasonForLeaving = new (allocateBuffer(alignedSize((sizeof(char)*length)+sizeof(char)))) char[length+1];
            memcpy(const_cast<char *>(report->mReasonForLeaving), pos, length);
          }
        }

        return true;
      }

    illegal_remaining:
      {
        ZS_LOG_WARNING(Trace, debug("malformed BYE") + ZS_PARAM("pos", reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->ptr()))  + ZS_PARAM("remaining", remaining))
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTCPPacket::parse(App *report)
    {
      if (0 != mAppCount) {
        auto temp = &(mFirstApp[mAppCount-1]);;
        temp->mNextApp = report;
      }
      ++mAppCount;

      const BYTE *pos = report->ptr();
      size_t remaining = report->size();

      {
        if (remaining < sizeof(DWORD)) goto illegal_remaining;

        report->mSSRC = UseHelper::getBE32(pos);
        advancePos(pos, remaining, sizeof(DWORD));

        if (remaining < sizeof(DWORD)) goto illegal_remaining;

        for (size_t index = 0; index < sizeof(DWORD); ++index) {
          report->mName[index] = static_cast<char>(*pos);
        }
        advancePos(pos, remaining, sizeof(DWORD));

        if (0 != remaining) {
          report->mData = pos;
          report->mDataSize = remaining;
        }

        return true;
      }

    illegal_remaining:
      {
        ZS_LOG_WARNING(Trace, debug("malformed APP") + ZS_PARAM("pos", reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->ptr()))  + ZS_PARAM("remaining", remaining))
      }
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool RTCPPacket::parse(TransportLayerFeedbackMessage *report)
    {
      if (0 != mTransportLayerFeedbackMessageCount) {
        auto temp = &(mFirstTransportLayerFeedbackMessage[mTransportLayerFeedbackMessageCount-1]);;
        temp->mNextTransportLayerFeedbackMessage = report;
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

        return true;
      }

    illegal_remaining:
      {
        ZS_LOG_WARNING(Trace, debug("malformed transport layer feedback message") + ZS_PARAM("pos", reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->ptr()))  + ZS_PARAM("remaining", remaining))
      }
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool RTCPPacket::parse(PayloadSpecificFeedbackMessage *report)
    {
      if (0 != mPayloadSpecificFeedbackMessageCount) {
        auto temp = &(mFirstPayloadSpecificFeedbackMessage[mPayloadSpecificFeedbackMessageCount-1]);;
        temp->mNextPayloadSpecificFeedbackMessage = report;
      }
      ++mPayloadSpecificFeedbackMessageCount;

      const BYTE *pos = report->ptr();
      size_t remaining = report->size();

      {
        if (remaining < (sizeof(DWORD)*2)) goto illegal_remaining;

        fill(report, pos, remaining);
        advancePos(pos, remaining, sizeof(DWORD)*2);

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

        return true;
      }

    illegal_remaining:
      {
        ZS_LOG_WARNING(Trace, debug("malformed payload specific feedback message") + ZS_PARAM("pos", reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->ptr()))  + ZS_PARAM("remaining", remaining))
      }
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool RTCPPacket::parse(XR *report)
    {
      if (0 != mXRCount) {
        auto temp = &(mFirstXR[mXRCount-1]);;
        temp->mNextXR = report;
      }
      ++mXRCount;

      const BYTE *pos = report->ptr();
      size_t remaining = report->size();

      {
        if (remaining < sizeof(DWORD)) goto illegal_remaining;

        report->mSSRC = UseHelper::getBE32(pos);
        advancePos(pos, remaining, sizeof(DWORD));

        // first count the totals for each XR block type
        while (remaining >= sizeof(DWORD)) {

          BYTE bt = pos[0];
          size_t blockLength = static_cast<size_t>(UseHelper::getBE16(&(pos[2]))) * sizeof(DWORD);

          advancePos(pos, remaining, sizeof(DWORD));

          ++report->mReportBlockCount;

          if (remaining < blockLength) {
            ZS_LOG_WARNING(Trace, debug("illegal XR block length found") + ZS_PARAM("block length", blockLength) + ZS_PARAM("remaining", remaining))
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
          
          advancePos(pos, remaining, blockLength);
        }

        if (0 != report->mLossRLEReportBlockCount) {
          report->mFirstLossRLEReportBlock = new (allocateBuffer(alignedSize(sizeof(XR::LossRLEReportBlock))*(report->mLossRLEReportBlockCount))) XR::LossRLEReportBlock[report->mLossRLEReportBlockCount];
        }
        if (0 != report->mDuplicateRLEReportBlockCount) {
          report->mFirstDuplicateRLEReportBlock = new (allocateBuffer(alignedSize(sizeof(XR::DuplicateRLEReportBlock))*(report->mDuplicateRLEReportBlockCount))) XR::DuplicateRLEReportBlock[report->mDuplicateRLEReportBlockCount];
        }
        if (0 != report->mPacketReceiptTimesReportBlockCount) {
          report->mFirstPacketReceiptTimesReportBlock = new (allocateBuffer(alignedSize(sizeof(XR::PacketReceiptTimesReportBlock))*(report->mPacketReceiptTimesReportBlockCount))) XR::PacketReceiptTimesReportBlock[report->mPacketReceiptTimesReportBlockCount];
        }
        if (0 != report->mReceiverReferenceTimeReportBlockCount) {
          report->mFirstReceiverReferenceTimeReportBlock = new (allocateBuffer(alignedSize(sizeof(XR::ReceiverReferenceTimeReportBlock))*(report->mReceiverReferenceTimeReportBlockCount))) XR::ReceiverReferenceTimeReportBlock[report->mReceiverReferenceTimeReportBlockCount];
        }
        if (0 != report->mDLRRReportBlockCount) {
          report->mFirstDLRRReportBlock = new (allocateBuffer(alignedSize(sizeof(XR::DLRRReportBlock))*(report->mDLRRReportBlockCount))) XR::DLRRReportBlock[report->mDLRRReportBlockCount];
        }
        if (0 != report->mStatisticsSummaryReportBlockCount) {
          report->mFirstStatisticsSummaryReportBlock = new (allocateBuffer(alignedSize(sizeof(XR::StatisticsSummaryReportBlock))*(report->mStatisticsSummaryReportBlockCount))) XR::StatisticsSummaryReportBlock[report->mStatisticsSummaryReportBlockCount];
        }
        if (0 != report->mVoIPMetricsReportBlockCount) {
          report->mFirstVoIPMetricsReportBlock = new (allocateBuffer(alignedSize(sizeof(XR::VoIPMetricsReportBlock))*(report->mVoIPMetricsReportBlockCount))) XR::VoIPMetricsReportBlock[report->mVoIPMetricsReportBlockCount];
        }
        if (0 != report->mUnknownReportBlockCount) {
          report->mFirstUnknownReportBlock = new (allocateBuffer(alignedSize(sizeof(XR::UnknownReportBlock))*(report->mUnknownReportBlockCount))) XR::UnknownReportBlock[report->mUnknownReportBlockCount];
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

        advancePos(pos, remaining, sizeof(DWORD));

        // parse each XR report block
        while (remaining >= sizeof(DWORD)) {

          BYTE bt = pos[0];
          BYTE typeSpecific = pos[1];
          size_t blockLength = static_cast<size_t>(UseHelper::getBE16(&(pos[2]))) * sizeof(DWORD);

          advancePos(pos, remaining, sizeof(DWORD));

          if (remaining < blockLength) {
            ZS_LOG_WARNING(Trace, debug("malformed XR block length found") + ZS_PARAM("block length", blockLength) + ZS_PARAM("remaining", remaining))
            return false;
          }

          switch (bt) {
            case XR::LossRLEReportBlock::kBlockType:                {
              auto reportBlock = &(report->mFirstLossRLEReportBlock[report->mLossRLEReportBlockCount]);
              if (0 != report->mLossRLEReportBlockCount) {
                auto temp = &(report->mFirstLossRLEReportBlock[report->mLossRLEReportBlockCount-1]);
                temp->mNextLossRLE = reportBlock;
              }
              ++(report->mLossRLEReportBlockCount);
              fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
              if (!parse(report, reportBlock)) return false;
              break;
            }
            case XR::DuplicateRLEReportBlock::kBlockType:           {
              auto reportBlock = &(report->mFirstDuplicateRLEReportBlock[report->mDuplicateRLEReportBlockCount]);
              if (0 != report->mDuplicateRLEReportBlockCount) {
                auto temp = &(report->mFirstDuplicateRLEReportBlock[report->mDuplicateRLEReportBlockCount-1]);
                temp->mNextDuplicateRLE = reportBlock;
              }
              ++(report->mDuplicateRLEReportBlockCount);
              fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
              if (!parse(report, reportBlock)) return false;
              break;
            }
            case XR::PacketReceiptTimesReportBlock::kBlockType:     {
              auto reportBlock = &(report->mFirstPacketReceiptTimesReportBlock[report->mPacketReceiptTimesReportBlockCount]);
              if (0 != report->mPacketReceiptTimesReportBlockCount) {
                auto temp = &(report->mFirstPacketReceiptTimesReportBlock[report->mPacketReceiptTimesReportBlockCount-1]);
                temp->mNextPacketReceiptTimesReportBlock = reportBlock;
              }
              ++(report->mPacketReceiptTimesReportBlockCount);
              fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
              if (!parse(report, reportBlock)) return false;
              break;
            }
            case XR::ReceiverReferenceTimeReportBlock::kBlockType:  {
              auto reportBlock = &(report->mFirstReceiverReferenceTimeReportBlock[report->mReceiverReferenceTimeReportBlockCount]);
              if (0 != report->mReceiverReferenceTimeReportBlockCount) {
                auto temp = &(report->mFirstReceiverReferenceTimeReportBlock[report->mReceiverReferenceTimeReportBlockCount-1]);
                temp->mNextReceiverReferenceTimeReportBlock = reportBlock;
              }
              ++(report->mReceiverReferenceTimeReportBlockCount);
              fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
              if (!parse(report, reportBlock)) return false;
              break;
            }
            case XR::DLRRReportBlock::kBlockType:                   {
              auto reportBlock = &(report->mFirstDLRRReportBlock[report->mDLRRReportBlockCount]);
              if (0 != report->mDLRRReportBlockCount) {
                auto temp = &(report->mFirstDLRRReportBlock[report->mDLRRReportBlockCount-1]);
                temp->mNextDLRRReportBlock = reportBlock;
              }
              ++(report->mDLRRReportBlockCount);
              fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
              if (!parse(report, reportBlock)) return false;
              break;
            }
            case XR::StatisticsSummaryReportBlock::kBlockType:      {
              auto reportBlock = &(report->mFirstStatisticsSummaryReportBlock[report->mStatisticsSummaryReportBlockCount]);
              if (0 != report->mStatisticsSummaryReportBlockCount) {
                auto temp = &(report->mFirstStatisticsSummaryReportBlock[report->mStatisticsSummaryReportBlockCount-1]);
                temp->mNextStatisticsSummaryReportBlock = reportBlock;
              }
              ++(report->mStatisticsSummaryReportBlockCount);
              fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
              if (!parse(report, reportBlock)) return false;
              break;
            }
            case XR::VoIPMetricsReportBlock::kBlockType:            {
              auto reportBlock = &(report->mFirstVoIPMetricsReportBlock[report->mVoIPMetricsReportBlockCount]);
              if (0 != report->mVoIPMetricsReportBlockCount) {
                auto temp = &(report->mFirstVoIPMetricsReportBlock[report->mVoIPMetricsReportBlockCount-1]);
                temp->mNextVoIPMetricsReportBlock = reportBlock;
              }
              ++(report->mVoIPMetricsReportBlockCount);
              fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
              if (!parse(report, reportBlock)) return false;
              break;
            }
            default:                                                {
              auto reportBlock = &(report->mFirstUnknownReportBlock[report->mUnknownReportBlockCount]);
              if (0 != report->mUnknownReportBlockCount) {
                auto temp = &(report->mFirstUnknownReportBlock[report->mUnknownReportBlockCount-1]);
                temp->mNextUnknownReportBlock = reportBlock;
              }
              ++(report->mUnknownReportBlockCount);
              fill(report, reportBlock, previousReportBlock, bt, typeSpecific, pos, blockLength);
              if (!parse(report, reportBlock)) return false;
              break;
            }
          }

          advancePos(pos, remaining, blockLength);
        }
        

        return true;
      }

    illegal_remaining:
      {
        ZS_LOG_WARNING(Trace, debug("malformed XR when parsing") + ZS_PARAM("pos", reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->ptr()))  + ZS_PARAM("remaining", remaining))
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTCPPacket::parse(UnknownReport *report)
    {
      if (0 != mUnknownReportCount) {
        auto temp = &(mFirstUnknownReport[mUnknownReportCount-1]);;
        temp->mNextUnknown = report;
      }
      ++mUnknownReportCount;

      return true;
    }
    
    //-------------------------------------------------------------------------
    void RTCPPacket::fill(FeedbackMessage *report, const BYTE *contents, size_t contentSize)
    {
      const BYTE *pos = contents;
      size_t remaining = contentSize;

      report->mSSRCOfPacketSender = UseHelper::getBE32(&(pos[0]));
      report->mSSRCOfMediaSource = UseHelper::getBE32(&(pos[4]));

      advancePos(pos, remaining, sizeof(DWORD)*2);

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

      report->mFirstGenericNACK = new (allocateBuffer(alignedSize(sizeof(GenericNACK))*possibleNACKs)) GenericNACK[possibleNACKs];

      while (remaining >= sizeof(DWORD)) {
        GenericNACK *nack = &(report->mFirstGenericNACK[report->mGenericNACKCount]);

        nack->mPID = UseHelper::getBE16(&(pos[2]));
        nack->mBLP = UseHelper::getBE16(&(pos[0]));

        advancePos(pos, remaining, sizeof(DWORD));

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
      typedef TransportLayerFeedbackMessage::TMMBRCommon TMMBRCommon;

      common->mSSRC = UseHelper::getBE32(&(pos[0]));
      common->mMxTBRExp = RTCP_GET_BITS(pos[4], 0x3F, 2);
      common->mMxTBRMantissa = RTCP_GET_BITS(UseHelper::getBE32(&(pos[4])), 0x1FFFF, 9);
      common->mMeasuredOverhead = RTCP_GET_BITS(UseHelper::getBE32(&(pos[6])), 0x1FF, 0);
    }
    
    //-------------------------------------------------------------------------
    bool RTCPPacket::parseTMMBR(TransportLayerFeedbackMessage *report)
    {
      typedef TransportLayerFeedbackMessage::TMMBR TMMBR;

      const BYTE *pos = report->fci();
      size_t remaining = report->fciSize();

      size_t possibleTMMBRs = remaining / (sizeof(DWORD)*2);

      ASSERT(0 != possibleTMMBRs)

      report->mFirstTMMBR = new (allocateBuffer(alignedSize(sizeof(TMMBR))*possibleTMMBRs)) TMMBR[possibleTMMBRs];

      while (remaining >= (sizeof(DWORD)*2)) {
        TMMBR *tmmbr = &(report->mFirstTMMBR[report->mTMMBRCount]);

        fillTMMBRCommon(report, tmmbr, pos);

        advancePos(pos, remaining, sizeof(DWORD)*2);

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

      {
        size_t possibleTMMBNs = remaining / (sizeof(DWORD)*2);

        ASSERT(0 != possibleTMMBNs)

        report->mFirstTMMBN = new (allocateBuffer(alignedSize(sizeof(TMMBN))*possibleTMMBNs)) TMMBN[possibleTMMBNs];

        while (remaining >= (sizeof(DWORD)*2)) {
          TMMBN *tmmbr = &(report->mFirstTMMBN[report->mTMMBNCount]);

          fillTMMBRCommon(report, tmmbr, pos);

          advancePos(pos, remaining, sizeof(DWORD)*2);

          ++(report->mTMMBNCount);
        }

        ASSERT(possibleTMMBNs == report->mTMMBNCount)

        return true;
      }

    illegal_remaining:
      {
        ZS_LOG_WARNING(Trace, debug("malformed report block range") + ZS_PARAM("pos", reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->fci()))  + ZS_PARAM("remaining", remaining))
      }
      return false;
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

      {
        size_t possibleSLIs = remaining / (sizeof(DWORD));

        ASSERT(0 != possibleSLIs)

        report->mFirstSLI = new (allocateBuffer(alignedSize(sizeof(SLI))*possibleSLIs)) SLI[possibleSLIs];

        while (remaining >= (sizeof(DWORD))) {
          SLI *sli = &(report->mFirstSLI[report->mSLICount]);

          sli->mFirst = RTCP_GET_BITS(UseHelper::getBE16(&(pos[0])), 0x1FFF, 3);
          sli->mNumber = static_cast<WORD>(RTCP_GET_BITS(UseHelper::getBE32(&(pos[0])), 0x1FFF, 6));
          sli->mPictureID = RTCP_GET_BITS(pos[3], 0x3F, 0);

          advancePos(pos, remaining, sizeof(DWORD));

          ++(report->mSLICount);
        }

        ASSERT(possibleSLIs == report->mSLICount)

        return true;
      }

    illegal_remaining:
      {
        ZS_LOG_WARNING(Trace, debug("malformed SLI payload specific feedback message") + ZS_PARAM("pos", reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->fci()))  + ZS_PARAM("remaining", remaining))
      }
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool RTCPPacket::parseRPSI(PayloadSpecificFeedbackMessage *report)
    {
      const BYTE *pos = report->fci();
      size_t remaining = report->fciSize();

      {
        if (remaining < sizeof(WORD)) goto illegal_remaining;

        report->mRPSI.mPB = pos[0];
        report->mRPSI.mZeroBit = RTCP_GET_BITS(pos[1], 0x1, 7);
        report->mRPSI.mPayloadType = RTCP_GET_BITS(pos[1], 0x7F, 0);

        advancePos(pos, remaining, sizeof(WORD));

        if (remaining > 0) {
          report->mRPSI.mNativeRPSIBitString = pos;
          report->mRPSI.mNativeRPSIBitStringSizeInBits = (remaining*8);
          if (report->mRPSI.mPB > report->mRPSI.mNativeRPSIBitStringSizeInBits) goto illegal_remaining;

          report->mRPSI.mNativeRPSIBitStringSizeInBits -= static_cast<size_t>(report->mRPSI.mPB);
          if (0 == report->mRPSI.mNativeRPSIBitStringSizeInBits) {
            report->mRPSI.mNativeRPSIBitString = NULL;
          }
        }

        return true;
      }

    illegal_remaining:
      {
        ZS_LOG_WARNING(Trace, debug("malformed RPSI payload specific feedback message") + ZS_PARAM("pos", reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->fci()))  + ZS_PARAM("remaining", remaining))
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
      common->mSSRC = UseHelper::getBE32(&(pos[0]));
      common->mSeqNr = pos[4];
      common->mReserved = RTCP_GET_BITS(UseHelper::getBE32(&(pos[4])), 0xFFFFFF, 0);
    }

    //-------------------------------------------------------------------------
    bool RTCPPacket::parseFIR(PayloadSpecificFeedbackMessage *report)
    {
      typedef PayloadSpecificFeedbackMessage::FIR FIR;

      const BYTE *pos = report->fci();
      size_t remaining = report->fciSize();

      size_t possibleFIRs = remaining / (sizeof(DWORD)*2);

      ASSERT(0 != possibleFIRs)

      report->mFirstFIR = new (allocateBuffer(alignedSize(sizeof(FIR))*possibleFIRs)) FIR[possibleFIRs];

      while (remaining >= (sizeof(DWORD)*2)) {
        FIR *fir = &(report->mFirstFIR[report->mFIRCount]);
        fillCodecControlCommon(report, fir, pos);

        advancePos(pos, remaining, sizeof(DWORD)*2);

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

      report->mFirstTSTR = new (allocateBuffer(alignedSize(sizeof(TSTR))*possibleTSTRs)) TSTR[possibleTSTRs];

      while (remaining >= (sizeof(DWORD)*2)) {
        TSTR *tstr = &(report->mFirstTSTR[report->mTSTRCount]);
        fillCodecControlCommon(report, tstr, pos);

        tstr->mControlSpecific = RTCP_GET_BITS(tstr->mReserved, 0x1F, 0);
        tstr->mReserved = RTCP_GET_BITS(tstr->mReserved, 0x7FFFF, 5);

        advancePos(pos, remaining, sizeof(DWORD)*2);

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

      report->mFirstTSTN = new (allocateBuffer(alignedSize(sizeof(TSTN))*possibleTSTNs)) TSTN[possibleTSTNs];

      while (remaining >= (sizeof(DWORD)*2)) {
        TSTN *tstn = &(report->mFirstTSTN[report->mTSTNCount]);
        fillCodecControlCommon(report, tstn, pos);

        tstn->mControlSpecific = RTCP_GET_BITS(tstn->mReserved, 0x1F, 0);
        tstn->mReserved = RTCP_GET_BITS(tstn->mReserved, 0x7FFFF, 5);

        advancePos(pos, remaining, sizeof(DWORD)*2);

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

          size_t length = UseHelper::getBE16(&(pos[6]));
          size_t modulas = length % sizeof(DWORD);
          size_t padding = ((0 != modulas) ? (sizeof(DWORD)-modulas) : 0);

          advancePos(pos, remaining, sizeof(DWORD)*2);

          ASSERT(remaining >= length)

          size_t skipLength = (((length + padding) > remaining) ? remaining : (length + padding));

          advancePos(pos, remaining, skipLength);
        }
        ASSERT(0 != possibleVBCMs)
      }

      pos = report->fci();
      remaining = report->fciSize();

      {
        ASSERT(0 != possibleVBCMs)

        report->mFirstVBCM = new (allocateBuffer(alignedSize(sizeof(VBCM))*possibleVBCMs)) VBCM[possibleVBCMs];

        while (remaining >= (sizeof(DWORD)*2)) {
          VBCM *vcbm = &(report->mFirstVBCM[report->mVBCMCount]);
          fillCodecControlCommon(report, vcbm, pos);

          size_t length = UseHelper::getBE16(&(pos[6]));
          size_t modulas = length % sizeof(DWORD);
          size_t padding = ((0 != modulas) ? (sizeof(DWORD)-modulas) : 0);

          advancePos(pos, remaining, sizeof(DWORD)*2);

          ASSERT(remaining >= length)

          if (length > 0) {
            vcbm->mVBCMOctetString = pos;
          }

          size_t skipLength = (((length + padding) > remaining) ? remaining : (length + padding));

          advancePos(pos, remaining, skipLength);

          ++(report->mVBCMCount);
        }

        ASSERT(possibleVBCMs == report->mVBCMCount)

        return true;
      }
    }

    //-------------------------------------------------------------------------
    bool RTCPPacket::parseAFB(PayloadSpecificFeedbackMessage *report)
    {
      typedef PayloadSpecificFeedbackMessage::AFB AFB;

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
      typedef PayloadSpecificFeedbackMessage::REMB REMB;

      if (!parseAFB(report)) return false;

      const BYTE *pos = report->fci();
      size_t remaining = report->fciSize();

      report->mHasREMB = true;

      {
        ASSERT(remaining >= sizeof(DWORD)*3)

        report->mREMB.mNumSSRC = pos[4];
        report->mREMB.mBRExp = RTCP_GET_BITS(pos[5], 0x3F, 2);
        report->mREMB.mBRMantissa = RTCP_GET_BITS(UseHelper::getBE32(&(pos[4])), 0x3FFFF, 0);

        size_t possibleSSRCs = (remaining - sizeof(DWORD)*2) / (sizeof(DWORD));

        advancePos(pos, remaining, sizeof(DWORD)*2);

        report->mREMB.mSSRCs = new (allocateBuffer(alignedSize(sizeof(DWORD))*possibleSSRCs)) DWORD[possibleSSRCs];

        size_t count = 0;
        while ((possibleSSRCs > 0) &&
               (count < report->mREMB.numSSRC()) &&
               (remaining >= sizeof(DWORD)))
        {
          report->mREMB.mSSRCs[count] = UseHelper::getBE32(pos);
          advancePos(pos, remaining, sizeof(DWORD));
          --possibleSSRCs;
          ++count;
        }

        if (count != report->mREMB.numSSRC()) goto illegal_remaining;
        return true;
      }

    illegal_remaining:
      {
        ZS_LOG_WARNING(Trace, debug("malformed REMB payload specific feedback message") + ZS_PARAM("pos", reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(report->fci()))  + ZS_PARAM("remaining", remaining))
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

        reportBlock->mReserved = RTCP_GET_BITS(reportBlock->mTypeSpecific, 0xF, 4);
        reportBlock->mThinning = RTCP_GET_BITS(reportBlock->mTypeSpecific, 0xF, 0);
        reportBlock->mSSRCOfSource = UseHelper::getBE32(&(pos[0]));
        reportBlock->mBeginSeq = UseHelper::getBE16(&(pos[4]));
        reportBlock->mEndSeq = UseHelper::getBE16(&(pos[6]));

        return true;
      }

    illegal_remaining:
      {
        ZS_LOG_WARNING(Trace, debug("malformed report block range") + ZS_PARAM("pos", reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(reportBlock->typeSpecificContents()))  + ZS_PARAM("remaining", remaining))
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

      {
        if (!parseCommonRange(xr, reportBlock)) return false;

        advancePos(pos, remaining, sizeof(DWORD)*2);

        size_t possibleRLEsCount = (remaining / sizeof(WORD));

        if (0 == possibleRLEsCount) return true;

        reportBlock->mChunks = new (allocateBuffer(alignedSize(sizeof(XR::RLEChunk)*possibleRLEsCount))) XR::RLEChunk[possibleRLEsCount];

        while (remaining >= sizeof(WORD))
        {
          XR::RLEChunk value = UseHelper::getBE16(pos);
          advancePos(pos, remaining, sizeof(WORD));

          if (0 == value) break;
          reportBlock->mChunks[reportBlock->mChunkCount] = value;
          ++(reportBlock->mChunkCount);
        }

        if (0 == reportBlock->mChunkCount) {
          reportBlock->mChunks = NULL;
        }

        return true;
      }

    illegal_size:
      {
        ZS_LOG_WARNING(Trace, debug("malformed RLE block") + ZS_PARAM("pos", reinterpret_cast<PTRNUMBER>(pos) - reinterpret_cast<PTRNUMBER>(reportBlock->typeSpecificContents()))  + ZS_PARAM("remaining", remaining))
      }
      return false;
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

      advancePos(pos, remaining, sizeof(DWORD)*2);

      size_t possibleTimes = sizeof(DWORD) / remaining;

      if (0 == possibleTimes) return true;

      reportBlock->mReceiptTimes = new (allocateBuffer(alignedSize(sizeof(DWORD))*possibleTimes)) DWORD[possibleTimes];

      while (remaining >= sizeof(DWORD)) {
        reportBlock->mReceiptTimes[reportBlock->mReceiptTimeCount] = UseHelper::getBE32(pos);
        advancePos(pos, remaining, sizeof(DWORD));
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
        ZS_LOG_WARNING(Trace, debug("malformed receiver reference time report block") + ZS_PARAM("remain", remaining))
        return false;
      }

      reportBlock->mNTPTimestampMS = UseHelper::getBE32(&(pos[0]));
      reportBlock->mNTPTimestampLS = UseHelper::getBE32(&(pos[4]));

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
        ZS_LOG_WARNING(Trace, debug("malformed dlr report block") + ZS_PARAM("remain", remaining))
        return false;
      }

      reportBlock->mSubBlocks = new (allocateBuffer(alignedSize(sizeof(XR::DLRRReportBlock::SubBlock))*possibleSubBlockCount)) XR::DLRRReportBlock::SubBlock[possibleSubBlockCount];

      while (remaining >= (sizeof(DWORD)*3))
      {
        auto subBlock = &(reportBlock->mSubBlocks[reportBlock->mSubBlockCount]);

        subBlock->mSSRC = UseHelper::getBE32(&(pos[0]));
        subBlock->mLRR = UseHelper::getBE32(&(pos[4]));
        subBlock->mDLRR = UseHelper::getBE32(&(pos[8]));

        advancePos(pos, remaining, sizeof(DWORD)*3);

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
        ZS_LOG_TRACE(debug("malformed statistics summary report block") + ZS_PARAM("remaining", remaining))
        return false;
      }

      advancePos(pos, remaining, sizeof(DWORD)*2);

      reportBlock->mLostPackets = UseHelper::getBE32(&(pos[0]));
      reportBlock->mDupPackets = UseHelper::getBE32(&(pos[4]));
      reportBlock->mMinJitter = UseHelper::getBE32(&(pos[8]));
      reportBlock->mMaxJitter = UseHelper::getBE32(&(pos[12]));
      reportBlock->mMeanJitter = UseHelper::getBE32(&(pos[16]));
      reportBlock->mDevJitter = UseHelper::getBE32(&(pos[20]));

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
        ZS_LOG_TRACE(debug("malformed voip metrics report block") + ZS_PARAM("remaining", remaining))
        return false;
      }

      reportBlock->mSSRCOfSource = UseHelper::getBE32(&(pos[0]));
      reportBlock->mLossRate = pos[4];
      reportBlock->mDiscardRate = pos[5];
      reportBlock->mBurstDensity = pos[6];
      reportBlock->mGapDensity = pos[7];
      reportBlock->mBurstDuration = UseHelper::getBE16(&(pos[8]));
      reportBlock->mGapDuration = UseHelper::getBE16(&(pos[10]));
      reportBlock->mRoundTripDelay = UseHelper::getBE16(&(pos[12]));
      reportBlock->mEndSystemDelay = UseHelper::getBE16(&(pos[14]));
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
      reportBlock->mJBNominal = UseHelper::getBE16(&(pos[26]));
      reportBlock->mJBMaximum = UseHelper::getBE16(&(pos[28]));
      reportBlock->mJBAbsMax = UseHelper::getBE16(&(pos[30]));

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
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCPPacket (allocation sizing routines)
    #pragma mark

    //-------------------------------------------------------------------------
    void *RTCPPacket::allocateBuffer(size_t size)
    {
      return internal::allocateBuffer(mAllocationPos, mAllocationSize, size);
    }

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

      for (Chunk *chunk = report->firstChunk(); NULL != chunk; ++chunk, ++chunkCount)
      {
        size_t chunkSize = sizeof(DWORD);

        for (auto *item = chunk->firstCName(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
        }

        for (auto *item = chunk->firstName(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
        }

        for (auto *item = chunk->firstEmail(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
        }

        for (auto *item = chunk->firstPhone(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
        }

        for (auto *item = chunk->firstLoc(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
        }

        for (auto *item = chunk->firstTool(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
        }

        for (auto *item = chunk->firstNote(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
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
        }

        for (auto *item = chunk->firstMid(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
        }

        for (auto *item = chunk->firstUnknown(); NULL != item; item = item->next())
        {
          chunkSize += (sizeof(BYTE)*2);
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          throwIfGreaterThanBitsAllow(len, 8);
          chunkSize += ((sizeof(BYTE))*len);
        }

        if (chunkSize == sizeof(DWORD)) {
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
      typedef RTCPPacket::TransportLayerFeedbackMessage TransportLayerFeedbackMessage;
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
          throwIfLessThan(count, 1);
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

      return boundarySize(result);
    }

    static size_t getPacketSizePayloadSpecificFeedbackMessage(const RTCPPacket::PayloadSpecificFeedbackMessage *fm)
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
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::REMB REMB;

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
          result += ((sizeof(DWORD)*2)*(count));
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

            size_t size = (sizeof(DWORD)*2) + vbcm->vbcmOctetStringSize();
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

      return boundarySize(result);
    }

    //-------------------------------------------------------------------------
    static size_t getPacketSizeXR(const RTCPPacket::XR *report)
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

      size_t result = (sizeof(DWORD)*2);

      for (const ReportBlock *reportBlock = report->firstReportBlock(); NULL != reportBlock; reportBlock = reportBlock->next())
      {
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
      }

      return boundarySize(result);
    }

    //-------------------------------------------------------------------------
    size_t RTCPPacket::getPacketSize(const Report *first)
    {
      size_t result = 0;

      const Report *final = NULL;

      for (const Report *report = first; NULL != report; report = report->next())
      {
        final = report;

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
            result += sizeof(DWORD) +  boundarySize(report->size());
            break;
          }
        }
      }

      if (NULL != final) {
        auto padding = final->padding();
        if (0 != padding) {
          result += padding;
        }
      }

      return boundarySize(result);
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
      ASSERT(remaining > sizeof(DWORD))
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
      typedef RTCPPacket::SenderReceiverCommonReport SenderReceiverCommonReport;
      typedef RTCPPacket::SenderReceiverCommonReport::ReportBlock ReportBlock;

      size_t count = 0;
      for (const ReportBlock *block = report->firstReportBlock(); NULL != block; block = block->next(), ++count)
      {
        ASSERT(count < report->rc())

        UseHelper::setBE32(&(pos[0]), block->ssrc());
        ASSERT(throwIfGreaterThanBitsAllow(block->cumulativeNumberOfPacketsLost(), 24))
        UseHelper::setBE32(&(pos[4]), block->cumulativeNumberOfPacketsLost());
        pos[5] = block->fractionLost();
        UseHelper::setBE32(&(pos[8]), block->extendedHighestSequenceNumberReceived());
        UseHelper::setBE32(&(pos[12]), block->interarrivalJitter());
        UseHelper::setBE32(&(pos[16]), block->lsr());
        UseHelper::setBE32(&(pos[20]), block->dlsr());

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

      UseHelper::setBE32(&(pos[4]), report->ssrcOfSender());
      UseHelper::setBE32(&(pos[8]), report->ntpTimestampMS());
      UseHelper::setBE32(&(pos[12]), report->ntpTimestampLS());
      UseHelper::setBE32(&(pos[16]), report->rtpTimestamp());
      UseHelper::setBE32(&(pos[20]), report->senderPacketCount());
      UseHelper::setBE32(&(pos[24]), report->senderOctetCount());

      advancePos(pos, remaining, sizeof(DWORD)*7);

      writePacketSenderReceiverCommonReport(report, pos, remaining);
    }

    //-------------------------------------------------------------------------
    static void writePacketReceiverReport(const RTCPPacket::ReceiverReport *report, BYTE * &pos, size_t &remaining)
    {
      typedef RTCPPacket::ReceiverReport ReceiverReport;
      pos[1] = ReceiverReport::kPayloadType;

      UseHelper::setBE32(&(pos[4]), report->ssrcOfPacketSender());

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

      for (Chunk *chunk = report->firstChunk(); NULL != chunk; ++chunk, ++chunkCount)
      {
        UseHelper::setBE32(pos, chunk->ssrc());
        advancePos(pos, remaining, sizeof(DWORD));

        BYTE *startPos = pos;

        for (auto *item = chunk->firstCName(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::CName::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = len;
          ASSERT(throwIfGreaterThanBitsAllow(len, 8))

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
        }

        for (auto *item = chunk->firstName(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Name::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = len;
          ASSERT(throwIfGreaterThanBitsAllow(len, 8))

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
        }

        for (auto *item = chunk->firstEmail(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Email::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = len;
          ASSERT(throwIfGreaterThanBitsAllow(len, 8))

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
        }

        for (auto *item = chunk->firstPhone(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Phone::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = len;
          ASSERT(throwIfGreaterThanBitsAllow(len, 8))

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
        }

        for (auto *item = chunk->firstLoc(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Loc::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = len;
          ASSERT(throwIfGreaterThanBitsAllow(len, 8))

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
        }

        for (auto *item = chunk->firstTool(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Tool::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = len;
          ASSERT(throwIfGreaterThanBitsAllow(len, 8))

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
        }

        for (auto *item = chunk->firstNote(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Note::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = len;
          ASSERT(throwIfGreaterThanBitsAllow(len, 8))

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
        }

        for (auto *item = chunk->firstPriv(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Priv::kItemType;
          size_t len1 = (NULL != item->mPrefix ? strlen(item->mPrefix) : 0);
          size_t len2 = (NULL != item->mValue ? strlen(item->mValue) : 0);
          size_t len = len1 + len2;
          if (len > 0) ++len;
          pos[1] = len;
          ASSERT(throwIfGreaterThanBitsAllow(len, 8))

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            pos[0] = len1;
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
        }

        for (auto *item = chunk->firstMid(); NULL != item; item = item->next())
        {
          pos[0] = Chunk::Mid::kItemType;
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = len;
          ASSERT(throwIfGreaterThanBitsAllow(len, 8))

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
        }

        for (auto *item = chunk->firstUnknown(); NULL != item; item = item->next())
        {
          pos[0] = item->type();
          size_t len = (NULL != item->mValue ? strlen(item->mValue) : 0);
          pos[1] = len;
          ASSERT(throwIfGreaterThanBitsAllow(len, 8))

          advancePos(pos, remaining, sizeof(WORD));

          if (len > 0) {
            memcpy(pos, item->value(), len*sizeof(BYTE));
            advancePos(pos, remaining, len*sizeof(BYTE));
          }
        }

        BYTE *endPos = pos;

        PTRNUMBER diff = reinterpret_cast<PTRNUMBER>(endPos) - reinterpret_cast<PTRNUMBER>(startPos);
        auto modulas = (diff % sizeof(DWORD));
        if (0 != modulas) {
          advancePos(pos, remaining, sizeof(DWORD)-modulas);
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
        UseHelper::setBE32(pos, report->ssrc(index));
        advancePos(pos, remaining, sizeof(DWORD));
      }

      if (NULL != report->reasonForLeaving()) {
        size_t len = strlen(report->reasonForLeaving());
        if (len > 0) {
          pos[0] = len;
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

      UseHelper::setBE32(&(pos[4]), report->ssrc());
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

      UseHelper::setBE32(&(pos[4]), report->ssrcOfPacketSender());
      UseHelper::setBE32(&(pos[8]), report->ssrcOfMediaSource());

      advancePos(pos, remaining, sizeof(DWORD)*3);

      switch (report->fmt()) {
        case GenericNACK::kFmt:
        {
          auto count = report->genericNACKCount();

          for (size_t index = 0; index < count; ++index)
          {
            auto item = report->genericNACKAtIndex(index);
            pos[0] = item->pid();
            pos[1] = item->blp();
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
            UseHelper::setBE32(&(pos[0]), item->ssrc());

            DWORD merged = RTCP_PACK_BITS(static_cast<DWORD>(item->mxTBRExp()), 0x3F, 26) |
                           RTCP_PACK_BITS(static_cast<DWORD>(item->mxTBRMantissa()), 0x1FFFF, 9) |
                           RTCP_PACK_BITS(static_cast<DWORD>(item->measuredOverhead()), 0x1FF, 0);

            UseHelper::setBE32(&(pos[4]), merged);

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
            UseHelper::setBE32(&(pos[0]), item->ssrc());

            DWORD merged = RTCP_PACK_BITS(static_cast<DWORD>(item->mxTBRExp()), 0x3F, 26) |
                           RTCP_PACK_BITS(static_cast<DWORD>(item->mxTBRMantissa()), 0x1FFFF, 9) |
                           RTCP_PACK_BITS(static_cast<DWORD>(item->measuredOverhead()), 0x1FF, 0);

            UseHelper::setBE32(&(pos[4]), merged);

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
      typedef RTCPPacket::PayloadSpecificFeedbackMessage::REMB REMB;

      pos[1] = PayloadSpecificFeedbackMessage::kPayloadType;

      UseHelper::setBE32(&(pos[4]), report->ssrcOfPacketSender());
      UseHelper::setBE32(&(pos[8]), report->ssrcOfMediaSource());

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

            UseHelper::setBE32(&(pos[0]), merged);
            advancePos(pos, remaining, sizeof(DWORD));
          }
          break;
        }
        case RPSI::kFmt:
        {
          auto rpsi = report->rpsi();

          pos[0] = rpsi->pb();
          pos[1] = RTCP_PACK_BITS(static_cast<BYTE>(rpsi->zeroBit()), 0x1, 7) |
                   RTCP_PACK_BITS(static_cast<BYTE>(rpsi->payloadType()), 0x7F, 0);

          auto size = rpsi->nativeRPSIBitStringSizeInBits();
          auto modulas = static_cast<BYTE>(size%8);
          size = size/8;
          if (0 != modulas) ++size;
          if (0 != size) {
            memcpy(&(pos[2]), rpsi->nativeRPSIBitString(), size);
            if (0 != modulas) {
              // fill the extra bits at the end of the bit stream with 0s
              BYTE &by = (pos[2+size-1]);
              by = by & ((1 << (8-modulas))-1);
            }
          }

          advancePos(pos, remaining, sizeof(WORD)+(sizeof(BYTE)*size));
          break;
        }
        case FIR::kFmt:
        {
          auto count = report->firCount();

          for (size_t index = 0; index < count; ++index)
          {
            auto item = report->firAtIndex(index);
            UseHelper::setBE32(&(pos[0]), item->ssrc());
            UseHelper::setBE32(&(pos[4]), item->reserved());
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
            UseHelper::setBE32(&(pos[0]), item->ssrc());

            DWORD merged = RTCP_PACK_BITS(static_cast<DWORD>(item->seqNr()), 0xFF, 24) |
                           RTCP_PACK_BITS(static_cast<DWORD>(item->reserved()), 0x7FFFF, 5) |
                           RTCP_PACK_BITS(static_cast<DWORD>(item->index()), 0x1F, 0);

            UseHelper::setBE32(&(pos[4]), merged);
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
            UseHelper::setBE32(&(pos[0]), item->ssrc());

            DWORD merged = RTCP_PACK_BITS(static_cast<DWORD>(item->seqNr()), 0xFF, 24) |
                           RTCP_PACK_BITS(static_cast<DWORD>(item->reserved()), 0x7FFFF, 5) |
                           RTCP_PACK_BITS(static_cast<DWORD>(item->index()), 0x1F, 0);

            UseHelper::setBE32(&(pos[4]), merged);
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

            pos[0] = item->seqNr();
            pos[1] = RTCP_PACK_BITS(static_cast<BYTE>(item->zeroBit()), 0x1, 7) |
                     RTCP_PACK_BITS(static_cast<BYTE>(item->payloadType()), 0x7F, 0);

            auto size = item->vbcmOctetStringSize();
            if (0 != size) {
              memcpy(&(pos[2]), item->vbcmOctetString(), size*sizeof(BYTE));
            }

            advancePos(pos, remaining, boundarySize((sizeof(DWORD)*2)+sizeof(WORD)+(size*sizeof(BYTE))));
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
            UseHelper::setBE32(&(pos[4]), merged);
            pos[4] = remb->numSSRC();
            advancePos(pos, remaining, sizeof(DWORD)*2);

            auto count = remb->numSSRC();
            for (size_t index = 0; index < count; ++index)
            {
              auto ssrc = remb->ssrcAtIndex(index);
              UseHelper::setBE32(pos, ssrc);
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

      advancePos(pos, remaining, sizeof(DWORD));

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

            UseHelper::setBE32(&(pos[0]), block->ssrcOfSource());
            UseHelper::setBE16(&(pos[4]), block->beginSeq());
            UseHelper::setBE16(&(pos[6]), block->endSeq());

            advancePos(pos, remaining, sizeof(DWORD)*2);

            size_t chunkCount = block->chunkCount();
            for (size_t index = 0; index < chunkCount; ++index)
            {
              RLEChunk chunk = block->chunkAtIndex(index);
              UseHelper::setBE16(pos, chunk);
              advancePos(pos, remaining, sizeof(WORD));
            }
            break;
          }
          case DuplicateRLEReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const DuplicateRLEReportBlock *>(reportBlock);
            UseHelper::setBE32(&(pos[0]), block->ssrcOfSource());
            UseHelper::setBE16(&(pos[4]), block->beginSeq());
            UseHelper::setBE16(&(pos[6]), block->endSeq());

            advancePos(pos, remaining, sizeof(DWORD)*2);

            size_t count = block->chunkCount();
            for (size_t index = 0; index < count; ++index)
            {
              RLEChunk chunk = block->chunkAtIndex(index);
              UseHelper::setBE16(pos, chunk);
              advancePos(pos, remaining, sizeof(WORD));
            }
            break;
          }
          case PacketReceiptTimesReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const PacketReceiptTimesReportBlock *>(reportBlock);
            UseHelper::setBE32(&(pos[0]), block->ssrcOfSource());
            UseHelper::setBE16(&(pos[4]), block->beginSeq());
            UseHelper::setBE16(&(pos[6]), block->endSeq());

            advancePos(pos, remaining, sizeof(DWORD)*2);

            size_t count = block->receiptTimeCount();
            for (size_t index = 0; index < count; ++index)
            {
              DWORD receiptTime = block->receiptTimeAtIndex(index);
              UseHelper::setBE32(pos, receiptTime);
              advancePos(pos, remaining, sizeof(DWORD));
            }
            break;
          }
          case ReceiverReferenceTimeReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const ReceiverReferenceTimeReportBlock *>(reportBlock);
            UseHelper::setBE32(&(pos[0]), block->ntpTimestampMS());
            UseHelper::setBE32(&(pos[4]), block->ntpTimestampLS());
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
              UseHelper::setBE32(&(pos[0]), subBlock->ssrc());
              UseHelper::setBE32(&(pos[4]), subBlock->lrr());
              UseHelper::setBE32(&(pos[8]), subBlock->dlrr());
              advancePos(pos, remaining, sizeof(DWORD)*3);
            }
            break;
          }
          case StatisticsSummaryReportBlock::kBlockType:
          {
            auto block = reinterpret_cast<const StatisticsSummaryReportBlock *>(reportBlock);
            UseHelper::setBE32(&(pos[0]), block->ssrcOfSource());
            UseHelper::setBE16(&(pos[4]), block->beginSeq());
            UseHelper::setBE16(&(pos[6]), block->endSeq());
            UseHelper::setBE32(&(pos[8]), block->lostPackets());
            UseHelper::setBE32(&(pos[12]), block->dupPackets());
            UseHelper::setBE32(&(pos[16]), block->minJitter());
            UseHelper::setBE32(&(pos[20]), block->maxJitter());
            UseHelper::setBE32(&(pos[24]), block->meanJitter());
            UseHelper::setBE32(&(pos[28]), block->devJitter());
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
            UseHelper::setBE32(&(pos[0]), block->ssrcOfSource());
            pos[4] = block->lossRate();
            pos[5] = block->discardRate();
            pos[6] = block->burstDensity();
            pos[7] = block->gapDensity();
            UseHelper::setBE16(&(pos[8]), block->burstDuration());
            UseHelper::setBE16(&(pos[10]), block->gapDuration());
            UseHelper::setBE16(&(pos[12]), block->roundTripDelay());
            UseHelper::setBE16(&(pos[14]), block->endSystemDelay());
            pos[16] = block->signalLevel();
            pos[17] = block->noiseLevel();
            pos[18] = block->rerl();
            pos[19] = block->Gmin();
            pos[20] = block->rxConfig();
            pos[21] = block->mReservedVoIP;
            UseHelper::setBE16(&(pos[22]), block->jbNominal());
            UseHelper::setBE16(&(pos[24]), block->jbMaximum());
            UseHelper::setBE16(&(pos[26]), block->jbAbsMax());
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

        UseHelper::setBE16(&(blockStart[2]), (boundarySize(diff)/sizeof(DWORD))-1);
      }
    }

    //-------------------------------------------------------------------------
    static void writePacketUnknown(const RTCPPacket::UnknownReport *report, BYTE * &pos, size_t &remaining)
    {
      typedef RTCPPacket::UnknownReport UnknownReport;
      pos[1] = report->pt();

      advancePos(pos, remaining, sizeof(DWORD));

      auto size = report->size();
      if (0 == size) return;

      ASSERT(NULL != report->ptr())

      memcpy(pos, report->ptr(), size);

      advancePos(pos, remaining, size);
    }

    //-------------------------------------------------------------------------
    void RTCPPacket::writePacket(const Report *first, BYTE * &pos, size_t &remaining)
    {
      ASSERT(sizeof(char) == sizeof(BYTE))
      ASSERT(NULL != first)

      const Report *final = NULL;

      for (const Report *report = first; NULL != report; report = report->next())
      {
        final = report;

        BYTE *startOfReport = pos;

        writePacketHeader(report, pos, remaining);

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
          advancePos(pos, remaining, sizeof(DWORD)-modulas);
          diff += (sizeof(DWORD)-modulas);
        }

        UseHelper::setBE16(&(pos[2]), (diff/sizeof(DWORD))-1);
      }

      if (NULL != final) {
        auto padding = final->padding();
        if (0 != padding) {
          if (padding > 1) {
            advancePos(pos, remaining, padding*sizeof(BYTE));
          }
          pos[0] = padding;
          advancePos(pos, remaining);
        }
      }

      ASSERT(0 == remaining)
    }

  }

}
