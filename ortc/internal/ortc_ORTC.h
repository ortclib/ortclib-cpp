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
#include <ortc/IORTC.h>

#define ORTC_QUEUE_MAIN_THREAD_NAME "org.ortc.ortcLib.MainThread"
#define ORTC_QUEUE_PIPELINE_THREAD_NAME "org.ortc.ortcLib.Pipeline"
#define ORTC_QUEUE_BLOCKING_MEDIA_STARTUP_THREAD_NAME "org.ortc.ortcLib.BlockingMedia"
#define ORTC_QUEUE_CERTIFICATE_GENERATION_NAME "org.ortc.ortcLib.CertificateGeneration"
#define ORTC_QUEUE_MEDIA_DEVICE_THREAD_NAME "org.ortc.ortcLib.MediaDeviceThread."
#define ORTC_QUEUE_RTP_THREAD_NAME "org.ortc.ortcLib.RTP."
#define ORTC_QUEUE_TOTAL_MEDIA_DEVICE_THREADS 4
#define ORTC_QUEUE_TOTAL_RTP_THREADS 4

#define ORTC_SETTING_ORTC_QUEUE_MAIN_THREAD_NAME "ortc/ortc/" ORTC_QUEUE_MAIN_THREAD_NAME
#define ORTC_SETTING_ORTC_QUEUE_PIPELINE_THREAD_NAME "ortc/ortc/" ORTC_QUEUE_PIPELINE_THREAD_NAME
#define ORTC_SETTING_ORTC_QUEUE_BLOCKING_MEDIA_STARTUP_THREAD_NAME "ortc/ortc/" ORTC_QUEUE_BLOCKING_MEDIA_STARTUP_THREAD_NAME
#define ORTC_SETTING_ORTC_QUEUE_CERTIFICATE_GENERATION_NAME "ortc/ortc/" ORTC_QUEUE_CERTIFICATE_GENERATION_NAME
#define ORTC_SETTING_ORTC_QUEUE_MEDIA_DEVICE_THREAD_NAME "ortc/ortc/" ORTC_QUEUE_MEDIA_DEVICE_THREAD_NAME
#define ORTC_SETTING_ORTC_QUEUE_RTP_THREAD_NAME "ortc/ortc/" ORTC_QUEUE_RTP_THREAD_NAME

namespace ortc
{
  namespace internal
  {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //
    // IORTCForInternal
    //

    interaction IORTCForInternal
    {
      ZS_DECLARE_TYPEDEF_PTR(IORTCForInternal, ForInternal)

      static void overrideQueueDelegate(IMessageQueuePtr queue) noexcept;
      static IMessageQueuePtr queueDelegate() noexcept;
      static IMessageQueuePtr queueORTC() noexcept;
      static IMessageQueuePtr queueORTCPipeline() noexcept;
      static IMessageQueuePtr queueBlockingMediaStartStopThread() noexcept;
      static IMessageQueuePtr queueMediaDevices() noexcept;
      static IMessageQueuePtr queueRTP() noexcept;
      static IMessageQueuePtr queueCertificateGeneration() noexcept;

      static Optional<Log::Level> webrtcLogLevel() noexcept;
    };

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //
    // Stack
    //

    class ORTC : public IORTC,
                 public IORTCForInternal,
                 public SharedRecursiveLock
    {
    protected:
      struct make_private {};

    public:
      friend interaction IORTC;
      friend interaction IORTCForInternal;

    public:
      ORTC(const make_private &) noexcept;

    protected:
      void init() noexcept;

    public:
      virtual ~ORTC() noexcept;

      static ORTCPtr create() noexcept;
      static ORTCPtr convert(IORTCPtr object) noexcept;

    protected:
      //---------------------------------------------------------------------
      //
      // ORTC => IORTC
      //

      static ORTCPtr singleton() noexcept;

      virtual PUID getID() const noexcept {return mID;}

      virtual void setup(IMessageQueuePtr defaultDelegateMessageQueue) noexcept;
#ifdef WINUWP
      virtual void setup(Windows::UI::Core::CoreDispatcher ^dispatcher) noexcept;
#endif //WINUWP

      virtual Milliseconds ntpServerTime() const noexcept;
      virtual void ntpServerTime(const Milliseconds &value) noexcept;

      virtual void defaultWebrtcLogLevel(Log::Level level) noexcept;
      virtual void webrtcLogLevel(Log::Level level) noexcept;

      virtual void startMediaTracing() noexcept;
      virtual void stopMediaTracing() noexcept;
      virtual bool isMediaTracing() noexcept;
      virtual bool saveMediaTrace(String filename) noexcept;
      virtual bool saveMediaTrace(String host, int port) noexcept;
      virtual bool isMRPInstalled() noexcept;

      //---------------------------------------------------------------------
      //
      // ORTC => IORTCForInternal
      //

      virtual void overrideQueueDelegate(IMessageQueuePtr queue) noexcept;
      virtual IMessageQueuePtr queueDelegate() const noexcept;
      virtual IMessageQueuePtr queueORTC() const noexcept;
      virtual IMessageQueuePtr queueMediaDevices() const noexcept;
      virtual IMessageQueuePtr queueRTP() const noexcept;
      virtual IMessageQueuePtr queueORTCPipeline() const noexcept;
      virtual IMessageQueuePtr queueBlockingMediaStartStopThread() const noexcept;
      virtual IMessageQueuePtr queueCertificateGeneration() const noexcept;

      virtual Optional<Log::Level> webrtcLogLevel() const noexcept;

      //---------------------------------------------------------------------
      //
      // ORTC => (internal)
      //

      Log::Params log(const char *message) const noexcept;
      static Log::Params slog(const char *message) noexcept;

      void internalSetup() noexcept;

    protected:
      //---------------------------------------------------------------------
      //
      // ORTC => (data)
      //

      AutoPUID mID;
      ORTCWeakPtr mThisWeak;
      RecursiveLock mLock;

      mutable IMessageQueuePtr mDelegateQueue;
      mutable IMessageQueuePtr mBlockingMediaStartStopThread;
      mutable IMessageQueuePtr mCertificateGeneration;

      mutable IMessageQueuePtr mMediaDeviceQueues[ORTC_QUEUE_TOTAL_MEDIA_DEVICE_THREADS];
      mutable IMessageQueuePtr mRTPQueues[ORTC_QUEUE_TOTAL_RTP_THREADS];
      mutable size_t mNextMediaQueueThread {};
      mutable size_t mNextRTPQueueThread {};

      Milliseconds mNTPServerTime {};

      Optional<Log::Level> mDefaultWebRTCLogLevel{};
      Optional<Log::Level> mWebRTCLogLevel {};
    };
  }
}
