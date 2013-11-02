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

#include <openpeer/services/types.h>

namespace ortc
{
  using zsLib::PUID;
  using zsLib::BYTE;
  using zsLib::WORD;
  using zsLib::DWORD;
  using zsLib::QWORD;
  using zsLib::ULONG;
  using zsLib::Time;
  using zsLib::Duration;
  using zsLib::String;

  using openpeer::services::SecureByteBlock;
  using openpeer::services::SecureByteBlockPtr;
  
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark (other)
  #pragma mark
  
  interaction IRTCConnection;
  typedef boost::shared_ptr<IRTCConnection> IRTCConnectionPtr;
  typedef boost::weak_ptr<IRTCConnection> IRTCConnectionWeakPtr;
  
  interaction IRTCConnectionDelegate;
  typedef boost::shared_ptr<IRTCConnectionDelegate> IRTCConnectionDelegatePtr;
  typedef boost::weak_ptr<IRTCConnectionDelegate> IRTCConnectionDelegateWeakPtr;
  typedef zsLib::Proxy<IRTCConnectionDelegate> IRTCConnectionDelegateProxy;
  
  interaction IRTCSocket;
  typedef boost::shared_ptr<IRTCSocket> IRTCSocketPtr;
  typedef boost::weak_ptr<IRTCSocket> IRTCSocketWeakPtr;
  
  interaction IRTCStream;
  typedef boost::shared_ptr<IRTCStream> IRTCStreamPtr;
  typedef boost::weak_ptr<IRTCStream> IRTCStreamWeakPtr;
  
  typedef std::list<IRTCStream> RTCStreamList;
  typedef boost::shared_ptr<RTCStreamList> RTCStreamListPtr;
  typedef boost::weak_ptr<RTCStreamList> RTCStreamListWeakPtr;

  interaction IRTCTrack;
  typedef boost::shared_ptr<IRTCTrack> IRTCTrackPtr;
  typedef boost::weak_ptr<IRTCTrack> IRTCTrackWeakPtr;
  
  typedef std::list<IRTCTrack> RTCTrackList;
  typedef boost::shared_ptr<RTCTrackList> RTCTrackListPtr;
  typedef boost::weak_ptr<RTCTrackList> RTCTrackListWeakPtr;

  interaction IRTCDTMFTrack;
  typedef boost::shared_ptr<IRTCDTMFTrack> IRTCDTMFTrackPtr;
  typedef boost::weak_ptr<IRTCDTMFTrack> IRTCDTMFTrackWeakPtr;
  
  interaction IRTCDTMFTrackDelegate;
  typedef boost::shared_ptr<IRTCDTMFTrackDelegate> IRTCDTMFTrackDelegatePtr;
  typedef boost::weak_ptr<IRTCDTMFTrackDelegate> IRTCDTMFTrackDelegateWeakPtr;
  typedef zsLib::Proxy<IRTCDTMFTrackDelegate> IRTCDTMFTrackDelegateProxy;

  interaction IRTCDataChannel;
  typedef boost::shared_ptr<IRTCDataChannel> IRTCDataChannelPtr;
  typedef boost::weak_ptr<IRTCDataChannel> IRTCDataChannelWeakPtr;
  
  interaction IRTCDataChannelDelegate;
  typedef boost::shared_ptr<IRTCDataChannelDelegate> IRTCDataChannelDelegatePtr;
  typedef boost::weak_ptr<IRTCDataChannelDelegate> IRTCDataChannelDelegateWeakPtr;
  typedef zsLib::Proxy<IRTCDataChannelDelegate> IRTCDataChannelDelegateProxy;

  interaction IMediaStream;
  typedef boost::shared_ptr<IMediaStream> IMediaStreamPtr;
  typedef boost::weak_ptr<IMediaStream> IMediaStreamWeakPtr;

  interaction IMediaStreamTrack;
  typedef boost::shared_ptr<IMediaStreamTrack> IMediaStreamTrackPtr;
  typedef boost::weak_ptr<IMediaStreamTrack> IMediaStreamTrackWeakPtr;
  
}
