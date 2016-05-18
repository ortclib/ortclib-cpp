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

        static ForPeerConnectionPtr create(const char *id);

        virtual String id() const = 0;

        virtual size_t size() const = 0;

        virtual void notifyAddTrack(IMediaStreamTrackPtr track) = 0;
        virtual void notifyRemoveTrack(IMediaStreamTrackPtr track) = 0;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStream
      #pragma mark

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
                    );

        static MediaStreamPtr convert(IMediaStreamPtr object);
        static MediaStreamPtr convert(ForPeerConnectionPtr object);

      protected:
        MediaStream(Noop) :
          Noop(true),
          MessageQueueAssociator(IMessageQueuePtr()),
          SharedRecursiveLock(SharedRecursiveLock::create())
        {}

        void init();

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStream => IMediaStream
        #pragma mark

        static ElementPtr toDebug(IMediaStreamPtr object);

        static MediaStreamPtr create(
                                     IMediaStreamDelegatePtr delegate,
                                     IMediaStreamPtr stream
                                     );
        static MediaStreamPtr create(
                                     IMediaStreamDelegatePtr delegate,
                                     const MediaStreamTrackList &tracks
                                     );

        virtual PUID getID() const override { return mID; }

        virtual IMediaStreamSubscriptionPtr subscribe(IMediaStreamDelegatePtr delegate) override;

        virtual String id() const override;
        virtual bool active() const override;

        virtual MediaStreamTrackListPtr getAudioTracks() const override;
        virtual MediaStreamTrackListPtr getVideoTracks() const override;
        virtual MediaStreamTrackListPtr getTracks() const override;
        virtual IMediaStreamTrackPtr getTrackByID(const char *id) const override;
        virtual size_t size() const override;

        virtual void addTrack(IMediaStreamTrackPtr track) override;
        virtual void removeTrack(IMediaStreamTrackPtr track) override;

        virtual IMediaStreamPtr clone() const override;

        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStream => ForPeerConnection
        #pragma mark

        static MediaStreamPtr create(const char *id);

        // (duplicate) virtual String id() const = 0;

        // (duplicate) virtual size_t size() const = 0;

        virtual void notifyAddTrack(IMediaStreamTrackPtr track) override;
        virtual void notifyRemoveTrack(IMediaStreamTrackPtr track) override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStream => (internal)
        #pragma mark

        Log::Params log(const char *message) const;
        Log::Params debug(const char *message) const;
        virtual ElementPtr toDebug() const;

        bool isShutdown() const { return false; }

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark MediaStream => (data)
        #pragma mark

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
      #pragma mark
      #pragma mark IMediaStreamFactory
      #pragma mark

      interaction IMediaStreamFactory
      {
        static IMediaStreamFactory &singleton();
        typedef IMediaStreamTypes::MediaStreamTrackList MediaStreamTrackList;

        virtual MediaStreamPtr create(
                                      IMediaStreamDelegatePtr delegate,
                                      IMediaStreamPtr stream
                                      );
        virtual MediaStreamPtr create(
                                      IMediaStreamDelegatePtr delegate,
                                      const MediaStreamTrackList &tracks
                                      );
        virtual MediaStreamPtr create(const char *id);
      };

      class MediaStreamFactory : public IFactory<IMediaStreamFactory> {};

    } // namespace internal
  } // namespace adapter
} // namespace ortc
