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
#include <ortc/IRTCTrack.h>

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
    
    interaction IRTCTrackForRTCConnection
    {
      IRTCTrackForRTCConnection &forRTCConnection() {return *this;}
      const IRTCTrackForRTCConnection &forRTCConnection() const {return *this;}
      
      static RTCTrackPtr create(
                                IMessageQueuePtr queue
                                );
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTCTrackForRTCStream
    #pragma mark
    
    interaction IRTCTrackForRTCStream
    {
      IRTCTrackForRTCStream &forRTCStream() {return *this;}
      const IRTCTrackForRTCStream &forRTCStream() const {return *this;}
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCTrack
    #pragma mark
    
    class RTCTrack : public Noop,
                     public MessageQueueAssociator,
                     public IRTCTrack,
                     public IRTCTrackForRTCConnection,
                     public IRTCTrackForRTCStream
    {
    public:
      friend interaction IRTCTrack;
      friend interaction IRTCTrackForRTCConnection;
      friend interaction IRTCTrackForRTCConnection;
      
    protected:
      RTCTrack(
               IMessageQueuePtr queue
               );
      
    public:
      virtual ~RTCTrack();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCTrack => IRTCTrack
      #pragma mark
      
      virtual IMediaStreamTrackPtr source();
      virtual String id();
      virtual RTCTrackKinds kind();
      virtual ULONG ssrc();
      virtual RTCCodecListPtr codecs();
      virtual RTCMediaAttributesListPtr mediaAttributes();
      virtual RtpExtHeadersMapPtr rtpExtHeaders();
      
      virtual void start();
      virtual void stop();
      virtual void remove();

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCTrack => IRTCTrackForRTCConnection
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCTrack => IRTCTrackForRTCStream
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCTrack => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCTrack => (data)
      #pragma mark
      
    protected:
      IMediaStreamTrackPtr mSource;
      String mID;
      RTCTrackKinds mKind;
      ULONG mSsrc;
      RTCCodecListPtr mCodecs;
      RTCMediaAttributesListPtr mMediaAttributes;
      RtpExtHeadersMapPtr mRtpExtHeaders;
    };
  }
}
