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
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTCTrack
  #pragma mark
  
  interaction IRTCTrack
  {
    enum RTCTrackKinds
    {
      RTCTrackKind_Audio,
      RTCTrackKind_Video
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
    
    virtual IMediaStreamTrack source() = 0;
    virtual RTCTrackKinds kind() = 0;
    virtual ULONG ssrc() = 0;
    virtual MsidListPtr msid() = 0;
    virtual RTCCodecListPtr codecs() = 0;
    virtual RTCMediaAttributesListPtr mediaAttributes() = 0;
    virtual RtpExtHeadersMapPtr rtpExtHeaders() = 0;
    
    virtual RTCTrackDescription getDescription() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void remove() = 0;
  };
}
