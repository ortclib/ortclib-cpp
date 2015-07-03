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

#include <ortc/internal/ortc_DataChannel.h>
#include <ortc/internal/ortc_SCTPTransport.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>


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
    #pragma mark IDataChannelForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void IDataChannelForSettings::applyDefaults()
    {
//      UseSettings::setUInt(ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE, 5*1024);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDataChannelForSCTPTransport
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr IDataChannelForSCTPTransport::toDebug(ForDataTransportPtr transport)
    {
      if (!transport) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(DataChannel, transport)->toDebug();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DataChannel
    #pragma mark
    
    //-------------------------------------------------------------------------
    DataChannel::DataChannel(
                             const make_private &,
                             IMessageQueuePtr queue,
                             IDataChannelDelegatePtr originalDelegate,
                             IDataTransportPtr transport
                             ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mDataTransport(SCTPTransport::convert(transport))
    {
      ZS_LOG_DETAIL(debug("created"))

      if (originalDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());
      }
    }

    //-------------------------------------------------------------------------
    void DataChannel::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    DataChannel::~DataChannel()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    DataChannelPtr DataChannel::convert(IDataChannelPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(DataChannel, object);
    }

    //-------------------------------------------------------------------------
    DataChannelPtr DataChannel::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(DataChannel, object);
    }

    //-------------------------------------------------------------------------
    DataChannelPtr DataChannel::convert(ForDataTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(DataChannel, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DataChannel => IDataChannel
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr DataChannel::toDebug(DataChannelPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    DataChannelPtr DataChannel::create(
                                       IDataChannelDelegatePtr delegate,
                                       IDataTransportPtr transport
                                       )
    {
      DataChannelPtr pThis(make_shared<DataChannel>(make_private {}, IORTCForInternal::queueORTC(), delegate, transport));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    IDataChannelSubscriptionPtr DataChannel::subscribe(IDataChannelDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to transport state"))

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return mDefaultSubscription;

      IDataChannelSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      IDataChannelDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        DataChannelPtr pThis = mThisWeak.lock();

#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 1
#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 2
      }

      if (isShutdown()) {
        mSubscriptions.clear();
      }
      
      return subscription;
    }

    //-------------------------------------------------------------------------
    IDataTransportPtr DataChannel::transport() const
    {
      return SCTPTransport::convert(mDataTransport.lock());
    }

    //-------------------------------------------------------------------------
    IDataChannelTypes::ParametersPtr DataChannel::parameters() const
    {
#define TODO 1
#define TODO 2
      return ParametersPtr();
    }

    //-------------------------------------------------------------------------
    IDataChannelTypes::States DataChannel::readyState() const
    {
      AutoRecursiveLock lock(*this);
      return mCurrentState;
    }

    //-------------------------------------------------------------------------
    ULONG DataChannel::bufferedAmount() const
    {
#define TODO 1
#define TODO 2
      return 0;
    }

    //-------------------------------------------------------------------------
    String DataChannel::binaryType() const
    {
#define TODO 1
#define TODO 2
      return String();
    }

    //-------------------------------------------------------------------------
    void DataChannel::close()
    {
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    void DataChannel::send(const String &data)
    {
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    void DataChannel::send(const SecureByteBlock &data)
    {
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    void DataChannel::send(
                           const BYTE *buffer,
                           size_t bufferSizeInBytes
                           )
    {
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DataChannel => IDataChannelForSCTPTransport
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DataChannel => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DataChannel::onWake()
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
    #pragma mark DataChannel => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DataChannel::onTimer(TimerPtr timer)
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
    #pragma mark DataChannel => IDataChannelAsyncDelegate
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DataChannel => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params DataChannel::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::DataChannel");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params DataChannel::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr DataChannel::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::DataChannel");

      auto dataTransport = mDataTransport.lock();

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      UseServicesHelper::debugAppend(resultEl, "data transport", dataTransport ? dataTransport->getID() : 0);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::isShuttingDown() const
    {
      return State_Closing == mCurrentState;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::isShutdown() const
    {
      return State_Closed == mCurrentState;
    }

    //-------------------------------------------------------------------------
    void DataChannel::step()
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
    bool DataChannel::stepBogusDoSomething()
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
    void DataChannel::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_SCTP_SESSION_IS_SHUTDOWN 1
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_SCTP_SESSION_IS_SHUTDOWN 2

        // grace shutdown process done here

        return;
      }

      //.......................................................................
      // final cleanup

      setState(State_Closing);

      mSubscriptions.clear();

      if (mDefaultSubscription) {
        mDefaultSubscription->cancel();
        mDefaultSubscription.reset();
      }

      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    void DataChannel::setState(States state)
    {
      if (state == mCurrentState) return;

      ZS_LOG_DETAIL(debug("state changed") + ZS_PARAM("new state", toString(state)) + ZS_PARAM("old state", toString(mCurrentState)))

      mCurrentState = state;

//      DataChannelPtr pThis = mThisWeak.lock();
//      if (pThis) {
//        mSubscriptions.delegate()->onDataChannelStateChanged(pThis, mCurrentState);
//      }
    }

    //-------------------------------------------------------------------------
    void DataChannel::setError(WORD errorCode, const char *inReason)
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
    #pragma mark IDataChannelFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IDataChannelFactory &IDataChannelFactory::singleton()
    {
      return DataChannelFactory::singleton();
    }

    //-------------------------------------------------------------------------
    DataChannelPtr IDataChannelFactory::create(
                                               IDataChannelDelegatePtr delegate,
                                               IDataTransportPtr transport
                                               )
    {
      if (this) {}
      return internal::DataChannel::create(delegate, transport);
    }

  } // internal namespace


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDataChannelTypes
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IDataChannelTypes::toString(States state)
  {
    switch (state) {
      case IDataChannelTypes::State_Connecting: return "connecting";
      case IDataChannelTypes::State_Open:       return "open";
      case IDataChannelTypes::State_Closing:    return "closing";
      case IDataChannelTypes::State_Closed:     return "closed";
    }

    ASSERT(false)
    return "UNDEFINED";
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDataChannelTypes::Parameters
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IDataChannelTypes::Parameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IDataChannelTypes::Parameters");

    UseServicesHelper::debugAppend(resultEl, "label", mLabel);
    UseServicesHelper::debugAppend(resultEl, "ordered", mOrdered);
    UseServicesHelper::debugAppend(resultEl, "max packet lifetime", mMaxPacketLifetime);
    UseServicesHelper::debugAppend(resultEl, "max retransmits", mMaxRetransmits);
    UseServicesHelper::debugAppend(resultEl, "protocol", mProtocol);
    UseServicesHelper::debugAppend(resultEl, "negotiated", mNegotiated);
    UseServicesHelper::debugAppend(resultEl, "id", mID);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IDataChannelTypes::Parameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("IDataChannelTypes:Parameters:");
    hasher.update(mLabel);
    hasher.update(":");
    hasher.update(mOrdered);
    hasher.update(":");
    hasher.update(mMaxPacketLifetime);
    hasher.update(":");
    hasher.update(mMaxRetransmits);
    hasher.update(":");
    hasher.update(mProtocol);
    hasher.update(":");
    hasher.update(mNegotiated);
    hasher.update(":");
    hasher.update(mID);

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDataChannelTypes
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IDataChannel::toDebug(IDataChannelPtr transport)
  {
    return internal::DataChannel::toDebug(internal::DataChannel::convert(transport));
  }

  //---------------------------------------------------------------------------
  IDataChannelPtr IDataChannel::create(
                                       IDataChannelDelegatePtr delegate,
                                       IDataTransportPtr transport
                                       )
  {
    return internal::IDataChannelFactory::singleton().create(delegate, transport);
  }


}
