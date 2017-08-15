/*

 Copyright (c) 2017, Hookflash Inc. / Optical Tone Ltd.
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

#include <ortc/internal/ortc_IMediaDevice.h>

//#include <ortc/internal/ortc_MediaEngine.h>

//#include <ortc/internal/ortc_MediaDeviceCaptureAudio.h>
//#include <ortc/internal/ortc_MediaDeviceCaptureVideo.h>
//#include <ortc/internal/ortc_MediaDeviceRenderAudio.h>
////#include <ortc/internal/ortc_RTPReceiverChannelAudio.h>
////#include <ortc/internal/ortc_RTPReceiverChannelVideo.h>
////#include <ortc/internal/ortc_RTPSenderChannelAudio.h>
////#include <ortc/internal/ortc_RTPSenderChannelVideo.h>
//#include <ortc/internal/ortc_RTPPacket.h>
//#include <ortc/internal/ortc_RTCPPacket.h>
//#include <ortc/internal/ortc_ORTC.h>
//#include <ortc/internal/ortc_StatsReport.h>
//#include <ortc/internal/ortc.events.h>
//#include <ortc/internal/platform.h>
//
//#include <ortc/IStatsReport.h>
//
//#include <ortc/IHelper.h>
//#include <ortc/services/IHTTP.h>
//
//#include <zsLib/ISettings.h>
//#include <zsLib/Singleton.h>
//#include <zsLib/Log.h>
//#include <zsLib/XML.h>
//#include <zsLib/SafeInt.h>
//
//#include <cryptopp/sha.h>
//
//#include <webrtc/base/event_tracer.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_MediaEngine) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    //-------------------------------------------------------------------------
    IMediaStreamTrackTypes::SettingsPtr obtainBestMode(
                                                       const std::list<IMediaStreamTrackTypes::TrackConstraintsPtr> &allConstraints,
                                                       const std::list<IMediaStreamTrackTypes::SettingsPtr> &availableModes
                                                       )
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::StringList, StringList);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraintsPtr, UseTrackConstraints);
      ZS_DECLARE_TYPEDEF_PTR(std::list<UseTrackConstraintsPtr>, UseTrackConstraintsList);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Settings, UseSettings);
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ConstraintSet, UseConstraintSet);

      //-----------------------------------------------------------------------
      struct ModeInfo
      {
        UseSettingsPtr settings_;
        bool rejected{};
        size_t idealCount_{};

        static bool containsString(
                                   const StringList &strs,
                                   const String &str
                                   )
        {
          for (auto iter = strs.begin(); iter != strs.end(); ++iter) { auto &value = (*iter); if (value == str) return true; }
          return false;
        }

        //---------------------------------------------------------------------
        bool innerProcessConstrain(
                                   const IMediaStreamTrackTypes::ConstrainString &constrain,
                                   const Optional<String> &value
                                   )
        {
          StringList exacts;
          StringList ideals;
          constrain.ideal(ideals);
          constrain.exact(exacts);
          if (!value.hasValue()) {
            return (exacts.size() < 1);
          }
          if (exacts.size() > 0) {
            if (!containsString(exacts, value.value())) return false;
          }
          if (ideals.size() < 1) return true;
          if (!containsString(ideals, value.value())) return true;
          ++idealCount_;
          return true;
        }

        //---------------------------------------------------------------------
        bool processConstrain(
                              const IMediaStreamTrackTypes::ConstrainString &constrain,
                              const Optional<String> &value
                              )
        {
          if (!innerProcessConstrain(constrain, value)) {
            rejected = true;
            return false;
          }
          return true;
        }

        //---------------------------------------------------------------------
        bool innerProcessConstrain(
                                   const IMediaStreamTrackTypes::ConstrainDouble &constrain,
                                   const Optional<double> &value
                                   )
        {
          if (constrain.mValue.hasValue()) {
            if (!value.hasValue()) return false;
            return constrain.mValue.value() == value.value();
          }
          if (!constrain.mRange.hasValue()) return true;

          auto &values = constrain.mRange.value();
          if (values.mMin.hasValue()) {
            if (!value.hasValue()) return false;
            if (value.value() < values.mMin.value()) return false;
          }
          if (values.mMax.hasValue()) {
            if (!value.hasValue()) return false;
            if (value.value() > values.mMax.value()) return false;
          }
          if (values.mExact.hasValue()) {
            if (!value.hasValue()) return false;
            if (value.value() != values.mExact.value()) return false;
          }
          if (!values.mIdeal.hasValue()) return true;
          if (!value.hasValue()) return true;
          if (values.mIdeal.value() != value.value()) return true;
          ++idealCount_;
          return true;
        }

        //---------------------------------------------------------------------
        bool processConstrain(
                              const IMediaStreamTrackTypes::ConstrainDouble &constrain,
                              const Optional<double> &value
                              )
        {
          if (!innerProcessConstrain(constrain, value)) {
            rejected = true;
            return false;
          }
          return true;
        }

        //---------------------------------------------------------------------
        bool innerProcessConstrain(
                                   const IMediaStreamTrackTypes::ConstrainLong &constrain,
                                   const Optional<LONG> &value
                                   )
        {
          if (constrain.mValue.hasValue()) {
            if (!value.hasValue()) return false;
            return constrain.mValue.value() == value.value();
          }
          if (!constrain.mRange.hasValue()) return true;

          auto &values = constrain.mRange.value();
          if (values.mMin.hasValue()) {
            if (!value.hasValue()) return false;
            if (value.value() < values.mMin.value()) return false;
          }
          if (values.mMax.hasValue()) {
            if (!value.hasValue()) return false;
            if (value.value() > values.mMax.value()) return false;
          }
          if (values.mExact.hasValue()) {
            if (!value.hasValue()) return false;
            if (value.value() != values.mExact.value()) return false;
          }
          if (!values.mIdeal.hasValue()) return true;
          if (!value.hasValue()) return true;
          if (values.mIdeal.value() != value.value()) return true;
          ++idealCount_;
          return true;
        }

        //---------------------------------------------------------------------
        bool processConstrain(
                              const IMediaStreamTrackTypes::ConstrainLong &constrain,
                              const Optional<LONG> &value
                              )
        {
          if (!innerProcessConstrain(constrain, value)) {
            rejected = true;
            return false;
          }
          return true;
        }


        //---------------------------------------------------------------------
        bool innerProcessConstrain(
                                   const IMediaStreamTrackTypes::ConstrainBoolean &constrain,
                                   const Optional<bool> &value
                                   )
        {
          if (constrain.mValue.hasValue()) {
            if (!value.hasValue()) return false;
            return constrain.mValue.value() == value.value();
          }
          if (!constrain.mParameters.hasValue()) return true;

          auto &values = constrain.mParameters.value();
          if (values.mExact.hasValue()) {
            if (!value.hasValue()) return false;
            if (value.value() != values.mExact.value()) return false;
          }
          if (!values.mIdeal.hasValue()) return true;
          if (!value.hasValue()) return true;
          if (values.mIdeal.value() != value.value()) return true;
          ++idealCount_;
          return true;
        }

        //---------------------------------------------------------------------
        bool processConstrain(
                              const IMediaStreamTrackTypes::ConstrainBoolean &constrain,
                              const Optional<bool> &value
                              )
        {
          if (!innerProcessConstrain(constrain, value)) {
            rejected = true;
            return false;
          }
          return true;
        }
        //---------------------------------------------------------------------
        bool checkConstrainSet(const UseConstraintSet &constrain)
        {
          auto &settings = (*settings_);

          if (!processConstrain(constrain.mWidth, settings.mWidth)) return false;
          if (!processConstrain(constrain.mHeight, settings.mHeight)) return false;
          if (!processConstrain(constrain.mAspectRatio, settings.mAspectRatio)) return false;
          if (!processConstrain(constrain.mFrameRate, settings.mFrameRate)) return false;
          if (!processConstrain(constrain.mFacingMode, settings.mFacingMode)) return false;
          if (!processConstrain(constrain.mOrientation, settings.mOrientation)) return false;
          if (!processConstrain(constrain.mVolume, settings.mVolume)) return false;
          if (!processConstrain(constrain.mSampleRate, settings.mSampleRate)) return false;
          if (!processConstrain(constrain.mSampleSize, settings.mSampleSize)) return false;
          if (!processConstrain(constrain.mEchoCancellation, settings.mEchoCancellation)) return false;
          if (!processConstrain(constrain.mLatency, settings.mLatency)) return false;
          if (!processConstrain(constrain.mChannelCount, settings.mChannelCount)) return false;
          if (!processConstrain(constrain.mDeviceID, settings.mDeviceID)) return false;
          if (!processConstrain(constrain.mGroupID, settings.mGroupID)) return false;
          if (!processConstrain(constrain.mEncodingFormat, settings.mEncodingFormat)) return false;

          return true;
        }
      };

      ZS_DECLARE_TYPEDEF_PTR(std::list<ModeInfo>, ModeInfoList);

      ModeInfoList outputModes;
      for (auto iter = availableModes.begin(); iter != availableModes.end(); ++iter)
      {
        ModeInfo info;
        info.settings_ = (*iter);
        outputModes.push_back(info);
      }

      for (auto iter = allConstraints.begin(); iter != allConstraints.end(); ++iter) {
        auto &constraints = *(*iter);

        for (auto iterMode = outputModes.begin(); iterMode != outputModes.end(); ++iterMode) {
          auto &mode = (*iterMode);
          if (mode.rejected) continue;

          if (!mode.checkConstrainSet(constraints)) continue;

          for (auto iterAdv = constraints.mAdvanced.begin(); iterAdv != constraints.mAdvanced.end(); ++iterAdv) {
            auto &advancedConstraints = *(*iter);
            if (!mode.checkConstrainSet(constraints)) break;
          }
        }
      }

      UseSettingsPtr foundSettings;
      size_t mostIdeal {};

      for (auto iterMode = outputModes.begin(); iterMode != outputModes.end(); ++iterMode) {
        auto &mode = (*iterMode);
        if (mode.rejected) continue;

        if ((foundSettings) ||
            (mode.idealCount_ <= mostIdeal)) continue;

        foundSettings = mode.settings_;
        mostIdeal = mode.idealCount_;
      }

      return foundSettings;
    }

    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaDevice
    #pragma mark

    //-------------------------------------------------------------------------
    const char *IMediaDevice::toString(States state)
    {
      switch (state)
      {
        case State_Pending:         return "pending";
        case State_Active:          return "active";
        case State_Reinitializing:  return "reinitialzing";
        case State_ShuttingDown:    return "shutting down";
        case State_Shutdown:        return "shutdown";
      }
      return "UNKNOWN";
    }

  } // internal namespace
}
