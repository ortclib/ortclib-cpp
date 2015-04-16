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

#include <ortc/internal/ortc_ORTC.h>

#include <openpeer/services/IHelper.h>
#include <openpeer/services/IMessageQueueManager.h>

#include <zsLib/Log.h>
#include <zsLib/XML.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IHelper, UseServicesHelper)
    ZS_DECLARE_TYPEDEF_PTR(openpeer::services::IMessageQueueManager, UseMessageQueueManager)

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
    ORTC::ORTC() :
      SharedRecursiveLock(SharedRecursiveLock::create())
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
      static SingletonLazySharedPtr<ORTC> singleton(ORTC::create());
      ORTCPtr result = singleton.singleton();
      if (!result) {
        ZS_LOG_WARNING(Detail, slog("singleton gone"))
      }
      return result;
    }

    //-------------------------------------------------------------------------
    void ORTC::setup(
                     IMessageQueuePtr defaultDelegateMessageQueue,
                     IMessageQueuePtr ortcMessageQueue
                     )
    {
      AutoRecursiveLock lock(mLock);

      if (defaultDelegateMessageQueue) {
        mDelegateQueue = defaultDelegateMessageQueue;
      }

      if (ortcMessageQueue) {
        mORTCQueue = ortcMessageQueue;
      }
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
      AutoRecursiveLock lock(*this);
      if (!mDelegateQueue) {
        mDelegateQueue = UseMessageQueueManager::getMessageQueueForGUIThread();
      }
      return mDelegateQueue;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueORTC() const
    {
      AutoRecursiveLock lock(*this);
      if (!mORTCQueue) {
        mORTCQueue = UseMessageQueueManager::getMessageQueue(ORTC_QUEUE_MAIN_THREAD_NAME);
      }
      return mORTCQueue;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr ORTC::queueBlockingMediaStartStopThread() const
    {
      AutoRecursiveLock lock(*this);
      if (!mBlockingMediaStartStopThread) {
        mBlockingMediaStartStopThread = UseMessageQueueManager::getMessageQueue(ORTC_QUEUE_BLOCKING_MEDIA_STARTUP_THREAD_NAME);
      }
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
      UseServicesHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params ORTC::slog(const char *message)
    {
      ElementPtr objectEl = Element::create("ortc::ORTC");
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
  void IORTC::setup(
                    IMessageQueuePtr defaultDelegateMessageQueue,
                    IMessageQueuePtr ortcMessageQueue
                    )
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->setup(defaultDelegateMessageQueue, ortcMessageQueue);
  }
}
