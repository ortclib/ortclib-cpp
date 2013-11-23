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

#include <common_types.h>

namespace ortc
{
  namespace internal
  {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaTransport
    #pragma mark
    
    interaction IMediaTransport
    {
      struct RtpRtcpStatistics
      {
        unsigned short fractionLost;
        unsigned int cumulativeLost;
        unsigned int extendedMax;
        unsigned int jitter;
        int rttMs;
        int bytesSent;
        int packetsSent;
        int bytesReceived;
        int packetsReceived;
      };
      
      virtual int getTransportStatistics(RtpRtcpStatistics &stat) = 0;
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IReceiveMediaTransportForMediaManager
    #pragma mark
    
    interaction IReceiveMediaTransportForMediaManager
    {
      IReceiveMediaTransportForMediaManager &forMediaManager() {return *this;}
      const IReceiveMediaTransportForMediaManager &forMediaManager() const {return *this;}
      
      static ReceiveMediaTransportPtr create();
      
      virtual int receivedRTPPacket(const void *data, unsigned int length) = 0;
      virtual int receivedRTCPPacket(const void *data, unsigned int length) = 0;
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ISendMediaTransportForMediaManager
    #pragma mark
    
    interaction ISendMediaTransportForMediaManager
    {
      typedef webrtc::Transport Transport;
      
      ISendMediaTransportForMediaManager &forMediaManager() {return *this;}
      const ISendMediaTransportForMediaManager &forMediaManager() const {return *this;}
      
      static SendMediaTransportPtr create();
      
      virtual int registerExternalTransport(Transport &transport) = 0;
      virtual int deregisterExternalTransport() = 0;
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaTransport
    #pragma mark
    
    class MediaTransport
    {
    public:
      friend interaction IMediaTransport;
      
    protected:
      MediaTransport();
      
    public:
      virtual ~MediaTransport();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaTransport => IMediaTransport
      #pragma mark
      
      virtual int getTransportStatistics(IMediaTransport::RtpRtcpStatistics &stat);
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaTransport => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaTransport => (data)
      #pragma mark
      
    protected:
      mutable RecursiveLock mLock;
      MediaTransportWeakPtr mThisWeak;
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ReceiveMediaTransport
    #pragma mark
    
    class ReceiveMediaTransport : public MediaTransport,
      public IMediaTransport,
      public IReceiveMediaTransportForMediaManager
    {
      
    public:
      friend interaction IReceiveMediaTransportForMediaManager;
      
    protected:
      ReceiveMediaTransport();
      
    public:
      virtual ~ReceiveMediaTransport();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark ReceiveMediaTransport => IMediaTransport
      #pragma mark
      
      virtual int getTransportStatistics(IMediaTransport::RtpRtcpStatistics &stat);
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark ReceiveMediaTransport => IReceiveMediaTransportForMediaManager
      #pragma mark
      
      virtual int receivedRTPPacket(const void *data, unsigned int length);
      virtual int receivedRTCPPacket(const void *data, unsigned int length);
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark ReceiveMediaTransport => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark ReceiveMediaTransport => (data)
      #pragma mark
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark SendMediaTransport
    #pragma mark
    
    class SendMediaTransport : public MediaTransport,
      public IMediaTransport,
      public ISendMediaTransportForMediaManager
    {
      
    public:
      friend interaction ISendMediaTransportForMediaManager;
      
    protected:
      SendMediaTransport();
      
    public:
      virtual ~SendMediaTransport();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark SendMediaTransport => IMediaTransport
      #pragma mark
      
      virtual int getTransportStatistics(IMediaTransport::RtpRtcpStatistics &stat);
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark SendMediaTransport => ISendMediaTransportForCallTransport
      #pragma mark
      
      virtual int registerExternalTransport(Transport &transport);
      virtual int deregisterExternalTransport();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark SendMediaTransport => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark SendMediaTransport => (data)
      #pragma mark
      
    protected:
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ILocalAudioStreamTrackForMediaManager
    #pragma mark
    
    interaction ILocalAudioStreamTrackForMediaManager
    {
    public:
      ILocalAudioStreamTrackForMediaManager &forMediaManager() {return *this;}
      const ILocalAudioStreamTrackForMediaManager &forMediaManager() const {return *this;}
      
      static LocalAudioStreamTrackPtr create(
                                             IMessageQueuePtr queue,
                                             IMediaStreamTrackDelegatePtr delegate
                                             );
      
      virtual ULONG getSSRC() = 0;

      virtual void start() = 0;
      virtual void stop() = 0;
      
      virtual SendMediaTransportPtr getTransport() = 0;
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ILocalAudioStreamTrackForRTCConnection
    #pragma mark
    
    interaction ILocalAudioStreamTrackForRTCConnection
    {
    public:
      ILocalAudioStreamTrackForRTCConnection &forRTCConnection() {return *this;}
      const ILocalAudioStreamTrackForRTCConnection &forRTCConnection() const {return *this;}
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteReceiveAudioStreamTrackForMediaManager
    #pragma mark
    
    interaction IRemoteReceiveAudioStreamTrackForMediaManager
    {
    public:
      IRemoteReceiveAudioStreamTrackForMediaManager &forMediaManager() {return *this;}
      const IRemoteReceiveAudioStreamTrackForMediaManager &forMediaManager() const {return *this;}
      
      static RemoteReceiveAudioStreamTrackPtr create(
                                                     IMessageQueuePtr queue,
                                                     IMediaStreamTrackDelegatePtr delegate
                                                     );
      
      virtual ULONG getSSRC() = 0;

      virtual void setEcEnabled(bool enabled) = 0;
      virtual void setAgcEnabled(bool enabled) = 0;
      virtual void setNsEnabled(bool enabled) = 0;
      
      virtual ReceiveMediaTransportPtr getTransport() = 0;
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteReceiveAudioStreamTrackForRTCConnection
    #pragma mark
    
    interaction IRemoteReceiveAudioStreamTrackForRTCConnection
    {
    public:
      IRemoteReceiveAudioStreamTrackForRTCConnection &forRTCConnection() {return *this;}
      const IRemoteReceiveAudioStreamTrackForRTCConnection &forRTCConnection() const {return *this;}
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteSendAudioStreamTrackForMediaManager
    #pragma mark
    
    interaction IRemoteSendAudioStreamTrackForMediaManager
    {
    public:
      IRemoteSendAudioStreamTrackForMediaManager &forMediaManager() {return *this;}
      const IRemoteSendAudioStreamTrackForMediaManager &forMediaManager() const {return *this;}
      
      static RemoteSendAudioStreamTrackPtr create(
                                                  IMessageQueuePtr queue,
                                                  IMediaStreamTrackDelegatePtr delegate
                                                  );
      
      virtual ULONG getSSRC() = 0;

      virtual SendMediaTransportPtr getTransport() = 0;
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteSendAudioStreamTrackForRTCConnection
    #pragma mark
    
    interaction IRemoteSendAudioStreamTrackForRTCConnection
    {
    public:
      IRemoteSendAudioStreamTrackForRTCConnection &forRTCConnection() {return *this;}
      const IRemoteSendAudioStreamTrackForRTCConnection &forRTCConnection() const {return *this;}
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ILocalVideoStreamTrackForMediaManager
    #pragma mark
    
    interaction ILocalVideoStreamTrackForMediaManager
    {
    public:
      enum CameraTypes
      {
        CameraType_None,
        CameraType_Front,
        CameraType_Back
      };
      
      static const char *toString(CameraTypes type);
      
      ILocalVideoStreamTrackForMediaManager &forMediaManager() {return *this;}
      const ILocalVideoStreamTrackForMediaManager &forMediaManager() const {return *this;}
      
      static LocalVideoStreamTrackPtr create(
                                             IMessageQueuePtr queue,
                                             IMediaStreamTrackDelegatePtr delegate
                                             );
      
      virtual ULONG getSSRC() = 0;

      virtual void start() = 0;
      virtual void stop() = 0;

      virtual void setContinuousVideoCapture(bool continuousVideoCapture) = 0;
      virtual bool getContinuousVideoCapture() = 0;
      
      virtual void setFaceDetection(bool faceDetection) = 0;
      virtual bool getFaceDetection() = 0;
      
      virtual CameraTypes getCameraType() const = 0;
      virtual void setCameraType(CameraTypes type) = 0;
      
      virtual void setRenderView(void *renderView) = 0;
      
      virtual void startRecord(String fileName, bool saveToLibrary = false) = 0;
      virtual void stopRecord() = 0;
      
      virtual SendMediaTransportPtr getTransport() = 0;
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ILocalVideoStreamTrackForRTCConnection
    #pragma mark
    
    interaction ILocalVideoStreamTrackForRTCConnection
    {
    public:
      ILocalVideoStreamTrackForRTCConnection &forRTCConnection() {return *this;}
      const ILocalVideoStreamTrackForRTCConnection &forRTCConnection() const {return *this;}
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteReceiveVideoStreamTrackForMediaManager
    #pragma mark
    
    interaction IRemoteReceiveVideoStreamTrackForMediaManager
    {
    public:
      IRemoteReceiveVideoStreamTrackForMediaManager &forMediaManager() {return *this;}
      const IRemoteReceiveVideoStreamTrackForMediaManager &forMediaManager() const {return *this;}
      
      static RemoteReceiveVideoStreamTrackPtr create(
                                                     IMessageQueuePtr queue,
                                                     IMediaStreamTrackDelegatePtr delegate
                                                     );
      
      virtual ULONG getSSRC() = 0;

      virtual void setRenderView(void *renderView) = 0;
      
      virtual ReceiveMediaTransportPtr getTransport() = 0;
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteReceiveVideoStreamTrackForRTCConnection
    #pragma mark
    
    interaction IRemoteReceiveVideoStreamTrackForRTCConnection
    {
    public:
      IRemoteReceiveVideoStreamTrackForRTCConnection &forRTCConnection() {return *this;}
      const IRemoteReceiveVideoStreamTrackForRTCConnection &forRTCConnection() const {return *this;}
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteSendVideoStreamTrackForMediaManager
    #pragma mark
    
    interaction IRemoteSendVideoStreamTrackForMediaManager
    {
    public:
      IRemoteSendVideoStreamTrackForMediaManager &forMediaManager() {return *this;}
      const IRemoteSendVideoStreamTrackForMediaManager &forMediaManager() const {return *this;}
      
      static RemoteSendVideoStreamTrackPtr create(
                                                  IMessageQueuePtr queue,
                                                  IMediaStreamTrackDelegatePtr delegate
                                                  );
      
      virtual ULONG getSSRC() = 0;

      virtual void setRenderView(void *renderView) = 0;
      
      virtual SendMediaTransportPtr getTransport() = 0;
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRemoteSendVideoStreamTrackForRTCConnection
    #pragma mark
    
    interaction IRemoteSendVideoStreamTrackForRTCConnection
    {
    public:
      IRemoteSendVideoStreamTrackForRTCConnection &forRTCConnection() {return *this;}
      const IRemoteSendVideoStreamTrackForRTCConnection &forRTCConnection() const {return *this;}
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack
    #pragma mark
    
    class MediaStreamTrack : public Noop,
      public MessageQueueAssociator
    {
    public:
      friend interaction IMediaStreamTrack;
      friend interaction ILocalAudioStreamTrackForMediaManager;
      friend interaction IRemoteReceiveAudioStreamTrackForMediaManager;
      friend interaction IRemoteSendAudioStreamTrackForMediaManager;
      friend interaction ILocalVideoStreamTrackForMediaManager;
      friend interaction IRemoteReceiveVideoStreamTrackForMediaManager;
      friend interaction IRemoteSendVideoStreamTrackForMediaManager;
      
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
      virtual IMediaStreamTrack::MediaStreamTrackStates readyState();
      virtual IMediaStreamTrackPtr clone();
      virtual void stop();

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => ILocalAudioStreamTrackForMediaManager, IRemoteReceiveAudioStreamTrackForMediaManager,
      #pragma mark                     IRemoteSendAudioStreamTrackForMediaManager, ILocalVideoStreamTrackForMediaManager.
      #pragma mark                     IRemoteReceiveVideoStreamTrackForMediaManager, IRemoteSendVideoStreamTrackForMediaManager
      #pragma mark
      
    protected:
      
      virtual ULONG getSSRC() = 0;
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => (internal)
      #pragma mark
      
    private:
      String log(const char *message) const;

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
      IMediaStreamTrack::MediaStreamTrackStates mReadyState;
      
      ULONG mSSRC;
      int mChannel;
      IMediaTransportPtr mTransport;
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark AudioStreamTrack
    #pragma mark
    
    class AudioStreamTrack : public MediaStreamTrack
    {
    public:
      friend interaction IMediaStreamTrack;
      
    protected:
      AudioStreamTrack(
                       IMessageQueuePtr queue,
                       IMediaStreamTrackDelegatePtr delegate
                       );
      
    public:
      virtual ~AudioStreamTrack();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark AudioStreamTrack => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark AudioStreamTrack => (data)
      #pragma mark
      
    protected:
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark VideoStreamTrack
    #pragma mark
    
    class VideoStreamTrack : public MediaStreamTrack
    {
    public:
      friend interaction IMediaStreamTrack;
      friend interaction ILocalVideoStreamTrackForMediaManager;
      friend interaction IRemoteReceiveVideoStreamTrackForMediaManager;
      friend interaction IRemoteSendVideoStreamTrackForMediaManager;
      
    protected:
      VideoStreamTrack(
                       IMessageQueuePtr queue,
                       IMediaStreamTrackDelegatePtr delegate
                       );
      
    public:
      virtual ~VideoStreamTrack();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark VideoStreamTrack => ILocalVideoStreamTrackForMediaManager, IRemoteReceiveVideoStreamTrackForMediaManager,
      #pragma mark                IRemoteSendVideoStreamTrackForMediaManager
      #pragma mark
      
      virtual void setRenderView(void *renderView);
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark VideoStreamTrack => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark VideoStreamTrack => (data)
      #pragma mark
      
    protected:
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark LocalAudioStreamTrack
    #pragma mark
    
    class LocalAudioStreamTrack : public AudioStreamTrack,
      public IMediaStreamTrack,
      public ILocalAudioStreamTrackForMediaManager,
      public ILocalAudioStreamTrackForRTCConnection
    {
    public:
      friend interaction ILocalAudioStreamTrackForMediaManager;
      friend interaction ILocalAudioStreamTrackForRTCConnection;
      
    protected:
      LocalAudioStreamTrack(
                            IMessageQueuePtr queue,
                            IMediaStreamTrackDelegatePtr delegate
                            );
      
    public:
      virtual ~LocalAudioStreamTrack();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark LocalAudioStreamTrack => IMediaStreamTrack
      #pragma mark
      
      virtual String kind();
      virtual String id();
      virtual String label();
      virtual bool enabled();
      virtual bool muted();
      virtual bool readonly();
      virtual bool remote();
      virtual IMediaStreamTrack::MediaStreamTrackStates readyState();
      virtual IMediaStreamTrackPtr clone();
      virtual void stop();

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark LocalAudioStreamTrack => ILocalAudioStreamTrackForMediaManager
      #pragma mark
      
      virtual ULONG getSSRC();
      virtual void start();
      //virtual void stop();
      virtual SendMediaTransportPtr getTransport();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark LocalAudioStreamTrack => ILocalAudioStreamTrackForRTCConnection
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark LocalAudioStreamTrack => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark LocalAudioStreamTrack => (data)
      #pragma mark
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteReceiveAudioStreamTrack
    #pragma mark
    
    class RemoteReceiveAudioStreamTrack : public AudioStreamTrack,
      public IMediaStreamTrack,
      public IRemoteReceiveAudioStreamTrackForMediaManager,
      public IRemoteReceiveAudioStreamTrackForRTCConnection
    {
    public:
      friend interaction IRemoteReceiveAudioStreamTrackForMediaManager;
      friend interaction IRemoteReceiveAudioStreamTrackForRTCConnection;
      
    protected:
      RemoteReceiveAudioStreamTrack(
                                    IMessageQueuePtr queue,
                                    IMediaStreamTrackDelegatePtr delegate
                                    );
      
    public:
      virtual ~RemoteReceiveAudioStreamTrack();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteReceiveAudioStreamTrack => IMediaStreamTrack
      #pragma mark
      
      virtual String kind();
      virtual String id();
      virtual String label();
      virtual bool enabled();
      virtual bool muted();
      virtual bool readonly();
      virtual bool remote();
      virtual IMediaStreamTrack::MediaStreamTrackStates readyState();
      virtual IMediaStreamTrackPtr clone();
      virtual void stop();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteReceiveAudioStreamTrack => IRemoteReceiveAudioStreamTrackForMediaManager
      #pragma mark
      
      virtual ULONG getSSRC();
      virtual void setEcEnabled(bool enabled);
      virtual void setAgcEnabled(bool enabled);
      virtual void setNsEnabled(bool enabled);
      virtual ReceiveMediaTransportPtr getTransport();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteReceiveAudioStreamTrack => IRemoteReceiveAudioStreamTrackForRTCConnection
      #pragma mark
      
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteReceiveAudioStreamTrack => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteReceiveAudioStreamTrack => (data)
      #pragma mark
      
    protected:
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteSendAudioStreamTrack
    #pragma mark
    
    class RemoteSendAudioStreamTrack : public AudioStreamTrack,
      public IMediaStreamTrack,
      public IRemoteSendAudioStreamTrackForMediaManager,
      public IRemoteSendAudioStreamTrackForRTCConnection
    {
    public:
      friend interaction IRemoteSendAudioStreamTrackForMediaManager;
      friend interaction IRemoteSendAudioStreamTrackForRTCConnection;
      
    protected:
      RemoteSendAudioStreamTrack(
                                 IMessageQueuePtr queue,
                                 IMediaStreamTrackDelegatePtr delegate
                                 );
      
    public:
      virtual ~RemoteSendAudioStreamTrack();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteSendAudioStreamTrack => IMediaStreamTrack
      #pragma mark
      
      virtual String kind();
      virtual String id();
      virtual String label();
      virtual bool enabled();
      virtual bool muted();
      virtual bool readonly();
      virtual bool remote();
      virtual IMediaStreamTrack::MediaStreamTrackStates readyState();
      virtual IMediaStreamTrackPtr clone();
      virtual void stop();

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteSendAudioStreamTrack => IRemoteSendAudioStreamTrackForMediaManager
      #pragma mark

      virtual ULONG getSSRC();
      virtual SendMediaTransportPtr getTransport();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteSendAudioStreamTrack => IRemoteSendAudioStreamTrackForRTCConnection
      #pragma mark
      
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteSendAudioStreamTrack => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteSendAudioStreamTrack => (data)
      #pragma mark
      
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark LocalVideoStreamTrack
    #pragma mark
    
    class LocalVideoStreamTrack : public VideoStreamTrack,
      public IMediaStreamTrack,
      public ILocalVideoStreamTrackForMediaManager,
      public ILocalVideoStreamTrackForRTCConnection
    {
    public:
      friend interaction IMediaStreamTrack;
      friend interaction ILocalVideoStreamTrackForMediaManager;
      friend interaction ILocalVideoStreamTrackForRTCConnection;
      
    protected:
      LocalVideoStreamTrack(
                            IMessageQueuePtr queue,
                            IMediaStreamTrackDelegatePtr delegate
                            );
      
    public:
      virtual ~LocalVideoStreamTrack();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark LocalVideoStreamTrack => IMediaStreamTrack
      #pragma mark
      
      virtual String kind();
      virtual String id();
      virtual String label();
      virtual bool enabled();
      virtual bool muted();
      virtual bool readonly();
      virtual bool remote();
      virtual IMediaStreamTrack::MediaStreamTrackStates readyState();
      virtual IMediaStreamTrackPtr clone();
      virtual void stop();

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark LocalVideoStreamTrack => ILocalVideoStreamTrackForMediaManager
      #pragma mark

      virtual ULONG getSSRC();
      
      virtual void start();
      //virtual void stop();

      virtual void setContinuousVideoCapture(bool continuousVideoCapture);
      virtual bool getContinuousVideoCapture();
      
      virtual void setFaceDetection(bool faceDetection);
      virtual bool getFaceDetection();
      
      virtual CameraTypes getCameraType() const;
      virtual void setCameraType(CameraTypes type);
      
      virtual void setRenderView(void *renderView);
      
      virtual void startRecord(String fileName, bool saveToLibrary = false);
      virtual void stopRecord();
      
      virtual SendMediaTransportPtr getTransport();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark LocalVideoStreamTrack => ILocalVideoStreamTrackForRTCConnection
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark LocalVideoStreamTrack => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark LocalVideoStreamTrack => (data)
      #pragma mark
      
    protected:
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteReceiveVideoStreamTrack
    #pragma mark
    
    class RemoteReceiveVideoStreamTrack : public VideoStreamTrack,
      public IMediaStreamTrack,
      public IRemoteReceiveVideoStreamTrackForMediaManager,
      public IRemoteReceiveVideoStreamTrackForRTCConnection
    {
    public:
      friend interaction IRemoteReceiveVideoStreamTrackForMediaManager;
      friend interaction IRemoteReceiveVideoStreamTrackForRTCConnection;
      
    protected:
      RemoteReceiveVideoStreamTrack(
                                    IMessageQueuePtr queue,
                                    IMediaStreamTrackDelegatePtr delegate
                                    );
      
    public:
      virtual ~RemoteReceiveVideoStreamTrack();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteReceiveVideoStreamTrack => IMediaStreamTrack
      #pragma mark
      
      virtual String kind();
      virtual String id();
      virtual String label();
      virtual bool enabled();
      virtual bool muted();
      virtual bool readonly();
      virtual bool remote();
      virtual IMediaStreamTrack::MediaStreamTrackStates readyState();
      virtual IMediaStreamTrackPtr clone();
      virtual void stop();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteReceiveVideoStreamTrack => IRemoteReceiveVideoStreamTrackForMediaManager
      #pragma mark
      
      virtual ULONG getSSRC();
      virtual void setRenderView(void *renderView);
      virtual ReceiveMediaTransportPtr getTransport();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteReceiveVideoStreamTrack => IRemoteReceiveVideoStreamTrackForRTCConnection
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteReceiveVideoStreamTrack => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteReceiveVideoStreamTrack => (data)
      #pragma mark
      
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RemoteSendVideoStreamTrack
    #pragma mark
    
    class RemoteSendVideoStreamTrack : public VideoStreamTrack,
      public IMediaStreamTrack,
      public IRemoteSendVideoStreamTrackForMediaManager,
      public IRemoteSendVideoStreamTrackForRTCConnection
    {
    public:
      friend interaction IRemoteSendVideoStreamTrackForMediaManager;
      friend interaction IRemoteSendVideoStreamTrackForRTCConnection;
      
    protected:
      RemoteSendVideoStreamTrack(
                                 IMessageQueuePtr queue,
                                 IMediaStreamTrackDelegatePtr delegate
                                 );
      
    public:
      virtual ~RemoteSendVideoStreamTrack();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteSendVideoStreamTrack => IMediaStreamTrack
      #pragma mark

      virtual String kind();
      virtual String id();
      virtual String label();
      virtual bool enabled();
      virtual bool muted();
      virtual bool readonly();
      virtual bool remote();
      virtual IMediaStreamTrack::MediaStreamTrackStates readyState();
      virtual IMediaStreamTrackPtr clone();
      virtual void stop();

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteSendVideoStreamTrack => IRemoteSendVideoStreamTrackForMediaManager
      #pragma mark
      
      virtual ULONG getSSRC();
      virtual void setRenderView(void *renderView);
      virtual SendMediaTransportPtr getTransport();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteSendVideoStreamTrack => IRemoteSendVideoStreamTrackForRTCConnection
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteSendVideoStreamTrack => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RemoteSendVideoStreamTrack => (data)
      #pragma mark
      
      
    };
  }
}
