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

#include <ortc/adapter/internal/ortc_adapter_SDPParser.h>

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
      #pragma mark
      #pragma mark ISDPTypes
      #pragma mark

      interaction ISDPTypes
      {
        ZS_DECLARE_TYPEDEF_PTR(ISessionDescriptionTypes::Description, Description);

        typedef BYTE PayloadType;
        typedef DWORD SSRCType;

        typedef std::list<SSRCType> SSRCList;
        typedef std::list<PayloadType> PayloadTypeList;

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
        ZS_DECLARE_TYPEDEF_PTR(std::list<ASCTPPortLinePtr>, ASCTPPortLineList);

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

        static const char *toString(LineTypes type);
        static LineTypes toLineType(const char *type);
        static LineTypes toLineType(const char type);

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

          Attribute_Last = Attribute_SCTPPort,
        };

        static const char *toString(Attributes attribute);
        static Attributes toAttribute(const char *attribute);

        static bool requiresValue(Attributes attribute);
        static bool requiresEmptyValue(Attributes attribute);

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

        static const char *toString(AttributeLevels level);
        static AttributeLevels toAttributeLevel(const char *level);

        static AttributeLevels getAttributeLevel(LineTypes lineType);
        static AttributeLevels getAttributeLevel(Attributes attribute);

        static bool supportedAtLevel(
                                     AttributeLevels currentLevel,
                                     LineTypes lineType
                                     );
        static bool supportedAtLevel(
                                     AttributeLevels currentLevel,
                                     Attributes attribute
                                     );

        
        enum Locations
        {
          Location_First,

          Location_Local = Location_First,
          Location_Remote,

          Location_Last = Location_Remote,
        };

        static const char *toString(Locations location);
        static Locations toLocation(const char *location);

        enum Directions
        {
          Direction_None        = 0x00,
          Direction_Send        = 0x01,
          Direction_Receive     = 0x02,

          Direction_SendReceive = Direction_Send | Direction_Receive,
        };

        static const char *toString(Directions direction);
        static Directions toDirection(const char *direction);
        static bool isValid(
                            Directions direction,
                            bool allowNone,
                            bool allowSend,
                            bool allowReceive,
                            bool allowSendReceive
                            );

        enum ActorRoles
        {
          ActorRole_Sender      = 0x01,
          ActorRole_Receiver    = 0x02,
          ActorRole_Transceiver = ActorRole_Sender | ActorRole_Receiver,
        };

        static const char *toString(ActorRoles actor);
        static ActorRoles toActorRole(const char *actor);

        static bool isApplicable(
                                 ActorRoles actor,
                                 Locations location,
                                 Directions direction
                                 );

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

          MediaLine(MLinePtr mline) : mMLine(mline) {}
        };

        struct VLine : public LineValue
        {
          ULONGLONG mVersion {};

          VLine(const char *value);
        };

        struct OLine : public LineValue
        {
          String mUsername{ "-" };
          ULONGLONG mSessionID {};
          ULONGLONG mSessionVersion {};
          String mNetType;
          String mAddrType;
          String mUnicastAddress;

          OLine(const char *value);
        };

        struct SLine : public LineValue
        {
          String mSessionName;

          SLine(const char *value);
        };

        struct BLine : public MediaLine
        {
          String mBWType;
          ULONGLONG mBandwidth {};

          BLine(MLinePtr mline, const char *value);
        };

        struct TLine : public LineValue
        {
          QWORD mStartTime {};
          QWORD mEndTime {};

          TLine(const char *value);
        };

        struct MLine : public LineValue
        {
          // media level flags
          Optional<bool> mBundleOnly;
          Optional<bool> mEndOfCandidates;
          Optional<Directions> mMediaDirection;
          Optional<bool> mRTCPMux;
          Optional<bool> mRTCPRSize;

          String mMedia;
          ULONGLONG mPort {};
          ULONGLONG mInteger {};
          String mProto;
          StringList mFmts;

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
          ASCTPPortLineList mASCTPPortLines;

          MLine(const char *value);
        };

        struct CLine : public MediaLine
        {
          String mNetType;
          String mAddrType;
          String mConnectionAddress;

          CLine(MLinePtr mline, const char *value);
        };

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------

        struct ALine : public LineValue
        {
        };

        struct AMediaLine : public ALine
        {
          MLinePtr mMLine;

          AMediaLine(MLinePtr mline) : mMLine(mline) {}
        };

        struct AMediaFlagLine : public AMediaLine
        {
          AMediaFlagLine(MLinePtr mline) : AMediaLine(mline) {}
        };

        struct AGroupLine : public ALine
        {
          String mSemantic;
          StringList mIdentificationTags;

          AGroupLine(const char *value);
        };

        struct AMSIDLine : public AMediaLine
        {
          String mID;
          String mAppData;

          AMSIDLine(MLinePtr mline, const char *value);
        };

        struct AMSIDSemanticLine : public AGroupLine
        {
          AMSIDSemanticLine(const char *value) : AGroupLine(value) {}
        };

        struct AICEUFragLine : public AMediaLine
        {
          String mICEUFrag;

          AICEUFragLine(MLinePtr mline, const char *value);
        };

        struct AICEPwdLine : public AMediaLine
        {
          String mICEPwd;

          AICEPwdLine(MLinePtr mline, const char *value);
        };

        struct AICEOptionsLine : public ALine
        {
          StringList mTags;

          AICEOptionsLine(const char *value);
        };

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

          ACandidateLine(MLinePtr mline, const char *value);
        };

        struct AFingerprintLine : public AMediaLine
        {
          String mHashFunc;
          String mFingerprint;

          AFingerprintLine(MLinePtr mline, const char *value);
        };

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

          ACryptoLine(MLinePtr mline, const char *value);
        };

        struct ASetupLine : public AMediaLine
        {
          String mSetup;

          ASetupLine(MLinePtr mline, const char *value);
        };

        struct AMIDLine : public AMediaLine
        {
          String mMID;

          AMIDLine(MLinePtr mline, const char *value);
        };

        struct AExtmapLine : public AMediaLine
        {
          WORD mID {};
          Directions mDirection {Direction_None};
          String mURI;
          String mExtensionAttributes;

          AExtmapLine(MLinePtr mline, const char *value);
        };

        struct AMediaDirectionLine : public AMediaLine
        {
          Directions mDirection {Direction_None};

          AMediaDirectionLine(MLinePtr mline, const char *value);
        };

        struct ARTPMapLine : public AMediaLine
        {
          PayloadType mPayloadType {};
          String mEncodingName;
          ULONG mClockRate {};
          Optional<ULONG> mEncodingParameters;

          ARTPMapLine(MLinePtr mline, const char *value);
        };

        struct AFMTPLine : public AMediaLine
        {
          ASSRCLinePtr mSourceLine;

          PayloadType mFormat {};
          StringList mFormatSpecific;

          AFMTPLine(MLinePtr mline, ASSRCLinePtr sourceLine, const char *value);
        };

        struct ARTCPLine : public AMediaLine
        {
          WORD mPort {};
          String mNetType;
          String mAddrType;
          String mConnectionAddress;

          ARTCPLine(MLinePtr mline, const char *value);
        };

        struct ARTCPFBLine : public AMediaLine
        {
          Optional<PayloadType> mPayloadType;  // if not set, then applies to all
          String mID;
          String mParam1;
          String mParam2;

          ARTCPFBLine(MLinePtr mline, const char *value);
        };

        struct APTimeLine : public AMediaLine
        {
          Milliseconds mPTime;

          APTimeLine(MLinePtr mline, const char *value);
        };

        struct AMaxPTimeLine : public AMediaLine
        {
          Milliseconds mMaxPTime;

          AMaxPTimeLine(MLinePtr mline, const char *value);
        };

        struct ASSRCLine : public AMediaLine
        {
          SSRCType mSSRC {};

          String mAttribute;
          StringList mAttributeValues;

          AFMTPLineList mAFMTPLines;

          ASSRCLine(MLinePtr mline, const char *value);
        };

        struct ASSRCGroupLine : public AMediaLine
        {
          String mSemantics;
          SSRCList mSSRCs;

          ASSRCGroupLine(MLinePtr mline, const char *value);
        };

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

          ARIDLine(MLinePtr mline, const char *value);
        };

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

          ASimulcastLine(MLinePtr mline, const char *value);
        };

        struct ASCTPPortLine : public AMediaLine
        {
          WORD mPort {};

          ASCTPPortLine(MLinePtr mline, const char *value);
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

          ElementPtr toDebug();
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
      #pragma mark
      #pragma mark SDPParser
      #pragma mark

      class SDPParser : public ISDPTypes
      {
      protected:
        static void parseLines(SDP &sdp);
        static void parseAttributes(SDP &sdp);
        static void validateAttributeLevels(SDP &sdp);
        static void parseLinesDetails(SDP &sdp);
        static void processFlagAttributes(SDP &sdp);
        static void processSessionLevelValues(SDP &sdp);
        static void processMediaLevelValues(SDP &sdp);
        static void processSourceLevelValues(SDP &sdp);

        static void createTransports(
                                     const SDP &sdp,
                                     Description &ioDescription
                                     );
        static void createDescriptionDetails(
                                             const SDP &sdp,
                                             Description &ioDescription
                                             );

      public:
        static SDPPtr parse(const char *blob);

        static DescriptionPtr createDescription(
                                                const SDP &sdp,
                                                Locations location
                                                );
      };
    }
  }
}
