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

#include <ortc/internal/ortc_RTPSenderChannel.h>
#include <ortc/internal/ortc_RTPSender.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_RTPListener.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
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

#include <webrtc/modules/rtp_rtcp/interface/rtp_header_parser.h>
#include <webrtc/video/video_send_stream.h>


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
    void IRTPSenderChannelForSettings::applyDefaults()
    {
//      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 5*1024);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelForRTPSender
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPSenderChannelForRTPSender::toDebug(ForRTPSenderPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannel, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelPtr IRTPSenderChannelForRTPSender::create(
                                                              RTPSenderPtr sender,
                                                              const Parameters &params
                                                              )
    {
      return internal::IRTPSenderChannelFactory::singleton().create(sender, params);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderChannelForMediaStreamTrack
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IRTPSenderChannelForMediaStreamTrack::toDebug(ForMediaStreamTrackPtr object)
    {
      if (!object) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannel, object)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel
    #pragma mark
    
    //-------------------------------------------------------------------------
    const char *RTPSenderChannel::toString(States state)
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
    RTPSenderChannel::RTPSenderChannel(
                                       const make_private &,
                                       IMessageQueuePtr queue,
                                       UseSenderPtr sender,
                                       const Parameters &params
                                       ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mSender(sender),
      mParameters(make_shared<Parameters>(params))
    {
      ZS_LOG_DETAIL(debug("created"))
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannel::~RTPSenderChannel()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelPtr RTPSenderChannel::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannel, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelPtr RTPSenderChannel::convert(ForRTPSenderPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannel, object);
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelPtr RTPSenderChannel::convert(ForMediaStreamTrackPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(RTPSenderChannel, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel => ForRTPSender
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr RTPSenderChannel::toDebug(RTPSenderChannelPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelPtr RTPSenderChannel::create(
                                                 RTPSenderPtr sender,
                                                 const Parameters &params
                                                 )
    {
      RTPSenderChannelPtr pThis(make_shared<RTPSenderChannel>(make_private {}, IORTCForInternal::queueORTC(), sender, params));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::notifyTransportState(ISecureTransportTypes::States state)
    {
      // do NOT lock this object here, instead notify self asynchronously
      IRTPSenderChannelAsyncDelegateProxy::create(mThisWeak.lock())->onSecureTransportState(state);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::notifyPackets(RTCPPacketListPtr packets)
    {
      // do NOT lock this object here, instead notify self asynchronously
      IRTPSenderChannelAsyncDelegateProxy::create(mThisWeak.lock())->onNotifyPackets(packets);
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::notifyUpdate(const Parameters &params)
    {
      // do NOT lock this object here, instead notify self asynchronously
      IRTPSenderChannelAsyncDelegateProxy::create(mThisWeak.lock())->onUpdate(make_shared<Parameters>(params));
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannel::handlePacket(RTCPPacketPtr packet)
    {
#define TODO 1
#define TODO 2
      {
        AutoRecursiveLock lock(*this);
      }
      return false;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel => IRTPSenderChannelForMediaStreamTrack
    #pragma mark
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannel::onWake()
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
    #pragma mark RTPSenderChannel => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannel::onTimer(TimerPtr timer)
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
    #pragma mark RTPSenderChannel => IRTPSenderChannelAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSenderChannel::onSecureTransportState(ISecureTransport::States state)
    {
      ZS_LOG_TRACE(log("notified secure transport state") + ZS_PARAM("state", ISecureTransport::toString(state)))

      AutoRecursiveLock lock(*this);

      mSecureTransportState = state;

      if (ISecureTransport::State_Closed == state) {
        ZS_LOG_DEBUG(log("secure channel closed (thus shutting down)"))
        cancel();
        return;
      }

#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::onNotifyPackets(RTCPPacketListPtr packets)
    {
      ZS_LOG_TRACE(log("notified rtcp packets") + ZS_PARAM("packets", packets->size()))

      AutoRecursiveLock lock(*this);

      // WARNING: Do NOT modify the contents of "packets" as the pointer to
      //          this list could have been sent to multiple receiver channels
      //          simultaneously. Use COW pattern if needing mutability.

#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::onUpdate(ParametersPtr params)
    {
      ZS_LOG_TRACE(log("on update") + params->toDebug())

      AutoRecursiveLock lock(*this);

#define TODO 1
#define TODO 2

      mParameters = params;
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSenderChannel => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPSenderChannel::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPSenderChannel");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPSenderChannel::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPSenderChannel::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::RTPSenderChannel");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      UseServicesHelper::debugAppend(resultEl, "secure transport state", ISecureTransport::toString(mSecureTransportState));

      auto sender = mSender.lock();
      UseServicesHelper::debugAppend(resultEl, "sender", sender ? sender->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannel::isShuttingDown() const
    {
      return State_ShuttingDown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannel::isShutdown() const
    {
      return State_Shutdown == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::step()
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
        ZS_LOG_TRACE(debug("dtls is not ready"))
        return;
      }

    ready:
      {
        ZS_LOG_TRACE(log("ready"))
      }
    }

    //-------------------------------------------------------------------------
    bool RTPSenderChannel::stepBogusDoSomething()
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
    void RTPSenderChannel::cancel()
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
    void RTPSenderChannel::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      RTPSenderChannelPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onRTPSenderChannelStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void RTPSenderChannel::setError(WORD errorCode, const char *inReason)
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
    #pragma mark IRTPSenderChannelFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPSenderChannelFactory &IRTPSenderChannelFactory::singleton()
    {
      return RTPSenderChannelFactory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPSenderChannelPtr IRTPSenderChannelFactory::create(
                                                         RTPSenderPtr sender,
                                                         const Parameters &params
                                                         )
    {
      if (this) {}
      return internal::RTPSenderChannel::create(sender, params);
    }

  } // internal namespace

}
