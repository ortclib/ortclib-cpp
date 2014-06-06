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

#include <ortc/internal/types.h>
#include <ortc/IORTC.h>

namespace ortc
{
  namespace internal
  {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IORTCForInternal
    #pragma mark

    interaction IORTCForInternal
    {
      ZS_DECLARE_TYPEDEF_PTR(IORTCForInternal, ForInternal)

      static IMessageQueuePtr queueDelegate();
      static IMessageQueuePtr queueORTC();
      static IMessageQueuePtr queueBlockingMediaStartStopThread();
    };

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark Stack
    #pragma mark

    class ORTC : public IORTC,
                 public IORTCForInternal
    {
    public:
      friend interaction IORTC;
      friend interaction IORTCForInternal;

    protected:
      ORTC();

    public:
      virtual ~ORTC();

      static ORTCPtr create();
      static ORTCPtr convert(IORTCPtr object);

    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark ORTC => IORTC
      #pragma mark

      static ORTCPtr singleton();

      virtual PUID getID() const {return mID;}

      virtual void setup(
                         IMessageQueuePtr defaultDelegateMessageQueue,
                         IMessageQueuePtr ortcMessageQueue,
                         IMessageQueuePtr blockingMediaStartStopThread
                         );

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark ORTC => IORTCForInternal
      #pragma mark

      virtual IMessageQueuePtr queueDelegate() const;
      virtual IMessageQueuePtr queueORTC() const;
      virtual IMessageQueuePtr queueBlockingMediaStartStopThread() const;

      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark ORTC => (internal)
      #pragma mark

      Log::Params log(const char *message) const;

    protected:
      //---------------------------------------------------------------------
      #pragma mark
      #pragma mark ORTC => (data)
      #pragma mark

      AutoPUID mID;
      ORTCWeakPtr mThisWeak;
      RecursiveLock mLock;

      IMessageQueuePtr mORTCQueue;
      IMessageQueuePtr mDelegateQueue;
      IMessageQueuePtr mBlockingMediaStartStopThread;
    };
  }
}
