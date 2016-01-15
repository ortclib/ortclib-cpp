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

#include <ortc/internal/types.h>
#include <ortc/internal/platform.h>
#include <ortc/internal/ortc_Helper.h>

#include <ortc/ICapabilities.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseSettings)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHTTP, UseHTTP)

  ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper)

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  using zsLib::Numeric;
  using zsLib::Log;

  //-----------------------------------------------------------------------
  static Log::Params slog(const char *message)
  {
    return Log::Params(message, "ortc::ICapabilities");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ICapabilities::CapabilityBool
  #pragma mark

  //---------------------------------------------------------------------------
  ICapabilities::CapabilityBool::CapabilityBool(
                                                ElementPtr elem,
                                                const char *objectName
                                                )
  {
    if (!elem) return;

    ElementPtr subEl = elem->findFirstChildElement(objectName);
    while (subEl) {

      String text = UseServicesHelper::getElementText(subEl);

      try {
        bool value = Numeric<bool>(text);
        insert(value);
      } catch(Numeric<bool>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, slog("value out of range") + ZS_PARAM("value", text))
      }
      
      subEl = subEl->findNextSiblingElement(objectName);
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityBool::createElement(
                                                          const char *objectName,
                                                          const char *objectValueName
                                                          ) const
  {
    ElementPtr outerEl = Element::create(objectName);

    for (auto iter = begin(); iter != end(); ++iter) {
      auto value = (*iter);

      UseHelper::adoptElementValue(outerEl, objectValueName, value);
    }

    if (!outerEl->hasChildren()) return ElementPtr();
    return outerEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityBool::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ICapabilities::CapabilityBool");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto value = (*iter);
      UseServicesHelper::debugAppend(resultEl, "value", value ? "true" : "false");
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICapabilities::CapabilityBool::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::ICapabilities::CapabilityBool:");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto value = (*iter);
      hasher.update(value);
    }

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ICapabilities::CapabilityLong
  #pragma mark


  //---------------------------------------------------------------------------
  ICapabilities::CapabilityLong::CapabilityLong(ElementPtr elem)
  {
    if (!elem) return;

    Optional<decltype(mMin)> min;
    Optional<decltype(mMax)> max;

    UseHelper::getElementValue(elem, "ortc::ICapabilities::CapabilityLong", "min", min);
    UseHelper::getElementValue(elem, "ortc::ICapabilities::CapabilityLong", "max", min);

    bool found = false;

    if (min.hasValue()) {
      mMin = min.value();
      found = true;
    }
    if (max.hasValue()) {
      mMax = max.value();
      found = true;
    }

    if (!found) {
      String str = UseServicesHelper::getElementText(elem);
      try {
        mMin = mMax = Numeric<decltype(mMin)>(str);
      } catch(Numeric<decltype(mMin)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, slog("max value out of range") + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityLong::createElement(const char *objectName) const
  {
    if (mMin == mMax) {
      return UseServicesHelper::createElementWithNumber(objectName, string(mMin));
    }

    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "min", mMin);
    UseHelper::adoptElementValue(elem, "max", mMax);

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityLong::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ICapabilities::CapabilityLong");

    if (mMin != mMax) {
      UseServicesHelper::debugAppend(resultEl, "min", mMin);
      UseServicesHelper::debugAppend(resultEl, "min", mMax);
    } else {
      UseServicesHelper::debugAppend(resultEl, "value", mMin);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICapabilities::CapabilityLong::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::ICapabilities::CapabilityLong:");

    hasher.update(mMin);
    hasher.update(":");
    hasher.update(mMax);

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ICapabilities::CapabilityDouble
  #pragma mark

  //---------------------------------------------------------------------------
  ICapabilities::CapabilityDouble::CapabilityDouble(ElementPtr elem)
  {
    if (!elem) return;

    bool found = false;

    Optional<decltype(mMin)> min;
    Optional<decltype(mMax)> max;

    UseHelper::getElementValue(elem, "ortc::ICapabilities::CapabilityDouble", "min", min);
    UseHelper::getElementValue(elem, "ortc::ICapabilities::CapabilityDouble", "max", max);

    if (min.hasValue()) {
      mMin = min.value();
      found = true;
    }
    if (max.hasValue()) {
      mMax = max.value();
      found = true;
    }

    if (!found) {
      String str = UseServicesHelper::getElementText(elem);
      try {
        mMin = mMax = Numeric<decltype(mMin)>(str);
      } catch(Numeric<decltype(mMin)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, slog("max value out of range") + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityDouble::createElement(const char *objectName) const
  {
    if (mMin == mMax) {
      return UseServicesHelper::createElementWithNumber(objectName, string(mMin));
    }

    ElementPtr outerEl = Element::create(objectName);

    outerEl->adoptAsLastChild(UseServicesHelper::createElementWithNumber("min", string(mMin)));
    outerEl->adoptAsLastChild(UseServicesHelper::createElementWithNumber("max", string(mMax)));

    return outerEl;
  }
  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityDouble::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ICapabilities::CapabilityDouble");

    if (mMin != mMax) {
      UseServicesHelper::debugAppend(resultEl, "min", mMin);
      UseServicesHelper::debugAppend(resultEl, "min", mMax);
    } else {
      UseServicesHelper::debugAppend(resultEl, "value", mMin);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICapabilities::CapabilityDouble::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::ICapabilities::CapabilityDouble:");

    hasher.update(mMin);
    hasher.update(":");
    hasher.update(mMax);

    return hasher.final();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ICapabilities::CapabilityString
  #pragma mark


  //---------------------------------------------------------------------------
  ICapabilities::CapabilityString::CapabilityString(
                                                    ElementPtr elem,
                                                    const char *objectName
                                                    )
  {
    if (!elem) return;

    ElementPtr subEl = elem->findFirstChildElement(objectName);
    while (subEl) {

      String text = UseServicesHelper::getElementTextAndDecode(subEl);
      insert(text);

      subEl = subEl->findNextSiblingElement(objectName);
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityString::createElement(
                                                            const char *objectName,
                                                            const char *objectValueName
                                                            ) const
  {
    ElementPtr outerEl = Element::create(objectName);

    for (auto iter = begin(); iter != end(); ++iter) {
      auto value = (*iter);

      auto innerEl = UseServicesHelper::createElementWithTextAndJSONEncode(objectValueName, value);

      outerEl->adoptAsLastChild(innerEl);
    }

    if (!outerEl->hasChildren()) return ElementPtr();
    return outerEl;
  }
  

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityString::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ICapabilities::CapabilityString");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto value = (*iter);
      UseServicesHelper::debugAppend(resultEl, "value", value);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICapabilities::CapabilityString::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::ICapabilities::CapabilityString:");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto value = (*iter);
      hasher.update(":");
      hasher.update(value);
    }

    return hasher.final();
  }

}
