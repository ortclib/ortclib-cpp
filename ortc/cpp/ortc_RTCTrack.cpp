/*

 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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

#if 0

#include <ortc/internal/ortc_RTCTrack.h>
#include <zsLib/Log.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  namespace internal
  {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTCTrackForRTCConnection
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTCTrackPtr IRTCTrackForRTCConnection::create(IMessageQueuePtr queue)
    {
      RTCTrackPtr pThis(new RTCTrack(queue));
      pThis->mThisWeak = pThis;
      return pThis;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCTrack
    #pragma mark
    
    //-----------------------------------------------------------------------
    RTCTrack::RTCTrack(IMessageQueuePtr queue) :
      MessageQueueAssociator(queue),
      mID(zsLib::createPUID()),
      mError(0),
      mKind(RTCTrackKind_Audio),
      mSsrc(0)
    {
    }
    
    //-----------------------------------------------------------------------
    RTCTrack::~RTCTrack()
    {
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCTrack => IRTCTrack
    #pragma mark
    
    //-----------------------------------------------------------------------
    IMediaStreamTrackPtr RTCTrack::source()
    {
      return IMediaStreamTrackPtr();
    }
    
    //-----------------------------------------------------------------------
    String RTCTrack::id()
    {
      return String();
    }
    
    //-----------------------------------------------------------------------
    RTCTrack::RTCTrackKinds RTCTrack::kind()
    {
      return RTCTrackKind_Audio;
    }
    
    //-----------------------------------------------------------------------
    ULONG RTCTrack::ssrc()
    {
      return 0;
    }
    
    //-----------------------------------------------------------------------
    RTCTrack::RTCCodecListPtr RTCTrack::codecs()
    {
      return RTCCodecListPtr();
    }
    
    //-----------------------------------------------------------------------
    RTCTrack::RTCMediaAttributesListPtr RTCTrack::mediaAttributes()
    {
      return RTCMediaAttributesListPtr();
    }
    
    //-----------------------------------------------------------------------
    RTCTrack::RtpExtHeadersMapPtr RTCTrack::rtpExtHeaders()
    {
      return RtpExtHeadersMapPtr();
    }
    
    //-----------------------------------------------------------------------
    void RTCTrack::start()
    {
      
    }
    
    //-----------------------------------------------------------------------
    void RTCTrack::stop()
    {
      
    }
    
    //-----------------------------------------------------------------------
    void RTCTrack::remove()
    {
      
    }
  }
}

#endif //0
