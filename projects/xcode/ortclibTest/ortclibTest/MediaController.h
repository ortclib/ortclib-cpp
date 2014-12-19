#import <Foundation/Foundation.h>
#include <ortc/internal/ortc_MediaManager.h>

@interface MediaController : NSObject 
{
    ortc::IMediaManagerDelegatePtr mediaManagerDelegatePtr;
    ortc::IMediaStreamPtr sendMediaStreamPtr;
    ortc::IMediaStreamPtr receiveMediaStreamPtr;
    int audioChannel;
    int forwardingAudioChannel;
    int videoChannel;
}

-(void)getUserMedia;
-(void)createReceiveForwardingVoiceChannel;
-(void)createSendForwardingVoiceChannel;
-(void)setSendMediaStream:(ortc::IMediaStreamPtr)stream;

@end
