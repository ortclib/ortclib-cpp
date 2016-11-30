/*

 Copyright (c) 2016, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/adapter/types.h>

#include <ortc/services/types.h>

#include <zsLib/IWakeDelegate.h>
#include <zsLib/IFactory.h>
#include <zsLib/Log.h>
#include <zsLib/MessageQueueAssociator.h>

namespace ortc
{
  namespace adapter
  {
    namespace internal
    {
      using zsLib::Noop;
      using zsLib::MessageQueueAssociator;

      using zsLib::Log;
      using zsLib::AutoPUID;
      using zsLib::AutoRecursiveLock;

      using zsLib::IFactory;

      using ortc::services::SharedRecursiveLock;

      using zsLib::IPromiseSettledDelegate;

      ZS_DECLARE_USING_PROXY(zsLib, IWakeDelegate);

      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark (forwards)
      #pragma mark

      ZS_DECLARE_INTERACTION_PTR(ISDPParser);

      ZS_DECLARE_CLASS_PTR(Helper);
      ZS_DECLARE_CLASS_PTR(MediaStream);
      ZS_DECLARE_CLASS_PTR(SDPParser);
      ZS_DECLARE_CLASS_PTR(SessionDescription);
      ZS_DECLARE_CLASS_PTR(PeerConnection);

      ZS_DECLARE_INTERACTION_PROXY(IPeerConnectionAsyncDelegate);

    } // namespace internal
  } // namespace adapter
} // namespace ortc
