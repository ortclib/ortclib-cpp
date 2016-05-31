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

#include <ortc/internal/ortc_ICEGathererRouter.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_Tracing.h>
#include <ortc/internal/platform.h>

#include <ortc/IICETransport.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/XML.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_icegatherer_router) }

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererRouter
    #pragma mark

    //-------------------------------------------------------------------------
    ICEGathererRouter::ICEGathererRouter(
                                         const make_private &,
                                         IMessageQueuePtr queue
                                         ) :
      SharedRecursiveLock(SharedRecursiveLock::create()),
      MessageQueueAssociator(queue)
    {
      EventWriteOrtcIceGathererRouterCreate(__func__, mID);
      ZS_LOG_BASIC(log("created"))
    }

    //-------------------------------------------------------------------------
    void ICEGathererRouter::init()
    {
      AutoRecursiveLock lock(*this);
      mTimer = Timer::create(mThisWeak.lock(), Seconds(30));
    }

    //-------------------------------------------------------------------------
    ICEGathererRouter::~ICEGathererRouter()
    {
      mThisWeak.reset();
      ZS_LOG_BASIC(log("destroyed"))
      cancel();
      EventWriteOrtcIceGathererRouterDestroy(__func__, mID);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererRouter => (friends)
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr ICEGathererRouter::toDebug(ICEGathererRouterPtr router)
    {
      if (!router) return ElementPtr();
      return router->toDebug();
    }

    //-------------------------------------------------------------------------
    ICEGathererRouterPtr ICEGathererRouter::create()
    {
      ICEGathererRouterPtr pThis(make_shared<ICEGathererRouter>(make_private {}, IORTCForInternal::queueORTC()));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    ICEGathererRouter::RoutePtr ICEGathererRouter::findRoute(
                                                             CandidatePtr localCandidate,
                                                             const IPAddress &remoteIP,
                                                             bool createRouteIfNeeded
                                                             )
    {
      AutoRecursiveLock lock(*this);

      LocalCandidateHash hash = localCandidate ? localCandidate->hash() : String();

      CandidateRemoteIPPair search(hash, remoteIP);

      auto found = mRoutes.find(search);
      if (found != mRoutes.end()) {
        RoutePtr route = (*found).second.lock();

        if (route) {
          EventWriteOrtcIceGathererRouterInternalEvent(__func__, mID, "found", hash, ((bool)localCandidate) ? localCandidate->mIP : String(), ((bool)localCandidate) ? localCandidate->mPort : 0, remoteIP.string());
          route->trace(__func__, "found");
          ZS_LOG_TRACE(log("route found") + route->toDebug() + ZS_PARAM("create route", createRouteIfNeeded))
          return route;
        }

        EventWriteOrtcIceGathererRouterInternalEvent(__func__, mID, "gone", hash, ((bool)localCandidate) ? localCandidate->mIP : String(), ((bool)localCandidate) ? localCandidate->mPort : 0, remoteIP.string());
        ZS_LOG_WARNING(Debug, log("route was previously found but is now gone") + (localCandidate ? localCandidate->toDebug() : ElementPtr()) + ZS_PARAM("remote ip", remoteIP.string()) + ZS_PARAM("create route", createRouteIfNeeded))
        mRoutes.erase(found);
      }

      if (!createRouteIfNeeded) {
        EventWriteOrtcIceGathererRouterInternalEvent(__func__, mID, "not found", hash, ((bool)localCandidate) ? localCandidate->mIP : String(), ((bool)localCandidate) ? localCandidate->mPort : 0, remoteIP.string());
        ZS_LOG_WARNING(Trace, log("route does not exist") + (localCandidate ? localCandidate->toDebug() : ElementPtr()) + ZS_PARAM("remote ip", remoteIP.string()))
        return RoutePtr();
      }

      RoutePtr route(make_shared<Route>());
      route->mLocalCandidate = localCandidate ? make_shared<Candidate>(*localCandidate) : CandidatePtr();
      route->mRemoteIP = remoteIP;

      EventWriteOrtcIceGathererRouterInternalEvent(__func__, mID, "created", hash, ((bool)localCandidate) ? localCandidate->mIP : String(), ((bool)localCandidate) ? localCandidate->mPort : 0, remoteIP.string());
      route->trace(__func__, "created");

      mRoutes[search] = route;

      ZS_LOG_DEBUG(log("route created") + route->toDebug())

      return route;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererRouter => (friends)
    #pragma mark

    //-------------------------------------------------------------------------
    void ICEGathererRouter::onTimer(TimerPtr timer)
    {
      ZS_LOG_DEBUG(log("on timer"))

      AutoRecursiveLock lock(*this);

      for (auto iter_doNotUse = mRoutes.begin(); iter_doNotUse != mRoutes.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto route = (*current).second.lock();

        auto candidateHash = ((*current).first).first;
        auto remoteIP = ((*current).first).second;

        if (route) {
          EventWriteOrtcIceGathererRouterInternalEvent(__func__, mID, "keep", candidateHash, NULL, 0, remoteIP.string());
          route->trace(__func__, "keep");
          ZS_LOG_TRACE(log("route still in use") + ZS_PARAM("candidate hash", candidateHash) + ZS_PARAM("remote ip", remoteIP.string()))
          continue;
        }

        EventWriteOrtcIceGathererRouterInternalEvent(__func__, mID, "prune", candidateHash, NULL, 0, remoteIP.string());
        ZS_LOG_TRACE(log("pruning route") + ZS_PARAM("candidate hash", candidateHash) + ZS_PARAM("remote ip", remoteIP.string()))
        mRoutes.erase(current);
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererRouter => (internals)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params ICEGathererRouter::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::ICEGathererRouter");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params ICEGathererRouter::slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::ICEGathererRouter");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params ICEGathererRouter::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr ICEGathererRouter::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::ICEGathererRouter");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "routes", mRoutes.size());

      UseServicesHelper::debugAppend(resultEl, "timer", mTimer ? mTimer->getID() : 0);

      return resultEl;
    }
    
    //-------------------------------------------------------------------------
    void ICEGathererRouter::cancel()
    {
      EventWriteOrtcIceGathererRouterCancel(__func__, mID);
      if (mTimer) {
        mTimer->cancel();
        mTimer.reset();
      }

      mRoutes.clear();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICEGathererRouter::Route
    #pragma mark

    //-------------------------------------------------------------------------
    void ICEGathererRouter::Route::trace(
                                         const char *function,
                                         const char *message
                                         ) const
    {
      if (mLocalCandidate) {
        EventWriteOrtcIceGathererRouterRouteTrace(
                                                  __func__,
                                                  function,
                                                  message,
                                                  mID,
                                                  mLocalCandidate->mInterfaceType,
                                                  mLocalCandidate->mFoundation,
                                                  mLocalCandidate->mPriority,
                                                  mLocalCandidate->mUnfreezePriority,
                                                  IICETypes::toString(mLocalCandidate->mProtocol),
                                                  mLocalCandidate->mIP,
                                                  mLocalCandidate->mPort,
                                                  IICETypes::toString(mLocalCandidate->mCandidateType),
                                                  IICETypes::toString(mLocalCandidate->mTCPType),
                                                  mLocalCandidate->mRelatedAddress,
                                                  mLocalCandidate->mRelatedPort,
                                                  mRemoteIP.string()
                                                  );
      } else {
        EventWriteOrtcIceGathererRouterRouteTrace(
                                                  __func__,
                                                  function,
                                                  message,
                                                  mID,
                                                  NULL,
                                                  NULL,
                                                  0,
                                                  0,
                                                  NULL,
                                                  NULL,
                                                  0,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  0,
                                                  mRemoteIP.string()
                                                  );
      }
    }

    //-------------------------------------------------------------------------
    ElementPtr ICEGathererRouter::Route::toDebug() const
    {
      ElementPtr objectEl = Element::create("ortc::ICEGathererRouter::Route");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      UseServicesHelper::debugAppend(objectEl, "local candidate", mLocalCandidate ? mLocalCandidate->toDebug() : ElementPtr());
      UseServicesHelper::debugAppend(objectEl, "remote ip", mRemoteIP.string());
      return objectEl;
    }

  }
}
