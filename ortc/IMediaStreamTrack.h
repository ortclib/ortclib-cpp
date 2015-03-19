/*
 
 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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
#include <ortc/IConstraints.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackTypes
  #pragma mark
  
  interaction IMediaStreamTrackTypes : public ICapabilities,
                                       public IConstraints
  {
    ZS_DECLARE_STRUCT_PTR(Capabilities)
    ZS_DECLARE_STRUCT_PTR(Settings)
    ZS_DECLARE_STRUCT_PTR(Constraints)
    ZS_DECLARE_STRUCT_PTR(ConstraintSet)

    ZS_DECLARE_TYPEDEF_PTR(std::list<ConstraintSetPtr>, ConstraintSetList)

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackTypes::States
    #pragma mark
    
    enum States
    {
      State_Live,
      State_Ended,
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackTypes::Capabilities
    #pragma mark

    struct Capabilities : Any {
      CapabilityLongPtr     mWidth;
      CapabilityLongPtr     mHeight;
      CapabilityDoublePtr   mAspectRatio;
      CapabilityDoublePtr   mFrameRate;
      CapabilityStringPtr   mFacingMode;
      CapabilityStringPtr   mOrientation;
      CapabilityDoublePtr   mVolume;
      CapabilityLongPtr     mSampleRate;
      CapabilityLongPtr     mSampleSize;
      CapabilityBool        mEchoCancellation;
      CapabilityStringPtr   mDeviceID;
      CapabilityStringPtr   mGroupID;
    };
    
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackTypes::Settings
    #pragma mark

    struct Settings : Any {
      LongPtr     mWidth;
      LongPtr     mHeight;
      DoublePtr   mAspectRatio;
      DoublePtr   mFrameRate;
      StringPtr   mFacingMode;
      StringPtr   mOrientation;
      DoublePtr   mVolume;
      LongPtr     mSampleRate;
      LongPtr     mSampleSize;
      BoolPtr     mEchoCancellation;
      StringPtr   mDeviceID;
      StringPtr   mGroupID;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackTypes::Constraints
    #pragma mark

    struct Constraints {
      ConstraintSetList mAdvanced;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackTypes::ConstraintSet
    #pragma mark

    struct ConstraintSet {
      ConstraintLongPtr     mWidth;
      ConstraintLongPtr     mHeight;
      ConstraintDoublePtr   mAspectRatio;
      ConstraintDoublePtr   mFrameRate;
      ConstraintStringPtr   mFacingMode;
      ConstraintStringPtr   mOrientation;
      ConstraintDoublePtr   mVolume;
      ConstraintLongPtr     mSampleRate;
      ConstraintLongPtr     mSampleSize;
      ConstraintBoolPtr     mEchoCancellation;
      ConstraintStringPtr   mDeviceID;
      ConstraintStringPtr   mGroupID;
    };

  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrack
  #pragma mark
  
  interaction IMediaStreamTrack : public Any,
                                  public IMediaStreamTrackTypes,
                                  public IStatsProvider
  {
    virtual PUID getID() const = 0;

    virtual String kind() const = 0;
    virtual String id() const = 0;
    virtual String label() const = 0;
    virtual bool enabled() const = 0;
    virtual void enabeld(bool enabled) = 0;
    virtual bool muted() const = 0;
    virtual bool readOnly() const = 0;
    virtual bool remote() const = 0;
    virtual bool readyState() const = 0;

    virtual IMediaStreamTrackPtr clone() const = 0;

    virtual void stop() = 0;

    virtual CapabilitiesPtr getCapabilities() const = 0;
    virtual SettingsPtr getSettings() const = 0;

    virtual ConstraintsPtr getConstraints() const = 0;

    virtual PromisePtr applyConstraints(const Constraints &constraints);
  };
  
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackDelegate
  #pragma mark

  interaction IMediaStreamTrackDelegate
  {
    virtual void onMediaStreamTrackMute(
                                        IMediaStreamTrackPtr track,
                                        bool isMuted
                                        ) = 0;

    virtual void onMediaStreamTrackEnded(IMediaStreamTrackPtr track) = 0;
    virtual void onMediaStreamTrackOverConstrained(IMediaStreamTrackPtr track) = 0;
  };
  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IMediaStreamTrackSubscription
  #pragma mark
  
  interaction IMediaStreamTrackSubscription
  {
    virtual PUID getID() const = 0;
    
    virtual void cancel() = 0;

    virtual void background() = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IMediaStreamTrackDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_METHOD_2(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
ZS_DECLARE_PROXY_METHOD_1(onMediaStreamTrackEnded, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_METHOD_1(onMediaStreamTrackOverConstrained, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IMediaStreamTrackDelegate, ortc::IMediaStreamTrackSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onMediaStreamTrackEnded, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onMediaStreamTrackOverConstrained, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
