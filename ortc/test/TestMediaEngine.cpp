/*
 
 Copyright (c) 2013, SMB Phone Inc.
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

#include "TestMediaEngine.h"
#include <ortc/internal/ortc_ORTC.h>
#include <zsLib/helpers.h>
#include <zsLib/XML.h>
#include <openpeer/services/IHelper.h>

#ifdef __QNX__
extern char *__progname;
#endif

namespace ortc { namespace test { ZS_DECLARE_SUBSYSTEM(ortclib) } }

namespace ortc { namespace test { ZS_IMPLEMENT_SUBSYSTEM(ortclib) } }

//#define ORTC_MEDIA_ENGINE_ENABLE_TIMER

using namespace std;

namespace ortc
{
  typedef openpeer::services::IHelper OPIHelper;
  
  namespace test
  {
    using zsLib::string;
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark TestMediaEngine
    #pragma mark

  //-----------------------------------------------------------------------
  TestMediaEngine::TestMediaEngine(IMessageQueuePtr queue, internal::IMediaEngineDelegatePtr delegate) :
      MediaEngine(zsLib::Noop(), queue, delegate),
      mReceiverAddress("127.0.0.1")
  {
#ifdef __QNX__
    slog2_buffer_set_config_t buffer_config;
    slog2_buffer_t buffer_handle;
    buffer_config.buffer_set_name = __progname;
    buffer_config.num_buffers = 1;
    buffer_config.verbosity_level = SLOG2_INFO;
    buffer_config.buffer_config[0].buffer_name = "media_logger";
    buffer_config.buffer_config[0].num_pages = 7;
    if (-1 == slog2_register(&buffer_config, &buffer_handle, 0)) {
        fprintf(stderr, "Error registering slogger2 buffer!\n");
    }
    mBufferHandle = buffer_handle;
#endif
    }
    
    //-----------------------------------------------------------------------
    TestMediaEngine::~TestMediaEngine()
    {
      destroyMediaEngine();
    }
    
    //-----------------------------------------------------------------------
    internal::Log::Params TestMediaEngine::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::TestMediaEngine");
      OPIHelper::debugAppend(objectEl, "id", mID);
      return internal::Log::Params(message, objectEl);
    }

    //-----------------------------------------------------------------------
    internal::MediaEnginePtr TestMediaEngine::create(internal::IMediaEngineDelegatePtr delegate)
    {
      TestMediaEnginePtr pThis = TestMediaEnginePtr(new TestMediaEngine(internal::IORTCForInternal::queueBlockingMediaStartStopThread(), delegate));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::setReceiverAddress(String receiverAddress)
    {
      internal::AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("set receiver address") + ZS_PARAM("value", receiverAddress))
      
      mReceiverAddress = receiverAddress;
    }
    
    //-----------------------------------------------------------------------
    String TestMediaEngine::getReceiverAddress() const
    {
      internal::AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("get receiver address") + ZS_PARAM("value", mReceiverAddress))
      
      return mReceiverAddress;
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark TestMediaEngine => IMediaEngine
    #pragma mark
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::registerVoiceExternalTransport(int channelId, Transport &transport)
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot be registered")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::deregisterVoiceExternalTransport(int channelId)
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot be deregistered")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::receivedVoiceRTPPacket(int channelId, const void *data, size_t length)
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot receive data")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::receivedVoiceRTCPPacket(int channelId, const void* data, size_t length)
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot receive data")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::registerVideoExternalTransport(int channelId, Transport &transport)
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot be registered")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::deregisterVideoExternalTransport(int channelId)
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot be deregistered")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::receivedVideoRTPPacket(int channelId, const void *data, size_t length)
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot receive data")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::receivedVideoRTCPPacket(int channelId, const void *data, size_t length)
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot receive data")
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => ITimerDelegate
    #pragma mark
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::onTimer(zsLib::TimerPtr timer)
    {
      internal::AutoRecursiveLock lock(mLock);
      
      if (timer == mVoiceStatisticsTimer)
      {
        unsigned int averageJitterMs;
        unsigned int maxJitterMs;
        unsigned int discardedPackets;
        
//        get(mLastError) = mVoiceRtpRtcp->GetRTPStatistics(mVoiceChannel, averageJitterMs, maxJitterMs, discardedPackets);
//        if (0 != mLastError) {
//          ZS_LOG_ERROR(Detail, log("failed to get RTP statistics for voice") + ZS_PARAM("error", mVoiceBase->LastError()))
//          return;
//        }
        
//        printf("=== Jitter buffer - Average jitter: %d, Max jitter: %d, Discarded pacets: %d\n", averageJitterMs, maxJitterMs, discardedPackets);
      }
    }
    
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaEngine => TraceCallback
    #pragma mark
    //-----------------------------------------------------------------------
    void TestMediaEngine::Print(const webrtc::TraceLevel level, const char *traceString, const int length)
    {
#ifdef __QNX__
      slog2f(mBufferHandle, 0, SLOG2_INFO, "%s", traceString);
#endif

      MediaEngine::Print(level, traceString, length);
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark TestMediaEngine => (internal)
    #pragma mark
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStartSendVoice(int channelId)
    {
      internal::MediaEngine::internalStartSendVoice(channelId);
      
#ifdef ORTC_MEDIA_ENGINE_ENABLE_TIMER
      mVoiceStatisticsTimer = zsLib::Timer::create(mThisWeak.lock(), zsLib::Seconds(1));
#endif
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStartReceiveVoice(int channelId)
    {
      internal::MediaEngine::internalStartReceiveVoice(channelId);
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStopSendVoice(int channelId)
    {
#ifdef ORTC_MEDIA_ENGINE_ENABLE_TIMER
      if (mVoiceStatisticsTimer) {
        mVoiceStatisticsTimer->cancel();
        mVoiceStatisticsTimer.reset();
      }
#endif
      internal::MediaEngine::internalStopSendVoice(channelId);
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStopReceiveVoice(int channelId)
    {
      internal::MediaEngine::internalStopReceiveVoice(channelId);
    }

    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStartVideoCapture(int captureId)
    {
      MediaEngine::internalStartVideoCapture(captureId);
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStopVideoCapture(int captureId)
    {
      MediaEngine::internalStopVideoCapture(captureId);
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStartSendVideoChannel(int channelId, int captureId)
    {
      MediaEngine::internalStartSendVideoChannel(channelId, captureId);
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStartReceiveVideoChannel(int channelId)
    {
      MediaEngine::internalStartReceiveVideoChannel(channelId);
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStopSendVideoChannel(int channelId)
    {
      MediaEngine::internalStopSendVideoChannel(channelId);
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStopReceiveVideoChannel(int channelId)
    {
      MediaEngine::internalStopReceiveVideoChannel(channelId);
    }

    //-----------------------------------------------------------------------
    int TestMediaEngine::internalRegisterVoiceSendTransport(int channelId)
    {
      voice_channel_transports_[channelId].reset(new VoiceChannelTransport(mVoiceNetwork, channelId));
      
      return 0;
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::internalDeregisterVoiceSendTransport(int channelId)
    {
      voice_channel_transports_[channelId].reset( NULL );
      
      return 0;
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::internalSetVoiceSendTransportParameters(int channelId)
    {
      get(mLastError) = voice_channel_transports_[channelId]->SetSendDestination(mReceiverAddress.c_str(), 20010);
      get(mLastError) = voice_channel_transports_[channelId]->SetLocalReceiver(20010);
      return mLastError;
    }
    
    int TestMediaEngine::internalSetVoiceReceiveTransportParameters(int channelId)
    {
      return 0;
    }

    //-----------------------------------------------------------------------
    int TestMediaEngine::internalRegisterVideoSendTransport(int channelId)
    {
      video_channel_transports_[channelId].reset(new VideoChannelTransport(mVideoNetwork, channelId));

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::internalDeregisterVideoSendTransport(int channelId)
    {
        video_channel_transports_[channelId].reset( NULL );
      
        return 0;
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::internalSetVideoSendTransportParameters(int channelId)
    {
      get(mLastError) = video_channel_transports_[channelId]->SetSendDestination(mReceiverAddress.c_str(), 20000);
      get(mLastError) = video_channel_transports_[channelId]->SetLocalReceiver(20000);
      return mLastError;
    }
    
    int TestMediaEngine::internalSetVideoReceiveTransportParameters(int channelId)
    {
      return 0;
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark TestMediaEngineFactory
    #pragma mark
    
    //-----------------------------------------------------------------------
    internal::MediaEnginePtr TestMediaEngineFactory::create(internal::IMediaEngineDelegatePtr delegate)
    {
      return TestMediaEngine::create(delegate);
    }
  }
}

using ortc::test::TestMediaEngineFactory;
using ortc::test::TestMediaEngineFactoryPtr;
using ortc::test::TestMediaEngine;
using ortc::test::TestMediaEnginePtr;

void doMediaEngineTest()
{
  TestMediaEngineFactoryPtr overrideFactory(new TestMediaEngineFactory);
  
  ortc::internal::Factory::override(overrideFactory);
}
