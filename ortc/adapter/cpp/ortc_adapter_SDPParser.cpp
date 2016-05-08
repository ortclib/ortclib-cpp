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

#include <ortc/internal/ortc_Helper.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/Log.h>
#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/XML.h>

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

        LineTypes checkLines[] = {
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
          case Attribute_RTCPRSize:          return true;
          case Attribute_PTime:             break;
          case Attribute_MaxPTime:          break;
          case Attribute_SSRC:              break;
          case Attribute_SSRCGroup:         break;
          case Attribute_Simulcast:         break;
          case Attribute_RID:               break;
          case Attribute_SCTPPort:          break;
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
        AttributeLevels checkLevels[] =
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
        for (size_t index = 0; AttributeLevel_None != checkLevels[index]; ++index) {
          if (0 == str.compareNoCase(toString(checkLevels[index]))) return checkLevels[index];
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
        ZS_THROW_NOT_IMPLEMENTED("temp");
      }

      //-----------------------------------------------------------------------
      ISDPTypes::Locations ISDPTypes::toLocation(const char *location)
      {
        ZS_THROW_NOT_IMPLEMENTED("temp");
      }

      //-----------------------------------------------------------------------
      const char *ISDPTypes::toString(Directions location)
      {
        ZS_THROW_NOT_IMPLEMENTED("temp");
      }

      //-----------------------------------------------------------------------
      ISDPTypes::Directions ISDPTypes::toDirection(const char *location)
      {
        ZS_THROW_NOT_IMPLEMENTED("temp");
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
        ZS_THROW_NOT_IMPLEMENTED("temp");
      }


      //-----------------------------------------------------------------------
      const char *ISDPTypes::toString(RTPDirectionContexts context)
      {
        ZS_THROW_NOT_IMPLEMENTED("temp");
      }

      //-----------------------------------------------------------------------
      ISDPTypes::RTPDirectionContexts ISDPTypes::toRTPDirectionContext(const char *location)
      {
        ZS_THROW_NOT_IMPLEMENTED("temp");
      }

      //-----------------------------------------------------------------------
      bool ISDPTypes::isApplicable(
                                   RTPDirectionContexts context,
                                   Locations location,
                                   Directions direction
                                   )
      {
        ZS_THROW_NOT_IMPLEMENTED("temp");
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
        ORTC_THROW_INVALID_PARAMETERS_IF(split.size() < 4);

        mMedia = split[0];
        mProto = split[2];

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
          auto value = (*iter).second;
          mTags.push_back(value);
        }
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

        if (split.size() > 10) {
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
          mRelAddr = split[9];
        }
        if (split.size() > 9) {
          try {
            mRelPort = Numeric<decltype(mRelPort.mType)>(split[9]);
          } catch (const Numeric<decltype(mRelPort.mType)>::ValueOutOfRange &) {
            ORTC_THROW_INVALID_PARAMETERS("rel port value out of range: " + split[9]);
          }
        }

        if (split.size() > 10) {
          for (auto index = 10; index < split.size(); index += 2)
          {
            ExtensionPair value(split[index], split[index + 1]);
            mExtensionPairs.push_back(value);
          }
        }
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
      ISDPTypes::AFMTPLine::AFMTPLine(MLinePtr mline, ASSRCLinePtr sourceLine, const char *value) :
        AMediaLine(mline),
        mSourceLine(sourceLine)
      {
        UseServicesHelper::SplitMap split;
        UseServicesHelper::split(String(value), split, " ");
        UseServicesHelper::splitTrim(split);
        UseServicesHelper::splitPruneEmpty(split);
        ORTC_THROW_INVALID_PARAMETERS_IF(2 != split.size());

        try {
          mFormat = Numeric<decltype(mFormat)>(split[0]);
        } catch (const Numeric<decltype(mFormat)>::ValueOutOfRange &) {
          ORTC_THROW_INVALID_PARAMETERS("payload type value out of range: " + split[0]);
        }

        for (auto iter = split.begin(); iter != split.end(); ++iter) {
          auto &value = (*iter).second;
          mFormatSpecific.push_back(value);
        }
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
                mline->mASCTPPortLines.push_back(ZS_DYNAMIC_PTR_CAST(ASCTPPortLine, info.mLineValue));
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
      ISDPTypes::SDPPtr SDPParser::parse(const char *blob)
      {
        if (!blob) return SDPPtr();

        SDPPtr sdp(make_shared<SDP>());

        sdp->mOriginal = String(blob);
        std::unique_ptr<char[]> rawBuffer(new char[sdp->mOriginal.length()]);
        sdp->mRawBuffer = std::move(rawBuffer);

        memcpy(sdp->mRawBuffer.get(), blob, sdp->mOriginal.length());

        parseLines(*sdp);
        parseAttributes(*sdp);
        validateAttributeLevels(*sdp);
        parseLinesDetails(*sdp);
        processFlagAttributes(*sdp);
        processSessionLevelValues(*sdp);
        processMediaLevelValues(*sdp);
        processSourceLevelValues(*sdp);

        return sdp;
      }

    }  // namespace internal
  } // namespace adapter
} // namespace ortc
