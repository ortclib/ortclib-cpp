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

#pragma once

#include <ortc/types.h>
#include <ortc/IMediaStreamTrack.h>
#include <ortc/IICETypes.h>

namespace ortc
{
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //
  // RTCPPacket
  //

  class RTCPPacket
  {
  protected:
    struct make_private {};

  public:
    typedef IMediaStreamTrackTypes::MediaChannelID MediaChannelID;

  public:
    //-----------------------------------------------------------------------
    //
    // RTCPPacket::Report
    //

    struct Report
    {
      Report *next() const noexcept           {return mNext;}

      const BYTE *ptr() const noexcept        {return mPtr;}
      size_t size() const noexcept            {return mSize;}

      BYTE version() const noexcept           {return mVersion;}
      size_t padding() const noexcept         {return static_cast<size_t>(mPadding);}
      BYTE reportSpecific() const noexcept    {return mReportSpecific;}

      BYTE pt() const noexcept                {return mPT;}
      static const char *ptToString(BYTE pt) noexcept;
      const char *ptToString() const noexcept {return ptToString(mPT);}

    public:
      Report *mNext {};

      const BYTE *mPtr {};
      size_t mSize {};

      BYTE mVersion {};
      BYTE mPadding {};
      BYTE mReportSpecific {};

      BYTE mPT {};
    };

    //-----------------------------------------------------------------------
    //
    // RTCPPacket::SenderReceiverCommonReport
    //

    struct SenderReceiverCommonReport : public Report
    {
      //---------------------------------------------------------------------
      //
      // RTCPPacket::SenderReceiverCommonReport::ReportBlock
      //

      struct ReportBlock
      {
        ReportBlock *next() const noexcept                            {return mNext;}

        DWORD ssrc() const noexcept                                   {return mSSRC;}
        BYTE fractionLost() const noexcept                            {return mFractionLost;}
        DWORD cumulativeNumberOfPacketsLost() const noexcept          {return mCumulativeNumberOfPacketsLost;}
        DWORD extendedHighestSequenceNumberReceived() const noexcept  {return mExtendedHighestSequenceNumberReceived;}
        DWORD interarrivalJitter() const noexcept                     {return mInterarrivalJitter;}
        DWORD lsr() const noexcept                                    {return mLSR;}
        DWORD dlsr() const noexcept                                   {return mDLSR;}

      public:
        ReportBlock *mNext {};

        DWORD mSSRC {};
        BYTE mFractionLost {};
        DWORD mCumulativeNumberOfPacketsLost {};
        DWORD mExtendedHighestSequenceNumberReceived {};
        DWORD mInterarrivalJitter {};
        DWORD mLSR {};  // last SR timestamp
        DWORD mDLSR {}; // delay since last SR
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::SenderReceiverCommonReport (public)
      //

      size_t rc() const noexcept                      {return static_cast<size_t>(mReportSpecific);}
      DWORD ssrcOfSender() const noexcept             {return mSSRCOfSender;}

      ReportBlock *firstReportBlock() const noexcept  {return mFirstReportBlock;}

      const BYTE *extension() const noexcept          {return mExtension;}
      size_t extensionSize() const noexcept           {return mExtensionSize;}

    public:
      DWORD mSSRCOfSender {};

      const BYTE *mExtension {};
      size_t mExtensionSize {};

      ReportBlock *mFirstReportBlock {};
    };

    //-----------------------------------------------------------------------
    //
    // RTCPPacket::SenderReport
    //

    struct SenderReport : public SenderReceiverCommonReport
    {
      // https://tools.ietf.org/html/rfc3550#section-6.4.1

      static const BYTE kPayloadType {200};

      SenderReport *nextSenderReport() const noexcept {return mNextSenderReport;}

      DWORD ntpTimestampMS() const noexcept           {return mNTPTimestampMS;}
      DWORD ntpTimestampLS() const noexcept           {return mNTPTimestampLS;}
      Time ntpTimestamp() const noexcept;
      DWORD rtpTimestamp() const noexcept             {return mRTPTimestamp;}
      DWORD senderPacketCount() const noexcept        {return mSenderPacketCount;}
      DWORD senderOctetCount() const noexcept         {return mSenderOctetCount;}

    public:
      SenderReport *mNextSenderReport {};

      DWORD mNTPTimestampMS {};
      DWORD mNTPTimestampLS {};
      DWORD mRTPTimestamp {};
      DWORD mSenderPacketCount {};
      DWORD mSenderOctetCount {};
    };

    //-----------------------------------------------------------------------
    //
    // RTCPPacket::ReceiverReport
    //

    struct ReceiverReport : public SenderReceiverCommonReport
    {
      // https://tools.ietf.org/html/rfc3550#section-6.4.2

      static const BYTE kPayloadType {201};

      ReceiverReport *nextReceiverReport() const noexcept {return mNextReceiverReport;}

      DWORD ssrcOfPacketSender() const noexcept           {return mSSRCOfSender;}

    public:
      ReceiverReport *mNextReceiverReport {};
    };

    //-----------------------------------------------------------------------
    //
    // RTCPPacket::SDES
    //

    struct SDES : public Report
    {
      // https://tools.ietf.org/html/rfc3550#section-6.5

      //---------------------------------------------------------------------
      //
      // RTCPPacket::SDES::Chunk
      //

      struct Chunk
      {
        static const BYTE kEndOfItemsType = 0;

        struct StringItem
        {
          BYTE type() const noexcept                {return mType;}
          size_t length() const noexcept            {return mLength;}
          const char *value() const noexcept        {return mValue;}

          static const char *typeToString(BYTE type) noexcept;
          const char *typeToString() const noexcept {return typeToString(mType);}

        public:
          StringItem *mNext {};

          BYTE mType {};
          size_t mLength {};
          const char *mValue {};
        };

        //-------------------------------------------------------------------
        //
        // RTCPPacket::SDES::Chunk::CName
        //

        struct CName : public StringItem
        {
          // https://tools.ietf.org/html/rfc3550#section-6.5.1

          static const BYTE kItemType = 1;

          CName *next() const noexcept                              {return static_cast<CName *>(mNext);}

          const char *userAndDomainName() const noexcept            {return value();}
        };

        //-------------------------------------------------------------------
        //
        // RTCPPacket::SDES::Chunk::Name
        //

        struct Name : public StringItem
        {
          // https://tools.ietf.org/html/rfc3550#section-6.5.2

          static const BYTE kItemType = 2;

          Name *next() const noexcept                               {return static_cast<Name *>(mNext);}

          const char *commonName() const noexcept                   {return value();}
        };

        //-------------------------------------------------------------------
        //
        // RTCPPacket::SDES::Chunk::Email
        //

        struct Email : public StringItem
        {
          // https://tools.ietf.org/html/rfc3550#section-6.5.3

