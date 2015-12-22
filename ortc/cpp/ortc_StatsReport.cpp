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
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

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
    IStatsReportTypes::StatsPtr StatsReport::getStats(const char *id)
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
  IStatsReportTypes::StatsPtr IStatsReportTypes::Stats::convert(AnyPtr any)
  {
    return ZS_DYNAMIC_PTR_CAST(Stats, any);
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
