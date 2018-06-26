

#ifndef C_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE

#include <wrapper/generated/c/c_helpers.h>
#include <zsLib/types.h>
#include <zsLib/eventing/types.h>
#include <zsLib/SafeInt.h>

#include <wrapper/generated/c/c_org_webrtc_EventQueue.h>
#include <wrapper/generated/org_webrtc_EventQueue.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wheader-hygiene"
#endif /* __clang__ */

using namespace wrapper;

//------------------------------------------------------------------------------
org_webrtc_EventQueue_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webrtc_EventQueue_wrapperClone(org_webrtc_EventQueue_t handle)
{
  typedef wrapper::org::webrtc::EventQueuePtr WrapperTypePtr;
  typedef WrapperTypePtr * WrapperTypePtrRawPtr;
  if (0 == handle) return 0;
  return reinterpret_cast<org_webrtc_EventQueue_t>(new WrapperTypePtr(*reinterpret_cast<WrapperTypePtrRawPtr>(handle)));
}

//------------------------------------------------------------------------------
void WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webrtc_EventQueue_wrapperDestroy(org_webrtc_EventQueue_t handle)
{
  typedef wrapper::org::webrtc::EventQueuePtr WrapperTypePtr;
  typedef WrapperTypePtr * WrapperTypePtrRawPtr;
  if (0 == handle) return;
  delete reinterpret_cast<WrapperTypePtrRawPtr>(handle);
}

//------------------------------------------------------------------------------
instance_id_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webrtc_EventQueue_wrapperInstanceId(org_webrtc_EventQueue_t handle)
{
  typedef wrapper::org::webrtc::EventQueuePtr WrapperTypePtr;
  typedef WrapperTypePtr * WrapperTypePtrRawPtr;
  if (0 == handle) return 0;
  return reinterpret_cast<instance_id_t>((*reinterpret_cast<WrapperTypePtrRawPtr>(handle)).get());
}

#if 0
//------------------------------------------------------------------------------
org_webrtc_EventQueue_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webrtc_EventQueue_wrapperCreate_EventQueue(zs_Any_t queue)
{
  auto wrapperThis = wrapper::org::webrtc::EventQueue::wrapper_create();
  wrapperThis->wrapper_init_org_webrtc_EventQueue(wrapper::zs_Any_wrapperFromHandle(queue));
  return wrapper::org_webrtc_EventQueue_wrapperToHandle(wrapperThis);
}
#endif /* 0 NOT USED */

//------------------------------------------------------------------------------
org_webrtc_EventQueue_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webrtc_EventQueue_getDefaultForUi()
{
  return wrapper::org_webrtc_EventQueue_wrapperToHandle(wrapper::org::webrtc::EventQueue::getDefaultForUi());
}

//------------------------------------------------------------------------------
org_webrtc_EventQueue_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webrtc_EventQueue_get_singleton()
{
  return wrapper::org_webrtc_EventQueue_wrapperToHandle(wrapper::org::webrtc::EventQueue::get_singleton());
}

//------------------------------------------------------------------------------
void WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webrtc_EventQueue_set_singleton(org_webrtc_EventQueue_t value)
{
  wrapper::org::webrtc::EventQueue::set_singleton(wrapper::org_webrtc_EventQueue_wrapperFromHandle(value));
}

#if 0
//------------------------------------------------------------------------------
zs_Any_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webrtc_EventQueue_get_queue(org_webrtc_EventQueue_t wrapperThisHandle)
{
  auto wrapperThis = wrapper::org_webrtc_EventQueue_wrapperFromHandle(wrapperThisHandle);
  return wrapper::zs_Any_wrapperToHandle(wrapperThis->get_queue());
}
#endif /* 0 NOT USED */


namespace wrapper
{
  //----------------------------------------------------------------------------
  org_webrtc_EventQueue_t org_webrtc_EventQueue_wrapperToHandle(wrapper::org::webrtc::EventQueuePtr value)
  {
    typedef org_webrtc_EventQueue_t CType;
    typedef wrapper::org::webrtc::EventQueuePtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (!value) return 0;
    return reinterpret_cast<CType>(new WrapperTypePtr(value));
  }

  //----------------------------------------------------------------------------
  wrapper::org::webrtc::EventQueuePtr org_webrtc_EventQueue_wrapperFromHandle(org_webrtc_EventQueue_t handle)
  {
    typedef wrapper::org::webrtc::EventQueuePtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (0 == handle) return WrapperTypePtr();
    return (*reinterpret_cast<WrapperTypePtrRawPtr>(handle));
  }


} /* namespace wrapper */

#ifdef __clang__
#pragma clang diagnostic pop
#endif /* __clang__ */

#endif /* ifndef C_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE */
