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
#include <ortc/IMediaStream.h>

#include <openpeer/services/IWakeDelegate.h>

namespace ortc
{
  namespace internal
  {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamForMediaManager
    #pragma mark
    
    interaction IMediaStreamForMediaManager
    {
      IMediaStreamForMediaManager &forMediaManager() {return *this;}
      const IMediaStreamForMediaManager &forMediaManager() const {return *this;}
      
      static MediaStreamPtr create(
                                   IMessageQueuePtr queue,
                                   IMediaStreamDelegatePtr delegate
                                   );
      
      virtual String id() = 0;
      virtual MediaStreamTrackListPtr getAudioTracks() = 0;
      virtual MediaStreamTrackListPtr getVideoTracks() = 0;
      virtual IMediaStreamTrackPtr getTrackById(String trackId) = 0;
      virtual void addTrack(IMediaStreamTrackPtr track) = 0;
      virtual void removeTrack(IMediaStreamTrackPtr track) = 0;

      virtual String getCNAME() = 0;
      virtual int getAudioChannel() = 0;
      virtual void setAudioChannel(int channel) = 0;
      virtual int getVideoChannel() = 0;
      virtual void setVideoChannel(int channel) = 0;

      virtual void setVoiceRecordFile(String fileName) = 0;
      virtual String getVoiceRecordFile() const = 0;
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamForRTCConnection
    #pragma mark
    
    interaction IMediaStreamForRTCConnection
    {
      IMediaStreamForRTCConnection &forRTCConnection() {return *this;}
      const IMediaStreamForRTCConnection &forRTCConnection() const {return *this;}
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
                        public IMediaStream,
                        public IMediaStreamForMediaManager,
                        public IMediaStreamForRTCConnection,
                        public IWakeDelegate
    {
    public:
      friend interaction IMediaStream;
      friend interaction IMediaStreamFactory;
      friend interaction IMediaStreamForMediaManager;
      friend interaction IMediaStreamForRTCConnection;

    protected:
      MediaStream(
                  IMessageQueuePtr queue,
                  IMediaStreamDelegatePtr delegate
                  );
      
    public:
      virtual ~MediaStream();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStream => IMediaStream
      #pragma mark
      
    protected:
      virtual String id();
      virtual MediaStreamTrackListPtr getAudioTracks();
      virtual MediaStreamTrackListPtr getVideoTracks();
      virtual IMediaStreamTrackPtr getTrackById(String trackId);
      virtual void addTrack(IMediaStreamTrackPtr track);
      virtual void removeTrack(IMediaStreamTrackPtr track);
      virtual IMediaStreamPtr clone();
      virtual bool inactive();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStream => IMediaStreamForMediaManager
      #pragma mark
      
    protected:
      virtual String getCNAME();
      virtual int getAudioChannel();
      virtual void setAudioChannel(int channel);
      virtual int getVideoChannel();
      virtual void setVideoChannel(int channel);

      virtual void setVoiceRecordFile(String fileName);
      virtual String getVoiceRecordFile() const;

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStream => IMediaStreamForRTCConnection
      #pragma mark
      
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStream => IWakeDelegate
      #pragma mark
      
      virtual void onWake();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStream => (internal)
      #pragma mark
      
      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;
      
      RecursiveLock &getLock() const {return mLock;}
      
      bool isShuttingDown() const;
      bool isShutdown() const;
      
      void step();
      
      void cancel();
      
      void setError(WORD error, const char *reason = NULL);

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStream => (data)
      #pragma mark
      
    protected:
      PUID mID;
      mutable RecursiveLock mLock;
      MediaStreamWeakPtr mThisWeak;
      MediaStreamPtr mGracefulShutdownReference;
      bool mShutdown {};
      
      IMediaStreamDelegateSubscriptions mSubscriptions;
      IMediaStreamSubscriptionPtr mDefaultSubscription;
      
      WORD mLastError {};
      String mLastErrorReason;
      
      String mStreamID;
      MediaStreamTrackListPtr mAudioTracks;
      MediaStreamTrackListPtr mVideoTracks;
      bool mInactive;
      
      String mCNAME;
      
      int mAudioChannel;
      int mVideoChannel;
      String mVoiceRecordFile;
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamFactory
    #pragma mark
    
    interaction IMediaStreamFactory
    {
      static IMediaStreamFactory &singleton();
      
      virtual MediaStreamPtr create(IMediaStreamDelegatePtr delegate);
    };

    class MediaStreamFactory : public IFactory<IMediaStreamFactory> {};
  }
}
