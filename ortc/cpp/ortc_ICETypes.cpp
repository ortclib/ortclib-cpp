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

#include <ortc/IICETypes.h>
#include <ortc/internal/types.h>
#include <ortc/internal/platform.h>
#include <ortc/internal/ortc_Helper.h>

#include <ortc/IHelper.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/XML.h>
#include <zsLib/Numeric.h>

#include <cryptopp/sha.h>


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);

  using zsLib::Numeric;
  using zsLib::Log;


  //-----------------------------------------------------------------------
  static Log::Params slog(const char *message)
  {
    return Log::Params(message, "ortc::IICETypes");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETypes::Roles
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IICETypes::toString(Roles roles)
  {
    switch (roles) {
      case Role_Controlling:  return "controlling";
      case Role_Controlled:   return "controlled";
    }
    return "unknown";
  }

  //---------------------------------------------------------------------------
  IICETypes::Roles IICETypes::toRole(const char *role) throw (InvalidParameters)
  {
    String str(role);
    for (IICETypes::Roles index = IICETypes::Role_First; index <= IICETypes::Role_Last; index = static_cast<IICETypes::Roles>(static_cast<std::underlying_type<IICETypes::Roles>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IICETypes::toString(index))) return index;
    }

    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return Role_First;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETypes::Components
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IICETypes::toString(Components component)
  {
    switch (component) {
      case Component_RTP:   return "rtp";
      case Component_RTCP:  return "rtcp";
    }
    return "unknown";
  }

  //---------------------------------------------------------------------------
  IICETypes::Components IICETypes::toComponent(const char *component) throw (InvalidParameters)
  {
    String str(component);
    for (IICETypes::Components index = IICETypes::Component_First; index <= IICETypes::Component_Last; index = static_cast<IICETypes::Components>(static_cast<std::underlying_type<IICETypes::Components>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IICETypes::toString(index))) return index;
    }

    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return Component_First;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETypes::Protocols
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IICETypes::toString(Protocols protocol)
  {
    switch (protocol) {
      case Protocol_UDP:  return "udp";
      case Protocol_TCP:  return "tcp";
    }
    return "unknown";
  }

  //---------------------------------------------------------------------------
  IICETypes::Protocols IICETypes::toProtocol(const char *protocol) throw (InvalidParameters)
  {
    String str(protocol);
    for (IICETypes::Protocols index = IICETypes::Protocol_First; index <= IICETypes::Protocol_Last; index = static_cast<IICETypes::Protocols>(static_cast<std::underlying_type<IICETypes::Protocols>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IICETypes::toString(index))) return index;
    }

    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return Protocol_First;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETypes::CandidateTypes
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IICETypes::toString(CandidateTypes candidateType)
  {
    switch (candidateType) {
      case CandidateType_Host:    return "host";
      case CandidateType_Srflex:  return "srflx";
      case CandidateType_Prflx:   return "prflx";
      case CandidateType_Relay:   return "relay";
    }
    return "unknown";
  }

  //---------------------------------------------------------------------------
  IICETypes::CandidateTypes IICETypes::toCandidateType(const char *candidateType) throw (InvalidParameters)
  {
    String str(candidateType);
    for (IICETypes::CandidateTypes index = IICETypes::CandidateType_First; index <= IICETypes::CandidateType_Last; index = static_cast<IICETypes::CandidateTypes>(static_cast<std::underlying_type<IICETypes::CandidateTypes>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IICETypes::toString(index))) return index;
    }

    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return CandidateType_First;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETypes::TCPCandidateTypes
  #pragma mark

  //---------------------------------------------------------------------------
  const char *IICETypes::toString(TCPCandidateTypes protocol)
  {
    switch (protocol) {
      case TCPCandidateType_Active:   return "active";
      case TCPCandidateType_Passive:  return "passive";
      case TCPCandidateType_SO:       return "so";
    }
    return "unknown";
  }

  //---------------------------------------------------------------------------
  IICETypes::TCPCandidateTypes IICETypes::toTCPCandidateType(const char *tcpCandidateType) throw (InvalidParameters)
  {
    String str(tcpCandidateType);
    for (IICETypes::TCPCandidateTypes index = IICETypes::TCPCandidateType_First; index <= IICETypes::TCPCandidateType_Last; index = static_cast<IICETypes::TCPCandidateTypes>(static_cast<std::underlying_type<IICETypes::TCPCandidateTypes>::type>(index) + 1)) {
      if (0 == str.compareNoCase(IICETypes::toString(index))) return index;
    }

    ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
    return TCPCandidateType_First;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETypes::GatherCandidate
  #pragma mark

  //---------------------------------------------------------------------------
  IICETypes::GatherCandidatePtr IICETypes::GatherCandidate::create(ElementPtr elem)
  {
    if (!elem) return GatherCandidatePtr();

    ElementPtr childEl = elem->getFirstChildElement();
    ElementPtr completeEl = elem->findFirstChildElement("complete");
    if ((completeEl) ||
        (!childEl)) {
      CandidateCompletePtr pThis(make_shared<CandidateComplete>(elem));
      return pThis;
    }

    CandidatePtr pThis(make_shared<Candidate>(elem));
    return pThis;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETypes::Candidate
  #pragma mark

  //---------------------------------------------------------------------------
  IICETypes::CandidatePtr IICETypes::Candidate::convert(GatherCandidatePtr candidate)
  {
    return ZS_DYNAMIC_PTR_CAST(Candidate, candidate);
  }

  //---------------------------------------------------------------------------
  IICETypes::Candidate::Candidate(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IICETypes::Candidate", "interfaceType", mInterfaceType);
    IHelper::getElementValue(elem, "ortc::IICETypes::Candidate", "foundation", mFoundation);

    {
      String str = IHelper::getElementText(elem->findFirstChildElement("component"));
      if (str.hasData()) {
        try {
          std::underlying_type<decltype(mComponent)>::type converted = Numeric<std::underlying_type<decltype(mComponent)>::type>(str);
          IICETypes::Components component = static_cast<decltype(mComponent)>(converted);
          if ((component >= IICETypes::Component_First) ||
              (component <= IICETypes::Component_Last)) {
            mComponent = component;
          } else {
            ZS_LOG_WARNING(Debug, slog("component value invalid") + ZS_PARAM("value", str))
          }
        } catch(const Numeric<std::underlying_type<decltype(mComponent)>::type>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog("component value invalid") + ZS_PARAM("value", str))
        }
      }
    }

    IHelper::getElementValue(elem, "ortc::IICETypes::Candidate", "priority", mPriority);
    IHelper::getElementValue(elem, "ortc::IICETypes::Candidate", "unfreezePriority", mUnfreezePriority);


    {
      String str = IHelper::getElementText(elem->findFirstChildElement("protocol"));
      if (str.hasData()) {
        try {
          mProtocol = IICETypes::toProtocol(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("protocol value invalid") + ZS_PARAM("value", str))
        }
      }
    }

    IHelper::getElementValue(elem, "ortc::IICETypes::Candidate", "ip", mIP);
    IHelper::getElementValue(elem, "ortc::IICETypes::Candidate", "port", mPort);

    {
      String str = IHelper::getElementText(elem->findFirstChildElement("type"));
      if (str.hasData()) {
        try {
          mCandidateType = IICETypes::toCandidateType(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("candidate type value invalid") + ZS_PARAM("value", str))
        }
      }
    }

    {
      String str = IHelper::getElementText(elem->findFirstChildElement("tcpType"));
      if (str.hasData()) {
        try {
          mTCPType = IICETypes::toTCPCandidateType(str);
        } catch(const InvalidParameters &) {
          ZS_LOG_WARNING(Debug, slog("candidate type value invalid") + ZS_PARAM("value", str))
        }
      }
    }

    IHelper::getElementValue(elem, "ortc::IICETypes::Candidate", "relatedAddress", mRelatedAddress);
    IHelper::getElementValue(elem, "ortc::IICETypes::Candidate", "relatedPort", mRelatedPort);
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETypes::Candidate::createElement(const char *objectName) const
  {
    if (NULL == objectName) objectName = "candidate";
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "interfaceType", mInterfaceType, false);
    IHelper::adoptElementValue(elem, "foundation", mFoundation, false);
    IHelper::adoptElementValue(elem, "component", static_cast<std::underlying_type<IICETypes::Components>::type>(mComponent));
    IHelper::adoptElementValue(elem, "priority", mPriority);
    IHelper::adoptElementValue(elem, "unfreezePriority", mUnfreezePriority);
    IHelper::adoptElementValue(elem, "protocol", IICETypes::toString(mProtocol), false);
    IHelper::adoptElementValue(elem, "ip", mIP, false);
    IHelper::adoptElementValue(elem, "port", mPort);
    IHelper::adoptElementValue(elem, "type", IICETypes::toString(mCandidateType), false);
    IHelper::adoptElementValue(elem, "tcpType", IICETypes::toString(mTCPType), false);
    IHelper::adoptElementValue(elem, "relatedAddress", mRelatedAddress, false);
    IHelper::adoptElementValue(elem, "relatedPort", mRelatedPort);

    if (!elem->hasChildren()) return ElementPtr();
    
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETypes::Candidate::toDebug() const
  {
    return createElement("ortc::IICETypes::Candidate");
  }

  //---------------------------------------------------------------------------
  String IICETypes::Candidate::hash(bool includePriorities) const
  {
    auto hasher = IHasher::sha1();

    hasher->update("IICETypes::Candidate:");
    hasher->update(mInterfaceType);
    hasher->update(":");
    hasher->update(mFoundation);
    hasher->update(":");
    hasher->update(static_cast<std::underlying_type<decltype(mComponent)>::type>(mComponent));
    hasher->update(":");
    if (includePriorities) {
      hasher->update(mPriority);
      hasher->update(":");
      hasher->update(mUnfreezePriority);
      hasher->update(":");
    }
    hasher->update(toString(mProtocol));
    hasher->update(":");
    hasher->update(mIP);
    hasher->update(":");
    hasher->update(mPort);
    hasher->update(":");
    hasher->update(toString(mCandidateType));
    hasher->update(":");
    if (Protocol_TCP == mProtocol) {
      hasher->update(toString(mTCPType));
      hasher->update(":");
    }
    hasher->update(mRelatedAddress);
    hasher->update(":");
    hasher->update(mRelatedPort);

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  IPAddress IICETypes::Candidate::ip() const
  {
    if (mIP.isEmpty()) return IPAddress();
    IPAddress ip(mIP, mPort);
    return ip;
  }

  //---------------------------------------------------------------------------
  IPAddress IICETypes::Candidate::relatedIP() const
  {
    if (mRelatedAddress.isEmpty()) return IPAddress();
    IPAddress ip(mRelatedAddress, mRelatedPort);
    return ip;
  }

  //---------------------------------------------------------------------------
  String IICETypes::Candidate::foundation(
                                          const char *relatedServerURL,
                                          const char *baseIP
                                          ) const
  {
    if (mFoundation.hasData()) return mFoundation;

    auto hasher = IHasher::sha1();

    hasher->update("foundation:");
    hasher->update(IICETypes::toString(mCandidateType));
    hasher->update(":");
    switch (mCandidateType) {
      case CandidateType_Host:    hasher->update(mIP); break;
      case CandidateType_Prflx:   hasher->update(mIP); break;
      case CandidateType_Relay:   {
        if (baseIP) {
          if (0 != (*baseIP)) {
            hasher->update(baseIP);
            break;
          }
        }
        hasher->update(mRelatedAddress);
        break;
      }
      case CandidateType_Srflex:  hasher->update(mRelatedAddress); break;
    }
    hasher->update(":");
    hasher->update(IICETypes::toString(mProtocol));
    if (relatedServerURL) {
      if (0 != (*relatedServerURL)) {
        hasher->update(":");
        hasher->update(relatedServerURL);
      }
    }

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETypes::Candidate
  #pragma mark

  //---------------------------------------------------------------------------
  IICETypes::CandidateCompletePtr IICETypes::CandidateComplete::convert(GatherCandidatePtr candidate)
  {
    return ZS_DYNAMIC_PTR_CAST(CandidateComplete, candidate);
  }

  //---------------------------------------------------------------------------
  IICETypes::CandidateComplete::CandidateComplete(ElementPtr elem)
  {
    if (!elem) return;

    {
      String str = IHelper::getElementText(elem->findFirstChildElement("component"));
      if (str.hasData()) {
        try {
          std::underlying_type<decltype(mComponent)>::type converted = Numeric<std::underlying_type<decltype(mComponent)>::type>(str);
          IICETypes::Components component = static_cast<decltype(mComponent)>(converted);
          if ((component >= IICETypes::Component_First) ||
              (component <= IICETypes::Component_Last)) {
            mComponent = component;
          } else {
            ZS_LOG_WARNING(Debug, slog("component value invalid") + ZS_PARAM("value", str))
          }
        } catch (const Numeric<std::underlying_type<decltype(mComponent)>::type>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog("component value invalid") + ZS_PARAM("value", str))
        }
      }
    }

    IHelper::getElementValue(elem, "ortc::IICETypes::CandidateComplete", "complete", mComplete);
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETypes::CandidateComplete::createElement(const char *objectName) const
  {
    if (NULL == objectName) objectName = "candidateComplete";

    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "component", static_cast<std::underlying_type<decltype(mComponent)>::type>(mComponent));
    IHelper::adoptElementValue(elem, "complete", mComplete);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }
  
  //---------------------------------------------------------------------------
  ElementPtr IICETypes::CandidateComplete::toDebug() const
  {
    return createElement("ortc::IICETypes::CandidateComplete");
  }

  //---------------------------------------------------------------------------
  String IICETypes::CandidateComplete::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("IICETypes::CandidateComplete:");
    hasher->update(static_cast<std::underlying_type<decltype(mComponent)>::type>(mComponent));
    hasher->update(":");
    hasher->update(mComplete);

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETypes::Parameters
  #pragma mark

  //---------------------------------------------------------------------------
  IICETypes::Parameters::Parameters(ElementPtr elem)
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IICETypes::Parameters", "useUnfreezePriority", mUseUnfreezePriority);
    IHelper::getElementValue(elem, "ortc::IICETypes::Parameters", "usernameFragment", mUsernameFragment);
    IHelper::getElementValue(elem, "ortc::IICETypes::Parameters", "password", mPassword);
    IHelper::getElementValue(elem, "ortc::IICETypes::Parameters", "iceLite", mICELite);
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETypes::Parameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "useCandidateFreezePriority", mUseUnfreezePriority);
    IHelper::adoptElementValue(elem, "usernameFragment", mUsernameFragment, false);
    IHelper::adoptElementValue(elem, "password", mPassword, false);
    IHelper::adoptElementValue(elem, "iceLite", mICELite);

    if (!elem->hasChildren()) return ElementPtr();

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETypes::Parameters::toDebug() const
  {
    return createElement("ortc::IICETypes::Parameters");
  }

  //---------------------------------------------------------------------------
  String IICETypes::Parameters::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update(mUseUnfreezePriority ? "Parameters:true:" : "Parameters:false:");
    hasher->update(mUsernameFragment);
    hasher->update(":");
    hasher->update(mPassword);
    return hasher->finalizeAsString();
  }
}
