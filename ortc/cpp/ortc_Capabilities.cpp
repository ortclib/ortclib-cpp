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
#include <ortc/IHelper.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>


namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);


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
  ICapabilities::CapabilityBoolean::CapabilityBoolean(
                                                      ElementPtr elem,
                                                      const char *objectName
                                                      )
  {
    if (!elem) return;

    ElementPtr subEl = elem->findFirstChildElement(objectName);
    while (subEl) {

      String text = IHelper::getElementText(subEl);

      try {
        bool value = Numeric<bool>(text);
        insert(value);
      } catch(const Numeric<bool>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, slog("value out of range") + ZS_PARAM("value", text))
      }
      
      subEl = subEl->findNextSiblingElement(objectName);
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityBoolean::createElement(
                                                             const char *objectName,
                                                             const char *objectValueName
                                                          ) const
  {
    ElementPtr outerEl = Element::create(objectName);

    for (auto iter = begin(); iter != end(); ++iter) {
      auto value = (*iter);

      IHelper::adoptElementValue(outerEl, objectValueName, value);
    }

    if (!outerEl->hasChildren()) return ElementPtr();
    return outerEl;
  }

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityBoolean::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ICapabilities::CapabilityBoolean");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto value = (*iter);
      IHelper::debugAppend(resultEl, "value", value ? "true" : "false");
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICapabilities::CapabilityBoolean::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::ICapabilities::CapabilityBoolean:");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto value = (*iter);
      hasher->update(value);
    }

    return hasher->finalizeAsString();
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

    IHelper::getElementValue(elem, "ortc::ICapabilities::CapabilityLong", "min", min);
    IHelper::getElementValue(elem, "ortc::ICapabilities::CapabilityLong", "max", min);

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
      String str = IHelper::getElementText(elem);
      try {
        mMin = mMax = Numeric<decltype(mMin)>(str);
      } catch(const Numeric<decltype(mMin)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, slog("max value out of range") + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityLong::createElement(const char *objectName) const
  {
    if (mMin == mMax) {
      return IHelper::createElementWithNumber(objectName, string(mMin));
    }

    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "min", mMin);
    IHelper::adoptElementValue(elem, "max", mMax);

    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityLong::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ICapabilities::CapabilityLong");

    if (mMin != mMax) {
      IHelper::debugAppend(resultEl, "min", mMin);
      IHelper::debugAppend(resultEl, "min", mMax);
    } else {
      IHelper::debugAppend(resultEl, "value", mMin);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICapabilities::CapabilityLong::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::ICapabilities::CapabilityLong:");

    hasher->update(mMin);
    hasher->update(":");
    hasher->update(mMax);

    return hasher->finalizeAsString();
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

    IHelper::getElementValue(elem, "ortc::ICapabilities::CapabilityDouble", "min", min);
    IHelper::getElementValue(elem, "ortc::ICapabilities::CapabilityDouble", "max", max);

    if (min.hasValue()) {
      mMin = min.value();
      found = true;
    }
    if (max.hasValue()) {
      mMax = max.value();
      found = true;
    }

    if (!found) {
      String str = IHelper::getElementText(elem);
      try {
        mMin = mMax = Numeric<decltype(mMin)>(str);
      } catch(const Numeric<decltype(mMin)>::ValueOutOfRange &) {
        ZS_LOG_WARNING(Debug, slog("max value out of range") + ZS_PARAM("value", str))
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityDouble::createElement(const char *objectName) const
  {
    if (mMin == mMax) {
      return IHelper::createElementWithNumber(objectName, string(mMin));
    }

    ElementPtr outerEl = Element::create(objectName);

    outerEl->adoptAsLastChild(IHelper::createElementWithNumber("min", string(mMin)));
    outerEl->adoptAsLastChild(IHelper::createElementWithNumber("max", string(mMax)));

    return outerEl;
  }
  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityDouble::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ICapabilities::CapabilityDouble");

    if (mMin != mMax) {
      IHelper::debugAppend(resultEl, "min", mMin);
      IHelper::debugAppend(resultEl, "min", mMax);
    } else {
      IHelper::debugAppend(resultEl, "value", mMin);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICapabilities::CapabilityDouble::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::ICapabilities::CapabilityDouble:");

    hasher->update(mMin);
    hasher->update(":");
    hasher->update(mMax);

    return hasher->finalizeAsString();
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

      String text = IHelper::getElementTextAndDecode(subEl);
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

      auto innerEl = IHelper::createElementWithTextAndJSONEncode(objectValueName, value);

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
      IHelper::debugAppend(resultEl, "value", value);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICapabilities::CapabilityString::hash() const
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::ICapabilities::CapabilityString:");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto value = (*iter);
      hasher->update(":");
      hasher->update(value);
    }

    return hasher->finalizeAsString();
  }

}
