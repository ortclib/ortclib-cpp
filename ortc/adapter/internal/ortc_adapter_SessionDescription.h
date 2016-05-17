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
      #pragma mark
      #pragma mark SessionDescription
      #pragma mark

      class SessionDescription : public SharedRecursiveLock,
                                 public ISessionDescription
      {
      protected:
        struct make_private {};

        void init();

      public:
        SessionDescription(
                           const make_private &,
                           SignalingTypes type,
                           const char *descriptionStr,
                           const Description *description
                           );

        static SessionDescriptionPtr convert(ISessionDescriptionPtr object);

        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark SessionDescription => ISessionDescription
        #pragma mark

        static ElementPtr toDebug(ISessionDescriptionPtr object);

        static SessionDescriptionPtr create(
                                            SignalingTypes type,
                                            const char *description
                                            );
        static SessionDescriptionPtr create(
                                            SignalingTypes type,
                                            const Description &description
                                            );

        virtual PUID getID() const override { return mID; }

        virtual SignalingTypes type() const override;
        virtual DescriptionPtr description() const override;
        virtual SignalingDescription formattedDescription() const override;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark SessionDescription => (internal)
        #pragma mark

        virtual ElementPtr toDebug() const;

      protected:
        //---------------------------------------------------------------------
        #pragma mark
        #pragma mark SessionDescription => (data)
        #pragma mark

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
      #pragma mark
      #pragma mark IMediaStreamFactory
      #pragma mark

      interaction ISessionDescriptionFactory
      {
        typedef ISessionDescriptionTypes::SignalingTypes SignalingTypes;
        ZS_DECLARE_TYPEDEF_PTR(ISessionDescriptionTypes::Description, Description);

        static ISessionDescriptionFactory &singleton();

        virtual SessionDescriptionPtr create(
                                             SignalingTypes type,
                                             const char *description
                                             );
        virtual SessionDescriptionPtr create(
                                             SignalingTypes type,
                                             const Description &description
                                             );
      };

      class SessionDescriptionFactory : public IFactory<ISessionDescriptionFactory> {};
    }
  }
}

