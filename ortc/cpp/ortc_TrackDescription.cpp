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

#include <ortc/TrackDescription.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/Log.h>
#include <zsLib/XML.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark TrackDescription
  #pragma mark

  //---------------------------------------------------------------------------
  TrackDescriptionPtr TrackDescription::create()
  {
    return TrackDescriptionPtr(new TrackDescription);
  }

  //---------------------------------------------------------------------------
  ElementPtr TrackDescription::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::TrackDescription");

    for (FlowList::const_iterator iter = mFlows.begin(); iter != mFlows.end(); ++iter)
    {
      RTPFlowParamsPtr flow = (*iter);
      UseServicesHelper::debugAppend(resultEl, "flow", flow->toDebug());
    }
    return resultEl;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPFlowParams
  #pragma mark

  RTPFlowParams::RTPFlowParams() :
    mID(zsLib::createPUID()),
    mSSRC(static_cast<DWORD>(UseServicesHelper::random(0, 0xFFFFFFFF)))
  {
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPRTXFlowParams
  #pragma mark

  //---------------------------------------------------------------------------
  RTPRTXFlowParamsPtr RTPRTXFlowParams::create()
  {
    RTPRTXFlowParamsPtr pThis(new RTPRTXFlowParams);

    return pThis;
  }

  //---------------------------------------------------------------------------
  ElementPtr RTPRTXFlowParams::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::RTPRTXFlowParams");

    UseServicesHelper::debugAppend(resultEl, "id", mID);
    UseServicesHelper::debugAppend(resultEl, "ssrc", mSSRC);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPFECFlowParams
  #pragma mark

  //---------------------------------------------------------------------------
  RTPFECFlowParamsPtr RTPFECFlowParams::create()
  {
    RTPFECFlowParamsPtr pThis(new RTPFECFlowParams);

    return pThis;
  }

  //---------------------------------------------------------------------------
  ElementPtr RTPFECFlowParams::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::RTPFECFlowParams");

    UseServicesHelper::debugAppend(resultEl, "id", mID);
    UseServicesHelper::debugAppend(resultEl, "ssrc", mSSRC);
    UseServicesHelper::debugAppend(resultEl, "mechanism", mMechanismg);

    return resultEl;
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPFECFlowParams
  #pragma mark

  //---------------------------------------------------------------------------
  RTPLayeredFlowParamsPtr RTPLayeredFlowParams::create()
  {
    RTPLayeredFlowParamsPtr pThis(new RTPLayeredFlowParams);

    return pThis;
  }

  //---------------------------------------------------------------------------
  ElementPtr RTPLayeredFlowParams::toDebug() const
  {
    ElementPtr resultEl = Element::create("ortc::RTPLayeredFlowParams");

    UseServicesHelper::debugAppend(resultEl, "id", mID);
    UseServicesHelper::debugAppend(resultEl, "ssrc", mSSRC);

    ElementPtr baseFlowsEl = Element::create("base flow ids");

    for (BaseFlowIDList::const_iterator iter = mBaseFlowIDs.begin(); iter != mBaseFlowIDs.end(); ++iter)
    {
      const BaseFlowID &value = (*iter);
      UseServicesHelper::debugAppend(baseFlowsEl, "id", value);
    }

    resultEl->adoptAsLastChild(baseFlowsEl);

    return resultEl;
  }
}
