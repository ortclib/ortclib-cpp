#include "MediaManagerDelegateWrapper.h"

#import "MediaController.h"

MediaManagerDelegateWrapper::MediaManagerDelegateWrapper(MediaController* mediaController) :
    mediaController(mediaController)
{
}

MediaManagerDelegateWrapperPtr MediaManagerDelegateWrapper::create(MediaController* mediaController)
{
    return MediaManagerDelegateWrapperPtr(new MediaManagerDelegateWrapper(mediaController));
}

void MediaManagerDelegateWrapper::onMediaManagerSuccessCallback(IMediaStreamPtr stream)
{
    [mediaController setSendMediaStream:stream];
}

void MediaManagerDelegateWrapper::onMediaManagerErrorCallback(IMediaManager::UserMediaError error)
{
    printf("******************* MEDIA MANAGER ERROR CALLBACK *******************\n");
}

void MediaManagerDelegateWrapper::onMediaManagerAudioRouteChanged(IMediaManager::OutputAudioRoutes audioRoute)
{
    printf("******************* MEDIA MANAGER AUDIO ROUTE CHANGED - NOT SUPPORTED *******************\n");
}
