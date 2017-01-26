/*

 Copyright (c) 2016, Hookflash Inc. / Hookflash Inc.
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


#include <ortc/adapter/internal/ortc_adapter_SDPParser.h>
#include <ortc/adapter/internal/ortc_adapter_SessionDescription.h>
#include <ortc/adapter/internal/ortc_adapter_Helper.h>

#include <ortc/internal/ortc_Helper.h>

#include <ortc/adapter/IHelper.h>

#include <zsLib/Log.h>
#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/XML.h>
#include <zsLib/SafeInt.h>

#include <cryptopp/sha.h>

#ifdef _MSC_VER
#pragma warning(3 : 4062)
#endif //_MSC_VER

namespace ortc { namespace adapter { ZS_DECLARE_SUBSYSTEM(ortclib_adapter) } }

namespace ortc
{
  namespace adapter
  {
    using zsLib::Stringize;
    using zsLib::Numeric;

    namespace internal
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Helpers
      #pragma mark

      //-----------------------------------------------------------------------
      static Log::Params slog(const char *message)
      {
        return Log::Params(message, "ortc::adapter::SDPParser");
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ISDPTypes
      #pragma mark

      //-----------------------------------------------------------------------
      const char *ISDPTypes::toString(LineTypes type)
      {
        switch (type)
        {
          case LineType_Unknown:                return "";

          case LineType_v_Version:              return "v";
          case LineType_o_Origin:               return "o";
          case LineType_s_SessionName:          return "s";
          case LineType_b_Bandwidth:            return "b";
          case LineType_t_Timing:               return "t";
          case LineType_a_Attributes:           return "a";
          case LineType_m_MediaLine:            return "m";
          case LineType_c_ConnectionDataLine:   return "c";
        }
        ORTC_THROW_NOT_SUPPORTED_ERRROR("unknown line type");
      }

      //-----------------------------------------------------------------------
      ISDPTypes::LineTypes ISDPTypes::toLineType(const char *type)
      {
        String str(type);
        
        static LineTypes checkLines[] = {
          LineType_v_Version,
          LineType_o_Origin,
          LineType_s_SessionName,
          LineType_b_Bandwidth,
          LineType_t_Timing,
          LineType_a_Attributes,
          LineType_m_MediaLine,
          LineType_c_ConnectionDataLine,
          LineType_Unknown
        };

        for (size_t index = 0; LineType_Unknown != checkLines[index]; ++index)
        {
          if (0 != str.compare(toString(checkLines[index]))) continue;
          return checkLines[index];
        }
        return LineType_Unknown;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::LineTypes ISDPTypes::toLineType(const char type)
      {
        Optional<ISDPTypes::LineTypes> result;

        switch (type)
        {
          case LineType_Unknown:              break;

          case LineType_v_Version:            return LineType_v_Version;
          case LineType_o_Origin:             return LineType_o_Origin;
          case LineType_s_SessionName:        return LineType_s_SessionName;
          case LineType_b_Bandwidth:          return LineType_b_Bandwidth;
          case LineType_t_Timing:             return LineType_t_Timing;
          case LineType_a_Attributes:         return LineType_a_Attributes;
          case LineType_m_MediaLine:          return LineType_m_MediaLine;
          case LineType_c_ConnectionDataLine: return LineType_c_ConnectionDataLine;
        }

        return LineType_Unknown;
      }

      //-----------------------------------------------------------------------
      const char *ISDPTypes::toString(Attributes attribute)
      {
        switch (attribute)
        {
          case Attribute_Unknown:           return "";

          case Attribute_Group:             return "group";
          case Attribute_BundleOnly:        return "bundle-only";
          case Attribute_MSID:              return "msid";
          case Attribute_MSIDSemantic:      return "msid-semantic";
          case Attribute_ICEUFrag:          return "ice-ufrag";
          case Attribute_ICEPwd:            return "ice-pwd";
          case Attribute_ICEOptions:        return "ice-options";
          case Attribute_ICELite:           return "ice-lite";
          case Attribute_Candidate:         return "candidate";
          case Attribute_EndOfCandidates:   return "end-of-candidates";
          case Attribute_Fingerprint:       return "fingerprint";
          case Attribute_Crypto:            return "crypto";
          case Attribute_Setup:             return "setup";
          case Attribute_MID:               return "mid";
          case Attribute_Extmap:            return "extmap";
          case Attribute_SendRecv:          return "sendrecv";
          case Attribute_SendOnly:          return "sendonly";
          case Attribute_RecvOnly:          return "recvonly";
          case Attribute_Inactive:          return "inactive";
          case Attribute_RTPMap:            return "rtpmap";
          case Attirbute_FMTP:              return "fmtp";
          case Attribute_RTCP:              return "rtcp";
          case Attribute_RTCPMux:           return "rtcp-mux";
          case Attribute_RTCPFB:            return "rtcp-fb";
          case Attribute_RTCPRSize:          return "rtcp-rsize";
          case Attribute_PTime:             return "ptime";
          case Attribute_MaxPTime:          return "maxptime";
          case Attribute_SSRC:              return "ssrc";
          case Attribute_SSRCGroup:         return "ssrc-group";
          case Attribute_Simulcast:         return "simulcast";
          case Attribute_RID:               return "rid";
          case Attribute_SCTPPort:          return "sctp-port";
          case Attribute_MaxMessageSize:    return "max-message-size";
        }
        ORTC_THROW_NOT_SUPPORTED_ERRROR("unknown attribute");
      }

      //-----------------------------------------------------------------------
      bool ISDPTypes::requiresValue(Attributes attribute)
      {
        switch (attribute)
        {
          case Attribute_Unknown:           break;

          case Attribute_Group:             return true;
          case Attribute_BundleOnly:        break;
          case Attribute_MSID:              return true;
          case Attribute_MSIDSemantic:      return true;
          case Attribute_ICEUFrag:          return true;
          case Attribute_ICEPwd:            return true;
          case Attribute_ICEOptions:        return true;
          case Attribute_ICELite:           break;
          case Attribute_Candidate:         return true;
          case Attribute_EndOfCandidates:   break;
          case Attribute_Fingerprint:       return true;
          case Attribute_Crypto:            return true;
          case Attribute_Setup:             return true;
          case Attribute_MID:               return true;
          case Attribute_Extmap:            return true;
          case Attribute_SendRecv:          break;
          case Attribute_SendOnly:          break;
          case Attribute_RecvOnly:          break;
          case Attribute_Inactive:          break;
          case Attribute_RTPMap:            return true;
          case Attirbute_FMTP:              return true;
          case Attribute_RTCP:              return true;
          case Attribute_RTCPMux:           break;
          case Attribute_RTCPFB:            return true;
          case Attribute_RTCPRSize:          break;
          case Attribute_PTime:             return true;
          case Attribute_MaxPTime:          return true;
          case Attribute_SSRC:              return true;
          case Attribute_SSRCGroup:         return true;
          case Attribute_Simulcast:         return true;
          case Attribute_RID:               return true;
          case Attribute_SCTPPort:          return true;
          case Attribute_MaxMessageSize:    return true;
        }
        return false;
      }

      //-----------------------------------------------------------------------
      bool ISDPTypes::requiresEmptyValue(Attributes attribute)
      {
        switch (attribute)
        {
          case Attribute_Unknown:           break;

          case Attribute_Group:             break;
          case Attribute_BundleOnly:        return true;
          case Attribute_MSID:              break;
          case Attribute_MSIDSemantic:      break;
          case Attribute_ICEUFrag:          break;
          case Attribute_ICEPwd:            break;
          case Attribute_ICEOptions:        break;
          case Attribute_ICELite:           return true;
          case Attribute_Candidate:         break;
          case Attribute_EndOfCandidates:   return true;
          case Attribute_Fingerprint:       break;
          case Attribute_Crypto:            break;
          case Attribute_Setup:             break;
          case Attribute_MID:               break;
          case Attribute_Extmap:            break;
          case Attribute_SendRecv:          return true;
          case Attribute_SendOnly:          return true;
          case Attribute_RecvOnly:          return true;
          case Attribute_Inactive:          return true;
          case Attribute_RTPMap:            break;
          case Attirbute_FMTP:              break;
          case Attribute_RTCP:              break;
          case Attribute_RTCPMux:           return true;
          case Attribute_RTCPFB:            break;
          case Attribute_RTCPRSize:         return true;
          case Attribute_PTime:             break;
          case Attribute_MaxPTime:          break;
          case Attribute_SSRC:              break;
          case Attribute_SSRCGroup:         break;
          case Attribute_Simulcast:         break;
          case Attribute_RID:               break;
          case Attribute_SCTPPort:          break;
          case Attribute_MaxMessageSize:    break;
        }
        return false;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::Attributes ISDPTypes::toAttribute(const char *attribute)
      {
        String str(attribute);
        for (Attributes index = Attribute_First; index <= Attribute_Last; index = static_cast<Attributes>(static_cast<std::underlying_type<Attributes>::type>(index) + 1)) {
          if (0 == str.compare(toString(index))) return index;
        }

        return Attribute_Unknown;
      }

      //-----------------------------------------------------------------------
      const char *ISDPTypes::toString(AttributeLevels level)
      {
        switch (level)
        {
          case AttributeLevel_None:                 return "none";
          case AttributeLevel_Session:              return "session";
          case AttributeLevel_Media:                return "media";
          case AttributeLevel_Source:               return "source";
          case AttributeLevel_SessionAndMedia:      return "session+media";
          case AttributeLevel_SessionAndSource:     return "session+source";
          case AttributeLevel_MediaAndSource:       return "media+source";
          case AttributeLevel_All:                  return "all";
        }
        ORTC_THROW_NOT_SUPPORTED_ERRROR("unknown attribute level");
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AttributeLevels ISDPTypes::toAttributeLevel(const char *level)
      {
        static AttributeLevels check[] =
        {
          AttributeLevel_Session,
          AttributeLevel_Media,
          AttributeLevel_Source,
          AttributeLevel_SessionAndMedia,
          AttributeLevel_SessionAndSource,
          AttributeLevel_MediaAndSource,
          AttributeLevel_MediaAndSource,
          AttributeLevel_None,
        };
        String str(level);
        for (size_t index = 0; AttributeLevel_None != check[index]; ++index) {
          if (0 == str.compareNoCase(toString(check[index]))) return check[index];
        }

        ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str);
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AttributeLevels ISDPTypes::getAttributeLevel(LineTypes lineType)
      {
        switch (lineType)
        {
          case LineType_Unknown:              break;

          case LineType_v_Version:            return AttributeLevel_Session;
          case LineType_o_Origin:             return AttributeLevel_Session;
          case LineType_s_SessionName:        return AttributeLevel_Session;
          case LineType_b_Bandwidth:          return AttributeLevel_SessionAndMedia;
          case LineType_t_Timing:             return AttributeLevel_Session;
          case LineType_a_Attributes:         return AttributeLevel_All;
          case LineType_m_MediaLine:          return AttributeLevel_Session;
          case LineType_c_ConnectionDataLine: return AttributeLevel_SessionAndMedia;
        }
        ORTC_THROW_NOT_SUPPORTED_ERRROR("unknown line type");
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AttributeLevels ISDPTypes::getAttributeLevel(Attributes attribute)
      {
        switch (attribute)
        {
          case Attribute_Unknown:           break;

          case Attribute_Group:             return AttributeLevel_Session;
          case Attribute_BundleOnly:        return AttributeLevel_Media;
          case Attribute_MSID:              return AttributeLevel_Media;
          case Attribute_MSIDSemantic:      return AttributeLevel_Session;
          case Attribute_ICEUFrag:          return AttributeLevel_SessionAndMedia;
          case Attribute_ICEPwd:            return AttributeLevel_SessionAndMedia;
          case Attribute_ICEOptions:        return AttributeLevel_Session;
          case Attribute_ICELite:           return AttributeLevel_Session;
          case Attribute_Candidate:         return AttributeLevel_Media;
          case Attribute_EndOfCandidates:   return AttributeLevel_Media;
          case Attribute_Fingerprint:       return AttributeLevel_SessionAndMedia;
          case Attribute_Crypto:            return AttributeLevel_Media;
          case Attribute_Setup:             return AttributeLevel_SessionAndMedia;
          case Attribute_MID:               return AttributeLevel_Media;
          case Attribute_Extmap:            return AttributeLevel_SessionAndMedia;
          case Attribute_SendRecv:          return AttributeLevel_SessionAndMedia;
          case Attribute_SendOnly:          return AttributeLevel_SessionAndMedia;
          case Attribute_RecvOnly:          return AttributeLevel_SessionAndMedia;
          case Attribute_Inactive:          return AttributeLevel_SessionAndMedia;
          case Attribute_RTPMap:            return AttributeLevel_Media;
          case Attirbute_FMTP:              return AttributeLevel_MediaAndSource;
          case Attribute_RTCP:              return AttributeLevel_Media;
          case Attribute_RTCPMux:           return AttributeLevel_Media;
          case Attribute_RTCPFB:            return AttributeLevel_Media;
          case Attribute_RTCPRSize:         return AttributeLevel_Media;
          case Attribute_PTime:             return AttributeLevel_Media;
          case Attribute_MaxPTime:          return AttributeLevel_Media;
          case Attribute_SSRC:              return AttributeLevel_Media;
          case Attribute_SSRCGroup:         return AttributeLevel_Media;
          case Attribute_Simulcast:         return AttributeLevel_Media;
          case Attribute_RID:               return AttributeLevel_Media;
          case Attribute_SCTPPort:          return AttributeLevel_Media;
          case Attribute_MaxMessageSize:    return AttributeLevel_Media;
        }
        ORTC_THROW_NOT_SUPPORTED_ERRROR("unknown attribute");
      }

      //-----------------------------------------------------------------------
      bool ISDPTypes::supportedAtLevel(
                                       AttributeLevels currentLevel,
                                       LineTypes lineType
                                       )
      {
        auto allowedLevels = getAttributeLevel(lineType);
        return (0 != (allowedLevels & currentLevel));
      }

      //-----------------------------------------------------------------------
      bool ISDPTypes::supportedAtLevel(
                                       AttributeLevels currentLevel,
                                       Attributes attribute
                                       )
      {
        auto allowedLevels = getAttributeLevel(attribute);
        return (0 != (allowedLevels & currentLevel));
      }

      //-----------------------------------------------------------------------
      const char *ISDPTypes::toString(Locations location)
      {
        switch (location)
        {
          case Location_Local:  return "local";
          case Location_Remote: return "remote";
        }
        ORTC_THROW_NOT_SUPPORTED_ERRROR("unknown location");
      }

      //-----------------------------------------------------------------------
      ISDPTypes::Locations ISDPTypes::toLocation(const char *location)
      {
        String str(location);
        for (Locations index = Location_First; index <= Location_Last; index = static_cast<Locations>(static_cast<std::underlying_type<Locations>::type>(index) + 1)) {
          if (0 == str.compare(toString(index))) return index;
        }
        ORTC_THROW_NOT_SUPPORTED_ERRROR("unknown location");
      }

      //-----------------------------------------------------------------------
      const char *ISDPTypes::toString(Directions direction)
      {
        switch (direction)
        {
          case Direction_None:        return "inactive";
          case Direction_Send:        return "send";
          case Direction_Receive:     return "recv";
          case Direction_SendReceive: return "sendrecv";
        }

        ORTC_THROW_NOT_SUPPORTED_ERRROR("unknown direction");
      }

      //-----------------------------------------------------------------------
      ISDPTypes::Directions ISDPTypes::toDirection(const char *direction)
      {
        static Directions check[] =
        {
          Direction_Send,
          Direction_Receive,
          Direction_SendReceive,
          Direction_None
        };

        String str(direction);
        if (str.isEmpty()) return Direction_None;

        for (size_t index = 0; index <= (sizeof(check)/sizeof(check[0])); ++index) {
          if (0 == str.compareNoCase(toString(check[index]))) return check[index];
        }

        ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str);
      }

      //-----------------------------------------------------------------------
      bool ISDPTypes::isValid(
                              Directions direction,
                              bool allowNone,
                              bool allowSend,
                              bool allowReceive,
                              bool allowSendReceive
                              )
      {
        if ((!allowNone) &&
            (Direction_None == direction)) return false;
        if ((!allowSend) &&
            (Direction_Send == direction)) return false;
        if ((!allowReceive) &&
            (Direction_Receive == direction)) return false;
        if ((!allowSendReceive) &&
            (Direction_SendReceive == direction)) return false;
        return true;
      }

      //-----------------------------------------------------------------------
      const char *ISDPTypes::toString(ActorRoles actor)
      {
        switch (actor)
        {
          case ActorRole_Sender:      return "sender";
          case ActorRole_Receiver:    return "receiver";
          case ActorRole_Transceiver: return "transceiver";
        }
        ORTC_THROW_NOT_SUPPORTED_ERRROR("unknown actor role");
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ActorRoles ISDPTypes::toActorRole(const char *actor)
      {
        static ActorRoles check[] =
        {
          ActorRole_Sender,
          ActorRole_Receiver,
          ActorRole_Transceiver
        };

        String str(actor);

        for (size_t index = 0; index <= (sizeof(check) / sizeof(check[0])); ++index) {
          if (0 == str.compareNoCase(toString(check[index]))) return check[index];
        }

        ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str);
      }

      //-----------------------------------------------------------------------
      bool ISDPTypes::isApplicable(
                                   ActorRoles actor,
                                   Locations location,
                                   Directions direction
                                   )
      {
        switch (actor)
        {
          case ActorRole_Sender:
          {
            switch (location)
            {
              case Location_Local:  return (0 != (Direction_Send & direction));
              case Location_Remote: return (0 != (Direction_Receive & direction));
            }
          }
          case ActorRole_Receiver:
          {
            switch (location)
            {
              case Location_Local:  return (0 != (Direction_Receive & direction));
              case Location_Remote: return (0 != (Direction_Send & direction));
            }
          }
          case ActorRole_Transceiver:
          {
            switch (location)
            {
              case Location_Local:  return (0 != (Direction_SendReceive & direction));
              case Location_Remote: return (0 != (Direction_SendReceive & direction));
            }
          }
        }
        ORTC_THROW_NOT_SUPPORTED_ERRROR("role / location was not found");
      }

      //-----------------------------------------------------------------------
      const char *ISDPTypes::toString(ProtocolTypes proto)
      {
        switch (proto)
        {
          case ProtocolType_Unknown:  return "";
          case ProtocolType_RTP:      return "UDP/TLS/RTP/SAVPF";
          case ProtocolType_SCTP:     return "UDP/DTLS/SCTP";
        }
        ORTC_THROW_NOT_SUPPORTED_ERRROR("unknown protocol type");
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ProtocolTypes ISDPTypes::toProtocolType(const char *proto)
      {
        String str(proto);
        IHelper::SplitMap protoSplit;
        IHelper::split(str, protoSplit, "/");
        ORTC_THROW_INVALID_PARAMETERS_IF(protoSplit.size() < 2);

        if (0 == protoSplit[0].compareNoCase("RTP")) {
          if (0 == protoSplit[1].compareNoCase("AVP")) return ProtocolType_RTP;
          if (0 == protoSplit[1].compareNoCase("SAVP")) return ProtocolType_RTP;
          if (0 == protoSplit[1].compareNoCase("AVPF")) return ProtocolType_RTP;
          if (0 == protoSplit[1].compareNoCase("SAVPF")) return ProtocolType_RTP;
          return ProtocolType_Unknown;
        }
        if (0 == protoSplit[0].compareNoCase("DTLS")) {
          if (0 == protoSplit[1].compareNoCase("SCTP")) return ProtocolType_SCTP;
          return ProtocolType_Unknown;
        }

        if (protoSplit.size() < 3) return ProtocolType_Unknown;

        if (0 != protoSplit[0].compareNoCase("UDP")) {
          if (0 != protoSplit[0].compareNoCase("TCP")) return ProtocolType_Unknown;
        }

        if (0 == protoSplit[1].compareNoCase("DTLS")) {
          if (0 != protoSplit[2].compareNoCase("SCTP")) return ProtocolType_Unknown;
          return ProtocolType_SCTP;
        }

        if (protoSplit.size() < 4) return ProtocolType_Unknown;
        if (0 == protoSplit[3].compareNoCase("SAVP")) return ProtocolType_RTP;
        if (0 == protoSplit[3].compareNoCase("SAVPF")) return ProtocolType_RTP;
        return ProtocolType_Unknown;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark LineValue types
      #pragma mark

      //-----------------------------------------------------------------------
      ISDPTypes::VLine::VLine(const char *value)
      {
        String str(value);
        str.trim();
        try {
          mVersion = Numeric<decltype(mVersion)>(str);
        } catch (const Numeric<decltype(mVersion)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("version value out of range: " + str);
        }
        // NOTE: Do not allow an alternative version as it may not be
        // correctly understood.
        ORTC_THROW_INVALID_PARAMETERS_IF(0 != mVersion);
      }

      //-----------------------------------------------------------------------
      ISDPTypes::OLine::OLine(const char *value)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(6 != split.size());

        mUsername = split[0];
        mNetType = split[3];
        mAddrType = split[4];
        mUnicastAddress = split[5];

        try {
          mSessionID = Numeric<decltype(mSessionID)>(split[1]);
        } catch (const Numeric<decltype(mSessionID)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("session id value out of range: " + split[1]);
        }
        try {
          mSessionVersion = Numeric<decltype(mSessionVersion)>(split[2]);
        } catch (const Numeric<decltype(mSessionVersion)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("session version value out of range: " + split[2]);
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::OLine::toString() const
      {
        String result;
        result.reserve(50);
        result.append(mUsername.hasData() ? mUsername : String("-"));
        result.append(" ");
        result.append(string(mSessionID));
        result.append(" ");
        result.append(string(mSessionVersion));
        result.append(" ");
        result.append(mNetType.hasData() ? mNetType : String("IN"));
        result.append(" ");
        result.append(mAddrType.hasData() ? mAddrType : String("IP4"));
        result.append(" ");
        result.append(mUnicastAddress.hasData() ? mUnicastAddress : String("127.0.0.1"));
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::SLine::SLine(const char *value) :
          mSessionName(value)
      {
      }

      //-----------------------------------------------------------------------
      ISDPTypes::BLine::BLine(MLinePtr mline, const char *value) :
        MediaLine(mline)
      {
        String str(value);
        IHelper::SplitMap split;
        IHelper::split(String(value), split, ":");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(2 != split.size());

        mBWType = split[0];
        try {
          mBandwidth = Numeric<decltype(mBandwidth)>(split[2]);
        } catch(const Numeric<decltype(mBandwidth)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("bandwidth value out of range: " + split[2]);
        }
      }

      //-----------------------------------------------------------------------
      ISDPTypes::TLine::TLine(const char *value)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(2 != split.size());

        try {
          mStartTime = Numeric<decltype(mStartTime)>(split[0]);
        } catch (const Numeric<decltype(mStartTime)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("start time value out of range: " + split[0]);
        }
        try {
          mEndTime = Numeric<decltype(mEndTime)>(split[1]);
        } catch (const Numeric<decltype(mEndTime)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("end time value out of range: " + split[1]);
        }
      }

      //-----------------------------------------------------------------------
      ISDPTypes::MLine::MLine(const char *value)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 3);

        mMedia = split[0];
        mProtoStr = split[2];
        mProto = toProtocolType(mProtoStr);

        // fix port
        {
          IHelper::SplitMap portSplit;
          IHelper::split(split[1], portSplit, "/");
          IHelper::splitTrim(portSplit);
          IHelper::splitPruneEmpty(portSplit);
          ORTC_THROW_INVALID_PARAMETERS_IF(portSplit.size() > 2);
          ORTC_THROW_INVALID_PARAMETERS_IF(portSplit.size() < 1);

          try {
            mPort = Numeric<decltype(mPort)>(portSplit[0]);
          } catch (const Numeric<decltype(mPort)>::ValueOutOfRange &) {
            ORTC_THROW_INVALID_PARAMETERS("port value out of range: " + portSplit[0]);
          }
          if (portSplit.size() > 1) {
            try {
              mInteger = Numeric<decltype(mInteger)>(portSplit[1]);
            } catch (const Numeric<decltype(mInteger)>::ValueOutOfRange &) {
              ORTC_THROW_INVALID_PARAMETERS("port value out of range: " + portSplit[1]);
            }
          }
        }

        for (size_t index = 3; index < split.size(); ++index)
        {
          mFmts.push_back(split[index]);
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::MLine::toString() const
      {
        String result;
        result.append(mMedia);
        result.append(" ");
        result.append(string(mPort));
        if (0 != mInteger) {
          result.append("/");
          result.append(string(mInteger));
        }
        result.append(" ");
        result.append(mProtoStr);
        result.append(" ");
        result.append(IHelper::combine(mFmts, " "));
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::CLine::CLine(MLinePtr mline, const char *value) :
        MediaLine(mline)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(3 != split.size());

        mNetType = split[0];
        mAddrType = split[1];
        mConnectionAddress = split[2];
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::CLine::toString() const
      {
        String result;
        result.append(mNetType.hasData() ? mNetType : String("IN"));
        result.append(" ");
        result.append(mAddrType.hasData() ? mAddrType : String("IP4"));
        result.append(" ");
        result.append(mConnectionAddress.hasData() ? mConnectionAddress : String("0.0.0.0"));
        return result;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ALine types
      #pragma mark

      //-----------------------------------------------------------------------
      ISDPTypes::AGroupLine::AGroupLine(const char *value)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 1);

        mSemantic = split[0];

        for (size_t index = 1; index < split.size(); ++index)
        {
          mIdentificationTags.push_back(split[index]);
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::AGroupLine::toString() const
      {
        if (mSemantic.isEmpty()) return String();

        String result("group:");
        result.append(mSemantic);
        result.append(" ");
        result.append(IHelper::combine(mIdentificationTags, " "));
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AMSIDLine::AMSIDLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 1);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() > 2);

        mID = split[0];
        if (split.size() > 1) {
          mAppData = split[1];
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::AMSIDLine::toString() const
      {
        String result;
        result.append("msid:");
        result.append(mID);
        if (mAppData.hasData()) {
          result.append(" ");
          result.append(mAppData);
        }
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AICEUFragLine::AICEUFragLine(MLinePtr mline, const char *value) :
        AMediaLine(mline),
        mICEUFrag(value)
      {
        mICEUFrag.trim();
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AICEPwdLine::AICEPwdLine(MLinePtr mline, const char *value) :
        AMediaLine(mline),
        mICEPwd(value)
      {
        mICEPwd.trim();
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AICEOptionsLine::AICEOptionsLine(const char *value)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 1);

        for (auto iter = split.begin(); iter != split.end(); ++iter) {
          auto &value = (*iter).second;
          mTags.push_back(value);
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::AICEOptionsLine::toString() const
      {
        String result;
        result.append("ice-options:");
        result.append(IHelper::combine(mTags, " "));
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ACandidateLine::ACandidateLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 8);

        if (split.size() > 8) {
          ORTC_THROW_INVALID_PARAMETERS_IF(0 != split.size() % 2);
        }

        mFoundation = split[0];
        try {
          mComponentID = Numeric<decltype(mComponentID)>(split[1]);
          if (!((mComponentID >= IICETypes::Component_First) &&
                (mComponentID <= IICETypes::Component_Last))) {
            ORTC_THROW_INVALID_PARAMETERS("component id value out of range: " + split[1]);
          }
        } catch (const Numeric<decltype(mComponentID)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("component id value out of range: " + split[1]);
        }
        mTransport = split[2];
        try {
          mPriority = Numeric<decltype(mPriority)>(split[3]);
        } catch (const Numeric<decltype(mPriority)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("priority value out of range: " + split[3]);
        }
        mConnectionAddress = split[4];
        try {
          mPort = Numeric<decltype(mPort)>(split[5]);
        } catch (const Numeric<decltype(mPort)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("port value out of range: " + split[5]);
        }
        mTyp = split[6];
        ORTC_THROW_INVALID_PARAMETERS_IF(0 != mTyp.compareNoCase("typ"));
        mCandidateType = split[7];

        if (split.size() > 8) {
          for (size_t index = 10; index < split.size(); index += 2)
          {
            ExtensionPair value(split[index], split[index + 1]);
            if (0 == value.first.compareNoCase("raddr")) {
              mRelAddr = value.second;
              continue;
            }
            if (0 == value.first.compareNoCase("rport")) {
              try {
                mRelPort = Numeric<decltype(mRelPort.mType)>(value.second);
              } catch (const Numeric<decltype(mRelPort.mType)>::ValueOutOfRange &) {
                ORTC_THROW_INVALID_PARAMETERS("rel port value out of range: " + split[9]);
              }
              continue;
            }

            mExtensionPairs.push_back(value);
          }
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::ACandidateLine::toString() const
      {
        String result;
        result.append("candidate:");
        result.append(mFoundation.hasData() ? mFoundation : String("+"));
        result.append(" ");
        result.append(string(mComponentID));
        result.append(" ");
        result.append(mTransport.hasData() ? mTransport : String("UDP"));
        result.append(" ");
        result.append(string(mPriority));
        result.append(" ");
        result.append(mConnectionAddress.hasData() ? mConnectionAddress : String("127.0.0.1"));
        result.append(" ");
        result.append(string(mPort));
        result.append(" ");
        result.append(mTyp.hasData() ? mTyp : String("typ"));
        result.append(" ");
        result.append(mCandidateType.hasData() ? mCandidateType : String("host"));
        if ((mRelAddr.hasData()) ||
            (mRelPort.hasValue()) ||
            (mExtensionPairs.size() > 0)) {
          result.append(" ");
          result.append(mRelAddr.hasData() ? mRelAddr : String("0.0.0.0"));
          result.append(" ");
          result.append(mRelPort.hasValue() ? string(mRelPort.value()) : String("0"));
          if (mExtensionPairs.size() > 0) {
            for (auto iter = mExtensionPairs.begin(); iter != mExtensionPairs.end(); ++iter) {
              auto &name = (*iter).first;
              auto &value = (*iter).second;
              if (name.isEmpty()) {
                ZS_LOG_WARNING(Debug, internal::slog("missing ice extension name") + ZS_PARAM("value", value));
                continue;
              }
              result.append(" ");
              result.append(name);
              result.append(" ");
              result.append(value);
            }
          }
        }
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AFingerprintLine::AFingerprintLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(2 != split.size());

        mHashFunc = split[0];
        mFingerprint = split[1];
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::AFingerprintLine::toString() const
      {
        String result;
        if (mHashFunc.isEmpty() && mFingerprint.isEmpty()) return result;

        result.append("fingerprint:");
        result.append(mHashFunc.hasData() ? mHashFunc : String("ignore"));
        result.append(" ");
        result.append(mFingerprint.hasData() ? mFingerprint : String("00"));
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ACryptoLine::ACryptoLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 3);

        try {
          mTag = Numeric<decltype(mTag)>(split[0]);
        } catch (const Numeric<decltype(mTag)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("tag value out of range: " + split[0]);
        }
        mCryptoSuite = split[1];

        IHelper::SplitMap keyParamsSplit;
        IHelper::split(String(split[2]), keyParamsSplit, ";");
        IHelper::splitTrim(keyParamsSplit);
        IHelper::splitPruneEmpty(keyParamsSplit);
        ORTC_THROW_INVALID_PARAMETERS_IF(keyParamsSplit.size() < 1);

        for (auto iter = keyParamsSplit.begin(); iter != keyParamsSplit.end(); ++iter) {
          auto &keyParam = (*iter).second;

          IHelper::SplitMap keyMethodInfoSplit;
          IHelper::split(keyParam, keyMethodInfoSplit, ":");
          IHelper::splitTrim(keyMethodInfoSplit);
          IHelper::splitPruneEmpty(keyMethodInfoSplit);
          ORTC_THROW_INVALID_PARAMETERS_IF(2 != keyMethodInfoSplit.size());
          
          mKeyParams.push_back(KeyParam(keyMethodInfoSplit[0], keyMethodInfoSplit[1]));
        }

        for (size_t index = 3; index < split.size(); ++index) {
          mSessionParams.push_back(split[index]);
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::ACryptoLine::toString() const
      {
        String result;
        result.append("crypto:");
        result.append(string(mTag));
        result.append(" ");
        result.append(mCryptoSuite.hasData() ? mCryptoSuite : String("_"));
        result.append(" ");

        bool inserted = false;
        for (auto iter = mKeyParams.begin(); iter != mKeyParams.end(); ++iter) {
          auto &keyMethod = (*iter).first;
          auto &keyInfo = (*iter).second;
          if ((keyMethod.isEmpty()) && (keyInfo.isEmpty())) continue;

          if (inserted) {
            result.append(";");
          }
          result.append(keyMethod.hasData() ? keyMethod : String("inline"));
          if (keyInfo.hasData()) {
            result.append(":");
            result.append(keyInfo);
          }
          inserted = true;
        }

        if (!inserted) {
          result.append("ignore:ignore");
        }

        if (mSessionParams.size() > 0) {
          result.append(" ");
          result.append(IHelper::combine(mSessionParams, " "));
        }

        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ASetupLine::ASetupLine(MLinePtr mline, const char *value) :
        AMediaLine(mline),
        mSetup(value)
      {
        mSetup.trim();
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AMIDLine::AMIDLine(MLinePtr mline, const char *value) :
        AMediaLine(mline),
        mMID(value)
      {
        mMID.trim();
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AExtmapLine::AExtmapLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 2);

        IHelper::SplitMap entrySplit;
        IHelper::split(split[0], entrySplit, "/");
        IHelper::splitTrim(entrySplit);
        IHelper::splitPruneEmpty(entrySplit);
        ORTC_THROW_INVALID_PARAMETERS_IF(entrySplit.size() < 1);

        try {
          mID = Numeric<decltype(mID)>(entrySplit[0]);
        } catch (const Numeric<decltype(mID)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("entry id value out of range: " + entrySplit[0]);
        }
        if (entrySplit.size() > 1) {
          mDirection = toDirection(entrySplit[1]);
          ORTC_THROW_INVALID_PARAMETERS_IF(!isValid(mDirection, false, true, true, true));
        } else {
          mDirection = Direction_SendReceive;
        }

        mURI = split[1];
        if (split.size() > 2) {
          mExtensionAttributes = split[2];
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::AExtmapLine::toString() const
      {
        String result;
        result.append("extmap:");
        result.append(string(mID));
        switch (mDirection)
        {
          case Direction_None:
          case Direction_SendReceive:   {
            break;
          }
          case Direction_Send:
          case Direction_Receive:       {
            result.append("/");
            result.append(ISDPTypes::toString(mDirection));
            break;
          }
        }
        result.append(" ");
        result.append(mURI.hasData() ? mURI : String("unknown:unknown"));
        if (mExtensionAttributes.hasData()) {
          result.append(" ");
          result.append(mExtensionAttributes);
        }
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AMediaDirectionLine::AMediaDirectionLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        String str(value);
        str.trim();
        mDirection = toDirection(str);
        ORTC_THROW_INVALID_PARAMETERS_IF(!isValid(mDirection, true, true, true, true));
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ARTPMapLine::ARTPMapLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(2 != split.size());

        try {
          mPayloadType = Numeric<decltype(mPayloadType)>(split[0]);
        } catch (const Numeric<decltype(mPayloadType)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("payload type value out of range: " + split[0]);
        }

        IHelper::SplitMap encodingSplit;
        IHelper::split(split[1], encodingSplit, "/");
        IHelper::splitTrim(encodingSplit);
        IHelper::splitPruneEmpty(encodingSplit);
        ORTC_THROW_INVALID_PARAMETERS_IF(encodingSplit.size() < 2);

        mEncodingName = encodingSplit[0];

        try {
          mClockRate = Numeric<decltype(mClockRate)>(encodingSplit[1]);
        } catch (const Numeric<decltype(mClockRate)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("clock rate value out of range: " + encodingSplit[1]);
        }

        if (encodingSplit.size() > 2) {
          try {
            mEncodingParameters = Numeric<decltype(mEncodingParameters.mType)>(encodingSplit[2]);
          } catch (const Numeric<decltype(mEncodingParameters.mType)>::ValueOutOfRange &) {
            ORTC_THROW_INVALID_PARAMETERS("clock rate value out of range: " + encodingSplit[2]);
          }
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::ARTPMapLine::toString() const
      {
        String result;
        result.append("rtpmap:");
        result.append(string(mPayloadType));
        result.append(" ");
        result.append(mEncodingName.hasData() ? mEncodingName : String("unknown"));
        result.append("/");
        result.append(string(mClockRate));
        if (mEncodingParameters.hasValue()) {
          result.append("/");
          result.append(string(mEncodingParameters.value()));
        }
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AFMTPLine::AFMTPLine(MLinePtr mline, ASSRCLinePtr sourceLine, const char *value) :
        AMediaLine(mline),
        mSourceLine(sourceLine)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 1);

        mFormatStr = split[0];
        if (isdigit((mFormatStr.c_str())[0])) {
          try {
            mFormat = Numeric<decltype(mFormat)>(split[0]);
          } catch (const Numeric<decltype(mFormat)>::ValueOutOfRange &) {
            ORTC_THROW_INVALID_PARAMETERS("payload type value out of range: " + split[0]);
          }
        }

        for (size_t index = 1; index < split.size(); ++index) {
          mFormatSpecific.push_back(split[index]);
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::AFMTPLine::toString() const
      {
        String result;
        if (mFormatSpecific.size() < 1) return result;

        result.append("fmtp:");
        if (mFormatStr.hasData()) {
          result.append(mFormatStr);
        } else {
          result.append(string(mFormat));
        }
        result.append(" ");
        result.append(IHelper::combine(mFormatSpecific, " "));
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ARTCPLine::ARTCPLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 1);

        try {
          mPort = Numeric<decltype(mPort)>(split[0]);
        } catch (const Numeric<decltype(mPort)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("rtcp port value out of range: " + split[0]);
        }

        if (split.size() > 1) {
          ORTC_THROW_INVALID_PARAMETERS_IF(4 != split.size());
          mNetType = split[1];
          mAddrType = split[2];
          mConnectionAddress = split[3];
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::ARTCPLine::toString() const
      {
        String result;
        result.append("rtcp:");
        result.append(string(mPort));
        if (mNetType.hasData() || mAddrType.hasData() || mConnectionAddress.hasData()) {
          result.append(" ");
          result.append(mNetType.hasData() ? mNetType : String("IN"));
          result.append(" ");
          result.append(mAddrType.hasData() ? mAddrType : String("IP4"));
          result.append(" ");
          result.append(mConnectionAddress.hasData() ? mConnectionAddress : String("127.0.0.1"));
        }
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ARTCPFBLine::ARTCPFBLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 2);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() > 4);

        if (0 != split[0].compare("*")) {
          try {
            mPayloadType = Numeric<decltype(mPayloadType.mType)>(split[0]);
          } catch (const Numeric<decltype(mPayloadType.mType)>::ValueOutOfRange &) {
            ORTC_THROW_INVALID_PARAMETERS("rtcp-fb payload type value out of range: " + split[0]);
          }
        }

        mID = split[1];
        if (split.size() > 1) {
          mParam1 = split[2];
        }
        if (split.size() > 2) {
          mParam2 = split[3];
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::ARTCPFBLine::toString() const
      {
        String result;
        if (mID.isEmpty()) return result;

        result.append("rtcp-fb:");
        result.append(mPayloadType.hasValue() ? string(mPayloadType.value()) : String("*"));
        result.append(" ");
        result.append(mID);
        if (mParam1.hasData()) {
          result.append(" ");
          result.append(mParam1);
          if (mParam2.hasData()) {
            result.append(" ");
            result.append(mParam2);
          }
        }
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::APTimeLine::APTimeLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        String str(value);
        try {
          mPTime = Milliseconds(Numeric<Milliseconds::rep>(str));
        } catch (const Numeric<Milliseconds::rep>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("payload time value out of range: " + str);
        }
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AMaxPTimeLine::AMaxPTimeLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        String str(value);
        try {
          mMaxPTime = Milliseconds(Numeric<Milliseconds::rep>(str));
        } catch (const Numeric<Milliseconds::rep>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("max payload time value out of range: " + str);
        }
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ASSRCLine::ASSRCLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 2);

        try {
          mSSRC = Numeric<decltype(mSSRC)>(split[0]);
        } catch (const Numeric<decltype(mSSRC)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("ssrc value out of range: " + split[0]);
        }

        size_t index = 1;
        for (; index < split.size(); ++index)
        {
          IHelper::SplitMap splitValues;
          IHelper::split(split[index], splitValues, ":");
          IHelper::splitTrim(splitValues);
          IHelper::splitPruneEmpty(splitValues);
          ORTC_THROW_INVALID_PARAMETERS_IF((splitValues.size() < 1) || (splitValues.size() > 2));

          mAttributeValues.push_back(KeyValuePair(splitValues[0], splitValues.size() > 1 ? splitValues[1] : String()));
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::ASSRCLine::toString() const
      {
        String result;
        result.append("ssrc:");
        result.append(string(mSSRC));
        for (auto iter = mAttributeValues.begin(); iter != mAttributeValues.end(); ++iter) {
          auto &keyValue = (*iter);
          result.append(" ");
          result.append(keyValue.first);
          if (keyValue.second.hasData()) {
            result.append(":");
            result.append(keyValue.second);
          }
        }
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ASSRCGroupLine::ASSRCGroupLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 2);

        mSemantics = split[0];

        for (size_t index = 1; index < split.size(); ++index) {
          String &str = split[index];

          SSRCType ssrc {};

          try {
            ssrc = Numeric<decltype(ssrc)>(str);
          } catch (const Numeric<decltype(ssrc)>::ValueOutOfRange &) {
            ORTC_THROW_INVALID_PARAMETERS("ssrc-group value out of range: " + str);
          }

          mSSRCs.push_back(ssrc);
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::ASSRCGroupLine::toString() const
      {
        String result;
        if (!mSemantics.hasData()) return result;
        if (mSSRCs.size() < 1) return result;

        result.append("ssrc-group:");
        result.append(mSemantics);
        for (auto iter = mSSRCs.begin(); iter != mSSRCs.end(); ++iter) {
          auto &ssrc = (*iter);
          result.append(" ");
          result.append(string(ssrc));
        }

        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ARIDLine::ARIDLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 3);

        mID = split[0];
        mDirection = toDirection(split[1]);
        ORTC_THROW_INVALID_PARAMETERS_IF(!isValid(mDirection, false, true, true, false));

        IHelper::SplitMap ridParamSplit;
        IHelper::split(split[2], ridParamSplit, ";");
        IHelper::splitTrim(ridParamSplit);
        IHelper::splitPruneEmpty(ridParamSplit);
        ORTC_THROW_INVALID_PARAMETERS_IF(ridParamSplit.size() < 1);

        for (auto iter = ridParamSplit.begin(); iter != ridParamSplit.end(); ++iter) {
          auto &param = (*iter).second;

          IHelper::SplitMap keyValueSplit;
          IHelper::split(param, keyValueSplit, "=");
          IHelper::splitTrim(keyValueSplit);
          IHelper::splitPruneEmpty(keyValueSplit);
          ORTC_THROW_INVALID_PARAMETERS_IF(keyValueSplit.size() < 1);

          if (keyValueSplit[0].compare("pt")) {
            // special handling
            ORTC_THROW_INVALID_PARAMETERS_IF(keyValueSplit.size() < 2);

            IHelper::SplitMap payloadTypeSplit;
            IHelper::split(keyValueSplit[1], payloadTypeSplit, ",");
            IHelper::splitTrim(payloadTypeSplit);
            IHelper::splitPruneEmpty(payloadTypeSplit);
            ORTC_THROW_INVALID_PARAMETERS_IF(payloadTypeSplit.size() < 1);
            for (auto iterPayload = payloadTypeSplit.begin(); iterPayload != payloadTypeSplit.end(); ++iterPayload) {
              auto &ptStr = (*iter).second;
              PayloadType pt {};

              try {
                pt = Numeric<decltype(pt)>(ptStr);
              } catch (const Numeric<decltype(pt)>::ValueOutOfRange &) {
                ORTC_THROW_INVALID_PARAMETERS("rid payload type value out of range: " + ptStr);
              }

              mPayloadTypes.push_back(pt);
            }
            continue;
          }

          mParams.push_back(RIDParam(keyValueSplit[0], keyValueSplit.size() > 1 ? keyValueSplit[1] : String()));
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::ARIDLine::toString() const
      {
        String result;
        if (!mID.hasData()) return result;
        result.append("rid:");
        result.append(mID);
        result.append(" ");
        switch (mDirection) {
          case ISDPTypes::Direction_None:
          case ISDPTypes::Direction_SendReceive:  return String();
          case ISDPTypes::Direction_Send:         
          case ISDPTypes::Direction_Receive:      result.append(ISDPTypes::toString(mDirection)); break;
        }
        bool inserted = false;
        if (mPayloadTypes.size() > 0) {
          result.append(" ");
          result.append("pt=");
          bool first = true;
          for (auto iter = mPayloadTypes.begin(); iter != mPayloadTypes.end(); ++iter) {
            auto &pt = (*iter);
            if (inserted) result.append(",");
            result.append(string(pt));
            inserted = true;
          }
        }
        if (mParams.size() > 0) {
          if (!inserted) {
            result.append(" ");
          }
          for (auto iter = mParams.begin(); iter != mParams.end(); ++iter) {
            auto &key = (*iter).first;
            auto &value = (*iter).second;
            if (inserted) result.append(";");
            result.append(key);
            if (value.hasData()) {
              result.append("=");
              result.append(value);
            }
            inserted = true;
          }
        }
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ASimulcastLine::ASimulcastLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        IHelper::SplitMap split;
        IHelper::split(String(value), split, " ");
        IHelper::splitTrim(split);
        IHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 2);
        ORTC_THROW_INVALID_PARAMETERS_IF(0 != (split.size()%2));

        for (size_t index = 0; index < split.size(); ++index)
        {
          SCValue scValue;
          scValue.mDirection = toDirection(split[index]);
          ORTC_THROW_INVALID_PARAMETERS_IF(!isValid(scValue.mDirection, false, true, true, false));

          IHelper::SplitMap altListSplit;
          IHelper::split(split[index+1], altListSplit, ";");
          IHelper::splitTrim(altListSplit);
          IHelper::splitPruneEmpty(altListSplit);
          ORTC_THROW_INVALID_PARAMETERS_IF(altListSplit.size() < 1);

          for (auto iter = altListSplit.begin(); iter != altListSplit.end(); ++iter) {
            auto &altValue = (*iter).second;

            SCIDList scids;
            IHelper::SplitMap scidListSplit;
            IHelper::split(altValue, scidListSplit, ",");
            IHelper::splitTrim(scidListSplit);
            IHelper::splitPruneEmpty(scidListSplit);
            ORTC_THROW_INVALID_PARAMETERS_IF(scidListSplit.size() < 1);

            for (auto iterScid = scidListSplit.begin(); iterScid != scidListSplit.end(); ++iterScid) {
              auto &scidValue = (*iterScid).second;

              SCID scid;

              if ('~' == (scidValue.c_str())[0]) {
                scid.mPaused = true;
                scid.mRID = scidValue.substr(1);
              } else {
                scid.mRID = scidValue;
              }

              scids.push_back(scid);
            }
            scValue.mAltSCIDs.push_back(scids);
          }
          mValues.push_back(scValue);
        }
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ASCTPPortLine::ASCTPPortLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        String str(value);
        try {
          mPort = Numeric<decltype(mPort)>(str);
        } catch (const Numeric<decltype(mPort)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("sctp port value out of range: " + str);
        }
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AMaxMessageSizeLine::AMaxMessageSizeLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        String str(value);
        try {
          mMaxMessageSize = Numeric<decltype(mMaxMessageSize)>(str);
        } catch (const Numeric<decltype(mMaxMessageSize)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("max message size value out of range: " + str);
        }
      }


    }  // namespace internal
  } // namespace adapter
} // namespace ortc
