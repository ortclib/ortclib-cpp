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


#include <zsLib/MessageQueueThread.h>

#include <ortc/IICEGatherer.h>
#include <ortc/ISettings.h>

#include <ortc/types.h>
#include <ortc/internal/types.h>
#include <ortc/internal/platform.h>


#include <openpeer/services/IHelper.h>

#include <zsLib/XML.h>

#include "config.h"
#include "testing.h"

#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif //HAVE_IFADDRS_H

namespace ortc { namespace test { ZS_DECLARE_SUBSYSTEM(ortc_test) } }

//using zsLib::String;
//using zsLib::ULONG;
//using zsLib::IMessageQueue;
using zsLib::Log;
using zsLib::AutoPUID;
//using zsLib::AutoRecursiveLock;
using namespace zsLib::XML;

ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
ZS_DECLARE_TYPEDEF_PTR(ortc::ISettings, UseSettings)

namespace ortc
{
  namespace test
  {
    namespace setup
    {

      ZS_DECLARE_CLASS_PTR(TestSetup)

      class TestSetup
      {
      public:
        //---------------------------------------------------------------------
        TestSetup()
        {
          calculateInterfaces_ifaddr();

          UseSettings::setUInt("tester/total-host-ips", mTotalHostIPs);
        }

        //---------------------------------------------------------------------
        void calculateInterfaces_ifaddr()
        {
#ifdef HAVE_GETIFADDRS
          // scope: use getifaddrs
          {
            ifaddrs *ifAddrStruct = NULL;
            ifaddrs *ifa = NULL;

            getifaddrs(&ifAddrStruct);

            for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next)
            {
              IPAddress ip;
              if (AF_INET == ifa->ifa_addr->sa_family) {
                ip = IPAddress(*((sockaddr_in *)ifa->ifa_addr));      // this is an IPv4 address
              } else if (AF_INET6 == ifa->ifa_addr->sa_family) {
                ip = IPAddress(*((sockaddr_in6 *)ifa->ifa_addr));     // this is an IPv6 address
              }

              // do not add these addresses...
              if (ip.isAddressEmpty()) continue;
              if (ip.isLoopback()) continue;
              if (ip.isAddrAny()) continue;

              ZS_LOG_TRACE(log("found host IP") + ZS_PARAM("ip", ip.string()) + ZS_PARAM("interface", ifa->ifa_name))

              ++mTotalHostIPs;
            }
            
            if (ifAddrStruct) {
              freeifaddrs(ifAddrStruct);
              ifAddrStruct = NULL;
            }
          }
#endif //HAVE_GETIFADDRS
        }

        //---------------------------------------------------------------------
        Log::Params log(const char *message) const
        {
          ElementPtr objectEl = Element::create("ortc::test::ICEGathererTester");
          UseServicesHelper::debugAppend(objectEl, "id", mID);
          return Log::Params(message, objectEl);
        }

      private:
        AutoPUID mID;

        size_t mTotalHostIPs {};
      };

    }
  }
}

ZS_DECLARE_USING_PTR(ortc::test::setup, TestSetup)


void doSetup()
{
  ortc::ISettings::applyDefaults();

  TestSetup setup;
}
