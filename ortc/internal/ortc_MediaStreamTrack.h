/*
 
 Copyright (c) 2013, SMB Phone Inc. / Hookflash Inc.
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
#include <ortc/IMediaStreamTrack.h>

namespace ortc
{
  namespace internal
  {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForMediaManager
    #pragma mark
    
    interaction IMediaStreamTrackForMediaManager
    {
      IMediaStreamTrackForMediaManager &forMediaManager() {return *this;}
      const IMediaStreamTrackForMediaManager &forMediaManager() const {return *this;}
      
      static MediaStreamTrackPtr create(
                                        IMessageQueuePtr queue,
                                        IMediaStreamTrackDelegatePtr delegate
                                        );
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTCConnection
    #pragma mark
    
    interaction IMediaStreamTrackForRTCConnection
    {
      IMediaStreamTrackForRTCConnection &forRTCConnection() {return *this;}
      const IMediaStreamTrackForRTCConnection &forRTCConnection() const {return *this;}
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack
    #pragma mark
    
    class MediaStreamTrack : public Noop,
      public MessageQueueAssociator,
      public IMediaStreamTrack,
      public IMediaStreamTrackForMediaManager,
      public IMediaStreamTrackForRTCConnection
    {
    public:
      friend interaction IMediaStreamTrack;
      friend interaction IMediaStreamTrackForMediaManager;
      friend interaction IMediaStreamTrackForRTCConnection;
      
    protected:
      MediaStreamTrack(
                  IMessageQueuePtr queue,
                  IMediaStreamTrackDelegatePtr delegate
                  );
      
    public:
      virtual ~MediaStreamTrack();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrack
      #pragma mark
      
    protected:
      virtual String kind();
      virtual String id();
      virtual String label();
      virtual bool enabled();
      virtual bool muted();
      virtual bool readonly();
      virtual bool remote();
      virtual MediaStreamTrackStates readyState();
      virtual IMediaStreamTrackPtr clone();
      virtual void stop();

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForMediaManager
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTCConnection
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => (data)
      #pragma mark
      
    protected:
      PUID mID;
      mutable RecursiveLock mLock;
      MediaStreamTrackWeakPtr mThisWeak;
      IMediaStreamTrackDelegatePtr mDelegate;
      
      int mError;
      
      String mKind;
      String mTrackID;
      String mLabel;
      bool mEnabled;
      bool mMuted;
      bool mReadonly;
      bool mRemote;
      MediaStreamTrackStates mReadyState;
    };
  }
}