          static const BYTE kItemType = 3;

          Email *next() const noexcept                              {return static_cast<Email *>(mNext);}

          const char *emailAddress() const noexcept                 {return value();}
        };

        //-------------------------------------------------------------------
        //
        // RTCPPacket::SDES::Chunk::Phone
        //

        struct Phone : public StringItem
        {
          // https://tools.ietf.org/html/rfc3550#section-6.5.4

          static const BYTE kItemType = 4;

          Phone *next() const noexcept                              {return static_cast<Phone *>(mNext);}

          const char *phoneNumber() const noexcept                  {return value();}
        };

        //-------------------------------------------------------------------
        //
        // RTCPPacket::SDES::Chunk::Loc
        //

        struct Loc : public StringItem
        {
          // https://tools.ietf.org/html/rfc3550#section-6.5.5

          static const BYTE kItemType = 5;

          Loc *next() const noexcept                                {return static_cast<Loc *>(mNext);}

          const char *geographicLocation() const noexcept           {return value();}
        };

        //-------------------------------------------------------------------
        //
        // RTCPPacket::SDES::Chunk::Tool
        //

        struct Tool : public StringItem
        {
          // https://tools.ietf.org/html/rfc3550#section-6.5.6

          static const BYTE kItemType = 6;

          Tool *next() const noexcept                               {return static_cast<Tool *>(mNext);}

          const char *nameVersion() const noexcept                  {return value();}
        };

        //-------------------------------------------------------------------
        //
        // RTCPPacket::SDES::Chunk::Note
        //

        struct Note : public StringItem
        {
          // https://tools.ietf.org/html/rfc3550#section-6.5.7

          static const BYTE kItemType = 7;

          Note *next() const noexcept                               {return static_cast<Note *>(mNext);}

          const char *note() const noexcept                         {return value();}
        };

        //-------------------------------------------------------------------
        //
        // RTCPPacket::SDES::Chunk::Priv
        //

        struct Priv : public StringItem
        {
          // https://tools.ietf.org/html/rfc3550#section-6.5.8

          static const BYTE kItemType = 8;

          Priv *next() const noexcept                               {return static_cast<Priv *>(mNext);}

          size_t prefixLength() const noexcept                      {return mPrefixLength;}
          const char *prefix() const noexcept                       {return mPrefix;}

        public:
          size_t mPrefixLength {};
          const char *mPrefix {};
        };

        //-------------------------------------------------------------------
        //
        // RTCPPacket::SDES::Chunk::Unknown
        //

        struct Unknown : public StringItem
        {
          Unknown *next() const noexcept                            {return static_cast<Unknown *>(mNext);}

          const BYTE *ptr() const noexcept                          {return reinterpret_cast<const BYTE *>(value());}
          size_t size() const noexcept                              {return length();}
        };

        //-------------------------------------------------------------------
        //
        // RTCPPacket::SDES::Chunk::Mid
        //

        struct Mid : public StringItem
        {
          // https://tools.ietf.org/html/draft-ietf-mmusic-sdp-bundle-negotiation-23#section-14.2
          static const BYTE kItemType = 12;  // TBD

          Mid *next() const noexcept                               {return static_cast<Mid *>(mNext);}

          const char *mid() const noexcept                         {return value();}
        };

        //-------------------------------------------------------------------
        //
        // RTCPPacket::SDES::Chunk::Rid
        //

        struct Rid : public StringItem
        {
          // https://tools.ietf.org/html/draft-roach-avtext-rid-02#section-4.1
          static const BYTE kItemType = 13;  // TBD

          Rid *next() const noexcept                               {return static_cast<Rid *>(mNext);}

          const char *rid() const noexcept                         {return value();}
        };

        //-------------------------------------------------------------------
        //
        // RTCPPacket::SDES::Chunk
        //

        Chunk *next() const noexcept                                {return mNext;}

        DWORD ssrc() const noexcept                                 {return mSSRC;}

        size_t count() const noexcept                               {return mCount;}

        CName *firstCName() const noexcept                          {return mFirstCName;}
        Name *firstName() const noexcept                            {return mFirstName;}
        Email *firstEmail() const noexcept                          {return mFirstEmail;}
        Phone *firstPhone() const noexcept                          {return mFirstPhone;}
        Loc *firstLoc() const noexcept                              {return mFirstLoc;}
        Tool *firstTool() const noexcept                            {return mFirstTool;}
        Note *firstNote() const noexcept                            {return mFirstNote;}
        Priv *firstPriv() const noexcept                            {return mFirstPriv;}
        Mid *firstMid() const noexcept                              {return mFirstMid;}
        Rid *firstRid() const noexcept                              {return mFirstRid;}
        Unknown *firstUnknown() const noexcept                      {return mFirstUnknown;}

        size_t cNameCount() const noexcept                          {return mCNameCount;}
        size_t nameCount() const noexcept                           {return mNameCount;}
        size_t emailCount() const noexcept                          {return mEmailCount;}
        size_t phoneCount() const noexcept                          {return mPhoneCount;}
        size_t locCount() const noexcept                            {return mLocCount;}
        size_t toolCount() const noexcept                           {return mToolCount;}
        size_t noteCount() const noexcept                           {return mNoteCount;}
        size_t privCount() const noexcept                           {return mPrivCount;}
        size_t midCount() const noexcept                            {return mMidCount;}
        size_t ridCount() const noexcept                            {return mRidCount;}
        size_t unknownCount() const noexcept                        {return mUnknownCount;}

        CName *cNameAtIndex(size_t index) const noexcept;
        Name *nameAtIndex(size_t index) const noexcept;
        Email *emailAtIndex(size_t index) const noexcept;
        Phone *phoneAtIndex(size_t index) const noexcept;
        Loc *locAtIndex(size_t index) const noexcept;
        Tool *toolAtIndex(size_t index) const noexcept;
        Note *noteAtIndex(size_t index) const noexcept;
        Priv *privAtIndex(size_t index) const noexcept;
        Mid *midAtIndex(size_t index) const noexcept;
        Rid *ridAtIndex(size_t index) const noexcept;
        Unknown *unknownAtIndex(size_t index) const noexcept;

      public:
        Chunk *mNext {};

        DWORD mSSRC {};

        size_t mCount {};

        size_t mCNameCount {};
        size_t mNameCount {};
        size_t mEmailCount {};
        size_t mPhoneCount {};
        size_t mLocCount {};
        size_t mToolCount {};
        size_t mNoteCount {};
        size_t mPrivCount {};
        size_t mMidCount {};
        size_t mRidCount {};
        size_t mUnknownCount {};

        CName *mFirstCName {};
        Name *mFirstName {};
        Email *mFirstEmail {};
        Phone *mFirstPhone {};
        Loc *mFirstLoc {};
        Tool *mFirstTool {};
        Note *mFirstNote {};
        Priv *mFirstPriv {};
        Mid *mFirstMid {};
        Rid *mFirstRid {};
        Unknown *mFirstUnknown {};
      };

