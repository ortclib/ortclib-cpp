/*

 Copyright (c) 2015, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/internal/ortc_IRTP.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/platform.h>

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_mediaengine) }

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTP
    #pragma mark

    //-------------------------------------------------------------------------
    const char *IRTP::toString(States state)
    {
      switch (state) {
        case State_Pending:       return "pending";
        case State_Ready:         return "ready";
        case State_ShuttingDown:  return "shutting down";
        case State_Shutdown:      return "shutdown";
      }
      return "UNDEFINED";    
    }
    //-------------------------------------------------------------------------
    IRTP::States IRTP::toState(const char *stateStr) throw (InvalidParameters)
    {
      String str(stateStr);
      for (IRTP::States index = IRTP::State_First; index <= IRTP::State_Last; index = static_cast<IRTP::States>(static_cast<std::underlying_type<IRTP::States>::type>(index) + 1)) {
        if (0 == str.compareNoCase(IRTP::toString(index))) return index;
      }

      ORTC_THROW_INVALID_PARAMETERS("Invalid parameter value: " + str)
      return IRTP::State_First;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPForMediaEngine
    #pragma mark

  } // namespace internal

}
