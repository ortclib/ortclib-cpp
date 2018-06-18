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


#include <ortc/adapter/internal/types.h>
#include <ortc/adapter/ISessionDescription.h>

#include <ortc/adapter/ISessionDescription.h>

#include <ortc/adapter/internal/ortc_adapter_SDPParser.h>

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
      //
      // SessionDescription
      //

      class SessionDescription : public SharedRecursiveLock,
                                 public ISessionDescription
      {
      protected:
        struct make_private {};

        void init() noexcept;

      public:
        SessionDescription(
                           const make_private &,
                           SignalingTypes type,
                           const char *descriptionStr,
                           const Description *description
                           ) noexcept;

        static SessionDescriptionPtr convert(ISessionDescriptionPtr object) noexcept;

        //---------------------------------------------------------------------
        //
        // SessionDescription => ISessionDescription
        //

        static ElementPtr toDebug(ISessionDescriptionPtr object) noexcept;

        static SessionDescriptionPtr create(
                                            SignalingTypes type,
                                            const char *description
                                            ) noexcept;
        static SessionDescriptionPtr create(
                                            SignalingTypes type,
                                            const Description &description
                                            ) noexcept;

        PUID getID() const noexcept override { return mID; }

        SignalingTypes type() const noexcept override;
        DescriptionPtr description() const noexcept override;
        SignalingDescription formattedDescription() const noexcept override;

      protected:
        //---------------------------------------------------------------------
        //
        // SessionDescription => (internal)
        //

        virtual ElementPtr toDebug() const noexcept;

      protected:
        //---------------------------------------------------------------------
        //
        // SessionDescription => (data)
        //

        AutoPUID mID;
        SessionDescriptionWeakPtr mThisWeak;

        SignalingTypes mType {SignalingType_JSON};
        mutable bool mConverted {false};
        mutable DescriptionPtr mDescription;
        mutable SignalingDescription mFormattedString;
        mutable ISDPTypes::SDPPtr mSDP;
      };

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //
      // IMediaStreamFactory
      //

      interaction ISessionDescriptionFactory
      {
        typedef ISessionDescriptionTypes::SignalingTypes SignalingTypes;
        ZS_DECLARE_TYPEDEF_PTR(ISessionDescriptionTypes::Description, Description);

        static ISessionDescriptionFactory &singleton() noexcept;

        virtual SessionDescriptionPtr create(
                                             SignalingTypes type,
                                             const char *description
                                             ) noexcept;
        virtual SessionDescriptionPtr create(
                                             SignalingTypes type,
                                             const Description &description
                                             ) noexcept;
      };

      class SessionDescriptionFactory : public IFactory<ISessionDescriptionFactory> {};
    }
  }
}

