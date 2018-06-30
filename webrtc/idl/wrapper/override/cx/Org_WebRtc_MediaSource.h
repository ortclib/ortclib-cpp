
#pragma once

#include <wrapper/generated/cx/types.h>

#include <wrapper/generated/org_ortc_MediaSource.h>

namespace Org {
  namespace Ortc {

    public ref class MediaSource sealed
    {
    private:
      zsLib::Lock lock_;

      struct Data : public zsLib::Any
      {
        Windows::Media::Core::IMediaSource^ source_;
      };

    internal:
      wrapper::org::ortc::MediaSourcePtr native_;

      struct WrapperCreate {};
      MediaSource(const WrapperCreate &) {}

      static MediaSource^ ToCx(wrapper::org::ortc::MediaSourcePtr value);
      static wrapper::org::ortc::MediaSourcePtr FromCx(MediaSource^ value);

    public:
      property Windows::Media::Core::IMediaSource^ Source { Windows::Media::Core::IMediaSource^ get(); }
    };

  } // namespace Ortc
} // namespace Org
