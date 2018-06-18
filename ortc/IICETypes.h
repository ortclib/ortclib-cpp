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
  //
  // IICETypes
  //
  
  interaction IICETypes
  {
    ZS_DECLARE_STRUCT_PTR(GatherCandidate);
    ZS_DECLARE_STRUCT_PTR(Candidate);
    ZS_DECLARE_STRUCT_PTR(CandidateComplete);
    ZS_DECLARE_TYPEDEF_PTR(std::list<Candidate>, CandidateList);
    ZS_DECLARE_STRUCT_PTR(Parameters);

    //-------------------------------------------------------------------------
    //
    // IICETypes::Roles
    //

    enum Roles
    {
      Role_First,

      Role_Controlling    = Role_First,
      Role_Controlled,

      Role_Last           = Role_Controlled,
    };

    static const char *toString(Roles role) noexcept;
    static Roles toRole(const char *role) noexcept(false); // throws InvalidParameters

    //-------------------------------------------------------------------------
    //
    // IICETypes::Components
    //

    enum Components
    {
      Component_First = 1,

      Component_RTP = Component_First,
      Component_RTCP,

      Component_Last = Component_RTCP
    };

    static const char *toString(Components component) noexcept;
    static Components toComponent(const char *component) noexcept(false); // throws InvalidParameters

    //-------------------------------------------------------------------------
    //
    // IICETypes::Protocols
    //

    enum Protocols
    {
      Protocol_First,

      Protocol_UDP = Protocol_First,
      Protocol_TCP,

      Protocol_Last = Protocol_TCP
    };

    static const char *toString(Protocols protocol) noexcept;
    static Protocols toProtocol(const char *protocol) noexcept(false); // throws InvalidParameters

    //-------------------------------------------------------------------------
    //
    // IICETypes::CandidateTypes
    //

    enum CandidateTypes
    {
      CandidateType_First,

      CandidateType_Host = CandidateType_First,
      CandidateType_Srflex,
      CandidateType_Prflx,
      CandidateType_Relay,

      CandidateType_Last = CandidateType_Relay
    };

    static const char *toString(CandidateTypes type) noexcept;
    static CandidateTypes toCandidateType(const char *type) noexcept(false); // throws InvalidParameters

    //-------------------------------------------------------------------------
    //
    // IICETypes::TCPCandidateTypes
    //

    enum TCPCandidateTypes
    {
      TCPCandidateType_First,

      TCPCandidateType_Active = TCPCandidateType_First,
      TCPCandidateType_Passive,
      TCPCandidateType_SO,

      TCPCandidateType_Last = TCPCandidateType_SO
    };

    static const char *toString(TCPCandidateTypes type) noexcept;
    static TCPCandidateTypes toTCPCandidateType(const char *type) noexcept(false); // throws InvalidParameters

    //-------------------------------------------------------------------------
    struct GatherCandidate
    {
      Components  mComponent {Component_RTP};

      static GatherCandidatePtr create(ElementPtr elem) noexcept;

      virtual ElementPtr createElement(const char *objectName = NULL) const noexcept = 0;

      virtual ~GatherCandidate() noexcept {} // make polymorphic

    protected:
      GatherCandidate() noexcept {}
      GatherCandidate(const GatherCandidate &op2) noexcept :
        mComponent(op2.mComponent)
      {}
    };

    //-------------------------------------------------------------------------
    //
    // IICETypes::Components
    //

    struct Candidate : public GatherCandidate {
      String            mInterfaceType;
      String            mFoundation;
      DWORD             mPriority {};
      DWORD             mUnfreezePriority {};
      Protocols         mProtocol {Protocol_UDP};
      String            mIP;
      WORD              mPort {};
      CandidateTypes    mCandidateType {CandidateType_Host};
      TCPCandidateTypes mTCPType {TCPCandidateType_Active};
      String            mRelatedAddress;
      WORD              mRelatedPort {};

      static CandidatePtr convert(GatherCandidatePtr candidate) noexcept;

      Candidate() noexcept;
      Candidate(const Candidate &op2) noexcept;
      Candidate(ElementPtr elem) noexcept;
      ~Candidate() noexcept;

      ElementPtr createElement(const char *objectName = "candidate") const noexcept override;

      ElementPtr toDebug() const noexcept;
      String hash(bool includePriorities = true) const noexcept;

      IPAddress ip() const noexcept;
      IPAddress relatedIP() const noexcept;
      String foundation(
                        const char *relatedServerURL = NULL,
                        const char *baseIP = NULL
                        ) const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IICETypes::Components
    //

    struct CandidateComplete : public GatherCandidate {
      bool        mComplete {true};

      static CandidateCompletePtr convert(GatherCandidatePtr candidate) noexcept;

      CandidateComplete() noexcept {}
      CandidateComplete(const CandidateComplete &op2) noexcept {(*this) = op2;}
      CandidateComplete(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName = "candidateComplete") const noexcept override;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IICETypes::Parameters
    //

    struct Parameters {
      bool mUseUnfreezePriority {false};
      String mUsernameFragment;
      String mPassword;
      bool mICELite {false};

      Parameters() noexcept {}
      Parameters(const Parameters &op2) noexcept {(*this) = op2;}
      Parameters(ElementPtr rootEl) noexcept;

      static ParametersPtr create(ElementPtr rootEl) noexcept { if (!rootEl) return ParametersPtr(); return make_shared<Parameters>(rootEl); }
      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };
  };
}
