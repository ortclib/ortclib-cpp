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

#include <ortc/ICapabilities.h>

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
  #pragma mark ICapabilities::CapabilityBool
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityBool::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ICapabilities::CapabilityBool");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto value = (*iter);
      UseServicesHelper::debugAppend(resultEl, "value", value ? "true" : "false");
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICapabilities::CapabilityBool::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::ICapabilities::CapabilityBool:");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto value = (*iter);
      hasher.update(value);
    }

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ICapabilities::CapabilityLong
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityLong::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ICapabilities::CapabilityLong");

    if (mMin != mMax) {
      UseServicesHelper::debugAppend(resultEl, "min", mMin);
      UseServicesHelper::debugAppend(resultEl, "min", mMax);
    } else {
      UseServicesHelper::debugAppend(resultEl, "value", mMin);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICapabilities::CapabilityLong::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::ICapabilities::CapabilityLong:");

    hasher.update(mMin);
    hasher.update(":");
    hasher.update(mMax);

    return hasher.final();
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ICapabilities::CapabilityDouble
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityDouble::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ICapabilities::CapabilityDouble");

    if (mMin != mMax) {
      UseServicesHelper::debugAppend(resultEl, "min", mMin);
      UseServicesHelper::debugAppend(resultEl, "min", mMax);
    } else {
      UseServicesHelper::debugAppend(resultEl, "value", mMin);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICapabilities::CapabilityDouble::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::ICapabilities::CapabilityDouble:");

    hasher.update(mMin);
    hasher.update(":");
    hasher.update(mMax);

    return hasher.final();
  }


  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark ICapabilities::CapabilityString
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr ICapabilities::CapabilityString::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::ICapabilities::CapabilityString");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto value = (*iter);
      UseServicesHelper::debugAppend(resultEl, "value", value);
    }

    return resultEl;
  }

  //---------------------------------------------------------------------------
  String ICapabilities::CapabilityString::hash() const
  {
    SHA1Hasher hasher;

    hasher.update("ortc::ICapabilities::CapabilityString:");

    for (auto iter = begin(); iter != end(); ++iter)
    {
      auto value = (*iter);
      hasher.update(":");
      hasher.update(value);
    }

    return hasher.final();
  }

}
