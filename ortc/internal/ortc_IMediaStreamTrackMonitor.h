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
#include <ortc/IRTPTypes.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackMonitor);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackMonitorMediaChannel);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackMonitorSubscriberMedia);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackMonitorSubscriberRTP);
    ZS_DECLARE_INTERACTION_PROXY(IMediaStreamTrackMonitorDelegate);

    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackMonitorMediaChannelDelegate);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackMonitorMediaSubscriberMediaDelegate);
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackMonitorMediaSubscriberRTPDelegate);

    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForMediaStreamTrackMonitor);

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackMonitorTypes
    #pragma mark
  
    interaction IMediaStreamTrackMonitorTypes
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::AudioFrame, AudioFrame);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::VideoFrame, VideoFrame);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::ImmutableParameters, ImmutableParameters);

      typedef PUID ChannelID;
      typedef PUID SubscriberID;

      typedef PUID MonitorChannelID;
      typedef PUID MonitorSubscriberMediaID;
      typedef PUID MonitorSubscriberRTPID;

      typedef AnyPtr MonitorChannelContextPtr;
      typedef AnyPtr MonitorSubscriberMediaContextPtr;
      typedef AnyPtr MonitorSubscriberRTPContextPtr;

      struct NotifyMediaChannelAvailableInput
      {
        ChannelID channelID_ {};
        ImmutableMediaChannelTracePtr inMediaChannelTrace_;
        IMediaStreamTrackMonitorMediaChannelDelegatePtr delegate;
      };

      struct NotifyMediaChannelAvailableOutput
      {
        MonitorChannelID id_ {};
        MonitorChannelContextPtr context_ {};
        IMediaStreamTrackMonitorMediaChannelPtr monitor_;
      };

      struct NotifyMediaChannelSubscribedMediaInput
      {
        SubscriberID subscriberID_ {};
        IMediaStreamTrackMonitorMediaSubscriberMediaDelegatePtr delegate_;
      };

      struct NotifyMediaChannelSubscribedMediaOutput
      {
        MonitorSubscriberMediaID id_ {};
        MonitorSubscriberMediaContextPtr context_ {};
        IMediaStreamTrackMonitorSubscriberMediaPtr monitor_;
      };

      struct NotifyMediaChannelSubscribedRTPInput
      {
        SubscriberID subscriberID_{};
        IMediaStreamTrackMonitorMediaSubscriberRTPDelegatePtr delegate_;
      };

      struct NotifyMediaChannelSubscribedRTPOutput
      {
        MonitorSubscriberRTPID id_ {};
        MonitorSubscriberRTPContextPtr context_ {};
        IMediaStreamTrackMonitorSubscriberRTPPtr monitor_;
      };

    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackMonitor
    #pragma mark

    interaction IMediaStreamTrackMonitor : public IMediaStreamTrackMonitorTypes
    {
      static IMediaStreamTrackMonitorPtr create(ortc::IMediaStreamTrackPtr track);

      virtual PUID getID() const = 0;

      virtual void stop() const = 0;

      virtual void notifyMediaChannelAvailable(
                                               NotifyMediaChannelAvailableInput &input,
                                               NotifyMediaChannelAvailableInput &output
                                               ) = 0;

      virtual void notifyMediaChannelSubscribedMedia(
                                                     NotifyMediaChannelSubscribedMediaInput &input,
                                                     NotifyMediaChannelSubscribedMediaInput &output
                                                     ) = 0;

      virtual void notifyMediaChannelSubscribedRTP(
                                                   NotifyMediaChannelSubscribedRTPInput &input,
                                                   NotifyMediaChannelSubscribedRTPInput &output
                                                   ) = 0;

    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackMonitorDelegate
    #pragma mark

    interaction IMediaStreamTrackMonitorDelegate
    {
      virtual ~IMediaStreamTrackMonitorDelegate() {}
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackMonitorSubscription
    #pragma mark
  
    interaction IMediaStreamTrackMonitorSubscription
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
    #pragma mark IMediaStreamTrackMonitorMediaChannel
    #pragma mark
  
    interaction IMediaStreamTrackMonitorMediaChannel : public IMediaStreamTrackMonitorTypes
    {
      virtual void notifyMediaChannelGone(
                                          MonitorChannelID id,
                                          MonitorChannelContextPtr context
                                          ) = 0;

      virtual void notifyMediaChannelRTPPacket(
                                               MonitorChannelID id,
                                               MonitorChannelContextPtr context,
                                               RTPPacketPtr packet,
                                               bool &outFilterPacket
                                               ) = 0;

      virtual void notifyMediaChannelRTCPPacket(
                                                MonitorChannelID id,
                                                MonitorChannelContextPtr context,
                                                RTCPPacketPtr packet,
                                                bool &outFilterPacket
                                                ) = 0;
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackMonitorMediaChannelDelegate
    #pragma mark

    interaction IMediaStreamTrackMonitorMediaChannelDelegate : public IMediaStreamTrackMonitorTypes
    {
      virtual void notifyMonitorMediaChannelRTPPacket(
                                                      MonitorChannelID id,
                                                      RTPPacketPtr packet
                                                      ) = 0;
      virtual void notifyMonitorMediaChannelRTCPPacket(
                                                       MonitorChannelID id,
                                                       RTPPacketPtr packet
                                                       ) = 0;
    };


    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackMonitorSubscriberMedia
    #pragma mark
  
    interaction IMediaStreamTrackMonitorSubscriberMedia : public IMediaStreamTrackMonitorTypes
    {
      virtual void notifyMonitorSubscriberMediaGone(
                                                    MonitorSubscriberMediaID id,
                                                    MonitorSubscriberMediaContextPtr context
                                                    ) = 0;

      virtual void notifyMonitorSubscriberMediaAudio(
                                                     MonitorSubscriberMediaID id,
                                                     MonitorSubscriberMediaContextPtr context,
                                                     AudioFramePtr frame,
                                                     bool &outFilterFrame
                                                     ) = 0;

      virtual void notifyMonitorSubscriberMediaVideo(
                                                     MonitorSubscriberMediaID id,
                                                     MonitorSubscriberMediaContextPtr context,
                                                     VideoFramePtr frame,
                                                     bool &outFilterFrame
                                                     ) = 0;
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackMonitorSubscriberMediaDelegate
    #pragma mark

    interaction IMediaStreamTrackMonitorSubscriberMediaDelegate : public IMediaStreamTrackMonitorTypes
    {
      virtual void notifyMonitorSubscriberMediaAudio(
                                                     MonitorSubscriberMediaID id,
                                                     AudioFramePtr frame
                                                     ) = 0;
      virtual void notifyMonitorSubscriberMediaVideo(
                                                     MonitorSubscriberMediaID id,
                                                     VideoFramePtr frame
                                                     ) = 0;
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackMonitorSubscriberRTP
    #pragma mark
  
    interaction IMediaStreamTrackMonitorSubscriberRTP : public IMediaStreamTrackMonitorTypes
    {
      virtual void notifyMonitorSubscriberRTPGone(
                                                  MonitorSubscriberRTPID id,
                                                  MonitorSubscriberMediaContextPtr context
                                                  ) = 0;

      virtual void notifyMonitorSubscriberEncoderRTPPacket(
                                                           MonitorSubscriberRTPID id,
                                                           MonitorSubscriberMediaContextPtr context,
                                                           RTPPacketPtr packet,
                                                           bool &outFilterPacket
                                                           ) = 0;

      virtual void notifyMonitorSubscriberEncoderRTPRTCPPacket(
                                                               MonitorSubscriberRTPID id,
                                                               MonitorSubscriberMediaContextPtr context,
                                                               RTCPPacketPtr packet,
                                                               bool &outFilterPacket
                                                               ) = 0;

      virtual void notifyMonitorSubscriberRTPFeedback(
                                                      MonitorSubscriberRTPID id,
                                                      MonitorSubscriberMediaContextPtr context,
                                                      RTCPPacketPtr packet,
                                                      bool &outFilterPacket
                                                      ) = 0;
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackMonitorSubscriberRTPDelegate
    #pragma mark

    interaction IMediaStreamTrackMonitorSubscriberRTPDelegate : public IMediaStreamTrackMonitorTypes
    {
      virtual void notifyMonitorSubscriberRTPEncoderRTPPacket(
                                                        MonitorSubscriberRTPID id,
                                                        RTPPacketPtr packet
                                                        ) = 0;
      virtual void notifyMonitorSubscriberRTPEncoderRTCPPacket(
                                                         MonitorSubscriberRTPID id,
                                                         RTPPacketPtr packet
                                                         ) = 0;

      virtual void notifyMonitorSubscriberRTPFeedback(
                                                      MonitorSubscriberRTPID id,
                                                      RTPPacketPtr packet
                                                      ) = 0;
    };
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IMediaStreamTrackMonitorDelegate)
//ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
//ZS_DECLARE_PROXY_METHOD_2(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::internal::IMediaStreamTrackMonitorDelegate, ortc::internal::IMediaStreamTrackMonitorSubscription)
//ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
//ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
