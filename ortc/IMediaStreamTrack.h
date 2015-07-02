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
#include <ortc/ICapabilities.h>
#include <ortc/IConstraints.h>
#include <ortc/IStatsProvider.h>

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
    ZS_DECLARE_STRUCT_PTR(ConstraintSet)
    ZS_DECLARE_STRUCT_PTR(TrackConstraints)
    ZS_DECLARE_STRUCT_PTR(Constraints)

    ZS_DECLARE_TYPEDEF_PTR(std::list<ConstraintSetPtr>, ConstraintSetList)

    enum Kinds
    {
      Kind_First,

      Kind_Audio = Kind_First,
      Kind_Video,

      Kind_Last = Kind_Video,
    };

    static const char *toString(Kinds kind);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackTypes::States
    #pragma mark
    
    enum States
    {
      State_First,

      State_Live = State_First,
      State_Ended,

      State_Last = State_Ended,
    };

    static const char *toString(States state);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackTypes::Capabilities
    #pragma mark

    struct Capabilities : Any {
      Optional<CapabilityLong>    mWidth;
      Optional<CapabilityLong>    mHeight;
      Optional<CapabilityDouble>  mAspectRatio;
      Optional<CapabilityDouble>  mFrameRate;
      Optional<CapabilityString>  mFacingMode;
      Optional<CapabilityString>  mOrientation;
      Optional<CapabilityDouble>  mVolume;
      Optional<CapabilityLong>    mSampleRate;
      Optional<CapabilityLong>    mSampleSize;
      Optional<CapabilityBool>    mEchoCancellation;
      Optional<CapabilityDouble>  mLatency;

      String                      mDeviceID;
      String                      mGroupID;

      static CapabilitiesPtr create();
      static CapabilitiesPtr create(const CapabilitiesPtr &value);
      static CapabilitiesPtr create(const Capabilities &value);
      ElementPtr toDebug() const;
      String hash() const;
    };
    
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackTypes::Settings
    #pragma mark

    struct Settings : Any {
      Optional<LONG>     mWidth;
      Optional<LONG>     mHeight;
      Optional<double>   mAspectRatio;
      Optional<double>   mFrameRate;
      Optional<String>   mFacingMode;
      Optional<String>   mOrientation;
      Optional<double>   mVolume;
      Optional<LONG>     mSampleRate;
      Optional<LONG>     mSampleSize;
      Optional<bool>     mEchoCancellation;
      Optional<double>   mLatency;
      Optional<String>   mDeviceID;
      Optional<String>   mGroupID;

      static SettingsPtr create();
      static SettingsPtr create(const SettingsPtr &value);
      static SettingsPtr create(const Settings &value);
      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackTypes::ConstraintSet
    #pragma mark

    struct ConstraintSet {
      ConstrainLong     mWidth;
      ConstrainLong     mHeight;
      ConstrainDouble   mAspectRatio;
      ConstrainDouble   mFrameRate;
      ConstrainString   mFacingMode;
      ConstrainString   mOrientation;
      ConstrainDouble   mVolume;
      ConstrainLong     mSampleRate;
      ConstrainLong     mSampleSize;
      ConstrainBool     mEchoCancellation;
      ConstrainDouble   mLatency;
      ConstrainString   mDeviceID;
      ConstrainString   mGroupID;

      static ConstraintSetPtr create();
      static ConstraintSetPtr create(const ConstraintSetPtr &value);
      static ConstraintSetPtr create(const ConstraintSet &value);
      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackTypes::TrackConstraints
    #pragma mark

    struct TrackConstraints {
      ConstraintSetList mAdvanced;

      static TrackConstraintsPtr create();
      static TrackConstraintsPtr create(const TrackConstraintsPtr &value);
      static TrackConstraintsPtr create(const TrackConstraints &value);
      ElementPtr toDebug() const;
      String hash() const;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackTypes::Constraints
    #pragma mark

    struct Constraints {
      TrackConstraintsPtr mVideo;
      TrackConstraintsPtr mAudio;

      static ConstraintsPtr create();
      static ConstraintsPtr create(const ConstraintsPtr &value);
      static ConstraintsPtr create(const Constraints &value);
      ElementPtr toDebug() const;
      String hash() const;
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
    static ElementPtr toDebug(IMediaStreamTrackPtr object);

    virtual PUID getID() const = 0;

    virtual IMediaStreamTrackSubscriptionPtr subscribe(IMediaStreamTrackDelegatePtr delegate) = 0;

    virtual Kinds kind() const = 0;
    virtual String id() const = 0;
    virtual String label() const = 0;
    virtual bool enabled() const = 0;
    virtual void enabeld(bool enabled) = 0;
    virtual bool muted() const = 0;
    virtual bool readOnly() const = 0;
    virtual bool remote() const = 0;
    virtual States readyState() const = 0;

    virtual IMediaStreamTrackPtr clone() const = 0;

    virtual void stop() = 0;

    virtual CapabilitiesPtr getCapabilities() const = 0;
    virtual TrackConstraintsPtr getConstraints() const = 0;
    virtual SettingsPtr getSettings() const = 0;

    virtual PromisePtr applyConstraints(const TrackConstraints &constraints) = 0;
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
