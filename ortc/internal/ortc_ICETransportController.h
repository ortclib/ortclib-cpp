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

#include <ortc/IICETransportController.h>
#include <ortc/IICETransport.h>

#include <zsLib/MessageQueueAssociator.h>

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IICETransportControllerForICETransport)

    ZS_DECLARE_INTERACTION_PROXY(ITransportControllerAsyncDelegate)

    ZS_DECLARE_INTERACTION_PTR(IICETransportForICETransportContoller)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportControllerForICETransport
    #pragma mark

    interaction IICETransportControllerForICETransport
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportControllerForICETransport, ForICETransport)

      static ElementPtr toDebug(ForICETransportPtr controller);

      virtual PUID getID() const = 0;

      virtual PromisePtr notifyWhenUnfrozen(
                                            ICETransportPtr transport,
                                            const String &localFoundation,
                                            const String &remoteFoundation
                                            ) = 0;

      virtual void notifyDetached(ICETransportPtr transport) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ITransportControllerAsyncDelegate
    #pragma mark

    interaction ITransportControllerAsyncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICETransportContoller, UseICETransport)

      typedef size_t AttachedOrderID;

      virtual void onTransportControllerNotifyWhenUnfrozen(
                                                           PromisePtr promise,
                                                           UseICETransportPtr transport,
                                                           const char *localFoundation,
                                                           const char *remoteFoundation
                                                           ) = 0;

      virtual void onTransportControllerNotifyDetached(
                                                       UseICETransportPtr transport,
                                                       AttachedOrderID detachedOrder
                                                       ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransportController
    #pragma mark
    
    class ICETransportController : public Noop,
                                   public MessageQueueAssociator,
                                   public SharedRecursiveLock,
                                   public IICETransportController,
                                   public IICETransportControllerForICETransport,
                                   public ITransportControllerAsyncDelegate,
                                   public IWakeDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IICETransportController;
      friend interaction IICETransportControllerFactory;
      friend interaction IICETransportControllerForICETransport;

      ZS_DECLARE_TYPEDEF_PTR(IICETransportForICETransportContoller, UseICETransport)

      typedef size_t AttachedOrderID;
      typedef std::pair<AttachedOrderID, UseICETransportPtr> AttachedTransportPair;
      typedef std::list<AttachedTransportPair> TransportList;

    public:
      ICETransportController(
                             const make_private &,
                             IMessageQueuePtr queue
                             );

    protected:
      ICETransportController(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~ICETransportController();

      static ICETransportControllerPtr convert(IICETransportControllerPtr object);
      static ICETransportControllerPtr convert(ForICETransportPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransportController => IICETransportController
      #pragma mark

      static ElementPtr toDebug(ICETransportControllerPtr controller);

      static ICETransportControllerPtr create();

      virtual PUID getID() const;

      virtual ICETransportList getTransports() const;
      virtual void addTransport(
                                IICETransportPtr transport,
                                Optional<size_t> index = Optional<size_t>()
                                ) throw(
                                        InvalidParameters,
                                        InvalidStateError
                                        );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransportController => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats(const StatsTypeSet &stats = StatsTypeSet()) const override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransportController => IICETransportControllerForICETransport
      #pragma mark

      // (duplicate) virtual PUID getID() const = 0;

      virtual PromisePtr notifyWhenUnfrozen(
                                            ICETransportPtr transport,
                                            const String &localFoundation,
                                            const String &remoteFoundation
                                            );

      virtual void notifyDetached(ICETransportPtr transport);

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransportController => ITransportControllerAsyncDelegate
      #pragma mark

      virtual void onTransportControllerNotifyWhenUnfrozen(
                                                           PromisePtr promise,
                                                           UseICETransportPtr transport,
                                                           const char *localFoundation,
                                                           const char *remoteFoundation
                                                           );

      virtual void onTransportControllerNotifyDetached(
                                                       UseICETransportPtr transport,
                                                       AttachedOrderID detachedOrder
                                                       );

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransportController => IWakeDelegate
      #pragma mark

      virtual void onWake();

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransportController => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      Log::Params debug(const char *message) const;
      ElementPtr toDebug() const;

      void step();

      void cancel();

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransportController => (data)
      #pragma mark

      AutoPUID mID;
      ICETransportControllerWeakPtr mThisWeak;

      TransportList mTransports;

      std::atomic<AttachedOrderID> mAttachOrder {};
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportControllerFactory
    #pragma mark

    interaction IICETransportControllerFactory
    {
      static IICETransportControllerFactory &singleton();

      virtual ICETransportControllerPtr create();
    };

    class ICETransportControllerFactory : public IFactory<IICETransportControllerFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::ITransportControllerAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::PromisePtr, PromisePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::ITransportControllerAsyncDelegate::UseICETransportPtr, UseICETransportPtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::internal::ITransportControllerAsyncDelegate::AttachedOrderID, AttachedOrderID)
ZS_DECLARE_PROXY_METHOD_4(onTransportControllerNotifyWhenUnfrozen, PromisePtr, UseICETransportPtr, const char *, const char *)
ZS_DECLARE_PROXY_METHOD_2(onTransportControllerNotifyDetached, UseICETransportPtr, AttachedOrderID)
ZS_DECLARE_PROXY_END()
