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

#include <ortc/IDTLSTransport.h>
#include <ortc/IRTPSender.h>

#include <openpeer/services/IWakeDelegate.h>

#include <zsLib/MessageQueueAssociator.h>

namespace ortc
{
  namespace internal
  {
    interaction IDTLSTransportForRTPSender;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark RTPSender
    #pragma mark
    
    class RTPSender : public Noop,
                      public MessageQueueAssociator,
                      public IRTPSender,
                      public IWakeDelegate,
                      public IDTLSTransportDelegate
    {
    public:
      friend interaction IRTPSender;
      friend interaction IRTPSenderFactory;
      friend interaction IRTPSenderForICE;

      typedef IDTLSTransportForRTPSender UseDTLSTransport;
      typedef boost::shared_ptr<UseDTLSTransport> UseDTLSTransportPtr;
      typedef boost::weak_ptr<UseDTLSTransport> UseDTLSTransportWeakPtr;

    protected:
      RTPSender(
                IMessageQueuePtr queue,
                IDTLSTransportPtr rtpTransport,
                IDTLSTransportPtr rtcpTransport = IDTLSTransportPtr()
                );

      RTPSender(Noop) : Noop(true), MessageQueueAssociator(IMessageQueuePtr()) {}

      void init();

    public:
      virtual ~RTPSender();

      static RTPSenderPtr convert(IRTPSenderPtr object);
      
    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => IRTPSender
      #pragma mark

      static ElementPtr toDebug(IRTPSenderPtr transport);

      static RTPSenderPtr create(
                                 IDTLSTransportPtr rtpTransport,
                                 IDTLSTransportPtr rtcpTransport = IDTLSTransportPtr()
                                 );

      virtual PUID getID() const;

      static CapabilitiesPtr getCapabilities();

      virtual TrackDescriptionPtr createParams(CapabilitiesPtr capabilities = CapabilitiesPtr());

      static TrackDescriptionPtr filterParams(
                                              TrackDescriptionPtr params,
                                              CapabilitiesPtr capabilities
                                              );

      virtual TrackDescriptionPtr getDescription();
      virtual void setDescription(TrackDescriptionPtr);

      virtual void attach(
                          IDTLSTransportPtr rtpTransport,
                          IDTLSTransportPtr rtcpTransport = IDTLSTransportPtr()
                          );

      virtual void start(TrackDescriptionPtr localTrackDescription);
      virtual void stop();

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => IWakeDelegate
      #pragma mark

      virtual void onWake();

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => IDTLSTransportDelegate
      #pragma mark

      virtual void onDTLSTransportStateChanged(
                                               IDTLSTransportPtr transport,
                                               IDTLSTransport::ConnectionStates state
                                               );

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;

      virtual RecursiveLock &getLock() const {return mLock;}

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();

      void cancel();

      void setError(WORD error, const char *reason = NULL);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark RTPSender => (data)
      #pragma mark

      AutoPUID mID;
      mutable RecursiveLock mLock;
      RTPSenderWeakPtr mThisWeak;
      RTPSenderPtr mGracefulShutdownReference;
      AutoBool mShutdown;

      AutoBool mStartCalled;

      AutoWORD mLastError;
      String mLastErrorReason;

      TrackDescriptionPtr mDescription;

      UseDTLSTransportPtr mRTPTransport;
      UseDTLSTransportPtr mRTCPTransport;

      IDTLSTransportSubscriptionPtr mRTPTransportSubscription;
      IDTLSTransportSubscriptionPtr mRTCPTransportSubscription;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderFactory
    #pragma mark

    interaction IRTPSenderFactory
    {
      static IRTPSenderFactory &singleton();

      virtual RTPSenderPtr create(
                                  IDTLSTransportPtr rtpTransport,
                                  IDTLSTransportPtr rtcpTransport = IDTLSTransportPtr()
                                  );
    };
  }
}
