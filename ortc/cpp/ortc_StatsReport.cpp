/*

 Copyright (c) 2015, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>


#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

  ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper)

  using zsLib::Log;
  using zsLib::Numeric;

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void IStatsReportForSettings::applyDefaults()
    {
//      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 5*1024);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark StatsReport
    #pragma mark
    
    //-------------------------------------------------------------------------
    StatsReport::StatsReport(
                             const make_private &,
                             IMessageQueuePtr queue,
                             const StatMap &stats
                             ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mStats(stats)
    {
      ZS_LOG_DETAIL(debug("created"))

      ZS_THROW_NOT_IMPLEMENTED("identity API in specification is not ready")
    }

    //-------------------------------------------------------------------------
    void StatsReport::init()
    {
      //AutoRecursiveLock lock(*this);
    }

    //-------------------------------------------------------------------------
    StatsReport::~StatsReport()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    StatsReportPtr StatsReport::convert(IStatsReportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(StatsReport, object);
    }

    //-------------------------------------------------------------------------
    StatsReportPtr StatsReport::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(StatsReport, object);
    }

    //-------------------------------------------------------------------------
    StatsReportPtr StatsReport::convert(ForInternalPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(StatsReport, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark StatsReport => IStatsReport
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr StatsReport::toDebug(StatsReportPtr report)
    {
      if (!report) return ElementPtr();
      return report->toDebug();
    }

    //-------------------------------------------------------------------------
    IStatsReportTypes::IDListPtr StatsReport::getStatesIDs() const
    {
      IDListPtr result(make_shared<IDList>());
      AutoRecursiveLock lock(*this);
      for (auto iter = mStats.begin(); iter != mStats.end(); ++iter) {
        auto &id = (*iter).first;
        result->push_back(id);
      }
      return result;
    }

    //-------------------------------------------------------------------------
    IStatsReportTypes::StatsPtr StatsReport::getStats(const char *id) const
    {
      AutoRecursiveLock lock(*this);

      auto found = mStats.find(String(id));
      if (found == mStats.end()) return StatsPtr();

      return (*found).second;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark StatsReport => IStatsReportForInternal
    #pragma mark
    
    //-------------------------------------------------------------------------
    StatsReportPtr StatsReport::create(const StatMap &stats)
    {
      StatsReportPtr pThis(make_shared<StatsReport>(make_private {}, IORTCForInternal::queueORTC(), stats));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark StatsReport => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params StatsReport::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::StatsReport");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params StatsReport::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr StatsReport::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::StatsReport");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "stat size", mStats.size());

      if (mStats.size() > 0) {
        ElementPtr statsEl = Element::create("stats");
        for (auto iter = mStats.begin(); iter != mStats.end(); ++iter) {
          ElementPtr statEl = Element::create("stat");

          auto &statID = (*iter).first;
          auto &stat = (*iter).second;

          UseServicesHelper::debugAppend(statEl, "id", statID);
          UseServicesHelper::debugAppend(statEl, stat ? stat->toDebug() : ElementPtr());
          UseServicesHelper::debugAppend(statsEl, statEl);
        }

        UseServicesHelper::debugAppend(resultEl, statsEl);
      }

      return resultEl;
    }

    //-------------------------------------------------------------------------
    void StatsReport::cancel()
    {
      //.......................................................................
      // final cleanup

      mStats.clear();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IStatsReportFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IStatsReportFactory &IStatsReportFactory::singleton()
    {
      return StatsReportFactory::singleton();
    }

    //-------------------------------------------------------------------------
    StatsReportPtr IStatsReportFactory::create(const StatMap &stats)
    {
      if (this) {}
      return internal::StatsReport::create(stats);
    }

  } // internal namespace


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes
  #pragma mark

  //---------------------------------------------------------------------------
  Optional<IStatsReportTypes::StatsTypes> IStatsReportTypes::toStatsType(const char *type)
  {
    String str(type);
    for (IStatsReportTypes::StatsTypes index = IStatsReportTypes::StatsType_First; index <= IStatsReportTypes::StatsType_Last; index = static_cast<IStatsReportTypes::StatsTypes>(static_cast<std::underlying_type<IStatsReportTypes::StatsTypes>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IStatsReportTypes::toString(index))) return index;
    }

    return Optional<StatsTypes>();
  }

  //---------------------------------------------------------------------------
  const char *IStatsReportTypes::toString(StatsTypes type)
  {
    switch (type)
    {
      case StatsType_InboundRTP:      return "inboundrtp";
      case StatsType_OutboundRTP:     return "outboundrtp";
      case StatsType_Codec:           return "codec";
      case StatsType_SCTPTransport:   return "sctptransport";
      case StatsType_DataChannel:     return "datachannel";
      case StatsType_Stream:          return "stream";
      case StatsType_Track:           return "track";
      case StatsType_ICEGatherer:     return "icegatherer";
      case StatsType_ICETransport:    return "icetransport";
      case StatsType_DTLSTransport:   return "dtlstransport";
      case StatsType_SRTPTransport:   return "srtptransport";
      case StatsType_Certificate:     return "certificate";
      case StatsType_Candidate:       return "candidate";
      case StatsType_CandidatePair:   return "candidatepair";
      case StatsType_LocalCandidate:  return "localcandidate";
      case StatsType_RemoteCandidate: return "remotecandidate";
    }

    return "undefined";
  }

  //---------------------------------------------------------------------------
  Optional<IStatsReportTypes::StatsICECandidatePairStates> IStatsReportTypes::toCandidatePairState(const char *type)
  {
    String str(type);
    for (IStatsReportTypes::StatsICECandidatePairStates index = IStatsReportTypes::StatsICECandidatePairState_First; index <= IStatsReportTypes::StatsICECandidatePairState_Last; index = static_cast<IStatsReportTypes::StatsICECandidatePairStates>(static_cast<std::underlying_type<IStatsReportTypes::StatsICECandidatePairStates>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IStatsReportTypes::toString(index))) return index;
    }

    return Optional<StatsICECandidatePairStates>();
  }

  //---------------------------------------------------------------------------
  const char *IStatsReportTypes::toString(StatsICECandidatePairStates state)
  {
    switch (state)
    {
      case StatsICECandidatePairState_Frozen:       return "frozen";
      case StatsICECandidatePairState_Waiting:      return "waiting";
      case StatsICECandidatePairState_InProgress:   return "inprogress";
      case StatsICECandidatePairState_Failed:       return "failed";
      case StatsICECandidatePairState_Succeeded:    return "succeeded";
      case StatsICECandidatePairState_Cancelled:    return "cancelled";
    }

    return "undefined";
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::Stats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::Stats::Stats() :
    mTimeStamp(zsLib::now())
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::Stats::Stats(const Stats &op2) :
    mTimeStamp(op2.mTimeStamp),
    mStatsType(op2.mStatsType),
    mID(op2.mID)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::Stats::Stats(ElementPtr rootEl)
  {
    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::Stats", "timestamp", mTimeStamp);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::Stats", "statsType", mStatsType);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::Stats", "id", mID);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::StatsPtr IStatsReportTypes::Stats::create(ElementPtr rootEl)
  {
    if (!rootEl) return StatsPtr();

    auto type = IStatsReportTypes::toStatsType(rootEl->getValue());
    if (!type.hasValue()) return make_shared<Stats>(rootEl);

    switch (type)
    {
      case StatsType_InboundRTP:      return InboundRTPStreamStats::create(rootEl);
      case StatsType_OutboundRTP:     return StatsPtr();
      case StatsType_Codec:           return Codec::create(rootEl);
      case StatsType_SCTPTransport:   return StatsPtr();
      case StatsType_DataChannel:     return StatsPtr();
      case StatsType_Stream:          return StatsPtr();
      case StatsType_Track:           return StatsPtr();
      case StatsType_ICEGatherer:     return StatsPtr();
      case StatsType_ICETransport:    return StatsPtr();
      case StatsType_DTLSTransport:   return StatsPtr();
      case StatsType_SRTPTransport:   return StatsPtr();
      case StatsType_Certificate:     return StatsPtr();
      case StatsType_Candidate:       return StatsPtr();
      case StatsType_CandidatePair:   return StatsPtr();
      case StatsType_LocalCandidate:  return StatsPtr();
      case StatsType_RemoteCandidate: return StatsPtr();
    }

    return StatsPtr();
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::StatsPtr IStatsReportTypes::Stats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(Stats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::Stats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Element::create(objectName);

    UseHelper::adoptElementValue(rootEl, "timestamp", mTimeStamp);
    UseHelper::adoptElementValue(rootEl, "statsType", mStatsType, false);
    UseHelper::adoptElementValue(rootEl, "id", mID, false);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::Stats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::Stats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::Stats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:Stats:");

    hasher.update(mTimeStamp);
    hasher.update(":");
    hasher.update(mStatsType);
    hasher.update(":");
    hasher.update(mID);

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::RTPStreamStats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::RTPStreamStats::RTPStreamStats(const RTPStreamStats &op2) :
    Stats(op2),
    mSSRC(op2.mSSRC),
    mAssociatedStatID(op2.mAssociatedStatID),
    mIsRemote(op2.mIsRemote),
    mMediaType(op2.mMediaType),
    mMediaTrackID(op2.mMediaTrackID),
    mTransportID(op2.mTransportID),
    mCodecID(op2.mCodecID),
    mFIRCount(op2.mFIRCount),
    mPLICount(op2.mPLICount),
    mNACKCount(op2.mNACKCount),
    mSLICount(op2.mSLICount)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::RTPStreamStats::RTPStreamStats(ElementPtr rootEl) :
    Stats(rootEl)
  {
    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::RTPStreamStats", "ssrc", mSSRC);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::RTPStreamStats", "associatedStatId", mAssociatedStatID);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::RTPStreamStats", "isRemote", mIsRemote);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::RTPStreamStats", "mediaType", mMediaType);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::RTPStreamStats", "mediaTrackId", mMediaTrackID);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::RTPStreamStats", "transportId", mTransportID);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::RTPStreamStats", "codecId", mCodecID);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::RTPStreamStats", "firCount", mFIRCount);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::RTPStreamStats", "pliCount", mPLICount);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::RTPStreamStats", "nackCount", mNACKCount);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::RTPStreamStats", "sliCount", mSLICount);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::RTPStreamStatsPtr IStatsReportTypes::RTPStreamStats::create(ElementPtr rootEl)
  {
    if (!rootEl) return RTPStreamStatsPtr();
    return make_shared<RTPStreamStats>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::RTPStreamStatsPtr IStatsReportTypes::RTPStreamStats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(RTPStreamStats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::RTPStreamStats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Stats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "ssrc", mSSRC);
    UseHelper::adoptElementValue(rootEl, "associatedStatId", mAssociatedStatID, false);
    UseHelper::adoptElementValue(rootEl, "isRemote", mIsRemote);
    UseHelper::adoptElementValue(rootEl, "mediaType", mMediaType, false);
    UseHelper::adoptElementValue(rootEl, "mediaTrackId", mMediaTrackID, false);
    UseHelper::adoptElementValue(rootEl, "transportId", mTransportID, false);
    UseHelper::adoptElementValue(rootEl, "codecId", mCodecID, false);
    UseHelper::adoptElementValue(rootEl, "firCount", mFIRCount);
    UseHelper::adoptElementValue(rootEl, "pliCount", mPLICount);
    UseHelper::adoptElementValue(rootEl, "nackCount", mNACKCount);
    UseHelper::adoptElementValue(rootEl, "sliCount", mSLICount);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::RTPStreamStats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::RTPStreamStats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::RTPStreamStats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:RTPStreamStats:");

    hasher.update(Stats::hash());

    hasher.update(mSSRC);
    hasher.update(":");
    hasher.update(mAssociatedStatID);
    hasher.update(":");
    hasher.update(mIsRemote);
    hasher.update(":");
    hasher.update(mMediaType);
    hasher.update(":");
    hasher.update(mMediaTrackID);
    hasher.update(":");
    hasher.update(mTransportID);
    hasher.update(":");
    hasher.update(mCodecID);
    hasher.update(":");
    hasher.update(mFIRCount);
    hasher.update(":");
    hasher.update(mPLICount);
    hasher.update(":");
    hasher.update(mNACKCount);
    hasher.update(":");
    hasher.update(mSLICount);

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::Codec
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::Codec::Codec(const Codec &op2) :
    Stats(op2),
    mPayloadType(op2.mPayloadType),
    mCodec(op2.mCodec),
    mClockRate(op2.mClockRate),
    mChannels(op2.mChannels),
    mParameters(op2.mParameters)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::Codec::Codec(ElementPtr rootEl) :
    Stats(rootEl)
  {
    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::Codec", "payloadType", mPayloadType);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::Codec", "codec", mCodec);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::Codec", "clockRate", mClockRate);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::Codec", "channels", mChannels);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::Codec", "parameters", mParameters);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::CodecPtr IStatsReportTypes::Codec::create(ElementPtr rootEl)
  {
    if (!rootEl) return CodecPtr();
    return make_shared<Codec>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::CodecPtr IStatsReportTypes::Codec::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(Codec, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::Codec::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Stats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "payloadType", mPayloadType);
    UseHelper::adoptElementValue(rootEl, "codec", mCodec, false);
    UseHelper::adoptElementValue(rootEl, "clockRate", mClockRate);
    UseHelper::adoptElementValue(rootEl, "channels", mChannels);
    UseHelper::adoptElementValue(rootEl, "parameters", mParameters, false);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::Codec::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::Codec");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::Codec::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:Codec:");

    hasher.update(Stats::hash());

    hasher.update(mPayloadType);
    hasher.update(":");
    hasher.update(mCodec);
    hasher.update(":");
    hasher.update(mClockRate);
    hasher.update(":");
    hasher.update(mChannels);
    hasher.update(":");
    hasher.update(mParameters);
    hasher.update(":");

    return hasher.final();
  }

  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::InboundRTPStreamStats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::InboundRTPStreamStats::InboundRTPStreamStats(const InboundRTPStreamStats &op2) :
    RTPStreamStats(op2),
    mPacketsReceived(op2.mPacketsReceived),
    mBytesReceived(op2.mBytesReceived),
    mPacketsLost(op2.mPacketsLost),
    mJitter(op2.mJitter),
    mFractionLost(op2.mFractionLost)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::InboundRTPStreamStats::InboundRTPStreamStats(ElementPtr rootEl) :
    RTPStreamStats(rootEl)
  {
    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::InboundRTPStreamStats", "packetsReceived", mPacketsReceived);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::InboundRTPStreamStats", "bytesReceived", mBytesReceived);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::InboundRTPStreamStats", "packetsLost", mPacketsLost);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::InboundRTPStreamStats", "jitter", mJitter);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::InboundRTPStreamStats", "fractionLost", mFractionLost);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::InboundRTPStreamStatsPtr IStatsReportTypes::InboundRTPStreamStats::create(ElementPtr rootEl)
  {
    if (!rootEl) return InboundRTPStreamStatsPtr();
    return make_shared<InboundRTPStreamStats>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::InboundRTPStreamStatsPtr IStatsReportTypes::InboundRTPStreamStats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(InboundRTPStreamStats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::InboundRTPStreamStats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = RTPStreamStats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "oacketsReceived", mPacketsReceived);
    UseHelper::adoptElementValue(rootEl, "bytesReceived", mBytesReceived);
    UseHelper::adoptElementValue(rootEl, "packetsLost", mPacketsLost);
    UseHelper::adoptElementValue(rootEl, "jitter", mJitter);
    UseHelper::adoptElementValue(rootEl, "fractionLost", mFractionLost);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::InboundRTPStreamStats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::InboundRTPStreamStats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::InboundRTPStreamStats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:InboundRTPStreamStats:");

    hasher.update(RTPStreamStats::hash());

    hasher.update(mPacketsReceived);
    hasher.update(":");
    hasher.update(mBytesReceived);
    hasher.update(":");
    hasher.update(mPacketsLost);
    hasher.update(":");
    hasher.update(mJitter);
    hasher.update(":");
    hasher.update(mFractionLost);
    hasher.update(":");

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IStatsReport::toDebug(IStatsReportPtr report)
  {
    return internal::StatsReport::toDebug(internal::StatsReport::convert(report));
  }

  //---------------------------------------------------------------------------
  IStatsReportPtr IStatsReport::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(IStatsReport, any);
  }


}
