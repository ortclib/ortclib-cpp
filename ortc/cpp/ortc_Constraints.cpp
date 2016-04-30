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

#include <ortc/IConstraints.h>

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

  typedef openpeer::services::Hasher<CryptoPP::SHA1> SHA1Hasher;

  ZS_DECLARE_TYPEDEF_PTR(ortc::internal::Helper, UseHelper)

  using zsLib::Numeric;
  using zsLib::Log;

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IConstraints
  #pragma mark


  //-----------------------------------------------------------------------
  static Log::Params slog(const char *message)
  {
    return Log::Params(message, "ortc::IConstraints");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IConstraints::ConstrainBooleanParameters
  #pragma mark


  //---------------------------------------------------------------------------
  IConstraints::ConstrainBooleanParameters::ConstrainBooleanParameters(ElementPtr elem)
  {
    if (!elem) return;

    UseHelper::getElementValue(elem, "ortc::IConstraints::ConstrainBooleanParameters", "exact", mExact);
    UseHelper::getElementValue(elem, "ortc::IConstraints::ConstrainBooleanParameters", "ideal", mIdeal);
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainBooleanParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "exact", mExact);
    UseHelper::adoptElementValue(elem, "ideal", mIdeal);

    if (!elem->hasChildren()) return ElementPtr();
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainBooleanParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainBooleanParameters");

    UseServicesHelper::debugAppend(resultEl, "exact", mExact);
    UseServicesHelper::debugAppend(resultEl, "ideal", mIdeal);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainBooleanParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IConstraints::ConstrainBooleanParameters:");

    hasher.update(mExact);
    hasher.update(":");
    hasher.update(mIdeal);

    return hasher.final();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IConstraints::ConstrainBool
  #pragma mark

  //---------------------------------------------------------------------------
  IConstraints::ConstrainBoolean::ConstrainBoolean(ElementPtr elem)
  {
    if (!elem) return;

    if (elem->getFirstChildElement()) {
      // treat as params...
      mParameters = ConstrainBooleanParameters(elem);
      return;
    }

    {
      String str = UseServicesHelper::getElementText(elem);
      if (str.hasData()) {
        try {
          mValue = Numeric<decltype(mValue.mType)>(str);
        } catch(Numeric<decltype(mValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog("value out of range") + ZS_PARAM("value", str))
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainBoolean::createElement(const char *objectName) const
  {
    if (mParameters.hasValue()) {
      return mParameters.value().createElement(objectName);
    }

    if (!mValue.hasValue()) return ElementPtr();
    return UseServicesHelper::createElementWithNumber(objectName, string(mValue.value()));
  }
  
  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainBoolean::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainBoolean");

    UseServicesHelper::debugAppend(resultEl, "value", mValue.hasValue() ? ((mValue.value()) ? "true" : "false") : "");
    UseServicesHelper::debugAppend(resultEl, mParameters.hasValue() ? mParameters.value().toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainBoolean::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IConstraints::ConstrainBoolean:");

    hasher.update(mValue);
    hasher.update(":");
    hasher.update(mParameters.hasValue() ? mParameters.value().hash() : String());

    return hasher.final();
  }



  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IConstraints::ConstrainLongRange
  #pragma mark

  //---------------------------------------------------------------------------
  IConstraints::ConstrainLongRange::ConstrainLongRange(ElementPtr elem)
  {
    if (!elem) return;

    UseHelper::getElementValue(elem, "ortc::IConstraints::ConstrainLongRange", "min", mMin);
    UseHelper::getElementValue(elem, "ortc::IConstraints::ConstrainLongRange", "max", mMax);
    UseHelper::getElementValue(elem, "ortc::IConstraints::ConstrainLongRange", "exact", mExact);
    UseHelper::getElementValue(elem, "ortc::IConstraints::ConstrainLongRange", "ideal", mIdeal);
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainLongRange::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "min", mMin);
    UseHelper::adoptElementValue(elem, "max", mMax);
    UseHelper::adoptElementValue(elem, "exact", mExact);
    UseHelper::adoptElementValue(elem, "ideal", mIdeal);

    if (!elem->hasChildren()) return ElementPtr();
    return elem;
  }
  
  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainLongRange::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainLongRange");

    UseServicesHelper::debugAppend(resultEl, "min", mMin);
    UseServicesHelper::debugAppend(resultEl, "max", mMax);
    UseServicesHelper::debugAppend(resultEl, "ideal", mIdeal);
    UseServicesHelper::debugAppend(resultEl, "exact", mExact);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainLongRange::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IConstraints::ConstrainLongRange:");

    hasher.update(mMin);
    hasher.update(":");
    hasher.update(mMax);
    hasher.update(":");
    hasher.update(mExact);
    hasher.update(":");
    hasher.update(mIdeal);

    return hasher.final();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IConstraints::ConstrainLong
  #pragma mark

  //---------------------------------------------------------------------------
  IConstraints::ConstrainLong::ConstrainLong(ElementPtr elem)
  {
    if (!elem) return;

    if (elem->getFirstChildElement()) {
      // treat as range...
      mRange = ConstrainLongRange(elem);
      return;
    }

    {
      String str = UseServicesHelper::getElementText(elem);
      if (str.hasData()) {
        try {
          mValue = Numeric<decltype(mValue.mType)>(str);
        } catch(Numeric<decltype(mValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog("value out of range") + ZS_PARAM("value", str))
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainLong::createElement(const char *objectName) const
  {
    if (mRange.hasValue()) {
      return mRange.value().createElement(objectName);
    }

    if (!mValue.hasValue()) return ElementPtr();
    return UseServicesHelper::createElementWithNumber(objectName, string(mValue.value()));
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainLong::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainLong");

    UseServicesHelper::debugAppend(resultEl, "value", mValue);
    UseServicesHelper::debugAppend(resultEl, mRange.hasValue() ? mRange.value().toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainLong::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IConstraints::ConstrainLong:");

    hasher.update(mValue);
    hasher.update(":");
    hasher.update(mRange.hasValue() ? mRange.value().hash() : String());

    return hasher.final();
  }



  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IConstraints::ConstrainDoubleRange
  #pragma mark


  //---------------------------------------------------------------------------
  IConstraints::ConstrainDoubleRange::ConstrainDoubleRange(ElementPtr elem)
  {
    if (!elem) return;

    UseHelper::getElementValue(elem, "ortc::IConstraints::ConstrainDoubleRange", "min", mMin);
    UseHelper::getElementValue(elem, "ortc::IConstraints::ConstrainDoubleRange", "max", mMax);
    UseHelper::getElementValue(elem, "ortc::IConstraints::ConstrainDoubleRange", "exact", mExact);
    UseHelper::getElementValue(elem, "ortc::IConstraints::ConstrainDoubleRange", "ideal", mIdeal);
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainDoubleRange::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    UseHelper::adoptElementValue(elem, "min", mMin);
    UseHelper::adoptElementValue(elem, "max", mMax);
    UseHelper::adoptElementValue(elem, "exact", mExact);
    UseHelper::adoptElementValue(elem, "ideal", mIdeal);

    if (!elem->hasChildren()) return ElementPtr();
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainDoubleRange::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainDoubleRange");

    UseServicesHelper::debugAppend(resultEl, "min", mMin);
    UseServicesHelper::debugAppend(resultEl, "max", mMax);
    UseServicesHelper::debugAppend(resultEl, "ideal", mIdeal);
    UseServicesHelper::debugAppend(resultEl, "exact", mExact);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainDoubleRange::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IConstraints::ConstrainDoubleRange:");

    hasher.update(mMin);
    hasher.update(":");
    hasher.update(mMax);
    hasher.update(":");
    hasher.update(mExact);
    hasher.update(":");
    hasher.update(mIdeal);
    
    return hasher.final();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IConstraints::ConstrainDouble
  #pragma mark

  //---------------------------------------------------------------------------
  IConstraints::ConstrainDouble::ConstrainDouble(ElementPtr elem)
  {
    if (!elem) return;

    if (elem->getFirstChildElement()) {
      // treat as range...
      mRange = ConstrainDoubleRange(elem);
      return;
    }

    {
      String str = UseServicesHelper::getElementText(elem);
      if (str.hasData()) {
        try {
          mValue = Numeric<decltype(mValue.mType)>(str);
        } catch(Numeric<decltype(mValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog("value out of range") + ZS_PARAM("value", str))
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainDouble::createElement(const char *objectName) const
  {
    if (mRange.hasValue()) {
      return mRange.value().createElement(objectName);
    }

    if (!mValue.hasValue()) return ElementPtr();
    return UseServicesHelper::createElementWithNumber(objectName, string(mValue.value()));
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainDouble::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainDouble");

    UseServicesHelper::debugAppend(resultEl, "value", mValue);
    UseServicesHelper::debugAppend(resultEl, mRange.hasValue() ? mRange.value().toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainDouble::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IConstraints::ConstrainDouble:");

    hasher.update(mValue);
    hasher.update(":");
    hasher.update(mRange.hasValue() ? mRange.value().hash() : String());

    return hasher.final();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IConstraints::StringOrStringList
  #pragma mark

  //---------------------------------------------------------------------------
  IConstraints::StringOrStringList::StringOrStringList(
                                                       ElementPtr elem,
                                                       const char *objectValueName
                                                       )
  {
    if (!elem) return;

    if (elem->getFirstChildElement()) {      
      // treat as list...
      ElementPtr subEl = elem->findFirstChildElement(objectValueName);
      while (subEl) {
        String str = UseServicesHelper::getElementTextAndDecode(subEl);

        if (str.hasData()) {
          if (!mValues.hasValue()) {
            mValues = StringList();
          }

          mValues.value().push_back(str);
        }

        subEl = subEl->findNextSiblingElement(objectValueName);
      }
      return;
    }

    {
      String str = UseServicesHelper::getElementTextAndDecode(elem);
      if (str.hasData()) {
        mValue = str;
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::StringOrStringList::createElement(
                                                             const char *objectName,
                                                             const char *objectValueName
                                                             ) const
  {
    if (mValues.hasValue()) {
      ElementPtr outerEl = Element::create(objectName);

      for (auto iter = mValues.value().begin(); iter != mValues.value().end(); ++iter) {
        auto value = (*iter);
        outerEl->adoptAsLastChild(UseServicesHelper::createElementWithTextAndJSONEncode(objectValueName, value));
      }

      if (outerEl->hasChildren()) return ElementPtr();
      return outerEl;
    }

    if (!mValue.hasValue()) return ElementPtr();
    return UseServicesHelper::createElementWithNumber(objectName, mValue.value());
  }
  
  //---------------------------------------------------------------------------
  ElementPtr IConstraints::StringOrStringList::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::StringOrStringList");

    UseServicesHelper::debugAppend(resultEl, "value", mValue);

    if (mValues.hasValue()) {
      ElementPtr valuesEl = Element::create("values");
      for (auto iter = mValues.value().begin(); iter != mValues.value().end(); ++iter) {
        auto str = (*iter);
        UseServicesHelper::debugAppend(valuesEl, "value", str);
      }
      UseServicesHelper::debugAppend(valuesEl, valuesEl);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::StringOrStringList::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IConstraints::StringOrStringList:");

    hasher.update(mValue);
    hasher.update(":");

    if (mValues.hasValue()) {
      for (auto iter = mValues.value().begin(); iter != mValues.value().end(); ++iter) {
        auto str = (*iter);
        hasher.update(":");
        hasher.update(str);
      }
    }

    return hasher.final();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IConstraints::ConstrainStringParameters
  #pragma mark

  //---------------------------------------------------------------------------
  IConstraints::ConstrainStringParameters::ConstrainStringParameters(ElementPtr elem)
  {
    if (!elem) return;

    ElementPtr exactEl = elem->findFirstChildElement("exact");
    if (exactEl) {
      mExact = StringOrStringList(exactEl);
    }

    ElementPtr idealEl = elem->findFirstChildElement("ideal");
    if (idealEl) {
      mIdeal = StringOrStringList(idealEl);
    }
  }
  
  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainStringParameters::createElement(const char *objectName) const
  {
    ElementPtr elem = Element::create(objectName);

    if (mExact.hasValue()) {
      elem->adoptAsLastChild(mExact.value().createElement("exact"));
    }
    if (mIdeal.hasValue()) {
      elem->adoptAsLastChild(mExact.value().createElement("ideal"));
    }
    if (!elem->hasChildren()) return ElementPtr();
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainStringParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainStringParameters");

    UseServicesHelper::debugAppend(resultEl, "exact", mExact.hasValue() ? mExact.value().toDebug()  : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, "ideal", mIdeal.hasValue() ? mIdeal.value().toDebug()  : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainStringParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IConstraints::ConstrainStringParameters:");

    hasher.update(mExact.hasValue() ? mExact.value().hash() : String());
    hasher.update(":");
    hasher.update(mIdeal.hasValue() ? mIdeal.value().hash() : String());

    return hasher.final();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IConstraints::ConstraintString
  #pragma mark

  //---------------------------------------------------------------------------
  IConstraints::ConstrainString::ConstrainString(
                                                 ElementPtr elem,
                                                 const char *objectValueName
                                                 )
  {
    if (!elem) return;

    ElementPtr exactEl = elem->findFirstChildElement("exact");
    ElementPtr idealEl = elem->findFirstChildElement("ideal");

    if ((exactEl) || (idealEl)) {
      mParameters = ConstrainStringParameters(elem);
      return;
    }

    StringOrStringList value(elem, objectValueName);
    if ((value.mValue.hasValue()) ||
        (value.mValues.hasValue())) {
      mValue = value;
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainString::createElement(
                                                          const char *objectName,
                                                          const char *objectValueName
                                                          ) const
  {
    ElementPtr elem = Element::create(objectName);

    if (mParameters.hasValue()) {
      return mParameters.value().createElement(objectName);
    }

    if (!mValue.hasValue()) return ElementPtr();
    return mValue.value().createElement(objectName, objectValueName);
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainString::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstraintString");

    UseServicesHelper::debugAppend(resultEl, mValue.hasValue() ? mValue.value().toDebug() : ElementPtr());
    UseServicesHelper::debugAppend(resultEl, mParameters.hasValue() ? mParameters.value().toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainString::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IConstraints::ConstraintString:");

    hasher.update(mValue.hasValue() ? mValue.value().hash() : String());
    hasher.update(":");
    hasher.update(mParameters.hasValue() ? mParameters.value().hash() : String());

    return hasher.final();
  }

}
