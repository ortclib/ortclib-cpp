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
  #pragma mark RTCConnectionSide
  #pragma mark
  
  struct RTCConnectionSide
  {
    
  };
  
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCIceCandidateDescription
  #pragma mark
  
  struct RTCIceCandidateDescription
  {
    
  };
  
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCTrackFilter
  #pragma mark
  
  struct RTCTrackFilter
  {
    
  };
  
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCTrackDescription
  #pragma mark
  
  struct RTCTrackDescription
  {
    
  };
  
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCCodec
  #pragma mark
  
  struct RTCCodec
  {
    
  };
  
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTCMediaAttributes
  #pragma mark
  
  struct RTCMediaAttributes
  {
    
  };
  
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

  interaction IRTCTrack;
  typedef boost::shared_ptr<IRTCTrack> IRTCTrackPtr;
  typedef boost::weak_ptr<IRTCTrack> IRTCTrackWeakPtr;
  
  typedef std::list<IRTCTrackPtr> RTCTrackList;
  typedef boost::shared_ptr<RTCTrackList> RTCTrackListPtr;
  typedef boost::weak_ptr<RTCTrackList> RTCTrackListWeakPtr;

  interaction IRTCSocket;
  typedef boost::shared_ptr<IRTCSocket> IRTCSocketPtr;
  typedef boost::weak_ptr<IRTCSocket> IRTCSocketWeakPtr;

  interaction IMediaStream;
  typedef boost::shared_ptr<IMediaStream> IMediaStreamPtr;
  typedef boost::weak_ptr<IMediaStream> IMediaStreamWeakPtr;
  
  typedef std::list<IMediaStreamPtr> MediaStreamList;
  typedef boost::shared_ptr<MediaStreamList> MediaStreamListPtr;
  typedef boost::weak_ptr<MediaStreamList> MediaStreamListWeakPtr;

  interaction IMediaStreamTrack;
  typedef boost::shared_ptr<IMediaStreamTrack> IMediaStreamTrackPtr;
  typedef boost::weak_ptr<IMediaStreamTrack> IMediaStreamTrackWeakPtr;
  
  interaction IRTCDTMFHandler;
  typedef boost::shared_ptr<IRTCDTMFHandler> IRTCDTMFHandlerPtr;
  typedef boost::weak_ptr<IRTCDTMFHandler> IRTCDTMFHandlerWeakPtr;
  
}
