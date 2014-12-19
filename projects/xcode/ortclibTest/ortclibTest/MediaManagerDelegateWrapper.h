#import <Foundation/Foundation.h>
#include <ortc/IMediaManager.h>

using namespace ortc;

@class MediaController;
class MediaManagerDelegateWrapper;

typedef boost::shared_ptr<MediaManagerDelegateWrapper> MediaManagerDelegateWrapperPtr;

class MediaManagerDelegateWrapper : public IMediaManagerDelegate
{
public:
    MediaManagerDelegateWrapper(MediaController* mediaController);
    
    static MediaManagerDelegateWrapperPtr create(MediaController* mediaController);
    
    virtual void onMediaManagerSuccessCallback(IMediaStreamPtr stream);
    virtual void onMediaManagerErrorCallback(IMediaManager::UserMediaError error);
    virtual void onMediaManagerAudioRouteChanged(IMediaManager::OutputAudioRoutes audioRoute);
  
private:
    MediaController* mediaController;
};
