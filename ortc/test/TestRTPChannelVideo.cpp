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


#include "TestRTPChannelVideo.h"

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
#include <webrtc/modules/video_capture/video_capture_factory.h>

namespace ortc { namespace test { ZS_DECLARE_SUBSYSTEM(ortc_test) } }

using zsLib::String;
using zsLib::ULONG;
using zsLib::DOUBLE;
using zsLib::PTRNUMBER;
using zsLib::IMessageQueue;
using zsLib::Log;
using zsLib::AutoPUID;
using zsLib::AutoRecursiveLock;
using namespace zsLib::XML;

ZS_DECLARE_TYPEDEF_PTR(zsLib::ISettings, UseSettings)
ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper)

namespace ortc
{
  namespace test
  {
    namespace rtpchannelvideo
    {
      ZS_DECLARE_CLASS_PTR(FakeMediaStreamTrack)
      ZS_DECLARE_CLASS_PTR(FakeReceiverChannel)
      ZS_DECLARE_CLASS_PTR(FakeSenderChannel)
      ZS_DECLARE_CLASS_PTR(RTPChannelVideoTester)

      ZS_DECLARE_USING_PTR(ortc::internal, MediaStreamTrack)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiverChannel)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiverChannelVideo)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSenderChannel)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPSenderChannelVideo)

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
        if (!mRemote) {
          webrtc::VideoCaptureModule::DeviceInfo* info = webrtc::VideoCaptureFactory::CreateDeviceInfo(0);
          if (!info) {
            return;
          }

          String videoDeviceID;
          int numCams = info->NumberOfDevices();
          for (int index = 0; index < numCams; ++index) {
            char deviceName[256];
            char deviceUniqueId[256];
            if (info->GetDeviceName(index, deviceName, sizeof(deviceName),
              deviceUniqueId, sizeof(deviceUniqueId)) != -1) {
              videoDeviceID = String(deviceUniqueId);
            }
          }

          mVideoCaptureModule = webrtc::VideoCaptureFactory::Create(0, videoDeviceID.c_str());
          if (!mVideoCaptureModule) {
            return;
          }

          mVideoCaptureModule->AddRef();

          mVideoCaptureModule->RegisterCaptureDataCallback(*this);

          LONG desiredWidth = 640;
          LONG desiredHeight = 480;
          DOUBLE desiredMaxFPS = 30;
          LONG minWidthDiff = std::numeric_limits<LONG>::max();
          LONG minHeightDiff = std::numeric_limits<LONG>::max();
          DOUBLE minFpsDiff = std::numeric_limits<double>::max();
          webrtc::VideoCaptureCapability bestCap;
          int32_t numCaps = info->NumberOfCapabilities(videoDeviceID.c_str());
          for (int32_t i = 0; i < numCaps; ++i) {
            webrtc::VideoCaptureCapability cap;
            if (info->GetCapability(videoDeviceID.c_str(), i, cap) != -1) {
              if (cap.rawType == webrtc::kVideoMJPEG || cap.rawType == webrtc::kVideoUnknown)
                continue;
              LONG widthDiff = abs((LONG)(cap.width - desiredWidth));
              LONG heightDiff = abs((LONG)(cap.height - desiredHeight));
              DOUBLE fpsDiff = abs((DOUBLE)(cap.maxFPS - desiredMaxFPS));
              if (widthDiff < minWidthDiff) {
                bestCap = cap;
                minWidthDiff = widthDiff;
                minHeightDiff = heightDiff;
                minFpsDiff = fpsDiff;
              } else if (widthDiff == minWidthDiff) {
                if (heightDiff < minHeightDiff) {
                  bestCap = cap;
                  minHeightDiff = heightDiff;
                  minFpsDiff = fpsDiff;
                } else if (heightDiff == minHeightDiff) {
                  if (fpsDiff < minFpsDiff) {
                    bestCap = cap;
                    minFpsDiff = fpsDiff;
                  }
                }
              }
            }
          }
          delete info;

          if (mVideoCaptureModule->StartCapture(bestCap) != 0) {
            mVideoCaptureModule->DeRegisterCaptureDataCallback();
            return;
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
      #pragma mark FakeMediaStreamTrack => IMediaStreamTrack
      #pragma mark

      //-------------------------------------------------------------------------
      void FakeMediaStreamTrack::stop()
      {
        if (mVideoCaptureModule) {
          mVideoCaptureModule->StopCapture();
          mVideoCaptureModule->DeRegisterCaptureDataCallback();
        }

        //if (mVideoRenderModule)
        //  mVideoRenderModule->StopRender(1);
      }

      //-------------------------------------------------------------------------
      void FakeMediaStreamTrack::setVideoRenderCallback(IMediaStreamTrackRenderCallbackPtr callback)
      {
        AutoRecursiveLock lock(*this);

#define WARNING_THIS_HAS_CHANGED 1
#define WARNING_THIS_HAS_CHANGED 2
        //mVideoRenderModule = webrtc::VideoRender::CreateVideoRender(1, nullptr, false);

        //mVideoRendererCallback = mVideoRenderModule->AddIncomingRenderStream(1, 0, 0.0, 0.0, 1.0, 1.0);

        //mVideoRenderModule->StartRender(1);
      }

      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => IMediaStreamTrackForRTPReceiverChannel
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => IMediaStreamTrackForRTPReceiverChannelVideo
      #pragma mark
#if 0
      //-------------------------------------------------------------------------
      void FakeMediaStreamTrack::renderVideoFrame(const webrtc::VideoFrame& videoFrame)
      {
        AutoRecursiveLock lock(*this);

        if (mVideoRendererCallback)
          mVideoRendererCallback->RenderFrame(1, videoFrame);
      }
#endif
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => IMediaStreamTrackForRTPSenderChannelVideo
      #pragma mark

      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      //-------------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => webrtc::VideoCaptureDataCallback
      #pragma mark

      //-------------------------------------------------------------------------
      void FakeMediaStreamTrack::OnIncomingCapturedFrame(const int32_t id, const webrtc::VideoFrame& videoFrame)
      {
        AutoRecursiveLock lock(*this);

        if (mVideoRendererCallback)
          mVideoRendererCallback->RenderFrame(1, videoFrame);

        auto senderChannel = mSenderChannel.lock();
        //if (senderChannel)
        //  senderChannel->sendVideoFrame(videoFrame);
      }

      //-------------------------------------------------------------------------
      void FakeMediaStreamTrack::OnCaptureDelayChanged(const int32_t id, const int32_t delay)
      {

      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeMediaStreamTrack => (friend RTPChannelTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::setTransport(RTPChannelVideoTesterPtr tester)
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
      void FakeMediaStreamTrack::linkReceiverChannelVideo(UseReceiverChannelVideoPtr channelVideo)
      {
        AutoRecursiveLock lock(*this);
        mReceiverChannelVideo = channelVideo;
      }

      //-----------------------------------------------------------------------
      void FakeMediaStreamTrack::linkSenderChannelVideo(UseSenderChannelVideoPtr channelVideo)
      {
        AutoRecursiveLock lock(*this);
        mSenderChannelVideo = channelVideo;
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
        ElementPtr objectEl = Element::create("ortc::test::rtpchannelvideo::FakeMediaStreamTrack");
        UseServicesHelper::debugAppend(objectEl, "id", MediaStreamTrack::getID());
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      ElementPtr FakeMediaStreamTrack::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr result = Element::create("ortc::test::rtpchannelvideo::FakeMediaStreamTrack");

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
      #pragma mark FakeReceiverChannel => IRTPReceiverForRTPReceiverChannelVideo
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

        ElementPtr result = Element::create("ortc::test::rtpchannelvideo::FakeReceiverChannel");

        UseServicesHelper::debugAppend(result, "kind", IMediaStreamTrackTypes::toString(mKind));

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());

        UseServicesHelper::debugAppend(result, mParameters ? mParameters->toDebug() : ElementPtr());

        return result;
      }

      //-----------------------------------------------------------------------
      int32_t FakeReceiverChannel::getAudioSamples(
                                                   const size_t numberOfSamples,
                                                   const uint8_t numberOfChannels,
                                                   void* audioSamples,
                                                   size_t& numberOfSamplesOut
                                                   )
      {
        return 0;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel => (friend RTPSenderTester)
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeReceiverChannel::setTransport(RTPChannelVideoTesterPtr tester)
      {
        AutoRecursiveLock lock(*this);
        mTester = tester;
      }

      //-----------------------------------------------------------------------
      void FakeReceiverChannel::linkChannelVideo(UseReceiverChannelVideoPtr channelVideo)
      {
        AutoRecursiveLock lock(*this);
        mReceiverChannelVideo = channelVideo;
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
        if (mReceiverChannelVideo)
          mReceiverChannelVideo->handlePacket(packet);
        return true;
      }

      //-----------------------------------------------------------------------
      bool FakeReceiverChannel::handlePacket(RTCPPacketPtr packet)
      {
        if (mReceiverChannelVideo)
          mReceiverChannelVideo->handlePacket(packet);
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
        ElementPtr objectEl = Element::create("ortc::test::rtpchannelvideo::FakeReceiverChannel");
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
      #pragma mark FakeSenderChannel => IRTPSenderChannelForRTPSenderChannelVideo
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

        ElementPtr result = Element::create("ortc::test::rtpchannelvideo::FakeSenderChannel");

        UseServicesHelper::debugAppend(result, "kind", IMediaStreamTrackTypes::toString(mKind));

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());

        UseServicesHelper::debugAppend(result, mParameters ? mParameters->toDebug() : ElementPtr());

        return result;
      }
#if 0
      //-------------------------------------------------------------------------
      void FakeSenderChannel::sendVideoFrame(const webrtc::VideoFrame& videoFrame)
      {
        if (!mSenderChannelVideo) return;
        mSenderChannelVideo->sendVideoFrame(videoFrame);
      }
#endif
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel => (friend RTPChannelTester)
      #pragma mark

      void FakeSenderChannel::setTransport(RTPChannelVideoTesterPtr tester)
      {
        AutoRecursiveLock lock(*this);
        mTester = tester;
      }

      void FakeSenderChannel::linkChannelVideo(UseSenderChannelVideoPtr channelVideo)
      {
        AutoRecursiveLock lock(*this);
        mSenderChannelVideo = channelVideo;
      }

      void FakeSenderChannel::linkMediaStreamTrack(UseMediaStreamTrackPtr track)
      {
        AutoRecursiveLock lock(*this);
        mTrack = track;
      }

      bool FakeSenderChannel::handlePacket(RTCPPacketPtr packet)
      {
        if (mSenderChannelVideo)
          mSenderChannelVideo->handlePacket(packet);
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
        ElementPtr objectEl = Element::create("ortc::test::rtpchannelvideo::FakeSenderChannel");
        UseServicesHelper::debugAppend(objectEl, "id", FakeSenderChannel::getID());
        return Log::Params(message, objectEl);
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelVideoTester::Expectations
      #pragma mark

      //-----------------------------------------------------------------------
      bool RTPChannelVideoTester::Expectations::operator==(const Expectations &op2) const
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
      #pragma mark RTPChannelVideoTester::OverrideReceiverChannelVideoFactory
      #pragma mark

      //-----------------------------------------------------------------------
      RTPChannelVideoTester::OverrideReceiverChannelVideoFactoryPtr RTPChannelVideoTester::OverrideReceiverChannelVideoFactory::create(RTPChannelVideoTesterPtr tester)
      {
        OverrideReceiverChannelVideoFactoryPtr pThis(make_shared<OverrideReceiverChannelVideoFactory>());
        pThis->mTester = tester;
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPReceiverChannelVideoPtr RTPChannelVideoTester::OverrideReceiverChannelVideoFactory::create(
                                                                                                    RTPReceiverChannelPtr receiverChannel,
                                                                                                    MediaStreamTrackPtr track,
                                                                                                    const Parameters &params
                                                                                                    )
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)

        return tester->createReceiverChannelVideo(receiverChannel, track, params);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelVideoTester::OverrideSenderChannelVideoFactory
      #pragma mark

      //-----------------------------------------------------------------------
      RTPChannelVideoTester::OverrideSenderChannelVideoFactoryPtr RTPChannelVideoTester::OverrideSenderChannelVideoFactory::create(RTPChannelVideoTesterPtr tester)
      {
        OverrideSenderChannelVideoFactoryPtr pThis(make_shared<OverrideSenderChannelVideoFactory>());
        pThis->mTester = tester;
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPSenderChannelVideoPtr RTPChannelVideoTester::OverrideSenderChannelVideoFactory::create(
                                                                                                RTPSenderChannelPtr senderChannel,
                                                                                                MediaStreamTrackPtr track,
                                                                                                const Parameters &params
                                                                                                )
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)

        return tester->createSenderChannelVideo(senderChannel, track, params);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelVideoTester
      #pragma mark

      //-----------------------------------------------------------------------
      RTPChannelVideoTesterPtr RTPChannelVideoTester::create(
                                                             IMessageQueuePtr queue,
                                                             bool overrideFactories,
                                                             void* videoSurface
                                                             )
      {
        RTPChannelVideoTesterPtr pThis(new RTPChannelVideoTester(queue, overrideFactories, videoSurface));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPChannelVideoTester::RTPChannelVideoTester(
                                                   IMessageQueuePtr queue,
                                                   bool overrideFactories,
                                                   void* videoSurface
                                                   ) :
          SharedRecursiveLock(SharedRecursiveLock::create()),
          MessageQueueAssociator(queue),
          mOverrideFactories(overrideFactories),
          mVideoSurface(videoSurface)
        {
        ZS_LOG_BASIC(log("rtpchannelvideo tester"))
      }

      //-----------------------------------------------------------------------
      RTPChannelVideoTester::~RTPChannelVideoTester()
      {
        ZS_LOG_BASIC(log("rtpchannelvideo tester"))

        if (mOverrideFactories) {
          RTPReceiverChannelVideoFactory::override(RTPReceiverChannelVideoFactoryPtr());
          RTPSenderChannelVideoFactory::override(RTPSenderChannelVideoFactoryPtr());
        }
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::init()
      {
        AutoRecursiveLock lock(*this);

        if (mOverrideFactories) {
          RTPReceiverChannelVideoFactory::override(OverrideReceiverChannelVideoFactory::create(mThisWeak.lock()));
          RTPSenderChannelVideoFactory::override(OverrideSenderChannelVideoFactory::create(mThisWeak.lock()));
        }
      }

      //-----------------------------------------------------------------------
      bool RTPChannelVideoTester::matches()
      {
        AutoRecursiveLock lock(*this);
        return mExpectationsFound == mExpecting;
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::close()
      {
        AutoRecursiveLock lock(*this);

        for (auto iter = mReceiverVideoChannels.begin(); iter != mReceiverVideoChannels.end(); ++iter)
        {
          UseReceiverChannelVideoForReceiverChannelPtr receiverChannel = (*iter).second;
        }

        mReceiverVideoChannels.clear();

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
      void RTPChannelVideoTester::closeByReset()
      {
        AutoRecursiveLock lock(*this);

        mReceiverVideoChannels.clear();
        mSenderVideoChannels.clear();
      }

      //-----------------------------------------------------------------------
      RTPChannelVideoTester::Expectations RTPChannelVideoTester::getExpectations() const
      {
        return mExpectationsFound;
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::connect(RTPChannelVideoTesterPtr remote)
      {
        AutoRecursiveLock lock(*this);
        AutoRecursiveLock lock2(*remote);

        TESTING_CHECK(remote)

        mConnectedTester = remote;
        remote->mConnectedTester = mThisWeak.lock();
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::createMediaStreamTrack(
                                                         const char *trackID,
                                                         bool remote
                                                         )
      {
        FakeMediaStreamTrackPtr track = getMediaStreamTrack(trackID);

        if (!track) {
          track = FakeMediaStreamTrack::create(getAssociatedMessageQueue(), remote);

#define WARNING_THIS_HAS_CHANGED 1
#define WARNING_THIS_HAS_CHANGED 2

#if 0
          track->setVideoRenderCallback(mVideoSurface);
#endif //0
          track->setTransport(mThisWeak.lock());
          attach(trackID, track);
        }

        TESTING_CHECK(track)
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::createReceiverChannel(
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
      void RTPChannelVideoTester::createSenderChannel(
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
      void RTPChannelVideoTester::createReceiverChannelVideo(
                                                             const char *receiverChannelID,
                                                             const char *receiverChannelVideoID,
                                                             const char *mediaStreamTrackID,
                                                             const char *parametersID
                                                             )
      {
        RTPReceiverChannelVideoPtr receiverChannelVideo = getReceiverChannelVideo(receiverChannelVideoID);

        if (!receiverChannelVideo) {
          auto params = getParameters(parametersID);
          //TESTING_CHECK(params)
          if (!params)
            params.reset(new Parameters());
          auto receiverChannel = getReceiverChannel(receiverChannelID);
          TESTING_CHECK(receiverChannel)
          auto track = getMediaStreamTrack(mediaStreamTrackID);
          TESTING_CHECK(track)
          receiverChannelVideo = UseReceiverChannelVideoForReceiverChannel::create(receiverChannel, track, *params);
          receiverChannel->linkChannelVideo(receiverChannelVideo);
          track->linkReceiverChannelVideo(receiverChannelVideo);
          attach(receiverChannelVideoID, receiverChannelVideo);
        }

        TESTING_CHECK(receiverChannelVideo)
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::createSenderChannelVideo(
                                                           const char *senderChannelID,
                                                           const char *senderChannelVideoID,
                                                           const char *mediaStreamTrackID,
                                                           const char *parametersID
                                                           )
      {
        RTPSenderChannelVideoPtr senderChannelVideo = getSenderChannelVideo(senderChannelVideoID);

        if (!senderChannelVideo) {
          auto params = getParameters(parametersID);
          //TESTING_CHECK(params)
          if (!params)
            params.reset(new Parameters());
          auto senderChannel = getSenderChannel(senderChannelID);
          TESTING_CHECK(senderChannel)
          auto track = getMediaStreamTrack(mediaStreamTrackID);
          TESTING_CHECK(track)
          senderChannelVideo = UseSenderChannelVideoForSenderChannel::create(senderChannel, track, *params);
          senderChannel->linkChannelVideo(senderChannelVideo);
          track->linkSenderChannelVideo(senderChannelVideo);
          attach(senderChannelVideoID, senderChannelVideo);
        }

        TESTING_CHECK(senderChannelVideo)
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::send(
                                       const char *senderChannelID,
                                       const char *parametersID
                                       )
      {
        AutoRecursiveLock lock(*this);
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::receive(
                                          const char *receiverChannelID,
                                          const char *parametersID
                                          )
      {
        AutoRecursiveLock lock(*this);
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::stop(const char *senderOrReceiverChannelID)
      {
        AutoRecursiveLock lock(*this);
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::attach(
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
      void RTPChannelVideoTester::attach(
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
      void RTPChannelVideoTester::attach(
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
      void RTPChannelVideoTester::attach(
                                         const char *receiverChannelVideoID,
                                         RTPReceiverChannelVideoPtr receiverChannelVideo
                                         )
      {
        TESTING_CHECK(receiverChannelVideo)

        String receiverChannelVideoIDStr(receiverChannelVideoID);

        AutoRecursiveLock lock(*this);

        FakeReceiverChannelPtr oldChannelVideo;

        auto found = mReceiverVideoChannels.find(receiverChannelVideoIDStr);

        if (found != mReceiverVideoChannels.end()) {
          auto &previousReceiverChannelVideo = (*found).second;
          previousReceiverChannelVideo = receiverChannelVideo;
          return;
        }

        mReceiverVideoChannels[receiverChannelVideoIDStr] = receiverChannelVideo;
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::attach(
                                         const char *senderChannelVideoID,
                                         RTPSenderChannelVideoPtr senderChannelVideo
                                         )
      {
        TESTING_CHECK(senderChannelVideo)

        String senderChannelVideoIDStr(senderChannelVideoID);

        AutoRecursiveLock lock(*this);

        FakeSenderChannelPtr oldChannelVideo;

        auto found = mSenderVideoChannels.find(senderChannelVideoIDStr);

        if (found != mSenderVideoChannels.end()) {
          auto &previousSenderChannelVideo = (*found).second;
          previousSenderChannelVideo = senderChannelVideo;
          return;
        }

        mSenderVideoChannels[senderChannelVideoIDStr] = senderChannelVideo;
      }

      //-----------------------------------------------------------------------
      FakeMediaStreamTrackPtr RTPChannelVideoTester::detachMediaStreamTrack(const char *trackID)
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
      FakeReceiverChannelPtr RTPChannelVideoTester::detachReceiverChannel(const char *receiverChannelID)
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
      FakeSenderChannelPtr RTPChannelVideoTester::detachSenderChannel(const char *senderChannelID)
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
      RTPReceiverChannelVideoPtr RTPChannelVideoTester::detachReceiverChannelVideo(const char *receiverChannelVideoID)
      {
        String receiverChannelVideoIDStr(receiverChannelVideoID);

        AutoRecursiveLock lock(*this);

        auto found = mReceiverVideoChannels.find(receiverChannelVideoIDStr);
        TESTING_CHECK(found != mReceiverVideoChannels.end())

        RTPReceiverChannelVideoPtr receiverChannelVideo = (*found).second;

        mReceiverVideoChannels.erase(found);

        return receiverChannelVideo;
      }

      //-----------------------------------------------------------------------
      RTPSenderChannelVideoPtr RTPChannelVideoTester::detachSenderChannelVideo(const char *senderChannelVideoID)
      {
        String senderChannelVideoIDStr(senderChannelVideoID);

        AutoRecursiveLock lock(*this);

        auto found = mSenderVideoChannels.find(senderChannelVideoIDStr);
        TESTING_CHECK(found != mSenderVideoChannels.end())

        RTPSenderChannelVideoPtr senderChannelVideo = (*found).second;

        mSenderVideoChannels.erase(found);

        return senderChannelVideo;
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::store(
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
      void RTPChannelVideoTester::store(
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
      void RTPChannelVideoTester::store(
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
      void RTPChannelVideoTester::store(
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
      RTPPacketPtr RTPChannelVideoTester::getRTPPacket(const char *packetID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mPackets.find(String(packetID));
        if (found == mPackets.end()) return RTPPacketPtr();
        return (*found).second.first;
      }

      //-----------------------------------------------------------------------
      RTCPPacketPtr RTPChannelVideoTester::getRTCPPacket(const char *packetID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mPackets.find(String(packetID));
        if (found == mPackets.end()) return RTCPPacketPtr();
        return (*found).second.second;
      }

      //-----------------------------------------------------------------------
      RTPChannelVideoTester::RTPSamplePtr RTPChannelVideoTester::getSample(const char *sampleID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mSamples.find(String(sampleID));
        if (found == mSamples.end()) return RTPSamplePtr();
        return (*found).second.second;
      }

      //-----------------------------------------------------------------------
      RTPChannelVideoTester::ParametersPtr RTPChannelVideoTester::getParameters(const char *parametersID)
      {
        AutoRecursiveLock lock(*this);

        auto found = mParameters.find(String(parametersID));
        if (found == mParameters.end()) return ParametersPtr();
        return make_shared<Parameters>(*((*found).second));
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::sendPacket(
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
      void RTPChannelVideoTester::expectPacket(
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
      #pragma mark RTPChannelVideoTester => IRTPReceiverChannelVideoForRTPReceiverChannel
      #pragma mark

      //-----------------------------------------------------------------------
      RTPReceiverChannelVideoPtr RTPChannelVideoTester::create(
                                                               RTPReceiverChannelPtr receiver,
                                                               const Parameters &params
                                                               )
      {
        return RTPReceiverChannelVideoPtr();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelVideoTester => IRTPReceiverChannelVideoForMediaStreamTrack
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelVideoTester => IRTPSenderChannelVideoForRTPSenderChannel
      #pragma mark

      //-----------------------------------------------------------------------
      RTPSenderChannelVideoPtr RTPChannelVideoTester::create(
                                                             RTPSenderChannelPtr sender,
                                                             const Parameters &params
                                                             )
      {
        return RTPSenderChannelVideoPtr();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelVideoTester => IRTPReceiverChannelVideoForMediaStreamTrack
      #pragma mark


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPChannelVideoTester => (friend fake media track, sender and receiver channel)
      #pragma mark

      //-----------------------------------------------------------------------
      RTPReceiverChannelVideoPtr RTPChannelVideoTester::createReceiverChannelVideo(
                                                                                   RTPReceiverChannelPtr receiverChannel,
                                                                                   MediaStreamTrackPtr track,
                                                                                   const Parameters &params
                                                                                   )
      {
        AutoRecursiveLock lock(*this);

        return RTPReceiverChannelVideoPtr();
      }

      //-----------------------------------------------------------------------
      RTPSenderChannelVideoPtr RTPChannelVideoTester::createSenderChannelVideo(
                                                                               RTPSenderChannelPtr senderChannel,
                                                                               MediaStreamTrackPtr track,
                                                                               const Parameters &params
                                                                               )
      {
        AutoRecursiveLock lock(*this);

        return RTPSenderChannelVideoPtr();
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::sendToConnectedTester(RTPPacketPtr packet)
      {
        if (mConnectedTester.lock()->mReceiverChannels.size() != 0) {
          FakeReceiverChannelPtr receiverChannel = mConnectedTester.lock()->mReceiverChannels.begin()->second;
          receiverChannel->handlePacket(packet);
        }
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::sendToConnectedTester(RTCPPacketPtr packet)
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
      #pragma mark RTPChannelVideoTester => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params RTPChannelVideoTester::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::rtpchannelvideo::RTPChannelVideoTester");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      FakeMediaStreamTrackPtr RTPChannelVideoTester::getMediaStreamTrack(const char *trackID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mMediaStreamTracks.find(String(trackID));
        if (mMediaStreamTracks.end() == found) return FakeMediaStreamTrackPtr();
        return (*found).second;
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelPtr RTPChannelVideoTester::getReceiverChannel(const char *receiverChannelID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mReceiverChannels.find(String(receiverChannelID));
        if (mReceiverChannels.end() == found) return FakeReceiverChannelPtr();
        return (*found).second;
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelPtr RTPChannelVideoTester::getSenderChannel(const char *senderChannelID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mSenderChannels.find(String(senderChannelID));
        if (mSenderChannels.end() == found) return FakeSenderChannelPtr();
        return (*found).second;
      }

      //-----------------------------------------------------------------------
      RTPReceiverChannelVideoPtr RTPChannelVideoTester::getReceiverChannelVideo(const char *receiverChannelVideoID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mReceiverVideoChannels.find(String(receiverChannelVideoID));
        if (mReceiverVideoChannels.end() == found) return RTPReceiverChannelVideoPtr();
        return (*found).second;
      }

      //-----------------------------------------------------------------------
      RTPSenderChannelVideoPtr RTPChannelVideoTester::getSenderChannelVideo(const char *senderChannelVideoID)
      {
        AutoRecursiveLock lock(*this);
        auto found = mSenderVideoChannels.find(String(senderChannelVideoID));
        if (mSenderVideoChannels.end() == found) return RTPSenderChannelVideoPtr();
        return (*found).second;
      }

      //-----------------------------------------------------------------------
      void RTPChannelVideoTester::expectData(
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
      void RTPChannelVideoTester::sendData(
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
      void RTPChannelVideoTester::getPackets(
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

ZS_DECLARE_USING_PTR(ortc::test::rtpchannelvideo, RTPChannelVideoTester)
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

void doTestRTPChannelVideo(void* localSurface, void* remoteSurface)
{
  if (!ORTC_TEST_DO_RTP_CHANNEL_VIDEO_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  UseSettings::applyDefaults();

  auto thread(zsLib::IMessageQueueThread::createBasic());

  RTPChannelVideoTesterPtr testObject1;
  RTPChannelVideoTesterPtr testObject2;

  TESTING_STDOUT() << "WAITING:      Waiting for RTPChannelVideo testing to complete (max wait is 180 seconds).\n";

  {
    ULONG testNumber = 0;
    ULONG maxSteps = 80;

    rtc::LogMessage::LogToDebug(rtc::LS_SENSITIVE);

    webrtc::TraceCallback* traceCallback = new ortc::test::rtpchannelvideo::WebRtcTraceCallback();

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
          testObject1 = RTPChannelVideoTester::create(thread, false, localSurface);
          testObject2 = RTPChannelVideoTester::create(thread, false, remoteSurface);

          TESTING_CHECK(testObject1)
          TESTING_CHECK(testObject2)
        }
        break;
      }
      case TEST_ADVANCED_MEDIA: {
        {
          testObject1 = RTPChannelVideoTester::create(thread, false, localSurface);
          testObject2 = RTPChannelVideoTester::create(thread, false, remoteSurface);

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
            if (testObject1) testObject1->createSenderChannelVideo("sc", "sca", "lt", "");
            if (testObject2) testObject2->createReceiverChannelVideo("rc", "rca", "rt", "");
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
            TESTING_SLEEP(500000)
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
            if (testObject1) testObject1->stop("sc");
            if (testObject2) testObject2->stop("rc");
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

      switch (testNumber) {
      default:
      {
        if (testObject1) { TESTING_CHECK(testObject1->matches()) }
        if (testObject2) { TESTING_CHECK(testObject2->matches()) }
        break;
      }
      }

      testObject1.reset();
      testObject2.reset();

      ++testNumber;
    } while (true);
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
