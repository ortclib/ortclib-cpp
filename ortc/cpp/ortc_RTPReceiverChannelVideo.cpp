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

#include <ortc/internal/ortc_RTPReceiverChannelVideo.h>
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
    void IRTPReceiverChannelVideoForSettings::applyDefaults()
    {
//      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 5*1024);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelVideo::ReceiverVideoRenderer
    #pragma mark

    //---------------------------------------------------------------------------
    void RTPReceiverChannelVideo::ReceiverVideoRenderer::setMediaStreamTrack(UseMediaStreamTrackPtr videoTrack)
    {
      mVideoTrack = videoTrack;
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::ReceiverVideoRenderer::RenderFrame(const webrtc::VideoFrame& video_frame, int time_to_render_ms)
    {
      mVideoTrack->renderVideoFrame(video_frame);
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelVideo::ReceiverVideoRenderer::IsTextureSupported() const
    {
      return false;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelVideoForRTPReceiverChannel
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPReceiverChannelVideoForRTPReceiverChannel::toDebug(ForRTPReceiverChannelPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr IRTPReceiverChannelVideoForRTPReceiverChannel::create(
                                                                                     RTPReceiverChannelPtr receiverChannel,
                                                                                     MediaStreamTrackPtr track,
                                                                                     const Parameters &params
                                                                                     )
    {
      return internal::IRTPReceiverChannelVideoFactory::singleton().create(receiverChannel, track, params);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPReceiverChannelVideoForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPReceiverChannelVideoForMediaStreamTrack::toDebug(ForMediaStreamTrackPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object)->toDebug();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelVideo
    #pragma mark
    
    //---------------------------------------------------------------------------
    const char *RTPReceiverChannelVideo::toString(States state)
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
    RTPReceiverChannelVideo::RTPReceiverChannelVideo(
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
      mModuleProcessThread(webrtc::ProcessThread::Create("RTPReceiverChannelVideoThread"))
    {
      ZS_LOG_DETAIL(debug("created"))

      ORTC_THROW_INVALID_PARAMETERS_IF(!receiverChannel)
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
      
      if (!mTrack) {
        ZS_LOG_ERROR(Detail, log("MediaStreamTrack is not set during RTPReceiverChannelVideo initialization procedure"))
        return;
      }

      mReceiverVideoRenderer.setMediaStreamTrack(mTrack);

      mCallStats = rtc::scoped_ptr<webrtc::CallStats>(new webrtc::CallStats());
      mCongestionController =
        rtc::scoped_ptr<webrtc::CongestionController>(new webrtc::CongestionController(
                                                                                       mModuleProcessThread.get(),
                                                                                       mCallStats.get())
                                                                                       );

      mModuleProcessThread->Start();
      mModuleProcessThread->RegisterModule(mCallStats.get());

      int numCpuCores = webrtc::CpuInfo::DetectNumberOfCores();

      mTransport = Transport::create(mThisWeak.lock());
      
      webrtc::Transport* transport = mTransport.get();
      webrtc::VideoReceiveStream::Config config(transport);
      webrtc::VideoReceiveStream::Decoder decoder;

      IRTPTypes::CodecParametersList::iterator codecIter = mParameters->mCodecs.begin();
      while (codecIter != mParameters->mCodecs.end()) {
        auto supportedCodec = IRTPTypes::toSupportedCodec(codecIter->mName);
        if (IRTPTypes::SupportedCodec_VP8 == supportedCodec) {
          webrtc::VideoDecoder* videoDecoder = webrtc::VideoDecoder::Create(webrtc::VideoDecoder::kVp8);
          decoder.decoder = videoDecoder;
          decoder.payload_name = codecIter->mName;
          decoder.payload_type = codecIter->mPayloadType;
          break;
        } else if (IRTPTypes::SupportedCodec_VP9 == supportedCodec) {
          webrtc::VideoDecoder* videoDecoder = webrtc::VideoDecoder::Create(webrtc::VideoDecoder::kVp9);
          decoder.decoder = videoDecoder;
          decoder.payload_name = codecIter->mName;
          decoder.payload_type = codecIter->mPayloadType;
          break;
        } else if (IRTPTypes::SupportedCodec_H264 == supportedCodec) {
          webrtc::VideoDecoder* videoDecoder = webrtc::VideoDecoder::Create(webrtc::VideoDecoder::kH264);
          decoder.decoder = videoDecoder;
          decoder.payload_name = codecIter->mName;
          decoder.payload_type = codecIter->mPayloadType;
          break;
        }
        codecIter++;
      }

      IRTPTypes::EncodingParametersList::iterator encodingParamIter = mParameters->mEncodings.begin();
      while (encodingParamIter != mParameters->mEncodings.end()) {
        if (encodingParamIter->mCodecPayloadType == decoder.payload_type) {
          config.rtp.remote_ssrc = encodingParamIter->mSSRC;
          break;
        }
      }

      config.rtp.local_ssrc = mParameters->mRTCP.mSSRC;
      if (mParameters->mRTCP.mReducedSize)
        config.rtp.rtcp_mode = webrtc::RtcpMode::kReducedSize;
      config.rtp.remb = true;
      config.rtp.nack.rtp_history_ms = 1000;
      config.decoders.push_back(decoder);
      config.renderer = &mReceiverVideoRenderer;

      mReceiveStream = rtc::scoped_ptr<webrtc::VideoReceiveStream>(
        new webrtc::internal::VideoReceiveStream(
                                                 numCpuCores,
                                                 mCongestionController.get(),
                                                 config,
                                                 NULL,
                                                 mModuleProcessThread.get(),
                                                 mCallStats.get()
                                                 ));

      mReceiveStream->Start();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideo::~RTPReceiverChannelVideo()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      mReceiveStream->Stop();

      mModuleProcessThread->Stop();

      cancel();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr RTPReceiverChannelVideo::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr RTPReceiverChannelVideo::convert(ForReceiverChannelFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr RTPReceiverChannelVideo::convert(ForRTPReceiverChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr RTPReceiverChannelVideo::convert(ForMediaStreamTrackFromMediaBasePtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object);
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr RTPReceiverChannelVideo::convert(ForMediaStreamTrackPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPReceiverChannelVideo, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelVideo => IRTPReceiverChannelMediaBaseForRTPReceiverChannel
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::notifyTransportState(ISecureTransportTypes::States state)
    {
#define TODO_HANDLE_CHANGE_IN_CONNECTIVITY 1
#define TODO_HANDLE_CHANGE_IN_CONNECTIVITY 2
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelVideo::handlePacket(RTPPacketPtr packet)
    {
      {
        AutoRecursiveLock lock(*this);
      }
      webrtc::PacketTime time;
      mReceiveStream->DeliverRtp(packet->buffer()->data(), packet->buffer()->size(), time);
      return true;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelVideo::handlePacket(RTCPPacketPtr packet)
    
    {
      {
        AutoRecursiveLock lock(*this);
      }
      mReceiveStream->DeliverRtcp(packet->buffer()->data(), packet->buffer()->size());
      return true;
    }
    
    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::handleUpdate(ParametersPtr params)
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
    #pragma mark RTPReceiverChannelVideo => IRTPReceiverChannelVideoForRTPReceiverChannel
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr RTPReceiverChannelVideo::create(
                                                               RTPReceiverChannelPtr receiverChannel,
                                                               MediaStreamTrackPtr track,
                                                               const Parameters &params
                                                               )
    {
      RTPReceiverChannelVideoPtr pThis(make_shared<RTPReceiverChannelVideo>(make_private {}, IORTCForInternal::queueORTC(), receiverChannel, track, params));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelVideo => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::onWake()
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
    #pragma mark RTPReceiverChannelVideo => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::onTimer(TimerPtr timer)
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
    #pragma mark RTPReceiverChannelVideo => IRTPReceiverChannelVideoAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiverChannelVideo => friend Transport
    #pragma mark

    //-------------------------------------------------------------------------
    bool  RTPReceiverChannelVideo::SendRtcp(const uint8_t* packet, size_t length)
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
    #pragma mark RTPSenderChannelVideo::Transport
    #pragma mark

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideo::Transport::Transport(
                                                  const make_private &,
                                                  RTPReceiverChannelVideoPtr outer
                                                  ) :
      mOuter(outer)
    {
    }
        
    //-------------------------------------------------------------------------
    RTPReceiverChannelVideo::Transport::~Transport()
    {
      mThisWeak.reset();
    }
    
    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::Transport::init()
    {
    }
    
    //-------------------------------------------------------------------------
    RTPReceiverChannelVideo::TransportPtr RTPReceiverChannelVideo::Transport::create(RTPReceiverChannelVideoPtr outer)
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
    bool RTPReceiverChannelVideo::Transport::SendRtp(
                                                     const uint8_t* packet,
                                                     size_t length,
                                                     const webrtc::PacketOptions& options
                                                     )
    {
      return true;
    }
    
    //-------------------------------------------------------------------------
    bool RTPReceiverChannelVideo::Transport::SendRtcp(const uint8_t* packet, size_t length)
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
    #pragma mark RTPReceiverChannelVideo => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPReceiverChannelVideo::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPReceiverChannelVideo");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPReceiverChannelVideo::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPReceiverChannelVideo::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPReceiverChannelVideo");

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
    bool RTPReceiverChannelVideo::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiverChannelVideo::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::step()
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
    bool RTPReceiverChannelVideo::stepBogusDoSomething()
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
    void RTPReceiverChannelVideo::cancel()
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
    void RTPReceiverChannelVideo::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPReceiverChannelVideoPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPReceiverChannelVideoStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPReceiverChannelVideo::setError(WORD errorCode, const char *inReason)
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
    #pragma mark IRTPReceiverChannelVideoFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPReceiverChannelVideoFactory &IRTPReceiverChannelVideoFactory::singleton()
    {
      return RTPReceiverChannelVideoFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPReceiverChannelVideoPtr IRTPReceiverChannelVideoFactory::create(
                                                                       RTPReceiverChannelPtr receiverChannel,
                                                                       MediaStreamTrackPtr track,
                                                                       const Parameters &params
                                                                       )
    {
      if (this) {}
      return internal::RTPReceiverChannelVideo::create(receiverChannel, track, params);
    }

  } // internal namespace
}
