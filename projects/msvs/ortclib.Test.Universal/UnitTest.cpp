#include "pch.h"
#include "CppUnitTest.h"

#include <ortc/test/testing.h>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

void doSetup();
void doTestRTPChannel();
void doTestRTPReceiver();
void doTestRTPSender();
void doTestRTPListener();
void doTestRTPPacket();
void doTestRTCPPacket();
void doTestSCTP();
void doTestDTLS();
void doTestSRTP();
void doTestICEGatherer();
void doTestICETransport();
void doTestMediaStreamTrack(void* videoSurface);
void doTestRTPChannelAudio();
void doTestRTPChannelVideo(void* localSurface, void* remoteSurface);

#if 0
TESTING_RUN_TEST_FUNC_0(doTestRTPChannel)
TESTING_RUN_TEST_FUNC_0(doTestRTPSender)
TESTING_RUN_TEST_FUNC_0(doTestRTPReceiver)
TESTING_RUN_TEST_FUNC_0(doTestRTPListener)
TESTING_RUN_TEST_FUNC_0(doTestRTPPacket)
TESTING_RUN_TEST_FUNC_0(doTestRTCPPacket)
TESTING_RUN_TEST_FUNC_0(doTestSCTP)
TESTING_RUN_TEST_FUNC_0(doTestSRTP)
TESTING_RUN_TEST_FUNC_0(doTestDTLS)
TESTING_RUN_TEST_FUNC_0(doTestICEGatherer)
TESTING_RUN_TEST_FUNC_0(doTestICETransport)
TESTING_RUN_TEST_FUNC_1(doTestMediaStreamTrack, videoSurface1)
TESTING_RUN_TEST_FUNC_0(doTestRTPChannelAudio)
TESTING_RUN_TEST_FUNC_2(doTestRTPChannelVideo, videoSurface1, videoSurface2)
#endif //0

namespace ortclib_Test
{
    TEST_CLASS(UnitTest1)
    {
    public:
        TEST_METHOD(TestMethod1)
        {
          Testing::setup();
          unsigned int totalFailures = Testing::getGlobalFailedVar();

          doTestRTPChannel();

          if (totalFailures != Testing::getGlobalFailedVar()) {
            Assert::Fail(L"BackoffRetry retry tests have failed", LINE_INFO());
          }
        }
    };
}
