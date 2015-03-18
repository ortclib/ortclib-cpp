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

#include <zsLib/Proxy.h>
#include <zsLib/IPAddress.h>
#include <zsLib/Promise.h>
#include <openpeer/services/types.h>

namespace ortc
{
  using zsLib::string;
  using zsLib::PUID;
  using zsLib::BYTE;
  using zsLib::WORD;
  using zsLib::DWORD;
  using zsLib::QWORD;
  using zsLib::USHORT;
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

  using zsLib::Optional;
  using zsLib::PromiseWith;

  ZS_DECLARE_USING_PTR(zsLib, Any)
  ZS_DECLARE_USING_PTR(zsLib, Promise)

  using openpeer::services::SharedRecursiveLock;
  using openpeer::services::LockedValue;

  ZS_DECLARE_USING_PTR(openpeer::services, SecureByteBlock)

  namespace JSON = zsLib::JSON;
  using JSON::Element;
  using JSON::ElementPtr;

  typedef zsLib::Exceptions::InvalidUsage InvalidStateError;
  typedef zsLib::Exceptions::InvalidArgument InvalidParameters;
  typedef zsLib::Exceptions::SyntaxError SyntaxError;

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark (forwards)
  #pragma mark

  ZS_DECLARE_INTERACTION_PTR(IDataChannel)
  ZS_DECLARE_INTERACTION_PTR(IDataTransport)
  ZS_DECLARE_INTERACTION_PTR(IDTLSTransport)
  ZS_DECLARE_INTERACTION_PTR(IDTMFSender)
  ZS_DECLARE_INTERACTION_PTR(IORTC)
  ZS_DECLARE_INTERACTION_PTR(IHelper)
  ZS_DECLARE_INTERACTION_PTR(IIdentity)
  ZS_DECLARE_INTERACTION_PTR(IICEGatherer)
  ZS_DECLARE_INTERACTION_PTR(IICETransport)
  ZS_DECLARE_INTERACTION_PTR(IICETransportController)
  ZS_DECLARE_INTERACTION_PTR(IMediaDevices)
  ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrack)
  ZS_DECLARE_INTERACTION_PTR(IRTPListener)
  ZS_DECLARE_INTERACTION_PTR(IRTPSender)
  ZS_DECLARE_INTERACTION_PTR(IRTPReceiver)
  ZS_DECLARE_INTERACTION_PTR(ISCTPTransport)
  ZS_DECLARE_INTERACTION_PTR(IStatsProvider)
  ZS_DECLARE_INTERACTION_PTR(IStatsReport)

  ZS_DECLARE_INTERACTION_PROXY(IDataChannelDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IDTLSTransportDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IDTMFSenderDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IICEGathererDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IICETransportDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IRTPListenerDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IMediaStreamTrackDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IRTPSenderDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IRTPReceiverDelegate)
  ZS_DECLARE_INTERACTION_PROXY(ISCTPTransportDelegate)

  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IDataChannelSubscription, IDataChannelDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IDTLSTransportSubscription, IDTLSTransportDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IDTMFSenderSubscription, IDTMFSenderDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IICEGathererSubscription, IICEGathererDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IICETransportSubscription, IICETransportDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IMediaStreamTrackSubscription, IMediaStreamTrackDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IRTPListenerSubscription, IRTPListenerDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IRTPSenderSubscription, IRTPSenderDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IRTPReceiverSubscription, IRTPReceiverDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(ISCTPTransportSubscription, ISCTPTransportDelegate)
}
