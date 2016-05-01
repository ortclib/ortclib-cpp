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

#include <map>
#include <set>

namespace ortc
{
  namespace adapter
  {
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes
    #pragma mark

    interaction ISessionDescriptionTypes
    {
      ZS_DECLARE_STRUCT_PTR(Description)
      ZS_DECLARE_STRUCT_PTR(Origin)
      ZS_DECLARE_STRUCT_PTR(ConnectionData)
      ZS_DECLARE_STRUCT_PTR(Transport)
      ZS_DECLARE_STRUCT_PTR(MediaLine)
      ZS_DECLARE_STRUCT_PTR(MediaStreamTrack)
      ZS_DECLARE_TYPEDEF_PTR(IICETypes::Parameters, TransportParameters)
      ZS_DECLARE_TYPEDEF_PTR(IDTLSTransport::Parameters, DTLSParameters)
      ZS_DECLARE_TYPEDEF_PTR(ISRTPSDESTransport::Parameters, SRTPSDESParameters)
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::Capabilities, RTPCapabilities)
      ZS_DECLARE_TYPEDEF_PTR(IRTPTypes::ParametersPtr, RTPParameters)

      typedef String MediaStreamTrackID;
      typedef std::map<MediaStreamTrackID, MediaStreamTrackPtr> MediaStreamTrackMap;

      typedef String MediaStreamID;
      typedef std::set<MediaStreamID> MediaStreamSet;

      typedef String TransportID;
      typedef std::map<TransportID, TransportPtr> TransportMap;

      typedef String MediaLineID;
      typedef std::map<MediaLineID, MediaLine> MediaLineMap;

      typedef String BundleID;

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

      struct Origin
      {
        String mUsername;
        String mSessionID;
        ULONGLONG mSessionVersion;
        String mNetType;
        String AddrType;
        String mUnicastAddress;
      };

      struct ConnectionData
      {
        String mNetType;
        String mAddrType;
        String mConnectionAddress;
      };

      struct Transport
      {
        TransportID mID;
        TransportParameters mIceParameters;
        DTLSParametersPtr mDTLSParameters;
        SRTPSDESParametersPtr mSRTPSDESParameters;
      };

      struct MediaLine
      {
        ZS_DECLARE_STRUCT_PTR(Details)

        struct Details
        {
          Optional<WORD> mPort;
          Optional<WORD> mRTCPPort;

          ConnectionDataPtr mConnectionData;      // optional; can be null;
          ConnectionDataPtr mRTCPConnectionData;  // optiona; can be null;

          String mTransportProtocol;
        };

        static Optional<IMediaStreamTrackTypes::Kinds> toMediaStreamTrackKind(const char *mediaType);

        MediaTypes mMediaType {MediaType_Unknown};
        String mMediaTypeUnknown;

        BundleID mBundleID;                 // all media lines that can be bundled must share the same bundle ID
        DetailsPtr mDetails;                // optional; can be null;
        RTPCapabilitiesPtr mCapabilities;

        TransportPtr mRTPTransport;
        TransportPtr mRTCPTransport;        // optional; can be null;
      };

      struct Description
      {
        OriginPtr mOrigin;                  // optional; can be null;
        String mSessionName;
        ConnectionDataPtr mConnectionData;  // optional; can be null;

        TransportMap mTransports;
        MediaLineMap mMediaLines;

        MediaStreamTrackMap mSendingTracks;
      };

      struct MediaTrack
      {
        MediaStreamTrackID mID;
        TransportPtr mRTPTransport;
        RTPParametersPtr mParameters;
        MediaStreamSet mMediaStreams;
      };

    };

    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    //---------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescription
    #pragma mark

    interface ISessionDescription : public ISessionDescriptionTypes
    {
      ZS_DECLARE_STRUCT_PTR(ParserOptions)
      ZS_DECLARE_STRUCT_PTR(GeneratorOptions)

      enum Modes
      {
        Mode_SDP,
        Mode_JSON,
      };

      struct ParserOptions
      {
        Optional<Modes> mMode;

        ParserOptions() {}
      };

      struct GeneratorOptions
      {
        Modes mMode {Mode_JSON};

        GeneratorOptions() {}
      };

      static DescriptionPtr parse(
                                  const char *description,
                                  const ParserOptions &options
                                  );

      static String generate(
                             const Description &description,
                             const GeneratorOptions &options
                             );

      virtual ~ISessionDescription() {}
    };
  }
}
