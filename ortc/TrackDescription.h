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

namespace ortc
{
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark TrackDescription
  #pragma mark

  struct TrackDescription
  {
    typedef std::list<RTPFlowParamsPtr> FlowList;

    FlowList mFlows;

    static TrackDescriptionPtr create();
    ElementPtr toDebug() const;

  protected:
    TrackDescription() {}
    TrackDescription(const TrackDescription &) {}
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPFlowParams
  #pragma mark

  struct RTPFlowParams
  {
    typedef DWORD SSRC;

    enum Types
    {
      Type_RTX,
      Type_FEC,
      Type_Layerd,
    };

    PUID mID;
    SSRC mSSRC;

    virtual ElementPtr toDebug() const = 0;
    virtual Types getType() const = 0;

  protected:
    RTPFlowParams();
    RTPFlowParams(const RTPFlowParams &) {}
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPRTXFlowParams
  #pragma mark

  struct RTPRTXFlowParams : public RTPFlowParams
  {
#ifndef _ANDROID	  
    static RTPRTXFlowParamsPtr convert(RTPFlowParams object);
#endif	  

    static RTPRTXFlowParamsPtr create();
    virtual ElementPtr toDebug() const;

    virtual Types getType() const {return RTPFlowParams::Type_RTX;}

  protected:
    RTPRTXFlowParams() {}
    RTPRTXFlowParams(const RTPRTXFlowParams &) {}
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPFECFlowParams
  #pragma mark

  struct RTPFECFlowParams : public RTPFlowParams
  {
    String mMechanismg;
#ifndef _ANDROID
    static RTPRTXFlowParamsPtr convert(RTPFlowParams object);
#endif
    static RTPFECFlowParamsPtr create();
    virtual ElementPtr toDebug() const;

    virtual Types getType() const {return RTPFlowParams::Type_FEC;}

  protected:
    RTPFECFlowParams() {}
    RTPFECFlowParams(const RTPFECFlowParams &) {}
  };

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark RTPLayeredFlowParams
  #pragma mark

  struct RTPLayeredFlowParams : public RTPFlowParams
  {
    typedef PUID BaseFlowID;

    typedef std::list<BaseFlowID> BaseFlowIDList;

    BaseFlowIDList mBaseFlowIDs;
#ifndef _ANDROID
    static RTPRTXFlowParamsPtr convert(RTPFlowParams object);
#endif
    static RTPLayeredFlowParamsPtr create();
    virtual ElementPtr toDebug() const;

    virtual Types getType() const {return RTPFlowParams::Type_Layerd;}

  protected:
    RTPLayeredFlowParams() {}
    RTPLayeredFlowParams(const RTPLayeredFlowParams &) {}
  };

}
