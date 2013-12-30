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

#pragma once

#ifdef __QNX__
#include <slog2.h>
#endif

#include "webrtc/test/channel_transport/include/channel_transport.h"
#include "webrtc/system_wrappers/interface/scoped_ptr.h"

#include <ortc/internal/ortc_MediaEngine.h>
#include <ortc/internal/ortc_Factory.h>

#include <zsLib/Timer.h>

namespace ortc
{
  namespace test
  {
    class TestMediaEngine;
    typedef boost::shared_ptr<TestMediaEngine> TestMediaEnginePtr;
    typedef boost::weak_ptr<TestMediaEngine> TestMediaEngineWeakPtr;
    
    class TestMediaEngineFactory;
    typedef boost::shared_ptr<TestMediaEngineFactory> TestMediaEngineFactoryPtr;
    typedef boost::weak_ptr<TestMediaEngineFactory> TestMediaEngineFactoryWeakPtr;

    typedef webrtc::test::VoiceChannelTransport VoiceChannelTransport;
    typedef webrtc::test::VideoChannelTransport VideoChannelTransport;

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark TestMediaEngine
    #pragma mark
    
    class TestMediaEngine : public internal::MediaEngine, public zsLib::ITimerDelegate
    {
    public:
      friend interaction TestMediaEngineFactory;

      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark TestMediaEngine
      #pragma mark
      
    protected:
      TestMediaEngine(IMessageQueuePtr queue, internal::IMediaEngineDelegatePtr delegate);
      
    public:
      ~TestMediaEngine();

    private:
      internal::Log::Params log(const char *message) const;

    protected:
      static internal::MediaEnginePtr create(internal::IMediaEngineDelegatePtr delegate);
      
    public:
      virtual void setReceiverAddress(String receiverAddress);
      virtual String getReceiverAddress() const;

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark TestMediaEngine => IMediaEngine
      #pragma mark
      
    protected:
      virtual int registerVoiceExternalTransport(Transport &transport);
      virtual int deregisterVoiceExternalTransport();
      virtual int receivedVoiceRTPPacket(const void *data, size_t length);
      virtual int receivedVoiceRTCPPacket(const void *data, size_t length);
      
      virtual int registerVideoExternalTransport(Transport &transport);
      virtual int deregisterVideoExternalTransport();
      virtual int receivedVideoRTPPacket(const void *data, size_t length);
      virtual int receivedVideoRTCPPacket(const void *data, size_t length);
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => ITimerDelegate
      #pragma mark
    protected:
      void onTimer(zsLib::TimerPtr timer);
      
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaEngine => TraceCallback
      #pragma mark
    protected:
      virtual void Print(const TraceLevel level, const char *traceString, const int length);

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark TestMediaEngine => (internal)
      #pragma mark
      
    protected:
      virtual void internalStartVoice();
      virtual void internalStopVoice();
      
      virtual int registerVoiceSendTransport();
      virtual int deregisterVoiceSendTransport();
      virtual int setVoiceSendTransportParameters();
      
      virtual void internalStartVideoCapture();
      virtual void internalStopVideoCapture();
      virtual void internalStartVideoChannel();
      virtual void internalStopVideoChannel();

      virtual int registerVideoSendTransport();
      virtual int deregisterVideoSendTransport();
      virtual int setVideoSendTransportParameters();
      
    private:
      webrtc::scoped_ptr<VoiceChannelTransport> voice_channel_transports_[32];
      webrtc::scoped_ptr<VideoChannelTransport> video_channel_transports_[32];

      String mReceiverAddress;
      zsLib::TimerPtr mVoiceStatisticsTimer;
#ifdef __QNX__
      slog2_buffer_t mBufferHandle;
#endif
    };
    
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark TestMediaEngineFactory
    #pragma mark
    
    class TestMediaEngineFactory : public internal::Factory
    {
    public:
      TestMediaEngineFactory() {}
      
      virtual internal::MediaEnginePtr create(internal::IMediaEngineDelegatePtr delegate);
    };
  }
}
