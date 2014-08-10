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

#include <ortc/types.h>
#include <openpeer/services/types.h>
#include <openpeer/services/IFactory.h>

namespace ortc
{
  namespace internal
  {
    using zsLib::string;
    using zsLib::Noop;
    using zsLib::MessageQueueAssociator;
    using zsLib::RecursiveLock;
    using zsLib::Log;
    using zsLib::Stringize;
    using zsLib::AutoBool;
    using zsLib::AutoPUID;
    using zsLib::AutoWORD;
    using zsLib::Singleton;
    using zsLib::SingletonLazySharedPtr;

    ZS_DECLARE_USING_PTR(zsLib, IMessageQueue)
    ZS_DECLARE_USING_PTR(zsLib, IMessageQueueNotify)
    ZS_DECLARE_USING_PTR(zsLib, IMessageQueueThread)
    ZS_DECLARE_USING_PTR(zsLib, IMessageQueueMessage)

    ZS_DECLARE_USING_PTR(zsLib, MessageQueue)
    ZS_DECLARE_USING_PTR(zsLib, MessageQueueThread)

    ZS_DECLARE_TYPEDEF_PTR(zsLib::AutoRecursiveLock, AutoRecursiveLock)

    ZS_DECLARE_USING_PTR(openpeer::services, IHTTP)
    ZS_DECLARE_USING_PTR(openpeer::services, IICESocket)
    ZS_DECLARE_USING_PTR(openpeer::services, IICESocketDelegate)
    ZS_DECLARE_USING_PTR(openpeer::services, IICESocketSubscription)
    ZS_DECLARE_USING_PTR(openpeer::services, IICESocketSession)
    ZS_DECLARE_USING_PTR(openpeer::services, IICESocketSessionDelegate)
    ZS_DECLARE_USING_PTR(openpeer::services, IICESocketSessionSubscription)

    ZS_DECLARE_USING_PROXY(openpeer::services, IWakeDelegate)

    using openpeer::services::IFactory;

    enum CameraTypes
    {
      CameraType_None,
      CameraType_Front,
      CameraType_Back
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (forwards)
    #pragma mark

    ZS_DECLARE_CLASS_PTR(Factory)
    ZS_DECLARE_CLASS_PTR(ORTC)
    ZS_DECLARE_CLASS_PTR(DTLSTransport)
    ZS_DECLARE_CLASS_PTR(ICETransport)
    ZS_DECLARE_CLASS_PTR(RTPSender)
    ZS_DECLARE_CLASS_PTR(RTPReceiver)
    ZS_DECLARE_CLASS_PTR(MediaManager)
    ZS_DECLARE_CLASS_PTR(MediaStream)
    ZS_DECLARE_CLASS_PTR(MediaTransport)
    ZS_DECLARE_CLASS_PTR(SendMediaTransport)
    ZS_DECLARE_CLASS_PTR(ReceiveMediaTransport)
    ZS_DECLARE_CLASS_PTR(MediaStreamTrack)
    ZS_DECLARE_CLASS_PTR(AudioStreamTrack)
    ZS_DECLARE_CLASS_PTR(LocalAudioStreamTrack)
    ZS_DECLARE_CLASS_PTR(RemoteReceiveAudioStreamTrack)
    ZS_DECLARE_CLASS_PTR(RemoteSendAudioStreamTrack)
    ZS_DECLARE_CLASS_PTR(LocalVideoStreamTrack)
    ZS_DECLARE_CLASS_PTR(RemoteReceiveVideoStreamTrack)
    ZS_DECLARE_CLASS_PTR(RemoteSendVideoStreamTrack)
    ZS_DECLARE_CLASS_PTR(MediaEngine)
    
    ZS_DECLARE_INTERACTION_PTR(IMediaTransport)
    ZS_DECLARE_INTERACTION_PTR(IMediaEngine)
    
    ZS_DECLARE_INTERACTION_PROXY(IMediaEngineDelegate)

    ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(IMediaEngineSubscription, IMediaEngineDelegate)

    class RTCStream;
    typedef boost::shared_ptr<RTCStream> RTCStreamPtr;
    typedef boost::weak_ptr<RTCStream> RTCStreamWeakPtr;
    
    class RTCTrack;
    typedef boost::shared_ptr<RTCTrack> RTCTrackPtr;
    typedef boost::weak_ptr<RTCTrack> RTCTrackWeakPtr;
    
    class RTCDTMFTrack;
    typedef boost::shared_ptr<RTCDTMFTrack> RTCDTMFTrackPtr;
    typedef boost::weak_ptr<RTCDTMFTrack> RTCDTMFTrackWeakPtr;
    
    class RTCDataChannel;
    typedef boost::shared_ptr<RTCDataChannel> RTCDataChannelPtr;
    typedef boost::weak_ptr<RTCDataChannel> RTCDataChannelWeakPtr;
  }
}
