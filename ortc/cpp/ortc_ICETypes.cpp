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

#include <openpeer/services/IHelper.h>
//#include <openpeer/services/IHTTP.h>
//#include <openpeer/services/ISettings.h>
//
#include <zsLib/XML.h>
//#include <zsLib/Numeric.h>
//
//#include <regex>
//
#include <cryptopp/sha.h>


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
//  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)
//  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings)
//
//  using zsLib::Numeric;
  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETypes
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
  IICETypes::Roles IICETypes::toRole(const char *role)
  {
    static Roles roles[] = {
      Role_Controlling,
      Role_Controlled,
    };

    String rolesStr(role);
    for (size_t loop = 0; loop < (sizeof(roles) / sizeof(roles[0])); ++loop) {
      if (rolesStr == toString(roles[loop])) return roles[loop];
    }

    ZS_THROW_INVALID_ARGUMENT("Invalid parameter value: " + rolesStr)
    return Role_Controlling;
  }

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
  IICETypes::Components IICETypes::toComponent(const char *component)
  {
    static Components components[] = {
      Component_RTP,
      Component_RTCP,
    };

    String componentStr(component);
    for (size_t loop = 0; loop < (sizeof(components) / sizeof(components[0])); ++loop) {
      if (componentStr == toString(components[loop])) return components[loop];
    }

    ZS_THROW_INVALID_ARGUMENT("Invalid parameter value: " + componentStr)
    return Component_RTP;
  }

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
  IICETypes::Protocols IICETypes::toProtocol(const char *protocol)
  {
    static Protocols protocols[] = {
      Protocol_UDP,
      Protocol_TCP,
    };

    String protocolStr(protocol);
    for (size_t loop = 0; loop < (sizeof(protocols) / sizeof(protocols[0])); ++loop) {
      if (protocolStr == toString(protocols[loop])) return protocols[loop];
    }

    ZS_THROW_INVALID_ARGUMENT("Invalid parameter value: " + protocolStr)
    return Protocol_UDP;
  }

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
  IICETypes::CandidateTypes IICETypes::toCandidateType(const char *candidateType)
  {
    static CandidateTypes candidateTypes[] = {
      CandidateType_Host,
      CandidateType_Srflex,
      CandidateType_Prflx,
      CandidateType_Relay,
    };

    String candidateTypeStr(candidateType);
    for (size_t loop = 0; loop < (sizeof(candidateTypes) / sizeof(candidateTypes[0])); ++loop) {
      if (candidateTypeStr == toString(candidateTypes[loop])) return candidateTypes[loop];
    }

    ZS_THROW_INVALID_ARGUMENT("Invalid parameter value: " + candidateTypeStr)
    return CandidateType_Host;
  }

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
  IICETypes::TCPCandidateTypes IICETypes::toTCPCandidateType(const char *tcpCandidateType)
  {
    static TCPCandidateTypes tcpCandidateTypes[] = {
      TCPCandidateType_Active,
      TCPCandidateType_Passive,
      TCPCandidateType_SO
    };

    String tcpCandidateTypeStr(tcpCandidateType);
    for (size_t loop = 0; loop < (sizeof(tcpCandidateTypes) / sizeof(tcpCandidateTypes[0])); ++loop) {
      if (tcpCandidateTypeStr == toString(tcpCandidateTypes[loop])) return tcpCandidateTypes[loop];
    }

    ZS_THROW_INVALID_ARGUMENT("Invalid parameter value: " + tcpCandidateTypeStr)
    return TCPCandidateType_SO;
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETypes::Candidate::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IICETypes::Candidate");

    UseServicesHelper::debugAppend(resultEl, "interface type", mInterfaceType);
    UseServicesHelper::debugAppend(resultEl, "foundation", mFoundation);
    UseServicesHelper::debugAppend(resultEl, "priority", mPriority);
    UseServicesHelper::debugAppend(resultEl, "unfreeze priority", mUnfreezePriority);
    UseServicesHelper::debugAppend(resultEl, "protocol", toString(mProtocol));
    UseServicesHelper::debugAppend(resultEl, "ip", mIP);
    UseServicesHelper::debugAppend(resultEl, "port", mPort);
    UseServicesHelper::debugAppend(resultEl, "candidate type", toString(mCandidateType));
    if (Protocol_TCP == mProtocol) {
      UseServicesHelper::debugAppend(resultEl, "tcp type", toString(mTCPType));
    }
    UseServicesHelper::debugAppend(resultEl, "related address", mRelatedAddress);
    UseServicesHelper::debugAppend(resultEl, "related port", mRelatedPort);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IICETypes::Candidate::hash(bool includePriorities) const
  {
    SHA1Hasher hasher;

    hasher.update(mInterfaceType);
    hasher.update(":");
    hasher.update(mFoundation);
    hasher.update(":");
    if (includePriorities) {
      hasher.update(string(mPriority));
      hasher.update(":");
      hasher.update(string(mUnfreezePriority));
      hasher.update(":");
    }
    hasher.update(toString(mProtocol));
    hasher.update(":");
    hasher.update(mIP);
    hasher.update(":");
    hasher.update(string(mPort));
    hasher.update(":");
    hasher.update(toString(mCandidateType));
    hasher.update(":");
    if (Protocol_TCP == mProtocol) {
      hasher.update(toString(mTCPType));
      hasher.update(":");
    }
    hasher.update(mRelatedAddress);
    hasher.update(":");
    hasher.update(string(mRelatedPort));

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  ElementPtr IICETypes::Parameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IICETypes::Parameters");

    UseServicesHelper::debugAppend(resultEl, "use candidate freeze priority", mUseCandidateFreezePriority);
    UseServicesHelper::debugAppend(resultEl, "username fragment", mUsernameFragment);
    UseServicesHelper::debugAppend(resultEl, "password", mPassword);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IICETypes::Parameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update(mUseCandidateFreezePriority ? "Parameters:true:" : "Parameters:false:");
    hasher.update(mUsernameFragment);
    hasher.update(":");
    hasher.update(mPassword);
    return hasher.final();
  }
}
