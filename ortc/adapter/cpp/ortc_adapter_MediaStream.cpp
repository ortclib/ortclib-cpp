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

#define ZS_DECLARE_TEMPLATE_GENERATE_IMPLEMENTATION

#include <ortc/adapter/internal/ortc_adapter_MediaStream.h>

#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_StatsReport.h>

#include <ortc/internal/ortc_ORTC.h>

#include <ortc/services/IHelper.h>

#include <zsLib/eventing/Log.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

namespace ortc { namespace adapter { ZS_DECLARE_SUBSYSTEM(org_ortc_adapter); } }

namespace ortc
{
  namespace adapter
  {
    namespace internal
    {
      ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper);
      ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IORTCForInternal, UseORTC);

      ZS_DECLARE_USING_PTR(ortc::internal, MediaStreamTrack);
      ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IStatsReportForInternal, UseStatsReport);

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // IMediaStreamForPeerConnection
      //

      IMediaStreamForPeerConnection::ForPeerConnectionPtr IMediaStreamForPeerConnection::create(const char *id) noexcept
      {
        return internal::IMediaStreamFactory::singleton().create(id);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // MediaStream
      //

      //-------------------------------------------------------------------------
      MediaStream::MediaStream(
                               const make_private &,
                               IMessageQueuePtr queue,
                               IMediaStreamDelegatePtr delegate,
                               const UseMediaStreamTrackList &tracks,
                               const char *id
                               ) noexcept :
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create()),
        mMSID(NULL == id ? UseServicesHelper::randomString(36) : String(id)),
        mTracks(tracks)
      {
        std::set<String> ids;
        for (auto iter_doNotUse = mTracks.begin(); iter_doNotUse != mTracks.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto track = (*current);
          auto trackID = track->id();
          auto found = ids.find(trackID);
          if (found == ids.end()) {
            ids.insert(trackID);
            continue;
          }
          mTracks.erase(current);
        }

        mDefaultSubscription = mSubscriptions.subscribe(delegate, UseORTC::queueDelegate());

        ZS_LOG_DEBUG(log("created"));
      }

      //-------------------------------------------------------------------------
      void MediaStream::init() noexcept
      {
      }

      //-------------------------------------------------------------------------
      MediaStream::~MediaStream() noexcept
      {
        mThisWeak.reset();

        ZS_LOG_DEBUG(log("destroyed"));
      }

      //-------------------------------------------------------------------------
      MediaStreamPtr MediaStream::convert(IMediaStreamPtr object) noexcept
      {
        return ZS_DYNAMIC_PTR_CAST(MediaStream, object);
      }

      //-------------------------------------------------------------------------
      MediaStreamPtr MediaStream::convert(ForPeerConnectionPtr object) noexcept
      {
        return ZS_DYNAMIC_PTR_CAST(MediaStream, object);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // MediaStream => IMediaStream
      //

      //-----------------------------------------------------------------------
      ElementPtr MediaStream::toDebug(IMediaStreamPtr object) noexcept
      {
        if (!object) return ElementPtr();
        return MediaStream::convert(object)->toDebug();
      }

      //-----------------------------------------------------------------------
      MediaStreamPtr MediaStream::create(
                                         IMediaStreamDelegatePtr delegate,
                                         IMediaStreamPtr stream
                                         ) noexcept
      {
        UseMediaStreamTrackList tempTracks;

        if (stream)
        {
          auto mediaStream = MediaStream::convert(stream);
          AutoRecursiveLock lock(*mediaStream);
          tempTracks = mediaStream->mTracks;
        }

        MediaStreamPtr pThis(make_shared<MediaStream>(make_private{}, UseORTC::queueORTC(), delegate, tempTracks));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      MediaStreamPtr MediaStream::create(
                                         IMediaStreamDelegatePtr delegate,
                                         const MediaStreamTrackList &tracks
                                         ) noexcept
      {
        UseMediaStreamTrackList tempTracks;
        for (auto iter = tracks.begin(); iter != tracks.end(); ++iter)
        {
          auto track = (*iter);
          ZS_ASSERT(nullptr != track);
          tempTracks.push_back(MediaStreamTrack::convert(track));
        }

        MediaStreamPtr pThis(make_shared<MediaStream>(make_private{}, UseORTC::queueORTC(), delegate, tempTracks));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      IMediaStreamSubscriptionPtr MediaStream::subscribe(IMediaStreamDelegatePtr originalDelegate) noexcept
      {
        ZS_LOG_DETAIL(log("subscribing to media stream"));

        AutoRecursiveLock lock(*this);
        if (!originalDelegate) return mDefaultSubscription;

        auto subscription = mSubscriptions.subscribe(originalDelegate, UseORTC::queueDelegate());

        auto delegate = mSubscriptions.delegate(subscription, true);

        if (delegate) {
          auto pThis = mThisWeak.lock();

#pragma ZS_BUILD_NOTE("TODO","Fire any missed events")
        }

        if (isShutdown()) {
          mSubscriptions.clear();
        }

        return subscription;
      }

      //-----------------------------------------------------------------------
      String MediaStream::id() const noexcept
      {
        return mMSID;
      }

      //-----------------------------------------------------------------------
      bool MediaStream::active() const noexcept
      {
        AutoRecursiveLock lock(*this);

        for (auto iter = mTracks.begin(); iter != mTracks.end(); ++iter)
        {
          auto track = (*iter);
          if (!track->isEnded()) return true;
        }
        return false;
      }

      //-----------------------------------------------------------------------
      IMediaStreamTypes::MediaStreamTrackListPtr MediaStream::getAudioTracks() const noexcept
      {
        auto result = make_shared<MediaStreamTrackList>();

        AutoRecursiveLock lock(*this);

        for (auto iter = mTracks.begin(); iter != mTracks.end(); ++iter)
        {
          auto track = (*iter);
          auto kind = track->kind();

          switch (kind)
          {
            case IMediaStreamTrackTypes::Kind_Audio: break;
            case IMediaStreamTrackTypes::Kind_Video: continue;
          }
          result->push_back(MediaStreamTrack::convert(track));
        }
        return result;
      }

      //-----------------------------------------------------------------------
      IMediaStreamTypes::MediaStreamTrackListPtr MediaStream::getVideoTracks() const noexcept
      {
        auto result = make_shared<MediaStreamTrackList>();

        AutoRecursiveLock lock(*this);

        for (auto iter = mTracks.begin(); iter != mTracks.end(); ++iter)
        {
          auto track = (*iter);
          auto kind = track->kind();

          switch (kind)
          {
            case IMediaStreamTrackTypes::Kind_Audio: continue;
            case IMediaStreamTrackTypes::Kind_Video: break;
          }
          result->push_back(MediaStreamTrack::convert(track));
        }
        return result;
      }

      //-----------------------------------------------------------------------
      IMediaStreamTypes::MediaStreamTrackListPtr MediaStream::getTracks() const noexcept
      {
        auto result = make_shared<MediaStreamTrackList>();

        AutoRecursiveLock lock(*this);

        for (auto iter = mTracks.begin(); iter != mTracks.end(); ++iter)
        {
          auto track = (*iter);
          result->push_back(MediaStreamTrack::convert(track));
        }
        return result;
      }

      //-----------------------------------------------------------------------
      IMediaStreamTrackPtr MediaStream::getTrackByID(const char *id) const noexcept
      {
        String idStr(id);

        AutoRecursiveLock lock(*this);

        for (auto iter = mTracks.begin(); iter != mTracks.end(); ++iter)
        {
          auto track = (*iter);
          if (idStr == track->id()) return MediaStreamTrack::convert(track);
        }

        return IMediaStreamTrackPtr();
      }

      //-----------------------------------------------------------------------
      size_t MediaStream::size() const noexcept
      {
        AutoRecursiveLock lock(*this);
        return mTracks.size();
      }

      //-----------------------------------------------------------------------
      void MediaStream::addTrack(IMediaStreamTrackPtr track) noexcept
      {
        ZS_ASSERT(track);
        AutoRecursiveLock lock(*this);
        auto result = getTrackByID(track->id());
        if (nullptr != result) {
          ZS_EVENTING_TRACE_OBJECT_PTR(Detail, track, "already added track (not added again)");
          ZS_LOG_WARNING(Debug, log("already added track (not added again)"));
          return;
        }
        mTracks.push_back(MediaStreamTrack::convert(track));
      }

      //-----------------------------------------------------------------------
      void MediaStream::removeTrack(IMediaStreamTrackPtr track) noexcept
      {
        if (!track) return;

        AutoRecursiveLock lock(*this);
        for (auto iter_doNotUse = mTracks.begin(); iter_doNotUse != mTracks.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto compareTrack = (*current);

          if (compareTrack->id() != track->id()) continue;
          mTracks.erase(current);
          break;
        }
      }
      
      //-----------------------------------------------------------------------
      IMediaStreamPtr MediaStream::clone() const noexcept
      {
        auto tracks = getTracks();
        ZS_ASSERT(nullptr != tracks);
        return MediaStream::create(IMediaStreamDelegatePtr(), *tracks);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // MediaStream => ForPeerConnection
      //

      //-----------------------------------------------------------------------
      MediaStream::PromiseWithStatsReportPtr MediaStream::getStats(const StatsTypeSet &stats) const noexcept
      {
        UseStatsReport::PromiseWithStatsReportList promises;

        {
          AutoRecursiveLock lock(*this);
          for (auto iter = mTracks.begin(); iter != mTracks.end(); ++iter) {
            IMediaStreamTrackPtr track = MediaStreamTrack::convert(*iter);

            auto promise = track->getStats(stats);
            promises.push_back(promise);
          }
        }

        return UseStatsReport::collectReports(promises);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // MediaStream => ForPeerConnection
      //

      //-----------------------------------------------------------------------
      MediaStreamPtr MediaStream::create(const char *id) noexcept
      {
        MediaStreamPtr pThis(make_shared<MediaStream>(make_private{}, UseORTC::queueORTC(), IMediaStreamDelegatePtr(), UseMediaStreamTrackList(), id));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      void MediaStream::notifyAddTrack(IMediaStreamTrackPtr track) noexcept
      {
        AutoRecursiveLock lock(*this);
        auto totalTracks = mTracks.size();
        addTrack(track);
        if (totalTracks == mTracks.size()) return;
        auto pThis = mThisWeak.lock();
        mSubscriptions.delegate()->onMediaStreamAddTrack(pThis, track);
      }

      //-----------------------------------------------------------------------
      void MediaStream::notifyRemoveTrack(IMediaStreamTrackPtr track) noexcept
      {
        AutoRecursiveLock lock(*this);
        auto totalTracks = mTracks.size();
        removeTrack(track);
        if (totalTracks == mTracks.size()) return;
        auto pThis = mThisWeak.lock();
        mSubscriptions.delegate()->onMediaStreamRemoveTrack(pThis, track);
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // MediaStream => (internal)
      //

      //-----------------------------------------------------------------------
      Log::Params MediaStream::log(const char *message) const noexcept
      {
        ElementPtr objectEl = Element::create("ortc::adapter::MediaStream");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      Log::Params MediaStream::debug(const char *message) const noexcept
      {
        return Log::Params(message, toDebug());
      }

      //-----------------------------------------------------------------------
      ElementPtr MediaStream::toDebug() const noexcept
      {
        AutoRecursiveLock lock(*this);

        ElementPtr resultEl = Element::create("ortc::adapter::MediaStreamTrack");

        UseServicesHelper::debugAppend(resultEl, "id", mID);
        UseServicesHelper::debugAppend(resultEl, "msid", mID);

        UseServicesHelper::debugAppend(resultEl, "subscriptions", mSubscriptions.size());
        UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

        UseServicesHelper::debugAppend(resultEl, "tracks", mTracks.size());

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // IMediaStreamFactory
      //

      //-------------------------------------------------------------------------
      IMediaStreamFactory &IMediaStreamFactory::singleton() noexcept
      {
        return MediaStreamFactory::singleton();
      }

      //-------------------------------------------------------------------------
      MediaStreamPtr IMediaStreamFactory::create(
                                                 IMediaStreamDelegatePtr delegate,
                                                 IMediaStreamPtr stream
                                                 ) noexcept
      {
        if (this) {}
        return internal::MediaStream::create(delegate, stream);
      }

      //-------------------------------------------------------------------------
      MediaStreamPtr IMediaStreamFactory::create(
                                                 IMediaStreamDelegatePtr delegate,
                                                 const MediaStreamTrackList &tracks
                                                 ) noexcept
      {
        if (this) {}
        return internal::MediaStream::create(delegate, tracks);
      }

      //-------------------------------------------------------------------------
      MediaStreamPtr IMediaStreamFactory::create(const char *id) noexcept
      {
        if (this) {}
        return internal::MediaStream::create(id);
      }

    }  // namespace internal

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaStream
    //

    //-------------------------------------------------------------------------
    ElementPtr IMediaStream::toDebug(IMediaStreamPtr stream) noexcept
    {
      return internal::MediaStream::toDebug(stream);
    }

    //-------------------------------------------------------------------------
    IMediaStreamPtr IMediaStream::create(
                                         IMediaStreamDelegatePtr delegate,
                                         IMediaStreamPtr stream
                                         ) noexcept
    {
      return internal::IMediaStreamFactory::singleton().create(delegate, stream);
    }

    //-------------------------------------------------------------------------
    IMediaStreamPtr IMediaStream::create(
                                         IMediaStreamDelegatePtr delegate,
                                         const MediaStreamTrackList &tracks
                                         ) noexcept
    {
      return internal::IMediaStreamFactory::singleton().create(delegate, tracks);
    }

  } // namespace adapter
} // namespace ortc
