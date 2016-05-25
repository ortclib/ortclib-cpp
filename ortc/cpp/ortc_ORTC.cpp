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
#include <ortc/internal/ortc_Tracing.h>
#include <ortc/internal/ortc_RTPMediaEngine.h>

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

    void initSubsystems();

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IORTCForInternal
    #pragma mark

    //-------------------------------------------------------------------------
    void IORTCForInternal::overrideQueueDelegate(IMessageQueuePtr queue)
    {
      return (ORTC::singleton())->overrideQueueDelegate(queue);
    }

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
    IMessageQueuePtr IORTCForInternal::queueCertificateGeneration()
    {
      return (ORTC::singleton())->queueCertificateGeneration();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ORTC
    #pragma mark

    //-------------------------------------------------------------------------
    ORTC::ORTC(const make_private &) :
      SharedRecursiveLock(SharedRecursiveLock::create())
    {
      EventRegisterOrtcLib();
      EventRegisterOrtcLibStatsReport();
      EventWriteOrtcCreate(__func__, mID);

      initSubsystems();
      UseServicesHelper::setup();
      zsLib::setup();
      ZS_LOG_DETAIL(log("created"))
    }

    //-------------------------------------------------------------------------
    ORTC::~ORTC()
    {
      mThisWeak.reset();
      ZS_LOG_DETAIL(log("destroyed"))

      EventWriteOrtcDestroy(__func__, mID);
      EventWriteOrtcStatsReportCommand("stop");
      EventUnregisterOrtcLibStatsReport();
      EventUnregisterOrtcLib();
    }

    //-------------------------------------------------------------------------
    void ORTC::init()
    {
    }

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::convert(IORTCPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ORTC, object);
    }

    //-------------------------------------------------------------------------
    ORTCPtr ORTC::create()
    {
      ORTCPtr pThis(make_shared<ORTC>(make_private{}));
      pThis->mThisWeak = pThis;
      pThis->init();
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
      AutoRecursiveLock lock(*UseServicesHelper::getGlobalLock());
      static SingletonLazySharedPtr<ORTC> singleton(ORTC::create());
      ORTCPtr result = singleton.singleton();
      if (!result) {
        ZS_LOG_WARNING(Detail, slog("singleton gone"))
      }
      return result;
    }

    //-------------------------------------------------------------------------
    void ORTC::setup(IMessageQueuePtr defaultDelegateMessageQueue)
    {
      AutoRecursiveLock lock(mLock);

      if (defaultDelegateMessageQueue) {
        mDelegateQueue = defaultDelegateMessageQueue;
      }
    }

    //-------------------------------------------------------------------------
    Milliseconds ORTC::ntpServerTime() const
    {
      AutoRecursiveLock(*this);
      return mNTPServerTime;
    }

    //-------------------------------------------------------------------------
    void ORTC::ntpServerTime(const Milliseconds &value)
    {
      {
        AutoRecursiveLock(*this);
        mNTPServerTime = value;
      }
      auto totalMilliseconds = value.count();
      IRTPMediaEngineForORTC::ntpServerTime(value);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Stack => IORTCForInternal
    #pragma mark

    //-------------------------------------------------------------------------
    void ORTC::overrideQueueDelegate(IMessageQueuePtr queue)
    {
      AutoRecursiveLock lock(*this);
      mDelegateQueue = queue;
    }

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
      return UseMessageQueueManager::getMessageQueue(ORTC_QUEUE_MAIN_THREAD_NAME);
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
    IMessageQueuePtr ORTC::queueCertificateGeneration() const
    {
      AutoRecursiveLock lock(*this);
      if (!mCertificateGeneration) {
        mCertificateGeneration = UseMessageQueueManager::getMessageQueue(ORTC_QUEUE_CERTIFICATE_GENERATION_NAME);
      }
      return mCertificateGeneration;
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
  void IORTC::setup(IMessageQueuePtr defaultDelegateMessageQueue)
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->setup(defaultDelegateMessageQueue);
  }

  //-------------------------------------------------------------------------
  Milliseconds IORTC::ntpServerTime()
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return Milliseconds();
    return singleton->ntpServerTime();
  }

  //-------------------------------------------------------------------------
  void IORTC::ntpServerTime(const Milliseconds &value)
  {
    auto singleton = internal::ORTC::singleton();
    if (!singleton) return;
    singleton->ntpServerTime(value);
  }

}
