/*

 Copyright (c) 2014, Hookflash Inc.
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

#include <ortc/ISettings.h>
#include <ortc/internal/types.h>

#include <openpeer/services/ISettings.h>

namespace ortc
{
  namespace internal
  {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ISettingsForORTC
    #pragma mark

    interaction ISettingsForORTC
    {
      static void verifyRequiredSettingsOnce() throw (InvalidStateError);
    };

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark Settings
    #pragma mark

    class Settings : public ISettings,
                     public openpeer::services::ISettingsDelegate,
                     public ISettingsForORTC
    {
    protected:
      struct make_private {};

    public:
      friend interaction ISettings;
      friend interaction ISettingsForORTC;

    public:
      Settings(const make_private &);

    public:
      ~Settings();

    protected:
      static SettingsPtr convert(ISettingsPtr setttings);

      static SettingsPtr create();

      static SettingsPtr singleton();

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark Settings => ISettings
      #pragma mark

      virtual void setup(ISettingsDelegatePtr delegate);

      virtual void applyDefaults();

      void verifyRequiredSettings() throw (InvalidStateError);

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark Settings => ISettingsForStack
      #pragma mark

      virtual void verifyRequiredSettingsOnce() throw (InvalidStateError);

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark Settings => services::ISettingsDelegate
      #pragma mark

      virtual String getString(const char *key) const;
      virtual LONG getInt(const char *key) const;
      virtual ULONG getUInt(const char *key) const;
      virtual bool getBool(const char *key) const;
      virtual float getFloat(const char *key) const;
      virtual double getDouble(const char *key) const;

      virtual void setString(
                             const char *key,
                             const char *value
                             );
      virtual void setInt(
                          const char *key,
                          LONG value
                          );
      virtual void setUInt(
                           const char *key,
                           ULONG value
                           );
      virtual void setBool(
                           const char *key,
                           bool value
                           );
      virtual void setFloat(
                            const char *key,
                            float value
                            );
      virtual void setDouble(
                             const char *key,
                             double value
                             );

      virtual void clear(const char *key);

      virtual void clearAll();

    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark Settings => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      static Log::Params slog(const char *message);

      virtual void applyDefaultsIfNoDelegatePresent();

    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark Settings => (data)
      #pragma mark

      mutable RecursiveLock mLock;
      AutoPUID mID;
      SettingsWeakPtr mThisWeak;

      ISettingsDelegatePtr mDelegate;

      bool mAppliedDefaults {};
      bool mVerifiedOnce {};
    };
  }
}
