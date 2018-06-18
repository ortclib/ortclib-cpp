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
#include <ortc/IRTPTypes.h>
#include <ortc/IStatsProvider.h>

#include <queue>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrackTypes
  //
  
  interaction IMediaStreamTrackTypes : public ICapabilities,
                                       public IConstraints
  {
    typedef PUID MediaChannelID;
    typedef std::list<MediaChannelID> MediaChannelIDList;
    typedef std::set<MediaChannelID> MediaChannelIDSet;
    
    ZS_DECLARE_STRUCT_PTR(Capabilities);
    ZS_DECLARE_STRUCT_PTR(Settings);
    ZS_DECLARE_TYPEDEF_PTR(Settings, ImmutableSettings);
    ZS_DECLARE_STRUCT_PTR(ConstraintSet);
    ZS_DECLARE_STRUCT_PTR(TrackConstraints);
    ZS_DECLARE_STRUCT_PTR(Constraints);
    ZS_DECLARE_STRUCT_PTR(OverconstrainedError);

    ZS_DECLARE_STRUCT_PTR(MediaChannelTrace);
    ZS_DECLARE_TYPEDEF_PTR(MediaChannelTrace, ImmutableMediaChannelTrace);
    ZS_DECLARE_STRUCT_PTR(Frame);
    ZS_DECLARE_STRUCT_PTR(AudioFrame);
    ZS_DECLARE_STRUCT_PTR(VideoFrame);

    ZS_DECLARE_PTR(MediaChannelIDList);
    ZS_DECLARE_PTR(MediaChannelIDSet);

    ZS_DECLARE_TYPEDEF_PTR(std::list<ConstraintSetPtr>, ConstraintSetList);
    ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, Parameters);
    ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::ImmutableParameters, ImmutableParameters);

    enum Kinds
    {
      Kind_First,

      Kind_Audio = Kind_First,
      Kind_Video,

      Kind_Last = Kind_Video,
    };

    static const char *toString(Kinds kind) noexcept;
    static Kinds toKind(const char *kind) noexcept(false);

    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackTypes::States
    //
    
    enum States
    {
      State_First,

      State_Live = State_First,
      State_Ended,

      State_Last = State_Ended,
    };

    static const char *toString(States state) noexcept;
    static States toState(const char *state) noexcept(false);

    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackTypes::MediaActivityStates
    //
    
    enum MediaActivityStates
    {
      MediaActivityState_First,

      MediaActivityState_Muted = MediaActivityState_First,
      MediaActivityState_Inactive,          // media source is not active
      MediaActivityState_ActiveButUnknown,  // media source is active but the active speaker state is unknown
      MediaActivityState_Background,        // the media is identified as being background (e.g. background noise, and no active speaker)
      MediaActivityState_Active,            // the media is identified as containing activity (e.g. active speaker / active face detected)

      MediaActivityState_Last = MediaActivityState_Active,
    };

    static const char *toString(MediaActivityStates state) noexcept;
    static MediaActivityStates toMediaActivityState(const char *state) noexcept;

    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackTypes::Capabilities
    //

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
      Optional<CapabilityBoolean> mEchoCancellation;
      Optional<CapabilityDouble>  mLatency;
      Optional<CapabilityLong>    mChannelCount;

      String                      mDeviceID;
      String                      mGroupID;

      Optional<CapabilityString>  mEncodingFormat;

      Capabilities() noexcept {}
      Capabilities(const Capabilities &op2) noexcept {(*this) = op2;}
      Capabilities(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "capabilities") const noexcept;

      static CapabilitiesPtr create() noexcept;
      static CapabilitiesPtr create(const CapabilitiesPtr &value) noexcept;
      static CapabilitiesPtr create(const Capabilities &value) noexcept;
      static CapabilitiesPtr convert(AnyPtr any) noexcept;
      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };
    
    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackTypes::Settings
    //

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
      Optional<LONG>     mChannelCount;
      Optional<String>   mDeviceID;
      Optional<String>   mGroupID;
      Optional<String>   mEncodingFormat;

      Settings() noexcept {}
      Settings(const Settings &op2) noexcept {(*this) = op2;}
      Settings(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "settings") const noexcept;

      static SettingsPtr create() noexcept;
      static SettingsPtr create(const SettingsPtr &value) noexcept;
      static SettingsPtr create(const Settings &value) noexcept;
      static SettingsPtr convert(AnyPtr any) noexcept;
      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackTypes::ConstraintSet
    //

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
      ConstrainBoolean  mEchoCancellation;
      ConstrainDouble   mLatency;
      ConstrainLong     mChannelCount;
      ConstrainString   mDeviceID;
      ConstrainString   mGroupID;
      ConstrainString   mEncodingFormat;

      ConstraintSet() noexcept {}
      ConstraintSet(const ConstraintSet &op2) noexcept {(*this) = op2;}
      ConstraintSet(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "constraintSet") const noexcept;

      static ConstraintSetPtr create() noexcept;
      static ConstraintSetPtr create(const ConstraintSetPtr &value) noexcept;
      static ConstraintSetPtr create(const ConstraintSet &value) noexcept;
      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackTypes::TrackConstraints
    //

    struct TrackConstraints : public ConstraintSet {
      ConstraintSetList mAdvanced;

      TrackConstraints() noexcept {}
      TrackConstraints(const TrackConstraints &op2) noexcept {(*this) = op2;}
      TrackConstraints(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "trackConstraints") const noexcept;

      static TrackConstraintsPtr create() noexcept;
      static TrackConstraintsPtr create(const TrackConstraintsPtr &value) noexcept;
      static TrackConstraintsPtr create(const TrackConstraints &value) noexcept;
      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackTypes::Constraints
    //

    struct Constraints {
      TrackConstraintsPtr mVideo;
      TrackConstraintsPtr mAudio;

      Constraints() noexcept {}
      Constraints(const Constraints &op2) noexcept {(*this) = op2;}
      Constraints(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "constraints") const noexcept;

      static ConstraintsPtr create() noexcept;
      static ConstraintsPtr create(const ConstraintsPtr &value) noexcept;
      static ConstraintsPtr create(const Constraints &value) noexcept;
      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackTypes::OverconstrainedError
    //

    struct OverconstrainedError : public Any {
      String mName;
      String mConstraint;
      String mMessage;

      OverconstrainedError() noexcept : mName("OverconstrainedError") {}
      OverconstrainedError(const OverconstrainedError &op2) noexcept { *this = (op2); }
    };

    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackTypes::MediaChannelTrace
    //

    struct MediaChannelTrace
    {
      MediaChannelTrace(MediaChannelID mediaChannelID) noexcept                 { mMediaChannelIDTracked.push_back(mediaChannelID); mMediaChannelIDs.insert(mediaChannelID); }
      MediaChannelTrace(
                        const ImmutableMediaChannelTrace &op2,
                        MediaChannelID mediaChannelID = 0
                        ) noexcept :
                                                                                 mMediaChannelIDTracked(op2.mMediaChannelIDTracked),
                                                                                 mMediaChannelIDs(op2.mMediaChannelIDs),
                                                                                 mKind(op2.mKind),
                                                                                 mSettings(op2.mSettings),
                                                                                 mParameters(op2.mParameters)
                                                                                 { if (0 == mediaChannelID) return; mMediaChannelIDTracked.push_back(mediaChannelID); mMediaChannelIDs.insert(mediaChannelID); }

      static MediaChannelTracePtr create(MediaChannelID mediaChannelID) noexcept { return make_shared<MediaChannelTrace>(mediaChannelID); }
      static MediaChannelTracePtr create(
                                         const MediaChannelTrace &op2,
                                         MediaChannelID mediaChannelID = 0
                                         ) noexcept                              { return make_shared<MediaChannelTrace>(op2, mediaChannelID); }

      bool hasChannel(MediaChannelID mediaChannelID) const  noexcept             { auto found = mMediaChannelIDs.find(mediaChannelID); return found != mMediaChannelIDs.end(); }

      PUID id() const noexcept                                                   { return mID; }
      Kinds kind() const noexcept                                                { return mKind; }
      ImmutableSettingsPtr settings() const noexcept                             { return mSettings; }
      ImmutableParametersPtr parameters() const noexcept                         { return mParameters; }
      MediaChannelID frontMediaChannelID() const noexcept                        { return mMediaChannelIDTracked.front(); }
      MediaChannelID endMediaChannelID() const noexcept                          { return mMediaChannelIDTracked.back(); }
      MediaChannelIDList::const_iterator begin() const noexcept                  { return mMediaChannelIDTracked.begin(); }
      MediaChannelIDList::const_iterator end() const noexcept                    { return mMediaChannelIDTracked.end(); }

    public:
      zsLib::AutoPUID mID;
      MediaChannelIDList mMediaChannelIDTracked;
      MediaChannelIDSet mMediaChannelIDs;
      Kinds mKind {Kind_First};
      ImmutableSettingsPtr mSettings;
      ImmutableParametersPtr mParameters;
    };

    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackTypes::Frame
    //

    struct Frame : public Any
    {
      virtual Kinds getKind() const noexcept = 0;

      virtual MediaActivityStates getMediaActivityState() const noexcept = 0;
      virtual uint64_t getTimestamp() const noexcept = 0;    // get the timestamp relative to the clock/sample rate
      virtual Milliseconds getNTPTime() const noexcept = 0;  // get the NTP time
      virtual Time getTime() const noexcept = 0;             // get the absolute time

      virtual const BYTE *getData() const noexcept = 0;      // get the pointer to the raw buffer
      virtual size_t getDataSize() const noexcept = 0;       // get the number of bytes contained within the raw buffer
    };

    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackTypes::AudioFrame
    //

    struct AudioFrame : public Frame
    {
      Kinds getKind() const noexcept override { return Kind_Audio; }
    };

    //-------------------------------------------------------------------------
    //
    // IMediaStreamTrackTypes::VideoFrame
    //

    struct VideoFrame : public Frame
    {
      enum DataFormats
      {
        DataFormat_NativeTexture,
        DataFormat_Packed,

        DataFormat_Y,
        DataFormat_U,
        DataFormat_V,
      };

      Kinds getKind() const noexcept override { return Kind_Video; }

      // returns true of available otherwise false
      virtual bool getEncodedData(
                                  DataFormats encodedDataFormat,
                                  const void * &outData,     // set to NULL if not available
                                  size_t &outDataSizeIfKnown // set to 0 if not available
                                  ) noexcept = 0;      // get the pointer to the raw buffer
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrack
  //
  
  interaction IMediaStreamTrack : public Any,
                                  public IMediaStreamTrackTypes,
                                  public IStatsProvider
  {
    static IMediaStreamTrackPtr convert(AnyPtr any) noexcept;

    virtual MediaChannelID getID() const noexcept = 0;

    virtual void trace(
                       const char *func = NULL,
                       const char *message = NULL
                       ) const noexcept = 0;

    virtual IMediaStreamTrackSubscriptionPtr subscribe(IMediaStreamTrackDelegatePtr delegate) noexcept = 0;

    virtual Kinds kind() const noexcept = 0;
    virtual String id() const noexcept = 0;
    virtual String deviceID() const noexcept = 0;  // created based om which original media device ID (only applicable if created from a device)
    virtual String label() const noexcept = 0;
    virtual bool enabled() const noexcept = 0;
    virtual void enabled(bool enabled) noexcept = 0;
    virtual bool muted() const noexcept = 0;
    virtual void muted(bool muted) noexcept = 0;
    virtual bool remote() const noexcept = 0;
    virtual States readyState() const noexcept = 0;

    virtual IMediaStreamTrackPtr clone() const noexcept = 0;

    virtual void stop() noexcept = 0;

    virtual CapabilitiesPtr getCapabilities() const noexcept = 0;
    virtual TrackConstraintsPtr getConstraints() const noexcept = 0;
    virtual SettingsPtr getSettings() const noexcept = 0;

    virtual PromisePtr applyConstraints(const TrackConstraints &constraints) noexcept = 0;

    virtual IMediaStreamTrackMediaSubscriptionPtr subscribeMedia(IMediaStreamTrackSyncMediaDelegatePtr delegate) noexcept = 0;

    // WARNING: extreme caution must be done not to cause a deadlock or delaying media by blocking. Gather information
    // needed within a lock but do any operations outside a lock as fast as possible.
    virtual IMediaStreamTrackMediaSubscriptionPtr subscribeMedia(IMediaStreamTrackAsyncMediaDelegatePtr delegate) noexcept = 0;
  };

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //
  // IMediaStreamTrackDelegate
  //

  interaction IMediaStreamTrackDelegate
  {
    ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::OverconstrainedError, OverconstrainedError);

    virtual void onMediaStreamTrackMute(
                                        IMediaStreamTrackPtr track,
                                        bool isMuted
                                        ) = 0;

    virtual void onMediaStreamTrackEnded(IMediaStreamTrackPtr track) = 0;
    virtual void onMediaStreamTrackOverConstrained(
                                                   IMediaStreamTrackPtr track,
                                                   OverconstrainedErrorPtr error
                                                   ) = 0;
  };
  
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrackSubscription
  //
  
  interaction IMediaStreamTrackSubscription
  {
    virtual PUID getID() const noexcept = 0;
    
    virtual void cancel() noexcept = 0;

    virtual void background() noexcept = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrackSyncMediaDelegate
  //

  interaction IMediaStreamTrackSyncMediaDelegate : public IMediaStreamTrackDelegate
  {
    ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
    ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::AudioFrame, AudioFrame);
    ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::VideoFrame, VideoFrame);

    virtual void notifyMediaStreamTrackMediaAudioFrame(
                                                       ImmutableMediaChannelTracePtr trace,
                                                       AudioFramePtr frame
                                                       ) noexcept = 0;
    virtual void notifyMediaStreamTrackMediaVideoFrame(
                                                       ImmutableMediaChannelTracePtr trace,
                                                       VideoFramePtr frame
                                                       ) noexcept = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrackAsyncMediaDelegate
  //

  interaction IMediaStreamTrackAsyncMediaDelegate : public IMediaStreamTrackDelegate
  {
    ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
    ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::AudioFrame, AudioFrame);
    ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::VideoFrame, VideoFrame);

    virtual void onMediaStreamTrackMediaAudioFrame(
                                                   ImmutableMediaChannelTracePtr trace,
                                                   AudioFramePtr frame
                                                   ) = 0;
    virtual void onMediaStreamTrackMediaVideoFrame(
                                                   ImmutableMediaChannelTracePtr trace,
                                                   VideoFramePtr frame
                                                   ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMediaStreamTrackMediaSubscription
  //
  
  interaction IMediaStreamTrackMediaSubscription
  {
    virtual PUID getID() const noexcept = 0;
    
    virtual void cancel() noexcept = 0;

    virtual void background() noexcept = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IMediaStreamTrackDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::OverconstrainedErrorPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_METHOD(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
ZS_DECLARE_PROXY_METHOD(onMediaStreamTrackEnded, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_METHOD(onMediaStreamTrackOverConstrained, IMediaStreamTrackPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IMediaStreamTrackDelegate, ortc::IMediaStreamTrackSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::OverconstrainedErrorPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamTrackEnded, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamTrackOverConstrained, IMediaStreamTrackPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()

ZS_DECLARE_PROXY_BEGIN(ortc::IMediaStreamTrackAsyncMediaDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::OverconstrainedErrorPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::ImmutableMediaChannelTracePtr, ImmutableMediaChannelTracePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::AudioFramePtr, AudioFramePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::VideoFramePtr, VideoFramePtr)
ZS_DECLARE_PROXY_METHOD(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
ZS_DECLARE_PROXY_METHOD(onMediaStreamTrackEnded, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_METHOD(onMediaStreamTrackOverConstrained, IMediaStreamTrackPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_METHOD(onMediaStreamTrackMediaAudioFrame, ImmutableMediaChannelTracePtr, AudioFramePtr)
ZS_DECLARE_PROXY_METHOD(onMediaStreamTrackMediaVideoFrame, ImmutableMediaChannelTracePtr, VideoFramePtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IMediaStreamTrackAsyncMediaDelegate, ortc::IMediaStreamTrackMediaSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::OverconstrainedErrorPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::ImmutableMediaChannelTracePtr, ImmutableMediaChannelTracePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::AudioFramePtr, AudioFramePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::VideoFramePtr, VideoFramePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamTrackEnded, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamTrackOverConstrained, IMediaStreamTrackPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamTrackMediaAudioFrame, ImmutableMediaChannelTracePtr, AudioFramePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamTrackMediaVideoFrame, ImmutableMediaChannelTracePtr, VideoFramePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()

ZS_DECLARE_PROXY_BEGIN(ortc::IMediaStreamTrackSyncMediaDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::OverconstrainedErrorPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::ImmutableMediaChannelTracePtr, ImmutableMediaChannelTracePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::AudioFramePtr, AudioFramePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::VideoFramePtr, VideoFramePtr)
ZS_DECLARE_PROXY_METHOD(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
ZS_DECLARE_PROXY_METHOD(onMediaStreamTrackEnded, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_METHOD(onMediaStreamTrackOverConstrained, IMediaStreamTrackPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_METHOD_SYNC(notifyMediaStreamTrackMediaAudioFrame, ImmutableMediaChannelTracePtr, AudioFramePtr)
ZS_DECLARE_PROXY_METHOD_SYNC(notifyMediaStreamTrackMediaVideoFrame, ImmutableMediaChannelTracePtr, VideoFramePtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::IMediaStreamTrackSyncMediaDelegate, ortc::IMediaStreamTrackMediaSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackPtr, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::OverconstrainedErrorPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::ImmutableMediaChannelTracePtr, ImmutableMediaChannelTracePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::AudioFramePtr, AudioFramePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IMediaStreamTrackTypes::VideoFramePtr, VideoFramePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamTrackMute, IMediaStreamTrackPtr, bool)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamTrackEnded, IMediaStreamTrackPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onMediaStreamTrackOverConstrained, IMediaStreamTrackPtr, OverconstrainedErrorPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_SYNC(notifyMediaStreamTrackMediaAudioFrame, ImmutableMediaChannelTracePtr, AudioFramePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_SYNC(notifyMediaStreamTrackMediaVideoFrame, ImmutableMediaChannelTracePtr, VideoFramePtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()

