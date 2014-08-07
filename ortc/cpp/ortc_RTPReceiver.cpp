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

#include <ortc/internal/ortc_RTPReceiver.h>
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
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)

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
    #pragma mark RTPReceiver
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTPReceiver::RTPReceiver(
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
    void RTPReceiver::init()
    {
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    RTPReceiver::~RTPReceiver()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();
      cancel();
    }

    //-------------------------------------------------------------------------
    RTPReceiverPtr RTPReceiver::convert(IRTPReceiverPtr object)
    {
      return boost::dynamic_pointer_cast<RTPReceiver>(object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver => IRTPReceiver
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr RTPReceiver::toDebug(IRTPReceiverPtr transport)
    {
      if (!transport) return ElementPtr();
      RTPReceiverPtr pThis = RTPReceiver::convert(transport);
      return pThis->toDebug();
    }

    //-------------------------------------------------------------------------
    RTPReceiverPtr RTPReceiver::create(
                                   IDTLSTransportPtr rtpTransport,
                                   IDTLSTransportPtr rtcpTransport
                                   )
    {
      RTPReceiverPtr pThis(new RTPReceiver(IORTCForInternal::queueORTC(), rtpTransport, rtcpTransport));
      pThis->mThisWeak.lock();
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    PUID RTPReceiver::getID() const
    {
      return mID;
    }

    //-------------------------------------------------------------------------
    IRTPReceiver::CapabilitiesPtr RTPReceiver::getCapabilities()
    {
      return CapabilitiesPtr();
    }

    //-------------------------------------------------------------------------
    TrackDescriptionPtr RTPReceiver::createParams(CapabilitiesPtr capabilities)
    {
      return TrackDescriptionPtr();
    }

    //-------------------------------------------------------------------------
    TrackDescriptionPtr RTPReceiver::filterParams(
                                                  TrackDescriptionPtr params,
                                                  CapabilitiesPtr capabilities
                                                  )
    {
      return TrackDescriptionPtr();
    }

    //-------------------------------------------------------------------------
    TrackDescriptionPtr RTPReceiver::getDescription()
    {
      AutoRecursiveLock lock(getLock());
      return mDescription;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::setDescription(TrackDescriptionPtr description)
    {
      AutoRecursiveLock lock(getLock());
      mDescription = description;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::attach(
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
    void RTPReceiver::start(TrackDescriptionPtr localTrackDescription)
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
    void RTPReceiver::stop()
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
    #pragma mark RTPReceiver => IRTPReceiverForDTLS
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPReceiver => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiver::onWake()
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
    #pragma mark RTPReceiver => IDTLSTransportDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void RTPReceiver::onDTLSTransportStateChanged(
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
    #pragma mark RTPReceiver => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params RTPReceiver::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::RTPReceiver");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params RTPReceiver::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr RTPReceiver::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::RTPReceiver");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);
      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", mShutdown);

      UseServicesHelper::debugAppend(resultEl, "start called", mStartCalled);

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      UseServicesHelper::debugAppend(resultEl, "description", mDescription ? mDescription->toDebug() : ElementPtr());

      UseServicesHelper::debugAppend(resultEl, "dtls rtp transport", UseDTLSTransport::toDebug(mRTPTransport));
      UseServicesHelper::debugAppend(resultEl, "dtls rtcp transport", UseDTLSTransport::toDebug(mRTCPTransport));

      UseServicesHelper::debugAppend(resultEl, "dtls rtp transport subscription", (bool)mRTPTransportSubscription);
      UseServicesHelper::debugAppend(resultEl, "dtls rtcp transport subscription", (bool)mRTCPTransportSubscription);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::isShuttingDown() const
    {
      return (bool)mGracefulShutdownReference;
    }

    //-------------------------------------------------------------------------
    bool RTPReceiver::isShutdown() const
    {
      if (mGracefulShutdownReference) return false;
      return mShutdown;
    }

    //-------------------------------------------------------------------------
    void RTPReceiver::step()
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
    void RTPReceiver::cancel()
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
    void RTPReceiver::setError(WORD errorCode, const char *inReason)
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
  #pragma mark IRTPReceiver
  #pragma mark


  //---------------------------------------------------------------------------
  const char *IRTPReceiver::toString(Options option)
  {
    switch (option) {
      case Option_Unknown:  return "Unknown";
    }
    return "UNDEFINED";
  }


  //---------------------------------------------------------------------------
  ElementPtr IRTPReceiver::toDebug(IRTPReceiverPtr transport)
  {
    return internal::RTPReceiver::toDebug(transport);
  }

  //---------------------------------------------------------------------------
  IRTPReceiverPtr IRTPReceiver::create(
                                   IDTLSTransportPtr rtpTransport,
                                   IDTLSTransportPtr rtcpTransport
                                   )
  {
    return internal::IRTPReceiverFactory::singleton().create(rtpTransport, rtcpTransport);
  }

  //---------------------------------------------------------------------------
  IRTPReceiver::CapabilitiesPtr IRTPReceiver::getCapabilities()
  {
    return internal::RTPReceiver::getCapabilities();
  }

  //---------------------------------------------------------------------------
  TrackDescriptionPtr IRTPReceiver::filterParams(
                                               TrackDescriptionPtr params,
                                               CapabilitiesPtr capabilities
                                               )
  {
    return internal::RTPReceiver::filterParams(params, capabilities);
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTPReceiver::Capabilities
  #pragma mark

  //---------------------------------------------------------------------------
  IRTPReceiver::CapabilitiesPtr IRTPReceiver::Capabilities::create()
  {
    return CapabilitiesPtr(new Capabilities);
  }

  //---------------------------------------------------------------------------
  ElementPtr IRTPReceiver::Capabilities::toDebug() const
  {
    if (mOptions.size() < 1) return ElementPtr();

    ElementPtr resultEl = Element::create("IRTPReceiver::Capabilities");

    for (OptionsList::const_iterator iter = mOptions.begin(); iter != mOptions.end(); ++iter)
    {
      const Options &option = (*iter);
      UseServicesHelper::debugAppend(resultEl, "option", IRTPReceiver::toString(option));
    }

    return resultEl;
  }

}
