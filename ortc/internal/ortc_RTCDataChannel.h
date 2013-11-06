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
#include <ortc/IRTCDataChannel.h>

namespace ortc
{
  namespace internal
  {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTCDataChannelForRTCConnection
    #pragma mark
    
    interaction IRTCDataChannelForRTCConnection
    {
      IRTCDataChannelForRTCConnection &forRTCConnection() {return *this;}
      const IRTCDataChannelForRTCConnection &forRTCConnection() const {return *this;}
      
      static RTCDataChannelPtr create(
                                      IMessageQueuePtr queue,
                                      IRTCDataChannelDelegatePtr delegate
                                      );
      
    };
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCDataChannel
    #pragma mark
    
    class RTCDataChannel : public Noop,
                           public MessageQueueAssociator,
                           public IRTCDataChannel,
                           public IRTCDataChannelForRTCConnection
    {
    public:
      friend interaction IRTCDataChannel;
      friend interaction IRTCDataChannelForRTCConnection;
      
    protected:
      RTCDataChannel(
                     IMessageQueuePtr queue,
                     IRTCDataChannelDelegatePtr delegate
                     );
      
    public:
      virtual ~RTCDataChannel();
      
    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCDataChannel => IRTCDataChannel
      #pragma mark
      
      virtual void send(BYTE* data);
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCDataChannel => IRTCDataChannelForRTCConnection
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCDataChannel => (internal)
      #pragma mark
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark RTCDataChannel => (data)
      #pragma mark
      
      
      
    };
  }
}
