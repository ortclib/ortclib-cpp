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
#include <openpeer/services/types.h>

namespace ortc
{
  using zsLib::string;
  using zsLib::PUID;
  using zsLib::BYTE;
  using zsLib::WORD;
  using zsLib::DWORD;
  using zsLib::QWORD;
  using zsLib::ULONG;
  using zsLib::Time;
  using zsLib::Hours;
  using zsLib::Minutes;
  using zsLib::Seconds;
  using zsLib::Milliseconds;
  using zsLib::Microseconds;
  using zsLib::Nanoseconds;
  using zsLib::String;
  using zsLib::RecursiveLock;
  using zsLib::IPAddress;
  using zsLib::IMessageQueue;
  using zsLib::IMessageQueuePtr;
  using zsLib::XML::Element;
  using zsLib::XML::ElementPtr;

  using openpeer::services::SharedRecursiveLock;
  using openpeer::services::LockedValue;

  ZS_DECLARE_USING_PTR(openpeer::services, SecureByteBlock)

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark (forwards)
  #pragma mark

  ZS_DECLARE_STRUCT_PTR(TrackDescription)
  ZS_DECLARE_STRUCT_PTR(RTPFlowParams)
  ZS_DECLARE_STRUCT_PTR(RTPRTXFlowParams)
  ZS_DECLARE_STRUCT_PTR(RTPFECFlowParams)
  ZS_DECLARE_STRUCT_PTR(RTPLayeredFlowParams)

  ZS_DECLARE_INTERACTION_PTR(IORTC)
  ZS_DECLARE_INTERACTION_PTR(IHelper)
  ZS_DECLARE_INTERACTION_PTR(IDTLSTransport)
  ZS_DECLARE_INTERACTION_PTR(IICETransport)
  ZS_DECLARE_INTERACTION_PTR(IRTCDTMFTrack)
  ZS_DECLARE_INTERACTION_PTR(IRTCDataChannel)
  ZS_DECLARE_INTERACTION_PTR(IRTCStream)
  ZS_DECLARE_INTERACTION_PTR(IRTCTrack)
  ZS_DECLARE_INTERACTION_PTR(IRTPSender)
  ZS_DECLARE_INTERACTION_PTR(IRTPReceiver)
  ZS_DECLARE_INTERACTION_PTR(IMediaManager)
  ZS_DECLARE_INTERACTION_PTR(IMediaStream)
  ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrack)
  
  ZS_DECLARE_TYPEDEF_PTR(std::list<IMediaStreamTrackPtr>, MediaStreamTrackList)

  ZS_DECLARE_INTERACTION_PROXY(IDTLSTransportDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IICETransportDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IMediaManagerDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IMediaStreamDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IMediaStreamTrackDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IRTCDataChannelDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IRTCDTMFTrackDelegate)

  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IDTLSTransportSubscription, IDTLSTransportDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IICETransportSubscription, IICETransportDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IMediaManagerSubscription, IMediaManagerDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IMediaStreamSubscription, IMediaStreamDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IMediaStreamTrackSubscription, IMediaStreamTrackDelegate)


  typedef std::list<IRTCStreamPtr> RTCStreamList;
  ZS_DECLARE_PTR(RTCStreamList)

  typedef std::list<IRTCTrackPtr> RTCTrackList;
  ZS_DECLARE_PTR(RTCTrackList)
}
