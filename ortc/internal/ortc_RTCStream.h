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
#include <ortc/IRTCStream.h>

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
    
    interaction IRTCStreamForRTCConnection
    {
      IRTCStreamForRTCConnection &forRTCConnection() {return *this;}
      const IRTCStreamForRTCConnection &forRTCConnection() const {return *this;}
      
      static RTCStreamPtr create(
                                 IMessageQueuePtr queue
                                 );
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCStream
    #pragma mark
    
    class RTCStream : public Noop,
                      public MessageQueueAssociator,
                      public IRTCStream,
                      public IRTCStreamForRTCConnection
    {
    public:
      friend interaction IRTCStream;
      friend interaction IRTCStreamForRTCConnection;
      
    protected:
      RTCStream(
                IMessageQueuePtr queue
                );
      
    public:
      virtual ~RTCStream();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCStream => IRTCStream
      #pragma mark
      
      virtual IMediaStreamTrackPtr source();
      virtual MsidListPtr msid();
      
      virtual RTCTrackListPtr tracks();
      
      virtual void start();
      virtual void stop();
      virtual void remove();

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCStream => IRTCStreamForRTCConnection
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCStream => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCStream => (data)
      #pragma mark
      
    };
  }
}
