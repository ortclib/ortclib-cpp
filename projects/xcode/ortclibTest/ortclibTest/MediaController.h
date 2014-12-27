#import <Foundation/Foundation.h>
#include <ortc/internal/ortc_MediaManager.h>

@interface MediaController : NSObject 
{
    ortc::IMediaManagerDelegatePtr mediaManagerDelegatePtr;
    std::map<std::string, ortc::IMediaStreamPtr> sendMediaStreams;
    std::map<std::string, ortc::IMediaStreamPtr> receiveMediaStreams;
    std::map<std::string, int> sendAudioChannels;
    std::map<std::string, int> receiveAudioChannels;
}

-(void)getUserMedia;
-(void)createReceiveForwardingAudioTrackWithPortNumber:(NSInteger)portNumber;
-(void)createSendForwardingAudioTrackWithReceiverIPAddress:(NSString*)receiverIPAddress
                                     andReceiverPortNumber:(NSInteger)receiverPortNumber
                             andSourceAudioTrackPortNumber:(NSInteger)sourceTrackPortNumber;
-(void)setSendMediaStream:(ortc::IMediaStreamPtr)stream;

@end
