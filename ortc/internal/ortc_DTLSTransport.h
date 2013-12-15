/*

 Copyright (c) 2013, SMB Phone Inc. / Hookflash Inc.
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
#include <ortc/internal/ortc_ICETransport.h>

#include <ortc/TrackDescription.h>
#include <ortc/IDTLSTransport.h>

#include <openpeer/services/IWakeDelegate.h>

#include <zsLib/MessageQueueAssociator.h>

namespace ortc
{
  namespace internal
  {
    interaction IICETransportForDTLSTransport;
    interaction IRTPReceiverForDTLSTransport;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportForRTPSender
    #pragma mark

    interaction IDTLSTransportForRTPSender
    {
      ZS_DECLARE_TYPEDEF_PTR(IDTLSTransportForRTPSender, ForRTPSender)

      static ElementPtr toDebug(ForRTPSenderPtr transport);

      virtual PUID getID() const = 0;

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
    #pragma mark IDTLSTransportForRTPReceiver
    #pragma mark

    interaction IDTLSTransportForRTPReceiver
    {
      ZS_DECLARE_TYPEDEF_PTR(IDTLSTransportForRTPReceiver, ForRTPReceiver)

      typedef RTPFlowParams::SSRC SSRC;

      static ElementPtr toDebug(ForRTPReceiverPtr transport);

      virtual PUID getID() const = 0;

      virtual void attachRouting(
                                 RTPReceiverPtr receiver,
                                 SSRC route
                                 ) = 0;

      virtual void detachRouting(
                                 RTPReceiver &receiver,
                                 SSRC route
                                 ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportForICETransport
    #pragma mark

    interaction IDTLSTransportForICETransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IDTLSTransportForICETransport, ForICETransport)

      virtual PUID getID() const = 0;

      virtual void handleReceivedPacket(
                                        ICETransportPtr iceTransport,
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportForDTLSContext
    #pragma mark

    interaction IDTLSTransportForDTLSContext
    {
      ZS_DECLARE_TYPEDEF_PTR(IDTLSTransportForDTLSContext, ForDTLSContext)

      virtual PUID getID() const = 0;

      virtual RecursiveLock &getLock() const = 0;

      virtual bool sendDTLSContextPacket(
                                         const BYTE *buffer,
                                         size_t bufferLengthInBytes
                                         ) = 0;

      virtual void notifyDTLSContextConnected() = 0;
      virtual void notifyDTLSContextShutdown() = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark DTLSTransport
    #pragma mark
    
    class DTLSTransport : public Noop,
                          public MessageQueueAssociator,
                          public IDTLSTransport,
                          public IDTLSTransportForRTPSender,
                          public IDTLSTransportForRTPReceiver,
                          public IDTLSTransportForICETransport,
                          public IDTLSTransportForDTLSContext,
                          public IWakeDelegate,
                          public IICETransportDelegate
    {
    public:
      friend interaction IDTLSTransport;
      friend interaction IDTLSTransportFactory;
      friend interaction IDTLSTransportForICETransport;

      ZS_DECLARE_TYPEDEF_PTR(IICETransportForDTLSTransport, UseICETransport)
      ZS_DECLARE_TYPEDEF_PTR(IRTPReceiverForDTLSTransport, UseRTPReceiver)

      ZS_DECLARE_TYPEDEF_PTR(std::list<SecureByteBlockPtr>, PendingDTLSBufferList)

      typedef std::map<SSRC, UseRTPReceiverWeakPtr> RTPRoutes;

    protected:
      class DTLSContext;
      typedef boost::shared_ptr<DTLSContext> DTLSContextPtr;
      typedef boost::weak_ptr<DTLSContext> DTLSContextWeakPtr;

      friend class DTLSContext;

    protected:
      DTLSTransport(
                    IMessageQueuePtr queue,
                    IDTLSTransportDelegatePtr delegate,
                    IICETransportPtr iceTransport
                    );

      DTLSTransport(Noop) : Noop(true), MessageQueueAssociator(IMessageQueuePtr()) {}

      void init();

    public:
      virtual ~DTLSTransport();

      static DTLSTransportPtr convert(IDTLSTransportPtr object);
      static DTLSTransportPtr convert(ForRTPSenderPtr object);
      static DTLSTransportPtr convert(ForRTPReceiverPtr object);
      static DTLSTransportPtr convert(ForICETransportPtr object);
      static DTLSTransportPtr convert(ForDTLSContextPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IDTLSTransport
      #pragma mark

      static ElementPtr toDebug(IDTLSTransportPtr transport);

      static DTLSTransportPtr create(
                                     IDTLSTransportDelegatePtr delegate,
                                     IICETransportPtr iceTransport
                                     );

      virtual PUID getID() const;

      virtual IDTLSTransportSubscriptionPtr subscribe(IDTLSTransportDelegatePtr delegate);

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

      virtual void attach(IICETransportPtr iceTransport);

      virtual void start(TransportInfoPtr localTransportInfo);
      virtual void stop();

      virtual ConnectionStates getState(
                                        WORD *outError = NULL,
                                        String *outReason = NULL
                                        );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IDTLSTransportForRTPSender
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      virtual bool sendPacket(
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              ) const;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IDTLSTransportForRTPReceiver
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      virtual void attachRouting(
                                 RTPReceiverPtr receiver,
                                 SSRC route
                                 );

      virtual void detachRouting(
                                 RTPReceiver &receiver,
                                 SSRC route
                                 );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IDTLSTransportForICETransport
      #pragma mark

      // (duplicated) virtual PUID getID() const;

      virtual void handleReceivedPacket(
                                        ICETransportPtr iceTransport,
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IDTLSTransportForDTLSContext
      #pragma mark

      // (duplicate) virtual PUID getID() const;

      // (duplicate) virtual RecursiveLock &getLock() const;

      virtual bool sendDTLSContextPacket(
                                         const BYTE *buffer,
                                         size_t bufferLengthInBytes
                                         );

      virtual void notifyDTLSContextConnected();
      virtual void notifyDTLSContextShutdown();

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IWakeDelegate
      #pragma mark

      virtual void onWake();

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => IICETransportDelegate
      #pragma mark

      virtual void onICETransportCandidatesChangeDetected(IICETransportPtr transport);

      virtual void onICETransportCandidate(
                                           IICETransportPtr transport,
                                           IICETransport::CandidateInfoPtr candidate
                                           );

      virtual void onICETransportEndOfCandidates(IICETransportPtr trannsport);

      virtual void onICETransportActiveCandidate(
                                                 IICETransportPtr transport,
                                                 IICETransport::CandidateInfoPtr localCandidate,
                                                 IICETransport::CandidateInfoPtr remoteCandidate
                                                 );

      virtual void onICETransportStateChanged(
                                              IICETransportPtr transport,
                                              IICETransport::ConnectionStates state
                                              );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport::DTLSContext
      #pragma mark

      class DTLSContext
      {
      public:
        typedef IDTLSTransportForDTLSContext Transport;
        typedef boost::shared_ptr<IDTLSTransportForDTLSContext> TransportPtr;
        typedef boost::weak_ptr<IDTLSTransportForDTLSContext> TransportWeakPtr;

      protected:
        DTLSContext(
                    DTLSTransportPtr transport,
                    bool inClientRole
                    );

        void init();

      public:
        ~DTLSContext();

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark DTLSTransport::DTLSContext => friend DTLSTransport
        #pragma mark

        static ElementPtr toDebug(DTLSContextPtr context);

        static DTLSContextPtr create(
                                     DTLSTransportPtr transport,
                                     IICETransport::Roles role
                                     );

        PUID getID() const {return mID;}

        void detach();

        void freeze();    // tell the DTLS context that the transport is temporarily unavailable
        void unfreeze();  // tell the DTLS context that the transport is now available again

        void shutdown();

        // returns true if handled, otherwise false
        bool handleIfDTLSContextPacket(
                                       const BYTE *buffer,
                                       size_t bufferLengthInBytes
                                       );

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark DTLSTransport::DTLSContext => (internal)
        #pragma mark

        Log::Params log(const char *message) const;
        Log::Params debug(const char *message) const;
        ElementPtr toDebug() const;

        RecursiveLock &getLock() const;

        bool isClient() const {return mInClientRole;}
        bool isServer() const {return !mInClientRole;}

        void cancel();

        bool sendPacket(
                        const BYTE *buffer,
                        size_t bufferLengthInBytes
                        ) const;

        void notifyConnected();
        void notifyShutdown();

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark DTLSTransport::DTLSContext => (data)
        #pragma mark

        AutoPUID mID;
        mutable RecursiveLock mBogusLock;
        DTLSContextWeakPtr mThisWeak;

        bool mInClientRole;

        TransportWeakPtr mTransport;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;

      virtual RecursiveLock &getLock() const {return mLock;}

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();            // WARNING: DO NOT CALL WITHIN CONTEXT OF LOCK
      bool stepICESession();
      bool stepFixState();
      bool stepSendPending();

      void cancel();

      void setState(ConnectionStates state);
      void setError(WORD error, const char *reason = NULL);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark DTLSTransport => (data)
      #pragma mark

      AutoPUID mID;
      mutable RecursiveLock mLock;
      DTLSTransportWeakPtr mThisWeak;
      DTLSTransportPtr mGracefulShutdownReference;

      IDTLSTransportDelegateSubscriptions mSubscriptions;
      IDTLSTransportSubscriptionPtr mDefaultSubscription;

      ConnectionStates mCurrentState;
      AutoBool mStartCalled;

      AutoWORD mLastError;
      String mLastErrorReason;

      TransportInfoPtr mLocal;
      TransportInfoPtr mRemote;

      UseICETransportPtr mICETransport;
      IICETransportSubscriptionPtr mICETransportSubscription;

      DTLSContextPtr mDTLSContext;

      AutoBool mContextToldToFreeze;
      AutoBool mContextToldToShutdown;

      AutoBool mContextIsConnected;

      PendingDTLSBufferListPtr mPendingBuffers;

      RTPRoutes mRTPRoutes;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportFactory
    #pragma mark

    interaction IDTLSTransportFactory
    {
      static IDTLSTransportFactory &singleton();

      virtual DTLSTransportPtr create(
                                      IDTLSTransportDelegatePtr delegate,
                                      IICETransportPtr iceTransport
                                      );
    };
  }
}
