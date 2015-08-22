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
    IDataChannelForSCTPTransport::ForDataTransportPtr IDataChannelForSCTPTransport::create(
                                                                                           UseDataTransportPtr transport,
                                                                                           const Parameters &params,
                                                                                           WORD localPort,
                                                                                           WORD remotePort
                                                                                           )
    {
      return IDataChannelFactory::singleton().create(transport, params, localPort, remotePort);
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
                             UseDataTransportPtr transport,
                             const Parameters &params,
                             WORD localPort,
                             WORD remotePort
                             ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mDataTransport(SCTPTransport::convert(transport)),
      mParameters(make_shared<Parameters>(params)),
      mLocalPort(localPort),
      mRemotePort(remotePort)
    {
      ZS_LOG_DETAIL(debug("created"))

      mBinaryType = "blob";

      if (originalDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());
      }

      if (0 != localPort) {
        mIncoming = true;
        // incoming connections must only retain a weak pointer to data
        // transport as data transport already has a strong pointer to
        // the data channel (and thus owns the data channel).
        mDataTransportWeak = mDataTransport;
        mDataTransport.reset();
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
                                       IDataTransportPtr transport,
                                       const Parameters &params
                                       )
    {
      DataChannelPtr pThis(make_shared<DataChannel>(make_private {}, IORTCForInternal::queueORTC(), delegate, SCTPTransport::convert(transport), params));
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
      return SCTPTransport::convert(getTransport());
    }

    //-------------------------------------------------------------------------
    IDataChannelTypes::ParametersPtr DataChannel::parameters() const
    {
      AutoRecursiveLock lock(*this);
      auto params = ParametersPtr(make_shared<Parameters>());
      if (mParameters) *params = *mParameters;
      return params;
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
      AutoRecursiveLock lock(*this);

      ULONG total = 0;

      for (auto iter = mOutgoingData.begin(); iter != mOutgoingData.end(); ++iter)
      {
        auto buffer = (*iter);
        total += buffer->SizeInBytes();
      }

      return total;
    }

    //-------------------------------------------------------------------------
    String DataChannel::binaryType() const
    {
      AutoRecursiveLock lock(*this);
      return mBinaryType;
    }

    //-------------------------------------------------------------------------
    void DataChannel::binaryType(const char *str)
    {
      AutoRecursiveLock lock(*this);
      mBinaryType = String(str);
    }

    //-------------------------------------------------------------------------
    void DataChannel::close()
    {
      ZS_LOG_DEBUG(log("close called"))
      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    void DataChannel::send(const String &data)
    {
      if (data.isEmpty()) return;
      send((const BYTE *)data.c_str(), data.length());
    }

    //-------------------------------------------------------------------------
    void DataChannel::send(const SecureByteBlock &data)
    {
      if (data.SizeInBytes() < 1) return;
      send(data.BytePtr(), data.SizeInBytes());
    }

    //-------------------------------------------------------------------------
    void DataChannel::send(
                           const BYTE *buffer,
                           size_t bufferSizeInBytes
                           )
    {
      if (0 == bufferSizeInBytes) return;
      if (NULL == buffer) return;

      AutoRecursiveLock lock(*this);

      if ((isShuttingDown()) &&
          (isShutdown())) {
        ZS_LOG_WARNING(Debug, log("cannot send data (as shutting down / shutdown)"))
        return;
      }

      SecureByteBlockPtr temp = UseServicesHelper::convertToBuffer(buffer, bufferSizeInBytes);
      mOutgoingData.push_back(temp);

      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DataChannel => IDataChannelForSCTPTransport
    #pragma mark

    //-------------------------------------------------------------------------
    DataChannel::ForDataTransportPtr DataChannel::create(
                                                         UseDataTransportPtr transport,
                                                         const Parameters &params,
                                                         WORD localPort,
                                                         WORD remotePort
                                                         )
    {
      DataChannelPtr pThis(make_shared<DataChannel>(make_private {}, IORTCForInternal::queueORTC(), IDataChannelDelegatePtr(), transport, params, localPort, remotePort));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::notifySendSCTPPacket(
                                           const BYTE *buffer,
                                           size_t bufferLengthInBytes
                                           )
    {
      UseDataTransportPtr transport;

      {
        AutoRecursiveLock lock(*this);

        transport = getTransport();
      }

      if (!transport) {
        ZS_LOG_WARNING(Trace, log("transport is gone (thus cannot send data channel packet)") + ZS_PARAM("buffer size", bufferLengthInBytes))
        return false;
      }

      return transport->notifySendSCTPPacket(buffer, bufferLengthInBytes);
    }

    //-------------------------------------------------------------------------
    bool DataChannel::handleSCTPPacket(
                                       const BYTE *buffer,
                                       size_t bufferLengthInBytes
                                       )
    {
      // scope: obtain whatever data is required inside lock to process SCTP packet
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
    #pragma mark DataChannel => ISCTPTransportForDataChannelDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DataChannel::onSCTPTransportStateChanged()
    {
      ZS_LOG_TRACE(log("on sctp transport state changed"))
      AutoRecursiveLock lock(*this);
      step();
    }

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

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      auto dataTransport = getTransport();
      UseServicesHelper::debugAppend(resultEl, "data transport", dataTransport ? dataTransport->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));
      UseServicesHelper::debugAppend(resultEl, "sctp ready", mSCTPReady);

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      UseServicesHelper::debugAppend(resultEl, "binary type", mBinaryType);
      UseServicesHelper::debugAppend(resultEl, "parameters", mParameters ? mParameters->toDebug() : ElementPtr());

      UseServicesHelper::debugAppend(resultEl, "incoming", mIncoming);

      UseServicesHelper::debugAppend(resultEl, "local port", mLocalPort);
      UseServicesHelper::debugAppend(resultEl, "remote port", mRemotePort);

      UseServicesHelper::debugAppend(resultEl, "issued connect", mIssuedConnect);
      UseServicesHelper::debugAppend(resultEl, "connect acked", mConnectAcked);

      UseServicesHelper::debugAppend(resultEl, "issued close", mIssuedClose);
      UseServicesHelper::debugAppend(resultEl, "close acked", mCloseAcked);

      UseServicesHelper::debugAppend(resultEl, "outoing data", mOutgoingData.size());

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
      if (!stepSCTPTransport()) goto not_ready;
      if (!stepIssueConnect()) goto not_ready;
      if (!stepWaitConnectAck()) goto not_ready;
      if (!stepOpen()) goto not_ready;
      if (!stepSendData()) goto not_ready;
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
    bool DataChannel::stepSCTPTransport()
    {
      auto transport = getTransport();
      if (!transport) {
        ZS_LOG_WARNING(Detail, log("sctp is gone (thus must shutdown)"))
        cancel();
        return false;
      }

      if ((transport->isShuttingDown()) ||
          (transport->isShutdown())) {
        ZS_LOG_WARNING(Detail, log("sctp is shutting down / shutdown (thus must shutdown)"))
        cancel();
        return false;
      }

      if (!transport->isReady()) {
        mSCTPReady = false;

        ZS_LOG_TRACE(log("sctp is not ready"))
        return false;
      }

      ZS_LOG_TRACE(log("sctp is ready"))

      mSCTPReady = true;

      if (0 == mLocalPort) {
        mLocalPort = mRemotePort = mDataTransport->allocateLocalPort(mThisWeak.lock());
        if (0 == mLocalPort) {
          ZS_LOG_WARNING(Detail, log("unable to allocate local port"))
          cancel();
          return false;
        }
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::stepIssueConnect()
    {
      if (mIncoming) {
        ZS_LOG_TRACE(log("incoming connection will not issue connect request"))
        return true;
      }

      if (mIssuedConnect) {
        ZS_LOG_TRACE(log("already issued connect"))
        return true;
      }

      mIssuedConnect = true;

#define TODO_SEND_CONNECT_REQUEST 1
#define TODO_SEND_CONNECT_REQUEST 2

      return true;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::stepWaitConnectAck()
    {
      if (mIncoming) {
        ZS_LOG_TRACE(log("incoming connection will not issue connect request"))
        return true;
      }

      if (!mConnectAcked) {
        ZS_LOG_TRACE(log("waiting for connection ack"))
        return false;
      }

      ZS_LOG_TRACE(log("incoming connection was acked"))
      return true;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::stepOpen()
    {
      setState(State_Open);
      return true;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::stepSendData()
    {
      for (auto iter_doNotUse = mOutgoingData.begin(); iter_doNotUse != mOutgoingData.end();)
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        SecureByteBlockPtr buffer = (*current);

#define TODO_SEND_BUFFER 1
#define TODO_SEND_BUFFER 2

        // consume the buffer as "sent"
        mOutgoingData.erase(current);
      }

      return true;
    }

    //-------------------------------------------------------------------------
    void DataChannel::cancel()
    {
      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(State_Closing);

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_SCTP_SESSION_IS_SHUTDOWN 1
#define TODO_OBJECT_IS_BEING_KEPT_ALIVE_UNTIL_SCTP_SESSION_IS_SHUTDOWN 2

        if (!mIssuedClose) {
#define TODO_SEND_CLOSE_REQUEST 1
#define TODO_SEND_CLOSE_REQUEST 2
        }

        auto transport = getTransport();
        if (transport) {
          if (!transport->isShutdown()) {
            // grace shutdown process done here
            if (!mCloseAcked) {
              ZS_LOG_WARNING(Debug, log("waiting for close to ACK"))
              return;
            }
          }
        }
      }

      //.......................................................................
      // final cleanup

      setState(State_Closed);

      mOutgoingData.clear();

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
    DataChannel::UseDataTransportPtr DataChannel::getTransport() const
    {
      if (mDataTransport) return mDataTransport;
      return mDataTransportWeak.lock();
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
                                               IDataTransportPtr transport,
                                               const Parameters &params
                                               )
    {
      if (this) {}
      return internal::DataChannel::create(delegate, transport, params);
    }

    //-------------------------------------------------------------------------
    IDataChannelFactory::ForDataTransportPtr IDataChannelFactory::create(
                                                                         UseDataTransportPtr transport,
                                                                         const Parameters &params,
                                                                         WORD localPort,
                                                                         WORD remotePort
                                                                         )
    {
      if (this) {}
      return internal::DataChannel::create(transport, params, localPort, remotePort);
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
                                       IDataTransportPtr transport,
                                       const Parameters &params
                                       )
  {
    return internal::IDataChannelFactory::singleton().create(delegate, transport, params);
  }


}
