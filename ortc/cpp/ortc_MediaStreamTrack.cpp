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

#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_MediaEngine.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/Log.h>
#include <zsLib/XML.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  typedef openpeer::services::IHelper OPIHelper;
  
  namespace internal
  {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IReceiveMediaTransportForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    ReceiveMediaTransportPtr IReceiveMediaTransportForMediaManager::create()
    {
      ReceiveMediaTransportPtr pThis(new ReceiveMediaTransport());
      pThis->mThisWeak = pThis;
      return pThis;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ISendMediaTransportForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    SendMediaTransportPtr ISendMediaTransportForMediaManager::create()
    {
      SendMediaTransportPtr pThis(new SendMediaTransport());
      pThis->mThisWeak = pThis;
      return pThis;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    MediaTransport::MediaTransport()
    {
      
    }
    
    //-------------------------------------------------------------------------
    MediaTransport::~MediaTransport()
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaTransport => IMediaTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    int MediaTransport::getTransportStatistics(IMediaTransport::RtpRtcpStatistics &stat)
    {
      return 0;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ReceiveMediaTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    ReceiveMediaTransport::ReceiveMediaTransport()
    {
      
    }
    
    //-------------------------------------------------------------------------
    ReceiveMediaTransport::~ReceiveMediaTransport()
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ReceiveMediaTransport => IMediaTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    int ReceiveMediaTransport::getTransportStatistics(IMediaTransport::RtpRtcpStatistics &stat)
    {
      return 0;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ReceiveMediaTransport => IReceiveMediaTransportForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    int ReceiveMediaTransport::receivedRTPPacket(const void *data, unsigned int length)
    {
      return 0;
    }
    
    //-------------------------------------------------------------------------
    int ReceiveMediaTransport::receivedRTCPPacket(const void *data, unsigned int length)
    {
      return 0;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark SendMediaTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    SendMediaTransport::SendMediaTransport()
    {
      
    }
    
    //-------------------------------------------------------------------------
    SendMediaTransport::~SendMediaTransport()
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark SendMediaTransport => IMediaTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    int SendMediaTransport::getTransportStatistics(IMediaTransport::RtpRtcpStatistics &stat)
    {
      return 0;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark SendMediaTransport => ISendMediaTransportForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    int SendMediaTransport::registerExternalTransport(Transport &transport)
    {
      return 0;
    }
    
    int SendMediaTransport::deregisterExternalTransport()
    {
      return 0;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ILocalAudioStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    LocalAudioStreamTrackPtr ILocalAudioStreamTrackForMediaManager::create(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate)
    {
      LocalAudioStreamTrackPtr pThis(new LocalAudioStreamTrack(queue, delegate));
      pThis->mThisWeak = pThis;
      return pThis;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteReceiveAudioStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    RemoteReceiveAudioStreamTrackPtr IRemoteReceiveAudioStreamTrackForMediaManager::create(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate)
    {
      RemoteReceiveAudioStreamTrackPtr pThis(new RemoteReceiveAudioStreamTrack(queue, delegate));
      pThis->mThisWeak = pThis;
      return pThis;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteSendAudioStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    RemoteSendAudioStreamTrackPtr IRemoteSendAudioStreamTrackForMediaManager::create(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate)
    {
      RemoteSendAudioStreamTrackPtr pThis(new RemoteSendAudioStreamTrack(queue, delegate));
      pThis->mThisWeak = pThis;
      return pThis;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ILocalVideoStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    LocalVideoStreamTrackPtr ILocalVideoStreamTrackForMediaManager::create(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate)
    {
      LocalVideoStreamTrackPtr pThis(new LocalVideoStreamTrack(queue, delegate));
      pThis->mThisWeak = pThis;
      return pThis;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteReceiveVideoStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    RemoteReceiveVideoStreamTrackPtr IRemoteReceiveVideoStreamTrackForMediaManager::create(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate)
    {
      RemoteReceiveVideoStreamTrackPtr pThis(new RemoteReceiveVideoStreamTrack(queue, delegate));
      pThis->mThisWeak = pThis;
      return pThis;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteSendVideoStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    RemoteSendVideoStreamTrackPtr IRemoteSendVideoStreamTrackForMediaManager::create(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate)
    {
      RemoteSendVideoStreamTrackPtr pThis(new RemoteSendVideoStreamTrack(queue, delegate));
      pThis->mThisWeak = pThis;
      return pThis;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack
    #pragma mark
    
    //-----------------------------------------------------------------------
    MediaStreamTrack::MediaStreamTrack(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate) :
      MessageQueueAssociator(queue),
      mID(zsLib::createPUID()),
      mEnabled(false),
      mMuted(false),
      mReadonly(false),
      mRemote(false),
      mReadyState(IMediaStreamTrack::MediaStreamTrackState_New),
      mSSRC(0),
      mChannel(-1)
    {
    }
    
    //-----------------------------------------------------------------------
    MediaStreamTrack::~MediaStreamTrack()
    {
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IMediaStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    String MediaStreamTrack::kind()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String MediaStreamTrack::id()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String MediaStreamTrack::label()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    bool MediaStreamTrack::enabled()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool MediaStreamTrack::muted()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool MediaStreamTrack::readonly()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool MediaStreamTrack::remote()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrack::MediaStreamTrackStates MediaStreamTrack::readyState()
    {
      return IMediaStreamTrack::MediaStreamTrackState_New;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrackPtr MediaStreamTrack::clone()
    {
      return IMediaStreamTrackPtr();
    }
    
    //-------------------------------------------------------------------------
    void MediaStreamTrack::stop()
    {
      
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => IWakeDelegate
    #pragma mark
    
    //-------------------------------------------------------------------------
    void MediaStreamTrack::onWake()
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
    #pragma mark MediaStreamTrack => (internal)
    #pragma mark
    
    //-------------------------------------------------------------------------
    Log::Params MediaStreamTrack::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::MediaStreamTrack");
      OPIHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }
    
    //-------------------------------------------------------------------------
    Log::Params MediaStreamTrack::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }
    
    //-------------------------------------------------------------------------
    ElementPtr MediaStreamTrack::toDebug() const
    {
      ElementPtr resultEl = Element::create("MediaStreamTrack");
      
      OPIHelper::debugAppend(resultEl, "id", mID);
      
      OPIHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);
      OPIHelper::debugAppend(resultEl, "graceful shutdown", mShutdown);
      
      OPIHelper::debugAppend(resultEl, "error", mLastError);
      OPIHelper::debugAppend(resultEl, "error reason", mLastErrorReason);
      
      return resultEl;
    }
    
    //-------------------------------------------------------------------------
    bool MediaStreamTrack::isShuttingDown() const
    {
      return (bool)mGracefulShutdownReference;
    }
    
    //-------------------------------------------------------------------------
    bool MediaStreamTrack::isShutdown() const
    {
      if (mGracefulShutdownReference) return false;
      return mShutdown;
    }
    
    //-------------------------------------------------------------------------
    void MediaStreamTrack::step()
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
    void MediaStreamTrack::cancel()
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
    void MediaStreamTrack::setError(WORD errorCode, const char *inReason)
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
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark AudioStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    AudioStreamTrack::AudioStreamTrack(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate) :
      MediaStreamTrack(queue, delegate)
    {
      
    }
    
    //-------------------------------------------------------------------------
    AudioStreamTrack::~AudioStreamTrack()
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark VideoStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    VideoStreamTrack::VideoStreamTrack(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate) :
      MediaStreamTrack(queue, delegate),
      mRenderView(NULL)
    {
      
    }
    
    //-------------------------------------------------------------------------
    VideoStreamTrack::~VideoStreamTrack()
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark VideoStreamTrack => ILocalVideoStreamTrackForMediaManager, IRemoteReceiveVideoStreamForMediaManager,
    #pragma mark                     IRemoteSendVideoStreamForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    void VideoStreamTrack::setRenderView(void *renderView)
    {
      mRenderView = renderView;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark LocalAudioStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    LocalAudioStreamTrack::LocalAudioStreamTrack(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate) :
      AudioStreamTrack(queue, delegate)
    {
      mTransport = ISendMediaTransportForMediaManager::create();
    }
    
    //-------------------------------------------------------------------------
    LocalAudioStreamTrack::~LocalAudioStreamTrack()
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark LocalAudioStreamTrack => IMediaStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    String LocalAudioStreamTrack::kind()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String LocalAudioStreamTrack::id()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String LocalAudioStreamTrack::label()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    bool LocalAudioStreamTrack::enabled()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool LocalAudioStreamTrack::muted()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool LocalAudioStreamTrack::readonly()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool LocalAudioStreamTrack::remote()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrack::MediaStreamTrackStates LocalAudioStreamTrack::readyState()
    {
      return IMediaStreamTrack::MediaStreamTrackState_New;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrackPtr LocalAudioStreamTrack::clone()
    {
      return IMediaStreamTrackPtr();
    }
    
    //-------------------------------------------------------------------------
    void LocalAudioStreamTrack::stop()
    {
      
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark LocalAudioStreamTrack => ILocalAudioStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    ULONG LocalAudioStreamTrack::getSSRC()
    {
      return 0;
    }
    
    //-------------------------------------------------------------------------
    void LocalAudioStreamTrack::start()
    {
      IMediaEnginePtr mediaEngine = IMediaEngine::singleton();
      
      mediaEngine->startSendVoice(0);
    }
    
    //-------------------------------------------------------------------------
//    void LocalAudioStreamTrack::stop()
//    {
//      
//    }
    
    //-------------------------------------------------------------------------
    SendMediaTransportPtr LocalAudioStreamTrack::getTransport()
    {
      return SendMediaTransportPtr();
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark LocalAudioStreamTrack => ILocalAudioStreamTrackForRTCConnection
    #pragma mark
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteReceiveAudioStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    RemoteReceiveAudioStreamTrack::RemoteReceiveAudioStreamTrack(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate) :
      AudioStreamTrack(queue, delegate)
    {
      IMediaEnginePtr mediaEngine = IMediaEngine::singleton();
      
      mediaEngine->startReceiveVoice(0);
    }
    
    //-------------------------------------------------------------------------
    RemoteReceiveAudioStreamTrack::~RemoteReceiveAudioStreamTrack()
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteReceiveAudioStreamTrack => IMediaStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    String RemoteReceiveAudioStreamTrack::kind()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String RemoteReceiveAudioStreamTrack::id()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String RemoteReceiveAudioStreamTrack::label()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    bool RemoteReceiveAudioStreamTrack::enabled()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool RemoteReceiveAudioStreamTrack::muted()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool RemoteReceiveAudioStreamTrack::readonly()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool RemoteReceiveAudioStreamTrack::remote()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrack::MediaStreamTrackStates RemoteReceiveAudioStreamTrack::readyState()
    {
      return IMediaStreamTrack::MediaStreamTrackState_New;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrackPtr RemoteReceiveAudioStreamTrack::clone()
    {
      return IMediaStreamTrackPtr();
    }
    
    //-------------------------------------------------------------------------
    void RemoteReceiveAudioStreamTrack::stop()
    {
      
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteReceiveAudioStreamTrack => IRemoteReceiveAudioStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    ULONG RemoteReceiveAudioStreamTrack::getSSRC()
    {
      return 0;
    }
    
    //-------------------------------------------------------------------------
    void RemoteReceiveAudioStreamTrack::setEcEnabled(bool enabled)
    {
      
    }
    
    //-------------------------------------------------------------------------
    void RemoteReceiveAudioStreamTrack::setAgcEnabled(bool enabled)
    {
      
    }
    
    //-------------------------------------------------------------------------
    void RemoteReceiveAudioStreamTrack::setNsEnabled(bool enabled)
    {
      
    }
    
    //-------------------------------------------------------------------------
    ReceiveMediaTransportPtr RemoteReceiveAudioStreamTrack::getTransport()
    {
      return ReceiveMediaTransportPtr();
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteReceiveAudioStreamTrack => IRemoteReceiveAudioStreamTrackForRTCConnection
    #pragma mark
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteSendAudioStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    RemoteSendAudioStreamTrack::RemoteSendAudioStreamTrack(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate) :
      AudioStreamTrack(queue, delegate)
    {
    }
    
    //-------------------------------------------------------------------------
    RemoteSendAudioStreamTrack::~RemoteSendAudioStreamTrack()
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteSendAudioStreamTrack => IMediaStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    String RemoteSendAudioStreamTrack::kind()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String RemoteSendAudioStreamTrack::id()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String RemoteSendAudioStreamTrack::label()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    bool RemoteSendAudioStreamTrack::enabled()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool RemoteSendAudioStreamTrack::muted()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool RemoteSendAudioStreamTrack::readonly()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool RemoteSendAudioStreamTrack::remote()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrack::MediaStreamTrackStates RemoteSendAudioStreamTrack::readyState()
    {
      return IMediaStreamTrack::MediaStreamTrackState_New;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrackPtr RemoteSendAudioStreamTrack::clone()
    {
      return IMediaStreamTrackPtr();
    }
    
    //-------------------------------------------------------------------------
    void RemoteSendAudioStreamTrack::stop()
    {
      
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteSendAudioStreamTrack => IRemoteSendAudioStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    ULONG RemoteSendAudioStreamTrack::getSSRC()
    {
      return 0;
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark LocalVideoStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    LocalVideoStreamTrack::LocalVideoStreamTrack(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate) :
      VideoStreamTrack(queue, delegate)
    {
      
    }
    
    //-------------------------------------------------------------------------
    LocalVideoStreamTrack::~LocalVideoStreamTrack()
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark LocalVideoStreamTrack => IMediaStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    String LocalVideoStreamTrack::kind()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String LocalVideoStreamTrack::id()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String LocalVideoStreamTrack::label()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    bool LocalVideoStreamTrack::enabled()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool LocalVideoStreamTrack::muted()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool LocalVideoStreamTrack::readonly()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool LocalVideoStreamTrack::remote()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrack::MediaStreamTrackStates LocalVideoStreamTrack::readyState()
    {
      return IMediaStreamTrack::MediaStreamTrackState_New;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrackPtr LocalVideoStreamTrack::clone()
    {
      return IMediaStreamTrackPtr();
    }
    
    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::stop()
    {
      IMediaEnginePtr mediaEngine = IMediaEngine::singleton();
      
      mediaEngine->stopVideoCapture(0);
      
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark LocalVideoStreamTrack => ILocalVideoStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    ULONG LocalVideoStreamTrack::getSSRC()
    {
      return 0;
    }
    
    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::start()
    {
      IMediaEnginePtr mediaEngine = IMediaEngine::singleton();
      
      mediaEngine->startVideoCapture(0);
      mediaEngine->startSendVideoChannel(0);
    }
    
    //-------------------------------------------------------------------------
//    void LocalVideoStreamTrack::stop()
//    {
//      
//    }

    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::setContinuousVideoCapture(bool continuousVideoCapture)
    {
      
    }
    
    //-------------------------------------------------------------------------
    bool LocalVideoStreamTrack::getContinuousVideoCapture()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::setFaceDetection(bool faceDetection)
    {
      
    }
    
    //-------------------------------------------------------------------------
    bool LocalVideoStreamTrack::getFaceDetection()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    CameraTypes LocalVideoStreamTrack::getCameraType() const
    {
      return CameraType_None;
    }
    
    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::setCameraType(CameraTypes type)
    {
      
    }
    
    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::setRenderView(void *renderView)
    {
      IMediaEnginePtr mediaEngine = IMediaEngine::singleton();
      
      mediaEngine->setRenderView(0, renderView);
      
      VideoStreamTrack::setRenderView(renderView);
    }
    
    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::startRecord(String fileName, bool saveToLibrary)
    {
      
    }
    
    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::stopRecord()
    {
      
    }
    
    //-------------------------------------------------------------------------
    SendMediaTransportPtr LocalVideoStreamTrack::getTransport()
    {
      return SendMediaTransportPtr();
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark LocalVideoStreamTrack => ILocalVideoStreamTrackForRTCConnection
    #pragma mark
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteReceiveVideoStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    RemoteReceiveVideoStreamTrack::RemoteReceiveVideoStreamTrack(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate) :
      VideoStreamTrack(queue, delegate)
    {
      IMediaEnginePtr mediaEngine = IMediaEngine::singleton();
      
//      mediaEngine->startReceiveVideoChannel(0);
    }
    
    //-------------------------------------------------------------------------
    RemoteReceiveVideoStreamTrack::~RemoteReceiveVideoStreamTrack()
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteReceiveVideoStreamTrack => IMediaStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    String RemoteReceiveVideoStreamTrack::kind()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String RemoteReceiveVideoStreamTrack::id()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String RemoteReceiveVideoStreamTrack::label()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    bool RemoteReceiveVideoStreamTrack::enabled()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool RemoteReceiveVideoStreamTrack::muted()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool RemoteReceiveVideoStreamTrack::readonly()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool RemoteReceiveVideoStreamTrack::remote()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrack::MediaStreamTrackStates RemoteReceiveVideoStreamTrack::readyState()
    {
      return IMediaStreamTrack::MediaStreamTrackState_New;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrackPtr RemoteReceiveVideoStreamTrack::clone()
    {
      return IMediaStreamTrackPtr();
    }
    
    //-------------------------------------------------------------------------
    void RemoteReceiveVideoStreamTrack::stop()
    {
      
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteReceiveVideoStreamTrack => IRemoteReceiveVideoStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    ULONG RemoteReceiveVideoStreamTrack::getSSRC()
    {
      return 0;
    }
    
    //-------------------------------------------------------------------------
    void RemoteReceiveVideoStreamTrack::setRenderView(void *renderView)
    {
      IMediaEnginePtr mediaEngine = IMediaEngine::singleton();
      
      mediaEngine->setRenderView(1, renderView);
      
      mediaEngine->startReceiveVideoChannel(0);
      
      VideoStreamTrack::setRenderView(renderView);
    }
    
    //-------------------------------------------------------------------------
    ReceiveMediaTransportPtr RemoteReceiveVideoStreamTrack::getTransport()
    {
      return ReceiveMediaTransportPtr();
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteSendVideoStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    RemoteSendVideoStreamTrack::RemoteSendVideoStreamTrack(IMessageQueuePtr queue, IMediaStreamTrackDelegatePtr delegate) :
      VideoStreamTrack(queue, delegate)
    {
      
    }
    
    //-------------------------------------------------------------------------
    RemoteSendVideoStreamTrack::~RemoteSendVideoStreamTrack()
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteSendVideoStreamTrack => IMediaStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    String RemoteSendVideoStreamTrack::kind()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String RemoteSendVideoStreamTrack::id()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    String RemoteSendVideoStreamTrack::label()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    bool RemoteSendVideoStreamTrack::enabled()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool RemoteSendVideoStreamTrack::muted()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool RemoteSendVideoStreamTrack::readonly()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    bool RemoteSendVideoStreamTrack::remote()
    {
      return false;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrack::MediaStreamTrackStates RemoteSendVideoStreamTrack::readyState()
    {
      return IMediaStreamTrack::MediaStreamTrackState_New;
    }
    
    //-------------------------------------------------------------------------
    IMediaStreamTrackPtr RemoteSendVideoStreamTrack::clone()
    {
      return IMediaStreamTrackPtr();
    }
    
    //-------------------------------------------------------------------------
    void RemoteSendVideoStreamTrack::stop()
    {
      
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteSendVideoStreamTrack => IRemoteSendVideoStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    ULONG RemoteSendVideoStreamTrack::getSSRC()
    {
      return 0;
    }
    
    //-------------------------------------------------------------------------
    void RemoteSendVideoStreamTrack::setRenderView(void *renderView)
    {
      VideoStreamTrack::setRenderView(renderView);
    }

    //-------------------------------------------------------------------------
    SendMediaTransportPtr RemoteSendVideoStreamTrack::getTransport()
    {
      return SendMediaTransportPtr();
    }

  }
}
