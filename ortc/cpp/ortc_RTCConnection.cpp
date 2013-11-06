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

#include <ortc/internal/ortc_RTCConnection.h>
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
    #pragma mark RTCConnection
    #pragma mark
    
    //-----------------------------------------------------------------------
    RTCConnection::RTCConnection(IMessageQueuePtr queue, IRTCConnectionDelegatePtr delegate) :
      MessageQueueAssociator(queue)
    {
    }
    
    //-----------------------------------------------------------------------
    RTCConnection::~RTCConnection()
    {
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCConnection => IRTCConnection
    #pragma mark
    
    //-----------------------------------------------------------------------
    IRTCSocketPtr RTCConnection::socket()
    {
      return IRTCSocketPtr();
    }
    
    //-----------------------------------------------------------------------
    RTCConnection::RTCConnectionStates RTCConnection::state()
    {
      return RTCConnectionState_New;
    }
    
    //-----------------------------------------------------------------------
    RTCConnection::RTCConnectionRoles RTCConnection::role()
    {
      return RTCConnectionRole_Controlling;
    }
    
    //-----------------------------------------------------------------------
    RTCConnection::RTCConnectionInfo RTCConnection::local()
    {
      return RTCConnectionInfo();
    }
    
    //-----------------------------------------------------------------------
    RTCConnection::RTCConnectionInfo RTCConnection::remote()
    {
      return RTCConnectionInfo();
    }
    
    //-----------------------------------------------------------------------
    void RTCConnection::addRemoteCandidate(RTCIceCandidateInfo candidate)
    {
      
    }
    
    //-----------------------------------------------------------------------
    void RTCConnection::connect()
    {
      
    }
    
    //-----------------------------------------------------------------------
    void RTCConnection::gather()
    {
      
    }
    
    //-----------------------------------------------------------------------
    IRTCStreamPtr RTCConnection::send(IMediaStreamPtr stream)
    {
      return IRTCStreamPtr();
    }
    
    //-----------------------------------------------------------------------
    IRTCTrackPtr RTCConnection::send(IMediaStreamTrackPtr track)
    {
      return IRTCTrackPtr();
    }
    
    //-----------------------------------------------------------------------
    IRTCStreamPtr RTCConnection::send(IRTCStreamPtr stream)
    {
      return IRTCStreamPtr();
    }
    
    //-----------------------------------------------------------------------
    IRTCTrackPtr RTCConnection::send(IRTCTrackPtr track)
    {
      return IRTCTrackPtr();
    }
    
    //-----------------------------------------------------------------------
    IRTCStreamPtr RTCConnection::receive(IRTCStreamPtr stream)
    {
      return IRTCStreamPtr();
    }
    
    //-----------------------------------------------------------------------
    IRTCTrackPtr RTCConnection::receive(IRTCTrackPtr track)
    {
      return IRTCTrackPtr();
    }
    
    //-----------------------------------------------------------------------
    RTCStreamListPtr RTCConnection::sendStreams()
    {
      return RTCStreamListPtr();
    }
    
    //-----------------------------------------------------------------------
    RTCTrackListPtr RTCConnection::sendTracks()
    {
      return RTCTrackListPtr();
    }
    
    //-----------------------------------------------------------------------
    RTCStreamListPtr RTCConnection::receiveStreams()
    {
      return RTCStreamListPtr();
    }
    
    //-----------------------------------------------------------------------
    RTCTrackListPtr RTCConnection::receiveTracks()
    {
      return RTCTrackListPtr();
    }
    
    //-----------------------------------------------------------------------
    void RTCConnection::close()
    {
      
    }
  }
}
