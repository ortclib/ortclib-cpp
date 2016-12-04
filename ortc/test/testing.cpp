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

#include "testing.h"
#include "config.h"

#include <zsLib/types.h>
#include <zsLib/helpers.h>
#include <zsLib/Log.h>
#include <ortc/services/ILogger.h>

#include <iostream>

namespace ortc { namespace test { ZS_IMPLEMENT_SUBSYSTEM(ortc_test) } }


#ifdef _WIN32
debugostream &getDebugCout()
{
  static debugostream gdebug;
  return gdebug;
}
#endif //_WIN32


typedef ortc::services::ILogger ILogger;

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

namespace Testing
{
  std::atomic_uint &getGlobalPassedVar()
  {
    static std::atomic_uint value {};
    return value;
  }
  
  std::atomic_uint &getGlobalFailedVar()
  {
    static std::atomic_uint value {};
    return value;
  }

  void passed()
  {
    ++getGlobalPassedVar();
  }
  void failed()
  {
    ++getGlobalFailedVar();
  }
  
  void installLogger()
  {
    TESTING_STDOUT() << "INSTALLING LOGGER...\n\n";
    ILogger::setLogLevel(zsLib::Log::Trace);
    ILogger::setLogLevel("zsLib", zsLib::Log::Trace);
    ILogger::setLogLevel("ortc_services", zsLib::Log::Trace);
    ILogger::setLogLevel("ortc_services_http", zsLib::Log::Trace);
    ILogger::setLogLevel("ortclib", zsLib::Log::Insane);

    if (ORTC_TEST_USE_DEBUGGER_LOGGING) {
      ILogger::installDebuggerLogger();
    }

    if (ORTC_TEST_USE_STDOUT_LOGGING) {
      ILogger::installStdOutLogger(false);
    }

    if (ORTC_TEST_USE_FIFO_LOGGING) {
      ILogger::installFileLogger(ORTC_TEST_FIFO_LOGGING_FILE, true);
    }

    if (ORTC_TEST_USE_TELNET_LOGGING) {
      bool serverMode = false;//(ORTC_TEST_DO_RUDPICESOCKET_CLIENT_TO_SERVER_TEST) && (!ORTC_TEST_RUNNING_AS_CLIENT);
      ILogger::installTelnetLogger(serverMode ? ORTC_TEST_TELNET_SERVER_LOGGING_PORT : ORTC_TEST_TELNET_LOGGING_PORT, zsLib::Seconds(60), true);

      for (int tries = 0; tries < 60; ++tries)
      {
        if (ILogger::isTelnetLoggerListening()) {
          break;
        }
        TESTING_SLEEP(1000)
      }
    }

    TESTING_STDOUT() << "INSTALLED LOGGER...\n\n";
  }
  
  void uninstallLogger()
  {
    TESTING_STDOUT() << "REMOVING LOGGER...\n\n";

    if (ORTC_TEST_USE_FIFO_LOGGING) {
      ILogger::uninstallFileLogger();
    }
    if (ORTC_TEST_USE_TELNET_LOGGING) {
      ILogger::uninstallTelnetLogger();
    }
    if (ORTC_TEST_USE_STDOUT_LOGGING) {
      ILogger::uninstallStdOutLogger();
    }
    if (ORTC_TEST_USE_DEBUGGER_LOGGING) {
      ILogger::uninstallDebuggerLogger();
    }

    TESTING_STDOUT() << "REMOVED LOGGER...\n\n";
  }
  
  class SetupInitializer
  {
  public:
    SetupInitializer()
    {
      srand(static_cast<signed int>(time(NULL)));
    }

    ~SetupInitializer()
    {
      output();
    }

  private:
    zsLib::SingletonManager::Initializer mInit;
  };

  static SetupInitializer &setupInitializer()
  {
    static SetupInitializer init;
    return init;
  }

  void setup()
  {
    setupInitializer();
  }

  void output()
  {
    TESTING_STDOUT() << "PASSED:       [" << Testing::getGlobalPassedVar() << "]\n";
    if (0 != Testing::getGlobalFailedVar()) {
      TESTING_STDOUT() << "***FAILED***: [" << Testing::getGlobalFailedVar() << "]\n";
    }
  }

  void runAllTests(void* videoSurface1, void* videoSurface2)
  {
    srand(static_cast<signed int>(time(NULL)));

    TESTING_INSTALL_LOGGER()

    setup();
    doSetup();

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

    TESTING_UNINSTALL_LOGGER()
  }
}
