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
#include <zsLib/Log.h>

namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib) }

namespace ortc
{
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
      
    }
    
    //-------------------------------------------------------------------------
    int ReceiveMediaTransport::receivedRTCPPacket(const void *data, unsigned int length)
    {
      
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
      
    }
    
    int SendMediaTransport::deregisterExternalTransport()
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ILocalAudioStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    LocalAudioStreamPtr ILocalAudioStreamTrackForMediaManager::create(IMessageQueuePtr queue, IMediaStreamDelegatePtr delegate)
    {
      LocalAudioStreamPtr pThis(new LocalAudioStream(queue, delegate));
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
    RemoteReceiveAudioStreamPtr IRemoteReceiveAudioStreamTrackForMediaManager::create(IMessageQueuePtr queue, IMediaStreamDelegatePtr delegate)
    {
      RemoteReceiveAudioStreamPtr pThis(new RemoteReceiveAudioStream(queue, delegate));
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
    RemoteSendAudioStreamPtr IRemoteSendAudioStreamTrackForMediaManager::create(IMessageQueuePtr queue, IMediaStreamDelegatePtr delegate)
    {
      RemoteSendAudioStreamPtr pThis(new RemoteSendAudioStream(queue, delegate));
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
    LocalVideoStreamPtr ILocalVideoStreamTrackForMediaManager::create(IMessageQueuePtr queue, IMediaStreamDelegatePtr delegate)
    {
      LocalVideoStreamPtr pThis(new LocalSendVideoStream(queue, delegate));
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
    RemoteReceiveVideoStreamPtr IRemoteReceiveVideoStreamTrackForMediaManager::create(IMessageQueuePtr queue, IMediaStreamDelegatePtr delegate)
    {
      RemoteReceiveVideoStreamPtr pThis(new RemoteReceiveVideoStream(queue, delegate));
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
    RemoteSendVideoStreamPtr IRemoteSendVideoStreamTrackForMediaManager::create(IMessageQueuePtr queue, IMediaStreamDelegatePtr delegate)
    {
      RemoteSendVideoStreamPtr pThis(new RemoteSendVideoStream(queue, delegate));
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
      mDelegate(delegate),
      mError(0),
      mEnabled(false),
      mMuted(false),
      mReadonly(false),
      mRemote(false),
      mReadyState(MediaStreamTrackState_New),
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
    
    String MediaStreamTrack::kind()
    {
      return String();
    }
    
    String MediaStreamTrack::id()
    {
      return String();
    }
    
    String MediaStreamTrack::label()
    {
      return String();
    }
    
    bool MediaStreamTrack::enabled()
    {
      return false;
    }
    
    bool MediaStreamTrack::muted()
    {
      return false;
    }
    
    bool MediaStreamTrack::readonly()
    {
      return false;
    }
    
    bool MediaStreamTrack::remote()
    {
      return false;
    }
    
    MediaStreamTrack::MediaStreamTrackStates MediaStreamTrack::readyState()
    {
      return MediaStreamTrackState_New;
    }
    
    IMediaStreamTrackPtr MediaStreamTrack::clone()
    {
      return IMediaStreamTrackPtr();
    }
    
    void MediaStreamTrack::stop()
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack => (internal)
    #pragma mark
    
    //-------------------------------------------------------------------------
    String MediaStreamTrack::log(const char *message) const
    {
      
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
      MediaStreamTrack(queue, delegate)
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
      mTransport = ISendMediaTransportForCallTransport::create();
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
      
    }
    
    //-------------------------------------------------------------------------
    void LocalAudioStreamTrack::start()
    {
      
    }
    
    //-------------------------------------------------------------------------
    void LocalAudioStreamTrack::stop()
    {
      
    }
    
    //-------------------------------------------------------------------------
    SendMediaTransportPtr LocalAudioStreamTrack::getTransport()
    {
      
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
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteReceiveAudioStreamTrack => IRemoteReceiveAudioStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    ULONG RemoteReceiveAudioStream::getSSRC()
    {
      
    }
    
    //-------------------------------------------------------------------------
    void RemoteReceiveAudioStream::setEcEnabled(bool enabled)
    {
      
    }
    
    //-------------------------------------------------------------------------
    void RemoteReceiveAudioStream::setAgcEnabled(bool enabled)
    {
      
    }
    
    //-------------------------------------------------------------------------
    void RemoteReceiveAudioStream::setNsEnabled(bool enabled)
    {
      
    }
    
    //-------------------------------------------------------------------------
    ReceiveMediaTransportPtr RemoteReceiveAudioStream::getTransport()
    {
      
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
    #pragma mark RemoteSendAudioStreamTrack => IRemoteSendAudioStreamTrackForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    ULONG RemoteSendAudioStream::getSSRC()
    {
      
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
      
    }
    
    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::start()
    {
      
    }
    
    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::stop()
    {
      
    }

    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::setContinuousVideoCapture(bool continuousVideoCapture)
    {
      
    }
    
    //-------------------------------------------------------------------------
    bool LocalVideoStreamTrack::getContinuousVideoCapture()
    {
      
    }
    
    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::setFaceDetection(bool faceDetection)
    {
      
    }
    
    //-------------------------------------------------------------------------
    bool LocalVideoStreamTrack::getFaceDetection()
    {
      
    }
    
    //-------------------------------------------------------------------------
    ILocalVideoStreamTrackForMediaManager::CameraTypes LocalVideoStreamTrack::getCameraType() const
    {
      
    }
    
    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::setCameraType(CameraTypes type)
    {
      
    }
    
    //-------------------------------------------------------------------------
    void LocalVideoStreamTrack::setRenderView(void *renderView)
    {
      
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
      
    }
    
    //-------------------------------------------------------------------------
    RemoteReceiveVideoStreamTrack::~RemoteReceiveVideoStreamTrack()
    {
      
    }
    
    //-------------------------------------------------------------------------
    void RemoteReceiveVideoStreamTrack::setRenderView(void *renderView)
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteReceiveVideoStreamTrack => IMediaStreamTrack
    #pragma mark
    
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
      
    }
    
    //-------------------------------------------------------------------------
    ReceiveMediaTransportPtr RemoteReceiveVideoStreamTrack::getTransport()
    {
      
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
    
    //-------------------------------------------------------------------------
    void RemoteSendVideoStreamTrack::setRenderView(void *renderView)
    {
      
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteSendVideoStreamTrack => IMediaStreamTrack
    #pragma mark
    
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
      
    }

    //-------------------------------------------------------------------------
    SendMediaTransportPtr RemoteSendVideoStreamTrack::getTransport()
    {
      
    }

  }
}
