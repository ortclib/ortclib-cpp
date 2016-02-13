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
#include <ortc/internal/ortc_Helper.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc_Tracing.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Numeric.h>
#include <zsLib/SafeInt.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/queue.h>

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
  typedef CryptoPP::ByteQueue ByteQueue;

  ZS_DECLARE_INTERACTION_TEAR_AWAY(IDataChannel, internal::DataChannel::TearAwayData)

  ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper)

  using zsLib::Numeric;
  using zsLib::Log;

  namespace internal
  {
    ZS_DECLARE_STRUCT_PTR(DataChannelHelper)
    ZS_DECLARE_TYPEDEF_PTR(DataChannelHelper, UseDataHelper)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    //-------------------------------------------------------------------------
    // http://tools.ietf.org/html/draft-ietf-rtcweb-data-protocol-09#section-8.2.1
    enum ControlMessageTypes
    {
      ControlMessageType_Unknown         = 0x00,

      ControlMessageType_DataChannelOpen = 0x03,
      ControlMessageType_DataChannelAck  = 0x02,
    };

    //-------------------------------------------------------------------------
    static const char *toString(ControlMessageTypes type)
    {
      switch(type) {
        case ControlMessageType_DataChannelOpen:  return "DATA_CHANNEL_OPEN";
        case ControlMessageType_DataChannelAck:   return "DATA_CHANNEL_ACK";
        default: break;
      }
      return "unknown";
    }

    //-------------------------------------------------------------------------
    // http://tools.ietf.org/html/draft-ietf-rtcweb-data-protocol-09#section-5.1
    enum DataChannelOpenMessageChannelTypes {
      DataChannelOpenMessageChannelType_RELIABLE = 0x00,
      DataChannelOpenMessageChannelType_RELIABLE_UNORDERED = 0x80,
      DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_REXMIT = 0x01,
      DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_REXMIT_UNORDERED = 0x81,
      DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_TIMED = 0x02,
      DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_TIMED_UNORDERED = 0x82,
    };

    //-------------------------------------------------------------------------
    static const char *toString(DataChannelOpenMessageChannelTypes type)
    {
      switch(type) {
        case DataChannelOpenMessageChannelType_RELIABLE:                          return "DATA_CHANNEL_RELIABLE";
        case DataChannelOpenMessageChannelType_RELIABLE_UNORDERED:                return "DATA_CHANNEL_RELIABLE_UNORDERED";
        case DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_REXMIT:           return "DATA_CHANNEL_PARTIAL_RELIABLE_REXMIT";
        case DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_REXMIT_UNORDERED: return "DATA_CHANNEL_PARTIAL_RELIABLE_REXMIT_UNORDERED";
        case DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_TIMED:            return "DATA_CHANNEL_PARTIAL_RELIABLE_TIMED";
        case DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_TIMED_UNORDERED:  return "DATA_CHANNEL_PARTIAL_RELIABLE_TIMED_UNORDERED";
        default: break;
      }
      return "unknown";
    }

    namespace data_channel
    {
      struct OpenPacket
      {
        BYTE mMessageType {ControlMessageType_DataChannelOpen};
        BYTE mChannelType {};
        WORD mPriority {};
        DWORD mReliabilityParameter {};
        WORD mLabelLength {};
        WORD mProtocolLength {};
        String mLabel;
        String mProtocol;
      };

      struct AckPacket
      {
        BYTE mMessageType{ControlMessageType_DataChannelAck};
      };
    }

    typedef data_channel::OpenPacket OpenPacket;
    typedef data_channel::AckPacket AckPacket;
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DataChannelHelper
    #pragma mark

    struct DataChannelHelper
    {
      //-----------------------------------------------------------------------
      static ControlMessageTypes getControlMessageType(
                                                       const BYTE *buffer,
                                                       size_t bufferLengthInBytes
                                                       )
      {
        if (bufferLengthInBytes < 1) return ControlMessageType_Unknown;
        BYTE type = 0;
        memcpy(&type, &(buffer[0]), sizeof(type));

        switch(type) {
          case ControlMessageType_DataChannelOpen:  return ControlMessageType_DataChannelOpen;
          case ControlMessageType_DataChannelAck:   return ControlMessageType_DataChannelAck;
        }

        return ControlMessageType_Unknown;
      }
    };

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
                                                                                           WORD sessionID
                                                                                           )
    {
      return IDataChannelFactory::singleton().create(transport, sessionID);
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
                             ParametersPtr params,
                             WORD sessionID
                             ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mDataTransport(transport),
      mParameters(params),
      mIncoming(ORTC_SCTP_INVALID_DATA_CHANNEL_SESSION_ID != sessionID),
      mSessionID(ORTC_SCTP_INVALID_DATA_CHANNEL_SESSION_ID == sessionID ? (params->mID.hasValue() ? params->mID.value() : ORTC_SCTP_INVALID_DATA_CHANNEL_SESSION_ID) : sessionID)
    {
      EventWriteOrtcDataChannelCreate(__func__, mID, ((bool)transport) ? transport->getID() : 0, ((bool)mParameters) ? UseServicesHelper::toString(mParameters->createElement("params")) : String(), mIncoming, mSessionID);
      ZS_LOG_DETAIL(debug("created"))

      mBinaryType = "blob";

      if (originalDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());
      }
    }

    //-------------------------------------------------------------------------
    void DataChannel::init()
    {
      AutoRecursiveLock lock(*this);
      auto transport = mDataTransport.lock();
      if (transport) {
        mDataTransportSubscription = transport->subscribe(mThisWeak.lock());
      }
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    DataChannel::~DataChannel()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();

      EventWriteOrtcDataChannelDestroy(__func__, mID);
    }

    //-------------------------------------------------------------------------
    DataChannelPtr DataChannel::convert(IDataChannelPtr object)
    {
      IDataChannelPtr original = IDataChannelTearAway::original(object);
      return ZS_DYNAMIC_PTR_CAST(DataChannel, original);
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
      ORTC_THROW_INVALID_PARAMETERS_IF(!transport)

      ORTC_THROW_INVALID_PARAMETERS_IF((Milliseconds() != params.mMaxPacketLifetime) &&
                                       (params.mMaxPacketLifetime.count() > UINT32_MAX));

      ORTC_THROW_INVALID_PARAMETERS_IF(params.mLabel.length() > UINT16_MAX)
      ORTC_THROW_INVALID_PARAMETERS_IF(params.mProtocol.length() > UINT16_MAX)

      DataChannelPtr pThis(make_shared<DataChannel>(make_private {}, IORTCForInternal::queueORTC(), delegate, SCTPTransport::convert(transport), make_shared<Parameters>(params)));
      pThis->mThisWeak = pThis;
      UseDataTransportPtr useTransport = pThis->mDataTransport.lock();

      ForDataTransportPtr forTransport = pThis;

      useTransport->registerNewDataChannel(forTransport, pThis->mSessionID);

      DataChannelPtr selectedDataChannel = DataChannel::convert(forTransport);

      if (selectedDataChannel->getID() == pThis->getID()) {
        pThis->init();
      } else {
        pThis->cancel();
      }

      return selectedDataChannel;
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

        if (State_Connecting != mCurrentState) {
          delegate->onDataChannelStateChanged(pThis, mCurrentState);
        }

        if (0 != mLastError) {
          delegate->onDataChannelError(pThis, mLastError, mLastErrorReason);
        }

        if ((mOutgoingBufferFillSize <= mBufferedAmountLowThreshold) &&
            (mBufferedAmountLowThresholdFired)) {
          delegate->onDataChannelBufferedAmountLow(pThis);
        }

        IWakeDelegateProxy::create(pThis)->onWake();
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
      AutoRecursiveLock lock(*this);
      auto params = make_shared<Parameters>();
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
    size_t DataChannel::bufferedAmount() const
    {
      AutoRecursiveLock lock(*this);
      return mOutgoingBufferFillSize;
    }

    //-------------------------------------------------------------------------
    size_t DataChannel::bufferedAmountLowThreshold() const
    {
      AutoRecursiveLock lock(*this);
      return mBufferedAmountLowThreshold;
    }

    //-------------------------------------------------------------------------
    void DataChannel::bufferedAmountLowThreshold(size_t value)
    {
      AutoRecursiveLock lock(*this);

      EventWriteOrtcDataChannelBufferedAmountLowThresholdChanged(__func__, mID, value, mBufferedAmountLowThreshold, mOutgoingBufferFillSize, mBufferedAmountLowThresholdFired);
      mBufferedAmountLowThreshold = value;

      if (mOutgoingBufferFillSize > mBufferedAmountLowThreshold) {
        mBufferedAmountLowThresholdFired = false;
      }
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
      EventWriteOrtcDataChannelClose(__func__, mID);
      ZS_LOG_DEBUG(log("close called"))
      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    void DataChannel::send(const String &data)
    {
      EventWriteOrtcDataChannelSendString(__func__, mID, data);

      AutoRecursiveLock lock(*this);
      if (data.isEmpty()) {
        send(SCTP_PPID_STRING_LAST, NULL, 0);
        return;
      }
      send(SCTP_PPID_STRING_LAST, (const BYTE *)data.c_str(), data.length());
    }

    //-------------------------------------------------------------------------
    void DataChannel::send(const SecureByteBlock &data)
    {
      EventWriteOrtcDataChannelSendBinary(__func__, mID, data.BytePtr(), data.SizeInBytes());
      AutoRecursiveLock lock(*this);
      send(SCTP_PPID_BINARY_LAST, data.BytePtr(), data.SizeInBytes());
    }

    //-------------------------------------------------------------------------
    void DataChannel::send(
                           const BYTE *buffer,
                           size_t bufferSizeInBytes
                           )
    {
      EventWriteOrtcDataChannelSendBinary(__func__, mID, buffer, bufferSizeInBytes);
      ORTC_THROW_INVALID_PARAMETERS_IF((NULL == buffer) && (0 != bufferSizeInBytes))

      AutoRecursiveLock lock(*this);
      send(SCTP_PPID_BINARY_LAST, buffer, bufferSizeInBytes);
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
                                                         WORD sessionID
                                                         )
    {
      DataChannelPtr pThis(make_shared<DataChannel>(make_private {}, IORTCForInternal::queueORTC(), IDataChannelDelegatePtr(), transport, ParametersPtr(), sessionID));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::handleSCTPPacket(SCTPPacketIncomingPtr packet)
    {
      EventWriteOrtcDataChannelSCTPTransportReceivedIncomingPacket(__func__, mID, zsLib::to_underlying(packet->mType), packet->mSessionID, packet->mSequenceNumber, packet->mTimestamp, packet->mFlags, ((bool)packet->mBuffer) ? packet->mBuffer->BytePtr() : NULL, ((bool)packet->mBuffer) ? packet->mBuffer->SizeInBytes() : 0);

      // scope: obtain whatever data is required inside lock to process SCTP packet
      {
        AutoRecursiveLock lock(*this);

        {
          if (isShutdown()) {
            ZS_LOG_WARNING(Debug, log("cannot handle incoming packet when already shutdown"))
            return false;
          }

          if (SCTP_PPID_CONTROL == packet->mType) {
            if (!packet->mBuffer) {
              ZS_LOG_WARNING(Detail, log("packet does not contain a buffer (which is not valid)"))
              return false;
            }

            if (isShuttingDown()) {
              ZS_LOG_WARNING(Detail, log("cannot handle control packet when shutting down"))
              return false;
            }

            auto type = UseDataHelper::getControlMessageType(packet->mBuffer->BytePtr(), packet->mBuffer->SizeInBytes());

            ZS_LOG_TRACE(log("received control packet") + ZS_PARAM("type", internal::toString(type)))

            switch (type) {
              case ControlMessageType_DataChannelOpen: {
                ZS_LOG_TRACE(log("handling data channel open packet"))
                bool result = handleOpenPacket(*packet->mBuffer);
                ZS_LOG_WARNING_IF(!result, Detail, log("faiiled to parse data channel open packet"))
                return result;
              }
              case ControlMessageType_DataChannelAck: {
                ZS_LOG_TRACE(log("handling data channel ack packet"))
                bool result = handleAckPacket(*packet->mBuffer);
                ZS_LOG_WARNING_IF(!result, Detail, log("faiiled to parse data channel ack packet"))
                return result;
              }
              default: {
                if (ZS_IS_LOGGING(Detail)) {
                  String base64 = UseServicesHelper::convertToBase64(*packet->mBuffer);
                  ZS_LOG_WARNING(Detail, log("control message type was not understood") + ZS_PARAM("wire in", base64))
                }
              }
            }
          }

          if (mSubscriptions.size() < 1) {
            ZS_LOG_TRACE(log("queue until there is a subscriber"))
            goto queue_for_later;
          }

          ZS_LOG_TRACE(log("forwarding as event"))
          goto forward_as_event;
        }

      queue_for_later:
        {
          ZS_LOG_TRACE(log("queuing incoming data") + packet->toDebug())
          mIncomingData.push_back(packet);
          return true;
        }

      forward_as_event:
        {
          forwardDataPacketAsEvent(*packet);
          return true;
        }
      }

      return false;
    }

    //-------------------------------------------------------------------------
    void DataChannel::requestShutdown()
    {
      EventWriteOrtcDataChannelSCTPTransportRequestShutdown(__func__, mID);
      ZS_LOG_TRACE(log("request shutdown"))
      IDataChannelAsyncDelegateProxy::create(mThisWeak.lock())->onRequestShutdown();
    }

    //-------------------------------------------------------------------------
    void DataChannel::notifyClosed()
    {
      EventWriteOrtcDataChannelSCTPTransportRequestNotifyClosed(__func__, mID);
      ZS_LOG_TRACE(log("notify closed"))
      IDataChannelAsyncDelegateProxy::create(mThisWeak.lock())->onNotifiedClosed();
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
      EventWriteOrtcDataChannelSCTPTransportStateChanged(__func__, mID);

      ZS_LOG_TRACE(log("on sctp transport state changed"))
      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DataChannel => IDataChannelAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DataChannel::onRequestShutdown()
    {
      ZS_LOG_TRACE(log("on request shutdown"))

      AutoRecursiveLock lock(*this);
      cancel();
    }

    //-------------------------------------------------------------------------
    void DataChannel::onNotifiedClosed()
    {
      ZS_LOG_TRACE(log("on notified closed"))

      AutoRecursiveLock lock(*this);
      mNotifiedClosed = true;
      cancel();
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
    #pragma mark DataChannel => IPromiseSettledDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void DataChannel::onPromiseSettled(PromisePtr promise)
    {
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForDataChannel::RejectReason, RejectReason)

      ZS_LOG_TRACE(log("on promise settled"))

      AutoRecursiveLock lock(*this);

      if (mSendReady) {
        if (mSendReady->isRejected()) {
          RejectReasonPtr reason = mSendReady->reason<RejectReason>();
          ZS_THROW_INVALID_ASSUMPTION_IF(!reason)

          EventWriteOrtcDataChannelSCTPTransportSendReadyFailure(__func__, mID, reason->mError, reason->mErrorReason);

          ZS_LOG_ERROR(Debug, log("cannot send data") + ZS_PARAM("error", reason->mError) + ZS_PARAM("reason", reason->mErrorReason))
          setError(reason->mError, reason->mErrorReason);
          cancel();
          return;
        }
      }

      EventWriteOrtcDataChannelSCTPTransportSendReady(__func__, mID);
      stepSendData(); // only the send promise exists so attempt to send data
    }

    //-------------------------------------------------------------------------
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

      auto dataTransport = mDataTransport.lock();
      UseServicesHelper::debugAppend(resultEl, "data transport", dataTransport ? dataTransport->getID() : 0);
      UseServicesHelper::debugAppend(resultEl, "data transport subscription", (bool)mDataTransportSubscription);

      UseServicesHelper::debugAppend(resultEl, "state", toString(mCurrentState));

      UseServicesHelper::debugAppend(resultEl, "incoming", mIncoming);
      UseServicesHelper::debugAppend(resultEl, "issued open", mIssuedOpen);
      UseServicesHelper::debugAppend(resultEl, "session id", ORTC_SCTP_INVALID_DATA_CHANNEL_SESSION_ID != mSessionID ? string(mSessionID) : String());

      UseServicesHelper::debugAppend(resultEl, "requested sctp shutdown", mRequestedSCTPShutdown);
      UseServicesHelper::debugAppend(resultEl, "notified closed", mNotifiedClosed);

      UseServicesHelper::debugAppend(resultEl, "error", mLastError);
      UseServicesHelper::debugAppend(resultEl, "error reason", mLastErrorReason);

      UseServicesHelper::debugAppend(resultEl, "binary type", mBinaryType);
      UseServicesHelper::debugAppend(resultEl, "parameters", mParameters ? mParameters->toDebug() : ElementPtr());

      UseServicesHelper::debugAppend(resultEl, "incoming data", mIncomingData.size());
      UseServicesHelper::debugAppend(resultEl, "outgoing data", mOutgoingData.size());
      UseServicesHelper::debugAppend(resultEl, "outgoing buffer fill size", mOutgoingBufferFillSize);
      UseServicesHelper::debugAppend(resultEl, "buffered amount low threshold", mBufferedAmountLowThreshold);
      UseServicesHelper::debugAppend(resultEl, "buffered amount low threshold fired", mBufferedAmountLowThresholdFired);

      UseServicesHelper::debugAppend(resultEl, "send ready", (bool)mSendReady);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::isOpen() const
    {
      return State_Open == mCurrentState;
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

      EventWriteOrtcDataChannelStep(__func__, mID);

      // ... other steps here ...
      if (!stepSCTPTransport()) goto not_ready;
      if (!stepIssueConnect()) goto not_ready;
      stepSendData(true);
      if (!stepWaitConnectAck()) goto not_ready;
      if (!stepOpen()) goto not_ready;
      if (!stepSendData()) goto not_ready;
      if (!stepDeliveryIncomingPacket()) goto not_ready;
      // ... other steps here ...

      goto ready;

    not_ready:
      {
        ZS_LOG_TRACE(debug("data channel is not ready"))
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
      EventWriteOrtcDataChannelStep(__func__, mID);

      auto transport = mDataTransport.lock();
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
        ZS_LOG_TRACE(log("waiting for transport to be ready"))
        return false;
      }

      ZS_LOG_TRACE(log("sctp is ready"))
      return true;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::stepIssueConnect()
    {
      EventWriteOrtcDataChannelStep(__func__, mID);

      if (mParameters) {
        if (mParameters->mNegotiated) {
          ZS_LOG_TRACE(log("no need to issue connect as channel is negotiated externally"))
          return true;
        }
      }

      if (mIncoming) {
        ZS_LOG_TRACE(log("incoming connection will not issue connect request"))
        return true;
      }

      if (mIssuedOpen) {
        ZS_LOG_TRACE(log("already issued connect"))
        return true;
      }

      ZS_LOG_DEBUG(log("issuing channel open"))

      mIssuedOpen = true;
      sendControlOpen();

      return true;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::stepWaitConnectAck()
    {
      EventWriteOrtcDataChannelStep(__func__, mID);

      if (mParameters) {
        if (mParameters->mNegotiated) {
          ZS_LOG_TRACE(log("no need to wait for ack as channel is negotiated externally"))
          return true;
        }
      }

      if (mIncoming) {
        ZS_LOG_TRACE(log("incoming connection will not issue connect request"))
        return true;
      }

      if (State_Connecting == mCurrentState) {
        ZS_LOG_TRACE(log("waiting for connection ack"))
        return false;
      }

      ZS_LOG_TRACE(log("incoming connection was acked"))
      return true;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::stepOpen()
    {
      EventWriteOrtcDataChannelStep(__func__, mID);

      setState(State_Open);
      return true;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::stepSendData(bool onlyControlPackets)
    {
      EventWriteOrtcDataChannelStep(__func__, mID);

      if (mSendReady) {
        if (!mSendReady->isResolved()) {
          ZS_LOG_TRACE(log("waiting for send to be ready"))
          return true;
        }

        ZS_LOG_DEBUG(log("send is now ready"))
        mSendReady.reset();
      }

      for (auto iter_doNotUse = mOutgoingData.begin(); iter_doNotUse != mOutgoingData.end();)
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        SCTPPacketOutgoingPtr packet = (*current);

        if (onlyControlPackets) {
          if (SCTP_PPID_CONTROL != packet->mType) {
            ZS_LOG_TRACE(log("delivered all control packets"))
            return true;
          }
        }

        if (!deliverOutgoing(packet)) {
          ZS_LOG_WARNING(Trace, log("unable to deliver pending data") + packet->toDebug())
          return true;
        }

        // consume the buffer as "sent"
        outgoingPacketRemoved(packet);
        mOutgoingData.erase(current);
      }

      return true;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::stepDeliveryIncomingPacket()
    {
      EventWriteOrtcDataChannelStep(__func__, mID);

      if (mSubscriptions.size() < 1) {
        ZS_LOG_TRACE(log("waiting for subscribers"))
        return true;
      }

      if (mIncomingData.size() < 1) {
        ZS_LOG_TRACE(log("no incoming packets to deliver"))
        return true;
      }

      ZS_LOG_DEBUG(log("deliverying incoming packets"))

      for (auto iter_doNotUse = mIncomingData.begin(); iter_doNotUse != mIncomingData.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto packet = (*current);
        forwardDataPacketAsEvent(*packet);

        mIncomingData.erase(current);
      }
      return true;
    }

    //-------------------------------------------------------------------------
    void DataChannel::cancel()
    {
      EventWriteOrtcDataChannelCancel(__func__, mID);

      //.......................................................................
      // try to gracefully shutdown

      if (isShutdown()) return;

      setState(State_Closing);

      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();

      if (mGracefulShutdownReference) {
        if (!mNotifiedClosed) {
          auto dataTransport = mDataTransport.lock();
          if (dataTransport) {
            if (!mRequestedSCTPShutdown) {
              dataTransport->requestShutdown(mThisWeak.lock(), mSessionID);
              mRequestedSCTPShutdown = true;
            }
            ZS_LOG_TRACE(log("waiting for data channel reset"))
            return;
          }
        }
      }

      //.......................................................................
      // final cleanup

      setState(State_Closed);

      mIncomingData.clear();
      mOutgoingData.clear();
      mOutgoingBufferFillSize = 0;

      mSubscriptions.clear();

      if (mDefaultSubscription) {
        mDefaultSubscription->cancel();
        mDefaultSubscription.reset();
      }

      if (mDataTransportSubscription) {
        mDataTransportSubscription->cancel();
        mDataTransportSubscription.reset();
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
      EventWriteOrtcDataChannelStateChangedEventFired(__func__, mID, toString(state));

      DataChannelPtr pThis = mThisWeak.lock();
      if (pThis) {
        mSubscriptions.delegate()->onDataChannelStateChanged(pThis, mCurrentState);
      }
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

      EventWriteOrtcDataChannelErrorEventFired(__func__, mID, mLastError, mLastErrorReason);

      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))

      DataChannelPtr pThis = mThisWeak.lock();
      if (pThis) {
        mSubscriptions.delegate()->onDataChannelError(mThisWeak.lock(), mLastError, mLastErrorReason);
      }
    }

    //-------------------------------------------------------------------------
    bool DataChannel::send(
                           SCTPPayloadProtocolIdentifier ppid,
                           const BYTE *buffer,
                           size_t bufferSizeInBytes
                           )
    {
      if ((isShuttingDown()) &&
          (isShutdown())) {
        ZS_LOG_WARNING(Debug, log("cannot send data (as shutting down / shutdown)"))
        return false;
      }

      if ((NULL == buffer) ||
          (0 == bufferSizeInBytes)) {

        buffer = NULL;
        bufferSizeInBytes = 0;

        switch (ppid) {
          case SCTP_PPID_BINARY_LAST: ppid = SCTP_PPID_BINARY_EMPTY; break;
          case SCTP_PPID_STRING_LAST: ppid = SCTP_PPID_STRING_EMPTY; break;
          default: {
            break;
          }
        }
      }

      SCTPPacketOutgoingPtr packet(make_shared<SCTPPacketOutgoing>());
      packet->mType = ppid;
      if (NULL != buffer) packet->mBuffer = UseServicesHelper::convertToBuffer(buffer, bufferSizeInBytes);

      // scope: check if buffering
      {
        if (mOutgoingData.size() > 0) {
          goto buffer_data;
        }

        if (!isOpen()) {
          if (mIncoming) goto buffer_data;
          if (!mIssuedOpen) goto buffer_data;
        }

        if (mSendReady) {
          if (!mSendReady->isSettled()) goto buffer_data;
          mSendReady.reset();
        }

        if (!deliverOutgoing(packet)) goto buffer_data;
        return true;
      }

    buffer_data:
      {
        ZS_LOG_TRACE(log("buffering data") + ZS_PARAM("ppid", internal::toString(ppid)) + ZS_PARAM("length", bufferSizeInBytes))
        mOutgoingData.push_back(packet);
        outgoingPacketAdded(packet);
      }

      return true;
    }

    //-------------------------------------------------------------------------
    void DataChannel::sendControlOpen()
    {
      ORTC_THROW_INVALID_STATE_IF(!mParameters)

      OpenPacket openPacket;

      if (mParameters->mOrdered) {
        if (mParameters->mMaxRetransmits.hasValue()) {
          openPacket.mChannelType = DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_REXMIT;
          openPacket.mReliabilityParameter = mParameters->mMaxRetransmits.value();
        } else if (Milliseconds() != mParameters->mMaxPacketLifetime) {
          openPacket.mChannelType = DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_TIMED;
          openPacket.mReliabilityParameter = SafeInt<decltype(openPacket.mReliabilityParameter)>(mParameters->mMaxPacketLifetime.count());
        } else {
          openPacket.mChannelType = DataChannelOpenMessageChannelType_RELIABLE;
        }
      } else {
        if (mParameters->mMaxRetransmits.hasValue()) {
          openPacket.mChannelType = DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_REXMIT_UNORDERED;
          openPacket.mReliabilityParameter = mParameters->mMaxRetransmits.value();
        } else if (Milliseconds() != mParameters->mMaxPacketLifetime) {
          openPacket.mChannelType = DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_TIMED_UNORDERED;
          openPacket.mReliabilityParameter = SafeInt<decltype(openPacket.mReliabilityParameter)>(mParameters->mMaxPacketLifetime.count());
        } else {
          openPacket.mChannelType = DataChannelOpenMessageChannelType_RELIABLE_UNORDERED;
        }
      }
      openPacket.mPriority = 0;
      openPacket.mLabel = mParameters->mLabel;
      openPacket.mLabelLength = static_cast<decltype(openPacket.mLabelLength)>(mParameters->mLabel.length());
      openPacket.mProtocol = mParameters->mProtocol;
      openPacket.mProtocolLength = static_cast<decltype(openPacket.mProtocolLength)>(mParameters->mProtocol.length());

      ByteQueue temp;

      temp.Put(openPacket.mMessageType);
      temp.Put(openPacket.mChannelType);
      temp.PutWord16(openPacket.mPriority);
      temp.PutWord32(openPacket.mReliabilityParameter);
      temp.PutWord16(openPacket.mLabelLength);
      temp.PutWord16(openPacket.mProtocolLength);
      if (openPacket.mLabel.length() > 0) {
        temp.Put((const BYTE *)(openPacket.mLabel.c_str()), openPacket.mLabel.length());
      }
      if (openPacket.mProtocol.length() > 0) {
        temp.Put((const BYTE *)(openPacket.mProtocol.c_str()), openPacket.mProtocol.length());
      }

      EventWriteOrtcDataChannelSendControlOpen(__func__, mID, openPacket.mMessageType, openPacket.mChannelType, openPacket.mPriority, openPacket.mReliabilityParameter, openPacket.mLabelLength, openPacket.mProtocolLength, openPacket.mLabel, openPacket.mProtocol);

      ZS_LOG_TRACE(log("sending open control packet") + ZS_PARAM("open message type", internal::toString(static_cast<DataChannelOpenMessageChannelTypes>(openPacket.mChannelType))) + mParameters->toDebug())

      SecureByteBlockPtr buffer(make_shared<SecureByteBlock>(SafeInt<size_t>(temp.CurrentSize())));

      temp.Get(buffer->BytePtr(), buffer->SizeInBytes());

      SCTPPacketOutgoingPtr packet(make_shared<SCTPPacketOutgoing>());
      packet->mType = SCTP_PPID_CONTROL;
      packet->mOrdered = true;
      packet->mBuffer = buffer;
      if (!deliverOutgoing(packet, false)) {
        mOutgoingData.push_front(packet);
        outgoingPacketAdded(packet);
      }
    }

    //-------------------------------------------------------------------------
    void DataChannel::sendControlAck()
    {
      AckPacket ackPacket;

      EventWriteOrtcDataChannelSendControlAck(__func__, mID, ackPacket.mMessageType);

      ByteQueue temp;
      temp.Put(ackPacket.mMessageType);

      SecureByteBlockPtr buffer(make_shared<SecureByteBlock>(SafeInt<size_t>(temp.CurrentSize())));

      temp.Get(buffer->BytePtr(), buffer->SizeInBytes());

      SCTPPacketOutgoingPtr packet(make_shared<SCTPPacketOutgoing>());
      packet->mType = SCTP_PPID_CONTROL;
      packet->mOrdered = true;
      packet->mBuffer = buffer;
      if (!deliverOutgoing(packet, false)) {
        mOutgoingData.push_front(packet);
        outgoingPacketAdded(packet);
      }
    }

    //-------------------------------------------------------------------------
    bool DataChannel::deliverOutgoing(
                                      SCTPPacketOutgoingPtr packet,
                                      bool fixPacket
                                      )
    {
      ZS_LOG_TRACE(log("delivering data") + packet->toDebug())

      packet->mSessionID = mSessionID;
      if (fixPacket) {
        ORTC_THROW_INVALID_STATE_IF(!mParameters)
        packet->mOrdered = mParameters->mOrdered;
        packet->mMaxPacketLifetime = mParameters->mMaxPacketLifetime;
        packet->mMaxRetransmits = mParameters->mMaxRetransmits;
      }

      auto transport = mDataTransport.lock();
      if (!transport) {
        ZS_LOG_WARNING(Debug, log("cannot send packet (as data transport is gone)"))
        return false;
      }

      EventWriteOrtcDataChannelSCTPTransportDeliverOutgoingPacket(__func__, mID, zsLib::to_underlying(packet->mType), packet->mSessionID, packet->mOrdered, packet->mMaxPacketLifetime.count(), packet->mMaxRetransmits.hasValue(), packet->mMaxRetransmits.value(), ((bool)packet->mBuffer) ? packet->mBuffer->BytePtr() : NULL, ((bool)packet) ? packet->mBuffer->SizeInBytes() : 0);

      mSendReady = transport->sendDataNow(packet);

      if (mSendReady) {
        mSendReady->thenWeak(mThisWeak.lock());
      }

      return !((bool)mSendReady);
    }

    //-------------------------------------------------------------------------
    bool DataChannel::handleOpenPacket(SecureByteBlock &buffer)
    {
      OpenPacket openPacket;

      // scope: parse incoming data channel open message
      {
        ByteQueue temp;
        temp.Put(buffer.BytePtr(), buffer.SizeInBytes());

        if (temp.Get(openPacket.mMessageType) != sizeof(openPacket.mMessageType)) return false;
        if (temp.Get(openPacket.mChannelType) != sizeof(openPacket.mChannelType)) return false;
        if (temp.GetWord16(openPacket.mPriority) != sizeof(openPacket.mPriority)) return false;
        CryptoPP::word32 reliabilityParam {};
        if (temp.GetWord32(reliabilityParam) != sizeof(reliabilityParam)) return false;
        openPacket.mReliabilityParameter = reliabilityParam;
        if (temp.GetWord16(openPacket.mLabelLength) != sizeof(openPacket.mLabelLength)) return false;
        if (temp.GetWord16(openPacket.mProtocolLength) != sizeof(openPacket.mProtocolLength)) return false;

        SecureByteBlock tempLabel(openPacket.mLabelLength);
        SecureByteBlock tempProtocol(openPacket.mProtocolLength);

        if (temp.Get(tempLabel.BytePtr(), tempLabel.SizeInBytes()) != tempLabel.SizeInBytes()) return false;
        if (temp.Get(tempProtocol.BytePtr(), tempProtocol.SizeInBytes()) != tempProtocol.SizeInBytes()) return false;

        openPacket.mLabel = UseServicesHelper::convertToString(tempLabel);
        openPacket.mProtocol = UseServicesHelper::convertToString(tempProtocol);

        EventWriteOrtcDataChannelReceivedControlOpen(__func__, mID, mIncoming, openPacket.mMessageType, openPacket.mChannelType, openPacket.mPriority, openPacket.mReliabilityParameter, openPacket.mLabelLength, openPacket.mProtocolLength, openPacket.mLabel, openPacket.mProtocol);

        if (mIncoming) {

          ParametersPtr params(make_shared<Parameters>());
          params->mID = mSessionID;
          switch (openPacket.mChannelType) {
            case DataChannelOpenMessageChannelType_RELIABLE:                          {
              params->mOrdered = true;
              break;
            }
            case DataChannelOpenMessageChannelType_RELIABLE_UNORDERED:                {
              params->mOrdered = false;
              break;
            }
            case DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_REXMIT:           {
              params->mOrdered = true;
              params->mMaxRetransmits = openPacket.mReliabilityParameter;
              break;
            }
            case DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_REXMIT_UNORDERED: {
              params->mOrdered = false;
              params->mMaxRetransmits = openPacket.mReliabilityParameter;
              break;
            }
            case DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_TIMED:            {
              params->mOrdered = true;
              params->mMaxPacketLifetime = Milliseconds(openPacket.mReliabilityParameter);
              break;
            }
            case DataChannelOpenMessageChannelType_PARTIAL_RELIABLE_TIMED_UNORDERED:  {
              params->mOrdered = false;
              params->mMaxPacketLifetime = Milliseconds(openPacket.mReliabilityParameter);
              break;
            }
            default: {
              ZS_LOG_WARNING(Debug, log("data channel open channel type is not undersood") + ZS_PARAM("channel type", openPacket.mChannelType))
              return false;
            }
          }
          params->mLabel = openPacket.mLabel;
          params->mProtocol = openPacket.mProtocol;

          if (mParameters) {
            ZS_LOG_WARNING(Debug, log("already received channel open message") + ZS_PARAM("original", mParameters->toDebug()) + ZS_PARAM("new", params->toDebug()))
            goto send_ack;
          }

          ZS_LOG_TRACE(log("incoming channel") + ZS_PARAM("open message type", internal::toString(static_cast<DataChannelOpenMessageChannelTypes>(openPacket.mChannelType))) + params->toDebug())

          mParameters = make_shared<Parameters>(*params);

          auto dataTransport = mDataTransport.lock();
          dataTransport->announceIncoming(mThisWeak.lock(), params);

          goto send_ack;
        }

        ZS_LOG_WARNING(Debug, log("received data channel open on non-incoming channel"))
        goto send_ack;
      }

    send_ack:
      {
        sendControlAck();
      }
      return true;
    }

    //-------------------------------------------------------------------------
    bool DataChannel::handleAckPacket(SecureByteBlock &buffer)
    {
      EventWriteOrtcDataChannelReceivedControlAck(__func__, mID, zsLib::to_underlying(ControlMessageType_DataChannelAck));

      if (mIncoming) {
        ZS_LOG_WARNING(Detail, log("received unexpected data channel ack on incoming channel"))
        return false;
      }

      if (!mIssuedOpen) {
        ZS_LOG_WARNING(Detail, log("received unexpected data channel ack when channel open was not sent"))
        return false;
      }

      ZS_LOG_TRACE(log("channel is now open (because of ACK to data channel open)"))
      setState(State_Open);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
      return true;
    }

    //-------------------------------------------------------------------------
    void DataChannel::forwardDataPacketAsEvent(const SCTPPacketIncoming &packet)
    {
      ZS_DECLARE_TYPEDEF_PTR(IDataChannelDelegate::MessageEventData, MessageEventData)

      MessageEventDataPtr data(make_shared<MessageEventData>());

      switch (packet.mType) {
        case SCTP_PPID_NONE:
        case SCTP_PPID_CONTROL:
        {
          ZS_LOG_WARNING(Detail, log("message type is not understood"))
          return;
        }
        case SCTP_PPID_BINARY_EMPTY:
        case SCTP_PPID_BINARY_PARTIAL:
        case SCTP_PPID_BINARY_LAST:
        {
          if (packet.mBuffer) {
            data->mBinary = packet.mBuffer;
          } else {
            data->mBinary = make_shared<SecureByteBlock>(); // empty buffer
          }
          ZS_LOG_TRACE(log("forwarding data binary packet") + ZS_PARAM("buffer size", data->mBinary->SizeInBytes()))
          if (ZS_IS_LOGGING(Insane)) {
            String base64 = UseServicesHelper::convertToBase64(*(data->mBinary));
            ZS_LOG_BASIC(log("forwarding data binary packet") + ZS_PARAM("wire in", base64))
          }
          break;
        }
        case SCTP_PPID_STRING_EMPTY:
        case SCTP_PPID_STRING_PARTIAL:
        case SCTP_PPID_STRING_LAST:
        {
          if (packet.mBuffer) {
            data->mText = UseServicesHelper::convertToString(*(packet.mBuffer));
          }
          ZS_LOG_TRACE(log("forwarding data text packet") + ZS_PARAM("text size", data->mText.length()))
          if (ZS_IS_LOGGING(Insane)) {
            ZS_LOG_BASIC(log("forwarding data text packet") + ZS_PARAM("text", data->mText))
          }
          break;
        }
      }

      EventWriteOrtcDataChannelMessageFiredEvent(__func__, mID, zsLib::to_underlying(packet.mType), packet.mSessionID, packet.mSequenceNumber, packet.mTimestamp, packet.mFlags, ((bool)packet.mBuffer) ? packet.mBuffer->BytePtr() : NULL, ((bool)packet.mBuffer) ? packet.mBuffer->SizeInBytes() : 0);

      mSubscriptions.delegate()->onDataChannelMessage(mThisWeak.lock(), data);
    }

    //-------------------------------------------------------------------------
    void DataChannel::outgoingPacketAdded(SCTPPacketOutgoingPtr packet)
    {
      if (!packet) return;
      if (!packet->mBuffer) return;

      EventWriteOrtcDataChannelOutgoingBufferPacket(__func__, mID, zsLib::to_underlying(packet->mType), packet->mSessionID, packet->mOrdered, packet->mMaxPacketLifetime.count(), packet->mMaxRetransmits.hasValue(), packet->mMaxRetransmits.value(), packet->mBuffer->BytePtr(), packet->mBuffer->SizeInBytes());

      auto previousFillSize = mOutgoingBufferFillSize;
      mOutgoingBufferFillSize += packet->mBuffer->SizeInBytes();

      if (previousFillSize <= mBufferedAmountLowThreshold) {
        if (mOutgoingBufferFillSize > mBufferedAmountLowThreshold) {
          // buffer is not "low" anymore
          mBufferedAmountLowThresholdFired = false;
        }
      }
    }

    //-------------------------------------------------------------------------
    void DataChannel::outgoingPacketRemoved(SCTPPacketOutgoingPtr packet)
    {
      if (!packet) return;
      if (!packet->mBuffer) return;

      EventWriteOrtcDataChannelOutgoingBufferPacketDelivered(__func__, mID, zsLib::to_underlying(packet->mType), packet->mSessionID, packet->mOrdered, packet->mMaxPacketLifetime.count(), packet->mMaxRetransmits.hasValue(), packet->mMaxRetransmits.value(), packet->mBuffer->BytePtr(), packet->mBuffer->SizeInBytes());

      size_t packetSize = packet->mBuffer->SizeInBytes();

      bool previouslyFired = mBufferedAmountLowThresholdFired;

      auto previousFillSize = mOutgoingBufferFillSize;
      mOutgoingBufferFillSize -= (mOutgoingBufferFillSize >= packetSize ? packetSize : mOutgoingBufferFillSize);

      if (previousFillSize > mBufferedAmountLowThreshold) {
        if (mOutgoingBufferFillSize <= mBufferedAmountLowThreshold) {
          mBufferedAmountLowThresholdFired = true;

          if (!previouslyFired) {
            auto pThis = mThisWeak.lock();
            if (pThis) {
              ZS_LOG_TRACE(log("buffer low threshold event") + ZS_PARAM("threshold", mBufferedAmountLowThreshold) + ZS_PARAM("fill size", mOutgoingBufferFillSize))
              // the threshold low value is now crossed so fire the buffer low event
              mSubscriptions.delegate()->onDataChannelBufferedAmountLow(pThis);
            }
          }
        }
      }
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
                                                                         WORD sessionID
                                                                         )
    {
      if (this) {}
      return internal::DataChannel::create(transport, sessionID);
    }

  } // internal namespace


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark (helpers)
  #pragma mark

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
  IDataChannelTypes::States IDataChannelTypes::toState(const char *state) throw (InvalidParameters)
  {
    String str(state);

    for (IDataChannelTypes::States index = IDataChannelTypes::State_First; index <= IDataChannelTypes::State_Last; index = static_cast<IDataChannelTypes::States>(static_cast<std::underlying_type<IDataChannelTypes::States>::type>(index) + 1)) {
      if (str == IDataChannelTypes::toString(index)) return index;
    }

    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return State_First;
  }
  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IDataChannelTypes::Parameters
  #pragma mark

  //---------------------------------------------------------------------------
  IDataChannelTypes::Parameters::Parameters(ElementPtr elem)
  {
    if (!elem) return;

    UseHelper::getElementValue(elem, "ortc::IDataChannelTypes::Parameters", "label", mLabel);
    UseHelper::getElementValue(elem, "ortc::IDataChannelTypes::Parameters", "ordered", mOrdered);
    UseHelper::getElementValue(elem, "ortc::IDataChannelTypes::Parameters", "maxPacketLifetime", mMaxPacketLifetime);
    UseHelper::getElementValue(elem, "ortc::IDataChannelTypes::Parameters", "maxRetransmits", mMaxRetransmits);
    UseHelper::getElementValue(elem, "ortc::IDataChannelTypes::Parameters", "protocol", mProtocol);
    UseHelper::getElementValue(elem, "ortc::IDataChannelTypes::Parameters", "negotiated", mNegotiated);
    UseHelper::getElementValue(elem, "ortc::IDataChannelTypes::Parameters", "id", mID);
  }

  //---------------------------------------------------------------------------
  ElementPtr IDataChannelTypes::Parameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "label", mLabel, false);
    UseHelper::adoptElementValue(elem, "ordered", mOrdered);
    UseHelper::adoptElementValue(elem, "maxPacketLifetime", mMaxPacketLifetime);
    UseHelper::adoptElementValue(elem, "maxRetransmits", mMaxRetransmits);
    UseHelper::adoptElementValue(elem, "protocol", mProtocol, false);
    UseHelper::adoptElementValue(elem, "negotiated", mNegotiated);
    UseHelper::adoptElementValue(elem, "id", mID);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

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
                                       ) throw (InvalidParameters, InvalidStateError)
  {
    return internal::IDataChannelFactory::singleton().create(delegate, transport, params);
  }


}
