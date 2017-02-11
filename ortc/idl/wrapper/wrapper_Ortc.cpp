/*

 Copyright (c) 2017, Optical Tone Ltd.
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


#include <zsLib/types.h>
#include <zsLib/String.h>

#include "generated/types.h"

#if 0
#include "generated/org_ortc_Ortc.h"

using std::make_shared;

namespace wrapper
{
  namespace org
  {
    namespace ortc
    {
      ZS_DECLARE_STRUCT_PTR(Ortc);
      ZS_DECLARE_STRUCT_PTR(Dispatcher);

      namespace internal
      {
        struct Ortc : public wrapper::org::ortc::Ortc
        {
          Ortc(zs::MessageQueuePtr queue);
          Ortc(DispatcherPtr queue);

          virtual zs::Milliseconds ntpServerTime() override
          {
          }

          virtual void ntpServerTime(zs::Milliseconds value) override
          {
          }

          virtual void setDefaultLogLevel(zs::log::Level level) override
          {
          }

          virtual void setLogLevel(String componenet, zs::log::Level level) override
          {
          }

          virtual void setDefaultEventingLevel(zs::log::Level level) override
          {
          }

          virtual void setEventingLevel(String componenet, zs::log::Level level) override
          {
          }

          virtual void startMediaTracing() override
          {
          }

          virtual void stopMediaTracing() override
          {
          }

          virtual bool isMediaTracing() override
          {
            return false;
          }

          virtual bool saveMediaTrace(String filename) override
          {
            return false;
          }

          virtual bool saveMediaTrace(String host, int port) override
          {
            return false;
          }
        };      
      }

      OrtcPtr Ortc::Factory::create(zs::MessageQueuePtr queue)
      {
        return make_shared<internal::Ortc>(queue);
      }

      OrtcPtr Ortc::Factory::create(DispatcherPtr dispatcher)
      {
        return make_shared<internal::Ortc>(dispatcher);
      }

    }
  }
}

#endif //0
