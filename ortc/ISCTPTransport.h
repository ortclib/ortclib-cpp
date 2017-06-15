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

#include <ortc/types.h>
#include <ortc/IDataTransport.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransportTypes
  #pragma mark
  
  interaction ISCTPTransportTypes
  {
    ZS_DECLARE_STRUCT_PTR(Capabilities);
    ZS_DECLARE_STRUCT_PTR(SocketOptions);
    ZS_DECLARE_TYPEDEF_PTR(zsLib::PromiseWith<SocketOptions>, PromiseWithSocketOptions);

    enum States
    {
      State_First,

      State_New           = State_First,
      State_Connecting,
      State_Connected,
      State_Closed,

      State_Last          = State_Closed,
    };

    static const char *toString(States state);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISCTPTransportTypes::Capabilities
    #pragma mark

    struct Capabilities {
      size_t mMaxMessageSize {};
      WORD mMinPort {};
      WORD mMaxPort {};
      WORD mMaxUsablePorts {};
      WORD mMaxSessionsPerPort {};

      Capabilities() {}
      Capabilities(const Capabilities &op2) {(*this) = op2;}
      Capabilities(ElementPtr rootEl);

      static CapabilitiesPtr create(ElementPtr rootEl) { if (!rootEl) return CapabilitiesPtr(); return make_shared<Capabilities>(rootEl); }
      ElementPtr createElement(const char *objectName) const;

      ElementPtr toDebug() const;
      String hash() const;
    };

    // Example set:
    // ISCTPTransportTypes::SocketOptions options;
    // ISCTPTransportTypes::SocketOptions::RTO rto;
    // rto.initial = Milliseconds(1000);
    // options.mRTO = rto;
    // transport->setOptions(options);
    //
    // Example get:
    // ISCTPTransportTypes::SocketOptions options;
    // ISCTPTransportTypes::SocketOptions::RTO rto;
    // options.mRTO = rto; // set the structure of interest
    // transport->getOptions(options);
    // auto min = options.mRTO.value().min;
    struct SocketOptions : public Any
    {
      // see SCTP_RTOINFO / sctp_rtoinfo
      struct RTO
      {
        Milliseconds initial_ {3000};
        Milliseconds max_ {60000};
        Milliseconds min_ {1000};
      };

      // see SCTP_ASSOCINFO / sctp_assocparams
      struct AssocParams
      {
        uint32_t peer_rwnd_ {};
        uint32_t local_rwnd_ {};
        uint32_t cookie_life_ {};
        uint16_t asocmaxrxt_ {};
        uint16_t number_peer_destinations_ {};
      };

      // see SCTP_INITMSG / sctp_initmsg
      struct InitMsg {
        uint16_t num_ostreams_ {};
        uint16_t max_instreams_ {};
        uint16_t max_attempts_ {};
        Milliseconds max_init_timeo_ {};
      };

      // see SCTP_PEER_ADDR_PARAMS / sctp_peer_addr_params
      struct PAddrParams {
        Optional<bool> hb_demand_;    // send user heartbeat immediately
        Optional<bool> hb_enabled_;   // heartbeat enabled
        Optional<bool> hb_is_zero_;   // heartbeat delay is zero
        Milliseconds hbinterval_;     // heartbeat interval
        uint16_t pathmaxrxt_ {};      // max number of retransmissions before this address is considered unreachable 
      };

      // see SCTP_DELAYED_SACK / sctp_sack_info
      struct SackInfo {
        Milliseconds delay_ {};
        uint32_t freq_ {};
      };

      // see SCTP_STATUS / sctp_status
      struct Status {
        int32_t  state_ {};
        uint32_t rwnd_ {};
        uint16_t unackdata_ {};
        uint16_t penddata_ {};
        uint16_t instrms_ {};
        uint16_t outstrms_ {};
        uint32_t fragmentation_point_ {};
      };

      Optional<RTO> mRTO;
      Optional<AssocParams> mAssocParams;
      Optional<InitMsg> mInitMsg;
      Optional<Seconds> mAutoClose;  // see SCTP_AUTOCLOSE (0 seconds = no auto close)
      Optional<bool> mDisableFragments; // see SCTP_DISABLE_FRAGMENTS
      Optional<PAddrParams> mPAddrParams;
      Optional<uint32_t> mMaxSeg; // see SCTP_MAXSEG
      Optional<int> mFragmentInterleave; // see SCTP_FRAGMENT_INTERLEAVE
      Optional<uint32_t> mPartialDeliveryPoint; // see SCTP_PARTIAL_DELIVERY_POINT
      Optional<bool> mAutoASCONF; // see SCTP_AUTO_ASCONF
      Optional<uint32_t> mMaximumBurst; // see SCTP_MAX_BURST
      Optional<uint32_t> mContext; // see SCTP_CONTEXT
      Optional<SackInfo> mSackInfo;
      Optional<Status> mStatus;

      bool hasValue() const;
    };
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransport
  #pragma mark
  
  interaction ISCTPTransport : public ISCTPTransportTypes,
                               public IDataTransport
  {
    static ElementPtr toDebug(ISCTPTransportPtr transport);

    static ISCTPTransportPtr convert(IDataTransportPtr object);

    static ISCTPTransportPtr create(
                                    ISCTPTransportDelegatePtr delegate,
                                    IDTLSTransportPtr transport,
                                    WORD localPort = 0    // 0 = port automatically chosen
                                    ) throw (InvalidParameters, InvalidStateError);

    static ISCTPTransportListenerSubscriptionPtr listen(
                                                        ISCTPTransportListenerDelegatePtr delegate,
                                                        IDTLSTransportPtr transport,
                                                        const Capabilities &remoteCapabilities
                                                        );

    virtual PUID getID() const = 0;

    static CapabilitiesPtr getCapabilities();

    virtual IDTLSTransportPtr transport() const = 0;
    virtual States state() const = 0;

    virtual WORD port() const = 0;

    virtual WORD localPort() const = 0;
    virtual Optional<WORD> remotePort() const = 0;

    virtual PromiseWithSocketOptionsPtr getOptions(const SocketOptions &inWhichOptions) = 0;
    virtual PromisePtr setOptions(const SocketOptions &inOptions) = 0;

    virtual void start(
                       const Capabilities &remoteCapabilities,
                       WORD remotePort = 0
                       ) throw (InvalidStateError, InvalidParameters) = 0;
    virtual void stop() = 0;

    virtual ISCTPTransportSubscriptionPtr subscribe(ISCTPTransportDelegatePtr delegate) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransportDelegate
  #pragma mark

  interaction ISCTPTransportDelegate
  {
    typedef ISCTPTransportTypes::States States;

    virtual void onSCTPTransportStateChange(
                                            ISCTPTransportPtr transport,
                                            States state
                                            ) = 0;
    virtual void onSCTPTransportDataChannel(
                                            ISCTPTransportPtr transport,
                                            IDataChannelPtr channel
                                            ) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransportSubscription
  #pragma mark

  interaction ISCTPTransportSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransportListenerDelegate
  #pragma mark

  interaction ISCTPTransportListenerDelegate
  {
    virtual void onSCTPTransport(ISCTPTransportPtr transport) = 0;
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISCTPTransportListenerSubscription
  #pragma mark

  interaction ISCTPTransportListenerSubscription
  {
    virtual PUID getID() const = 0;

    virtual void cancel() = 0;

    virtual void background() = 0;
  };

}


ZS_DECLARE_PROXY_BEGIN(ortc::ISCTPTransportDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::ISCTPTransportPtr, ISCTPTransportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IDataChannelPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_METHOD_2(onSCTPTransportStateChange, ISCTPTransportPtr, States)
ZS_DECLARE_PROXY_METHOD_2(onSCTPTransportDataChannel, ISCTPTransportPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::ISCTPTransportDelegate, ortc::ISCTPTransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::ISCTPTransportPtr, ISCTPTransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::IDataChannelPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onSCTPTransportStateChange, ISCTPTransportPtr, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_2(onSCTPTransportDataChannel, ISCTPTransportPtr, IDataChannelPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()

ZS_DECLARE_PROXY_BEGIN(ortc::ISCTPTransportListenerDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::ISCTPTransportPtr, ISCTPTransportPtr)
ZS_DECLARE_PROXY_METHOD_1(onSCTPTransport, ISCTPTransportPtr)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::ISCTPTransportListenerDelegate, ortc::ISCTPTransportListenerSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::ISCTPTransportPtr, ISCTPTransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD_1(onSCTPTransport, ISCTPTransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
