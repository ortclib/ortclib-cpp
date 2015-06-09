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

#include <ortc/internal/ortc_SRTPSDESTransport.h>
#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
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
    #pragma mark SRTPSDESTransport => IStatsProvider
    #pragma mark

    //-------------------------------------------------------------------------
    IStatsProvider::PromiseWithStatsReportPtr SRTPSDESTransport::getStats() const throw(InvalidStateError)
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return PromiseWithStatsReportPtr();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPSDESTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    SRTPSDESTransport::SRTPSDESTransport(
                                         const make_private &,
                                         IMessageQueuePtr queue,
                                         ISRTPSDESTransportDelegatePtr originalDelegate,
                                         IICETransportPtr iceTransport,
                                         const CryptoParameters &encryptParameters,
                                         const CryptoParameters &decryptParameters
                                         ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mICETransport(ICETransport::convert(iceTransport))
    {
      ZS_LOG_DETAIL(debug("created"))

      mDefaultSubscription = mSubscriptions.subscribe(ISRTPSDESTransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate), queue);
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::init()
    {
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransport::~SRTPSDESTransport()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();
      cancel();
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr SRTPSDESTransport::convert(ISRTPSDESTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SRTPSDESTransport, object);
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr SRTPSDESTransport::convert(ForRTPSenderPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SRTPSDESTransport, object);
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr SRTPSDESTransport::convert(ForICETransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SRTPSDESTransport, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPSDESTransport => ISRTPSDESTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr SRTPSDESTransport::toDebug(SRTPSDESTransportPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr SRTPSDESTransport::create(
                                                   ISRTPSDESTransportDelegatePtr delegate,
                                                   IICETransportPtr iceTransport,
                                                   const CryptoParameters &encryptParameters,
                                                   const CryptoParameters &decryptParameters
                                                   )
    {
      SRTPSDESTransportPtr pThis(make_shared<SRTPSDESTransport>(make_private{}, IORTCForInternal::queueORTC(), delegate, iceTransport, encryptParameters, decryptParameters));
      pThis->mThisWeak.lock();
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    PUID SRTPSDESTransport::getID() const
    {
      return mID;
    }

    //-------------------------------------------------------------------------
    ISRTPSDESTransportSubscriptionPtr SRTPSDESTransport::subscribe(ISRTPSDESTransportDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to transport state"))

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return mDefaultSubscription;

      ISRTPSDESTransportSubscriptionPtr subscription = mSubscriptions.subscribe(ISRTPSDESTransportDelegateProxy::create(IORTCForInternal::queueDelegate(), originalDelegate));

      ISRTPSDESTransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        SRTPSDESTransportPtr pThis = mThisWeak.lock();

      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    IICETransportPtr SRTPSDESTransport::transport() const
    {
      AutoRecursiveLock lock(*this);
      return ICETransport::convert(mICETransport);
    }

    //-------------------------------------------------------------------------
    ISRTPSDESTransportTypes::ParametersPtr SRTPSDESTransport::getLocalParameters()
    {
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
      return ParametersPtr();
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::stop()
    {
      AutoRecursiveLock lock(*this);
#define TODO_COMPLETE 1
#define TODO_COMPLETE 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPSDESTransport => ISRTPSDESTransportForRTPSender
    #pragma mark

    //-------------------------------------------------------------------------
    bool SRTPSDESTransport::sendPacket(
                                   const BYTE *buffer,
                                   size_t bufferLengthInBytes
                                   )
    {
      ZS_LOG_TRACE(log("sending packet") + ZS_PARAM("length", bufferLengthInBytes))

      UseICETransportPtr transport;
      SecureByteBlockPtr packet;

      {
        AutoRecursiveLock lock(*this);
        if (!mICETransport) {
          ZS_LOG_WARNING(Debug, log("no ice transport is attached"))
          return false;
        }
        transport = mICETransport;

#define TODO_ENCRYPT_PACKET 1
#define TODO_ENCRYPT_PACKET 2
        packet = UseServicesHelper::convertToBuffer(buffer, bufferLengthInBytes);
      }

      // WARNING: Best to not send packet to ice transport inside an object lock
      return transport->sendPacket(*packet, packet->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPSDESTransport => ISRTPSDESTransportForICETransport
    #pragma mark

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::handleReceivedPacket(
                                                 IICETypes::Components viaComponent,
                                                 const BYTE *buffer,
                                                 size_t bufferLengthInBytes
                                                 )
    {
      ZS_LOG_TRACE(log("handle receive packet") + ZS_PARAM("length", bufferLengthInBytes))

      // scope: pre-validation check
      {
        AutoRecursiveLock lock(*this);
#define TODO_DETERMINE_IF_DTLS_PACKET_VS_SRTP_PACKET_AND_HANDLE 1
#define TODO_DETERMINE_IF_DTLS_PACKET_VS_SRTP_PACKET_AND_HANDLE 2
      }

      // WARNING: Forward packet to data channel or RTP listener outside of object lock
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::handleReceivedSTUNPacket(
                                                     IICETypes::Components viaComponent,
                                                     STUNPacketPtr packet
                                                     )
    {
      ZS_LOG_TRACE(log("handle receive stun packet") + packet->toDebug())

      // scope: pre-validation check
      {
        AutoRecursiveLock lock(*this);
#define TODO_FIGURE_OUT_WHERE_TO_FORWARD_PACKET 1
#define TODO_FIGURE_OUT_WHERE_TO_FORWARD_PACKET 2
      }

      // WARNING: Forward packet to data channel or RTP listener outside of object lock
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPSDESTransport => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::onWake()
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
    #pragma mark SRTPSDESTransport => IICETransportDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::onICETransportStateChanged(
                                                   IICETransportPtr transport,
                                                   IICETransport::States state
                                                   )
    {
#define TODO_IMPLEMENT 1
#define TODO_IMPLEMENT 2
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::onICETransportCandidatePairAvailable(
                                                             IICETransportPtr transport,
                                                             CandidatePairPtr candidatePair
                                                             )
    {
#define TODO_IMPLEMENT 1
#define TODO_IMPLEMENT 2
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::onICETransportCandidatePairGone(
                                                        IICETransportPtr transport,
                                                        CandidatePairPtr candidatePair
                                                        )
    {
#define TODO_IMPLEMENT 1
#define TODO_IMPLEMENT 2
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::onICETransportCandidatePairChanged(
                                                           IICETransportPtr transport,
                                                           CandidatePairPtr candidatePair
                                                           )
    {
#define TODO_IMPLEMENT 1
#define TODO_IMPLEMENT 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPSDESTransport => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params SRTPSDESTransport::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::SRTPSDESTransport");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params SRTPSDESTransport::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr SRTPSDESTransport::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::SRTPSDESTransport");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      UseServicesHelper::debugAppend(resultEl, "ice transport", mICETransport ? mICETransport->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool SRTPSDESTransport::isShuttingDown() const
    {
      return (bool)mGracefulShutdownReference;
    }

    //-------------------------------------------------------------------------
    bool SRTPSDESTransport::isShutdown() const
    {
      if (mGracefulShutdownReference) return false;
//      return State_Closed == mCurrentState;
#define TODO 1
#define TODO 2
      return true;
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::step()
    {
      ZS_LOG_DEBUG(debug("step"))

      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }

#define TODO_GET_INTO_VALIDATED_STATE 1
#define TODO_GET_INTO_VALIDATED_STATE 2
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_DTLS_SESSION_IS_SHUTDOWN 1
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_DTLS_SESSION_IS_SHUTDOWN 2
      }

      //.......................................................................
      // final cleanup

#define TODO 1
#define TODO 2

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void SRTPSDESTransport::setError(WORD errorCode, const char *inReason)
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
    #pragma mark ISRTPSDESTransportFactory
    #pragma mark

    //-------------------------------------------------------------------------
    ISRTPSDESTransportFactory &ISRTPSDESTransportFactory::singleton()
    {
      return SRTPSDESTransportFactory::singleton();
    }

    //-------------------------------------------------------------------------
    SRTPSDESTransportPtr ISRTPSDESTransportFactory::create(
                                                           ISRTPSDESTransportDelegatePtr delegate,
                                                           IICETransportPtr iceTransport,
                                                           const CryptoParameters &encryptParameters,
                                                           const CryptoParameters &decryptParameters
                                                           )
    {
      if (this) {}
      return internal::SRTPSDESTransport::create(delegate, iceTransport, encryptParameters, decryptParameters);
    }

  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISRTPSDESTransportTypes
  #pragma mark

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISRTPSDESTransportTypes::CryptoParameters
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr ISRTPSDESTransportTypes::CryptoParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ISRTPSDESTransportTypes::CryptoParameters");

    UseServicesHelper::debugAppend(resultEl, "tag", mTag);
    UseServicesHelper::debugAppend(resultEl, "crypto suite", mCryptoSuite);

    {
      ElementPtr listEl = Element::create("keyParams");

      for (auto iter = mKeyParams.begin(); iter != mKeyParams.end(); ++iter) {
        auto keyParam = (*iter);
        ElementPtr keyParamEl = keyParam.toDebug();
        UseServicesHelper::debugAppend(listEl, keyParamEl);
      }
      UseServicesHelper::debugAppend(resultEl, listEl);
    }

    {
      ElementPtr listEl = Element::create("sessionParams");

      for (auto iter = mSessionParams.begin(); iter != mSessionParams.end(); ++iter) {
        auto sessionParam = (*iter);
        UseServicesHelper::debugAppend(listEl, "param", sessionParam);
      }
      UseServicesHelper::debugAppend(resultEl, listEl);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ISRTPSDESTransportTypes::CryptoParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ISRTPSDESTransportTypes:CryptoParameters:");

    hasher.update(string(mTag));
    hasher.update(":");
    hasher.update(mCryptoSuite);

    hasher.update(":keyParams");

    {
      for (auto iter = mKeyParams.begin(); iter != mKeyParams.end(); ++iter) {
        auto keyParam = (*iter);
        auto hash = keyParam.hash();
        hasher.update(":");
        hasher.update(hash);
      }
    }

    hasher.update(":sessionParams");

    {
      ElementPtr listEl = Element::create("sessionParams");

      for (auto iter = mSessionParams.begin(); iter != mSessionParams.end(); ++iter) {
        auto sessionParam = (*iter);
        hasher.update(":");
        hasher.update(sessionParam);
      }
    }

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISRTPSDESTransportTypes::Parameters
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr ISRTPSDESTransportTypes::Parameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ISRTPSDESTransportTypes::Parameters");

    {
      for (auto iter = mCryptoParams.begin(); iter != mCryptoParams.end(); ++iter) {
        auto cryptoParam = (*iter);
        ElementPtr cryptoParamEl = cryptoParam.toDebug();
        UseServicesHelper::debugAppend(resultEl, cryptoParamEl);
      }
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ISRTPSDESTransportTypes::Parameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ISRTPSDESTransportTypes:Parameters:");

    {
      for (auto iter = mCryptoParams.begin(); iter != mCryptoParams.end(); ++iter) {
        auto cryptoParam = (*iter);
        auto hash = cryptoParam.hash();
        hasher.update(":");
        hasher.update(hash);
      }
    }

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISRTPSDESTransport
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr ISRTPSDESTransport::toDebug(ISRTPSDESTransportPtr transport)
  {
    return internal::SRTPSDESTransport::toDebug(internal::SRTPSDESTransport::convert(transport));
  }

  //---------------------------------------------------------------------------
  ISRTPSDESTransportPtr ISRTPSDESTransport::create(
                                                   ISRTPSDESTransportDelegatePtr delegate,
                                                   IICETransportPtr iceTransport,
                                                   const CryptoParameters &encryptParameters,
                                                   const CryptoParameters &decryptParameters
                                                   )
  {
    return internal::ISRTPSDESTransportFactory::singleton().create(delegate, iceTransport, encryptParameters, decryptParameters);
  }


}
