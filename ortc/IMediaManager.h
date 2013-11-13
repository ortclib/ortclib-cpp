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
    
    virtual void getUserMedia(MediaStreamConstraints constraints) = 0;
  };
  
  interaction IMediaManagerDelegate
  {
    typedef IMediaManager::UserMediaError UserMediaError;
    
    virtual void onMediaManagerSuccessCallback(IMediaStreamPtr stream) = 0;
    virtual void onMediaManagerErrorCallback(UserMediaError error) = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IMediaManagerDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamPtr, IMediaStreamPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaManager::UserMediaError, UserMediaError)
ZS_DECLARE_PROXY_METHOD_1(onMediaManagerSuccessCallback, IMediaStreamPtr)
ZS_DECLARE_PROXY_METHOD_1(onMediaManagerErrorCallback, UserMediaError)
ZS_DECLARE_PROXY_END()