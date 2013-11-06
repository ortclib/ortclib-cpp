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
      
      
    };
  }
}