      static const BYTE kPayloadType {202};

      SDES *nextSDES() const noexcept                 {return mNextSDES;}

      size_t sc() const noexcept                      {return static_cast<size_t>(mReportSpecific);}

      Chunk *firstChunk() const noexcept              {return mFirstChunk;}

    public:
      SDES *mNextSDES {};

      Chunk *mFirstChunk {};
    };

    //-----------------------------------------------------------------------
    //
    // RTCPPacket::Bye
    //

    struct Bye : public Report
    {
      // https://tools.ietf.org/html/rfc3550#section-6.6

      static const BYTE kPayloadType {203};

      Bye *nextBye() const noexcept                   {return mNextBye;}

      size_t sc() const noexcept                      {return static_cast<size_t>(mReportSpecific);}

      DWORD ssrc(size_t index) const noexcept;

      const char *reasonForLeaving() const noexcept   {return mReasonForLeaving;}

    public:
      Bye *mNextBye {};

      const char *mReasonForLeaving {};

      DWORD *mSSRCs {};
    };

    //-----------------------------------------------------------------------
    //
    // RTCPPacket::App
    //

    struct App : public Report
    {
      // https://tools.ietf.org/html/rfc3550#section-6.7

      static const BYTE kPayloadType {204};

      App *nextApp() const noexcept                   {return mNextApp;}

      BYTE subtype() const noexcept                   {return mReportSpecific;}

      DWORD ssrc() const noexcept                     {return mSSRC;}
      const char *name() const noexcept               {return &(mName[0]);}

      const BYTE *data() const noexcept               {return mData;}
      size_t dataSize() const noexcept                {return mDataSize;}

    public:
      App *mNextApp {};

      DWORD mSSRC {};
      char mName[sizeof(DWORD)+1] {};

      const BYTE *mData {};
      size_t mDataSize {};
    };

    //-----------------------------------------------------------------------
    //
    // RTCPPacket::FeedbackMessage
    //

    struct FeedbackMessage : public Report
    {
      // https://tools.ietf.org/html/rfc4585#section-6.1

      BYTE fmt() const noexcept                                       {return mReportSpecific;}

      DWORD ssrcOfPacketSender() const noexcept                       {return mSSRCOfPacketSender;}
      DWORD ssrcOfMediaSource() const noexcept                        {return mSSRCOfMediaSource;}

      const BYTE *fci() const noexcept                                {return mFCI;}
      size_t fciSize() const noexcept                                 {return mFCISize;}

      static const char *fmtToString(BYTE pt, BYTE fmt, DWORD subFmt = 0) noexcept;
      const char *fmtToString() const noexcept;

    public:
      DWORD mSSRCOfPacketSender {};
      DWORD mSSRCOfMediaSource {};

      const BYTE *mFCI {};
      size_t mFCISize {};
    };

    //-----------------------------------------------------------------------
    //
    // RTCPPacket::TransportLayerFeedbackMessage
    //

    struct TransportLayerFeedbackMessage : public FeedbackMessage
    {
      // https://tools.ietf.org/html/rfc4585#section-6.2

      //---------------------------------------------------------------------
      //
      // RTCPPacket::TransportLayerFeedbackMessage::GenericNACK
      //

      struct GenericNACK
      {
        // https://tools.ietf.org/html/rfc4585#section-6.2.1

        static const BYTE kFmt {1};

        WORD pid() const noexcept                                     {return mPID;}
        WORD blp() const noexcept                                     {return mBLP;}

