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

#include <ortc/types.h>

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETypes
  #pragma mark
  
  interaction IICETypes
  {
    ZS_DECLARE_STRUCT_PTR(Candidate)
    ZS_DECLARE_TYPEDEF_PTR(std::list<Candidate>, CandidateList)
    ZS_DECLARE_STRUCT_PTR(Parameters)
    ZS_DECLARE_TYPEDEF_PTR(Candidate, GatherCandidate)

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::Roles
    #pragma mark

    enum Roles
    {
      Role_Controlling,
      Role_Controlled,
    };

    static const char *toString(Roles role);
    static Roles toRole(const char *role);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::Components
    #pragma mark

    enum Components
    {
      Component_RTP,
      Component_RTCP,
    };

    static const char *toString(Components component);
    static Components toComponent(const char *component);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::Protocols
    #pragma mark

    enum Protocols
    {
      Protocol_UDP,
      Protocol_TCP,
    };

    static const char *toString(Protocols protocol);
    static Protocols toProtocol(const char *protocol);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::CandidateTypes
    #pragma mark

    enum CandidateTypes
    {
      CandidateType_Host,
      CandidateType_Srflex,
      CandidateType_Prflx,
      CandidateType_Relay,
    };

    static const char *toString(CandidateTypes type);
    static CandidateTypes toCandidateType(const char *type);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::TCPCandidateTypes
    #pragma mark

    enum TCPCandidateTypes
    {
      TCPCandidateType_Active,
      TCPCandidateType_Passive,
      TCPCandidateType_SO,
    };

    static const char *toString(TCPCandidateTypes type);
    static TCPCandidateTypes toTCPCandidateType(const char *type);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::Components
    #pragma mark

    struct Candidate {
      String            mFoundation;
      DWORD             mPriority {};
      Protocols         mProtocol {Protocol_UDP};
      String            mIP;
      WORD              mPort {};
      CandidateTypes    mCandidateType {CandidateType_Host};
      TCPCandidateTypes mTCPType {TCPCandidateType_SO};
      String            mRelatedAddress;
      WORD              mRelatedPort {};
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETypes::Parameters
    #pragma mark

    struct Parameters {
      String mUsernameFragment;
      String mPassword;
    };
  };
}
