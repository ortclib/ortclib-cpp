/*

 Copyright (c) 2015, Hookflash Inc.
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

namespace ortc
{
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISettings
  #pragma mark

  interaction ISettings
  {
    static void setup(ISettingsDelegatePtr delegate);

    static void setString(
                          const char *key,
                          const char *value
                          );
    static void setInt(
                       const char *key,
                       LONG value
                       );
    static void setUInt(
                        const char *key,
                        ULONG value
                        );
    static void setBool(
                        const char *key,
                        bool value
                        );
    static void setFloat(
                         const char *key,
                         float value
                         );
    static void setDouble(
                          const char *key,
                          double value
                          );

    static void clear(const char *key);

    static bool apply(const char *jsonSettings);
    static void applyDefaults();

    static void clearAll();

    static void verifySettingExists(const char *key) throw (InvalidStateError);

    static void verifyRequiredSettings() throw (InvalidStateError);

    virtual ~ISettings() {} // to make settings polymorphic
  };

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISettingsDelegate
  #pragma mark

  interaction ISettingsDelegate
  {
    // WARNING: These methods are called synchronously from any thread
    //          and must NOT block on any kind of lock that might be
    //          blocked calling inside to the SDK (directly or indirectly).

    virtual String getString(const char *key) const = 0;
    virtual LONG getInt(const char *key) const = 0;
    virtual ULONG getUInt(const char *key) const = 0;
    virtual bool getBool(const char *key) const = 0;
    virtual float getFloat(const char *key) const = 0;
    virtual double getDouble(const char *key) const = 0;

    virtual void setString(
                           const char *key,
                           const char *value
                           ) = 0;
    virtual void setInt(
                        const char *key,
                        LONG value
                        ) = 0;
    virtual void setUInt(
                         const char *key,
                         ULONG value
                         ) = 0;
    virtual void setBool(
                         const char *key,
                         bool value
                         ) = 0;
    virtual void setFloat(
                          const char *key,
                          float value
                          ) = 0;
    virtual void setDouble(
                           const char *key,
                           double value
                           ) = 0;

    virtual void clear(const char *key) = 0;

    virtual void clearAll() = 0;
  };
}
