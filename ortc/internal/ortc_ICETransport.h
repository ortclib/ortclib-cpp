/*

 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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

#if 0

#include <ortc/internal/types.h>
#include <ortc/IICETransport.h>

#include <openpeer/services/IICESocket.h>
#include <openpeer/services/IICESocketSession.h>
#include <openpeer/services/IWakeDelegate.h>

#include <zsLib/MessageQueueAssociator.h>

namespace ortc
{
  namespace internal
  {
    interaction IDTLSTransportForICETransport;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForDTLS
    #pragma mark

    interaction IICETransportForDTLSTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForDTLSTransport, ForDTLSTransport)

      typedef IICETransport::ConnectionStates ConnectionStates;
      typedef IICETransport::Roles Roles;

      static ElementPtr toDebug(ForDTLSTransportPtr transport);

      virtual PUID getID() const = 0;

      virtual void attach(DTLSTransportPtr dtlsTransport) = 0;
      virtual void detach(DTLSTransport &dtlsTransport) = 0;

      virtual ConnectionStates getState(
                                        WORD *outError = NULL,
                                        String *outReason = NULL
                                        ) = 0;

      virtual Roles getRole() = 0;

      virtual bool sendPacket(
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              ) const = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransport
    #pragma mark

    class ICETransport : public Noop,
                         public MessageQueueAssociator,
                         public IICETransport,
                         public IICETransportForDTLSTransport,
                         public IWakeDelegate,
                         public IICESocketDelegate,
                         public IICESocketSessionDelegate
    {
    public:
      friend interaction IICETransport;
      friend interaction IICETransportFactory;
      friend interaction IICETransportForDTLSTransport;

      ZS_DECLARE_TYPEDEF_PTR(IDTLSTransportForICETransport, UseDTLSTransport)

      typedef IICETransport::ConnectionStates ConnectionStates;
      typedef IICETransport::Roles Roles;

      typedef std::list<CandidateInfoPtr> CandidateInfoList;
      typedef IICESocket::CandidateList CandidateListInner;
      typedef CandidateInfoList CandidateListOuter;

    protected:
      ICETransport(
                   IMessageQueuePtr queue,
                   IICETransportDelegatePtr delegate,
                   ServerListPtr servers
                   );

      ICETransport(Noop) : Noop(true), MessageQueueAssociator(IMessageQueuePtr()) {}

      void init();

    public:
      virtual ~ICETransport();

      static ICETransportPtr convert(IICETransportPtr object);
      static ICETransportPtr convert(ForDTLSTransportPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransport
      #pragma mark

      static ElementPtr toDebug(IICETransportPtr transport);

      static ICETransportPtr create(
                                    IICETransportDelegatePtr delegate,
                                    ServerListPtr servers
                                    );

      virtual PUID getID() const;

      virtual IICETransportSubscriptionPtr subscribe(IICETransportDelegatePtr delegate);

      static CapabilitiesPtr getCapabilities();

      virtual TransportInfoPtr createParams(CapabilitiesPtr capabilities = CapabilitiesPtr());

      static TransportInfoPtr filterParams(
                                           TransportInfoPtr params,
                                           CapabilitiesPtr capabilities
                                           );

      virtual TransportInfoPtr getLocal();
      virtual TransportInfoPtr getRemote();

      virtual void setLocal(TransportInfoPtr info);
      virtual void setRemote(TransportInfoPtr info);

      virtual void start(
                         TransportInfoPtr localTransportInfo,
                         Roles role
                         );
      virtual void stop();

      virtual ConnectionStates getState(
                                        WORD *outError = NULL,
                                        String *outReason = NULL
                                        );
      virtual Roles getRole();

      virtual void addRemoteCandidate(CandidateInfoPtr candidate);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICETransportForDTLS
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      virtual void attach(DTLSTransportPtr dtlsTransport);
      virtual void detach(DTLSTransport &dtlsTransport);

      // (duplicate) virtual ConnectionStates getState(
      //                                               WORD *outError = NULL,
      //                                               String *outReason = NULL
      //                                               );

      // (duplicate) virtual Roles getRole();

      virtual bool sendPacket(
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              ) const;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IWakeDelegate
      #pragma mark

      virtual void onWake();

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICESocketDelegate
      #pragma mark

      virtual void onICESocketStateChanged(
                                           IICESocketPtr socket,
                                           ICESocketStates state
                                           );

      virtual void onICESocketCandidatesChanged(IICESocketPtr socket);


      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IICESocketSessionDelegate
      #pragma mark

      virtual void onICESocketSessionStateChanged(
                                                  IICESocketSessionPtr session,
                                                  ICESocketSessionStates state
                                                  );

      virtual void onICESocketSessionNominationChanged(IICESocketSessionPtr session);

      virtual void handleICESocketSessionReceivedPacket(
                                                        IICESocketSessionPtr session,
                                                        const BYTE *buffer,
                                                        size_t bufferLengthInBytes
                                                        );

      virtual bool handleICESocketSessionReceivedSTUNPacket(
                                                            IICESocketSessionPtr session,
                                                            STUNPacketPtr stun,
                                                            const String &localUsernameFrag,
                                                            const String &remoteUsernameFrag
                                                            );

      virtual void onICESocketSessionWriteReady(IICESocketSessionPtr session);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;

      RecursiveLock &getLock() const {return mLock;}

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepSocketCreate();
      bool stepSocket();
      bool stepSession();

      void cancel();

      void setState(ConnectionStates state);
      void setError(WORD error, const char *reason = NULL);

      CandidateInfoPtr findOrCreate(
                                    const IICESocket::Candidate &candidate,
                                    bool *outDidCreate = NULL
                                    );

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => (data)
      #pragma mark

      mutable RecursiveLock mLock;
      ICETransportWeakPtr mThisWeak;
      ICETransportPtr mGracefulShutdownReference;
      AutoPUID mID;

      IICETransportDelegateSubscriptions mSubscriptions;
      IICETransportSubscriptionPtr mDefaultSubscription;

      ConnectionStates mCurrentState;
      bool mStartCalled {};

      WORD mLastError {};
      String mLastErrorReason;

      ServerListPtr mServers;

      Roles mDefaultRole;

      TransportInfoPtr mLocal;
      TransportInfoPtr mRemote;

      IICESocketPtr mSocket;
      IICESocketSubscriptionPtr mSocketSubscription;
      bool mNotifiedCandidatesEnd {};
      String mCandidatesVersion;
      CandidateListInner mLocalCandidatesInner;
      CandidateListOuter mLocalCandidatesOuter;

      IICESocketSessionPtr mSession;
      IICESocketSessionSubscriptionPtr mSessionSubscription;
      bool mNominationChanged {};

      CandidateListOuter mPendingRemoteCandidates;
      CandidateListInner mAddedRemoteCandidates;

      PUID mAttachedDTLSTransportID;
      UseDTLSTransportWeakPtr mDTLSTransport;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportFactory
    #pragma mark

    interaction IICETransportFactory
    {
      static IICETransportFactory &singleton();

      virtual ICETransportPtr create(
                                     IICETransportDelegatePtr delegate,
                                     IICETransport::ServerListPtr servers
                                     );
    };

    class ICETransportFactory : public IFactory<IICETransportFactory> {};
  }
}

#endif //0
