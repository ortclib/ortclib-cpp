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



#include <ortc/internal/ortc_ORTC.h>

#include <ortc/IICEGatherer.h>

#include <ortc/types.h>
#include <ortc/internal/types.h>
#include <ortc/internal/platform.h>


#include <ortc/services/IHelper.h>

#include <zsLib/ISettings.h>
#include <zsLib/IMessageQueueThread.h>
#include <zsLib/IMessageQueueManager.h>
#include <zsLib/XML.h>

#include "config.h"
#include "testing.h"

#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif //HAVE_IFADDRS_H

#ifdef HAVE_IPHLPAPI_H
#include <Iphlpapi.h>
#endif //HAVE_IPHLPAPI_H

#ifdef WINRT
using namespace Windows::Networking::Connectivity;
#endif //WINRT

#ifdef WINRT
#include <ppltasks.h>
#endif //WINRT

#if defined(WINRT) && !defined(HAVE_GETADAPTERADDRESSES)
using Windows::Foundation::Collections::IVectorView;
using namespace concurrency;
using Windows::Networking::HostNameType;
using namespace Windows::UI::Core;
#endif //defined(WINRT) && !defined(HAVE_GETADAPTERADDRESSES)


namespace ortc { namespace test { ZS_DECLARE_SUBSYSTEM(ortc_test) } }

//using zsLib::String;
//using zsLib::ULONG;
//using zsLib::IMessageQueue;
using zsLib::Log;
using zsLib::AutoPUID;
using zsLib::RecursiveLock;
using zsLib::AutoRecursiveLock;
using namespace zsLib::XML;

