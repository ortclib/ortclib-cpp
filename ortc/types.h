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

#pragma once

#include <ortc/services/types.h>

#include <zsLib/Proxy.h>
#include <zsLib/IPAddress.h>
#include <zsLib/Promise.h>
#include <zsLib/String.h>

namespace ortc
{
  using std::make_shared;
  using std::size_t;

  using zsLib::string;
  using zsLib::PUID;
  using zsLib::BYTE;
  using zsLib::WORD;
  using zsLib::DWORD;
  using zsLib::QWORD;
  using zsLib::USHORT;
  using zsLib::UCHAR;
  using zsLib::UINT;
  using zsLib::LONG;
  using zsLib::ULONG;
  using zsLib::ULONGLONG;
  using zsLib::Time;
  using zsLib::Hours;
  using zsLib::Minutes;
  using zsLib::Seconds;
  using zsLib::Milliseconds;
  using zsLib::Microseconds;
  using zsLib::Nanoseconds;
  using zsLib::String;
  using zsLib::RecursiveLock;
  using zsLib::IMessageQueue;
  using zsLib::IMessageQueuePtr;
  using zsLib::IPAddress;

  using zsLib::Optional;
  using zsLib::PromiseWith;

  ZS_DECLARE_USING_PTR(zsLib, Any)
  ZS_DECLARE_USING_PTR(zsLib, Promise)

  ZS_DECLARE_STRUCT_PTR(ErrorAny)

  struct ErrorAny : public Any
  {
    typedef WORD ErrorCode;

    ErrorCode mErrorCode {};
    String mName;
    String mReason;

    ErrorAny() {}

    ErrorAny(WORD errorCode, const char *reason, const char *name = NULL) :
      mErrorCode(errorCode),
      mReason(reason),
      mName(name) {}

    static ErrorAnyPtr create(WORD errorCode, const char *reason, const char *name = NULL) { return make_shared<ErrorAny>(errorCode, reason, name); }
  };

  using ortc::services::SharedRecursiveLock;
  using ortc::services::LockedValue;

  ZS_DECLARE_USING_PTR(ortc::services, SecureByteBlock)

  namespace JSON = zsLib::JSON;
  using JSON::Element;
  using JSON::ElementPtr;

  typedef zsLib::Exceptions::InvalidUsage InvalidStateError;
  typedef zsLib::Exceptions::InvalidArgument InvalidParameters;
  typedef zsLib::Exceptions::SyntaxError SyntaxError;
  typedef zsLib::Exceptions::NotImplemented NotSupportedError;

#define ORTC_THROW_INVALID_STATE(xMessage) ZS_THROW_INVALID_USAGE(xMessage)
#define ORTC_THROW_INVALID_PARAMETERS(xMessage) ZS_THROW_INVALID_ARGUMENT(xMessage)
#define ORTC_THROW_SYNTAX_ERROR(xMessage) ZS_THROW_CUSTOM(SyntaxError, xMessage)
#define ORTC_THROW_NOT_SUPPORTED_ERRROR(xMessage) ZS_THROW_NOT_IMPLEMENTED(xMessage)

#define ORTC_THROW_INVALID_STATE_IF(xExpression) ZS_THROW_INVALID_USAGE_IF(xExpression)
#define ORTC_THROW_INVALID_PARAMETERS_IF(xExpression) ZS_THROW_INVALID_ARGUMENT_IF(xExpression)
#define ORTC_THROW_SYNTAX_ERROR_IF(xExpression) ZS_THROW_CUSTOM_IF(SyntaxError, xExpression)
#define ORTC_THROW_NOT_SUPPORTED_ERROR_IF(xExpression) ZS_THROW_NOT_IMPLEMENTED_IF(xExpression)

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark (forwards)
  #pragma mark

  interaction IRTPTransport
  {
    virtual PUID getID() const = 0;
  };

  interaction IRTCPTransport
  {
    virtual PUID getID() const = 0;
  };

