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
#include <ortc/IMediaManager.h>

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
    #pragma mark MediaManager
    #pragma mark
    
    class MediaManager : public Noop,
                         public MessageQueueAssociator,
                         public IMediaManager,
                         public IWakeDelegate
    {
    public:
      friend interaction IMediaManager;
      friend interaction IMediaManagerFactory;

    protected:
      MediaManager(
                   IMessageQueuePtr queue,
                   IMediaManagerDelegatePtr delegate
                   );
      
      static MediaManagerPtr singleton(IMediaManagerDelegatePtr delegate = IMediaManagerDelegatePtr());
      
      static void setup(IMediaManagerDelegatePtr delegate);

      static MediaManagerPtr create(IMessageQueuePtr queue, IMediaManagerDelegatePtr delegate);
      
    public:
      virtual ~MediaManager();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaManager => IMediaManager
      #pragma mark
      
      virtual PUID getID() const;

      virtual IMediaManagerSubscriptionPtr subscribe(IMediaManagerDelegatePtr delegate);

      virtual void getUserMedia(MediaStreamConstraints constraints);
      
      virtual void setDefaultVideoOrientation(VideoOrientations orientation);
      virtual VideoOrientations getDefaultVideoOrientation();
      virtual void setRecordVideoOrientation(VideoOrientations orientation);
      virtual VideoOrientations getRecordVideoOrientation();
      virtual void setVideoOrientation();
      
      virtual void setMuteEnabled(bool enabled);
      virtual bool getMuteEnabled();
      virtual void setLoudspeakerEnabled(bool enabled);
      virtual bool getLoudspeakerEnabled();
      virtual OutputAudioRoutes getOutputAudioRoute();
      
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaManager => IWakeDelegate
      #pragma mark
      
      virtual void onWake();

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaManager => (internal)
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
      #pragma mark MediaManager => (data)
      #pragma mark
      
    protected:
      AutoPUID mID;
      mutable RecursiveLock mLock;
      MediaManagerWeakPtr mThisWeak;
      MediaManagerPtr mGracefulShutdownReference;
      bool mShutdown {};

      IMediaManagerDelegateSubscriptions mSubscriptions;
      IMediaManagerSubscriptionPtr mDefaultSubscription;

      WORD mLastError {};
      String mLastErrorReason;
      
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaManagerFactory
    #pragma mark
    
    interaction IMediaManagerFactory
    {
      static IMediaManagerFactory &singleton();
      
      virtual MediaManagerPtr create(IMediaManagerDelegatePtr delegate);
    };

    class MediaManagerFactory : public IFactory<IMediaManagerFactory> {};
  }
}
