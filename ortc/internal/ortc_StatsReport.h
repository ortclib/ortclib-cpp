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

#pragma once

#include <ortc/internal/types.h>

#include <ortc/IStatsReport.h>
#include <ortc/IStatsProvider.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>

//#define ORTC_SETTING_SRTP_TRANSPORT_WARN_OF_KEY_LIFETIME_EXHAUGSTION_WHEN_REACH_PERCENTAGE_USSED "ortc/srtp/warm-key-lifetime-exhaustion-when-reach-percentage-used"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IStatsReportForInternal);

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportForInternal
    //

    interaction IStatsReportForInternal
    {
      ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, ForInternal);

      ZS_DECLARE_TYPEDEF_PTR(IStatsReportTypes::Stats, Stats);

      ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::PromiseWithStatsReport, PromiseWithStatsReport);
      ZS_DECLARE_TYPEDEF_PTR(std::list<PromiseWithStatsReportPtr>, PromiseWithStatsReportList);

      typedef String StatID;
      typedef std::map<StatID, StatsPtr> StatMap;

      static StatsReportPtr create(const StatMap &stats) noexcept;

      static PromiseWithStatsReportPtr collectReports(
                                                      const PromiseWithStatsReportList &promises,
                                                      PromiseWithStatsReportPtr previouslyCreatedPromiseToResolve = PromiseWithStatsReportPtr()
                                                      ) noexcept;

      virtual ~IStatsReportForInternal() noexcept {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // StatsReport
    //
    
    class StatsReport : public Noop,
                        public MessageQueueAssociator,
                        public SharedRecursiveLock,
                        public IStatsReport,
                        public IStatsReportForInternal,
                        public IPromiseSettledDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IStatsReport;
      friend interaction IStatsReportFactory;

      ZS_DECLARE_TYPEDEF_PTR(IStatsReportTypes::Stats, Stats)

      ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::PromiseWithStatsReport, PromiseWithStatsReport);
      ZS_DECLARE_TYPEDEF_PTR(std::list<PromiseWithStatsReportPtr>, PromiseWithStatsReportList);

      typedef std::map<PUID, PromiseWithStatsReportPtr> PromiseWithStatsReportMap;

    public:
      StatsReport(
                  const make_private &,
                  IMessageQueuePtr queue,
                  const StatMap &stats
                  ) noexcept;

    protected:
      StatsReport(Noop) noexcept :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init() noexcept;

      void init(
                PromiseWithStatsReportPtr resolvePromise,
                const PromiseWithStatsReportList &promises
                ) noexcept;

    public:
      virtual ~StatsReport() noexcept;

      static StatsReportPtr convert(IStatsReportPtr object) noexcept;
      static StatsReportPtr convert(ForInternalPtr object) noexcept;

    protected:

      //-----------------------------------------------------------------------
      //
      // StatsReport => IStatsReport
      //

      static ElementPtr toDebug(StatsReportPtr report) noexcept;

      PUID getID() const noexcept override {return mID;}

      IDListPtr getStatesIDs() const noexcept override;
      StatsPtr getStats(const char *id) const noexcept override;

      //-----------------------------------------------------------------------
      //
      // StatsReport => IStatsReportForInternal
      //

      static StatsReportPtr create(const StatMap &stats) noexcept;

      static PromiseWithStatsReportPtr collectReports(
                                                      const PromiseWithStatsReportList &promises,
                                                      PromiseWithStatsReportPtr previouslyCreatedPromiseToResolve = PromiseWithStatsReportPtr()
                                                      ) noexcept;

      //-----------------------------------------------------------------------
      //
      // StatsReport => IPromiseSettledDelegate
      //

      void onPromiseSettled(PromisePtr promise) override;

    protected:
      //-----------------------------------------------------------------------
      //
      // StatsReport => (internal)
      //

      Log::Params log(const char *message) const noexcept;
      static Log::Params slog(const char *message) noexcept;
      Log::Params debug(const char *message) const noexcept;
      virtual ElementPtr toDebug() const noexcept;

      void cancel() noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // StatsReport => (data)
      //

      AutoPUID mID;
      StatsReportWeakPtr mThisWeak;

      PromiseWithStatsReportWeakPtr mResolvePromise;
      PromiseWithStatsReportMap mPendingResolution;

      StatMap mStats;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IStatsReportFactory
    //

    interaction IStatsReportFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal::StatMap, StatMap);
      ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::PromiseWithStatsReport, PromiseWithStatsReport);
      ZS_DECLARE_TYPEDEF_PTR(std::list<PromiseWithStatsReportPtr>, PromiseWithStatsReportList);

      static IStatsReportFactory &singleton() noexcept;

      virtual StatsReportPtr create(const StatMap &stats) noexcept;

      virtual PromiseWithStatsReportPtr collectReports(
                                                       const PromiseWithStatsReportList &promises,
                                                       PromiseWithStatsReportPtr previouslyCreatedPromiseToResolve = PromiseWithStatsReportPtr()
                                                       ) noexcept;
    };

    class StatsReportFactory : public IFactory<IStatsReportFactory> {};
  }
}