#define ORTC_TEST_QUEUE_FOR_DELEGATE "org.ortc.test.delegateQueue"

ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper)
ZS_DECLARE_TYPEDEF_PTR(zsLib::ISettings, UseSettings)
ZS_DECLARE_TYPEDEF_PTR(ortc::internal::IORTCForInternal, UseORTC)
ZS_DECLARE_TYPEDEF_PTR(zsLib::IMessageQueueManager, UseMessageQueueManager)

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
          calculateInterfaces_Win32();
          calculateInterfaces_WinRT();

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

        //-------------------------------------------------------------------------
        void calculateInterfaces_Win32()
        {
#ifdef HAVE_GETADAPTERADDRESSES
          // https://msdn.microsoft.com/en-us/library/windows/desktop/aa365915(v=vs.85).aspx

#undef MALLOC
#undef FREE

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

          // scope: use GetAdaptersAddresses
      {
        ULONG flags = GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_MULTICAST;

        DWORD dwSize = 0;
        DWORD dwRetVal = 0;

        ULONG family = AF_UNSPEC;

        LPVOID lpMsgBuf = NULL;

        PIP_ADAPTER_ADDRESSES pAddresses = NULL;

        // Allocate a 15 KB buffer to start with.
        ULONG outBufLen = WORKING_BUFFER_SIZE;
        ULONG Iterations = 0;

        PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
        PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
        //PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
        //PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
        //IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
        //IP_ADAPTER_PREFIX *pPrefix = NULL;

        outBufLen = WORKING_BUFFER_SIZE;

        do {

          pAddresses = (IP_ADAPTER_ADDRESSES *)MALLOC(outBufLen);
          ZS_THROW_BAD_STATE_IF(NULL == pAddresses)

            dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

          if (dwRetVal != ERROR_BUFFER_OVERFLOW) break;

          FREE(pAddresses);
          pAddresses = NULL;

          Iterations++;
        } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

        if (NO_ERROR == dwRetVal) {
          pCurrAddresses = pAddresses;
          while (pCurrAddresses) {

            // discover information about the adapter
            {
              switch (pCurrAddresses->OperStatus) {
              case IfOperStatusDown:           goto next_address;
              case IfOperStatusNotPresent:     goto next_address;
              case IfOperStatusLowerLayerDown: goto next_address;
              }

              IPAddress ip;

              ULONG adapterMetric = ULONG_MAX;

              pUnicast = pCurrAddresses->FirstUnicastAddress;
              while (pUnicast) {
                // scan unicast addresses
                {
                  if (pUnicast->Address.lpSockaddr) {
                    switch (pUnicast->Address.lpSockaddr->sa_family) {
                    case AF_INET: {
                      ip = IPAddress(*((sockaddr_in *)pUnicast->Address.lpSockaddr));
                      adapterMetric = pCurrAddresses->Ipv4Metric;
                      break;
                    }
                    case AF_INET6: {
                      ip = IPAddress(*((sockaddr_in6 *)pUnicast->Address.lpSockaddr));
                      adapterMetric = pCurrAddresses->Ipv6Metric;
                      break;
                    }
                    }
                  }

                  if (ip.isAddressEmpty()) goto next_unicast;
                  if (ip.isLoopback()) goto next_unicast;
                  if (ip.isAddrAny()) goto next_unicast;

                  String friendlyName(pCurrAddresses->FriendlyName);
                  String description(pCurrAddresses->Description);

                  ZS_LOG_TRACE(log("found host IP") + ZS_PARAM("ip", ip.string()) + ZS_PARAM("interface", friendlyName) + ZS_PARAM("description", description))

                  ++mTotalHostIPs;
                }

              next_unicast:
                {
                  pUnicast = pUnicast->Next;
                }
              }
            }

          next_address:
            {
              pCurrAddresses = pCurrAddresses->Next;
            }
          }
        }
        else {
          ZS_LOG_WARNING(Detail, log("failed to obtain IP address information") + ZS_PARAMIZE(dwRetVal))
        }

        FREE(pAddresses);
      }
#endif //HAVE_GETADAPTERADDRESSES
        }

        //-------------------------------------------------------------------------
        void calculateInterfaces_WinRT()
        {
#if defined(WINRT) && !defined(HAVE_GETADAPTERADDRESSES)
          typedef Windows::Networking::Sockets::DatagramSocket DatagramSocket;
          typedef Windows::Networking::HostName HostName;
          typedef Windows::Networking::EndpointPair EndpointPair;

          // http://stackoverflow.com/questions/10336521/query-local-ip-address

          // Use WinRT GetHostNames to search for IP addresses
      {
        typedef std::map<String, bool> HostNameMap;
        typedef std::list<ConnectionProfile ^> ConnectionProfileList;

        HostNameMap previousFound;
        ConnectionProfileList profiles;

        // discover connection profiles
        {
          auto connectionProfiles = NetworkInformation::GetConnectionProfiles();
          for (auto iter = connectionProfiles->First(); iter->HasCurrent; iter->MoveNext()) {
            auto profile = iter->Current;
            if (nullptr == profile) {
              ZS_LOG_WARNING(Trace, log("found null profile"))
                continue;
            }
            profiles.push_back(profile);
          }

          ConnectionProfile ^current = NetworkInformation::GetInternetConnectionProfile();
          if (current) {
            ZS_LOG_INSANE("found current profile")
              profiles.push_back(current);
          }
        }

        // search connection profiles with host names found
        {
          auto hostnames = NetworkInformation::GetHostNames();
          for (auto iter = hostnames->First(); iter->HasCurrent; iter->MoveNext()) {
            auto hostname = iter->Current;
            if (nullptr == hostname) continue;

            String canonicalName;
            if (hostname->CanonicalName) {
              canonicalName = String(hostname->CanonicalName->Data());
            }

            String displayName;
            if (hostname->DisplayName) {
              displayName = String(hostname->DisplayName->Data());
            }

            String rawName;
            if (hostname->RawName) {
              rawName = String(hostname->RawName->Data());
            }

            String useName = rawName;

            auto found = previousFound.find(useName);
            if (found != previousFound.end()) {
              ZS_LOG_INSANE(log("already found IP") + ZS_PARAMIZE(useName))
                continue;
            }

            ConnectionProfile ^hostProfile = nullptr;
            if (hostname->IPInformation) {
              if (hostname->IPInformation->NetworkAdapter) {
                auto hostNetworkAdapter = hostname->IPInformation->NetworkAdapter;
                for (auto profileIter = profiles.begin(); profileIter != profiles.end(); ++profileIter) {
                  auto profile = (*profileIter);
                  auto adapter = profile->NetworkAdapter;
                  if (nullptr == adapter) {
                    ZS_LOG_WARNING(Insane, log("found null adapter"))
                      continue;
                  }
                  if (adapter->NetworkAdapterId != hostNetworkAdapter->NetworkAdapterId) {
                    ZS_LOG_INSANE(log("adapter does not match host adapter"))
                      continue;
                  }
                  // match found
                  hostProfile = profile;
                  break;
                }
              }
            }

            previousFound[useName] = true;

            IPAddress ip;

            if (IPAddress::isConvertable(useName)) {
              try {
                IPAddress temp(useName);
                ip = temp;
              }
              catch (IPAddress::Exceptions::ParseError &) {
                ZS_LOG_TRACE(log("name failed to resolve as IP") + ZS_PARAM("name", useName))
              }

              if (ip.isAddressEmpty()) continue;
              if (ip.isLoopback()) continue;
              if (ip.isAddrAny()) continue;
            }

            String profileName;

            if (hostProfile) {
              if (hostProfile->ProfileName) {
                profileName = String(hostProfile->ProfileName->Data());
              }
            }

            if (profileName.hasData()) {

              {
                AutoRecursiveLock lock(mLock);
                auto found = mFoundIPs.find(ip.string());
                if (found == mFoundIPs.end()) {
                  mFoundIPs[ip.string()] = true;
                }
              }

//              mPendingHostIPs.push_back(HostIPSorter::prepare(profileName, useName, ip, mInterfaceMappings, mOptions));
              continue;
            }
            //}
            //else {
            //  mPendingHostIPs.push_back(HostIPSorter::prepare(useName, ip, mOptions));
            //}

            PUID id = mID;
            auto pThis = this;

            Platform::String ^hostnameStr = ref new Platform::String(useName.wstring().c_str());
            Platform::String ^serviceNameStr = ref new Platform::String(L"0");

            HostName ^hostname2;
            try {
              hostname2 = ref new HostName(hostnameStr);
            }
            catch (Platform::Exception ^ex) {
              ZS_LOG_WARNING(Detail, log("exception caught") + ZS_PARAM("error", String(ex->Message->Data())))
              return;
            }
            HostNameType debugtype = hostname2->Type;

            ++mExpecting;

            create_task(DatagramSocket::GetEndpointPairsAsync(hostname2, serviceNameStr), mCancellationTokenSource.get_token())
              .then([id, pThis](task<IVectorView<EndpointPair^>^> previousTask)
            {
              if (!pThis) {
                return;
              }

              try
              {
                // Check if any previous task threw an exception.
                IVectorView<EndpointPair ^> ^response = previousTask.get();

                bool isSRV = false;

                if (nullptr != response) {
                  for (size_t index = 0; index != response->Size; ++index) {
                    EndpointPair ^pair = response->GetAt(index);
                    if (!pair) {
                      ZS_LOG_WARNING(Detail, slog(id, "endpoint pair is null"))
                      continue;
                    }

                    if (nullptr == pair->RemoteHostName) {
                      ZS_LOG_WARNING(Detail, slog(id, "remote host name is null"))
                      continue;
                    }

                    if (nullptr == pair->RemoteHostName->RawName) {
                      ZS_LOG_WARNING(Detail, slog(id, "remote host raw name is null"))
                      continue;
                    }

                    String host = String(pair->RemoteHostName->RawName->Data());
                    String canonical;
                    if (pair->RemoteHostName->CanonicalName) {
                      canonical = String(pair->RemoteHostName->CanonicalName->Data());
                    }
                    String displayName;
                    if (pair->RemoteHostName->DisplayName) {
                      displayName = String(pair->RemoteHostName->DisplayName->Data());
                    }
                    String service;
                    if (pair->RemoteServiceName) {
                      service = String(pair->RemoteServiceName->Data());
                    }

                    HostNameType type = pair->RemoteHostName->Type;
                    bool isIPv4 = (HostNameType::Ipv4 == type);
                    bool isIPv6 = (HostNameType::Ipv6 == type);

                    IPAddress ip;

                    try {
                      IPAddress tempIP(host);
                      ip = tempIP;
                    }
                    catch (IPAddress::Exceptions::ParseError &) {
                      ZS_LOG_WARNING(Debug, slog(id, "failed to convert to IP") + ZS_PARAM("host", host))
                      continue;
                    }

                    {
                      AutoRecursiveLock lock(pThis->mLock);
                      auto found = pThis->mFoundIPs.find(ip.string());
                      if (found == pThis->mFoundIPs.end()) {
                        pThis->mFoundIPs[ip.string()] = true;
                      }
                    }

                    ZS_LOG_TRACE(slog(id, "found result") + ZS_PARAM("host", host) + ZS_PARAM("canonical", canonical) + ZS_PARAM("display name", displayName) + ZS_PARAM("service", service) + ZS_PARAM("ip", ip.string()))

                  }
                }

                if (pThis) {
                  ++(pThis->mDone);
                }

              }
              catch (const task_canceled&) {
                ZS_LOG_WARNING(Detail, slog(id, "task cancelled"))
                if (pThis) {
                  ++(pThis->mDone);
                }
              }
              catch (Platform::Exception ^ex) {
                if (pThis) {
                  ZS_LOG_WARNING(Detail, slog(id, "exception caught") + ZS_PARAM("error", String(ex->Message->Data())))
                  ++(pThis->mDone);
                }
              }
            }, task_continuation_context::use_arbitrary());


          }
        }

        while (mExpecting != mDone)
        {
          TESTING_SLEEP(0)
          break;
        }

        mTotalHostIPs = mFoundIPs.size();
      }
#endif //defined(WINRT) && !defined(HAVE_GETADAPTERADDRESSES)
        }


        //---------------------------------------------------------------------
        Log::Params log(const char *message) const
        {
          ElementPtr objectEl = Element::create("ortc::test::ICEGathererTester");
          UseServicesHelper::debugAppend(objectEl, "id", mID);
          return Log::Params(message, objectEl);
        }

        //---------------------------------------------------------------------
        static Log::Params slog(PUID id, const char *message)
        {
          ElementPtr objectEl = Element::create("ortc::test::ICEGathererTester");
          UseServicesHelper::debugAppend(objectEl, "id", id);
          return Log::Params(message, objectEl);
        }

      public:
        mutable RecursiveLock mLock;

        std::map<String, bool> mFoundIPs;

        AutoPUID mID;

        std::atomic<size_t> mExpecting {};
        std::atomic<size_t> mDone {};
        std::atomic<size_t> mTotalHostIPs {};

#if defined(WINRT) && !defined(HAVE_GETADAPTERADDRESSES)
        concurrency::cancellation_token_source mCancellationTokenSource;
#endif //defined(WINRT) && !defined(HAVE_GETADAPTERADDRESSES)
      };

    }
  }
}

ZS_DECLARE_USING_PTR(ortc::test::setup, TestSetup)


void doSetup()
{
  ortc::internal::IORTCForInternal::overrideQueueDelegate(UseMessageQueueManager::getMessageQueue(ORTC_TEST_QUEUE_FOR_DELEGATE));
  UseSettings::applyDefaults();

  TestSetup setup;
}
