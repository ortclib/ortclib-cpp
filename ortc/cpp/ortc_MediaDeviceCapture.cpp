/*

 Copyright (c) 2017, Hookflash Inc. / Optical Tone Ltd.
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

#include <ortc/internal/ortc_IMediaDeviceCapture.h>

#include <ortc/internal/ortc_MediaEngine.h>

//#include <ortc/internal/ortc_MediaDeviceCaptureAudio.h>
//#include <ortc/internal/ortc_MediaDeviceCaptureVideo.h>
//#include <ortc/internal/ortc_MediaDeviceRenderAudio.h>
////#include <ortc/internal/ortc_RTPReceiverChannelAudio.h>
////#include <ortc/internal/ortc_RTPReceiverChannelVideo.h>
////#include <ortc/internal/ortc_RTPSenderChannelAudio.h>
////#include <ortc/internal/ortc_RTPSenderChannelVideo.h>
//#include <ortc/internal/ortc_RTPPacket.h>
//#include <ortc/internal/ortc_RTCPPacket.h>
//#include <ortc/internal/ortc_ORTC.h>
//#include <ortc/internal/ortc_StatsReport.h>
//#include <ortc/internal/ortc.events.h>
//#include <ortc/internal/platform.h>
//
//#include <ortc/IStatsReport.h>
//
//#include <ortc/IHelper.h>
//#include <ortc/services/IHTTP.h>
//
//#include <zsLib/ISettings.h>
//#include <zsLib/Singleton.h>
//#include <zsLib/Log.h>
//#include <zsLib/XML.h>
//#include <zsLib/SafeInt.h>
//
//#include <cryptopp/sha.h>
//
//#include <ortc/internal/webrtc_pre_include.h>
//#include <webrtc/base/event_tracer.h>
//#include <ortc/internal/webrtc_post_include.h>


#ifdef __GNUC__
#error MOVE THIS TO PROJECT SETTING RATHER THAN PUTTING ON INDIVIDUAL FILES
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#endif //__GNUC__

namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_media_engine) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // (helpers)
    //


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IMediaDeviceCapture
    //

    //-------------------------------------------------------------------------
    IMediaDeviceCapture::MediaDeviceCapturePromisePtr IMediaDeviceCapture::create(
                                                                                  MediaDeviceObjectID repaceExistingDeviceObjectID,
                                                                                  Kinds kind,
                                                                                  const TrackConstraints &constraints,
                                                                                  IMediaDeviceCaptureDelegatePtr delegate
                                                                                  ) noexcept
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaEngineForMediaDeviceCapture, UseEngine);
      return UseEngine::createMediaDeviceCapture(repaceExistingDeviceObjectID, kind, constraints, delegate);
    }

  } // internal namespace
}


#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif //__GNUC__
