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

#include <ortc/internal/ortc_RTCStream.h>
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
    #pragma mark IRTCStreamForRTCConnection
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTCStreamPtr IRTCStreamForRTCConnection::create(IMessageQueuePtr queue)
    {
      return RTCStreamPtr();
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCStream
    #pragma mark
    
    //-----------------------------------------------------------------------
    RTCStream::RTCStream(IMessageQueuePtr queue) :
      MessageQueueAssociator(queue)
    {
    }
    
    //-----------------------------------------------------------------------
    RTCStream::~RTCStream()
    {
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCStream => IRTCStream
    #pragma mark

    //-----------------------------------------------------------------------
    IMediaStreamTrackPtr RTCStream::source()
    {
      return IMediaStreamTrackPtr();
    }
    
    //-----------------------------------------------------------------------
    RTCStream::MsidListPtr RTCStream::msid()
    {
      return MsidListPtr();
    }
    
    //-----------------------------------------------------------------------
    RTCTrackListPtr RTCStream::tracks()
    {
      return RTCTrackListPtr();
    }
    
    //-----------------------------------------------------------------------
    void RTCStream::start()
    {
      
    }
    
    //-----------------------------------------------------------------------
    void RTCStream::stop()
    {
      
    }
    
    //-----------------------------------------------------------------------
    void RTCStream::remove()
    {
      
    }

  }
}
