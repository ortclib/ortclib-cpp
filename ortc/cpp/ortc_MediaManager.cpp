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

#include <ortc/internal/ortc_MediaManager.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_MediaStream.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/Log.h>
#include <zsLib/XML.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)

  namespace internal
  {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaManager
    #pragma mark
    
    //-----------------------------------------------------------------------
    MediaManager::MediaManager(
                               IMessageQueuePtr queue,
                               IMediaManagerDelegatePtr originalDelegate
                               ) :
      MessageQueueAssociator(queue)
    {
      mDefaultSubscription = mSubscriptions.subscribe(IMediaManagerDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate), queue);
    }
    
    //-----------------------------------------------------------------------
    MediaManagerPtr MediaManager::singleton(IMediaManagerDelegatePtr delegate)
    {
      static MediaManagerPtr manager = MediaManager::create(IORTCForInternal::queueORTC(), delegate);
      return manager;
    }
    
    //-----------------------------------------------------------------------
    void MediaManager::setup(IMediaManagerDelegatePtr delegate)
    {
      singleton(delegate);
    }

    //-------------------------------------------------------------------------
    MediaManagerPtr MediaManager::create(IMessageQueuePtr queue, IMediaManagerDelegatePtr delegate)
    {
      MediaManagerPtr pThis(new MediaManager(queue, delegate));
      pThis->mThisWeak = pThis;
      return pThis;
    }
    
    //-----------------------------------------------------------------------
    MediaManager::~MediaManager()
    {
      if (isNoop()) return;
      
      mThisWeak.reset();
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaManager => IMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    PUID MediaManager::getID() const
    {
      return mID;
    }
    
    //-------------------------------------------------------------------------
    IMediaManagerSubscriptionPtr MediaManager::subscribe(IMediaManagerDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to media manager events"))
      
      AutoRecursiveLock lock(getLock());
      if (!originalDelegate) return mDefaultSubscription;
      
      IMediaManagerSubscriptionPtr subscription = mSubscriptions.subscribe(IMediaManagerDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate));
      
      IMediaManagerDelegatePtr delegate = mSubscriptions.delegate(subscription, true);
      
      if (delegate) {
        MediaManagerPtr pThis = mThisWeak.lock();
      }
      
      return subscription;
    }

    //-----------------------------------------------------------------------
    void MediaManager::getUserMedia(MediaStreamConstraints constraints)
    {
      MediaStreamPtr mediaStream = IMediaStreamForMediaManager::create(getAssociatedMessageQueue(), IMediaStreamDelegatePtr());
      LocalAudioStreamTrackPtr localAudioStreamTrack = ILocalAudioStreamTrackForMediaManager::create(getAssociatedMessageQueue(), IMediaStreamTrackDelegatePtr());
      LocalVideoStreamTrackPtr localVideoStreamTrack = ILocalVideoStreamTrackForMediaManager::create(getAssociatedMessageQueue(), IMediaStreamTrackDelegatePtr());

      mediaStream->forMediaManager().addTrack(localAudioStreamTrack);
      mediaStream->forMediaManager().addTrack(localVideoStreamTrack);
      
      try {
        mSubscriptions.delegate()->onMediaManagerSuccessCallback(mediaStream);
      } catch (IMediaStreamDelegateProxy::Exceptions::DelegateGone &) {
      }
    }
    
    //-------------------------------------------------------------------------
    void MediaManager::setDefaultVideoOrientation(VideoOrientations orientation)
    {
      AutoRecursiveLock lock(mLock);

    }
    
    //-------------------------------------------------------------------------
    MediaManager::VideoOrientations MediaManager::getDefaultVideoOrientation()
    {
      AutoRecursiveLock lock(mLock);
      
      return VideoOrientation_LandscapeLeft;
    }
    
    //-------------------------------------------------------------------------
    void MediaManager::setRecordVideoOrientation(VideoOrientations orientation)
    {
      AutoRecursiveLock lock(mLock);
    }
    
    //-------------------------------------------------------------------------
    MediaManager::VideoOrientations MediaManager::getRecordVideoOrientation()
    {
      AutoRecursiveLock lock(mLock);
      
      return VideoOrientation_LandscapeLeft;
    }
    
    //-----------------------------------------------------------------------
    void MediaManager::setVideoOrientation()
    {
      AutoRecursiveLock lock(mLock);
    }
    
    //-----------------------------------------------------------------------
    void MediaManager::setMuteEnabled(bool enabled)
    {
      AutoRecursiveLock lock(mLock);
    }
    
    //-----------------------------------------------------------------------
    bool MediaManager::getMuteEnabled()
    {
      AutoRecursiveLock lock(mLock);
      
      bool enabled = false;

      return enabled;
    }
    
    //-----------------------------------------------------------------------
    void MediaManager::setLoudspeakerEnabled(bool enabled)
    {
      AutoRecursiveLock lock(mLock);
    }
    
    //-----------------------------------------------------------------------
    bool MediaManager::getLoudspeakerEnabled()
    {
      AutoRecursiveLock lock(mLock);
      
      bool enabled = false;

      return enabled;
    }
    
    //-----------------------------------------------------------------------
    IMediaManager::OutputAudioRoutes MediaManager::getOutputAudioRoute()
    {
      AutoRecursiveLock lock(mLock);
      
      return OutputAudioRoute_Headphone;
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaManager => IWakeDelegate
    #pragma mark
    
    //-------------------------------------------------------------------------
    void MediaManager::onWake()
    {
      ZS_LOG_DEBUG(log("wake"))
      
      AutoRecursiveLock lock(getLock());
      step();
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaManager => (internal)
    #pragma mark
    
    //-------------------------------------------------------------------------
    Log::Params MediaManager::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::MediaManager");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }
    
    //-------------------------------------------------------------------------
    Log::Params MediaManager::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }
    
    //-------------------------------------------------------------------------
    ElementPtr MediaManager::toDebug() const
    {
      ElementPtr resultEl = Element::create("MediaManager");
      
      UseServicesHelper::debugAppend(resultEl, "id", mID);
      
      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);
      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", mShutdown);
      
      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      return resultEl;
    }
    
    //-------------------------------------------------------------------------
    bool MediaManager::isShuttingDown() const
    {
      return (bool)mGracefulShutdownReference;
    }
    
    //-------------------------------------------------------------------------
    bool MediaManager::isShutdown() const
    {
      if (mGracefulShutdownReference) return false;
      return mShutdown;
    }
    
    //-------------------------------------------------------------------------
    void MediaManager::step()
    {
      ZS_LOG_DEBUG(debug("step"))
      
      AutoRecursiveLock lock(getLock());
      
      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }
      
    }
    
    //-------------------------------------------------------------------------
    void MediaManager::cancel()
    {
      //.......................................................................
      // start the shutdown process
      
      //.......................................................................
      // try to gracefully shutdown
      
      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();
      
      if (mGracefulShutdownReference) {
      }
      
      //.......................................................................
      // final cleanup
      
      get(mShutdown) = true;
      
      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-----------------------------------------------------------------------
    void MediaManager::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);

      if (0 != mLastError) {
        ZS_LOG_WARNING(Detail, debug("error already set thus ignoring new error") + ZS_PARAM("new error", errorCode) + ZS_PARAM("new reason", reason))
        return;
      }
      
      get(mLastError) = errorCode;
      mLastErrorReason = reason;
      
      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaManagerFactory
    #pragma mark
    
    //-------------------------------------------------------------------------
    IMediaManagerFactory &IMediaManagerFactory::singleton()
    {
      return MediaManagerFactory::singleton();
    }
    
    //-------------------------------------------------------------------------
    MediaManagerPtr IMediaManagerFactory::create(IMediaManagerDelegatePtr delegate)
    {
      if (this) {}
      return MediaManager::create(IMessageQueuePtr(), delegate);
    }

  }
  
  //-----------------------------------------------------------------------
  //-----------------------------------------------------------------------
  //-----------------------------------------------------------------------
  //-----------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaManager
  #pragma mark
  
  //-----------------------------------------------------------------------
  IMediaManagerPtr IMediaManager::singleton()
  {
    return internal::MediaManager::singleton();
  }
  
  //-----------------------------------------------------------------------
  void IMediaManager::setup(IMediaManagerDelegatePtr delegate)
  {
    internal::MediaManager::setup(delegate);
  }
}
