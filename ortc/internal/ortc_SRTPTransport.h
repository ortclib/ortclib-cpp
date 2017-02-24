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

#pragma once

#include <ortc/internal/types.h>
#include <ortc/internal/ortc_ISRTPTransport.h>
#include <ortc/internal/ortc_ISecureTransport.h>

#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/ITimer.h>

// Forward declaration to avoid pulling in libsrtp headers here
struct srtp_event_data_t;
struct srtp_ctx_t;
struct srtp_policy_t;

//#define ORTC_SETTING_SRTP_TRANSPORT_WARN_OF_KEY_LIFETIME_EXHAUGSTION_WHEN_REACH_PERCENTAGE_USSED "ortc/srtp/warm-key-lifetime-exhaustion-when-reach-percentage-used"

#pragma warning(push)
#pragma warning(disable:4351)

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(SRTPInit);

    ZS_DECLARE_INTERACTION_PTR(ISecureTransportForSRTPTransport);

    ZS_DECLARE_INTERACTION_PTR(ISRTPTransportForSecureTransport);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISRTPTransportForSettings
    #pragma mark

    interaction ISRTPTransportForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(ISRTPTransportForSettings, ForSettings)

      static void applyDefaults();

      virtual ~ISRTPTransportForSettings() {}
    };
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISRTPTransportForSecureTransport
    #pragma mark

    interaction ISRTPTransportForSecureTransport
    {
      ZS_DECLARE_TYPEDEF_PTR(ISRTPTransportForSecureTransport, ForSecureTransport)

      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForSRTPTransport, UseSecureTransport)
      ZS_DECLARE_TYPEDEF_PTR(ISRTPSDESTransport::Parameters, Parameters)
      ZS_DECLARE_TYPEDEF_PTR(ISRTPSDESTransport::CryptoParameters, CryptoParameters)
      ZS_DECLARE_TYPEDEF_PTR(ISRTPSDESTransport::KeyParameters, KeyParameters)

      static ElementPtr toDebug(ForSecureTransportPtr transport);

      static ParametersPtr getLocalParameters();

      static ForSecureTransportPtr create(
                                          ISRTPTransportDelegatePtr delegate,
                                          UseSecureTransportPtr transport,
                                          const CryptoParameters &encryptParameters,
                                          const CryptoParameters &decryptParameters
                                          );

      virtual PUID getID() const = 0;

      virtual ISRTPTransportSubscriptionPtr subscribe(ISRTPTransportDelegatePtr delegate) = 0;

      virtual bool handleReceivedPacket(
                                        IICETypes::Components viaTransport,
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        ) = 0;

      virtual bool sendPacket(
                              IICETypes::Components sendOverICETransport,
                              IICETypes::Components component,
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPTransport
    #pragma mark
    
    class SRTPTransport : public Noop,
                          public MessageQueueAssociator,
                          public SharedRecursiveLock,
                          public ISRTPTransport,
                          public ISRTPTransportForSecureTransport,
                          public IWakeDelegate,
                          public zsLib::ITimerDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction ISRTPTransport;
      friend interaction ISRTPTransportFactory;
      friend interaction ISRTPTransportForSecureTransport;

      ZS_DECLARE_STRUCT_PTR(KeyingMaterial)
      ZS_DECLARE_STRUCT_PTR(DirectionMaterial)

      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForSRTPTransport, UseSecureTransport)
      ZS_DECLARE_TYPEDEF_PTR(ISRTPSDESTransport::Parameters, Parameters)
      ZS_DECLARE_TYPEDEF_PTR(ISRTPSDESTransport::CryptoParameters, CryptoParameters)
      ZS_DECLARE_TYPEDEF_PTR(ISRTPSDESTransport::KeyParameters, KeyParameters)

      ZS_DECLARE_TYPEDEF_PTR(SecureByteBlock, MKIValue)

      struct MKIValueCompare
      {
        bool operator() (const SecureByteBlockPtr &op1, const SecureByteBlockPtr &op2) const;
      };

      typedef std::map<MKIValuePtr, KeyingMaterialPtr, MKIValueCompare> KeyMap;
      typedef std::list<KeyingMaterialPtr> KeyList;

      enum Directions
      {
        Direction_First,

        Direction_Encrypt = Direction_First,
        Direction_Decrypt,

        Direction_Last = Direction_Decrypt
      };
      static const char *toString(Directions state);

    public:
      SRTPTransport(
                    const make_private &,
                    IMessageQueuePtr queue,
                    ISRTPTransportDelegatePtr delegate,
                    UseSecureTransportPtr secureTransport,
                    const CryptoParameters &encryptParameters,
                    const CryptoParameters &decryptParameters
                    ) throw(InvalidParameters);

    protected:
      SRTPTransport(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~SRTPTransport();

      static SRTPTransportPtr convert(ISRTPTransportPtr object);
      static SRTPTransportPtr convert(ForSecureTransportPtr object);

    protected:

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPTransport => ISRTPTransport
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPTransport => ISRTPTransportForSecureTransport
      #pragma mark

      static ElementPtr toDebug(SRTPTransportPtr transport);

      static SRTPTransportPtr create(
                                     ISRTPTransportDelegatePtr delegate,
                                     UseSecureTransportPtr transport,
                                     const CryptoParameters &encryptParameters,
                                     const CryptoParameters &decryptParameters
                                     ) throw(InvalidParameters);

      virtual PUID getID() const override {return mID;}

      virtual ISRTPTransportSubscriptionPtr subscribe(ISRTPTransportDelegatePtr delegate) override;

      virtual bool handleReceivedPacket(
                                        IICETypes::Components viaTransport,
                                        const BYTE *buffer,
                                        size_t bufferLengthInBytes
                                        ) override;

      virtual bool sendPacket(
                              IICETypes::Components sendOverICETransport,
                              IICETypes::Components component,
                              const BYTE *buffer,
                              size_t bufferLengthInBytes
                              ) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPTransport => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPTransport => ITimerDelegate
      #pragma mark

      virtual void onTimer(ITimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPTransport => ISRTPTransportAsyncDelegate
      #pragma mark


    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPTransport => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      static Log::Params slog(const char *message);
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      void cancel();

      void updateTotalPackets(
                              Directions direction,
                              IICETypes::Components component,
                              KeyingMaterialPtr &keyingMaterial
                              );

      static size_t parseLifetime(const String &lifetime) throw(InvalidParameters);

      static SecureByteBlockPtr convertIntegerToBigEndianEncodedBuffer(
                                                                       const String &base10Value,
                                                                       size_t maxByteLength
                                                                       ) throw(InvalidParameters);

    public:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPTransport::SRTPSession
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPTransport::KeyingMaterial
      #pragma mark

      struct KeyingMaterial
      {
        KeyParameters mOriginalValues;
        SecureByteBlockPtr mMKIValue;

        size_t mLifetime {};
        size_t mTotalPackets[IICETypes::Component_Last+1] {};

        SecureByteBlockPtr mKeySalt;  // key and salt

        //libSRTP Session material
        Lock mSRTPSessionLock;
        srtp_ctx_t* mSRTPSession {};

        // E.g. (converted into proper useable format by crypto routines)

        ElementPtr toDebug() const;
        String hash() const;
      };

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPTransport::DirectionMaterial
      #pragma mark

      struct DirectionMaterial
      {
        size_t mAuthenticationTagLength[IICETypes::Component_Last+1] {};

        size_t mMKILength {};
        MKIValuePtr mTempMKIHolder;

        KeyList mKeyList;         // keys in order they are specified

        KeyMap mKeys;             // when MKI length > 0, lookup map based on MKI

        KeyingMaterialPtr mOldKey;

        size_t mTotalPackets[IICETypes::Component_Last+1] {};
        size_t mMaxTotalLifetime[IICETypes::Component_Last+1] {};

        ElementPtr toDebug() const;
        String hash() const;
      };

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark SRTPTransport => (data)
      #pragma mark

      AutoPUID mID;
      SRTPTransportWeakPtr mThisWeak;

      ISRTPTransportDelegateSubscriptions mSubscriptions;
      ISRTPTransportSubscriptionPtr mDefaultSubscription;

      UseSecureTransportWeakPtr mSecureTransport; // either DTLS or SDES transport

      CryptoParameters mParams[Direction_Last+1];

      ULONG mLastRemainingLeastKeyPercentageReported {100};
      ULONG mLastRemainingOverallPercentageReported {100};

      DirectionMaterial mMaterial[Direction_Last+1];

      SRTPInitPtr mSRTPInit;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISRTPTransportFactory
    #pragma mark

    interaction ISRTPTransportFactory
    {
      ZS_DECLARE_TYPEDEF_PTR(ISecureTransportForSRTPTransport, UseSecureTransport)
      ZS_DECLARE_TYPEDEF_PTR(ISRTPSDESTransport::CryptoParameters, CryptoParameters)

      static ISRTPTransportFactory &singleton();

      virtual SRTPTransportPtr create(
                                      ISRTPTransportDelegatePtr delegate,
                                      UseSecureTransportPtr transport,
                                      const CryptoParameters &encryptParameters,
                                      const CryptoParameters &decryptParameters
                                      );
    };

    class SRTPTransportFactory : public IFactory<ISRTPTransportFactory> {};
  }
}

#pragma warning(pop)
