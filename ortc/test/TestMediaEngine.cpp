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

#ifdef __QNX__
extern char *__progname;
#endif

namespace ortc { namespace test { ZS_DECLARE_SUBSYSTEM(ortclib) } }

namespace ortc { namespace test { ZS_IMPLEMENT_SUBSYSTEM(ortclib) } }

//#define ORTC_MEDIA_ENGINE_ENABLE_TIMER

using namespace std;

namespace ortc
{
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
  TestMediaEngine::TestMediaEngine() :
      MediaEngine(zsLib::Noop()),
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
    String TestMediaEngine::log(const char *message) const
    {
      return String("TestMediaEngine [") + zsLib::string(mID) + "] " + message;
    }

    //-----------------------------------------------------------------------
    internal::MediaEnginePtr TestMediaEngine::create(internal::IMediaEngineDelegatePtr delegate)
    {
      TestMediaEnginePtr pThis(new TestMediaEngine());
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-----------------------------------------------------------------------
    void TestMediaEngine::setLogLevel()
    {
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::setReceiverAddress(String receiverAddress)
    {
      internal::AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("set receiver address - value: ") + receiverAddress)
      
      mReceiverAddress = receiverAddress;
    }
    
    //-----------------------------------------------------------------------
    String TestMediaEngine::getReceiverAddress() const
    {
      internal::AutoRecursiveLock lock(mLock);
      
      ZS_LOG_DEBUG(log("get receiver address - value: ") + mReceiverAddress)
      
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
    int TestMediaEngine::registerVoiceExternalTransport(Transport &transport)
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot be registered")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::deregisterVoiceExternalTransport()
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot be deregistered")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::receivedVoiceRTPPacket(const void *data, size_t length)
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot receive data")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::receivedVoiceRTCPPacket(const void* data, size_t length)
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot receive data")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::registerVideoExternalTransport(Transport &transport)
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot be registered")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::deregisterVideoExternalTransport()
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot be deregistered")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::receivedVideoRTPPacket(const void *data, size_t length)
    {
      ZS_THROW_INVALID_USAGE("external transport is disabled - cannot receive data")
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::receivedVideoRTCPPacket(const void *data, size_t length)
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
        
        mError = mVoiceRtpRtcp->GetRTPStatistics(mVoiceChannel, averageJitterMs, maxJitterMs, discardedPackets);
        if (0 != mError) {
          ZS_LOG_ERROR(Detail, log("failed to get RTP statistics for voice (error: ") + string(mVoiceBase->LastError()) + ")")
          return;
        }
        
        //printf("=== Jitter buffer - Average jitter: %d, Max jitter: %d, Discarded pacets: %d\n", averageJitterMs, maxJitterMs, discardedPackets);
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
#ifndef __QNX__
      printf("%s\n", traceString);
#else
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
    void TestMediaEngine::internalStartVoice()
    {
      internal::MediaEngine::internalStartSendVoice();
      
#ifdef ORTC_MEDIA_ENGINE_ENABLE_TIMER
      mVoiceStatisticsTimer = zsLib::Timer::create(mThisWeak.lock(), zsLib::Seconds(1));
#endif
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStopVoice()
    {
#ifdef ORTC_MEDIA_ENGINE_ENABLE_TIMER
      if (mVoiceStatisticsTimer) {
        mVoiceStatisticsTimer->cancel();
        mVoiceStatisticsTimer.reset();
      }
#endif
      internal::MediaEngine::internalStopSendVoice();
    }

    //-----------------------------------------------------------------------
    int TestMediaEngine::registerVoiceSendTransport()
    {
      voice_channel_transports_[mVoiceChannel].reset( new VoiceChannelTransport(mVoiceNetwork, mVoiceChannel));

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::deregisterVoiceSendTransport()
    {
      voice_channel_transports_[mVideoChannel].reset( NULL );
      
      return 0;
    }

    //-----------------------------------------------------------------------
    int TestMediaEngine::setVoiceSendTransportParameters()
    {
      mError = voice_channel_transports_[mVoiceChannel]->SetSendDestination(mReceiverAddress.c_str(), 20010);
      mError = voice_channel_transports_[mVoiceChannel]->SetLocalReceiver(20010);
      return mError;
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStartVideoCapture()
    {
      MediaEngine::internalStartVideoCapture();
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStopVideoCapture()
    {
      MediaEngine::internalStopVideoCapture();
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStartVideoChannel()
    {
      MediaEngine::internalStartSendVideoChannel();
    }
    
    //-----------------------------------------------------------------------
    void TestMediaEngine::internalStopVideoChannel()
    {
      MediaEngine::internalStopSendVideoChannel();
    }

    //-----------------------------------------------------------------------
    int TestMediaEngine::registerVideoSendTransport()
    {
      video_channel_transports_[mVideoChannel].reset( new VideoChannelTransport(mVideoNetwork, mVideoChannel));

      return 0;
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::deregisterVideoSendTransport()
    {
        video_channel_transports_[mVideoChannel].reset( NULL );
      
        return 0;
    }
    
    //-----------------------------------------------------------------------
    int TestMediaEngine::setVideoSendTransportParameters()
    {
      mError = video_channel_transports_[mVideoChannel]->SetSendDestination(mReceiverAddress.c_str(), 20000);
      mError = video_channel_transports_[mVideoChannel]->SetLocalReceiver(20000);
      return mError;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark TestMediaEngineFactory
    #pragma mark
    
    //-----------------------------------------------------------------------
    internal::MediaEnginePtr TestMediaEngineFactory::createMediaEngine(internal::IMediaEngineDelegatePtr delegate)
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
