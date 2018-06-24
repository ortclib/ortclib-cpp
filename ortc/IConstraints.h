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

#pragma once

#include <ortc/types.h>

#include <list>
#include <zsLib/types.h>

namespace ortc
{
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //
  // IConstraints
  //
  
  interaction IConstraints
  {
    ZS_DECLARE_STRUCT_PTR(ConstrainBoolean);
    ZS_DECLARE_STRUCT_PTR(ConstrainBooleanParameters);
    ZS_DECLARE_STRUCT_PTR(ConstrainLong);
    ZS_DECLARE_STRUCT_PTR(ConstrainLongRange);
    ZS_DECLARE_STRUCT_PTR(ConstrainDouble);
    ZS_DECLARE_STRUCT_PTR(ConstrainDoubleRange);
    ZS_DECLARE_STRUCT_PTR(ConstrainString);
    ZS_DECLARE_STRUCT_PTR(ConstrainStringParameters);
    ZS_DECLARE_STRUCT_PTR(StringOrStringList);

    ZS_DECLARE_TYPEDEF_PTR(zsLib::LONG, Long);
    ZS_DECLARE_TYPEDEF_PTR(bool, Bool);
    ZS_DECLARE_TYPEDEF_PTR(double, Double);
    ZS_DECLARE_TYPEDEF_PTR(String, String);

    ZS_DECLARE_TYPEDEF_PTR(std::list<String>, StringList);

    //-------------------------------------------------------------------------
    //
    // IConstraints::ConstrainBooleanParameters
    //

    struct ConstrainBooleanParameters {
      Optional<bool> mExact;
      Optional<bool> mIdeal;

      ConstrainBooleanParameters() noexcept {}
      ConstrainBooleanParameters(const ConstrainBooleanParameters &op2) noexcept {(*this) = op2;}
      ConstrainBooleanParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IConstraints::ConstrainBoolean
    //

    struct ConstrainBoolean {
      Optional<bool> mValue;
      Optional<ConstrainBooleanParameters> mParameters;

      ConstrainBoolean() noexcept {}
      ConstrainBoolean(const ConstrainBoolean &op2) noexcept {(*this) = op2;}
      ConstrainBoolean(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IConstraints::ConstrainLongRange
    //

    struct ConstrainLongRange {
      Optional<LONG> mMin;
      Optional<LONG> mMax;
      Optional<LONG> mExact;
      Optional<LONG> mIdeal;

      ConstrainLongRange() noexcept {}
      ConstrainLongRange(const ConstrainLongRange &op2) noexcept {(*this) = op2;}
      ConstrainLongRange(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IConstraints::ConstrainLong
    //

    struct ConstrainLong {
      Optional<LONG> mValue;
      Optional<ConstrainLongRange> mRange;

      ConstrainLong() noexcept {}
      ConstrainLong(const ConstrainLong &op2) noexcept {(*this) = op2;}
      ConstrainLong(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IConstraints::ConstrainDoubleRange
    //

    struct ConstrainDoubleRange {
      Optional<double> mMin;
      Optional<double> mMax;
      Optional<double> mExact;
      Optional<double> mIdeal;

      ConstrainDoubleRange() noexcept {}
      ConstrainDoubleRange(const ConstrainDoubleRange &op2) noexcept {(*this) = op2;}
      ConstrainDoubleRange(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IConstraints::ConstrainDouble
    //

    struct ConstrainDouble {
      Optional<double> mValue;
      Optional<ConstrainDoubleRange> mRange;

      ConstrainDouble() noexcept {}
      ConstrainDouble(const ConstrainDouble &op2) noexcept {(*this) = op2;}
      ConstrainDouble(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IConstraints::StringOrStringList
    //

    struct StringOrStringList {
      Optional<String> mValue;
      Optional<StringList> mValues;

      StringOrStringList() noexcept {}
      StringOrStringList(const StringOrStringList &op2) noexcept {(*this) = op2;}
      StringOrStringList(
                         ElementPtr elem,
                         const char *objectValueName = "value"
                         ) noexcept;

      // {"objectName": {"abc"}}
      // {"objectName": {"objectValueName": ["a","b","c"]}}
      ElementPtr createElement(
                               const char *objectName,
                               const char *objectValueName = "value"
                               ) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IConstraints::ConstraintStringParameters
    //

    struct ConstrainStringParameters {
      Optional<StringOrStringList> mExact;
      Optional<StringOrStringList> mIdeal;

      ConstrainStringParameters() noexcept {}
      ConstrainStringParameters(const ConstrainStringParameters &op2) noexcept {(*this) = op2;}
      ConstrainStringParameters(ElementPtr elem) noexcept;

      ElementPtr createElement(const char *objectName) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;
    };

    //-------------------------------------------------------------------------
    //
    // IConstraints::ConstraintString
    //

    struct ConstrainString {
      Optional<StringOrStringList> mValue;
      Optional<ConstrainStringParameters> mParameters;

      ConstrainString() noexcept {}
      ConstrainString(const ConstrainString &op2) noexcept {(*this) = op2;}
      ConstrainString(
                      ElementPtr elem,
                      const char *objectValueName = "value"
                      ) noexcept;

      ElementPtr createElement(
                               const char *objectName,
                               const char *objectValueName = "value"
                               ) const noexcept;

      ElementPtr toDebug() const noexcept;
      String hash() const noexcept;

      void exact(StringList &values) const noexcept;
      void ideal(StringList &values) const noexcept;
    };

  };
}
