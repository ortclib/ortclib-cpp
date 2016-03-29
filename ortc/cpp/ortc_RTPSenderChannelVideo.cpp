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

#include <ortc/internal/ortc_RTPSenderChannelVideo.h>
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

#include <webrtc/system_wrappers/include/cpu_info.h>

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
    void IRTPSenderChannelVideoForSettings::applyDefaults()
    {
//      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 5*1024);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelVideoForRTPSenderChannel
    #pragma mark

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr IRTPSenderChannelVideoForRTPSenderChannel::create(
                                                                               RTPSenderChannelPtr senderChannel,
                                                                               MediaStreamTrackPtr track,
                                                                               const Parameters &params
                                                                               )
    {
      return internal::IRTPSenderChannelVideoFactory::singleton().create(senderChannel, track, params);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelVideo
    #pragma mark
    
    //-------------------------------------------------------------------------
    const char *RTPSenderChannelVideo::toString(States state)
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
    RTPSenderChannelVideo::RTPSenderChannelVideo(
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
      mParameters(make_shared<Parameters>(params)),
      mSetupChannelEvent(Event::create()),
      mCloseChannelEvent(Event::create()),
      mModuleProcessThread(webrtc::ProcessThread::Create("RTPSenderChannelVideoThread"))
    {
      ZS_LOG_DETAIL(debug("created"))

      ORTC_THROW_INVALID_PARAMETERS_IF(!senderChannel)
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::init()
    {
      {
        AutoRecursiveLock lock(*this);
        IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

        PromiseWithRTPMediaEngineSetupChannelResultPtr senderChannelPromise = UseMediaEngine::setupChannel(mThisWeak.lock());
        if (senderChannelPromise->isRejected())
          return;
        senderChannelPromise->then(mThisWeak.lock());
      }

      mSetupChannelEvent->wait();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideo::~RTPSenderChannelVideo()
    {
      if (isNoop()) return;

      PromiseWithRTPMediaEngineCloseChannelResultPtr senderChannelPromise = UseMediaEngine::closeChannel(mThisWeak.lock());
      if (senderChannelPromise->isRejected())
        return;
      senderChannelPromise->then(mThisWeak.lock());

      mCloseChannelEvent->wait();

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr RTPSenderChannelVideo::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr RTPSenderChannelVideo::convert(ForRTPSenderChannelFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr RTPSenderChannelVideo::convert(ForRTPSenderChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr RTPSenderChannelVideo::convert(ForMediaStreamTrackFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr RTPSenderChannelVideo::convert(ForMediaStreamTrackPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelVideo => IRTPSenderChannelMediaBaseForRTPSenderChannel
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::onTrackChanged(UseBaseMediaStreamTrackPtr track)
    {
      AutoRecursiveLock lock(*this);
      mTrack = MediaStreamTrack::convert(track);  // NOTE: track might be NULL
#define TODO_HANDLE_CHANGE_IN_TRACK 1
#define TODO_HANDLE_CHANGE_IN_TRACK 2
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::notifyTransportState(ISecureTransportTypes::States state)
    {
#define TODO_HANDLE_CHANGE_IN_CONNECTIVITY 1
#define TODO_HANDLE_CHANGE_IN_CONNECTIVITY 2
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelVideo::handlePacket(RTCPPacketPtr packet)
    {
      {
        AutoRecursiveLock lock(*this);
      }
      if (mSendStream)
        mSendStream->DeliverRtcp(packet->buffer()->data(), packet->buffer()->size());
      return true;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::handleUpdate(ParametersPtr params)
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
    #pragma mark RTPSenderChannelVideo => IRTPSenderChannelVideoForRTPSenderChannel
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr RTPSenderChannelVideo::create(
                                                           RTPSenderChannelPtr senderChannel,
                                                           MediaStreamTrackPtr track,
                                                           const Parameters &params
                                                           )
    {
      RTPSenderChannelVideoPtr pThis(make_shared<RTPSenderChannelVideo>(make_private {}, IORTCForInternal::queueORTC(), senderChannel, track, params));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelVideo => IRTPSenderChannelVideoForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::sendVideoFrame(const webrtc::VideoFrame& videoFrame)
    {
      if (!mSendStream) return;
      mSendStream->Input()->IncomingCapturedFrame(videoFrame);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelVideo => IRTPSenderChannelMediaBaseForRTPMediaEngine
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::setupChannel()
    {
      AutoRecursiveLock lock(*this);

      if (!mTrack) {
        ZS_LOG_ERROR(Detail, log("MediaStreamTrack is not set during RTPSenderChannelVideo initialization procedure"))
          return;
      }

      mCallStats = rtc::scoped_ptr<webrtc::CallStats>(new webrtc::CallStats());
      mCongestionController =
        rtc::scoped_ptr<webrtc::CongestionController>(new webrtc::CongestionController(
                                                                                       mModuleProcessThread.get(),
                                                                                       mCallStats.get())
                                                                                       );

      mModuleProcessThread->Start();

      int numCpuCores = webrtc::CpuInfo::DetectNumberOfCores();

      mTransport = Transport::create(mThisWeak.lock());

      webrtc::VideoSendStream::Config config(mTransport.get());
      webrtc::VideoEncoderConfig encoderConfig;
      std::map<uint32_t, webrtc::RtpState> suspendedSSRCs;

      IRTPTypes::CodecParametersList::iterator codecIter = mParameters->mCodecs.begin();
      while (codecIter != mParameters->mCodecs.end()) {
        auto supportedCodec = IRTPTypes::toSupportedCodec(codecIter->mName);
        if (IRTPTypes::SupportedCodec_VP8 == supportedCodec) {
          webrtc::VideoEncoder* videoEncoder = webrtc::VideoEncoder::Create(webrtc::VideoEncoder::kVp8);
          config.encoder_settings.encoder = videoEncoder;
          config.encoder_settings.payload_name = codecIter->mName;
          config.encoder_settings.payload_type = codecIter->mPayloadType;
          webrtc::VideoStream stream;
          stream.width = 640;
          stream.height = 480;
          stream.max_framerate = 30;
          stream.min_bitrate_bps = 30000;
          stream.target_bitrate_bps = 2000000;
          stream.max_bitrate_bps = 2000000;
          stream.max_qp = 56;
          webrtc::VideoCodecVP8 videoCodec = webrtc::VideoEncoder::GetDefaultVp8Settings();
          videoCodec.automaticResizeOn = true;
          videoCodec.denoisingOn = true;
          videoCodec.frameDroppingOn = true;
          encoderConfig.min_transmit_bitrate_bps = 0;
          encoderConfig.content_type = webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo;
          encoderConfig.streams.push_back(stream);
          encoderConfig.encoder_specific_settings = &videoCodec;
          break;
        } else if (IRTPTypes::SupportedCodec_VP9 == supportedCodec) {
          webrtc::VideoEncoder* videoEncoder = webrtc::VideoEncoder::Create(webrtc::VideoEncoder::kVp9);
          config.encoder_settings.encoder = videoEncoder;
          config.encoder_settings.payload_name = codecIter->mName;
          config.encoder_settings.payload_type = codecIter->mPayloadType;
          webrtc::VideoStream stream;
          stream.width = 640;
          stream.height = 480;
          stream.max_framerate = 30;
          stream.min_bitrate_bps = 30000;
          stream.target_bitrate_bps = 2000000;
          stream.max_bitrate_bps = 2000000;
          stream.max_qp = 56;
          webrtc::VideoCodecVP9 videoCodec = webrtc::VideoEncoder::GetDefaultVp9Settings();
          videoCodec.frameDroppingOn = true;
          encoderConfig.min_transmit_bitrate_bps = 0;
          encoderConfig.content_type = webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo;
          encoderConfig.streams.push_back(stream);
          encoderConfig.encoder_specific_settings = &videoCodec;
          break;
        } else if (IRTPTypes::SupportedCodec_H264 == supportedCodec) {
          webrtc::VideoEncoder* videoEncoder = webrtc::VideoEncoder::Create(webrtc::VideoEncoder::kH264);
          config.encoder_settings.encoder = videoEncoder;
          config.encoder_settings.payload_name = codecIter->mName;
          config.encoder_settings.payload_type = codecIter->mPayloadType;
          webrtc::VideoStream stream;
          stream.width = 640;
          stream.height = 480;
          stream.max_framerate = 30;
          stream.min_bitrate_bps = 30000;
          stream.target_bitrate_bps = 2000000;
          stream.max_bitrate_bps = 2000000;
          stream.max_qp = 56;
          webrtc::VideoCodecH264 videoCodec = webrtc::VideoEncoder::GetDefaultH264Settings();
          videoCodec.frameDroppingOn = true;
          encoderConfig.min_transmit_bitrate_bps = 0;
          encoderConfig.content_type = webrtc::VideoEncoderConfig::ContentType::kRealtimeVideo;
          encoderConfig.streams.push_back(stream);
          encoderConfig.encoder_specific_settings = &videoCodec;
          break;
        }
        IRTPTypes::RTCPFeedbackList::iterator rtcpFeedbackIter = codecIter->mRTCPFeedback.begin();
        while (rtcpFeedbackIter != codecIter->mRTCPFeedback.end()) {
          IRTPTypes::KnownFeedbackTypes feedbackType = IRTPTypes::toKnownFeedbackType(rtcpFeedbackIter->mType);
          IRTPTypes::KnownFeedbackParameters feedbackParameter = IRTPTypes::toKnownFeedbackParameter(rtcpFeedbackIter->mParameter);
          if (IRTPTypes::KnownFeedbackType_NACK == feedbackType && IRTPTypes::KnownFeedbackParameter_Unknown == feedbackParameter) {
            config.rtp.nack.rtp_history_ms = 1000;
          }
          rtcpFeedbackIter++;
        }
        codecIter++;
      }

      IRTPTypes::EncodingParametersList::iterator encodingParamIter = mParameters->mEncodings.begin();
      while (encodingParamIter != mParameters->mEncodings.end()) {
        if (encodingParamIter->mCodecPayloadType == config.encoder_settings.payload_type) {
          config.rtp.ssrcs.push_back(encodingParamIter->mSSRC);
          break;
        }
        encodingParamIter++;
      }
      if (config.rtp.ssrcs.size() == 0)
        config.rtp.ssrcs.push_back(1000);

      IRTPTypes::HeaderExtensionParametersList::iterator headerExtensionIter = mParameters->mHeaderExtensions.begin();
      while (headerExtensionIter != mParameters->mHeaderExtensions.end()) {
        IRTPTypes::HeaderExtensionURIs headerExtensionURI = IRTPTypes::toHeaderExtensionURI(headerExtensionIter->mURI);
        switch (headerExtensionURI) {
        case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_TransmissionTimeOffsets:
        case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_AbsoluteSendTime:
        case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_3gpp_VideoOrientation:
        case IRTPTypes::HeaderExtensionURIs::HeaderExtensionURI_TransportSequenceNumber:
          config.rtp.extensions.push_back(webrtc::RtpExtension(headerExtensionIter->mURI, headerExtensionIter->mID));
          break;
        default:
          break;
        }
        headerExtensionIter++;
      }

      config.rtp.c_name = mParameters->mRTCP.mCName;

      mSendStream = rtc::scoped_ptr<webrtc::VideoSendStream>(
        new webrtc::internal::VideoSendStream(
                                              numCpuCores,
                                              mModuleProcessThread.get(),
                                              mCallStats.get(),
                                              mCongestionController.get(),
                                              config,
                                              encoderConfig,
                                              suspendedSSRCs
                                              ));

      mSendStream->Start();
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::closeChannel()
    {
      if (mSendStream)
        mSendStream->Stop();

      mModuleProcessThread->Stop();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelVideo => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::onWake()
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
    #pragma mark RTPSenderChannelVideo => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::onTimer(TimerPtr timer)
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
    void RTPSenderChannelVideo::onPromiseSettled(PromisePtr promise)
    {
      ZS_LOG_DEBUG(log("promise settled") + ZS_PARAM("promise", promise->getID()))

      AutoRecursiveLock lock(*this);
      step();

      if (ZS_DYNAMIC_PTR_CAST(PromiseWithRTPMediaEngineSetupChannelResult, promise)) {
        mSetupChannelEvent->notify();
        mSetupChannelEvent->reset();
      } else if (ZS_DYNAMIC_PTR_CAST(PromiseWithRTPMediaEngineCloseChannelResult, promise)) {
        mCloseChannelEvent->notify();
        mCloseChannelEvent->reset();
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelVideo => IRTPSenderChannelVideoAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannelVideo => friend Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPSenderChannelVideo::SendRtp(
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
    bool RTPSenderChannelVideo::SendRtcp(const uint8_t* packet, size_t length)
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
    #pragma mark RTPSenderChannelVideo::Transport
    #pragma mark

    //-------------------------------------------------------------------------
    RTPSenderChannelVideo::Transport::Transport(
                                                const make_private &,
                                                RTPSenderChannelVideoPtr outer
                                                ) :
      mOuter(outer)
    {
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideo::Transport::~Transport()
    {
      mThisWeak.reset();
    }
    
    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::Transport::init()
    {
    }
    
    //-------------------------------------------------------------------------
    RTPSenderChannelVideo::TransportPtr RTPSenderChannelVideo::Transport::create(RTPSenderChannelVideoPtr outer)
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
    #pragma mark RTPSenderChannelVideo::Transport => webrtc::Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool RTPSenderChannelVideo::Transport::SendRtp(
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
    bool RTPSenderChannelVideo::Transport::SendRtcp(const uint8_t* packet, size_t length)
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
    #pragma mark RTPSenderChannelVideo => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPSenderChannelVideo::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPSenderChannelVideo");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPSenderChannelVideo::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPSenderChannelVideo::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPSenderChannelVideo");

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
    bool RTPSenderChannelVideo::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelVideo::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

      // ... other steps here ...
      if (!stepPromiseEngine()) goto not_ready;
      if (!stepPromiseExampleDeviceResource()) goto not_ready;
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
    bool RTPSenderChannelVideo::stepPromiseEngine()
    {
      if (mMediaEngine) {
        ZS_LOG_TRACE(log("already setup engine"))
          return true;
      }

      if (!mMediaEnginePromise) {
        mMediaEnginePromise = UseMediaEngine::create();
      }

      if (!mMediaEnginePromise->isSettled()) {
        ZS_LOG_TRACE(log("waiting for media engine promise to resolve"))
          return false;
      }

      mMediaEngine = mMediaEnginePromise->value();

      if (!mMediaEngine) {
        ZS_LOG_WARNING(Detail, log("failed to initialize media"))
          cancel();
        return false;
      }

      ZS_LOG_DEBUG(log("media engine is setup") + ZS_PARAM("engine", mMediaEngine->getID()))
        return true;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelVideo::stepPromiseExampleDeviceResource()
    {
      if (mDeviceResource) {
        ZS_LOG_TRACE(log("already setup device resource"))
          return true;
      }

      if (!mDeviceResourcePromise) {
        mDeviceResourcePromise = UseMediaEngine::getDeviceResource("camera");
      }

      if (!mDeviceResourcePromise->isSettled()) {
        ZS_LOG_TRACE(log("waiting for media device resource promise to resolve"))
          return false;
      }

      mDeviceResource = mDeviceResourcePromise->value();

      if (!mDeviceResource) {
        ZS_LOG_WARNING(Detail, log("failed to initialize device resource"))
          cancel();
        return false;
      }

      ZS_LOG_DEBUG(log("media device is setup") + ZS_PARAM("device", mDeviceResource->getDeviceID()))
        return true;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannelVideo::stepSetup()
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
    void RTPSenderChannelVideo::cancel()
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

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPSenderChannelVideoPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPSenderChannelVideoStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannelVideo::setError(WORD errorCode, const char *inReason)
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
    #pragma mark IRTPSenderChannelVideoFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPSenderChannelVideoFactory &IRTPSenderChannelVideoFactory::singleton()
    {
      return RTPSenderChannelVideoFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelVideoPtr IRTPSenderChannelVideoFactory::create(
                                                                   RTPSenderChannelPtr senderChannel,
                                                                   MediaStreamTrackPtr track,
                                                                   const Parameters &params
                                                                   )
    {
      if (this) {}
      return internal::RTPSenderChannelVideo::create(senderChannel, track, params);
    }

  } // internal namespace

}
