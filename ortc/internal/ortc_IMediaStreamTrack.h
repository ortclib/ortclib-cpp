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

#include <ortc/internal/types.h>
#include <ortc/IMediaStreamTrack.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrack);
    ZS_DECLARE_INTERACTION_PROXY(IMediaStreamTrackRTPDelegate);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackRTPSubscription);

    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForMediaStreamTrackChannel);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForMediaStreamTrackSubscriber);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForMediaStreamTrackSubscriberMedia);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForMediaStreamTrackSubscriberRTP);

    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackChannelForMediaStreamTrack);

    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackSubscriberForMediaStreamTrack);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackSubscriberMediaForMediaStreamTrack);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackSubscriberRTPForMediaStreamTrack);

    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPReceiver);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPReceiverChannel);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSender);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSenderChannel);

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrack
    #pragma mark
  
    interaction IMediaStreamTrack : public ortc::IMediaStreamTrack
    {
      // WARNING: extreme caution must be done not to cause a deadlock or delaying media by blocking. Gather information
      // needed within a lock but do any operations outside a lock as fast as possible.
      virtual IMediaStreamTrackRTPSubscriptionPtr subscribeRTP(
                                                               const Parameters &rtpEncodingParams,
                                                               IMediaStreamTrackRTPDelegatePtr delegate
                                                               ) = 0;
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackRTPDelegate
    #pragma mark

    interaction IMediaStreamTrackRTPDelegate : public IMediaStreamTrackDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);

      virtual void notifyMediaStreamTrackRTPPacket(
                                                   ImmutableMediaChannelTracePtr trace,
                                                   RTPPacketPtr packet
                                                   ) = 0;
      virtual void notifyMediaStreamTrackRTCPPacket(
                                                    ImmutableMediaChannelTracePtr trace,
                                                    RTCPPacketPtr packet
                                                    ) = 0;
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackRTPSubscription
    #pragma mark
  
    interaction IMediaStreamTrackRTPSubscription
    {
      virtual PUID getID() const = 0;
    
      virtual void cancel() = 0;

      virtual void background() = 0;
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForMediaStreamTrackChannel
    #pragma mark
  
    interaction IMediaStreamTrackForMediaStreamTrackChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForMediaStreamTrackChannel, ForMediaStreamTrackChannel);

      virtual ~IMediaStreamTrackForMediaStreamTrackChannel() {}
    };


    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForMediaStreamTrackSubscriber
    #pragma mark
  
    interaction IMediaStreamTrackForMediaStreamTrackSubscriber
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForMediaStreamTrackSubscriber, ForMediaStreamTrackSubscriber);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackSubscriberForMediaStreamTrack, UseSubscriber);

      virtual void notifySubscriberCancelled(UseSubscriberPtr subscriber) = 0;
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForMediaStreamTrackSubscriberMedia
    #pragma mark
  
    interaction IMediaStreamTrackForMediaStreamTrackSubscriberMedia : public IMediaStreamTrackForMediaStreamTrackSubscriber
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForMediaStreamTrackSubscriberMedia, ForMediaStreamTrackSubscriberMedia);

      virtual ~IMediaStreamTrackForMediaStreamTrackSubscriberMedia() {}
    };

    
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForMediaStreamTrackSubscriberRTP
    #pragma mark
  
    interaction IMediaStreamTrackForMediaStreamTrackSubscriberRTP : public IMediaStreamTrackForMediaStreamTrackSubscriber
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForMediaStreamTrackSubscriberRTP, ForMediaStreamTrackSubscriberRTP);

      virtual ~IMediaStreamTrackForMediaStreamTrackSubscriberRTP() {}
    };

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForMediaDevices
    #pragma mark

    interaction IMediaStreamTrackForMediaDevices
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForMediaDevices, ForMediaDevices);

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraints, TrackConstraints);

      static ForMediaDevicesPtr createForMediaDevices(
                                                      IMediaStreamTrackTypes::Kinds kind,
                                                      const TrackConstraints &constraints
                                                      );

      virtual PUID getID() const = 0;
    };

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForMediaStreamTrackSelector
    #pragma mark

    interaction IMediaStreamTrackForMediaStreamTrackSelector
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForMediaStreamTrackSelector, ForMediaStreamTrackSelector);

      static ForMediaStreamTrackSelectorPtr createForMediaStreamTrackSelector(IMediaStreamTrackTypes::Kinds kind);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPReceiver
    #pragma mark

    interaction IMediaStreamTrackForRTPReceiver
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiver, ForReceiver);

      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);

      static ForReceiverPtr createForReceiver(IMediaStreamTrackTypes::Kinds kind);

      virtual PUID getID() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPReceiverChannel
    #pragma mark

    interaction IMediaStreamTrackForRTPReceiverChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiverChannel, ForReceiverChannel);

      typedef IMediaStreamTrackTypes::Kinds Kinds;

      virtual PUID getID() const = 0;
      virtual Kinds kind() const = 0;
    };


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPSender
    #pragma mark

    interaction IMediaStreamTrackForRTPSender
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSender, ForSender);

      typedef IMediaStreamTrackTypes::Kinds Kinds;

      virtual PUID getID() const = 0;
      virtual Kinds kind() const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPSenderChannel
    #pragma mark

    interaction IMediaStreamTrackForRTPSenderChannel
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSenderChannel, ForSenderChannel);

      virtual PUID getID() const = 0;
    };

  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IMediaStreamTrackRTPDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::OverconstrainedErrorPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::ImmutableMediaChannelTracePtr, ImmutableMediaChannelTracePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::RTPPacketPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::RTCPPacketPtr, RTCPPacketPtr)
ZS_DECLARE_PROXY_METHOD_2(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
ZS_DECLARE_PROXY_METHOD_1(onMediaStreamTrackEnded, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_METHOD_2(onMediaStreamTrackOverConstrained, IMediaStreamTrackPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_METHOD_SYNC_2(notifyMediaStreamTrackRTPPacket, ImmutableMediaChannelTracePtr, RTPPacketPtr)
ZS_DECLARE_PROXY_METHOD_SYNC_2(notifyMediaStreamTrackRTCPPacket, ImmutableMediaChannelTracePtr, RTCPPacketPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::internal::IMediaStreamTrackRTPDelegate, ortc::internal::IMediaStreamTrackRTPSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::OverconstrainedErrorPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::ImmutableMediaChannelTracePtr, ImmutableMediaChannelTracePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::RTPPacketPtr, RTPPacketPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::RTCPPacketPtr, RTCPPacketPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onMediaStreamTrackEnded, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onMediaStreamTrackOverConstrained, IMediaStreamTrackPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_SYNC_2(notifyMediaStreamTrackRTPPacket, ImmutableMediaChannelTracePtr, RTPPacketPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_SYNC_2(notifyMediaStreamTrackRTCPPacket, ImmutableMediaChannelTracePtr, RTCPPacketPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
