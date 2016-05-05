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
      ZS_DECLARE_TYPEDEF_PTR(std::list<ICECandidatePtr>, ICECandidateList);

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
        SignalingType_Auto,
        SignalingType_JSON,
        SignalingType_SDPOffer,
        SignalingType_SDPPreanswer,
        SignalingType_SDPAnswer,
        SignalingType_SDPRollback,
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
        MediaDirection_Inactive,
        MediaDirection_SendOnly,
        MediaDirection_ReceiveOnly,
        MediaDirection_SendReceive,
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
        };

        DetailsPtr mRTP;
        DetailsPtr mRTCP;
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
        };

        TransportID mID;
        ParametersPtr mRTP;
        ParametersPtr mRTCP;
      };

      struct MediaLine : public Any
      {
        ZS_DECLARE_STRUCT_PTR(Details);

        struct Details
        {
          Optional<size_t> mInternalIndex;
          String mProtocol;

          ConnectionDataPtr mConnectionData;      // optional; can be null;
        };

        MediaLineID mID;
        TransportID mTransportID;

        String mMediaType;
        DetailsPtr mDetails;
      };

      struct RTPMediaLine : public MediaLine
      {
        MediaDirections mMediaDirection {MediaDirection_SendReceive};
        RTPCapabilitiesPtr mCapabilities;
      };

      struct SCTPMediaLine : public MediaLine
      {
        SCTPCapabilitiesPtr mCapabilities;
        Optional<WORD> mPort;
      };

      struct RTPSender
      {
        ZS_DECLARE_STRUCT_PTR(Details);

        struct Details
        {
          Optional<size_t> mInternalRTPMediaLineIndex;
        };

        SenderID mID;
        DetailsPtr mDetails;
        MediaLineID mRTPMediaLineID;
        RTPParametersPtr mParameters;
        MediaStreamSet mMediaStreams;
      };

      struct ICECandidate
      {
        String mMid;
        Optional<size_t> mMLineIndex;
        IICETypes::Components mComponent {IICETypes::Component_RTP};
        IICETypes::GatherCandidatePtr mCandidate;

        static ICECandidatePtr create(ElementPtr candidateEl);
        static ICECandidatePtr createFromSDP(const char *string);
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
        };

        DetailsPtr mDetails;                // optional; can be null;

        TransportList mTransports;
        RTPMediaList mRTPMediaLines;
        SCTPMediaList mSCTPMediaLines;
        RTPSenderList mRTPSenders;
      };
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescription
    #pragma mark

    interface ISessionDescription : public ISessionDescriptionTypes
    {
      static ISessionDescriptionPtr create(
                                           SignalingTypes type,
                                           const char *description
                                           );
      static ISessionDescriptionPtr create(
                                           SignalingTypes type,
                                           const Description &description
                                           );

      virtual SignalingTypes type() const = 0;
      virtual DescriptionPtr description() const = 0;
      virtual SignalingDescription formattedDescription() const = 0;
    };
  }
}
