/*
 
 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/internal/types.h>
#include <ortc/internal/ortc_IMediaStreamTrack.h>

#include <unordered_map>

#define ORTC_SETTING_MEDIA_CHANNEL_TRACE_HELPER_USAGE_INACTIVITY_AGE_PURGE_TIME_IN_SECONDS "ortc/media-channel-trace-helper/usage-inactivity-age-purge-time-in-seconds"
#define ORTC_SETTING_MEDIA_CHANNEL_TRACE_HELPER_USAGE_INACTIVITY_AGE_CHECK_MODULAS_VALUE "ortc/media-channel-trace-helper/usage-inactivity-age-check-modulas-value"

namespace ortc
{
  namespace internal
  {
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaChannelTraceHelper
    #pragma mark

    class MediaChannelTraceHelper : public SharedRecursiveLock
    {
    public:
      typedef IMediaStreamTrackTypes::MediaChannelID MediaChannelID;
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::ImmutableMediaChannelTrace, ImmutableMediaChannelTrace);

      typedef uintptr_t TraceHandle;

      struct TraceSourceInfo
      {
        int lastUsageAt_ {};
        ImmutableMediaChannelTracePtr trace_;
      };

      typedef std::unordered_map<TraceHandle, TraceSourceInfo> TraceSourceInfoMap;

    public:
      MediaChannelTraceHelper(MediaChannelID mediaChannelID);

      ImmutableMediaChannelTracePtr trace(ImmutableMediaChannelTracePtr sourceTrace);

    protected:
      void doPurge();

    protected:
      AutoPUID id_;
      MediaChannelID selfChannelID_ {};
      std::atomic<int> lastUsageCount_ {};
      TraceSourceInfoMap traceSources_;

      Time lastAgeCountTimeAt_ {};
      int lastAgeCountValueAtTime_ {};
      Seconds agePurge_ {};
      int ageCheck_ {};
    };
  }
}
