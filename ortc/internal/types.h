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
#include <ortc/services/types.h>

#include <zsLib/eventing/types.h>

#include <zsLib/IFactory.h>
#include <zsLib/IWakeDelegate.h>
#include <zsLib/WeightedMovingAverage.h>

namespace ortc
{
  namespace internal
  {
    using std::make_shared;
    using zsLib::IFactory;

    using zsLib::UINT;
    using zsLib::PTRNUMBER;
    using zsLib::string;
    using zsLib::Noop;
    using zsLib::MessageQueueAssociator;
    using zsLib::RecursiveLock;
    using zsLib::AutoRecursiveLock;
    using zsLib::Lock;
    using zsLib::AutoLock;
    using zsLib::Log;
    using zsLib::Stringize;
    using zsLib::AutoPUID;
    using zsLib::Singleton;
    using zsLib::SingletonLazySharedPtr;
    using zsLib::IPAddress;
    using zsLib::WeightedMovingAverageDouble;
    using zsLib::WeightedMovingAverageUsingTotalDouble;

    ZS_DECLARE_USING_PTR(zsLib, IMessageQueue);
    ZS_DECLARE_USING_PTR(zsLib, IMessageQueueNotify);
    ZS_DECLARE_USING_PTR(zsLib, IMessageQueueThread);
    ZS_DECLARE_USING_PTR(zsLib, IMessageQueueMessage);

    ZS_DECLARE_USING_PTR(zsLib, IMessageQueue);
    ZS_DECLARE_USING_PTR(zsLib, IMessageQueueThread);
    ZS_DECLARE_USING_PTR(zsLib, ISettings);
    ZS_DECLARE_USING_PTR(zsLib, ISettingsApplyDefaultsDelegate);
    ZS_DECLARE_USING_PTR(zsLib, ITimer);
    ZS_DECLARE_USING_PTR(zsLib, Socket);
    ZS_DECLARE_USING_PTR(zsLib, Promise);
    ZS_DECLARE_USING_PROXY(zsLib, IWakeDelegate);

    ZS_DECLARE_USING_PTR(zsLib, ISingletonManagerDelegate);

    ZS_DECLARE_TYPEDEF_PTR(zsLib::AutoRecursiveLock, AutoRecursiveLock);

    ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);

    using ortc::services::SharedRecursiveLock;

    ZS_DECLARE_USING_PTR(ortc::services, IDNS);
    ZS_DECLARE_USING_PTR(ortc::services, IDNSQuery);
    ZS_DECLARE_USING_PTR(ortc::services, IBackOffTimer);
    ZS_DECLARE_USING_PTR(ortc::services, IBackOffTimerPattern);
    ZS_DECLARE_USING_PTR(ortc::services, STUNPacket);
    ZS_DECLARE_USING_PTR(ortc::services, ISTUNRequester);
    ZS_DECLARE_USING_PTR(zsLib, IPromiseSettledDelegate);

    ZS_DECLARE_USING_PROXY(ortc::services, IBackOffTimerDelegate);
    ZS_DECLARE_USING_PROXY(ortc::services, IDNSDelegate);
    ZS_DECLARE_USING_PROXY(ortc::services, ISTUNDiscoveryDelegate);
    ZS_DECLARE_USING_PROXY(ortc::services, ITURNSocketDelegate);
    ZS_DECLARE_USING_PROXY(zsLib, IPromiseDelegate);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (forwards)
    #pragma mark

    ZS_DECLARE_CLASS_PTR(RTPPacket);
    ZS_DECLARE_CLASS_PTR(RTCPPacket);

    ZS_DECLARE_INTERACTION_PTR(IDataTransportForSecureTransport);
    ZS_DECLARE_INTERACTION_PTR(ISecureTransport);
    ZS_DECLARE_INTERACTION_PTR(ISecureTransportForRTPSender);
    ZS_DECLARE_INTERACTION_PTR(ISecureTransportForSRTPTransport);
    ZS_DECLARE_INTERACTION_PTR(ISecureTransportForICETransport);
    ZS_DECLARE_INTERACTION_PTR(ISecureTransportForRTPListener);
    ZS_DECLARE_INTERACTION_PTR(ISRTPTransport);

    ZS_DECLARE_CLASS_PTR(ORTC);
    ZS_DECLARE_CLASS_PTR(Certificate);
    ZS_DECLARE_CLASS_PTR(DataChannel);
    ZS_DECLARE_CLASS_PTR(DTMFSender);
    ZS_DECLARE_CLASS_PTR(DTLSTransport);
    ZS_DECLARE_CLASS_PTR(ICEGatherer);
    ZS_DECLARE_CLASS_PTR(ICEGathererRouter);
    ZS_DECLARE_CLASS_PTR(ICETransport);
    ZS_DECLARE_CLASS_PTR(ICETransportController);
    ZS_DECLARE_CLASS_PTR(Identity);
    ZS_DECLARE_CLASS_PTR(MediaDevices);
    ZS_DECLARE_CLASS_PTR(MediaStreamTrack);
    ZS_DECLARE_CLASS_PTR(RTPListener);
    ZS_DECLARE_CLASS_PTR(RTPMediaEngine);
    ZS_DECLARE_CLASS_PTR(RTPReceiver);
    ZS_DECLARE_CLASS_PTR(RTPReceiverChannel);
    ZS_DECLARE_CLASS_PTR(RTPReceiverChannelAudio);
    ZS_DECLARE_CLASS_PTR(RTPReceiverChannelVideo);
    ZS_DECLARE_CLASS_PTR(RTPSender);
    ZS_DECLARE_CLASS_PTR(RTPSenderChannel);
    ZS_DECLARE_CLASS_PTR(RTPSenderChannelAudio);
    ZS_DECLARE_CLASS_PTR(RTPSenderChannelVideo);
    ZS_DECLARE_CLASS_PTR(StatsReport);
    ZS_DECLARE_CLASS_PTR(SCTPTransport);
    ZS_DECLARE_CLASS_PTR(SRTPSDESTransport);
    ZS_DECLARE_CLASS_PTR(SRTPTransport);

    ZS_DECLARE_INTERACTION_PROXY(ISecureTransportDelegate);
    ZS_DECLARE_INTERACTION_PROXY(ISRTPTransportDelegate);

    ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(ISRTPTransportSubscription, ISRTPTransportDelegate);
    ZS_DECLARE_INTERACTION_PROXY_SUBSCRIPTION(ISecureTransportSubscription, ISecureTransportDelegate);
  }
}
