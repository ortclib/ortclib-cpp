/*

 Copyright (c) 2017, Optical Tone Ltd.
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

#include <ortc/internal/types.h>
#include <ortc/internal/ortc_RTPTypes.h>
#include <ortc/internal/ortc_IRTP.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IRTPForMediaEngine);

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTP
    #pragma mark

    interaction IRTP
    {
      typedef PUID RTPObjectID;

      enum States
      {
        State_First,

        State_Pending = State_First,
        State_Ready,
        State_ShuttingDown,
        State_Shutdown,

        State_Last = State_Shutdown,
      };

      static const char *toString(States state);
      static States toState(const char *stateStr) throw (InvalidParameters);

      virtual RTPObjectID getID() const = 0;
      virtual void cancel() = 0;

      virtual States getState() const = 0;

      virtual ~IRTP() {}
    };

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPForMediaEngine
    #pragma mark

    interaction IRTPForMediaEngine
    {
      ZS_DECLARE_TYPEDEF_PTR(IRTPForMediaEngine, ForMediaEngine);

      typedef IRTP::RTPObjectID RTPObjectID;
      ZS_DECLARE_TYPEDEF_PTR(IRTP::States, States);

      virtual RTPObjectID getID() const = 0;
      virtual void shutdown() = 0;

      virtual States getState() const = 0;
      bool isShuttingDown() const { return IRTP::State_ShuttingDown == getState(); }
      bool isShutdown() const     { return IRTP::State_Shutdown == getState(); }

      virtual ~IRTPForMediaEngine() {}
    };

  }
}
