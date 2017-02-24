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
    #pragma mark
    #pragma mark ISessionDescriptionTypes
    #pragma mark

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

      static const char *toString(SignalingTypes type);
      static SignalingTypes toSignalingType(const char *type);

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

      static const char *toString(MediaTypes mediaType);
      static MediaTypes toMediaType(const char *mediaType);
      static Optional<IMediaStreamTrackTypes::Kinds> toMediaStreamTrackKind(MediaTypes mediaType);

      enum MediaDirections
      {
        MediaDirection_First,

        MediaDirection_SendReceive = MediaDirection_First,
        MediaDirection_SendOnly,
        MediaDirection_ReceiveOnly,
        MediaDirection_Inactive,

        MediaDirection_Last = MediaDirection_Inactive,
      };

      static const char *toString(MediaDirections mediaDirection);
      static MediaDirections toMediaDirection(const char *mediaDirection);

      struct ConnectionData
      {
        ZS_DECLARE_STRUCT_PTR(Details);

        struct Details
        {
          Optional<WORD> mPort;
          String mNetType;
          String mAddrType;
          String mConnectionAddress;

          Details() {}
          Details(const Details &op2) { (*this) = op2; }
          Details(ElementPtr rootEl);

          static DetailsPtr create(ElementPtr rootEl) { if (!rootEl) return DetailsPtr(); return make_shared<Details>(rootEl); }
          ElementPtr createElement(const char *objectName = "details") const;

          ElementPtr toDebug() const;
          String hash() const;
        };

        DetailsPtr mRTP;
        DetailsPtr mRTCP;

        ConnectionData() {}
        ConnectionData(const ConnectionData &op2);
        ConnectionData(ElementPtr);

        static ConnectionDataPtr create(ElementPtr rootEl) { if (!rootEl) return ConnectionDataPtr(); return make_shared<ConnectionData>(rootEl); }
        ElementPtr createElement(const char *objectName = "connectionData") const;

        ElementPtr toDebug() const;
        String hash() const;
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

          Parameters() {}
          Parameters(const Parameters &op2);
          Parameters(ElementPtr rootEl);

          Parameters &operator=(const Parameters &op2) = delete;

          static ParametersPtr create(ElementPtr rootEl) { if (!rootEl) return ParametersPtr(); return make_shared<Parameters>(rootEl); }
          ElementPtr createElement(const char *objectName) const;

          ElementPtr toDebug() const;
          String hash() const;
        };

        TransportID mID;
        ParametersPtr mRTP;
        ParametersPtr mRTCP;
        bool mUseMux {true};

        Transport() {}
        Transport(const Transport &op2);
        Transport(ElementPtr rootEl);

        Transport &operator=(const Transport &op2) = delete;

        static TransportPtr create(ElementPtr rootEl) { if (!rootEl) return TransportPtr(); return make_shared<Transport>(rootEl); }
        ElementPtr createElement(const char *objectName = "transport") const;

        ElementPtr toDebug() const;
        String hash() const;
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

          Details() {}
          Details(const Details &op2);
          Details(ElementPtr rootEl);

          Details &operator=(const Details &op2) = delete;

          static DetailsPtr create(ElementPtr rootEl) { if (!rootEl) return DetailsPtr(); return make_shared<Details>(rootEl); }
          ElementPtr createElement(const char *objectName = "details") const;

          ElementPtr toDebug() const;
          String hash() const;
        };

        MediaLineID mID;
        TransportID mTransportID;

        String mMediaType;
        DetailsPtr mDetails;

      protected:
        MediaLine() {}
        MediaLine(const MediaLine &op2);
        MediaLine(ElementPtr rootEl);

        MediaLine &operator=(const MediaLine &op2) = delete;

        ElementPtr createElement(const char *objectName) const;
        ElementPtr toDebug() const;
        String hash() const;
      };

      struct RTPMediaLine : public MediaLine
      {
        RTPCapabilitiesPtr mSenderCapabilities;
        RTPCapabilitiesPtr mReceiverCapabilities;

        RTPMediaLine() {}
        RTPMediaLine(const RTPMediaLine &op2);
        RTPMediaLine(ElementPtr rootEl);

        RTPMediaLine &operator=(const RTPMediaLine &op2) = delete;

        static RTPMediaLinePtr create(ElementPtr rootEl) { if (!rootEl) return RTPMediaLinePtr(); return make_shared<RTPMediaLine>(rootEl); }
        ElementPtr createElement(const char *objectName = "rtpMediaLine") const;

        ElementPtr toDebug() const;
        String hash() const;
      };

      struct SCTPMediaLine : public MediaLine
      {
        SCTPCapabilitiesPtr mCapabilities;
        Optional<WORD> mPort;

        SCTPMediaLine() {}
        SCTPMediaLine(const SCTPMediaLine &op2);
        SCTPMediaLine(ElementPtr rootEl);

        SCTPMediaLine &operator=(const SCTPMediaLine &op2) = delete;

        static SCTPMediaLinePtr create(ElementPtr rootEl) { if (!rootEl) return SCTPMediaLinePtr(); return make_shared<SCTPMediaLine>(rootEl); }
        ElementPtr createElement(const char *objectName = "sctpMediaLine") const;

        ElementPtr toDebug() const;
        String hash() const;
      };

      struct RTPSender
      {
        ZS_DECLARE_STRUCT_PTR(Details);

        struct Details
        {
          Optional<size_t> mInternalRTPMediaLineIndex;

          Details() {}
          Details(const Details &op2);
          Details(ElementPtr rootEl);

          Details &operator=(const Details &op2) = delete;

          static DetailsPtr create(ElementPtr rootEl) { if (!rootEl) return DetailsPtr(); return make_shared<Details>(rootEl); }
          ElementPtr createElement(const char *objectName = "details") const;

          ElementPtr toDebug() const;
          String hash() const;
        };

        SenderID mID;
        DetailsPtr mDetails;
        MediaLineID mRTPMediaLineID;
        RTPParametersPtr mParameters;
        String mMediaStreamTrackID;
        MediaStreamSet mMediaStreamIDs;

        RTPSender() {}
        RTPSender(const RTPSender &op2);
        RTPSender(ElementPtr rootEL);

        RTPSender &operator=(const RTPSender &op2) = delete;

        static RTPSenderPtr create(ElementPtr rootEl) { if (!rootEl) return RTPSenderPtr(); return make_shared<RTPSender>(rootEl); }
        ElementPtr createElement(const char *objectName = "rtpSender") const;

        ElementPtr toDebug() const;
        String hash() const;
      };

      struct ICECandidate
      {
        String mMid;
        Optional<size_t> mMLineIndex;
        IICETypes::GatherCandidatePtr mCandidate;

        ICECandidate() {}
        ICECandidate(const ICECandidate &op2);

        ICECandidate &operator=(const ICECandidate &op2) = delete;

        static ICECandidatePtr create(ElementPtr rootEl);
        ElementPtr createElement(const char *objectName = "iceCandidate") const;

        ElementPtr toDebug() const;
        String hash() const;

        static ICECandidatePtr createFromJSON(ElementPtr rootEl) { return create(rootEl); }
        static ICECandidatePtr createFromSDP(const char *value);
        String toSDP() const;
        ElementPtr toJSON() const;
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

          Details() {}
          Details(const Details &op2);
          Details(ElementPtr rootEl);

          Details &operator=(const Details &op2) = delete;

          static DetailsPtr create(ElementPtr rootEl) { if (!rootEl) return DetailsPtr(); return make_shared<Details>(rootEl); }
          ElementPtr createElement(const char *objectName = "details") const;

          ElementPtr toDebug() const;
          String hash() const;
        };

        DetailsPtr mDetails;                // optional; can be null;

        TransportList mTransports;
        RTPMediaList mRTPMediaLines;
        SCTPMediaList mSCTPMediaLines;
        RTPSenderList mRTPSenders;

        Description() {}
        Description(const Description &op2);
        Description(ElementPtr rootEl);

        Description &operator=(const Description &op2) = delete;

        static DescriptionPtr create(ElementPtr rootEl) { if (!rootEl) return DescriptionPtr(); return make_shared<Description>(rootEl); }
        ElementPtr createElement(const char *objectName = "session") const;

        ElementPtr toDebug() const;
        String hash() const;
      };
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescription
    #pragma mark

    interface ISessionDescription : public ISessionDescriptionTypes,
                                    public Any
    {
      static ISessionDescriptionPtr create(
                                           SignalingTypes type,
                                           const char *description
                                           );
      static ISessionDescriptionPtr create(
                                           SignalingTypes type,
                                           const Description &description
                                           );

      static ElementPtr toDebug(ISessionDescriptionPtr object);

      virtual PUID getID() const = 0;

      virtual SignalingTypes type() const = 0;
      virtual DescriptionPtr description() const = 0;
      virtual SignalingDescription formattedDescription() const = 0;
    };
  }
}
