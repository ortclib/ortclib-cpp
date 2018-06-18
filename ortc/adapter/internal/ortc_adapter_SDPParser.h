/*

 Copyright (c) 2016, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/adapter/internal/types.h>
#include <ortc/adapter/ISessionDescription.h>

namespace ortc
{
  namespace adapter
  {
    namespace internal
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // ISDPTypes
      //

      interaction ISDPTypes
      {
        ZS_DECLARE_TYPEDEF_PTR(ISessionDescriptionTypes::Description, Description);

        typedef BYTE PayloadType;
        typedef DWORD SSRCType;

        typedef std::list<SSRCType> SSRCList;
        typedef std::list<PayloadType> PayloadTypeList;

        typedef std::pair<String, String> KeyValuePair;
        typedef std::list<KeyValuePair> KeyValueList;

        typedef String BundleID;
        typedef std::set<BundleID> BundledSet;
        typedef std::map<BundleID, BundledSet> BundleMap;

        ZS_DECLARE_TYPEDEF_PTR(std::list<String>, StringList);

        ZS_DECLARE_STRUCT_PTR(LineTypeInfo);
        ZS_DECLARE_STRUCT_PTR(SDP);
        ZS_DECLARE_STRUCT_PTR(LineValue);
        ZS_DECLARE_STRUCT_PTR(MediaLine);
        ZS_DECLARE_STRUCT_PTR(VLine);
        ZS_DECLARE_STRUCT_PTR(OLine);
        ZS_DECLARE_STRUCT_PTR(SLine);
        ZS_DECLARE_STRUCT_PTR(BLine);
        ZS_DECLARE_STRUCT_PTR(TLine);
        ZS_DECLARE_STRUCT_PTR(MLine);
        ZS_DECLARE_STRUCT_PTR(CLine);

        ZS_DECLARE_STRUCT_PTR(AMediaFlagLine);

        ZS_DECLARE_STRUCT_PTR(AGroupLine);
        ZS_DECLARE_STRUCT_PTR(AMSIDLine);
        ZS_DECLARE_STRUCT_PTR(AMSIDSemanticLine);
        ZS_DECLARE_STRUCT_PTR(AICEUFragLine);
        ZS_DECLARE_STRUCT_PTR(AICEPwdLine);
        ZS_DECLARE_STRUCT_PTR(AICEOptionsLine);
        ZS_DECLARE_STRUCT_PTR(AFingerprintLine);
        ZS_DECLARE_STRUCT_PTR(AExtmapLine);
        ZS_DECLARE_STRUCT_PTR(ACandidateLine);
        ZS_DECLARE_STRUCT_PTR(ACryptoLine);
        ZS_DECLARE_STRUCT_PTR(ASetupLine);
        ZS_DECLARE_STRUCT_PTR(AMIDLine);
        ZS_DECLARE_STRUCT_PTR(ARTPMapLine);
        ZS_DECLARE_STRUCT_PTR(AFMTPLine);
        ZS_DECLARE_STRUCT_PTR(ARTCPLine);
        ZS_DECLARE_STRUCT_PTR(ARTCPFBLine);
        ZS_DECLARE_STRUCT_PTR(APTimeLine);
        ZS_DECLARE_STRUCT_PTR(AMaxPTimeLine);
        ZS_DECLARE_STRUCT_PTR(ASSRCLine);
        ZS_DECLARE_STRUCT_PTR(ASSRCGroupLine);
        ZS_DECLARE_STRUCT_PTR(ASimulcastLine);
        ZS_DECLARE_STRUCT_PTR(ARIDLine);
        ZS_DECLARE_STRUCT_PTR(ASCTPPortLine);
        ZS_DECLARE_STRUCT_PTR(AMaxMessageSizeLine);

        ZS_DECLARE_TYPEDEF_PTR(std::list<LineTypeInfo>, LineTypeInfoList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<BLinePtr>, BLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<MLinePtr>, MediaLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<AGroupLinePtr>, AGroupLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<AMSIDSemanticLinePtr>, AMSIDSemanticLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<AMSIDLinePtr>, AMSIDLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<AFingerprintLinePtr>, AFingerprintLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<AExtmapLinePtr>, AExtmapLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<ACandidateLinePtr>, ACandidateLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<ACryptoLinePtr>, ACryptoLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<ARTPMapLinePtr>, ARTPMapLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<AFMTPLinePtr>, AFMTPLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<ARTCPFBLinePtr>, ARTCPFBLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<ASSRCLinePtr>, ASSRCLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<ASSRCGroupLinePtr>, ASSRCGroupLineList);
        ZS_DECLARE_TYPEDEF_PTR(std::list<ARIDLinePtr>, ARIDLineList);

        enum LineTypes
        {
          LineType_Unknown = '*',

          LineType_v_Version = 'v',
          LineType_o_Origin = 'o',
          LineType_s_SessionName = 's',
          LineType_b_Bandwidth = 'b',
          LineType_t_Timing = 't',
          LineType_a_Attributes = 'a',
          LineType_m_MediaLine = 'm',
          LineType_c_ConnectionDataLine = 'c',
        };

        static const char *toString(LineTypes type) noexcept(false);
        static LineTypes toLineType(const char *type) noexcept;
        static LineTypes toLineType(const char type) noexcept;

        enum Attributes
        {
          Attribute_First,

          Attribute_Unknown = Attribute_First,

          Attribute_Group,
          Attribute_BundleOnly,
          Attribute_MSID,
          Attribute_MSIDSemantic,
          Attribute_ICEUFrag,
          Attribute_ICEPwd,
          Attribute_ICEOptions,
          Attribute_ICELite,
          Attribute_Candidate,
          Attribute_EndOfCandidates,
          Attribute_Fingerprint,
          Attribute_Crypto,
          Attribute_Setup,
          Attribute_MID,
          Attribute_Extmap,
          Attribute_SendRecv,
          Attribute_SendOnly,
          Attribute_RecvOnly,
          Attribute_Inactive,
          Attribute_RTPMap,
          Attirbute_FMTP,
          Attribute_RTCP,
          Attribute_RTCPMux,
          Attribute_RTCPFB,
          Attribute_RTCPRSize,
          Attribute_PTime,
          Attribute_MaxPTime,
          Attribute_SSRC,
          Attribute_SSRCGroup,
          Attribute_Simulcast,
          Attribute_RID,
          Attribute_SCTPPort,
          Attribute_MaxMessageSize,

          Attribute_Last = Attribute_MaxMessageSize,
        };

        static const char *toString(Attributes attribute) noexcept;
        static Attributes toAttribute(const char *attribute) noexcept;

        static bool requiresValue(Attributes attribute) noexcept;
        static bool requiresEmptyValue(Attributes attribute) noexcept;

        enum AttributeLevels
        {
          AttributeLevel_None             = 0x0,
          AttributeLevel_Session          = 0x1,
          AttributeLevel_Media            = 0x2,
          AttributeLevel_Source           = 0x4,

          AttributeLevel_SessionAndMedia  = AttributeLevel_Session | AttributeLevel_Media,
          AttributeLevel_SessionAndSource = AttributeLevel_Session | AttributeLevel_Source,
          AttributeLevel_MediaAndSource   = AttributeLevel_Media | AttributeLevel_Source,
          AttributeLevel_All              = AttributeLevel_Session | AttributeLevel_Media | AttributeLevel_Source,
        };

        static const char *toString(AttributeLevels level) noexcept;
        static AttributeLevels toAttributeLevel(const char *level) noexcept(false);

        static AttributeLevels getAttributeLevel(LineTypes lineType) noexcept;
        static AttributeLevels getAttributeLevel(Attributes attribute) noexcept;

        static bool supportedAtLevel(
                                     AttributeLevels currentLevel,
                                     LineTypes lineType
                                     ) noexcept;
        static bool supportedAtLevel(
                                     AttributeLevels currentLevel,
                                     Attributes attribute
                                     ) noexcept;

        enum Locations
        {
          Location_First,

          Location_Local = Location_First,
          Location_Remote,

          Location_Last = Location_Remote,
        };

        static const char *toString(Locations location) noexcept;
        static Locations toLocation(const char *location) noexcept(false);

        enum Directions
        {
          Direction_None        = 0x00,
          Direction_Send        = 0x01,
          Direction_Receive     = 0x02,

          Direction_SendReceive = Direction_Send | Direction_Receive,
        };

        static const char *toStringForA(Directions direction) noexcept;
        static const char *toStringForRID(Directions direction) noexcept;
        static Directions toDirection(const char *direction) noexcept(false);
        static bool isValid(
                            Directions direction,
                            bool allowNone,
                            bool allowSend,
                            bool allowReceive,
                            bool allowSendReceive
                            ) noexcept;

        enum ActorRoles
        {
          ActorRole_Sender      = 0x01,
          ActorRole_Receiver    = 0x02,
          ActorRole_Transceiver = ActorRole_Sender | ActorRole_Receiver,
        };

        static const char *toString(ActorRoles actor) noexcept;
        static ActorRoles toActorRole(const char *actor) noexcept(false);

        static bool isApplicable(
                                 ActorRoles actor,
                                 Locations location,
                                 Directions direction
                                 ) noexcept;

        enum ProtocolTypes
        {
          ProtocolType_First,

          ProtocolType_Unknown = ProtocolType_First,

          ProtocolType_RTP,
          ProtocolType_SCTP,

          ProtocolType_Last = ProtocolType_SCTP,
        };

        static const char *toString(ProtocolTypes proto) noexcept;
        static ProtocolTypes toProtocolType(const char *proto) noexcept(false);

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------

        struct LineValue : public Any
        {
        };

        struct MediaLine : public LineValue
        {
          MLinePtr mMLine;

          MediaLine(MLinePtr mline) noexcept : mMLine(mline) {}
        };

        // https://tools.ietf.org/html/rfc4566#section-5.1
        struct VLine : public LineValue
        {
          ULONGLONG mVersion {};

          VLine(const char *value) noexcept(false);
        };

        // https://tools.ietf.org/html/rfc4566#section-5.2
        struct OLine : public LineValue
        {
          String mUsername {"-"};
          ULONGLONG mSessionID {};
          ULONGLONG mSessionVersion {};
          String mNetType;
          String mAddrType;
          String mUnicastAddress;

          OLine(const Noop &) noexcept {}
          OLine(const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const OLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc4566#section-5.3
        struct SLine : public LineValue
        {
          String mSessionName;

          SLine(const Noop &) noexcept {}
          SLine(const char *value) noexcept;
          String toString() const noexcept { return mSessionName.hasData() ? mSessionName : String("-"); }
          static String toString(const SLinePtr &line) noexcept { if (!line) return String("-"); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc4566#section-5.8
        struct BLine : public MediaLine
        {
          String mBWType;
          ULONGLONG mBandwidth {};

          BLine(MLinePtr mline, const char *value) noexcept(false);
        };

        // https://tools.ietf.org/html/rfc4566#section-5.9
        struct TLine : public LineValue
        {
          QWORD mStartTime {};
          QWORD mEndTime {};

          TLine(const Noop &) noexcept {}
          TLine(const char *value) noexcept(false);
          String toString() const noexcept { return string(mStartTime) + " " + string(mEndTime); }
          static String toString(const TLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc4566#section-5.14
        struct MLine : public LineValue
        {
          // data
          String mMedia;
          ULONGLONG mPort {};
          ULONGLONG mInteger {};
          ProtocolTypes mProto {ProtocolType_Unknown};
          String mProtoStr;
          StringList mFmts;

          // media level flags
          Optional<bool> mBundleOnly;
          Optional<bool> mEndOfCandidates;
          Optional<Directions> mMediaDirection;
          Optional<bool> mRTCPMux;
          Optional<bool> mRTCPRSize;

          // media level values
          CLinePtr mCLine;
          AICEUFragLinePtr mAICEUFragLine;
          AICEPwdLinePtr mAICEPwdLine;
          ASetupLinePtr mASetupLine;
          AMIDLinePtr mAMIDLine;
          ARTCPLinePtr mARTCPLine;
          APTimeLinePtr mAPTimeLine;
          AMaxPTimeLinePtr mAMaxPTimeLine;
          ASimulcastLinePtr mASimulcastLine;
          ASCTPPortLinePtr mASCTPPortLine;
          AMaxMessageSizeLinePtr mAMaxMessageSize;

          BLineList mBLines;
          AMSIDLineList mAMSIDLines;
          ACandidateLineList mACandidateLines;
          AFingerprintLineList mAFingerprintLines;
          ACryptoLineList mACryptoLines;
          AExtmapLineList mAExtmapLines;
          ARTPMapLineList mARTPMapLines;
          AFMTPLineList mAFMTPLines;
          ARTCPFBLineList mARTCPFBLines;
          ASSRCLineList mASSRCLines;
          ASSRCGroupLineList mASSRCGroupLines;
          ARIDLineList mARIDLines;

          MLine(const Noop &) noexcept {}
          MLine(const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const MLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc4566#section-5.7
        struct CLine : public MediaLine
        {
          String mNetType;
          String mAddrType;
          String mConnectionAddress;

          CLine(const Noop &) noexcept : MediaLine(nullptr) {}
          CLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const CLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------

        // https://tools.ietf.org/html/rfc4566#section-5.13
        struct ALine : public LineValue
        {
        };

        struct AMediaLine : public ALine
        {
          MLinePtr mMLine;

          AMediaLine(MLinePtr mline) noexcept : mMLine(mline) {}
        };

        struct AMediaFlagLine : public AMediaLine
        {
          AMediaFlagLine(MLinePtr mline) noexcept : AMediaLine(mline) {}
        };

        // https://tools.ietf.org/html/rfc5888#section-5
        struct AGroupLine : public ALine
        {
          String mSemantic;
          StringList mIdentificationTags;

          AGroupLine(const Noop &) noexcept {}
          AGroupLine(const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const AGroupLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/draft-ietf-mmusic-msid-13#section-2
        struct AMSIDLine : public AMediaLine
        {
          String mID;
          String mAppData;

          AMSIDLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          AMSIDLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const AMSIDLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/draft-ietf-mmusic-msid-13#appendix-B.5
        // a=msid-semantic: WMS CESG4hDt2y4kkEizBkB4vbj7GQpC6hF7XTLR
        struct AMSIDSemanticLine : public AGroupLine
        {
          AMSIDSemanticLine(const char *value) noexcept : AGroupLine(value) {}
        };

        // https://tools.ietf.org/html/rfc5245#section-15.4
        struct AICEUFragLine : public AMediaLine
        {
          String mICEUFrag;

          AICEUFragLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          AICEUFragLine(MLinePtr mline, const char *value) noexcept;
          String toString() const noexcept { return "ice-ufrag:" + mICEUFrag; }
          static String toString(const AICEUFragLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc5245#section-15.4
        struct AICEPwdLine : public AMediaLine
        {
          String mICEPwd;

          AICEPwdLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          AICEPwdLine(MLinePtr mline, const char *value) noexcept;
          String toString() const noexcept { return "ice-pwd:" + mICEPwd; }
          static String toString(const AICEPwdLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc5245#section-15.5
        struct AICEOptionsLine : public AMediaLine
        {
          StringList mTags;

          AICEOptionsLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          AICEOptionsLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const AICEOptionsLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc5245#section-15.1
        struct ACandidateLine : public AMediaLine
        {
          typedef String ExtensionAttName;
          typedef String ExtensionAttValue;
          typedef std::pair<ExtensionAttName, ExtensionAttValue> ExtensionPair;
          typedef std::list<ExtensionPair> ExtensionPairList;

          String mFoundation;
          WORD mComponentID {};
          String mTransport;
          DWORD mPriority {};
          String mConnectionAddress;
          WORD mPort {};
          String mTyp {"typ"};  // literal string value
          String mCandidateType;
          String mRelAddr;
          Optional<WORD> mRelPort;

          ExtensionPairList mExtensionPairs;

          ACandidateLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          ACandidateLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const ACandidateLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc4572#section-5
        struct AFingerprintLine : public AMediaLine
        {
          String mHashFunc;
          String mFingerprint;

          AFingerprintLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          AFingerprintLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const AFingerprintLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc4568#section-9.1
        struct ACryptoLine : public AMediaLine
        {
          typedef String KeyMethod;
          typedef String KeyInfo;
          typedef std::pair<KeyMethod, KeyInfo> KeyParam;
          typedef std::list<KeyParam> KeyParamList;

          ULONG mTag {};
          String mCryptoSuite;
          KeyParamList mKeyParams;
          StringList mSessionParams;

          ACryptoLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          ACryptoLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const ACryptoLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc4145#section-4
        // https://tools.ietf.org/html/rfc5763#section-5
        struct ASetupLine : public AMediaLine
        {
          String mSetup;

          ASetupLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          ASetupLine(MLinePtr mline, const char *value) noexcept;
          String toString() const noexcept { return "setup:" + mSetup; }
          static String toString(const ASetupLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc5888#section-9.1
        struct AMIDLine : public AMediaLine
        {
          String mMID;

          AMIDLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          AMIDLine(MLinePtr mline, const char *value) noexcept;
          String toString() const noexcept { return "mid:" + mMID; }
          static String toString(const AMIDLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc5285#section-7
        struct AExtmapLine : public AMediaLine
        {
          WORD mID {};
          Directions mDirection {Direction_None};
          String mURI;
          String mExtensionAttributes;

          AExtmapLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          AExtmapLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const AExtmapLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc4566#section-6
        struct AMediaDirectionLine : public AMediaLine
        {
          Directions mDirection {Direction_None};

          AMediaDirectionLine(MLinePtr mline, const char *value) noexcept(false);
        };

        // https://tools.ietf.org/html/rfc4566#section-6
        struct ARTPMapLine : public AMediaLine
        {
          PayloadType mPayloadType {};
          String mEncodingName;
          ULONG mClockRate {};
          Optional<ULONG> mEncodingParameters;

          ARTPMapLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          ARTPMapLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const ARTPMapLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc4566#section-6
        // https://tools.ietf.org/html/rfc4566#section-9
        struct AFMTPLine : public AMediaLine
        {
          ASSRCLinePtr mSourceLine;

          PayloadType mFormat {};
          String mFormatStr;
          StringList mFormatSpecific;

          AFMTPLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          AFMTPLine(MLinePtr mline, ASSRCLinePtr sourceLine, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const AFMTPLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc3605#section-2.1
        struct ARTCPLine : public AMediaLine
        {
          WORD mPort {};
          String mNetType;
          String mAddrType;
          String mConnectionAddress;

          ARTCPLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          ARTCPLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const ARTCPLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc4585#section-4.2
        struct ARTCPFBLine : public AMediaLine
        {
          Optional<PayloadType> mPayloadType;  // if not set, then applies to all
          String mID;
          String mParam1;
          String mParam2;

          ARTCPFBLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          ARTCPFBLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const ARTCPFBLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc4566#section-6
        struct APTimeLine : public AMediaLine
        {
          Milliseconds mPTime {};

          APTimeLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          APTimeLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept { return "ptime:" + string(mPTime.count()); }
          static String toString(const APTimeLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc4566#section-6
        struct AMaxPTimeLine : public AMediaLine
        {
          Milliseconds mMaxPTime {};

          AMaxPTimeLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          AMaxPTimeLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept { return "maxptime:" + string(mMaxPTime.count()); }
          static String toString(const AMaxPTimeLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc5576#section-4.1
        struct ASSRCLine : public AMediaLine
        {
          SSRCType mSSRC {};

          KeyValueList mAttributeValues;

          AFMTPLineList mAFMTPLines;

          ASSRCLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          ASSRCLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const ASSRCLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/rfc5576#section-4.2
        struct ASSRCGroupLine : public AMediaLine
        {
          String mSemantics;
          SSRCList mSSRCs;

          ASSRCGroupLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          ASSRCGroupLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const ASSRCGroupLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/draft-ietf-mmusic-rid-05#section-10
        struct ARIDLine : public AMediaLine
        {
          typedef String RIDParamKey;
          typedef String RIDParamValue;
          typedef std::pair<RIDParamKey, RIDParamValue> RIDParam;
          typedef std::list<RIDParam> RIDParamList;

          String mID;
          Directions mDirection {Direction_None};
          PayloadTypeList mPayloadTypes;
          RIDParamList mParams;

          ARIDLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          ARIDLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept;
          static String toString(const ARIDLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/draft-ietf-mmusic-sdp-simulcast-04#section-6.1
        struct ASimulcastLine : public AMediaLine
        {
          struct SCID
          {
            Optional<bool> mPaused;
            String mRID;
          };
          typedef std::list<SCID> SCIDList;
          typedef std::list<SCIDList> AltSCIDList;

          struct SCValue
          {
            Directions mDirection {Direction_None};
            AltSCIDList mAltSCIDs;
          };
          typedef std::list<SCValue> SCValueList;

          SCValueList mValues;

          ASimulcastLine(MLinePtr mline, const char *value) noexcept(false);
        };

        // https://tools.ietf.org/html/draft-ietf-mmusic-sctp-sdp-16#section-5.2
        struct ASCTPPortLine : public AMediaLine
        {
          WORD mPort {};

          ASCTPPortLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          ASCTPPortLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept { return "sctp-port:" +string(mPort); }
          static String toString(const ASCTPPortLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        // https://tools.ietf.org/html/draft-ietf-mmusic-sctp-sdp-16#section-6.2
        struct AMaxMessageSizeLine : public AMediaLine
        {
          size_t mMaxMessageSize {};

          AMaxMessageSizeLine(const Noop &) noexcept : AMediaLine(nullptr) {}
          AMaxMessageSizeLine(MLinePtr mline, const char *value) noexcept(false);
          String toString() const noexcept { return "max-message-size:" + string(mMaxMessageSize); }
          static String toString(const AMaxMessageSizeLinePtr &line) noexcept { if (!line) return String(); return line->toString(); }
        };

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------

        struct LineTypeInfo
        {
          LineTypes mLineType {LineType_Unknown};
          Attributes mAttribute {Attribute_Unknown};
          AttributeLevels mAttributeLevel {AttributeLevel_None};
          const char *mValue {};

          LineValuePtr mLineValue;

          ElementPtr toDebug() noexcept;
        };

        struct SDP
        {
          String mOriginal;
          std::unique_ptr<char[]> mRawBuffer;

          // attribute values
          Optional<Directions> mMediaDirection;

          // attribute flags
          Optional<bool> mICELite;

          LineTypeInfoList mLineInfos;

          VLinePtr mVLine;
          OLinePtr mOLine;
          SLinePtr mSLine;
          TLinePtr mTLine;
          CLinePtr mCLine;

          AICEUFragLinePtr mAICEUFragLine;
          AICEPwdLinePtr mAICEPwdLine;
          AICEOptionsLinePtr mAICEOptionsLine;
          ASetupLinePtr mASetupLine;

          MediaLineList mMLines;
          AGroupLineList mAGroupLines;
          AMSIDSemanticLineList mAMSIDSemanticLines;
          AFingerprintLineList mAFingerprintLines;
          AExtmapLineList mAExtmapLines;
        };
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // SDPParser
      //

      class SDPParser : public ISDPTypes
      {
      public:
        struct GeneratorOptions
        {
          Locations mLocation {Location_Local};
          bool mBundleOnly {false};
        };

      protected:
        static void parseLines(SDP &sdp) noexcept(false);
        static void parseAttributes(SDP &sdp) noexcept(false);
        static void validateAttributeLevels(SDP &sdp) noexcept(false);
        static void parseLinesDetails(SDP &sdp) noexcept(false);
        static void processFlagAttributes(SDP &sdp) noexcept(false);
        static void processSessionLevelValues(SDP &sdp) noexcept(false);
        static void processMediaLevelValues(SDP &sdp) noexcept(false);
        static void processSourceLevelValues(SDP &sdp) noexcept(false);

        static void createDescriptionDetails(
                                             const SDP &sdp,
                                             Description &ioDescription
                                             ) noexcept;
        static void createTransports(
                                     const SDP &sdp,
                                     Description &ioDescription
                                     ) noexcept;
        static void createRTPMediaLines(
                                        Locations location,
                                        const SDP &sdp,
                                        Description &ioDescription
                                        ) noexcept;
        static void createSCTPMediaLines(
                                         Locations location,
                                         const SDP &sdp,
                                         Description &ioDescription
                                         ) noexcept;

        static void createRTPSenderLines(
                                         Locations location,
                                         const SDP &sdp,
                                         Description &ioDescription
                                         ) noexcept(false);

        static void createSDPSessionLevel(
                                          const GeneratorOptions &options,
                                          const Description &description,
                                          SDP &ioSDP
                                          ) noexcept(false);
        static void createSDPMediaLevel(
                                        const GeneratorOptions &options,
                                        const Description &description,
                                        SDP &ioSDP
                                        ) noexcept(false);

        static void generateSessionLevel(
                                         const SDP &sdp,
                                         String &ioResult
                                         ) noexcept;
        static void generateMediaLevel(
                                       const SDP &sdp,
                                       String &ioResult
                                       ) noexcept;

      public:
        static SDPPtr parse(const char *blob) noexcept(false);
        static String generate(const SDP &sdp) noexcept(false);

        static SDPPtr createSDP(
                                const GeneratorOptions &options,
                                const Description &description
                                ) noexcept(false);
        static DescriptionPtr createDescription(
                                                Locations location,
                                                const SDP &sdp
                                                ) noexcept(false);

        static String getCandidateSDP(const IICETypes::Candidate &candidate) noexcept;
        static IICETypes::GatherCandidatePtr getCandidateFromSDP(const char *candidate) noexcept;
      };
    }
  }
}
