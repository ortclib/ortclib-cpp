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

#if 0

#include <ortc/internal/ortc_ORTC.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/Log.h>
#include <zsLib/XML.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  namespace internal
  {
    using openpeer::services::IHelper;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IORTCForInternal
    #pragma mark

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueDelegate()
    {
      return (ORTC::singleton())->queueDelegate();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueORTC()
    {
      return (ORTC::singleton())->queueORTC();
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr IORTCForInternal::queueBlockingMediaStartStopThread()
    {
      return (ORTC::singleton())->queueBlockingMediaStartStopThread();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ORTC
    #pragma mark

    //-------------------------------------------------------------------------
    ORTC::ORTC()
    {
      ZS_LOG_DETAIL(log("created"))
    }

    //-------------------------------------------------------------------------
    ORTC::~ORTC()
    {
      mThisWeak.reset();
      ZS_LOG_DETAIL(log("destroyed"))
    }

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::convert(IORTCPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ORTC, object);
    }

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::create()
    {
      ORTCPtr pThis(new ORTC());
      pThis->mThisWeak = pThis;
      return pThis;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ORTC => IORTC
    #pragma mark

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::singleton()
    {
      static ORTCPtr singleton = ORTC::create();
      return singleton;
    }

    //-------------------------------------------------------------------------
    void ORTC::setup(
                     IMessageQueuePtr defaultDelegateMessageQueue,
                     IMessageQueuePtr ortcMessageQueue,
                     IMessageQueuePtr blockingMediaStartStopThread
                     )
    {
      AutoRecursiveLock lock(mLock);

      if (defaultDelegateMessageQueue) {
        mDelegateQueue = defaultDelegateMessageQueue;
      }

      if (ortcMessageQueue) {
        mORTCQueue = ortcMessageQueue;
      }

      if (blockingMediaStartStopThread) {
        mBlockingMediaStartStopThread = blockingMediaStartStopThread;
      }

      ZS_THROW_INVALID_ARGUMENT_IF(!mDelegateQueue)
      ZS_THROW_INVALID_ARGUMENT_IF(!mORTCQueue)
      ZS_THROW_INVALID_ARGUMENT_IF(!mBlockingMediaStartStopThread)
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Stack => IORTCForInternal
    #pragma mark

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueDelegate() const
    {
      return mDelegateQueue;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueORTC() const
    {
      return mORTCQueue;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueBlockingMediaStartStopThread() const
    {
      return mBlockingMediaStartStopThread;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Stack => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params ORTC::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::ORTC");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IORTC
  #pragma mark

  //---------------------------------------------------------------------------
  IORTCPtr IORTC::singleton()
  {
    return internal::ORTC::singleton();
  }
}

#endif //0
