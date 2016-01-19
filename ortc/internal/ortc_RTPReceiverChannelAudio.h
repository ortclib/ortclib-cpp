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
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_RTPReceiverChannelMediaBase.h>

#include <ortc/IICETransport.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IRTPTypes.h>

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Timer.h>


//#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelAudioForSettings)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelAudioForRTPReceiverChannel)
    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelAudioForMediaStreamTrack)

    ZS_DECLARE_INTERACTION_PTR(IRTPReceiverChannelForRTPReceiverChannelAudio)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPReceiverChannelAudio)

    ZS_DECLARE_INTERACTION_PROXY(IRTPReceiverChannelAudioAsyncDelegate)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelAudioForSettings
    #pragma mark

    interaction IRTPReceiverChannelAudioForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelAudioForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IRTPReceiverChannelAudioForSettings() {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelAudioForRTPReceiverChannel
    #pragma mark

    interaction IRTPReceiverChannelAudioForRTPReceiverChannel : public IRTPReceiverChannelMediaBaseForRTPReceiverChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelAudioForRTPReceiverChannel, ForRTPReceiverChannel)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static ElementPtr toDebug(ForRTPReceiverChannelPtr object);

      static RTPReceiverChannelAudioPtr create(
                                               RTPReceiverChannelPtr receiver,
                                               const Parameters &params
                                               );
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelAudioForMediaStreamTrack
    #pragma mark

    interaction IRTPReceiverChannelAudioForMediaStreamTrack : public IRTPReceiverChannelMediaBaseForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelAudioForMediaStreamTrack, ForMediaStreamTrack)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelAudioAsyncDelegate
    #pragma mark

    interaction IRTPReceiverChannelAudioAsyncDelegate
    {
      virtual ~IRTPReceiverChannelAudioAsyncDelegate() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelAudio
    #pragma mark
    
    class RTPReceiverChannelAudio : public Noop,
                                    public MessageQueueAssociator,
                                    public SharedRecursiveLock,
                                    public IRTPReceiverChannelAudioForSettings,
                                    public IRTPReceiverChannelAudioForRTPReceiverChannel,
                                    public IRTPReceiverChannelAudioForMediaStreamTrack,
                                    public IWakeDelegate,
                                    public zsLib::ITimerDelegate,
                                    public IRTPReceiverChannelAudioAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPReceiverChannelAudio;
      friend interaction IRTPReceiverChannelAudioFactory;
      friend interaction IRTPReceiverChannelAudioForSettings;
      friend interaction IRTPReceiverChannelMediaBaseForRTPReceiverChannel;
      friend interaction IRTPReceiverChannelAudioForRTPReceiverChannel;
      friend interaction IRTPReceiverChannelMediaBaseForMediaStreamTrack;
      friend interaction IRTPReceiverChannelAudioForMediaStreamTrack;

      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelForRTPReceiverChannelAudio, UseChannel)
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiverChannelAudio, UseMediaStreamTrack)

      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelMediaBaseForRTPReceiverChannel, ForReceiverChannelFromMediaBase)
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverChannelMediaBaseForMediaStreamTrack, ForMediaStreamTrackFromMediaBase)

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
      RTPReceiverChannelAudio(
                              const make_private &,
                              IMessageQueuePtr queue,
                              UseChannelPtr receiverChannel,
                              const Parameters &params
                              );

    protected:
      RTPReceiverChannelAudio(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~RTPReceiverChannelAudio();

      static RTPReceiverChannelAudioPtr convert(ForSettingsPtr object);
      static RTPReceiverChannelAudioPtr convert(ForReceiverChannelFromMediaBasePtr object);
      static RTPReceiverChannelAudioPtr convert(ForRTPReceiverChannelPtr object);
      static RTPReceiverChannelAudioPtr convert(ForMediaStreamTrackFromMediaBasePtr object);
      static RTPReceiverChannelAudioPtr convert(ForMediaStreamTrackPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelAudio => IRTPReceiverChannelMediaBaseForRTPReceiverChannel
      #pragma mark

      virtual PUID getID() const override {return mID;}

      virtual bool handlePacket(RTPPacketPtr packet) override;

      virtual bool handlePacket(RTCPPacketPtr packet) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelAudio => IRTPReceiverChannelAudioForRTPReceiverChannel
      #pragma mark

      static RTPReceiverChannelAudioPtr create(
                                               RTPReceiverChannelPtr receiver,
                                               const Parameters &params
                                               );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelAudio => IRTPReceiverChannelAudioForMediaStreamTrack
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelAudio => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelAudio => ITimerDelegate
      #pragma mark

      virtual void onTimer(TimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelAudio => IRTPReceiverChannelAudioAsyncDelegate
      #pragma mark

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelAudio => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepBogusDoSomething();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPReceiverChannelAudio => (data)
      #pragma mark

      AutoPUID mID;
      RTPReceiverChannelAudioWeakPtr mThisWeak;
      RTPReceiverChannelAudioPtr mGracefulShutdownReference;

      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      UseChannelWeakPtr mReceiverChannel;

      ParametersPtr mParameters;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelAudioFactory
    #pragma mark

    interaction IRTPReceiverChannelAudioFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static IRTPReceiverChannelAudioFactory &singleton();

      virtual RTPReceiverChannelAudioPtr create(
                                                RTPReceiverChannelPtr receiverChannel,
                                                const Parameters &params
                                                );
    };

    class RTPReceiverChannelAudioFactory : public IFactory<IRTPReceiverChannelAudioFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPReceiverChannelAudioAsyncDelegate)
ZS_DECLARE_PROXY_END()
