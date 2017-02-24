/*

 Copyright (c) 2015, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/internal/types.h>
#include <ortc/internal/ortc.h>
#include <ortc/internal/ortc.events.h>
#include <zsLib/Log.h>

namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_webrtc) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_dtlstransport) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_icegatherer) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_icegatherer_router) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_icetransport) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_icetransport_controller) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_mediadevices) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_mediastreamtrack) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_rtp_rtcp_packet) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_rtplistener) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_rtpmediaengine) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_rtpreceiver) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_rtpsender) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_rtptypes) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_sctp_datachannel) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_srtp) }
namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib_stats) }

ZS_EVENTING_EXCLUSIVE(OrtcLib);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_webrtc, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_dtlstransport, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_icegatherer, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_icegatherer_router, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_icetransport, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_icetransport_controller, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_mediadevices, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_mediastreamtrack, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_rtp_rtcp_packet, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_rtplistener, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_rtpmediaengine, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_rtpreceiver, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_rtpsender, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_rtptypes, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_sctp_datachannel, Debug);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_srtp, Debug);
ZS_EVENTING_EXCLUSIVE(x);

ZS_EVENTING_EXCLUSIVE(OrtcLibStatsReport);
ZS_EVENTING_SUBSYSTEM_DEFAULT_LEVEL(ortclib_stats, Debug);
ZS_EVENTING_EXCLUSIVE(x);

namespace ortc
{
  namespace internal
  {
    void initSubsystems()
    {
      ZS_GET_SUBSYSTEM_LOG_LEVEL(ZS_GET_OTHER_SUBSYSTEM(ortc, ortclib));
      ZS_GET_SUBSYSTEM_LOG_LEVEL(ZS_GET_OTHER_SUBSYSTEM(ortc, ortclib_webrtc));
    }
  }
}
