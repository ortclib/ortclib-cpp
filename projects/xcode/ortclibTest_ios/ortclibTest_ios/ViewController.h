#import <UIKit/UIKit.h>
#include <ortc/internal/ortc_MediaManager.h>

@interface ViewController : UIViewController
{
    IBOutlet UIButton* _btnTest1;
    IBOutlet UIButton* _btnTest2;
    IBOutlet UIButton* _btnTest3;
    IBOutlet UIButton* _btnTest4;
    IBOutlet UIButton* _btnTest5;
    IBOutlet UIButton* _btnTest6;
    IBOutlet UITextField* _sendIPAddressTextField;
    IBOutlet UITextField* _sendPortTextField;
    IBOutlet UITextField* _receivePortTextField1;
    IBOutlet UITextField* _receivePortTextField2;
    IBOutlet UIImageView* _imgView1;
    IBOutlet UIImageView* _imgView2;
  
    ortc::IMediaManagerDelegatePtr mediaManagerDelegatePtr;
    ortc::IMediaStreamPtr sendMediaStreamPtr;
    ortc::IMediaStreamPtr receiveMediaStream1Ptr;
    ortc::IMediaStreamPtr receiveMediaStream2Ptr;
    int sendAudioChannel;
    int receiveAudioChannel1;
    int receiveAudioChannel2;
    int videoChannel;
}

-(IBAction)test1;
-(IBAction)test2;
-(IBAction)test3;
-(IBAction)test4;
-(IBAction)test5;
-(IBAction)test6;

-(void)setSendMediaStream:(ortc::IMediaStreamPtr)stream;

@end
