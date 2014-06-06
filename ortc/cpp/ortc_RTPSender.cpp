/*

 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/internal/ortc_RTPSender.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_ORTC.h>

#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  typedef openpeer::services::IHelper OPIHelper;

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
    #pragma mark RTPSender
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPSender::RTPSender(
                         IMessageQueuePtr queue,
                         IDTLSTransportPtr rtpTransport,
                         IDTLSTransportPtr rtcpTransport
                         ) :
      MessageQueueAssociator(queue),
      mRTPTransport(DTLSTransport::convert(rtpTransport)),
      mRTCPTransport(DTLSTransport::convert(rtcpTransport))
    {
      ZS_LOG_DETAIL(debug("created"))

    }

    //-------------------------------------------------------------------------
    void RTPSender::init()
    {
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPSender::~RTPSender()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();
      cancel();
    }

    //-------------------------------------------------------------------------
    RTPSenderPtr RTPSender::convert(IRTPSenderPtr object)
    {
      return boost::dynamic_pointer_cast<RTPSender>(object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IRTPSender
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr RTPSender::toDebug(IRTPSenderPtr transport)
    {
      if (!transport) return ElementPtr();
      RTPSenderPtr pThis = RTPSender::convert(transport);
      return pThis->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPSenderPtr RTPSender::create(
                                   IDTLSTransportPtr rtpTransport,
                                   IDTLSTransportPtr rtcpTransport
                                   )
    {
      RTPSenderPtr pThis(new RTPSender(IORTCForInternal::queueORTC(), rtpTransport, rtcpTransport));
      pThis->mThisWeak.lock();
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    PUID RTPSender::getID() const
    {
      return mID;
    }

    //-------------------------------------------------------------------------
    IRTPSender::CapabilitiesPtr RTPSender::getCapabilities()
    {
      return CapabilitiesPtr();
    }

    //-------------------------------------------------------------------------
    TrackDescriptionPtr RTPSender::createParams(CapabilitiesPtr capabilities)
    {
      return TrackDescriptionPtr();
    }

    //-------------------------------------------------------------------------
    TrackDescriptionPtr RTPSender::filterParams(
                                                               TrackDescriptionPtr params,
                                                               CapabilitiesPtr capabilities
                                                               )
    {
      return TrackDescriptionPtr();
    }

    //-------------------------------------------------------------------------
    TrackDescriptionPtr RTPSender::getDescription()
    {
      AutoRecursiveLock lock(getLock());
      return mDescription;
    }

    //-------------------------------------------------------------------------
    void RTPSender::setDescription(TrackDescriptionPtr description)
    {
      AutoRecursiveLock lock(getLock());
      mDescription = description;
    }

    //-------------------------------------------------------------------------
    void RTPSender::attach(
                           IDTLSTransportPtr inRtpTransport,
                           IDTLSTransportPtr inRtcpTransport
                           )
    {
      AutoRecursiveLock lock(getLock());

      UseDTLSTransportPtr rtpTransport = DTLSTransport::convert(inRtpTransport);
      UseDTLSTransportPtr rtcpTransport = DTLSTransport::convert(inRtcpTransport);

      if ((rtpTransport == mRTPTransport) &&
          (rtcpTransport == mRTCPTransport)) {
        ZS_LOG_DEBUG(log("attach called but passed in same attachment values (noop)"))
        return;
      }

      // kick start step to fix up state later
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();

      if (rtpTransport != mRTPTransport) {
        if (mRTPTransport) {
          ZS_LOG_DEBUG(log("detaching current RTP DTLS transport") + ZS_PARAM("transport", mRTPTransport->getID()))
#define DO_DETACH 1
#define DO_DETACH 2
        }

        ZS_LOG_DEBUG(log("attaching new RTP DTLS transport") + ZS_PARAM("transport", rtpTransport->getID()))

        mRTPTransport = rtpTransport;
#define DO_ATTACH 1
#define DO_ATTACH 2
      }

      if (rtcpTransport != mRTCPTransport) {
        if (mRTCPTransport) {
          ZS_LOG_DEBUG(log("detaching current RTCP DTLS transport") + ZS_PARAM("transport", mRTCPTransport->getID()))
#define DO_DETACH 1
#define DO_DETACH 2
        }

        ZS_LOG_DEBUG(log("attaching new RTCP DTLS transport") + ZS_PARAM("transport", rtcpTransport->getID()))

        mRTCPTransport = rtcpTransport;
#define DO_ATTACH 1
#define DO_ATTACH 2
      }
    }

    //-------------------------------------------------------------------------
    void RTPSender::start(TrackDescriptionPtr localTrackDescription)
    {
      ZS_LOG_DEBUG(log("stop"))

      AutoRecursiveLock lock(getLock());

      ZS_LOG_DEBUG(log("start called"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_WARNING(Detail, log("already shutdown"));
        return;
      }

      get(mStartCalled) = true;

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    void RTPSender::stop()
    {
      ZS_LOG_DEBUG(log("stop"))

      AutoRecursiveLock lock(getLock());

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_TRACE(log("already shutting down/shutdown"))
        return;
      }

      mGracefulShutdownReference = mThisWeak.lock();

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSender::onWake()
    {
      ZS_LOG_DEBUG(log("wake"))

      AutoRecursiveLock lock(getLock());
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => IDTLSTransportDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPSender::onDTLSTransportStateChanged(
                                                IDTLSTransportPtr transport,
                                                IDTLSTransport::ConnectionStates state
                                                )
    {
      ZS_LOG_DEBUG(log("dtls transport state change") + ZS_PARAM("transport", transport->getID()))
      AutoRecursiveLock lock(getLock());
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPSender::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPSender");
      OPIHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPSender::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPSender::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPSender");

      OPIHelper::debugAppend(resultEl, "id", mID);

      OPIHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);
      OPIHelper::debugAppend(resultEl, "graceful shutdown", mShutdown);

      OPIHelper::debugAppend(resultEl, "start called", mStartCalled);

      OPIHelper::debugAppend(resultEl, "error", mLastError);
      OPIHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      OPIHelper::debugAppend(resultEl, "description", mDescription ? mDescription->toDebug() : ElementPtr());

      OPIHelper::debugAppend(resultEl, "dtls rtp transport", UseDTLSTransport::toDebug(mRTPTransport));
      OPIHelper::debugAppend(resultEl, "dtls rtcp transport", UseDTLSTransport::toDebug(mRTCPTransport));

      OPIHelper::debugAppend(resultEl, "dtls rtp transport subscription", (bool)mRTPTransportSubscription);
      OPIHelper::debugAppend(resultEl, "dtls rtcp transport subscription", (bool)mRTCPTransportSubscription);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool RTPSender::isShuttingDown() const
    {
      return (bool)mGracefulShutdownReference;
    }

    //-------------------------------------------------------------------------
    bool RTPSender::isShutdown() const
    {
      if (mGracefulShutdownReference) return false;
      return mShutdown;
    }

    //-------------------------------------------------------------------------
    void RTPSender::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      AutoRecursiveLock lock(getLock());

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

    }

    //-------------------------------------------------------------------------
    void RTPSender::cancel()
    {
      //.......................................................................
      // start the shutdown process

      //.......................................................................
      // try to gracefully shutdown

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
      }

      //.......................................................................
      // final cleanup

      get(mShutdown) = true;

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();

      if (mRTPTransportSubscription) {
        mRTPTransportSubscription->cancel();
        mRTPTransportSubscription.reset();
      }

      if (mRTCPTransportSubscription) {
        mRTCPTransportSubscription->cancel();
        mRTCPTransportSubscription.reset();
      }
    }

    //-------------------------------------------------------------------------
    void RTPSender::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      if (reason.isEmpty()) {
        reason = IHTTP::toString(IHTTP::toStatusCode(errorCode));
      }

      if (0 != mLastError) {
        ZS_LOG_WARNING(Detail, debug("error already set thus ignoring new error") + ZS_PARAM("new error", errorCode) + ZS_PARAM("new reason", reason))
        return;
      }

      get(mLastError) = errorCode;
      mLastErrorReason = reason;

      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }

  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPSender
  #pragma mark


  //---------------------------------------------------------------------------
  const char *IRTPSender::toString(Options option)
  {
    switch (option) {
      case Option_Unknown:  return "Unknown";
    }
    return "UNDEFINED";
  }


  //---------------------------------------------------------------------------
  ElementPtr IRTPSender::toDebug(IRTPSenderPtr transport)
  {
    return internal::RTPSender::toDebug(transport);
  }

  //---------------------------------------------------------------------------
  IRTPSenderPtr IRTPSender::create(
                                   IDTLSTransportPtr rtpTransport,
                                   IDTLSTransportPtr rtcpTransport
                                   )
  {
    return internal::IRTPSenderFactory::singleton().create(rtpTransport, rtcpTransport);
  }

  //---------------------------------------------------------------------------
  IRTPSender::CapabilitiesPtr IRTPSender::getCapabilities()
  {
    return internal::RTPSender::getCapabilities();
  }

  //---------------------------------------------------------------------------
  TrackDescriptionPtr IRTPSender::filterParams(
                                               TrackDescriptionPtr params,
                                               CapabilitiesPtr capabilities
                                               )
  {
    return internal::RTPSender::filterParams(params, capabilities);
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPSender::Capabilities
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPSender::CapabilitiesPtr IRTPSender::Capabilities::create()
  {
    return CapabilitiesPtr(new Capabilities);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPSender::Capabilities::toDebug() const
  {
    if (mOptions.size() < 1) return ElementPtr();

    ElementPtr resultEl = Element::create("IRTPSender::Capabilities");

    for (OptionsList::const_iterator iter = mOptions.begin(); iter != mOptions.end(); ++iter)
    {
      const Options &option = (*iter);
      OPIHelper::debugAppend(resultEl, "option", IRTPSender::toString(option));
    }

    return resultEl;
  }

}
