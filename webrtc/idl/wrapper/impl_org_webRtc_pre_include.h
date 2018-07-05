
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100)
#pragma warning(disable: 4458)
#pragma warning(disable: 4244)
#pragma warning(disable: 4242)
#endif //_MSC_VER

namespace cricket
{
  enum CaptureState;
  enum MediaType;
}

// forward for webrtc can go here
namespace rtc
{
  class Thread;
  class RTCCertificate;
  class IntervalRange;
  class KeyParams;
  struct SSLFingerprint;

  struct RSAParams;

  enum ECCurve;
  enum KeyType;
}

namespace webrtc
{
  enum class SdpSemantics;
  enum class SdpType;
  enum class RtpTransceiverDirection;

  struct DataBuffer;
  struct DataChannelInit;
  struct RtpTransceiverInit;
  struct RtpParameters;
  struct RtpEncodingParameters;
  struct RtpRtxParameters;
  struct RtpFecParameters;
  struct RtcpFeedback;
  struct RtpCodecParameters;
  struct RtpCodecCapability;
  struct RtpHeaderExtensionCapability;
  struct RtpExtension;
  struct RtpCapabilities;
  struct RtpParameters;

  class RtpSource;
  class AudioTrackInterface;
  class VideoTrackInterface;
  class DtmfSenderInterface;
  class RtpSenderInterface;
  class RtpReceiverInterface;
  class RtpTransceiverInterface;
  class SessionDescriptionInterface;
  class PeerConnectionInterface;
  class PeerConnectionFactoryInterface;
  class PeerConnectionFactory;

  class MediaSourceInterface;
  class AudioSourceInterface;
  class VideoTrackSourceInterface;

  class RTCError;

  class VideoFrame;
}
