
#pragma once


#ifndef CPPWINRT_USE_GENERATED_ORG_ORTC_MEDIASOURCE

#include "types.h"

#include "Org/Ortc/MediaSource.g.h"
#include <wrapper/generated/org_ortc_MediaSource.h>

namespace winrt {
  namespace Org {
    namespace Ortc {
      namespace implementation {


        /// <summary>
        /// MediaSource represents an object holder for a platform specific
        /// media source.
        /// </summary>
        struct MediaSource : MediaSourceT<MediaSource>
        {
          // internal
          wrapper::org::ortc::MediaSourcePtr native_;

          struct WrapperCreate {};
          MediaSource(const WrapperCreate &) {}

          // ToCppWinrtImpl
          static winrt::com_ptr< Org::Ortc::implementation::MediaSource > ToCppWinrtImpl(wrapper::org::ortc::MediaSourcePtr value);
          static winrt::com_ptr< Org::Ortc::implementation::MediaSource > ToCppWinrtImpl(Org::Ortc::MediaSource const & value);
          static winrt::com_ptr< Org::Ortc::implementation::MediaSource > ToCppWinrtImpl(winrt::com_ptr< Org::Ortc::implementation::MediaSource > const & value);
          static winrt::com_ptr< Org::Ortc::implementation::MediaSource > ToCppWinrtImpl(Org::Ortc::IMediaSource const & value);

          // ToCppWinrt
          static Org::Ortc::MediaSource ToCppWinrt(wrapper::org::ortc::MediaSourcePtr value);
          static Org::Ortc::MediaSource ToCppWinrt(Org::Ortc::MediaSource const & value);
          static Org::Ortc::MediaSource ToCppWinrt(winrt::com_ptr< Org::Ortc::implementation::MediaSource > const & value);
          static Org::Ortc::MediaSource ToCppWinrt(Org::Ortc::IMediaSource const & value);

          // ToCppWinrtInterface
          static Org::Ortc::IMediaSource ToCppWinrtInterface(wrapper::org::ortc::MediaSourcePtr value);
          static Org::Ortc::IMediaSource ToCppWinrtInterface(Org::Ortc::MediaSource const & value);
          static Org::Ortc::IMediaSource ToCppWinrtInterface(winrt::com_ptr< Org::Ortc::implementation::MediaSource > const & value);
          static Org::Ortc::IMediaSource ToCppWinrtInterface(Org::Ortc::IMediaSource const & value);

          // FromCppWinrt
          static wrapper::org::ortc::MediaSourcePtr FromCppWinrt(wrapper::org::ortc::MediaSourcePtr value);
          static wrapper::org::ortc::MediaSourcePtr FromCppWinrt(winrt::com_ptr< Org::Ortc::implementation::MediaSource > const & value);
          static wrapper::org::ortc::MediaSourcePtr FromCppWinrt(Org::Ortc::MediaSource const & value);
          static wrapper::org::ortc::MediaSourcePtr FromCppWinrt(Org::Ortc::IMediaSource const & value);




        public:
          MediaSource();
          /// <summary>
          /// Cast from Org::Ortc::IMediaSource to MediaSource
          /// </summary>
          static Org::Ortc::MediaSource CastFromIMediaSource(Org::Ortc::IMediaSource const & value);

          /// <summary>
          /// Gets or sets the platform specific media source.
          /// </summary>
          Windows::Foundation::IInspectable Source();
          void Source(Windows::Foundation::IInspectable const & value);
          /// <summary>
          /// Gets or sets the media track associated to the media source.
          /// </summary>
          Windows::Foundation::IInspectable Track();

        };

      } // namepsace implementation

      namespace factory_implementation {

        struct MediaSource : MediaSourceT<MediaSource, implementation::MediaSource>
        {
        };

      } // namespace factory_implementation

    } // namespace Ortc
  } // namespace Org
} // namespace winrt
#endif //ifndef CPPWINRT_USE_GENERATED_ORG_ORTC_MEDIASOURCE
