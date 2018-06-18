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

#include <ortc/IHelper.h>

#include <ortc/IConstraints.h>

#include <ortc/IHelper.h>

#include <zsLib/eventing/IHasher.h>

#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>


namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc) }

namespace ortc
{
  ZS_DECLARE_USING_PTR(zsLib::eventing, IHasher);

  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);

  using zsLib::Numeric;
  using zsLib::Log;

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IConstraints
  //


  //-----------------------------------------------------------------------
  static Log::Params slog(const char *message) noexcept
  {
    return Log::Params(message, "ortc::IConstraints");
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IConstraints::ConstrainBooleanParameters
  //


  //---------------------------------------------------------------------------
  IConstraints::ConstrainBooleanParameters::ConstrainBooleanParameters(ElementPtr elem) noexcept
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IConstraints::ConstrainBooleanParameters", "exact", mExact);
    IHelper::getElementValue(elem, "ortc::IConstraints::ConstrainBooleanParameters", "ideal", mIdeal);
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainBooleanParameters::createElement(const char *objectName) const noexcept
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "exact", mExact);
    IHelper::adoptElementValue(elem, "ideal", mIdeal);

    if (!elem->hasChildren()) return ElementPtr();
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainBooleanParameters::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainBooleanParameters");

    IHelper::debugAppend(resultEl, "exact", mExact);
    IHelper::debugAppend(resultEl, "ideal", mIdeal);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainBooleanParameters::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IConstraints::ConstrainBooleanParameters:");

    hasher->update(mExact);
    hasher->update(":");
    hasher->update(mIdeal);

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IConstraints::ConstrainBool
  //

  //---------------------------------------------------------------------------
  IConstraints::ConstrainBoolean::ConstrainBoolean(ElementPtr elem) noexcept
  {
    if (!elem) return;

    if (elem->getFirstChildElement()) {
      // treat as params...
      mParameters = ConstrainBooleanParameters(elem);
      return;
    }

    {
      String str = IHelper::getElementText(elem);
      if (str.hasData()) {
        try {
          mValue = Numeric<decltype(mValue.mType)>(str);
        } catch(const Numeric<decltype(mValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog("value out of range") + ZS_PARAM("value", str))
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainBoolean::createElement(const char *objectName) const noexcept
  {
    if (mParameters.hasValue()) {
      return mParameters.value().createElement(objectName);
    }

    if (!mValue.hasValue()) return ElementPtr();
    return IHelper::createElementWithNumber(objectName, string(mValue.value()));
  }
  
  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainBoolean::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainBoolean");

    IHelper::debugAppend(resultEl, "value", mValue.hasValue() ? ((mValue.value()) ? "true" : "false") : "");
    IHelper::debugAppend(resultEl, mParameters.hasValue() ? mParameters.value().toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainBoolean::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IConstraints::ConstrainBoolean:");

    hasher->update(mValue);
    hasher->update(":");
    hasher->update(mParameters.hasValue() ? mParameters.value().hash() : String());

    return hasher->finalizeAsString();
  }



  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IConstraints::ConstrainLongRange
  //

  //---------------------------------------------------------------------------
  IConstraints::ConstrainLongRange::ConstrainLongRange(ElementPtr elem) noexcept
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IConstraints::ConstrainLongRange", "min", mMin);
    IHelper::getElementValue(elem, "ortc::IConstraints::ConstrainLongRange", "max", mMax);
    IHelper::getElementValue(elem, "ortc::IConstraints::ConstrainLongRange", "exact", mExact);
    IHelper::getElementValue(elem, "ortc::IConstraints::ConstrainLongRange", "ideal", mIdeal);
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainLongRange::createElement(const char *objectName) const noexcept
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "min", mMin);
    IHelper::adoptElementValue(elem, "max", mMax);
    IHelper::adoptElementValue(elem, "exact", mExact);
    IHelper::adoptElementValue(elem, "ideal", mIdeal);

    if (!elem->hasChildren()) return ElementPtr();
    return elem;
  }
  
  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainLongRange::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainLongRange");

    IHelper::debugAppend(resultEl, "min", mMin);
    IHelper::debugAppend(resultEl, "max", mMax);
    IHelper::debugAppend(resultEl, "ideal", mIdeal);
    IHelper::debugAppend(resultEl, "exact", mExact);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainLongRange::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IConstraints::ConstrainLongRange:");

    hasher->update(mMin);
    hasher->update(":");
    hasher->update(mMax);
    hasher->update(":");
    hasher->update(mExact);
    hasher->update(":");
    hasher->update(mIdeal);

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IConstraints::ConstrainLong
  //

  //---------------------------------------------------------------------------
  IConstraints::ConstrainLong::ConstrainLong(ElementPtr elem) noexcept
  {
    if (!elem) return;

    if (elem->getFirstChildElement()) {
      // treat as range...
      mRange = ConstrainLongRange(elem);
      return;
    }

    {
      String str = IHelper::getElementText(elem);
      if (str.hasData()) {
        try {
          mValue = Numeric<decltype(mValue.mType)>(str);
        } catch(const Numeric<decltype(mValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog("value out of range") + ZS_PARAM("value", str))
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainLong::createElement(const char *objectName) const noexcept
  {
    if (mRange.hasValue()) {
      return mRange.value().createElement(objectName);
    }

    if (!mValue.hasValue()) return ElementPtr();
    return IHelper::createElementWithNumber(objectName, string(mValue.value()));
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainLong::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainLong");

    IHelper::debugAppend(resultEl, "value", mValue);
    IHelper::debugAppend(resultEl, mRange.hasValue() ? mRange.value().toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainLong::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IConstraints::ConstrainLong:");

    hasher->update(mValue);
    hasher->update(":");
    hasher->update(mRange.hasValue() ? mRange.value().hash() : String());

    return hasher->finalizeAsString();
  }



  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IConstraints::ConstrainDoubleRange
  //


  //---------------------------------------------------------------------------
  IConstraints::ConstrainDoubleRange::ConstrainDoubleRange(ElementPtr elem) noexcept
  {
    if (!elem) return;

    IHelper::getElementValue(elem, "ortc::IConstraints::ConstrainDoubleRange", "min", mMin);
    IHelper::getElementValue(elem, "ortc::IConstraints::ConstrainDoubleRange", "max", mMax);
    IHelper::getElementValue(elem, "ortc::IConstraints::ConstrainDoubleRange", "exact", mExact);
    IHelper::getElementValue(elem, "ortc::IConstraints::ConstrainDoubleRange", "ideal", mIdeal);
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainDoubleRange::createElement(const char *objectName) const noexcept
  {
    ElementPtr elem = Element::create(objectName);

    IHelper::adoptElementValue(elem, "min", mMin);
    IHelper::adoptElementValue(elem, "max", mMax);
    IHelper::adoptElementValue(elem, "exact", mExact);
    IHelper::adoptElementValue(elem, "ideal", mIdeal);

    if (!elem->hasChildren()) return ElementPtr();
    return elem;
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainDoubleRange::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainDoubleRange");

    IHelper::debugAppend(resultEl, "min", mMin);
    IHelper::debugAppend(resultEl, "max", mMax);
    IHelper::debugAppend(resultEl, "ideal", mIdeal);
    IHelper::debugAppend(resultEl, "exact", mExact);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainDoubleRange::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IConstraints::ConstrainDoubleRange:");

    hasher->update(mMin);
    hasher->update(":");
    hasher->update(mMax);
    hasher->update(":");
    hasher->update(mExact);
    hasher->update(":");
    hasher->update(mIdeal);
    
    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IConstraints::ConstrainDouble
  //

  //---------------------------------------------------------------------------
  IConstraints::ConstrainDouble::ConstrainDouble(ElementPtr elem) noexcept
  {
    if (!elem) return;

    if (elem->getFirstChildElement()) {
      // treat as range...
      mRange = ConstrainDoubleRange(elem);
      return;
    }

    {
      String str = IHelper::getElementText(elem);
      if (str.hasData()) {
        try {
          mValue = Numeric<decltype(mValue.mType)>(str);
        } catch(const Numeric<decltype(mValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog("value out of range") + ZS_PARAM("value", str))
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainDouble::createElement(const char *objectName) const noexcept
  {
    if (mRange.hasValue()) {
      return mRange.value().createElement(objectName);
    }

    if (!mValue.hasValue()) return ElementPtr();
    return IHelper::createElementWithNumber(objectName, string(mValue.value()));
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainDouble::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainDouble");

    IHelper::debugAppend(resultEl, "value", mValue);
    IHelper::debugAppend(resultEl, mRange.hasValue() ? mRange.value().toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainDouble::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IConstraints::ConstrainDouble:");

    hasher->update(mValue);
    hasher->update(":");
    hasher->update(mRange.hasValue() ? mRange.value().hash() : String());

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IConstraints::StringOrStringList
  //

  //---------------------------------------------------------------------------
  IConstraints::StringOrStringList::StringOrStringList(
                                                       ElementPtr elem,
                                                       const char *objectValueName
                                                       ) noexcept
  {
    if (!elem) return;

    if (elem->getFirstChildElement()) {      
      // treat as list...
      ElementPtr subEl = elem->findFirstChildElement(objectValueName);
      while (subEl) {
        String str = IHelper::getElementTextAndDecode(subEl);

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
      String str = IHelper::getElementTextAndDecode(elem);
      if (str.hasData()) {
        mValue = str;
      }
    }
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::StringOrStringList::createElement(
                                                             const char *objectName,
                                                             const char *objectValueName
                                                             ) const noexcept
  {
    if (mValues.hasValue()) {
      ElementPtr outerEl = Element::create(objectName);

      for (auto iter = mValues.value().begin(); iter != mValues.value().end(); ++iter) {
        auto value = (*iter);
        outerEl->adoptAsLastChild(IHelper::createElementWithTextAndJSONEncode(objectValueName, value));
      }

      if (outerEl->hasChildren()) return ElementPtr();
      return outerEl;
    }

    if (!mValue.hasValue()) return ElementPtr();
    return IHelper::createElementWithNumber(objectName, mValue.value());
  }
  
  //---------------------------------------------------------------------------
  ElementPtr IConstraints::StringOrStringList::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::StringOrStringList");

    IHelper::debugAppend(resultEl, "value", mValue);

    if (mValues.hasValue()) {
      ElementPtr valuesEl = Element::create("values");
      for (auto iter = mValues.value().begin(); iter != mValues.value().end(); ++iter) {
        auto str = (*iter);
        IHelper::debugAppend(valuesEl, "value", str);
      }
      IHelper::debugAppend(valuesEl, valuesEl);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::StringOrStringList::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IConstraints::StringOrStringList:");

    hasher->update(mValue);
    hasher->update(":");

    if (mValues.hasValue()) {
      for (auto iter = mValues.value().begin(); iter != mValues.value().end(); ++iter) {
        auto str = (*iter);
        hasher->update(":");
        hasher->update(str);
      }
    }

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IConstraints::ConstrainStringParameters
  //

  //---------------------------------------------------------------------------
  IConstraints::ConstrainStringParameters::ConstrainStringParameters(ElementPtr elem) noexcept
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
  ElementPtr IConstraints::ConstrainStringParameters::createElement(const char *objectName) const noexcept
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
  ElementPtr IConstraints::ConstrainStringParameters::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainStringParameters");

    IHelper::debugAppend(resultEl, "exact", mExact.hasValue() ? mExact.value().toDebug()  : ElementPtr());
    IHelper::debugAppend(resultEl, "ideal", mIdeal.hasValue() ? mIdeal.value().toDebug()  : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainStringParameters::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IConstraints::ConstrainStringParameters:");

    hasher->update(mExact.hasValue() ? mExact.value().hash() : String());
    hasher->update(":");
    hasher->update(mIdeal.hasValue() ? mIdeal.value().hash() : String());

    return hasher->finalizeAsString();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IConstraints::ConstraintString
  //

  //---------------------------------------------------------------------------
  IConstraints::ConstrainString::ConstrainString(
                                                 ElementPtr elem,
                                                 const char *objectValueName
                                                 ) noexcept
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
                                                          ) const noexcept
  {
    ElementPtr elem = Element::create(objectName);

    if (mParameters.hasValue()) {
      return mParameters.value().createElement(objectName);
    }

    if (!mValue.hasValue()) return ElementPtr();
    return mValue.value().createElement(objectName, objectValueName);
  }

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainString::toDebug() const noexcept
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstraintString");

    IHelper::debugAppend(resultEl, mValue.hasValue() ? mValue.value().toDebug() : ElementPtr());
    IHelper::debugAppend(resultEl, mParameters.hasValue() ? mParameters.value().toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainString::hash() const noexcept
  {
    auto hasher = IHasher::sha1();

    hasher->update("ortc::IConstraints::ConstraintString:");

    hasher->update(mValue.hasValue() ? mValue.value().hash() : String());
    hasher->update(":");
    hasher->update(mParameters.hasValue() ? mParameters.value().hash() : String());

    return hasher->finalizeAsString();
  }

  //---------------------------------------------------------------------------
  void IConstraints::ConstrainString::exact(StringList &values) const noexcept
  {
    if (mValue.hasValue()) {
      if (mValue.value().mValue.hasValue()) {
        values.push_back(mValue.value().mValue.value());
        return;
      }
      if (mValue.value().mValues.hasValue()) {
        auto &source = mValue.value().mValues.value();
        values.insert(values.end(), source.begin(), source.end());
        return;
      }
    }

    if (!mParameters.hasValue()) return;
    if (!mParameters.value().mExact.hasValue()) return;
    if (mParameters.value().mExact.value().mValue.hasValue()) {
      values.push_back(mValue.value().mValue.value());
      return;
    }
    if (!mParameters.value().mExact.value().mValues.hasValue()) return;
    auto &source = mParameters.value().mExact.value().mValues.value();
    values.insert(values.end(), source.begin(), source.end());
  }

  //---------------------------------------------------------------------------
  void IConstraints::ConstrainString::ideal(StringList &values) const noexcept
  {
    if (!mParameters.hasValue()) return;
    if (!mParameters.value().mIdeal.hasValue()) return;
    if (mParameters.value().mIdeal.value().mValue.hasValue()) {
      values.push_back(mValue.value().mValue.value());
      return;
    }
    if (!mParameters.value().mIdeal.value().mValues.hasValue()) return;
    auto &source = mParameters.value().mIdeal.value().mValues.value();
    values.insert(values.end(), source.begin(), source.end());
  }

}