      public:
        WORD mPID {};
        WORD mBLP {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::TransportLayerFeedbackMessage::TMMBRCommon
      //

      struct TMMBRCommon
      {
        DWORD ssrc() const noexcept                                   {return mSSRC;}

        BYTE mxTBRExp() const noexcept                                {return mMxTBRExp;}
        DWORD mxTBRMantissa() const noexcept                          {return mMxTBRMantissa;}
        WORD measuredOverhead() const noexcept                        {return mMeasuredOverhead;}

      public:
        DWORD mSSRC {};

        BYTE mMxTBRExp {};
        DWORD mMxTBRMantissa {};
        WORD mMeasuredOverhead {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::TransportLayerFeedbackMessage::TMMBR
      //

      struct TMMBR : public TMMBRCommon
      {
        // http://tools.ietf.org/html/rfc5104#section-4.2.1.1

        static const BYTE kFmt {3};

      public:
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::TransportLayerFeedbackMessage::TMMBN
      //

      struct TMMBN : public TMMBRCommon
      {
        // http://tools.ietf.org/html/rfc5104#section-4.2.2.1

        static const BYTE kFmt {4};

      public:
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::TransportLayerFeedbackMessage (public)
      //

      static const BYTE kPayloadType {205};

      TransportLayerFeedbackMessage *nextTransportLayerFeedbackMessage() const  {return mNextTransportLayerFeedbackMessage;}

      size_t genericNACKCount() const noexcept                        {return mGenericNACKCount;}
      GenericNACK *genericNACKAtIndex(size_t index) const noexcept;

      size_t tmmbrCount() const noexcept                              {return mTMMBRCount;}
      TMMBR *tmmbrAtIndex(size_t index) const noexcept;

      size_t tmmbnCount() const noexcept                              {return mTMMBNCount;}
      TMMBN *tmmbnAtIndex(size_t index) const noexcept;

      TransportLayerFeedbackMessage *unknown() const noexcept         {return mUnknown;}

    public:
      TransportLayerFeedbackMessage *mNextTransportLayerFeedbackMessage {};

      size_t mGenericNACKCount {};
      GenericNACK *mFirstGenericNACK {};

      size_t mTMMBRCount {};
      TMMBR *mFirstTMMBR {};

      size_t mTMMBNCount {};
      TMMBN *mFirstTMMBN {};

      TransportLayerFeedbackMessage *mUnknown {};
    };

    //-----------------------------------------------------------------------
    //
    // RTCPPacket::PayloadSpecificFeedbackMessage
    //

    struct PayloadSpecificFeedbackMessage : public FeedbackMessage
    {
      // https://tools.ietf.org/html/rfc4585#section-6.3

      //---------------------------------------------------------------------
      //
      // RTCPPacket::PayloadSpecificFeedbackMessage::PLI
      //

      struct PLI
      {
        // https://tools.ietf.org/html/rfc4585#section-6.3.1

        static const BYTE kFmt {1};
      public:
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::PayloadSpecificFeedbackMessage::SLI
      //

      struct SLI
      {
        // https://tools.ietf.org/html/rfc4585#section-6.3.2

        static const BYTE kFmt {2};

        WORD first() const noexcept                       {return mFirst;}
        WORD number() const noexcept                      {return mNumber;}
        BYTE pictureID() const noexcept                   {return mPictureID;}

      public:
        WORD mFirst {};
        WORD mNumber {};
        BYTE mPictureID {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::PayloadSpecificFeedbackMessage::RPSI
      //

      struct RPSI
      {
        // https://tools.ietf.org/html/rfc4585#section-6.3.3

        static const BYTE kFmt {3};

        BYTE zeroBit() const noexcept                         {return mZeroBit;}
        BYTE payloadType() const noexcept                     {return mPayloadType;}

        const BYTE *nativeRPSIBitString() const noexcept      {return mNativeRPSIBitString;}
        size_t nativeRPSIBitStringSizeInBits() const noexcept {return mNativeRPSIBitStringSizeInBits;}

      public:
        BYTE mZeroBit {};
        BYTE mPayloadType {};

        const BYTE *mNativeRPSIBitString {};
        size_t mNativeRPSIBitStringSizeInBits {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::PayloadSpecificFeedbackMessage::FIR
      //

      struct CodecControlCommon
      {
        DWORD ssrc() const noexcept                       {return mSSRC;}
        BYTE seqNr() const noexcept                       {return mSeqNr;}
        DWORD reserved() const noexcept                   {return mReserved;}

      public:
        DWORD mSSRC {};
        BYTE mSeqNr {};
        DWORD mReserved {};
        DWORD mControlSpecific {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::PayloadSpecificFeedbackMessage::FIR
      //

      struct FIR : public CodecControlCommon
      {
        // http://tools.ietf.org/html/rfc5104#section-4.3.1.1

        static const BYTE kFmt {4};

      public:
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::PayloadSpecificFeedbackMessage::TSTR
      //

      struct TSTR : public CodecControlCommon
      {
        // http://tools.ietf.org/html/rfc5104#section-4.3.2.1

        static const BYTE kFmt {5};

        BYTE index() const noexcept                       {return static_cast<BYTE>(mControlSpecific);}

      public:
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::PayloadSpecificFeedbackMessage::TSTN
      //

      struct TSTN : public CodecControlCommon
      {
        // http://tools.ietf.org/html/rfc5104#section-4.3.3.1

        static const BYTE kFmt {6};

        BYTE index() const noexcept                       {return static_cast<BYTE>(mControlSpecific);}

      public:
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::PayloadSpecificFeedbackMessage::VBCM
      //

      struct VBCM : public CodecControlCommon
      {
        // http://tools.ietf.org/html/rfc5104#section-4.3.4.1

        static const BYTE kFmt {7};

        BYTE zeroBit() const noexcept;
        BYTE payloadType() const noexcept;

        const BYTE *vbcmOctetString() const noexcept    {return mVBCMOctetString;}
        size_t vbcmOctetStringSize() const noexcept;

      public:
        const BYTE *mVBCMOctetString {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::PayloadSpecificFeedbackMessage::AFB
      //

      struct AFB
      {
        // https://tools.ietf.org/html/rfc4585#section-6.4

        static const BYTE kFmt {15};

        const BYTE *data() const noexcept                 {return mData;}
        size_t dataSize() const noexcept                  {return mDataSize;}

      public:
        const BYTE *mData {};
        size_t mDataSize {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::PayloadSpecificFeedbackMessage::REMB
      //

      struct REMB
      {
        // https://tools.ietf.org/html/draft-alvestrand-rmcat-remb-03#section-2.2

        static const BYTE kFmt {15};

        size_t numSSRC() const noexcept                   {return static_cast<size_t>(mNumSSRC);}
        BYTE brExp() const noexcept                       {return mBRExp;}
        DWORD brMantissa() const noexcept                 {return mBRMantissa;}

        DWORD ssrcAtIndex(size_t index) const noexcept;

      public:
        BYTE mNumSSRC {};
        BYTE mBRExp {};
        DWORD mBRMantissa {};
        DWORD *mSSRCs {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::PayloadSpecificFeedbackMessage (public)
      //

      static const BYTE kPayloadType {206};

      PayloadSpecificFeedbackMessage *nextPayloadSpecificFeedbackMessage() const  {return mNextPayloadSpecificFeedbackMessage;}

      size_t sliCount() const noexcept                        {return mSLICount;}
      size_t firCount() const noexcept                        {return mFIRCount;}
      size_t tstrCount() const noexcept                       {return mTSTRCount;}
      size_t tstnCount() const noexcept                       {return mTSTNCount;}
      size_t vbcmCount() const noexcept                       {return mVBCMCount;}

      PLI *pli() const noexcept;
      SLI *sliAtIndex(size_t index) const noexcept;
      RPSI *rpsi() const noexcept;
      FIR *firAtIndex(size_t index) const noexcept;
      TSTR *tstrAtIndex(size_t index) const noexcept;
      TSTN *tstnAtIndex(size_t index) const noexcept;
      VBCM *vbcmAtIndex(size_t index) const noexcept;
      AFB *afb() const noexcept;
      REMB *remb() const noexcept;
      PayloadSpecificFeedbackMessage *unknown() const noexcept {return mUnknown;}

    public:
      PayloadSpecificFeedbackMessage *mNextPayloadSpecificFeedbackMessage {};

      PLI mPLI {};
      size_t mSLICount {};
      SLI *mFirstSLI {};
      RPSI mRPSI {};
      size_t mFIRCount {};
      FIR *mFirstFIR {};
      size_t mTSTRCount {};
      TSTR *mFirstTSTR {};
      size_t mTSTNCount {};
      TSTN *mFirstTSTN {};
      size_t mVBCMCount {};
      VBCM *mFirstVBCM {};
      AFB mAFB {};
      bool mHasREMB {false};
      REMB mREMB {};

      PayloadSpecificFeedbackMessage *mUnknown {};
    };

    //-----------------------------------------------------------------------
    //
    // RTCPPacket::XR
    //

    struct XR : public Report
    {
      // https://tools.ietf.org/html/rfc3611#section-2
      // https://tools.ietf.org/html/rfc3611#section-3

      typedef WORD RLEChunk;

      //---------------------------------------------------------------------
      //
      // RTCPPacket::XR::ReportBlock
      //

      struct ReportBlock
      {
        ReportBlock *next() const noexcept                          {return mNext;}

        BYTE blockType() const noexcept                             {return mBlockType;}
        BYTE typeSpecific() const noexcept                          {return mTypeSpecific;}

        const BYTE *typeSpecificContents() const noexcept           {return mTypeSpecificContents;}
        size_t typeSpecificContentSize() const noexcept             {return mTypeSpecificContentSize;}

        static const char *blockTypeToString(BYTE blockType) noexcept;
        const char *blockTypeToString() const noexcept              {return blockTypeToString(mBlockType);}

      public:
        ReportBlock *mNext {};

        BYTE mBlockType {};
        BYTE mTypeSpecific {};

        const BYTE *mTypeSpecificContents {};
        size_t mTypeSpecificContentSize {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::XR::ReportBlockRange
      //

      struct ReportBlockRange : public ReportBlock
      {
        BYTE reserved() const noexcept;
        BYTE thinning() const noexcept;

        DWORD ssrcOfSource() const noexcept                         {return mSSRCOfSource;}
        WORD beginSeq() const noexcept                              {return mBeginSeq;}
        WORD endSeq() const noexcept                                {return mEndSeq;}

      public:
        DWORD mSSRCOfSource {};
        WORD mBeginSeq {};
        WORD mEndSeq {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::XR::RLEReportBlock
      //

      struct RLEReportBlock : public ReportBlockRange
      {
        size_t chunkCount() const noexcept                          {return mChunkCount;}
        RLEChunk chunkAtIndex(size_t index) const noexcept;

      public:
        size_t mChunkCount {};
        RLEChunk *mChunks {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::XR::LossRLEReportBlock
      //

      struct LossRLEReportBlock : public RLEReportBlock
      {
        // https://tools.ietf.org/html/rfc3611#section-4.1

        static const BYTE kBlockType {1};

        LossRLEReportBlock *nextLossRLE() const noexcept            {return mNextLossRLE;}

      public:
        LossRLEReportBlock *mNextLossRLE {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::XR::DuplicateRLEReportBlock
      //

      struct DuplicateRLEReportBlock : public RLEReportBlock
      {
        // https://tools.ietf.org/html/rfc3611#section-4.2

        static const BYTE kBlockType {2};

        DuplicateRLEReportBlock *nextDuplicateRLE() const noexcept {return mNextDuplicateRLE;}

      public:
        DuplicateRLEReportBlock *mNextDuplicateRLE {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::XR::PacketReceiptTimesReportBlock
      //

      struct PacketReceiptTimesReportBlock : public ReportBlockRange
      {
        // https://tools.ietf.org/html/rfc3611#section-4.3

        static const BYTE kBlockType {3};

        PacketReceiptTimesReportBlock *nextPacketReceiptTimesReportBlock() const noexcept {return mNextPacketReceiptTimesReportBlock;}

        size_t receiptTimeCount() const noexcept                                          {return mReceiptTimeCount;}
        DWORD receiptTimeAtIndex(size_t index) const noexcept;

      public:
        PacketReceiptTimesReportBlock *mNextPacketReceiptTimesReportBlock {};

        size_t mReceiptTimeCount {};
        DWORD *mReceiptTimes {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::XR::ReceiverReferenceTimeReportBlock
      //

      struct ReceiverReferenceTimeReportBlock : public ReportBlock
      {
        // https://tools.ietf.org/html/rfc3611#section-4.4

        static const BYTE kBlockType {4};

        ReceiverReferenceTimeReportBlock *nextReceiverReferenceTimeReportBlock() const noexcept {return mNextReceiverReferenceTimeReportBlock;}

        DWORD ntpTimestampMS() const noexcept         {return mNTPTimestampMS;}
        DWORD ntpTimestampLS() const noexcept         {return mNTPTimestampLS;}
        Time ntpTimestamp() const noexcept;

      public:
        ReceiverReferenceTimeReportBlock *mNextReceiverReferenceTimeReportBlock {};

        DWORD mNTPTimestampMS {};
        DWORD mNTPTimestampLS {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::XR::DLRRReportBlock
      //

      struct DLRRReportBlock : public ReportBlock
      {
        // https://tools.ietf.org/html/rfc3611#section-4.5

        static const BYTE kBlockType {5};

        struct SubBlock
        {
          DWORD ssrc() const noexcept {return mSSRC;}
          DWORD lrr() const noexcept  {return mLRR;}
          DWORD dlrr() const noexcept {return mDLRR;}

          DWORD mSSRC {};
          DWORD mLRR {};
          DWORD mDLRR {};
        };

        DLRRReportBlock *nextDLRRReportBlock() const noexcept {return mNextDLRRReportBlock;}

        size_t subBlockCount() const noexcept {return mSubBlockCount;}
        SubBlock *subBlockAtIndex(size_t index) const noexcept;

      public:
        DLRRReportBlock *mNextDLRRReportBlock {};

        size_t mSubBlockCount {};
        SubBlock *mSubBlocks {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::XR::StatisticsSummaryReportBlock
      //

      struct StatisticsSummaryReportBlock : public ReportBlockRange
      {
        // https://tools.ietf.org/html/rfc3611#section-4.6

        static const BYTE kBlockType {6};

        StatisticsSummaryReportBlock *nextStatisticsSummaryReportBlock() const noexcept {return mNextStatisticsSummaryReportBlock;}

        bool lossReportFlag() const noexcept;
        bool duplicateReportFlag() const noexcept;
        bool jitterFlag() const noexcept;
        bool ttlFlag() const noexcept;
        bool hopLimitFlag() const noexcept;

        DWORD lostPackets() const noexcept            {return mLostPackets;}
        DWORD dupPackets() const noexcept             {return mDupPackets;}

        DWORD minJitter() const noexcept              {return mMinJitter;}
        DWORD maxJitter() const noexcept              {return mMaxJitter;}
        DWORD meanJitter() const noexcept             {return mMeanJitter;}
        DWORD devJitter() const noexcept              {return mDevJitter;}

        BYTE minTTL() const noexcept                  {return mMinTTLOrHL;}
        BYTE maxTTL() const noexcept                  {return mMaxTTLOrHL;}
        BYTE meanTTL() const noexcept                 {return mMeanTTLOrHL;}
        BYTE devTTL() const noexcept                  {return mDevTTLOrHL;}

        BYTE minHopLimit() const noexcept             {return mMinTTLOrHL;}
        BYTE maxHopLimit() const noexcept             {return mMaxTTLOrHL;}
        BYTE meanHopLimit() const noexcept            {return mMeanTTLOrHL;}
        BYTE devHopLimit() const noexcept             {return mDevTTLOrHL;}

      public:
        StatisticsSummaryReportBlock *mNextStatisticsSummaryReportBlock {};

        DWORD mLostPackets {};
        DWORD mDupPackets {};
        DWORD mMinJitter {};
        DWORD mMaxJitter {};
        DWORD mMeanJitter {};
        DWORD mDevJitter {};

        BYTE mMinTTLOrHL {};
        BYTE mMaxTTLOrHL {};
        BYTE mMeanTTLOrHL {};
        BYTE mDevTTLOrHL {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::XR::VoIPMetricsReportBlock
      //

      struct VoIPMetricsReportBlock : public ReportBlock
      {
        // https://tools.ietf.org/html/rfc3611#section-4.7

        static const BYTE kBlockType {7};

        VoIPMetricsReportBlock *nextVoIPMetricsReportBlock() const noexcept {return mNextVoIPMetricsReportBlock;}

        DWORD ssrcOfSource() const noexcept             {return mSSRCOfSource;}

        BYTE lossRate() const noexcept                  {return mLossRate;}
        BYTE discardRate() const noexcept               {return mDiscardRate;}
        BYTE burstDensity() const noexcept              {return mBurstDensity;}
        BYTE gapDensity() const noexcept                {return mGapDensity;}

        WORD burstDuration() const noexcept             {return mBurstDuration;}
        WORD gapDuration() const noexcept               {return mGapDuration;}

        WORD roundTripDelay() const noexcept            {return mRoundTripDelay;}
        WORD endSystemDelay() const noexcept            {return mEndSystemDelay;}

        BYTE signalLevel() const noexcept               {return mSignalLevel;}
        BYTE noiseLevel() const noexcept                {return mNoiseLevel;}
        BYTE rerl() const noexcept                      {return mRERL;}
        BYTE Gmin() const noexcept                      {return mGmin;}

        BYTE rFactor() const noexcept                   {return mRFactor;}
        BYTE extRFactor() const noexcept                {return mExtRFactor;}
        BYTE mosLQ() const noexcept                     {return mMOSLQ;}
        BYTE mosCQ() const noexcept                     {return mMOSCQ;}

        BYTE rxConfig() const noexcept                  {return mRXConfig;}
        BYTE plc() const noexcept;
        BYTE jba() const noexcept;
        BYTE jbRate() const noexcept;

        WORD jbNominal() const noexcept                 {return mJBNominal;}
        WORD jbMaximum() const noexcept                 {return mJBMaximum;}
        WORD jbAbsMax() const noexcept                  {return mJBAbsMax;}

      public:
        VoIPMetricsReportBlock *mNextVoIPMetricsReportBlock {};

        DWORD mSSRCOfSource {};

        BYTE mLossRate {};
        BYTE mDiscardRate {};
        BYTE mBurstDensity {};
        BYTE mGapDensity {};

        WORD mBurstDuration {};
        WORD mGapDuration {};

        WORD mRoundTripDelay {};
        WORD mEndSystemDelay {};

        BYTE mSignalLevel {};
        BYTE mNoiseLevel {};
        BYTE mRERL {};
        BYTE mGmin {};

        BYTE mRFactor {};
        BYTE mExtRFactor {};
        BYTE mMOSLQ {};
        BYTE mMOSCQ {};

        BYTE mRXConfig {};
        BYTE mReservedVoIP {};

        WORD mJBNominal {};
        WORD mJBMaximum {};
        WORD mJBAbsMax {};
      };

      //---------------------------------------------------------------------
      //
      // RTCPPacket::XR::UnknownReportBlock
      //

      struct UnknownReportBlock : public ReportBlock
      {
        UnknownReportBlock *nextUnknownReportBlock() const noexcept {return mNextUnknownReportBlock;}

      public:
        UnknownReportBlock *mNextUnknownReportBlock {};
      };

      //-------------------------------------------------------------------
      //
      // RTCPPacket::XR::RunLength
      //

      struct RunLength
      {
        // https://tools.ietf.org/html/rfc3611#section-4.1.1

        RunLength(RLEChunk chunk) noexcept;

        BYTE runType() const noexcept {return mRunType;}
        size_t runLength() const noexcept {return static_cast<size_t>(mRunLength);}

      public:
        BYTE mRunType {};   // Zero indicates a run of 0s.  One indicates a run of 1s.
        WORD mRunLength {}; // A value between 1 and 16,383.
      };

      //-------------------------------------------------------------------
      //
      // RTCPPacket::XR::BitVector
      //

      struct BitVector
      {
        // https://tools.ietf.org/html/rfc3611#section-4.1.2

        BitVector(RLEChunk chunk) noexcept;

        BYTE bitAtIndex(size_t index) const noexcept;  // Zero indicates loss.
        WORD bitVector() const noexcept { return mBitVector; }

      public:
        WORD mBitVector {};
      };

      //-------------------------------------------------------------------
      //
      // RTCPPacket::XR
      //

      static const BYTE kPayloadType {207};

      XR *nextXR() const noexcept                                                             {return mNextXR;}

      BYTE reserved() const noexcept                                                          {return mReportSpecific;}

      DWORD ssrc() const noexcept                                                             {return mSSRC;}

      ReportBlock *firstReportBlock() const noexcept                                          {return mFirstReportBlock;}

      LossRLEReportBlock *firstLossRLEReportBlock() const noexcept                            {return mFirstLossRLEReportBlock;}
      DuplicateRLEReportBlock *firstDuplicateRLEReportBlock() const noexcept                  {return mFirstDuplicateRLEReportBlock;}
      PacketReceiptTimesReportBlock *firstPacketReceiptTimesReportBlock() const noexcept      {return mFirstPacketReceiptTimesReportBlock;}
      ReceiverReferenceTimeReportBlock *firstReceiverReferenceTimeReportBlock() const noexcept {return mFirstReceiverReferenceTimeReportBlock;}
      DLRRReportBlock *firstDLRRReportBlock() const noexcept                                  {return mFirstDLRRReportBlock;}
      StatisticsSummaryReportBlock *firstStatisticsSummaryReportBlock() const noexcept        {return mFirstStatisticsSummaryReportBlock;}
      VoIPMetricsReportBlock *firstVoIPMetricsReportBlock() const noexcept                    {return mFirstVoIPMetricsReportBlock;}
      UnknownReportBlock *firstUnknownReportBlock() const noexcept                            {return mFirstUnknownReportBlock;}

      size_t reportBlockCount() const noexcept                                                {return mReportBlockCount;}

      size_t lossRLEReportBlockCount() const noexcept                                         {return mLossRLEReportBlockCount;}
      size_t duplicateRLEReportBlockCount() const noexcept                                    {return mDuplicateRLEReportBlockCount;}
      size_t packetReceiptTimesReportBlockCount() const noexcept                              {return mPacketReceiptTimesReportBlockCount;}
      size_t receiverReferenceTimeReportBlockCount() const noexcept                           {return mReceiverReferenceTimeReportBlockCount;}
      size_t dlrrReportBlockCount() const noexcept                                            {return mDLRRReportBlockCount;}
      size_t statisticsSummaryReportBlockCount() const noexcept                               {return mStatisticsSummaryReportBlockCount;}
      size_t voIPMetricsReportBlockCount() const noexcept                                     {return mVoIPMetricsReportBlockCount;}
      size_t unknownReportBlockCount() const noexcept                                         {return mUnknownReportBlockCount;}

      LossRLEReportBlock *lossRLEReportBlockAtIndex(size_t index) const noexcept;
      DuplicateRLEReportBlock *duplicateRLEReportBlockAtIndex(size_t index) const noexcept;
      PacketReceiptTimesReportBlock *packetReceiptTimesReportBlockAtIndex(size_t index) const noexcept;
      ReceiverReferenceTimeReportBlock *receiverReferenceTimeReportBlockAtIndex(size_t index) const noexcept;
      DLRRReportBlock *dlrrReportBlockAtIndex(size_t index) const noexcept;
      StatisticsSummaryReportBlock *statisticsSummaryReportBlockAtIndex(size_t index) const noexcept;
      VoIPMetricsReportBlock *voIPMetricsReportBlockAtIndex(size_t index) const noexcept;
      UnknownReportBlock *unknownReportBlockAtIndex(size_t index) const noexcept;

      static bool isRunLengthChunk(RLEChunk chunk) noexcept;
      static bool isBitVectorChunk(RLEChunk chunk) noexcept;

      static RunLength runLength(RLEChunk chunk) noexcept                                     {return RunLength(chunk);}
      static BitVector bitVector(RLEChunk chunk) noexcept                                     {return BitVector(chunk);}

    public:
      XR *mNextXR {};

      DWORD mSSRC {};

      size_t mReportBlockCount {};

      size_t mLossRLEReportBlockCount {};
      size_t mDuplicateRLEReportBlockCount {};
      size_t mPacketReceiptTimesReportBlockCount {};
      size_t mReceiverReferenceTimeReportBlockCount {};
      size_t mDLRRReportBlockCount {};
      size_t mStatisticsSummaryReportBlockCount {};
      size_t mVoIPMetricsReportBlockCount {};
      size_t mUnknownReportBlockCount {};

      ReportBlock *mFirstReportBlock {};

      LossRLEReportBlock *mFirstLossRLEReportBlock {};
      DuplicateRLEReportBlock *mFirstDuplicateRLEReportBlock {};
      PacketReceiptTimesReportBlock *mFirstPacketReceiptTimesReportBlock {};
      ReceiverReferenceTimeReportBlock *mFirstReceiverReferenceTimeReportBlock {};
      DLRRReportBlock *mFirstDLRRReportBlock {};
      StatisticsSummaryReportBlock *mFirstStatisticsSummaryReportBlock {};
      VoIPMetricsReportBlock *mFirstVoIPMetricsReportBlock {};
      UnknownReportBlock *mFirstUnknownReportBlock {};
    };

    //-----------------------------------------------------------------------
    //
    // RTCPPacket::UnknownReport
    //

    struct UnknownReport : public Report
    {
      UnknownReport *nextUnknown() const noexcept         {return mNextUnknown;}

    public:
      UnknownReport *mNextUnknown {};
    };

  public:
    //-----------------------------------------------------------------------
    //
    // (public)
    //

    RTCPPacket(
               const make_private &,
               MediaChannelID mediaChannelID
               ) noexcept;
    ~RTCPPacket() noexcept;

    static RTCPPacketPtr create(
                                const BYTE *buffer,
                                size_t bufferLengthInBytes,
                                MediaChannelID mediaChannelID = 0
                                ) noexcept(false); // throws InvalidParameters
    static RTCPPacketPtr create(
                                const SecureByteBlock &buffer,
                                MediaChannelID mediaChannelID = 0
                                ) noexcept(false); // throws InvalidParameters
    static RTCPPacketPtr create(
                                SecureByteBlockPtr buffer,
                                MediaChannelID mediaChannelID = 0
                                ) noexcept(false);  // NOTE: ownership of buffer is taken / assumed // throws InvalidParameters
    static RTCPPacketPtr create(
                                const Report *first,
                                MediaChannelID mediaChannelID = 0
                                ) noexcept(false); // throws InvalidParameters
    static SecureByteBlockPtr generateFrom(const Report *first) noexcept(false); // throws InvalidParameters

    const BYTE *ptr() const noexcept;
    size_t size() const noexcept;
    SecureByteBlockPtr buffer() const noexcept;

    Report *first() const noexcept                                                      {return mFirst;}

    SenderReport *firstSenderReport() const noexcept                                    {return mFirstSenderReport;}
    ReceiverReport *firstReceiverReport() const noexcept                                {return mFirstReceiverReport;}
    SDES *firstSDES() const noexcept                                                    {return mFirstSDES;}
    Bye *firstBye() const noexcept                                                      {return mFirstBye;}
    App *firstApp() const noexcept                                                      {return mFirstApp;}
    TransportLayerFeedbackMessage *firstTransportLayerFeedbackMessage() const noexcept  {return mFirstTransportLayerFeedbackMessage;}
    PayloadSpecificFeedbackMessage *firstPayloadSpecificFeedbackMessage() const noexcept {return mFirstPayloadSpecificFeedbackMessage;}
    XR *firstXR() const noexcept                                                        {return mFirstXR;}
    UnknownReport *firstUnknownReport() const noexcept                                  {return mFirstUnknownReport;}

    size_t count() const noexcept                                                       {return mCount;}

    size_t senderReportCount() const noexcept                                           {return mSenderReportCount;}
    size_t receiverReportCount() const noexcept                                         {return mReceiverReportCount;}
    size_t sdesCount() const noexcept                                                   {return mSDESCount;}
    size_t byeCount() const noexcept                                                    {return mByeCount;}
    size_t appCount() const noexcept                                                    {return mAppCount;}
    size_t transportLayerFeedbackMessageCount() const noexcept                          {return mTransportLayerFeedbackMessageCount;}
    size_t payloadSpecificFeedbackMessage() const noexcept                              {return mPayloadSpecificFeedbackMessageCount;}
    size_t xrCount() const noexcept                                                     {return mXRCount;}
    size_t unknownReportCount() const noexcept                                          {return mUnknownReportCount;}
      
    SenderReport *senderReportAtIndex(size_t index) const noexcept;
    ReceiverReport *receiverReportAtIndex(size_t index) const noexcept;
    SDES *sdesAtIndex(size_t index) const noexcept;
    Bye *byeAtIndex(size_t index) const noexcept;
    App *appAtIndex(size_t index) const noexcept;
    TransportLayerFeedbackMessage *transportLayerFeedbackReportAtIndex(size_t index) const noexcept;
    PayloadSpecificFeedbackMessage *payloadSpecificFeedbackReportAtIndex(size_t index) const noexcept;
    XR *xrAtIndex(size_t index) const noexcept;
    UnknownReport *unknownAtIndex(size_t index) const noexcept;

    void trace(
               const char *func = NULL,
               const char *message = NULL
               ) const noexcept;

  protected:
    //-----------------------------------------------------------------------
    //
    // (internal)
    //

    bool parse() noexcept;

    bool getAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, BYTE pt, const BYTE *contents, size_t contentSize) noexcept;
    bool getSenderReportAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getReceiverReportAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getSDESAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getByeAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getAppAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getTransportLayerFeedbackMessageAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getPayloadSpecificFeedbackMessageAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getXRAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getUnknownReportAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize) noexcept;

    bool getTransportLayerFeedbackMessageGenericNACKAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize) noexcept;
    bool getTransportLayerFeedbackMessageTMMBRAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize) noexcept;
    bool getTransportLayerFeedbackMessageTMMBNAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize) noexcept;

    bool getPayloadSpecificFeedbackMessagePLIAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize) noexcept;
    bool getPayloadSpecificFeedbackMessageSLIAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize) noexcept;
    bool getPayloadSpecificFeedbackMessageRPSIAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize) noexcept;
    bool getPayloadSpecificFeedbackMessageFIRAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize) noexcept;
    bool getPayloadSpecificFeedbackMessageTSTRAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize) noexcept;
    bool getPayloadSpecificFeedbackMessageTSTNAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize) noexcept;
    bool getPayloadSpecificFeedbackMessageVBCMAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize) noexcept;
    bool getPayloadSpecificFeedbackMessageAFBAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize) noexcept;
    bool getPayloadSpecificFeedbackMessageREMBAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize) noexcept;

    bool getXRLossRLEReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getXRDuplicateRLEReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getXRPacketReceiptTimesReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getXRReceiverReferenceTimeReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getXRDLRRReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getXRStatisticsSummaryReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getXRVoIPMetricsReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize) noexcept;
    bool getXRUnknownReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize) noexcept;

    bool parse(Report * &ioLastReport, BYTE version, BYTE padding, BYTE reportSpecific, BYTE pt, const BYTE *contents, size_t contentSize) noexcept;
    void fill(Report *report, BYTE version, BYTE padding, BYTE reportSpecific, BYTE pt, const BYTE *contents, size_t contentSize) noexcept;

    bool parseCommon(
                      SenderReceiverCommonReport *report,
                      size_t detailedHeaderSize
                      ) noexcept;
    bool parse(SenderReport *report) noexcept;
    bool parse(ReceiverReport *report) noexcept;
    bool parse(SDES *report) noexcept;
    bool parse(Bye *report) noexcept;
    bool parse(App *report) noexcept;
    bool parse(TransportLayerFeedbackMessage *report) noexcept;
    bool parse(PayloadSpecificFeedbackMessage *report) noexcept;
    bool parse(XR *report) noexcept;
    bool parse(UnknownReport *report) noexcept;

    void fill(FeedbackMessage *report, const BYTE *contents, size_t contentSize) noexcept;
    void fill(XR *report, XR::ReportBlock *reportBlock, XR::ReportBlock * &ioPreviousReportBlock, BYTE blockType, BYTE typeSpecific, const BYTE *contents, size_t contentSize) noexcept;

    bool parseGenericNACK(TransportLayerFeedbackMessage *report) noexcept;
    void fillTMMBRCommon(TransportLayerFeedbackMessage *report, TransportLayerFeedbackMessage::TMMBRCommon *common, const BYTE *pos) noexcept;
    bool parseTMMBR(TransportLayerFeedbackMessage *report) noexcept;
    bool parseTMMBN(TransportLayerFeedbackMessage *report) noexcept;
    bool parseUnknown(TransportLayerFeedbackMessage *report) noexcept;

    //CodecControlCommon
    bool parsePLI(PayloadSpecificFeedbackMessage *report) noexcept;
    bool parseSLI(PayloadSpecificFeedbackMessage *report) noexcept;
    bool parseRPSI(PayloadSpecificFeedbackMessage *report) noexcept;
    void fillCodecControlCommon(PayloadSpecificFeedbackMessage *report, PayloadSpecificFeedbackMessage::CodecControlCommon *common, const BYTE *pos) noexcept;
    bool parseFIR(PayloadSpecificFeedbackMessage *report) noexcept;
    bool parseTSTR(PayloadSpecificFeedbackMessage *report) noexcept;
    bool parseTSTN(PayloadSpecificFeedbackMessage *report) noexcept;
    bool parseVBCM(PayloadSpecificFeedbackMessage *report) noexcept;
    bool parseAFB(PayloadSpecificFeedbackMessage *report) noexcept;
    bool parseREMB(PayloadSpecificFeedbackMessage *report) noexcept;
    bool parseUnknown(PayloadSpecificFeedbackMessage *report) noexcept;

    bool parseCommonRange(XR *xr, XR::ReportBlockRange *reportBlock) noexcept;
    bool parseCommonRLE(XR *xr, XR::RLEReportBlock *reportBlock) noexcept;
    bool parse(XR *xr, XR::LossRLEReportBlock *reportBlock) noexcept;
    bool parse(XR *xr, XR::DuplicateRLEReportBlock *reportBlock) noexcept;
    bool parse(XR *xr, XR::PacketReceiptTimesReportBlock *reportBlock) noexcept;
    bool parse(XR *xr, XR::ReceiverReferenceTimeReportBlock *reportBlock) noexcept;
    bool parse(XR *xr, XR::DLRRReportBlock *reportBlock) noexcept;
    bool parse(XR *xr, XR::StatisticsSummaryReportBlock *reportBlock) noexcept;
    bool parse(XR *xr, XR::VoIPMetricsReportBlock *reportBlock) noexcept;
    bool parse(XR *xr, XR::UnknownReportBlock *reportBlock) noexcept;

    void *allocateBuffer(size_t size) noexcept;

    static size_t getPacketSize(const Report *first) noexcept(false); // throws InvalidParameters
    static void writePacket(const Report *first, BYTE * &ioPos, size_t &ioRemaining) noexcept;

  public:
    SecureByteBlockPtr mBuffer;
    SecureByteBlockPtr mAllocationBuffer;

    MediaChannelID mMediaChannelID {};

    BYTE *mAllocationPos {};
    size_t mAllocationSize {};

    Report *mFirst {};

    size_t mCount {};

    size_t mSenderReportCount {};
    size_t mReceiverReportCount {};
    size_t mSDESCount {};
    size_t mByeCount {};
    size_t mAppCount {};
    size_t mTransportLayerFeedbackMessageCount {};
    size_t mPayloadSpecificFeedbackMessageCount {};
    size_t mXRCount {};
    size_t mUnknownReportCount {};

    SenderReport *mFirstSenderReport {};
    ReceiverReport *mFirstReceiverReport {};
    SDES *mFirstSDES {};
    Bye *mFirstBye {};
    App *mFirstApp {};
    TransportLayerFeedbackMessage *mFirstTransportLayerFeedbackMessage {};
    PayloadSpecificFeedbackMessage *mFirstPayloadSpecificFeedbackMessage {};
    XR *mFirstXR {};
    UnknownReport *mFirstUnknownReport {};
  };

} // namespace ortc