  ZS_DECLARE_INTERACTION_PTR(ICertificate);
  ZS_DECLARE_INTERACTION_PTR(IDataChannel);
  ZS_DECLARE_INTERACTION_PTR(IDataTransport);
  ZS_DECLARE_INTERACTION_PTR(IDTLSTransport);
  ZS_DECLARE_INTERACTION_PTR(IDTMFSender);
  ZS_DECLARE_INTERACTION_PTR(IORTC);
  ZS_DECLARE_INTERACTION_PTR(IHelper);
  ZS_DECLARE_INTERACTION_PTR(IIdentity);
  ZS_DECLARE_INTERACTION_PTR(IICEGatherer);
  ZS_DECLARE_INTERACTION_PTR(IICETransport);
  ZS_DECLARE_INTERACTION_PTR(IICETransportController);
  ZS_DECLARE_INTERACTION_PTR(IMediaDevices);
  ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackRenderCallback);
  ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrack);
  ZS_DECLARE_INTERACTION_PTR(IRTPListener);
  ZS_DECLARE_INTERACTION_PTR(IRTPSender);
  ZS_DECLARE_INTERACTION_PTR(IRTPReceiver);
  ZS_DECLARE_INTERACTION_PTR(IRTPTransport);
  ZS_DECLARE_INTERACTION_PTR(IRTPTypes);
  ZS_DECLARE_INTERACTION_PTR(IRTCPTransport);
  ZS_DECLARE_INTERACTION_PTR(ISCTPTransport);
  ZS_DECLARE_INTERACTION_PTR(ISRTPSDESTransport);
  ZS_DECLARE_INTERACTION_PTR(IStatsProvider);
  ZS_DECLARE_INTERACTION_PTR(IStatsReport);

  ZS_DECLARE_INTERACTION_PROXY(IDataChannelDelegate);
  ZS_DECLARE_INTERACTION_PROXY(IDTLSTransportDelegate);
  ZS_DECLARE_INTERACTION_PROXY(IDTMFSenderDelegate);
  ZS_DECLARE_INTERACTION_PROXY(IICEGathererDelegate);
  ZS_DECLARE_INTERACTION_PROXY(IICETransportDelegate);
  ZS_DECLARE_INTERACTION_PROXY(IRTPListenerDelegate);
  ZS_DECLARE_INTERACTION_PROXY(IMediaDevicesDelegate);
  ZS_DECLARE_INTERACTION_PROXY(IMediaStreamTrackDelegate);
  ZS_DECLARE_INTERACTION_PROXY(IRTPSenderDelegate);
  ZS_DECLARE_INTERACTION_PROXY(IRTPReceiverDelegate);
  ZS_DECLARE_INTERACTION_PROXY(ISCTPTransportDelegate);
  ZS_DECLARE_INTERACTION_PROXY(ISCTPTransportListenerDelegate);
  ZS_DECLARE_INTERACTION_PROXY(ISRTPSDESTransportDelegate);

  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IDataChannelSubscription, IDataChannelDelegate);
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IDTLSTransportSubscription, IDTLSTransportDelegate);
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IDTMFSenderSubscription, IDTMFSenderDelegate);
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IICEGathererSubscription, IICEGathererDelegate);
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IICETransportSubscription, IICETransportDelegate);
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IMediaDevicesSubscription, IMediaDevicesDelegate);
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IMediaStreamTrackSubscription, IMediaStreamTrackDelegate);
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IRTPListenerSubscription, IRTPListenerDelegate);
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IRTPSenderSubscription, IRTPSenderDelegate);
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IRTPReceiverSubscription, IRTPReceiverDelegate);
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(ISCTPTransportSubscription, ISCTPTransportDelegate);
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(ISCTPTransportListenerSubscription, ISCTPTransportListenerDelegate);
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(ISRTPSDESTransportSubscription, ISRTPSDESTransportDelegate);


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark (mixed case versions of interfaces are available externally)
  #pragma mark

#ifndef ORTCLIB_INTERNAL
#if 0
  ZS_DECLARE_TYPEDEF_PTR(IDTLSTransport, IDtlsTransport);
  ZS_DECLARE_TYPEDEF_PTR(IDTMFSender, IDtmfSender);
  ZS_DECLARE_TYPEDEF_PTR(IORTC, IOrtc);
  ZS_DECLARE_TYPEDEF_PTR(IICEGatherer, IIceGatherer);
  ZS_DECLARE_TYPEDEF_PTR(IICETransport, IIceTransport);
  ZS_DECLARE_TYPEDEF_PTR(IICETransportController, IIceTransportController);
  ZS_DECLARE_TYPEDEF_PTR(IRTPListener, IRtpListener);
  ZS_DECLARE_TYPEDEF_PTR(IRTPSender, IRtpSender);
  ZS_DECLARE_TYPEDEF_PTR(IRTPReceiver, IRtpReceiver);
  ZS_DECLARE_TYPEDEF_PTR(IRTPTransport, IRtpTransport);
  ZS_DECLARE_TYPEDEF_PTR(IRTCPTransport, IRtcpTransport);
  ZS_DECLARE_TYPEDEF_PTR(ISCTPTransport, ISctpTransport);
  ZS_DECLARE_TYPEDEF_PTR(ISRTPSDESTransport, ISrtpSdesTransport);

  ZS_DECLARE_TYPEDEF_PTR(IDTLSTransportDelegate, IDtlsTransportDelegate);
  ZS_DECLARE_TYPEDEF_PTR(IDTMFSenderDelegate, IDtmfSenderDelegate);
  ZS_DECLARE_TYPEDEF_PTR(IICEGathererDelegate, IIceGathererDelegate);
  ZS_DECLARE_TYPEDEF_PTR(IICETransportDelegate, IIceTransportDelegate);
  ZS_DECLARE_TYPEDEF_PTR(IRTPListenerDelegate, IRtpListenerDelegate);
  ZS_DECLARE_TYPEDEF_PTR(IRTPSenderDelegate, IRtpSenderDelegate);
  ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverDelegate, IRtpReceiverDelegate);
  ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportDelegate, ISctpTransportDelegate);
  ZS_DECLARE_TYPEDEF_PTR(ISRTPSDESTransportDelegate, ISrtpSdesTransportDelegate);

  ZS_DECLARE_TYPEDEF_PTR(IDTLSTransportSubscription, IDtlsTransportSubscription);
  ZS_DECLARE_TYPEDEF_PTR(IDTMFSenderSubscription, IDtmfSenderSubscription);
  ZS_DECLARE_TYPEDEF_PTR(IICEGathererSubscription, IIceGathererSubscription);
  ZS_DECLARE_TYPEDEF_PTR(IICETransportSubscription, IIceTransportSubscription);
  ZS_DECLARE_TYPEDEF_PTR(IRTPListenerSubscription, IRtpListenerSubscription);
  ZS_DECLARE_TYPEDEF_PTR(IRTPSenderSubscription, IRtpSenderSubscription);
  ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverSubscription, IRtpReceiverSubscription);
  ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportSubscription, ISctpTransportSubscription);
  ZS_DECLARE_TYPEDEF_PTR(ISRTPSDESTransportSubscription, ISrtpSdesTransportSubscription);
#endif //0
#endif //ORTCLIB_INTERNAL
}
