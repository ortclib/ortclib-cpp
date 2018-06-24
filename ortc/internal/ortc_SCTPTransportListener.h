/*

 Copyright (c) 2015, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/internal/types.h>

#include <ortc/ISCTPTransport.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IICETransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>
#include <zsLib/ProxySubscriptions.h>
#include <zsLib/TearAway.h>

#include <usrsctp.h>

#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE      "ortc/sctp/max-message-size"

#define ORTC_SETTING_SCTP_TRANSPORT_LISTENER_MAX_PORTS    "ortc/sctp/max-ports"
#define ORTC_SETTING_SCTP_TRANSPORT_LISTENER_MIN_PORT     "ortc/sctp/min-port"
#define ORTC_SETTING_SCTP_TRANSPORT_LISTENER_MAX_PORT     "ortc/sctp/max-port"
#define ORTC_SETTING_SCTP_TRANSPORT_LISTENER_DEFAULT_PORT "ortc/sctp/default-port"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(SCTPTransportListener);

    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForSCTPTransportListener);

    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportListenerForSettings);
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportListenerForSCTPTransport);

    ZS_DECLARE_INTERACTION_PROXY(ISCTPTransportListenerAsyncDelegate);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISCTPTransportListenerForSettings
    //

    interaction ISCTPTransportListenerForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportListenerForSettings, ForSettings);

      static void applyDefaults() noexcept;

      virtual ~ISCTPTransportListenerForSettings() noexcept {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISCTPTransportListenerForSCTPTransport
    //

    interaction ISCTPTransportListenerForSCTPTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportListenerForSCTPTransport, ForSCTPTransport);

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForSCTPTransportListener, UseSCTPTransport);

      static ElementPtr toDebug(ForSCTPTransportPtr transport) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual WORD allocateLocalPort() noexcept = 0;
      virtual void deallocateLocalPort(WORD previouslyAllocatedLocalPort) noexcept = 0;

      virtual void registerNewTransport(
                                        IDTLSTransportPtr dtlsTransport,
                                        UseSCTPTransportPtr &ioTransport,
                                        WORD &ioLocalPort,
                                        bool localPortWasPreallocated,
                                        WORD &ioRemotePort
                                        ) noexcept(false) = 0; // throws InvalidParameters

      virtual void announceTransport(
                                     UseSCTPTransportPtr transport,
                                     WORD localPort,
                                     WORD remotePort
                                     ) noexcept = 0;

      virtual void notifyShutdown(
                                  UseSCTPTransport &transport,
                                  WORD localPort,
                                  WORD remotePort
                                  ) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISCTPTransportListenerAsyncDelegate
    //

    interaction ISCTPTransportListenerAsyncDelegate
    {
      virtual ~ISCTPTransportListenerAsyncDelegate() noexcept {};
    };
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::ISCTPTransportListenerAsyncDelegate)
ZS_DECLARE_PROXY_END()

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SCTPTransportListener
    //

    class SCTPTransportListener : public Noop,
                                  public MessageQueueAssociator,
                                  public SharedRecursiveLock,
                                  public ISCTPTransportTypes,
                                  public ISCTPTransportListenerForSettings,
                                  public ISCTPTransportListenerForSCTPTransport,
                                  public IDataTransportForSecureTransport,
                                  public ISCTPTransportListenerAsyncDelegate,
                                  public IWakeDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction ISCTPTransportListenerFactory;
      friend interaction ISCTPTransportListenerForSettings;
      friend interaction ISCTPTransportListenerForSCTPTransport;
      friend interaction IDataTransportForSecureTransport;

      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, UseSecureTransport);
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForSCTPTransportListener, UseSCTPTransport);

      typedef DWORD LocalRemoteTupleID;
      typedef std::map<LocalRemoteTupleID, UseSCTPTransportPtr> TransportMap;

      typedef std::pair<LocalRemoteTupleID, UseSCTPTransportPtr> TupleSCTPTransportPair;

      typedef PUID SCTPTransportID;
      typedef WORD Port;
      typedef std::pair<UseSCTPTransportPtr, Port> TransportPortPair;
      typedef std::map<SCTPTransportID, UseSCTPTransportPtr> TransportIDMap;
      typedef std::map<SCTPTransportID, TransportPortPair> TransportWithPortMap;

      typedef std::map<WORD, size_t> AllocatedPortMap;

    public:
      SCTPTransportListener(
                            const make_private &,
                            IMessageQueuePtr queue,
                            UseSecureTransportPtr secureTransport
                            ) noexcept;

      SCTPTransportListener(Noop) noexcept :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

    protected:
      void init() noexcept;

    public:

      virtual ~SCTPTransportListener() noexcept;

      static SCTPTransportListenerPtr convert(ForSettingsPtr object) noexcept;
      static SCTPTransportListenerPtr convert(ForSCTPTransportPtr object) noexcept;
      static SCTPTransportListenerPtr convert(ForSecureTransportPtr object) noexcept;

      static ElementPtr toDebug(SCTPTransportListenerPtr object) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // SCTPTransport => ISCTPTransport
      //

      static CapabilitiesPtr getCapabilities() noexcept;

      static ISCTPTransportListenerSubscriptionPtr listen(
                                                          ISCTPTransportListenerDelegatePtr delegate,
                                                          IDTLSTransportPtr transport,
                                                          const Capabilities &remoteCapabilities
                                                          ) noexcept(false); // throws InvalidParameters
      
      //-----------------------------------------------------------------------
      //
      // SCTPTransport => ISCTPTransportListenerForSCTPTransport
      //

      PUID getID() const noexcept override {return mID;}

      WORD allocateLocalPort() noexcept override;
      void deallocateLocalPort(WORD previouslyAllocatedLocalPort) noexcept override;

      void registerNewTransport(
                                IDTLSTransportPtr dtlsTransport,
                                UseSCTPTransportPtr &ioTransport,
                                WORD &ioLocalPort,
                                bool localPortWasPreallocated,
                                WORD &ioRemotePort
                                ) noexcept(false) override; // throws InvalidParameters

      void announceTransport(
                             UseSCTPTransportPtr transport,
                             WORD localPort,
                             WORD remotePort
                             ) noexcept override;

      void notifyShutdown(
                          UseSCTPTransport &transport,
                          WORD localPort,
                          WORD remotePort
                          ) noexcept override;

      //-----------------------------------------------------------------------
      //
      // SCTPTransport => IDataTransportForSecureTransport
      //

      // (duplciate) static ElementPtr toDebug(ForDTLSTransportPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      static ForSecureTransportPtr create(UseSecureTransportPtr transport) noexcept;

      bool handleDataPacket(
                            const BYTE *buffer,
                            size_t bufferLengthInBytes
                            ) noexcept override;

      //-----------------------------------------------------------------------
      //
      // SCTPTransportListener => ISCTPTransportListenerAsyncDelegate
      //

      //-----------------------------------------------------------------------
      //
      // SCTPTransportListener => IWakeDelegate
      //

      void onWake() override;

    protected:
      //-----------------------------------------------------------------------
      //
      // SCTPTransport => (internal)
      //

      Log::Params log(const char *message) const noexcept;
      static Log::Params slog(const char *message) noexcept;
      Log::Params debug(const char *message) const noexcept;
      virtual ElementPtr toDebug() const noexcept;

      bool isShuttingDown() const noexcept;
      bool isShutdown() const noexcept;

      void step() noexcept;
      void cancel() noexcept;

      virtual ISCTPTransportListenerSubscriptionPtr subscribe(
                                                              ISCTPTransportListenerDelegatePtr originalDelegate,
                                                              const Capabilities &remoteCapabilities
                                                              ) noexcept;

      WORD allocateLocalPort(WORD remotePort) noexcept;
      void allocatePort(
                        AllocatedPortMap &useMap,
                        WORD port
                        ) noexcept;
      void deallocatePort(
                          AllocatedPortMap &useMap,
                          WORD port
                          ) noexcept;

    protected:
      //-----------------------------------------------------------------------
      //
      // SCTPTransport => (data)
      //

      AutoPUID mID;
      SCTPTransportListenerWeakPtr mThisWeak;
      SCTPTransportListenerPtr mGracefulShutdownReference;

      ISCTPTransportListenerDelegateSubscriptions mSubscriptions;

      UseSecureTransportWeakPtr mSecureTransport;

      bool mShutdown {false};

      TransportMap mTransports;
      TransportWithPortMap mPendingTransports;
      TransportIDMap mAnnouncedTransports;

      AllocatedPortMap mAllocatedLocalPorts;
      AllocatedPortMap mAllocatedRemotePorts;

      size_t mMaxPorts {};

      WORD mCurrentAllocationPort {};
      WORD mMinAllocationPort {5000};
      WORD mMaxAllocationPort {65535};
      WORD mNextAllocationIncremement {1};

      CapabilitiesPtr mRemoteCapabilities;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISCTPTransportListenerFactory
    //

    interaction ISCTPTransportListenerFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportTypes::Capabilities, Capabilities);

      ZS_DECLARE_TYPEDEF_PTR(IDataTransportForSecureTransport, ForSecureTransport);
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, UseSecureTransport);

      static ISCTPTransportListenerFactory &singleton() noexcept;

      virtual ISCTPTransportListenerSubscriptionPtr listen(
                                                           ISCTPTransportListenerDelegatePtr delegate,
                                                           IDTLSTransportPtr transport,
                                                           const Capabilities &remoteCapabilities
                                                           ) noexcept;

      virtual ForSecureTransportPtr create(UseSecureTransportPtr transport) noexcept;

      virtual CapabilitiesPtr getCapabilities() noexcept;
    };

    class SCTPTransportListenerFactory : public IFactory<ISCTPTransportListenerFactory> {};
  }
}
