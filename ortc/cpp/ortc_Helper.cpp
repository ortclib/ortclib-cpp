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


#include <ortc/internal/ortc_Helper.h>

#include <ortc/services/IHelper.h>

#include <zsLib/Log.h>
#include <zsLib/Numeric.h>
#include <zsLib/Stringize.h>
#include <zsLib/XML.h>


#ifdef _WIN32
namespace std {
  inline time_t mktime(struct tm *timeptr) { return ::mktime(timeptr); }
}
#endif //_WIN32

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  namespace internal
  {
    typedef ortc::services::IHelper UseServicesHelper;

    using zsLib::Log;
    using zsLib::Numeric;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params Helper::slog(const char *message)
    {
      return Log::Params(message, "ortc::Helper");
    }

    //-------------------------------------------------------------------------
    Log::Params Helper::slog(
                             const char *logObjectName,
                             const char *message
                             )
    {
      return Log::Params(message, logObjectName);
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 bool &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<bool>(str);
        } catch(const Numeric<bool>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 CHAR &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<CHAR>(str);
        } catch(const Numeric<CHAR>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 UCHAR &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<UCHAR>(str);
        } catch(const Numeric<UCHAR>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 SHORT &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<SHORT>(str);
        } catch(const Numeric<SHORT>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 USHORT &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<USHORT>(str);
        } catch(const Numeric<USHORT>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 LONG &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<LONG>(str);
        } catch(const Numeric<LONG>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 ULONG &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<ULONG>(str);
        } catch(const Numeric<ULONG>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 LONGLONG &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<LONGLONG>(str);
        } catch(const Numeric<LONGLONG>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 ULONGLONG &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<ULONGLONG>(str);
        } catch(const Numeric<ULONGLONG>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }
    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 INT &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<INT>(str);
        } catch(const Numeric<INT>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 UINT &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<UINT>(str);
        } catch(const Numeric<UINT>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 FLOAT &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<FLOAT>(str);
        } catch(const Numeric<FLOAT>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 DOUBLE &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<DOUBLE>(str);
        } catch(const Numeric<DOUBLE>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 String &outValue
                                 )
    {
      if (!elem) return;
      outValue = UseServicesHelper::getElementTextAndDecode(elem->findFirstChildElement(subElementName));
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Time &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<Time>(str);
        } catch(const Numeric<Time>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Milliseconds &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<Milliseconds>(str);
        } catch(const Numeric<Milliseconds>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Microseconds &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<Microseconds>(str);
        } catch(const Numeric<Microseconds>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<bool> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<decltype(outValue.mType)>(str);
        } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<CHAR> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<decltype(outValue.mType)>(str);
        } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<UCHAR> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<decltype(outValue.mType)>(str);
        } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<SHORT> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<decltype(outValue.mType)>(str);
        } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<USHORT> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<decltype(outValue.mType)>(str);
        } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<LONG> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<decltype(outValue.mType)>(str);
        } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<ULONG> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<decltype(outValue.mType)>(str);
        } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<LONGLONG> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<decltype(outValue.mType)>(str);
        } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<ULONGLONG> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<decltype(outValue.mType)>(str);
        } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<INT> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<decltype(outValue.mType)>(str);
        } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<UINT> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<decltype(outValue.mType)>(str);
        } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<FLOAT> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<decltype(outValue.mType)>(str);
        } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<DOUBLE> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<decltype(outValue.mType)>(str);
        } catch(const Numeric<decltype(outValue.mType)>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<String> &outValue
                                 )
    {
      if (!elem) return;
      ElementPtr subEl = elem->findFirstChildElement(subElementName);
      if (subEl) {
        outValue = UseServicesHelper::getElementTextAndDecode(elem->findFirstChildElement(subElementName));
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<Time> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<Time>(str);
        } catch(const Numeric<Time>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<Milliseconds> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<Milliseconds>(str);
        } catch(const Numeric<Milliseconds>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::getElementValue(
                                 ElementPtr elem,
                                 const char *logObjectName,
                                 const char *subElementName,
                                 Optional<Microseconds> &outValue
                                 )
    {
      if (!elem) return;
      String str = UseServicesHelper::getElementText(elem->findFirstChildElement(subElementName));
      if (str.hasData()) {
        try {
          outValue = Numeric<Microseconds>(str);
        } catch(const Numeric<Microseconds>::ValueOutOfRange &) {
          ZS_LOG_WARNING(Debug, slog(logObjectName, (String(subElementName) + "value out of range").c_str()) + ZS_PARAM("value", str))
        }
      }
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   bool value
                                   )
    {
      if (!elem) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   CHAR value
                                   )
    {
      if (!elem) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(static_cast<INT>(value))));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   UCHAR value
                                   )
    {
      if (!elem) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   SHORT value
                                   )
    {
      if (!elem) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   USHORT value
                                   )
    {
      if (!elem) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   LONG value
                                   )
    {
      if (!elem) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   ULONG value
                                   )
    {
      if (!elem) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   LONGLONG value
                                   )
    {
      if (!elem) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   ULONGLONG value
                                   )
    {
      if (!elem) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   INT value
                                   )
    {
      if (!elem) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   UINT value
                                   )
    {
      if (!elem) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   FLOAT value
                                   )
    {
      if (!elem) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   DOUBLE value
                                   )
    {
      if (!elem) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const String &value,
                                   bool adoptEmptyValue
                                   )
    {
      if (!elem) return;

      if (value.isEmpty()) {
        if (!adoptEmptyValue) return;
      }
      elem->adoptAsLastChild(UseServicesHelper::createElementWithTextAndJSONEncode(subElementName, value));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Time &value
                                   )
    {
      if (!elem) return;
      if (Time() == value) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Milliseconds &value
                                   )
    {
      if (!elem) return;
      if (Milliseconds() == value) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Microseconds &value
                                   )
    {
      if (!elem) return;
      if (Microseconds() == value) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value)));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<bool> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<CHAR> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(static_cast<INT>(value.value()))));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<UCHAR> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<SHORT> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<USHORT> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<LONG> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<ULONG> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<LONGLONG> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<ULONGLONG> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<INT> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<UINT> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<FLOAT> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<DOUBLE> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<String> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithTextAndJSONEncode(subElementName, value.value()));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<Time> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<Milliseconds> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

    //-------------------------------------------------------------------------
    void Helper::adoptElementValue(
                                   ElementPtr elem,
                                   const char *subElementName,
                                   const Optional<Microseconds> &value
                                   )
    {
      if (!elem) return;
      if (!value.hasValue()) return;
      elem->adoptAsLastChild(UseServicesHelper::createElementWithNumber(subElementName, string(value.value())));
    }

  }  //ortc::internal

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IHelper
  #pragma mark

}

