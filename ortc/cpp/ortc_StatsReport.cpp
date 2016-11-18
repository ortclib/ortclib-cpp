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
#include <ortc/internal/ortc.stats.events.h>
#include <ortc/internal/platform.h>

#include <ortc/services/ISettings.h>
#include <ortc/services/IHelper.h>
#include <ortc/services/IHTTP.h>

#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>
#include <zsLib/date.h>
#include <zsLib/SafeInt.h>

#include <cryptopp/sha.h>


#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG

using namespace date;

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_stats) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::ISettings, UseSettings)
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP)

  ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper)

  using zsLib::Log;
  using zsLib::Numeric;

  typedef ortc::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

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
    static Log::Params slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::StatsReport");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    static double getTimestamp(const Time &originalTimestamp)
    {
      // Time since 1970-01-01T00:00:00Z in milliseconds.
      auto t = day_point(jan / 1 / 1601);

      auto diff = originalTimestamp - t;
      auto milli = zsLib::toMilliseconds(diff);
      return double(milli.count());
    }

    //-------------------------------------------------------------------------
    static void reportInt32(const char *reportID, double timestamp, const char *statName, int32 value)
    {
      ZS_EVENTING_4(
                    x, i, Debug, StatsReportInt32, ols, Stats, Info,
                    string, stat_group_name, reportID,
                    double, timestamp, timestamp,
                    string, stat_name, statName,
                    int32, stat_value, value
                    );
      ZS_EVENTING_ASSIGN_VALUE(StatsReportInt32, 101);
      ZS_LOG_INSANE(slog("report uint32") + ZS_PARAM("report id", reportID) + ZS_PARAM("stat name", statName) + ZS_PARAM("timestamp", timestamp) + ZS_PARAM("value", value));
    }

    //-------------------------------------------------------------------------
    static void reportInt64(const char *reportID, double timestamp, const char *statName, int64 value)
    {
      ZS_EVENTING_4(
                    x, i, Debug, StatsReportInt64, ols, Stats, Info,
                    string, stat_group_name, reportID,
                    double, timestamp, timestamp,
                    string, stat_name, statName,
                    int64, stat_value, value
                    );
      ZS_EVENTING_ASSIGN_VALUE(StatsReportInt64, 102);

      ZS_LOG_INSANE(slog("report uint64") + ZS_PARAM("report id", reportID) + ZS_PARAM("stat name", statName) + ZS_PARAM("timestamp", timestamp) + ZS_PARAM("value", value));
    }

    //-------------------------------------------------------------------------
    static void reportFloat(const char *reportID, double timestamp, const char *statName, float value)
    {
      ZS_EVENTING_4(
                    x, i, Debug, StatsReportFloat, ols, Stats, Info,
                    string, stat_group_name, reportID,
                    double, timestamp, timestamp,
                    string, stat_name, statName,
                    float, stat_value, value
                    );
      ZS_EVENTING_ASSIGN_VALUE(StatsReportFloat, 103);

      ZS_LOG_INSANE(slog("report float") + ZS_PARAM("report id", reportID) + ZS_PARAM("stat name", statName) + ZS_PARAM("timestamp", timestamp) + ZS_PARAM("value", value));
    }

    //-------------------------------------------------------------------------
    static void reportBool(const char *reportID, double timestamp, const char *statName, bool value)
    {
      ZS_EVENTING_4(
                    x, i, Debug, StatsReportBool, ols, Stats, Info,
                    string, stat_group_name, reportID,
                    double, timestamp, timestamp,
                    string, stat_name, statName,
                    bool, stat_value, value
                    );
      ZS_EVENTING_ASSIGN_VALUE(StatsReportBool, 105);
      ZS_LOG_INSANE(slog("report bool") + ZS_PARAM("report id", reportID) + ZS_PARAM("stat name", statName) + ZS_PARAM("timestamp", timestamp) + ZS_PARAM("value", value));
    }

    //-------------------------------------------------------------------------
    static void reportString(const char *reportID, double timestamp, const char *statName, const char *value)
    {
      ZS_EVENTING_4(
                    x, i, Debug, StatsReportString, ols, Stats, Info,
                    string, stat_group_name, reportID,
                    double, timestamp, timestamp,
                    string, stat_name, statName,
                    string, stat_value, value
                    );
      ZS_EVENTING_ASSIGN_VALUE(StatsReportString, 104);
      ZS_LOG_INSANE(slog("report string") + ZS_PARAM("report id", reportID) + ZS_PARAM("stat name", statName) + ZS_PARAM("timestamp", timestamp) + ZS_PARAM("value", value));
    }

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
    #pragma mark IStatsReportForInternal
    #pragma mark

    //-------------------------------------------------------------------------
    StatsReportPtr IStatsReportForInternal::create(const StatMap &stats)
    {
      return IStatsReportFactory::singleton().create(stats);
    }

    //-------------------------------------------------------------------------
    IStatsReportForInternal::PromiseWithStatsReportPtr IStatsReportForInternal::collectReports(
                                                                                               const PromiseWithStatsReportList &promises,
                                                                                               PromiseWithStatsReportPtr previouslyCreatedPromiseToResolve
                                                                                               )
    {
      return IStatsReportFactory::singleton().collectReports(promises, previouslyCreatedPromiseToResolve);
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
      ZS_LOG_DEBUG(debug("created"))
    }

    //-------------------------------------------------------------------------
    void StatsReport::init()
    {
    }

    //-------------------------------------------------------------------------
    void StatsReport::init(
                           PromiseWithStatsReportPtr resolvePromise,
                           const PromiseWithStatsReportList &promises
                           )
    {
      bool resolveNow = false;
      auto pThis = mThisWeak.lock();

      {
        AutoRecursiveLock lock(*this);

        mResolvePromise = resolvePromise;

        for (auto iter = promises.begin(); iter != promises.end(); ++iter) {
          auto &promise = (*iter);
          if (!promise) continue;

          mPendingResolution[promise->getID()] = promise;
          promise->thenWeak(pThis);
        }

        resolveNow = (mPendingResolution.size() < 1);
      }

      if (resolveNow) {
        ZS_LOG_TRACE(log("resolving immediately (as no collected promises are needing to be resolved)"));
        resolvePromise->resolve(pThis);
      }
    }

    //-------------------------------------------------------------------------
    StatsReport::~StatsReport()
    {
      if (isNoop()) return;

      ZS_LOG_DEBUG(log("destroyed"))
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
    StatsReport::PromiseWithStatsReportPtr StatsReport::collectReports(
                                                                       const PromiseWithStatsReportList &promises,
                                                                       PromiseWithStatsReportPtr previouslyCreatedPromiseToResolve
                                                                       )
    {
      StatsReportPtr pThis(make_shared<StatsReport>(make_private{}, IORTCForInternal::queueORTC(), StatMap()));
      pThis->mThisWeak = pThis;
      if (!previouslyCreatedPromiseToResolve) {
        previouslyCreatedPromiseToResolve = PromiseWithStatsReport::create(IORTCForInternal::queueDelegate());
      }
      pThis->init(previouslyCreatedPromiseToResolve, promises);
      previouslyCreatedPromiseToResolve->setReferenceHolder(pThis);
      return previouslyCreatedPromiseToResolve;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark StatsReport => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void StatsReport::onPromiseSettled(PromisePtr promise)
    {
      if (!promise) return;

      PromiseWithStatsReportPtr resolve;

      {
        AutoRecursiveLock lock(*this);
        
        auto found = mPendingResolution.find(promise->getID());
        if (found == mPendingResolution.end()) return;

        PromiseWithStatsReportPtr resolvedPromise = (*found).second;
        mPendingResolution.erase(found);

        IStatsReportPtr report = resolvedPromise->value();

        if (report) {
          auto ids = report->getStatesIDs();
          if (ids) {
            for (auto iter = ids->begin(); iter != ids->end(); ++iter) {
              auto &id = (*iter);
              auto stat = report->getStats(id);
              if (!stat) continue;
              mStats[id] = stat;
            }
          }
        }

        if (mPendingResolution.size() < 1) resolve = mResolvePromise.lock();
      }

      if (!resolve) return;

      ZS_LOG_TRACE(log("all promises are resolved"));

      resolve->resolve(mThisWeak.lock());
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

      UseServicesHelper::debugAppend(resultEl, "resolve promise", (bool)mResolvePromise.lock());
      UseServicesHelper::debugAppend(resultEl, "pending resolution", mPendingResolution.size());

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

    //-------------------------------------------------------------------------
    IStatsReportFactory::PromiseWithStatsReportPtr IStatsReportFactory::collectReports(
                                                                                       const PromiseWithStatsReportList &promises,
                                                                                       PromiseWithStatsReportPtr previouslyCreatedPromiseToResolve
                                                                                       )
    {
      if (this) {}
      return internal::StatsReport::collectReports(promises, previouslyCreatedPromiseToResolve);
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
  const char *IStatsReportTypes::toString(const Optional<StatsTypes> &type)
  {
    if (!type.hasValue()) return "";
    return toString(type.value());
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
  bool IStatsReportTypes::StatsTypeSet::hasStatType(StatsTypes type) const
  {
    if (size() < 1) return true;

    auto found = find(type);
    if (found == end()) return false;
    return true;
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
    mTimestamp(zsLib::now()),
    mStatsType(IStatsReportTypes::StatsType_First)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::Stats::Stats(const Stats &op2) :
    mTimestamp(op2.mTimestamp),
    mStatsType(op2.mStatsType),
    mStatsTypeOther(op2.mStatsTypeOther),
    mID(op2.mID)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::Stats::Stats(ElementPtr rootEl)
  {
    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::Stats", "timestamp", mTimestamp);
    {
      String value;
      UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::Stats", "statsType", value);
      mStatsType = IStatsReportTypes::toStatsType(value);
      if (!mStatsType.hasValue()) {
        mStatsTypeOther = value;
      }
    }
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
      case StatsType_OutboundRTP:     return OutboundRTPStreamStats::create(rootEl);
      case StatsType_Codec:           return Codec::create(rootEl);
      case StatsType_SCTPTransport:   return SCTPTransportStats::create(rootEl);
      case StatsType_DataChannel:     return DataChannelStats::create(rootEl);
      case StatsType_Stream:          return MediaStreamStats::create(rootEl);
      case StatsType_Track:           return MediaStreamTrackStats::create(rootEl);
      case StatsType_ICEGatherer:     return ICEGathererStats::create(rootEl);
      case StatsType_ICETransport:    return ICETransportStats::create(rootEl);
      case StatsType_DTLSTransport:   return DTLSTransportStats::create(rootEl);
      case StatsType_SRTPTransport:   return SRTPTransportStats::create(rootEl);
      case StatsType_Certificate:     return CertificateStats::create(rootEl);
      case StatsType_Candidate:       return ICECandidateAttributes::create(rootEl);
      case StatsType_CandidatePair:   return ICECandidatePairStats::create(rootEl);
      case StatsType_LocalCandidate:  return ICECandidateAttributes::create(rootEl);
      case StatsType_RemoteCandidate: return ICECandidateAttributes::create(rootEl);
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

    UseHelper::adoptElementValue(rootEl, "timestamp", mTimestamp);
    UseHelper::adoptElementValue(rootEl, "statsType", statsType(), false);
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

    hasher.update(mTimestamp);
    hasher.update(":");
    hasher.update(mStatsType);
    hasher.update(":");
    hasher.update(mID);

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::Stats::eventTrace() const
  {
    auto timestamp = internal::getTimestamp(mTimestamp);
    eventTrace(timestamp);
    internal::reportString(mID, timestamp, "statsTypeFinal", statsType());
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::Stats::eventTrace(double timestamp) const
  {
    internal::reportString(mID, timestamp, "statsType", statsType());
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
  void IStatsReportTypes::RTPStreamStats::eventTrace(double timestamp) const
  {
    Stats::eventTrace(timestamp);

    if (mSSRC.hasValue()) {
      internal::reportInt64(mID, timestamp, "ssrc", SafeInt<int64>(mSSRC.value()));
    }
    internal::reportString(mID, timestamp, "associatedStatId", mAssociatedStatID);
    internal::reportBool(mID, timestamp, "isRemote", mIsRemote);
    internal::reportString(mID, timestamp, "mediaType", mMediaType);
    internal::reportString(mID, timestamp, "mediaTrackId", mMediaTrackID);
    internal::reportString(mID, timestamp, "transportId", mTransportID);
    internal::reportString(mID, timestamp, "codecId", mCodecID);
    internal::reportInt32(mID, timestamp, "firCount", SafeInt<int32>(mFIRCount));
    internal::reportInt32(mID, timestamp, "pliCount", SafeInt<int32>(mPLICount));
    internal::reportInt32(mID, timestamp, "nackCount", SafeInt<int32>(mNACKCount));
    internal::reportInt32(mID, timestamp, "sliCount", SafeInt<int32>(mSLICount));
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
    mStatsType = IStatsReportTypes::StatsType_Codec;

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
  void IStatsReportTypes::Codec::eventTrace(double timestamp) const
  {
    Stats::eventTrace(timestamp);

    if (mPayloadType.hasValue()) {
      internal::reportInt32(mID, timestamp, "payloadType", SafeInt<int32>(mPayloadType.value()));
    }
    internal::reportString(mID, timestamp, "codec", mCodec);
    internal::reportInt32(mID, timestamp, "clockRate", SafeInt<int32>(mClockRate));
    if (mChannels.hasValue()) {
      internal::reportInt32(mID, timestamp, "channels", SafeInt<int32>(mChannels.value()));
    }
    internal::reportString(mID, timestamp, "parameters", mParameters);
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
    mFractionLost(op2.mFractionLost),
    mEndToEndDelay(op2.mEndToEndDelay)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::InboundRTPStreamStats::InboundRTPStreamStats(ElementPtr rootEl) :
    RTPStreamStats(rootEl)
  {
    mStatsType = IStatsReportTypes::StatsType_InboundRTP;

    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::InboundRTPStreamStats", "packetsReceived", mPacketsReceived);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::InboundRTPStreamStats", "bytesReceived", mBytesReceived);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::InboundRTPStreamStats", "packetsLost", mPacketsLost);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::InboundRTPStreamStats", "jitter", mJitter);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::InboundRTPStreamStats", "fractionLost", mFractionLost);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::InboundRTPStreamStats", "endToEndDelay", mEndToEndDelay);
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

    UseHelper::adoptElementValue(rootEl, "packetsReceived", mPacketsReceived);
    UseHelper::adoptElementValue(rootEl, "bytesReceived", mBytesReceived);
    UseHelper::adoptElementValue(rootEl, "packetsLost", mPacketsLost);
    UseHelper::adoptElementValue(rootEl, "jitter", mJitter);
    UseHelper::adoptElementValue(rootEl, "fractionLost", mFractionLost);
    UseHelper::adoptElementValue(rootEl, "endToEndDelay", mEndToEndDelay);

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
    hasher.update(mEndToEndDelay);
    hasher.update(":");

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::InboundRTPStreamStats::eventTrace(double timestamp) const
  {
    RTPStreamStats::eventTrace(timestamp);

    internal::reportInt32(mID, timestamp, "packetsReceived", SafeInt<int32>(mPacketsReceived));
    internal::reportInt64(mID, timestamp, "bytesReceived", SafeInt<int64>(mBytesReceived));
    internal::reportInt32(mID, timestamp, "packetsLost", SafeInt<int32>(mPacketsLost));
    internal::reportFloat(mID, timestamp, "jitter", static_cast<float>(mJitter));
    internal::reportFloat(mID, timestamp, "fractionLost", static_cast<float>(mFractionLost));
    internal::reportInt64(mID, timestamp, "endToEndDelay", SafeInt<int64>(mEndToEndDelay.count()));
#ifndef ORTC_EXCLUDE_WEBRTC_COMPATIBILITY_STATS
    internal::reportInt64(mID, timestamp, "winrtEndToEndDelayMs", SafeInt<int64>(mEndToEndDelay.count()));
    internal::reportInt32(mID, timestamp, "googFirsReceived", SafeInt<int32>(mFIRCount));
    internal::reportInt32(mID, timestamp, "googPlisReceived", SafeInt<int32>(mPLICount));
    internal::reportInt32(mID, timestamp, "googNacksReceived", SafeInt<int32>(mNACKCount));
#endif //ndef ORTC_EXCLUDE_WEBRTC_COMPATIBILITY_STATS
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::OutboundRTPStreamStats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::OutboundRTPStreamStats::OutboundRTPStreamStats(const OutboundRTPStreamStats &op2) :
    RTPStreamStats(op2),
    mPacketsSent(op2.mPacketsSent),
    mBytesSent(op2.mBytesSent),
    mTargetBitrate(op2.mTargetBitrate),
    mRoundTripTime(op2.mRoundTripTime)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::OutboundRTPStreamStats::OutboundRTPStreamStats(ElementPtr rootEl) :
    RTPStreamStats(rootEl)
  {
    mStatsType = IStatsReportTypes::StatsType_OutboundRTP;

    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::OutboundRTPStreamStats", "packetsSent", mPacketsSent);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::OutboundRTPStreamStats", "bytesSent", mBytesSent);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::OutboundRTPStreamStats", "targetBitrate", mTargetBitrate);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::OutboundRTPStreamStats", "roundTripTime", mRoundTripTime);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::OutboundRTPStreamStatsPtr IStatsReportTypes::OutboundRTPStreamStats::create(ElementPtr rootEl)
  {
    if (!rootEl) return OutboundRTPStreamStatsPtr();
    return make_shared<OutboundRTPStreamStats>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::OutboundRTPStreamStatsPtr IStatsReportTypes::OutboundRTPStreamStats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(OutboundRTPStreamStats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::OutboundRTPStreamStats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = RTPStreamStats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "packetsSent", mPacketsSent);
    UseHelper::adoptElementValue(rootEl, "bytesSent", mBytesSent);
    UseHelper::adoptElementValue(rootEl, "targetBitrate", mTargetBitrate);
    UseHelper::adoptElementValue(rootEl, "roundTripTime", mRoundTripTime);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::OutboundRTPStreamStats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::OutboundRTPStreamStats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::OutboundRTPStreamStats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:OutboundRTPStreamStats:");

    hasher.update(RTPStreamStats::hash());

    hasher.update(mPacketsSent);
    hasher.update(":");
    hasher.update(mBytesSent);
    hasher.update(":");
    hasher.update(mTargetBitrate);
    hasher.update(":");
    hasher.update(mRoundTripTime);

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::OutboundRTPStreamStats::eventTrace(double timestamp) const
  {
    RTPStreamStats::eventTrace(timestamp);

    internal::reportInt32(mID, timestamp, "packetsSent", SafeInt<int32>(mPacketsSent));
    internal::reportInt64(mID, timestamp, "bytesSent", SafeInt<int64>(mBytesSent));
    internal::reportFloat(mID, timestamp, "targetBitrate", static_cast<float>(mTargetBitrate));
    internal::reportFloat(mID, timestamp, "roundTripTime", static_cast<float>(mRoundTripTime));
#ifndef ORTC_EXCLUDE_WEBRTC_COMPATIBILITY_STATS
    internal::reportFloat(mID, timestamp, "googRtt", static_cast<float>(mRoundTripTime));
    internal::reportInt32(mID, timestamp, "googFirsSent", SafeInt<int32>(mFIRCount));
    internal::reportInt32(mID, timestamp, "googPlisSent", SafeInt<int32>(mPLICount));
    internal::reportInt32(mID, timestamp, "googNacksSent", SafeInt<int32>(mNACKCount));
#endif //ORTC_EXCLUDE_WEBRTC_COMPATIBILITY_STATS
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::SCTPTransportStats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::SCTPTransportStats::SCTPTransportStats(const SCTPTransportStats &op2) :
    Stats(op2),
    mDataChannelsOpened(op2.mDataChannelsOpened),
    mDataChannelsClosed(op2.mDataChannelsClosed)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::SCTPTransportStats::SCTPTransportStats(ElementPtr rootEl) :
    Stats(rootEl)
  {
    mStatsType = IStatsReportTypes::StatsType_SCTPTransport;

    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::SCTPTransportStats", "dataChannelsOpened", mDataChannelsOpened);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::SCTPTransportStats", "dataChannelsClosed", mDataChannelsClosed);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::SCTPTransportStatsPtr IStatsReportTypes::SCTPTransportStats::create(ElementPtr rootEl)
  {
    if (!rootEl) return SCTPTransportStatsPtr();
    return make_shared<SCTPTransportStats>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::SCTPTransportStatsPtr IStatsReportTypes::SCTPTransportStats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(SCTPTransportStats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::SCTPTransportStats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Stats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "dataChannelsOpened", mDataChannelsOpened);
    UseHelper::adoptElementValue(rootEl, "dataChannelsClosed", mDataChannelsClosed);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::SCTPTransportStats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::SCTPTransportStats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::SCTPTransportStats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:SCTPTransportStats:");

    hasher.update(Stats::hash());

    hasher.update(mDataChannelsOpened);
    hasher.update(":");
    hasher.update(mDataChannelsClosed);

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::SCTPTransportStats::eventTrace(double timestamp) const
  {
    Stats::eventTrace(timestamp);

    internal::reportInt32(mID, timestamp, "dataChannelsOpen", SafeInt<int32>(mDataChannelsOpened));
    internal::reportInt32(mID, timestamp, "dataChannelsClosed", SafeInt<int32>(mDataChannelsClosed));
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::MediaStreamStats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::MediaStreamStats::MediaStreamStats(const MediaStreamStats &op2) :
    Stats(op2),
    mStreamID(op2.mStreamID),
    mTrackIDs(op2.mTrackIDs)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::MediaStreamStats::MediaStreamStats(ElementPtr rootEl) :
    Stats(rootEl)
  {
    mStatsType = IStatsReportTypes::StatsType_Stream;

    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::SCTPTransportStats", "dataChannelsOpened", mStreamID);

    {
      ElementPtr trackIDsEl = rootEl->findFirstChildElement("trackIds");
      if (trackIDsEl) {
        ElementPtr trackIDEl = trackIDsEl->findFirstChildElement("trackId");
        while (trackIDEl) {
          String value = UseServicesHelper::getElementTextAndDecode(trackIDEl);
          trackIDEl = trackIDEl->findNextSiblingElement("trackId");

          if (value.isEmpty()) continue;
          mTrackIDs.push_front(value);
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::MediaStreamStatsPtr IStatsReportTypes::MediaStreamStats::create(ElementPtr rootEl)
  {
    if (!rootEl) return MediaStreamStatsPtr();
    return make_shared<MediaStreamStats>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::MediaStreamStatsPtr IStatsReportTypes::MediaStreamStats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(MediaStreamStats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::MediaStreamStats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Stats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "streamId", mStreamID, false);

    if (mTrackIDs.size() > 0) {
      ElementPtr trackIDsEl = Element::create("trackIds");
      for (auto iter = mTrackIDs.begin(); iter != mTrackIDs.end(); ++iter) {
        auto &value = *(iter);
        UseHelper::adoptElementValue(trackIDsEl, "trackId", value, false);
      }
      if (trackIDsEl->hasChildren()) {
        rootEl->adoptAsLastChild(trackIDsEl);
      }
    }

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::MediaStreamStats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::MediaStreamStats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::MediaStreamStats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:MediaStreamStats:");

    hasher.update(Stats::hash());

    hasher.update(mStreamID);
    hasher.update(":tracks:c94ff2e0568fae77366ca8824b1e22852f6933ae");

    for (auto iter = mTrackIDs.begin(); iter != mTrackIDs.end(); ++iter) {
      auto &value = (*iter);
      hasher.update(":");
      hasher.update(value);
    }
    hasher.update(":tracks");

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::MediaStreamStats::eventTrace(double timestamp) const
  {
    Stats::eventTrace(timestamp);

    internal::reportString(mID, timestamp, "streamId", mStreamID);
    internal::reportInt32(mID, timestamp, "tracks", SafeInt<int32>(mTrackIDs.size()));
    unsigned long index = 0;
    for (auto iter = mTrackIDs.begin(); iter != mTrackIDs.end(); ++iter, ++index) {
      auto &trackID = (*iter);
      internal::reportString(mID, timestamp, (String("trackId") + string(index)).c_str(), mStreamID);
    }
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::MediaStreamTrackStats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::MediaStreamTrackStats::MediaStreamTrackStats(const MediaStreamTrackStats &op2) :
    Stats(op2),
    mTrackID(op2.mTrackID),
    mRemoteSource(op2.mRemoteSource),
    mSSRCIDs(op2.mSSRCIDs),
    mFrameWidth(op2.mFrameWidth),
    mFrameHeight(op2.mFrameHeight),
    mFramesPerSecond(op2.mFramesPerSecond),
    mFramesSent(op2.mFramesSent),
    mFramesReceived(op2.mFramesReceived),
    mFramesDecoded(op2.mFramesDecoded),
    mFramesDropped(op2.mFramesDropped),
    mFramesCorrupted(op2.mFramesCorrupted),
    mAudioLevel(op2.mAudioLevel),
    mEchoReturnLoss(op2.mEchoReturnLoss),
    mEchoReturnLossEnhancement(op2.mEchoReturnLossEnhancement)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::MediaStreamTrackStats::MediaStreamTrackStats(ElementPtr rootEl) :
    Stats(rootEl)
  {
    mStatsType = IStatsReportTypes::StatsType_Track;

    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::MediaStreamTrackStats", "trackId", mTrackID);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::MediaStreamTrackStats", "remoteSource", mRemoteSource);

    {
      ElementPtr trackIDsEl = rootEl->findFirstChildElement("ssrcIds");
      if (trackIDsEl) {
        ElementPtr trackIDEl = trackIDsEl->findFirstChildElement("ssrcId");
        while (trackIDEl) {
          String value = UseServicesHelper::getElementText(trackIDEl);
          trackIDEl = trackIDEl->findNextSiblingElement("ssrcId");
          if (value.isEmpty()) continue;
          try {
            //HERE
            SSRCType ssrcId = Numeric<SSRCType>(value);
            mSSRCIDs.push_front(ssrcId);
          } catch (const Numeric<SSRCType>::ValueOutOfRange &) {
            ZS_LOG_WARNING(Debug, internal::slog("ssrc value out of range") + ZS_PARAM("value", value));
          }
        }
      }
    }

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::MediaStreamTrackStats", "frameWidth", mFrameWidth);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::MediaStreamTrackStats", "frameHeight", mFrameHeight);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::MediaStreamTrackStats", "framesPerSecond", mFramesPerSecond);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::MediaStreamTrackStats", "framesSent", mFramesSent);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::MediaStreamTrackStats", "framesReceived", mFramesReceived);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::MediaStreamTrackStats", "framesDecoded", mFramesDecoded);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::MediaStreamTrackStats", "framesCorrupted", mFramesCorrupted);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::MediaStreamTrackStats", "audioLevel", mAudioLevel);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::MediaStreamTrackStats", "echoReturnLoss", mEchoReturnLoss);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::MediaStreamTrackStats", "echoReturnLossEnhancement", mEchoReturnLossEnhancement);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::MediaStreamTrackStatsPtr IStatsReportTypes::MediaStreamTrackStats::create(ElementPtr rootEl)
  {
    if (!rootEl) return MediaStreamTrackStatsPtr();
    return make_shared<MediaStreamTrackStats>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::MediaStreamTrackStatsPtr IStatsReportTypes::MediaStreamTrackStats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(MediaStreamTrackStats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::MediaStreamTrackStats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Stats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "trackId", mTrackID, false);
    UseHelper::adoptElementValue(rootEl, "remoteSource", mRemoteSource);

    if (mSSRCIDs.size() > 0) {
      ElementPtr ssrcIDsEl = Element::create("ssrcIds");
      for (auto iter = mSSRCIDs.begin(); iter != mSSRCIDs.end(); ++iter) {
        auto &value = *(iter);
        UseHelper::adoptElementValue(ssrcIDsEl, "ssrcId", value);
      }
      if (ssrcIDsEl->hasChildren()) {
        rootEl->adoptAsLastChild(ssrcIDsEl);
      }
    }

    UseHelper::adoptElementValue(rootEl, "frameWidth", mFrameWidth);
    UseHelper::adoptElementValue(rootEl, "frameHeight", mFrameHeight);
    UseHelper::adoptElementValue(rootEl, "framesPerSecond", mFramesPerSecond);
    UseHelper::adoptElementValue(rootEl, "framesSent", mFramesSent);
    UseHelper::adoptElementValue(rootEl, "framesReceived", mFramesReceived);
    UseHelper::adoptElementValue(rootEl, "framesDecoded", mFramesDecoded);
    UseHelper::adoptElementValue(rootEl, "framesDropped", mFramesDropped);
    UseHelper::adoptElementValue(rootEl, "framesCorrupted", mFramesCorrupted);
    UseHelper::adoptElementValue(rootEl, "audioLevel", mAudioLevel);
    UseHelper::adoptElementValue(rootEl, "echoReturnLoss", mEchoReturnLoss);
    UseHelper::adoptElementValue(rootEl, "echoReturnLossEnhancement", mEchoReturnLossEnhancement);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::MediaStreamTrackStats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::MediaStreamTrackStats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::MediaStreamTrackStats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:MediaStreamTrackStats:");

    hasher.update(Stats::hash());

    hasher.update(mTrackID);
    hasher.update(":");
    hasher.update(mRemoteSource);

    hasher.update(":ssrcs:4a2f24cc4bfc91dbd040942775fb3818851495c7");

    for (auto iter = mSSRCIDs.begin(); iter != mSSRCIDs.end(); ++iter) {
      auto &value = (*iter);
      hasher.update(":");
      hasher.update(value);
    }
    hasher.update(":ssrcs:");
    hasher.update(mFrameWidth);
    hasher.update(":");
    hasher.update(mFrameHeight);
    hasher.update(":");
    hasher.update(mFramesPerSecond);
    hasher.update(":");
    hasher.update(mFramesSent);
    hasher.update(":");
    hasher.update(mFramesReceived);
    hasher.update(":");
    hasher.update(mFramesDecoded);
    hasher.update(":");
    hasher.update(mFramesDropped);
    hasher.update(":");
    hasher.update(mFramesCorrupted);
    hasher.update(":");
    hasher.update(mAudioLevel);
    hasher.update(":");
    hasher.update(mEchoReturnLoss);
    hasher.update(":");
    hasher.update(mEchoReturnLossEnhancement);

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::MediaStreamTrackStats::eventTrace(double timestamp) const
  {
    Stats::eventTrace(timestamp);

    internal::reportString(mID, timestamp, "trackId", mTrackID);
    internal::reportBool(mID, timestamp, "remoteSource", mRemoteSource);
    internal::reportInt32(mID, timestamp, "ssrcs", SafeInt<int32>(mSSRCIDs.size()));
    unsigned long index = 0;
    for (auto iter = mSSRCIDs.begin(); iter != mSSRCIDs.end(); ++iter, ++index) {
      auto &ssrcID = (*iter);
      internal::reportInt64(mID, timestamp, (String("ssrcId") + string(index)).c_str(), SafeInt<int64>(ssrcID));
    }
    internal::reportInt32(mID, timestamp, "frameWidth", SafeInt<int32>(mFrameWidth));
    internal::reportInt32(mID, timestamp, "frameHeight", SafeInt<int32>(mFrameHeight));
    internal::reportFloat(mID, timestamp, "framesPerSecond", static_cast<float>(mFramesPerSecond));
    internal::reportInt32(mID, timestamp, "framesSent", SafeInt<int32>(mFramesSent));
    internal::reportInt32(mID, timestamp, "framesReceived", SafeInt<int32>(mFramesReceived));
    internal::reportInt32(mID, timestamp, "framesDecoded", SafeInt<int32>(mFramesDecoded));
    internal::reportInt32(mID, timestamp, "framesDropped", SafeInt<int32>(mFramesDropped));
    internal::reportInt32(mID, timestamp, "framesCorrupted", SafeInt<int32>(mFramesCorrupted));
    internal::reportFloat(mID, timestamp, "audioLevel", static_cast<float>(mAudioLevel));
    internal::reportFloat(mID, timestamp, "echoReturnLoss", static_cast<float>(mEchoReturnLoss));
    internal::reportFloat(mID, timestamp, "echoReturnLossEnhancement", static_cast<float>(mEchoReturnLossEnhancement));

#ifndef ORTC_EXCLUDE_WEBRTC_COMPATIBILITY_STATS
    if (mRemoteSource) {
      internal::reportFloat(mID, timestamp, "googFrameHeightReceived", static_cast<float>(mFrameWidth));
      internal::reportFloat(mID, timestamp, "googFrameWidthReceived", static_cast<float>(mFrameHeight));
      internal::reportInt32(mID, timestamp, "googFrameRateReceived", SafeInt<int32>(mFramesPerSecond));
      internal::reportFloat(mID, timestamp, "audioOutputLevel", static_cast<float>(mAudioLevel));
    } else {
      internal::reportFloat(mID, timestamp, "googFrameWidthSent", static_cast<float>(mFrameWidth));
      internal::reportFloat(mID, timestamp, "googFrameHeightSent", static_cast<float>(mFrameHeight));
      internal::reportInt32(mID, timestamp, "googFrameRateSent", SafeInt<int32>(mFramesPerSecond));
      internal::reportFloat(mID, timestamp, "audioInputLevel", static_cast<float>(mAudioLevel));
    }
#endif //ORTC_EXCLUDE_WEBRTC_COMPATIBILITY_STATS
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::DataChannelStats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::DataChannelStats::DataChannelStats(const DataChannelStats &op2) :
    Stats(op2),
    mLabel(op2.mLabel),
    mProtocol(op2.mProtocol),
    mDataChannelID(op2.mDataChannelID),
    mState(op2.mState),
    mMessagesSent(op2.mMessagesSent),
    mBytesSent(op2.mBytesSent),
    mMessagesReceived(op2.mMessagesReceived),
    mBytesReceived(op2.mBytesReceived)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::DataChannelStats::DataChannelStats(ElementPtr rootEl) :
    Stats(rootEl)
  {
    mStatsType = IStatsReportTypes::StatsType_DataChannel;

    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::DataChannelStats", "label", mLabel);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::DataChannelStats", "protocol", mProtocol);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::DataChannelStats", "datachannelId", mDataChannelID);

    {
      String str;
      UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::DataChannelStats", "state", str);
      if (str.hasData()) {
        mState = IDataChannelTypes::toState(str);
      }
    }

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::DataChannelStats", "messagesSent", mMessagesSent);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::DataChannelStats", "bytesSent", mBytesSent);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::DataChannelStats", "messagesReceived", mMessagesReceived);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::DataChannelStats", "bytesReceived", mBytesReceived);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::DataChannelStatsPtr IStatsReportTypes::DataChannelStats::create(ElementPtr rootEl)
  {
    if (!rootEl) return DataChannelStatsPtr();
    return make_shared<DataChannelStats>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::DataChannelStatsPtr IStatsReportTypes::DataChannelStats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(DataChannelStats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::DataChannelStats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Stats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "label", mLabel, false);
    UseHelper::adoptElementValue(rootEl, "protocol", mProtocol, false);
    UseHelper::adoptElementValue(rootEl, "datachannelId", mDataChannelID);
    UseHelper::adoptElementValue(rootEl, "state", IDataChannelTypes::toString(mState), false);
    UseHelper::adoptElementValue(rootEl, "messagesSent", mMessagesSent);
    UseHelper::adoptElementValue(rootEl, "bytesSent", mBytesSent);
    UseHelper::adoptElementValue(rootEl, "messagesReceived", mMessagesReceived);
    UseHelper::adoptElementValue(rootEl, "bytesReceived", mBytesReceived);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::DataChannelStats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::DataChannelStats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::DataChannelStats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:DataChannelStats:");

    hasher.update(Stats::hash());

    hasher.update(mLabel);
    hasher.update(":");
    hasher.update(mProtocol);
    hasher.update(":");
    hasher.update(mDataChannelID);
    hasher.update(":");
    hasher.update(IDataChannelTypes::toString(mState));
    hasher.update(":");
    hasher.update(mMessagesSent);
    hasher.update(":");
    hasher.update(mBytesSent);
    hasher.update(":");
    hasher.update(mMessagesReceived);
    hasher.update(":");
    hasher.update(mBytesReceived);
    hasher.update(":");

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::DataChannelStats::eventTrace(double timestamp) const
  {
    Stats::eventTrace(timestamp);

    internal::reportString(mID, timestamp, "label", mLabel);
    internal::reportString(mID, timestamp, "protocol", mProtocol);
    internal::reportInt32(mID, timestamp, "dataChannelId", SafeInt<int32>(mDataChannelID));
    internal::reportString(mID, timestamp, "state", IDataChannelTypes::toString(mState));
    internal::reportInt32(mID, timestamp, "messagesSent", SafeInt<int32>(mMessagesSent));
    internal::reportInt64(mID, timestamp, "bytesSent", SafeInt<int64>(mBytesSent));
    internal::reportInt32(mID, timestamp, "messagesReceived", SafeInt<int32>(mMessagesReceived));
    internal::reportInt64(mID, timestamp, "bytesReceived", SafeInt<int64>(mBytesReceived));
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::ICEGathererStats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICEGathererStats::ICEGathererStats(const ICEGathererStats &op2) :
    Stats(op2),
    mBytesSent(op2.mBytesSent),
    mBytesReceived(op2.mBytesReceived),
    mRTCPGathererStatsID(op2.mRTCPGathererStatsID)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICEGathererStats::ICEGathererStats(ElementPtr rootEl) :
    Stats(rootEl)
  {
    mStatsType = IStatsReportTypes::StatsType_ICEGatherer;

    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICEGathererStats", "bytesSent", mBytesSent);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICEGathererStats", "bytesReceived", mBytesReceived);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICEGathererStats", "rtcpGathererStatsId", mRTCPGathererStatsID);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICEGathererStatsPtr IStatsReportTypes::ICEGathererStats::create(ElementPtr rootEl)
  {
    if (!rootEl) return ICEGathererStatsPtr();
    return make_shared<ICEGathererStats>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICEGathererStatsPtr IStatsReportTypes::ICEGathererStats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(ICEGathererStats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::ICEGathererStats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Stats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "bytesSent", mBytesSent);
    UseHelper::adoptElementValue(rootEl, "bytesReceived", mBytesReceived);
    UseHelper::adoptElementValue(rootEl, "rtcpGathererStatsId", mRTCPGathererStatsID, false);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::ICEGathererStats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::ICEGathererStats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::ICEGathererStats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:ICEGathererStats:");

    hasher.update(Stats::hash());

    hasher.update(mBytesSent);
    hasher.update(":");
    hasher.update(mBytesReceived);
    hasher.update(":");
    hasher.update(mRTCPGathererStatsID);
    hasher.update(":");

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::ICEGathererStats::eventTrace(double timestamp) const
  {
    Stats::eventTrace(timestamp);

    internal::reportInt64(mID, timestamp, "bytesSent", SafeInt<int64>(mBytesSent));
    internal::reportInt64(mID, timestamp, "bytesSent", SafeInt<int64>(mBytesReceived));
    internal::reportString(mID, timestamp, "rtcpGathererStatsId", mRTCPGathererStatsID);
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::ICETransportStats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICETransportStats::ICETransportStats(const ICETransportStats &op2) :
    Stats(op2),
    mBytesSent(op2.mBytesSent),
    mBytesReceived(op2.mBytesReceived),
    mRTCPTransportStatsID(op2.mRTCPTransportStatsID),
    mActiveConnection(op2.mActiveConnection),
    mSelectedCandidatePairID(op2.mSelectedCandidatePairID)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICETransportStats::ICETransportStats(ElementPtr rootEl) :
    Stats(rootEl)
  {
    mStatsType = IStatsReportTypes::StatsType_ICETransport;

    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICETransportStats", "bytesSent", mBytesSent);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICETransportStats", "bytesReceived", mBytesReceived);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICETransportStats", "rtcpTransportStatsId", mRTCPTransportStatsID);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICETransportStats", "activeConnection", mActiveConnection);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICETransportStats", "selectedCandidatePairId", mSelectedCandidatePairID);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICETransportStatsPtr IStatsReportTypes::ICETransportStats::create(ElementPtr rootEl)
  {
    if (!rootEl) return ICETransportStatsPtr();
    return make_shared<ICETransportStats>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICETransportStatsPtr IStatsReportTypes::ICETransportStats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(ICETransportStats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::ICETransportStats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Stats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "bytesSent", mBytesSent);
    UseHelper::adoptElementValue(rootEl, "bytesReceived", mBytesReceived);
    UseHelper::adoptElementValue(rootEl, "rtcpTransportStatsId", mRTCPTransportStatsID, false);
    UseHelper::adoptElementValue(rootEl, "activeConnection", mActiveConnection);
    UseHelper::adoptElementValue(rootEl, "selectedCandidatePairId", mSelectedCandidatePairID, false);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::ICETransportStats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::ICETransportStats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::ICETransportStats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:ICETransportStats:");

    hasher.update(Stats::hash());

    hasher.update(mBytesSent);
    hasher.update(":");
    hasher.update(mBytesReceived);
    hasher.update(":");
    hasher.update(mRTCPTransportStatsID);
    hasher.update(":");
    hasher.update(mActiveConnection);
    hasher.update(":");
    hasher.update(mSelectedCandidatePairID);
    hasher.update(":");

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::ICETransportStats::eventTrace(double timestamp) const
  {
    Stats::eventTrace(timestamp);

    internal::reportInt64(mID, timestamp, "bytesSent", SafeInt<int64>(mBytesSent));
    internal::reportInt64(mID, timestamp, "bytesSent", SafeInt<int64>(mBytesReceived));
    internal::reportString(mID, timestamp, "rtcpTransportStatsId", mRTCPTransportStatsID);
    internal::reportBool(mID, timestamp, "activeConnection", mActiveConnection);
    internal::reportString(mID, timestamp, "selectedCandidatePairId", mSelectedCandidatePairID);
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::DTLSTransportStats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::DTLSTransportStats::DTLSTransportStats(const DTLSTransportStats &op2) :
    Stats(op2),
    mLocalCertificateID(op2.mLocalCertificateID),
    mRemoteCertificateID(op2.mRemoteCertificateID)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::DTLSTransportStats::DTLSTransportStats(ElementPtr rootEl) :
    Stats(rootEl)
  {
    mStatsType = IStatsReportTypes::StatsType_DTLSTransport;

    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::DTLSTransportStats", "localCertificateId", mLocalCertificateID);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::DTLSTransportStats", "remoteCertificateId", mRemoteCertificateID);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::DTLSTransportStatsPtr IStatsReportTypes::DTLSTransportStats::create(ElementPtr rootEl)
  {
    if (!rootEl) return DTLSTransportStatsPtr();
    return make_shared<DTLSTransportStats>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::DTLSTransportStatsPtr IStatsReportTypes::DTLSTransportStats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(DTLSTransportStats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::DTLSTransportStats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Stats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "localCertificateId", mLocalCertificateID, false);
    UseHelper::adoptElementValue(rootEl, "remoteCertificateId", mRemoteCertificateID, false);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::DTLSTransportStats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::DTLSTransportStats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::DTLSTransportStats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:DTLSTransportStats:");

    hasher.update(Stats::hash());

    hasher.update(mLocalCertificateID);
    hasher.update(":");
    hasher.update(mRemoteCertificateID);
    hasher.update(":");

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::DTLSTransportStats::eventTrace(double timestamp) const
  {
    Stats::eventTrace(timestamp);

    internal::reportString(mID, timestamp, "localCertificateId", mLocalCertificateID);
    internal::reportString(mID, timestamp, "remoteCertificateId", mRemoteCertificateID);
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::SRTPTransportStats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::SRTPTransportStats::SRTPTransportStats(const SRTPTransportStats &op2) :
    Stats(op2)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::SRTPTransportStats::SRTPTransportStats(ElementPtr rootEl) :
    Stats(rootEl)
  {
    mStatsType = IStatsReportTypes::StatsType_SRTPTransport;

    if (!rootEl) return;
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::SRTPTransportStatsPtr IStatsReportTypes::SRTPTransportStats::create(ElementPtr rootEl)
  {
    if (!rootEl) return SRTPTransportStatsPtr();
    return make_shared<SRTPTransportStats>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::SRTPTransportStatsPtr IStatsReportTypes::SRTPTransportStats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(SRTPTransportStats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::SRTPTransportStats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Stats::createElement(objectName);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::SRTPTransportStats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::SRTPTransportStats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::SRTPTransportStats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:SRTPTransportStats:");

    hasher.update(Stats::hash());

    hasher.update(":");

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::SRTPTransportStats::eventTrace(double timestamp) const
  {
    Stats::eventTrace(timestamp);
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::ICECandidateAttributes
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICECandidateAttributes::ICECandidateAttributes(const ICECandidateAttributes &op2) :
    Stats(op2),
    mRelatedID(op2.mRelatedID),
    mIPAddress(op2.mIPAddress),
    mPortNumber(op2.mPortNumber),
    mTransport(op2.mTransport),
    mCandidateType(op2.mCandidateType),
    mPriority(op2.mPriority),
    mAddressSourceURL(op2.mAddressSourceURL)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICECandidateAttributes::ICECandidateAttributes(ElementPtr rootEl) :
    Stats(rootEl)
  {
    mStatsType = IStatsReportTypes::StatsType_Candidate;

    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidateAttributes", "relatedId", mRelatedID);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidateAttributes", "ipAddress", mIPAddress);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidateAttributes", "portNumber", mPortNumber);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidateAttributes", "transport", mTransport);

    {
      String str;
      UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidateAttributes", "candidateType", str);
      if (str.hasData()) {
        try {
          mCandidateType = IICETypes::toCandidateType(str);
        } catch (const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, internal::slog("candidate type is not value") + ZS_PARAM("type", str));
        }
      }
    }

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidateAttributes", "priority", mPriority);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidateAttributes", "addressSourceUrl", mAddressSourceURL);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICECandidateAttributesPtr IStatsReportTypes::ICECandidateAttributes::create(ElementPtr rootEl)
  {
    if (!rootEl) return ICECandidateAttributesPtr();
    return make_shared<ICECandidateAttributes>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICECandidateAttributesPtr IStatsReportTypes::ICECandidateAttributes::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(ICECandidateAttributes, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::ICECandidateAttributes::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Stats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "relatedId", mRelatedID, false);
    UseHelper::adoptElementValue(rootEl, "ipAddress", mIPAddress, false);
    UseHelper::adoptElementValue(rootEl, "portNumber", mPortNumber);
    UseHelper::adoptElementValue(rootEl, "transport", mTransport, false);
    UseHelper::adoptElementValue(rootEl, "candidateType", IICETypes::toString(mCandidateType), false);
    UseHelper::adoptElementValue(rootEl, "priority", mPriority);
    UseHelper::adoptElementValue(rootEl, "addressSourceUrl", mAddressSourceURL, false);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::ICECandidateAttributes::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::ICECandidateAttributes");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::ICECandidateAttributes::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:ICECandidateAttributes:");

    hasher.update(Stats::hash());

    hasher.update(mRelatedID);
    hasher.update(":");
    hasher.update(mIPAddress);
    hasher.update(":");
    hasher.update(mPortNumber);
    hasher.update(":");
    hasher.update(mTransport);
    hasher.update(":");
    hasher.update(IICETypes::toString(mCandidateType));
    hasher.update(":");
    hasher.update(mPriority);
    hasher.update(":");
    hasher.update(mAddressSourceURL);
    hasher.update(":");

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::ICECandidateAttributes::eventTrace(double timestamp) const
  {
    Stats::eventTrace(timestamp);

    internal::reportString(mID, timestamp, "relatedId", mRelatedID);
    internal::reportString(mID, timestamp, "ipAddress", mIPAddress);
    internal::reportInt32(mID, timestamp, "portNumber", SafeInt<int32>(mPortNumber));
    internal::reportString(mID, timestamp, "transport", mTransport);
    internal::reportString(mID, timestamp, "candidateType", IICETypes::toString(mCandidateType));
    internal::reportInt64(mID, timestamp, "priority", SafeInt<int64>(mPriority));
    internal::reportString(mID, timestamp, "addressSourceUrl", mAddressSourceURL);
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::ICECandidatePairStats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICECandidatePairStats::ICECandidatePairStats(const ICECandidatePairStats &op2) :
    Stats(op2),
    mTransportID(op2.mTransportID),
    mLocalCandidateID(op2.mLocalCandidateID),
    mRemoteCandidateID(op2.mRemoteCandidateID),
    mState(op2.mState),
    mPriority(op2.mPriority),
    mNominated(op2.mNominated),
    mWritable(op2.mWritable),
    mReadable(op2.mReadable),
    mBytesSent(op2.mBytesSent),
    mBytesReceived(op2.mBytesReceived),
    mRoundTripTime(op2.mRoundTripTime),
    mAvailableOutgoingBitrate(op2.mAvailableOutgoingBitrate),
    mAvailableIncomingBitrate(op2.mAvailableIncomingBitrate)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICECandidatePairStats::ICECandidatePairStats(ElementPtr rootEl) :
    Stats(rootEl)
  {
    mStatsType = IStatsReportTypes::StatsType_CandidatePair;

    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidatePairStats", "transportId", mTransportID);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidatePairStats", "localCandidateId", mLocalCandidateID);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidatePairStats", "remoteCandidateId", mRemoteCandidateID);

    {
      String str;
      UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidatePairStats", "state", str);
      if (str.hasData()) {
        try {
          mState = IStatsReportTypes::toCandidatePairState(str);
        } catch (const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, internal::slog("candidate pair state is not value") + ZS_PARAM("type", str));
        }
      }
    }

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidatePairStats", "priority", mPriority);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidatePairStats", "nominated", mNominated);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidatePairStats", "writable", mWritable);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidatePairStats", "readable", mReadable);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidatePairStats", "bytesSent", mBytesSent);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidatePairStats", "bytesReceived", mBytesReceived);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidatePairStats", "roundTripTime", mRoundTripTime);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidatePairStats", "availableOutgoingBitrate", mAvailableOutgoingBitrate);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::ICECandidatePairStats", "availableIncomingBitrate", mAvailableIncomingBitrate);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICECandidatePairStatsPtr IStatsReportTypes::ICECandidatePairStats::create(ElementPtr rootEl)
  {
    if (!rootEl) return ICECandidatePairStatsPtr();
    return make_shared<ICECandidatePairStats>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::ICECandidatePairStatsPtr IStatsReportTypes::ICECandidatePairStats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(ICECandidatePairStats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::ICECandidatePairStats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Stats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "transportId", mTransportID, false);
    UseHelper::adoptElementValue(rootEl, "localCandidateId", mLocalCandidateID, false);
    UseHelper::adoptElementValue(rootEl, "remoteCandidateId", mRemoteCandidateID, false);
    UseHelper::adoptElementValue(rootEl, "state", IStatsReportTypes::toString(mState), false);
    UseHelper::adoptElementValue(rootEl, "priority", mPriority);
    UseHelper::adoptElementValue(rootEl, "nominated", mNominated);
    UseHelper::adoptElementValue(rootEl, "writable", mWritable);
    UseHelper::adoptElementValue(rootEl, "readable", mReadable);
    UseHelper::adoptElementValue(rootEl, "bytesSent", mBytesSent);
    UseHelper::adoptElementValue(rootEl, "bytesReceived", mBytesReceived);
    UseHelper::adoptElementValue(rootEl, "roundTripTime", mRoundTripTime);
    UseHelper::adoptElementValue(rootEl, "availableOutgoingBitrate", mAvailableOutgoingBitrate);
    UseHelper::adoptElementValue(rootEl, "availableIncomingBitrate", mAvailableIncomingBitrate);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::ICECandidatePairStats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::ICECandidatePairStats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::ICECandidatePairStats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:ICECandidatePairStats:");

    hasher.update(Stats::hash());

    hasher.update(mTransportID);
    hasher.update(":");
    hasher.update(mLocalCandidateID);
    hasher.update(":");
    hasher.update(mRemoteCandidateID);
    hasher.update(":");
    hasher.update(IStatsReportTypes::toString(mState));
    hasher.update(":");
    hasher.update(mPriority);
    hasher.update(":");
    hasher.update(mNominated);
    hasher.update(":");
    hasher.update(mWritable);
    hasher.update(":");
    hasher.update(mReadable);
    hasher.update(":");
    hasher.update(mBytesSent);
    hasher.update(":");
    hasher.update(mRoundTripTime);
    hasher.update(":");
    hasher.update(mAvailableOutgoingBitrate);
    hasher.update(":");
    hasher.update(mAvailableIncomingBitrate);
    hasher.update(":");

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::ICECandidatePairStats::eventTrace(double timestamp) const
  {
    Stats::eventTrace(timestamp);

    internal::reportString(mID, timestamp, "transportId", mTransportID);
    internal::reportString(mID, timestamp, "localCandidateId", mLocalCandidateID);
    internal::reportString(mID, timestamp, "remoteCandidateId", mRemoteCandidateID);
    internal::reportString(mID, timestamp, "state", IStatsReportTypes::toString(mState));
    internal::reportInt64(mID, timestamp, "priority", static_cast<int64>(mPriority));
    internal::reportBool(mID, timestamp, "nominated", mNominated);
    internal::reportBool(mID, timestamp, "writable", mWritable);
    internal::reportBool(mID, timestamp, "readable", mReadable);
    internal::reportInt64(mID, timestamp, "bytesSent", SafeInt<int64>(mBytesSent));
    internal::reportInt64(mID, timestamp, "bytesReceived", SafeInt<int64>(mBytesReceived));
    internal::reportFloat(mID, timestamp, "roundTripTime", static_cast<float>(mRoundTripTime));
    internal::reportFloat(mID, timestamp, "availableOutgoingBitrate", static_cast<float>(mAvailableOutgoingBitrate));
    internal::reportFloat(mID, timestamp, "availableIncomingBitrate", static_cast<float>(mAvailableIncomingBitrate));
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IStatsReportTypes::CertificateStats
  #pragma mark

  //---------------------------------------------------------------------------
  IStatsReportTypes::CertificateStats::CertificateStats(const CertificateStats &op2) :
    Stats(op2),
    mFingerprint(op2.mFingerprint),
    mFingerprintAlgorithm(op2.mFingerprintAlgorithm),
    mBase64Certificate(op2.mBase64Certificate),
    mIssuerCertificateID(op2.mIssuerCertificateID)
  {
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::CertificateStats::CertificateStats(ElementPtr rootEl) :
    Stats(rootEl)
  {
    mStatsType = IStatsReportTypes::StatsType_Certificate;

    if (!rootEl) return;

    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::CertificateStats", "fingerprint", mFingerprint);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::CertificateStats", "fingerprintAlgorithm", mFingerprintAlgorithm);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::CertificateStats", "base64Certificate", mBase64Certificate);
    UseHelper::getElementValue(rootEl, "ortc::IStatsReportTypes::CertificateStats", "issuerCertificateID", mIssuerCertificateID);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::CertificateStatsPtr IStatsReportTypes::CertificateStats::create(ElementPtr rootEl)
  {
    if (!rootEl) return CertificateStatsPtr();
    return make_shared<CertificateStats>(rootEl);
  }

  //---------------------------------------------------------------------------
  IStatsReportTypes::CertificateStatsPtr IStatsReportTypes::CertificateStats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(CertificateStats, any);
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::CertificateStats::createElement(const char *objectName) const
  {
    ElementPtr rootEl = Stats::createElement(objectName);

    UseHelper::adoptElementValue(rootEl, "fingerprint", mFingerprint, false);
    UseHelper::adoptElementValue(rootEl, "fingerprintAlgorithm", mFingerprintAlgorithm, false);
    UseHelper::adoptElementValue(rootEl, "base64Certificate", mBase64Certificate, false);
    UseHelper::adoptElementValue(rootEl, "issuerCertificateID", mIssuerCertificateID, false);

    if (!rootEl->hasChildren()) return ElementPtr();

    return rootEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr IStatsReportTypes::CertificateStats::toDebug() const
  {
    return Element::create("ortc::IStatsReportTypes::CertificateStats");
  }

  //---------------------------------------------------------------------------
  String IStatsReportTypes::CertificateStats::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IStatsReportTypes:CertificateStats:");

    hasher.update(Stats::hash());

    hasher.update(mFingerprint);
    hasher.update(":");
    hasher.update(mFingerprintAlgorithm);
    hasher.update(":");
    hasher.update(mBase64Certificate);
    hasher.update(":");
    hasher.update(mIssuerCertificateID);
    hasher.update(":");

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  void IStatsReportTypes::CertificateStats::eventTrace(double timestamp) const
  {
    Stats::eventTrace(timestamp);

    internal::reportString(mID, timestamp, "fingerprint", mFingerprint);
    internal::reportString(mID, timestamp, "fingerprintAlgorithm", mFingerprintAlgorithm);
    internal::reportString(mID, timestamp, "base64Certificate", mBase64Certificate);
    internal::reportString(mID, timestamp, "issuerCertificateId", mIssuerCertificateID);
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
