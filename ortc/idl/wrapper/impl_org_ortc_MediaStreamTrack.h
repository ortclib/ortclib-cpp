
#pragma once

#include "types.h"
#include "generated/org_ortc_MediaStreamTrack.h"

#include <ortc/IMediaStreamTrack.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct MediaStreamTrack : public wrapper::org::ortc::MediaStreamTrack,
                                  public ::ortc::IMediaStreamTrackDelegate
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrack, NativeType);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrack, IMediaStreamTrack);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrackSubscription, NativeTypeSubscription);
          ZS_DECLARE_TYPEDEF_PTR(MediaStreamTrack, WrapperImplType);
          ZS_DECLARE_TYPEDEF_PTR(wrapper::org::ortc::MediaStreamTrack, WrapperType);

          MediaStreamTrackWeakPtr thisWeak_;
          NativeTypePtr native_;
          zsLib::Lock lock_;
          std::atomic<size_t> subscriptionCount_{};
          bool defaultSubscription_ {true};
          NativeTypeSubscriptionPtr subscription_;
          MediaSourcePtr mediaSource_;

          MediaStreamTrack();
          virtual ~MediaStreamTrack();

          // methods MediaStreamTrack
          virtual wrapper::org::ortc::MediaStreamTrackPtr clone() override;
          virtual void stop() override;
          virtual wrapper::org::ortc::MediaTrackCapabilitiesPtr getCapabilities() override;
          virtual wrapper::org::ortc::MediaTrackConstraintsPtr getConstraints() override;
          virtual wrapper::org::ortc::MediaTrackSettingsPtr getSettings() override;
          virtual PromisePtr applyConstraints(wrapper::org::ortc::MediaTrackConstraintsPtr constraints) override;

          // properties MediaStreamTrack
          virtual uint64_t get_objectId() override;
          virtual wrapper::org::ortc::MediaStreamTrackKind get_kind() override;
          virtual String get_id() override;
          virtual String get_deviceId() override;
          virtual String get_label() override;
          virtual bool get_enabled() override;
          virtual void set_enabled(bool value) override;
          virtual bool get_muted() override;
          virtual void set_muted(bool value) override;
          virtual bool get_remote() override;
          virtual wrapper::org::ortc::MediaStreamTrackState get_readyState() override;
          virtual wrapper::org::ortc::MediaSourcePtr get_source() override;

          virtual void wrapper_onObserverCountChanged(size_t count) override;

          // IMediaStreamTrackDelegate
          virtual void onMediaStreamTrackMute(
                                              IMediaStreamTrackPtr track,
                                              bool isMuted
                                              ) override;

          virtual void onMediaStreamTrackEnded(IMediaStreamTrackPtr track) override;
          virtual void onMediaStreamTrackOverConstrained(
                                                         IMediaStreamTrackPtr track,
                                                         OverconstrainedErrorPtr error
                                                         ) override;

          static WrapperImplTypePtr toWrapper(NativeTypePtr track);
          static NativeTypePtr toNative(WrapperTypePtr wrapper);

          void subscribe();
        };


      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

