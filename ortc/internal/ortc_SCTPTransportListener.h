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
    ZS_DECLARE_CLASS_PTR(SCTPTransportListener)

    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportForSCTPTransportListener)

    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportListenerForSettings)
    ZS_DECLARE_INTERACTION_PTR(ISCTPTransportListenerForSCTPTransport)

    ZS_DECLARE_INTERACTION_PROXY(ISCTPTransportListenerAsyncDelegate)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportListenerForSettings
    #pragma mark

    interaction ISCTPTransportListenerForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportListenerForSettings, ForSettings)

      static void applyDefaults();

      virtual ~ISCTPTransportListenerForSettings() {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportListenerForSCTPTransport
    #pragma mark

    interaction ISCTPTransportListenerForSCTPTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportListenerForSCTPTransport, ForSCTPTransport)

      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForSCTPTransportListener, UseSCTPTransport)

      static ElementPtr toDebug(ForSCTPTransportPtr transport);

      virtual PUID getID() const = 0;

      virtual WORD allocateLocalPort() = 0;
      virtual void deallocateLocalPort(WORD previouslyAllocatedLocalPort) = 0;

      virtual void registerNewTransport(
                                        IDTLSTransportPtr dtlsTransport,
                                        UseSCTPTransportPtr &ioTransport,
                                        WORD &ioLocalPort,
                                        bool localPortWasPreallocated,
                                        WORD &ioRemotePort
                                        ) = 0;

      virtual void announceTransport(
                                     UseSCTPTransportPtr transport,
                                     WORD localPort,
                                     WORD remotePort
                                     ) = 0;

      virtual void notifyShutdown(
                                  UseSCTPTransport &transport,
                                  WORD localPort,
                                  WORD remotePort
                                  ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportListenerAsyncDelegate
    #pragma mark

    interaction ISCTPTransportListenerAsyncDelegate
    {
      virtual ~ISCTPTransportListenerAsyncDelegate() {};
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
    #pragma mark
    #pragma mark SCTPTransportListener
    #pragma mark

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

      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, UseSecureTransport)
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportForSCTPTransportListener, UseSCTPTransport)

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
                            );

      SCTPTransportListener(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

    protected:
      void init();

    public:

      virtual ~SCTPTransportListener();

      static SCTPTransportListenerPtr convert(ForSettingsPtr object);
      static SCTPTransportListenerPtr convert(ForSCTPTransportPtr object);
      static SCTPTransportListenerPtr convert(ForSecureTransportPtr object);

      static ElementPtr toDebug(SCTPTransportListenerPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ISCTPTransport
      #pragma mark

      static CapabilitiesPtr getCapabilities();

      static ISCTPTransportListenerSubscriptionPtr listen(
                                                          ISCTPTransportListenerDelegatePtr delegate,
                                                          IDTLSTransportPtr transport,
                                                          const Capabilities &remoteCapabilities
                                                          );
      
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => ISCTPTransportListenerForSCTPTransport
      #pragma mark

      virtual PUID getID() const override {return mID;}

      virtual WORD allocateLocalPort() override;
      virtual void deallocateLocalPort(WORD previouslyAllocatedLocalPort) override;

      virtual void registerNewTransport(
                                        IDTLSTransportPtr dtlsTransport,
                                        UseSCTPTransportPtr &ioTransport,
                                        WORD &ioLocalPort,
                                        bool localPortWasPreallocated,
                                        WORD &ioRemotePort
                                        ) override;

      virtual void announceTransport(
                                     UseSCTPTransportPtr transport,
                                     WORD localPort,
                                     WORD remotePort
                                     ) override;

      virtual void notifyShutdown(
                                  UseSCTPTransport &transport,
                                  WORD localPort,
                                  WORD remotePort
                                  ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => IDataTransportForSecureTransport
      #pragma mark

      // (duplciate) static ElementPtr toDebug(ForDTLSTransportPtr transport);

      // (duplicate) virtual PUID getID() const = 0;

      static ForSecureTransportPtr create(UseSecureTransportPtr transport);

      virtual bool handleDataPacket(
                                    const BYTE *buffer,
                                    size_t bufferLengthInBytes
                                    ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransportListener => ISCTPTransportListenerAsyncDelegate
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransportListener => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      static Log::Params slog(const char *message);
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      void cancel();

      virtual ISCTPTransportListenerSubscriptionPtr subscribe(
                                                              ISCTPTransportListenerDelegatePtr originalDelegate,
                                                              const Capabilities &remoteCapabilities
                                                              );

      WORD allocateLocalPort(WORD remotePort);
      void allocatePort(
                        AllocatedPortMap &useMap,
                        WORD port
                        );
      void deallocatePort(
                          AllocatedPortMap &useMap,
                          WORD port
                          );

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SCTPTransport => (data)
      #pragma mark

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
    #pragma mark
    #pragma mark ISCTPTransportListenerFactory
    #pragma mark

    interaction ISCTPTransportListenerFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransportTypes::Capabilities, Capabilities)

      ZS_DECLARE_TYPEDEF_PTR(IDataTransportForSecureTransport, ForSecureTransport)
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, UseSecureTransport)

      static ISCTPTransportListenerFactory &singleton();

      virtual ISCTPTransportListenerSubscriptionPtr listen(
                                                           ISCTPTransportListenerDelegatePtr delegate,
                                                           IDTLSTransportPtr transport,
                                                           const Capabilities &remoteCapabilities
                                                           );

      virtual ForSecureTransportPtr create(UseSecureTransportPtr transport);

      virtual CapabilitiesPtr getCapabilities();
    };

    class SCTPTransportListenerFactory : public IFactory<ISCTPTransportListenerFactory> {};
  }
}
