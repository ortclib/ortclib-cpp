/*

 Copyright (c) 2016, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/adapter/types.h>
#include <ortc/IICETypes.h>
#include <ortc/IRTPTypes.h>
#include <ortc/IDTLSTransport.h>
#include <ortc/IMediaStreamTrack.h>
#include <ortc/ISRTPSDESTransport.h>
#include <ortc/ISCTPTransport.h>

#include <map>
#include <set>

namespace ortc
{
  namespace adapter
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISessionDescriptionTypes
    //

    interaction ISessionDescriptionTypes
    {
      ZS_DECLARE_STRUCT_PTR(Description);
      ZS_DECLARE_STRUCT_PTR(ConnectionData);
      ZS_DECLARE_STRUCT_PTR(Transport);
      ZS_DECLARE_STRUCT_PTR(MediaLine);
      ZS_DECLARE_STRUCT_PTR(RTPMediaLine);
      ZS_DECLARE_STRUCT_PTR(SCTPMediaLine);
      ZS_DECLARE_STRUCT_PTR(RTPSender);
      ZS_DECLARE_STRUCT_PTR(ICECandidate);
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::Parameters, ICETransportParameters);
      ZS_DECLARE_TYPEDEF_PTR(IDTLSTransport::Parameters, DTLSParameters);
      ZS_DECLARE_TYPEDEF_PTR(ISRTPSDESTransport::Parameters, SRTPSDESParameters);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Capabilities, RTPCapabilities);
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Parameters, RTPParameters);
      ZS_DECLARE_TYPEDEF_PTR(ISCTPTransport::Capabilities, SCTPCapabilities);
      ZS_DECLARE_TYPEDEF_PTR(std::list<IICETypes::CandidatePtr>, ICECandidateList);

      typedef String SignalingDescription;

      typedef String MediaStreamID;
      typedef std::set<MediaStreamID> MediaStreamSet;

      typedef String TransportID;
      typedef String MediaLineID;
      typedef String SenderID;

      ZS_DECLARE_TYPEDEF_PTR(std::list<TransportPtr>, TransportList);
      ZS_DECLARE_TYPEDEF_PTR(std::list<RTPMediaLinePtr>, RTPMediaList);
      ZS_DECLARE_TYPEDEF_PTR(std::list<SCTPMediaLinePtr>, SCTPMediaList);
      ZS_DECLARE_TYPEDEF_PTR(std::list<RTPSenderPtr>, RTPSenderList);

      enum SignalingTypes
      {
        SignalingType_First,

        SignalingType_JSON = SignalingType_First,
        SignalingType_SDPOffer,
        SignalingType_SDPPranswer,
        SignalingType_SDPAnswer,
        SignalingType_SDPRollback,

        SignalingType_Last = SignalingType_SDPRollback,
      };

      static const char *toString(SignalingTypes type) noexcept;
      static SignalingTypes toSignalingType(const char *type) noexcept(false);

      enum MediaTypes
      {
        MediaType_First,

        MediaType_Unknown = MediaType_First,

        MediaType_Audio,
        MediaType_Video,
        MediaType_Text,
        MediaType_Application,

        MediaType_Last = MediaType_Application,
      };

      static const char *toString(MediaTypes mediaType) noexcept;
      static MediaTypes toMediaType(const char *mediaType) noexcept;
      static Optional<IMediaStreamTrackTypes::Kinds> toMediaStreamTrackKind(MediaTypes mediaType) noexcept;

      enum MediaDirections
      {
        MediaDirection_First,

        MediaDirection_SendReceive = MediaDirection_First,
        MediaDirection_SendOnly,
        MediaDirection_ReceiveOnly,
        MediaDirection_Inactive,

        MediaDirection_Last = MediaDirection_Inactive,
      };

      static const char *toString(MediaDirections mediaDirection) noexcept;
      static MediaDirections toMediaDirection(const char *mediaDirection) noexcept(false);

      struct ConnectionData
      {
        ZS_DECLARE_STRUCT_PTR(Details);

        struct Details
        {
          Optional<WORD> mPort;
          String mNetType;
          String mAddrType;
          String mConnectionAddress;

          Details() noexcept {}
          Details(const Details &op2) noexcept { (*this) = op2; }
          Details(ElementPtr rootEl) noexcept;

          static DetailsPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return DetailsPtr(); return make_shared<Details>(rootEl); }
          ElementPtr createElement(const char *objectName = "details") const noexcept;

          ElementPtr toDebug() const noexcept;
          String hash() const noexcept;
        };

        DetailsPtr mRTP;
        DetailsPtr mRTCP;

        ConnectionData() noexcept {}
        ConnectionData(const ConnectionData &op2) noexcept;
        ConnectionData(ElementPtr) noexcept;

        static ConnectionDataPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return ConnectionDataPtr(); return make_shared<ConnectionData>(rootEl); }
        ElementPtr createElement(const char *objectName = "connectionData") const noexcept;

        ElementPtr toDebug() const noexcept;
        String hash() const noexcept;
      };

      struct Transport
      {
        ZS_DECLARE_STRUCT_PTR(Parameters);

        struct Parameters
        {          
          ICETransportParametersPtr mICEParameters;
          DTLSParametersPtr mDTLSParameters;
          SRTPSDESParametersPtr mSRTPSDESParameters;
          ICECandidateList mICECandidates;
          bool mEndOfCandidates {false};

          Parameters() noexcept {}
          Parameters(const Parameters &op2) noexcept;
          Parameters(ElementPtr rootEl) noexcept;

          Parameters &operator=(const Parameters &op2) noexcept = delete;

          static ParametersPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return ParametersPtr(); return make_shared<Parameters>(rootEl); }
          ElementPtr createElement(const char *objectName) const noexcept;

          ElementPtr toDebug() const noexcept;
          String hash() const noexcept;
        };

        TransportID mID;
        ParametersPtr mRTP;
        ParametersPtr mRTCP;
        bool mUseMux {true};

        Transport() noexcept {}
        Transport(const Transport &op2) noexcept;
        Transport(ElementPtr rootEl) noexcept;

        Transport &operator=(const Transport &op2) noexcept = delete;

        static TransportPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return TransportPtr(); return make_shared<Transport>(rootEl); }
        ElementPtr createElement(const char *objectName = "transport") const noexcept;

        ElementPtr toDebug() const noexcept;
        String hash() const noexcept;
      };

      struct MediaLine : public Any
      {
        ZS_DECLARE_STRUCT_PTR(Details);

        struct Details
        {
          Optional<size_t> mInternalIndex;
          String mPrivateTransportID;

          String mProtocol;

          ConnectionDataPtr mConnectionData;
          MediaDirections mMediaDirection {MediaDirection_SendReceive};

          Details() noexcept {}
          Details(const Details &op2) noexcept;
          Details(ElementPtr rootEl) noexcept;

          Details &operator=(const Details &op2) noexcept = delete;

          static DetailsPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return DetailsPtr(); return make_shared<Details>(rootEl); }
          ElementPtr createElement(const char *objectName = "details") const noexcept;

          ElementPtr toDebug() const noexcept;
          String hash() const noexcept;
        };

        MediaLineID mID;
        TransportID mTransportID;

        String mMediaType;
        DetailsPtr mDetails;

      protected:
        MediaLine() noexcept {}
        MediaLine(const MediaLine &op2) noexcept;
        MediaLine(ElementPtr rootEl) noexcept;

        MediaLine &operator=(const MediaLine &op2) noexcept = delete;

        ElementPtr createElement(const char *objectName) const noexcept;
        ElementPtr toDebug() const noexcept;
        String hash() const noexcept;
      };

      struct RTPMediaLine : public MediaLine
      {
        RTPCapabilitiesPtr mSenderCapabilities;
        RTPCapabilitiesPtr mReceiverCapabilities;

        RTPMediaLine() noexcept {}
        RTPMediaLine(const RTPMediaLine &op2) noexcept;
        RTPMediaLine(ElementPtr rootEl) noexcept;

        RTPMediaLine &operator=(const RTPMediaLine &op2) noexcept = delete;

        static RTPMediaLinePtr create(ElementPtr rootEl) noexcept { if (!rootEl) return RTPMediaLinePtr(); return make_shared<RTPMediaLine>(rootEl); }
        ElementPtr createElement(const char *objectName = "rtpMediaLine") const noexcept;

        ElementPtr toDebug() const noexcept;
        String hash() const noexcept;
      };

      struct SCTPMediaLine : public MediaLine
      {
        SCTPCapabilitiesPtr mCapabilities;
        Optional<WORD> mPort;

        SCTPMediaLine() noexcept {}
        SCTPMediaLine(const SCTPMediaLine &op2) noexcept;
        SCTPMediaLine(ElementPtr rootEl) noexcept;

        SCTPMediaLine &operator=(const SCTPMediaLine &op2) noexcept = delete;

        static SCTPMediaLinePtr create(ElementPtr rootEl) noexcept { if (!rootEl) return SCTPMediaLinePtr(); return make_shared<SCTPMediaLine>(rootEl); }
        ElementPtr createElement(const char *objectName = "sctpMediaLine") const noexcept;

        ElementPtr toDebug() const noexcept;
        String hash() const noexcept;
      };

      struct RTPSender
      {
        ZS_DECLARE_STRUCT_PTR(Details);

        struct Details
        {
          Optional<size_t> mInternalRTPMediaLineIndex;

          Details() noexcept {}
          Details(const Details &op2) noexcept;
          Details(ElementPtr rootEl) noexcept;

          Details &operator=(const Details &op2) noexcept = delete;

          static DetailsPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return DetailsPtr(); return make_shared<Details>(rootEl); }
          ElementPtr createElement(const char *objectName = "details") const noexcept;

          ElementPtr toDebug() const noexcept;
          String hash() const noexcept;
        };

        SenderID mID;
        DetailsPtr mDetails;
        MediaLineID mRTPMediaLineID;
        RTPParametersPtr mParameters;
        String mMediaStreamTrackID;
        MediaStreamSet mMediaStreamIDs;

        RTPSender() noexcept {}
        RTPSender(const RTPSender &op2) noexcept;
        RTPSender(ElementPtr rootEL) noexcept;

        RTPSender &operator=(const RTPSender &op2) noexcept = delete;

        static RTPSenderPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return RTPSenderPtr(); return make_shared<RTPSender>(rootEl); }
        ElementPtr createElement(const char *objectName = "rtpSender") const noexcept;

        ElementPtr toDebug() const noexcept;
        String hash() const noexcept;
      };

      struct ICECandidate
      {
        String mMid;
        Optional<size_t> mMLineIndex;
        IICETypes::GatherCandidatePtr mCandidate;

        ICECandidate() noexcept {}
        ICECandidate(const ICECandidate &op2) noexcept;

        ICECandidate &operator=(const ICECandidate &op2) noexcept = delete;

        static ICECandidatePtr create(ElementPtr rootEl) noexcept;
        ElementPtr createElement(const char *objectName = "iceCandidate") const noexcept;

        ElementPtr toDebug() const noexcept;
        String hash() const noexcept;

        static ICECandidatePtr createFromJSON(ElementPtr rootEl) noexcept { return create(rootEl); }
        static ICECandidatePtr createFromSDP(const char *value) noexcept;
        String toSDP() const noexcept;
        ElementPtr toJSON() const noexcept;
      };

      struct Description
      {
        ZS_DECLARE_STRUCT_PTR(Details);

        struct Details
        {
          String mUsername {"-"};
          ULONGLONG mSessionID {};
          ULONGLONG mSessionVersion {};
          ConnectionData::DetailsPtr mUnicaseAddress;
          String mSessionName;
          QWORD mStartTime {};
          QWORD mEndTime {};
          ConnectionDataPtr mConnectionData;  // optional; can be null;

          Details() noexcept {}
          Details(const Details &op2) noexcept;
          Details(ElementPtr rootEl) noexcept;

          Details &operator=(const Details &op2) noexcept = delete;

          static DetailsPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return DetailsPtr(); return make_shared<Details>(rootEl); }
          ElementPtr createElement(const char *objectName = "details") const noexcept;

          ElementPtr toDebug() const noexcept;
          String hash() const noexcept;
        };

        DetailsPtr mDetails;                // optional; can be null;

        TransportList mTransports;
        RTPMediaList mRTPMediaLines;
        SCTPMediaList mSCTPMediaLines;
        RTPSenderList mRTPSenders;

        Description() noexcept {}
        Description(const Description &op2) noexcept;
        Description(ElementPtr rootEl) noexcept;

        Description &operator=(const Description &op2) noexcept = delete;

        static DescriptionPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return DescriptionPtr(); return make_shared<Description>(rootEl); }
        ElementPtr createElement(const char *objectName = "session") const noexcept;

        ElementPtr toDebug() const noexcept;
        String hash() const noexcept;
      };
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // ISessionDescription
    //

    interface ISessionDescription : public ISessionDescriptionTypes,
                                    public Any
    {
      static ISessionDescriptionPtr create(
                                           SignalingTypes type,
                                           const char *description
                                           ) noexcept;
      static ISessionDescriptionPtr create(
                                           SignalingTypes type,
                                           const Description &description
                                           ) noexcept;

      static ElementPtr toDebug(ISessionDescriptionPtr object) noexcept;

      virtual PUID getID() const noexcept = 0;

      virtual SignalingTypes type() const noexcept = 0;
      virtual DescriptionPtr description() const noexcept = 0;
      virtual SignalingDescription formattedDescription() const noexcept = 0;
    };
  }
}
