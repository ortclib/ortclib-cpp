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

#include <ortc/internal/ortc_MediaEngine.h>
#include <ortc/internal/ortc_ORTC.h>

#include <zsLib/helpers.h>
#include <zsLib/XML.h>

#include <openpeer/services/IHelper.h>

#include <boost/thread.hpp>

#include <video_capture_factory.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#ifdef TARGET_OS_IPHONE
#include <sys/sysctl.h>
#endif

#define ORTC_MEDIA_ENGINE_VOICE_CODEC_ISAC
//#define ORTC_MEDIA_ENGINE_VOICE_CODEC_OPUS
#define ORTC_MEDIA_ENGINE_INVALID_CHANNEL (-1)
#define ORTC_MEDIA_ENGINE_MTU (576)

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_webrtc) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)

  namespace internal
  {
    typedef zsLib::ThreadPtr ThreadPtr;

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngine
    #pragma mark
    
    //-------------------------------------------------------------------------
    IMediaEnginePtr IMediaEngine::singleton()
    {
      return MediaEngine::singleton();
    }
    
    //-----------------------------------------------------------------------
    void IMediaEngine::setup(IMediaEngineDelegatePtr delegate)
    {
      MediaEngine::setup(delegate);
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine
    #pragma mark
    
    //-----------------------------------------------------------------------
    MediaEngine::MediaEngine(
                             IMessageQueuePtr queue,
                             IMediaEngineDelegatePtr delegate
                             ) :
      MessageQueueAssociator(queue),
      mMtu(ORTC_MEDIA_ENGINE_MTU),
      mID(zsLib::createPUID()),
      mVoiceRecordFile(""),
      mDefaultVideoOrientation(webrtc::CapturedFrameOrientation_LandscapeLeft),
      mRecordVideoOrientation(webrtc::CapturedFrameOrientation_LandscapeLeft),
      mVoiceEngine(NULL),
      mVoiceBase(NULL),
      mVoiceCodec(NULL),
      mVoiceNetwork(NULL),
      mVoiceRtpRtcp(NULL),
      mVoiceAudioProcessing(NULL),
      mVoiceVolumeControl(NULL),
      mVoiceHardware(NULL),
      mVoiceFile(NULL),
      mVoiceEngineReady(false),
      mVcpm(NULL),
      mVideoEngine(NULL),
      mVideoBase(NULL),
      mVideoNetwork(NULL),
      mVideoRender(NULL),
      mVideoCapture(NULL),
      mVideoRtpRtcp(NULL),
      mVideoCodec(NULL),
      mVideoEngineReady(false),
      mLifetimeInProgress(false)
    {
#ifdef TARGET_OS_IPHONE
      int name[] = {CTL_HW, HW_MACHINE};
      size_t size;
      sysctl(name, 2, NULL, &size, NULL, 0);
      char *machine = (char *)malloc(size);
      sysctl(name, 2, machine, &size, NULL, 0);
      mMachineName = machine;
      free(machine);
#endif
    }
    
    MediaEngine::MediaEngine(
                             Noop,
                             IMessageQueuePtr queue,
                             internal::IMediaEngineDelegatePtr delegate
                             ) :
      Noop(true),
      MessageQueueAssociator(queue),
      mMtu(ORTC_MEDIA_ENGINE_MTU),
      mID(zsLib::createPUID()),
      mVoiceRecordFile(""),
      mDefaultVideoOrientation(webrtc::CapturedFrameOrientation_LandscapeLeft),
      mRecordVideoOrientation(webrtc::CapturedFrameOrientation_LandscapeLeft),
      mVoiceEngine(NULL),
      mVoiceBase(NULL),
      mVoiceCodec(NULL),
      mVoiceNetwork(NULL),
      mVoiceRtpRtcp(NULL),
      mVoiceAudioProcessing(NULL),
      mVoiceVolumeControl(NULL),
      mVoiceHardware(NULL),
      mVoiceFile(NULL),
      mVoiceEngineReady(false),
      mVcpm(NULL),
      mVideoEngine(NULL),
      mVideoBase(NULL),
      mVideoNetwork(NULL),
      mVideoRender(NULL),
      mVideoCapture(NULL),
      mVideoRtpRtcp(NULL),
      mVideoCodec(NULL),
      mVideoEngineReady(false),
      mLifetimeInProgress(false)
    {
#ifdef TARGET_OS_IPHONE
      int name[] = {CTL_HW, HW_MACHINE};
      size_t size;
      sysctl(name, 2, NULL, &size, NULL, 0);
      char *machine = (char *)malloc(size);
      sysctl(name, 2, machine, &size, NULL, 0);
      mMachineName = machine;
      free(machine);
#endif
    }
    
    //-----------------------------------------------------------------------
    MediaEngine::~MediaEngine()
    {
      if(isNoop()) return;
      
      destroyMediaEngine();
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::init()
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_BASIC(log("init media engine"))
      
      mVoiceEngine = webrtc::VoiceEngine::Create();
      if (mVoiceEngine == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to create voice engine"))
        return;
      }
      mVoiceBase = webrtc::VoEBase::GetInterface(mVoiceEngine);
      if (mVoiceBase == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to get interface for voice base"))
        return;
      }
      mVoiceCodec = webrtc::VoECodec::GetInterface(mVoiceEngine);
      if (mVoiceCodec == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to get interface for voice codec"))
        return;
      }
      mVoiceNetwork = webrtc::VoENetwork::GetInterface(mVoiceEngine);
      if (mVoiceNetwork == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to get interface for voice network"))
        return;
      }
      mVoiceRtpRtcp = webrtc::VoERTP_RTCP::GetInterface(mVoiceEngine);
      if (mVoiceRtpRtcp == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to get interface for voice RTP/RTCP"))
        return;
      }
      mVoiceAudioProcessing = webrtc::VoEAudioProcessing::GetInterface(mVoiceEngine);
      if (mVoiceAudioProcessing == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to get interface for audio processing"))
        return;
      }
      mVoiceVolumeControl = webrtc::VoEVolumeControl::GetInterface(mVoiceEngine);
      if (mVoiceVolumeControl == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to get interface for volume control"))
        return;
      }
      mVoiceHardware = webrtc::VoEHardware::GetInterface(mVoiceEngine);
      if (mVoiceHardware == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to get interface for audio hardware"))
        return;
      }
      mVoiceFile = webrtc::VoEFile::GetInterface(mVoiceEngine);
      if (mVoiceFile == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to get interface for voice file"))
        return;
      }
      
      get(mLastError) = mVoiceBase->Init();
      if (mLastError < 0) {
        ZS_LOG_ERROR(Detail, log("failed to initialize voice base") + ZS_PARAM("error", mVoiceBase->LastError()))
        return;
      } else if (mVoiceBase->LastError() > 0) {
        ZS_LOG_WARNING(Detail, log("an error has occured during voice base init") + ZS_PARAM("error", mVoiceBase->LastError()))
      }
      get(mLastError) = mVoiceBase->RegisterVoiceEngineObserver(*this);
      if (mLastError < 0) {
        ZS_LOG_ERROR(Detail, log("failed to register voice engine observer") + ZS_PARAM("error", mVoiceBase->LastError()))
        return;
      }
      
      mVideoEngine = webrtc::VideoEngine::Create();
      if (mVideoEngine == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to create video engine"))
        return;
      }
      
      mVideoBase = webrtc::ViEBase::GetInterface(mVideoEngine);
      if (mVideoBase == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to get interface for video base"))
        return;
      }
      mVideoCapture = webrtc::ViECapture::GetInterface(mVideoEngine);
      if (mVideoCapture == NULL) {
        ZS_LOG_ERROR(Detail, log("failed get interface for video capture"))
        return;
      }
      mVideoRtpRtcp = webrtc::ViERTP_RTCP::GetInterface(mVideoEngine);
      if (mVideoRtpRtcp == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to get interface for video RTP/RTCP"))
        return;
      }
      mVideoNetwork = webrtc::ViENetwork::GetInterface(mVideoEngine);
      if (mVideoNetwork == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to get interface for video network"))
        return;
      }
      mVideoRender = webrtc::ViERender::GetInterface(mVideoEngine);
      if (mVideoRender == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to get interface for video render"))
        return;
      }
      mVideoCodec = webrtc::ViECodec::GetInterface(mVideoEngine);
      if (mVideoCodec == NULL) {
        ZS_LOG_ERROR(Detail, log("failed to get interface for video codec"))
        return;
      }
      
      get(mLastError) = mVideoBase->Init();
      if (mLastError < 0) {
        ZS_LOG_ERROR(Detail, log("failed to initialize video base") + ZS_PARAM("error", mVideoBase->LastError()))
        return;
      } else if (mVideoBase->LastError() > 0) {
        ZS_LOG_WARNING(Detail, log("an error has occured during video base init") + ZS_PARAM("error", mVideoBase->LastError()))
      }
      
      get(mLastError) = mVideoBase->SetVoiceEngine(mVoiceEngine);
      if (mLastError < 0) {
        ZS_LOG_ERROR(Detail, log("failed to set voice engine for video base") + ZS_PARAM("error", mVideoBase->LastError()))
        return;
      }
      
      Log::Level logLevel = ZS_GET_LOG_LEVEL();
      
      unsigned int traceFilter;
      switch (logLevel) {
        case Log::None:
          traceFilter = webrtc::kTraceNone;
          break;
        case Log::Basic:
          traceFilter = webrtc::kTraceWarning | webrtc::kTraceError | webrtc::kTraceCritical;
          break;
        case Log::Detail:
          traceFilter = webrtc::kTraceStateInfo | webrtc::kTraceWarning | webrtc::kTraceError | webrtc::kTraceCritical | webrtc::kTraceApiCall;
          break;
        case Log::Debug:
          traceFilter = webrtc::kTraceDefault | webrtc::kTraceDebug | webrtc::kTraceInfo;
          break;
        case Log::Trace:
          traceFilter = webrtc::kTraceAll;
          break;
        default:
          traceFilter = webrtc::kTraceNone;
          break;
      }
      
      if (logLevel != Log::None) {
        get(mLastError) = mVoiceEngine->SetTraceFilter(traceFilter);
        if (mLastError < 0) {
          ZS_LOG_ERROR(Detail, log("failed to set trace filter for voice") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
        get(mLastError) = mVoiceEngine->SetTraceCallback(this);
        if (mLastError < 0) {
          ZS_LOG_ERROR(Detail, log("failed to set trace callback for voice") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
        get(mLastError) = mVideoEngine->SetTraceFilter(traceFilter);
        if (mLastError < 0) {
          ZS_LOG_ERROR(Detail, log("failed to set trace filter for video") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        get(mLastError) = mVideoEngine->SetTraceCallback(this);
        if (mLastError < 0) {
          ZS_LOG_ERROR(Detail, log("failed to set trace callback for video") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
      }
    }

    //-----------------------------------------------------------------------
    MediaEnginePtr MediaEngine::singleton(IMediaEngineDelegatePtr delegate)
    {
      static SingletonLazySharedPtr<MediaEngine> singleton(IMediaEngineFactory::singleton().create(delegate));
      return singleton.singleton();
    }

    //-----------------------------------------------------------------------
    void MediaEngine::setup(IMediaEngineDelegatePtr delegate)
    {
      singleton(delegate);
    }
    
    //-----------------------------------------------------------------------
    MediaEnginePtr MediaEngine::create(IMediaEngineDelegatePtr delegate)
    {
      MediaEnginePtr pThis = MediaEnginePtr(new MediaEngine(IORTCForInternal::queueBlockingMediaStartStopThread(), delegate));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-----------------------------------------------------------------------
    void MediaEngine::destroyMediaEngine()
    {
      // scope: delete voice engine
      {
        if (mVoiceBase) {
          get(mLastError) = mVoiceBase->DeRegisterVoiceEngineObserver();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to deregister voice engine observer") + ZS_PARAM("error", mVoiceBase->LastError()))
            return;
          }
          get(mLastError) = mVoiceBase->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release voice base") + ZS_PARAM("error", mVoiceBase->LastError()))
            return;
          }
        }
        
        if (mVoiceCodec) {
          get(mLastError) = mVoiceCodec->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release voice codec") + ZS_PARAM("error", mVoiceBase->LastError()))
            return;
          }
        }
        
        if (mVoiceNetwork) {
          get(mLastError) = mVoiceNetwork->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release voice network") + ZS_PARAM("error", mVoiceBase->LastError()))
            return;
          }
        }
        
        if (mVoiceRtpRtcp) {
          get(mLastError) = mVoiceRtpRtcp->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release voice RTP/RTCP") + ZS_PARAM("error", mVoiceBase->LastError()))
            return;
          }
        }
        
        if (mVoiceAudioProcessing) {
          get(mLastError) = mVoiceAudioProcessing->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release audio processing") + ZS_PARAM("error", mVoiceBase->LastError()))
            return;
          }
        }
        
        if (mVoiceVolumeControl) {
          get(mLastError) = mVoiceVolumeControl->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release volume control") + ZS_PARAM("error", mVoiceBase->LastError()))
            return;
          }
        }
        
        if (mVoiceHardware) {
          get(mLastError) = mVoiceHardware->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release audio hardware") + ZS_PARAM("error", mVoiceBase->LastError()))
            return;
          }
        }
        
        if (mVoiceFile) {
          get(mLastError) = mVoiceFile->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release voice file") + ZS_PARAM("error", mVoiceBase->LastError()))
            return;
          }
        }
        
        if (!VoiceEngine::Delete(mVoiceEngine)) {
          ZS_LOG_ERROR(Detail, log("failed to delete voice engine"))
          return;
        }
      }
      
      // scope; delete video engine
      {
        if (mVideoBase) {
          get(mLastError) = mVideoBase->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release video base") + ZS_PARAM("error", mVideoBase->LastError()))
            return;
          }
        }
        
        if (mVideoNetwork) {
          get(mLastError) = mVideoNetwork->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release video network") + ZS_PARAM("error", mVideoBase->LastError()))
            return;
          }
        }
        
        if (mVideoRender) {
          get(mLastError) = mVideoRender->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release video render") + ZS_PARAM("error", mVideoBase->LastError()))
            return;
          }
        }
        
        if (mVideoCapture) {
          get(mLastError) = mVideoCapture->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release video capture") + ZS_PARAM("error", mVideoBase->LastError()))
            return;
          }
        }
        
        if (mVideoRtpRtcp) {
          get(mLastError) = mVideoRtpRtcp->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release video RTP/RTCP") + ZS_PARAM("error", mVideoBase->LastError()))
            return;
          }
        }
        
        if (mVideoCodec) {
          get(mLastError) = mVideoCodec->Release();
          if (mLastError < 0) {
            ZS_LOG_ERROR(Detail, log("failed to release video codec") + ZS_PARAM("error", mVideoBase->LastError()))
            return;
          }
        }
        
        if (!VideoEngine::Delete(mVideoEngine)) {
          ZS_LOG_ERROR(Detail, log("failed to delete video engine"))
          return;
        }
      }
    }
    
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    void MediaEngine::setDefaultVideoOrientation(CapturedFrameOrientation orientation)
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("set default video orientation") + ZS_PARAM("orientation", orientation))
      
      mDefaultVideoOrientation = orientation;
    }
    
    //-------------------------------------------------------------------------
    webrtc::CapturedFrameOrientation MediaEngine::getDefaultVideoOrientation()
    {
      AutoRecursiveLock lock(mLock);
    
      ZS_LOG_DEBUG(log("get default video orientation"))
      
      return mDefaultVideoOrientation;
    }
    
    //-------------------------------------------------------------------------
    void MediaEngine::setRecordVideoOrientation(CapturedFrameOrientation orientation)
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("set record video orientation") + ZS_PARAM("orientation", orientation))
      
      mRecordVideoOrientation = orientation;
    }
    
    //-------------------------------------------------------------------------
    webrtc::CapturedFrameOrientation MediaEngine::getRecordVideoOrientation()
    {
      AutoRecursiveLock lock(mLock);

      ZS_LOG_DEBUG(log("get record video orientation"))
      
      return mRecordVideoOrientation;
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::setVideoOrientation()
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("set video orientation and codec parameters"))

      //get(mLastError) = setVideoCaptureRotation(captureId);
      //get(mLastError) = setVideoCodecParameters(channelId, captureId);
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::setRenderView(int sourceId, void *renderView)
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("set render view"))
      
      VideoSourceInfoMap::iterator sourceInfoIter = mVideoSourceInfos.find(sourceId);
      
      if (sourceInfoIter != mVideoSourceInfos.end()) {
        sourceInfoIter->second.mRenderView = renderView;
        return;
      }

      VideoChannelInfoMap::iterator channelInfoIter = mVideoChannelInfos.find(sourceId);
      
      if (channelInfoIter != mVideoChannelInfos.end()) {
        channelInfoIter->second.mRenderView = renderView;
        return;
      }
      
      ZS_LOG_ERROR(Detail, log("video source not found") + ZS_PARAM("sourceId", sourceId))
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::setEcEnabled(int channelId, bool enabled)
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("set EC enabled") + ZS_PARAM("value", (enabled ? "true" : "false")))
      
      VoiceChannelInfoMap::iterator iter = mVoiceChannelInfos.find(channelId);
      if (iter == mVoiceChannelInfos.end()) {
        ZS_LOG_ERROR(Detail, log("voice channel not found") + ZS_PARAM("channelId", channelId))
        return;
      }
      
      webrtc::EcModes ecMode = getEcMode();
      if (ecMode == webrtc::kEcUnchanged) {
        return;
      }
      get(mLastError) = mVoiceAudioProcessing->SetEcStatus(enabled, ecMode);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to set acoustic echo canceller status") + ZS_PARAM("error", mVoiceBase->LastError()))
        return;
      }
      if (ecMode == webrtc::kEcAecm && enabled) {
        get(mLastError) = mVoiceAudioProcessing->SetAecmMode(webrtc::kAecmSpeakerphone);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to set acoustic echo canceller mobile mode") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
      }
      
      iter->second.mEcEnabled = enabled;
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::setAgcEnabled(int channelId, bool enabled)
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("set AGC enabled") + ZS_PARAM("value", enabled ? "true" : "false"))
      
      VoiceChannelInfoMap::iterator iter = mVoiceChannelInfos.find(channelId);
      if (iter == mVoiceChannelInfos.end()) {
        ZS_LOG_ERROR(Detail, log("voice channel not found") + ZS_PARAM("channelId", channelId))
        return;
      }
      
      get(mLastError) = mVoiceAudioProcessing->SetAgcStatus(enabled, webrtc::kAgcAdaptiveDigital);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to set automatic gain control status") + ZS_PARAM("error", mVoiceBase->LastError()))
        return;
      }
      
      iter->second.mAgcEnabled = enabled;
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::setNsEnabled(int channelId, bool enabled)
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("set NS enabled") + ZS_PARAM("value", enabled ? "true" : "false"))
      
      VoiceChannelInfoMap::iterator iter = mVoiceChannelInfos.find(channelId);
      if (iter == mVoiceChannelInfos.end()) {
        ZS_LOG_ERROR(Detail, log("voice channel not found") + ZS_PARAM("channelId", channelId))
        return;
      }

      get(mLastError) = mVoiceAudioProcessing->SetNsStatus(enabled, webrtc::kNsLowSuppression);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to set noise suppression status") + ZS_PARAM("error", mVoiceBase->LastError()))
        return;
      }
      
      iter->second.mNsEnabled = enabled;
    }
    
    //-------------------------------------------------------------------------
    void MediaEngine::setVoiceRecordFile(String fileName)
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("set voice record file") + ZS_PARAM("value", fileName))
      
      mVoiceRecordFile = fileName;
    }
    
    //-------------------------------------------------------------------------
    String MediaEngine::getVoiceRecordFile() const
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("get voice record file") + ZS_PARAM("value", mVoiceRecordFile))
      
      return mVoiceRecordFile;
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::setMuteEnabled(bool enabled)
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("set microphone mute enabled") + ZS_PARAM("value", enabled ? "true" : "false"))
      
      get(mLastError) = mVoiceVolumeControl->SetInputMute(-1, enabled);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to set microphone mute") + ZS_PARAM("error", mVoiceBase->LastError()))
        return;
      }
    }

    //-----------------------------------------------------------------------
    bool MediaEngine::getMuteEnabled()
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("get microphone mute enabled"))
      
      bool enabled;
      get(mLastError) = mVoiceVolumeControl->GetInputMute(-1, enabled);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to set microphone mute") + ZS_PARAM("error", mVoiceBase->LastError()))
        return false;
      }
      
      return enabled;
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::setLoudspeakerEnabled(bool enabled)
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("set loudspeaker enabled") + ZS_PARAM("value", enabled ? "true" : "false"))
      
      get(mLastError) = mVoiceHardware->SetLoudspeakerStatus(enabled);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to set loudspeaker") + ZS_PARAM("error", mVoiceBase->LastError()))
        return;
      }
    }
    
    //-----------------------------------------------------------------------
    bool MediaEngine::getLoudspeakerEnabled()
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("get loudspeaker enabled"))
      
      bool enabled;
      get(mLastError) = mVoiceHardware->GetLoudspeakerStatus(enabled);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to get loudspeaker") + ZS_PARAM("error", mVoiceBase->LastError()))
        return false;
      }
      
      return enabled;
    }
    
    //-----------------------------------------------------------------------
    webrtc::OutputAudioRoute MediaEngine::getOutputAudioRoute()
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("get output audio route"))
      
      OutputAudioRoute route;
      get(mLastError) = mVoiceHardware->GetOutputAudioRoute(route);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to get output audio route") + ZS_PARAM("error", mVoiceBase->LastError()))
        return webrtc::kOutputAudioRouteBuiltInSpeaker;
      }
      
      return route;
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::setContinuousVideoCapture(int captureId, bool continuousVideoCapture)
    {
      AutoRecursiveLock lock(mLifetimeLock);
      
      ZS_LOG_DEBUG(log("set continuous video capture") + ZS_PARAM("value", continuousVideoCapture ? "true" : "false"))
      
      VideoSourceLifetimeStateMap::iterator iter = mVideoSourceLifetimeStates.find(captureId);
      if (iter == mVideoSourceLifetimeStates.end()) {
        ZS_LOG_ERROR(Detail, log("video source not found") + ZS_PARAM("captureId", captureId))
        return;
      }

      iter->second.mLifetimeContinuousVideoCapture = continuousVideoCapture;
    }
    
    //-----------------------------------------------------------------------
    bool MediaEngine::getContinuousVideoCapture(int captureId)
    {
      AutoRecursiveLock lock(mLifetimeLock);
      
      ZS_LOG_DEBUG(log("get continuous video capture"))
      
      VideoSourceLifetimeStateMap::iterator iter = mVideoSourceLifetimeStates.find(captureId);
      if (iter == mVideoSourceLifetimeStates.end()) {
        ZS_LOG_ERROR(Detail, log("video source not found") + ZS_PARAM("captureId", captureId))
        return false;
      }
      
      return iter->second.mLifetimeContinuousVideoCapture;
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::setFaceDetection(int captureId, bool faceDetection)
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("set face detection ") + ZS_PARAM("value", faceDetection ? "true" : "false"))
      
      VideoSourceInfoMap::iterator iter = mVideoSourceInfos.find(captureId);
      if (iter == mVideoSourceInfos.end()) {
        ZS_LOG_ERROR(Detail, log("video source not found") + ZS_PARAM("captureId", captureId))
        return;
      }

      iter->second.mFaceDetection = faceDetection;
    }
    
    //-----------------------------------------------------------------------
    bool MediaEngine::getFaceDetection(int captureId)
    {
      AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("get face detection"))
      
      VideoSourceInfoMap::iterator iter = mVideoSourceInfos.find(captureId);
      if (iter == mVideoSourceInfos.end()) {
        ZS_LOG_ERROR(Detail, log("video source not found") + ZS_PARAM("captureId", captureId))
        return false;
      }
      
      return iter->second.mFaceDetection;
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::setCameraType(int captureId, CameraTypes type)
    {
      {
        AutoRecursiveLock lock(mLifetimeLock);
        
        VideoSourceLifetimeStateMap::iterator iter = mVideoSourceLifetimeStates.find(captureId);
        if (iter == mVideoSourceLifetimeStates.end()) {
          ZS_LOG_ERROR(Detail, log("video source not found") + ZS_PARAM("captureId", captureId))
          return;
        }
        
        iter->second.mLifetimeWantCameraType = type;
      }
      
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }
    
    //-----------------------------------------------------------------------
    CameraTypes MediaEngine::getCameraType(int captureId)
    {
      AutoRecursiveLock lock(mLifetimeLock);  // WARNING: THIS IS THE LIFETIME LOCK AND NOT THE MAIN OBJECT LOCK
      
      VideoSourceLifetimeStateMap::iterator iter = mVideoSourceLifetimeStates.find(captureId);
      if (iter == mVideoSourceLifetimeStates.end()) {
        ZS_LOG_ERROR(Detail, log("video source not found") + ZS_PARAM("captureId", captureId))
        return CameraType_None;
      }
      
      return iter->second.mLifetimeWantCameraType;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::createVideoSource()
    {
      return internalCreateVideoSource();
    }
    
    //-----------------------------------------------------------------------
    std::list<int> MediaEngine::getVideoSources()
    {
      return std::list<int>();
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::createVideoChannel()
    {
      return internalCreateVideoChannel();
    }

    //-----------------------------------------------------------------------
    std::list<int> MediaEngine::getVideoChannels()
    {
      return std::list<int>();
    }

    //-----------------------------------------------------------------------
    void MediaEngine::startVideoCapture(int captureId)
    {
      {
        AutoRecursiveLock lock(mLifetimeLock);
        
        VideoSourceLifetimeStateMap::iterator iter = mVideoSourceLifetimeStates.find(captureId);
        if (iter == mVideoSourceLifetimeStates.end()) {
          ZS_LOG_ERROR(Detail, log("video source not found") + ZS_PARAM("captureId", captureId))
          return;
        }
        
        iter->second.mLifetimeWantVideoCapture = true;
      }
      
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::stopVideoCapture(int captureId)
    {
      {
        AutoRecursiveLock lock(mLifetimeLock);
        
        VideoSourceLifetimeStateMap::iterator iter = mVideoSourceLifetimeStates.find(captureId);
        if (iter == mVideoSourceLifetimeStates.end()) {
          ZS_LOG_ERROR(Detail, log("video source not found") + ZS_PARAM("captureId", captureId))
          return;
        }
        
        iter->second.mLifetimeWantVideoCapture = false;
      }
      
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::startSendVideoChannel(int channelId, int captureId)
    {
      {
        AutoRecursiveLock lock(mLifetimeLock);
        
        VideoChannelLifetimeStateMap::iterator iter = mVideoChannelLifetimeStates.find(channelId);
        if (iter == mVideoChannelLifetimeStates.end()) {
          ZS_LOG_ERROR(Detail, log("video channel not found") + ZS_PARAM("channelId", channelId))
          return;
        }
        
        iter->second.mLifetimeWantSendVideoChannel = true;
      }
      
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::startReceiveVideoChannel(int channelId)
    {
      {
        AutoRecursiveLock lock(mLifetimeLock);
        
        VideoChannelLifetimeStateMap::iterator iter = mVideoChannelLifetimeStates.find(channelId);
        if (iter == mVideoChannelLifetimeStates.end()) {
          ZS_LOG_ERROR(Detail, log("video channel not found") + ZS_PARAM("channelId", channelId))
          return;
        }
        
        iter->second.mLifetimeWantReceiveVideoChannel = true;
      }
      
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::stopSendVideoChannel(int channelId)
    {
      {
        AutoRecursiveLock lock(mLifetimeLock);
        
        VideoChannelLifetimeStateMap::iterator iter = mVideoChannelLifetimeStates.find(channelId);
        if (iter == mVideoChannelLifetimeStates.end()) {
          ZS_LOG_ERROR(Detail, log("video channel not found") + ZS_PARAM("channelId", channelId))
          return;
        }
        
        iter->second.mLifetimeWantSendVideoChannel = false;
      }
      
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::stopReceiveVideoChannel(int channelId)
    {
      {
        AutoRecursiveLock lock(mLifetimeLock);
        
        VideoChannelLifetimeStateMap::iterator iter = mVideoChannelLifetimeStates.find(channelId);
        if (iter == mVideoChannelLifetimeStates.end()) {
          ZS_LOG_ERROR(Detail, log("video channel not found") + ZS_PARAM("channelId", channelId))
          return;
        }
        
        iter->second.mLifetimeWantReceiveVideoChannel = false;
      }
      
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::createVoiceChannel()
    {
      return internalCreateVoiceChannel();
    }
    
    //-----------------------------------------------------------------------
    std::list<int> MediaEngine::getVoiceChannels()
    {
      return std::list<int>();
    }

    //-----------------------------------------------------------------------
    void MediaEngine::startSendVoice(int channelId)
    {
      {
        AutoRecursiveLock lock(mLifetimeLock);
        
        VoiceChannelLifetimeStateMap::iterator iter = mVoiceChannelLifetimeStates.find(channelId);
        if (iter == mVoiceChannelLifetimeStates.end()) {
          ZS_LOG_ERROR(Detail, log("voice channel not found") + ZS_PARAM("channelId", channelId))
          return;
        }
        
        iter->second.mLifetimeWantSendAudio = true;
      }
      
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::startReceiveVoice(int channelId)
    {
      {
        AutoRecursiveLock lock(mLifetimeLock);
        
        VoiceChannelLifetimeStateMap::iterator iter = mVoiceChannelLifetimeStates.find(channelId);
        if (iter == mVoiceChannelLifetimeStates.end()) {
          ZS_LOG_ERROR(Detail, log("voice channel not found") + ZS_PARAM("channelId", channelId))
          return;
        }
        
        iter->second.mLifetimeWantReceiveAudio = true;
      }
      
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::stopSendVoice(int channelId)
    {
      {
        AutoRecursiveLock lock(mLifetimeLock);
        
        VoiceChannelLifetimeStateMap::iterator iter = mVoiceChannelLifetimeStates.find(channelId);
        if (iter == mVoiceChannelLifetimeStates.end()) {
          ZS_LOG_ERROR(Detail, log("voice channel not found") + ZS_PARAM("channelId", channelId))
          return;
        }
        
        iter->second.mLifetimeWantSendAudio = false;
      }
      
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::stopReceiveVoice(int channelId)
    {
      {
        AutoRecursiveLock lock(mLifetimeLock);
        
        VoiceChannelLifetimeStateMap::iterator iter = mVoiceChannelLifetimeStates.find(channelId);
        if (iter == mVoiceChannelLifetimeStates.end()) {
          ZS_LOG_ERROR(Detail, log("voice channel not found") + ZS_PARAM("channelId", channelId))
          return;
        }
        
        iter->second.mLifetimeWantReceiveAudio = false;
      }
      
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::getVideoTransportStatistics(int channelId, CallStatistics &stat)
    {
      AutoRecursiveLock lock(mLock);

      unsigned short fractionLost;
      unsigned int cumulativeLost;
      unsigned int extendedMax;
      unsigned int jitter;
      int rttMs;

      get(mLastError) = mVideoRtpRtcp->GetReceivedRTCPStatistics(channelId, fractionLost, cumulativeLost, extendedMax, jitter, rttMs);
      if (0 != mLastError) {
        ZS_LOG_ERROR(Detail, log("failed to get received RTCP statistics for video") + ZS_PARAM("error", mVideoBase->LastError()))
        return mLastError;
      }

      unsigned int bytesSent;
      unsigned int packetsSent;
      unsigned int bytesReceived;
      unsigned int packetsReceived;

      get(mLastError) = mVideoRtpRtcp->GetRTPStatistics(channelId, bytesSent, packetsSent, bytesReceived, packetsReceived);
      if (0 != mLastError) {
        ZS_LOG_ERROR(Detail, log("failed to get RTP statistics for video") + ZS_PARAM("error", mVideoBase->LastError()))
        return mLastError;
      }

      stat.fractionLost = fractionLost;
      stat.cumulativeLost = cumulativeLost;
      stat.extendedMax = extendedMax;
      //stat.jitter = jitter;
      stat.rttMs = rttMs;
      stat.bytesSent = bytesSent;
      stat.packetsSent = packetsSent;
      stat.bytesReceived = bytesReceived;
      stat.packetsReceived = packetsReceived;

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::getVoiceTransportStatistics(int channelId, CallStatistics &stat)
    {
      AutoRecursiveLock lock(mLock);

      webrtc::CallStatistics callStat;

      get(mLastError) = mVoiceRtpRtcp->GetRTCPStatistics(channelId, callStat);
      if (0 != mLastError) {
        ZS_LOG_ERROR(Detail, log("failed to get RTCP statistics for voice") + ZS_PARAM("error", mVoiceBase->LastError()))
        return mLastError;
      }

      stat.fractionLost = callStat.fractionLost;
      stat.cumulativeLost = callStat.cumulativeLost;
      stat.extendedMax = callStat.extendedMax;
//        stat.jitter = callStat.jitterSamples;
      stat.rttMs = callStat.rttMs;
      stat.bytesSent = callStat.bytesSent;
      stat.packetsSent = callStat.packetsSent;
      stat.bytesReceived = callStat.bytesReceived;
      stat.packetsReceived = callStat.packetsReceived;

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::registerVoiceExternalTransport(int channelId, Transport &transport)
    {
      AutoRecursiveLock lock(mLock);

      ZS_LOG_DEBUG(log("register voice external transport"))
      
      mVoiceChannelInfos[channelId].mRedirectTransport->redirect(&transport);

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::deregisterVoiceExternalTransport(int channelId)
    {
      AutoRecursiveLock lock(mLock);

      ZS_LOG_DEBUG(log("deregister voice external transport"))
      
      mVoiceChannelInfos[channelId].mRedirectTransport->redirect(NULL);

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::receivedVoiceRTPPacket(int channelId, const void *data, unsigned int length)
    {
      int channel = ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
      {
        AutoRecursiveLock lock(mMediaEngineReadyLock);
        if (mVoiceEngineReady)
          channel = channelId;
      }
      
      if (ORTC_MEDIA_ENGINE_INVALID_CHANNEL == channel) {
        ZS_LOG_WARNING(Debug, log("voice channel is not ready yet"))
        return -1;
      }
      
      get(mLastError) = mVoiceNetwork->ReceivedRTPPacket(channel, data, length);
      if (0 != mLastError) {
        ZS_LOG_ERROR(Detail, log("received voice RTP packet failed") + ZS_PARAM("error", mVoiceBase->LastError()))
        return mLastError;
      }

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::receivedVoiceRTCPPacket(int channelId, const void* data, unsigned int length)
    {
      int channel = ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
      {
        AutoRecursiveLock lock(mMediaEngineReadyLock);
        if (mVoiceEngineReady)
          channel = channelId;
      }
      
      if (ORTC_MEDIA_ENGINE_INVALID_CHANNEL == channel) {
        ZS_LOG_WARNING(Debug, log("voice channel is not ready yet"))
        return -1;
      }
      
      get(mLastError) = mVoiceNetwork->ReceivedRTCPPacket(channel, data, length);
      if (0 != mLastError) {
        ZS_LOG_ERROR(Detail, log("received voice RTCP packet failed") + ZS_PARAM("error", mVoiceBase->LastError()))
        return mLastError;
      }

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::registerVideoExternalTransport(int channelId, Transport &transport)
    {
      AutoRecursiveLock lock(mLock);

      ZS_LOG_DEBUG(log("register voice external transport"))
       
      mVideoChannelInfos[channelId].mRedirectTransport->redirect(&transport);

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::deregisterVideoExternalTransport(int channelId)
    {
      AutoRecursiveLock lock(mLock);

      ZS_LOG_DEBUG(log("deregister voice external transport"))
       
      mVideoChannelInfos[channelId].mRedirectTransport->redirect(NULL);

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::receivedVideoRTPPacket(int channelId, const void *data, unsigned int length)
    {
      int channel = ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
      {
        AutoRecursiveLock lock(mMediaEngineReadyLock);
        if (mVoiceEngineReady)
          channel = channelId;
      }

      if (ORTC_MEDIA_ENGINE_INVALID_CHANNEL == channel) {
        ZS_LOG_WARNING(Debug, log("voice channel is not ready yet"))
        return -1;
      }

      get(mLastError) = mVoiceNetwork->ReceivedRTPPacket(channel, data, length);
      if (0 != mLastError) {
        ZS_LOG_ERROR(Detail, log("received voice RTP packet failed") + ZS_PARAM("error", mVoiceBase->LastError()))
        return mLastError;
      }

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::receivedVideoRTCPPacket(int channelId, const void* data, unsigned int length)
    {
      int channel = ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
      {
        AutoRecursiveLock lock(mMediaEngineReadyLock);
        if (mVoiceEngineReady)
          channel = channelId;
      }
       
      if (ORTC_MEDIA_ENGINE_INVALID_CHANNEL == channel) {
        ZS_LOG_WARNING(Debug, log("voice channel is not ready yet"))
        return -1;
      }
       
      get(mLastError) = mVoiceNetwork->ReceivedRTCPPacket(channel, data, length);
      if (0 != mLastError) {
        ZS_LOG_ERROR(Detail, log("received voice RTCP packet failed") + ZS_PARAM("error", mVoiceBase->LastError()))
        return mLastError;
      }

      return 0;
    }

    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => TraceCallback
    #pragma mark
    
    //-----------------------------------------------------------------------
    void MediaEngine::Print(const webrtc::TraceLevel level, const char *traceString, const int length)
    {
      switch (level) {
        case webrtc::kTraceApiCall:
        case webrtc::kTraceStateInfo:
          ZS_LOG_DETAIL(log(traceString))
          break;
        case webrtc::kTraceDebug:
        case webrtc::kTraceInfo:
          ZS_LOG_DEBUG(log(traceString))
          break;
        case webrtc::kTraceWarning:
          ZS_LOG_WARNING(Detail, log(traceString))
          break;
        case webrtc::kTraceError:
          ZS_LOG_ERROR(Detail, log(traceString))
          break;
        case webrtc::kTraceCritical:
          ZS_LOG_FATAL(Detail, log(traceString))
          break;
        case webrtc::kTraceModuleCall:
        case webrtc::kTraceMemory:
        case webrtc::kTraceTimer:
        case webrtc::kTraceStream:
          ZS_LOG_TRACE(log(traceString))
          break;
        default:
          ZS_LOG_TRACE(log(traceString))
          break;
      }
    }
    
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => VoiceEngineObserver
    #pragma mark
    
    //-----------------------------------------------------------------------
    void MediaEngine::CallbackOnError(const int errCode, const int channel)
    {
      ZS_LOG_ERROR(Detail, log("Voice engine error") + ZS_PARAM("error", errCode))
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::CallbackOnOutputAudioRouteChange(const webrtc::OutputAudioRoute inRoute)
    {
      mSubscriptions.delegate()->onMediaEngineAudioRouteChanged(inRoute);
      
      ZS_LOG_DEBUG(log("Audio output route changed") + ZS_PARAM("route", inRoute))
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::CallbackOnAudioSessionInterruptionBegin()
    {
      mSubscriptions.delegate()->onMediaEngineAudioSessionInterruptionBegan();
      
      ZS_LOG_DEBUG(log("Audio session interruption began"))
    }

    //-----------------------------------------------------------------------
    void MediaEngine::CallbackOnAudioSessionInterruptionEnd()
    {
      mSubscriptions.delegate()->onMediaEngineAudioSessionInterruptionEnded();
      
      ZS_LOG_DEBUG(log("Audio session interruption ended"))
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => ViECaptureObserver
    #pragma mark
    
    //-------------------------------------------------------------------------
    void MediaEngine::BrightnessAlarm(const int capture_id, const webrtc::Brightness brightness)
    {
      
    }
    
    //-------------------------------------------------------------------------
    void MediaEngine::CapturedFrameRate(const int capture_id, const unsigned char frame_rate)
    {
      
    }
    
    //-------------------------------------------------------------------------
    void MediaEngine::NoPictureAlarm(const int capture_id, const webrtc::CaptureAlarm alarm)
    {
      
    }
    
    //-------------------------------------------------------------------------
    void MediaEngine::FaceDetected(const int capture_id)
    {
      mSubscriptions.delegate()->onMediaEngineFaceDetected(capture_id);
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => IWakeDelegate
    #pragma mark
    
    //-------------------------------------------------------------------------
    void MediaEngine::onWake()
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
    #pragma mark MediaEngine => (internal)
    #pragma mark
    
    //-------------------------------------------------------------------------
    Log::Params MediaEngine::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::MediaEngine");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }
    
    //-------------------------------------------------------------------------
    Log::Params MediaEngine::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }
    
    //-------------------------------------------------------------------------
    ElementPtr MediaEngine::toDebug() const
    {
      ElementPtr resultEl = Element::create("MediaStreamTrack");
      
      UseServicesHelper::debugAppend(resultEl, "id", mID);
      
      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);
      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", mShutdown);

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);
      
      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool MediaEngine::isShuttingDown() const
    {
      return (bool)mGracefulShutdownReference;
    }
    
    //-------------------------------------------------------------------------
    bool MediaEngine::isShutdown() const
    {
      if (mGracefulShutdownReference) return false;
      return mShutdown;
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => (internal)
    #pragma mark
    
    //-----------------------------------------------------------------------
    void MediaEngine::step()
    {
      ZS_LOG_DEBUG(debug("step"))
      
      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

      ZS_LOG_DEBUG(log("media engine lifetime message"))
      
      // attempt to get the lifetime lock
      {
        AutoRecursiveLock lock(mLifetimeLock);
        if (mLifetimeInProgress) {
          ZS_LOG_ERROR(Detail, log("could not obtain media lifetime lock"))
          return;
        }
        
        mLifetimeInProgress = true;
      }
      
      for (VoiceSourceLifetimeStateMap::iterator iter = mVoiceSourceLifetimeStates.begin(); iter != mVoiceSourceLifetimeStates.end(); iter++) {
        
        VoiceSourceLifetimeState state = iter->second;
        int sourceId = state.mSourceId;
      }
      
      for (VoiceChannelLifetimeStateMap::iterator iter = mVoiceChannelLifetimeStates.begin(); iter != mVoiceChannelLifetimeStates.end(); iter++) {
        
        VoiceChannelLifetimeState state = iter->second;
        int channelId = state.mChannelId;
        
        bool wantSendAudio = false;
        bool wantReceiveAudio = false;
        bool hasSendAudio = false;
        bool hasReceiveAudio = false;
        
        {
          AutoRecursiveLock lock(mLifetimeLock);
          
          wantSendAudio = state.mLifetimeWantSendAudio;
          wantReceiveAudio = state.mLifetimeWantReceiveAudio;
          hasSendAudio = state.mLifetimeHasSendAudio;
          hasReceiveAudio = state.mLifetimeHasReceiveAudio;
        }
        
        {
          AutoRecursiveLock lock(mLock);
          
          if (wantSendAudio) {
            if (!hasSendAudio) {
              internalStartSendVoice(channelId);
            }
          } else {
            if (hasSendAudio) {
              internalStopSendVoice(channelId);
            }
          }
          
          if (wantReceiveAudio) {
            if (!hasReceiveAudio) {
              internalStartReceiveVoice(channelId);
            }
          } else {
            if (hasReceiveAudio) {
              internalStopReceiveVoice(channelId);
            }
          }
        }
        
        {
          AutoRecursiveLock lock(mLifetimeLock);
          
          mVoiceChannelLifetimeStates[channelId].mLifetimeHasSendAudio = wantSendAudio;
          mVoiceChannelLifetimeStates[channelId].mLifetimeHasReceiveAudio = wantReceiveAudio;
        }
      }
      
      for (VideoSourceLifetimeStateMap::iterator iter = mVideoSourceLifetimeStates.begin(); iter != mVideoSourceLifetimeStates.end(); iter++) {
        
        VideoSourceLifetimeState state = iter->second;
        int sourceId = state.mSourceId;
        
        bool wantVideoCapture = false;
        bool hasVideoCapture = false;
        CameraTypes wantCameraType = CameraType_None;
        
        {
          AutoRecursiveLock lock(mLifetimeLock);
          
          wantVideoCapture = state.mLifetimeWantVideoCapture;
          hasVideoCapture = state.mLifetimeHasVideoCapture;
          wantCameraType = state.mLifetimeWantCameraType;
        }
        
        {
          AutoRecursiveLock lock(mLock);
          
          if (wantVideoCapture) {
            if (!hasVideoCapture) {
              internalStartVideoCapture(sourceId);
            }
          }
        }
        
        {
          AutoRecursiveLock lock(mLifetimeLock);
          
          mVideoSourceLifetimeStates[sourceId].mLifetimeHasVideoCapture = wantVideoCapture;
        }
      }
      
      for (VideoChannelLifetimeStateMap::iterator iter = mVideoChannelLifetimeStates.begin(); iter != mVideoChannelLifetimeStates.end(); iter++) {
        
        VideoChannelLifetimeState state = iter->second;
        int channelId = state.mChannelId;
        
        bool wantSendVideoChannel = false;
        bool wantReceiveVideoChannel = false;
        bool hasSendVideoChannel = false;
        bool hasReceiveVideoChannel = false;
        
        {
          AutoRecursiveLock lock(mLifetimeLock);
          
          wantSendVideoChannel = state.mLifetimeWantSendVideoChannel;
          wantReceiveVideoChannel = state.mLifetimeWantReceiveVideoChannel;
          hasSendVideoChannel = state.mLifetimeHasSendVideoChannel;
          hasReceiveVideoChannel = state.mLifetimeHasReceiveVideoChannel;
        }
        
        {
          AutoRecursiveLock lock(mLock);
          
          if (wantSendVideoChannel) {
            if (!hasSendVideoChannel) {
              internalStartSendVideoChannel(channelId, mVideoSourceLifetimeStates.begin()->second.mSourceId);
            }
          } else {
            if (hasSendVideoChannel) {
              internalStopSendVideoChannel(channelId);
            }
          }
          
          if (wantReceiveVideoChannel) {
            if (!hasReceiveVideoChannel) {
              internalStartReceiveVideoChannel(channelId);
            }
          } else {
            if (hasReceiveVideoChannel) {
              internalStopReceiveVideoChannel(channelId);
            }
          }
        }
        
        {
          AutoRecursiveLock lock(mLifetimeLock);
          
          mVideoChannelLifetimeStates[channelId].mLifetimeHasSendVideoChannel = wantSendVideoChannel;
          mVideoChannelLifetimeStates[channelId].mLifetimeHasReceiveVideoChannel = wantReceiveVideoChannel;
        }
      }
      
      for (VideoSourceLifetimeStateMap::iterator iter = mVideoSourceLifetimeStates.begin(); iter != mVideoSourceLifetimeStates.end(); iter++) {
        
        VideoSourceLifetimeState state = iter->second;
        int sourceId = state.mSourceId;
        
        bool wantVideoCapture = false;
        bool hasVideoCapture = false;
        
        {
          AutoRecursiveLock lock(mLifetimeLock);
          
          wantVideoCapture = state.mLifetimeWantVideoCapture;
          hasVideoCapture = state.mLifetimeHasVideoCapture;
        }
        
        {
          AutoRecursiveLock lock(mLock);
          
          if (!wantVideoCapture) {
            if (hasVideoCapture) {
              internalStopVideoCapture(sourceId);
            }
          }
        }
        
        {
          AutoRecursiveLock lock(mLifetimeLock);
          
          mVideoSourceLifetimeStates[sourceId].mLifetimeHasVideoCapture = wantVideoCapture;
        }
      }
      
      {
        AutoRecursiveLock lock(mLifetimeLock);

        mLifetimeInProgress = false;
      }
      
      ZS_LOG_DEBUG(log("media engine lifetime thread completed"))
    }
    
    //-------------------------------------------------------------------------
    void MediaEngine::cancel()
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
    void MediaEngine::setError(WORD errorCode, const char *inReason)
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
    
    //-----------------------------------------------------------------------
    int MediaEngine::internalCreateVoiceChannel()
    {
      {
        AutoRecursiveLock lock(mLock);
        
        int channelId = mVoiceBase->CreateChannel();
        if (channelId < 0) {
          ZS_LOG_ERROR(Detail, log("could not create voice channel") + ZS_PARAM("error", mVoiceBase->LastError()))
          return ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
        }
        
        VoiceChannelInfo info(channelId);
        VoiceChannelLifetimeState lifetimeState(channelId);
        
        mVoiceChannelInfos[channelId] = info;
        mVoiceChannelLifetimeStates[channelId] = lifetimeState;
        
        return channelId;
      }
    }

    //-----------------------------------------------------------------------
    void MediaEngine::internalStartSendVoice(int channelId)
    {
      {
        AutoRecursiveLock lock(mLock);
        
        ZS_LOG_DEBUG(log("start send voice"))
        
        VoiceChannelInfoMap::iterator iter = mVoiceChannelInfos.find(channelId);
        if (iter == mVoiceChannelInfos.end()) {
          ZS_LOG_ERROR(Detail, log("voice channel not found") + ZS_PARAM("channelId", channelId))
          return;
        }
        
        get(mLastError) = internalRegisterVoiceSendTransport(channelId);
        if (mLastError != 0)
          return;
        
        webrtc::CodecInst cinst;
        memset(&cinst, 0, sizeof(webrtc::CodecInst));
        for (int idx = 0; idx < mVoiceCodec->NumOfCodecs(); idx++) {
          get(mLastError) = mVoiceCodec->GetCodec(idx, cinst);
          if (mLastError != 0) {
            ZS_LOG_ERROR(Detail, log("failed to get voice codec") + ZS_PARAM("error", mVoiceBase->LastError()))
            return;
          }
#ifdef ORTC_MEDIA_ENGINE_VOICE_CODEC_ISAC
          if (strcmp(cinst.plname, "ISAC") == 0) {
            strcpy(cinst.plname, "ISAC");
            cinst.pltype = 103;
            cinst.rate = 32000;
            cinst.pacsize = 480; // 30ms
            cinst.plfreq = 16000;
            cinst.channels = 1;
            get(mLastError) = mVoiceCodec->SetSendCodec(channelId, cinst);
            if (mLastError != 0) {
              ZS_LOG_ERROR(Detail, log("failed to set send voice codec") + ZS_PARAM("error", mVoiceBase->LastError()))
              return;
            }
            break;
          }
#elif defined ORTC_MEDIA_ENGINE_VOICE_CODEC_OPUS
          if (strcmp(cinst.plname, "OPUS") == 0) {
            strcpy(cinst.plname, "OPUS");
            cinst.pltype = 110;
            cinst.rate = 20000;
            cinst.pacsize = 320; // 20ms
            cinst.plfreq = 16000;
            cinst.channels = 1;
            get(mLastError) = mVoiceCodec->SetSendCodec(mVoiceChannel, cinst);
            if (mLastError != 0) {
              ZS_LOG_ERROR(Detail, log("failed to set send voice codec") + ZS_PARAM("error", mVoiceBase->LastError()))
              return;
            }
            break;
          }
#endif
        }
        
        get(mLastError) = internalSetVoiceSendTransportParameters(channelId);
        if (mLastError != 0)
          return;
        
        get(mLastError) = mVoiceBase->StartSend(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to start sending voice") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
      }
      
      {
        AutoRecursiveLock lock(mMediaEngineReadyLock);
        mVoiceEngineReady = true;
      }
    }

    //-----------------------------------------------------------------------
    void MediaEngine::internalStartReceiveVoice(int channelId)
    {
      {
        AutoRecursiveLock lock(mLock);
        
        ZS_LOG_DEBUG(log("start receive voice"))
        
        VoiceChannelInfoMap::iterator iter = mVoiceChannelInfos.find(channelId);
        if (iter == mVoiceChannelInfos.end()) {
          ZS_LOG_ERROR(Detail, log("voice channel not found") + ZS_PARAM("channelId", channelId))
          return;
        }
        
        webrtc::EcModes ecMode = getEcMode();
        if (ecMode == webrtc::kEcUnchanged) {
          return;
        }
        get(mLastError) = mVoiceAudioProcessing->SetEcStatus(mVoiceChannelInfos[channelId].mEcEnabled, ecMode);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to set acoustic echo canceller status") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
        if (ecMode == webrtc::kEcAecm && mVoiceChannelInfos[channelId].mEcEnabled) {
          get(mLastError) = mVoiceAudioProcessing->SetAecmMode(webrtc::kAecmSpeakerphone);
          if (mLastError != 0) {
            ZS_LOG_ERROR(Detail, log("failed to set acoustic echo canceller mobile mode") + ZS_PARAM("error", mVoiceBase->LastError()))
            return;
          }
        }
        get(mLastError) = mVoiceAudioProcessing->SetAgcStatus(mVoiceChannelInfos[channelId].mAgcEnabled, webrtc::kAgcAdaptiveDigital);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to set automatic gain control status") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
        get(mLastError) = mVoiceAudioProcessing->SetNsStatus(mVoiceChannelInfos[channelId].mNsEnabled, webrtc::kNsLowSuppression);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to set noise suppression status") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
        
        get(mLastError) = mVoiceVolumeControl->SetInputMute(-1, false);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to set microphone mute") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
#ifdef TARGET_OS_IPHONE
        get(mLastError) = mVoiceHardware->SetLoudspeakerStatus(false);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to set loudspeaker") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
#endif
        
        get(mLastError) = internalSetVoiceReceiveTransportParameters(channelId);
        if (mLastError != 0)
          return;
        
        get(mLastError) = mVoiceBase->StartReceive(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to start receiving voice") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
        get(mLastError) = mVoiceBase->StartPlayout(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to start playout") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
        
        webrtc::CodecInst cfinst;
        memset(&cfinst, 0, sizeof(webrtc::CodecInst));
        for (int idx = 0; idx < mVoiceCodec->NumOfCodecs(); idx++) {
          get(mLastError) = mVoiceCodec->GetCodec(idx, cfinst);
          if (mLastError != 0) {
            ZS_LOG_ERROR(Detail, log("failed to get voice codec") + ZS_PARAM("error", mVoiceBase->LastError()))
            return;
          }
          if (strcmp(cfinst.plname, "VORBIS") == 0) {
            strcpy(cfinst.plname, "VORBIS");
            cfinst.pltype = 109;
            cfinst.rate = 32000;
            cfinst.pacsize = 480; // 30ms
            cfinst.plfreq = 16000;
            cfinst.channels = 1;
            break;
          }
        }

        if (!mVoiceRecordFile.empty()) {
          get(mLastError) = mVoiceFile->StartRecordingCall(mVoiceRecordFile, &cfinst);
          if (mLastError != 0) {
            ZS_LOG_ERROR(Detail, log("failed to start call recording") + ZS_PARAM("error", mVoiceBase->LastError()))
          }
        }
      }
      
      {
        AutoRecursiveLock lock(mMediaEngineReadyLock);
        mVoiceEngineReady = true;
      }
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::internalStopSendVoice(int channelId)
    {
      {
        AutoRecursiveLock lock(mMediaEngineReadyLock);
        mVoiceEngineReady = false;
      }
      
      {
        AutoRecursiveLock lock(mLock);
        
        ZS_LOG_DEBUG(log("stop send voice"))
        
        get(mLastError) = mVoiceBase->StopSend(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to stop sending voice") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
        get(mLastError) = internalDeregisterVoiceSendTransport(channelId);
        if (0 != mLastError)
          return;
        get(mLastError) = mVoiceBase->DeleteChannel(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to delete voice channel") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
        channelId = ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
      }
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::internalStopReceiveVoice(int channelId)
    {
      {
        AutoRecursiveLock lock(mMediaEngineReadyLock);
        mVoiceEngineReady = false;
      }
      
      {
        AutoRecursiveLock lock(mLock);
        
        ZS_LOG_DEBUG(log("stop receive voice"))

        get(mLastError) = mVoiceBase->StopPlayout(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to stop playout") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
        get(mLastError) = mVoiceBase->StopReceive(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to stop receiving voice") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
        if (!mVoiceRecordFile.empty()) {
          get(mLastError) = mVoiceFile->StopRecordingCall();
          if (mLastError != 0) {
            ZS_LOG_ERROR(Detail, log("failed to stop call recording") + ZS_PARAM("error", mVoiceBase->LastError()))
          }
          mVoiceRecordFile.erase();
        }
        if (0 != mLastError)
          return;
        get(mLastError) = mVoiceBase->DeleteChannel(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to delete voice channel") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
        channelId = ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
      }
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::internalCreateVideoSource()
    {
      {
        AutoRecursiveLock lock(mLock);
        
//        ZS_LOG_DEBUG(log("create video source") + ZS_PARAM("camera type", mVideoSourceInfos[captureId].mCameraType == CameraType_Back ? "back" : "front"))
        
        int captureId;
        const unsigned int KMaxDeviceNameLength = 128;
        const unsigned int KMaxUniqueIdLength = 256;
        char deviceName[KMaxDeviceNameLength];
        memset(deviceName, 0, KMaxDeviceNameLength);
        char uniqueId[KMaxUniqueIdLength];
        memset(uniqueId, 0, KMaxUniqueIdLength);
        uint32_t captureIdx;
        /*
        if (mVideoSourceInfos[captureId].mCameraType == CameraType_Back)
        {
          captureIdx = 0;
        }
        else if (mVideoSourceInfos[captureId].mCameraType == CameraType_Front)
        {
          captureIdx = 1;
        }
        else
        {
          ZS_LOG_ERROR(Detail, log("camera type is not set"))
          return;
        }
        */
        captureIdx = 1;
        
        webrtc::VideoCaptureModule::DeviceInfo *devInfo = webrtc::VideoCaptureFactory::CreateDeviceInfo(0);
        if (devInfo == NULL) {
          ZS_LOG_ERROR(Detail, log("failed to create video capture device info"))
          return ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
        }
        
        get(mLastError) = devInfo->GetDeviceName(captureIdx, deviceName,
                                                 KMaxDeviceNameLength, uniqueId,
                                                 KMaxUniqueIdLength);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to get video device name"))
          return ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
        }
        
        strcpy(mVideoSourceInfos[captureId].mDeviceUniqueId, uniqueId);
        
        mVcpm = webrtc::VideoCaptureFactory::Create(1, uniqueId);
        if (mVcpm == NULL) {
          ZS_LOG_ERROR(Detail, log("failed to create video capture module"))
          return ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
        }
        
        get(mLastError) = mVideoCapture->AllocateCaptureDevice(*mVcpm, captureId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to allocate video capture device") + ZS_PARAM("error", mVideoBase->LastError()))
          return ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
        }
        mVcpm->AddRef();
        delete devInfo;
        
        VideoSourceInfo info(captureId);
        VideoSourceLifetimeState lifetimeState(captureId);
        
        mVideoSourceInfos[captureId] = info;
        mVideoSourceLifetimeStates[captureId] = lifetimeState;
        
        mVideoSourceInfos[captureId].mCameraType = CameraType_Front;
        
        return captureId;
      }
    }
   
    //-----------------------------------------------------------------------
    int MediaEngine::internalCreateVideoChannel()
    {
      {
        AutoRecursiveLock lock(mLock);
        
        int channelId;
        get(mLastError) = mVideoBase->CreateChannel(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("could not create video channel") + ZS_PARAM("error", mVoiceBase->LastError()))
          return ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
        }
        
        VideoChannelInfo info(channelId);
        VideoChannelLifetimeState lifetimeState(channelId);
        
        mVideoChannelInfos[channelId] = info;
        mVideoChannelLifetimeStates[channelId] = lifetimeState;
        
        return channelId;
      }
    }

    //-----------------------------------------------------------------------
    void MediaEngine::internalStartVideoCapture(int captureId)
    {
      {
        AutoRecursiveLock lock(mLock);
        
        ZS_LOG_DEBUG(log("start video capture") + ZS_PARAM("camera type", mVideoSourceInfos[captureId].mCameraType == CameraType_Back ? "back" : "front"))
        
        VideoSourceInfoMap::iterator source_iter = mVideoSourceInfos.find(captureId);
        if (source_iter == mVideoSourceInfos.end()) {
          ZS_LOG_ERROR(Detail, log("video source not found") + ZS_PARAM("captureId", captureId))
          return;
        }

#if defined(TARGET_OS_IPHONE) || defined(__QNX__)
        void *captureView = mVideoSourceInfos[captureId].mRenderView;
#else
        void *captureView = NULL;
#endif
#ifndef __QNX__
        if (captureView == NULL) {
          ZS_LOG_ERROR(Detail, log("capture view is not set"))
          return;
        }
#endif
        
        get(mLastError) = mVideoCapture->RegisterObserver(captureId, *this);
        if (mLastError < 0) {
          ZS_LOG_ERROR(Detail, log("failed to register video capture observer") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        
#ifdef TARGET_OS_IPHONE
        get(mLastError) = mVideoCapture->SetDefaultCapturedFrameOrientation(captureId, mDefaultVideoOrientation);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to set default orientation on video capture device") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        
        setVideoCaptureRotation(captureId);
        
        webrtc::RotateCapturedFrame orientation;
        get(mLastError) = mVideoCapture->GetOrientation(mVideoSourceInfos[captureId].mDeviceUniqueId, orientation);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to get orientation from video capture device") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
#else
        webrtc::RotateCapturedFrame orientation = webrtc::RotateCapturedFrame_0;
#endif
        
        int width = 0, height = 0, maxFramerate = 0, maxBitrate = 0;
        get(mLastError) = getVideoCaptureParameters(mVideoSourceInfos[captureId].mCameraType,
                                                    orientation, width, height, maxFramerate, maxBitrate);
        if (mLastError != 0)
          return;
        
        webrtc::CaptureCapability capability;
        capability.width = width;
        capability.height = height;
        capability.maxFPS = maxFramerate;
        capability.rawType = webrtc::kVideoI420;
        capability.faceDetection = mVideoSourceInfos[captureId].mFaceDetection;
        get(mLastError) = mVideoCapture->StartCapture(captureId, capability);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to start capturing") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        
#ifndef __QNX__
        get(mLastError) = mVideoRender->AddRenderer(captureId, captureView, 0, 0.0F, 0.0F, 1.0F,
                                           1.0F);
        if (0 != mLastError) {
          ZS_LOG_ERROR(Detail, log("failed to add renderer for video capture") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        
        get(mLastError) = mVideoRender->StartRender(captureId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to start rendering video capture") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
#endif
      }
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::internalStopVideoCapture(int captureId)
    {
      {
        AutoRecursiveLock lock(mLock);
        
        ZS_LOG_DEBUG(log("stop video capture"))
        
#ifndef __QNX__
        get(mLastError) = mVideoRender->StopRender(captureId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to stop rendering video capture") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        get(mLastError) = mVideoRender->RemoveRenderer(captureId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to remove renderer for video capture") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
#endif
        get(mLastError) = mVideoCapture->StopCapture(captureId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to stop video capturing") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        get(mLastError) = mVideoCapture->ReleaseCaptureDevice(captureId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to release video capture device") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        
        if (mVcpm != NULL)
          mVcpm->Release();
        
        mVcpm = NULL;
      }
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::internalStartSendVideoChannel(int channelId, int captureId)
    {
      {
        AutoRecursiveLock lock(mLock);
        
        ZS_LOG_DEBUG(log("start send video channel"))
        
        VideoChannelInfoMap::iterator channel_iter = mVideoChannelInfos.find(channelId);
        if (channel_iter == mVideoChannelInfos.end()) {
          ZS_LOG_ERROR(Detail, log("video channel not found") + ZS_PARAM("channelId", channelId))
          return;
        }
        
        VideoSourceInfoMap::iterator source_iter = mVideoSourceInfos.find(captureId);
        if (source_iter == mVideoSourceInfos.end()) {
          ZS_LOG_ERROR(Detail, log("video source not found") + ZS_PARAM("captureId", captureId))
          return;
        }
        
        get(mLastError) = internalRegisterVideoSendTransport(channelId);
        if (0 != mLastError)
          return;
        
        get(mLastError) = mVideoNetwork->SetMTU(channelId, mMtu);
        if (0 != mLastError) {
          ZS_LOG_ERROR(Detail, log("failed to set MTU for video channel") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        
        get(mLastError) = mVideoCapture->ConnectCaptureDevice(captureId, channelId);
        if (0 != mLastError) {
          ZS_LOG_ERROR(Detail, log("failed to connect capture device to video channel") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        
        get(mLastError) = mVideoRtpRtcp->SetRTCPStatus(channelId, webrtc::kRtcpCompound_RFC4585);
        if (0 != mLastError) {
          ZS_LOG_ERROR(Detail, log("failed to set video RTCP status") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        
        get(mLastError) = mVideoRtpRtcp->SetKeyFrameRequestMethod(channelId,
                                                         webrtc::kViEKeyFrameRequestPliRtcp);
        if (0 != mLastError) {
          ZS_LOG_ERROR(Detail, log("failed to set key frame request method") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        
        get(mLastError) = mVideoRtpRtcp->SetTMMBRStatus(channelId, true);
        if (0 != mLastError) {
          ZS_LOG_ERROR(Detail, log("failed to set temporary max media bit rate status") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        
        webrtc::VideoCodec videoCodec;
        memset(&videoCodec, 0, sizeof(VideoCodec));
        for (int idx = 0; idx < mVideoCodec->NumberOfCodecs(); idx++) {
          get(mLastError) = mVideoCodec->GetCodec(idx, videoCodec);
          if (mLastError != 0) {
            ZS_LOG_ERROR(Detail, log("failed to get video codec") + ZS_PARAM("error", mVideoBase->LastError()))
            return;
          }
          if (videoCodec.codecType == webrtc::kVideoCodecVP8) {
            get(mLastError) = mVideoCodec->SetSendCodec(channelId, videoCodec);
            if (mLastError != 0) {
              ZS_LOG_ERROR(Detail, log("failed to set send video codec") + ZS_PARAM("error", mVideoBase->LastError()))
              return;
            }
            break;
          }
        }
        
        get(mLastError) = setVideoCodecParameters(channelId, captureId);
        if (mLastError != 0) {
          return;
        }
        
        get(mLastError) = internalSetVideoSendTransportParameters(channelId);
        if (mLastError != 0)
          return;
        
        get(mLastError) = mVideoBase->StartSend(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to start sending video") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
      }
      
      {
        AutoRecursiveLock lock(mMediaEngineReadyLock);
        mVideoEngineReady = true;
      }
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::internalStartReceiveVideoChannel(int channelId)
    {
      {
        AutoRecursiveLock lock(mLock);
        
        ZS_LOG_DEBUG(log("start receive video channel"))
        
        VideoChannelInfoMap::iterator iter = mVideoChannelInfos.find(channelId);
        if (iter == mVideoChannelInfos.end()) {
          ZS_LOG_ERROR(Detail, log("video channel not found") + ZS_PARAM("channelId", channelId))
          return;
        }

#if defined(TARGET_OS_IPHONE) || defined(__QNX__)
        void *channelView = mVideoChannelInfos[channelId].mRenderView;
#else
        void *channelView = NULL;
#endif
        if (channelView == NULL) {
          ZS_LOG_ERROR(Detail, log("channel view is not set"))
          return;
        }
        
#ifdef TARGET_OS_IPHONE
        OutputAudioRoute route;
        get(mLastError) = mVoiceHardware->GetOutputAudioRoute(route);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to get output audio route") + ZS_PARAM("error", mVoiceBase->LastError()))
          return;
        }
        if (route != webrtc::kOutputAudioRouteHeadphone)
        {
          get(mLastError) = mVoiceHardware->SetLoudspeakerStatus(true);
          if (mLastError != 0) {
            ZS_LOG_ERROR(Detail, log("failed to set loudspeaker") + ZS_PARAM("error", mVoiceBase->LastError()))
            return;
          }
        }
#endif
        
        get(mLastError) = mVideoRender->AddRenderer(channelId, channelView, 0, 0.0F, 0.0F, 1.0F,
                                           1.0F);
        if (0 != mLastError) {
          ZS_LOG_ERROR(Detail, log("failed to add renderer for video channel") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        
        get(mLastError) = internalSetVideoReceiveTransportParameters(channelId);
        if (mLastError != 0)
          return;
        
        get(mLastError) = mVideoBase->StartReceive(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to start receiving video") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        get(mLastError) = mVideoRender->StartRender(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to start rendering video channel") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
      }
      
      {
        AutoRecursiveLock lock(mMediaEngineReadyLock);
        mVideoEngineReady = true;
      }
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::internalStopSendVideoChannel(int channelId)
    {
      {
        AutoRecursiveLock lock(mMediaEngineReadyLock);
        mVideoEngineReady = false;
      }
      
      {
        AutoRecursiveLock lock(mLock);
        
        ZS_LOG_DEBUG(log("stop send video channel"))

        get(mLastError) = mVideoBase->StopSend(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to stop sending video") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        get(mLastError) = mVideoCapture->DisconnectCaptureDevice(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to disconnect capture device from video channel") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        get(mLastError) = internalDeregisterVideoSendTransport(channelId);
        if (0 != mLastError)
          return;
        get(mLastError) = mVideoBase->DeleteChannel(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to delete video channel") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        
        channelId = ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
      }
    }
    
    //-----------------------------------------------------------------------
    void MediaEngine::internalStopReceiveVideoChannel(int channelId)
    {
      {
        AutoRecursiveLock lock(mMediaEngineReadyLock);
        mVideoEngineReady = false;
      }
      
      {
        AutoRecursiveLock lock(mLock);
        
        ZS_LOG_DEBUG(log("stop receive video channel"))
        
        get(mLastError) = mVideoRender->StopRender(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to stop rendering video channel") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        get(mLastError) = mVideoRender->RemoveRenderer(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to remove renderer for video channel") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        get(mLastError) = mVideoBase->StopReceive(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to stop receiving video") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        get(mLastError) = mVideoBase->DeleteChannel(channelId);
        if (mLastError != 0) {
          ZS_LOG_ERROR(Detail, log("failed to delete video channel") + ZS_PARAM("error", mVideoBase->LastError()))
          return;
        }
        
        channelId = ORTC_MEDIA_ENGINE_INVALID_CHANNEL;
      }
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::internalRegisterVoiceSendTransport(int channelId)
    {
      if (NULL != mVoiceChannelInfos[channelId].mTransport) {
        get(mLastError) = mVoiceNetwork->RegisterExternalTransport(channelId, *mVoiceChannelInfos[channelId].mTransport);
        if (0 != mLastError) {
          ZS_LOG_ERROR(Detail, log("failed to register voice external transport") + ZS_PARAM("error", mVoiceBase->LastError()))
          return mLastError;
        }
      } else {
        ZS_LOG_ERROR(Detail, log("external voice transport is not set"))
        return -1;
      }

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::internalDeregisterVoiceSendTransport(int channelId)
    {
      get(mLastError) = mVoiceNetwork->DeRegisterExternalTransport(channelId);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to deregister voice external transport") + ZS_PARAM("error", mVoiceBase->LastError()))
        return mLastError;
      }

      return 0;
    }

    //-----------------------------------------------------------------------
    int MediaEngine::internalSetVoiceSendTransportParameters(int channelId)
    {
      // No transport parameters for external transport.
      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::internalSetVoiceReceiveTransportParameters(int channelId)
    {
      // No transport parameters for external transport.
      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::internalRegisterVideoSendTransport(int channelId)
    {
      if (NULL != mVideoChannelInfos[channelId].mTransport) {
        get(mLastError) = mVideoNetwork->RegisterSendTransport(channelId, *mVideoChannelInfos[channelId].mTransport);
        if (0 != mLastError) {
          ZS_LOG_ERROR(Detail, log("failed to register video external transport") + ZS_PARAM("error", mVideoBase->LastError()))
          return mLastError;
        }
      } else {
        ZS_LOG_ERROR(Detail, log("external video transport is not set"))
        return -1;
      }

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::internalDeregisterVideoSendTransport(int channelId)
    {
      get(mLastError) = mVideoNetwork->DeregisterSendTransport(channelId);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to deregister video external transport") + ZS_PARAM("error", mVideoBase->LastError()))
        return mLastError;
      }

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::internalSetVideoSendTransportParameters(int channelId)
    {
      // No transport parameters for external transport.
      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::internalSetVideoReceiveTransportParameters(int channelId)
    {
      // No transport parameters for external transport.
      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::getVideoCaptureParameters(CameraTypes cameraType, webrtc::RotateCapturedFrame orientation,
                                               int& width, int& height, int& maxFramerate, int& maxBitrate)
    {
#ifdef TARGET_OS_IPHONE
      String iPadString("iPad");
      String iPad2String("iPad2");
      String iPadMiniString("iPad2,5");
      String iPad3String("iPad3");
      String iPad4String("iPad3,4");
      String iPhoneString("iPhone");
      String iPhone4SString("iPhone4,1");
      String iPhone5String("iPhone5");
      String iPodString("iPod");
      String iPod4String("iPod4,1");
      String iPod5String("iPod5,1");
      if (cameraType == CameraType_Back) {
        if (orientation == webrtc::RotateCapturedFrame_0 || orientation == webrtc::RotateCapturedFrame_180) {
          if (mMachineName.compare(0, iPod5String.size(), iPod5String) >= 0) {
            width = 960;
            height = 540;
            maxFramerate = 15;
            maxBitrate = 500;
          } else if (mMachineName.compare(0, iPod4String.size(), iPod4String) >= 0) {
            width = 640;
            height = 360;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPodString.size(), iPodString) >= 0) {
            width = 160;
            height = 90;
            maxFramerate = 5;
            maxBitrate = 100;
          } else if (mMachineName.compare(0, iPhone5String.size(), iPhone5String) >= 0) {
            width = 960;
            height = 540;
            maxFramerate = 15;
            maxBitrate = 500;
          } else if (mMachineName.compare(0, iPhone4SString.size(), iPhone4SString) >= 0) {
            width = 640;
            height = 360;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPhoneString.size(), iPhoneString) >= 0) {
            width = 160;
            height = 90;
            maxFramerate = 5;
            maxBitrate = 100;
          } else if (mMachineName.compare(0, iPadMiniString.size(), iPadMiniString) >= 0) {
            width = 960;
            height = 540;
            maxFramerate = 15;
            maxBitrate = 500;
          } else if (mMachineName.compare(0, iPad2String.size(), iPad2String) >= 0) {
            width = 640;
            height = 360;
            maxFramerate = 15;
            maxBitrate = 400;
          } else {
            ZS_LOG_ERROR(Detail, log("machine name is not supported"))
            return -1;
          }
        } else if (orientation == webrtc::RotateCapturedFrame_90 || orientation == webrtc::RotateCapturedFrame_270) {
          if (mMachineName.compare(0, iPod5String.size(), iPod5String) >= 0) {
            width = 540;
            height = 960;
            maxFramerate = 15;
            maxBitrate = 500;
          } else if (mMachineName.compare(0, iPod4String.size(), iPod4String) >= 0) {
            width = 360;
            height = 640;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPodString.size(), iPodString) >= 0) {
            width = 90;
            height = 160;
            maxFramerate = 5;
            maxBitrate = 100;
          } else if (mMachineName.compare(0, iPhone5String.size(), iPhone5String) >= 0) {
            width = 540;
            height = 960;
            maxFramerate = 15;
            maxBitrate = 500;
          } else if (mMachineName.compare(0, iPhone4SString.size(), iPhone4SString) >= 0) {
            width = 360;
            height = 640;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPhoneString.size(), iPhoneString) >= 0) {
            width = 90;
            height = 160;
            maxFramerate = 5;
            maxBitrate = 100;
          } else if (mMachineName.compare(0, iPadMiniString.size(), iPadMiniString) >= 0) {
            width = 540;
            height = 960;
            maxFramerate = 15;
            maxBitrate = 500;
          } else if (mMachineName.compare(0, iPad2String.size(), iPad2String) >= 0) {
            width = 360;
            height = 640;
            maxFramerate = 15;
            maxBitrate = 400;
          } else {
            ZS_LOG_ERROR(Detail, log("machine name is not supported"))
            return -1;
          }
        }
      } else if (cameraType == CameraType_Front) {
        if (orientation == webrtc::RotateCapturedFrame_0 || orientation == webrtc::RotateCapturedFrame_180) {
          if (mMachineName.compare(0, iPod5String.size(), iPod5String) >= 0) {
            width = 640;
            height = 360;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPod4String.size(), iPod4String) >= 0) {
            width = 640;
            height = 480;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPodString.size(), iPodString) >= 0) {
            width = 160;
            height = 120;
            maxFramerate = 5;
            maxBitrate = 100;
          } else if (mMachineName.compare(0, iPhone5String.size(), iPhone5String) >= 0) {
            width = 640;
            height = 360;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPhone4SString.size(), iPhone4SString) >= 0) {
            width = 640;
            height = 480;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPhoneString.size(), iPhoneString) >= 0) {
            width = 160;
            height = 120;
            maxFramerate = 5;
            maxBitrate = 100;
          } else if (mMachineName.compare(0, iPad4String.size(), iPad4String) >= 0) {
            width = 640;
            height = 360;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPad3String.size(), iPad3String) >= 0) {
            width = 640;
            height = 480;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPadMiniString.size(), iPadMiniString) >= 0) {
            width = 640;
            height = 360;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPadString.size(), iPadString) >= 0) {
            width = 640;
            height = 480;
            maxFramerate = 15;
            maxBitrate = 400;
          } else {
            ZS_LOG_ERROR(Detail, log("machine name is not supported"))
            return -1;
          }
        } else if (orientation == webrtc::RotateCapturedFrame_90 || orientation == webrtc::RotateCapturedFrame_270) {
          if (mMachineName.compare(0, iPod5String.size(), iPod5String) >= 0) {
            width = 360;
            height = 640;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPod4String.size(), iPod4String) >= 0) {
            width = 480;
            height = 640;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPodString.size(), iPodString) >= 0) {
            width = 120;
            height = 160;
            maxFramerate = 5;
            maxBitrate = 100;
          } else if (mMachineName.compare(0, iPhone5String.size(), iPhone5String) >= 0) {
            width = 360;
            height = 640;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPhone4SString.size(), iPhone4SString) >= 0) {
            width = 480;
            height = 640;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPhoneString.size(), iPhoneString) >= 0) {
            width = 120;
            height = 160;
            maxFramerate = 5;
            maxBitrate = 100;
          } else if (mMachineName.compare(0, iPad4String.size(), iPad4String) >= 0) {
            width = 360;
            height = 640;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPad3String.size(), iPad3String) >= 0) {
            width = 480;
            height = 640;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPadMiniString.size(), iPadMiniString) >= 0) {
            width = 360;
            height = 640;
            maxFramerate = 15;
            maxBitrate = 400;
          } else if (mMachineName.compare(0, iPadString.size(), iPadString) >= 0) {
            width = 480;
            height = 640;
            maxFramerate = 15;
            maxBitrate = 400;
          } else {
            ZS_LOG_ERROR(Detail, log("machine name is not supported"))
            return -1;
          }
        }
      } else {
        ZS_LOG_ERROR(Detail, log("camera type is not set"))
        return -1;
      }
#else
      width = 180;
      height = 320;
      maxFramerate = 15;
      maxBitrate = 250;
#endif
      return 0;
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::setVideoCodecParameters(int channelId, int captureId)
    {
#ifdef TARGET_OS_IPHONE
      webrtc::RotateCapturedFrame orientation;
      get(mLastError) = mVideoCapture->GetOrientation(mVideoSourceInfos[captureId].mDeviceUniqueId, orientation);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to get orientation from video capture device") + ZS_PARAM("error", mVideoBase->LastError()))
        return mLastError;
      }
#else
      webrtc::RotateCapturedFrame orientation = webrtc::RotateCapturedFrame_0;
#endif
      
      int width = 0, height = 0, maxFramerate = 0, maxBitrate = 0;
      get(mLastError) = getVideoCaptureParameters(mVideoSourceInfos[captureId].mCameraType, orientation,
                                                  width, height, maxFramerate, maxBitrate);
      if (mLastError != 0)
        return mLastError;
      
      webrtc::VideoCodec videoCodec;
      memset(&videoCodec, 0, sizeof(VideoCodec));
      get(mLastError) = mVideoCodec->GetSendCodec(channelId, videoCodec);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to get video codec") + ZS_PARAM("error", mVideoBase->LastError()))
        return mLastError;
      }
      videoCodec.width = width;
      videoCodec.height = height;
      videoCodec.maxFramerate = maxFramerate;
      videoCodec.maxBitrate = maxBitrate;
      get(mLastError) = mVideoCodec->SetSendCodec(channelId, videoCodec);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to set send video codec") + ZS_PARAM("error", mVideoBase->LastError()))
        return mLastError;
      }
      
      ZS_LOG_DEBUG(log("video codec size ") + ZS_PARAM("width", width) + ZS_PARAM("height", height))
      
      return 0;
    }

    //-----------------------------------------------------------------------
    int MediaEngine::setVideoCaptureRotation(int captureId)
    {
      webrtc::RotateCapturedFrame orientation;
      get(mLastError) = mVideoCapture->GetOrientation(mVideoSourceInfos[captureId].mDeviceUniqueId, orientation);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to get orientation from video capture device") + ZS_PARAM("error", mVideoBase->LastError()))
        return mLastError;
      }
      get(mLastError) = mVideoCapture->SetRotateCapturedFrames(captureId, orientation);
      if (mLastError != 0) {
        ZS_LOG_ERROR(Detail, log("failed to set rotation for video capture device") + ZS_PARAM("error", mVideoBase->LastError()))
        return mLastError;
      }
      
      const char *rotationString = NULL;
      switch (orientation) {
        case webrtc::RotateCapturedFrame_0:
          rotationString = "0 degrees";
          break;
        case webrtc::RotateCapturedFrame_90:
          rotationString = "90 degrees";
          break;
        case webrtc::RotateCapturedFrame_180:
          rotationString = "180 degrees";
          break;
        case webrtc::RotateCapturedFrame_270:
          rotationString = "270 degrees";
          break;
        default:
          break;
      }
      
      if (rotationString) {
        ZS_LOG_DEBUG(log("video capture rotation set") + ZS_PARAM("rotation", rotationString))
      }
      
      return 0;
    }
    
    //-----------------------------------------------------------------------
    webrtc::EcModes MediaEngine::getEcMode()
    {
#ifdef TARGET_OS_IPHONE
      String iPadString("iPad");
      String iPad2String("iPad2");
      String iPad3String("iPad3");
      String iPhoneString("iPhone");
      String iPhone5String("iPhone5");
      String iPhone4SString("iPhone4,1");
      String iPodString("iPod");
      String iPod4String("iPod4,1");
      
      if (mMachineName.compare(0, iPod4String.size(), iPod4String) >= 0) {
        return webrtc::kEcAec;
      } else if (mMachineName.compare(0, iPodString.size(), iPodString) >= 0) {
        return webrtc::kEcAecm;
      } else if (mMachineName.compare(0, iPhone5String.size(), iPhone5String) >= 0) {
        return webrtc::kEcAec;
      } else if (mMachineName.compare(0, iPhone4SString.size(), iPhone4SString) >= 0) {
        return webrtc::kEcAec;
      } else if (mMachineName.compare(0, iPhoneString.size(), iPhoneString) >= 0) {
        return webrtc::kEcAecm;
      } else if (mMachineName.compare(0, iPad3String.size(), iPad3String) >= 0) {
        return webrtc::kEcAec;
      } else if (mMachineName.compare(0, iPad2String.size(), iPad2String) >= 0) {
        return webrtc::kEcAec;
      } else if (mMachineName.compare(0, iPadString.size(), iPadString) >= 0) {
        return webrtc::kEcAecm;
      } else {
        ZS_LOG_ERROR(Detail, log("machine name is not supported"))
        return webrtc::kEcUnchanged;
      }
#elif defined(__QNX__)
      return webrtc::kEcAec;
#else
      return webrtc::kEcUnchanged;
#endif
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::VoiceSourceInfo
    #pragma mark
    
    //-----------------------------------------------------------------------
    MediaEngine::VoiceSourceInfo::VoiceSourceInfo(int sourceId) :
      mSourceId(sourceId)
    { }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::VoiceChannelInfo
    #pragma mark
    
    //-----------------------------------------------------------------------
    MediaEngine::VoiceChannelInfo::VoiceChannelInfo(int channelId) :
      mChannelId(channelId),
      mEcEnabled(false),
      mAgcEnabled(false),
      mNsEnabled(false),
      mRedirectTransport(new RedirectTransport("voice")),
      mTransport(mRedirectTransport)
    { }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::VideoSourceInfo
    #pragma mark
    
    //-----------------------------------------------------------------------
    MediaEngine::VideoSourceInfo::VideoSourceInfo(int sourceId) :
      mSourceId(sourceId),
      mCameraType(CameraType_None),
      mRenderView(NULL),
      mFaceDetection(false)
    { }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::VideoChannelInfo
    #pragma mark
    
    //-----------------------------------------------------------------------
    MediaEngine::VideoChannelInfo::VideoChannelInfo(int channelId) :
      mChannelId(channelId),
      mRenderView(NULL),
      mRedirectTransport(new RedirectTransport("video")),
      mTransport(mRedirectTransport)
    { }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::VoiceSourceLifetimeState
    #pragma mark
    
    //-----------------------------------------------------------------------
    MediaEngine::VoiceSourceLifetimeState::VoiceSourceLifetimeState(int sourceId) :
      mSourceId(sourceId)
    { }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::VoiceChannelLifetimeState
    #pragma mark
    
    //-----------------------------------------------------------------------
    MediaEngine::VoiceChannelLifetimeState::VoiceChannelLifetimeState(int channelId) :
      mChannelId(channelId),
      mLifetimeWantSendAudio(false),
      mLifetimeWantReceiveAudio(false),
      mLifetimeHasSendAudio(false),
      mLifetimeHasReceiveAudio(false)
    { }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::VideoSourceLifetimeState
    #pragma mark
    
    //-----------------------------------------------------------------------
    MediaEngine::VideoSourceLifetimeState::VideoSourceLifetimeState(int sourceId) :
      mSourceId(sourceId),
      mLifetimeWantVideoCapture(false),
      mLifetimeWantRecordVideoCapture(false),
      mLifetimeWantCameraType(CameraType_None),
      mLifetimeContinuousVideoCapture(false),
      mLifetimeSaveVideoToLibrary(false),
      mLifetimeHasVideoCapture(false)
    { }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::VideoChannelLifetimeState
    #pragma mark
    
    //-----------------------------------------------------------------------
    MediaEngine::VideoChannelLifetimeState::VideoChannelLifetimeState(int channelId) :
      mChannelId(channelId),
      mLifetimeWantSendVideoChannel(false),
      mLifetimeWantReceiveVideoChannel(false),
      mLifetimeHasSendVideoChannel(false),
      mLifetimeHasReceiveVideoChannel(false)
    { }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::RedirectTransport
    #pragma mark
    
    //-----------------------------------------------------------------------
    MediaEngine::RedirectTransport::RedirectTransport(const char *transportType) :
      mID(zsLib::createPUID()),
      mTransportType(transportType),
      mTransport(0)
    { }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::RedirectTransport => webrtc::Transport
    #pragma mark
    
    //-----------------------------------------------------------------------
    int MediaEngine::RedirectTransport::SendPacket(int channel, const void *data, int len)
    {
      Transport *transport = NULL;
      {
        AutoRecursiveLock lock(mLock);
        transport = mTransport;
      }
      if (!transport) {
        ZS_LOG_WARNING(Debug, log("RTP packet cannot be sent as no transport is not registered") + ZS_PARAM("channel", channel) + ZS_PARAM("length", len))
        return 0;
      }
      
      return transport->SendPacket(channel, data, len);
    }
    
    //-----------------------------------------------------------------------
    int MediaEngine::RedirectTransport::SendRTCPPacket(int channel, const void *data, int len)
    {
      Transport *transport = NULL;
      {
        AutoRecursiveLock lock(mLock);
        transport = mTransport;
      }
      if (!transport) {
        ZS_LOG_WARNING(Debug, log("RTCP packet cannot be sent as no transport is not registered") + ZS_PARAM("channel", channel) + ZS_PARAM("length", len))
        return 0;
      }
      
      return transport->SendRTCPPacket(channel, data, len);
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::RedirectTransport => friend MediaEngine
    #pragma mark
    
    //-----------------------------------------------------------------------
    void MediaEngine::RedirectTransport::redirect(Transport *transport)
    {
      AutoRecursiveLock lock(mLock);
      mTransport = transport;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine::RedirectTransport => (internal)
    #pragma mark
    
    //-----------------------------------------------------------------------
    Log::Params MediaEngine::RedirectTransport::log(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::MediaEngine::RedirectTransport");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      UseServicesHelper::debugAppend(objectEl, "transport type", mTransportType);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineFactory
    #pragma mark
    
    //-------------------------------------------------------------------------
    IMediaEngineFactory &IMediaEngineFactory::singleton()
    {
      return MediaEngineFactory::singleton();
    }

    //-------------------------------------------------------------------------
    MediaEnginePtr IMediaEngineFactory::create(IMediaEngineDelegatePtr delegate)
    {
      if (this) {}
      return MediaEngine::create(delegate);
    }
  }
}