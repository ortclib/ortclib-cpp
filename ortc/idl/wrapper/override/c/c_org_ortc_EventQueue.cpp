

#ifndef C_USE_GENERATED_ORG_ORTC_EVENTQUEUE

#include <wrapper/generated/c/c_helpers.h>
#include <zsLib/types.h>
#include <zsLib/eventing/types.h>
#include <zsLib/SafeInt.h>

#include <wrapper/generated/c/c_org_ortc_EventQueue.h>
#include <wrapper/generated/org_ortc_EventQueue.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wheader-hygiene"
#endif /* __clang__ */

using namespace wrapper;

//------------------------------------------------------------------------------
org_ortc_EventQueue_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_wrapperClone(org_ortc_EventQueue_t handle)
{
  typedef wrapper::org::ortc::EventQueuePtr WrapperTypePtr;
  typedef WrapperTypePtr * WrapperTypePtrRawPtr;
  if (0 == handle) return 0;
  return reinterpret_cast<org_ortc_EventQueue_t>(new WrapperTypePtr(*reinterpret_cast<WrapperTypePtrRawPtr>(handle)));
}

//------------------------------------------------------------------------------
void ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_wrapperDestroy(org_ortc_EventQueue_t handle)
{
  typedef wrapper::org::ortc::EventQueuePtr WrapperTypePtr;
  typedef WrapperTypePtr * WrapperTypePtrRawPtr;
  if (0 == handle) return;
  delete reinterpret_cast<WrapperTypePtrRawPtr>(handle);
}

//------------------------------------------------------------------------------
instance_id_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_wrapperInstanceId(org_ortc_EventQueue_t handle)
{
  typedef wrapper::org::ortc::EventQueuePtr WrapperTypePtr;
  typedef WrapperTypePtr * WrapperTypePtrRawPtr;
  if (0 == handle) return 0;
  return reinterpret_cast<instance_id_t>((*reinterpret_cast<WrapperTypePtrRawPtr>(handle)).get());
}

#if 0
//------------------------------------------------------------------------------
org_ortc_EventQueue_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_wrapperCreate_EventQueue(zs_Any_t queue)
{
  auto wrapperThis = wrapper::org::ortc::EventQueue::wrapper_create();
  wrapperThis->wrapper_init_org_ortc_EventQueue(wrapper::zs_Any_wrapperFromHandle(queue));
  return wrapper::org_ortc_EventQueue_wrapperToHandle(wrapperThis);
}
#endif /* 0 NOT USED */

//------------------------------------------------------------------------------
org_ortc_EventQueue_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_getDefaultForUi()
{
  return wrapper::org_ortc_EventQueue_wrapperToHandle(wrapper::org::ortc::EventQueue::getDefaultForUi());
}

//------------------------------------------------------------------------------
org_ortc_EventQueue_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_get_singleton()
{
  return wrapper::org_ortc_EventQueue_wrapperToHandle(wrapper::org::ortc::EventQueue::get_singleton());
}

//------------------------------------------------------------------------------
void ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_set_singleton(org_ortc_EventQueue_t value)
{
  wrapper::org::ortc::EventQueue::set_singleton(wrapper::org_ortc_EventQueue_wrapperFromHandle(value));
}

#if 0
//------------------------------------------------------------------------------
zs_Any_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_get_queue(org_ortc_EventQueue_t wrapperThisHandle)
{
  auto wrapperThis = wrapper::org_ortc_EventQueue_wrapperFromHandle(wrapperThisHandle);
  return wrapper::zs_Any_wrapperToHandle(wrapperThis->get_queue());
}
#endif /* 0 NOT USED */


namespace wrapper
{
  //----------------------------------------------------------------------------
  org_ortc_EventQueue_t org_ortc_EventQueue_wrapperToHandle(wrapper::org::ortc::EventQueuePtr value)
  {
    typedef org_ortc_EventQueue_t CType;
    typedef wrapper::org::ortc::EventQueuePtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (!value) return 0;
    return reinterpret_cast<CType>(new WrapperTypePtr(value));
  }

  //----------------------------------------------------------------------------
  wrapper::org::ortc::EventQueuePtr org_ortc_EventQueue_wrapperFromHandle(org_ortc_EventQueue_t handle)
  {
    typedef wrapper::org::ortc::EventQueuePtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (0 == handle) return WrapperTypePtr();
    return (*reinterpret_cast<WrapperTypePtrRawPtr>(handle));
  }


} /* namespace wrapper */

#ifdef __clang__
#pragma clang diagnostic pop
#endif /* __clang__ */

#endif /* ifndef C_USE_GENERATED_ORG_ORTC_EVENTQUEUE */
