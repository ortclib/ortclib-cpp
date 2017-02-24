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


#include <ortc/adapter/internal/ortc_adapter_SessionDescription.h>
#include <ortc/adapter/internal/ortc_adapter_SDPParser.h>

#include <ortc/adapter/IHelper.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/Log.h>
#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

#ifdef _MSC_VER
#pragma warning(3 : 4062)
#endif //_MSC_VER

namespace ortc { namespace adapter { ZS_DECLARE_SUBSYSTEM(ortclib_adapter) } }

namespace ortc
{
  namespace adapter
  {
    ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);

    using zsLib::Stringize;
    using zsLib::Numeric;

    namespace internal
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark Helpers
      #pragma mark

      //-----------------------------------------------------------------------
      static Log::Params slog(const char *message)
      {
        return Log::Params(message, "ortc::adapter::ISessionDescriptionTypes");
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SessionDescription
      #pragma mark

      //-----------------------------------------------------------------------
      SessionDescription::SessionDescription(
                                             const make_private &,
                                             SignalingTypes type,
                                             const char *descriptionStr,
                                             const Description *description
                                             ) :
        SharedRecursiveLock(SharedRecursiveLock::create()),
        mType(type),
        mDescription(description ? make_shared<Description>(*description) : DescriptionPtr()),
        mFormattedString(descriptionStr)
      {
      }

      //-----------------------------------------------------------------------
      void SessionDescription::init()
      {
      }

      //-----------------------------------------------------------------------
      SessionDescriptionPtr SessionDescription::convert(ISessionDescriptionPtr object)
      {
        return ZS_DYNAMIC_PTR_CAST(SessionDescription, object);
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SessionDescription => ISessionDescription
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr SessionDescription::toDebug(ISessionDescriptionPtr object)
      {
        if (!object) return ElementPtr();
        return convert(object)->toDebug();
      }

      //-----------------------------------------------------------------------
      SessionDescriptionPtr SessionDescription::create(
                                                       SignalingTypes type,
                                                       const char *description
                                                       )
      {
        auto pThis(make_shared<SessionDescription>(make_private{}, type, description, nullptr));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      SessionDescriptionPtr SessionDescription::create(
                                                       SignalingTypes type,
                                                       const Description &description
                                                       )
      {
        auto pThis(make_shared<SessionDescription>(make_private{}, type, nullptr, &description));
        pThis->mThisWeak = pThis;
        pThis->init();
        return pThis;
      }

      //-----------------------------------------------------------------------
      ISessionDescriptionTypes::SignalingTypes SessionDescription::type() const
      {
        return mType;
      }

      //-----------------------------------------------------------------------
      ISessionDescriptionTypes::DescriptionPtr SessionDescription::description() const
      {
        AutoRecursiveLock lock(*this);

        if (!mDescription) {
          // only attempt to convert once (failures will not attempt again)
          if (mConverted) return mDescription;
          mConverted = true;

          switch (mType) {
            case SignalingType_JSON: {
              auto rootEl = IHelper::toJSON(mFormattedString);
              mDescription = ISessionDescriptionTypes::Description::create(rootEl);
              break;
            }
            case SignalingType_SDPOffer:
            case SignalingType_SDPPranswer:
            case SignalingType_SDPAnswer:
            case SignalingType_SDPRollback: {
              mSDP = SDPParser::parse(mFormattedString);
              if (mSDP) {
                mDescription = SDPParser::createDescription(ISDPTypes::Location_Local, *mSDP);
              }
              break;
            }
          }
        }
        return mDescription;
      }

      //-----------------------------------------------------------------------
      ISessionDescriptionTypes::SignalingDescription SessionDescription::formattedDescription() const
      {
        AutoRecursiveLock lock(*this);
        if (mFormattedString.isEmpty()) {
          if (mConverted) return SignalingDescription();
          mConverted = true;
          switch (mType) {
            case SignalingType_JSON: {
              if (!mDescription) return SignalingDescription();

              ElementPtr rootEl = mDescription->createElement();
              if (!rootEl) return SignalingDescription();

              mFormattedString = zsLib::IHelper::toString(rootEl);
              break;
            }
            case SignalingType_SDPOffer:
            case SignalingType_SDPPranswer:
            case SignalingType_SDPAnswer:
            case SignalingType_SDPRollback: {
              if (!mSDP) {
                if (!mDescription) return SignalingDescription();
                SDPParser::GeneratorOptions options;
                mSDP = SDPParser::createSDP(options, *mDescription);
                if (!mSDP) return SignalingDescription();
              }
              mFormattedString = SDPParser::generate(*mSDP);
              break;
            }
          }
        }

        return mFormattedString;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SessionDescription => (internal)
      #pragma mark

      //-----------------------------------------------------------------------
      ElementPtr SessionDescription::toDebug() const
      {
        ElementPtr resultEl = Element::create("ortc::adapter::internal::SessionDescription");

        IHelper::debugAppend(resultEl, "id", mID);

        IHelper::debugAppend(resultEl, "type", ISessionDescriptionTypes::toString(mType));

        IHelper::debugAppend(resultEl, "converted", mConverted);
        IHelper::debugAppend(resultEl, mDescription ? mDescription->toDebug() : ElementPtr());

        IHelper::debugAppend(resultEl, "formatted", mFormattedString);

        IHelper::debugAppend(resultEl, "sdp", (bool)mSDP);

        return resultEl;
      }

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark IMediaStreamFactory
      #pragma mark

      //-----------------------------------------------------------------------
      ISessionDescriptionFactory &ISessionDescriptionFactory::singleton()
      {
        return SessionDescriptionFactory::singleton();
      }

      //-----------------------------------------------------------------------
      SessionDescriptionPtr ISessionDescriptionFactory::create(
                                                               SignalingTypes type,
                                                               const char *description
                                                               )
      {
        if (this) {}
        return internal::SessionDescription::create(type, description);
      }

      //-----------------------------------------------------------------------
      SessionDescriptionPtr ISessionDescriptionFactory::create(
                                                               SignalingTypes type,
                                                               const Description &description
                                                               )
      {
        if (this) {}
        return internal::SessionDescription::create(type, description);
      }

    }  // namespace internal

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes
    #pragma mark

    //-------------------------------------------------------------------------
    const char *ISessionDescriptionTypes::toString(SignalingTypes type)
    {
      switch (type)
      {
        case SignalingType_JSON:          return "json";
        case SignalingType_SDPOffer:      return "offer";
        case SignalingType_SDPPranswer:   return "pranswer";
        case SignalingType_SDPAnswer:     return "answer";
        case SignalingType_SDPRollback:   return "rollback";
      }
      ORTC_THROW_NOT_SUPPORTED_ERRROR("unknown signaling type");
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::SignalingTypes ISessionDescriptionTypes::toSignalingType(const char *type)
    {
      String str(type);
      for (SignalingTypes index = SignalingType_First; index <= SignalingType_Last; index = static_cast<SignalingTypes>(static_cast<std::underlying_type<SignalingTypes>::type>(index) + 1)) {
        if (0 == str.compareNoCase(toString(index))) return index;
      }

      ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    }

    //-------------------------------------------------------------------------
    const char *ISessionDescriptionTypes::toString(MediaTypes mediaType)
    {
      switch (mediaType)
      {
        case MediaType_Unknown:     return "";
        case MediaType_Audio:       return "audio";
        case MediaType_Video:       return "video";
        case MediaType_Text:        return "text";
        case MediaType_Application: return "application";
      }
      ORTC_THROW_NOT_SUPPORTED_ERRROR("unknown media type");
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::MediaTypes ISessionDescriptionTypes::toMediaType(const char *mediaType)
    {
      String str(mediaType);
      for (MediaTypes index = MediaType_First; index <= MediaType_Last; index = static_cast<MediaTypes>(static_cast<std::underlying_type<MediaTypes>::type>(index) + 1)) {
        if (0 == str.compareNoCase(toString(index))) return index;
      }
      return MediaType_Unknown;
    }

    //-------------------------------------------------------------------------
    Optional<IMediaStreamTrackTypes::Kinds> ISessionDescriptionTypes::toMediaStreamTrackKind(MediaTypes mediaType)
    {
      switch (mediaType)
      {
        case MediaType_Unknown:     break;
        case MediaType_Audio:       return IMediaStreamTrackTypes::Kind_Audio;
        case MediaType_Video:       return IMediaStreamTrackTypes::Kind_Video;
        case MediaType_Text:        break;
        case MediaType_Application: break;
      }
      return Optional<IMediaStreamTrackTypes::Kinds>();
    }

    //-------------------------------------------------------------------------
    const char *ISessionDescriptionTypes::toString(MediaDirections mediaDirection)
    {
      switch (mediaDirection)
      {
        case MediaDirection_SendReceive:  return "sendrecv";
        case MediaDirection_SendOnly:     return "sendonly";
        case MediaDirection_ReceiveOnly:  return "recvonly";
        case MediaDirection_Inactive:     return "inactive";
      }
      ORTC_THROW_NOT_SUPPORTED_ERRROR("unknown media direction");
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::MediaDirections ISessionDescriptionTypes::toMediaDirection(const char *mediaDirection)
    {
      String str(mediaDirection);
      for (MediaDirections index = MediaDirection_First; index <= MediaDirection_Last; index = static_cast<MediaDirections>(static_cast<std::underlying_type<MediaDirections>::type>(index) + 1)) {
        if (0 == str.compareNoCase(toString(index))) return index;
      }

      ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes::ConnectionData
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::ConnectionData::ConnectionData(const ConnectionData &op2)
    {
      mRTP = (op2.mRTP ? make_shared<Details>(*op2.mRTP) : DetailsPtr());
      mRTCP = (op2.mRTCP ? make_shared<Details>(*op2.mRTCP) : DetailsPtr());
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::ConnectionData::ConnectionData(ElementPtr rootEl)
    {
      mRTP = Details::create(rootEl->findFirstChildElement("rtp"));
      mRTCP = Details::create(rootEl->findFirstChildElement("rtcp"));
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::ConnectionData::createElement(const char *objectName) const
    {
      ElementPtr rootEl = Element::create(objectName);

      rootEl->adoptAsLastChild(mRTP ? mRTP->createElement("rtp") : ElementPtr());
      rootEl->adoptAsLastChild(mRTCP ? mRTCP->createElement("rtcp") : ElementPtr());

      if (!rootEl->hasChildren()) return ElementPtr();

      return rootEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::ConnectionData::toDebug() const
    {
      return createElement("ortc::adapter::ISessionDescriptionTypes::ConnectionData");
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::ConnectionData::hash() const
    {
      auto hasher = IHasher::sha1();

      hasher->update("adapter::ISessionDescriptionTypes::ConnectionData:");

      hasher->update(mRTP ? mRTP->hash() : String());
      hasher->update(":");
      hasher->update(mRTCP ? mRTCP->hash() : String());
      hasher->update(":end");

      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes::ConnectionData::Details
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::ConnectionData::Details::Details(ElementPtr rootEl)
    {
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::ConnectionData::Details", "port", mPort);
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::ConnectionData::Details", "netType", mNetType);
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::ConnectionData::Details", "addrType", mAddrType);
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::ConnectionData::Details", "connectionAddress", mConnectionAddress);
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::ConnectionData::Details::createElement(const char *objectName) const
    {
      ElementPtr rootEl = Element::create(objectName);

      IHelper::adoptElementValue(rootEl, "port", mPort);
      IHelper::adoptElementValue(rootEl, "netType", mNetType, false);
      IHelper::adoptElementValue(rootEl, "addrType", mAddrType, false);
      IHelper::adoptElementValue(rootEl, "connectionAddress", mConnectionAddress, false);

      if (!rootEl->hasChildren()) return ElementPtr();

      return rootEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::ConnectionData::Details::toDebug() const
    {
      return createElement("ortc::adapter::ISessionDescriptionTypes::ConnectionData::Details");
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::ConnectionData::Details::hash() const
    {
      auto hasher = IHasher::sha1();

      hasher->update("adapter::ISessionDescriptionTypes::ConnectionData:Details:");

      hasher->update(mPort);
      hasher->update(":");
      hasher->update(mNetType);
      hasher->update(":");
      hasher->update(mAddrType);
      hasher->update(":");
      hasher->update(mConnectionAddress);
      hasher->update(":end");

      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes::Transport
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::Transport::Transport(const Transport &op2) :
      mID(op2.mID)
    {
      mRTP = (op2.mRTP ? make_shared<Parameters>(*op2.mRTP) : ParametersPtr());
      mRTCP = (op2.mRTCP ? make_shared<Parameters>(*op2.mRTCP) : ParametersPtr());
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::Transport::Transport(ElementPtr rootEl)
    {
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::Transport", "id", mID);
      mRTP = Parameters::create(rootEl->findFirstChildElement("rtp"));
      mRTCP = Parameters::create(rootEl->findFirstChildElement("rtcp"));
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::Transport", "useMux", mUseMux);
    }
    
    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::Transport::createElement(const char *objectName) const
    {
      ElementPtr rootEl = Element::create(objectName);

      IHelper::adoptElementValue(rootEl, "id", mID, false);
      rootEl->adoptAsLastChild(mRTP ? mRTP->createElement("rtp") : ElementPtr());
      rootEl->adoptAsLastChild(mRTCP ? mRTCP->createElement("rtcp") : ElementPtr());
      rootEl->adoptAsLastChild(mRTCP ? mRTCP->createElement("rtcp") : ElementPtr());
      IHelper::adoptElementValue(rootEl, "useMux", mUseMux);

      if (!rootEl->hasChildren()) return ElementPtr();

      return rootEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::Transport::toDebug() const
    {
      return createElement("ortc::adapter::ISessionDescriptionTypes::Transport");
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::Transport::hash() const
    {
      auto hasher = IHasher::sha1();

      hasher->update("adapter::ISessionDescriptionTypes::Transport:");

      hasher->update(mID);
      hasher->update(":");
      hasher->update(mRTP ? mRTP->hash() : String());
      hasher->update(":");
      hasher->update(mRTCP ? mRTCP->hash() : String());
      hasher->update(":");
      hasher->update(mUseMux);
      hasher->update(":end");

      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes::Transport::Parameters
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::Transport::Parameters::Parameters(const Parameters &op2) :
      mEndOfCandidates(op2.mEndOfCandidates)
    {
      mICEParameters = (op2.mICEParameters ? make_shared<ICETransportParameters>(*op2.mICEParameters) : ICETransportParametersPtr());
      mDTLSParameters = (op2.mDTLSParameters ? make_shared<DTLSParameters>(*op2.mDTLSParameters) : DTLSParametersPtr());
      mSRTPSDESParameters = (op2.mSRTPSDESParameters ? make_shared<SRTPSDESParameters>(*op2.mSRTPSDESParameters) : SRTPSDESParametersPtr());

      for (auto iter = op2.mICECandidates.begin(); iter != op2.mICECandidates.end(); ++iter) {
        auto candidate = (*iter);
        if (!candidate) continue;

        mICECandidates.push_back(make_shared<IICETypes::Candidate>(*candidate));
      }
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::Transport::Parameters::Parameters(ElementPtr rootEl)
    {
      if (!rootEl) return;

      mICEParameters = ICETransportParameters::create(rootEl->findFirstChildElement("iceParameters"));
      mDTLSParameters = DTLSParameters::create(rootEl->findFirstChildElement("dtlsParameters"));
      mSRTPSDESParameters = SRTPSDESParameters::create(rootEl->findFirstChildElement("srtpSdesParameters"));

      ElementPtr candidatesEl = rootEl->findFirstChildElement("candidates");
      if (candidatesEl) {
        ElementPtr candidateEl = candidatesEl->findFirstChildElement("candidate");
        while (candidateEl) {
          auto candidate = IICETypes::Candidate::convert(IICETypes::Candidate::create(candidateEl));
          candidateEl = candidateEl->findNextSiblingElement("candidate");
          if (!candidate) continue;
          mICECandidates.push_back(candidate);
        }
        ElementPtr completeEl = candidatesEl->findFirstChildElement("candidateComplete");
        if (!completeEl) {
          completeEl = candidatesEl->findFirstChildElement("complete");
        }
        auto complete = IICETypes::CandidateComplete::convert(IICETypes::CandidateComplete::create(completeEl));
        if (complete) {
          mEndOfCandidates = complete->mComplete;
        }
      }
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::Transport::Parameters", "endOfCandidates", mEndOfCandidates);
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::Transport::Parameters::createElement(const char *objectName) const
    {
      ElementPtr rootEl = Element::create(objectName);

      rootEl->adoptAsLastChild(mICEParameters ? mICEParameters->createElement("iceParameters") : ElementPtr());
      rootEl->adoptAsLastChild(mDTLSParameters ? mDTLSParameters->createElement("dtlsParameters") : ElementPtr());
      rootEl->adoptAsLastChild(mSRTPSDESParameters ? mSRTPSDESParameters->createElement("srtpSdesParameters") : ElementPtr());

      ElementPtr iceCandidatesEl = Element::create("candidates");
      for (auto iter = mICECandidates.begin(); iter != mICECandidates.end(); ++iter) {
        auto candidate = (*iter);
        if (!candidate) continue;
        iceCandidatesEl->adoptAsLastChild(candidate->createElement());
      }
      if (mEndOfCandidates) {
        auto complete = make_shared<IICETypes::CandidateComplete>();
        iceCandidatesEl->adoptAsLastChild(complete->createElement());
      }
      if (iceCandidatesEl->hasChildren()) {
        rootEl->adoptAsLastChild(iceCandidatesEl);
      }
      if (mEndOfCandidates) {
        IHelper::adoptElementValue(rootEl, "endOfCandidates", mEndOfCandidates);
      }

      if (!rootEl->hasChildren()) return ElementPtr();

      return rootEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::Transport::Parameters::toDebug() const
    {
      return createElement("ortc::adapter::ISessionDescriptionTypes::Transport:Parameters");
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::Transport::Parameters::hash() const
    {
      auto hasher = IHasher::sha1();

      hasher->update("adapter::ISessionDescriptionTypes::Transport:Parameters:");

      hasher->update(mICEParameters ? mICEParameters->hash() : String());
      hasher->update(":");
      hasher->update(mDTLSParameters ? mDTLSParameters->hash() : String());
      hasher->update(":");
      hasher->update(mSRTPSDESParameters ? mSRTPSDESParameters->hash() : String());
      hasher->update(":iceCandidates:e5b06c5ff5eedd9f3708345a612fac9dac682a42:");
      for (auto iter = mICECandidates.begin(); iter != mICECandidates.end(); ++iter) {
        auto candidate = (*iter);
        if (!candidate) continue;
        hasher->update(candidate->hash());
        hasher->update(":");
      }
      hasher->update(mEndOfCandidates);
      hasher->update(":end");

      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes::MediaLine
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::MediaLine::MediaLine(const MediaLine &op2) :
      mID(op2.mID),
      mTransportID(op2.mTransportID),
      mMediaType(op2.mMediaType)
    {
      mDetails = (op2.mDetails ? make_shared<Details>(*op2.mDetails) : DetailsPtr());
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::MediaLine::MediaLine(ElementPtr rootEl)
    {
      if (!rootEl) return;

      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::MediaLine", "id", mID);
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::MediaLine", "transportId", mTransportID);
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::MediaLine", "mediaType", mMediaType);
      mDetails = Details::create(rootEl->findFirstChildElement("details"));
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::MediaLine::createElement(const char *objectName) const
    {
      ElementPtr rootEl = Element::create(objectName);

      IHelper::adoptElementValue(rootEl, "id", mID, false);
      IHelper::adoptElementValue(rootEl, "transportId", mTransportID, false);
      IHelper::adoptElementValue(rootEl, "mediaType", mMediaType, false);
      rootEl->adoptAsLastChild(mDetails ? mDetails->createElement() : ElementPtr());

      if (!rootEl->hasChildren()) return ElementPtr();

      return rootEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::MediaLine::toDebug() const
    {
      return createElement("ortc::adapter::ISessionDescriptionTypes::MediaLine");
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::MediaLine::hash() const
    {
      auto hasher = IHasher::sha1();

      hasher->update("adapter::ISessionDescriptionTypes::MediaLine:");

      hasher->update(mID);
      hasher->update(":");
      hasher->update(mTransportID);
      hasher->update(":");
      hasher->update(mMediaType);
      hasher->update(":");
      hasher->update(mDetails ? mDetails->hash() : String());
      hasher->update(":end");

      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes::MediaLine::Details
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::MediaLine::Details::Details(const Details &op2) :
      mInternalIndex(op2.mInternalIndex),
      mPrivateTransportID(op2.mPrivateTransportID),
      mProtocol(op2.mProtocol),
      mMediaDirection(op2.mMediaDirection)
    {
      mConnectionData = (op2.mConnectionData ? make_shared<ConnectionData>(*op2.mConnectionData) : ConnectionDataPtr());
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::MediaLine::Details::Details(ElementPtr rootEl)
    {
      if (!rootEl) return;

      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::MediaLine::Details", "index", mInternalIndex);
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::MediaLine::Details", "privateTransportId", mPrivateTransportID);
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::MediaLine::Details", "protocol", mProtocol);
      String directionStr;
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::MediaLine::Details", "direction", directionStr);
      if (directionStr.hasData()) {
        try {
          mMediaDirection = toMediaDirection(directionStr);
        } catch (const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, internal::slog("invalid direction") + ZS_PARAM("direction", directionStr))
        }
      }
      mConnectionData = ConnectionData::create(rootEl->findFirstChildElement("connectinData"));
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::MediaLine::Details::createElement(const char *objectName) const
    {
      ElementPtr rootEl = Element::create(objectName);

      IHelper::adoptElementValue(rootEl, "index", mInternalIndex);
      IHelper::adoptElementValue(rootEl, "privateTransportId", mPrivateTransportID, false);
      IHelper::adoptElementValue(rootEl, "protocol", mProtocol, false);
      IHelper::adoptElementValue(rootEl, "direction", toString(mMediaDirection), false);
      rootEl->adoptAsLastChild(mConnectionData ? mConnectionData->createElement() : ElementPtr());

      if (!rootEl->hasChildren()) return ElementPtr();

      return rootEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::MediaLine::Details::toDebug() const
    {
      return createElement("ortc::adapter::ISessionDescriptionTypes::MediaLine::Details");
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::MediaLine::Details::hash() const
    {
      auto hasher = IHasher::sha1();

      hasher->update("adapter::ISessionDescriptionTypes::MediaLine:Details:");

      hasher->update(mInternalIndex);
      hasher->update(":");
      hasher->update(mPrivateTransportID);
      hasher->update(":");
      hasher->update(mProtocol);
      hasher->update(":");
      hasher->update(toString(mMediaDirection));
      hasher->update(":");
      hasher->update(mConnectionData ? mConnectionData->hash() : String());
      hasher->update(":end");

      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes::RTPMediaLine
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::RTPMediaLine::RTPMediaLine(const RTPMediaLine &op2) :
      MediaLine(op2)
    {
      mSenderCapabilities = (op2.mSenderCapabilities ? make_shared<RTPCapabilities>(*op2.mSenderCapabilities) : RTPCapabilitiesPtr());
      mReceiverCapabilities = (op2.mSenderCapabilities ? make_shared<RTPCapabilities>(*op2.mReceiverCapabilities) : RTPCapabilitiesPtr());
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::RTPMediaLine::RTPMediaLine(ElementPtr rootEl) :
      MediaLine(rootEl)
    {
      mSenderCapabilities = RTPCapabilities::create(rootEl->findFirstChildElement("senderCapabilities"));
      mReceiverCapabilities = RTPCapabilities::create(rootEl->findFirstChildElement("receiverCapabilities"));
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::RTPMediaLine::createElement(const char *objectName) const
    {
      if (!objectName) objectName = "rtpMediaLine";

      ElementPtr rootEl = MediaLine::createElement(objectName);

      rootEl->adoptAsLastChild(mSenderCapabilities ? mSenderCapabilities->createElement("senderCapabilities") : ElementPtr());
      rootEl->adoptAsLastChild(mReceiverCapabilities ? mReceiverCapabilities->createElement("receiverCapabilities") : ElementPtr());

      if (!rootEl->hasChildren()) return ElementPtr();

      return rootEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::RTPMediaLine::toDebug() const
    {
      return createElement("ortc::adapter::ISessionDescriptionTypes::RTPMediaLine");
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::RTPMediaLine::hash() const
    {
      auto hasher = IHasher::sha1();

      hasher->update("adapter::ISessionDescriptionTypes::RTPMediaLine:");

      hasher->update(MediaLine::hash());
      hasher->update(":");
      hasher->update(mSenderCapabilities ? mSenderCapabilities->hash() : String());
      hasher->update(":");
      hasher->update(mReceiverCapabilities ? mReceiverCapabilities->hash() : String());
      hasher->update(":end");

      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes::SCTPMediaLine
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::SCTPMediaLine::SCTPMediaLine(const SCTPMediaLine &op2) :
      MediaLine(op2),
      mPort(op2.mPort)
    {
      mCapabilities = (op2.mDetails ? make_shared<SCTPCapabilities>(*op2.mCapabilities) : SCTPCapabilitiesPtr());
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::SCTPMediaLine::SCTPMediaLine(ElementPtr rootEl) :
      MediaLine(rootEl)
    {
      if (!rootEl) return;

      mCapabilities = SCTPCapabilities::create(rootEl->findFirstChildElement("capabilities"));
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::SCTPMediaLine", "port", mPort);
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::SCTPMediaLine::createElement(const char *objectName) const
    {
      if (!objectName) objectName = "sctpMediaLine";

      ElementPtr rootEl = MediaLine::createElement(objectName);

      rootEl->adoptAsLastChild(mCapabilities ? mCapabilities->createElement("capabilities") : ElementPtr());
      IHelper::adoptElementValue(rootEl, "port", mPort);

      if (!rootEl->hasChildren()) return ElementPtr();

      return rootEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::SCTPMediaLine::toDebug() const
    {
      return createElement("ortc::adapter::ISessionDescriptionTypes::SCTPMediaLine");
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::SCTPMediaLine::hash() const
    {
      auto hasher = IHasher::sha1();

      hasher->update("adapter::ISessionDescriptionTypes::SCTPMediaLine:");

      hasher->update(MediaLine::hash());
      hasher->update(":");
      hasher->update(mCapabilities ? mCapabilities->hash() : String());
      hasher->update(":");
      hasher->update(mPort);
      hasher->update(":end");

      return hasher->finalizeAsString();
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes::RTPSender
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::RTPSender::RTPSender(const RTPSender &op2) :
      mID(op2.mID),
      mRTPMediaLineID(op2.mRTPMediaLineID),
      mMediaStreamTrackID(op2.mMediaStreamTrackID),
      mMediaStreamIDs(op2.mMediaStreamIDs)
    {
      mDetails = (op2.mDetails ? make_shared<Details>(*op2.mDetails) : DetailsPtr());
      mParameters = (op2.mParameters ? (make_shared<RTPParameters>(*op2.mParameters)) : RTPParametersPtr());
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::RTPSender::RTPSender(ElementPtr rootEl)
    {
      if (!rootEl) return;

      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::RTPSender", "id", mID);
      mDetails = Details::create(rootEl->findFirstChildElement("details"));
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::RTPSender", "rtpMediaLineId", mRTPMediaLineID);
      mParameters = RTPParameters::create(rootEl->findFirstChildElement("rtpParameters"));
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::RTPSender", "mediaStreamTrackId", mMediaStreamTrackID);

      // scope: get media stream IDs
      {
        ElementPtr mediaStreamIDsEl = rootEl->findFirstChildElement("mediaSteamIds");
        if (mediaStreamIDsEl) {
          ElementPtr mediaStreamIDEl = mediaStreamIDsEl->findFirstChildElement("mediaStreamId");
          while (mediaStreamIDEl)
          {
            String mediaStreamID = IHelper::getElementTextAndDecode(mediaStreamIDEl);
            mediaStreamIDEl = mediaStreamIDEl->findNextSiblingElement("mediaStreamId");
            if (mediaStreamID.isEmpty()) continue;
            mMediaStreamIDs.insert(mediaStreamID);
          }
        }
      }
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::RTPSender::createElement(const char *objectName) const
    {
      if (!objectName) objectName = "rtpSender";

      ElementPtr rootEl = Element::create(objectName);

      IHelper::adoptElementValue(rootEl, "id", mID, false);
      if (mDetails) {
        rootEl->adoptAsLastChild(mDetails->createElement());
      }
      IHelper::adoptElementValue(rootEl, "rtpMediaLineId", mRTPMediaLineID, false);
      if (mParameters) {
        rootEl->adoptAsLastChild(mParameters->createElement("rtpParameters"));
      }
      IHelper::adoptElementValue(rootEl, "mediaStreamTrackId", mMediaStreamTrackID, false);

      if (mMediaStreamIDs.size() > 0) {
        ElementPtr mediaStreamIDsEl = Element::create("mediaSteamIds");
        for (auto iter = mMediaStreamIDs.begin(); iter != mMediaStreamIDs.end(); ++iter)
        {
          auto streamID = (*iter);
          IHelper::adoptElementValue(mediaStreamIDsEl, "mediaSteamId", streamID, false);
        }
        if (mediaStreamIDsEl->hasChildren()) {
          rootEl->adoptAsLastChild(mediaStreamIDsEl);
        }
      }

      if (!rootEl->hasChildren()) return ElementPtr();

      return rootEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::RTPSender::toDebug() const
    {
      return createElement("ortc::adapter::ISessionDescriptionTypes::RTPSender");
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::RTPSender::hash() const
    {
      auto hasher = IHasher::sha1();

      hasher->update("adapter::ISessionDescriptionTypes::RTPSender:");

      hasher->update(mID);
      hasher->update(":");
      hasher->update(mDetails ? mDetails->hash() : String());
      hasher->update(":");
      hasher->update(mRTPMediaLineID);
      hasher->update(":");
      hasher->update(mMediaStreamTrackID);
      hasher->update(":");
      hasher->update(mParameters ? mParameters->hash() : String());
      hasher->update(":mediaStreamIds:c67e2347f3017b042808a99de9e935a17409226c");

      for (auto iter = mMediaStreamIDs.begin(); iter != mMediaStreamIDs.end(); ++iter)
      {
        auto streamID = (*iter);
        hasher->update(streamID);
        hasher->update(":");
      }
      hasher->update(":end");

      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes::RTPSender::Details
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::RTPSender::Details::Details(const Details &op2) :
      mInternalRTPMediaLineIndex(op2.mInternalRTPMediaLineIndex)
    {
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::RTPSender::Details::Details(ElementPtr rootEl)
    {
      if (!rootEl) return;

      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::RTPSender::Details", "index", mInternalRTPMediaLineIndex);
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::RTPSender::Details::createElement(const char *objectName) const
    {
      if (!objectName) objectName = "details";

      ElementPtr rootEl = Element::create(objectName);

      IHelper::adoptElementValue(rootEl, "index", mInternalRTPMediaLineIndex);

      if (!rootEl->hasChildren()) return ElementPtr();

      return rootEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::RTPSender::Details::toDebug() const
    {
      return createElement("ortc::adapter::ISessionDescriptionTypes::RTPSender::Details");
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::RTPSender::Details::hash() const
    {
      auto hasher = IHasher::sha1();

      hasher->update("adapter::ISessionDescriptionTypes::RTPSender:Details:");

      hasher->update(mInternalRTPMediaLineIndex);

      hasher->update(":end");

      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes::ICECandidate
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::ICECandidate::ICECandidate(const ICECandidate &op2) :
      mMid(op2.mMid),
      mMLineIndex(op2.mMLineIndex)
    {
      if (op2.mCandidate) {
        {
          auto iceCandidate = IICETypes::Candidate::convert(op2.mCandidate);
          if (iceCandidate) {
            mCandidate = make_shared<IICETypes::Candidate>(*iceCandidate);
          }
        }
        {
          auto iceCandidate = IICETypes::CandidateComplete::convert(op2.mCandidate);
          if (iceCandidate) {
            mCandidate = make_shared<IICETypes::CandidateComplete>(*iceCandidate);
          }
        }
      }
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::ICECandidatePtr ISessionDescriptionTypes::ICECandidate::create(ElementPtr rootEl)
    {
      if (!rootEl) return ICECandidatePtr();

      auto result = make_shared<ICECandidate>();

      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::ICECandidate", "mid", result->mMid);
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::ICECandidate", "index", result->mMLineIndex);

      bool isComplete = false;

      ElementPtr candidateEl = rootEl->findFirstChildElement("candidate");
      if (!candidateEl) {
        isComplete = true;
        candidateEl = rootEl->findFirstChildElement("candidateComplete");
        if (!candidateEl) {
          candidateEl = rootEl->findFirstChildElement("complete");
        }
      }

      if (!candidateEl) return ICECandidatePtr();

      if (isComplete) {
        result->mCandidate = IICETypes::CandidateComplete::create(candidateEl);
        return result;
      }

      result->mCandidate = IICETypes::Candidate::create(candidateEl);
      return result;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::ICECandidate::createElement(const char *objectName) const
    {
      if (!objectName) objectName = "iceCandidate";

      ElementPtr candidateEl;

      {
        auto iceCandidate = IICETypes::Candidate::convert(mCandidate);
        if (iceCandidate) {
          candidateEl = iceCandidate->createElement();
        }
      }
      {
        auto iceCandidate = IICETypes::CandidateComplete::convert(mCandidate);
        if (iceCandidate) {
          candidateEl = iceCandidate->createElement();
        }
      }

      if (!candidateEl) return ElementPtr();

      ElementPtr rootEl = Element::create(objectName);

      IHelper::adoptElementValue(rootEl, "mid", mMid, false);
      IHelper::adoptElementValue(rootEl, "index", mMLineIndex);
      rootEl->adoptAsLastChild(candidateEl);
      return rootEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::ICECandidate::toDebug() const
    {
      return createElement("ortc::adapter::ISessionDescriptionTypes::ICECandidate");
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::ICECandidate::hash() const
    {
      auto hasher = IHasher::sha1();

      hasher->update("adapter::ISessionDescriptionTypes::ICECandidate:");

      hasher->update(mMid);
      hasher->update(":");
      hasher->update(mMLineIndex);
      hasher->update(":");

      {
        auto iceCandidate = IICETypes::Candidate::convert(mCandidate);
        if (iceCandidate) {
          hasher->update(iceCandidate->hash());
        }
      }
      hasher->update(":");
      {
        auto iceCandidate = IICETypes::CandidateComplete::convert(mCandidate);
        if (iceCandidate) {
          hasher->update(iceCandidate->hash());
        }
      }

      hasher->update(":end");
      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::ICECandidatePtr ISessionDescriptionTypes::ICECandidate::createFromSDP(const char *value)
    {
      String str(value);

      auto result = make_shared<ICECandidate>();
      auto candidate = internal::SDPParser::getCandidateFromSDP(value);
      result->mCandidate = candidate;
      return result;
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::ICECandidate::toSDP() const
    {
      {
        auto candidateComplete = ZS_DYNAMIC_PTR_CAST(IICETypes::CandidateComplete, mCandidate);
        if (candidateComplete) {
          return String("a=end-of-candidates");
        }
      }

      {
        auto candidate = ZS_DYNAMIC_PTR_CAST(IICETypes::Candidate, mCandidate);
        if (candidate) {
          return internal::SDPParser::getCandidateSDP(*candidate);
        }
      }
      return String();
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::ICECandidate::toJSON() const
    {
      return createElement();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes::Description
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::Description::Description(const Description &op2) :
      mDetails(op2.mDetails ? make_shared<Details>(*op2.mDetails) : DetailsPtr())
    {
      for (auto iter = op2.mTransports.begin(); iter != op2.mTransports.end(); ++iter)
      {
        auto transport = (*iter);
        if (!transport) continue;
        mTransports.push_back(make_shared<Transport>(*transport));
      }
      for (auto iter = op2.mRTPMediaLines.begin(); iter != op2.mRTPMediaLines.end(); ++iter)
      {
        auto mediaLine = (*iter);
        if (!mediaLine) continue;
        mRTPMediaLines.push_back(make_shared<RTPMediaLine>(*mediaLine));
      }
      for (auto iter = op2.mSCTPMediaLines.begin(); iter != op2.mSCTPMediaLines.end(); ++iter)
      {
        auto mediaLine = (*iter);
        if (!mediaLine) continue;
        mSCTPMediaLines.push_back(make_shared<SCTPMediaLine>(*mediaLine));
      }
      for (auto iter = op2.mRTPSenders.begin(); iter != op2.mRTPSenders.end(); ++iter)
      {
        auto rtpSender = (*iter);
        if (!rtpSender) continue;
        mRTPSenders.push_back(make_shared<RTPSender>(*rtpSender));
      }
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::Description::Description(ElementPtr rootEl)
    {
      if (!rootEl) return;

      mDetails = Details::create(rootEl->findFirstChildElement("details"));

      // scope: check for transports
      {
        ElementPtr transportsEl = rootEl->findFirstChildElement("transports");
        if (transportsEl) {
          ElementPtr transportEl = transportsEl->findFirstChildElement("transport");
          while (transportEl) {
            auto transport = Transport::create(transportEl);
            transportEl = transportEl->findNextSiblingElement("transport");

            if (!transport) continue;
            mTransports.push_back(transport);
          }
        }
      }

      // scope: check for rtp media lines
      {
        ElementPtr mediaLinesEl = rootEl->findFirstChildElement("rtpMediaLines");
        if (mediaLinesEl) {
          ElementPtr mediaLineEl = mediaLinesEl->findFirstChildElement("rtpMediaLine");
          while (mediaLineEl) {
            auto mediaLine = RTPMediaLine::create(mediaLineEl);
            mediaLineEl = mediaLineEl->findNextSiblingElement("rtpMediaLine");

            if (!mediaLine) continue;
            mRTPMediaLines.push_back(mediaLine);
          }
        }
      }

      // scope: check for sctp media lines
      {
        ElementPtr mediaLinesEl = rootEl->findFirstChildElement("sctpMediaLines");
        if (mediaLinesEl) {
          ElementPtr mediaLineEl = mediaLinesEl->findFirstChildElement("sctpMediaLine");
          while (mediaLineEl) {
            auto mediaLine = SCTPMediaLine::create(mediaLineEl);
            mediaLineEl = mediaLineEl->findNextSiblingElement("sctpMediaLine");

            if (!mediaLine) continue;
            mSCTPMediaLines.push_back(mediaLine);
          }
        }
      }

      // scope: check for rtp senders
      {
        ElementPtr rtpSendersEl = rootEl->findFirstChildElement("rtpSenders");
        if (rtpSendersEl) {
          ElementPtr rtpSenderEl = rtpSendersEl->findFirstChildElement("rtpSender");
          while (rtpSenderEl) {
            auto rtpSender = RTPSender::create(rtpSenderEl);
            rtpSenderEl = rtpSenderEl->findNextSiblingElement("rtpSender");

            if (!rtpSender) continue;
            mRTPSenders.push_back(rtpSender);
          }
        }
      }
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::Description::createElement(const char *objectName) const
    {
      if (!objectName) objectName = "session";

      ElementPtr rootEl = Element::create(objectName);

      if (mDetails) {
        rootEl->adoptAsLastChild(mDetails->createElement());
      }

      if (mTransports.size() > 0) {
        ElementPtr transportsEl = Element::create("transports");
        for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter)
        {
          auto transport = (*iter);
          if (!transport) continue;
          transportsEl->adoptAsLastChild(transport->createElement());
        }
        rootEl->adoptAsLastChild(transportsEl);
      }

      if (mRTPMediaLines.size() > 0) {
        ElementPtr mediaLinesEl = Element::create("rtpMediaLines");
        for (auto iter = mRTPMediaLines.begin(); iter != mRTPMediaLines.end(); ++iter)
        {
          auto mediaLine = (*iter);
          if (!mediaLine) continue;
          mediaLinesEl->adoptAsLastChild(mediaLine->createElement());
        }
        rootEl->adoptAsLastChild(mediaLinesEl);
      }
      if (mSCTPMediaLines.size() > 0) {
        ElementPtr mediaLinesEl = Element::create("sctpMediaLines");
        for (auto iter = mSCTPMediaLines.begin(); iter != mSCTPMediaLines.end(); ++iter)
        {
          auto mediaLine = (*iter);
          if (!mediaLine) continue;
          mediaLinesEl->adoptAsLastChild(mediaLine->createElement());
        }
        rootEl->adoptAsLastChild(mediaLinesEl);
      }
      if (mRTPSenders.size() > 0) {
        ElementPtr rtpSendersEl = Element::create("rtpSenders");
        for (auto iter = mRTPSenders.begin(); iter != mRTPSenders.end(); ++iter)
        {
          auto rtpSender = (*iter);
          if (!rtpSender) continue;
          rtpSendersEl->adoptAsLastChild(rtpSender->createElement());
        }
        rootEl->adoptAsLastChild(rtpSendersEl);
      }

      if (!rootEl->hasChildren()) return ElementPtr();

      return rootEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::Description::toDebug() const
    {
      return createElement("ortc::adapter::ISessionDescriptionTypes::Description");
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::Description::hash() const
    {
      auto hasher = IHasher::sha1();

      hasher->update("adapter::ISessionDescriptionTypes::Description:");

      hasher->update(mDetails ? mDetails->hash() : String());

      hasher->update(":transports:436e7244e8d2c773ff7827e40ab1775cfc898a36:");

      for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter)
      {
        auto transport = (*iter);
        if (!transport) continue;
        hasher->update(transport->hash());
        hasher->update(":");
      }
      hasher->update(":rtpMediaLines:b45495b53fe9e894816a794e959e475d68a31d0e:");
      for (auto iter = mRTPMediaLines.begin(); iter != mRTPMediaLines.end(); ++iter)
      {
        auto mediaLine = (*iter);
        if (!mediaLine) continue;
        hasher->update(mediaLine->hash());
        hasher->update(":");
      }
      hasher->update(":sctpMediaLines:d62607480f61c36891f6b97a55623d83fff0749c:");
      for (auto iter = mSCTPMediaLines.begin(); iter != mSCTPMediaLines.end(); ++iter)
      {
        auto mediaLine = (*iter);
        if (!mediaLine) continue;
        hasher->update(mediaLine->hash());
        hasher->update(":");
      }
      hasher->update(":rtpSenders:02a37fd161e55bdb384517cecb6f8c52ca5fa113:");
      for (auto iter = mRTPSenders.begin(); iter != mRTPSenders.end(); ++iter)
      {
        auto sender = (*iter);
        if (!sender) continue;
        hasher->update(sender->hash());
        hasher->update(":");
      }
      hasher->update(":end");

      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescriptionTypes::Description::Details
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::Description::Description::Details::Details(const Details &op2) :
      mUsername(op2.mUsername),
      mSessionID(op2.mSessionID),
      mSessionVersion(op2.mSessionVersion),
      mUnicaseAddress(op2.mUnicaseAddress ? make_shared<ConnectionData::Details>(*op2.mUnicaseAddress) : ConnectionData::DetailsPtr()),
      mSessionName(op2.mSessionName),
      mStartTime(op2.mStartTime),
      mEndTime(op2.mEndTime),
      mConnectionData(op2.mConnectionData ? make_shared<ConnectionData>(*op2.mConnectionData) : ConnectionDataPtr())
    {
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::Description::Details::Details(ElementPtr rootEl)
    {
      if (!rootEl) return;

      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::Description::Details", "username", mUsername);

      {
        String sessionID;
        IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::Description::Details", "id", sessionID);

        try {
          mSessionID = Numeric<decltype(mSessionID)>(sessionID);
        } catch (const Numeric<decltype(mSessionID)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, internal::slog("session id is out of range: ") + ZS_PARAM("value", sessionID));
        }
      }

      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::Description::Details", "version", mSessionVersion);
      mUnicaseAddress = ConnectionData::Details::create(rootEl->findFirstChildElement("unicastAddress"));
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::Description::Details", "name", mSessionName);
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::Description::Details", "startTime", mStartTime);
      IHelper::getElementValue(rootEl, "ortc::adapter::ISessionDescriptionTypes::Description::Details", "endTime", mEndTime);
      mConnectionData = ConnectionData::create(rootEl->findFirstChildElement("connectionData"));
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::Description::Details::createElement(const char *objectName) const
    {
      if (!objectName) objectName = "details";

      ElementPtr rootEl = Element::create(objectName);

      IHelper::adoptElementValue(rootEl, "username", mUsername, false);
      IHelper::adoptElementValue(rootEl, "id", string(mSessionID), false);
      IHelper::adoptElementValue(rootEl, "version", mSessionVersion);
      if (mUnicaseAddress) {
        rootEl->adoptAsLastChild(mUnicaseAddress->createElement("unicastAddress"));
      }
      IHelper::adoptElementValue(rootEl, "name", mSessionName, false);
      IHelper::adoptElementValue(rootEl, "startTime", mStartTime);
      IHelper::adoptElementValue(rootEl, "endTime", mEndTime);
      if (mConnectionData) {
        rootEl->adoptAsLastChild(mConnectionData->createElement());
      }

      if (!rootEl->hasChildren()) return ElementPtr();

      return rootEl;
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::Description::Details::toDebug() const
    {
      return createElement("ortc::adapter::ISessionDescriptionTypes::Description::Details");
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::Description::Details::hash() const
    {
      auto hasher = IHasher::sha1();

      hasher->update("adapter::ISessionDescriptionTypes::Description:Details:");

      hasher->update(mUsername);
      hasher->update(":");
      hasher->update(mSessionID);
      hasher->update(":");
      hasher->update(mSessionVersion);
      hasher->update(":");
      if (mUnicaseAddress) {
        hasher->update(mUnicaseAddress->hash());
      }
      hasher->update(":");
      hasher->update(mSessionName);
      hasher->update(":");
      hasher->update(mStartTime);
      hasher->update(":");
      hasher->update(mEndTime);
      hasher->update(":");
      if (mConnectionData) {
        hasher->update(mConnectionData->hash());
      }
      hasher->update(":end");

      return hasher->finalizeAsString();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescription
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionPtr ISessionDescription::create(
                                                       SignalingTypes type,
                                                       const char *description
                                                       )
    {
      return internal::ISessionDescriptionFactory::singleton().create(type, description);
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionPtr ISessionDescription::create(
                                                       SignalingTypes type,
                                                       const Description &description
                                                       )
    {
      return internal::ISessionDescriptionFactory::singleton().create(type, description);
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescription::toDebug(ISessionDescriptionPtr object)
    {
      return internal::SessionDescription::toDebug(object);
    }

  } // namespace adapter
} // namespace ortc
