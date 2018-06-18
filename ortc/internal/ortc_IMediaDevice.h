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

#include <ortc/IMediaStreamTrack.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IMediaDeviceForMediaEngine);

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // (helpers)
    //

    IMediaStreamTrackTypes::SettingsPtr obtainBestMode(
                                                       const std::list<IMediaStreamTrackTypes::TrackConstraintsPtr> &allConstraints,
                                                       const std::list<IMediaStreamTrackTypes::SettingsPtr> &availableModes
                                                       ) noexcept;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaDevice
    //

    interaction IMediaDevice
    {
      typedef PUID MediaDeviceObjectID;

      enum States
      {
        State_First,

        State_Pending = State_First,
        State_Active,
        State_Reinitializing,
        State_ShuttingDown,
        State_Shutdown,

        State_Last = State_Shutdown,
      };

      static const char *toString(States state) noexcept;

      virtual MediaDeviceObjectID getID() const noexcept = 0;
      virtual void cancel() noexcept = 0;

      virtual States getState() const noexcept = 0;
    };

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //
    // IMediaDeviceForMediaEngine
    //

    interaction IMediaDeviceForMediaEngine
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaDeviceForMediaEngine, ForMediaEngine);
      ZS_DECLARE_TYPEDEF_PTR(IMediaDevice::States, States);

      virtual bool isDeviceIdle() noexcept = 0;
      virtual void shutdown() noexcept = 0;

      virtual States getState() const noexcept = 0;
      bool isShuttingDown() const noexcept  { return IMediaDevice::State_ShuttingDown == getState(); }
      bool isShutdown() const noexcept      { return IMediaDevice::State_Shutdown == getState(); }
    };

  }
}
