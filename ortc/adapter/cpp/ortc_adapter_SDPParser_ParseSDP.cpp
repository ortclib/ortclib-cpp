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

          ++pos;
          if ('=' != *pos) {
            ++pos;
            continue;
          }
          ++pos;

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
          info.mValue = attributeValue;
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
              case Attribute_RTCPMux:           flag->mMLine->mRTCPMux = true; goto remove_line;
              case Attribute_RTCPRSize:         flag->mMLine->mRTCPRSize = true; goto remove_line;
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
      ISDPTypes::SDPPtr SDPParser::parse(const char *blob)
      {
        if (!blob) return SDPPtr();

        SDPPtr sdp(make_shared<SDP>());

        sdp->mOriginal = String(blob);
        std::unique_ptr<char[]> rawBuffer(new char[sdp->mOriginal.length()+1]);
        sdp->mRawBuffer = std::move(rawBuffer);

        memset(sdp->mRawBuffer.get(), 0, sizeof(char)*(sdp->mOriginal.length()+1));
        memcpy(sdp->mRawBuffer.get(), blob, sizeof(char)*(sdp->mOriginal.length()));

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


    }  // namespace internal
  } // namespace adapter
} // namespace ortc
