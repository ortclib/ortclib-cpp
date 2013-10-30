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
#include <ortc/IRTCTrack.h>

namespace ortc
{
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTCConnection
  #pragma mark
  
  interaction IRTCConnection
  {
    typedef std::map<String, String> RtpExtHeadersMap;
    typedef boost::shared_ptr<RtpExtHeadersMap> RtpExtHeadersMapPtr;
    typedef boost::weak_ptr<RtpExtHeadersMap> RtpExtHeadersMapWeakPtr;
    
    virtual RTCConnectionSide local() = 0;
    virtual RTCConnectionSide remote() = 0;
    
    virtual IRTCSocket getLocalSocket() = 0;
    virtual void setLocalCandidate(RTCIceCandidateDescription candidate) = 0;
    virtual void setRemoteCandidate(RTCIceCandidateDescription candidate) = 0;
    virtual void connect();
    virtual void update();
    virtual void addStream(IMediaStreamPtr stream, bool autostart = true) = 0;
    virtual void removeStream(IMediaStreamPtr stream) = 0;
    virtual IRTCTrackPtr track(IMediaStreamTrackPtr track) = 0;
    virtual RTCTrackListPtr tracks() = 0;
    virtual RTCTrackListPtr tracks(RTCTrackFilter filter) = 0;
    virtual void receiveTrack(RTCTrackDescription trackDescription) = 0;
    virtual IRTCDTMFHandlerPtr addDtmfHandler() = 0;
    virtual IRTCDTMFHandlerPtr addDtmfHandler(IMediaStreamPtr container) = 0;
    virtual IRTCDTMFHandlerPtr addDtmfHandler(IMediaStreamTrackPtr container) = 0;
    virtual MediaStreamListPtr getSendingStreams() = 0;
    virtual MediaStreamListPtr getReceivingStreams() = 0;
    virtual void close();
  };
  
  interaction IRTCConnectionDelegate
  {
    virtual void onRTCConnectionCandidate(RTCIceCandidateDescription candidate) = 0;
    virtual void onRTCConnectionEndOfCandidates() = 0;
    virtual void onRTCConnectionActiveCandidate(RTCIceCandidateDescription localCandidate, RTCIceCandidateDescription remoteCandidate) = 0;
    virtual void onRTCConnectionConnected() = 0;
    virtual void onRTCConnectionDisconnected() = 0;
    virtual void onRTCConnectionAddStream(IMediaStreamPtr stream) = 0;
    virtual void onRTCConnectionUnknownTrack(IRTCConnection::RtpExtHeadersMapPtr rtpExtHeaders) = 0;
    virtual void onRTCConnectionAddDtmfHandler(IRTCDTMFHandlerPtr handler) = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IRTCConnectionDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::RTCIceCandidateDescription, RTCIceCandidateDescription)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTCConnection::RtpExtHeadersMapPtr, RtpExtHeadersMapPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamPtr, IMediaStreamPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTCDTMFHandlerPtr, IRTCDTMFHandlerPtr)
ZS_DECLARE_PROXY_METHOD_1(onRTCConnectionCandidate, RTCIceCandidateDescription)
ZS_DECLARE_PROXY_METHOD_0(onRTCConnectionEndOfCandidates)
ZS_DECLARE_PROXY_METHOD_2(onRTCConnectionActiveCandidate, RTCIceCandidateDescription, RTCIceCandidateDescription)
ZS_DECLARE_PROXY_METHOD_0(onRTCConnectionConnected)
ZS_DECLARE_PROXY_METHOD_0(onRTCConnectionDisconnected)
ZS_DECLARE_PROXY_METHOD_1(onRTCConnectionAddStream, IMediaStreamPtr)
ZS_DECLARE_PROXY_METHOD_1(onRTCConnectionUnknownTrack, RtpExtHeadersMapPtr)
ZS_DECLARE_PROXY_METHOD_1(onRTCConnectionAddDtmfHandler, IRTCDTMFHandlerPtr)
ZS_DECLARE_PROXY_END()
