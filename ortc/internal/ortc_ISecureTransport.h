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

#include <ortc/internal/types.h>

#include <ortc/IICETypes.h>

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportTypes
    //

    interaction ISecureTransportTypes
    {
      enum States
      {
        State_Pending,
        State_Connected,
        State_Disconnected,
        State_Closed,
      };

      static const char *toString(States state) noexcept;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransport
    //

    interaction ISecureTransport : public ISecureTransportTypes
    {
      virtual PUID getID() const noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportDelegate
    //

    interaction ISecureTransportDelegate
    {
      typedef ISecureTransportTypes::States States;

      virtual void onSecureTransportStateChanged(
                                                 ISecureTransportPtr transport,
                                                 States state
                                                 ) = 0;
    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //
    // ISecureTransportSubscription
    //

    interaction ISecureTransportSubscription
    {
      virtual PUID getID() const noexcept = 0;

      virtual void cancel() noexcept = 0;

      virtual void background() noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportForRTPSender
    //

    interaction ISecureTransportForRTPSender
    {
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForRTPSender, ForRTPSender);

      static ElementPtr toDebug(ForRTPSenderPtr transport) noexcept;

      static void getSendingTransport(
                                      IRTPTransportPtr inRTPTransport,
                                      IRTCPTransportPtr inRTCPTransport,
                                      IICETypes::Components &outWhenSendingRTPUseSendOverComponent,
                                      IICETypes::Components &outWhenSendingRTCPUseSendOverComponent,
                                      ForRTPSenderPtr &outRTPSecureTransport,
                                      ForRTPSenderPtr &outRTCPSecureTransport
                                      ) noexcept(false);

      virtual PUID getID() const noexcept = 0;

      virtual ISecureTransportSubscriptionPtr subscribe(ISecureTransportDelegatePtr delegate) noexcept = 0;

      virtual ISecureTransportTypes::States state(ISecureTransportTypes::States ignored = ISecureTransportTypes::States()) const noexcept = 0;

      virtual bool sendPacket(
                              IICETypes::Components sendOverICETransport,
                              IICETypes::Components packetType,
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              ) noexcept = 0;

      virtual IICETransportPtr getICETransport() const noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportForRTPReceiver
    //

    interaction ISecureTransportForRTPReceiver
    {
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForRTPReceiver, ForRTPReceiver);

      static ElementPtr toDebug(ForRTPReceiverPtr transport) noexcept;

      static void getReceivingTransport(
                                        IRTPTransportPtr inRTPTransport,
                                        IRTCPTransportPtr inRTCPTransport,
                                        IICETypes::Components &outWhenReceivingRTPUseReceiveOverComponent,
                                        IICETypes::Components &outWhenReceivingRTCPUseReceiveOverComponent,
                                        ForRTPReceiverPtr &outRTPSecureTransport,
                                        ForRTPReceiverPtr &outRTCPSecureTransport
                                        ) noexcept(false);

      virtual PUID getID() const noexcept = 0;

      virtual ISecureTransportSubscriptionPtr subscribe(ISecureTransportDelegatePtr delegate) noexcept = 0;

      virtual ISecureTransportTypes::States state(ISecureTransportTypes::States ignored = ISecureTransportTypes::States()) const noexcept = 0;

      virtual bool sendPacket(
                              IICETypes::Components sendOverICETransport,
                              IICETypes::Components packetType,
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              ) noexcept = 0;

      virtual IICETransportPtr getICETransport() const noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportForSRTPTransport
    //

    interaction ISecureTransportForSRTPTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForSRTPTransport, ForSRTP);

      static ElementPtr toDebug(ForSRTPPtr transport) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual bool sendEncryptedPacket(
                                       IICETypes::Components sendOverICETransport,
                                       IICETypes::Components packetType,
                                       const BYTE *buffer,
                                       size_t bufferLengthInBytes
                                       ) noexcept = 0;

      virtual bool handleReceivedDecryptedPacket(
                                                 IICETypes::Components viaTransport,
                                                 IICETypes::Components packetType,
                                                 const BYTE *buffer,
                                                 size_t bufferLengthInBytes
                                                 ) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportForICETransport
    //

    interaction ISecureTransportForICETransport
    {
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForICETransport, ForICETransport);

      static ElementPtr toDebug(ForICETransportPtr transport) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual void notifyAssociateTransportCreated(
                                                   IICETypes::Components associatedComponent,
                                                   ICETransportPtr assoicated
                                                   ) noexcept = 0;

      virtual bool handleReceivedPacket(
                                        IICETypes::Components viaComponent,
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        ) noexcept = 0;

      virtual void handleReceivedSTUNPacket(
                                            IICETypes::Components viaComponent,
                                            STUNPacketPtr packet
                                            ) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportForRTPListener
    //

    interaction ISecureTransportForRTPListener
    {
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForRTPListener, ForRTPListener)

      static ElementPtr toDebug(ForRTPListenerPtr transport) noexcept;

      static ForRTPListenerPtr convert(IRTPTransportPtr transport) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual RTPListenerPtr getListener() const noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportForDataTransport
    //

    interaction ISecureTransportForDataTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, ForDataTransport);

      ZS_DECLARE_TYPEDEF_PTR(IDataTransportForSecureTransport, UseDataTransport);

      static ElementPtr toDebug(ForDataTransportPtr transport) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual ISecureTransportSubscriptionPtr subscribe(ISecureTransportDelegatePtr delegate) noexcept = 0;

      virtual ISecureTransportTypes::States state(ISecureTransportTypes::States ignored = ISecureTransportTypes::States()) const noexcept = 0;

      virtual bool isClientRole() const noexcept = 0;

      virtual UseDataTransportPtr getDataTransport() const noexcept = 0;

      virtual bool sendDataPacket(
                                  const BYTE *buffer,
                                  size_t bufferLengthInBytes
                                  ) noexcept = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IDataTransportForSecureTransport
    //

    interaction IDataTransportForSecureTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IDataTransportForSecureTransport, ForSecureTransport);

      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForDataTransport, UseSecureTransport);

      static ElementPtr toDebug(ForSecureTransportPtr transport) noexcept;

      static ForSecureTransportPtr create(UseSecureTransportPtr transport) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual bool handleDataPacket(
                                    const BYTE *buffer,
                                    size_t bufferLengthInBytes
                                    ) noexcept = 0;
    };

  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::ISecureTransportDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::ISecureTransportPtr, ISecureTransportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::ISecureTransportTypes::States, States)
ZS_DECLARE_PROXY_METHOD(onSecureTransportStateChanged, ISecureTransportPtr, States)
ZS_DECLARE_PROXY_END()

ZS_DECLARE_PROXY_SUBSCRIPTIONS_BEGIN(ortc::internal::ISecureTransportDelegate, ortc::internal::ISecureTransportSubscription)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::internal::ISecureTransportPtr, ISecureTransportPtr)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_TYPEDEF(ortc::internal::ISecureTransportTypes::States, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_METHOD(onSecureTransportStateChanged, ISecureTransportPtr, States)
ZS_DECLARE_PROXY_SUBSCRIPTIONS_END()
