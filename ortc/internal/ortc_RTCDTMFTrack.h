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
#include <ortc/IRTCDTMFTrack.h>
#include <ortc/internal/ortc_RTCTrack.h>

namespace ortc
{
  namespace internal
  {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTCDTMFTrackForRTCConnection
    #pragma mark
    
    interaction IRTCDTMFTrackForRTCConnection
    {
      IRTCDTMFTrackForRTCConnection &forRTCConnection() {return *this;}
      const IRTCDTMFTrackForRTCConnection &forRTCConnection() const {return *this;}
      
      static RTCDTMFTrackPtr create(
                                    IMessageQueuePtr queue,
                                    IRTCDTMFTrackDelegatePtr delegate
                                    );
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCDTMFTrack
    #pragma mark
    
    class RTCDTMFTrack : public RTCTrack,
                         public IRTCDTMFTrack,
                         public IRTCDTMFTrackForRTCConnection
    {
    public:
      friend interaction IRTCDTMFTrack;
      friend interaction IRTCDTMFTrackForRTCConnection;
      
    protected:
      RTCDTMFTrack(
                   IMessageQueuePtr queue,
                   IRTCDTMFTrackDelegatePtr delegate
                   );
      
    public:
      virtual ~RTCDTMFTrack();
      
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCDTMFTrack => IRTCDTMFTrack
      #pragma mark
      
    protected:
      virtual void playTones(String tones, ULONG duration = 100, ULONG interToneGap = 70);
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCDTMFTrack => IRTCDTMFTrackForRTCConnection
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCDTMFTrack => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCDTMFTrack => (data)
      #pragma mark
      
    protected:
      IRTCDTMFTrackDelegatePtr mDelegate;
    };
  }
}
