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

namespace ortc
{
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark IConstraints
  #pragma mark
  
  interaction IConstraints
  {
    ZS_DECLARE_STRUCT_PTR(Constraints)
    ZS_DECLARE_STRUCT_PTR(ConstraintSet)
    ZS_DECLARE_STRUCT_PTR(ConstraintBool)
    ZS_DECLARE_STRUCT_PTR(ConstraintBoolParameters)
    ZS_DECLARE_STRUCT_PTR(ConstraintLong)
    ZS_DECLARE_STRUCT_PTR(ConstraintLongRange)
    ZS_DECLARE_STRUCT_PTR(ConstraintDouble)
    ZS_DECLARE_STRUCT_PTR(ConstraintDoubleRange)
    ZS_DECLARE_STRUCT_PTR(ConstraintString)
    ZS_DECLARE_STRUCT_PTR(ConstraintStringParameters)
    ZS_DECLARE_STRUCT_PTR(StringOrStringList)

    ZS_DECLARE_TYPEDEF_PTR(zsLib::LONG, LongPtr)
    ZS_DECLARE_TYPEDEF_PTR(bool, BoolPtr)
    ZS_DECLARE_TYPEDEF_PTR(double, DoublePtr)
    ZS_DECLARE_TYPEDEF_PTR(String, StringPtr)

    ZS_DECLARE_TYPEDEF_PTR(std::list<ConstraintSetPtr>, ConstraintSetList)
    ZS_DECLARE_TYPEDEF_PTR(std::list<String>, StringList)

    static BoolPtr createLong(bool value);
    static LongPtr createLong(LONG value);
    static DoublePtr createDouble(double value);
    static StringOrStringListPtr createString(const String &value);
    static StringOrStringListPtr createString(const StringList &values);

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IConstraints::ConstraintBool
    #pragma mark

    struct ConstraintBool {
      BoolPtr mValue;
      ConstraintBoolParametersPtr mParameters;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IConstraints::ConstraintBoolParameters
    #pragma mark

    struct ConstraintBoolParameters {
      BoolPtr mExact;
      BoolPtr mIdeal;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IConstraints::ConstraintLong
    #pragma mark

    struct ConstraintLong {
      LongPtr mValue;
      ConstraintLongRangePtr mRange;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IConstraints::ConstraintLongRange
    #pragma mark

    struct ConstraintLongRange {
      LongPtr mMin;
      LongPtr mMax;
      LongPtr mExact;
      LongPtr mIdeal;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IConstraints::ConstraintDouble
    #pragma mark

    struct ConstraintDouble {
      DoublePtr mValue;
      ConstraintDoubleRangePtr mRange;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IConstraints::ConstraintDoubleRange
    #pragma mark

    struct ConstraintDoubleRange {
      DoublePtr mMin;
      DoublePtr mMax;
      DoublePtr mExact;
      DoublePtr mIdeal;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IConstraints::ConstraintString
    #pragma mark

    struct ConstraintString {
      StringOrStringListPtr mValue;
      ConstraintStringParametersPtr mParameters;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IConstraints::ConstraintStringParameters
    #pragma mark

    struct ConstraintStringParameters {
      StringOrStringListPtr mExact;
      StringOrStringListPtr mIdeal;
    };

    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IConstraints::StringOrStringList
    #pragma mark

    struct StringOrStringList {
      StringPtr mValue;
      StringListPtr mValues;
    };
  };
}
