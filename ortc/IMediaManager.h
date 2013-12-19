/*
 
 Copyright (c) 2013, SMB Phone Inc. / Hookflash Inc.
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 
 */

#pragma once

#include <ortc/types.h>

namespace ortc
{
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaManager
  #pragma mark
  
  interaction IMediaManager
  {
    struct MediaStreamConstraints;
    struct UserMediaError;
    
    typedef std::map<String, String> MediaStreamConstraintsMap;
    typedef boost::shared_ptr<MediaStreamConstraintsMap> MediaStreamConstraintsMapPtr;
    typedef boost::weak_ptr<MediaStreamConstraintsMap> MediaStreamConstraintsMapWeakPtr;
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark VideoOrientations
    #pragma mark
    
    enum VideoOrientations
    {
      VideoOrientation_LandscapeLeft,
      VideoOrientation_PortraitUpsideDown,
      VideoOrientation_LandscapeRight,
      VideoOrientation_Portrait
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark OutputAudioRoutes
    #pragma mark
    
    enum OutputAudioRoutes
    {
      OutputAudioRoute_Headphone,
      OutputAudioRoute_BuiltInReceiver,
      OutputAudioRoute_BuiltInSpeaker
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamConstraints
    #pragma mark
    
    struct MediaStreamConstraints
    {
      MediaStreamConstraintsMap mConstraints;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark UserMediaError
    #pragma mark
    
    struct UserMediaError
    {
      String mConstraintName;
    };
    
    static IMediaManagerPtr singleton();
    
    static void setup(IMediaManagerDelegatePtr delegate);
    
    virtual PUID getID() const = 0;
    
    virtual IMediaManagerSubscriptionPtr subscribe(IMediaManagerDelegatePtr delegate) = 0;

    virtual void getUserMedia(MediaStreamConstraints constraints) = 0;
    
    virtual void setDefaultVideoOrientation(VideoOrientations orientation) = 0;
    virtual VideoOrientations getDefaultVideoOrientation() = 0;
    virtual void setRecordVideoOrientation(VideoOrientations orientation) = 0;
    virtual VideoOrientations getRecordVideoOrientation() = 0;
    virtual void setVideoOrientation() = 0;
    
    virtual void setMuteEnabled(bool enabled) = 0;
    virtual bool getMuteEnabled() = 0;
    virtual void setLoudspeakerEnabled(bool enabled) = 0;
    virtual bool getLoudspeakerEnabled() = 0;
    virtual OutputAudioRoutes getOutputAudioRoute() = 0;
  };
  
  interaction IMediaManagerDelegate
  {
    typedef IMediaManager::UserMediaError UserMediaError;
    typedef IMediaManager::OutputAudioRoutes OutputAudioRoutes;
    
    virtual void onMediaManagerSuccessCallback(IMediaStreamPtr stream) = 0;
    virtual void onMediaManagerErrorCallback(UserMediaError error) = 0;
    virtual void onMediaManagerAudioRouteChanged(OutputAudioRoutes audioRoute) = 0;
  };
  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaManagerSubscription
  #pragma mark
  
  interaction IMediaManagerSubscription
  {
    virtual PUID getID() const = 0;
    
    virtual void cancel() = 0;
    
    virtual void background() = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IMediaManagerDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamPtr, IMediaStreamPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaManager::UserMediaError, UserMediaError)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaManager::OutputAudioRoutes, OutputAudioRoutes)
ZS_DECLARE_PROXY_METHOD_1(onMediaManagerSuccessCallback, IMediaStreamPtr)
ZS_DECLARE_PROXY_METHOD_1(onMediaManagerErrorCallback, UserMediaError)
ZS_DECLARE_PROXY_METHOD_1(onMediaManagerAudioRouteChanged, OutputAudioRoutes)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IMediaManagerDelegate, ortc::IMediaManagerSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamPtr, IMediaStreamPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaManager::UserMediaError, UserMediaError)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaManager::OutputAudioRoutes, OutputAudioRoutes)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onMediaManagerSuccessCallback, IMediaStreamPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onMediaManagerErrorCallback, UserMediaError)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onMediaManagerAudioRouteChanged, OutputAudioRoutes)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
