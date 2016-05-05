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

//#include <openpeer/services/IHelper.h>
//
#include <zsLib/Log.h>
//#include <zsLib/Numeric.h>
//#include <zsLib/Stringize.h>
//#include <zsLib/XML.h>


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib); }

namespace ortc
{
  namespace adapter
  {
    namespace internal
    {
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SessionDescription
      #pragma mark

    }  // namespace internal

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISessionDescription
    #pragma mark

    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::ICECandidatePtr ISessionDescriptionTypes::ICECandidate::create(ElementPtr candidateEl)
    {
      return ICECandidatePtr();
    }
    //-------------------------------------------------------------------------
    ISessionDescriptionTypes::ICECandidatePtr ISessionDescriptionTypes::ICECandidate::createFromSDP(const char *string)
    {
      return ICECandidatePtr();
    }

    //-------------------------------------------------------------------------
    String ISessionDescriptionTypes::ICECandidate::toSDP() const
    {
      return String();
    }

    //-------------------------------------------------------------------------
    ElementPtr ISessionDescriptionTypes::ICECandidate::toJSON() const
    {
      return ElementPtr();
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionPtr ISessionDescription::create(
                                                       SignalingTypes type,
                                                       const char *description
                                                       )
    {
      return ISessionDescriptionPtr();
    }

    //-------------------------------------------------------------------------
    ISessionDescriptionPtr ISessionDescription::create(
                                                       SignalingTypes type,
                                                       const Description &description
                                                       )
    {
      return ISessionDescriptionPtr();
    }

  } // namespace adapter
} // namespace ortc
