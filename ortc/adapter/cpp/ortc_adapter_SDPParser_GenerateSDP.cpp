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
      #pragma mark SDPParser
      #pragma mark

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
          for (auto iterSub = mline.mARTCPFBLines.begin(); iterSub != mline.mARTCPFBLines.end(); ++iterSub) {
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


    }  // namespace internal
  } // namespace adapter
} // namespace ortc
