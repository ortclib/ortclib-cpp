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

#pragma once

#include <ortc/internal/types.h>
#include <ortc/IRTCConnection.h>

#include <zsLib/MessageQueueAssociator.h>

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
    
    class RTCConnection : public Noop,
                          public MessageQueueAssociator,
                          public IRTCConnection
    {
    public:
      friend interaction IRTCConnection;
      
    protected:
      RTCConnection(
                   IMessageQueuePtr queue,
                   IRTCConnectionDelegatePtr delegate
                   );
      
    public:
      virtual ~RTCConnection();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCConnection => IRTCConnection
      #pragma mark
      
      virtual IRTCSocketPtr socket();
      virtual RTCConnectionStates state();
      virtual RTCConnectionRoles role();
      virtual RTCConnectionInfo local();
      virtual RTCConnectionInfo remote();
      
      virtual void addRemoteCandidate(RTCIceCandidateInfo candidate);
      virtual void connect();
      virtual void gather();
      virtual IRTCStreamPtr send(IMediaStreamPtr stream);
      virtual IRTCTrackPtr send(IMediaStreamTrackPtr track);
      virtual IRTCStreamPtr send(IRTCStreamPtr stream);
      virtual IRTCTrackPtr send(IRTCTrackPtr track);
      virtual IRTCStreamPtr receive(IRTCStreamPtr stream);
      virtual IRTCTrackPtr receive(IRTCTrackPtr track);
      virtual RTCStreamListPtr sendStreams();
      virtual RTCTrackListPtr sendTracks();
      virtual RTCStreamListPtr receiveStreams();
      virtual RTCTrackListPtr receiveTracks();
      virtual void close();
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCConnection => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCConnection => (data)
      #pragma mark
      
    };
  }
}
