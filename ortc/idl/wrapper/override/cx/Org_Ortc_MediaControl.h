
#pragma once

#include <wrapper/generated/cx/cx_Helpers.h>
#include <wrapper/generated/org_ortc_MediaControl.h>

namespace Org {
  namespace Ortc {

    /// <summary>
    /// Interface for controlling the behaviour of media.
    /// </summary>
    public ref class MediaControl sealed
    {
    internal:
      wrapper::org::ortc::MediaControlPtr native_;

      struct WrapperCreate {};
      MediaControl(const WrapperCreate &) {}

      static MediaControl^ ToCx(wrapper::org::ortc::MediaControlPtr value);
      static wrapper::org::ortc::MediaControlPtr FromCx(MediaControl^ value);

    public:
      /// <summary>
      /// Notify that media engine the application orientation has changed.
      /// </summary>
      static property Windows::Graphics::Display::DisplayOrientations DisplayOrientation
      {
        Windows::Graphics::Display::DisplayOrientations get();
        void set(Windows::Graphics::Display::DisplayOrientations value);
      }
    };

  } // namespace Ortc
} // namespace Org
