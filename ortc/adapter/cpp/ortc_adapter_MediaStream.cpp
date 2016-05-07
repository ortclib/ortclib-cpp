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

#include <ortc/adapter/internal/ortc_adapter_MediaStream.h>

#include <ortc/internal/ortc_MediaStreamTrack.h>

#include <ortc/internal/ortc_ORTC.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/Log.h>
#include <zsLib/XML.h>

namespace ortc { namespace adapter { ZS_DECLARE_SUBSYSTEM(ortclib_adapter); } }

namespace ortc
{
  namespace adapter
  {
    namespace internal
    {
      ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper);
      ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IORTCForInternal, UseORTC);

      ZS_DECLARE_USING_PTR(ortc::internal, MediaStreamTrack);

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IMediaStreamForPeerConnection
      #pragma mark


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStream
      #pragma mark

      //-------------------------------------------------------------------------
      MediaStream::MediaStream(
                               const make_private &,
                               IMessageQueuePtr queue,
                               IMediaStreamDelegatePtr delegate,
                               const UseMediaStreamTrackList &tracks
                               ) :
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create()),
        mMSID(UseServicesHelper::randomString(36)),
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
      }

      //-------------------------------------------------------------------------
      void MediaStream::init()
      {
      }

      //-------------------------------------------------------------------------
      MediaStreamPtr MediaStream::convert(IMediaStreamPtr object)
      {
        return ZS_DYNAMIC_PTR_CAST(MediaStream, object);
      }

      //-------------------------------------------------------------------------
      MediaStreamPtr MediaStream::convert(ForPeerConnectionPtr object)
      {
        return ZS_DYNAMIC_PTR_CAST(MediaStream, object);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStream => IMediaStream
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr MediaStream::toDebug(IMediaStreamPtr object)
      {
        if (!object) return ElementPtr();
        return MediaStream::convert(object)->toDebug();
      }

      //-----------------------------------------------------------------------
      MediaStreamPtr MediaStream::create(
                                         IMediaStreamDelegatePtr delegate,
                                         IMediaStreamPtr stream
                                         )
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
                                         )
      {
        UseMediaStreamTrackList tempTracks;
        for (auto iter = tracks.begin(); iter != tracks.end(); ++iter)
        {
          auto track = (*iter);
          ORTC_THROW_INVALID_PARAMETERS_IF(nullptr == track);
          tempTracks.push_back(MediaStreamTrack::convert(track));
        }

        MediaStreamPtr pThis(make_shared<MediaStream>(make_private{}, UseORTC::queueORTC(), delegate, tempTracks));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      IMediaStreamSubscriptionPtr MediaStream::subscribe(IMediaStreamDelegatePtr originalDelegate)
      {
        ZS_LOG_DETAIL(log("subscribing to media stream"));

        AutoRecursiveLock lock(*this);
        if (!originalDelegate) return mDefaultSubscription;

        auto subscription = mSubscriptions.subscribe(originalDelegate, UseORTC::queueDelegate());

        auto delegate = mSubscriptions.delegate(subscription, true);

        if (delegate) {
          auto pThis = mThisWeak.lock();

#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 1
#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 2
        }

        if (isShutdown()) {
          mSubscriptions.clear();
        }

        return subscription;
      }

      //-----------------------------------------------------------------------
      String MediaStream::id() const
      {
        return mMSID;
      }

      //-----------------------------------------------------------------------
      bool MediaStream::active() const
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
      IMediaStreamTypes::MediaStreamTrackListPtr MediaStream::getAudioTracks() const
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
      IMediaStreamTypes::MediaStreamTrackListPtr MediaStream::getVideoTracks() const
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
      IMediaStreamTypes::MediaStreamTrackListPtr MediaStream::getTracks() const
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
      IMediaStreamTrackPtr MediaStream::getTrackByID(const char *id) const
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
      void MediaStream::addTrack(IMediaStreamTrackPtr track)
      {
        ORTC_THROW_INVALID_PARAMETERS_IF(!track);
        AutoRecursiveLock lock(*this);
        auto result = getTrackByID(track->id());
        if (nullptr != result) {
          ZS_LOG_WARNING(Debug, log("already added track (not added again)") + IMediaStreamTrack::toDebug(track));
          return;
        }
        mTracks.push_back(MediaStreamTrack::convert(track));
      }

      //-----------------------------------------------------------------------
      void MediaStream::removeTrack(IMediaStreamTrackPtr track)
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
      IMediaStreamPtr MediaStream::clone() const
      {
        auto tracks = getTracks();
        ZS_THROW_INVALID_ASSUMPTION_IF(nullptr == tracks);
        return MediaStream::create(IMediaStreamDelegatePtr(), *tracks);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStream => ForPeerConnection
      #pragma mark

      //-----------------------------------------------------------------------
      void MediaStream::notifyAddTrack(IMediaStreamTrackPtr track)
      {
        AutoRecursiveLock lock(*this);
        auto totalTracks = mTracks.size();
        addTrack(track);
        if (totalTracks == mTracks.size()) return;
        auto pThis = mThisWeak.lock();
        mSubscriptions.delegate()->onMediaStreamAddTrack(pThis, track);
      }

      //-----------------------------------------------------------------------
      void MediaStream::notifyRemoveTrack(IMediaStreamTrackPtr track)
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
      #pragma mark
      #pragma mark MediaStream => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params MediaStream::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::adapter::MediaStream");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      Log::Params MediaStream::debug(const char *message) const
      {
        return Log::Params(message, toDebug());
      }

      //-----------------------------------------------------------------------
      ElementPtr MediaStream::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr resultEl = Element::create("ortc::MediaStreamTrack");

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
      #pragma mark
      #pragma mark IMediaStreamFactory
      #pragma mark

      //-------------------------------------------------------------------------
      IMediaStreamFactory &IMediaStreamFactory::singleton()
      {
        return IMediaStreamFactory::singleton();
      }

      //-------------------------------------------------------------------------
      MediaStreamPtr IMediaStreamFactory::create(
                                                 IMediaStreamDelegatePtr delegate,
                                                 IMediaStreamPtr stream
                                                 )
      {
        if (this) {}
        return internal::MediaStream::create(delegate, stream);
      }

      //-------------------------------------------------------------------------
      MediaStreamPtr IMediaStreamFactory::create(
                                                 IMediaStreamDelegatePtr delegate,
                                                 const MediaStreamTrackList &tracks
                                                 )
      {
        if (this) {}
        return internal::MediaStream::create(delegate, tracks);
      }

    }  // namespace internal

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStream
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IMediaStream::toDebug(IMediaStreamPtr stream)
    {
      return internal::MediaStream::toDebug(stream);
    }

    //-------------------------------------------------------------------------
    IMediaStreamPtr IMediaStream::create(
                                         IMediaStreamDelegatePtr delegate,
                                         IMediaStreamPtr stream
                                         )
    {
      return internal::IMediaStreamFactory::singleton().create(delegate, stream);
    }

    //-------------------------------------------------------------------------
    IMediaStreamPtr IMediaStream::create(
                                         IMediaStreamDelegatePtr delegate,
                                         const MediaStreamTrackList &tracks
                                         )
    {
      return internal::IMediaStreamFactory::singleton().create(delegate, tracks);
    }

  } // namespace adapter
} // namespace ortc
