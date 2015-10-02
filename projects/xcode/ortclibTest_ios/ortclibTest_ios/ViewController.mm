#import "ViewController.h"

#include "testing.h"

#include <iostream>

#include <zsLib/Log.h>
#include <zsLib/Singleton.h>

@implementation ViewController

-(IBAction)test1
{
  zsLib::SingletonManager::Initializer init;
  
  std::cout << "TEST NOW STARTING...\n\n";
  
  Testing::runAllTests();
  Testing::output();
  
  if (0 != Testing::getGlobalFailedVar()) {
    std::cout << "TEST FAILED!\n\n";
  }
}

-(IBAction)test2
{
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

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self)
        self.title = @"ORTC Test";
  

    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:@"orientationChanged" object:nil];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return NO;
}

@end
