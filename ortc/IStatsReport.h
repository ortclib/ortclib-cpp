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
#include <ortc/IDataChannelTypes.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IStatsReportTypes
  //

  interaction IStatsReportTypes
  {
    typedef IRTPTypes::SSRCType SSRCType;
    typedef std::list<IRTPTypes::SSRCType> SSRCTypeList;
    typedef IRTPTypes::PayloadType PayloadType;
    typedef IICETypes::CandidateTypes StatsICECandidateTypes;
    typedef std::list<String> StringList;

    ZS_DECLARE_STRUCT_PTR(StatsTypeSet);

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
    //
    // IStatsReportTypes::StatsType
    //

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

    static Optional<StatsTypes> toStatsType(const char *type) noexcept;
    static const char *toString(StatsTypes type) noexcept;
    static const char *toString(const Optional<StatsTypes> &type) noexcept;

    struct StatsTypeSet : public std::set<IStatsReportTypes::StatsTypes>
    {
      bool hasStatType(StatsTypes stat) const noexcept;
    };

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

    static Optional<StatsICECandidatePairStates> toCandidatePairState(const char *type) noexcept;
    static const char *toString(StatsICECandidatePairStates type) noexcept;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::Stats
    //

    struct Stats : public Any
    {
      Time                  mTimestamp;
      Optional<StatsTypes>  mStatsType;
      String                mStatsTypeOther;
      String                mID;

      Stats() noexcept;
      Stats(const Stats &op2) noexcept;
      Stats(ElementPtr rootEl) noexcept;

      String statsType() const noexcept { if (mStatsType.hasValue()) return IStatsReportTypes::toString(mStatsType); return mStatsTypeOther; }

      static StatsPtr create(const Stats &source) noexcept;
      static StatsPtr create(ElementPtr rootEl) noexcept;

      static StatsPtr convert(AnyPtr any) noexcept;

      virtual ElementPtr createElement(const char *objectName) const noexcept;

      virtual ElementPtr toDebug() const noexcept;
      virtual String hash() const noexcept;

      virtual void eventTrace() const noexcept;

      Stats &operator=(const Stats &op2) noexcept = delete;

    protected:
      virtual void eventTrace(double timestamp) const noexcept;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::RTPStreamStats
    //

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

      RTPStreamStats() noexcept {}
      RTPStreamStats(const RTPStreamStats &op2) noexcept;
      RTPStreamStats(ElementPtr rootEl) noexcept;

      static RTPStreamStatsPtr create(ElementPtr rootEl) noexcept;
      static RTPStreamStatsPtr create(const RTPStreamStats &op2) noexcept { return std::dynamic_pointer_cast<RTPStreamStats>(Stats::create(op2)); }

      static RTPStreamStatsPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName) const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      RTPStreamStats &operator=(const RTPStreamStats &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::Codec
    //

    struct Codec : public Stats
    {
      Optional<PayloadType>     mPayloadType;
      String                    mCodec;
      unsigned long             mClockRate {};
      Optional<unsigned long>   mChannels {};
      String                    mParameters;

      Codec() noexcept { mStatsType = IStatsReportTypes::StatsType_Codec; }
      Codec(const Codec &op2) noexcept;
      Codec(ElementPtr rootEl) noexcept;

      static CodecPtr create(ElementPtr rootEl) noexcept;
      static CodecPtr create(const Codec &op2) noexcept { return std::make_shared<Codec>(op2); }

      static CodecPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "codec") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      Codec &operator=(const Codec &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::InboundRTPStreamStats
    //

    struct InboundRTPStreamStats : public RTPStreamStats
    {
      unsigned long       mPacketsReceived {};
      unsigned long long  mBytesReceived {};
      unsigned long       mPacketsLost {};
      double              mJitter {};
      double              mFractionLost {};
      Milliseconds        mEndToEndDelay {};

      InboundRTPStreamStats() noexcept { mStatsType = IStatsReportTypes::StatsType_InboundRTP; }
      InboundRTPStreamStats(const InboundRTPStreamStats &op2) noexcept;
      InboundRTPStreamStats(ElementPtr rootEl) noexcept;

      static InboundRTPStreamStatsPtr create(ElementPtr rootEl) noexcept;
      static InboundRTPStreamStatsPtr create(const InboundRTPStreamStats &op2) noexcept { return std::make_shared<InboundRTPStreamStats>(op2); }

      static InboundRTPStreamStatsPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "inboundrtp") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      InboundRTPStreamStats &operator=(const InboundRTPStreamStats &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::OutboundRTPStreamStats
    //

    struct OutboundRTPStreamStats : public RTPStreamStats
    {
      unsigned long       mPacketsSent {};
      unsigned long long  mBytesSent {};
      double              mTargetBitrate {};
      double              mRoundTripTime {};

      OutboundRTPStreamStats() noexcept { mStatsType = IStatsReportTypes::StatsType_OutboundRTP; }
      OutboundRTPStreamStats(const OutboundRTPStreamStats &op2) noexcept;
      OutboundRTPStreamStats(ElementPtr rootEl) noexcept;

      static OutboundRTPStreamStatsPtr create(ElementPtr rootEl) noexcept;
      static OutboundRTPStreamStatsPtr create(const OutboundRTPStreamStats &op2) noexcept { return std::make_shared<OutboundRTPStreamStats>(op2); }

      static OutboundRTPStreamStatsPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "outboundrtp") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      OutboundRTPStreamStats &operator=(const OutboundRTPStreamStats &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::SCTPTransportStats
    //

    struct SCTPTransportStats : public Stats
    {
      unsigned long mDataChannelsOpened {};
      unsigned long mDataChannelsClosed {};

      SCTPTransportStats() noexcept { mStatsType = IStatsReportTypes::StatsType_SCTPTransport; }
      SCTPTransportStats(const SCTPTransportStats &op2) noexcept;
      SCTPTransportStats(ElementPtr rootEl) noexcept;

      static SCTPTransportStatsPtr create(ElementPtr rootEl) noexcept;
      static SCTPTransportStatsPtr create(const SCTPTransportStats &op2) noexcept { return std::make_shared<SCTPTransportStats>(op2); }

      static SCTPTransportStatsPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "sctptransport") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      SCTPTransportStats &operator=(const SCTPTransportStats &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::MediaStreamStats
    //

    struct MediaStreamStats : public Stats
    {
      String      mStreamID;
      StringList  mTrackIDs;

      MediaStreamStats() noexcept { mStatsType = IStatsReportTypes::StatsType_Stream; }
      MediaStreamStats(const MediaStreamStats &op2) noexcept;
      MediaStreamStats(ElementPtr rootEl) noexcept;

      static MediaStreamStatsPtr create(ElementPtr rootEl) noexcept;
      static MediaStreamStatsPtr create(const MediaStreamStats &op2) noexcept { return std::make_shared<MediaStreamStats>(op2); }

      static MediaStreamStatsPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "stream") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      MediaStreamStats &operator=(const MediaStreamStats &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }
    
    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::MediaStreamTrackStats
    //

    struct MediaStreamTrackStats : public Stats
    {
      String          mTrackID;
      bool            mRemoteSource {};
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

      MediaStreamTrackStats() noexcept { mStatsType = IStatsReportTypes::StatsType_Track; }
      MediaStreamTrackStats(const MediaStreamTrackStats &op2) noexcept;
      MediaStreamTrackStats(ElementPtr rootEl) noexcept;

      static MediaStreamTrackStatsPtr create(ElementPtr rootEl) noexcept;
      static MediaStreamTrackStatsPtr create(const MediaStreamTrackStats &op2) noexcept { return std::make_shared<MediaStreamTrackStats>(op2); }

      static MediaStreamTrackStatsPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "track") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      MediaStreamTrackStats &operator=(const MediaStreamStats &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::DataChannelStats
    //

    struct DataChannelStats : public Stats
    {
      String                    mLabel;
      String                    mProtocol;
      long                      mDataChannelID {};
      IDataChannelTypes::States mState {IDataChannelTypes::State_First};
      unsigned long             mMessagesSent {};
      unsigned long long        mBytesSent {};
      unsigned long             mMessagesReceived {};
      unsigned long long        mBytesReceived {};

      DataChannelStats() noexcept { mStatsType = IStatsReportTypes::StatsType_DataChannel; }
      DataChannelStats(const DataChannelStats &op2) noexcept;
      DataChannelStats(ElementPtr rootEl) noexcept;

      static DataChannelStatsPtr create(ElementPtr rootEl) noexcept;
      static DataChannelStatsPtr create(const DataChannelStats &op2) noexcept { return std::make_shared<DataChannelStats>(op2); }

      static DataChannelStatsPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "datachannel") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      DataChannelStats &operator=(const DataChannelStats &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::ICEGathererStats
    //

    struct ICEGathererStats : public Stats
    {
      unsigned long long  mBytesSent{};
      unsigned long long  mBytesReceived{};
      String              mRTCPGathererStatsID;

      ICEGathererStats() noexcept { mStatsType = IStatsReportTypes::StatsType_ICEGatherer; }
      ICEGathererStats(const ICEGathererStats &op2) noexcept;
      ICEGathererStats(ElementPtr rootEl) noexcept;

      static ICEGathererStatsPtr create(ElementPtr rootEl) noexcept;
      static ICEGathererStatsPtr create(const ICEGathererStats &op2) noexcept { return std::make_shared<ICEGathererStats>(op2); }

      static ICEGathererStatsPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "icegatherer") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      ICEGathererStats &operator=(const ICEGathererStats &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::ICETransportStats
    //

    struct ICETransportStats : public Stats
    {
      unsigned long long  mBytesSent {};
      unsigned long long  mBytesReceived {};
      String              mRTCPTransportStatsID;
      bool                mActiveConnection {};
      String              mSelectedCandidatePairID;

      ICETransportStats() noexcept { mStatsType = IStatsReportTypes::StatsType_ICETransport; }
      ICETransportStats(const ICETransportStats &op2) noexcept;
      ICETransportStats(ElementPtr rootEl) noexcept;

      static ICETransportStatsPtr create(ElementPtr rootEl) noexcept;
      static ICETransportStatsPtr create(const ICETransportStats &op2) noexcept { return std::make_shared<ICETransportStats>(op2); }

      static ICETransportStatsPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "icetransport") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      ICETransportStats &operator=(const ICETransportStats &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::DTLSTransportStats
    //

    struct DTLSTransportStats : public Stats
    {
      String mLocalCertificateID;
      String mRemoteCertificateID;

      DTLSTransportStats() noexcept { mStatsType = IStatsReportTypes::StatsType_DTLSTransport; }
      DTLSTransportStats(const DTLSTransportStats &op2) noexcept;
      DTLSTransportStats(ElementPtr rootEl) noexcept;

      static DTLSTransportStatsPtr create(ElementPtr rootEl) noexcept;
      static DTLSTransportStatsPtr create(const DTLSTransportStats &op2) noexcept { return std::make_shared<DTLSTransportStats>(op2); }

      static DTLSTransportStatsPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "dtlstransport") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      DTLSTransportStats &operator=(const DTLSTransportStats &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::SRTPTransportStats
    //

    struct SRTPTransportStats : public Stats
    {
      SRTPTransportStats() noexcept { mStatsType = IStatsReportTypes::StatsType_SRTPTransport; }
      SRTPTransportStats(const SRTPTransportStats &op2) noexcept;
      SRTPTransportStats(ElementPtr rootEl) noexcept;

      static SRTPTransportStatsPtr create(ElementPtr rootEl) noexcept;
      static SRTPTransportStatsPtr create(const SRTPTransportStats &op2) noexcept { return std::make_shared<SRTPTransportStats>(op2); }

      static SRTPTransportStatsPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "srtptransport") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      SRTPTransportStats &operator=(const SRTPTransportStats &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::ICECandidateAttributes
    //

    struct ICECandidateAttributes : public Stats
    {
      String                  mRelatedID;
      String                  mIPAddress;
      unsigned long           mPortNumber {};
      String                  mTransport;
      StatsICECandidateTypes  mCandidateType {IICETypes::CandidateType_First};
      unsigned long           mPriority {};
      String                  mAddressSourceURL;

      ICECandidateAttributes() noexcept { mStatsType = IStatsReportTypes::StatsType_Candidate; }
      ICECandidateAttributes(const ICECandidateAttributes &op2) noexcept;
      ICECandidateAttributes(ElementPtr rootEl) noexcept;

      static ICECandidateAttributesPtr create(ElementPtr rootEl) noexcept;
      static ICECandidateAttributesPtr create(const ICECandidateAttributes &op2) noexcept { return std::make_shared<ICECandidateAttributes>(op2); }

      static ICECandidateAttributesPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "dtlstransport") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      ICECandidateAttributes &operator=(const ICECandidateAttributes &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::ICECandidatePairStats
    //

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

      ICECandidatePairStats() noexcept { mStatsType = IStatsReportTypes::StatsType_CandidatePair; }
      ICECandidatePairStats(const ICECandidatePairStats &op2) noexcept;
      ICECandidatePairStats(ElementPtr rootEl) noexcept;

      static ICECandidatePairStatsPtr create(ElementPtr rootEl) noexcept;
      static ICECandidatePairStatsPtr create(const ICECandidatePairStats &op2) noexcept { return std::make_shared<ICECandidatePairStats>(op2); }

      static ICECandidatePairStatsPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "candidatepair") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      ICECandidatePairStats &operator=(const ICECandidatePairStats &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportTypes::CertificateStats
    //

    struct CertificateStats : public Stats
    {
      String mFingerprint;
      String mFingerprintAlgorithm;
      String mBase64Certificate;
      String mIssuerCertificateID;

      CertificateStats() noexcept { mStatsType = IStatsReportTypes::StatsType_Certificate; }
      CertificateStats(const CertificateStats &op2) noexcept;
      CertificateStats(ElementPtr rootEl) noexcept;

      static CertificateStatsPtr create(ElementPtr rootEl) noexcept;
      static CertificateStatsPtr create(const CertificateStats &op2) noexcept { return std::make_shared<CertificateStats>(op2); }

      static CertificateStatsPtr convert(AnyPtr any) noexcept;

      ElementPtr createElement(const char *objectName = "certificate") const noexcept override;

      ElementPtr toDebug() const noexcept override;
      String hash() const noexcept override;

      CertificateStats &operator=(const CertificateStats &op2) noexcept = delete;

      void eventTrace() const noexcept override { Stats::eventTrace(); }

    protected:
      void eventTrace(double timestamp) const noexcept override;
    };

  };
  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IStatsReport
  //

  interaction IStatsReport : public IStatsReportTypes,
                             public Any
  {
    static ElementPtr toDebug(IStatsReportPtr report) noexcept;
    static IStatsReportPtr convert(AnyPtr any) noexcept;

    virtual PUID getID() const noexcept = 0;

    virtual IDListPtr getStatesIDs() const noexcept = 0;
    virtual StatsPtr getStats(const char *id) const noexcept = 0;
  };

}
