/*

 Copyright (c) 2013, SMB Phone Inc. / Hookflash Inc.
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
