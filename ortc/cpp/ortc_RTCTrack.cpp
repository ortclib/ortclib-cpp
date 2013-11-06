/*
 * Copyright (C) 2013 SMB Phone Inc. / Hookflash Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <ortc/internal/ortc_RTCTrack.h>
#include <zsLib/Log.h>

namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib) }

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
      return RTCTrackPtr();
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
      MessageQueueAssociator(queue)
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
