#import "MediaController.h"
#import "MediaManagerDelegateWrapper.h"
#include <openpeer/services/ILogger.h>
#include <ortc/IORTC.h>
#include <ortc/internal/ortc_MediaManager.h>
#include <ortc/internal/ortc_MediaStream.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/internal/ortc_MediaEngine.h>
#include <ortc/test/TestMediaEngine.h>
#include <zsLib/MessageQueueThread.h>

@implementation MediaController

-(id)init
{
    self = [super init];
    if (self)
    {
        IORTC::singleton()->setup(zsLib::MessageQueueThread::createBasic("ortc.defaultDelegateMessageQueue"),
                                  zsLib::MessageQueueThread::createBasic("ortc.ortcMessageQueue"),
                                  zsLib::MessageQueueThread::createBasic("ortc.blockingMediaStartStopThread"));
      
        ortc::test::TestMediaEngineFactoryPtr overrideFactory(new ortc::test::TestMediaEngineFactory);
      
        openpeer::services::IFactory<ortc::internal::IMediaEngineFactory>::override(overrideFactory);
      
        openpeer::services::ILogger::setLogLevel("ortclib", zsLib::Log::None);
        openpeer::services::ILogger::setLogLevel("ortclib_webrtc", zsLib::Log::None);
        openpeer::services::ILogger::installStdOutLogger(false);
//        openpeer::services::ILogger::installTelnetLogger(59999, 60, true);
      
        ortc::internal::IMediaEnginePtr mediaEngine = ortc::internal::IMediaEngine::singleton();
    }
    return self;
}

-(void)getUserMedia
{
    IMediaManager::setup(mediaManagerDelegatePtr);
  
    IMediaManagerPtr mediaManager = IMediaManager::singleton();
  
    mediaManager->getUserMedia(IMediaManager::MediaStreamConstraints());
}

-(void)createReceiveForwardingAudioTrackWithPortNumber:(NSInteger)portNumber
{
    ortc::IMediaStreamPtr receiveMediaStreamPtr =
        ortc::internal::IMediaStreamForMediaManager::create(IMessageQueuePtr(), IMediaStreamDelegatePtr());
    ortc::internal::RemoteReceiveAudioStreamTrackPtr remoteAudioStreamTrack =
        ortc::internal::IRemoteReceiveAudioStreamTrackForMediaManager::create(IMessageQueuePtr(), IMediaStreamTrackDelegatePtr());
  
    ortc::internal::IMediaEnginePtr mediaEngine = ortc::internal::IMediaEngine::singleton();
  
    int audioChannel = mediaEngine->createReceiveForwardingVoiceChannel();
  
    ortc::test::TestMediaEnginePtr testMediaEngine = boost::dynamic_pointer_cast<ortc::test::TestMediaEngine>(mediaEngine);
    testMediaEngine->setVoiceReceivePort(audioChannel, (int)portNumber);

    ortc::internal::MediaStreamPtr mediaStream = boost::dynamic_pointer_cast<ortc::internal::MediaStream>(receiveMediaStreamPtr);
  
    mediaStream->forMediaManager().addTrack(remoteAudioStreamTrack);

    mediaStream->forMediaManager().setAudioChannel(audioChannel);
  
    char trackKey[30];
    std::sprintf(trackKey, "%d", (int)portNumber);
  
    receiveMediaStreams[trackKey] = receiveMediaStreamPtr;
    receiveAudioChannels[trackKey] = audioChannel;
}

-(void)createSendForwardingAudioTrackWithReceiverIPAddress:(NSString*)receiverIPAddress
                                     andReceiverPortNumber:(NSInteger)receiverPortNumber
                             andSourceAudioTrackPortNumber:(NSInteger)sourceTrackPortNumber
{
    const char* receiverIPAddressString = [receiverIPAddress UTF8String];
  
    char trackKey[30];
    std::sprintf(trackKey, "%s_%d", receiverIPAddressString, (int)receiverPortNumber);

    ortc::IMediaStreamPtr sendMediaStreamPtr =
        ortc::internal::IMediaStreamForMediaManager::create(IMessageQueuePtr(), IMediaStreamDelegatePtr());
    ortc::internal::RemoteSendAudioStreamTrackPtr remoteAudioStreamTrack =
        ortc::internal::IRemoteSendAudioStreamTrackForMediaManager::create(IMessageQueuePtr(), IMediaStreamTrackDelegatePtr());

    ortc::internal::IMediaEnginePtr mediaEngine = ortc::internal::IMediaEngine::singleton();
  
    char sourceTrackKey[30];
    std::sprintf(sourceTrackKey, "%d", (int)sourceTrackPortNumber);

    int audioChannel = mediaEngine->createSendForwardingVoiceChannel(receiveAudioChannels[sourceTrackKey]);
  
    ortc::test::TestMediaEnginePtr testMediaEngine = boost::dynamic_pointer_cast<ortc::test::TestMediaEngine>(mediaEngine);
    testMediaEngine->setSendAddress(audioChannel, receiverIPAddressString);
    testMediaEngine->setVoiceSendPort(audioChannel, (int)receiverPortNumber);
  
    ortc::internal::MediaStreamPtr mediaStream = boost::dynamic_pointer_cast<ortc::internal::MediaStream>(sendMediaStreamPtr);
  
    mediaStream->forMediaManager().addTrack(remoteAudioStreamTrack);
  
    mediaStream->forMediaManager().setAudioChannel(audioChannel);
  
    sendMediaStreams[trackKey] = sendMediaStreamPtr;
    sendAudioChannels[trackKey] = audioChannel;
}

-(void)setSendMediaStream:(ortc::IMediaStreamPtr)stream
{
}

@end
