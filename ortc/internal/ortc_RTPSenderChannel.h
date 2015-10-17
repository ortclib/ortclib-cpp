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

#include <ortc/IDTLSTransport.h>
#include <ortc/IICETransport.h>
#include <ortc/IRTPTypes.h>

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Timer.h>

#include <webrtc/base/scoped_ptr.h>
#include <webrtc/modules/utility/interface/process_thread.h>
#include <webrtc/Transport.h>
#include <webrtc/video/transport_adapter.h>
#include <webrtc/video_engine/vie_channel_group.h>
#include <webrtc/video_send_stream.h>
#include <webrtc/modules/video_capture/include/video_capture.h>


//#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForSettings)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForRTPSender)
    ZS_DECLARE_INTERACTION_PTR(IRTPSenderChannelForMediaStreamTrack)

    ZS_DECLARE_INTERACTION_PTR(IRTPSenderForRTPSenderChannel)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSenderChannel)

    ZS_DECLARE_INTERACTION_PROXY(IRTPSenderChannelAsyncDelegate)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelForSettings
    #pragma mark

    interaction IRTPSenderChannelForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IRTPSenderChannelForSettings() {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelForRTPSender
    #pragma mark

    interaction IRTPSenderChannelForRTPSender
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForRTPSender, ForRTPSender)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static ElementPtr toDebug(ForRTPSenderPtr object);

      static RTPSenderChannelPtr create(
                                        RTPSenderPtr sender,
                                        const Parameters &params
                                        );

      virtual PUID getID() const = 0;

      virtual void update(const Parameters &params) = 0;

      virtual bool handlePacket(RTCPPacketPtr packet) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelForMediaStreamTrack
    #pragma mark

    interaction IRTPSenderChannelForMediaStreamTrack
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderChannelForMediaStreamTrack, ForMediaStreamTrack)

      static ElementPtr toDebug(ForMediaStreamTrackPtr object);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelAsyncDelegate
    #pragma mark

    interaction IRTPSenderChannelAsyncDelegate
    {
      virtual ~IRTPSenderChannelAsyncDelegate() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel
    #pragma mark
    
    class RTPSenderChannel : public Noop,
                             public MessageQueueAssociator,
                             public SharedRecursiveLock,
                             public IRTPSenderChannelForSettings,
                             public IRTPSenderChannelForRTPSender,
                             public IRTPSenderChannelForMediaStreamTrack,
                             public IWakeDelegate,
                             public zsLib::ITimerDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IRTPSenderChannel;
      friend interaction IRTPSenderChannelFactory;
      friend interaction IRTPSenderChannelForSettings;
      friend interaction IRTPSenderChannelForRTPSender;
      friend interaction IRTPSenderChannelForMediaStreamTrack;

      ZS_DECLARE_TYPEDEF_PTR(IRTPSenderForRTPSenderChannel, UseSender)
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannel, UseMediaStreamTrack)

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      enum States
      {
        State_Pending,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,
      };
      static const char *toString(States state);

    public:
      RTPSenderChannel(
                       const make_private &,
                       IMessageQueuePtr queue,
                       UseSenderPtr sender,
                       const Parameters &params
                       );

    protected:
      RTPSenderChannel(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~RTPSenderChannel();

      static RTPSenderChannelPtr convert(ForSettingsPtr object);
      static RTPSenderChannelPtr convert(ForRTPSenderPtr object);
      static RTPSenderChannelPtr convert(ForMediaStreamTrackPtr object);


    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannel => IRTPSenderChannelForRTPSender
      #pragma mark

      static ElementPtr toDebug(RTPSenderChannelPtr receiver);

      static RTPSenderChannelPtr create(
                                        RTPSenderPtr sender,
                                        const Parameters &params
                                        );

      virtual PUID getID() const override {return mID;}

      virtual void update(const Parameters &params) override;

      virtual bool handlePacket(RTCPPacketPtr packet) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannel => IRTPSenderChannelForMediaStreamTrack
      #pragma mark

      // (duplicate) static ElementPtr toDebug(ForMediaStreamTrackPtr receiver);

      // (duplicate) virtual PUID getID() const = 0;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannel => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannel => ITimerDelegate
      #pragma mark

      virtual void onTimer(TimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannel => IRTPSenderChannelAsyncDelegate
      #pragma mark

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSenderChannel => (internal)
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
      #pragma mark RTPSenderChannel => (data)
      #pragma mark

      AutoPUID mID;
      RTPSenderChannelWeakPtr mThisWeak;
      RTPSenderChannelPtr mGracefulShutdownReference;

      States mCurrentState {State_Pending};

      WORD mLastError {};
      String mLastErrorReason;

      UseSenderWeakPtr mSender;

      ParametersPtr mParameters;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelFactory
    #pragma mark

    interaction IRTPSenderChannelFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters)

      static IRTPSenderChannelFactory &singleton();

      virtual RTPSenderChannelPtr create(
                                         RTPSenderPtr sender,
                                         const Parameters &params
                                         );
    };

    class RTPSenderChannelFactory : public IFactory<IRTPSenderChannelFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IRTPSenderChannelAsyncDelegate)
//ZS_DECLARE_PROXY_METHOD_0(onWhatever)
ZS_DECLARE_PROXY_END()
