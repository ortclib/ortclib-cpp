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
#include <ortc/IICETypes.h>
#include <ortc/IRTPTypes.h>
#include <ortc/IDataChannel.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes
  #pragma mark

  interaction IStatsReportTypes
  {
    typedef IRTPTypes::SSRCType SSRCType;
    typedef std::list<IRTPTypes::SSRCType> SSRCTypeList;
    typedef IRTPTypes::PayloadType PayloadType;
    typedef IICETypes::CandidateTypes StatsICECandidateTypes;
    typedef std::list<String> StringList;

    ZS_DECLARE_STRUCT_PTR(Stats);
    ZS_DECLARE_STRUCT_PTR(RTPStreamStats);
    ZS_DECLARE_STRUCT_PTR(Codec);
    ZS_DECLARE_STRUCT_PTR(InboundRTPStreamStats);
    ZS_DECLARE_STRUCT_PTR(OutboundRTPStreamStats);
    ZS_DECLARE_STRUCT_PTR(SCTPTransportStats);
    ZS_DECLARE_STRUCT_PTR(MediaStreamStats);
    ZS_DECLARE_STRUCT_PTR(MediaStreamTrackStats);
    ZS_DECLARE_STRUCT_PTR(DataChannelStats);
    ZS_DECLARE_STRUCT_PTR(ICEGathererStats);
    ZS_DECLARE_STRUCT_PTR(ICETransportStats);
    ZS_DECLARE_STRUCT_PTR(DTLSTransportStats);
    ZS_DECLARE_STRUCT_PTR(SRTPTransportStats);
    ZS_DECLARE_STRUCT_PTR(ICECandidateAttributes);
    ZS_DECLARE_STRUCT_PTR(ICECandidatePairStats);
    ZS_DECLARE_STRUCT_PTR(CertificateStats);
    ZS_DECLARE_TYPEDEF_PTR(std::list<String>, IDList);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::StatsType
    #pragma mark

    enum StatsTypes
    {
      StatsType_First,

      StatsType_InboundRTP = StatsType_First,
      StatsType_OutboundRTP,
      StatsType_Codec,
      StatsType_SCTPTransport,
      StatsType_DataChannel,
      StatsType_Stream,
      StatsType_Track,
      StatsType_ICEGatherer,
      StatsType_ICETransport,
      StatsType_DTLSTransport,
      StatsType_SRTPTransport,
      StatsType_Certificate,
      StatsType_Candidate,
      StatsType_CandidatePair,
      StatsType_LocalCandidate,
      StatsType_RemoteCandidate,

      StatsType_Last = StatsType_RemoteCandidate
    };

    static Optional<StatsTypes> toStatsType(const char *type);
    static const char *toString(StatsTypes type);

    enum StatsICECandidatePairStates
    {
      StatsICECandidatePairState_First,

      StatsICECandidatePairState_Frozen = StatsICECandidatePairState_First,
      StatsICECandidatePairState_Waiting,
      StatsICECandidatePairState_InProgress,
      StatsICECandidatePairState_Failed,
      StatsICECandidatePairState_Succeeded,
      StatsICECandidatePairState_Cancelled,

      StatsICECandidatePairState_Last = StatsICECandidatePairState_Cancelled,
    };

    static Optional<StatsICECandidatePairStates> toCandidatePairState(const char *type);
    static const char *toString(StatsICECandidatePairStates type);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::Stats
    #pragma mark

    struct Stats : public Any
    {
      Time      mTimeStamp;
      String    mStatsType;
      String    mID;

      Stats();
      Stats(const Stats &op2);
      Stats(ElementPtr rootEl);

      static StatsPtr create(ElementPtr rootEl);

      static StatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName) const;

      virtual ElementPtr toDebug() const;
      virtual String hash() const;

      Stats &operator=(const Stats &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::RTPStreamStats
    #pragma mark

    struct RTPStreamStats : public Stats
    {
      Optional<SSRCType>  mSSRC;
      String              mAssociatedStatID;
      bool                mIsRemote {false};
      String              mMediaType;
      String              mMediaTrackID;
      String              mTransportID;
      String              mCodecID;
      unsigned long       mFIRCount {};
      unsigned long       mPLICount {};
      unsigned long       mNACKCount {};
      unsigned long       mSLICount {};

      RTPStreamStats() {}
      RTPStreamStats(const RTPStreamStats &op2);
      RTPStreamStats(ElementPtr rootEl);

      static RTPStreamStatsPtr create(ElementPtr rootEl);

      static RTPStreamStatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName) const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      RTPStreamStats &operator=(const RTPStreamStats &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::Codec
    #pragma mark

    struct Codec : public Stats
    {
      Optional<PayloadType>     mPayloadType;
      String                    mCodec;
      unsigned long             mClockRate {};
      Optional<unsigned long>   mChannels {};
      String                    mParameters;

      Codec() {}
      Codec(const Codec &op2);
      Codec(ElementPtr rootEl);

      static CodecPtr create(ElementPtr rootEl);

      static CodecPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "codec") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      Codec &operator=(const Codec &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::InboundRTPStreamStats
    #pragma mark

    struct InboundRTPStreamStats : public RTPStreamStats
    {
      unsigned long       mPacketsReceived {};
      unsigned long long  mBytesReceived {};
      unsigned long       mPacketsLost {};
      double              mJitter {};
      double              mFractionLost {};

      InboundRTPStreamStats() {}
      InboundRTPStreamStats(const InboundRTPStreamStats &op2);
      InboundRTPStreamStats(ElementPtr rootEl);

      static InboundRTPStreamStatsPtr create(ElementPtr rootEl);

      static InboundRTPStreamStatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "inboundrtp") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      InboundRTPStreamStats &operator=(const InboundRTPStreamStats &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::InboundRTPStreamStats
    #pragma mark

    struct OutboundRTPStreamStats : public RTPStreamStats
    {
      unsigned long       mPacketsSent {};
      unsigned long long  mBytesSent {};
      double              mTargetBitrate {};
      double              mToundTripTime {};

      OutboundRTPStreamStats() {}
      OutboundRTPStreamStats(const OutboundRTPStreamStats &op2);
      OutboundRTPStreamStats(ElementPtr rootEl);

      static OutboundRTPStreamStatsPtr create(ElementPtr rootEl);

      static OutboundRTPStreamStatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "outboundrtp") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      OutboundRTPStreamStats &operator=(const OutboundRTPStreamStats &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::SCTPTransportStats
    #pragma mark

    struct SCTPTransportStats : public Stats
    {
      unsigned long mDataChannelsOpened {};
      unsigned long mDataChannelsClosed {};

      SCTPTransportStats() {}
      SCTPTransportStats(const SCTPTransportStats &op2);
      SCTPTransportStats(ElementPtr rootEl);

      static SCTPTransportStatsPtr create(ElementPtr rootEl);

      static SCTPTransportStatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "sctptransport") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      SCTPTransportStats &operator=(const SCTPTransportStats &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::MediaStreamStats
    #pragma mark

    struct MediaStreamStats : public Stats
    {
      String      mStreamIDs;
      StringList  mTrackIDs;

      MediaStreamStats() {}
      MediaStreamStats(const SCTPTransportStats &op2);
      MediaStreamStats(ElementPtr rootEl);

      static MediaStreamStatsPtr create(ElementPtr rootEl);

      static MediaStreamStatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "stream") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      MediaStreamStats &operator=(const MediaStreamStats &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::MediaStreamTrackStats
    #pragma mark

    struct MediaStreamTrackStats : public Stats
    {
      String          mTrackID;
      boolean         mRemoteSource {};
      SSRCTypeList    mSSRCIDs;
      unsigned long   mFrameWidth {};
      unsigned long   mFrameHeight {};
      double          mFramesPerSecond {};
      unsigned long   mFramesSent {};
      unsigned long   mFramesReceived {};
      unsigned long   mFramesDecoded {};
      unsigned long   mFramesDropped {};
      unsigned long   mFramesCorrupted {};
      double          mAudioLevel {};
      double          mEchoReturnLoss {};
      double          mEchoReturnLossEnhancement {};

      MediaStreamTrackStats() {}
      MediaStreamTrackStats(const MediaStreamTrackStats &op2);
      MediaStreamTrackStats(ElementPtr rootEl);

      static MediaStreamTrackStatsPtr create(ElementPtr rootEl);

      static MediaStreamTrackStatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "track") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      MediaStreamTrackStats &operator=(const MediaStreamStats &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::DataChannelStats
    #pragma mark

    struct DataChannelStats : public Stats
    {
      String                    mLabel;
      String                    mProtocol;
      long                      mDatachannelID {};
      IDataChannelTypes::States mState {IDataChannelTypes::State_First};
      unsigned long             mMessagesSent {};
      unsigned long long        mBytesSent {};
      unsigned long             mMessagesReceived {};
      unsigned long long        mBytesReceived {};

      DataChannelStats() {}
      DataChannelStats(const DataChannelStats &op2);
      DataChannelStats(ElementPtr rootEl);

      static DataChannelStatsPtr create(ElementPtr rootEl);

      static DataChannelStatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "datachannel") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      DataChannelStats &operator=(const DataChannelStats &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::ICEGathererStats
    #pragma mark

    struct ICEGathererStats : public Stats
    {
      unsigned long long  mBytesSent{};
      unsigned long long  mBytesReceived{};
      String              mRTCPGathererStatsID;

      ICEGathererStats() {}
      ICEGathererStats(const ICEGathererStats &op2);
      ICEGathererStats(ElementPtr rootEl);

      static ICEGathererStatsPtr create(ElementPtr rootEl);

      static ICEGathererStatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "icegatherer") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      ICEGathererStats &operator=(const ICEGathererStats &op2) = delete;
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::ICETransportStats
    #pragma mark

    struct ICETransportStats : public Stats
    {
      unsigned long long  mBytesSent {};
      unsigned long long  mBytesReceived {};
      String              mRTCPTransportStatsID;
      bool                mActiveConnection {};
      String              mSelectedCandidatePairID;

      ICETransportStats() {}
      ICETransportStats(const ICETransportStats &op2);
      ICETransportStats(ElementPtr rootEl);

      static ICETransportStatsPtr create(ElementPtr rootEl);

      static ICETransportStatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "icetransport") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      ICETransportStats &operator=(const ICETransportStats &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::DTLSTransportStats
    #pragma mark

    struct DTLSTransportStats : public Stats
    {
      String mLocalCertificateID;
      String mRemoteCertificateID;

      DTLSTransportStats() {}
      DTLSTransportStats(const DTLSTransportStats &op2);
      DTLSTransportStats(ElementPtr rootEl);

      static DTLSTransportStatsPtr create(ElementPtr rootEl);

      static DTLSTransportStatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "dtlstransport") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      DTLSTransportStats &operator=(const DTLSTransportStats &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::SRTPTransportStats
    #pragma mark

    struct SRTPTransportStats : public Stats
    {
      SRTPTransportStats() {}
      SRTPTransportStats(const SRTPTransportStats &op2);
      SRTPTransportStats(ElementPtr rootEl);

      static SRTPTransportStatsPtr create(ElementPtr rootEl);

      static SRTPTransportStatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "srtptransport") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      SRTPTransportStats &operator=(const SRTPTransportStats &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::ICECandidateAttributes
    #pragma mark

    struct ICECandidateAttributes : public Stats
    {
      String                  mIPAddress;
      unsigned long           mPortNumber {};
      String                  mTransport;
      StatsICECandidateTypes  mCandidateType {IICETypes::CandidateType_First};
      unsigned long           mPriority {};
      String                  mAddressSourceURL;

      ICECandidateAttributes() {}
      ICECandidateAttributes(const ICECandidateAttributes &op2);
      ICECandidateAttributes(ElementPtr rootEl);

      static ICECandidateAttributesPtr create(ElementPtr rootEl);

      static ICECandidateAttributesPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "dtlstransport") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      ICECandidateAttributes &operator=(const ICECandidateAttributes &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::ICECandidatePairStats
    #pragma mark

    struct ICECandidatePairStats : public Stats
    {
      String                        mTransportID;
      String                        mLocalCandidateID;
      String                        mRemoteCandidateID;
      StatsICECandidatePairStates   mState {StatsICECandidatePairState_First};
      unsigned long long            mPriority {};
      bool                          mNominated {};
      bool                          mWritable {};
      bool                          mReadable {};
      unsigned long long            mBytesSent {};
      unsigned long long            mBytesReceived {};
      double                        mRoundTripTime {};
      double                        mAvailableOutgoingBitrate {};
      double                        mAvailableIncomingBitrate {};

      ICECandidatePairStats() {}
      ICECandidatePairStats(const ICECandidatePairStats &op2);
      ICECandidatePairStats(ElementPtr rootEl);

      static ICECandidatePairStatsPtr create(ElementPtr rootEl);

      static ICECandidatePairStatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "candidatepair") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      ICECandidatePairStats &operator=(const ICECandidatePairStats &op2) = delete;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportTypes::CertificateStats
    #pragma mark

    struct CertificateStats : public Stats
    {
      String mFingerprint;
      String mFingerprintAlgorithm;
      String mBase64Certificate;
      String mIssuerCertificateID;

      CertificateStats() {}
      CertificateStats(const CertificateStats &op2);
      CertificateStats(ElementPtr rootEl);

      static ICECandidatePairStatsPtr create(ElementPtr rootEl);

      static ICECandidatePairStatsPtr convert(AnyPtr any);

      virtual ElementPtr createElement(const char *objectName = "certificate") const;

      virtual ElementPtr toDebug() const override;
      virtual String hash() const override;

      CertificateStats &operator=(const CertificateStats &op2) = delete;
    };

  };
  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReport
  #pragma mark

  interaction IStatsReport : public IStatsReportTypes,
                             public Any
  {
    static ElementPtr toDebug(IStatsReportPtr report);
    static IStatsReportPtr convert(AnyPtr any);

    virtual PUID getID() const = 0;

    virtual IDListPtr getStatesIDs() const = 0;
    virtual StatsPtr getStats(const char *id) const = 0;
  };

}
