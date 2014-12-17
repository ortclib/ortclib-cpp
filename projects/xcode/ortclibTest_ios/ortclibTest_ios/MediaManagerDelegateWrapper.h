#import <Foundation/Foundation.h>
#include <ortc/IMediaManager.h>

using namespace ortc;

@class ViewController;
class MediaManagerDelegateWrapper;

typedef std::shared_ptr<MediaManagerDelegateWrapper> MediaManagerDelegateWrapperPtr;

class MediaManagerDelegateWrapper : public IMediaManagerDelegate
{
public:
  MediaManagerDelegateWrapper(ViewController* viewController);
  
  static MediaManagerDelegateWrapperPtr create(ViewController* viewController);
  
  virtual void onMediaManagerSuccessCallback(IMediaStreamPtr stream);
  virtual void onMediaManagerErrorCallback(IMediaManager::UserMediaError error);
  virtual void onMediaManagerAudioRouteChanged(IMediaManager::OutputAudioRoutes audioRoute);
  
private:
  ViewController* viewController;
};
