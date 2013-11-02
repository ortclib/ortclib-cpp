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
    struct RTCConnectionOptions;
    struct RTCIceServer;
    struct RTCConnectionInfo;
    struct RTCIceConnectionInfo;
    struct RTCDtlsConnectionInfo;
    struct RTCIceCandidateInfo;
    
    typedef std::list<RTCIceServer> RTCIceServerList;
    typedef boost::shared_ptr<RTCIceServerList> RTCIceServerListPtr;
    typedef boost::weak_ptr<RTCIceServerList> RTCIceServerListWeakPtr;
    
    typedef std::list<String> URLList;
    typedef boost::shared_ptr<URLList> URLListPtr;
    typedef boost::weak_ptr<URLList> URLListWeakPtr;
    
    typedef std::map<String, String> RtpExtHeadersMap;
    typedef boost::shared_ptr<RtpExtHeadersMap> RtpExtHeadersMapPtr;
    typedef boost::weak_ptr<RtpExtHeadersMap> RtpExtHeadersMapWeakPtr;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCConnectionStates
    #pragma mark
    
    enum RTCConnectionStates
    {
      RTCConnectionState_New,
      RTCConnectionState_Searching,
      RTCConnectionState_Halted,
      RTCConnectionState_Connected,
      RTCConnectionState_Closed
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCConnectionRoles
    #pragma mark
    
    enum RTCConnectionRoles
    {
      RTCConnectionRole_Controlling,
      RTCConnectionRole_Controlled
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCConnectionOptions
    #pragma mark
    
    struct RTCConnectionOptions
    {
      RTCIceServerListPtr iceServers();
      IRTCSocketPtr socket();
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCIceServer
    #pragma mark
    
    struct RTCIceServer
    {
      URLListPtr url();
      String credential();
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCConnectionInfo
    #pragma mark
    
    struct RTCConnectionInfo
    {
      RTCIceConnectionInfo ice();
      RTCDtlsConnectionInfo dtls();
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCIceConnectionInfo
    #pragma mark
    
    struct RTCIceConnectionInfo
    {
      String usernameFrag();
      String password();
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCDtlsConnectionInfo
    #pragma mark
    
    struct RTCDtlsConnectionInfo
    {
      BYTE* fingerprint(String hashFunction);
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCIceCandidateInfo
    #pragma mark
    
    struct RTCIceCandidateInfo
    {
      String foundation();
      int component();
      String transport();
      int priority();
      String connectionAddress();
      int connectionPort();
      String type();
      String relAddress();
      int relPort();
    };
    
    virtual IRTCSocketPtr socket() = 0;
    virtual RTCConnectionStates state() = 0;
    virtual RTCConnectionRoles role() = 0;
    virtual RTCConnectionInfo local() = 0;
    virtual RTCConnectionInfo remote() = 0;
    
    virtual void addRemoteCandidate(RTCIceCandidateInfo candidate) = 0;
    virtual void connect() = 0;
    virtual void gather() = 0;
    virtual IRTCStreamPtr send(IMediaStreamPtr stream) = 0;
    virtual IRTCTrackPtr send(IMediaStreamTrackPtr track) = 0;
    virtual IRTCStreamPtr send(IRTCStreamPtr stream) = 0;
    virtual IRTCTrackPtr send(IRTCTrackPtr track) = 0;
    virtual IRTCStreamPtr receive(IRTCStreamPtr stream) = 0;
    virtual IRTCTrackPtr receive(IRTCTrackPtr track) = 0;
    virtual RTCStreamListPtr sendStreams() = 0;
    virtual RTCTrackListPtr sendTracks() = 0;
    virtual RTCStreamListPtr receiveStreams() = 0;
    virtual RTCTrackListPtr receiveTracks() = 0;
    virtual void close() = 0;
  };
  
  interaction IRTCConnectionDelegate
  {
    virtual void onRTCConnectionCandidate(IRTCConnection::RTCIceCandidateInfo candidate) = 0;
    virtual void onRTCConnectionEndOfCandidates() = 0;
    virtual void onRTCConnectionActiveCandidate(IRTCConnection::RTCIceCandidateInfo localCandidate,
                                                IRTCConnection::RTCIceCandidateInfo remoteCandidate) = 0;
    virtual void onRTCConnectionStateChanged(IRTCConnection::RTCConnectionStates state) = 0;
    virtual void onRTCConnectionNetworkChange() = 0;
    virtual void onRTCConnectionUnknownTrack(IRTCConnection::RtpExtHeadersMapPtr rtpExtHeaders) = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IRTCConnectionDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTCConnection::RTCIceCandidateInfo, RTCIceCandidateInfo)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTCConnection::RTCConnectionStates, RTCConnectionStates)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IRTCConnection::RtpExtHeadersMapPtr, RtpExtHeadersMapPtr)
ZS_DECLARE_PROXY_METHOD_1(onRTCConnectionCandidate, RTCIceCandidateInfo)
ZS_DECLARE_PROXY_METHOD_0(onRTCConnectionEndOfCandidates)
ZS_DECLARE_PROXY_METHOD_2(onRTCConnectionActiveCandidate, RTCIceCandidateInfo, RTCIceCandidateInfo)
ZS_DECLARE_PROXY_METHOD_1(onRTCConnectionStateChanged, RTCConnectionStates)
ZS_DECLARE_PROXY_METHOD_0(onRTCConnectionNetworkChange)
ZS_DECLARE_PROXY_METHOD_1(onRTCConnectionUnknownTrack, RtpExtHeadersMapPtr)
ZS_DECLARE_PROXY_END()
