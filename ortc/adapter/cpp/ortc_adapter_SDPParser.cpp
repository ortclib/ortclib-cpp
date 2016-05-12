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

#include <openpeer/services/IHelper.h>

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

    ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper);
    ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper);

    ZS_DECLARE_TYPEDEF_PTR(ortc::adapter::IHelper, UseAdapterHelper);

    typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

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
      static String createTransportIDFromIndex(size_t index)
      {
        SHA1Hasher hasher;
        hasher.update("transport_index:");
        hasher.update(index);
        return hasher.final();
      }

      //-----------------------------------------------------------------------
      static String createMediaLineIDFromIndex(size_t index)
      {
        SHA1Hasher hasher;
        hasher.update("media_line_index:");
        hasher.update(index);
        return hasher.final();
      }

      //-----------------------------------------------------------------------
      static String createSenderIDFromIndex(size_t index)
      {
        SHA1Hasher hasher;
        hasher.update("sender_index:");
        hasher.update(index);
        return hasher.final();
      }

      //-----------------------------------------------------------------------
      static String eol()
      {
        static String result("\r\n");
        return result;
      }

      //-----------------------------------------------------------------------
      static void appendLine(String &ioResult, const char *line)
      {
        if (NULL == line) return;
        if ('\0' == *line) return;
        ioResult.append(line);
        ioResult.append(eol());
      }

      //-----------------------------------------------------------------------
      static void appendLine(String &ioResult, const String &line)
      {
        if (line.isEmpty()) return;
        ioResult.append(line);
        ioResult.append(eol());
      }

      //-----------------------------------------------------------------------
      static void appendLine(String &ioResult, char letter, const char *line)
      {
        if (NULL == line) return;
        if ('\0' == *line) return;

        char prefix[3];
        prefix[0] = letter;
        prefix[1] = '=';
        prefix[2] = '\0';

        ioResult.append(&(prefix[0]));
        ioResult.append(line);
        ioResult.append(eol());
      }

      //-----------------------------------------------------------------------
      static void appendLine(String &ioResult, char letter, const String &line)
      {
        if (line.isEmpty()) return;

        char prefix[3];
        prefix[0] = letter;
        prefix[1] = '=';
        prefix[2] = '\0';

        ioResult.append(&(prefix[0]));
        ioResult.append(line);
        ioResult.append(eol());
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
        UseServicesHelper::SplitMap protoSplit;
        UseServicesHelper::split(str, protoSplit, "/");
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
          return ProtocolType_Unknown;
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitPruneEmpty(split);
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, ":");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitPruneEmpty(split);
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 3);

        mMedia = split[0];
        mProtoStr = split[2];
        mProto = toProtocolType(mProtoStr);

        // fix port
        {
          UseServicesHelper::SplitMap portSplit;
          UseServicesHelper::split(split[1], portSplit, "/");
          UseServicesHelper::splitTrim(portSplit);
          UseServicesHelper::splitPruneEmpty(portSplit);
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

        for (size_t index = 4; index < split.size(); ++index)
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
        result.append(mProtoStr);
        result.append(UseServicesHelper::combine(mFmts, " "));
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::CLine::CLine(MLinePtr mline, const char *value) :
        MediaLine(mline)
      {
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitPruneEmpty(split);
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitPruneEmpty(split);
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

        String result(mSemantic);
        result.append(UseServicesHelper::combine(mIdentificationTags, " "));
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AMSIDLine::AMSIDLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitPruneEmpty(split);
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
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
        result.append(UseServicesHelper::combine(mTags, " "));
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ACandidateLine::ACandidateLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 8);

        if (split.size() > 8) {
          ORTC_THROW_INVALID_PARAMETERS_IF(0 != split.size() % 2);
        }

        mFoundation = split[0];
        try {
          mComponentID = Numeric<decltype(mComponentID)>(split[1]);
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
          for (auto index = 10; index < split.size(); index += 2)
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 3);

        try {
          mTag = Numeric<decltype(mTag)>(split[0]);
        } catch (const Numeric<decltype(mTag)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("tag value out of range: " + split[0]);
        }
        mCryptoSuite = split[1];

        UseServicesHelper::SplitMap keyParamsSplit;
        UseServicesHelper::split(String(split[2]), keyParamsSplit, ";");
        UseServicesHelper::splitTrim(keyParamsSplit);
        UseServicesHelper::splitPruneEmpty(keyParamsSplit);
        ORTC_THROW_INVALID_PARAMETERS_IF(keyParamsSplit.size() < 1);

        for (auto iter = keyParamsSplit.begin(); iter != keyParamsSplit.end(); ++iter) {
          auto &keyParam = (*iter).second;

          UseServicesHelper::SplitMap keyMethodInfoSplit;
          UseServicesHelper::split(keyParam, keyMethodInfoSplit, ":");
          UseServicesHelper::splitTrim(keyMethodInfoSplit);
          UseServicesHelper::splitPruneEmpty(keyMethodInfoSplit);
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
          result.append(UseServicesHelper::combine(mSessionParams, " "));
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 2);

        UseServicesHelper::SplitMap entrySplit;
        UseServicesHelper::split(split[0], entrySplit, "/");
        UseServicesHelper::splitTrim(entrySplit);
        UseServicesHelper::splitPruneEmpty(entrySplit);
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(2 != split.size());

        try {
          mPayloadType = Numeric<decltype(mPayloadType)>(split[0]);
        } catch (const Numeric<decltype(mPayloadType)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("payload type value out of range: " + split[0]);
        }

        UseServicesHelper::SplitMap encodingSplit;
        UseServicesHelper::split(split[1], encodingSplit, "/");
        UseServicesHelper::splitTrim(encodingSplit);
        UseServicesHelper::splitPruneEmpty(encodingSplit);
        ORTC_THROW_INVALID_PARAMETERS_IF(encodingSplit.size() < 1);

        try {
          mClockRate = Numeric<decltype(mClockRate)>(encodingSplit[0]);
        } catch (const Numeric<decltype(mClockRate)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("clock rate value out of range: " + split[0]);
        }

        if (encodingSplit.size() > 1) {
          try {
            mEncodingParameters = Numeric<decltype(mEncodingParameters.mType)>(encodingSplit[1]);
          } catch (const Numeric<decltype(mEncodingParameters.mType)>::ValueOutOfRange &) {
            ORTC_THROW_INVALID_PARAMETERS("clock rate value out of range: " + encodingSplit[1]);
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
        result.append(string(mClockRate));
        if (mEncodingParameters.hasValue()) {
          result.append(" ");
          result.append(string(mEncodingParameters.value()));
        }
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::AFMTPLine::AFMTPLine(MLinePtr mline, ASSRCLinePtr sourceLine, const char *value) :
        AMediaLine(mline),
        mSourceLine(sourceLine)
      {
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 1);

        mFormatStr = split[0];
        if (isdigit(mFormatStr[0])) {
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
        result.append(UseServicesHelper::combine(mFormatSpecific, " "));
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ARTCPLine::ARTCPLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 2);

        try {
          mSSRC = Numeric<decltype(mSSRC)>(split[0]);
        } catch (const Numeric<decltype(mSSRC)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("ssrc value out of range: " + split[0]);
        }

        mAttribute = split[1];

        for (size_t index = 2; index < split.size(); ++index) {
          mAttributeValues.push_back(split[index]);
        }
      }

      //-----------------------------------------------------------------------
      String ISDPTypes::ASSRCLine::toString() const
      {
        String result;
        result.append("ssrc:");
        result.append(string(mSSRC));
        if (mAttribute.hasData()) {
          result.append(" ");
          result.append(mAttribute);
          if (mAttributeValues.size() > 0) {
            result.append(" ");
            result.append(UseServicesHelper::combine(mAttributeValues, " "));
          }
        }
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::ASSRCGroupLine::ASSRCGroupLine(MLinePtr mline, const char *value) :
        AMediaLine(mline)
      {
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 3);

        mID = split[0];
        mDirection = toDirection(split[1]);
        ORTC_THROW_INVALID_PARAMETERS_IF(!isValid(mDirection, false, true, true, false));

        UseServicesHelper::SplitMap ridParamSplit;
        UseServicesHelper::split(split[2], ridParamSplit, ";");
        UseServicesHelper::splitTrim(ridParamSplit);
        UseServicesHelper::splitPruneEmpty(ridParamSplit);
        ORTC_THROW_INVALID_PARAMETERS_IF(ridParamSplit.size() < 1);

        for (auto iter = ridParamSplit.begin(); iter != ridParamSplit.end(); ++iter) {
          auto &param = (*iter).second;

          UseServicesHelper::SplitMap keyValueSplit;
          UseServicesHelper::split(param, keyValueSplit, "=");
          UseServicesHelper::splitTrim(keyValueSplit);
          UseServicesHelper::splitPruneEmpty(keyValueSplit);
          ORTC_THROW_INVALID_PARAMETERS_IF(keyValueSplit.size() < 1);

          if (keyValueSplit[0].compare("pt")) {
            // special handling
            ORTC_THROW_INVALID_PARAMETERS_IF(keyValueSplit.size() < 2);

            UseServicesHelper::SplitMap payloadTypeSplit;
            UseServicesHelper::split(keyValueSplit[1], payloadTypeSplit, ",");
            UseServicesHelper::splitTrim(payloadTypeSplit);
            UseServicesHelper::splitPruneEmpty(payloadTypeSplit);
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
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 2);
        ORTC_THROW_INVALID_PARAMETERS_IF(0 != (split.size()%2));

        for (size_t index = 0; index < split.size(); ++index)
        {
          SCValue scValue;
          scValue.mDirection = toDirection(split[index]);
          ORTC_THROW_INVALID_PARAMETERS_IF(!isValid(scValue.mDirection, false, true, true, false));

          UseServicesHelper::SplitMap altListSplit;
          UseServicesHelper::split(split[index+1], altListSplit, ";");
          UseServicesHelper::splitTrim(altListSplit);
          UseServicesHelper::splitPruneEmpty(altListSplit);
          ORTC_THROW_INVALID_PARAMETERS_IF(altListSplit.size() < 1);

          for (auto iter = altListSplit.begin(); iter != altListSplit.end(); ++iter) {
            auto &altValue = (*iter).second;

            SCIDList scids;
            UseServicesHelper::SplitMap scidListSplit;
            UseServicesHelper::split(altValue, scidListSplit, ",");
            UseServicesHelper::splitTrim(scidListSplit);
            UseServicesHelper::splitPruneEmpty(scidListSplit);
            ORTC_THROW_INVALID_PARAMETERS_IF(scidListSplit.size() < 1);

            for (auto iterScid = scidListSplit.begin(); iterScid != scidListSplit.end(); ++iterScid) {
              auto &scidValue = (*iterScid).second;

              SCID scid;

              if ('~' == scidValue[0]) {
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

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SDPParser
      #pragma mark

      //-----------------------------------------------------------------------
      void SDPParser::parseLines(SDP &sdp)
      {
        char *pos = sdp.mRawBuffer.get();

        bool lastWasEOL = true;

        while ('\0' != *pos)
        {
          if (('\r' == *pos) ||
              ('\n' == *pos)) {
            *pos = '\0';
            ++pos;
            lastWasEOL = true;
            continue;
          }

          if (!lastWasEOL) {
            ++pos;
            continue;
          }

          lastWasEOL = false;

          auto lineType = toLineType(*pos);
          if (LineType_Unknown == lineType) {
            ++pos;
            continue;
          }

          LineTypeInfo info;
          info.mLineType = lineType;
          info.mValue = pos;
          sdp.mLineInfos.push_back(info);
          ++pos;
        }
      }

      //-----------------------------------------------------------------------
      void SDPParser::parseAttributes(SDP &sdp)
      {
        for (auto iter = sdp.mLineInfos.begin(); iter != sdp.mLineInfos.end(); ++iter)
        {
          auto &info = (*iter);
          if (LineType_a_Attributes != info.mLineType) continue;

          char *pos = const_cast<char *>(info.mValue);

          char *attributeName = pos;
          const char *attributeValue {};

          while ('\0' != *pos)
          {
            // when the ':' is reached the value must start
            if (':' != *pos) {
              // continue skipping while not hitting white space
              if (!isspace(*pos)) {
                ++pos;
                continue;
              }

              // value name ends at white space
              *pos = '\0';
              ++pos;

              // be friendly do bad white space infront of a ':' by skipping it....
              while ('\0' != *pos) {
                if (!isspace(*pos)) break;
                ++pos;
              }
              // if hit nul character, there is no value 
              if ('\0' == *pos) continue;

              // otherwise a ':' MUST be hit
              ORTC_THROW_INVALID_PARAMETERS_IF(':' != *pos);
              continue;
            }

            // break value at the point of the colon
            *pos = '\0';
            ++pos;

            // skip over white space before any value
            while ('\0' != *pos)
            {
              if (!isspace(*pos)) break;
              ++pos;
            }

            if ('\0' != *pos) {
              // if there was some kind of non-white space before nul
              /// character then a value must exist
              attributeValue = pos;
            }
            break;
          }

          info.mAttribute = toAttribute(attributeName);
          if (Attribute_Unknown == info.mAttribute) {
            ZS_LOG_WARNING(Trace, slog("attribute is not understood") + ZS_PARAM("attribute name", attributeName) + ZS_PARAM("attribute value", attributeValue))
            continue;
          }

          // make sure the value is either present or missing depending on the
          // attribute type
          if (info.mValue) {
            ORTC_THROW_INVALID_PARAMETERS_IF(requiresEmptyValue(info.mAttribute));
          } else {
            ORTC_THROW_INVALID_PARAMETERS_IF(requiresValue(info.mAttribute));
          }
        }
      }

      //-----------------------------------------------------------------------
      void SDPParser::validateAttributeLevels(SDP &sdp)
      {
        auto currentLevel = AttributeLevel_Session;

        for (auto iter = sdp.mLineInfos.begin(); iter != sdp.mLineInfos.end(); ++iter)
        {
          auto &info = (*iter);

          if (LineType_Unknown == info.mLineType) continue;

          if (LineType_m_MediaLine == info.mLineType) {
            // immediately switch to the session level (so that media level is legal)
            currentLevel = AttributeLevel_Session;
          }

          if (LineType_a_Attributes == info.mLineType) {
            if (Attribute_Unknown == info.mAttribute) continue; // skip any unknown attributes
          }

          if (AttributeLevel_Source == currentLevel) {

            AttributeLevels allowedLevels = AttributeLevel_None;

            if (LineType_a_Attributes == info.mLineType) {
              allowedLevels = getAttributeLevel(info.mAttribute);
            } else {
              allowedLevels = getAttributeLevel(info.mLineType);
            }

            if (0 == (AttributeLevel_Source & allowedLevels)) {
              // line/attribute is not legal at source level (but maybe switching back to a media level attribute)
              if (0 != (AttributeLevel_Media & allowedLevels)) {
                // this line/attribute is moving out of source level back to media level
                currentLevel = AttributeLevel_Media;
              }
            }
          }

          ORTC_THROW_INVALID_PARAMETERS_IF(!supportedAtLevel(currentLevel, info.mLineType));
          if (LineType_a_Attributes == info.mLineType) {
            // throw an exception if this attribute is not legal at this level
            ORTC_THROW_INVALID_PARAMETERS_IF(!supportedAtLevel(currentLevel, info.mAttribute));
          }

          info.mAttributeLevel = currentLevel;  // record the attribute's level

          // switch to new level if line/attribute causes a level switch
          switch (info.mLineType)
          {
            case LineType_Unknown:                break;

            case LineType_v_Version:              break;
            case LineType_o_Origin:               break;
            case LineType_s_SessionName:          break;
            case LineType_b_Bandwidth:            break;
            case LineType_t_Timing:               break;
            case LineType_a_Attributes:           {
              if (Attribute_SSRC == info.mAttribute) {
                currentLevel = AttributeLevel_Source;
                break;
              }
              break;
            }
            case LineType_m_MediaLine:            {
              currentLevel = AttributeLevel_Media;
              break;
            }
            case LineType_c_ConnectionDataLine:   break;
          }
        }
      }

      //-----------------------------------------------------------------------
      void SDPParser::parseLinesDetails(SDP &sdp)
      {
        AttributeLevels currentLevel = AttributeLevel_Session;

        MLinePtr currentMLine;
        ASSRCLinePtr currentSourceLine;

        for (auto iter = sdp.mLineInfos.begin(); iter != sdp.mLineInfos.end(); ++iter)
        {
          auto &info = (*iter);

          if (AttributeLevel_Session == info.mAttributeLevel) {
            currentMLine.reset();
            currentSourceLine.reset();
          }
          if (AttributeLevel_Media == info.mAttributeLevel) {
            currentSourceLine.reset();
          }

          switch (info.mLineType)
          {
            case LineType_Unknown:                continue;  // will not happen

            case LineType_v_Version:              info.mLineValue = make_shared<VLine>(info.mValue); break;
            case LineType_o_Origin:               info.mLineValue = make_shared<OLine>(info.mValue); break;
            case LineType_s_SessionName:          info.mLineValue = make_shared<SLine>(info.mValue); break;
            case LineType_b_Bandwidth:            {
              if (AttributeLevel_Session == info.mAttributeLevel) {
                // https://tools.ietf.org/html/draft-ietf-rtcweb-jsep-14
                // Section 5.2.1 - ignore b= for session level
                info.mLineType = LineType_Unknown;  // make sure this line type is ignored in the future
                continue;
              }
              info.mLineValue = make_shared<BLine>(currentMLine, info.mValue); break;
              break;
            }
            case LineType_t_Timing:               info.mLineValue = make_shared<TLine>(info.mValue); break;
            case LineType_a_Attributes:           {
              break;
            }
            case LineType_m_MediaLine:            {
              currentMLine = make_shared<MLine>(info.mValue);
              info.mLineValue = currentMLine;
              break;
            }
            case LineType_c_ConnectionDataLine:   info.mLineValue = make_shared<CLine>(currentMLine, info.mValue); break;
          }

          if (LineType_a_Attributes != info.mLineType) continue;  // special handling for attributes

          switch (info.mAttribute)
          {
            case Attribute_Unknown:           break;

            case Attribute_Group:             info.mLineValue = make_shared<AGroupLine>(info.mValue); break;
            case Attribute_BundleOnly:        info.mLineValue = make_shared<AMediaFlagLine>(currentMLine); break;
            case Attribute_MSID:              info.mLineValue = make_shared<AMSIDLine>(currentMLine, info.mValue); break;
            case Attribute_MSIDSemantic:      info.mLineValue = make_shared<AMSIDSemanticLine>(info.mValue); break;
            case Attribute_ICEUFrag:          info.mLineValue = make_shared<AICEUFragLine>(currentMLine, info.mValue); break;
            case Attribute_ICEPwd:            info.mLineValue = make_shared<AICEPwdLine>(currentMLine, info.mValue); break;
            case Attribute_ICEOptions:        info.mLineValue = make_shared<AICEOptionsLine>(info.mValue); break;
            case Attribute_ICELite:           break;  // session level flag; struct not needed
            case Attribute_Candidate:         info.mLineValue = make_shared<ACandidateLine>(currentMLine, info.mValue); break;
            case Attribute_EndOfCandidates:   info.mLineValue = make_shared<AMediaFlagLine>(currentMLine); break;
            case Attribute_Fingerprint:       info.mLineValue = make_shared<AFingerprintLine>(currentMLine, info.mValue); break;
            case Attribute_Crypto:            info.mLineValue = make_shared<ACryptoLine>(currentMLine, info.mValue); break;
            case Attribute_Setup:             info.mLineValue = make_shared<ASetupLine>(currentMLine, info.mValue); break;
            case Attribute_MID:               info.mLineValue = make_shared<AMIDLine>(currentMLine, info.mValue); break;
            case Attribute_Extmap:            info.mLineValue = make_shared<AExtmapLine>(currentMLine, info.mValue); break;
            case Attribute_SendRecv:          info.mLineValue = make_shared<AMediaDirectionLine>(currentMLine, toString(info.mAttribute)); break;
            case Attribute_SendOnly:          info.mLineValue = make_shared<AMediaDirectionLine>(currentMLine, toString(info.mAttribute)); break;
            case Attribute_RecvOnly:          info.mLineValue = make_shared<AMediaDirectionLine>(currentMLine, toString(info.mAttribute)); break;
            case Attribute_Inactive:          info.mLineValue = make_shared<AMediaDirectionLine>(currentMLine, toString(info.mAttribute)); break;
            case Attribute_RTPMap:            info.mLineValue = make_shared<ARTPMapLine>(currentMLine, info.mValue); break;
            case Attirbute_FMTP:              info.mLineValue = make_shared<AFMTPLine>(currentMLine, currentSourceLine, info.mValue); break;
            case Attribute_RTCP:              info.mLineValue = make_shared<ARTCPLine>(currentMLine, info.mValue); break;
            case Attribute_RTCPMux:           info.mLineValue = make_shared<AMediaFlagLine>(currentMLine); break;
            case Attribute_RTCPFB:            info.mLineValue = make_shared<ARTCPFBLine>(currentMLine, info.mValue); break;
            case Attribute_RTCPRSize:         info.mLineValue = make_shared<AMediaFlagLine>(currentMLine); break;
            case Attribute_PTime:             info.mLineValue = make_shared<APTimeLine>(currentMLine, info.mValue); break;
            case Attribute_MaxPTime:          info.mLineValue = make_shared<AMaxPTimeLine>(currentMLine, info.mValue); break;
            case Attribute_SSRC:              info.mLineValue = make_shared<ASSRCLine>(currentMLine, info.mValue); break;
            case Attribute_SSRCGroup:         info.mLineValue = make_shared<ASSRCGroupLine>(currentMLine, info.mValue); break;
            case Attribute_Simulcast:         info.mLineValue = make_shared<ASimulcastLine>(currentMLine, info.mValue); break;
            case Attribute_RID:               info.mLineValue = make_shared<ARIDLine>(currentMLine, info.mValue); break;
            case Attribute_SCTPPort:          info.mLineValue = make_shared<ASCTPPortLine>(currentMLine, info.mValue); break;
            case Attribute_MaxMessageSize:    info.mLineValue = make_shared<AMaxMessageSizeLine>(currentMLine, info.mValue); break;
          }
        }
      }

      //-----------------------------------------------------------------------
      void SDPParser::processFlagAttributes(SDP &sdp)
      {
        for (auto iter_doNotUse = sdp.mLineInfos.begin(); iter_doNotUse != sdp.mLineInfos.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &info = (*current);
          if (LineType_a_Attributes != info.mLineType) continue;

          if (!requiresEmptyValue(info.mAttribute)) continue;

          {
            // this is a "flag" attribute
            switch (info.mAttribute)
            {
              case Attribute_Unknown:           continue;

              case Attribute_Group:             continue;
              case Attribute_BundleOnly:        break;
              case Attribute_MSID:              continue;
              case Attribute_MSIDSemantic:      continue;
              case Attribute_ICEUFrag:          continue;
              case Attribute_ICEPwd:            continue;
              case Attribute_ICEOptions:        continue;
              case Attribute_ICELite: {
                sdp.mICELite = true;
                continue;
              }
              case Attribute_Candidate:         continue;
              case Attribute_EndOfCandidates:   break;
              case Attribute_Fingerprint:       continue;
              case Attribute_Crypto:            continue;
              case Attribute_Setup:             continue;
              case Attribute_MID:               continue;
              case Attribute_Extmap:            continue;
              case Attribute_SendRecv:          
              case Attribute_SendOnly:          
              case Attribute_RecvOnly:          
              case Attribute_Inactive: {
                auto direction = ZS_DYNAMIC_PTR_CAST(AMediaDirectionLine, info.mLineValue);
                ZS_THROW_INVALID_ASSUMPTION_IF(!direction);
                if (direction->mMLine) {
                  ORTC_THROW_INVALID_PARAMETERS_IF(direction->mMLine->mMediaDirection.hasValue());
                  direction->mMLine->mMediaDirection = direction->mDirection;
                } else {
                  ORTC_THROW_INVALID_PARAMETERS_IF(sdp.mMediaDirection.hasValue());
                  sdp.mMediaDirection = direction->mDirection;
                }
                sdp.mLineInfos.erase(current);
                goto remove_line;;
              }
              case Attribute_RTPMap:            continue;
              case Attirbute_FMTP:              continue;
              case Attribute_RTCP:              continue;
              case Attribute_RTCPMux:           break;
              case Attribute_RTCPFB:            continue;
              case Attribute_RTCPRSize:         break;
              case Attribute_PTime:             continue;
              case Attribute_MaxPTime:          continue;
              case Attribute_SSRC:              continue;
              case Attribute_SSRCGroup:         continue;
              case Attribute_Simulcast:         continue;
              case Attribute_RID:               continue;
              case Attribute_SCTPPort:          continue;
              case Attribute_MaxMessageSize:    continue;
            }

            auto flag = ZS_DYNAMIC_PTR_CAST(AMediaFlagLine, info.mLineValue);
            ZS_THROW_INVALID_ASSUMPTION_IF(!flag);

            ORTC_THROW_INVALID_PARAMETERS_IF(!flag->mMLine);

            switch (info.mAttribute)
            {
              case Attribute_BundleOnly:        flag->mMLine->mBundleOnly = true; goto remove_line;
              case Attribute_EndOfCandidates:   flag->mMLine->mEndOfCandidates = true; goto remove_line;
              case Attribute_RTCPMux:           flag->mMLine->mRTCPMux = true; break; goto remove_line;
              case Attribute_RTCPRSize:         flag->mMLine->mRTCPRSize = true; break; goto remove_line;
              default:                          break;
            }
          }
          continue;

        remove_line:
          {
            // no longer need to process this line
            sdp.mLineInfos.erase(current);
          }
        }
      }

      //-----------------------------------------------------------------------
      void SDPParser::processSessionLevelValues(SDP &sdp)
      {
        for (auto iter_doNotUse = sdp.mLineInfos.begin(); iter_doNotUse != sdp.mLineInfos.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &info = (*current);

          if (AttributeLevel_Session != info.mAttributeLevel) continue;

          switch (info.mLineType)
          {
            case LineType_Unknown:        goto remove_line;

            case LineType_v_Version: {
              ORTC_THROW_INVALID_PARAMETERS_IF((bool)sdp.mVLine);
              sdp.mVLine = ZS_DYNAMIC_PTR_CAST(VLine, info.mLineValue);
              goto remove_line;
            }
            case LineType_o_Origin: {
              ORTC_THROW_INVALID_PARAMETERS_IF((bool)sdp.mOLine);
              sdp.mOLine = ZS_DYNAMIC_PTR_CAST(OLine, info.mLineValue);
              goto remove_line;
            }
            case LineType_s_SessionName: {
              ORTC_THROW_INVALID_PARAMETERS_IF((bool)sdp.mSLine);
              sdp.mSLine = ZS_DYNAMIC_PTR_CAST(SLine, info.mLineValue);
              goto remove_line;
            }
            case LineType_b_Bandwidth:    goto remove_line;
            case LineType_t_Timing: {
              ORTC_THROW_INVALID_PARAMETERS_IF((bool)sdp.mTLine);
              sdp.mTLine = ZS_DYNAMIC_PTR_CAST(TLine, info.mLineValue);
              goto remove_line;
            }
            case LineType_a_Attributes:   break;
            case LineType_m_MediaLine: {
              sdp.mMLines.push_back(ZS_DYNAMIC_PTR_CAST(MLine, info.mLineValue));
              goto remove_line;
            }
            case LineType_c_ConnectionDataLine: {
              ORTC_THROW_INVALID_PARAMETERS_IF((bool)sdp.mCLine);
              sdp.mCLine = ZS_DYNAMIC_PTR_CAST(CLine, info.mLineValue);
              goto remove_line;
            }
          }

          switch (info.mAttribute)
          {
            case Attribute_Unknown:           goto remove_line;

            case Attribute_Group: {
              sdp.mAGroupLines.push_back(ZS_DYNAMIC_PTR_CAST(AGroupLine, info.mLineValue));
              goto remove_line;
            }
            case Attribute_BundleOnly:        continue;
            case Attribute_MSID:              continue;
            case Attribute_MSIDSemantic: {
              sdp.mAMSIDSemanticLines.push_back(ZS_DYNAMIC_PTR_CAST(AMSIDSemanticLine, info.mLineValue));
              goto remove_line;
            }
            case Attribute_ICEUFrag: {
              ORTC_THROW_INVALID_PARAMETERS_IF((bool)sdp.mAICEUFragLine);
              sdp.mAICEUFragLine = ZS_DYNAMIC_PTR_CAST(AICEUFragLine, info.mLineValue);
              goto remove_line;
            }
            case Attribute_ICEPwd: {
              ORTC_THROW_INVALID_PARAMETERS_IF((bool)sdp.mAICEPwdLine);
              sdp.mAICEPwdLine = ZS_DYNAMIC_PTR_CAST(AICEPwdLine, info.mLineValue);
              goto remove_line;
            }
            case Attribute_ICEOptions: {
              ORTC_THROW_INVALID_PARAMETERS_IF((bool)sdp.mAICEOptionsLine);
              sdp.mAICEOptionsLine = ZS_DYNAMIC_PTR_CAST(AICEOptionsLine, info.mLineValue);
              goto remove_line;
            }
            case Attribute_ICELite:           continue;
            case Attribute_Candidate:         continue;
            case Attribute_EndOfCandidates:   continue;
            case Attribute_Fingerprint: {
              sdp.mAFingerprintLines.push_back(ZS_DYNAMIC_PTR_CAST(AFingerprintLine, info.mLineValue));
              goto remove_line;
            }
            case Attribute_Crypto:            continue;
            case Attribute_Setup: {
              ORTC_THROW_INVALID_PARAMETERS_IF((bool)sdp.mASetupLine);
              sdp.mASetupLine = ZS_DYNAMIC_PTR_CAST(ASetupLine, info.mLineValue);
              goto remove_line;
            }
            case Attribute_MID:               continue;
            case Attribute_Extmap: {
              sdp.mAExtmapLines.push_back(ZS_DYNAMIC_PTR_CAST(AExtmapLine, info.mLineValue));
              goto remove_line;
            }
            case Attribute_SendRecv:          continue;
            case Attribute_SendOnly:          continue;
            case Attribute_RecvOnly:          continue;
            case Attribute_Inactive:          continue;
            case Attribute_RTPMap:            continue;
            case Attirbute_FMTP:              continue;
            case Attribute_RTCP:              continue;
            case Attribute_RTCPMux:           continue;
            case Attribute_RTCPFB:            continue;
            case Attribute_RTCPRSize:         continue;
            case Attribute_PTime:             continue;
            case Attribute_MaxPTime:          continue;
            case Attribute_SSRC:              continue;
            case Attribute_SSRCGroup:         continue;
            case Attribute_Simulcast:         continue;
            case Attribute_RID:               continue;
            case Attribute_SCTPPort:          continue;
            case Attribute_MaxMessageSize:    continue;
          }

          continue;

        remove_line:
          {
            // no longer need to process this line
            sdp.mLineInfos.erase(current);
          }
        }
      }

      //-----------------------------------------------------------------------
      void SDPParser::processMediaLevelValues(SDP &sdp)
      {
        for (auto iter_doNotUse = sdp.mLineInfos.begin(); iter_doNotUse != sdp.mLineInfos.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &info = (*current);

          if (AttributeLevel_Media != info.mAttributeLevel) continue;

          // scope: process line
          {
            switch (info.mLineType)
            {
              case LineType_Unknown:                goto remove_line;

              case LineType_v_Version:              continue;
              case LineType_o_Origin:               continue;
              case LineType_s_SessionName:          continue;
              case LineType_b_Bandwidth:            {
                auto bline = ZS_DYNAMIC_PTR_CAST(BLine, info.mLineValue);
                ORTC_THROW_INVALID_PARAMETERS_IF(!bline->mMLine);
                bline->mMLine->mBLines.push_back(bline);
                bline->mMLine.reset();  // no longer point back to mline
                goto remove_line;
              }
              case LineType_t_Timing:               continue;
              case LineType_a_Attributes:           break;
              case LineType_m_MediaLine:            continue;
              case LineType_c_ConnectionDataLine:   {
                auto cline = ZS_DYNAMIC_PTR_CAST(CLine, info.mLineValue);
                ORTC_THROW_INVALID_PARAMETERS_IF(!cline->mMLine);
                ORTC_THROW_INVALID_PARAMETERS_IF((bool)cline->mMLine->mCLine);
                cline->mMLine->mCLine = cline;
                cline->mMLine.reset();  // no longer point back to mline
                goto remove_line;
              }
            }

            auto mediaLine = ZS_DYNAMIC_PTR_CAST(AMediaLine, info.mLineValue);
            ZS_THROW_INVALID_ASSUMPTION_IF(!mediaLine);

            MLinePtr mline = mediaLine->mMLine;
            mediaLine->mMLine.reset();

            ORTC_THROW_INVALID_PARAMETERS_IF(!mline);

            switch (info.mAttribute)
            {
              case Attribute_Unknown:           break;

              case Attribute_Group:             continue;
              case Attribute_BundleOnly:        continue;
              case Attribute_MSID:              {
                mline->mAMSIDLines.push_back(ZS_DYNAMIC_PTR_CAST(AMSIDLine, info.mLineValue));
                goto remove_line;
              }
              case Attribute_MSIDSemantic:      continue;
              case Attribute_ICEUFrag:          {
                ORTC_THROW_INVALID_PARAMETERS_IF(mline->mAICEUFragLine);
                mline->mAICEUFragLine = ZS_DYNAMIC_PTR_CAST(AICEUFragLine, info.mLineValue);
                goto remove_line;
              }
              case Attribute_ICEPwd:            {
                ORTC_THROW_INVALID_PARAMETERS_IF(mline->mAICEPwdLine);
                mline->mAICEPwdLine = ZS_DYNAMIC_PTR_CAST(AICEPwdLine, info.mLineValue);
                goto remove_line;
              }
              case Attribute_ICEOptions:        continue;
              case Attribute_ICELite:           continue;
              case Attribute_Candidate:         {
                mline->mACandidateLines.push_back(ZS_DYNAMIC_PTR_CAST(ACandidateLine, info.mLineValue));
                goto remove_line;
              }
              case Attribute_EndOfCandidates:   continue;
              case Attribute_Fingerprint:       {
                mline->mAFingerprintLines.push_back(ZS_DYNAMIC_PTR_CAST(AFingerprintLine, info.mLineValue));
                goto remove_line;
              }
              case Attribute_Crypto:            {
                mline->mACryptoLines.push_back(ZS_DYNAMIC_PTR_CAST(ACryptoLine, info.mLineValue));
                goto remove_line;
              }
              case Attribute_Setup:             {
                ORTC_THROW_INVALID_PARAMETERS_IF(mline->mASetupLine);
                mline->mASetupLine = ZS_DYNAMIC_PTR_CAST(ASetupLine, info.mLineValue);
                goto remove_line;
              }
              case Attribute_MID:               {
                // NOTE: While technically a=mid is allowed multiple times, within
                // the context of the jsep draft it's only every used once and
                // mid would get confused if more than value was present thus
                // make it illegal.
                ORTC_THROW_INVALID_PARAMETERS_IF(mline->mAMIDLine);
                mline->mAMIDLine = ZS_DYNAMIC_PTR_CAST(AMIDLine, info.mLineValue);
                goto remove_line;
              }
              case Attribute_Extmap:            {
                mline->mAExtmapLines.push_back(ZS_DYNAMIC_PTR_CAST(AExtmapLine, info.mLineValue));
                goto remove_line;
              }
              case Attribute_SendRecv:          continue;
              case Attribute_SendOnly:          continue;
              case Attribute_RecvOnly:          continue;
              case Attribute_Inactive:          continue;
              case Attribute_RTPMap:            {
                mline->mARTPMapLines.push_back(ZS_DYNAMIC_PTR_CAST(ARTPMapLine, info.mLineValue));
                goto remove_line;
              }
              case Attirbute_FMTP:              {
                mline->mAFMTPLines.push_back(ZS_DYNAMIC_PTR_CAST(AFMTPLine, info.mLineValue));
                goto remove_line;
              }
              case Attribute_RTCP:              {
                ORTC_THROW_INVALID_PARAMETERS_IF(mline->mARTCPLine);
                mline->mARTCPLine = ZS_DYNAMIC_PTR_CAST(ARTCPLine, info.mLineValue);
                goto remove_line;
              }
              case Attribute_RTCPMux:           continue;
              case Attribute_RTCPFB:            {
                mline->mARTCPFBLines.push_back(ZS_DYNAMIC_PTR_CAST(ARTCPFBLine, info.mLineValue));
                goto remove_line;
              }
              case Attribute_RTCPRSize:         continue;
              case Attribute_PTime:             {
                ORTC_THROW_INVALID_PARAMETERS_IF(mline->mAPTimeLine);
                mline->mAPTimeLine = ZS_DYNAMIC_PTR_CAST(APTimeLine, info.mLineValue);
                goto remove_line;
              }
              case Attribute_MaxPTime:          {
                ORTC_THROW_INVALID_PARAMETERS_IF(mline->mAMaxPTimeLine);
                mline->mAMaxPTimeLine = ZS_DYNAMIC_PTR_CAST(AMaxPTimeLine, info.mLineValue);
                goto remove_line;
              }
              case Attribute_SSRC:              {
                mline->mASSRCLines.push_back(ZS_DYNAMIC_PTR_CAST(ASSRCLine, info.mLineValue));
                goto remove_line;
              }
              case Attribute_SSRCGroup:         {
                mline->mASSRCGroupLines.push_back(ZS_DYNAMIC_PTR_CAST(ASSRCGroupLine, info.mLineValue));
                goto remove_line;
              }
              case Attribute_Simulcast:         {
                ORTC_THROW_INVALID_PARAMETERS_IF(mline->mASimulcastLine);
                mline->mASimulcastLine = ZS_DYNAMIC_PTR_CAST(ASimulcastLine, info.mLineValue);
                goto remove_line;
              }
              case Attribute_RID:               {
                mline->mARIDLines.push_back(ZS_DYNAMIC_PTR_CAST(ARIDLine, info.mLineValue));
                goto remove_line;
              }
              case Attribute_SCTPPort:          {
                ORTC_THROW_INVALID_PARAMETERS_IF(mline->mASCTPPortLine);
                mline->mASCTPPortLine = ZS_DYNAMIC_PTR_CAST(ASCTPPortLine, info.mLineValue);
                goto remove_line;
              }
              case Attribute_MaxMessageSize: {
                ORTC_THROW_INVALID_PARAMETERS_IF(mline->mAMaxMessageSize);
                mline->mAMaxMessageSize = ZS_DYNAMIC_PTR_CAST(AMaxMessageSizeLine, info.mLineValue);
                goto remove_line;
              }
            }
          }
          continue;

        remove_line:
          {
            // no longer need to process this line
            sdp.mLineInfos.erase(current);
          }
        }
      }

      //-----------------------------------------------------------------------
      void SDPParser::processSourceLevelValues(SDP &sdp)
      {
        for (auto iter_doNotUse = sdp.mLineInfos.begin(); iter_doNotUse != sdp.mLineInfos.end(); )
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto &info = (*current);

          if (AttributeLevel_Source != info.mAttributeLevel) continue;

          // scope: process line
          {
            switch (info.mLineType)
            {
              case LineType_Unknown:                goto remove_line;

              case LineType_v_Version:              continue;
              case LineType_o_Origin:               continue;
              case LineType_s_SessionName:          continue;
              case LineType_b_Bandwidth:            continue;
              case LineType_t_Timing:               continue;
              case LineType_a_Attributes:           break;
              case LineType_m_MediaLine:            continue;
              case LineType_c_ConnectionDataLine:   continue;
            }

            switch (info.mAttribute)
            {
              case Attribute_Unknown:           break;

              case Attribute_Group:             continue;
              case Attribute_BundleOnly:        continue;
              case Attribute_MSID:              continue;
              case Attribute_MSIDSemantic:      continue;
              case Attribute_ICEUFrag:          continue;
              case Attribute_ICEPwd:            continue;
              case Attribute_ICEOptions:        continue;
              case Attribute_ICELite:           continue;
              case Attribute_Candidate:         continue;
              case Attribute_EndOfCandidates:   continue;
              case Attribute_Fingerprint:       continue;
              case Attribute_Crypto:            continue;
              case Attribute_Setup:             continue;
              case Attribute_MID:               continue;
              case Attribute_Extmap:            continue;
              case Attribute_SendRecv:          continue;
              case Attribute_SendOnly:          continue;
              case Attribute_RecvOnly:          continue;
              case Attribute_Inactive:          continue;
              case Attribute_RTPMap:            continue;
              case Attirbute_FMTP:              {
                auto fmtpLine = ZS_DYNAMIC_PTR_CAST(AFMTPLine, info.mLineValue);
                ORTC_THROW_INVALID_PARAMETERS_IF(!fmtpLine->mMLine);
                ORTC_THROW_INVALID_PARAMETERS_IF(!fmtpLine->mSourceLine);
                fmtpLine->mSourceLine->mAFMTPLines.push_back(fmtpLine);

                // no longer point back to mline or source line
                fmtpLine->mMLine.reset();
                fmtpLine->mSourceLine.reset();
                goto remove_line;
              }
              case Attribute_RTCP:              continue;
              case Attribute_RTCPMux:           continue;
              case Attribute_RTCPFB:            continue;
              case Attribute_RTCPRSize:         continue;
              case Attribute_PTime:             continue;
              case Attribute_MaxPTime:          continue;
              case Attribute_SSRC:              continue;
              case Attribute_SSRCGroup:         continue;
              case Attribute_Simulcast:         continue;
              case Attribute_RID:               continue;
              case Attribute_SCTPPort:          continue;
              case Attribute_MaxMessageSize:    continue;
            }
          }

          continue;

        remove_line:
          {
            // no longer need to process this line
            sdp.mLineInfos.erase(current);
          }
        }
      }

      //-----------------------------------------------------------------------
      void SDPParser::createDescriptionDetails(
        const SDP &sdp,
        Description &ioDescription
        )
      {
        bool needsDetails = ((bool)sdp.mOLine) ||
          ((bool)sdp.mSLine) ||
          ((bool)sdp.mTLine) ||
          ((bool)sdp.mCLine) ||
          ((bool)sdp.mCLine);

        if (!needsDetails) return;

        auto details = make_shared<Description::Details>();
        ioDescription.mDetails = details;

        details->mUsername = ((bool)sdp.mOLine) ? sdp.mOLine->mUsername : String();
        details->mSessionID = ((bool)sdp.mOLine) ? sdp.mOLine->mSessionID : 0;
        details->mSessionVersion = ((bool)sdp.mOLine) ? sdp.mOLine->mSessionVersion : 0;
        details->mSessionName = ((bool)sdp.mSLine) ? sdp.mSLine->mSessionName : String();
        details->mStartTime = ((bool)sdp.mTLine) ? sdp.mTLine->mStartTime : 0;
        details->mEndTime = ((bool)sdp.mTLine) ? sdp.mTLine->mEndTime : 0;

        bool needUnicastAddress = ((bool)sdp.mOLine);
        if (needUnicastAddress) {
          auto unicast = make_shared<ISessionDescriptionTypes::ConnectionData::Details>();

          unicast->mNetType = ((bool)sdp.mOLine) ? sdp.mOLine->mNetType : String();
          unicast->mAddrType = ((bool)sdp.mOLine) ? sdp.mOLine->mAddrType : String();
          unicast->mConnectionAddress = ((bool)sdp.mOLine) ? sdp.mOLine->mUnicastAddress : String();

          details->mUnicaseAddress = unicast;
        }

        bool needConnectionData = ((bool)sdp.mCLine);
        if (needConnectionData) {
          auto connectionData = make_shared<ISessionDescriptionTypes::ConnectionData>();
          connectionData->mRTP = make_shared<ISessionDescriptionTypes::ConnectionData::Details>();

          connectionData->mRTP->mNetType = ((bool)sdp.mCLine) ? sdp.mCLine->mNetType : String();
          connectionData->mRTP->mAddrType = ((bool)sdp.mCLine) ? sdp.mCLine->mAddrType : String();
          connectionData->mRTP->mConnectionAddress = ((bool)sdp.mCLine) ? sdp.mCLine->mConnectionAddress : String();

          details->mConnectionData = connectionData;
        }
      }

      //-----------------------------------------------------------------------
      static IDTLSTransportTypes::Roles toDtlsRole(const char *setup)
      {
        String str(setup);

        static const char *checkSetup[] =
        {
          "active",
          "passive",
          "actpass",
          "holdconn"
        };
        static IDTLSTransportTypes::Roles resultRole[] =
        {
          IDTLSTransportTypes::Role_Client,
          IDTLSTransportTypes::Role_Server,
          IDTLSTransportTypes::Role_Auto,
          IDTLSTransportTypes::Role_Auto
        };

        for (size_t index = 0; index <= (sizeof(resultRole) / sizeof(resultRole[0])); ++index) {
          if (0 == str.compareNoCase(checkSetup[index])) return resultRole[index];
        }
        return IDTLSTransportTypes::Role_Auto;
      }

      //-----------------------------------------------------------------------
      static void convertCrypto(
                                const ISDPTypes::ACryptoLineList &inCryptoLines,
                                ISRTPSDESTransportTypes::Parameters &outCrypto
                                )
      {
        for (auto iter = inCryptoLines.begin(); iter != inCryptoLines.end(); ++iter) {
          auto &acrypto = *(*iter);

          ISRTPSDESTransportTypes::CryptoParameters cryptoParams;
          cryptoParams.mTag = SafeInt<decltype(cryptoParams.mTag)>(acrypto.mTag);
          cryptoParams.mCryptoSuite = acrypto.mCryptoSuite;

          for (auto iterKeyParms = acrypto.mKeyParams.begin(); iterKeyParms != acrypto.mKeyParams.end(); ++iterKeyParms) {
            auto &akeyParam = (*iterKeyParms);
            ISRTPSDESTransportTypes::KeyParameters keyParams;
            keyParams.mKeyMethod = akeyParam.first;

            UseServicesHelper::SplitMap keyInfoSplit;
            UseServicesHelper::split(akeyParam.second, keyInfoSplit, "|");
            UseServicesHelper::splitTrim(keyInfoSplit);
            UseServicesHelper::splitPruneEmpty(keyInfoSplit);
            ORTC_THROW_INVALID_PARAMETERS_IF(keyInfoSplit.size() < 1);

            keyParams.mKeySalt = keyInfoSplit[0];
            if (keyInfoSplit.size() > 1) {
              keyParams.mLifetime = keyInfoSplit[1];
            }
            if (keyInfoSplit.size() > 2) {
              UseServicesHelper::SplitMap mkiSplit;
              UseServicesHelper::split(keyInfoSplit[2], mkiSplit, "|");
              UseServicesHelper::splitTrim(mkiSplit);
              UseServicesHelper::splitPruneEmpty(mkiSplit);
              ORTC_THROW_INVALID_PARAMETERS_IF(mkiSplit.size() < 2);

              keyParams.mMKIValue = mkiSplit[0];
              try {
                keyParams.mMKILength = Numeric<decltype(keyParams.mMKILength)>(mkiSplit[1]);
              }
              catch (const Numeric<decltype(keyParams.mMKILength)>::ValueOutOfRange &) {
                ORTC_THROW_INVALID_PARAMETERS("mki length value is out of range: " + mkiSplit[1]);
              }
            }

            cryptoParams.mKeyParams.push_back(keyParams);
          }

          for (auto iterSessionParms = acrypto.mSessionParams.begin(); iterSessionParms != acrypto.mSessionParams.end(); ++iterSessionParms) {
            auto &asessionParam = (*iterSessionParms);
            cryptoParams.mSessionParams.push_back(asessionParam);
          }

          outCrypto.mCryptoParams.push_back(cryptoParams);
        }
      }

      //-----------------------------------------------------------------------
      static void convertDTLSFingerprints(
                                          const ISDPTypes::AFingerprintLineList &inFingerprintLines,
                                          IDTLSTransportTypes::Parameters &outParameters
                                          )
      {

        for (auto iter = inFingerprintLines.begin(); iter != inFingerprintLines.end(); ++iter) {
          auto &afingerprint = *(*iter);
          ICertificateTypes::Fingerprint certFingerprint;
          certFingerprint.mAlgorithm = afingerprint.mHashFunc;
          certFingerprint.mValue = afingerprint.mFingerprint;
          outParameters.mFingerprints.push_back(certFingerprint);
        }
      }

      //-----------------------------------------------------------------------
      static IICETypes::CandidatePtr convertCandidate(const ISDPTypes::ACandidateLine &acandidate)
      {
        auto candidate = make_shared<IICETypes::Candidate>();

        candidate->mFoundation = acandidate.mFoundation;
        candidate->mPriority = acandidate.mPriority;
        candidate->mProtocol = IICETypes::toProtocol(acandidate.mTransport);
        candidate->mIP = acandidate.mConnectionAddress;
        candidate->mPort = acandidate.mPort;
        candidate->mCandidateType = IICETypes::toCandidateType(acandidate.mCandidateType);
        candidate->mRelatedAddress = acandidate.mRelAddr;
        candidate->mRelatedPort = acandidate.mRelPort;

        for (auto iterExt = acandidate.mExtensionPairs.begin(); iterExt != acandidate.mExtensionPairs.end(); ++iterExt) {
          auto &extName = (*iterExt).first;
          auto &extValue = (*iterExt).second;

          if (0 == extName.compareNoCase("tcptype")) {
            candidate->mTCPType = IICETypes::toTCPCandidateType(extValue);
            continue;
          }
          if (0 == extName.compareNoCase("unfreezepriority")) {
            try {
              candidate->mUnfreezePriority = Numeric<decltype(candidate->mUnfreezePriority)>(extValue);
            }
            catch (const Numeric<decltype(candidate->mUnfreezePriority)>::ValueOutOfRange &) {
              ORTC_THROW_INVALID_PARAMETERS("unfreeze priority is out of range: " + extValue);
            }
            continue;
          }
          if (0 == extName.compareNoCase("interfacetype")) {
            candidate->mInterfaceType = extValue;
            continue;
          }
        }

        return candidate;
      }

      //-----------------------------------------------------------------------
      static void convertCandidates(
                                    const ISDPTypes::ACandidateLineList &inCandidateLines,
                                    ISessionDescriptionTypes::ICECandidateList &outRTPCandidates,
                                    ISessionDescriptionTypes::Transport::ParametersPtr &ioRTCPTransport
                                    )
      {
        for (auto iter = inCandidateLines.begin(); iter != inCandidateLines.end(); ++iter)
        {
          auto &acandidate = *(*iter);
          auto candidate = convertCandidate(acandidate);

          if (0 == acandidate.mComponentID) {
            outRTPCandidates.push_back(candidate);
          } else {
            if (!ioRTCPTransport) {
              ioRTCPTransport = make_shared<ISessionDescriptionTypes::Transport::Parameters>();
            }
            ioRTCPTransport->mICECandidates.push_back(candidate);
          }
        }
      }

      //-----------------------------------------------------------------------
      void SDPParser::createTransports(
                                       const SDP &sdp,
                                       Description &ioDescription
                                       )
      {
        size_t index = 0;

        for (auto iter = sdp.mMLines.begin(); iter != sdp.mMLines.end(); ++iter, ++index)
        {
          auto &mline = *(*iter);

          if (!mline.mAICEUFragLine) continue;  // no ice transport = no transport

          auto transport = make_shared<ISessionDescriptionTypes::Transport>();
          transport->mRTP = make_shared<ISessionDescriptionTypes::Transport::Parameters>();

          bool requireRTCP = (mline.mRTCPMux.hasValue() ? (!mline.mRTCPMux.value()) : true);
          if (requireRTCP) {
            transport->mRTCP = make_shared<ISessionDescriptionTypes::Transport::Parameters>();
          }

          transport->mRTP->mICEParameters = make_shared<IICETypes::Parameters>();
          transport->mRTP->mICEParameters->mUsernameFragment = ((bool)mline.mAICEUFragLine) ? mline.mAICEUFragLine->mICEUFrag : String();
          transport->mRTP->mICEParameters->mPassword = ((bool)mline.mAICEPwdLine) ? mline.mAICEPwdLine->mICEPwd : String();
          transport->mRTP->mICEParameters->mICELite = sdp.mICELite.hasValue() ? sdp.mICELite.value() : false;

          if (mline.mAFingerprintLines.size() > 0) {
            transport->mRTP->mDTLSParameters = make_shared<IDTLSTransportTypes::Parameters>();
            transport->mRTP->mDTLSParameters->mRole = ((bool)mline.mASetupLine) ? toDtlsRole(mline.mASetupLine->mSetup) : IDTLSTransportTypes::Role_Auto;
            convertDTLSFingerprints(mline.mAFingerprintLines, *(transport->mRTP->mDTLSParameters));
          }

          if (mline.mACryptoLines.size() > 0) {
            transport->mRTP->mSRTPSDESParameters = make_shared<ISRTPSDESTransportTypes::Parameters>();
            convertCrypto(mline.mACryptoLines, *transport->mRTP->mSRTPSDESParameters);
          }

          convertCandidates(mline.mACandidateLines, transport->mRTP->mICECandidates, transport->mRTCP);

          if ((mline.mEndOfCandidates.hasValue()) &&
              (mline.mEndOfCandidates.value())) {
            transport->mRTP->mEndOfCandidates = true;
            if (transport->mRTCP) {
              transport->mRTCP->mEndOfCandidates = true;
            }
          }

          if (mline.mAMIDLine) {
            transport->mID = mline.mAMIDLine->mMID;
          } else {
            transport->mID = createTransportIDFromIndex(index);
          }
        }
      }

      //-----------------------------------------------------------------------
      static ISessionDescriptionTypes::MediaDirections toDirection(ISDPTypes::Directions direction)
      {
        switch (direction)
        {
          case ISDPTypes::Direction_None: return ISessionDescriptionTypes::MediaDirection_Inactive;
          case ISDPTypes::Direction_Send: return ISessionDescriptionTypes::MediaDirection_SendOnly;
          case ISDPTypes::Direction_Receive: return ISessionDescriptionTypes::MediaDirection_ReceiveOnly;
          case ISDPTypes::Direction_SendReceive: return ISessionDescriptionTypes::MediaDirection_SendReceive;
        }
        ORTC_THROW_NOT_SUPPORTED_ERRROR("sdp direction was not handled");
      }

      //-----------------------------------------------------------------------
      static void fillMediaLine(
                                size_t index,
                                const ISDPTypes::SDP &sdp,
                                const ISDPTypes::MLine &mline,
                                ISessionDescriptionTypes::Description &description,
                                ISessionDescriptionTypes::MediaLine &mediaLine
                                )
      {
        mediaLine.mDetails = make_shared<ISessionDescriptionTypes::MediaLine::Details>();
        mediaLine.mDetails->mInternalIndex = index;
        mediaLine.mDetails->mProtocol = mline.mProtoStr;

        mediaLine.mDetails->mConnectionData = make_shared<ISessionDescriptionTypes::ConnectionData>();
        mediaLine.mDetails->mConnectionData->mRTP = make_shared<ISessionDescriptionTypes::ConnectionData::Details>();
        mediaLine.mDetails->mConnectionData->mRTP->mPort = SafeInt<decltype(mediaLine.mDetails->mConnectionData->mRTP->mPort.mType)>(mline.mPort);

        if (mline.mCLine) {
          mediaLine.mDetails->mConnectionData->mRTP->mNetType = mline.mCLine->mNetType;
          mediaLine.mDetails->mConnectionData->mRTP->mAddrType = mline.mCLine->mAddrType;
          mediaLine.mDetails->mConnectionData->mRTP->mConnectionAddress = mline.mCLine->mConnectionAddress;
        }
        if (mline.mARTCPLine) {
          mediaLine.mDetails->mConnectionData->mRTCP = make_shared<ISessionDescriptionTypes::ConnectionData::Details>();
          mediaLine.mDetails->mConnectionData->mRTCP->mNetType = mline.mARTCPLine->mNetType;
          mediaLine.mDetails->mConnectionData->mRTCP->mAddrType = mline.mARTCPLine->mAddrType;
          mediaLine.mDetails->mConnectionData->mRTCP->mConnectionAddress = mline.mARTCPLine->mConnectionAddress;
          mediaLine.mDetails->mConnectionData->mRTCP->mPort = mline.mARTCPLine->mPort;
        }

        mediaLine.mMediaType = mline.mMedia;
        if (mline.mMediaDirection.hasValue()) {
          mediaLine.mDetails->mMediaDirection = toDirection(mline.mMediaDirection.value());
        } else if (sdp.mMediaDirection.hasValue()) {
          mediaLine.mDetails->mMediaDirection = toDirection(sdp.mMediaDirection.value());
        }

        String foundBundleID;
        String searchForTransportID;

        if (mline.mAMIDLine) {
          mediaLine.mID = mline.mAMIDLine->mMID;
          searchForTransportID = mediaLine.mID;

          // figure out if this transport is part of a bundle
          for (auto iter = sdp.mAGroupLines.begin(); iter != sdp.mAGroupLines.end(); ++iter)
          {
            auto &group = *(*iter);

            String firstBundleID;

            if (0 != group.mSemantic.compareNoCase("BUNDLE")) continue;
            for (auto iterID = group.mIdentificationTags.begin(); iterID != group.mIdentificationTags.end(); ++iterID)
            {
              auto &mid = (*iterID);
              if (firstBundleID.isEmpty()) firstBundleID = mid;
              if (mid != mediaLine.mID) continue;
              foundBundleID = firstBundleID;
              break;
            }

            if (foundBundleID.hasData()) break;
          }
        } else {
          mediaLine.mID = createMediaLineIDFromIndex(index);
          searchForTransportID = createTransportIDFromIndex(index);
        }

        for (auto iter = description.mTransports.begin(); iter != description.mTransports.end(); ++iter) {
          auto &transport = *(*iter);
          if (transport.mID != searchForTransportID) continue;
          mediaLine.mDetails->mPrivateTransportID = searchForTransportID;
        }

        if (foundBundleID.hasData()) {
          bool found = false;
          for (auto iter = description.mTransports.begin(); iter != description.mTransports.end(); ++iter) {
            auto &transport = *(*iter);
            if (transport.mID != foundBundleID) continue;
            mediaLine.mTransportID = foundBundleID;
            found = true;
          }
          if (!found) foundBundleID.clear();
        }

        if (foundBundleID.isEmpty()) {
          mediaLine.mTransportID = mediaLine.mDetails->mPrivateTransportID;
        }

        if (mediaLine.mTransportID == mediaLine.mDetails->mPrivateTransportID) {
          mediaLine.mDetails->mPrivateTransportID.clear();
        }
      }

      //-----------------------------------------------------------------------
      static void fixIntoCodecSpecificList(
                                           const ISDPTypes::StringList &formatSpecificList,
                                           ISDPTypes::KeyValueList &outKeyValues
                                           )
      {
        String params = UseServicesHelper::combine(formatSpecificList, ";");

        UseServicesHelper::SplitMap formatSplit;
        UseServicesHelper::split(params, formatSplit, ";");
        UseServicesHelper::splitTrim(formatSplit);
        UseServicesHelper::splitPruneEmpty(formatSplit);

        for (auto iter = formatSplit.begin(); iter != formatSplit.end(); ++iter) {
          auto &keyValue = (*iter).second;

          UseServicesHelper::SplitMap keyValueSplit;
          UseServicesHelper::split(params, keyValueSplit, "=");
          UseServicesHelper::splitTrim(keyValueSplit);
          UseServicesHelper::splitPruneEmpty(keyValueSplit);
          ORTC_THROW_INVALID_PARAMETERS_IF(keyValueSplit.size() < 1);

          String key = keyValueSplit[0];
          String value = (keyValueSplit.size() > 1 ? keyValueSplit[1] : String());

          outKeyValues.push_back(ISDPTypes::KeyValuePair(key, value));
        }
      }

      //-----------------------------------------------------------------------
      void fillCodecFormatSpecific(
                                   IRTPTypes::SupportedCodecs supportedCodec,
                                   IRTPTypes::CodecCapability &codecCapability,
                                   ISDPTypes::AFMTPLine &format
                                   )
      {
        switch (supportedCodec) {
          case IRTPTypes::SupportedCodec_Unknown:  return;

          // audio codecs
          case IRTPTypes::SupportedCodec_Opus:            {
            ISDPTypes::KeyValueList keyValues;
            fixIntoCodecSpecificList(format.mFormatSpecific, keyValues);

            auto opusParams = make_shared<IRTPTypes::OpusCodecCapabilityParameters>();

            for (auto iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
              auto &key = (*iter).first;
              auto &value = (*iter).second;

              if (0 == key.compareNoCase("maxplaybackrate")) {
                try {
                  opusParams->mMaxPlaybackRate = Numeric<decltype(opusParams->mMaxPlaybackRate.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mMaxPlaybackRate.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus max playback rate is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("sprop-maxcapturerate")) {
                try {
                  opusParams->mSPropMaxCaptureRate = Numeric<decltype(opusParams->mSPropMaxCaptureRate.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mSPropMaxCaptureRate.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus sprop max capture rate is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("maxptime")) {
                try {
                  codecCapability.mMaxPTime = Milliseconds(Numeric<Milliseconds::rep>(value));
                } catch (const Numeric<Milliseconds::rep>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max ptime is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("ptime")) {
                try {
                  codecCapability.mPTime = Milliseconds(Numeric<Milliseconds::rep>(value));
                } catch (const Numeric<Milliseconds::rep>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("ptime is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("maxaveragebitrate")) {
                try {
                  opusParams->mMaxAverageBitrate = Numeric<decltype(opusParams->mMaxAverageBitrate.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mMaxAverageBitrate.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus max average bitrate is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("stereo")) {
                try {
                  opusParams->mStereo = Numeric<decltype(opusParams->mStereo.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mStereo.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus stero value is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("sprop-stereo")) {
                try {
                  opusParams->mSPropStereo = Numeric<decltype(opusParams->mSPropStereo.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mSPropStereo.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus sprop-stereo value is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("cbr")) {
                try {
                  opusParams->mCBR = Numeric<decltype(opusParams->mCBR.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mCBR.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus cbr value is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("useinbandfec")) {
                try {
                  opusParams->mUseInbandFEC = Numeric<decltype(opusParams->mUseInbandFEC.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mUseInbandFEC.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus use inband fec value is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("usedtx")) {
                try {
                  opusParams->mUseDTX = Numeric<decltype(opusParams->mUseDTX.mType)>(value);
                } catch (const Numeric<decltype(opusParams->mUseDTX.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("opus use inband fec value is not valid: " + value);
                }
              }
            }
            codecCapability.mParameters = opusParams;
            break;
          }
          case IRTPTypes::SupportedCodec_Isac:            return;
          case IRTPTypes::SupportedCodec_G722:            return;
          case IRTPTypes::SupportedCodec_ILBC:            return;
          case IRTPTypes::SupportedCodec_PCMU:            return;
          case IRTPTypes::SupportedCodec_PCMA:            return;

          // video codecs
          case IRTPTypes::SupportedCodec_VP8:             {
            ISDPTypes::KeyValueList keyValues;
            fixIntoCodecSpecificList(format.mFormatSpecific, keyValues);

            auto vp8Params = make_shared<IRTPTypes::VP8CodecCapabilityParameters>();
            for (auto iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
              auto &key = (*iter).first;
              auto &value = (*iter).second;

              if (0 == key.compareNoCase("max-fr")) {
                try {
                  vp8Params->mMaxFR = Numeric<decltype(vp8Params->mMaxFR.mType)>(value);
                } catch (const Numeric<decltype(vp8Params->mMaxFR.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("vp8 max fr is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-fs")) {
                try {
                  vp8Params->mMaxFS = Numeric<decltype(vp8Params->mMaxFS.mType)>(value);
                } catch (const Numeric<decltype(vp8Params->mMaxFS.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("vp8 max fs is not valid: " + value);
                }
              }
            }

            codecCapability.mParameters = vp8Params;
            break;
          }
          case IRTPTypes::SupportedCodec_VP9:             return;
          case IRTPTypes::SupportedCodec_H264:            {
            ISDPTypes::KeyValueList keyValues;
            fixIntoCodecSpecificList(format.mFormatSpecific, keyValues);

            auto h264Params = make_shared<IRTPTypes::H264CodecCapabilityParameters>();
            for (auto iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
              auto &key = (*iter).first;
              auto &value = (*iter).second;

              if (0 == key.compareNoCase("packetization-mode")) {
                try {
                  h264Params->mPacketizationModes.push_back(Numeric<IRTPTypes::H264CodecCapabilityParameters::PacketizationModeList::value_type>(value));
                } catch (const Numeric<IRTPTypes::H264CodecCapabilityParameters::PacketizationModeList::value_type>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("h264 packetization mode is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("profile-level-id")) {
                try {
                  h264Params->mProfileLevelID = Numeric<decltype(h264Params->mProfileLevelID.mType)>(value, true, 16);
                } catch (const Numeric<decltype(h264Params->mProfileLevelID.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("profile level id is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-mbps")) {
                try {
                  h264Params->mMaxMBPS = Numeric<decltype(h264Params->mMaxMBPS.mType)>(value);
                } catch (const Numeric<decltype(h264Params->mMaxMBPS.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max mbps is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-smbps")) {
                try {
                  h264Params->mMaxSMBPS = Numeric<decltype(h264Params->mMaxSMBPS.mType)>(value);
                } catch (const Numeric<decltype(h264Params->mMaxSMBPS.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max smbps is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-fs")) {
                try {
                  h264Params->mMaxFS = Numeric<decltype(h264Params->mMaxFS.mType)>(value);
                } catch (const Numeric<decltype(h264Params->mMaxFS.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max fs is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-cpb")) {
                try {
                  h264Params->mMaxCPB = Numeric<decltype(h264Params->mMaxCPB.mType)>(value);
                } catch (const Numeric<decltype(h264Params->mMaxCPB.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max cpb is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-dpb")) {
                try {
                  h264Params->mMaxDPB = Numeric<decltype(h264Params->mMaxDPB.mType)>(value);
                } catch (const Numeric<decltype(h264Params->mMaxDPB.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max dpb is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("max-br")) {
                try {
                  h264Params->mMaxBR = Numeric<decltype(h264Params->mMaxBR.mType)>(value);
                } catch (const Numeric<decltype(h264Params->mMaxBR.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("max br is not valid: " + value);
                }
              }
            }

            codecCapability.mParameters = h264Params;
            break;
          }

          // RTX
          case IRTPTypes::SupportedCodec_RTX:             {
            ISDPTypes::KeyValueList keyValues;
            fixIntoCodecSpecificList(format.mFormatSpecific, keyValues);

            bool foundApt = false;

            auto rtxParams = make_shared<IRTPTypes::RTXCodecCapabilityParameters>();
            for (auto iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
              auto &key = (*iter).first;
              auto &value = (*iter).second;

              if (0 == key.compareNoCase("apt")) {
                try {
                  rtxParams->mApt = Numeric<decltype(rtxParams->mApt)>(value);
                  foundApt = true;
                } catch (const Numeric<decltype(rtxParams->mApt)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("rtx apt is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("rtx-time")) {
                try {
                  rtxParams->mRTXTime = Milliseconds(Numeric<Milliseconds::rep>(value));
                } catch (const Numeric<Milliseconds::rep>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("rtx time is not valid: " + value);
                }
              }
            }

            ORTC_THROW_INVALID_PARAMETERS_IF(!foundApt);

            codecCapability.mParameters = rtxParams;
            break;
          }

          // FEC
          case IRTPTypes::SupportedCodec_RED:             return;
          case IRTPTypes::SupportedCodec_ULPFEC:          return;
          case IRTPTypes::SupportedCodec_FlexFEC:         {
            ISDPTypes::KeyValueList keyValues;
            fixIntoCodecSpecificList(format.mFormatSpecific, keyValues);

            bool foundRepairWindow = false;

            auto flexFECParams = make_shared<IRTPTypes::FlexFECCodecCapabilityParameters>();
            for (auto iter = keyValues.begin(); iter != keyValues.end(); ++iter) {
              auto &key = (*iter).first;
              auto &value = (*iter).second;

              if (0 == key.compareNoCase("ToP")) {
                try {
                  ULONG top = Numeric<ULONG>(value);
                  flexFECParams->mToP = IRTPTypes::FlexFECCodecCapabilityParameters::ToPs(top);
                } catch (const Numeric<ULONG>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("flexfec ToP fr is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("L")) {
                try {
                  flexFECParams->mL = Numeric<decltype(flexFECParams->mL.mType)>(value);
                } catch (const Numeric<decltype(flexFECParams->mL.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("flexfec L is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("D")) {
                try {
                  flexFECParams->mD = Numeric<decltype(flexFECParams->mD.mType)>(value);
                } catch (const Numeric<decltype(flexFECParams->mD.mType)>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("flexfec D is not valid: " + value);
                }
              } else if (0 == key.compareNoCase("repair-window")) {
                try {
                  flexFECParams->mRepairWindow = Microseconds(Numeric<Microseconds::rep>(value));
                  foundRepairWindow = true;
                } catch (const Numeric<Microseconds::rep>::ValueOutOfRange &) {
                  ORTC_THROW_INVALID_PARAMETERS("flexfec D is not valid: " + value);
                }
              }
            }

            ORTC_THROW_INVALID_PARAMETERS_IF(!foundRepairWindow);

            codecCapability.mParameters = flexFECParams;
            break;
          }

          case IRTPTypes::SupportedCodec_CN:              return;

          case IRTPTypes::SupportedCodec_TelephoneEvent:  return;
        }
      }

      //-----------------------------------------------------------------------
      bool fillCapabilities(
                            ISDPTypes::Locations location,
                            const ISDPTypes::SDP &sdp,
                            const ISDPTypes::MLine &mline,
                            ISessionDescriptionTypes::Description &description,
                            ISessionDescriptionTypes::MediaLine &mediaLine,
                            IRTPTypes::Capabilities &senderCapabilities,
                            IRTPTypes::Capabilities &receiverCapabilities
                            )
      {
        auto matchCodecKind = IRTPTypes::toCodecKind(mline.mMedia);
        if ((IRTPTypes::CodecKind_Audio != matchCodecKind) &&
            (IRTPTypes::CodecKind_Video != matchCodecKind)) {
          ZS_LOG_WARNING(Debug, internal::slog("unable to understand media kind") + ZS_PARAM("media", mline.mMedia));
          return false;
        }

        Milliseconds ptime {};
        Milliseconds maxPTime{};
        if (mline.mAPTimeLine) {
          ptime = mline.mAPTimeLine->mPTime;
        }
        if (mline.mAMaxPTimeLine) {
          maxPTime = mline.mAMaxPTimeLine->mMaxPTime;
        }

        bool foundULPFEC = false;
        bool foundRED = false;
        bool foundFlexFEC = false;

        for (auto iter = mline.mFmts.begin(); iter != mline.mFmts.end(); ++iter)
        {
          auto &payloadTypeStr = (*iter);

          ISDPTypes::PayloadType pt {};

          try {
            pt = Numeric<decltype(pt)>(payloadTypeStr);
          } catch (const Numeric<decltype(pt)>::ValueOutOfRange &) {
            ORTC_THROW_INVALID_PARAMETERS("media line payload type is not understood");
          }

          ISDPTypes::ARTPMapLinePtr rtpMap;
          ISDPTypes::AFMTPLinePtr format;

          for (auto iterRTPMaps = mline.mARTPMapLines.begin(); iterRTPMaps != mline.mARTPMapLines.end(); ++iterRTPMaps) {
            auto checkRTPMap = (*iterRTPMaps);
            if (checkRTPMap->mPayloadType != pt) continue;

            rtpMap = checkRTPMap;
            break;
          }

          for (auto iterFMTP = mline.mAFMTPLines.begin(); iterFMTP != mline.mAFMTPLines.end(); ++iterFMTP) {
            auto checkFMTP = (*iterFMTP);
            if (checkFMTP->mFormat != pt) continue;

            format = checkFMTP;
            break;
          }

          if (!rtpMap) {
            auto reservedType = IRTPTypes::toReservedCodec(pt);
            auto supportedType = IRTPTypes::toSupportedCodec(reservedType);

            if (IRTPTypes::SupportedCodec_Unknown == supportedType) {
              ZS_LOG_WARNING(Debug, internal::slog("codec payload type is not understood") + ZS_PARAM("payload type", pt));
              continue;
            }

            rtpMap = make_shared<ISDPTypes::ARTPMapLine>(Noop{});
            rtpMap->mPayloadType = pt;
            rtpMap->mEncodingName = IRTPTypes::toString(supportedType);
            rtpMap->mClockRate = IRTPTypes::getDefaultClockRate(reservedType);
          }

          auto supportedType = IRTPTypes::toSupportedCodec(rtpMap->mEncodingName);
          if (IRTPTypes::SupportedCodec_Unknown == supportedType) {
            ZS_LOG_WARNING(Debug, internal::slog("codec payload type is not supported") + ZS_PARAM("codec name", supportedType));
            continue;
          }

          bool fillKind = false;
          auto codecKind = IRTPTypes::getCodecKind(supportedType);
          if (IRTPTypes::CodecKind_Audio == codecKind) {
            ORTC_THROW_INVALID_PARAMETERS_IF(IRTPTypes::CodecKind_Video == matchCodecKind);
            fillKind = true;
          } else if (IRTPTypes::CodecKind_Video == codecKind) {
            ORTC_THROW_INVALID_PARAMETERS_IF(IRTPTypes::CodecKind_Audio == matchCodecKind);
            fillKind = true;
          }

          auto codecCapability = make_shared<IRTPTypes::CodecCapability>();
          codecCapability->mName = rtpMap->mEncodingName;
          codecCapability->mKind = (fillKind ? String(IRTPTypes::toString(codecKind)) : String());
          codecCapability->mClockRate = rtpMap->mClockRate;
          codecCapability->mPreferredPayloadType = pt;
          codecCapability->mPTime = ptime;
          if (IRTPTypes::CodecKind_Audio == codecKind) {
            codecCapability->mNumChannels = rtpMap->mEncodingParameters;
          }
          if (format) {
            fillCodecFormatSpecific(supportedType, *codecCapability, *format);
          }
          if (IRTPTypes::requiresCapabilityParameters(supportedType)) {
            ORTC_THROW_INVALID_PARAMETERS_IF(!codecCapability->mParameters);
          }

          switch (supportedType) {
            case IRTPTypes::SupportedCodec_RED:     foundRED = true; break;
            case IRTPTypes::SupportedCodec_ULPFEC:  foundULPFEC = true; break;
            case IRTPTypes::SupportedCodec_FlexFEC: foundFlexFEC = true; break;
            default: break;
          }

          for (auto iterFB = mline.mARTCPFBLines.begin(); iterFB != mline.mARTCPFBLines.end(); ++iterFB) {
            auto &fb = *(*iterFB);

            if (fb.mPayloadType.hasValue()) {
              if (fb.mPayloadType.value() != codecCapability->mPreferredPayloadType) continue;
            }
            IRTPTypes::RTCPFeedback fbInfo;
            fbInfo.mType = fb.mID;
            fbInfo.mParameter = fb.mParam1;
            codecCapability->mRTCPFeedback.push_back(fbInfo);
          }

          senderCapabilities.mCodecs.push_back(*codecCapability);
          receiverCapabilities.mCodecs.push_back(*codecCapability);
        }

        if (foundRED) {
          if (foundULPFEC) {
            senderCapabilities.mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC));
            receiverCapabilities.mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED_ULPFEC));
          } else {
            senderCapabilities.mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED));
            receiverCapabilities.mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_RED));
          }
        }
        if (foundFlexFEC) {
          senderCapabilities.mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_FlexFEC));
          receiverCapabilities.mFECMechanisms.push_back(IRTPTypes::toString(IRTPTypes::KnownFECMechanism_FlexFEC));
        }

        for (auto iter = mline.mAExtmapLines.begin(); iter != mline.mAExtmapLines.end(); ++iter) {
          auto &extmap = *(*iter);

          IRTPTypes::HeaderExtension ext;
          ext.mKind = IRTPTypes::toString(matchCodecKind);
          ext.mPreferredEncrypt = false;
          ext.mPreferredID = SafeInt<decltype(ext.mPreferredID)>(extmap.mID);
          ext.mURI = extmap.mURI;

          // remote sender goes to sender capabilities
          if (ISDPTypes::isApplicable(ISDPTypes::ActorRole_Sender, ISDPTypes::Location_Local, extmap.mDirection)) {
            senderCapabilities.mHeaderExtensions.push_back(ext);
          }
          // remote receiver goes to receiver capabilities
          if (ISDPTypes::isApplicable(ISDPTypes::ActorRole_Receiver, ISDPTypes::Location_Local, extmap.mDirection)) {
            receiverCapabilities.mHeaderExtensions.push_back(ext);
          }
        }

        return true;
      }

      //-----------------------------------------------------------------------
      void SDPParser::createRTPMediaLines(
                                          Locations location,
                                          const SDP &sdp,
                                          Description &ioDescription
                                          )
      {
        size_t index = 0;

        for (auto iter = sdp.mMLines.begin(); iter != sdp.mMLines.end(); ++iter, ++index)
        {
          auto &mline = *(*iter);

          if (ProtocolType_RTP != mline.mProto) continue;

          auto mediaLine = make_shared<ISessionDescriptionTypes::RTPMediaLine>();
          fillMediaLine(index, sdp, mline, ioDescription, *mediaLine);

          if (mediaLine->mTransportID.isEmpty()) {
            ZS_LOG_WARNING(Debug, internal::slog("could not match RTP media line to a transport (thus ignoring mline)") + mediaLine->toDebug());
            continue;
          }

          mediaLine->mSenderCapabilities = make_shared<IRTPTypes::Capabilities>();
          mediaLine->mReceiverCapabilities = make_shared<IRTPTypes::Capabilities>();

          fillCapabilities(location, sdp, mline, ioDescription, *mediaLine, *mediaLine->mSenderCapabilities, *mediaLine->mReceiverCapabilities);

          ioDescription.mRTPMediaLines.push_back(mediaLine);
        }
      }

      //-----------------------------------------------------------------------
      void SDPParser::createSCTPMediaLines(
                                           Locations location,
                                           const SDP &sdp,
                                           Description &ioDescription
                                           )
      {
        size_t index = 0;

        for (auto iter = sdp.mMLines.begin(); iter != sdp.mMLines.end(); ++iter, ++index)
        {
          auto &mline = *(*iter);

          if (ProtocolType_SCTP != mline.mProto) continue;

          auto mediaLine = make_shared<ISessionDescriptionTypes::SCTPMediaLine>();
          fillMediaLine(index, sdp, mline, ioDescription, *mediaLine);

          if (mediaLine->mTransportID.isEmpty()) {
            ZS_LOG_WARNING(Debug, internal::slog("could not match SCTP media line to a transport (thus ignoring mline)") + mediaLine->toDebug());
            continue;
          }

          mediaLine->mCapabilities = make_shared<ISCTPTransportTypes::Capabilities>();

          if (mline.mASCTPPortLine) {
            mediaLine->mPort = SafeInt<decltype(mediaLine->mPort.mType)>(mline.mASCTPPortLine->mPort);
          }

          mediaLine->mCapabilities->mMaxMessageSize = SafeInt<decltype(mediaLine->mCapabilities->mMaxMessageSize)>(mline.mAMaxMessageSize ? mline.mAMaxMessageSize->mMaxMessageSize : 0xFFFF);

          ioDescription.mSCTPMediaLines.push_back(mediaLine);
        }
      }

      //-----------------------------------------------------------------------
      static void fillREDFormatParameters(
                                          const ISDPTypes::MLine &mline,
                                          IRTPTypes::Parameters &parameters
                                          )
      {
        String redStr(IRTPTypes::toString(IRTPTypes::SupportedCodec_RED));
        for (auto iterCodec = parameters.mCodecs.begin(); iterCodec != parameters.mCodecs.end(); ++iterCodec) {
          auto &codec = (*iterCodec);
          if (0 != codec.mName.compareNoCase(redStr)) continue;

          IRTPTypes::REDCodecParametersPtr redParameters;

          auto redPayloadType = codec.mPayloadType;
          for (auto iterFormats = mline.mAFMTPLines.begin(); iterFormats != mline.mAFMTPLines.end(); ++iterFormats) {
            auto &format = *(*iterFormats);
            if (format.mFormat != redPayloadType) continue;
            String combinedREDFormat = UseServicesHelper::combine(format.mFormatSpecific, "/");
            UseServicesHelper::SplitMap redSplit;
            UseServicesHelper::split(combinedREDFormat, redSplit, "/");
            UseServicesHelper::splitTrim(redSplit);
            UseServicesHelper::splitPruneEmpty(redSplit);
            for (auto iterRED = redSplit.begin(); iterRED != redSplit.end(); ++iterRED) {
              auto &redFormatPayloadTypeStr = (*iterRED).second;
              try {
                IRTPTypes::PayloadType redFormatPayloadType = Numeric<IRTPTypes::PayloadType>(redFormatPayloadTypeStr);
                if (nullptr == redParameters) redParameters = make_shared<IRTPTypes::REDCodecParameters>();
                redParameters->mPayloadTypes.push_back(redFormatPayloadType);
              } catch (const Numeric<ISDPTypes::PayloadType>::ValueOutOfRange &) {
                ORTC_THROW_INVALID_PARAMETERS("RED payload specific format is not valid: " + combinedREDFormat);
              }
            }
            break;
          }

          codec.mParameters = redParameters;
        }
      }

      //-----------------------------------------------------------------------
      static void fillStreamIDs(
                                const ISDPTypes::SDP &sdp,
                                const ISDPTypes::MLine &mline,
                                ISessionDescriptionTypes::RTPSender &sender
                                )
      {
        // scope: first check for MSID lines
        {
          for (auto iter = mline.mAMSIDLines.begin(); iter != mline.mAMSIDLines.end(); ++iter) {
            auto msidLine = *(*iter);
            if (sender.mMediaStreamTrackID.isEmpty()) {
              sender.mMediaStreamTrackID = msidLine.mAppData;
            }
            if (msidLine.mID.hasData()) {
              sender.mMediaStreamIDs.insert(msidLine.mID);
            }
          }

          if (sender.mMediaStreamIDs.size() > 0) return;
        }

        // scope: check the a=ssrc lines
        {
          for (auto iter = mline.mASSRCLines.begin(); iter != mline.mASSRCLines.end(); ++iter) {
            auto &ssrcLine = *(*iter);
            if (0 != ssrcLine.mAttribute.compareNoCase("msid")) continue;

            if (ssrcLine.mAFMTPLines.size() < 2) continue;

            auto &id = ssrcLine.mAttributeValues.front();
            auto &appData = *(++(ssrcLine.mAttributeValues.begin()));

            if (sender.mMediaStreamTrackID.isEmpty()) {
              sender.mMediaStreamTrackID = appData;
            }
            if (id.hasData()) {
              sender.mMediaStreamIDs.insert(id);
            }
          }
        }

      }

      //-----------------------------------------------------------------------
      void SDPParser::createRTPSenderLines(
                                           Locations location,
                                           const SDP &sdp,
                                           Description &ioDescription
                                           )
      {
        size_t index = 0;

        for (auto iter = sdp.mMLines.begin(); iter != sdp.mMLines.end(); ++iter, ++index)
        {
          auto &mline = *(*iter);

          if (ProtocolType_RTP != mline.mProto) continue;

          if (mline.mMediaDirection.hasValue()) {
            if (!isApplicable(ActorRole_Sender, Location_Local, mline.mMediaDirection.value())) continue;
          }

          auto sender = make_shared<ISessionDescriptionTypes::RTPSender>();
          sender->mDetails = make_shared<ISessionDescription::RTPSender::Details>();
          sender->mDetails->mInternalRTPMediaLineIndex = index;

          String mid = (mline.mAMIDLine ? mline.mAMIDLine->mMID : String());

          sender->mID = mid.isEmpty() ? createSenderIDFromIndex(index) : mid;
          sender->mRTPMediaLineID = mid.isEmpty() ? createMediaLineIDFromIndex(index) : mid;

          ISessionDescriptionTypes::RTPMediaLinePtr foundMediaLine;

          for (auto iterMedia = ioDescription.mRTPMediaLines.begin(); iterMedia != ioDescription.mRTPMediaLines.end(); ++iterMedia) {
            auto &mediaLine = (*iterMedia);

            if (mediaLine->mID == sender->mRTPMediaLineID) continue;
            foundMediaLine = mediaLine;
            break;
          }

          if (!foundMediaLine) {
            ZS_LOG_WARNING(Debug, internal::slog("did not find associated media line") + ZS_PARAM("media line id", sender->mRTPMediaLineID));
            continue;
          }

          sender->mParameters = UseAdapterHelper::capabilitiesToParameters(*foundMediaLine->mSenderCapabilities);

          sender->mParameters->mMuxID = mid;
          sender->mParameters->mRTCP.mMux = (mline.mRTCPMux.hasValue() ? mline.mRTCPMux.value() : false);
          sender->mParameters->mRTCP.mReducedSize = (mline.mRTCPRSize.hasValue() ? mline.mRTCPRSize.value() : false);
          fillREDFormatParameters(mline, *sender->mParameters);
          fillStreamIDs(sdp, mline, *sender);

#define TODO_FIX_FOR_SIMULCAST_SDP_WHEN_SPECIFICATION_IS_MORE_SETTLED 1
#define TODO_FIX_FOR_SIMULCAST_SDP_WHEN_SPECIFICATION_IS_MORE_SETTLED 2

          IRTPTypes::EncodingParameters encoding;

          for (auto iterSSRC = mline.mASSRCLines.begin(); iterSSRC != mline.mASSRCLines.end(); ++iterSSRC) {
            auto &ssrc = *(*iterSSRC);
            if (0 != ssrc.mAttribute.compareNoCase("cname")) continue;
            ORTC_THROW_INVALID_PARAMETERS_IF(ssrc.mAttributeValues.size() < 1);
            sender->mParameters->mRTCP.mCName = ssrc.mAttributeValues.front();
            encoding.mSSRC = ssrc.mSSRC;
            break;
          }

          for (auto iterSSRC = mline.mASSRCGroupLines.begin(); iterSSRC != mline.mASSRCGroupLines.end(); ++iterSSRC) {
            auto &ssrc = *(*iterSSRC);
            if (0 != ssrc.mSemantics.compareNoCase("FID")) continue;
            ORTC_THROW_INVALID_PARAMETERS_IF(ssrc.mSSRCs.size() < 2);
            if (!encoding.mSSRC.hasValue()) encoding.mSSRC = ssrc.mSSRCs.front();
            encoding.mRTX = IRTPTypes::RTXParameters();
            encoding.mRTX.value().mSSRC = (*(++(ssrc.mSSRCs.begin())));
            break;
          }

          for (auto iterSSRC = mline.mASSRCGroupLines.begin(); iterSSRC != mline.mASSRCGroupLines.end(); ++iterSSRC) {
            auto &ssrc = *(*iterSSRC);
            if (0 != ssrc.mSemantics.compareNoCase("FEC-FR")) continue;
            ORTC_THROW_INVALID_PARAMETERS_IF(ssrc.mSSRCs.size() < 2);
            if (!encoding.mSSRC.hasValue()) encoding.mSSRC = ssrc.mSSRCs.front();
            encoding.mFEC = IRTPTypes::FECParameters();
            encoding.mFEC.value().mSSRC = (*(++(ssrc.mSSRCs.begin())));

            auto &mechanisms = foundMediaLine->mSenderCapabilities->mFECMechanisms;
            if (mechanisms.size() > 0) {
              encoding.mFEC.value().mMechanism = mechanisms.front();
            }
            break;
          }

          sender->mParameters->mEncodings.push_back(encoding);

          ioDescription.mRTPSenders.push_back(sender);
        }
      }


      //-----------------------------------------------------------------------
      ISDPTypes::SDPPtr SDPParser::parse(const char *blob)
      {
        if (!blob) return SDPPtr();

        SDPPtr sdp(make_shared<SDP>());

        sdp->mOriginal = String(blob);
        std::unique_ptr<char[]> rawBuffer(new char[sdp->mOriginal.length()]);
        sdp->mRawBuffer = std::move(rawBuffer);

        memcpy(sdp->mRawBuffer.get(), blob, sdp->mOriginal.length());

        try {
          parseLines(*sdp);
          parseAttributes(*sdp);
          validateAttributeLevels(*sdp);
          parseLinesDetails(*sdp);
          processFlagAttributes(*sdp);
          processSessionLevelValues(*sdp);
          processMediaLevelValues(*sdp);
          processSourceLevelValues(*sdp);
        } catch (const SafeIntException &e) {
          ORTC_THROW_INVALID_PARAMETERS("value found out of legal value range" + string(e.m_code));
        }

        return sdp;
      }

      //-----------------------------------------------------------------------
      void appendToBundle(
        ISDPTypes::BundleMap &ioBundle,
        const ISessionDescription::MediaLine &mediaLine
        )
      {
        if (mediaLine.mID.isEmpty()) return;
        if (mediaLine.mTransportID.isEmpty()) return;

        auto found = ioBundle.find(mediaLine.mTransportID);
        if (found == ioBundle.end()) {
          ISDPTypes::BundledSet bundles;
          bundles.insert(mediaLine.mID);
          ioBundle[mediaLine.mID] = bundles;
          return;
        }

        auto &bundleSet = (*found).second;
        bundleSet.insert(mediaLine.mID);
      }

      //-----------------------------------------------------------------------
      void SDPParser::createSDPSessionLevel(
                                            const GeneratorOptions &options,
                                            const Description &description,
                                            SDP &ioSDP
                                            )
      {
        auto &result = ioSDP;
        result.mOLine = make_shared<OLine>(Noop{});
        result.mSLine = make_shared<SLine>(Noop{});
        result.mTLine = make_shared<TLine>(Noop{});

        if (description.mDetails) {
          result.mOLine->mUsername = description.mDetails->mUsername;
          result.mOLine->mSessionID = description.mDetails->mSessionID;
          result.mOLine->mSessionVersion = description.mDetails->mSessionVersion;
          if (description.mDetails->mConnectionData) {
            if (description.mDetails->mConnectionData->mRTP) {
              result.mOLine->mNetType = description.mDetails->mConnectionData->mRTP->mNetType;
              result.mOLine->mAddrType = description.mDetails->mConnectionData->mRTP->mAddrType;
              result.mOLine->mUnicastAddress = description.mDetails->mConnectionData->mRTP->mConnectionAddress;
            }
          }
          result.mSLine->mSessionName = description.mDetails->mSessionName;
        }

        ISDPTypes::BundleMap bundles;

        for (auto iter = description.mRTPMediaLines.begin(); iter != description.mRTPMediaLines.end(); ++iter) {
          auto &mediaLine = *(*iter);
          appendToBundle(bundles, mediaLine);
        }
        for (auto iter = description.mSCTPMediaLines.begin(); iter != description.mSCTPMediaLines.end(); ++iter) {
          auto &mediaLine = *(*iter);
          appendToBundle(bundles, mediaLine);
        }

        for (auto iter = bundles.begin(); iter != bundles.end(); ++iter) {
          auto &bundleID = (*iter).first;
          auto &bundleSet = (*iter).second;

          if (bundleSet.size() < 2) continue;

          auto groupLine = make_shared<AGroupLine>(Noop{});
          groupLine->mSemantic = "BUNDLE";

          for (auto iterSet = bundleSet.begin(); iterSet != bundleSet.end(); ++iterSet) {
            auto &mid = (*iterSet);
            groupLine->mIdentificationTags.push_back(mid);
          }

          result.mAGroupLines.push_back(groupLine);
        }
      }

      //-----------------------------------------------------------------------
      static void createSDPMediaLineBase(
                                         const ISessionDescriptionTypes::Description &description,
                                         const ISessionDescriptionTypes::MediaLine &mediaLine,
                                         ISDPTypes::SDP &ioSDP,
                                         ISDPTypes::MLine &ioMLine
                                         )
      {
        auto &result = ioSDP;
        auto &mline = ioMLine;

        ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails);

        mline.mMedia = mediaLine.mMediaType;
        mline.mPort = 9;
        mline.mProtoStr = mediaLine.mDetails->mProtocol;
        mline.mProto = ISDPTypes::toProtocolType(mline.mProtoStr);

        // figure out the port based upon the associated transport's ICE candidates
        for (auto iter = description.mTransports.begin(); iter != description.mTransports.end(); ++iter) {
          auto &transport = *(*iter);
          if (0 != transport.mID.compareNoCase(mediaLine.mTransportID)) continue;

          if (!transport.mRTP) break;
          if (transport.mRTP->mICECandidates.size() < 1) break;

          for (auto iterCandidate = transport.mRTP->mICECandidates.begin(); iterCandidate != transport.mRTP->mICECandidates.end(); ++iterCandidate) {
            auto &candidate = *(*iterCandidate);
            if (IICETypes::CandidateType_Host != candidate.mCandidateType) continue;
            mline.mPort = candidate.mPort;
          }

          if (9 != mline.mPort) break;
        }
      }

      //-----------------------------------------------------------------------
      static void createSDPRTPMediaLine(
                                        const ISessionDescriptionTypes::Description &description,
                                        const ISessionDescriptionTypes::RTPMediaLine &mediaLine,
                                        ISDPTypes::SDP &ioSDP,
                                        ISDPTypes::MLine &ioMLine
                                        )
      {
        auto &result = ioSDP;
        auto &mline = ioMLine;

        createSDPMediaLineBase(description, mediaLine, ioSDP, ioMLine);

        ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mReceiverCapabilities);
        ORTC_THROW_INVALID_PARAMETERS_IF(mediaLine.mReceiverCapabilities->mCodecs.size() < 1);

        auto &codecs = mediaLine.mReceiverCapabilities->mCodecs;
        for (auto iter = codecs.begin(); iter != codecs.end(); ++iter) {
          auto &codec = (*iter);
          mline.mFmts.push_back(string(codec.mPreferredPayloadType));
        }
      }

      //-----------------------------------------------------------------------
      static void createSDPSCTPMediaLine(
                                         const ISessionDescriptionTypes::Description &description,
                                         const ISessionDescriptionTypes::SCTPMediaLine &mediaLine,
                                         ISDPTypes::SDP &ioSDP,
                                         ISDPTypes::MLine &ioMLine
                                         )
      {
        auto &result = ioSDP;
        auto &mline = ioMLine;

        createSDPMediaLineBase(description, mediaLine, ioSDP, ioMLine);

        mline.mFmts.push_back("webrtc-datachannel");
      }

      //-----------------------------------------------------------------------
      static void figureOutBundle(
                                  const ISessionDescriptionTypes::Description &description,
                                  const ISessionDescriptionTypes::MediaLine &mediaLine,
                                  ISDPTypes::SDP &ioSDP,
                                  ISessionDescriptionTypes::TransportPtr &outAnnounceTransport
                                  )
      {
        ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails);

        String searchTransport = mediaLine.mDetails->mPrivateTransportID;
        if (searchTransport.isEmpty()) {
          searchTransport = mediaLine.mTransportID;
        }

        for (auto iter = description.mTransports.begin(); iter != description.mTransports.end(); ++iter) {
          auto transport = (*iter);
          if (transport->mID != mediaLine.mDetails->mPrivateTransportID) continue;
          outAnnounceTransport = transport;
        }

        if (mediaLine.mDetails->mPrivateTransportID.hasData()) return;

        ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails->mInternalIndex.hasValue());

        auto compareValue = mediaLine.mDetails->mInternalIndex.value();

        for (auto iter = description.mRTPMediaLines.begin(); iter != description.mRTPMediaLines.end(); ++iter) {
          auto &checkMediaLine = *(*iter);
          if (!checkMediaLine.mDetails) continue;
          if (!checkMediaLine.mDetails->mInternalIndex.hasValue()) continue;
          if (checkMediaLine.mDetails->mInternalIndex.value() < compareValue) {
            // no need to announce this transport
            outAnnounceTransport.reset();
            return;
          }
        }

        for (auto iter = description.mSCTPMediaLines.begin(); iter != description.mSCTPMediaLines.end(); ++iter) {
          auto &checkMediaLine = *(*iter);
          if (!checkMediaLine.mDetails) continue;
          if (!checkMediaLine.mDetails->mInternalIndex.hasValue()) continue;
          if (checkMediaLine.mDetails->mInternalIndex.value() < compareValue) {
            // no need to announce this transport
            outAnnounceTransport.reset();
            return;
          }
        }
      }

      //-----------------------------------------------------------------------
      static ISDPTypes::ACandidateLinePtr fillCandidate(
                                                        const IICETypes::Candidate &candidate,
                                                        WORD component
                                                        )
      {
        auto aCandidate = make_shared<ISDPTypes::ACandidateLine>(Noop{});
        aCandidate->mFoundation = candidate.mFoundation;
        aCandidate->mComponentID = component;
        aCandidate->mTransport = IICETypes::toString(candidate.mProtocol);
        aCandidate->mPriority = candidate.mPriority;
        aCandidate->mConnectionAddress = candidate.mIP;
        aCandidate->mPort = candidate.mPort;
        aCandidate->mCandidateType = IICETypes::toString(candidate.mCandidateType);
        aCandidate->mRelAddr = candidate.mRelatedAddress;
        aCandidate->mRelPort = candidate.mRelatedPort;

        if (IICETypes::Protocol_TCP == candidate.mProtocol) {
          aCandidate->mExtensionPairs.push_back(ISDPTypes::ACandidateLine::ExtensionPair("tcptype", IICETypes::toString(candidate.mTCPType)));
        }

        if (0 != candidate.mUnfreezePriority) {
          aCandidate->mExtensionPairs.push_back(ISDPTypes::ACandidateLine::ExtensionPair("unfreezepriority", string(candidate.mUnfreezePriority)));
        }
        if (candidate.mInterfaceType.hasData()) {
          aCandidate->mExtensionPairs.push_back(ISDPTypes::ACandidateLine::ExtensionPair("interfacetype", candidate.mInterfaceType));
        }
        return aCandidate;
      }

      //-----------------------------------------------------------------------
      static void fillCandidates(
                                 const ISessionDescriptionTypes::MediaLine &mediaLine,
                                 const ISessionDescriptionTypes::Transport &transport,
                                 ISDPTypes::MLine &ioMLine,
                                 const ISessionDescriptionTypes::ICECandidateList &candidates,
                                 WORD component
                                 )
      {
        auto &mline = ioMLine;

        for (auto iter = candidates.begin(); iter != candidates.end(); ++iter) {
          auto &candidate = *(*iter);
          mline.mACandidateLines.push_back(fillCandidate(candidate, component));
        }
      }

      //-----------------------------------------------------------------------
      static void fillCandidates(
                                 const ISessionDescriptionTypes::MediaLine &mediaLine,
                                 const ISessionDescriptionTypes::Transport &transport,
                                 ISDPTypes::MLine &ioMLine
                                 )
      {
        auto &mline = ioMLine;

        // figure out if the end of candidates flag needs to be set
        if (transport.mRTP) {
          if (transport.mRTCP) {
            if ((transport.mRTP->mEndOfCandidates) &&
                (transport.mRTCP->mEndOfCandidates)) {
              mline.mEndOfCandidates = true;
            }
          } else {
            if (transport.mRTP->mEndOfCandidates) {
              mline.mEndOfCandidates = true;
            }
          }
        }

        // fill in candidates for RTP/RTCP
        if (transport.mRTP) fillCandidates(mediaLine, transport, mline, transport.mRTP->mICECandidates, 0);
        if (transport.mRTCP) fillCandidates(mediaLine, transport, mline, transport.mRTCP->mICECandidates, 1);
      }

      //-----------------------------------------------------------------------
      void fillMediaLine(
                         const SDPParser::GeneratorOptions &options,
                         const ISessionDescriptionTypes::Description &description,
                         const ISessionDescriptionTypes::MediaLine &mediaLine,
                         ISDPTypes::SDP &ioSDP,
                         ISDPTypes::MLine &ioMLine
                         )
      {
        auto &result = ioSDP;
        auto &mline = ioMLine;

        ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails);

        ISessionDescriptionTypes::TransportPtr transport;
        figureOutBundle(description, mediaLine, result, transport);

        if (transport) {
          fillCandidates(mediaLine, *transport, mline);
        } else {
          if (options.mBundleOnly) {
            mline.mBundleOnly = true;
          }
        }

        mline.mMediaDirection = ISDPTypes::toDirection(ISessionDescription::toString(mediaLine.mDetails->mMediaDirection));
        mline.mRTCPMux = true;
        mline.mRTCPRSize = true;
        mline.mAMIDLine = make_shared<ISDPTypes::AMIDLine>(Noop{});
        mline.mAMIDLine->mMID = mediaLine.mID;
        mline.mCLine = make_shared<ISDPTypes::CLine>(Noop{});
        mline.mCLine->mNetType = "IN";
        mline.mCLine->mAddrType = "IP4";
        mline.mCLine->mConnectionAddress = "0.0.0.0";
        mline.mARTCPLine = make_shared<ISDPTypes::ARTCPLine>(Noop{});
        mline.mARTCPLine->mPort = 0;
        mline.mARTCPLine->mNetType = "IN";
        mline.mARTCPLine->mAddrType = "IP4";
        mline.mARTCPLine->mConnectionAddress = "0.0.0.0";

        // select port for RTCP based on RTCP candidate (if applicable)
        if (transport) {
          if (transport->mRTP) {
            mline.mAICEUFragLine = make_shared<ISDPTypes::AICEUFragLine>(Noop{});
            mline.mAICEPwdLine = make_shared<ISDPTypes::AICEPwdLine>(Noop{});
            ORTC_THROW_INVALID_PARAMETERS_IF(!transport->mRTP->mICEParameters);
            mline.mAICEUFragLine->mICEUFrag = transport->mRTP->mICEParameters->mUsernameFragment;
            mline.mAICEPwdLine->mICEPwd = transport->mRTP->mICEParameters->mPassword;
            if (transport->mRTP->mICEParameters->mICELite) result.mICELite = true;

            if (transport->mRTP->mDTLSParameters) {
              for (auto iter = transport->mRTP->mDTLSParameters->mFingerprints.begin(); iter != transport->mRTP->mDTLSParameters->mFingerprints.end(); ++iter) {
                auto &fingerprint = (*iter);
                auto fingerprintLine = make_shared<ISDPTypes::AFingerprintLine>(Noop{});
                fingerprintLine->mHashFunc = fingerprint.mAlgorithm;
                fingerprintLine->mFingerprint = fingerprint.mValue;
                mline.mAFingerprintLines.push_back(fingerprintLine);
              }
              mline.mASetupLine = make_shared<ISDPTypes::ASetupLine>(Noop{});
              auto role = transport->mRTP->mDTLSParameters->mRole;
              switch (role) {
                case IDTLSTransportTypes::Role_Auto: mline.mASetupLine->mSetup = "actpass";
                case IDTLSTransportTypes::Role_Client: mline.mASetupLine->mSetup = "active";
                case IDTLSTransportTypes::Role_Server: mline.mASetupLine->mSetup = "passive";
              }
            }
            if (transport->mRTP->mSRTPSDESParameters) {
              for (auto iter = transport->mRTP->mSRTPSDESParameters->mCryptoParams.begin(); iter != transport->mRTP->mSRTPSDESParameters->mCryptoParams.end(); ++iter) {
                auto &crypto = (*iter);
                auto cryptoLine = make_shared<ISDPTypes::ACryptoLine>(Noop{});
                cryptoLine->mTag = crypto.mTag;
                cryptoLine->mCryptoSuite = crypto.mCryptoSuite;
                for (auto iterKeyParam = crypto.mKeyParams.begin(); iterKeyParam != crypto.mKeyParams.end(); ++iterKeyParam) {
                  auto &keyParam = (*iterKeyParam);

                  UseServicesHelper::SplitMap splitsMKI;
                  splitsMKI[0] = keyParam.mMKIValue;
                  splitsMKI[1] = string(keyParam.mMKILength);
                  UseServicesHelper::splitPruneEmpty(splitsMKI);

                  UseServicesHelper::SplitMap splits;
                  splits[0] = keyParam.mKeySalt;
                  splits[1] = keyParam.mLifetime;
                  splits[2] = UseServicesHelper::combine(splitsMKI, ":");
                  UseServicesHelper::splitPruneEmpty(splits);

                  cryptoLine->mKeyParams.push_back(ISDPTypes::KeyValuePair(keyParam.mKeyMethod, UseServicesHelper::combine(splits,"|")));
                }
                cryptoLine->mSessionParams = crypto.mSessionParams;
                mline.mACryptoLines.push_back(cryptoLine);
              }
            }
          }

          if (transport->mRTCP) {
            for (auto iter = transport->mRTCP->mICECandidates.begin(); iter != transport->mRTCP->mICECandidates.end(); ++iter) {
              auto &candidate = *(*iter);
              if (IICETypes::CandidateType_Host != candidate.mCandidateType) continue;
              mline.mARTCPLine->mPort = candidate.mPort;
              break;
            }
          }
        }
      }

      //-----------------------------------------------------------------------
      static void fillFormat(
                             IRTPTypes::OpusCodecCapabilityParameters &source,
                             ISDPTypes::AFMTPLine &result
                             )
      {
        if (source.mMaxPlaybackRate.hasValue()) {
          result.mFormatSpecific.push_back(String("maxplaybackrate=") + string(source.mMaxPlaybackRate.value()) + ";");
        }
        if (source.mMaxAverageBitrate.hasValue()) {
          result.mFormatSpecific.push_back(String("maxaveragebitrate=") + string(source.mMaxAverageBitrate.value()) + ";");
        }
        if (source.mStereo.hasValue()) {
          result.mFormatSpecific.push_back(String("stereo=") + (source.mStereo.value() ? "true" : "false") + ";");
        }
        if (source.mCBR.hasValue()) {
          result.mFormatSpecific.push_back(String("cbr=") + (source.mCBR.value() ? "true" : "false") + ";");
        }
        if (source.mUseInbandFEC.hasValue()) {
          result.mFormatSpecific.push_back(String("useinbandfec=") + (source.mUseInbandFEC.value() ? "true" : "false") + ";");
        }
        if (source.mUseDTX.hasValue()) {
          result.mFormatSpecific.push_back(String("dtx=") + (source.mUseDTX.value() ? "true" : "false") + ";");
        }
        if (source.mSPropMaxCaptureRate.hasValue()) {
          result.mFormatSpecific.push_back(String("sprop-maxcapturerate=") + string(source.mSPropMaxCaptureRate.value()) + ";");
        }
        if (source.mSPropStereo.hasValue()) {
          result.mFormatSpecific.push_back(String("sprop-stereo=") + (source.mSPropStereo.value() ? "true" : "false") + ";");
        }
      }

      //-----------------------------------------------------------------------
      static void fillFormat(
                             IRTPTypes::VP8CodecCapabilityParameters &source,
                             ISDPTypes::AFMTPLine &result
                             )
      {
        if (source.mMaxFR.hasValue()) {
          result.mFormatSpecific.push_back(String("max-fr=") + string(source.mMaxFR.value()) + ";");
        }
        if (source.mMaxFS.hasValue()) {
          result.mFormatSpecific.push_back(String("max-fs=") + string(source.mMaxFS.value()) + ";");
        }
      }

      //-----------------------------------------------------------------------
      static void fillFormat(
                             IRTPTypes::H264CodecCapabilityParameters &source,
                             ISDPTypes::AFMTPLine &result
                             )
      {
        if (source.mProfileLevelID.hasValue()) {
          result.mFormatSpecific.push_back(String("profile-level-id=") + string(source.mProfileLevelID.value()) + ";");
        }
        for (auto iter = source.mPacketizationModes.begin(); iter != source.mPacketizationModes.end(); ++iter) {
          auto mode = (*iter);
          result.mFormatSpecific.push_back(String("packetization-mode=") + string(mode) + ";");
        }
        if (source.mMaxMBPS.hasValue()) {
          result.mFormatSpecific.push_back(String("max-mbps=") + string(source.mMaxMBPS.value()) + ";");
        }
        if (source.mMaxSMBPS.hasValue()) {
          result.mFormatSpecific.push_back(String("max-smbps=") + string(source.mMaxSMBPS.value()) + ";");
        }
        if (source.mMaxFS.hasValue()) {
          result.mFormatSpecific.push_back(String("max-fs=") + string(source.mMaxFS.value()) + ";");
        }
        if (source.mMaxCPB.hasValue()) {
          result.mFormatSpecific.push_back(String("max-cpb=") + string(source.mMaxCPB.value()) + ";");
        }
        if (source.mMaxDPB.hasValue()) {
          result.mFormatSpecific.push_back(String("max-dpb=") + string(source.mMaxDPB.value()) + ";");
        }
        if (source.mMaxBR.hasValue()) {
          result.mFormatSpecific.push_back(String("max-br=") + string(source.mMaxBR.value()) + ";");
        }
      }

      //-----------------------------------------------------------------------
      static void fillFormat(
                             IRTPTypes::RTXCodecCapabilityParameters &source,
                             ISDPTypes::AFMTPLine &result
                             )
      {
        if (Milliseconds() != source.mRTXTime) {
          result.mFormatSpecific.push_back(String("apt=") + string(source.mApt) + ";rtx-time=" + string(source.mRTXTime.count()) + ";");
          return;
        }
        result.mFormatSpecific.push_back(String("apt=") + string(source.mApt) + ";");
      }

      //-----------------------------------------------------------------------
      static void fillFormat(
                             IRTPTypes::FlexFECCodecCapabilityParameters &source,
                             ISDPTypes::AFMTPLine &result
                             )
      {
        if (source.mL.hasValue()) {
          result.mFormatSpecific.push_back(String("L:") + string(source.mL.value()) + ";");
        }
        if (source.mD.hasValue()) {
          result.mFormatSpecific.push_back(String("D:") + string(source.mD.value()) + ";");
        }
        if (source.mToP.hasValue()) {
          result.mFormatSpecific.push_back(String("ToP:") + string(static_cast<std::underlying_type<IRTPTypes::FlexFECCodecCapabilityParameters::ToPs>::type>(source.mToP.value())) + ";");
        }
        if (Microseconds() != source.mRepairWindow) {
          result.mFormatSpecific.push_back(String("repair-window:") + string(source.mRepairWindow.count()));
        }
      }

      //-----------------------------------------------------------------------
      static ISDPTypes::AFMTPLinePtr fillFormat(
                                                const IRTPTypes::CodecCapability &codec,
                                                ISDPTypes::PayloadType pt
                                                )
      {
        if (!codec.mParameters) return nullptr;

        auto format = make_shared<ISDPTypes::AFMTPLine>(Noop{});
        format->mFormat = pt;

        auto supportedCodec = IRTPTypes::toSupportedCodec(codec.mName);
        switch (supportedCodec) {
          case IRTPTypes::SupportedCodec_Unknown: break;

          // audio codecs
          case IRTPTypes::SupportedCodec_Opus:     {
            auto params = ZS_DYNAMIC_PTR_CAST(IRTPTypes::OpusCodecCapabilityParameters, codec.mParameters);
            if (!params) return nullptr;
            fillFormat(*params, *format);
            break;
          }
          case IRTPTypes::SupportedCodec_Isac:     break;
          case IRTPTypes::SupportedCodec_G722:     break;
          case IRTPTypes::SupportedCodec_ILBC:     break;
          case IRTPTypes::SupportedCodec_PCMU:     break;
          case IRTPTypes::SupportedCodec_PCMA:     break;

          // video codecs
          case IRTPTypes::SupportedCodec_VP8:      {
            auto params = ZS_DYNAMIC_PTR_CAST(IRTPTypes::VP8CodecCapabilityParameters, codec.mParameters);
            if (!params) return nullptr;
            fillFormat(*params, *format);
            break;
          }
          case IRTPTypes::SupportedCodec_VP9:      break;
          case IRTPTypes::SupportedCodec_H264:     {
            auto params = ZS_DYNAMIC_PTR_CAST(IRTPTypes::H264CodecCapabilityParameters, codec.mParameters);
            if (!params) return nullptr;
            fillFormat(*params, *format);
            break;
          }

          // RTX
          case IRTPTypes::SupportedCodec_RTX: {
            auto params = ZS_DYNAMIC_PTR_CAST(IRTPTypes::RTXCodecCapabilityParameters, codec.mParameters);
            if (!params) return nullptr;
            fillFormat(*params, *format);
            break;
          }

          // FEC
          case IRTPTypes::SupportedCodec_RED:      break;
          case IRTPTypes::SupportedCodec_ULPFEC:   break;
          case IRTPTypes::SupportedCodec_FlexFEC: {
            auto params = ZS_DYNAMIC_PTR_CAST(IRTPTypes::FlexFECCodecCapabilityParameters, codec.mParameters);
            if (!params) return nullptr;
            fillFormat(*params, *format);
            break;
          }

          case IRTPTypes::SupportedCodec_CN:       break;

          case IRTPTypes::SupportedCodec_TelephoneEvent:   break;
        }

        if (format->mFormatSpecific.size() < 1) return nullptr;

        return format;
      }

      //-----------------------------------------------------------------------
      static void fillRTPMediaLine(
                                   const SDPParser::GeneratorOptions &options,
                                   const ISessionDescriptionTypes::Description &description,
                                   const ISessionDescriptionTypes::RTPMediaLine &mediaLine,
                                   ISDPTypes::SDP &ioSDP,
                                   ISDPTypes::MLine &ioMLine
                                   )
      {
        auto &result = ioSDP;
        auto &mline = ioMLine;

        if (mediaLine.mReceiverCapabilities) {
          Milliseconds ptime {};
          Milliseconds maxPTime {};
          bool ignorePTime = false;

          for (auto iter = mediaLine.mReceiverCapabilities->mCodecs.begin(); iter != mediaLine.mReceiverCapabilities->mCodecs.end(); iter) {
            auto &codec = (*iter);
            auto rtpmap = make_shared<ISDPTypes::ARTPMapLine>(Noop{});
            rtpmap->mPayloadType = codec.mPreferredPayloadType;
            rtpmap->mEncodingName = codec.mName;
            rtpmap->mClockRate = codec.mClockRate.hasValue() ? codec.mClockRate.value() : 0;
            if (!ignorePTime) {
              if (Milliseconds() != codec.mPTime) {
                if (Milliseconds() == ptime) {
                  ptime = codec.mPTime;
                } else {
                  ignorePTime = true;
                }
              }
            }
            if (Milliseconds() != codec.mMaxPTime) {
              if (Milliseconds() == maxPTime) {
                maxPTime = codec.mMaxPTime;
              } else if (codec.mMaxPTime.count() < maxPTime.count()) {
                maxPTime = codec.mMaxPTime;
              }
            }
            if (codec.mNumChannels.hasValue()) {
              rtpmap->mEncodingParameters = codec.mNumChannels.value();
            }
            mline.mARTPMapLines.push_back(rtpmap);
            auto format = fillFormat(codec, codec.mPreferredPayloadType);
            if (format) {
              mline.mAFMTPLines.push_back(format);
            }
          }
          if ((!ignorePTime) &&
            (Milliseconds() != ptime)) {
            mline.mAPTimeLine = make_shared<ISDPTypes::APTimeLine>(Noop{});
          }
          if (Milliseconds() != maxPTime) {
            mline.mAMaxPTimeLine = make_shared<ISDPTypes::AMaxPTimeLine>(Noop{});
          }
        }
      }
      
      //-----------------------------------------------------------------------
      static void addCNAME(
                           ISDPTypes::MLine &mline,
                           ISDPTypes::SSRCType ssrc,
                           const String &cname
                           )
      {
        if (!cname.hasData()) return;

        auto ssrcRTCP = make_shared<ISDPTypes::ASSRCLine>(Noop{});
        ssrcRTCP->mSSRC = ssrc;
        ssrcRTCP->mAttribute = "cname";
        ssrcRTCP->mAttributeValues.push_back(cname);
        mline.mASSRCLines.push_back(ssrcRTCP);
      }

      //-----------------------------------------------------------------------
      static void addSSRCGroup(
                              ISDPTypes::MLine &mline,
                              const char *groupSemantic,
                              ISDPTypes::SSRCType mainSSRC,
                              ISDPTypes::SSRCType relatedSSRC,
                              const String &cname
                              )
      {
        auto ssrcFID = make_shared<ISDPTypes::ASSRCGroupLine>(Noop{});
        ssrcFID->mSemantics = String(groupSemantic);
        ssrcFID->mSSRCs.push_back(mainSSRC);
        ssrcFID->mSSRCs.push_back(relatedSSRC);
        mline.mASSRCGroupLines.push_back(ssrcFID);
        addCNAME(mline, relatedSSRC, cname);
      }

      //-----------------------------------------------------------------------
      void SDPParser::createSDPMediaLevel(
                                          const GeneratorOptions &options,
                                          const Description &description,
                                          SDP &ioSDP
                                          )
      {
        auto &result = ioSDP;

        if ((description.mRTPMediaLines.size() < 1) &&
            (description.mSCTPMediaLines.size() < 1)) return;

        size_t largestIndex {};
        for (auto iter = description.mRTPMediaLines.begin(); iter != description.mRTPMediaLines.end(); ++iter) {
          auto &mediaLine = *(*iter);
          ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails);
          ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails->mInternalIndex.hasValue());
          auto index = mediaLine.mDetails->mInternalIndex.value();
          if (index > largestIndex) largestIndex = index;
        }
        for (auto iter = description.mSCTPMediaLines.begin(); iter != description.mSCTPMediaLines.end(); ++iter) {
          auto &mediaLine = *(*iter);
          ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails);
          ORTC_THROW_INVALID_PARAMETERS_IF(!mediaLine.mDetails->mInternalIndex.hasValue());
          auto index = mediaLine.mDetails->mInternalIndex.value();
          if (index > largestIndex) largestIndex = index;
        }

        for (size_t index = 0; index <= largestIndex; ++index) {
          {
            // scope: check rtp media lines for entry
            {
              for (auto iter = description.mRTPMediaLines.begin(); iter != description.mRTPMediaLines.end(); ++iter) {
                auto &mediaLine = *(*iter);
                auto value = mediaLine.mDetails->mInternalIndex.value();
                if (index != value) continue;
                auto mline = make_shared<ISDPTypes::MLine>(Noop{});
                createSDPRTPMediaLine(description, mediaLine, result, *mline);
                result.mMLines.push_back(mline);
                fillMediaLine(options, description, mediaLine, result, *mline);
                fillRTPMediaLine(options, description, mediaLine, result, *mline);

                if (mediaLine.mReceiverCapabilities) {
                  for (auto iterCodec = mediaLine.mReceiverCapabilities->mCodecs.begin(); iterCodec != mediaLine.mReceiverCapabilities->mCodecs.end(); ++iterCodec) {
                    auto &codec = (*iterCodec);
                    for (auto iterFB = codec.mRTCPFeedback.begin(); iterFB != codec.mRTCPFeedback.end(); ++iterFB) {
                      auto &fb = (*iterFB);
                      auto aFB = make_shared<ISDPTypes::ARTCPFBLine>(Noop{});
                      aFB->mPayloadType = codec.mPreferredPayloadType;
                      aFB->mID = fb.mType;
                      aFB->mParam1 = fb.mParameter;
                      mline->mARTCPFBLines.push_back(aFB);
                    }
                  }

                  for (auto iterHeaderExt = mediaLine.mReceiverCapabilities->mHeaderExtensions.begin(); iterHeaderExt != mediaLine.mReceiverCapabilities->mHeaderExtensions.end(); ++iterHeaderExt) {
                    auto &headerExt = (*iterHeaderExt);
                    auto aExt = make_shared<ISDPTypes::AExtmapLine>(Noop{});
                    aExt->mURI = headerExt.mURI;
                    aExt->mDirection = ISDPTypes::Direction_SendReceive;
                    aExt->mID = headerExt.mPreferredID;
                    mline->mAExtmapLines.push_back(aExt);
                  }
                }

                for (auto iterSender = description.mRTPSenders.begin(); iterSender != description.mRTPSenders.end(); ++iterSender) {
                  auto &sender = *(*iterSender);
                  if (!sender.mParameters) continue;
                  if (sender.mParameters->mEncodings.size() < 1) continue;
                  auto &encoding = *(sender.mParameters->mEncodings.begin());
                  if (!encoding.mSSRC.mHasValue) continue;

                  addCNAME(*mline, encoding.mSSRC.value(), sender.mParameters->mRTCP.mCName);

                  auto aMSLabel = make_shared<ISDPTypes::ASSRCLine>(Noop{});
                  aMSLabel->mAttribute = "mslabel";

                  if (sender.mMediaStreamTrackID.hasData()) {
                    if (sender.mMediaStreamIDs.size() > 0) {
                      for (auto iterMSID = sender.mMediaStreamIDs.begin(); iterMSID != sender.mMediaStreamIDs.end(); ++iterMSID) {
                        auto &msid = (*iterMSID);
                        auto ssrcMSID = make_shared<ISDPTypes::ASSRCLine>(Noop{});
                        auto aMSID = make_shared<ISDPTypes::AMSIDLine>(Noop{});
                        ssrcMSID->mSSRC = encoding.mSSRC.value();
                        ssrcMSID->mAttribute = "msid";
                        ssrcMSID->mAttributeValues.push_back(msid);
                        ssrcMSID->mAttributeValues.push_back(sender.mMediaStreamTrackID);
                        aMSID->mID = msid;
                        aMSID->mAppData = sender.mMediaStreamTrackID;

                        aMSLabel->mAttributeValues.push_back(msid);

                        mline->mASSRCLines.push_back(ssrcMSID);
                        mline->mAMSIDLines.push_back(aMSID);
                      }
                    }

#define NOTE_THIS_IS_FOR_LEGACY_REMOVE_WHEN_POSSIBLE 1
#define NOTE_THIS_IS_FOR_LEGACY_REMOVE_WHEN_POSSIBLE 2

                    // NOTE: LEGACY SUPPORT ONLY
                    auto ssrcLabel = make_shared<ISDPTypes::ASSRCLine>(Noop{});
                    ssrcLabel->mSSRC = encoding.mSSRC.value();
                    ssrcLabel->mAttribute = "label";
                    ssrcLabel->mAttributeValues.push_back(sender.mMediaStreamTrackID);
                    mline->mASSRCLines.push_back(ssrcLabel);
                    if (aMSLabel->mAttributeValues.size() > 0) {
                      mline->mASSRCLines.push_back(aMSLabel);
                    }
                  }
                  if (encoding.mRTX.hasValue()) {
                    if (encoding.mRTX.value().mSSRC.hasValue()) {
                      addSSRCGroup(*mline, "FID", encoding.mSSRC.value(), encoding.mRTX.value().mSSRC.value(), sender.mParameters->mRTCP.mCName);
                    }
                  }
                  if (encoding.mFEC.hasValue()) {
                    if (encoding.mFEC.value().mSSRC.hasValue()) {
                      addSSRCGroup(*mline, "FEC-FR", encoding.mSSRC.value(), encoding.mFEC.value().mSSRC.value(), sender.mParameters->mRTCP.mCName);
                    }
                  }
                }
                goto found;
              }
            }

            // scope: check sctp media lines for entry
            {
              for (auto iter = description.mSCTPMediaLines.begin(); iter != description.mSCTPMediaLines.end(); ++iter) {
                auto &mediaLine = *(*iter);
                auto value = mediaLine.mDetails->mInternalIndex.value();
                if (index != value) continue;
                auto mline = make_shared<ISDPTypes::MLine>(Noop{});
                createSDPSCTPMediaLine(description, mediaLine, result, *mline);
                result.mMLines.push_back(mline);
                fillMediaLine(options, description, mediaLine, result, *mline);
                if (mediaLine.mCapabilities) {
                  auto sctpPort = make_shared<ASCTPPortLine>(Noop{});
                  sctpPort->mPort = mediaLine.mCapabilities->mMaxPort;
                  auto maxMessageSize = make_shared<AMaxMessageSizeLine>(Noop{});
                  maxMessageSize->mMaxMessageSize = mediaLine.mCapabilities->mMaxMessageSize;
                  mline->mASCTPPortLine = sctpPort;
                  mline->mAMaxMessageSize = maxMessageSize;
                  auto format = make_shared<ISDPTypes::AFMTPLine>(Noop{});
                  format->mFormatStr = "webrtc-datachannel";
                  format->mFormatSpecific.push_back(String("max-message-size") + string(mediaLine.mCapabilities->mMaxMessageSize));
                  mline->mAFMTPLines.push_back(format);
                }
                goto found;
              }
            }
            goto not_found;
          }

        not_found:
          {
            auto mline = make_shared<ISDPTypes::MLine>(Noop{});
            mline->mMedia = "application";
            mline->mPort = 0;
            mline->mProto = ISDPTypes::ProtocolType_Unknown;
            mline->mProtoStr = "UDP/UNKNOWN";
            mline->mFmts.push_back("unknown");
            auto format = make_shared<ISDPTypes::AFMTPLine>(Noop{});
            format->mFormatStr = "unknown";
            format->mFormatSpecific.push_back(String("unknown=0"));
            mline->mAFMTPLines.push_back(format);
            continue;
          }

        found:
          {
          }
        }
      }

      //-----------------------------------------------------------------------
      void SDPParser::generateSessionLevel(
                                           const SDP &sdp,
                                           String &ioResult
                                           )
      {
        appendLine(ioResult, 'v', "0");
        appendLine(ioResult, 'o', ISDPTypes::OLine::toString(sdp.mOLine));
        appendLine(ioResult, 's', ISDPTypes::SLine::toString(sdp.mSLine));
        appendLine(ioResult, 't', ISDPTypes::TLine::toString(sdp.mTLine));
        appendLine(ioResult, 'c', ISDPTypes::CLine::toString(sdp.mCLine));

        if (sdp.mMediaDirection.hasValue()) {
          appendLine(ioResult, 'a', ISDPTypes::toString(sdp.mMediaDirection.value()));
        }

        if ((sdp.mICELite.hasValue()) &&
            (sdp.mICELite.value())) {
          appendLine(ioResult, 'a', "ice-lite");
        }

        appendLine(ioResult, 'a', ISDPTypes::AICEUFragLine::toString(sdp.mAICEUFragLine));
        appendLine(ioResult, 'a', ISDPTypes::AICEPwdLine::toString(sdp.mAICEPwdLine));
        appendLine(ioResult, 'a', ISDPTypes::AICEOptionsLine::toString(sdp.mAICEOptionsLine));
        appendLine(ioResult, 'a', ISDPTypes::ASetupLine::toString(sdp.mASetupLine));

        for (auto iter = sdp.mAGroupLines.begin(); iter != sdp.mAGroupLines.end(); ++iter) {
          auto &group = *(*iter);
          appendLine(ioResult, 'a', group.toString());
        }
        for (auto iter = sdp.mAMSIDSemanticLines.begin(); iter != sdp.mAMSIDSemanticLines.end(); ++iter) {
          auto &msidSemantic = *(*iter);
          appendLine(ioResult, 'a', msidSemantic.toString());
        }

      }

      //-----------------------------------------------------------------------
      void SDPParser::generateMediaLevel(
                                         const SDP &sdp,
                                         String &ioResult
                                         )
      {
        for (auto iter = sdp.mMLines.begin(); iter != sdp.mMLines.end(); ++iter) {
          auto mline = *(*iter);
          appendLine(ioResult, 'm', mline.toString());

          appendLine(ioResult, 'c', ISDPTypes::CLine::toString(mline.mCLine));
          if ((mline.mBundleOnly.hasValue()) &&
              (mline.mBundleOnly.value())) {
            appendLine(ioResult, 'a', "bundle-only");
          }
          appendLine(ioResult, 'a', ISDPTypes::AICEUFragLine::toString(mline.mAICEUFragLine));
          appendLine(ioResult, 'a', ISDPTypes::AICEPwdLine::toString(mline.mAICEPwdLine));
          appendLine(ioResult, 'a', ISDPTypes::ASetupLine::toString(mline.mASetupLine));
          appendLine(ioResult, 'a', ISDPTypes::AMIDLine::toString(mline.mAMIDLine));
          appendLine(ioResult, 'a', ISDPTypes::ARTCPLine::toString(mline.mARTCPLine));
          appendLine(ioResult, 'a', ISDPTypes::APTimeLine::toString(mline.mAPTimeLine));
          appendLine(ioResult, 'a', ISDPTypes::AMaxPTimeLine::toString(mline.mAMaxPTimeLine));
          //appendLine(ioResult, 'a', ISDPTypes::ASimulcastLine::toString(mline.mASimulcastLine));
          appendLine(ioResult, 'a', ISDPTypes::ASCTPPortLine::toString(mline.mASCTPPortLine));
          appendLine(ioResult, 'a', ISDPTypes::AMaxMessageSizeLine::toString(mline.mAMaxMessageSize));

          if (mline.mMediaDirection.hasValue()) {
            appendLine(ioResult, 'a', ISDPTypes::toString(mline.mMediaDirection.value()));
          }
          if ((mline.mRTCPMux.hasValue()) &&
              (mline.mRTCPMux.value())) {
            appendLine(ioResult, 'a', "rtcp-mux");
          }
          if ((mline.mRTCPRSize.hasValue()) &&
              (mline.mRTCPRSize.value())) {
            appendLine(ioResult, 'a', "rtcp-rsize");
          }

          for (auto iterSub = mline.mAMSIDLines.begin(); iterSub != mline.mAMSIDLines.end(); ++iterSub) {
            auto &msid = *(*iterSub);
            appendLine(ioResult, 'a', msid.toString());
          }
          for (auto iterSub = mline.mAFingerprintLines.begin(); iterSub != mline.mAFingerprintLines.end(); ++iterSub) {
            auto &fingerprint = *(*iterSub);
            appendLine(ioResult, 'a', fingerprint.toString());
          }
          for (auto iterSub = mline.mACryptoLines.begin(); iterSub != mline.mACryptoLines.end(); ++iterSub) {
            auto &crypto = *(*iterSub);
            appendLine(ioResult, 'a', crypto.toString());
          }
          for (auto iterSub = mline.mAExtmapLines.begin(); iterSub != mline.mAExtmapLines.end(); ++iterSub) {
            auto &extmap = *(*iterSub);
            appendLine(ioResult, 'a', extmap.toString());
          }
          for (auto iterSub = mline.mARTPMapLines.begin(); iterSub != mline.mARTPMapLines.end(); ++iterSub) {
            auto &rtpmap = *(*iterSub);
            appendLine(ioResult, 'a', rtpmap.toString());
          }
          for (auto iterSub = mline.mAFMTPLines.begin(); iterSub != mline.mAFMTPLines.end(); ++iterSub) {
            auto &fmtp = *(*iterSub);
            appendLine(ioResult, 'a', fmtp.toString());
          }
          for (auto iterSub = mline.mACandidateLines.begin(); iterSub != mline.mACandidateLines.end(); ++iterSub) {
            auto &candidate = *(*iterSub);
            appendLine(ioResult, 'a', candidate.toString());
          }
          if ((mline.mEndOfCandidates.hasValue()) &&
              (mline.mEndOfCandidates.value())) {
            appendLine(ioResult, 'a', "end-of-candidates");
          }

          for (auto iterSub = mline.mARIDLines.begin(); iterSub != mline.mARIDLines.end(); ++iterSub) {
            auto &rid = *(*iterSub);
            appendLine(ioResult, 'a', rid.toString());
          }
          for (auto iterSub = mline.mASSRCLines.begin(); iterSub != mline.mASSRCLines.end(); ++iterSub) {
            auto &ssrc = *(*iterSub);
            appendLine(ioResult, 'a', ssrc.toString());
            for (auto iterSSrcSub = ssrc.mAFMTPLines.begin(); iterSSrcSub != ssrc.mAFMTPLines.end(); ++iterSSrcSub) {
              auto &ssrcFmtp = *(*iterSSrcSub);
              appendLine(ioResult, 'a', ssrcFmtp.toString());
            }
          }
          for (auto iterSub = mline.mASSRCGroupLines.begin(); iterSub != mline.mASSRCGroupLines.end(); ++iterSub) {
            auto &ssrcGroup = *(*iterSub);
            appendLine(ioResult, 'a', ssrcGroup.toString());
          }
        }
      }

      //-----------------------------------------------------------------------
      String SDPParser::generate(const SDP &sdp)
      {
        String result;
        result.reserve(2500);
        try {
          generateSessionLevel(sdp, result);
          generateMediaLevel(sdp, result);
        } catch (const SafeIntException &e) {
          ORTC_THROW_INVALID_PARAMETERS("value found out of legal value range" + string(e.m_code));
        }
        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::SDPPtr SDPParser::createSDP(
                                             const GeneratorOptions &options,
                                             const Description &description
                                             )
      {
        SDPPtr result(make_shared<SDP>());

        try {
          createSDPSessionLevel(options, description, *result);
          createSDPMediaLevel(options, description, *result);
        } catch (const SafeIntException &e) {
          ORTC_THROW_INVALID_PARAMETERS("value found out of legal value range" + string(e.m_code));
        }

        return result;
      }

      //-----------------------------------------------------------------------
      ISDPTypes::DescriptionPtr SDPParser::createDescription(
                                                             Locations location,
                                                             const SDP &sdp
                                                             )
      {
        DescriptionPtr result(make_shared<Description>());

        try {
          createDescriptionDetails(sdp, *result);
          createTransports(sdp, *result);
          createRTPMediaLines(location, sdp, *result);
          createSCTPMediaLines(location, sdp, *result);
        } catch (const SafeIntException &e) {
          ORTC_THROW_INVALID_PARAMETERS("value found out of legal value range" + string(e.m_code));
        }

        return result;
      }

      //-----------------------------------------------------------------------
      String SDPParser::getCandidateSDP(
                                        const IICETypes::Candidate &candidate,
                                        WORD componentID
                                        )
      {
        auto result = internal::fillCandidate(candidate, componentID);
        if (!result) {
          ZS_LOG_WARNING(Debug, internal::slog("unable to convert candidate to string") + candidate.toDebug());
          return String();
        }
        return result->toString();
      }

      //-----------------------------------------------------------------------
      IICETypes::GatherCandidatePtr SDPParser::getCandidateFromSDP(
                                                                   const char *candidate,
                                                                   WORD &outComponentID
                                                                   )
      {
        String str(candidate);

        {
          String prefix = str.substr(0, strlen("a="));
          if (0 == prefix.compareNoCase("a=")) {
            str = str.substr(strlen("a="));
            str.trim();
          }
        }

        if (0 == str.compareNoCase("end-of-candidates")) {
          auto result = make_shared<IICETypes::CandidateComplete>();
          result->mComplete = true;
          return result;
        }

        IICETypes::CandidatePtr result;

        {
          String prefix = str.substr(0, strlen("candidate:"));
          if (0 == prefix.compareNoCase("candidate:")) {
            str = str.substr(strlen("candidate:"));
            str.trim();
          }
        }

        try {
          auto aCandidate = make_shared<ACandidateLine>(nullptr, str);
          result = internal::convertCandidate(*aCandidate);
          outComponentID = aCandidate->mComponentID;
        } catch (const SafeIntException &e) {
          ZS_LOG_WARNING(Debug, internal::slog("unable to convert to candidate") + ZS_PARAM("candidate", str) + ZS_PARAM("e", e.m_code));
          return result;
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, internal::slog("unable to convert to candidate") + ZS_PARAM("candidate", str));
          return result;
        }

        return result;
      }

    }  // namespace internal
  } // namespace adapter
} // namespace ortc
