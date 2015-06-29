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

#include <ortc/internal/ortc_Settings.h>

#include <ortc/internal/ortc_Certificate.h>
#include <ortc/internal/ortc_ICEGatherer.h>
#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_DTLSTransport.h>
#include <ortc/internal/ortc_SCTPTransport.h>
#include <ortc/internal/ortc_SRTPTransport.h>

#include <openpeer/services/IHelper.h>
#include <openpeer/services/ISettings.h>

#include <zsLib/XML.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettings, UseServicesSettings)
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::ISettingsDelegate, UseServicesSettingsDelegate)

  namespace internal
  {
    using openpeer::services::IHelper;

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark ISettingsForORTC
    #pragma mark

    //-----------------------------------------------------------------------
    void ISettingsForORTC::verifyRequiredSettingsOnce() throw (InvalidStateError)
    {
      SettingsPtr singleton = Settings::singleton();
      if (!singleton) return;
      singleton->verifyRequiredSettingsOnce();
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark Settings
    #pragma mark

    //-----------------------------------------------------------------------
    Settings::Settings(const make_private &)
    {
      ZS_LOG_DETAIL(log("created"))
    }

    //-----------------------------------------------------------------------
    Settings::~Settings()
    {
      mThisWeak.reset();
      ZS_LOG_DETAIL(log("destroyed"))
    }

    //-----------------------------------------------------------------------
    SettingsPtr Settings::convert(ISettingsPtr settings)
    {
      return ZS_DYNAMIC_PTR_CAST(Settings, settings);
    }

    //-----------------------------------------------------------------------
    SettingsPtr Settings::create()
    {
      SettingsPtr pThis(make_shared<Settings>(make_private{}));
      pThis->mThisWeak = pThis;
      return pThis;
    }

    //-----------------------------------------------------------------------
    SettingsPtr Settings::singleton()
    {
      static SingletonLazySharedPtr<Settings> singleton(Settings::create());
      SettingsPtr result = singleton.singleton();
      if (!result) {
        ZS_LOG_WARNING(Detail, slog("singleton gone"))
      }
      return result;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark Settings => ISettings
    #pragma mark

    //-----------------------------------------------------------------------
    void Settings::setup(ISettingsDelegatePtr delegate)
    {
      {
        AutoRecursiveLock lock(mLock);
        mDelegate = delegate;

        ZS_LOG_DEBUG(log("setup called") + ZS_PARAM("has delegate", (bool)delegate))
      }

      UseServicesSettings::setup(delegate ? mThisWeak.lock() : UseServicesSettingsDelegatePtr());
    }

    //-----------------------------------------------------------------------
    void Settings::applyDefaults()
    {
      UseServicesSettings::applyDefaults();

      IICEGathererForSettings::applyDefaults();
      IICETransportForSettings::applyDefaults();
      ICertificateForSettings::applyDefaults();
      IDTLSTransportForSettings::applyDefaults();
      ISCTPTransportForSettings::applyDefaults();
      ISRTPTransportForSettings::applyDefaults();

      {
        AutoRecursiveLock lock(mLock);
        mAppliedDefaults = true;
      }
    }

    //-----------------------------------------------------------------------
    void Settings::verifyRequiredSettings() throw (InvalidStateError)
    {
      applyDefaultsIfNoDelegatePresent();

      // check any required settings here:
      //UseServicesSettings::verifySettingExists(OPENPEER_COMMON_SETTING_APPLICATION_NAME);

      UseServicesSettings::verifyRequiredSettings();
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark Settings => ISettingsForStack
    #pragma mark

    //-----------------------------------------------------------------------
    void Settings::verifyRequiredSettingsOnce() throw (InvalidStateError)
    {
      {
        AutoRecursiveLock lock(mLock);
        if (mVerifiedOnce) return;
      }

      verifyRequiredSettings();

      {
        AutoRecursiveLock lock(mLock);
        mVerifiedOnce = true;
      }
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark Settings => ISettingsDelegate
    #pragma mark

    //-----------------------------------------------------------------------
    String Settings::getString(const char *key) const
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;
      }

      if (!delegate) return UseServicesSettings::getString(key);
      return delegate->getString(key);
    }

    //-----------------------------------------------------------------------
    LONG Settings::getInt(const char *key) const
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;
      }

      if (!delegate) return UseServicesSettings::getInt(key);
      return delegate->getInt(key);
    }

    //-----------------------------------------------------------------------
    ULONG Settings::getUInt(const char *key) const
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;
      }

      if (!delegate) return UseServicesSettings::getUInt(key);
      return delegate->getUInt(key);
    }

    //-----------------------------------------------------------------------
    bool Settings::getBool(const char *key) const
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;
      }

      if (!delegate) return UseServicesSettings::getBool(key);
      return delegate->getBool(key);
    }

    //-----------------------------------------------------------------------
    float Settings::getFloat(const char *key) const
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;
      }

      if (!delegate) return UseServicesSettings::getFloat(key);
      return delegate->getFloat(key);
    }

    //-----------------------------------------------------------------------
    double Settings::getDouble(const char *key) const
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;
      }

      if (!delegate) return UseServicesSettings::getDouble(key);
      return delegate->getDouble(key);
    }

    //-----------------------------------------------------------------------
    void Settings::setString(
                             const char *key,
                             const char *value
                             )
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;
      }

      if (!delegate) {
        UseServicesSettings::setString(key, value);
        return;
      }
      delegate->setString(key, value);
    }

    //-----------------------------------------------------------------------
    void Settings::setInt(
                          const char *key,
                          LONG value
                          )
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;
      }

      if (!delegate) {
        UseServicesSettings::setInt(key, value);
        return;
      }
      delegate->setInt(key, value);
    }

    //-----------------------------------------------------------------------
    void Settings::setUInt(
                           const char *key,
                           ULONG value
                           )
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;
      }

      if (!delegate) {
        UseServicesSettings::setUInt(key, value);
        return;
      }
      delegate->setUInt(key, value);
    }

    //-----------------------------------------------------------------------
    void Settings::setBool(
                           const char *key,
                           bool value
                           )
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;
      }

      if (!delegate) {
        UseServicesSettings::setBool(key, value);
        return;
      }
      delegate->setBool(key, value);
    }

    //-----------------------------------------------------------------------
    void Settings::setFloat(
                            const char *key,
                            float value
                            )
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;
      }

      if (!delegate) {
        UseServicesSettings::setFloat(key, value);
        return;
      }
      delegate->setFloat(key, value);
    }

    //-----------------------------------------------------------------------
    void Settings::setDouble(
                             const char *key,
                             double value
                             )
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;
      }

      if (!delegate) {
        UseServicesSettings::setDouble(key, value);
        return;
      }
      delegate->setDouble(key, value);
    }

    //-----------------------------------------------------------------------
    void Settings::clear(const char *key)
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;
      }

      if (!delegate) {
        UseServicesSettings::clear(key);
        return;
      }
      delegate->clear(key);
    }

    //-----------------------------------------------------------------------
    void Settings::clearAll()
    {
      ISettingsDelegatePtr delegate;
      {
        AutoRecursiveLock lock(mLock);
        delegate = mDelegate;

        mVerifiedOnce = false;
      }

      if (!delegate) {
        UseServicesSettings::clearAll();
        return;
      }
      delegate->clearAll();
    }

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark Settings => (internal)
    #pragma mark

    //-----------------------------------------------------------------------
    Log::Params Settings::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::Settings");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-----------------------------------------------------------------------
    Log::Params Settings::slog(const char *message)
    {
      return Log::Params(message, "ortc::Settings");
    }

    //-----------------------------------------------------------------------
    void Settings::applyDefaultsIfNoDelegatePresent()
    {
      {
        AutoRecursiveLock lock(mLock);
        if (mDelegate) return;

        if (mAppliedDefaults) return;
      }

      ZS_LOG_WARNING(Detail, log("To prevent issues with missing settings, the default settings are being applied. Recommend installing a settings delegate to fetch settings required from a externally."))

      applyDefaults();
    }
    
  }

  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark ISettings
  #pragma mark

  //-------------------------------------------------------------------------
  void ISettings::setup(ISettingsDelegatePtr delegate)
  {
    internal::SettingsPtr singleton = internal::Settings::singleton();
    if (!singleton) return;
    singleton->setup(delegate);
  }

  //-------------------------------------------------------------------------
  String ISettings::getString(const char *key)
  {
    return UseServicesSettings::getString(key);
  }

  //-------------------------------------------------------------------------
  LONG ISettings::getInt(const char *key)
  {
    return UseServicesSettings::getInt(key);
  }

  //-------------------------------------------------------------------------
  ULONG ISettings::getUInt(const char *key)
  {
    return UseServicesSettings::getUInt(key);
  }

  //-------------------------------------------------------------------------
  bool ISettings::getBool(const char *key)
  {
    return UseServicesSettings::getBool(key);
  }

  //-------------------------------------------------------------------------
  float ISettings::getFloat(const char *key)
  {
    return UseServicesSettings::getFloat(key);
  }

  //-------------------------------------------------------------------------
  double ISettings::getDouble(const char *key)
  {
    return UseServicesSettings::getDouble(key);
  }

  //-------------------------------------------------------------------------
  void ISettings::setString(
                            const char *key,
                            const char *value
                            )
  {
    UseServicesSettings::setString(key, value);
  }

  //-------------------------------------------------------------------------
  void ISettings::setInt(
                         const char *key,
                         LONG value
                         )
  {
    UseServicesSettings::setInt(key, value);
  }

  //-------------------------------------------------------------------------
  void ISettings::setUInt(
                          const char *key,
                          ULONG value
                          )
  {
    UseServicesSettings::setUInt(key, value);
  }

  //-------------------------------------------------------------------------
  void ISettings::setBool(
                          const char *key,
                          bool value
                          )
  {
    UseServicesSettings::setBool(key, value);
  }

  //-------------------------------------------------------------------------
  void ISettings::setFloat(
                           const char *key,
                           float value
                           )
  {
    UseServicesSettings::setFloat(key, value);
  }

  //-------------------------------------------------------------------------
  void ISettings::setDouble(
                            const char *key,
                            double value
                            )
  {
    UseServicesSettings::setDouble(key, value);
  }

  //-------------------------------------------------------------------------
  void ISettings::clear(const char *key)
  {
    UseServicesSettings::clear(key);
  }
  
  //-------------------------------------------------------------------------
  bool ISettings::apply(const char *jsonSettings)
  {
    return UseServicesSettings::apply(jsonSettings);
  }

  //-------------------------------------------------------------------------
  void ISettings::applyDefaults()
  {
    internal::SettingsPtr singleton = internal::Settings::singleton();
    if (!singleton) return;
    singleton->applyDefaults();
  }

  //-------------------------------------------------------------------------
  void ISettings::clearAll()
  {
    UseServicesSettings::clearAll();
  }

  //-------------------------------------------------------------------------
  void ISettings::verifySettingExists(const char *key) throw (InvalidStateError)
  {
    UseServicesSettings::verifySettingExists(key);
  }

  //-------------------------------------------------------------------------
  void ISettings::verifyRequiredSettings() throw (InvalidStateError)
  {
    internal::SettingsPtr singleton = internal::Settings::singleton();
    if (!singleton) return;
    singleton->verifyRequiredSettings();
  }
}
