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

#include <ortc/internal/ortc_ISecureTransport.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_SRTPSDESTransport.h>
#include <ortc/internal/ortc_SCTPTransportListener.h>
#include <ortc/internal/ortc_SCTPTransport.h>
#include <ortc/internal/platform.h>

//#include <ortc/services/IHelper.h>
//#include <ortc/services/IHTTP.h>
//
//#include <zsLib/Stringize.h>
//#include <zsLib/Log.h>
#include <zsLib/XML.h>
//
//#include <cryptopp/sha.h>


#ifdef __GNUC__
#error MOVE THIS TO PROJECT SETTING RATHER THAN PUTTING ON INDIVIDUAL FILES
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-local-typedef"
#endif //__GNUC__

namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc) }

namespace ortc
{
//  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHelper, UseServicesHelper)
//  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP)
//
//  typedef ortc::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // (helpers)
    //

    //-------------------------------------------------------------------------
    static Log::Params secure_slog(const char *message) noexcept
    {
      ElementPtr objectEl = Element::create("ortc::ISecureTransport");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportTypes
    //
    
    //-------------------------------------------------------------------------
    const char *ISecureTransportTypes::toString(States state) noexcept
    {
      switch (state) {
        case State_Pending:       return "Pending";
        case State_Connected:     return "Connected";
        case State_Disconnected:  return "Disconnected";
        case State_Closed:        return "Closed";
      }
      return "unknown";
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransport
    //
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportForRTPSender
    //
    
    //-------------------------------------------------------------------------
    ElementPtr ISecureTransportForRTPSender::toDebug(ForRTPSenderPtr transport) noexcept
    {
      if (!transport) return ElementPtr();

      {
        auto pThis = DTLSTransport::convert(transport);
        if (pThis) return DTLSTransport::toDebug(pThis);
      }

      {
        auto pThis = SRTPSDESTransport::convert(transport);
        if (pThis) return SRTPSDESTransport::toDebug(pThis);
      }
      return ElementPtr();
    }
    
    //-------------------------------------------------------------------------
    void ISecureTransportForRTPSender::getSendingTransport(
                                                           IRTPTransportPtr inRTPTransport,
                                                           IRTCPTransportPtr inRTCPTransport,
                                                           IICETypes::Components &outWhenSendingRTPUseSendOverComponent,
                                                           IICETypes::Components &outWhenSendingRTCPUseSendOverComponent,
                                                           ForRTPSenderPtr &outRTPSecureTransport,
                                                           ForRTPSenderPtr &outRTCPSecureTransport
                                                           ) noexcept(false)
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForSecureTransport, UseICETransport);

      outWhenSendingRTPUseSendOverComponent = IICETypes::Component_RTP;
      outWhenSendingRTCPUseSendOverComponent = IICETypes::Component_RTP;

      outRTPSecureTransport = ForRTPSenderPtr();
      outRTCPSecureTransport = ForRTPSenderPtr();

      if (inRTCPTransport) {
        if (inRTPTransport->getID() == inRTCPTransport->getID()) {
          ZS_LOG_WARNING(Trace, secure_slog("rtcp transport is rtp transport (thus ignoring rtcp parameter)"))
          inRTCPTransport = IRTCPTransportPtr();
        }
      }

      {
        auto result = IDTLSTransport::convert(inRTPTransport);
        if (result) {
          auto dtlsTransport = DTLSTransport::convert(result);

          if (dtlsTransport) {
            outRTPSecureTransport = dtlsTransport;
            outRTCPSecureTransport = dtlsTransport;

            if (inRTCPTransport) {
              UseICETransportPtr rtpICETransport = ICETransport::convert(ForRTPSenderPtr(dtlsTransport)->getICETransport());
              auto rtcpDTLSTransport = DTLSTransport::convert(IDTLSTransport::convert(inRTCPTransport));
              ORTC_THROW_INVALID_PARAMETERS_IF(!rtcpDTLSTransport)

              auto rtcpICETransport = UseICETransportPtr(ICETransport::convert(ForRTPSenderPtr(rtcpDTLSTransport)->getICETransport()));
              ORTC_THROW_INVALID_PARAMETERS_IF(!rtcpICETransport)

              auto relatedRTPICETransport = UseICETransportPtr(rtcpICETransport->getRTPTransport());
              ORTC_THROW_INVALID_STATE_IF(!relatedRTPICETransport)
              ORTC_THROW_INVALID_PARAMETERS_IF(relatedRTPICETransport->getID() != rtpICETransport->getID()) // ICE RTP related to ICE RTCP must be the same ICE RTP transport related to RTP transport

              outWhenSendingRTCPUseSendOverComponent = IICETypes::Component_RTCP;
              outRTCPSecureTransport = rtcpDTLSTransport;
            }
            return;
          }
        }
      }

      {
        auto result = ISRTPSDESTransport::convert(inRTPTransport);
        if (result) {
          auto sdesTransport = SRTPSDESTransport::convert(result);
          ORTC_THROW_INVALID_PARAMETERS_IF(!sdesTransport)      // not a dtls transport and not an SDES transport, what is this transport type?

          outRTPSecureTransport = sdesTransport;
          outRTCPSecureTransport = sdesTransport;

          if (inRTCPTransport) {
            UseICETransportPtr rtcpICETransport = ICETransport::convert(IICETransport::convert(inRTCPTransport));
            ORTC_THROW_INVALID_PARAMETERS_IF(!rtcpICETransport) // must be an ICE transport

            UseICETransportPtr relatedRTPTransport = rtcpICETransport->getRTPTransport();
            ORTC_THROW_INVALID_STATE_IF(!relatedRTPTransport) // must have a related ICE transport

            UseICETransportPtr rtpTransport = ICETransport::convert(ForRTPSenderPtr(sdesTransport)->getICETransport());
            ORTC_THROW_INVALID_STATE_IF(!rtpTransport) // must have a ICE transport

            ORTC_THROW_INVALID_PARAMETERS_IF(relatedRTPTransport->getID() != rtpTransport->getID()) // ICE RTP related to ICE RTCP must be the same ICE RTP transport related to RTP transport

            outWhenSendingRTCPUseSendOverComponent = IICETypes::Component_RTCP;
          }

        }
      }
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportForRTPReceiver
    //

    //-------------------------------------------------------------------------
    ElementPtr ISecureTransportForRTPReceiver::toDebug(ForRTPReceiverPtr transport) noexcept
    {
      if (!transport) return ElementPtr();

      {
        auto pThis = DTLSTransport::convert(transport);
        if (pThis) return DTLSTransport::toDebug(pThis);
      }

      {
        auto pThis = SRTPSDESTransport::convert(transport);
        if (pThis) return SRTPSDESTransport::toDebug(pThis);
      }
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    void ISecureTransportForRTPReceiver::getReceivingTransport(
                                                               IRTPTransportPtr inRTPTransport,
                                                               IRTCPTransportPtr inRTCPTransport,
                                                               IICETypes::Components &outWhenReceivingRTPUseReceiveOverComponent,
                                                               IICETypes::Components &outWhenReceivingRTCPUseReceiveOverComponent,
                                                               ForRTPReceiverPtr &outRTPSecureTransport,
                                                               ForRTPReceiverPtr &outRTCPSecureTransport
                                                               ) noexcept(false)
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForSecureTransport, UseICETransport);

      outWhenReceivingRTPUseReceiveOverComponent = IICETypes::Component_RTP;
      outWhenReceivingRTCPUseReceiveOverComponent = IICETypes::Component_RTP;

      outRTPSecureTransport = ForRTPReceiverPtr();
      outRTCPSecureTransport = ForRTPReceiverPtr();

      if (inRTCPTransport) {
        if (inRTPTransport->getID() == inRTCPTransport->getID()) {
          ZS_LOG_WARNING(Trace, secure_slog("rtcp transport is rtp transport (thus ignoring rtcp parameter)"))
            inRTCPTransport = IRTCPTransportPtr();
        }
      }

      {
        auto result = IDTLSTransport::convert(inRTPTransport);
        if (result) {
          auto dtlsTransport = DTLSTransport::convert(result);

          if (dtlsTransport) {
            outRTPSecureTransport = dtlsTransport;
            outRTCPSecureTransport = dtlsTransport;

            if (inRTCPTransport) {
              UseICETransportPtr rtpICETransport = ICETransport::convert(ForRTPReceiverPtr(dtlsTransport)->getICETransport());
              auto rtcpDTLSTransport = DTLSTransport::convert(IDTLSTransport::convert(inRTCPTransport));
              ORTC_THROW_INVALID_PARAMETERS_IF(!rtcpDTLSTransport)

              auto rtcpICETransport = UseICETransportPtr(ICETransport::convert(ForRTPReceiverPtr(rtcpDTLSTransport)->getICETransport()));
              ORTC_THROW_INVALID_PARAMETERS_IF(!rtcpICETransport)

              auto relatedRTPICETransport = UseICETransportPtr(rtcpICETransport->getRTPTransport());
              ORTC_THROW_INVALID_STATE_IF(!relatedRTPICETransport)
              ORTC_THROW_INVALID_PARAMETERS_IF(relatedRTPICETransport->getID() != rtpICETransport->getID()) // ICE RTP related to ICE RTCP must be the same ICE RTP transport related to RTP transport

              outWhenReceivingRTCPUseReceiveOverComponent = IICETypes::Component_RTCP;
              outRTCPSecureTransport = rtcpDTLSTransport;
            }
            return;
          }
        }
      }

      {
        auto result = ISRTPSDESTransport::convert(inRTPTransport);
        if (result) {
          auto sdesTransport = SRTPSDESTransport::convert(result);
          ORTC_THROW_INVALID_PARAMETERS_IF(!sdesTransport)      // not a dtls transport and not an SDES transport, what is this transport type?

          outRTPSecureTransport = sdesTransport;
          outRTCPSecureTransport = sdesTransport;

          if (inRTCPTransport) {
            UseICETransportPtr rtcpICETransport = ICETransport::convert(IICETransport::convert(inRTCPTransport));
            ORTC_THROW_INVALID_PARAMETERS_IF(!rtcpICETransport) // must be an ICE transport

            UseICETransportPtr relatedRTPTransport = rtcpICETransport->getRTPTransport();
            ORTC_THROW_INVALID_STATE_IF(!relatedRTPTransport) // must have a related ICE transport

            UseICETransportPtr rtpTransport = ICETransport::convert(ForRTPReceiverPtr(sdesTransport)->getICETransport());
            ORTC_THROW_INVALID_STATE_IF(!rtpTransport) // must have a ICE transport

            ORTC_THROW_INVALID_PARAMETERS_IF(relatedRTPTransport->getID() != rtpTransport->getID()) // ICE RTP related to ICE RTCP must be the same ICE RTP transport related to RTP transport

            outWhenReceivingRTCPUseReceiveOverComponent = IICETypes::Component_RTCP;
          }

        }
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportForSRTPTransport
    //
    
    //-------------------------------------------------------------------------
    ElementPtr ISecureTransportForSRTPTransport::toDebug(ForSRTPPtr transport) noexcept
    {
      if (!transport) return ElementPtr();

      {
        auto pThis = DTLSTransport::convert(transport);
        if (pThis) return DTLSTransport::toDebug(pThis);
      }

      {
        auto pThis = SRTPSDESTransport::convert(transport);
        if (pThis) return SRTPSDESTransport::toDebug(pThis);
      }
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportForICETransport
    //
    
    //-------------------------------------------------------------------------
    ElementPtr ISecureTransportForICETransport::toDebug(ForICETransportPtr transport) noexcept
    {
      if (!transport) return ElementPtr();

      {
        auto pThis = DTLSTransport::convert(transport);
        if (pThis) return DTLSTransport::toDebug(pThis);
      }

      {
        auto pThis = SRTPSDESTransport::convert(transport);
        if (pThis) return SRTPSDESTransport::toDebug(pThis);
      }
      return ElementPtr();
    }
 
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportForRTPListener
    //

    //-------------------------------------------------------------------------
    ElementPtr ISecureTransportForRTPListener::toDebug(ForRTPListenerPtr transport) noexcept
    {
      if (!transport) return ElementPtr();

      {
        auto pThis = DTLSTransport::convert(transport);
        if (pThis) return DTLSTransport::toDebug(pThis);
      }

      {
        auto pThis = SRTPSDESTransport::convert(transport);
        if (pThis) return SRTPSDESTransport::toDebug(pThis);
      }
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    ISecureTransportForRTPListener::ForRTPListenerPtr ISecureTransportForRTPListener::convert(IRTPTransportPtr transport) noexcept
    {
      if (!transport) return ForRTPListenerPtr();
      return ZS_DYNAMIC_PTR_CAST(ForRTPListener, transport);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISecureTransportForSCTPTransport
    //

    //-------------------------------------------------------------------------
    ElementPtr ISecureTransportForDataTransport::toDebug(ForDataTransportPtr transport) noexcept
    {
      if (!transport) return ElementPtr();

      {
        auto pThis = DTLSTransport::convert(transport);
        if (pThis) return DTLSTransport::toDebug(pThis);
      }

      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // IDataTransportForSecureTransport
    //

    //-------------------------------------------------------------------------
    ElementPtr IDataTransportForSecureTransport::toDebug(ForSecureTransportPtr transport) noexcept
    {
      if (!transport) return ElementPtr();

      {
        auto pThis = SCTPTransportListener::convert(transport);
        if (pThis) return SCTPTransportListener::toDebug(pThis);
      }

      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    IDataTransportForSecureTransport::ForSecureTransportPtr IDataTransportForSecureTransport::create(UseSecureTransportPtr transport) noexcept
    {
      if (!transport) return ForSecureTransportPtr();

      return internal::ISCTPTransportListenerFactory::singleton().create(transport);
    }
  }

}


#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif //__GNUC__
