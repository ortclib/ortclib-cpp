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

#include <ortc/internal/types.h>

#include <ortc/IICETypes.h>

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCPPacket
    #pragma mark

    class RTCPPacket
    {
    protected:
      struct make_private {};

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCPPacket::Report
      #pragma mark

      struct Report
      {
        Report *next() const            {return mNext;}

        const BYTE *ptr() const         {return mPtr;}
        size_t size() const             {return mSize;}

        BYTE version() const            {return mVersion;}
        size_t padding() const          {return static_cast<size_t>(mPadding);}
        BYTE reportSpecific() const     {return mReportSpecific;}

        BYTE pt() const                 {return mPT;}
        static const char *ptToString(BYTE pt);
        const char *ptToString() const  {return ptToString(mPT);}

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
      #pragma mark
      #pragma mark RTCPPacket::SenderReceiverCommonReport
      #pragma mark

      struct SenderReceiverCommonReport : public Report
      {
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::SenderReceiverCommonReport::ReportBlock
        #pragma mark

        struct ReportBlock
        {
          ReportBlock *next() const                           {return mNext;}

          DWORD ssrc() const                                  {return mSSRC;}
          BYTE fractionLost() const                           {return mFractionLost;}
          DWORD cumulativeNumberOfPacketsLost() const         {return mCumulativeNumberOfPacketsLost;}
          DWORD extendedHighestSequenceNumberReceived() const {return mExtendedHighestSequenceNumberReceived;}
          DWORD interarrivalJitter() const                    {return mInterarrivalJitter;}
          DWORD lsr() const                                   {return mLSR;}
          DWORD dlsr() const                                  {return mDLSR;}

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
        #pragma mark
        #pragma mark RTCPPacket::SenderReceiverCommonReport (public)
        #pragma mark

        size_t rc() const                       {return static_cast<size_t>(mReportSpecific);}
        DWORD ssrcOfSender() const              {return mSSRCOfSender;}

        ReportBlock *firstReportBlock() const   {return mFirstReportBlock;}

        const BYTE *extension() const           {return mExtension;}
        size_t extensionSize() const            {return mExtensionSize;}

      public:
        DWORD mSSRCOfSender {};

        const BYTE *mExtension {};
        size_t mExtensionSize {};

        ReportBlock *mFirstReportBlock {};
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCPPacket::SenderReport
      #pragma mark

      struct SenderReport : public SenderReceiverCommonReport
      {
        // https://tools.ietf.org/html/rfc3550#section-6.4.1

        static const BYTE kPayloadType {200};

        SenderReport *nextSenderReport() const  {return mNextSenderReport;}

        DWORD ntpTimestampMS() const            {return mNTPTimestampMS;}
        DWORD ntpTimestampLS() const            {return mNTPTimestampLS;}
        Time ntpTimestamp() const;
        DWORD rtpTimestamp() const              {return mRTPTimestamp;}
        DWORD senderPacketCount() const         {return mSenderPacketCount;}
        DWORD senderOctetCount() const          {return mSenderOctetCount;}

      public:
        SenderReport *mNextSenderReport {};

        DWORD mNTPTimestampMS {};
        DWORD mNTPTimestampLS {};
        DWORD mRTPTimestamp {};
        DWORD mSenderPacketCount {};
        DWORD mSenderOctetCount {};
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCPPacket::ReceiverReport
      #pragma mark

      struct ReceiverReport : public SenderReceiverCommonReport
      {
        // https://tools.ietf.org/html/rfc3550#section-6.4.2

        static const BYTE kPayloadType {201};

        ReceiverReport *nextReceiverReport() const  {return mNextReceiverReport;}

        DWORD ssrcOfPacketSender() const            {return mSSRCOfSender;}

      public:
        ReceiverReport *mNextReceiverReport {};
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCPPacket::SDES
      #pragma mark

      struct SDES : public Report
      {
        // https://tools.ietf.org/html/rfc3550#section-6.5

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::SDES::Chunk
        #pragma mark

        struct Chunk
        {
          static const BYTE kEndOfItemsType = 0;

          struct StringItem
          {
            BYTE type() const                 {return mType;}
            size_t length() const             {return mLength;}
            const char *value() const         {return mValue;}

            static const char *typeToString(BYTE type);
            const char *typeToString() const  {return typeToString(mType);}

          public:
            StringItem *mNext {};

            BYTE mType {};
            size_t mLength {};
            const char *mValue {};
          };

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark RTCPPacket::SDES::Chunk::CName
          #pragma mark

          struct CName : public StringItem
          {
            // https://tools.ietf.org/html/rfc3550#section-6.5.1

            static const BYTE kItemType = 1;

            CName *next() const                               {return static_cast<CName *>(mNext);}

            const char *userAndDomainName() const             {return value();}
          };

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark RTCPPacket::SDES::Chunk::Name
          #pragma mark

          struct Name : public StringItem
          {
            // https://tools.ietf.org/html/rfc3550#section-6.5.2

            static const BYTE kItemType = 2;

            Name *next() const                                {return static_cast<Name *>(mNext);}

            const char *commonName() const                    {return value();}
          };

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark RTCPPacket::SDES::Chunk::Email
          #pragma mark

          struct Email : public StringItem
          {
            // https://tools.ietf.org/html/rfc3550#section-6.5.3

            static const BYTE kItemType = 3;

            Email *next() const                               {return static_cast<Email *>(mNext);}

            const char *emailAddress() const                  {return value();}
          };

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark RTCPPacket::SDES::Chunk::Phone
          #pragma mark

          struct Phone : public StringItem
          {
            // https://tools.ietf.org/html/rfc3550#section-6.5.4

            static const BYTE kItemType = 4;

            Phone *next() const                               {return static_cast<Phone *>(mNext);}

            const char *phoneNumber() const                   {return value();}
          };

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark RTCPPacket::SDES::Chunk::Loc
          #pragma mark

          struct Loc : public StringItem
          {
            // https://tools.ietf.org/html/rfc3550#section-6.5.5

            static const BYTE kItemType = 5;

            Loc *next() const                                 {return static_cast<Loc *>(mNext);}

            const char *geographicLocation() const            {return value();}
          };

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark RTCPPacket::SDES::Chunk::Tool
          #pragma mark

          struct Tool : public StringItem
          {
            // https://tools.ietf.org/html/rfc3550#section-6.5.6

            static const BYTE kItemType = 6;

            Tool *next() const                                {return static_cast<Tool *>(mNext);}

            const char *nameVersion() const                   {return value();}
          };

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark RTCPPacket::SDES::Chunk::Note
          #pragma mark

          struct Note : public StringItem
          {
            // https://tools.ietf.org/html/rfc3550#section-6.5.7

            static const BYTE kItemType = 7;

            Note *next() const                                {return static_cast<Note *>(mNext);}

            const char *note() const                          {return value();}
          };

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark RTCPPacket::SDES::Chunk::Priv
          #pragma mark

          struct Priv : public StringItem
          {
            // https://tools.ietf.org/html/rfc3550#section-6.5.8

            static const BYTE kItemType = 8;

            Priv *next() const                                {return static_cast<Priv *>(mNext);}

            size_t prefixLength() const                       {return mPrefixLength;}
            const char *prefix() const                        {return mPrefix;}

          public:
            size_t mPrefixLength {};
            const char *mPrefix {};
          };

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark RTCPPacket::SDES::Chunk::Unknown
          #pragma mark

          struct Unknown : public StringItem
          {
            Unknown *next() const                             {return static_cast<Unknown *>(mNext);}

            const BYTE *ptr() const                           {return reinterpret_cast<const BYTE *>(value());}
            size_t size() const                               {return length();}
          };

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark RTCPPacket::SDES::Chunk::Mid
          #pragma mark

          struct Mid : public StringItem
          {
            // https://tools.ietf.org/html/draft-ietf-mmusic-sdp-bundle-negotiation-23#section-14.2
            static const BYTE kItemType = 9;  // TBD

            Mid *next() const                                {return static_cast<Mid *>(mNext);}

            const char *mid() const                          {return value();}
          };

          //-------------------------------------------------------------------
          #pragma mark
          #pragma mark RTCPPacket::SDES::Chunk
          #pragma mark

          Chunk *next() const                                 {return mNext;}

          DWORD ssrc() const                                  {return mSSRC;}

          size_t count() const                                {return mCount;}

          CName *firstCName() const                           {return mFirstCName;}
          Name *firstName() const                             {return mFirstName;}
          Email *firstEmail() const                           {return mFirstEmail;}
          Phone *firstPhone() const                           {return mFirstPhone;}
          Loc *firstLoc() const                               {return mFirstLoc;}
          Tool *firstTool() const                             {return mFirstTool;}
          Note *firstNote() const                             {return mFirstNote;}
          Priv *firstPriv() const                             {return mFirstPriv;}
          Mid *firstMid() const                               {return mFirstMid;}
          Unknown *firstUnknown() const                       {return mFirstUnknown;}

          size_t cNameCount() const                           {return mCNameCount;}
          size_t nameCount() const                            {return mNameCount;}
          size_t emailCount() const                           {return mEmailCount;}
          size_t phoneCount() const                           {return mPhoneCount;}
          size_t locCount() const                             {return mLocCount;}
          size_t toolCount() const                            {return mToolCount;}
          size_t noteCount() const                            {return mNoteCount;}
          size_t privCount() const                            {return mPrivCount;}
          size_t midCount() const                             {return mMidCount;}
          size_t unknownCount() const                         {return mUnknownCount;}

          CName *cNameAtIndex(size_t index) const;
          Name *nameAtIndex(size_t index) const;
          Email *emailAtIndex(size_t index) const;
          Phone *phoneAtIndex(size_t index) const;
          Loc *locAtIndex(size_t index) const;
          Tool *toolAtIndex(size_t index) const;
          Note *noteAtIndex(size_t index) const;
          Priv *privAtIndex(size_t index) const;
          Mid *midAtIndex(size_t index) const;
          Unknown *unknownAtIndex(size_t index) const;

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
          Unknown *mFirstUnknown {};
        };

        static const BYTE kPayloadType {202};

        SDES *nextSDES() const                  {return mNextSDES;}

        size_t sc() const                       {return static_cast<size_t>(mReportSpecific);}

        Chunk *firstChunk() const               {return mFirstChunk;}

      public:
        SDES *mNextSDES {};

        Chunk *mFirstChunk {};
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCPPacket::Bye
      #pragma mark

      struct Bye : public Report
      {
        // https://tools.ietf.org/html/rfc3550#section-6.6

        static const BYTE kPayloadType {203};

        Bye *nextBye() const                    {return mNextBye;}

        size_t sc() const                       {return static_cast<size_t>(mReportSpecific);}

        DWORD ssrc(size_t index) const;

        const char *reasonForLeaving() const    {return mReasonForLeaving;}

      public:
        Bye *mNextBye {};

        const char *mReasonForLeaving {};

        DWORD *mSSRCs {};
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCPPacket::App
      #pragma mark

      struct App : public Report
      {
        // https://tools.ietf.org/html/rfc3550#section-6.7

        static const BYTE kPayloadType {204};

        App *nextApp() const                    {return mNextApp;}

        BYTE subtype() const                    {return mReportSpecific;}

        DWORD ssrc() const                      {return mSSRC;}
        const char *name() const                {return &(mName[0]);}

        const BYTE *data() const                {return mData;}
        size_t dataSize() const                 {return mDataSize;}

      public:
        App *mNextApp {};

        DWORD mSSRC {};
        char mName[sizeof(DWORD)+1] {};

        const BYTE *mData {};
        size_t mDataSize {};
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCPPacket::FeedbackMessage
      #pragma mark

      struct FeedbackMessage : public Report
      {
        // https://tools.ietf.org/html/rfc4585#section-6.1

        BYTE fmt() const                                        {return mReportSpecific;}

        DWORD ssrcOfPacketSender() const                        {return mSSRCOfPacketSender;}
        DWORD ssrcOfMediaSource() const                         {return mSSRCOfMediaSource;}

        const BYTE *fci() const                                 {return mFCI;}
        size_t fciSize() const                                  {return mFCISize;}

        static const char *fmtToString(BYTE pt, BYTE fmt, DWORD subFmt = 0);
        const char *fmtToString() const;

      public:
        DWORD mSSRCOfPacketSender {};
        DWORD mSSRCOfMediaSource {};

        const BYTE *mFCI {};
        size_t mFCISize {};
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCPPacket::TransportLayerFeedbackMessage
      #pragma mark

      struct TransportLayerFeedbackMessage : public FeedbackMessage
      {
        // https://tools.ietf.org/html/rfc4585#section-6.2

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::TransportLayerFeedbackMessage::GenericNACK
        #pragma mark

        struct GenericNACK
        {
          // https://tools.ietf.org/html/rfc4585#section-6.2.1

          static const BYTE kFmt {1};

          WORD pid() const                                      {return mPID;}
          WORD blp() const                                      {return mBLP;}

        public:
          WORD mPID {};
          WORD mBLP {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::TransportLayerFeedbackMessage::TMMBRCommon
        #pragma mark

        struct TMMBRCommon
        {
          DWORD ssrc() const                                    {return mSSRC;}

          BYTE mxTBRExp() const                                 {return mMxTBRExp;}
          DWORD mxTBRMantissa() const                           {return mMxTBRMantissa;}
          WORD measuredOverhead() const                         {return mMeasuredOverhead;}

        public:
          DWORD mSSRC {};

          BYTE mMxTBRExp {};
          DWORD mMxTBRMantissa {};
          WORD mMeasuredOverhead {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::TransportLayerFeedbackMessage::TMMBR
        #pragma mark

        struct TMMBR : public TMMBRCommon
        {
          // http://tools.ietf.org/html/rfc5104#section-4.2.1.1

          static const BYTE kFmt {3};

        public:
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::TransportLayerFeedbackMessage::TMMBN
        #pragma mark

        struct TMMBN : public TMMBRCommon
        {
          // http://tools.ietf.org/html/rfc5104#section-4.2.2.1

          static const BYTE kFmt {4};

        public:
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::TransportLayerFeedbackMessage (public)
        #pragma mark

        static const BYTE kPayloadType {205};

        TransportLayerFeedbackMessage *nextTransportLayerFeedbackMessage() const  {return mNextTransportLayerFeedbackMessage;}

        size_t genericNACKCount() const                         {return mGenericNACKCount;}
        GenericNACK *genericNACKAtIndex(size_t index) const;

        size_t tmmbrCount() const                               {return mTMMBRCount;}
        TMMBR *tmmbrAtIndex(size_t index) const;

        size_t tmmbnCount() const                               {return mTMMBNCount;}
        TMMBN *tmmbnAtIndex(size_t index) const;

        TransportLayerFeedbackMessage *unknown() const          {return mUnknown;}

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
      #pragma mark
      #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage
      #pragma mark

      struct PayloadSpecificFeedbackMessage : public FeedbackMessage
      {
        // https://tools.ietf.org/html/rfc4585#section-6.3

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage::PLI
        #pragma mark

        struct PLI
        {
          // https://tools.ietf.org/html/rfc4585#section-6.3.1

          static const BYTE kFmt {1};
        public:
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage::SLI
        #pragma mark

        struct SLI
        {
          // https://tools.ietf.org/html/rfc4585#section-6.3.2

          static const BYTE kFmt {2};

          WORD first() const                        {return mFirst;}
          WORD number() const                       {return mNumber;}
          BYTE pictureID() const                    {return mPictureID;}

        public:
          WORD mFirst {};
          WORD mNumber {};
          BYTE mPictureID {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage::RPSI
        #pragma mark

        struct RPSI
        {
          // https://tools.ietf.org/html/rfc4585#section-6.3.3

          static const BYTE kFmt {3};

          BYTE zeroBit() const                          {return mZeroBit;}
          BYTE payloadType() const                      {return mPayloadType;}

          const BYTE *nativeRPSIBitString() const       {return mNativeRPSIBitString;}
          size_t nativeRPSIBitStringSizeInBits() const  {return mNativeRPSIBitStringSizeInBits;}

        public:
          BYTE mZeroBit {};
          BYTE mPayloadType {};

          const BYTE *mNativeRPSIBitString {};
          size_t mNativeRPSIBitStringSizeInBits {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage::FIR
        #pragma mark

        struct CodecControlCommon
        {
          DWORD ssrc() const                        {return mSSRC;}
          BYTE seqNr() const                        {return mSeqNr;}
          DWORD reserved() const                    {return mReserved;}

        public:
          DWORD mSSRC {};
          BYTE mSeqNr {};
          DWORD mReserved {};
          DWORD mControlSpecific {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage::FIR
        #pragma mark

        struct FIR : public CodecControlCommon
        {
          // http://tools.ietf.org/html/rfc5104#section-4.3.1.1

          static const BYTE kFmt {4};

        public:
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage::TSTR
        #pragma mark

        struct TSTR : public CodecControlCommon
        {
          // http://tools.ietf.org/html/rfc5104#section-4.3.2.1

          static const BYTE kFmt {5};

          BYTE index() const                        {return static_cast<BYTE>(mControlSpecific);}

        public:
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage::TSTN
        #pragma mark

        struct TSTN : public CodecControlCommon
        {
          // http://tools.ietf.org/html/rfc5104#section-4.3.3.1

          static const BYTE kFmt {6};

          BYTE index() const                        {return static_cast<BYTE>(mControlSpecific);}

        public:
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage::VBCM
        #pragma mark

        struct VBCM : public CodecControlCommon
        {
          // http://tools.ietf.org/html/rfc5104#section-4.3.4.1

          static const BYTE kFmt {7};

          BYTE zeroBit() const;
          BYTE payloadType() const;

          const BYTE *vbcmOctetString() const     {return mVBCMOctetString;}
          size_t vbcmOctetStringSize() const;

        public:
          const BYTE *mVBCMOctetString {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage::AFB
        #pragma mark

        struct AFB
        {
          // https://tools.ietf.org/html/rfc4585#section-6.4

          static const BYTE kFmt {15};

          const BYTE *data() const                  {return mData;}
          size_t dataSize() const                   {return mDataSize;}

        public:
          const BYTE *mData {};
          size_t mDataSize {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage::REMB
        #pragma mark

        struct REMB
        {
          // https://tools.ietf.org/html/draft-alvestrand-rmcat-remb-03#section-2.2

          static const BYTE kFmt {15};

          size_t numSSRC() const                    {return static_cast<size_t>(mNumSSRC);}
          BYTE brExp() const                        {return mBRExp;}
          DWORD brMantissa() const                  {return mBRMantissa;}

          DWORD ssrcAtIndex(size_t index) const;

        public:
          BYTE mNumSSRC {};
          BYTE mBRExp {};
          DWORD mBRMantissa {};
          DWORD *mSSRCs {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::PayloadSpecificFeedbackMessage (public)
        #pragma mark

        static const BYTE kPayloadType {206};

        PayloadSpecificFeedbackMessage *nextPayloadSpecificFeedbackMessage() const  {return mNextPayloadSpecificFeedbackMessage;}

        size_t sliCount() const                         {return mSLICount;}
        size_t firCount() const                         {return mFIRCount;}
        size_t tstrCount() const                        {return mTSTRCount;}
        size_t tstnCount() const                        {return mTSTNCount;}
        size_t vbcmCount() const                        {return mVBCMCount;}

        PLI *pli() const;
        SLI *sliAtIndex(size_t index) const;
        RPSI *rpsi() const;
        FIR *firAtIndex(size_t index) const;
        TSTR *tstrAtIndex(size_t index) const;
        TSTN *tstnAtIndex(size_t index) const;
        VBCM *vbcmAtIndex(size_t index) const;
        AFB *afb() const;
        REMB *remb() const;
        PayloadSpecificFeedbackMessage *unknown() const {return mUnknown;}

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
      #pragma mark
      #pragma mark RTCPPacket::XR
      #pragma mark

      struct XR : public Report
      {
        // https://tools.ietf.org/html/rfc3611#section-2
        // https://tools.ietf.org/html/rfc3611#section-3

        typedef WORD RLEChunk;

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::XR::ReportBlock
        #pragma mark

        struct ReportBlock
        {
          ReportBlock *next() const                           {return mNext;}

          BYTE blockType() const                              {return mBlockType;}
          BYTE typeSpecific() const                           {return mTypeSpecific;}

          const BYTE *typeSpecificContents() const            {return mTypeSpecificContents;}
          size_t typeSpecificContentSize() const              {return mTypeSpecificContentSize;}

          static const char *blockTypeToString(BYTE blockType);
          const char *blockTypeToString() const               {return blockTypeToString(mBlockType);}

        public:
          ReportBlock *mNext {};

          BYTE mBlockType {};
          BYTE mTypeSpecific {};

          const BYTE *mTypeSpecificContents {};
          size_t mTypeSpecificContentSize {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::XR::ReportBlockRange
        #pragma mark

        struct ReportBlockRange : public ReportBlock
        {
          BYTE reserved() const;
          BYTE thinning() const;

          DWORD ssrcOfSource() const                          {return mSSRCOfSource;}
          WORD beginSeq() const                               {return mBeginSeq;}
          WORD endSeq() const                                 {return mEndSeq;}

        public:
          DWORD mSSRCOfSource {};
          WORD mBeginSeq {};
          WORD mEndSeq {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::XR::RLEReportBlock
        #pragma mark

        struct RLEReportBlock : public ReportBlockRange
        {
          size_t chunkCount() const                           {return mChunkCount;}
          RLEChunk chunkAtIndex(size_t index) const;

        public:
          size_t mChunkCount {};
          RLEChunk *mChunks {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::XR::LossRLEReportBlock
        #pragma mark

        struct LossRLEReportBlock : public RLEReportBlock
        {
          // https://tools.ietf.org/html/rfc3611#section-4.1

          static const BYTE kBlockType {1};

          LossRLEReportBlock *nextLossRLE() const             {return mNextLossRLE;}

        public:
          LossRLEReportBlock *mNextLossRLE {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::XR::DuplicateRLEReportBlock
        #pragma mark

        struct DuplicateRLEReportBlock : public RLEReportBlock
        {
          // https://tools.ietf.org/html/rfc3611#section-4.2

          static const BYTE kBlockType {2};

          DuplicateRLEReportBlock *nextDuplicateRLE() const {return mNextDuplicateRLE;}

        public:
          DuplicateRLEReportBlock *mNextDuplicateRLE {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::XR::PacketReceiptTimesReportBlock
        #pragma mark

        struct PacketReceiptTimesReportBlock : public ReportBlockRange
        {
          // https://tools.ietf.org/html/rfc3611#section-4.3

          static const BYTE kBlockType {3};

          PacketReceiptTimesReportBlock *nextPacketReceiptTimesReportBlock() const  {return mNextPacketReceiptTimesReportBlock;}

          size_t receiptTimeCount() const                                           {return mReceiptTimeCount;}
          DWORD receiptTimeAtIndex(size_t index) const;

        public:
          PacketReceiptTimesReportBlock *mNextPacketReceiptTimesReportBlock {};

          size_t mReceiptTimeCount {};
          DWORD *mReceiptTimes {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::XR::ReceiverReferenceTimeReportBlock
        #pragma mark

        struct ReceiverReferenceTimeReportBlock : public ReportBlock
        {
          // https://tools.ietf.org/html/rfc3611#section-4.4

          static const BYTE kBlockType {4};

          ReceiverReferenceTimeReportBlock *nextReceiverReferenceTimeReportBlock() const  {return mNextReceiverReferenceTimeReportBlock;}

          DWORD ntpTimestampMS() const          {return mNTPTimestampMS;}
          DWORD ntpTimestampLS() const          {return mNTPTimestampLS;}
          Time ntpTimestamp() const;

        public:
          ReceiverReferenceTimeReportBlock *mNextReceiverReferenceTimeReportBlock {};

          DWORD mNTPTimestampMS {};
          DWORD mNTPTimestampLS {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::XR::DLRRReportBlock
        #pragma mark

        struct DLRRReportBlock : public ReportBlock
        {
          // https://tools.ietf.org/html/rfc3611#section-4.5

          static const BYTE kBlockType {5};

          struct SubBlock
          {
            DWORD ssrc() const  {return mSSRC;}
            DWORD lrr() const   {return mLRR;}
            DWORD dlrr() const  {return mDLRR;}

            DWORD mSSRC {};
            DWORD mLRR {};
            DWORD mDLRR {};
          };

          DLRRReportBlock *nextDLRRReportBlock() const {return mNextDLRRReportBlock;}

          size_t subBlockCount() const {return mSubBlockCount;}
          SubBlock *subBlockAtIndex(size_t index) const;

        public:
          DLRRReportBlock *mNextDLRRReportBlock {};

          size_t mSubBlockCount {};
          SubBlock *mSubBlocks {};
        };

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::XR::StatisticsSummaryReportBlock
        #pragma mark

        struct StatisticsSummaryReportBlock : public ReportBlockRange
        {
          // https://tools.ietf.org/html/rfc3611#section-4.6

          static const BYTE kBlockType {6};

          StatisticsSummaryReportBlock *nextStatisticsSummaryReportBlock() const {return mNextStatisticsSummaryReportBlock;}

          bool lossReportFlag() const;
          bool duplicateReportFlag() const;
          bool jitterFlag() const;
          bool ttlFlag() const;
          bool hopLimitFlag() const;

          DWORD lostPackets() const             {return mLostPackets;}
          DWORD dupPackets() const              {return mDupPackets;}

          DWORD minJitter() const               {return mMinJitter;}
          DWORD maxJitter() const               {return mMaxJitter;}
          DWORD meanJitter() const              {return mMeanJitter;}
          DWORD devJitter() const               {return mDevJitter;}

          BYTE minTTL() const                   {return mMinTTLOrHL;}
          BYTE maxTTL() const                   {return mMaxTTLOrHL;}
          BYTE meanTTL() const                  {return mMeanTTLOrHL;}
          BYTE devTTL() const                   {return mDevTTLOrHL;}

          BYTE minHopLimit() const              {return mMinTTLOrHL;}
          BYTE maxHopLimit() const              {return mMaxTTLOrHL;}
          BYTE meanHopLimit() const             {return mMeanTTLOrHL;}
          BYTE devHopLimit() const              {return mDevTTLOrHL;}

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
        #pragma mark
        #pragma mark RTCPPacket::XR::VoIPMetricsReportBlock
        #pragma mark

        struct VoIPMetricsReportBlock : public ReportBlock
        {
          // https://tools.ietf.org/html/rfc3611#section-4.7

          static const BYTE kBlockType {7};

          VoIPMetricsReportBlock *nextVoIPMetricsReportBlock() const {return mNextVoIPMetricsReportBlock;}

          DWORD ssrcOfSource() const              {return mSSRCOfSource;}

          BYTE lossRate() const                   {return mLossRate;}
          BYTE discardRate() const                {return mDiscardRate;}
          BYTE burstDensity() const               {return mBurstDensity;}
          BYTE gapDensity() const                 {return mGapDensity;}

          WORD burstDuration() const              {return mBurstDuration;}
          WORD gapDuration() const                {return mGapDuration;}

          WORD roundTripDelay() const             {return mRoundTripDelay;}
          WORD endSystemDelay() const             {return mEndSystemDelay;}

          BYTE signalLevel() const                {return mSignalLevel;}
          BYTE noiseLevel() const                 {return mNoiseLevel;}
          BYTE rerl() const                       {return mRERL;}
          BYTE Gmin() const                       {return mGmin;}

          BYTE rFactor() const                    {return mRFactor;}
          BYTE extRFactor() const                 {return mExtRFactor;}
          BYTE mosLQ() const                      {return mMOSLQ;}
          BYTE mosCQ() const                      {return mMOSCQ;}

          BYTE rxConfig() const                   {return mRXConfig;}
          BYTE plc() const;
          BYTE jba() const;
          BYTE jbRate() const;

          WORD jbNominal() const                  {return mJBNominal;}
          WORD jbMaximum() const                  {return mJBMaximum;}
          WORD jbAbsMax() const                   {return mJBAbsMax;}

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
        #pragma mark
        #pragma mark RTCPPacket::XR::UnknownReportBlock
        #pragma mark

        struct UnknownReportBlock : public ReportBlock
        {
          UnknownReportBlock *nextUnknownReportBlock() const {return mNextUnknownReportBlock;}

        public:
          UnknownReportBlock *mNextUnknownReportBlock {};
        };

        //-------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::XR::RunLength
        #pragma mark

        struct RunLength
        {
          // https://tools.ietf.org/html/rfc3611#section-4.1.1

          RunLength(RLEChunk chunk);

          BYTE runType() const {return mRunType;}
          size_t runLength() const {return static_cast<size_t>(mRunLength);}

        public:
          BYTE mRunType {};   // Zero indicates a run of 0s.  One indicates a run of 1s.
          WORD mRunLength {}; // A value between 1 and 16,383.
        };

        //-------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::XR::BitVector
        #pragma mark

        struct BitVector
        {
          // https://tools.ietf.org/html/rfc3611#section-4.1.2

          BitVector(RLEChunk chunk);

          BYTE bitAtIndex(size_t index) const;  // Zero indicates loss.

        public:
          WORD mBitVector {};
        };

        //-------------------------------------------------------------------
        #pragma mark
        #pragma mark RTCPPacket::XR
        #pragma mark

        static const BYTE kPayloadType {207};

        XR *nextXR() const                                                              {return mNextXR;}

        BYTE reserved() const                                                           {return mReportSpecific;}

        DWORD ssrc() const                                                              {return mSSRC;}

        ReportBlock *firstReportBlock() const                                           {return mFirstReportBlock;}

        LossRLEReportBlock *firstLossRLEReportBlock() const                             {return mFirstLossRLEReportBlock;}
        DuplicateRLEReportBlock *firstDuplicateRLEReportBlock() const                   {return mFirstDuplicateRLEReportBlock;}
        PacketReceiptTimesReportBlock *firstPacketReceiptTimesReportBlock() const       {return mFirstPacketReceiptTimesReportBlock;}
        ReceiverReferenceTimeReportBlock *firstReceiverReferenceTimeReportBlock() const {return mFirstReceiverReferenceTimeReportBlock;}
        DLRRReportBlock *firstDLRRReportBlock() const                                   {return mFirstDLRRReportBlock;}
        StatisticsSummaryReportBlock *firstStatisticsSummaryReportBlock() const         {return mFirstStatisticsSummaryReportBlock;}
        VoIPMetricsReportBlock *firstVoIPMetricsReportBlock() const                     {return mFirstVoIPMetricsReportBlock;}
        UnknownReportBlock *firstUnknownReportBlock() const                             {return mFirstUnknownReportBlock;}

        size_t reportBlockCount() const                                                 {return mReportBlockCount;}

        size_t lossRLEReportBlockCount() const                                          {return mLossRLEReportBlockCount;}
        size_t duplicateRLEReportBlockCount() const                                     {return mDuplicateRLEReportBlockCount;}
        size_t packetReceiptTimesReportBlockCount() const                               {return mPacketReceiptTimesReportBlockCount;}
        size_t receiverReferenceTimeReportBlockCount() const                            {return mReceiverReferenceTimeReportBlockCount;}
        size_t dlrrReportBlockCount() const                                             {return mDLRRReportBlockCount;}
        size_t statisticsSummaryReportBlockCount() const                                {return mStatisticsSummaryReportBlockCount;}
        size_t voIPMetricsReportBlockCount() const                                      {return mVoIPMetricsReportBlockCount;}
        size_t unknownReportBlockCount() const                                          {return mUnknownReportBlockCount;}

        LossRLEReportBlock *lossRLEReportBlockAtIndex(size_t index) const;
        DuplicateRLEReportBlock *duplicateRLEReportBlockAtIndex(size_t index) const;
        PacketReceiptTimesReportBlock *packetReceiptTimesReportBlockAtIndex(size_t index) const;
        ReceiverReferenceTimeReportBlock *receiverReferenceTimeReportBlockAtIndex(size_t index) const;
        DLRRReportBlock *dlrrReportBlockAtIndex(size_t index) const;
        StatisticsSummaryReportBlock *statisticsSummaryReportBlockAtIndex(size_t index) const;
        VoIPMetricsReportBlock *voIPMetricsReportBlockAtIndex(size_t index) const;
        UnknownReportBlock *unknownReportBlockAtIndex(size_t index) const;

        static bool isRunLengthChunk(RLEChunk chunk);
        static bool isBitVectorChunk(RLEChunk chunk);

        static RunLength runLength(RLEChunk chunk)                                      {return RunLength(chunk);}
        static BitVector bitVector(RLEChunk chunk)                                      {return BitVector(chunk);}

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
      #pragma mark
      #pragma mark RTCPPacket::UnknownReport
      #pragma mark

      struct UnknownReport : public Report
      {
        UnknownReport *nextUnknown() const          {return mNextUnknown;}

      public:
        UnknownReport *mNextUnknown {};
      };

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark (public)
      #pragma mark

      RTCPPacket(const make_private &);
      ~RTCPPacket();

      static RTCPPacketPtr create(const BYTE *buffer, size_t bufferLengthInBytes);
      static RTCPPacketPtr create(const SecureByteBlock &buffer);
      static RTCPPacketPtr create(SecureByteBlockPtr buffer);  // NOTE: ownership of buffer is taken
      static RTCPPacketPtr create(const Report *first);
      static SecureByteBlockPtr generateFrom(const Report *first);

      const BYTE *ptr() const;
      size_t size() const;
      SecureByteBlockPtr buffer() const;

      Report *first() const                                                       {return mFirst;}

      SenderReport *firstSenderReport() const                                     {return mFirstSenderReport;}
      ReceiverReport *firstReceiverReport() const                                 {return mFirstReceiverReport;}
      SDES *firstSDES() const                                                     {return mFirstSDES;}
      Bye *firstBye() const                                                       {return mFirstBye;}
      App *firstApp() const                                                       {return mFirstApp;}
      TransportLayerFeedbackMessage *firstTransportLayerFeedbackMessage() const   {return mFirstTransportLayerFeedbackMessage;}
      PayloadSpecificFeedbackMessage *firstPayloadSpecificFeedbackMessage() const {return mFirstPayloadSpecificFeedbackMessage;}
      XR *firstXR() const                                                         {return mFirstXR;}
      UnknownReport *firstUnknownReport() const                                   {return mFirstUnknownReport;}

      size_t count() const                                                        {return mCount;}

      size_t senderReportCount() const                                            {return mSenderReportCount;}
      size_t receiverReportCount() const                                          {return mReceiverReportCount;}
      size_t sdesCount() const                                                    {return mSDESCount;}
      size_t byeCount() const                                                     {return mByeCount;}
      size_t appCount() const                                                     {return mAppCount;}
      size_t transportLayerFeedbackMessageCount() const                           {return mTransportLayerFeedbackMessageCount;}
      size_t payloadSpecificFeedbackMessage() const                               {return mPayloadSpecificFeedbackMessageCount;}
      size_t xrCount() const                                                      {return mXRCount;}
      size_t unknownReportCount() const                                           {return mUnknownReportCount;}
      
      SenderReport *senderReportAtIndex(size_t index) const;
      ReceiverReport *receiverReportAtIndex(size_t index) const;
      SDES *sdesAtIndex(size_t index) const;
      Bye *byeAtIndex(size_t index) const;
      App *appAtIndex(size_t index) const;
      TransportLayerFeedbackMessage *transportLayerFeedbackReportAtIndex(size_t index) const;
      PayloadSpecificFeedbackMessage *payloadSpecificFeedbackReportAtIndex(size_t index) const;
      XR *xrAtIndex(size_t index) const;
      UnknownReport *unknownAtIndex(size_t index) const;

      ElementPtr toDebug() const;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark (internal)
      #pragma mark

      static Log::Params slog(const char *message);
      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;

      bool parse();

      bool getAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, BYTE pt, const BYTE *contents, size_t contentSize);
      bool getSenderReportAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize);
      bool getReceiverReportAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize);
      bool getSDESAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize);
      bool getByeAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize);
      bool getAppAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize);
      bool getTransportLayerFeedbackMessageAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize);
      bool getPayloadSpecificFeedbackMessageAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize);
      bool getXRAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize);
      bool getUnknownReportAllocationSize(BYTE version, BYTE padding, BYTE reportSpecific, const BYTE *contents, size_t contentSize);

      bool getTransportLayerFeedbackMessageGenericNACKAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize);
      bool getTransportLayerFeedbackMessageTMMBRAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize);
      bool getTransportLayerFeedbackMessageTMMBNAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize);

      bool getPayloadSpecificFeedbackMessagePLIAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize);
      bool getPayloadSpecificFeedbackMessageSLIAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize);
      bool getPayloadSpecificFeedbackMessageRPSIAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize);
      bool getPayloadSpecificFeedbackMessageFIRAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize);
      bool getPayloadSpecificFeedbackMessageTSTRAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize);
      bool getPayloadSpecificFeedbackMessageTSTNAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize);
      bool getPayloadSpecificFeedbackMessageVBCMAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize);
      bool getPayloadSpecificFeedbackMessageAFBAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize);
      bool getPayloadSpecificFeedbackMessageREMBAllocationSize(BYTE fmt, const BYTE *contents, size_t contentSize);

      bool getXRLossRLEReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize);
      bool getXRDuplicateRLEReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize);
      bool getXRPacketReceiptTimesReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize);
      bool getXRReceiverReferenceTimeReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize);
      bool getXRDLRRReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize);
      bool getXRStatisticsSummaryReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize);
      bool getXRVoIPMetricsReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize);
      bool getXRUnknownReportBlockAllocationSize(BYTE typeSpecific, const BYTE *contents, size_t contentSize);

      bool parse(Report * &ioLastReport, BYTE version, BYTE padding, BYTE reportSpecific, BYTE pt, const BYTE *contents, size_t contentSize);
      void fill(Report *report, BYTE version, BYTE padding, BYTE reportSpecific, BYTE pt, const BYTE *contents, size_t contentSize);

      bool parseCommon(
                       SenderReceiverCommonReport *report,
                       size_t detailedHeaderSize
                       );
      bool parse(SenderReport *report);
      bool parse(ReceiverReport *report);
      bool parse(SDES *report);
      bool parse(Bye *report);
      bool parse(App *report);
      bool parse(TransportLayerFeedbackMessage *report);
      bool parse(PayloadSpecificFeedbackMessage *report);
      bool parse(XR *report);
      bool parse(UnknownReport *report);

      void fill(FeedbackMessage *report, const BYTE *contents, size_t contentSize);
      void fill(XR *report, XR::ReportBlock *reportBlock, XR::ReportBlock * &ioPreviousReportBlock, BYTE blockType, BYTE typeSpecific, const BYTE *contents, size_t contentSize);

      bool parseGenericNACK(TransportLayerFeedbackMessage *report);
      void fillTMMBRCommon(TransportLayerFeedbackMessage *report, TransportLayerFeedbackMessage::TMMBRCommon *common, const BYTE *pos);
      bool parseTMMBR(TransportLayerFeedbackMessage *report);
      bool parseTMMBN(TransportLayerFeedbackMessage *report);
      bool parseUnknown(TransportLayerFeedbackMessage *report);

      //CodecControlCommon
      bool parsePLI(PayloadSpecificFeedbackMessage *report);
      bool parseSLI(PayloadSpecificFeedbackMessage *report);
      bool parseRPSI(PayloadSpecificFeedbackMessage *report);
      void fillCodecControlCommon(PayloadSpecificFeedbackMessage *report, PayloadSpecificFeedbackMessage::CodecControlCommon *common, const BYTE *pos);
      bool parseFIR(PayloadSpecificFeedbackMessage *report);
      bool parseTSTR(PayloadSpecificFeedbackMessage *report);
      bool parseTSTN(PayloadSpecificFeedbackMessage *report);
      bool parseVBCM(PayloadSpecificFeedbackMessage *report);
      bool parseAFB(PayloadSpecificFeedbackMessage *report);
      bool parseREMB(PayloadSpecificFeedbackMessage *report);
      bool parseUnknown(PayloadSpecificFeedbackMessage *report);

      bool parseCommonRange(XR *xr, XR::ReportBlockRange *reportBlock);
      bool parseCommonRLE(XR *xr, XR::RLEReportBlock *reportBlock);
      bool parse(XR *xr, XR::LossRLEReportBlock *reportBlock);
      bool parse(XR *xr, XR::DuplicateRLEReportBlock *reportBlock);
      bool parse(XR *xr, XR::PacketReceiptTimesReportBlock *reportBlock);
      bool parse(XR *xr, XR::ReceiverReferenceTimeReportBlock *reportBlock);
      bool parse(XR *xr, XR::DLRRReportBlock *reportBlock);
      bool parse(XR *xr, XR::StatisticsSummaryReportBlock *reportBlock);
      bool parse(XR *xr, XR::VoIPMetricsReportBlock *reportBlock);
      bool parse(XR *xr, XR::UnknownReportBlock *reportBlock);

      void *allocateBuffer(size_t size);

      static size_t getPacketSize(const Report *first);
      static void writePacket(const Report *first, BYTE * &ioPos, size_t &ioRemaining);

    public:
      SecureByteBlockPtr mBuffer;
      SecureByteBlockPtr mAllocationBuffer;

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

  }
}

