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
