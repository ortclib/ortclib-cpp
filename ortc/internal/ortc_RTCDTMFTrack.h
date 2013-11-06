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
      
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCDTMFTrack => IRTCDTMFTrack
      #pragma mark
      
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
      
    };
  }
}
