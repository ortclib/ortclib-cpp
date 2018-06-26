

#ifndef C_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER

#include <wrapper/generated/c/c_helpers.h>
#include <zsLib/types.h>
#include <zsLib/eventing/types.h>
#include <zsLib/SafeInt.h>

#include <wrapper/generated/c/c_org_ortc_EventQueueMaker.h>
#include <wrapper/generated/org_ortc_EventQueueMaker.h>
#include <wrapper/generated/c/c_org_ortc_EventQueue.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wheader-hygiene"
#endif /* __clang__ */

using namespace wrapper;

#if 0
//------------------------------------------------------------------------------
org_ortc_EventQueue_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueueMaker_bindQueue(zs_Any_t queue)
{
  return wrapper::org_ortc_EventQueue_wrapperToHandle(wrapper::org::ortc::EventQueueMaker::bindQueue(wrapper::zs_Any_wrapperFromHandle(queue)));
}

//------------------------------------------------------------------------------
zs_Any_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueueMaker_extractQueue(org_ortc_EventQueue_t queue)
{
  return wrapper::zs_Any_wrapperToHandle(wrapper::org::ortc::EventQueueMaker::extractQueue(wrapper::org_ortc_EventQueue_wrapperFromHandle(queue)));
}
#endif //0

namespace wrapper
{
#if 0
  //----------------------------------------------------------------------------
  org_ortc_EventQueueMaker_t org_ortc_EventQueueMaker_wrapperToHandle(wrapper::org::ortc::EventQueueMakerPtr value)
  {
    typedef org_ortc_EventQueueMaker_t CType;
    typedef wrapper::org::ortc::EventQueueMakerPtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (!value) return 0;
    return reinterpret_cast<CType>(new WrapperTypePtr(value));
  }

  //----------------------------------------------------------------------------
  wrapper::org::ortc::EventQueueMakerPtr org_ortc_EventQueueMaker_wrapperFromHandle(org_ortc_EventQueueMaker_t handle)
  {
    typedef wrapper::org::ortc::EventQueueMakerPtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (0 == handle) return WrapperTypePtr();
    return (*reinterpret_cast<WrapperTypePtrRawPtr>(handle));
  }
#endif //0

} /* namespace wrapper */

#ifdef __clang__
#pragma clang diagnostic pop
#endif /* __clang__ */

#endif /* ifndef C_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER */

