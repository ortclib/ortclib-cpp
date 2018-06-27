
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4100)
#pragma warning(disable: 4458)
#pragma warning(disable: 4244)
#pragma warning(disable: 4242)
#endif //_MSC_VER

// forward for webrtc can go here
namespace rtc
{
  class Thread;
}

namespace webrtc
{
  class PeerConnectionInterface;
  class PeerConnectionFactoryInterface;

  class RTCError;

  class VideoFrame;
}
