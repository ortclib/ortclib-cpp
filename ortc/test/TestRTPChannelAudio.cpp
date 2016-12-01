/*
 
 Copyright (c) 2016, Hookflash Inc.
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


#include "TestRTPChannelAudio.h"

#include <ortc/internal/ortc_RTPPacket.h>
#include <ortc/internal/ortc_RTCPPacket.h>
#include <ortc/IRTPTypes.h>

#include <zsLib/IMessageQueueThread.h>
#include <zsLib/ISettings.h>
#include <zsLib/XML.h>

#include "config.h"
#include "testing.h"

#include <webrtc/base/logging.h>
#include <webrtc/system_wrappers/include/trace.h>
#include <webrtc/modules/audio_device/audio_device_impl.h>

namespace ortc { namespace test { ZS_DECLARE_SUBSYSTEM(ortc_test) } }

using zsLib::String;
using zsLib::ULONG;
using zsLib::PTRNUMBER;
using zsLib::IMessageQueue;
using zsLib::Log;
using zsLib::AutoPUID;
using zsLib::AutoRecursiveLock;
using zsLib::IPromiseResolutionDelegate;
using namespace zsLib::XML;

ZS_DECLARE_TYPEDEF_PTR(zsLib::ISettings, UseSettings)
ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper)

namespace ortc
{
  namespace test
  {
    namespace rtpchannelaudio
    {
      ZS_DECLARE_CLASS_PTR(FakeMediaStreamTrack)
      ZS_DECLARE_CLASS_PTR(FakeReceiverChannel)
      ZS_DECLARE_CLASS_PTR(FakeSenderChannel)
      ZS_DECLARE_CLASS_PTR(RTPChannelAudioTester)
      
      ZS_DECLARE_USING_PTR(ortc::internal, MediaStreamTrack)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiverChannel)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiverChannelAudio)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSenderChannel)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSenderChannelAudio)
      
      using zsLib::AutoRecursiveLock;

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark WebRtcTraceCallback
      #pragma mark

      class WebRtcTraceCallback : public webrtc::TraceCallback
      {
      public:

        virtual void Print(webrtc::TraceLevel level, const char* message, int length)
        {
          rtc::LoggingSeverity sev = rtc::LS_VERBOSE;
          if (level == webrtc::kTraceError || level == webrtc::kTraceCritical)
            sev = rtc::LS_ERROR;
          else if (level == webrtc::kTraceWarning)
            sev = rtc::LS_WARNING;
          else if (level == webrtc::kTraceStateInfo || level == webrtc::kTraceInfo)
            sev = rtc::LS_INFO;
          else if (level == webrtc::kTraceTerseInfo)
            sev = rtc::LS_INFO;

          // Skip past boilerplate prefix text
          if (length < 72) {
            std::string msg(message, length);
            LOG(LS_ERROR) << "Malformed webrtc log message: ";
            LOG_V(sev) << msg;
          } else {
            std::string msg(message + 71, length - 72);
            LOG_V(sev) << "webrtc: " << msg;
          }
        }
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack
      #pragma mark
      
      //-----------------------------------------------------------------------
      FakeMediaStreamTrack::FakeMediaStreamTrack(IMessageQueuePtr queue, bool remote) :
        MediaStreamTrack(Noop(true), queue),
        mRemote(remote)
      {
        mAudioDeviceModule =
          webrtc::AudioDeviceModuleImpl::Create(1, webrtc::AudioDeviceModule::kWindowsWasapiAudio);
        if (!mAudioDeviceModule) {
          ZS_LOG_ERROR(Detail, log("cannot create AudioDeviceModule"))
          return;
        }

        mAudioDeviceModule->AddRef();
        mAudioDeviceModule->Init();

        const char* recordingDeviceID = NULL;
        const char* playoutDeviceID = NULL;

        int numMics = mAudioDeviceModule->RecordingDevices();
        for (int index = 0; index < numMics; ++index) {
          char deviceName[webrtc::kAdmMaxDeviceNameSize];
          char deviceGuid[webrtc::kAdmMaxGuidSize];
          if (mAudioDeviceModule->RecordingDeviceName(index, deviceName, deviceGuid) != -1) {
            recordingDeviceID = deviceGuid;
            break;
          }
        }

        int numSpeaks = mAudioDeviceModule->PlayoutDevices();
        for (int index = 0; index < numSpeaks; ++index) {
          char deviceName[webrtc::kAdmMaxDeviceNameSize];
          char deviceGuid[webrtc::kAdmMaxGuidSize];
          if (mAudioDeviceModule->PlayoutDeviceName(index, deviceName, deviceGuid) != -1) {
            playoutDeviceID = deviceGuid;
            break;
          }
        }
      }

      //-----------------------------------------------------------------------
      FakeMediaStreamTrack::~FakeMediaStreamTrack()
      {
        mThisWeak.reset();
      }

      //-----------------------------------------------------------------------
      FakeMediaStreamTrackPtr FakeMediaStreamTrack::create(
                                                           IMessageQueuePtr queue,
                                                           bool remote
                                                           )
      {
        FakeMediaStreamTrackPtr pThis(make_shared<FakeMediaStreamTrack>(queue, remote));
        pThis->mThisWeak = pThis;
        return pThis;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => IMediaStreamTrackForRTPReceiverChannelAudio
      #pragma mark
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => IMediaStreamTrackForRTPSenderChannelAudio
      #pragma mark
#if 0
      webrtc::AudioDeviceModule* FakeMediaStreamTrack::getAudioDeviceModule()
      {
        return mAudioDeviceModule;
      }

      void FakeMediaStreamTrack::start()
      {
        AutoRecursiveLock lock(*this);
        if (!mRemote) {
          mAudioDeviceModule->SetRecordingDevice(webrtc::AudioDeviceModule::kDefaultCommunicationDevice);
          mAudioDeviceModule->InitRecording();
          mAudioDeviceModule->StartRecording();
        } else {
          mAudioDeviceModule->SetPlayoutDevice(webrtc::AudioDeviceModule::kDefaultCommunicationDevice);
          mAudioDeviceModule->InitPlayout();
          mAudioDeviceModule->StartPlayout();
        }
      }
#endif
      void FakeMediaStreamTrack::stop()
      {
        AutoRecursiveLock lock(*this);
        if (mAudioDeviceModule) {
          if (!mRemote)
            mAudioDeviceModule->StopRecording();
          else
            mAudioDeviceModule->StopPlayout();
          mAudioDeviceModule->RegisterAudioCallback(nullptr);
          mAudioDeviceModule->Terminate();
        }
      }

      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => webrtc::AudioTransport
      #pragma mark

      //-------------------------------------------------------------------------
      int32_t FakeMediaStreamTrack::RecordedDataIsAvailable(
                                                            const void* audioSamples,
                                                            const size_t nSamples,
                                                            const size_t nBytesPerSample,
                                                            const uint8_t nChannels,
                                                            const uint32_t samplesPerSec,
                                                            const uint32_t totalDelayMS,
                                                            const int32_t clockDrift,
                                                            const uint32_t currentMicLevel,
                                                            const bool keyPressed,
                                                            uint32_t& newMicLevel
                                                            )
      {
        auto senderChannel = mSenderChannel.lock();
        if (senderChannel) return senderChannel->sendAudioSamples(audioSamples, nSamples, nChannels);
        return 0;
      }

      //-------------------------------------------------------------------------
      int32_t FakeMediaStreamTrack::NeedMorePlayData(
                                                     const size_t nSamples,
                                                     const size_t nBytesPerSample,
                                                     const uint8_t nChannels,
                                                     const uint32_t samplesPerSec,
                                                     void* audioSamples,
                                                     size_t& nSamplesOut,
                                                     int64_t* elapsed_time_ms,
                                                     int64_t* ntp_time_ms
                                                     )
      {
        auto receiverChannel = mReceiverChannel.lock();
        if (receiverChannel) return receiverChannel->getAudioSamples(nSamples, nChannels, audioSamples, nSamplesOut);
        return 0;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => (friend RTPChannelTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::setTransport(RTPChannelAudioTesterPtr tester)
      {
        AutoRecursiveLock lock(*this);
        mTester = tester;
      }

      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::linkReceiverChannel(UseReceiverChannelPtr channel)
      {
        AutoRecursiveLock lock(*this);
        mReceiverChannel = channel;
      }

      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::linkSenderChannel(UseSenderChannelPtr channel)
      {
        AutoRecursiveLock lock(*this);
        mSenderChannel = channel;
      }

      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::linkReceiverChannelAudio(UseReceiverChannelAudioPtr channelAudio)
      {
        AutoRecursiveLock lock(*this);
        mReceiverChannelAudio = channelAudio;
      }

      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::linkSenderChannelAudio(UseSenderChannelAudioPtr channelAudio)
      {
        AutoRecursiveLock lock(*this);
        mSenderChannelAudio = channelAudio;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => (internal)
      #pragma mark
      
      //-----------------------------------------------------------------------
      Log::Params FakeMediaStreamTrack::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpchannelaudio::FakeMediaStreamTrack");
        UseServicesHelper::debugAppend(objectEl, "id", MediaStreamTrack::getID());
        return Log::Params(message, objectEl);
      }
      
      //-----------------------------------------------------------------------
      ElementPtr FakeMediaStreamTrack::toDebug() const
      {
        AutoRecursiveLock lock(*this);
        
        ElementPtr result = Element::create("ortc::test::rtpchannelaudio::FakeMediaStreamTrack");
        
        UseServicesHelper::debugAppend(result, "kind", IMediaStreamTrackTypes::toString(mKind));
        
        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());
        
        return result;
      }
      
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel
      #pragma mark
      
      //-----------------------------------------------------------------------
      FakeReceiverChannel::FakeReceiverChannel(
                                               IMessageQueuePtr queue,
                                               const Parameters &params,
                                               UseMediaStreamTrackPtr track
                                               ) :
          RTPReceiverChannel(Noop(true), queue),
          mParameters(make_shared<Parameters>(params)),
          mTrack(track)
      {
      }
      
      //-----------------------------------------------------------------------
      FakeReceiverChannel::~FakeReceiverChannel()
      {
        mThisWeak.reset();
      }
      
      //-----------------------------------------------------------------------
      FakeReceiverChannelPtr FakeReceiverChannel::create(
                                                         IMessageQueuePtr queue,
                                                         const Parameters &params,
                                                         UseMediaStreamTrackPtr track
                                                         )
      {
        FakeReceiverChannelPtr pThis(make_shared<FakeReceiverChannel>(queue, params, track));
        pThis->mThisWeak = pThis;
        return pThis;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel => IRTPReceiverForRTPReceiverChannelBase
      #pragma mark

      //-----------------------------------------------------------------------
      bool FakeReceiverChannel::sendPacket(RTCPPacketPtr packet)
      {
        mTester.lock()->sendToConnectedTester(packet);
        return true;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel => IRTPReceiverForRTPReceiverChannelAudio
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel => IRTPReceiverChannelForMediaStreamTrack
      #pragma mark
      
      //-----------------------------------------------------------------------
      ElementPtr FakeReceiverChannel::toDebug() const
      {
        AutoRecursiveLock lock(*this);
        
        ElementPtr result = Element::create("ortc::test::rtpchannelaudio::FakeReceiverChannel");
        
        UseServicesHelper::debugAppend(result, "kind", IMediaStreamTrackTypes::toString(mKind));
        
        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());
        
        UseServicesHelper::debugAppend(result, mParameters ? mParameters->toDebug() : ElementPtr());

        return result;
      }
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel => (friend RTPSenderTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeReceiverChannel::setTransport(RTPChannelAudioTesterPtr tester)
      {
        AutoRecursiveLock lock(*this);
        mTester = tester;
      }

      //-----------------------------------------------------------------------
      void FakeReceiverChannel::linkChannelAudio(UseReceiverChannelAudioPtr channelAudio)
      {
        AutoRecursiveLock lock(*this);
        mReceiverChannelAudio = channelAudio;
      }

      //-----------------------------------------------------------------------
      void FakeReceiverChannel::linkMediaStreamTrack(UseMediaStreamTrackPtr track)
      {
        AutoRecursiveLock lock(*this);
        mTrack = track;
      }

      //-----------------------------------------------------------------------
      bool FakeReceiverChannel::handlePacket(RTPPacketPtr packet)
      {
        if (mReceiverChannelAudio)
          mReceiverChannelAudio->handlePacket(packet);
        return true;
      }

      //-----------------------------------------------------------------------
      bool FakeReceiverChannel::handlePacket(RTCPPacketPtr packet)
      {
        if (mReceiverChannelAudio)
          mReceiverChannelAudio->handlePacket(packet);
        return true;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel => (internal)
      #pragma mark
      
      //-----------------------------------------------------------------------
      Log::Params FakeReceiverChannel::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpchannelaudio::FakeReceiverChannel");
        UseServicesHelper::debugAppend(objectEl, "id", FakeReceiverChannel::getID());
        return Log::Params(message, objectEl);
      }
      
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel
      #pragma mark
      
      //-----------------------------------------------------------------------
      FakeSenderChannel::FakeSenderChannel(
                                           IMessageQueuePtr queue,
                                           const Parameters &params,
                                           UseMediaStreamTrackPtr track
                                           ) :
          RTPSenderChannel(Noop(true), queue),
          mParameters(make_shared<Parameters>(params)),
          mTrack(track)
      {
      }
      
      //-----------------------------------------------------------------------
      FakeSenderChannel::~FakeSenderChannel()
      {
        mThisWeak.reset();
      }
      
      //-----------------------------------------------------------------------
      FakeSenderChannelPtr FakeSenderChannel::create(
                                                     IMessageQueuePtr queue,
                                                     const Parameters &params,
                                                     UseMediaStreamTrackPtr track
                                                     )
      {
        FakeSenderChannelPtr pThis(make_shared<FakeSenderChannel>(queue, params, track));
        pThis->mThisWeak = pThis;
        return pThis;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel => IRTPSenderChannelForRTPSenderChannelBase
      #pragma mark

      //-----------------------------------------------------------------------
      bool FakeSenderChannel::sendPacket(RTPPacketPtr packet)
      {
        mTester.lock()->sendToConnectedTester(packet);
        return true;
      }

      //-----------------------------------------------------------------------
      bool FakeSenderChannel::sendPacket(RTCPPacketPtr packet)
      {
        mTester.lock()->sendToConnectedTester(packet);
        return false;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel => IRTPSenderChannelForRTPSenderChannelAudio
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel => IRTPSenderChannelForMediaStreamTrack
      #pragma mark
      
      //-----------------------------------------------------------------------
      ElementPtr FakeSenderChannel::toDebug() const
      {
        AutoRecursiveLock lock(*this);
        
        ElementPtr result = Element::create("ortc::test::rtpchannelaudio::FakeSenderChannel");
        
        UseServicesHelper::debugAppend(result, "kind", IMediaStreamTrackTypes::toString(mKind));
        
        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());
        
        UseServicesHelper::debugAppend(result, mParameters ? mParameters->toDebug() : ElementPtr());
        
        return result;
      }
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel => (friend RTPChannelTester)
      #pragma mark

      void FakeSenderChannel::setTransport(RTPChannelAudioTesterPtr tester)
      {
        AutoRecursiveLock lock(*this);
        mTester = tester;
      }

      void FakeSenderChannel::linkChannelAudio(UseSenderChannelAudioPtr channelAudio)
      {
        AutoRecursiveLock lock(*this);
        mSenderChannelAudio = channelAudio;
      }

      void FakeSenderChannel::linkMediaStreamTrack(UseMediaStreamTrackPtr track)
      {
        AutoRecursiveLock lock(*this);
        mTrack = track;
      }

      bool FakeSenderChannel::handlePacket(RTCPPacketPtr packet)
      {
        if (mSenderChannelAudio)
          mSenderChannelAudio->handlePacket(packet);
        return true;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel => (internal)
      #pragma mark
      
      //-----------------------------------------------------------------------
      Log::Params FakeSenderChannel::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpchannelaudio::FakeSenderChannel");
        UseServicesHelper::debugAppend(objectEl, "id", FakeSenderChannel::getID());
        return Log::Params(message, objectEl);
      }
      
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelAudioTester::Expectations
      #pragma mark
      
      //-----------------------------------------------------------------------
      bool RTPChannelAudioTester::Expectations::operator==(const Expectations &op2) const
      {
        return (mReceivedPackets == op2.mReceivedPackets) &&
        
        (mSenderChannelConflict == op2.mSenderChannelConflict) &&
        (mSenderChannelError == op2.mSenderChannelError);
      }
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelAudioTester::OverrideReceiverChannelAudioFactory
      #pragma mark
      
      //-----------------------------------------------------------------------
      RTPChannelAudioTester::OverrideReceiverChannelAudioFactoryPtr RTPChannelAudioTester::OverrideReceiverChannelAudioFactory::create(RTPChannelAudioTesterPtr tester)
      {
        OverrideReceiverChannelAudioFactoryPtr pThis(make_shared<OverrideReceiverChannelAudioFactory>());
        pThis->mTester = tester;
        return pThis;
      }
      
      //-----------------------------------------------------------------------
      RTPReceiverChannelAudioPtr RTPChannelAudioTester::OverrideReceiverChannelAudioFactory::create(
                                                                                                    RTPReceiverChannelPtr receiverChannel,
                                                                                                    MediaStreamTrackPtr track,
                                                                                                    const Parameters &params
                                                                                                    )
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)
        
        return tester->createReceiverChannelAudio(receiverChannel, track, params);
      }
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelAudioTester::OverrideSenderChannelAudioFactory
      #pragma mark
      
      //-----------------------------------------------------------------------
      RTPChannelAudioTester::OverrideSenderChannelAudioFactoryPtr RTPChannelAudioTester::OverrideSenderChannelAudioFactory::create(RTPChannelAudioTesterPtr tester)
      {
        OverrideSenderChannelAudioFactoryPtr pThis(make_shared<OverrideSenderChannelAudioFactory>());
        pThis->mTester = tester;
        return pThis;
      }
      
      //-----------------------------------------------------------------------
      RTPSenderChannelAudioPtr RTPChannelAudioTester::OverrideSenderChannelAudioFactory::create(
                                                                                                RTPSenderChannelPtr senderChannel,
                                                                                                MediaStreamTrackPtr track,
                                                                                                const Parameters &params
                                                                                                )
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)
        
        return tester->createSenderChannelAudio(senderChannel, track, params);
      }
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelAudioTester
      #pragma mark
      
      //-----------------------------------------------------------------------
      RTPChannelAudioTesterPtr RTPChannelAudioTester::create(
                                                             IMessageQueuePtr queue,
                                                             bool overrideFactories
                                                             )
      {
        RTPChannelAudioTesterPtr pThis(new RTPChannelAudioTester(queue, overrideFactories));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }
      
      //-----------------------------------------------------------------------
      RTPChannelAudioTester::RTPChannelAudioTester(
                                                   IMessageQueuePtr queue,
                                                   bool overrideFactories
                                                   ) :
        SharedRecursiveLock(SharedRecursiveLock::create()),
        MessageQueueAssociator(queue),
        mOverrideFactories(overrideFactories)
      {
        ZS_LOG_BASIC(log("rtpchannelaudio tester"))
      }
      
      //-----------------------------------------------------------------------
      RTPChannelAudioTester::~RTPChannelAudioTester()
      {
        ZS_LOG_BASIC(log("rtpchannelaudio tester"))
        
        if (mOverrideFactories) {
          RTPReceiverChannelAudioFactory::override(RTPReceiverChannelAudioFactoryPtr());
          RTPSenderChannelAudioFactory::override(RTPSenderChannelAudioFactoryPtr());
        }
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::init()
      {
        AutoRecursiveLock lock(*this);
        
        if (mOverrideFactories) {
          RTPReceiverChannelAudioFactory::override(OverrideReceiverChannelAudioFactory::create(mThisWeak.lock()));
          RTPSenderChannelAudioFactory::override(OverrideSenderChannelAudioFactory::create(mThisWeak.lock()));
        }
      }
      
      //-----------------------------------------------------------------------
      bool RTPChannelAudioTester::matches()
      {
        AutoRecursiveLock lock(*this);
        return mExpectationsFound == mExpecting;
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::close()
      {
        AutoRecursiveLock lock(*this);
        
        for (auto iter = mReceiverAudioChannels.begin(); iter != mReceiverAudioChannels.end(); ++iter)
        {
          UseReceiverChannelAudioForReceiverChannelPtr receiverChannel = (*iter).second;
        }
        
        mReceiverAudioChannels.clear();
        
        for (auto iter = mAttached.begin(); iter != mAttached.end(); ++iter) {
          auto &receiverChannel = (*iter).second.first;
          auto &senderChannel = (*iter).second.second;
          
          if (receiverChannel) {
          }
          if (senderChannel) {
          }
        }
        
        mAttached.clear();
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::closeByReset()
      {
        AutoRecursiveLock lock(*this);
        
        mReceiverAudioChannels.clear();
        mSenderAudioChannels.clear();
      }
      
      //-----------------------------------------------------------------------
      RTPChannelAudioTester::Expectations RTPChannelAudioTester::getExpectations() const
      {
        return mExpectationsFound;
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::connect(RTPChannelAudioTesterPtr remote)
      {
        AutoRecursiveLock lock(*this);
        AutoRecursiveLock lock2(*remote);
        
        TESTING_CHECK(remote)
        
        mConnectedTester = remote;
        remote->mConnectedTester = mThisWeak.lock();
      }

      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::createMediaStreamTrack(
                                                         const char *trackID,
                                                         bool remote
                                                         )
      {
        FakeMediaStreamTrackPtr track = getMediaStreamTrack(trackID);

        if (!track) {
          track = FakeMediaStreamTrack::create(getAssociatedMessageQueue(), remote);
          track->setTransport(mThisWeak.lock());
          attach(trackID, track);
        }

        TESTING_CHECK(track)
      }

      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::createReceiverChannel(
                                                        const char *receiverChannelID,
                                                        const char *mediaStreamTrackID,
                                                        const char *parametersID
                                                        )
      {
        FakeReceiverChannelPtr receiverChannel = getReceiverChannel(receiverChannelID);
        
        if (!receiverChannel) {
          auto params = getParameters(parametersID);
          //TESTING_CHECK(params)
          if (!params)
            params.reset(new Parameters());
          auto track = getMediaStreamTrack(mediaStreamTrackID);
          TESTING_CHECK(track)
          receiverChannel = FakeReceiverChannel::create(getAssociatedMessageQueue(), *params, track);
          receiverChannel->setTransport(mThisWeak.lock());
          track->linkReceiverChannel(receiverChannel);
          attach(receiverChannelID, receiverChannel);
        }
        
        TESTING_CHECK(receiverChannel)
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::createSenderChannel(
                                                      const char *senderChannelID,
                                                      const char *mediaStreamTrackID,
                                                      const char *parametersID
                                                      )
      {
        FakeSenderChannelPtr senderChannel = getSenderChannel(senderChannelID);
        
        if (!senderChannel) {
          auto params = getParameters(parametersID);
          //TESTING_CHECK(params)
          if (!params)
            params.reset(new Parameters());
          auto track = getMediaStreamTrack(mediaStreamTrackID);
          TESTING_CHECK(track)
          senderChannel = FakeSenderChannel::create(getAssociatedMessageQueue(), *params, track);
          senderChannel->setTransport(mThisWeak.lock());
          track->linkSenderChannel(senderChannel);
          attach(senderChannelID, senderChannel);
        }
        
        TESTING_CHECK(senderChannel)
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::createReceiverChannelAudio(
                                                             const char *receiverChannelID,
                                                             const char *receiverChannelAudioID,
                                                             const char *mediaStreamTrackID,
                                                             const char *parametersID
                                                             )
      {
        RTPReceiverChannelAudioPtr receiverChannelAudio = getReceiverChannelAudio(receiverChannelAudioID);
        
        if (!receiverChannelAudio) {
          auto params = getParameters(parametersID);
          //TESTING_CHECK(params)
          if (!params)
            params.reset(new Parameters());
          auto receiverChannel = getReceiverChannel(receiverChannelID);
          TESTING_CHECK(receiverChannel)
          auto track = getMediaStreamTrack(mediaStreamTrackID);
          TESTING_CHECK(track)
          receiverChannelAudio = UseReceiverChannelAudioForReceiverChannel::create(receiverChannel, track, *params);
          receiverChannel->linkChannelAudio(receiverChannelAudio);
          track->linkReceiverChannelAudio(receiverChannelAudio);
          attach(receiverChannelAudioID, receiverChannelAudio);
        }
        
        TESTING_CHECK(receiverChannelAudio)
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::createSenderChannelAudio(
                                                           const char *senderChannelID,
                                                           const char *senderChannelAudioID,
                                                           const char *mediaStreamTrackID,
                                                           const char *parametersID
                                                           )
      {
        RTPSenderChannelAudioPtr senderChannelAudio = getSenderChannelAudio(senderChannelAudioID);
        
        if (!senderChannelAudio) {
          auto params = getParameters(parametersID);
          //TESTING_CHECK(params)
          if (!params)
            params.reset(new Parameters());
          auto senderChannel = getSenderChannel(senderChannelID);
          TESTING_CHECK(senderChannel)
          auto track = getMediaStreamTrack(mediaStreamTrackID);
          TESTING_CHECK(track)
          senderChannelAudio = UseSenderChannelAudioForSenderChannel::create(senderChannel, track, *params);
          senderChannel->linkChannelAudio(senderChannelAudio);
          track->linkSenderChannelAudio(senderChannelAudio);
          attach(senderChannelAudioID, senderChannelAudio);
        }
        
        TESTING_CHECK(senderChannelAudio)
      }

      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::send(
                                       const char *senderChannelID,
                                       const char *parametersID
                                       )
      {
        AutoRecursiveLock lock(*this);
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::receive(
                                          const char *receiverChannelID,
                                          const char *parametersID
                                          )
      {
        AutoRecursiveLock lock(*this);
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::stop(const char *senderOrReceiverChannelID)
      {
        AutoRecursiveLock lock(*this);
      }

      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::attach(
                                         const char *trackID,
                                         FakeMediaStreamTrackPtr mediaStreamTrack
                                         )
      {
        TESTING_CHECK(mediaStreamTrack)

        String trackIDStr(trackID);

        AutoRecursiveLock lock(*this);

        auto found = mMediaStreamTracks.find(trackIDStr);

        if (found != mMediaStreamTracks.end()) {
          auto &previousTrack = (*found).second;
          if (previousTrack) {
          }

          previousTrack = mediaStreamTrack;
          return;
        }

        mMediaStreamTracks[trackIDStr] = mediaStreamTrack;
      }

      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::attach(
                                         const char *receiverChannelID,
                                         FakeReceiverChannelPtr receiverChannel
                                         )
      {
        TESTING_CHECK(receiverChannel)

        String receiverChannelIDStr(receiverChannelID);

        AutoRecursiveLock lock(*this);

        RTPReceiverChannelPtr oldChannel;

        auto found = mReceiverChannels.find(receiverChannelIDStr);

        if (found != mReceiverChannels.end()) {
          auto &previousReceiverChannel = (*found).second;
          if (previousReceiverChannel) {
          }

          previousReceiverChannel = receiverChannel;
          return;
        }

        mReceiverChannels[receiverChannelIDStr] = receiverChannel;
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::attach(
                                         const char *senderChannelID,
                                         FakeSenderChannelPtr senderChannel
                                         )
      {
        TESTING_CHECK(senderChannel)

        String senderChannelIDStr(senderChannelID);

        AutoRecursiveLock lock(*this);

        RTPReceiverChannelPtr oldChannel;

        auto found = mSenderChannels.find(senderChannelIDStr);

        if (found != mSenderChannels.end()) {
          auto &previousSenderChannel = (*found).second;
          if (previousSenderChannel) {
          }

          previousSenderChannel = senderChannel;
          return;
        }

        mSenderChannels[senderChannelIDStr] = senderChannel;
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::attach(
                                         const char *receiverChannelAudioID,
                                         RTPReceiverChannelAudioPtr receiverChannelAudio
                                         )
      {
        TESTING_CHECK(receiverChannelAudio)
        
        String receiverChannelAudioIDStr(receiverChannelAudioID);
        
        AutoRecursiveLock lock(*this);
        
        FakeReceiverChannelPtr oldChannelAudio;
        
        auto found = mReceiverAudioChannels.find(receiverChannelAudioIDStr);
        
        if (found != mReceiverAudioChannels.end()) {
          auto &previousReceiverChannelAudio = (*found).second;
          previousReceiverChannelAudio = receiverChannelAudio;
          return;
        }
        
        mReceiverAudioChannels[receiverChannelAudioIDStr] = receiverChannelAudio;
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::attach(
                                         const char *senderChannelAudioID,
                                         RTPSenderChannelAudioPtr senderChannelAudio
                                         )
      {
        TESTING_CHECK(senderChannelAudio)
        
        String senderChannelAudioIDStr(senderChannelAudioID);
        
        AutoRecursiveLock lock(*this);
        
        FakeSenderChannelPtr oldChannelAudio;
        
        auto found = mSenderAudioChannels.find(senderChannelAudioIDStr);
        
        if (found != mSenderAudioChannels.end()) {
          auto &previousSenderChannelAudio = (*found).second;
          previousSenderChannelAudio = senderChannelAudio;
          return;
        }
        
        mSenderAudioChannels[senderChannelAudioIDStr] = senderChannelAudio;
      }

      //-----------------------------------------------------------------------
      FakeMediaStreamTrackPtr RTPChannelAudioTester::detachMediaStreamTrack(const char *trackID)
      {
        String trackIDStr(trackID);

        AutoRecursiveLock lock(*this);

        auto found = mMediaStreamTracks.find(trackIDStr);
        TESTING_CHECK(found != mMediaStreamTracks.end())

        FakeMediaStreamTrackPtr track = (*found).second;

        mMediaStreamTracks.erase(found);

        return track;
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelPtr RTPChannelAudioTester::detachReceiverChannel(const char *receiverChannelID)
      {
        String receiverChannelIDStr(receiverChannelID);
        
        AutoRecursiveLock lock(*this);
        
        auto foundAttached = mAttached.find(receiverChannelIDStr);
        TESTING_CHECK(foundAttached != mAttached.end())
        
        auto &currentReceiverChannel = (*foundAttached).second.first;
        auto &currentSenderChannel = (*foundAttached).second.second;
        
        currentReceiverChannel.reset();
        
        if (!currentSenderChannel) mAttached.erase(foundAttached);

        auto foundReceiverChannel = mReceiverChannels.find(receiverChannelIDStr);
        TESTING_CHECK(foundReceiverChannel != mReceiverChannels.end())

        FakeReceiverChannelPtr receiverChannel = (*foundReceiverChannel).second;

        mReceiverChannels.erase(foundReceiverChannel);

        return receiverChannel;
      }
      
      //-----------------------------------------------------------------------
      FakeSenderChannelPtr RTPChannelAudioTester::detachSenderChannel(const char *senderChannelID)
      {
        String senderChannelIDStr(senderChannelID);

        AutoRecursiveLock lock(*this);
        
        auto foundAttached = mAttached.find(senderChannelIDStr);
        TESTING_CHECK(foundAttached != mAttached.end())
        
        auto &currentReceiverChannel = (*foundAttached).second.first;
        auto &currentSenderChannel = (*foundAttached).second.second;
        
        currentSenderChannel.reset();
        if (!currentReceiverChannel) mAttached.erase(foundAttached);

        auto foundSenderChannel = mSenderChannels.find(senderChannelIDStr);
        TESTING_CHECK(foundSenderChannel != mSenderChannels.end())

        FakeSenderChannelPtr senderChannel = (*foundSenderChannel).second;

        mSenderChannels.erase(foundSenderChannel);

        return senderChannel;
      }
      
      //-----------------------------------------------------------------------
      RTPReceiverChannelAudioPtr RTPChannelAudioTester::detachReceiverChannelAudio(const char *receiverChannelAudioID)
      {
        String receiverChannelAudioIDStr(receiverChannelAudioID);
        
        AutoRecursiveLock lock(*this);
        
        auto found = mReceiverAudioChannels.find(receiverChannelAudioIDStr);
        TESTING_CHECK(found != mReceiverAudioChannels.end())
        
        RTPReceiverChannelAudioPtr receiverChannelAudio = (*found).second;
        
        mReceiverAudioChannels.erase(found);
        
        return receiverChannelAudio;
      }
      
      //-----------------------------------------------------------------------
      RTPSenderChannelAudioPtr RTPChannelAudioTester::detachSenderChannelAudio(const char *senderChannelAudioID)
      {
        String senderChannelAudioIDStr(senderChannelAudioID);
        
        AutoRecursiveLock lock(*this);
        
        auto found = mSenderAudioChannels.find(senderChannelAudioIDStr);
        TESTING_CHECK(found != mSenderAudioChannels.end())
        
        RTPSenderChannelAudioPtr senderChannelAudio = (*found).second;
        
        mSenderAudioChannels.erase(found);
        
        return senderChannelAudio;
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::store(
                                        const char *packetID,
                                        RTPPacketPtr packet
                                        )
      {
        TESTING_CHECK(packet)
        AutoRecursiveLock lock(*this);
        
        auto found = mPackets.find(String(packetID));
        if (found != mPackets.end()) {
          (*found).second.first = packet;
          return;
        }
        
        mPackets[String(packetID)] = PacketPair(packet, RTCPPacketPtr());
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::store(
                                        const char *packetID,
                                        RTCPPacketPtr packet
                                        )
      {
        TESTING_CHECK(packet)
        AutoRecursiveLock lock(*this);
        
        auto found = mPackets.find(String(packetID));
        if (found != mPackets.end()) {
          (*found).second.second = packet;
          return;
        }
        
        mPackets[String(packetID)] = PacketPair(RTPPacketPtr(), packet);
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::store(
                                        const char *sampleID,
                                        RTPSamplePtr sample
                                        )
      {
        TESTING_CHECK(sample)
        AutoRecursiveLock lock(*this);
        
        auto found = mSamples.find(String(sampleID));
        if (found != mSamples.end()) {
          (*found).second.second = sample;
          return;
        }
        
        mSamples[String(sampleID)] = SamplePair(RTPSamplePtr(), sample);
      }

      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::store(
                                        const char *parametersID,
                                        const Parameters &params
                                        )
      {
        AutoRecursiveLock lock(*this);
        
        auto found = mParameters.find(String(parametersID));
        if (found != mParameters.end()) {
          (*found).second = make_shared<Parameters>(params);
          return;
        }
        
        mParameters[String(parametersID)] = make_shared<Parameters>(params);
      }
      
      //-----------------------------------------------------------------------
      RTPPacketPtr RTPChannelAudioTester::getRTPPacket(const char *packetID)
      {
        AutoRecursiveLock lock(*this);
        
        auto found = mPackets.find(String(packetID));
        if (found == mPackets.end()) return RTPPacketPtr();
        return (*found).second.first;
      }
      
      //-----------------------------------------------------------------------
      RTCPPacketPtr RTPChannelAudioTester::getRTCPPacket(const char *packetID)
      {
        AutoRecursiveLock lock(*this);
        
        auto found = mPackets.find(String(packetID));
        if (found == mPackets.end()) return RTCPPacketPtr();
        return (*found).second.second;
      }
      
      //-----------------------------------------------------------------------
      RTPChannelAudioTester::RTPSamplePtr RTPChannelAudioTester::getSample(const char *sampleID)
      {
        AutoRecursiveLock lock(*this);
        
        auto found = mSamples.find(String(sampleID));
        if (found == mSamples.end()) return RTPSamplePtr();
        return (*found).second.second;
      }
      
      //-----------------------------------------------------------------------
      RTPChannelAudioTester::ParametersPtr RTPChannelAudioTester::getParameters(const char *parametersID)
      {
        AutoRecursiveLock lock(*this);
        
        auto found = mParameters.find(String(parametersID));
        if (found == mParameters.end()) return ParametersPtr();
        return make_shared<Parameters>(*((*found).second));
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::sendPacket(
                                             const char *senderOrReceiverChannelID,
                                             const char *packetID
                                             )
      {
        RTPPacketPtr rtp;
        RTCPPacketPtr rtcp;
        
        getPackets(packetID, rtp, rtcp);
        
        if (rtp) {
          sendData(senderOrReceiverChannelID, rtp->buffer());
        }
        if (rtcp) {
          sendData(senderOrReceiverChannelID, rtcp->buffer());
        }
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::expectPacket(
                                          const char *senderOrReceiverChannelID,
                                          const char *packetID
                                          )
      {
        RTPPacketPtr rtp;
        RTCPPacketPtr rtcp;
        
        getPackets(packetID, rtp, rtcp);
        
        {
          AutoRecursiveLock lock(*this);
          ++mExpecting.mReceivedPackets;
        }
        
        if (rtp) {
        }
        if (rtcp) {
        }
      }
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelAudioTester => IRTPReceiverChannelAudioForRTPReceiverChannel
      #pragma mark
      
      //-----------------------------------------------------------------------
      RTPReceiverChannelAudioPtr RTPChannelAudioTester::create(
                                                               RTPReceiverChannelPtr receiver,
                                                               const Parameters &params
                                                               )
      {
        return RTPReceiverChannelAudioPtr();
      }
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelAudioTester => IRTPReceiverChannelAudioForMediaStreamTrack
      #pragma mark
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelAudioTester => IRTPSenderChannelAudioForRTPSenderChannel
      #pragma mark
      
      //-----------------------------------------------------------------------
      RTPSenderChannelAudioPtr RTPChannelAudioTester::create(
                                                             RTPSenderChannelPtr sender,
                                                             const Parameters &params
                                                             )
      {
        return RTPSenderChannelAudioPtr();
      }
      
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelAudioTester => IRTPReceiverChannelAudioForMediaStreamTrack
      #pragma mark


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelAudioTester => (friend fake media track, sender and receiver channel)
      #pragma mark
      
      //-----------------------------------------------------------------------
      RTPReceiverChannelAudioPtr RTPChannelAudioTester::createReceiverChannelAudio(
                                                                                   RTPReceiverChannelPtr receiverChannel,
                                                                                   MediaStreamTrackPtr track,
                                                                                   const Parameters &params
                                                                                   )
      {
        AutoRecursiveLock lock(*this);
        
        return RTPReceiverChannelAudioPtr();
      }
      
      //-----------------------------------------------------------------------
      RTPSenderChannelAudioPtr RTPChannelAudioTester::createSenderChannelAudio(
                                                                               RTPSenderChannelPtr senderChannel,
                                                                               MediaStreamTrackPtr track,
                                                                               const Parameters &params
                                                                               )
      {
        AutoRecursiveLock lock(*this);
        
        return RTPSenderChannelAudioPtr();
      }

      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::sendToConnectedTester(RTPPacketPtr packet)
      {
        if (mConnectedTester.lock()->mReceiverChannels.size() != 0) {
          FakeReceiverChannelPtr receiverChannel = mConnectedTester.lock()->mReceiverChannels.begin()->second;
          receiverChannel->handlePacket(packet);
        }
      }

      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::sendToConnectedTester(RTCPPacketPtr packet)
      {
        if (mConnectedTester.lock()->mSenderChannels.size() != 0) {
          FakeSenderChannelPtr senderChannel = mConnectedTester.lock()->mSenderChannels.begin()->second;
          senderChannel->handlePacket(packet);
        }
        if (mConnectedTester.lock()->mReceiverChannels.size() != 0) {
          FakeReceiverChannelPtr receiverChannel = mConnectedTester.lock()->mReceiverChannels.begin()->second;
          receiverChannel->handlePacket(packet);
        }
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelAudioTester => (internal)
      #pragma mark
      
      //-----------------------------------------------------------------------
      Log::Params RTPChannelAudioTester::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpchannelaudio::RTPChannelAudioTester");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      FakeMediaStreamTrackPtr RTPChannelAudioTester::getMediaStreamTrack(const char *trackID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mMediaStreamTracks.find(String(trackID));
        if (mMediaStreamTracks.end() == found) return FakeMediaStreamTrackPtr();
        return (*found).second;
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelPtr RTPChannelAudioTester::getReceiverChannel(const char *receiverChannelID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mReceiverChannels.find(String(receiverChannelID));
        if (mReceiverChannels.end() == found) return FakeReceiverChannelPtr();
        return (*found).second;
      }
      
      //-----------------------------------------------------------------------
      FakeSenderChannelPtr RTPChannelAudioTester::getSenderChannel(const char *senderChannelID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mSenderChannels.find(String(senderChannelID));
        if (mSenderChannels.end() == found) return FakeSenderChannelPtr();
        return (*found).second;
      }
      
      //-----------------------------------------------------------------------
      RTPReceiverChannelAudioPtr RTPChannelAudioTester::getReceiverChannelAudio(const char *receiverChannelAudioID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mReceiverAudioChannels.find(String(receiverChannelAudioID));
        if (mReceiverAudioChannels.end() == found) return RTPReceiverChannelAudioPtr();
        return (*found).second;
      }
      
      //-----------------------------------------------------------------------
      RTPSenderChannelAudioPtr RTPChannelAudioTester::getSenderChannelAudio(const char *senderChannelAudioID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mSenderAudioChannels.find(String(senderChannelAudioID));
        if (mSenderAudioChannels.end() == found) return RTPSenderChannelAudioPtr();
        return (*found).second;
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::expectData(
                                             const char *senderOrReceiverChannelID,
                                             SecureByteBlockPtr secureBuffer
                                             )
      {
        {
          FakeSenderChannelPtr senderChannel = getSenderChannel(senderOrReceiverChannelID);
          if (senderChannel) {
          }
        }
        
        {
          FakeReceiverChannelPtr receiverChannel = getReceiverChannel(senderOrReceiverChannelID);
          if (receiverChannel) {
          }
        }
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::sendData(
                                           const char *senderOrReceiverChannelID,
                                           SecureByteBlockPtr secureBuffer
                                           )
      {
        {
          FakeSenderChannelPtr senderChannel = getSenderChannel(senderOrReceiverChannelID);
          if (senderChannel) {
            //senderChannel->sendPacket(secureBuffer);
          }
        }
        {
          FakeReceiverChannelPtr receiverChannel = getReceiverChannel(senderOrReceiverChannelID);
          if (receiverChannel) {
            //receiverChannel->sendPacket(secureBuffer);
          }
        }
      }
      
      //-----------------------------------------------------------------------
      void RTPChannelAudioTester::getPackets(
                                             const char *packetID,
                                             RTPPacketPtr &outRTP,
                                             RTCPPacketPtr &outRTCP
                                             )
      {
        AutoRecursiveLock lock(*this);
        auto found = mPackets.find(String(packetID));
        TESTING_CHECK(found != mPackets.end())
        
        outRTP = (*found).second.first;
        outRTCP = (*found).second.second;
      }
    }
  }
}

ZS_DECLARE_USING_PTR(ortc::test::rtpchannelaudio, RTPChannelAudioTester)
ZS_DECLARE_USING_PTR(ortc, IRTPTypes)

ZS_DECLARE_USING_PTR(ortc::internal, RTPPacket)
ZS_DECLARE_USING_PTR(ortc::internal, RTCPPacket)
ZS_DECLARE_TYPEDEF_PTR(ortc::internal::ISecureTransportTypes, ISecureTransportTypes)

ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::Parameters, Parameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::CodecParameters, CodecParameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::EncodingParameters, EncodingParameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::RTXParameters, RTXParameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::FECParameters, FECParameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IMediaStreamTrackTypes, IMediaStreamTrackTypes)

using ortc::IICETypes;
using zsLib::Optional;
using zsLib::WORD;
using zsLib::DWORD;
using zsLib::BYTE;
using zsLib::Milliseconds;
using ortc::SecureByteBlock;
using ortc::SecureByteBlockPtr;

#define TEST_BASIC_MEDIA 0
#define TEST_ADVANCED_MEDIA 1

static void bogusSleep()
{
  for (int loop = 0; loop < 100; ++loop)
  {
    TESTING_SLEEP(100)
  }
}

void doTestRTPChannelAudio()
{
  if (!ORTC_TEST_DO_RTP_CHANNEL_AUDIO_TEST) return;
  
  TESTING_INSTALL_LOGGER();
  
  TESTING_SLEEP(1000)
  
  UseSettings::applyDefaults();
  
  auto thread(zsLib::IMessageQueueThread::createBasic());
  
  RTPChannelAudioTesterPtr testObject1;
  RTPChannelAudioTesterPtr testObject2;
  
  TESTING_STDOUT() << "WAITING:      Waiting for RTPChannelAudio testing to complete (max wait is 180 seconds).\n";
  
  {
    ULONG testNumber = 0;
    ULONG maxSteps = 80;

    rtc::LogMessage::LogToDebug(rtc::LS_SENSITIVE);

    webrtc::TraceCallback* traceCallback = new ortc::test::rtpchannelaudio::WebRtcTraceCallback();

    webrtc::Trace::CreateTrace();
    webrtc::Trace::SetTraceCallback(traceCallback);
    webrtc::Trace::set_level_filter(webrtc::kTraceAll);

    do
    {
      TESTING_STDOUT() << "TESTING       ---------->>>>>>>>>> " << testNumber << " <<<<<<<<<<----------\n";
      
      bool quit = false;
      ULONG expecting = 0;
      
      switch (testNumber) {
        case TEST_BASIC_MEDIA: {
          {
            testObject1 = RTPChannelAudioTester::create(thread, false);
            testObject2 = RTPChannelAudioTester::create(thread, false);
            
            TESTING_CHECK(testObject1)
            TESTING_CHECK(testObject2)
          }
          break;
        }
        case TEST_ADVANCED_MEDIA: {
          {
            testObject1 = RTPChannelAudioTester::create(thread, false);
            testObject2 = RTPChannelAudioTester::create(thread, false);
            
            TESTING_CHECK(testObject1)
            TESTING_CHECK(testObject2)
          }
          break;
        }
        default:  quit = true; break;
      }
      if (quit) break;
      
      expecting = 0;
      expecting += (testObject1 ? 1 : 0);
      expecting += (testObject2 ? 1 : 0);
      
      ULONG found = 0;
      ULONG lastFound = 0;
      ULONG step = 0;
      
      bool lastStepReached = false;
      
      while ((found < expecting) ||
             (!lastStepReached))
      {
        TESTING_SLEEP(1000)
        ++step;
        if (step >= maxSteps) {
          TESTING_CHECK(false)
          break;
        }
        
        found = 0;
        
        switch (testNumber) {
          case TEST_BASIC_MEDIA: {
            switch (step) {
              case 2: {
                if (testObject1) testObject1->connect(testObject2);
                //    bogusSleep();
                break;
              }
              case 3: {
                if (testObject1) testObject1->createMediaStreamTrack("lt", false);
                if (testObject2) testObject2->createMediaStreamTrack("rt", true);
                if (testObject1) testObject1->createSenderChannel("sc", "lt", "");
                if (testObject2) testObject2->createReceiverChannel("rc", "rt", "");
                if (testObject1) testObject1->createSenderChannelAudio("sc", "sca", "lt", "");
                if (testObject2) testObject2->createReceiverChannelAudio("rc", "rca", "rt", "");
                //    bogusSleep();
                break;
              }
              case 4: {
                if (testObject1) testObject1->send("sc", "");
                if (testObject2) testObject2->receive("rc", "");
                //    bogusSleep();
                break;
              }
              case 5: {
                TESTING_SLEEP(5000)
                //    bogusSleep();
                break;
              }
              case 6: {
                //    bogusSleep();
                break;
              }
              case 7: {
                //    bogusSleep();
                break;
              }
              case 8: {
                //    bogusSleep();
                break;
              }
              case 9: {
                //    bogusSleep();
                break;
              }
              case 10: {
                //    bogusSleep();
                break;
              }
              case 11: {
                //    bogusSleep();
                break;
              }
              case 12: {
                //    bogusSleep();
                break;
              }
              case 14: {
                //    bogusSleep();
                break;
              }
              case 15: {
                //    bogusSleep();
                break;
              }
              case 20: {
                //bogusSleep();
                break;
              }
              case 21: {
                //bogusSleep();
                break;
              }
              case 22: {
                //bogusSleep();
                break;
              }
              case 23: {
                if (testObject1) testObject1->stop("sc");
                if (testObject2) testObject2->stop("rc");
                //bogusSleep();
                break;
              }
              case 24: {
                if (testObject1) testObject1->close();
                if (testObject2) testObject2->close();
                //bogusSleep();
                break;
              }
              case 25: {
                lastStepReached = true;
                break;
              }
              default: {
                // nothing happening in this step
                break;
              }
            }
            break;
          }
          case TEST_ADVANCED_MEDIA: {
            switch (step) {
              case 2: {
                if (testObject1) testObject1->connect(testObject2);
                //    bogusSleep();
                break;
              }
              case 3: {
                //    bogusSleep();
                break;
              }
              case 4: {
                //    bogusSleep();
                break;
              }
              case 5:
              {
                //    bogusSleep();
                break;
              }
              case 6: {
                //    bogusSleep();
                break;
              }
              case 10: {
                //    bogusSleep();
                break;
              }
              case 11: {
                //    bogusSleep();
                break;
              }
              case 12: {
                //    bogusSleep();
                break;
              }
              case 13: {
                //    bogusSleep();
                break;
              }
              case 14: {
                //    bogusSleep();
                break;
              }
              case 15: {
                //    bogusSleep();
                break;
              }
              case 16: {
                //    bogusSleep();
                break;
              }
              case 17: {
                //    bogusSleep();
                break;
              }
              case 18: {
                //    bogusSleep();
                break;
              }
              case 20: {
                //    bogusSleep();
                break;
              }
              case 21: {
                //    bogusSleep();
                break;
              }
              case 22: {
                //    bogusSleep();
                break;
              }
              case 23: {
                //    bogusSleep();
                break;
              }
              case 24: {
                //    bogusSleep();
                break;
              }
              case 25: {
                //    bogusSleep();
                break;
              }
              case 26: {
                //    bogusSleep();
                break;
              }
              case 27: {
                //    bogusSleep();
                break;
              }
              case 28: {
                //    bogusSleep();
                break;
              }
              case 29: {
                //    bogusSleep();
                break;
              }
              case 30: {
                //    bogusSleep();
                break;
              }
              case 35: {
                //    bogusSleep();
                break;
              }
              case 36: {
                //bogusSleep();
                break;
              }
              case 37: {
                //bogusSleep();
                break;
              }
              case 38: {
                //bogusSleep();
                break;
              }
              case 39: {
                //bogusSleep();
                break;
              }
              case 40: {
                lastStepReached = true;
                break;
              }
              default: {
                // nothing happening in this step
                break;
              }
            }
            break;
          }
          default: {
            // none defined
            break;
          }
        }
        
        if (0 == found) {
          found += (testObject1 ? (testObject1->matches() ? 1 : 0) : 0);
          found += (testObject2 ? (testObject2->matches() ? 1 : 0) : 0);
        }
        
        if (lastFound != found) {
          lastFound = found;
          TESTING_STDOUT() << "FOUND:        [" << found << "].\n";
        }
      }
      
      TESTING_EQUAL(found, expecting)
      
      TESTING_SLEEP(2000)
      
      //switch (testNumber) {
      //  default:
        {
          if (testObject1) {TESTING_CHECK(testObject1->matches())}
          if (testObject2) {TESTING_CHECK(testObject2->matches())}
      //    break;
        }
      //}
      
      testObject1.reset();
      testObject2.reset();

      ++testNumber;
    } while (true);

    delete (ortc::test::rtpchannelaudio::WebRtcTraceCallback*)traceCallback;
  }
  
  TESTING_STDOUT() << "WAITING:      All channels have finished. Waiting for 'bogus' events to process (1 second wait).\n";
  TESTING_SLEEP(1000)

  // wait for shutdown
  {
    IMessageQueue::size_type count = 0;
    do
    {
      count = thread->getTotalUnprocessedMessages();
      if (0 != count)
        std::this_thread::yield();
    } while (count > 0);
    
    thread->waitForShutdown();
  }
  TESTING_UNINSTALL_LOGGER();
  zsLib::proxyDump();
  TESTING_EQUAL(zsLib::proxyGetTotalConstructed(), 0);
}

