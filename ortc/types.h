/*

 Copyright (c) 2013, SMB Phone Inc. / Hookflash Inc.
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
  using zsLib::Duration;
  using zsLib::String;
  using zsLib::RecursiveLock;
  using zsLib::IPAddress;
  using zsLib::IMessageQueue;
  using zsLib::IMessageQueuePtr;
  using zsLib::XML::Element;
  using zsLib::XML::ElementPtr;

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
  ZS_DECLARE_INTERACTION_PTR(IRTPSender)
  ZS_DECLARE_INTERACTION_PTR(IRTPReceiver)

  ZS_DECLARE_INTERACTION_PROXY(IDTLSTransportDelegate)
  ZS_DECLARE_INTERACTION_PROXY(IICETransportDelegate)

  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IDTLSTransportSubscription, IDTLSTransportDelegate)
  ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IICETransportSubscription, IICETransportDelegate)










  interaction IRTCStream;
  typedef boost::shared_ptr<IRTCStream> IRTCStreamPtr;
  typedef boost::weak_ptr<IRTCStream> IRTCStreamWeakPtr;
  
  typedef std::list<IRTCStreamPtr> RTCStreamList;
  typedef boost::shared_ptr<RTCStreamList> RTCStreamListPtr;
  typedef boost::weak_ptr<RTCStreamList> RTCStreamListWeakPtr;

  interaction IRTCTrack;
  typedef boost::shared_ptr<IRTCTrack> IRTCTrackPtr;
  typedef boost::weak_ptr<IRTCTrack> IRTCTrackWeakPtr;
  
  typedef std::list<IRTCTrackPtr> RTCTrackList;
  typedef boost::shared_ptr<RTCTrackList> RTCTrackListPtr;
  typedef boost::weak_ptr<RTCTrackList> RTCTrackListWeakPtr;

  interaction IRTCDTMFTrack;
  typedef boost::shared_ptr<IRTCDTMFTrack> IRTCDTMFTrackPtr;
  typedef boost::weak_ptr<IRTCDTMFTrack> IRTCDTMFTrackWeakPtr;
  
  interaction IRTCDTMFTrackDelegate;
  typedef boost::shared_ptr<IRTCDTMFTrackDelegate> IRTCDTMFTrackDelegatePtr;
  typedef boost::weak_ptr<IRTCDTMFTrackDelegate> IRTCDTMFTrackDelegateWeakPtr;
  typedef zsLib::Proxy<IRTCDTMFTrackDelegate> IRTCDTMFTrackDelegateProxy;

  interaction IRTCDataChannel;
  typedef boost::shared_ptr<IRTCDataChannel> IRTCDataChannelPtr;
  typedef boost::weak_ptr<IRTCDataChannel> IRTCDataChannelWeakPtr;
  
  interaction IRTCDataChannelDelegate;
  typedef boost::shared_ptr<IRTCDataChannelDelegate> IRTCDataChannelDelegatePtr;
  typedef boost::weak_ptr<IRTCDataChannelDelegate> IRTCDataChannelDelegateWeakPtr;
  typedef zsLib::Proxy<IRTCDataChannelDelegate> IRTCDataChannelDelegateProxy;
  
  interaction IMediaManager;
  typedef boost::shared_ptr<IMediaManager> IMediaManagerPtr;
  typedef boost::weak_ptr<IMediaManager> IMediaManagerWeakPtr;
  
  interaction IMediaManagerDelegate;
  typedef boost::shared_ptr<IMediaManagerDelegate> IMediaManagerDelegatePtr;
  typedef boost::weak_ptr<IMediaManagerDelegate> IMediaManagerDelegateWeakPtr;
  typedef zsLib::Proxy<IMediaManagerDelegate> IMediaManagerDelegateProxy;
  
  interaction IMediaManagerSubscription;
  typedef boost::shared_ptr<IMediaManagerSubscription> IMediaManagerSubscriptionPtr;
  typedef boost::weak_ptr<IMediaManagerSubscription> IMediaManagerSubscriptionWeakPtr;
  typedef zsLib::ProxySubscriptions<IMediaManagerDelegate, IMediaManagerSubscription> IMediaManagerDelegateSubscriptions;

  interaction IMediaStream;
  typedef boost::shared_ptr<IMediaStream> IMediaStreamPtr;
  typedef boost::weak_ptr<IMediaStream> IMediaStreamWeakPtr;
  
  interaction IMediaStreamDelegate;
  typedef boost::shared_ptr<IMediaStreamDelegate> IMediaStreamDelegatePtr;
  typedef boost::weak_ptr<IMediaStreamDelegate> IMediaStreamDelegateWeakPtr;
  typedef zsLib::Proxy<IMediaStreamDelegate> IMediaStreamDelegateProxy;
  
  interaction IMediaStreamSubscription;
  typedef boost::shared_ptr<IMediaStreamSubscription> IMediaStreamSubscriptionPtr;
  typedef boost::weak_ptr<IMediaStreamSubscription> IMediaStreamSubscriptionWeakPtr;
  typedef zsLib::ProxySubscriptions<IMediaStreamDelegate, IMediaStreamSubscription> IMediaStreamDelegateSubscriptions;

  interaction IMediaStreamTrack;
  typedef boost::shared_ptr<IMediaStreamTrack> IMediaStreamTrackPtr;
  typedef boost::weak_ptr<IMediaStreamTrack> IMediaStreamTrackWeakPtr;
  
  typedef std::list<IMediaStreamTrackPtr> MediaStreamTrackList;
  typedef boost::shared_ptr<MediaStreamTrackList> MediaStreamTrackListPtr;
  typedef boost::weak_ptr<MediaStreamTrackList> MediaStreamTrackListWeakPtr;

  interaction IMediaStreamTrackDelegate;
  typedef boost::shared_ptr<IMediaStreamTrackDelegate> IMediaStreamTrackDelegatePtr;
  typedef boost::weak_ptr<IMediaStreamTrackDelegate> IMediaStreamTrackDelegateWeakPtr;
  typedef zsLib::Proxy<IMediaStreamTrackDelegate> IMediaStreamTrackDelegateProxy;
  
  interaction IMediaStreamTrackSubscription;
  typedef boost::shared_ptr<IMediaStreamTrackSubscription> IMediaStreamTrackSubscriptionPtr;
  typedef boost::weak_ptr<IMediaStreamTrackSubscription> IMediaStreamTrackSubscriptionWeakPtr;
  typedef zsLib::ProxySubscriptions<IMediaStreamTrackDelegate, IMediaStreamTrackSubscription> IMediaStreamTrackDelegateSubscriptions;

}
