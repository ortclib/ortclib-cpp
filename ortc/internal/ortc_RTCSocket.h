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
#include <ortc/IRTCSocket.h>

namespace ortc
{
  namespace internal
  {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTCCSocketForRTCConnection
    #pragma mark
    
    interaction IRTCSocketForRTCConnection
    {
      IRTCSocketForRTCConnection &forRTCConnection() {return *this;}
      const IRTCSocketForRTCConnection &forRTCConnection() const {return *this;}
      
      static RTCSocketPtr create(
                                 IMessageQueuePtr queue
                                 );
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCSocket
    #pragma mark
    
    class RTCSocket : public Noop,
                      public MessageQueueAssociator,
                      public IRTCSocket,
                      public IRTCSocketForRTCConnection
    {
    public:
      friend interaction IRTCSocket;
      friend interaction IRTCSocketForRTCConnection;
      
    protected:
      RTCSocket(
                IMessageQueuePtr queue
                );
      
    public:
      virtual ~RTCSocket();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCSocket => IRTCSocket
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCSocket => IRTCSocketForRTCConnection
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCSocket => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCSocket => (data)
      #pragma mark
      
    };
  }
}
