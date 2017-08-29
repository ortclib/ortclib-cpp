/*

 Copyright (c) 2017, Hookflash Inc.
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

#ifdef ORTC_USE_ALTERNATIVE_MEDIASTREAMTRACK_MONITOR
#include <ortc/cpp/ortc_MediaStreamTrackMonitor_Alternative.cpp>
#else // ORTC_USE_ALTERNATIVE_MEDIASTREAMTRACK_MONITOR

#include <ortc/internal/ortc_MediaStreamTrackMonitor.h>
#include <ortc/internal/platform.h>

#include <zsLib/Log.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_MediaStreamTrackMonitor) }

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    // foreward declaration
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrackMonitorSettingsDefaults
    #pragma mark

    //-------------------------------------------------------------------------
    void installMediaStreamTrackMonitorSettingsDefaults()
    {
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackMonitor
    #pragma mark

    IMediaStreamTrackMonitorPtr IMediaStreamTrackMonitor::create(ortc::IMediaStreamTrackPtr track)
    {
      return IMediaStreamTrackMonitorPtr();
    }
  } // internal namespace
}

#endif // ORTC_USE_ALTERNATIVE_MEDIASTREAMTRACK_MONITOR
