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

#include <ortc/internal/ortc_ICETransportController.h>
#include <ortc/internal/ortc_ICETransport.h>
#include <ortc/internal/ortc_ORTC.h>
#include <ortc/internal/ortc.events.h>
#include <ortc/internal/ortc_StatsReport.h>
#include <ortc/internal/platform.h>

#include <ortc/IHelper.h>

#include <ortc/services/IHTTP.h>

#include <zsLib/Stringize.h>
#include <zsLib/Log.h>
#include <zsLib/XML.h>

#include <cryptopp/sha.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib_icetransport_controller) }

namespace ortc
{
  ZS_DECLARE_TYPEDEF_PTR(ortc::services::IHTTP, UseHTTP);
  ZS_DECLARE_TYPEDEF_PTR(internal::IStatsReportForInternal, UseStatsReport);

  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helpers)
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportControllerForICETransport
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr IICETransportControllerForICETransport::toDebug(ForICETransportPtr transport)
    {
      return ICETransportController::toDebug(ICETransportController::convert(transport));
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransportController
    #pragma mark
    
    //-------------------------------------------------------------------------
    ICETransportController::ICETransportController(
                                                   const make_private &,
                                                   IMessageQueuePtr queue
                                                   ) :
      MessageQueueAssociator(queue),
      SharedRecursiveLock(SharedRecursiveLock::create())
    {
      ZS_EVENTING_1(x, i, Detail, IceTransportControllerCreate, ol, IceTransportController, Start, puid, id, mID);

      ZS_LOG_DETAIL(debug("created"))
    }

    //-------------------------------------------------------------------------
    void ICETransportController::init()
    {
    }

    //-------------------------------------------------------------------------
    ICETransportController::~ICETransportController()
    {
      if (isNoop()) return;

      ZS_LOG_DETAIL(log("destroyed"))
      mThisWeak.reset();
      cancel();
      ZS_EVENTING_1(x, i, Detail, IceTransportControllerDestroy, ol, IceTransportController, Stop, puid, id, mID);
    }

    //-------------------------------------------------------------------------
    ICETransportControllerPtr ICETransportController::convert(IICETransportControllerPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICETransportController, object);
    }

    //-------------------------------------------------------------------------
    ICETransportControllerPtr ICETransportController::convert(ForICETransportPtr object)
    {
      return ZS_DYNAMIC_PTR_CAST(ICETransportController, object);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransportController => IICETransportController
    #pragma mark
    
    //-------------------------------------------------------------------------
    ElementPtr ICETransportController::toDebug(ICETransportControllerPtr transport)
    {
      if (!transport) return ElementPtr();
      return transport->toDebug();
    }

    //-------------------------------------------------------------------------
    ICETransportControllerPtr ICETransportController::create()
    {
      ICETransportControllerPtr pThis(make_shared<ICETransportController>(make_private{}, IORTCForInternal::queueORTC()));
      pThis->mThisWeak.lock();
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    PUID ICETransportController::getID() const
    {
      return mID;
    }

    //-------------------------------------------------------------------------
    IICETransportController::ICETransportList ICETransportController::getTransports() const
    {
      ICETransportList result;

      AutoRecursiveLock lock(*this);

      for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter)
      {
        auto transport = (*iter).second;

        result.push_back(ICETransport::convert(transport));
      }

      return result;
    }

    //-------------------------------------------------------------------------
    void ICETransportController::addTransport(
                                              IICETransportPtr inTransport,
                                              Optional<size_t> index
                                              ) throw(
                                                      InvalidParameters,
                                                      InvalidStateError
                                                      )
    {
      UseICETransportPtr transport = ICETransport::convert(inTransport);

      ORTC_THROW_INVALID_PARAMETERS_IF(!transport)
      ORTC_THROW_INVALID_PARAMETERS_IF(IICETypes::Component_RTCP == transport->component())
      ORTC_THROW_INVALID_STATE_IF(IICETransport::State_Closed == transport->state())

      AttachedOrderID order = ++mAttachOrder;

      {
        AutoRecursiveLock lock(*this);

        // make sure the transport isn't already attached
        for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter) {
          auto existingTransport = (*iter).second;
          ORTC_THROW_INVALID_PARAMETERS_IF(transport->getID() == existingTransport->getID())
        }

        // now that it's not attached, insert the transport
        if (!index.hasValue()) {
          ZS_LOG_DETAIL(log("adding transport to controller") + UseICETransport::toDebug(transport));
          ZS_EVENTING_5(
                        x, i, Detail, IceTransportControllerInternalTransportAttachedEvent, ol, IceTransportController, InternalEvent,
                        puid, id, mID,
                        puid, iceTransportObjectId, transport->getID(),
                        size_t, attachedOrderId, order,
                        bool, hasInsertIndex, false,
                        size_t, insertIndex, 0
                        );

          mTransports.push_back(AttachedTransportPair(order, transport));
          goto attached;
        }

        size_t loop = 0;
        auto dest = index.value();
        for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter, ++loop) {
          if (loop == dest) {
            ZS_LOG_DETAIL(log("adding transport to controller") + UseICETransport::toDebug(transport) + ZS_PARAM("index", dest));
            ZS_EVENTING_5(
                          x, i, Detail, IceTransportControllerInternalTransportAttachedEvent, ol, IceTransportController, InternalEvent,
                          puid, id, mID,
                          puid, iceTransportObjectId, transport->getID(),
                          size_t, attachedOrderId, order,
                          bool, hasInsertIndex, true,
                          size_t, insertIndex, index.value()
                          );
            mTransports.insert(iter, AttachedTransportPair(order, transport));
            goto attached;
          }
        }

        ORTC_THROW_INVALID_PARAMETERS(String("Index passed in was not valid, index=") + string(dest) + ", total=" + string(mTransports.size()))
      }

    attached:
      {
        transport->notifyControllerAttached(mThisWeak.lock());
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransportController => IStatsProvider
    #pragma mark

    //-------------------------------------------------------------------------
    ICETransportController::PromiseWithStatsReportPtr ICETransportController::getStats(const StatsTypeSet &stats) const
    {
      UseStatsReport::PromiseWithStatsReportList promises;

      {
        AutoRecursiveLock lock(*this);
        for (auto iter = mTransports.begin(); iter != mTransports.end(); ++iter) {
          IICETransportPtr transport = ICETransport::convert((*iter).second);

          auto promise = transport->getStats(stats);
          promises.push_back(promise);
        }
      }

      return UseStatsReport::collectReports(promises);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransportController => IICETransportControllerForICETransport
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransportController::notifyDetached(ICETransportPtr inTransport)
    {
      UseICETransportPtr transport = inTransport;

      AttachedOrderID order = ++mAttachOrder;

      ITransportControllerAsyncDelegateProxy::create(mThisWeak.lock())->onTransportControllerNotifyDetached(transport, order);
    }

    //-------------------------------------------------------------------------
    PromisePtr ICETransportController::notifyWhenUnfrozen(
                                                          ICETransportPtr transport,
                                                          const String &localFoundation,
                                                          const String &remoteFoundation
                                                          )
    {
      PromisePtr promise = Promise::create(IORTCForInternal::queueORTC());
      ITransportControllerAsyncDelegateProxy::create(mThisWeak.lock())->onTransportControllerNotifyWhenUnfrozen(promise, transport, localFoundation, remoteFoundation);
      return promise;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransportController => ITransportControllerAsyncDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransportController::onTransportControllerNotifyWhenUnfrozen(
                                                                         PromisePtr promise,
                                                                         UseICETransportPtr transport,
                                                                         const char *localFoundation,
                                                                         const char *remoteFoundation
                                                                         )
    {
      String localFoundationStr(localFoundation);
      String remoteFoundationStr(remoteFoundation);

      {
        AutoRecursiveLock lock(*this);
        for (auto iter_doNotUse = mTransports.begin(); iter_doNotUse != mTransports.end(); ) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto compareTransport = (*current).second;
          if (transport == compareTransport) {
            // this transport is not dependent upon any existing transport
            goto not_dependent;
          }

          if (!compareTransport->hasCandidatePairFoundation(localFoundationStr, remoteFoundationStr, promise)) continue;

          ZS_EVENTING_4(
                        x, i, Debug, IceTransportControllerWaitUntilUnfrozen, ol, IceTransportController, Info,
                        puid, id, mID,
                        puid, iceTransportObjectId, transport->getID(),
                        string, localFoundation, localFoundation,
                        string, remoteFoundation, remoteFoundation
                        );

          ZS_LOG_TRACE(log("freezing candidate based on high priority trasport") + ZS_PARAM("frozen transport", transport->getID()) + ZS_PARAM("frozen against transport id", compareTransport->getID()) + ZS_PARAMIZE(localFoundation) + ZS_PARAMIZE(remoteFoundation))
          return;
        }
        goto not_dependent;
      }

    not_dependent:
      {
        ZS_EVENTING_4(
                      x, i, Debug, IceTransportControllerNoNeedToWaitUntilUnfrozen, ol, IceTransportController, Info,
                      puid, id, mID,
                      puid, iceTransportObjectId, transport->getID(),
                      string, localFoundation, localFoundation,
                      string, remoteFoundation, remoteFoundation
                      );

        ZS_LOG_TRACE(log("no transport to feeze against") + ZS_PARAM("transport", transport->getID()) + ZS_PARAMIZE(localFoundation) + ZS_PARAMIZE(remoteFoundation))
        promise->resolve();
      }
    }

    //-------------------------------------------------------------------------
    void ICETransportController::onTransportControllerNotifyDetached(
                                                                     UseICETransportPtr transport,
                                                                     AttachedOrderID detachedOrder
                                                                     )
    {
      ZS_EVENTING_3(
                    x, i, Detail, IceTransportControllerInternalTransportDetachedEvent, ol, IceTransportController, InternalEvent,
                    puid, id, mID,
                    puid, iceTransportObjectId, transport->getID(),
                    size_t, detachedOrder, detachedOrder
                    );

      {
        AutoRecursiveLock lock(*this);

        for (auto iter_doNotUse = mTransports.begin(); iter_doNotUse != mTransports.end(); ) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto attachedOrderID = (*current).first;
          auto compareTransport = (*current).second;

          if (transport != compareTransport) continue;

          if (detachedOrder < attachedOrderID) {
            ZS_LOG_WARNING(Detail, log("cannot detach transport because it was added after it was notified of being detached") + ZS_PARAM("transport id", transport->getID()))
            return;
          }

          mTransports.erase(current);
        }
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransportController => IWakeDelegate
    #pragma mark

    //-------------------------------------------------------------------------
    void ICETransportController::onWake()
    {
      ZS_EVENTING_1(x, i, Debug, IceTransportControllerInternalWakeEvent, ol, IceTransportController, Info, puid, id, mID);
      ZS_LOG_DEBUG(log("wake"));

      AutoRecursiveLock lock(*this);
      step();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark ICETransportController => (internal)
    #pragma mark

    //-------------------------------------------------------------------------
    Log::Params ICETransportController::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::ICETransportController");
      IHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    Log::Params ICETransportController::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr ICETransportController::toDebug() const
    {
      ElementPtr resultEl = Element::create("ortc::ICETransportController");

      IHelper::debugAppend(resultEl, "id", mID);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    void ICETransportController::step()
    {
      ZS_LOG_DEBUG(debug("step"));
      ZS_EVENTING_1(x, i, Debug, IceTransportControllerStep, ol, IceTransportController, Step, puid, id, mID);
    }

    //-------------------------------------------------------------------------
    void ICETransportController::cancel()
    {
      ZS_EVENTING_1(x, i, Debug, IceTransportControllerCancel, ol, IceTransportController, Cancel, puid, id, mID);

      //.......................................................................
      // try to gracefully shutdown

      //.......................................................................
      // final cleanup

      for (auto iter_doNotUse = mTransports.begin(); iter_doNotUse != mTransports.end(); ) {
        auto current = iter_doNotUse;
        ++iter_doNotUse;

        auto transport = (*current).second;

        transport->notifyControllerDetached(*this);
      }
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportControllerFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IICETransportControllerFactory &IICETransportControllerFactory::singleton()
    {
      return ICETransportControllerFactory::singleton();
    }

    //-------------------------------------------------------------------------
    ICETransportControllerPtr IICETransportControllerFactory::create()
    {
      if (this) {}
      return internal::ICETransportController::create();
    }

  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransportControllerTypes
  #pragma mark

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  #pragma mark
  #pragma mark IICETransportController
  #pragma mark

  //---------------------------------------------------------------------------
  ElementPtr IICETransportController::toDebug(IICETransportControllerPtr transport)
  {
    return internal::ICETransportController::toDebug(internal::ICETransportController::convert(transport));
  }

  //---------------------------------------------------------------------------
  IICETransportControllerPtr IICETransportController::create()
  {
    return internal::IICETransportControllerFactory::singleton().create();
  }


}
