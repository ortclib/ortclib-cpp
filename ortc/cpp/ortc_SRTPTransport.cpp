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

#include <ortc/internal/ortc_SRTPTransport.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/platform.h>

#include <openpeer/services/ISettings.h>
#include <openpeer/services/IHelper.h>
#include <openpeer/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/Numeric.h>
#include <zsLib/XML.h>

#include <cryptopp/integer.h>
#include <cryptopp/sha.h>

#ifdef HAVE_TGMATH_H
#include <tgmath.h>
#else
#include <math.h>
#endif //HAVE_TGMATH_H

#ifdef _DEBUG
#define ASSERT(x) ZS_THROW_BAD_STATE_IF(!(x))
#else
#define ASSERT(x)
#endif //_DEBUG

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

#define ORTC_SRTPTRANSPORT_ILLEGAL_MKI_LEGNTH (0xFFFF)


namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  typedef CryptoPP::Integer Integer;

  using zsLib::Numeric;

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportForSettings
    #pragma mark

    //-------------------------------------------------------------------------
    void ISRTPTransportForSettings::applyDefaults()
    {
      UseSettings::setUInt(ORTC_SETTING_SRTP_TRANSPORT_WARN_OF_KEY_LIFETIME_EXHAUGSTION_WHEN_REACH_PERCENTAGE_USSED, 90);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISRTPTransportForSecureTransport
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr ISRTPTransportForSecureTransport::toDebug(ForSecureTransportPtr transport)
    {
      if (!transport) return ElementPtr();
      return ZS_DYNAMIC_PTR_CAST(SRTPTransport, transport)->toDebug();
    }

    //---------------------------------------------------------------------------
    ISRTPTransportForSecureTransport::ForSecureTransportPtr ISRTPTransportForSecureTransport::create(
                                                                                                     ISRTPTransportDelegatePtr delegate,
                                                                                                     UseSecureTransportPtr transport,
                                                                                                     const CryptoParameters &encryptParameters,
                                                                                                     const CryptoParameters &decryptParameters
                                                                                                     )
    {
      return internal::ISRTPTransportFactory::singleton().create(delegate, transport, encryptParameters, decryptParameters);
    }
    

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPTransport
    #pragma mark
    
    //---------------------------------------------------------------------------
    bool SRTPTransport::MKIValueCompare::operator() (const SecureByteBlockPtr &op1, const SecureByteBlockPtr &op2) const
    {
      if (!op1) {
        if (!op2) return false;
        return true;
      } else if (!op2) return false;

      int compare = UseServicesHelper::compare(*op1, *op2);
      return compare < 0;
    }

    //---------------------------------------------------------------------------
    const char *SRTPTransport::toString(Directions state)
    {
      switch (state) {
        case Direction_Encrypt:  return "encrypt";
        case Direction_Decrypt:  return "decrypt";
      }
      return "UNDEFINED";
    }

    //-------------------------------------------------------------------------
    SRTPTransport::SRTPTransport(
                                 const make_private &,
                                 IMessageQueuePtr queue,
                                 ISRTPTransportDelegatePtr originalDelegate,
                                 UseSecureTransportPtr secureTransport,
                                 const CryptoParameters &encryptParameters,
                                 const CryptoParameters &decryptParameters
                                 ) throw(InvalidParameters) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create()),
      mSecureTransport(secureTransport)
    {
      ZS_LOG_DETAIL(debug("created"))

      mParams[Direction_Encrypt] = encryptParameters;
      mParams[Direction_Decrypt] = decryptParameters;

      if (originalDelegate) {
        mDefaultSubscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueORTC()); // using ORTC queue and not delegate queue since this is an internal only class
      }

      for (size_t loop = Direction_First; loop <= Direction_Last; ++loop) {
        size_t mkiLength = ORTC_SRTPTRANSPORT_ILLEGAL_MKI_LEGNTH;

        for (auto iter = mParams[loop].mKeyParams.begin(); iter != mParams[loop].mKeyParams.end(); ++iter) {
          auto keyParam = (*iter);
          if (ORTC_SRTPTRANSPORT_ILLEGAL_MKI_LEGNTH != mkiLength) {
            ORTC_THROW_INVALID_PARAMETERS_IF(mkiLength != keyParam.mMKILength)  // must ALL be the same size
          } else {
            mkiLength = keyParam.mMKILength;
          }

          KeyingMaterialPtr keyingMaterial(make_shared<KeyingMaterial>());

          ORTC_THROW_INVALID_PARAMETERS_IF((keyParam.mMKIValue.hasData()) && (0 == mkiLength))

          keyingMaterial->mOriginalValues = keyParam;
          keyingMaterial->mLifetime = parseLifetime(keyParam.mLifetime);

#define TODO_EXTRACT_AND_FILL_IN_OTHER_KEYING_MATERIAL_VALUES 1
#define TODO_EXTRACT_AND_FILL_IN_OTHER_KEYING_MATERIAL_VALUES 2

          if (0 != mkiLength) {
            keyingMaterial->mMKIValue = convertIntegerToBigEndianEncodedBuffer(keyParam.mMKIValue, mkiLength);
            mMaterial[loop].mKeys[keyingMaterial->mMKIValue] = keyingMaterial;
          } else {
            ASSERT(!((bool)mMaterial[loop].mKey)) // not possible
            mMaterial[loop].mKey = keyingMaterial;
          }

          if (Direction_Encrypt == loop) {
            mMaterial[loop].mKeyList.push_back(keyingMaterial); // when encrypting order matters so need all keys in a list
            mMaxTotalLifetime += keyingMaterial->mLifetime;
          } else {
            mMaxTotalLifetime = (0 == mMaxTotalLifetime ? keyingMaterial->mLifetime : (mMaxTotalLifetime > keyingMaterial->mLifetime ? keyingMaterial->mLifetime : mMaxTotalLifetime));
          }
        }

        ORTC_THROW_INVALID_PARAMETERS_IF((mMaterial[loop].mKeys.size() > 1) && (0 == mkiLength))
        ORTC_THROW_INVALID_PARAMETERS_IF((mMaterial[loop].mKeys.size() < 1) && (0 != mkiLength))

        ORTC_THROW_INVALID_PARAMETERS_IF((!((bool)mMaterial[loop].mKey)) && (0 == mkiLength))
        ORTC_THROW_INVALID_PARAMETERS_IF(((bool)mMaterial[loop].mKey) && (0 != mkiLength))

        mMaterial[loop].mMKILength = mkiLength;
        mMaterial[loop].mTempMKIHolder = SecureByteBlockPtr(make_shared<SecureByteBlock>(mkiLength));
      }
    }

    //-------------------------------------------------------------------------
    void SRTPTransport::init()
    {
      AutoRecursiveLock lock(*this);
      IWakeDelegateProxy::create(mThisWeak.lock())->onWake();
    }

    //-------------------------------------------------------------------------
    SRTPTransport::~SRTPTransport()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();

      cancel();
    }

    //-------------------------------------------------------------------------
    SRTPTransportPtr SRTPTransport::convert(ISRTPTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SRTPTransport, object);
    }

    //-------------------------------------------------------------------------
    SRTPTransportPtr SRTPTransport::convert(ForSettingsPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SRTPTransport, object);
    }

    //-------------------------------------------------------------------------
    SRTPTransportPtr SRTPTransport::convert(ForSecureTransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(SRTPTransport, object);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPTransport => ISRTPTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPTransport => ISRTPTransportForSecureTransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr SRTPTransport::toDebug(SRTPTransportPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    SRTPTransportPtr SRTPTransport::create(
                                           ISRTPTransportDelegatePtr delegate,
                                           UseSecureTransportPtr transport,
                                           const CryptoParameters &encryptParameters,
                                           const CryptoParameters &decryptParameters
                                           ) throw(InvalidParameters)
    {
      SRTPTransportPtr pThis(make_shared<SRTPTransport>(make_private {}, IORTCForInternal::queueORTC(), delegate, transport, encryptParameters, decryptParameters));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    ISRTPTransportSubscriptionPtr SRTPTransport::subscribe(ISRTPTransportDelegatePtr originalDelegate)
    {
      ZS_LOG_DETAIL(log("subscribing to transport state"))

      AutoRecursiveLock lock(*this);
      if (!originalDelegate) return mDefaultSubscription;

      ISRTPTransportSubscriptionPtr subscription = mSubscriptions.subscribe(originalDelegate, IORTCForInternal::queueDelegate());

      ISRTPTransportDelegatePtr delegate = mSubscriptions.delegate(subscription, true);

      if (delegate) {
        SRTPTransportPtr pThis = mThisWeak.lock();

        if (100 != mLastRemainingPercentageReported) {
          delegate->onSRTPTransportLifetimeRemaining(pThis, mLastRemainingPercentageReported);
        }

#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 1
#define TODO_DO_WE_NEED_TO_TELL_ABOUT_ANY_MISSED_EVENTS 2
      }

      return subscription;
    }

    //-------------------------------------------------------------------------
    bool SRTPTransport::handleReceivedPacket(
                                             const BYTE *buffer,
                                             size_t bufferLengthInBytes
                                             )
    {
      UseSecureTransportPtr transport;
      SecureByteBlockPtr encryptedBuffer;
      IICETypes::Components component = IICETypes::Component_RTP;

      KeyingMaterialPtr keyingMaterial; // WARNING: do NOT modify contents of what pointer is pointing to outside of a lock (shouldn't need to change contents anyway)

      const BYTE *pPacketMKI {NULL};

#define TODO_SET_pPacketMKI_TO_POINT_TO_MKI_VALUE_INSIDE_PACKET_IF_APPLICABLE 1
#define TODO_SET_pPacketMKI_TO_POINT_TO_MKI_VALUE_INSIDE_PACKET_IF_APPLICABLE 2
      // NOTE: *** WARNING ***
      // DO NOT TRUST THE INCOMING PACKET. Assume every size, index and
      // value inside the incoming packet is malicious. Thus double check
      // indexes, positions, length are within range of the packet BEFORE
      // extracting or continuing. If anything looks wrong then immediately
      // log a warning and abort out of the decoding process IMMEDIATELY.

      {
        AutoRecursiveLock lock(*this);

        if (0 == mLastRemainingPercentageReported) {
          ZS_LOG_WARNING(Detail, log("cannot encrypt packet as packet lifetime is exhausted"))
          return false;
        }

        transport = mSecureTransport.lock();
        if (!transport) {
          ZS_LOG_WARNING(Debug, log("nowhere to send packet as secure transport is gone"))
          return false;
        }

        if (0 == mLastRemainingPercentageReported) {
          ZS_LOG_WARNING(Detail, log("cannot decrypt packet as packet lifetime is exhausted (and continuing to decrypt would violate security principles)"))
          return false;
        }

        DirectionMaterial &material = mMaterial[Direction_Decrypt];

        if (0 != material.mMKILength) {
          if (NULL == pPacketMKI) {
            ZS_LOG_WARNING(Debug, log("packet mki value was not present (thus aborting decryption)") + ZS_PARAM("buffer length in bytes", bufferLengthInBytes))
            return false;
          }

          ASSERT(((bool)material.mTempMKIHolder)) // must be present
          ASSERT(material.mMKILength == material.mTempMKIHolder->SizeInBytes()) // must be identical

#define WARNING_DOUBLE_CHECK_THAT_THE_MKI_VALUE_SOURCE_POINTER_FROM_PACKET_PLUS_LENGTH_OF_MKI_WOULD_NOT_EXCEED_TOTAL_PACKET_LENGTH 1
#define WARNING_DOUBLE_CHECK_THAT_THE_MKI_VALUE_SOURCE_POINTER_FROM_PACKET_PLUS_LENGTH_OF_MKI_WOULD_NOT_EXCEED_TOTAL_PACKET_LENGTH 2

          memcpy(material.mTempMKIHolder->BytePtr(), pPacketMKI, material.mTempMKIHolder->SizeInBytes());

          auto found = material.mKeys.find(material.mTempMKIHolder);
          if (found == material.mKeys.end()) {
            ZS_LOG_WARNING(Debug, log("no key was found with packet's MKI value") + ZS_PARAM("mki value", UseServicesHelper::convertToHex(*(material.mTempMKIHolder))))
            return false;
          }

          keyingMaterial = (*found).second;
        } else {
          keyingMaterial = material.mKey;
        }


        ASSERT(((bool)keyingMaterial))

        if (!keyingMaterial) {
          ZS_LOG_WARNING(Debug, log("no keying material found to decrypt packet") + ZS_PARAM("buffer length in bytes", bufferLengthInBytes))
          return false;
        }

#define TODO_FIGURE_OUT_IF_THIS_IS_AN_RTP_PACKET_OR_RTCP_PACKET_AND_SET_component 1
#define TODO_FIGURE_OUT_IF_THIS_IS_AN_RTP_PACKET_OR_RTCP_PACKET_AND_SET_component 2

        if (keyingMaterial->mTotalPackets[component] + 1 > keyingMaterial->mLifetime) {
          ZS_LOG_WARNING(Debug, log("cannot use keying material as it's lifetime is exhausted") + keyingMaterial->toDebug())
          return false;
        }

        ++(keyingMaterial->mTotalPackets[component]);
        updateTotalPackets(Direction_Decrypt, component);
      }

      ASSERT(((bool)transport))

#define WARNING_IF_POSSIBLE_PERFORM_DECRYPTION_OUTSIDE_OF_OBJECT_LOCK 1
#define WARNING_IF_POSSIBLE_PERFORM_DECRYPTION_OUTSIDE_OF_OBJECT_LOCK 2

#define WARNING_SHOULD_NOT_REACH_HERE_UNLESS_encryptedBuffer_IS_VALID 1
#define WARNING_SHOULD_NOT_REACH_HERE_UNLESS_encryptedBuffer_IS_VALID 2

      ASSERT(((bool)encryptedBuffer))

      return transport->handleReceivedPacket(component, encryptedBuffer->BytePtr(), encryptedBuffer->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    bool SRTPTransport::sendPacket(
                                   IICETypes::Components component,
                                   const BYTE *buffer,
                                   size_t bufferLengthInBytes
                                   )
    {
      UseSecureTransportPtr transport;
      KeyingMaterialPtr keyingMaterial;

      SecureByteBlockPtr encryptedBuffer;

      {
        AutoRecursiveLock lock(*this);

        if (0 == mLastRemainingPercentageReported) {
          ZS_LOG_WARNING(Detail, log("cannot encrypt packet as packet lifetime is exhausted"))
          return false;
        }

        transport = mSecureTransport.lock();
        if (!transport) {
          ZS_LOG_WARNING(Debug, log("nowhere to send packet as secure transport is gone"))
          return false;
        }

        DirectionMaterial &material = mMaterial[Direction_Encrypt];

        while (true) {
          if (material.mKeyList.size() < 1) {
            ZS_LOG_WARNING(Debug, log("no more keying material is present (all lifetimes are exhausted)") + material.toDebug())
            return false;
          }

          KeyingMaterialPtr keyingMaterial = material.mKeyList.front();

          ASSERT(((bool)keyingMaterial))

          if (keyingMaterial->mTotalPackets[component] + 1 > keyingMaterial->mLifetime) {
            ZS_LOG_WARNING(Debug, log("cannot use keying material as it's lifetime is exhausted") + keyingMaterial->toDebug())
            material.mKeyList.pop_front();
            continue; // try another key
          }

          break;
        }

        ++(keyingMaterial->mTotalPackets[component]);
        updateTotalPackets(Direction_Encrypt, component);
      }

#define WARNING_IF_POSSIBLE_DO_ENCRYPTION_OUTSIDE_OF_OBJECT_LOCK 1
#define WARNING_IF_POSSIBLE_DO_ENCRYPTION_OUTSIDE_OF_OBJECT_LOCK 2

#define WARNING_SHOULD_NOT_REACH_HERE_UNLESS_encryptedBuffer_IS_VALID 1
#define WARNING_SHOULD_NOT_REACH_HERE_UNLESS_encryptedBuffer_IS_VALID 2

      ASSERT(((bool)transport))
      ASSERT(((bool)encryptedBuffer))

      // do NOT call this method from within a lock
      return transport->sendPacket(component, encryptedBuffer->BytePtr(), encryptedBuffer->SizeInBytes());
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPTransport => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void SRTPTransport::onWake()
    {
      ZS_LOG_DEBUG(log("wake"))

      AutoRecursiveLock lock(*this);
#define REMOVE_THIS_IF_NOT_NEEDED 1
#define REMOVE_THIS_IF_NOT_NEEDED 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPTransport => ITimerDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void SRTPTransport::onTimer(TimerPtr timer)
    {
      ZS_LOG_DEBUG(log("timer") + ZS_PARAM("timer id", timer->getID()))

      AutoRecursiveLock lock(*this);
#define TODO 1
#define TODO 2
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPTransport => ISRTPTransportAsyncDelegate
    #pragma mark


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPTransport => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params SRTPTransport::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::SRTPTransport");
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params SRTPTransport::slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::SRTPTransport");
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params SRTPTransport::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr SRTPTransport::toDebug() const
    {
      AutoRecursiveLock lock(*this);

      ElementPtr resultEl = Element::create("ortc::SRTPTransport");

      UseServicesHelper::debugAppend(resultEl, "id", mID);

      UseServicesHelper::debugAppend(resultEl, "subscribers", mSubscriptions.size());
      UseServicesHelper::debugAppend(resultEl, "default subscription", (bool)mDefaultSubscription);

      UseSecureTransportPtr secureTransport = mSecureTransport.lock();
      UseServicesHelper::debugAppend(resultEl, "secure transport", secureTransport ? secureTransport->getID() : 0);

      UseServicesHelper::debugAppend(resultEl, "encrypt params", mParams[Direction_Encrypt].toDebug());
      UseServicesHelper::debugAppend(resultEl, "decrypt params", mParams[Direction_Decrypt].toDebug());

      UseServicesHelper::debugAppend(resultEl, "max total lifetime", mMaxTotalLifetime);

      {
        for (size_t loopDirection = Direction_First; loopDirection != Direction_Last; ++loopDirection) {
          for (size_t loopComponent = IICETypes::Component_First; loopComponent != IICETypes::Component_Last; ++loopComponent) {
            const char *message = "UNDEFINED";
            switch (loopDirection) {
              case Direction_Encrypt: {
                switch (loopComponent) {
                  case IICETypes::Component_RTP:    message = "total RTP packets encrypted"; break;
                  case IICETypes::Component_RTCP:   message = "total RTCP packets encrypted"; break;
                }
                break;
              }
              case Direction_Decrypt: {
                switch (loopComponent) {
                  case IICETypes::Component_RTP:    message = "total RTP packets decrypted"; break;
                  case IICETypes::Component_RTCP:   message = "total RTCP packets decrypted"; break;
                }
                break;
              }
            }

            UseServicesHelper::debugAppend(resultEl, message, mTotalPackets[loopDirection][loopComponent]);
          }
        }

        UseServicesHelper::debugAppend(resultEl, "last remaining percentage reported", mLastRemainingPercentageReported);
      }

      for (size_t loopDirection = Direction_First; loopDirection != Direction_Last; ++loopDirection) {
        UseServicesHelper::debugAppend(resultEl, toString((Directions)loopDirection), mMaterial[loopDirection].toDebug());
      }

      return resultEl;
    }

    //-------------------------------------------------------------------------
    void SRTPTransport::cancel()
    {
      //.......................................................................
      // final cleanup

      mSubscriptions.clear();

      if (mDefaultSubscription) {
        mDefaultSubscription->cancel();
        mDefaultSubscription.reset();
      }
    }

    //-------------------------------------------------------------------------
    void SRTPTransport::updateTotalPackets(
                                           Directions direction,
                                           IICETypes::Components component
                                           )
    {
      size_t largestTotalPackets = 0;

      ++(mTotalPackets[direction][component]);

      for (size_t loopDirection = Direction_First; loopDirection != Direction_Last; ++loopDirection) {
        for (size_t loopComponent = IICETypes::Component_First; loopComponent <= IICETypes::Component_Last; ++loopComponent) {
          if (mTotalPackets[loopDirection][loopComponent] > largestTotalPackets) largestTotalPackets = mTotalPackets[loopDirection][loopComponent];
        }
      }

      size_t consumed = (0 == mMaxTotalLifetime ? 100 : ((largestTotalPackets * 100) / mMaxTotalLifetime));
      if (consumed > 100) consumed = 100;

      size_t remaining = 100 - consumed;

      if (0 == remaining) {
        if (largestTotalPackets < mMaxTotalLifetime) remaining = 1; // do not report 0% percent remaining until FULLY exhausted (treat as 1% remaining in this case)
      }

      if (remaining >= mLastRemainingPercentageReported) return;    // do not report a larger value than last time

      mLastRemainingPercentageReported = remaining;

      auto pThis = mThisWeak.lock();
      if (pThis) {
        mSubscriptions.delegate()->onSRTPTransportLifetimeRemaining(pThis, mLastRemainingPercentageReported);
      }
    }

    //-------------------------------------------------------------------------
    size_t SRTPTransport::parseLifetime(const String &lifetime) throw(InvalidParameters)
    {
      ORTC_THROW_INVALID_PARAMETERS_IF(lifetime.isEmpty())

      UseServicesHelper::SplitMap splitValues;

      UseServicesHelper::split(lifetime, splitValues, '^');

      if (splitValues.size() < 2) {
        try {
          return Numeric<size_t>(lifetime);
        } catch(Numeric<size_t>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Detail, slog("lifetime value out of range") + ZS_PARAM("lifetime", lifetime))
          ORTC_THROW_INVALID_PARAMETERS("unable to parse lifetime: " + lifetime)
        }
      }

      if (splitValues.size() > 2) {
        ZS_LOG_ERROR(Detail, slog("unable to parse lifetime") + ZS_PARAM("lifetime", lifetime))
        ORTC_THROW_INVALID_PARAMETERS("unable to parse lifetime: " + lifetime)
      }

      size_t base {};
      size_t exponent {};

      try {
        base = Numeric<size_t>(splitValues[0]);
        exponent = Numeric<size_t>(splitValues[1]);
      } catch (Numeric<size_t>::ValueOutOfRange &) {
        ZS_LOG_ERROR(Detail, slog("unable to parse lifetime") + ZS_PARAM("lifetime", lifetime))
        ORTC_THROW_INVALID_PARAMETERS("unable to parse lifetime:" + lifetime)
      }

      try {
        return
#ifndef HAVE_TGMATH_H
          static_cast<size_t>(
#endif //HAVE_TGMATH_H
        pow(base, exponent)
#ifndef HAVE_TGMATH_H
        )
#endif //HAVE_TGMATH_H
        ;
      }
      catch (...) {
        ZS_LOG_ERROR(Detail, slog("unable to parse lifetime") + ZS_PARAM("lifetime", lifetime))
        ORTC_THROW_INVALID_PARAMETERS("unable to parse lifetime:" + lifetime)
      }

      ASSERT(false) // should never hit this point
      return 0;
    }

    //-------------------------------------------------------------------------
    SecureByteBlockPtr SRTPTransport::convertIntegerToBigEndianEncodedBuffer(
                                                                             const String &base10Value,
                                                                             size_t maxByteLength
                                                                             ) throw(InvalidParameters)
    {
      SecureByteBlockPtr output(make_shared<SecureByteBlock>(maxByteLength));

      if (base10Value.isEmpty()) return output;

      try {
        Integer value(base10Value); // convert from base 10 into big number class

        size_t minSizeNeeded = value.MinEncodedSize();
        ORTC_THROW_INVALID_PARAMETERS_IF(minSizeNeeded > maxByteLength)

        // this will encode in big endian and pad with most significant "0"
        // values as needed
        value.Encode(output->BytePtr(), output->SizeInBytes());

      } catch(...) {
        ORTC_THROW_INVALID_PARAMETERS("unable to convert integer: " + base10Value)
      }

      return output;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPTransport::KeyingMaterial
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr SRTPTransport::KeyingMaterial::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::SRTPTransport::KeyingMaterial");

      UseServicesHelper::debugAppend(resultEl, mOriginalValues.toDebug());
      UseServicesHelper::debugAppend(resultEl, "mki (hex)", mMKIValue ? UseServicesHelper::convertToHex(*mMKIValue) : String());

      UseServicesHelper::debugAppend(resultEl, "lifetime", mLifetime);

      for (size_t loopComponent = IICETypes::Component_First; loopComponent <= IICETypes::Component_Last; ++loopComponent) {
        const char *message = "UNDEFINED";
        switch (((IICETypes::Components) loopComponent)) {
          case IICETypes::Component_RTP:    message = "total RTP packets"; break;
          case IICETypes::Component_RTCP:   message = "total RTCP packets"; break;
        }
        UseServicesHelper::debugAppend(resultEl, message, mTotalPackets[IICETypes::Component_RTP]);
      }

#define FILL_IN_WITH_MORE_STUFF_HERE 1
#define FILL_IN_WITH_MORE_STUFF_HERE 2

      return resultEl;
    }

    //-------------------------------------------------------------------------
    String SRTPTransport::KeyingMaterial::hash() const
    {
      SHA1Hasher hasher;

      hasher.update("ortc:SRTPTransport::KeyingMaterial:");

      hasher.update(mOriginalValues.hash());
      hasher.update(":");
      hasher.update(mMKIValue ? UseServicesHelper::convertToHex(*mMKIValue) : String());

      hasher.update(":");
      hasher.update(string(mLifetime));

      for (size_t loopComponent = IICETypes::Component_First; loopComponent <= IICETypes::Component_Last; ++loopComponent) {
        hasher.update(":");
        hasher.update(string(mTotalPackets[loopComponent]));
      }

      return hasher.final();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark SRTPTransport::DirectionMaterial
    #pragma mark

    //-------------------------------------------------------------------------
    ElementPtr SRTPTransport::DirectionMaterial::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::SRTPTransport::DirectionMaterial");

      UseServicesHelper::debugAppend(resultEl, "mki length", mMKILength);

      UseServicesHelper::debugAppend(resultEl, "temp mki holder (hex)", mTempMKIHolder ? UseServicesHelper::convertToHex(*mTempMKIHolder) : String());

      for (auto iter = mKeys.begin(); iter != mKeys.end(); ++iter)
      {
        auto keyingMaterial = (*iter).second;
        UseServicesHelper::debugAppend(resultEl, keyingMaterial->toDebug());
      }

      return resultEl;
    }

    //-------------------------------------------------------------------------
    String SRTPTransport::DirectionMaterial::hash() const
    {
      SHA1Hasher hasher;

      hasher.update("ortc:SRTPTransport::DirectionMaterial:");

      hasher.update(string(mMKILength));
      hasher.update(":");
      hasher.update(mTempMKIHolder ? string(mTempMKIHolder->SizeInBytes()) : "0");  // do not hex encode because value is bogus temporary (but size must be fixed)

      for (auto iter = mKeys.begin(); iter != mKeys.end(); ++iter)
      {
        auto keyingMaterial = (*iter).second;

        auto hash = keyingMaterial->hash();

        hasher.update(":");
        hasher.update(hash);
      }

      return hasher.final();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ISRTPTransportFactory
    #pragma mark

    //-------------------------------------------------------------------------
    ISRTPTransportFactory &ISRTPTransportFactory::singleton()
    {
      return SRTPTransportFactory::singleton();
    }

    //-------------------------------------------------------------------------
    SRTPTransportPtr ISRTPTransportFactory::create(
                                                   ISRTPTransportDelegatePtr delegate,
                                                   UseSecureTransportPtr transport,
                                                   const CryptoParameters &encryptParameters,
                                                   const CryptoParameters &decryptParameters
                                                   )
    {
      if (this) {}
      return internal::SRTPTransport::create(delegate, transport, encryptParameters, decryptParameters);
    }


  } // internal namespace


}
