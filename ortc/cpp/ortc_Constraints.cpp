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

#include <ortc/IConstraints.h>

#include <openpeer/services/IHelper.h>

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

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IConstraints
  #pragma mark



  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IConstraints::ConstrainBoolParameters
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IConstraints::ConstrainBoolParameters::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainBoolParameters");

    UseServicesHelper::debugAppend(resultEl, "exact", mExact);
    UseServicesHelper::debugAppend(resultEl, "ideal", mIdeal);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainBoolParameters::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IConstraints::ConstrainBoolParameters:");

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
  ElementPtr IConstraints::ConstrainBool::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::IConstraints::ConstrainBool");

    UseServicesHelper::debugAppend(resultEl, "value", mValue.hasValue() ? ((mValue.value()) ? "true" : "false") : "");
    UseServicesHelper::debugAppend(resultEl, mParameters.hasValue() ? mParameters.value().toDebug() : ElementPtr());

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String IConstraints::ConstrainBool::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::IConstraints::ConstrainBool:");

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
