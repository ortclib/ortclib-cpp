// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "RTMediaStreamSource.h"
#include "Org_Ortc_EventQueue.h"

namespace Org
{
  namespace Ortc
  {

    void FrameCounterHelper::FireEvent(
      Platform::String^ id,
      Platform::String^ str
    )
    {
      auto dispatcher = EventQueue::Singleton->Queue;

      if (dispatcher != nullptr)
      {
        dispatcher->RunAsync(
          Windows::UI::Core::CoreDispatcherPriority::Normal,
          ref new Windows::UI::Core::DispatchedHandler([id, str] { FramesPerSecondChanged(id, str); })
        );
      }
      else 
      {
        FramesPerSecondChanged(id, str);
      }
    }

    void ResolutionHelper::FireEvent(
      Platform::String^ id,
      unsigned int width,
      unsigned int heigth
    )
    {
      auto dispatcher = EventQueue::Singleton->Queue;
      if (dispatcher != nullptr)
      {
        dispatcher->RunAsync(
          Windows::UI::Core::CoreDispatcherPriority::Normal,
          ref new Windows::UI::Core::DispatchedHandler([id, width, heigth] { ResolutionChanged(id, width, heigth); })
        );
      }
      else
      {
        ResolutionChanged(id, width, heigth);
      }
    }

  } // namespace ortc
}  // namespace org

