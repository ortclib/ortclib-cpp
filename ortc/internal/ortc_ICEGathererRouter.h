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

#include <ortc/internal/types.h>

#include <ortc/IICEGatherer.h>

#include <zsLib/ITimer.h>

#include <tuple>

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICEGathererRouter
    #pragma mark

    class ICEGathererRouter : public SharedRecursiveLock,
                              public MessageQueueAssociator,
                              public zsLib::ITimerDelegate
    {
    protected:
      struct make_private {};

    public:
      ZS_DECLARE_STRUCT_PTR(Route)

      ZS_DECLARE_TYPEDEF_PTR(IICETypes::Candidate, Candidate)

      typedef String LocalCandidateHash;
      typedef std::pair<LocalCandidateHash, IPAddress> CandidateRemoteIPPair;
      typedef std::map<CandidateRemoteIPPair, RouteWeakPtr> CandidateRemoteIPToRouteMap;

    public:
      ICEGathererRouter(
                        const make_private &,
                        IMessageQueuePtr queue
                        );

    protected:
      void init();

    public:
      ~ICEGathererRouter();

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGathererRouter => (friends)
      #pragma mark

      static ElementPtr toDebug(ICEGathererRouterPtr router);

      static ICEGathererRouterPtr create();

      virtual PUID getID() const {return mID;}

      virtual RoutePtr findRoute(
                                 CandidatePtr localCandidate,
                                 const IPAddress &remoteIP,
                                 bool createRouteIfNeeded
                                 );

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGathererRouter => ITimerDelegate
      #pragma mark

      void onTimer(ITimerPtr timer);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGathererRouter => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      static Log::Params slog(const char *message);
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;

      void cancel();

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGathererRouter::Route
      #pragma mark

      struct Route
      {
        AutoPUID mID;

        CandidatePtr mLocalCandidate;
        IPAddress mRemoteIP;

        void trace(const char *function, const char *message = NULL) const;
        ElementPtr toDebug() const;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICEGathererRouter (data)
      #pragma mark

      AutoPUID mID;
      ICEGathererRouterWeakPtr mThisWeak;

      CandidateRemoteIPToRouteMap mRoutes;

      ITimerPtr mTimer;
    };
  }
}
