/*

 Copyright (c) 2015, Hookflash Inc. / Hookflash Inc.
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
#include <ortc/internal/ortc_RTPSenderChannelMediaBase.h>
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_RTPMediaEngine.h>

#include <ortc/IRTPTypes.h>
#include <ortc/IMediaStreamTrack.h>

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Timer.h>
#include <zsLib/Event.h>

#include <webrtc/transport.h>

//#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_USING_PTR(zsLib, Event)

    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelAudioForSettings)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelMediaBaseForRTPSenderChannel)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelAudioForRTPSenderChannel)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelMediaBaseForMediaStreamTrack)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelAudioForMediaStreamTrack)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelAudioForRTPMediaEngine)

    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForRTPSenderChannelMediaBase)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForRTPSenderChannelAudio)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSenderChannelMediaBase)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSenderChannelAudio)

    ZS_DECLARE_INTERACTION_PROXY(IRTPSenderChannelAudioAsyncDelegate)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelAudioAudioForSettings
    #pragma mark

    interaction IRTPSenderChannelAudioForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelAudioForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IRTPSenderChannelAudioForSettings() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelAudioForRTPSenderChannel
    #pragma mark

    interaction IRTPSenderChannelAudioForRTPSenderChannel : public IRTPSenderChannelMediaBaseForRTPSenderChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelAudioForRTPSenderChannel, ForRTPSenderChannel)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static RTPSenderChannelAudioPtr create(
                                             RTPSenderChannelPtr senderChannel,
                                             MediaStreamTrackPtr track,
                                             const Parameters &params
                                             );
      
      virtual int32_t sendAudioSamples(
                                       const void* audioSamples,
                                       const size_t numberOfSamples,
                                       const uint8_t numberOfChannels
                                       ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelAudioForMediaStreamTrack
    #pragma mark

    interaction IRTPSenderChannelAudioForMediaStreamTrack : public IRTPSenderChannelMediaBaseForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelAudioForMediaStreamTrack, ForMediaStreamTrack)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelAudioForRTPMediaEngine
    #pragma mark

    interaction IRTPSenderChannelAudioForRTPMediaEngine : public IRTPSenderChannelMediaBaseForRTPMediaEngine
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelAudioForRTPMediaEngine, ForRTPMediaEngine)

      static ElementPtr toDebug(ForRTPMediaEnginePtr object);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelAudioAsyncDelegate
    #pragma mark

    interaction IRTPSenderChannelAudioAsyncDelegate
    {
      virtual ~IRTPSenderChannelAudioAsyncDelegate() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio
    #pragma mark
    
    class RTPSenderChannelAudio : public Noop,
                                  public MessageQueueAssociator,
                                  public SharedRecursiveLock,
                                  public IRTPSenderChannelAudioForSettings,
                                  public IRTPSenderChannelAudioForRTPSenderChannel,
                                  public IRTPSenderChannelAudioForMediaStreamTrack,
                                  public IRTPSenderChannelAudioForRTPMediaEngine,
                                  public IWakeDelegate,
                                  public zsLib::ITimerDelegate,
                                  public zsLib::IPromiseSettledDelegate,
                                  public IRTPSenderChannelAudioAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPSenderChannelAudio;
      friend interaction IRTPSenderChannelAudioFactory;
      friend interaction IRTPSenderChannelAudioForSettings;
      friend interaction IRTPSenderChannelMediaBaseForRTPSenderChannel;
      friend interaction IRTPSenderChannelAudioForRTPSenderChannel;
      friend interaction IRTPSenderChannelMediaBaseForMediaStreamTrack;
      friend interaction IRTPSenderChannelAudioForMediaStreamTrack;
      friend interaction IRTPSenderChannelAudioForRTPMediaEngine;

      ZS_DECLARE_CLASS_PTR(Transport)
      friend class Transport;

      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForRTPSenderChannelAudio, UseChannel)
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannelMediaBase, UseBaseMediaStreamTrack)
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannelAudio, UseMediaStreamTrack)
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineForRTPSenderChannelAudio, UseMediaEngine)
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineDeviceResource, UseDeviceResource)
      ZS_DECLARE_TYPEDEF_PTR(IRTPMediaEngineAudioSenderChannelResource, UseChannelResource)

      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelMediaBaseForRTPSenderChannel, ForRTPSenderChannelFromMediaBase)
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelMediaBaseForMediaStreamTrack, ForMediaStreamTrackFromMediaBase)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)
      typedef std::list<RTCPPacketPtr> RTCPPacketList;
      ZS_DECLARE_PTR(RTCPPacketList)

      enum States
      {
        State_Pending,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state);

    public:
      RTPSenderChannelAudio(
                            const make_private &,
                            IMessageQueuePtr queue,
                            UseChannelPtr senderChannel,
                            UseMediaStreamTrackPtr track,
                            const Parameters &params
                            );

    protected:
      RTPSenderChannelAudio(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~RTPSenderChannelAudio();

      static RTPSenderChannelAudioPtr convert(ForSettingsPtr object);
      static RTPSenderChannelAudioPtr convert(ForRTPSenderChannelFromMediaBasePtr object);
      static RTPSenderChannelAudioPtr convert(ForRTPSenderChannelPtr object);
      static RTPSenderChannelAudioPtr convert(ForMediaStreamTrackFromMediaBasePtr object);
      static RTPSenderChannelAudioPtr convert(ForMediaStreamTrackPtr object);
      static RTPSenderChannelAudioPtr convert(ForRTPMediaEnginePtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelAudio => IRTPSenderChannelMediaBaseForRTPSenderChannel
      #pragma mark

      virtual PUID getID() const override {return mID;}

      virtual void onTrackChanged(UseBaseMediaStreamTrackPtr track) override;

      virtual void notifyTransportState(ISecureTransportTypes::States state) override;

      virtual bool handlePacket(RTCPPacketPtr packet) override;

      virtual void handleUpdate(ParametersPtr params) override;
      
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelAudio => IRTPSenderChannelAudioForRTPSenderChannel
      #pragma mark

      static RTPSenderChannelAudioPtr create(
                                             RTPSenderChannelPtr senderChannel,
                                             MediaStreamTrackPtr track,
                                             const Parameters &params
                                             );

      virtual int32_t sendAudioSamples(
                                       const void* audioSamples,
                                       const size_t numberOfSamples,
                                       const uint8_t numberOfChannels
                                       ) override;
      
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelAudio => IRTPSenderChannelAudioForMediaStreamTrack
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelAudio => IRTPSenderChannelMediaBaseForRTPMediaEngine
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForRTPMediaEnginePtr object);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelAudio => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelAudio => ITimerDelegate
      #pragma mark

      virtual void onTimer(TimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelAudio => IPromiseSettledDelegate
      #pragma mark

      virtual void onPromiseSettled(PromisePtr promise) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelAudio => IRTPSenderChannelAudioAsyncDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelAudio => friend Transport
      #pragma mark

      virtual bool SendRtp(
                           const uint8_t* packet,
                           size_t length,
                           const webrtc::PacketOptions& options
                           );

      virtual bool SendRtcp(const uint8_t* packet, size_t length);

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelAudio::Transport
      #pragma mark

      class Transport : public webrtc::Transport
      {
        struct make_private {};

      protected:
        void init();
        
      public:
        Transport(
                  const make_private &,
                  RTPSenderChannelAudioPtr outer
                  );
        
        ~Transport();
        
        static TransportPtr create(RTPSenderChannelAudioPtr outer);
        
      public:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark RTPSenderChannelAudio::Transport => webrtc::Transport
        #pragma mark

        virtual bool SendRtp(
                             const uint8_t* packet,
                             size_t length,
                             const webrtc::PacketOptions& options
                             ) override;
        
        virtual bool SendRtcp(const uint8_t* packet, size_t length) override;
        
      private:
        TransportWeakPtr mThisWeak;
        RTPSenderChannelAudioWeakPtr mOuter;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelAudio => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepSetupChannel();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannelAudio => (data)
      #pragma mark

      AutoPUID mID;
      RTPSenderChannelAudioWeakPtr mThisWeak;
      RTPSenderChannelAudioPtr mGracefulShutdownReference;

      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      UseChannelWeakPtr mSenderChannel;

      ParametersPtr mParameters;

      PromiseWithRTPMediaEngineChannelResourcePtr mSetupChannelPromise;
      UseChannelResourcePtr mChannelResource;

      PromisePtr mCloseChannelPromise;

      Optional<IMediaStreamTrackTypes::Kinds> mKind;
      UseMediaStreamTrackPtr mTrack;

      TransportPtr mTransport;  // allow lifetime of callback to exist separate from "this" object
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelAudioFactory
    #pragma mark

    interaction IRTPSenderChannelAudioFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static IRTPSenderChannelAudioFactory &singleton();

      virtual RTPSenderChannelAudioPtr create(
                                              RTPSenderChannelPtr sender,
                                              MediaStreamTrackPtr track,
                                              const Parameters &params
                                              );
    };

    class RTPSenderChannelAudioFactory : public IFactory<IRTPSenderChannelAudioFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPSenderChannelAudioAsyncDelegate)
ZS_DECLARE_PROXY_END()
