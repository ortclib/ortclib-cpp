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
    #pragma mark
    #pragma mark ISecureTransportForRTPSender
    #pragma mark

    interaction ISecureTransportForRTPSender
    {
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForRTPSender, ForRTPSender)

      static ElementPtr toDebug(ForRTPSenderPtr transport);

      virtual PUID getID() const = 0;

      virtual bool sendPacket(
                              IICETypes::Components sendOverICETransport,
                              IICETypes::Components packetType,
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISecureTransportForSRTP
    #pragma mark

    interaction ISecureTransportForSRTP
    {
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForSRTP, ForSRTP)

      static ElementPtr toDebug(ForSRTPPtr transport);

      virtual PUID getID() const = 0;

      virtual bool sendEncryptedPacket(
                                       IICETypes::Components sendOverICETransport,
                                       IICETypes::Components packetType,
                                       const BYTE *buffer,
                                       size_t bufferLengthInBytes
                                       ) = 0;

      virtual bool handleReceivedDecryptedPacket(
                                                 IICETypes::Components viaTransport,
                                                 IICETypes::Components packetType,
                                                 const BYTE *buffer,
                                                 size_t bufferLengthInBytes
                                                 ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISecureTransportForICETransport
    #pragma mark

    interaction ISecureTransportForICETransport
    {
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForICETransport, ForICETransport)

      static ElementPtr toDebug(ForICETransportPtr transport);

      virtual PUID getID() const = 0;

      virtual void handleReceivedPacket(
                                        IICETypes::Components viaComponent,
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        ) = 0;

      virtual void handleReceivedSTUNPacket(
                                            IICETypes::Components viaComponent,
                                            STUNPacketPtr packet
                                            ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISecureTransportForRTPListener
    #pragma mark

    interaction ISecureTransportForRTPListener
    {
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForRTPListener, ForRTPListener)

      static ElementPtr toDebug(ForRTPListenerPtr transport);

      static ForRTPListenerPtr convert(IRTPTransportPtr transport);

      virtual PUID getID() const = 0;

      virtual RTPListenerPtr getListener() const = 0;
    };

  }
}
