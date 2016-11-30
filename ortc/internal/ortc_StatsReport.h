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
    #pragma mark
    #pragma mark IStatsReportForInternal
    #pragma mark

    interaction IStatsReportForInternal
    {
      ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal, ForInternal);

      ZS_DECLARE_TYPEDEF_PTR(IStatsReportTypes::Stats, Stats);

      ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::PromiseWithStatsReport, PromiseWithStatsReport);
      ZS_DECLARE_TYPEDEF_PTR(std::list<PromiseWithStatsReportPtr>, PromiseWithStatsReportList);

      typedef String StatID;
      typedef std::map<StatID, StatsPtr> StatMap;

      static StatsReportPtr create(const StatMap &stats);

      static PromiseWithStatsReportPtr collectReports(
                                                      const PromiseWithStatsReportList &promises,
                                                      PromiseWithStatsReportPtr previouslyCreatedPromiseToResolve = PromiseWithStatsReportPtr()
                                                      );

      virtual ~IStatsReportForInternal() {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark StatsReport
    #pragma mark
    
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
                  );

    protected:
      StatsReport(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

      void init(
                PromiseWithStatsReportPtr resolvePromise,
                const PromiseWithStatsReportList &promises
                );

    public:
      virtual ~StatsReport();

      static StatsReportPtr convert(IStatsReportPtr object);
      static StatsReportPtr convert(ForInternalPtr object);

    protected:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark StatsReport => IStatsReport
      #pragma mark

      static ElementPtr toDebug(StatsReportPtr report);

      virtual PUID getID() const override {return mID;}

      virtual IDListPtr getStatesIDs() const override;
      virtual StatsPtr getStats(const char *id) const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark StatsReport => IStatsReportForInternal
      #pragma mark

      static StatsReportPtr create(const StatMap &stats);

      static PromiseWithStatsReportPtr collectReports(
                                                      const PromiseWithStatsReportList &promises,
                                                      PromiseWithStatsReportPtr previouslyCreatedPromiseToResolve = PromiseWithStatsReportPtr()
                                                      );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark StatsReport => IPromiseSettledDelegate
      #pragma mark

      virtual void onPromiseSettled(PromisePtr promise) override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark StatsReport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      static Log::Params slog(const char *message);
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      void cancel();

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark StatsReport => (data)
      #pragma mark

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
    #pragma mark
    #pragma mark IStatsReportFactory
    #pragma mark

    interaction IStatsReportFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IStatsReportForInternal::StatMap, StatMap);
      ZS_DECLARE_TYPEDEF_PTR(IStatsProviderTypes::PromiseWithStatsReport, PromiseWithStatsReport);
      ZS_DECLARE_TYPEDEF_PTR(std::list<PromiseWithStatsReportPtr>, PromiseWithStatsReportList);

      static IStatsReportFactory &singleton();

      virtual StatsReportPtr create(const StatMap &stats);

      virtual PromiseWithStatsReportPtr collectReports(
                                                       const PromiseWithStatsReportList &promises,
                                                       PromiseWithStatsReportPtr previouslyCreatedPromiseToResolve = PromiseWithStatsReportPtr()
                                                       );
    };

    class StatsReportFactory : public IFactory<IStatsReportFactory> {};
  }
}
