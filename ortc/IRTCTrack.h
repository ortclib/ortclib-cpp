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

namespace ortc
{
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTCTrack
  #pragma mark
  
  interaction IRTCTrack
  {
    struct RTCCodec;
    struct RTCCodecParam;
    struct RTCMediaAttributes;
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCTrackKinds
    #pragma mark
    
    enum RTCTrackKinds
    {
      RTCTrackKind_Audio,
      RTCTrackKind_Video
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTCCodecParam
    #pragma mark
    
    struct RTCCodecParam
    {
      String mName;
      String mValue;
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
      typedef std::list<RTCCodecParam> RTCCodecParamList;
      
      BYTE mPayloadId;
      String mName;
      int mClockRate;
      int mNumChannels;
      RTCCodecParamList mParams;
      
      RTCCodec() : mPayloadId(0), mClockRate(0), mNumChannels(0) { }
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
      int mVideoMaxWidth;
      int mVideoMaxHeight;
      
      RTCMediaAttributes() : mVideoMaxWidth(0), mVideoMaxHeight(0) { }
    };

    typedef std::list<String> MsidList;
    typedef boost::shared_ptr<MsidList> MsidListPtr;
    typedef boost::weak_ptr<MsidList> MsidListWeakPtr;
    
    typedef std::list<RTCCodec> RTCCodecList;
    typedef boost::shared_ptr<RTCCodecList> RTCCodecListPtr;
    typedef boost::weak_ptr<RTCCodecList> RTCCodecListWeakPtr;
    
    typedef std::list<RTCMediaAttributes> RTCMediaAttributesList;
    typedef boost::shared_ptr<RTCMediaAttributesList> RTCMediaAttributesListPtr;
    typedef boost::weak_ptr<RTCMediaAttributesList> RTCMediaAttributesListWeakPtr;
    
    typedef std::map<String, String> RtpExtHeadersMap;
    typedef boost::shared_ptr<RtpExtHeadersMap> RtpExtHeadersMapPtr;
    typedef boost::weak_ptr<RtpExtHeadersMap> RtpExtHeadersMapWeakPtr;
    
    virtual IMediaStreamTrackPtr source() = 0;
    virtual String id() = 0;
    virtual RTCTrackKinds kind() = 0;
    virtual ULONG ssrc() = 0;
    virtual RTCCodecListPtr codecs() = 0;
    virtual RTCMediaAttributesListPtr mediaAttributes() = 0;
    virtual RtpExtHeadersMapPtr rtpExtHeaders() = 0;
    
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void remove() = 0;
  };
}
