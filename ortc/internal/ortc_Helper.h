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


#include <ortc/internal/types.h>
#include <ortc/IHelper.h>

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Helper
    #pragma mark

    class Helper : public IHelper
    {
    public:
      typedef zsLib::CHAR CHAR;
      typedef zsLib::UCHAR UCHAR;
      typedef zsLib::SHORT SHORT;
      typedef zsLib::USHORT USHORT;
      typedef zsLib::LONG LONG;
      typedef zsLib::ULONG ULONG;
      typedef zsLib::LONGLONG LONGLONG;
      typedef zsLib::ULONGLONG ULONGLONG;
      typedef zsLib::INT INT;
      typedef zsLib::UINT UINT;
      typedef zsLib::FLOAT FLOAT;
      typedef zsLib::DOUBLE DOUBLE;

      static Log::Params slog(const char *message);
      static Log::Params slog(
                              const char *logObjectName,
                              const char *message
                              );

      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, bool &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, CHAR &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, UCHAR &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, SHORT &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, USHORT &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, LONG &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, ULONG &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, LONGLONG &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, ULONGLONG &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, INT &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, UINT &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, FLOAT &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, DOUBLE &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, String &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Time &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Milliseconds &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Microseconds &outValue);

      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<bool> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<CHAR> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<UCHAR> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<SHORT> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<USHORT> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<LONG> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<ULONG> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<LONGLONG> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<ULONGLONG> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<INT> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<UINT> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<FLOAT> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<DOUBLE> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<String> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<Time> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<Milliseconds> &outValue);
      static void getElementValue(ElementPtr elem, const char *logObjectName, const char *subElementName, Optional<Microseconds> &outValue);

      static void adoptElementValue(ElementPtr elem, const char *subElementName, bool value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, CHAR value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, UCHAR value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, SHORT value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, USHORT value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, LONG value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, ULONG value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, LONGLONG value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, ULONGLONG value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, INT value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, UINT value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, FLOAT value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, DOUBLE value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const String &value, bool adoptEmptyValue);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Time &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Milliseconds &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Microseconds &value);

      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<bool> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<CHAR> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<UCHAR> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<SHORT> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<USHORT> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<LONG> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<ULONG> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<LONGLONG> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<ULONGLONG> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<INT> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<UINT> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<FLOAT> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<DOUBLE> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<String> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<Time> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<Milliseconds> &value);
      static void adoptElementValue(ElementPtr elem, const char *subElementName, const Optional<Microseconds> &value);

    };
  }
}

