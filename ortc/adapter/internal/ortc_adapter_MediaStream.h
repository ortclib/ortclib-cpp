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

#pragma once


#include <ortc/adapter/internal/types.h>
#include <ortc/adapter/IMediaStream.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForMediaStream);
  } // namespace internal

  namespace adapter
  {
    namespace internal
    {
      ZS_DECLARE_INTERACTION_PTR(IMediaStreamForPeerConnection);

      interaction IMediaStreamForPeerConnection
      {
        ZS_DECLARE_TYPEDEF_PTR(IMediaStreamForPeerConnection, ForPeerConnection);

        static ForPeerConnectionPtr create(const char *id) noexcept;

        virtual String id() const noexcept = 0;

        virtual size_t size() const noexcept = 0;

        virtual void notifyAddTrack(IMediaStreamTrackPtr track) noexcept = 0;
        virtual void notifyRemoveTrack(IMediaStreamTrackPtr track) noexcept = 0;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // MediaStream
      //

      class MediaStream : public Noop,
                          public MessageQueueAssociator,
                          public SharedRecursiveLock,
                          public IMediaStream,
                          public IMediaStreamForPeerConnection
      {
      protected:
        struct make_private {};

        friend interaction IMediaStreamFactory;
        friend interaction IMediaStream;
        friend interaction IMediaStreamForPeerConnection;

        ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IMediaStreamTrackForMediaStream, UseMediaStreamTrack);
        ZS_DECLARE_TYPEDEF_PTR(std::list<UseMediaStreamTrackPtr>, UseMediaStreamTrackList);

      public:
        MediaStream(
                    const make_private &,
                    IMessageQueuePtr queue,
                    IMediaStreamDelegatePtr delegate,
                    const UseMediaStreamTrackList &tracks,
                    const char *id = NULL
                    ) noexcept;

        ~MediaStream() noexcept;

        static MediaStreamPtr convert(IMediaStreamPtr object) noexcept;
        static MediaStreamPtr convert(ForPeerConnectionPtr object) noexcept;

      protected:
        MediaStream(Noop) noexcept :
          Noop(true),
          MessageQueueAssociator(IMessageQueuePtr()),
          SharedRecursiveLock(SharedRecursiveLock::create())
        {}

        void init() noexcept;

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // MediaStream => IMediaStream
        //

        static ElementPtr toDebug(IMediaStreamPtr object) noexcept;

        static MediaStreamPtr create(
                                     IMediaStreamDelegatePtr delegate,
                                     IMediaStreamPtr stream
                                     ) noexcept;
        static MediaStreamPtr create(
                                     IMediaStreamDelegatePtr delegate,
                                     const MediaStreamTrackList &tracks
                                     ) noexcept;

        PUID getID() const noexcept override { return mID; }

        IMediaStreamSubscriptionPtr subscribe(IMediaStreamDelegatePtr delegate) noexcept override;

        String id() const noexcept override;
        bool active() const noexcept override;

        MediaStreamTrackListPtr getAudioTracks() const noexcept override;
        MediaStreamTrackListPtr getVideoTracks() const noexcept override;
        MediaStreamTrackListPtr getTracks() const noexcept override;
        IMediaStreamTrackPtr getTrackByID(const char *id) const noexcept override;
        size_t size() const noexcept override;

        void addTrack(IMediaStreamTrackPtr track) noexcept override;
        void removeTrack(IMediaStreamTrackPtr track) noexcept override;

        IMediaStreamPtr clone() const noexcept override;

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // MediaStream => IStatsProvider
        //

        virtual PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const noexcept override;

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //
        // MediaStream => ForPeerConnection
        //

        static MediaStreamPtr create(const char *id) noexcept;

        // (duplicate) virtual String id() const noexcept= 0;

        // (duplicate) virtual size_t size() const noexcept= 0;

        virtual void notifyAddTrack(IMediaStreamTrackPtr track) noexcept override;
        virtual void notifyRemoveTrack(IMediaStreamTrackPtr track) noexcept override;

      protected:
        //---------------------------------------------------------------------
        //
        // MediaStream => (internal)
        //

        Log::Params log(const char *message) const noexcept;
        Log::Params debug(const char *message) const noexcept;
        virtual ElementPtr toDebug() const noexcept;

        bool isShutdown() const noexcept { return false; }

      protected:
        //---------------------------------------------------------------------
        //
        // MediaStream => (data)
        //

        AutoPUID mID;
        String mMSID;
        MediaStreamWeakPtr mThisWeak;
        //MediaStreamPtr mGracefulShutdownReference; // this is intentionally commented because graceful shutdown is not needed (yet, maybe never)

        IMediaStreamDelegateSubscriptions mSubscriptions;
        IMediaStreamSubscriptionPtr mDefaultSubscription;

        UseMediaStreamTrackList mTracks;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // IMediaStreamFactory
      //

      interaction IMediaStreamFactory
      {
        static IMediaStreamFactory &singleton() noexcept;
        typedef IMediaStreamTypes::MediaStreamTrackList MediaStreamTrackList;

        virtual MediaStreamPtr create(
                                      IMediaStreamDelegatePtr delegate,
                                      IMediaStreamPtr stream
                                      ) noexcept;
        virtual MediaStreamPtr create(
                                      IMediaStreamDelegatePtr delegate,
                                      const MediaStreamTrackList &tracks
                                      ) noexcept;
        virtual MediaStreamPtr create(const char *id) noexcept;
      };

      class MediaStreamFactory : public IFactory<IMediaStreamFactory> {};

    } // namespace internal
  } // namespace adapter
} // namespace ortc
