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
  // IDataChannelTypes
  //
  
  interaction IDataChannelTypes
  {
    ZS_DECLARE_STRUCT_PTR(Parameters);

    //-------------------------------------------------------------------------
    //
    // IDataChannelTypes::States
    //

    enum States {
      State_First,

      State_Connecting    = State_First,
      State_Open,
      State_Closing,
      State_Closed,

      State_Last          = State_Closed,
    };

    static const char *toString(States state) noexcept;
    static Optional<States> toState(const char *state) noexcept;

    //-------------------------------------------------------------------------
    //
    // IDataChannelTypes::Parameters
    //

    struct Parameters {
      String            mLabel;
      bool              mOrdered {true};
      Milliseconds      mMaxPacketLifetime {};
      Optional<DWORD>   mMaxRetransmits;
      String            mProtocol;
      bool              mNegotiated {false};
      Optional<USHORT>  mID;

      Parameters() noexcept {}
      Parameters(const Parameters &op2) noexcept {(*this) = op2;}
      Parameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };
  };
}
