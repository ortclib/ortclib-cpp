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

#include <ortc/internal/ortc_MediaChannelTraceHelper.h>
#include <ortc/internal/platform.h>

#include <zsLib/ISettings.h>
#include <zsLib/Singleton.h>
#include <zsLib/SafeInt.h>

#include <set>

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG


namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_media_stream_track) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib, ISettings);

  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(MediaChannelTraceHelperDefaults);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaChannelTraceHelperDefaults
    #pragma mark

    class MediaChannelTraceHelperDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~MediaChannelTraceHelperDefaults()
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static MediaChannelTraceHelperDefaultsPtr singleton()
      {
        static SingletonLazySharedPtr<MediaChannelTraceHelperDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static MediaChannelTraceHelperDefaultsPtr create()
      {
        auto pThis(make_shared<MediaChannelTraceHelperDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      virtual void notifySettingsApplyDefaults() override
      {
        ISettings::setUInt(ORTC_SETTING_MEDIA_CHANNEL_TRACE_HELPER_USAGE_INACTIVITY_AGE_PURGE_TIME_IN_SECONDS, 30);
        ISettings::setInt(ORTC_SETTING_MEDIA_CHANNEL_TRACE_HELPER_USAGE_INACTIVITY_AGE_CHECK_MODULAS_VALUE, 512);
      }
    };

    //-------------------------------------------------------------------------
    void installMediaChannelTraceHelperDefaults()
    {
      MediaChannelTraceHelperDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaChannelTraceHelper
    #pragma mark

    //-------------------------------------------------------------------------
    MediaChannelTraceHelper::MediaChannelTraceHelper(MediaChannelID mediaChannelID) :
      SharedRecursiveLock(SharedRecursiveLock::create()),
      selfChannelID_(mediaChannelID),
      lastAgeCountTimeAt_(zsLib::now()),
      agePurge_(SafeInt<decltype(agePurge_)::rep>(ISettings::getUInt(ORTC_SETTING_MEDIA_CHANNEL_TRACE_HELPER_USAGE_INACTIVITY_AGE_PURGE_TIME_IN_SECONDS))),
      ageCheck_(SafeInt<decltype(ageCheck_)>(ISettings::getInt(ORTC_SETTING_MEDIA_CHANNEL_TRACE_HELPER_USAGE_INACTIVITY_AGE_CHECK_MODULAS_VALUE)))
    {
      ASSERT(ageCheck_ > 1);  // assert value is set properly
      if (ageCheck_ < 1) ageCheck_ = 1;
    }

    //-------------------------------------------------------------------------
    MediaChannelTraceHelper::ImmutableMediaChannelTracePtr MediaChannelTraceHelper::trace(ImmutableMediaChannelTracePtr sourceTrace)
    {
      ASSERT((bool)sourceTrace);
      uintptr_t handle = reinterpret_cast<uintptr_t>(sourceTrace.get());

      int checkCount = (++lastUsageCount_);
      bool shouldCheck = (0 == (checkCount % ageCheck_));

      {
        AutoRecursiveLock lock(*this);
        auto found = traceSources_.find(handle);
        if (found != traceSources_.end()) {
          auto &info = (*found).second;
          info.lastUsageAt_ = checkCount;
          if (shouldCheck) {
            ImmutableMediaChannelTracePtr result = info.trace_;
            doPurge();
            return result;
          }
          return info.trace_;
        }

        TraceSourceInfo info;
        info.lastUsageAt_ = checkCount;
        info.trace_ = IMediaStreamTrackTypes::MediaChannelTrace::create(*sourceTrace, selfChannelID_);
        traceSources_[handle] = info;

        if (shouldCheck) {
          ImmutableMediaChannelTracePtr result = info.trace_;
          doPurge();
          return result;
        }
        return info.trace_;
      }
    }

    //-------------------------------------------------------------------------
    void MediaChannelTraceHelper::doPurge()
    {
      auto tick = zsLib::now();

      if (lastAgeCountTimeAt_ + agePurge_ > tick) return; // not time to purge yet

      if (lastUsageCount_ < 0) lastUsageCount_ = 0; // count roll over prevention

      // setup values to check for next time
      lastAgeCountTimeAt_ = tick;
      int lastAgeCountValueAtTime = lastAgeCountValueAtTime_;
      lastAgeCountValueAtTime_ = lastUsageCount_;

      for (auto iter_doNotUse = traceSources_.begin(); iter_doNotUse != traceSources_.end(); )
      {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto &info = (*current).second;
        if (info.lastUsageAt_ > lastAgeCountValueAtTime) continue;

        traceSources_.erase(current);
      }
    }

  } // namespace internal
} // namespace ortc
