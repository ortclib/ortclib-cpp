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

#include <ortc/types.h>

namespace ortc
{
  namespace internal
  {
    using zsLib::Noop;
    using zsLib::MessageQueue;
    using zsLib::IMessageQueuePtr;
    using zsLib::MessageQueuePtr;
    using zsLib::MessageQueueAssociator;
    using zsLib::IMessageQueueNotify;
    using zsLib::IMessageQueueMessagePtr;
    using zsLib::IMessageQueueThread;
    using zsLib::MessageQueueThread;
    using zsLib::IMessageQueueThreadPtr;
    using zsLib::MessageQueueThreadPtr;
    
    class RTCConnection;
    typedef boost::shared_ptr<RTCConnection> RTCConnectionPtr;
    typedef boost::weak_ptr<RTCConnection> RTCConnectionWeakPtr;
    
    class RTCSocket;
    typedef boost::shared_ptr<RTCSocket> RTCSocketPtr;
    typedef boost::weak_ptr<RTCSocket> RTCSocketWeakPtr;
    
    class RTCStream;
    typedef boost::shared_ptr<RTCStream> RTCStreamPtr;
    typedef boost::weak_ptr<RTCStream> RTCStreamWeakPtr;
    
    class RTCTrack;
    typedef boost::shared_ptr<RTCTrack> RTCTrackPtr;
    typedef boost::weak_ptr<RTCTrack> RTCTrackWeakPtr;
    
    class RTCDTMFTrack;
    typedef boost::shared_ptr<RTCDTMFTrack> RTCDTMFTrackPtr;
    typedef boost::weak_ptr<RTCDTMFTrack> RTCDTMFTrackWeakPtr;
    
    class RTCDataChannel;
    typedef boost::shared_ptr<RTCDataChannel> RTCDataChannelPtr;
    typedef boost::weak_ptr<RTCDataChannel> RTCDataChannelWeakPtr;
  }
}
