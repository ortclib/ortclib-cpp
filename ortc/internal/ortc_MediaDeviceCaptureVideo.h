/*

 Copyright (c) 2017, Optical Tone Ltd.
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

#include <ortc/internal/types.h>

#include <ortc/internal/ortc_IMediaDeviceCapture.h>
#include <ortc/internal/ortc_MediaChannelTraceHelper.h>
#include <ortc/IMediaDevices.h>

#include <zsLib/IWakeDelegate.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IMediaDeviceCaptureVideoForMediaEngine);

    ZS_DECLARE_INTERACTION_PTR(IMediaEngineForMediaDeviceCaptureVideo);

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo (helpers)
    //


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MediaDeviceCaptureVideo
    //
    
    class MediaDeviceCaptureVideo : public Any,
                                    public Noop,
                                    public MessageQueueAssociator,
                                    public SharedRecursiveLock,
                                    public IMediaDeviceCaptureVideoForMediaEngine,
                                    public zsLib::IWakeDelegate,
                                    public zsLib::IPromiseSettledDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IMediaDeviceCaptureVideoForMediaEngine;

      ZS_DECLARE_CLASS_PTR(Media);
      ZS_DECLARE_CLASS_PTR(MediaSubscriber);

      friend class Media;
      friend class MediaSubscriber;

      typedef PUID ObjectID;
      typedef std::map<ObjectID, MediaSubscriberPtr> MediaSubscriberMap;
      ZS_DECLARE_PTR(MediaSubscriberMap);

      typedef IMediaStreamTrackTypes::MediaChannelID MediaChannelID;
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::MediaChannelTrace, MediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithSettingsList, PromiseWithSettingsList);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevice::States, States);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::VideoFrame, VideoFrame);

      ZS_DECLARE_TYPEDEF_PTR(IMediaDevicesTypes::PromiseWithSettingsList, UsePromiseWithSettingsList);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Settings, UseSettings);
      ZS_DECLARE_TYPEDEF_PTR(std::list<UseSettingsPtr>, SettingsList);

      ZS_DECLARE_TYPEDEF_PTR(std::list<IMediaStreamTrackTypes::TrackConstraintsPtr>, TrackConstraintsList);

      struct PendingSubscriber
      {
        MediaDeviceCapturePromisePtr promise_;
        MediaDeviceObjectID repaceExistingDeviceObjectID_ {};
        TrackConstraintsPtr constraints_;
        IMediaDeviceCaptureDelegatePtr delegate_;
      };

      ZS_DECLARE_TYPEDEF_PTR(std::list<PendingSubscriber>, PendingSubscriberList);

    public:
      MediaDeviceCaptureVideo(
                              const make_private &,
                              IMessageQueuePtr queue,
                              UseMediaEnginePtr mediaEngine,
                              const String &deviceID
                              ) noexcept;

    protected:
      MediaDeviceCaptureVideo(Noop, IMessageQueuePtr queue = IMessageQueuePtr()) noexcept :
        Noop(true),
        MessageQueueAssociator(queue),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init() noexcept;

    public:
      virtual ~MediaDeviceCaptureVideo() noexcept;

      static MediaDeviceCaptureVideoPtr create(
                                               UseMediaEnginePtr mediaEngine,
                                               const String &deviceID
                                               ) noexcept;

      static MediaDeviceCaptureVideoPtr convert(ForMediaEnginePtr object) noexcept;


      //-----------------------------------------------------------------------
      //
      // MediaDeviceCaptureVideo => (for Media)
      //

      void notifyMediaStateChanged() noexcept;
      void notifyMediaFailure(
                              MediaPtr media,
                              WORD errorCode,
                              const char *reason
                              ) noexcept;

      //-----------------------------------------------------------------------
      //
      // MediaDeviceCaptureVideo => (for MediaSubscribers)
      //

      PUID getID() const noexcept { return id_; }
      void notifySusbcriberGone() noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // MediaDeviceCaptureVideo => IMediaDeviceForMediaEngine
      //

      bool isDeviceIdle() noexcept override;
      void shutdown() noexcept override;

      States getState() const noexcept override;

      //-----------------------------------------------------------------------
      //
      // MediaDeviceCaptureVideo => IMediaDeviceCaptureForMediaEngine
      //

      virtual void mediaDeviceCaptureSubscribe(
                                               MediaDeviceCapturePromisePtr promise,
                                               MediaDeviceObjectID repaceExistingDeviceObjectID,
                                               TrackConstraintsPtr constraints,
                                               IMediaDeviceCaptureDelegatePtr delegate
                                               ) noexcept override;

      //-----------------------------------------------------------------------
      //
      // MediaDeviceCaptureVideo => IMediaDeviceCaptureVideoForMediaEngine
      //

      //-----------------------------------------------------------------------
      //
      // MediaDeviceCaptureVideo => IWakeDelegate
      //

      void onWake() override;

      //-----------------------------------------------------------------------
      //
      // MediaDeviceCaptureVideo => IPromiseSettledDelegate
      //

      void onPromiseSettled(PromisePtr promise) override;

    protected:
      //-----------------------------------------------------------------------
      //
      // MediaDeviceCaptureVideo => (internal)
      //

      bool isPending() const noexcept { return IMediaDevice::State_Pending == currentState_; }
      bool isReady() const noexcept { return IMediaDevice::State_Active == currentState_; }
      bool isReinitializing() const noexcept { return IMediaDevice::State_Reinitializing == currentState_; }
      bool isShuttingDown() const noexcept { return IMediaDevice::State_ShuttingDown == currentState_; }
      bool isShutdown() const noexcept { return IMediaDevice::State_Shutdown == currentState_; }

      void cancel() noexcept;

      bool stepShutdownPendingRequests() noexcept;
      bool stepShutdownSubscribers() noexcept;
      bool stepShutdownMedia() noexcept;

      void step() noexcept;
      bool stepMediaReinitializationShutdown() noexcept;
      bool stepDiscoverModes() noexcept;
      bool stepFigureOutMode() noexcept;
      bool stepWaitForMediaDevice() noexcept;

      void setState(States state) noexcept;
      void setError(PromisePtr promise) noexcept;
      void setError(WORD error, const char *inReason) noexcept;

    public:
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // MediaDeviceCaptureVideo::Media
      //

      class Media : public MessageQueueAssociator,
                    public SharedRecursiveLock,
                    public zsLib::IWakeDelegate
      {
      protected:
        struct make_private {};

        enum MediaStates
        {
          MediaState_Pending,
          MediaState_Ready,
          MediaState_ShuttingDown,
          MediaState_Shutdown,
        };

        static const char *toString(MediaStates state) noexcept;

      public:
        ZS_DECLARE_TYPEDEF_PTR(MediaDeviceCaptureVideo, UseOuter);

      public:
        Media(
              const make_private &,
              IMessageQueuePtr queue,
              UseOuterPtr outer,
              const String &deviceID,
              UseSettingsPtr settings
              ) noexcept;

      protected:
        void init() noexcept;

      public:
        virtual ~Media() noexcept;

        static MediaPtr create(
                               IMessageQueuePtr queue,
                               UseOuterPtr outer,
                               const String &deviceID,
                               UseSettingsPtr settings
                               ) noexcept;

      public:
        //---------------------------------------------------------------------
        //
        // MediaDeviceCaptureVideo::Media => (for MediaDeviceCaptureVideo)
        //

        MediaChannelID getID() const noexcept { return id_; }

        void shutdown() noexcept;
        bool isReady() const noexcept;
        bool isShuttingDown() const noexcept;
        bool isShutdown() const noexcept;

        void notifySubscribersChanged(MediaSubscriberMapPtr subscribers) noexcept;

        //-----------------------------------------------------------------------
        //
        // MediaDeviceCaptureVideo::Media => IWakeDelegate
        //

        void onWake() override;

      protected:
        //---------------------------------------------------------------------
        //
        // MediaDeviceCaptureVideo::Media => (internal)
        //

        void cancel() noexcept;
        void step() noexcept;

        void setState(MediaStates state) noexcept;
        void setError(PromisePtr promise) noexcept;
        void setError(WORD errorCode, const char *inReason) noexcept;

        void internalNotifySubscribersOfFrame(VideoFramePtr frame) noexcept;

      public:
        //---------------------------------------------------------------------
        //
        // MediaDeviceCaptureVideo::Media => (data)
        //

        AutoPUID id_;
        MediaWeakPtr thisWeak_;

        MediaPtr gracefulShutdownReference_;

        UseOuterWeakPtr outer_;
        MediaSubscriberMapPtr subscribers_;
        String deviceID_;
        UseSettingsPtr settings_;
        MediaChannelTracePtr trace_;

        MediaStates currentState_ {MediaState_Pending};
        WORD lastError_ {};
        String lastErrorReason_;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // MediaDeviceCaptureVideo::MediaSubscriber
      //

      class MediaSubscriber : public MessageQueueAssociator,
                              public SharedRecursiveLock,
                              public IMediaDeviceCaptureVideo
      {
      protected:
        struct make_private {};

      public:
        ZS_DECLARE_TYPEDEF_PTR(MediaDeviceCaptureVideo, UseOuter);

      public:
        MediaSubscriber(
                        const make_private &,
                        IMessageQueuePtr queue,
                        UseOuterPtr outer,
                        TrackConstraintsPtr constraints,
                        IMediaDeviceCaptureDelegatePtr delegate
                        ) noexcept;

      protected:
        void init() noexcept;

      public:
        virtual ~MediaSubscriber() noexcept;

        static MediaSubscriberPtr create(
                                         IMessageQueuePtr queue,
                                         UseOuterPtr outer,
                                         TrackConstraintsPtr constraints,
                                         IMediaDeviceCaptureDelegatePtr delegate
                                         ) noexcept;

        //---------------------------------------------------------------------
        //
        // MediaDeviceCaptureVideo::MediaSubscriber => (for MediaDeviceCaptureVideo)
        //

        MediaDeviceObjectID getID() const noexcept override { return id_; }
        void shutdown() noexcept;
        bool isShutdown() const noexcept;
        void notifyStateChanged(States state) noexcept;
        TrackConstraintsPtr getConstraints() const  noexcept { return constraints_; }

        //---------------------------------------------------------------------
        //
        // MediaDeviceCaptureVideo::MediaSubscriber => (for Media)
        //

        void notifyFrame(
                         ImmutableMediaChannelTracePtr trace,
                         VideoFramePtr frame
                         ) noexcept;

      protected:
        //---------------------------------------------------------------------
        //
        // MediaDeviceCaptureVideo::MediaSubscriber => IMediaDevice
        //

        // (duplicate) virtual MediaDeviceObjectID getID() const override { return id_; }
        void cancel() noexcept override;

        States getState() const noexcept override;

        //---------------------------------------------------------------------
        //
        // MediaDeviceCaptureVideo::MediaSubscriber => IMediaDeviceCapture
        //

        //---------------------------------------------------------------------
        //
        // MediaDeviceCaptureVideo::MediaSubscriber => IMediaDeviceCaptureVideo
        //

        //---------------------------------------------------------------------
        //
        // MediaDeviceCaptureVideo::MediaSubscriber => (internal)
        //

        void setState(States state) noexcept;

      protected:
        //---------------------------------------------------------------------
        //
        // MediaDeviceCaptureVideo::MediaSubscriber => (data)
        //

        AutoPUID id_;
        MediaSubscriberWeakPtr thisWeak_;

        UseOuterWeakPtr outer_;
        TrackConstraintsPtr constraints_;

        IMediaDeviceCaptureDelegateWeakPtr notifyDelegate_;
        IMediaDeviceCaptureDelegatePtr delegate_;

        States lastReportedState_;

        MediaChannelTraceHelper traceHelper_;
      };

    protected:
      //-----------------------------------------------------------------------
      //
      // MediaDeviceCaptureVideo => (data)
      //

      AutoPUID id_;
      MediaDeviceCaptureVideoWeakPtr thisWeak_;

      MediaDeviceCaptureVideoPtr gracefulShutdownReference_;

      UseMediaEngineWeakPtr mediaEngine_;
      String deviceID_;

      States currentState_ {IMediaDevice::State_Pending};
      WORD lastError_ {};
      String lastErrorReason_;

      PromiseWithSettingsListPtr deviceModesPromise_;
      SettingsList deviceModes_;

      PendingSubscriberList pendingSubscribers_;
      MediaSubscriberMapPtr subscribers_;

      std::atomic<bool> recheckMode_ {};
      UseSettingsPtr requiredSettings_;
      MediaPtr media_;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaDeviceCaptureVideoFactory
    //

    interaction IMediaDeviceCaptureVideoFactory
    {
      static IMediaDeviceCaptureVideoFactory &singleton() noexcept;

      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDeviceCaptureVideo, UseMediaEngine);

      virtual MediaDeviceCaptureVideoPtr create(
                                                UseMediaEnginePtr mediaEngine,
                                                const String &deviceID
                                                ) noexcept;
    };

    class MediaDeviceCaptureVideoFactory : public IFactory<IMediaDeviceCaptureVideoFactory> {};
  }
}
