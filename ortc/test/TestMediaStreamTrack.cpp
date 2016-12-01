/*

 Copyright (c) 2015, Hookflash Inc.
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


#include "TestMediaStreamTrack.h"

#include <ortc/internal/ortc_ORTC.h>

#include <zsLib/IMessageQueueThread.h>
#include <zsLib/XML.h>

#include <cmath>

#include "config.h"
#include "testing.h"

#include <webrtc/base/logging.h>
#include <webrtc/system_wrappers/include/trace.h>

namespace ortc { namespace test { ZS_DECLARE_SUBSYSTEM(ortc_test) } }

using zsLib::String;
using zsLib::ULONG;
using zsLib::FLOAT;
using zsLib::SHORT;
using zsLib::PTRNUMBER;
using zsLib::IMessageQueue;
using zsLib::Log;
using zsLib::AutoPUID;
using zsLib::AutoRecursiveLock;
using namespace zsLib::XML;
using zsLib::IPromiseResolutionDelegate;
using ortc::IMediaDevices;
using ortc::internal::IORTCForInternal;

ZS_DECLARE_TYPEDEF_PTR(zsLib::ISettings, UseSettings)
ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper)
ZS_DECLARE_TYPEDEF_PTR(zsLib::Promise, Promise)
ZS_DECLARE_TYPEDEF_PTR(ortc::IMediaDevicesTypes::Constraints, Constraints)

namespace ortc
{
  namespace test
  {
    namespace mediastreamtrack
    {
      ZS_DECLARE_CLASS_PTR(FakeReceiver)
      ZS_DECLARE_CLASS_PTR(FakeReceiverChannel)
      ZS_DECLARE_CLASS_PTR(MediaStreamTrackTester)

      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiver)
      ZS_DECLARE_USING_PTR(ortc::internal, RTPReceiverChannel)
      ZS_DECLARE_USING_PTR(ortc::internal, MediaStreamTrack)

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
      #pragma mark PromiseWithMediaStreamTrackListCallback
      #pragma mark

      class PromiseWithMediaStreamTrackListCallback : public IPromiseResolutionDelegate
      {
      private:
        PromiseWithMediaStreamTrackListCallback(MediaStreamTrackTesterPtr tester, IMediaStreamTrackTypes::Kinds kind) : 
          mTester(tester),
          mKind(kind)
        {
        }

      public:
        static PromiseWithMediaStreamTrackListCallbackPtr create(MediaStreamTrackTesterPtr tester, IMediaStreamTrackTypes::Kinds kind)
        {
          return PromiseWithMediaStreamTrackListCallbackPtr(new PromiseWithMediaStreamTrackListCallback(tester, kind));
        }

        virtual void onPromiseResolved(PromisePtr promise)
        {
          if (mKind == IMediaStreamTrackTypes::Kinds::Kind_Video)
          {
            ortc::IMediaDevicesTypes::MediaStreamTrackListPtr trackList = promise->value<ortc::IMediaDevicesTypes::MediaStreamTrackList>();
            mTester.lock()->mLocalVideoMediaStreamTrack = MediaStreamTrack::convert(*trackList->begin());

            ZS_LOG_DEBUG(log("getUserMedia promise resolved for video") + IMediaStreamTrack::toDebug(mTester.lock()->mLocalVideoMediaStreamTrack))
          
#define WARNING_THIS_HAS_CHANGED 1
#define WARNING_THIS_HAS_CHANGED 2

#if 0
            IMediaStreamTrackPtr(mTester.lock()->mLocalVideoMediaStreamTrack)->setVideoRenderCallback(mTester.lock()->mVideoSurface);
#endif //0

            mTester.lock()->mVideoSender = FakeSender::create(mTester.lock(), mTester.lock()->mLocalVideoMediaStreamTrack);
          }
          else if (mKind == IMediaStreamTrackTypes::Kinds::Kind_Audio)
          {
            ortc::IMediaDevicesTypes::MediaStreamTrackListPtr trackList = promise->value<ortc::IMediaDevicesTypes::MediaStreamTrackList>();

            ZS_LOG_DEBUG(log("getUserMedia promise resolved for audio") + IMediaStreamTrack::toDebug(mTester.lock()->mLocalVideoMediaStreamTrack))

            mTester.lock()->mLocalAudioMediaStreamTrack = MediaStreamTrack::convert(*trackList->begin());

            mTester.lock()->mAudioSender = FakeSender::create(mTester.lock(), mTester.lock()->mLocalAudioMediaStreamTrack);
          }
        }

        virtual void onPromiseRejected(PromisePtr promise)
        {
        }

        ~PromiseWithMediaStreamTrackListCallback()
        {
        }

        Log::Params log(const char *message) const
        {
          ElementPtr objectEl = Element::create("ortc::test::mediastreamtrack::PromiseWithMediaStreamTrackListCallback");
          return Log::Params(message, objectEl);
        }

      private:
        MediaStreamTrackTesterWeakPtr mTester;
        IMediaStreamTrackTypes::Kinds mKind;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark PromiseWithDeviceListCallback
      #pragma mark

      class PromiseWithDeviceListCallback : public IPromiseResolutionDelegate
      {
      private:
        PromiseWithDeviceListCallback(MediaStreamTrackTesterPtr tester, IMediaStreamTrackTypes::Kinds kind) : 
          mTester(tester),
          mKind(kind)
        {
        }

      public:
        static PromiseWithDeviceListCallbackPtr create(MediaStreamTrackTesterPtr tester, IMediaStreamTrackTypes::Kinds kind)
        {
          return PromiseWithDeviceListCallbackPtr(new PromiseWithDeviceListCallback(tester, kind));
        }

        virtual void onPromiseResolved(PromisePtr promise)
        {
          if (mKind == IMediaStreamTrackTypes::Kinds::Kind_Video)
          {
            ortc::IMediaDevicesTypes::DeviceListPtr deviceList = promise->value<ortc::IMediaDevicesTypes::DeviceList>();
              
            ZS_LOG_DEBUG(log("enumerateDevices promise resolved"))

            ortc::IMediaDevicesTypes::Device device;
            ortc::IMediaDevicesTypes::DeviceList::iterator iter = deviceList->begin();
            while (iter != deviceList->end())
            {
              ZS_LOG_DEBUG(log("device") + iter->toDebug())
              
              if (iter->mKind == IMediaDevicesTypes::DeviceKinds::DeviceKind_VideoInput)
              {
                device = *iter;
              }
              iter++;
            }
            ortc::IMediaDevicesTypes::ConstraintSetPtr constraintSet = ortc::IMediaDevicesTypes::ConstraintSet::create();
            constraintSet->mDeviceID.mValue.value().mValue.value() = device.mDeviceID;
            constraintSet->mWidth.mValue.value() = 800;
            constraintSet->mHeight.mValue.value() = 600;
            constraintSet->mFrameRate.mValue.value() = 30;
            ConstraintsPtr constraints = Constraints::create();
            constraints->mVideo = ortc::IMediaStreamTrackTypes::TrackConstraints::create();
            constraints->mVideo->mAdvanced.push_back(constraintSet);
            mTester.lock()->mVideoPromiseWithMediaStreamTrackList = IMediaDevices::getUserMedia(*constraints);
            mTester.lock()->mVideoPromiseWithMediaStreamTrackList->then(PromiseWithMediaStreamTrackListCallback::create(mTester.lock(), mKind));

            ZS_LOG_DEBUG(log("getUserMedia method called for video") + constraintSet->toDebug())
          }
          else if (mKind == IMediaStreamTrackTypes::Kinds::Kind_Audio)
          {
            ortc::IMediaDevicesTypes::DeviceListPtr deviceList = promise->value<ortc::IMediaDevicesTypes::DeviceList>();

            ZS_LOG_DEBUG(log("enumerateDevices promise resolved"))

            ortc::IMediaDevicesTypes::Device inputDevice;
            ortc::IMediaDevicesTypes::Device outputDevice;
            ortc::IMediaDevicesTypes::DeviceList::iterator iter = deviceList->begin();
            while (iter != deviceList->end())
            {
              ZS_LOG_DEBUG(log("device") + iter->toDebug())

              if (iter->mKind == IMediaDevicesTypes::DeviceKinds::DeviceKind_AudioInput)
              {
                inputDevice = *iter;
              }
              if (iter->mKind == IMediaDevicesTypes::DeviceKinds::DeviceKind_AudioOutput)
              {
                outputDevice = *iter;
              }
              iter++;
            }
            ortc::IMediaDevicesTypes::ConstraintSetPtr constraintSet = ortc::IMediaDevicesTypes::ConstraintSet::create();
            constraintSet->mDeviceID.mValue.value().mValue.value() = inputDevice.mDeviceID;
            constraintSet->mSampleRate.mValue.value() = 48000;
            ConstraintsPtr constraints = Constraints::create();
            constraints->mAudio = ortc::IMediaStreamTrackTypes::TrackConstraints::create();
            constraints->mAudio->mAdvanced.push_back(constraintSet);
            mTester.lock()->mAudioPromiseWithMediaStreamTrackList = IMediaDevices::getUserMedia(*constraints);
            mTester.lock()->mAudioPromiseWithMediaStreamTrackList->then(PromiseWithMediaStreamTrackListCallback::create(mTester.lock(), mKind));

            ZS_LOG_DEBUG(log("getUserMedia method called for audio") + constraintSet->toDebug())
          }
        }

        virtual void onPromiseRejected(PromisePtr promise)
        {
        }

        ~PromiseWithDeviceListCallback()
        {
        }

        Log::Params log(const char *message) const
        {
          ElementPtr objectEl = Element::create("ortc::test::mediastreamtrack::PromiseWithDeviceListCallback");
          return Log::Params(message, objectEl);
        }

      private:
        MediaStreamTrackTesterWeakPtr mTester;
        IMediaStreamTrackTypes::Kinds mKind;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark (helpers)
      #pragma mark


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver
      #pragma mark

      //-----------------------------------------------------------------------
      FakeReceiver::FakeReceiver(
                                 const make_private &,
                                 MediaStreamTrackTesterPtr tester,
                                 IMediaStreamTrackTypes::Kinds kind,
                                 IMessageQueuePtr queue
                                 ) :
        RTPReceiver(Noop(true)),
        mTester(tester)
      {
        ZS_LOG_BASIC(log("created"))

        mTrack = UseMediaStreamTrack::create(kind);
        mChannel = FakeReceiverChannel::create(tester, mThisWeak.lock(), MediaStreamTrack::convert(mTrack));
      }

      //-----------------------------------------------------------------------
      FakeReceiver::~FakeReceiver()
      {
        mThisWeak.reset();

        ZS_LOG_BASIC(log("destroyed"))
      }

      //-----------------------------------------------------------------------
      FakeReceiverPtr FakeReceiver::create(
                                           MediaStreamTrackTesterPtr tester,
                                           IMediaStreamTrackTypes::Kinds kind
                                           )
      {
        FakeReceiverPtr pThis(make_shared<FakeReceiver>(make_private{}, tester, kind));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      void FakeReceiver::init()
      {
        mTrack->setReceiver(mThisWeak.lock());
        mTrack->notifyActiveReceiverChannel(RTPReceiverChannel::convert(mChannel));
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver => IRTPReceiver
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeReceiver::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr resultEl = Element::create("ortc::test::mediastreamtrack::FakeReceiver");

        UseServicesHelper::debugAppend(resultEl, "id", getID());

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver => IFakeReceiverAsyncDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiver => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params FakeReceiver::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::mediastreamtrack::FakeReceiver");
        UseServicesHelper::debugAppend(objectEl, "id", RTPReceiver::getID());
        return Log::Params(message, objectEl);
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
                                               const make_private &,
                                               MediaStreamTrackTesterPtr tester,
                                               UseReceiverPtr receiver,
                                               IMediaStreamTrackPtr track,
                                               IMessageQueuePtr queue
                                               ) :
        RTPReceiverChannel(Noop(true), queue),
        mTester(tester),
        mTrack(MediaStreamTrack::convert(track)),
        mSentVideoFrames(0),
        mSentAudioSamples(0)
      {
        ZS_LOG_BASIC(log("created"))

        mReceiver = receiver;
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannel::~FakeReceiverChannel()
      {
        mThisWeak.reset();

        ZS_LOG_BASIC(log("destroyed"))
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelPtr FakeReceiverChannel::create(
                                                         MediaStreamTrackTesterPtr tester,
                                                         UseReceiverPtr receiver,
                                                         IMediaStreamTrackPtr track
                                                         )
      {
        FakeReceiverChannelPtr pThis(make_shared<FakeReceiverChannel>(make_private{}, tester, receiver, track, IORTCForInternal::queueDelegate()));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      void FakeReceiverChannel::init()
      {
        AutoRecursiveLock lock(*this);

        if (IMediaStreamTrackPtr(MediaStreamTrack::convert(mTrack))->kind() == IMediaStreamTrackTypes::Kinds::Kind_Video)
          mTimer = ITimer::create(mThisWeak.lock(), Milliseconds(30));
      }

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

        ElementPtr result = Element::create("ortc::test::mediastreamtrack::FakeReceiverChannel");

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());

        auto receiver = mReceiver.lock();
        UseServicesHelper::debugAppend(result, "receiver", receiver ? receiver->getID() : 0);

        return result;
      }

      //-------------------------------------------------------------------------
      int32_t FakeReceiverChannel::getAudioSamples(
                                                   const size_t numberOfSamples,
                                                   const uint8_t numberOfChannels,
                                                   void* audioSamples,
                                                   size_t& numberOfSamplesOut
                                                   )
      {
        mSentAudioSamples += numberOfSamples;

        if (mSentAudioSamples > 144000)
          return 0;

        const UINT sampleRate = 48000;
        const UINT waveFrequency = 5000;
        const FLOAT pi = static_cast<FLOAT>(3.14159265);
        const UINT wavePeriodInSamples = sampleRate / waveFrequency;
        SHORT* audioSamplesShort = (SHORT*)audioSamples;

        for (size_t i = 0; i < numberOfSamples; ++i)
        {
          float x = float(i % wavePeriodInSamples) / float(wavePeriodInSamples) * 2 * pi;
          audioSamplesShort[2 * i + 0] = static_cast<SHORT>(sin(x) * SHRT_MAX);
          audioSamplesShort[2 * i + 1] = static_cast<SHORT>(sin(x) * SHRT_MAX);
        }
        numberOfSamplesOut = numberOfSamples;

        TESTING_EQUAL(numberOfSamplesOut, 480)

        mTester.lock()->notifySentAudioSamples(numberOfSamples);
        if (mSentAudioSamples == 144000)
          mTester.lock()->notifyRemoteAudioTrackEvent();

        return 0;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel => IFakeReceiverChannelAsyncDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeReceiverChannel => ITimerDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void FakeReceiverChannel::onTimer(ITimerPtr timer)
      {
        mSentVideoFrames++;

        if (mSentVideoFrames > 100 || !mTrack)
          return;

        const int width = 640;
        const int height = 480;

        webrtc::VideoFrame frame;
        frame.CreateEmptyFrame(width, height, width, width / 2, width / 2);

        uint8_t* yBuffer = frame.buffer(webrtc::kYPlane);
        for (int i = 0; i < height; i++)
        {
          memset(yBuffer, 0, width);
          int lineOffset = 640 * mSentVideoFrames / 100;
          int lineWidth = lineOffset + 60 <= 640 ? 60 : 640 - lineOffset;
          memset(yBuffer + lineOffset, 255, lineWidth);
          yBuffer += width;
        }

        uint8_t* uBuffer = frame.buffer(webrtc::kUPlane);
        uint8_t* vBuffer = frame.buffer(webrtc::kVPlane);
        for (int i = 0; i < height / 2; i++)
        {
          memset(uBuffer, 128, width / 2);
          memset(vBuffer, 128, width / 2);
          uBuffer += width / 2;
          vBuffer += width / 2;
        }

        //mTrack->renderVideoFrame(frame);

        TESTING_EQUAL(frame.width(), 640)
        TESTING_EQUAL(frame.height(), 480)

        mTester.lock()->notifySentVideoFrame();
        if (mSentVideoFrames == 100)
          mTester.lock()->notifyRemoteVideoTrackEvent();
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
        ElementPtr objectEl = Element::create("ortc::test::mediastreamtrack::FakeReceiverChannel");
        UseServicesHelper::debugAppend(objectEl, "id", RTPReceiverChannel::getID());
        return Log::Params(message, objectEl);
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSender
      #pragma mark

      //-----------------------------------------------------------------------
      FakeSender::FakeSender(
                             const make_private &,
                             MediaStreamTrackTesterPtr tester,
                             IMediaStreamTrackPtr track,
                             IMessageQueuePtr queue
                             ) :
        RTPSender(Noop(true)),
        mTester(tester),
        mChannel(FakeSenderChannel::create(tester, mThisWeak.lock(), track))
      {
        ZS_LOG_BASIC(log("created"))

        mTrack = MediaStreamTrack::convert(track);
      }

      //-----------------------------------------------------------------------
      FakeSender::~FakeSender()
      {
        mThisWeak.reset();

        ZS_LOG_BASIC(log("destroyed"))
      }

      //-----------------------------------------------------------------------
      FakeSenderPtr FakeSender::create(
                                       MediaStreamTrackTesterPtr tester,
                                       IMediaStreamTrackPtr track
                                       )
      {
        FakeSenderPtr pThis(make_shared<FakeSender>(make_private{}, tester, track));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      void FakeSender::init()
      {
        mTrack->setSender(mThisWeak.lock());
        mTrack->notifyAttachSenderChannel(RTPSenderChannel::convert(mChannel));
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSender => IRTPSenderForMediaStreamTrack
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr FakeSender::toDebug() const
      {
        AutoRecursiveLock lock(*this);

        ElementPtr resultEl = Element::create("ortc::test::mediastreamtrack::FakeSender");

        UseServicesHelper::debugAppend(resultEl, "id", getID());

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSender => IFakeSenderAsyncDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSender => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params FakeSender::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::mediastreamtrack::FakeSender");
        UseServicesHelper::debugAppend(objectEl, "id", FakeSender::getID());
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
                                           const make_private &,
                                           MediaStreamTrackTesterPtr tester,
                                           UseSenderPtr sender,
                                           IMediaStreamTrackPtr track,
                                           IMessageQueuePtr queue
                                           ) :
        RTPSenderChannel(Noop(true), queue),
        mTester(tester),
        mTrack(MediaStreamTrack::convert(track)),
        mReceivedVideoFrames(0),
        mReceivedAudioSamples(0)
      {
        ZS_LOG_BASIC(log("created"))

        mSender = sender;
      }

      //-----------------------------------------------------------------------
      FakeSenderChannel::~FakeSenderChannel()
      {
        mThisWeak.reset();

        ZS_LOG_BASIC(log("destroyed"))
      }

      //-----------------------------------------------------------------------
      FakeSenderChannelPtr FakeSenderChannel::create(
                                                     MediaStreamTrackTesterPtr tester,
                                                     UseSenderPtr sender,
                                                     IMediaStreamTrackPtr track
                                                     )
      {
        FakeSenderChannelPtr pThis(make_shared<FakeSenderChannel>(make_private{}, tester, sender, track, IORTCForInternal::queueDelegate()));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      void FakeSenderChannel::init()
      {
      }

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

        ElementPtr result = Element::create("ortc::test::mediastreamtrack::FakeSenderChannel");

        UseServicesHelper::debugAppend(result, "tester", (bool)mTester.lock());

        auto sender = mSender.lock();
        UseServicesHelper::debugAppend(result, "sender", sender ? sender->getID() : 0);

        return result;
      }

      //-----------------------------------------------------------------------
      //void FakeSenderChannel::sendVideoFrame(
      //                                         const uint8_t* videoFrame,
      //                                         const size_t videoFrameSize
      //                                         )
      //{
      //  mReceivedVideoFrames++;
      //  if (mReceivedVideoFrames > 100)
      //    return;

      //  TESTING_EQUAL(videoFrameSize, 640 * 480)

      //  mTester.lock()->notifyReceivedVideoFrame();
      //  if (mReceivedVideoFrames == 100)
      //    IFakeSenderChannelAsyncDelegateProxy::create(mThisWeak.lock())->onNotifyLocalVideoTrackEvent();
      //}

      //-----------------------------------------------------------------------
      //void FakeSenderChannel::sendAudioSamples(
      //                                         const void* audioSamples,
      //                                         const size_t numberOfSamples,
      //                                         const uint8_t numberOfChannels
      //                                         )
      //{
      //  mReceivedAudioSamples += numberOfSamples;
      //  if (mReceivedAudioSamples > 144000)
      //    return;

      //  TESTING_EQUAL(numberOfSamples, 480)

      //  mTester.lock()->notifyReceivedAudioSamples(numberOfSamples);
      //  if (mReceivedAudioSamples == 144000)
      //    mTester.lock()->notifyLocalAudioTrackEvent();
      //}

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark FakeSenderChannel => IFakeSenderChannelAsyncDelegate
      #pragma mark

      void FakeSenderChannel::onNotifyLocalVideoTrackEvent()
      {
        if (mTester.lock()->mLocalVideoMediaStreamTrack)
          IMediaStreamTrackPtr(mTester.lock()->mLocalVideoMediaStreamTrack)->stop();
        mTester.lock()->notifyLocalVideoTrackEvent();
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
        ElementPtr objectEl = Element::create("ortc::test::mediastreamtrack::FakeSenderChannel");
        UseServicesHelper::debugAppend(objectEl, "id", RTPSenderChannel::getID());
        return Log::Params(message, objectEl);
      }


      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester::Expectations
      #pragma mark

      //-----------------------------------------------------------------------
      bool MediaStreamTrackTester::Expectations::operator==(const Expectations &op2) const
      {
        return (mSentVideoFrames == op2.mSentVideoFrames) &&
          (mSentAudioSamples == op2.mSentAudioSamples) &&
          (mReceivedVideoFrames == op2.mReceivedVideoFrames) &&
          (mReceivedAudioSamples == op2.mReceivedAudioSamples);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester::OverrideReceiverFactory
      #pragma mark

      //-----------------------------------------------------------------------
      MediaStreamTrackTester::OverrideReceiverFactoryPtr MediaStreamTrackTester::OverrideReceiverFactory::create(MediaStreamTrackTesterPtr tester)
      {
        OverrideReceiverFactoryPtr pThis(make_shared<OverrideReceiverFactory>());
        pThis->mTester = tester;
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPReceiverPtr MediaStreamTrackTester::OverrideReceiverFactory::create(
                                                                             IRTPReceiverDelegatePtr delegate,
                                                                             IMediaStreamTrackTypes::Kinds kind,
                                                                             IRTPTransportPtr transport,
                                                                             IRTCPTransportPtr rtcpTransport
                                                                             )
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)

        return tester->create(delegate, kind, transport, rtcpTransport);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester::OverrideReceiverChannelFactory
      #pragma mark

      //-----------------------------------------------------------------------
      MediaStreamTrackTester::OverrideReceiverChannelFactoryPtr MediaStreamTrackTester::OverrideReceiverChannelFactory::create(MediaStreamTrackTesterPtr tester)
      {
        OverrideReceiverChannelFactoryPtr pThis(make_shared<OverrideReceiverChannelFactory>());
        pThis->mTester = tester;
        return pThis;
      }

      //-----------------------------------------------------------------------
      RTPReceiverChannelPtr MediaStreamTrackTester::OverrideReceiverChannelFactory::create(
                                                                                           RTPReceiverPtr receiver,
                                                                                           MediaStreamTrackPtr track,
                                                                                           const Parameters &params,
                                                                                           const RTCPPacketList &packets
                                                                                           )
      {
        auto tester = mTester.lock();
        TESTING_CHECK(tester)

        return tester->create(receiver, track, params, packets);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester
      #pragma mark

      //-----------------------------------------------------------------------
      MediaStreamTrackTesterPtr MediaStreamTrackTester::create(
                                                               IMessageQueuePtr queue,
                                                               bool overrideFactories,
                                                               void* videoSurface
                                                               )
      {
        MediaStreamTrackTesterPtr pThis(new MediaStreamTrackTester(queue, overrideFactories, videoSurface));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      MediaStreamTrackTester::MediaStreamTrackTester(
                                                     IMessageQueuePtr queue,
                                                     bool overrideFactories,
                                                     void* videoSurface
                                                     ) :
        SharedRecursiveLock(SharedRecursiveLock::create()),
        MessageQueueAssociator(queue),
        mOverrideFactories(overrideFactories),
        mVideoSurface(videoSurface),
        mLocalVideoTrackEvent(Event::create()),
        mRemoteVideoTrackEvent(Event::create()),
        mLocalAudioTrackEvent(Event::create()),
        mRemoteAudioTrackEvent(Event::create())
      {
        ZS_LOG_BASIC(log("mediastreamtrack tester"))
      }

      //-----------------------------------------------------------------------
      MediaStreamTrackTester::~MediaStreamTrackTester()
      {
        ZS_LOG_BASIC(log("mediastreamtrack tester"))

        if (mOverrideFactories) {
          RTPReceiverFactory::override(RTPReceiverFactoryPtr());
          RTPReceiverChannelFactory::override(RTPReceiverChannelFactoryPtr());
        }
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::init()
      {
        AutoRecursiveLock lock(*this);

        if (mOverrideFactories) {
          RTPReceiverFactory::override(OverrideReceiverFactory::create(mThisWeak.lock()));
          RTPReceiverChannelFactory::override(OverrideReceiverChannelFactory::create(mThisWeak.lock()));
        }
      }

      //-----------------------------------------------------------------------
      bool MediaStreamTrackTester::matches(const Expectations &op2)
      {
        AutoRecursiveLock lock(*this);
        return mExpectations == op2;
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::close()
      {
        AutoRecursiveLock lock(*this);
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::closeByReset()
      {
        AutoRecursiveLock lock(*this);
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::startLocalVideoTrack()
      {
        mVideoPromiseWithDeviceList = IMediaDevices::enumerateDevices();
        mVideoPromiseWithDeviceList->then(PromiseWithDeviceListCallback::create(mThisWeak.lock(), IMediaStreamTrackTypes::Kinds::Kind_Video));

        ZS_LOG_DEBUG(log("enumerateDevices method called"))

        mLocalVideoTrackEvent->wait();
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::startRemoteVideoTrack()
      {
        mVideoReceiver = FakeReceiver::create(mThisWeak.lock(), IMediaStreamTrackTypes::Kinds::Kind_Video);

        mRemoteVideoMediaStreamTrack = MediaStreamTrack::convert(mVideoReceiver->track());

#define WARNING_THIS_HAS_CHANGED 1
#define WARNING_THIS_HAS_CHANGED 2

#if 0
        IMediaStreamTrackPtr(mRemoteVideoMediaStreamTrack)->setVideoRenderCallback(mVideoSurface);
#endif //0

        mRemoteVideoTrackEvent->wait();
        if (mRemoteVideoMediaStreamTrack)
          IMediaStreamTrackPtr(mRemoteVideoMediaStreamTrack)->stop();
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::startLocalAudioTrack()
      {
        mAudioPromiseWithDeviceList = IMediaDevices::enumerateDevices();
        mAudioPromiseWithDeviceList->then(PromiseWithDeviceListCallback::create(mThisWeak.lock(), IMediaStreamTrackTypes::Kinds::Kind_Audio));

        ZS_LOG_DEBUG(log("enumerateDevices method called"))

        mLocalAudioTrackEvent->wait();

        if (mLocalAudioMediaStreamTrack)
          IMediaStreamTrackPtr(mLocalAudioMediaStreamTrack)->stop();
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::startRemoteAudioTrack()
      {
        mAudioReceiver = FakeReceiver::create(mThisWeak.lock(), IMediaStreamTrackTypes::Kinds::Kind_Audio);

        mRemoteAudioMediaStreamTrack = MediaStreamTrack::convert(mAudioReceiver->track());

        mRemoteAudioTrackEvent->wait();
        if (mRemoteAudioMediaStreamTrack)
          IMediaStreamTrackPtr(mRemoteAudioMediaStreamTrack)->stop();
      }

      //-----------------------------------------------------------------------
      MediaStreamTrackTester::Expectations MediaStreamTrackTester::getExpectations() const
      {
        return mExpectations;
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::createReceiverChannel(
                                                         const char *receiverID,
                                                         const char *parametersID
                                                         )
      {
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::stop(const char *senderOrReceiverID)
      {
        AutoRecursiveLock lock(*this);
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::attach(
                                          const char *receiverChannelID,
                                          FakeReceiverChannelPtr receiverChannel
                                          )
      {
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelPtr MediaStreamTrackTester::detachReceiverChannel(const char *receiverChannelID)
      {
        String receiverChannelIDStr(receiverChannelID);

        AutoRecursiveLock lock(*this);

        return FakeReceiverChannelPtr();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester::IMediaStreamTrackDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::onMediaStreamTrackMute(
                                                          IMediaStreamTrackPtr track,
                                                          bool isMuted
                                                          )
      {

      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::onMediaStreamTrackEnded(IMediaStreamTrackPtr track)
      {

      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::onMediaStreamTrackOverConstrained(
                                                                     IMediaStreamTrackPtr track,
                                                                     OverconstrainedErrorPtr error
                                                                     )
      {
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester => (friend fake receiver)
      #pragma mark

      RTPReceiverPtr MediaStreamTrackTester::create(
                                                    IRTPReceiverDelegatePtr delegate,
                                                    IMediaStreamTrackTypes::Kinds kind,
                                                    IRTPTransportPtr transport,
                                                    IRTCPTransportPtr rtcpTransport
                                                    )
      {
        return RTPReceiverPtr();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester => (friend fake receiver channel)
      #pragma mark

      RTPReceiverChannelPtr MediaStreamTrackTester::create(
                                                           RTPReceiverPtr receiver,
                                                           MediaStreamTrackPtr track,
                                                           const Parameters &params,
                                                           const RTCPPacketList &packets
                                                           )
      {
        return RTPReceiverChannelPtr();
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::notifySentVideoFrame()
      {
        ZS_LOG_BASIC(log("notified sent video frame"))

        AutoRecursiveLock lock(*this);
        ++mExpectations.mSentVideoFrames;
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::notifyRemoteVideoTrackEvent()
      {
        ZS_LOG_BASIC(log("notified remote video track event"))

        AutoRecursiveLock lock(*this);
        mRemoteVideoTrackEvent->notify();
        mRemoteVideoTrackEvent->reset();
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::notifySentAudioSamples(int numberOfSamples)
      {
        ZS_LOG_BASIC(log("notified sent audio samples"))

        AutoRecursiveLock lock(*this);
        mExpectations.mSentAudioSamples += numberOfSamples;
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::notifyRemoteAudioTrackEvent()
      {
        ZS_LOG_BASIC(log("notified remote audio track event"))

        AutoRecursiveLock lock(*this);
        mRemoteAudioTrackEvent->notify();
        mRemoteAudioTrackEvent->reset();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester => (friend fake sender)
      #pragma mark

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester => (friend fake sender channel)
      #pragma mark

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::notifyReceivedVideoFrame()
      {
        ZS_LOG_BASIC(log("notified received video frame"))

        AutoRecursiveLock lock(*this);
        ++mExpectations.mReceivedVideoFrames;
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::notifyLocalVideoTrackEvent()
      {
        ZS_LOG_BASIC(log("notified local video track event"))

        AutoRecursiveLock lock(*this);
        mLocalVideoTrackEvent->notify();
        mLocalVideoTrackEvent->reset();
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::notifyReceivedAudioSamples(int numberOfSamples)
      {
        ZS_LOG_BASIC(log("notified received audio samples"))

        AutoRecursiveLock lock(*this);
        mExpectations.mReceivedAudioSamples += numberOfSamples;
      }

      //-----------------------------------------------------------------------
      void MediaStreamTrackTester::notifyLocalAudioTrackEvent()
      {
        ZS_LOG_BASIC(log("notified local audio track event"))

        AutoRecursiveLock lock(*this);
        mLocalAudioTrackEvent->notify();
        mLocalAudioTrackEvent->reset();
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrackTester => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      Log::Params MediaStreamTrackTester::log(const char *message) const
      {
        ElementPtr objectEl = Element::create("ortc::test::mediastreamtrack::MediaStreamTrackTester");
        UseServicesHelper::debugAppend(objectEl, "id", mID);
        return Log::Params(message, objectEl);
      }

      //-----------------------------------------------------------------------
      FakeReceiverPtr MediaStreamTrackTester::getReceiver(const char *receiverID)
      {
        AutoRecursiveLock lock(*this);
        return FakeReceiverPtr();
      }

      //-----------------------------------------------------------------------
      FakeReceiverChannelPtr MediaStreamTrackTester::getReceiverChannel(const char *receiverID)
      {
        AutoRecursiveLock lock(*this);
        return FakeReceiverChannelPtr();
      }
    }
  }
}

ZS_DECLARE_USING_PTR(ortc::test::mediastreamtrack, FakeReceiver)
ZS_DECLARE_USING_PTR(ortc::test::mediastreamtrack, MediaStreamTrackTester)
ZS_DECLARE_USING_PTR(ortc, IRTPReceiver)

ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::Parameters, Parameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::CodecParameters, CodecParameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IRTPTypes::EncodingParameters, EncodingParameters)
ZS_DECLARE_TYPEDEF_PTR(ortc::IMediaStreamTrackTypes, IMediaStreamTrackTypes)

using zsLib::Optional;
using zsLib::WORD;
using zsLib::BYTE;
using zsLib::Milliseconds;

#define TEST_BASIC_ROUTING 0

static void bogusSleep()
{
  for (int loop = 0; loop < 100; ++loop)
  {
    TESTING_SLEEP(100)
  }
}

void doTestMediaStreamTrack(void* videoSurface)
{
  if (!ORTC_TEST_DO_RTP_MEDIA_STREAM_TRACK_TEST) return;

  TESTING_INSTALL_LOGGER();

  TESTING_SLEEP(1000)

  UseSettings::applyDefaults();

  auto thread(zsLib::IMessageQueueThread::createBasic());

  MediaStreamTrackTesterPtr testObject;

  TESTING_STDOUT() << "WAITING:      Waiting for MediaStreamTrack testing to complete (max wait is 180 seconds).\n";

  {
    ULONG testNumber = 0;
    ULONG maxSteps = 80;

    do
    {
      TESTING_STDOUT() << "TESTING       ---------->>>>>>>>>> " << testNumber << " <<<<<<<<<<----------\n";

      bool quit = false;
      ULONG expecting = 0;

      MediaStreamTrackTester::Expectations expectations;

      switch (testNumber) {
      case TEST_BASIC_ROUTING: {
        {
          testObject = MediaStreamTrackTester::create(thread, true, videoSurface);

          TESTING_CHECK(testObject)

          expectations.mReceivedVideoFrames = 100;
          expectations.mSentVideoFrames = 100;
          expectations.mReceivedAudioSamples = 144000;
          expectations.mSentAudioSamples = 144000;
        }
        break;
      }
      default:  quit = true; break;
      }
      if (quit) break;

      expecting = 0;
      expecting += (testObject ? 1 : 0);

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
        case TEST_BASIC_ROUTING: {
          switch (step) {
          case 2: {
            if (testObject) testObject->startLocalVideoTrack();
        //  bogusSleep();
            break;
          }
          case 3: {
            if (testObject) testObject->startRemoteVideoTrack();
        //  bogusSleep();
            break;
          }
          case 4: {
            if (testObject) testObject->startLocalAudioTrack();
        //  bogusSleep();
            break;
          }
          case 5: {
            if (testObject) testObject->startRemoteAudioTrack();
        //  bogusSleep();
            break;
          }
          case 6: {
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
          found += (testObject ? (testObject->matches(expectations) ? 1 : 0) : 0);
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
          if (testObject) { TESTING_CHECK(testObject->matches(expectations)) }
      //    break;
        }
      //}

      testObject.reset();

      ++testNumber;
    } while (true);
  }

  TESTING_STDOUT() << "WAITING:      All MediaStreamTrack tests have finished. Waiting for 'bogus' events to process (1 second wait).\n";
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

