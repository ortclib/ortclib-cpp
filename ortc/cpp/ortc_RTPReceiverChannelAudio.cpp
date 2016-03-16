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

#include <ortc/internal/ortc_RTPReceiverChannelAudio.h>
#include <ortc/internal/ortc_RTPReceiverChannel.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#include <webrtc/system_wrappers/include/cpu_info.h>
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
  ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper)
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
    void IRTPReceiverChannelAudioForSettings::applyDefaults()
    {
//      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 5*1024);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelAudioForRTPReceiverChannel
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPReceiverChannelAudioForRTPReceiverChannel::toDebug(ForRTPReceiverChannelPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr IRTPReceiverChannelAudioForRTPReceiverChannel::create(
                                                                                     RTPReceiverChannelPtr receiverChannel,
                                                                                     MediaStreamTrackPtr track,
                                                                                     const Parameters &params
                                                                                     )
    {
      return internal::IRTPReceiverChannelAudioFactory::singleton().create(receiverChannel, track, params);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelAudioForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPReceiverChannelAudioForMediaStreamTrack::toDebug(ForMediaStreamTrackPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object)->toDebug();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelAudio
    #pragma mark
    
    //---------------------------------------------------------------------------
    const char *RTPReceiverChannelAudio::toString(States state)
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
    RTPReceiverChannelAudio::RTPReceiverChannelAudio(
                                                     const make_private &,
                                                     IMessageQueuePtr queue,
                                                     UseChannelPtr receiverChannel,
                                                     UseMediaStreamTrackPtr track,
                                                     const Parameters &params
                                                     ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mReceiverChannel(receiverChannel),
      mTrack(track),
      mParameters(make_shared<Parameters>(params)),
      mModuleProcessThread(webrtc::ProcessThread::Create("RTPReceiverChannelAudioThread"))
    {
      ZS_LOG_DETAIL(debug("created"))

      ORTC_THROW_INVALID_PARAMETERS_IF(!receiverChannel)
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

#define FIX_ME_MOVE_THIS_AS_POST_MEDIA_ENGINE_READY_STEP 1
#define FIX_ME_MOVE_THIS_AS_POST_MEDIA_ENGINE_READY_STEP 2

      mTransport = Transport::create(mThisWeak.lock());

      mVoiceEngine = rtc::scoped_ptr<webrtc::VoiceEngine, VoiceEngineDeleter>(webrtc::VoiceEngine::Create());
      mCallStats = rtc::scoped_ptr<webrtc::CallStats>(new webrtc::CallStats());
      mCongestionController = 
        rtc::scoped_ptr<webrtc::CongestionController>(new webrtc::CongestionController(
                                                                                       mModuleProcessThread.get(),
                                                                                       mCallStats.get())
                                                                                       );

      mModuleProcessThread->Start();
      mModuleProcessThread->RegisterModule(mCallStats.get());

      webrtc::VoEBase::GetInterface(mVoiceEngine.get())->Init(mTrack->getAudioDeviceModule());

      mChannel = webrtc::VoEBase::GetInterface(mVoiceEngine.get())->CreateChannel();
      
      webrtc::CodecInst codec;
      IRTPTypes::CodecParametersList::iterator codecIter = mParameters->mCodecs.begin();
      while (codecIter != mParameters->mCodecs.end()) {
        auto supportedCodec = IRTPTypes::toSupportedCodec(codecIter->mName);
        if (IRTPTypes::SupportedCodec_Opus == supportedCodec) {
          codec = getAudioCodec(codecIter->mName);
          webrtc::VoECodec::GetInterface(mVoiceEngine.get())->SetRecPayloadType(mChannel, codec);
          break;
        } else if (IRTPTypes::SupportedCodec_Isac == supportedCodec) {
          codec = getAudioCodec(codecIter->mName);
          webrtc::VoECodec::GetInterface(mVoiceEngine.get())->SetRecPayloadType(mChannel, codec);
          break;
        } else if (IRTPTypes::SupportedCodec_G722 == supportedCodec) {
          codec = getAudioCodec(codecIter->mName);
          webrtc::VoECodec::GetInterface(mVoiceEngine.get())->SetRecPayloadType(mChannel, codec);
          break;
        } else if (IRTPTypes::SupportedCodec_ILBC == supportedCodec) {
          codec = getAudioCodec(codecIter->mName);
          webrtc::VoECodec::GetInterface(mVoiceEngine.get())->SetRecPayloadType(mChannel, codec);
          break;
        } else if (IRTPTypes::SupportedCodec_PCMU == supportedCodec) {
          codec = getAudioCodec(codecIter->mName);
          webrtc::VoECodec::GetInterface(mVoiceEngine.get())->SetRecPayloadType(mChannel, codec);
          break;
        } else if (IRTPTypes::SupportedCodec_PCMA == supportedCodec) {
          codec = getAudioCodec(codecIter->mName);
          webrtc::VoECodec::GetInterface(mVoiceEngine.get())->SetRecPayloadType(mChannel, codec);
          break;
        }
        IRTPTypes::RTCPFeedbackList::iterator rtcpFeedbackIter = codecIter->mRTCPFeedback.begin();
        while (rtcpFeedbackIter != codecIter->mRTCPFeedback.end()) {
          IRTPTypes::KnownFeedbackTypes feedbackType = IRTPTypes::toKnownFeedbackType(rtcpFeedbackIter->mType);
          IRTPTypes::KnownFeedbackParameters feedbackParameter = IRTPTypes::toKnownFeedbackParameter(rtcpFeedbackIter->mParameter);
          if (IRTPTypes::KnownFeedbackType_NACK == feedbackType && IRTPTypes::KnownFeedbackParameter_Unknown == feedbackParameter) {
            webrtc::VoERTP_RTCP::GetInterface(mVoiceEngine.get())->SetNACKStatus(mChannel, true, 250);
          }
          rtcpFeedbackIter++;
        }
        codecIter++;
      }

      webrtc::AudioReceiveStream::Config config;
      config.voe_channel_id = mChannel;
      
      IRTPTypes::EncodingParametersList::iterator encodingParamIter = mParameters->mEncodings.begin();
      while (encodingParamIter != mParameters->mEncodings.end()) {
        if (encodingParamIter->mCodecPayloadType == codec.pltype) {
          config.rtp.remote_ssrc = encodingParamIter->mSSRC;
          break;
        }
        encodingParamIter++;
      }

      IRTPTypes::HeaderExtensionParametersList::iterator headerExtensionIter = mParameters->mHeaderExtensions.begin();
      while (headerExtensionIter != mParameters->mHeaderExtensions.end()) {
        IRTPTypes::HeaderExtensionURIs headerExtensionURI = IRTPTypes::toHeaderExtensionURI(headerExtensionIter->mURI);
        switch (headerExtensionURI) {
        case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_ClienttoMixerAudioLevelIndication:
          webrtc::VoERTP_RTCP::GetInterface(mVoiceEngine.get())->SetReceiveAudioLevelIndicationStatus(mChannel, true, headerExtensionIter->mID);
          config.rtp.extensions.push_back(webrtc::RtpExtension(headerExtensionIter->mURI, headerExtensionIter->mID));
          break;
        case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_AbsoluteSendTime:
          webrtc::VoERTP_RTCP::GetInterface(mVoiceEngine.get())->SetReceiveAbsoluteSenderTimeStatus(mChannel, true, headerExtensionIter->mID);
          config.rtp.extensions.push_back(webrtc::RtpExtension(headerExtensionIter->mURI, headerExtensionIter->mID));
          break;
        default:
          break;
        }
        headerExtensionIter++;
      }

      webrtc::VoERTP_RTCP::GetInterface(mVoiceEngine.get())->SetLocalSSRC(mChannel, mParameters->mRTCP.mSSRC);
      config.rtp.local_ssrc = mParameters->mRTCP.mSSRC;
      config.receive_transport = mTransport.get();
      config.rtcp_send_transport = mTransport.get();
      config.combined_audio_video_bwe = true;

      mReceiveStream = rtc::scoped_ptr<webrtc::AudioReceiveStream>(
          new webrtc::internal::AudioReceiveStream(
                                                   mCongestionController->GetRemoteBitrateEstimator(false),
                                                   config,
                                                   mVoiceEngine.get()
                                                   ));

      webrtc::VoENetwork::GetInterface(mVoiceEngine.get())->RegisterExternalTransport(mChannel, *mTransport);

      mTrack->start();

      webrtc::VoEBase::GetInterface(mVoiceEngine.get())->StartReceive(mChannel);
      webrtc::VoEBase::GetInterface(mVoiceEngine.get())->StartPlayout(mChannel);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudio::~RTPReceiverChannelAudio()
    {
      if (isNoop()) return;

      if (mVoiceEngine) {
        webrtc::VoEBase::GetInterface(mVoiceEngine.get())->StopPlayout(mChannel);
        webrtc::VoEBase::GetInterface(mVoiceEngine.get())->StopReceive(mChannel);
        webrtc::VoENetwork::GetInterface(mVoiceEngine.get())->DeRegisterExternalTransport(mChannel);
      }

#define FIX_ME_WARNING_NO_TRACK_IS_NOT_STOPPED_JUST_BECAUSE_A_RECEIVER_CHANNEL_IS_DONE 1
#define FIX_ME_WARNING_NO_TRACK_IS_NOT_STOPPED_JUST_BECAUSE_A_RECEIVER_CHANNEL_IS_DONE 2

      if (mTrack)
        mTrack->stop();

      mModuleProcessThread->Stop();

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr RTPReceiverChannelAudio::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr RTPReceiverChannelAudio::convert(ForReceiverChannelFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr RTPReceiverChannelAudio::convert(ForRTPReceiverChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr RTPReceiverChannelAudio::convert(ForMediaStreamTrackFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr RTPReceiverChannelAudio::convert(ForMediaStreamTrackPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelAudio, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelAudio => IRTPReceiverChannelMediaBaseForRTPReceiverChannel
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::notifyTransportState(ISecureTransportTypes::States state)
    {
#define TODO_HANDLE_CHANGE_IN_CONNECTIVITY 1
#define TODO_HANDLE_CHANGE_IN_CONNECTIVITY 2
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::handlePacket(RTPPacketPtr packet)
    {
      {
        AutoRecursiveLock lock(*this);
      }
      webrtc::PacketTime time(packet->timestamp(), 0);
      //mReceiveStream->DeliverRtp(packet->buffer()->data(), packet->buffer()->size(), time);
      if (mVoiceEngine) {
        webrtc::VoENetwork::GetInterface(mVoiceEngine.get())->ReceivedRTPPacket(
          mChannel, packet->buffer()->data(), packet->buffer()->size(), time);
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::handlePacket(RTCPPacketPtr packet)
    
    {
      {
        AutoRecursiveLock lock(*this);
      }
      //mReceiveStream->DeliverRtcp(packet->buffer()->data(), packet->buffer()->size());
      if (mVoiceEngine) {
        webrtc::VoENetwork::GetInterface(mVoiceEngine.get())->ReceivedRTCPPacket(
          mChannel, packet->buffer()->data(), packet->buffer()->size());
      }
      return true;
    }
    
    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::handleUpdate(ParametersPtr params)
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
    #pragma mark RTPReceiverChannelAudio => IRTPReceiverChannelAudioForRTPReceiverChannel
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr RTPReceiverChannelAudio::create(
                                                               RTPReceiverChannelPtr receiverChannel,
                                                               MediaStreamTrackPtr track,
                                                               const Parameters &params
                                                               )
    {
      RTPReceiverChannelAudioPtr pThis(make_shared<RTPReceiverChannelAudio>(make_private {}, IORTCForInternal::queueORTC(), receiverChannel, track, params));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    int32_t RTPReceiverChannelAudio::getAudioSamples(
                                                     const size_t numberOfSamples,
                                                     const uint8_t numberOfChannels,
                                                     void *audioSamples,
                                                     size_t& numberOfSamplesOut
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
    #pragma mark RTPReceiverChannelAudio => IRTPReceiverChannelAudioForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelAudio => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::onWake()
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
    #pragma mark RTPReceiverChannelAudio => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::onTimer(TimerPtr timer)
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
    #pragma mark RTPReceiverChannelAudio => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::onPromiseSettled(PromisePtr promise)
    {
      ZS_LOG_DEBUG(log("promise settled") + ZS_PARAM("promise", promise->getID()))

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelAudio => IRTPReceiverChannelAudioAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelAudio => friend Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::SendRtcp(const uint8_t* packet, size_t length)
    {
      auto channel = mReceiverChannel.lock();
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
    RTPReceiverChannelAudio::Transport::Transport(
                                                  const make_private &,
                                                  RTPReceiverChannelAudioPtr outer
                                                  ) :
      mOuter(outer)
    {
    }
        
    //-------------------------------------------------------------------------
    RTPReceiverChannelAudio::Transport::~Transport()
    {
      mThisWeak.reset();
    }
    
    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::Transport::init()
    {
    }
    
    //-------------------------------------------------------------------------
    RTPReceiverChannelAudio::TransportPtr RTPReceiverChannelAudio::Transport::create(RTPReceiverChannelAudioPtr outer)
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
    bool RTPReceiverChannelAudio::Transport::SendRtp(
                                                     const uint8_t* packet,
                                                     size_t length,
                                                     const webrtc::PacketOptions& options
                                                     )
    {
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::Transport::SendRtcp(const uint8_t* packet, size_t length)
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
    #pragma mark RTPReceiverChannelAudio => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPReceiverChannelAudio::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPReceiverChannelAudio");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPReceiverChannelAudio::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPReceiverChannelAudio::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPReceiverChannelAudio");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      auto receiverChannel = mReceiverChannel.lock();
      UseServicesHelper::debugAppend(resultEl, "receiver channel", receiverChannel ? receiverChannel->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

      // ... other steps here ...
      if (!stepPromise()) goto not_ready;
      if (!stepSetup()) goto not_ready;
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
    bool RTPReceiverChannelAudio::stepPromise()
    {
      if (mMediaEngine) {
        ZS_LOG_TRACE(log("already setup engine"))
        return true;
      }

      if (!mMediaEnginePromise) {
        mMediaEnginePromise = UseMediaEngine::create();
      }

      if (!mMediaEngineRegistration) {
        if (!mMediaEnginePromise->isSettled()) {
          ZS_LOG_TRACE(log("waiting for media engine promise to resolve"))
          return false;
        }

        mMediaEngineRegistration = mMediaEnginePromise->value();
        if (!mMediaEnginePromise) {
          ZS_LOG_WARNING(Detail, log("failed to initialize media"))
          cancel();
          return false;
        }

      }

      mMediaEngine = mMediaEngineRegistration->engine<UseMediaEngine>();

      if (!mMediaEngine) {
        ZS_LOG_WARNING(Detail, log("failed to initialize media"))
        cancel();
        return false;
      }

      ZS_LOG_DEBUG(log("media engine is setup") + ZS_PARAM("engine", mMediaEngine->getID()))
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelAudio::stepSetup()
    {
#define TODO_IMPLEMENT_THIS 1
#define TODO_IMPLEMENT_THIS 2
      return false; // NOTE: return true if setup is complete
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(State_ShuttingDown);

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
//        return;
      }

      //.......................................................................
      // final cleanup

      setState(State_Shutdown);

      // cannot hold any more references to the media engine registration or
      // the media engine itself
      mMediaEngine.reset();
      mMediaEngineRegistration.reset();

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPReceiverChannelAudioPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPReceiverChannelAudioStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelAudio::setError(WORD errorCode, const char *inReason)
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
    webrtc::CodecInst RTPReceiverChannelAudio::getAudioCodec(String payloadName)
    {
      webrtc::CodecInst codec;
      int numOfCodecs = webrtc::VoECodec::GetInterface(mVoiceEngine.get())->NumOfCodecs();
      for (int i = 0; i < numOfCodecs; ++i) {
        webrtc::CodecInst currentCodec;
        webrtc::VoECodec::GetInterface(mVoiceEngine.get())->GetCodec(i, currentCodec);
        if (0 == String(currentCodec.plname).compareNoCase(payloadName)) {
          codec = currentCodec;
          break;
        }
      }
      return codec;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelAudioFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPReceiverChannelAudioFactory &IRTPReceiverChannelAudioFactory::singleton()
    {
      return RTPReceiverChannelAudioFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelAudioPtr IRTPReceiverChannelAudioFactory::create(
                                                                       RTPReceiverChannelPtr receiverChannel,
                                                                       MediaStreamTrackPtr track,
                                                                       const Parameters &params
                                                                       )
    {
      if (this) {}
      return internal::RTPReceiverChannelAudio::create(receiverChannel, track, params);
    }

  } // internal namespace
}
