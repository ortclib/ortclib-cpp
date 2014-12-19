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
      
        openpeer::services::ILogger::setLogLevel("ortclib", zsLib::Log::Debug);
        openpeer::services::ILogger::setLogLevel("ortclib_webrtc", zsLib::Log::Debug);
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

-(void)createReceiveForwardingVoiceChannel
{
    receiveMediaStreamPtr = ortc::internal::IMediaStreamForMediaManager::create(IMessageQueuePtr(), IMediaStreamDelegatePtr());
    ortc::internal::RemoteReceiveAudioStreamTrackPtr remoteAudioStreamTrack =
        ortc::internal::IRemoteReceiveAudioStreamTrackForMediaManager::create(IMessageQueuePtr(), IMediaStreamTrackDelegatePtr());
  
    ortc::internal::IMediaEnginePtr mediaEngine = ortc::internal::IMediaEngine::singleton();
  
    audioChannel = mediaEngine->createReceiveForwardingVoiceChannel();
  
    ortc::test::TestMediaEnginePtr testMediaEngine = boost::dynamic_pointer_cast<ortc::test::TestMediaEngine>(mediaEngine);
    testMediaEngine->setVoiceReceivePort(audioChannel, 20010);

    ortc::internal::MediaStreamPtr mediaStream = boost::dynamic_pointer_cast<ortc::internal::MediaStream>(receiveMediaStreamPtr);
  
    mediaStream->forMediaManager().addTrack(remoteAudioStreamTrack);

    mediaStream->forMediaManager().setAudioChannel(audioChannel);
}

-(void)createSendForwardingVoiceChannel
{
    NSString* receiverIPAddressString = @"192.168.1.10";
  
    const char* receiverIPAddress = [receiverIPAddressString UTF8String];
  
    sendMediaStreamPtr = ortc::internal::IMediaStreamForMediaManager::create(IMessageQueuePtr(), IMediaStreamDelegatePtr());
    ortc::internal::RemoteSendAudioStreamTrackPtr remoteAudioStreamTrack =
        ortc::internal::IRemoteSendAudioStreamTrackForMediaManager::create(IMessageQueuePtr(), IMediaStreamTrackDelegatePtr());

    ortc::internal::IMediaEnginePtr mediaEngine = ortc::internal::IMediaEngine::singleton();
  
    forwardingAudioChannel = mediaEngine->createSendForwardingVoiceChannel(audioChannel);
  
    ortc::test::TestMediaEnginePtr testMediaEngine = boost::dynamic_pointer_cast<ortc::test::TestMediaEngine>(mediaEngine);
    testMediaEngine->setSendAddress(forwardingAudioChannel, receiverIPAddress);
    testMediaEngine->setVoiceSendPort(forwardingAudioChannel, 20012);
  
    ortc::internal::MediaStreamPtr mediaStream = boost::dynamic_pointer_cast<ortc::internal::MediaStream>(sendMediaStreamPtr);
  
    mediaStream->forMediaManager().addTrack(remoteAudioStreamTrack);
  
    mediaStream->forMediaManager().setAudioChannel(forwardingAudioChannel);
}

-(void)setSendMediaStream:(ortc::IMediaStreamPtr)stream
{
    sendMediaStreamPtr = stream;
}

@end
