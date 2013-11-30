#import "ViewController.h"
#import "MediaManagerDelegateWrapper.h"
#include <ortc/internal/ortc_MediaManager.h>
#include <ortc/internal/ortc_MediaStream.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>
#include <ortc/test/TestMediaEngine.h>

@implementation ViewController

-(IBAction)test1
{
    IMediaManager::setup(mediaManagerDelegatePtr);
  
    IMediaManagerPtr mediaManager = IMediaManager::singleton();
  
    mediaManager->getUserMedia(IMediaManager::MediaStreamConstraints());
}

-(IBAction)test2
{
    ortc::MediaStreamTrackListPtr audioTracks = mediaStreamPtr->getAudioTracks();
    ortc::MediaStreamTrackListPtr videoTracks = mediaStreamPtr->getVideoTracks();
  
    ortc::internal::LocalAudioStreamTrackPtr localAudioStreamTrack =
        boost::dynamic_pointer_cast<ortc::internal::LocalAudioStreamTrack>(audioTracks->front());
    ortc::internal::LocalVideoStreamTrackPtr localVideoStreamTrack =
        boost::dynamic_pointer_cast<ortc::internal::LocalVideoStreamTrack>(videoTracks->front());
  
    localVideoStreamTrack->forMediaManager().setRenderView((__bridge void*)_imgView1);
}

-(IBAction)test3
{
}

-(IBAction)test4
{
}

-(IBAction)test5
{
}

-(IBAction)test6
{
}

-(void)setMediaStream:(ortc::IMediaStreamPtr)stream
{
    mediaStreamPtr = stream;
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self)
        self.title = @"ORTC Media Test";
  
//    NSString* documents = [NSHomeDirectory() stringByAppendingString:@"/Documents/"];
//    const char* buffer = [documents UTF8String];
//    const char* receiverIPAddress = [receiverIPAddressTextField.text UTF8String];
//    const char* receiverIPAddress = "127.0.0.1";
  
    mediaManagerDelegatePtr = MediaManagerDelegateWrapper::create();
  
    ortc::test::TestMediaEngineFactoryPtr overrideFactory(new ortc::test::TestMediaEngineFactory);
  
    ortc::internal::Factory::override(overrideFactory);
  
//    IClient::setLogLevel(IClient::Log::Trace);
//    IClient::setLogLevel("ortclib", IClient::Log::Debug);         // recommend Debug
//    IClient::installTelnetLogger(59999, 60, true);
  
    ortc::internal::IMediaEnginePtr mediaEngine = ortc::internal::IMediaEngine::singleton();

    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationChanged) name:@"orientationChanged" object:nil];
    [[UIApplication sharedApplication] setIdleTimerDisabled: YES];
  
    [_imgView1 addObserver:self forKeyPath:@"image"
                 options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld)
                 context:NULL];
    [_imgView2 addObserver:self forKeyPath:@"image"
                 options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld)
                 context:NULL];

}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change
                       context:(void *)context
{
    if (object == _imgView1 && [keyPath isEqualToString:@"image"])
    {
        UIImage* image = [change objectForKey:NSKeyValueChangeNewKey];
        [_imgView1 setFrame:CGRectMake(_imgView1.frame.origin.x, _imgView1.frame.origin.y, image.size.width, image.size.height)];
    }
    else if (object == _imgView2 && [keyPath isEqualToString:@"image"])
    {
        UIImage* image = [change objectForKey:NSKeyValueChangeNewKey];
        [_imgView2 setFrame:CGRectMake(_imgView2.frame.origin.x, _imgView2.frame.origin.y, image.size.width, image.size.height)];
    }
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:@"orientationChanged" object:nil];
    [_imgView1 removeObserver:self forKeyPath:@"image"];
    [_imgView2 removeObserver:self forKeyPath:@"image"];
}

- (void)orientationChanged
{
    ortc::internal::IMediaEnginePtr mediaEngine = ortc::internal::IMediaEngine::singleton();
  
    mediaEngine->setVideoOrientation();
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return YES;
}

-(void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
    [[NSNotificationCenter defaultCenter] postNotificationName:@"orientationChanged" object:nil];
}

@end
