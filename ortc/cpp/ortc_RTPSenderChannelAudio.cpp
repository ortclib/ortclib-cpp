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

#include <ortc/internal/ortc_RTPSenderChannelAudio.h>
#include <ortc/internal/ortc_RTPSenderChannel.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <webrtc/voice_engine/include/voe_codec.h>
#include <webrtc/voice_engine/include/voe_rtp_rtcp.h>
#include <webrtc/voice_engine/include/voe_network.h>

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void IRTPSenderChannelAudioForSettings::applyDefaults()
    {
//      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 5*1024);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelAudioForRTPSenderChannel
    #pragma mark

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr IRTPSenderChannelAudioForRTPSenderChannel::create(
                                                                               RTPSenderChannelPtr senderChannel,
                                                                               MediaStreamTrackPtr track,
                                                                               const Parameters &params
                                                                               )
    {
      return internal::IRTPSenderChannelAudioFactory::singleton().create(senderChannel, track, params);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio
    #pragma mark
    
    //-------------------------------------------------------------------------
    const char *RTPSenderChannelAudio::toString(States state)
    {
      switch (state) {
        case State_Pending:       return "pending";
        case State_Ready:         return "ready";
        case State_ShuttingDown:  return "shutting down";
        case State_Shutdown:      return "shutdown";
      }
      return "UNDEFINED";
    }
    
    //-------------------------------------------------------------------------
    RTPSenderChannelAudio::RTPSenderChannelAudio(
                                                 const make_private &,
                                                 IMessageQueuePtr queue,
                                                 UseChannelPtr senderChannel,
                                                 UseMediaStreamTrackPtr track,
                                                 const Parameters &params
                                                 ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mSenderChannel(senderChannel),
      mTrack(track),
      mParameters(make_shared<Parameters>(params))
    {
      ZS_LOG_DETAIL(debug("created"))

      ORTC_THROW_INVALID_PARAMETERS_IF(!senderChannel)
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

      mVoiceEngine = rtc::scoped_ptr<webrtc::VoiceEngine, VoiceEngineDeleter>(webrtc::VoiceEngine::Create());

      webrtc::VoEBase::GetInterface(mVoiceEngine.get())->Init(mTrack->getAudioDeviceModule());

      mChannel = webrtc::VoEBase::GetInterface(mVoiceEngine.get())->CreateChannel();
      webrtc::VoERTP_RTCP::GetInterface(mVoiceEngine.get())->SetRTCPStatus(mChannel, true);
      webrtc::VoERTP_RTCP::GetInterface(mVoiceEngine.get())->SetLocalSSRC(mChannel, 1000);
      webrtc::VoERTP_RTCP::GetInterface(mVoiceEngine.get())->SetRTCP_CNAME(mChannel, "test-cname");

      mTransport = Transport::create(mThisWeak.lock());

      webrtc::AudioSendStream::Config config(mTransport.get());
      config.voe_channel_id = mChannel;
      config.rtp.ssrc = 1000;

      mSendStream = rtc::scoped_ptr<webrtc::AudioSendStream>(
          new webrtc::internal::AudioSendStream(
                                                config,
                                                mVoiceEngine.get()
                                                ));

      webrtc::CodecInst opusCodec;
      int ncodecs = webrtc::VoECodec::GetInterface(mVoiceEngine.get())->NumOfCodecs();
      for (int i = 0; i < ncodecs; ++i) {
        webrtc::CodecInst codec;
        webrtc::VoECodec::GetInterface(mVoiceEngine.get())->GetCodec(i, codec);
        if (0 == String(codec.plname).compareNoCase("opus")) {
          opusCodec = codec;
          break;
        }
      }

      webrtc::VoECodec::GetInterface(mVoiceEngine.get())->SetSendCodec(mChannel, opusCodec);

      webrtc::VoENetwork::GetInterface(mVoiceEngine.get())->RegisterExternalTransport(mChannel, *mTransport);

      mTrack->start();

      webrtc::VoEBase::GetInterface(mVoiceEngine.get())->StartSend(mChannel);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudio::~RTPSenderChannelAudio()
    {
      if (isNoop()) return;

      webrtc::VoENetwork::GetInterface(mVoiceEngine.get())->DeRegisterExternalTransport(mChannel);
      webrtc::VoEBase::GetInterface(mVoiceEngine.get())->StopSend(0);

      mTrack->stop();

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr RTPSenderChannelAudio::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr RTPSenderChannelAudio::convert(ForRTPSenderChannelFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr RTPSenderChannelAudio::convert(ForRTPSenderChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr RTPSenderChannelAudio::convert(ForMediaStreamTrackFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr RTPSenderChannelAudio::convert(ForMediaStreamTrackPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => IRTPSenderChannelMediaBaseForRTPSenderChannel
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::handlePacket(RTCPPacketPtr packet)
    {
      mSendStream->DeliverRtcp(packet->buffer()->data(), packet->buffer()->size());
      return true;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::handleUpdate(ParametersPtr params)
    {
#define TODO_UPDATE_PARAMETERS 1
#define TODO_UPDATE_PARAMETERS 2
      {
        AutoRecursiveLock lock(*this);
        mParameters = make_shared<Parameters>(*params);
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => IRTPSenderChannelAudioForRTPSenderChannel
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr RTPSenderChannelAudio::create(
                                                           RTPSenderChannelPtr senderChannel,
                                                           MediaStreamTrackPtr track,
                                                           const Parameters &params
                                                           )
    {
      RTPSenderChannelAudioPtr pThis(make_shared<RTPSenderChannelAudio>(make_private {}, IORTCForInternal::queueORTC(), senderChannel, track, params));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    int32_t RTPSenderChannelAudio::sendAudioSamples(
                                                    const void* audioSamples,
                                                    const size_t numberOfSamples,
                                                    const uint8_t numberOfChannels
                                                    )
    {
#define TODO_IMPLEMENT_THIS 1
#define TODO_IMPLEMENT_THIS 2
      return 0;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => IRTPSenderChannelAudioForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::onWake()
    {
      ZS_LOG_DEBUG(log("wake"))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::onTimer(TimerPtr timer)
    {
      ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()))

      AutoRecursiveLock lock(*this);
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => IRTPSenderChannelAudioAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => friend Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::SendRtp(
                                        const uint8_t* packet,
                                        size_t length,
                                        const webrtc::PacketOptions& options
                                        )
    {
      auto channel = mSenderChannel.lock();
      if (!channel) return false;
      return channel->sendPacket(RTPPacket::create(packet, length));
    }
    
    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::SendRtcp(const uint8_t* packet, size_t length)
    {
      auto channel = mSenderChannel.lock();
      if (!channel) return false;
      return channel->sendPacket(RTCPPacket::create(packet, length));
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio::Transport
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPSenderChannelAudio::Transport::Transport(
                                                const make_private &,
                                                RTPSenderChannelAudioPtr outer
                                                ) :
      mOuter(outer)
    {
    }
    
    //-------------------------------------------------------------------------
    RTPSenderChannelAudio::Transport::~Transport()
    {
      mThisWeak.reset();
    }
    
    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::Transport::init()
    {
    }
    
    //-------------------------------------------------------------------------
    RTPSenderChannelAudio::TransportPtr RTPSenderChannelAudio::Transport::create(RTPSenderChannelAudioPtr outer)
    {
      TransportPtr pThis(make_shared<Transport>(make_private{}, outer));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio::Transport => webrtc::Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::Transport::SendRtp(
                                                   const uint8_t* packet,
                                                   size_t length,
                                                   const webrtc::PacketOptions& options
                                                   )
    {
      auto outer = mOuter.lock();
      if (!outer) return false;
      return outer->SendRtp(packet, length, options);
    }
    
    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::Transport::SendRtcp(const uint8_t* packet, size_t length)
    {
      auto outer = mOuter.lock();
      if (!outer) return false;
      return outer->SendRtcp(packet, length);
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPSenderChannelAudio::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPSenderChannelAudio");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPSenderChannelAudio::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPSenderChannelAudio::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPSenderChannelAudio");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      auto senderChannel = mSenderChannel.lock();
      UseServicesHelper::debugAppend(resultEl, "sender channel", senderChannel ? senderChannel->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

      // ... other steps here ...
      if (!stepBogusDoSomething()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("not ready"))
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("ready"))
        setState(State_Ready);
      }
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelAudio::stepBogusDoSomething()
    {
      if ( /* step already done */ false ) {
        ZS_LOG_TRACE(log("already completed do something"))
        return true;
      }

      if ( /* cannot do step yet */ false) {
        ZS_LOG_DEBUG(log("waiting for XYZ to complete before continuing"))
        return false;
      }

      ZS_LOG_DEBUG(log("doing step XYZ"))

      // ....
#define TODO 1
#define TODO 2

      return true;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
//        return;
      }

      //.......................................................................
      // final cleanup

      setState(State_Shutdown);

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPSenderChannelAudioPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPSenderChannelAudioStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelAudio::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = UseHTTP::toString(UseHTTP::toStatusCode(errorCode));
      }

      if (0 != mLastError) {
        ZS_LOG_WARNING(Detail, debug("error already set thus ignoring new error") + ZS_PARAM("new error", errorCode) + ZS_PARAM("new reason", reason))
        return;
      }

      mLastError = errorCode;
      mLastErrorReason = reason;

      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelAudioFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPSenderChannelAudioFactory &IRTPSenderChannelAudioFactory::singleton()
    {
      return RTPSenderChannelAudioFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelAudioPtr IRTPSenderChannelAudioFactory::create(
                                                                   RTPSenderChannelPtr senderChannel,
                                                                   MediaStreamTrackPtr track,
                                                                   const Parameters &params
                                                                   )
    {
      if (this) {}
      return internal::RTPSenderChannelAudio::create(senderChannel, track, params);
    }

  } // internal namespace

}
