
#pragma once

#include "types.h"
#include "generated/org_ortc_Json.h"

#include <ortc/IMediaStreamTrack.h>
#include <ortc/IStatsReport.h>
#include <ortc/IICETypes.h>
#include <ortc/IRTPTypes.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IMediaDevices.h>
#include <ortc/adapter/IPeerConnection.h>

#include <zsLib/Log.h>
#include <zsLib/SafeInt.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct Helper
        {
          typedef PromiseWithHolderPtr< wrapper::org::ortc::RTCStatsReportPtr > PromiseWithStatsReport;
          ZS_DECLARE_PTR(PromiseWithStatsReport);

          ZS_DECLARE_TYPEDEF_PTR(::ortc::IMediaStreamTrackTypes, IMediaStreamTrackTypes);
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsProvider, IStatsProvider);

          typedef list< ::ortc::IMediaStreamTrackPtr > MediaStreamTrackList;
          ZS_DECLARE_PTR(MediaStreamTrackList);

          typedef list< wrapper::org::ortc::MediaStreamTrackPtr > WrapperMediaStreamTrackList;
          ZS_DECLARE_PTR(WrapperMediaStreamTrackList);

          static zsLib::IMessageQueuePtr getGuiQueue() noexcept;

          static wrapper::org::ortc::log::Level toWrapper(zsLib::Log::Level level) noexcept;
          static zsLib::Log::Level toNative(wrapper::org::ortc::log::Level level) noexcept;

          static const char *toNative(wrapper::org::ortc::log::Component component) noexcept;

          static wrapper::org::ortc::MediaStreamTrackKind toWrapper(IMediaStreamTrackTypes::Kinds kind) noexcept;
          static IMediaStreamTrackTypes::Kinds toNative(wrapper::org::ortc::MediaStreamTrackKind kind) noexcept;

          static wrapper::org::ortc::MediaStreamTrackState toWrapper(IMediaStreamTrackTypes::States state) noexcept;

          static ::ortc::IMediaDevicesTypes::DeviceKinds toNative(wrapper::org::ortc::MediaDeviceKind kind) noexcept;
          static wrapper::org::ortc::MediaDeviceKind toWrapper(::ortc::IMediaDevicesTypes::DeviceKinds kind) noexcept;

          static ::ortc::IStatsReportTypes::StatsTypes toNative(wrapper::org::ortc::RTCStatsType type) noexcept;
          static wrapper::org::ortc::RTCStatsType toWrapper(::ortc::IStatsReportTypes::StatsTypes type) noexcept;

          static wrapper::org::ortc::RTCSctpTransportState toWrapper(::ortc::ISCTPTransportTypes::States state) noexcept;
          static wrapper::org::ortc::RTCDataChannelState toWrapper(::ortc::IDataChannelTypes::States state) noexcept;

          static wrapper::org::ortc::RTCIceCandidatePairState toWrapper(::ortc::IStatsReportTypes::StatsICECandidatePairStates state) noexcept;

          static wrapper::org::ortc::adapter::RTCPeerConnectionSignalingMode toWrapper(::ortc::adapter::IPeerConnectionTypes::SignalingModes mode) noexcept;
          static ::ortc::adapter::IPeerConnectionTypes::SignalingModes toNative(wrapper::org::ortc::adapter::RTCPeerConnectionSignalingMode mode) noexcept;

          static wrapper::org::ortc::adapter::RTCRtcpMuxPolicy toWrapper(::ortc::adapter::IPeerConnectionTypes::RTCPMuxPolicies policy) noexcept;
          static ::ortc::adapter::IPeerConnectionTypes::RTCPMuxPolicies toNative(wrapper::org::ortc::adapter::RTCRtcpMuxPolicy policy) noexcept;

          static wrapper::org::ortc::adapter::RTCBundlePolicy toWrapper(::ortc::adapter::IPeerConnectionTypes::BundlePolicies policy) noexcept;
          static ::ortc::adapter::IPeerConnectionTypes::BundlePolicies toNative(wrapper::org::ortc::adapter::RTCBundlePolicy policy) noexcept;

          static wrapper::org::ortc::adapter::RTCSignalingState toWrapper(::ortc::adapter::IPeerConnectionTypes::SignalingStates state) noexcept;
          static wrapper::org::ortc::adapter::RTCIceConnectionState toPeerConnectionWrapper(::ortc::IICETransportTypes::States state) noexcept;
          static wrapper::org::ortc::adapter::RTCPeerConnectionState toWrapper(::ortc::adapter::IPeerConnectionTypes::PeerConnectionStates state) noexcept;

          static wrapper::org::ortc::RTCIceGatherFilterPolicy toWrapper(::ortc::IICEGathererTypes::FilterPolicies policy) noexcept;
          static ::ortc::IICEGathererTypes::FilterPolicies toNative(wrapper::org::ortc::RTCIceGatherFilterPolicy policy) noexcept;

          static wrapper::org::ortc::RTCIceCredentialType toWrapper(::ortc::IICEGathererTypes::CredentialTypes policy) noexcept;
          static ::ortc::IICEGathererTypes::CredentialTypes toNative(wrapper::org::ortc::RTCIceCredentialType policy) noexcept;

          static wrapper::org::ortc::RTCIceGathererState toWrapper(::ortc::IICEGathererTypes::States state) noexcept;
          static ::ortc::IICEGathererTypes::States toNative(wrapper::org::ortc::RTCIceGathererState state) noexcept;

          static wrapper::org::ortc::RTCIceComponent toWrapper(::ortc::IICETypes::Components component) noexcept;
          static wrapper::org::ortc::RTCIceProtocol toWrapper(::ortc::IICETypes::Protocols protocol) noexcept;
          static wrapper::org::ortc::RTCIceCandidateType toWrapper(::ortc::IICETypes::CandidateTypes type) noexcept;
          static wrapper::org::ortc::RTCIceTcpCandidateType toWrapper(::ortc::IICETypes::TCPCandidateTypes type) noexcept;

          static wrapper::org::ortc::RTCIceTransportState toWrapper(::ortc::IICETransportTypes::States state) noexcept;
          static ::ortc::IICETransportTypes::States toNative(wrapper::org::ortc::RTCIceTransportState state) noexcept;

          static wrapper::org::ortc::RTCIceRole toWrapper(::ortc::IICETypes::Roles role) noexcept;
          static ::ortc::IICETypes::Roles toNative(wrapper::org::ortc::RTCIceRole role) noexcept;

          static wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsSignal toWrapper(::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signals signal) noexcept;
          static ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Signals toNative(wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsSignal signal) noexcept;

          static wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsApplication toWrapper(::ortc::IRTPTypes::OpusCodecCapabilityOptions::Applications application) noexcept;
          static ::ortc::IRTPTypes::OpusCodecCapabilityOptions::Applications toNative(wrapper::org::ortc::RTCRtpOpusCodecCapabilityOptionsApplication application) noexcept;

          static wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP toWrapper(::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToPs top) noexcept;
          static ::ortc::IRTPTypes::FlexFECCodecCapabilityParameters::ToPs toNative(wrapper::org::ortc::RTCRtpFlexFecCodecCapabilityParametersToP top) noexcept;

          static wrapper::org::ortc::RTCRtpDegradationPreference toWrapper(::ortc::IRTPTypes::DegradationPreferences preference) noexcept;
          static ::ortc::IRTPTypes::DegradationPreferences toNative(wrapper::org::ortc::RTCRtpDegradationPreference preference) noexcept;

          static wrapper::org::ortc::RTCRtpPriorityType toWrapper(::ortc::IRTPTypes::PriorityTypes type) noexcept;
          static ::ortc::IRTPTypes::PriorityTypes toNative(wrapper::org::ortc::RTCRtpPriorityType type) noexcept;

          static wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType toWrapper(::ortc::adapter::ISessionDescriptionTypes::SignalingTypes type) noexcept;
          static ::ortc::adapter::ISessionDescriptionTypes::SignalingTypes toNative(wrapper::org::ortc::adapter::RTCSessionDescriptionSignalingType type) noexcept;

          static wrapper::org::ortc::RTCDtlsRole toWrapper(::ortc::IDTLSTransportTypes::Roles role) noexcept;
          static ::ortc::IDTLSTransportTypes::Roles toNative(wrapper::org::ortc::RTCDtlsRole role) noexcept;

          static wrapper::org::ortc::RTCDtlsTransportState toWrapper(::ortc::IDTLSTransportTypes::States state) noexcept;
          static ::ortc::IDTLSTransportTypes::States toNative(wrapper::org::ortc::RTCDtlsTransportState state) noexcept;

          static wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection toWrapper(::ortc::adapter::ISessionDescriptionTypes::MediaDirections direction) noexcept;
          static ::ortc::adapter::ISessionDescriptionTypes::MediaDirections toNative(wrapper::org::ortc::adapter::RTCSessionDescriptionMediaDirection direction) noexcept;

          static WrapperMediaStreamTrackListPtr toWrapper(MediaStreamTrackListPtr tracks) noexcept;
          static MediaStreamTrackListPtr toNative(WrapperMediaStreamTrackListPtr tracks) noexcept;

          static PromisePtr toWrapper(PromisePtr promise) noexcept;
          static void reject(
                             PromisePtr nativePromise,
                             PromisePtr wrapperPromise
                             ) noexcept;

          static PromiseWithStatsReportPtr getStats(IStatsProviderPtr native, wrapper::org::ortc::RTCStatsTypeSetPtr statTypes) noexcept;

          template <typename optionalType1, typename optionalType2>
          static void optionalSafeIntConvert(const optionalType1 &inputType, optionalType2 &outputType) noexcept
          {
            outputType = inputType.hasValue() ? optionalType2(SafeInt<typename optionalType2::UseType>(inputType.value())) : optionalType2();
          }

        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

