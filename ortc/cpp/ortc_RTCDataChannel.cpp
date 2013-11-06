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

#include <ortc/internal/ortc_RTCDataChannel.h>
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
    #pragma mark IRTCDataChannelForRTCConnection
    #pragma mark
    
    //-------------------------------------------------------------------------
    RTCDataChannelPtr IRTCDataChannelForRTCConnection::create(IMessageQueuePtr queue, IRTCDataChannelDelegatePtr delegate)
    {
      return RTCDataChannelPtr();
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCDataChannel
    #pragma mark
    
    //-----------------------------------------------------------------------
    RTCDataChannel::RTCDataChannel(IMessageQueuePtr queue, IRTCDataChannelDelegatePtr delegate) :
      MessageQueueAssociator(queue)
    {
    }
    
    //-----------------------------------------------------------------------
    RTCDataChannel::~RTCDataChannel()
    {
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCDataChannel => IRTCDataChannel
    #pragma mark
    
    //-----------------------------------------------------------------------
    void RTCDataChannel::send(BYTE* data)
    {
      
    }
  }
}
