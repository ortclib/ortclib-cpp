/*

 Copyright (c) 2013, SMB Phone Inc. / Hookflash Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 
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
    #pragma mark RTCIceServer
    #pragma mark
    
    struct RTCIceServer
    {
      URLListPtr mURL;
      String mCredential;
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
      RTCIceServerListPtr mIceServers;
      IRTCSocketPtr mSocket;
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
      String mUsernameFrag;
      String mPassword;
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
      typedef std::map<String, BYTE*> CertificateFingerprintMap;
      
      CertificateFingerprintMap mFingerprintMap;
      
      BYTE* fingerprint(String hashFunction) { return mFingerprintMap[hashFunction]; }
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
      RTCIceConnectionInfo mIce;
      RTCDtlsConnectionInfo mDtlsConnectionInfo;
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
      String mFoundation;
      int mComponent;
      String mTransport;
      int mPriority;
      String mConnectionAddress;
      int mConnectionPort;
      String mType;
      String mRelAddress;
      int mRelPort;
      
      RTCIceCandidateInfo() :
        mComponent(0),
        mPriority(0),
        mConnectionPort(0),
        mRelPort(0)
      { }
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
