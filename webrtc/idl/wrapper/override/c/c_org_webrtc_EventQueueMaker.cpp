

#ifndef C_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER

#include <wrapper/generated/c/c_helpers.h>
#include <zsLib/types.h>
#include <zsLib/eventing/types.h>
#include <zsLib/SafeInt.h>

#include <wrapper/generated/c/c_org_webrtc_EventQueueMaker.h>
#include <wrapper/generated/org_webrtc_EventQueueMaker.h>
#include <wrapper/generated/c/c_org_webrtc_EventQueue.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wheader-hygiene"
#endif /* __clang__ */

using namespace wrapper;

#if 0
//------------------------------------------------------------------------------
org_webrtc_EventQueue_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webrtc_EventQueueMaker_bindQueue(zs_Any_t queue)
{
  return wrapper::org_webrtc_EventQueue_wrapperToHandle(wrapper::org::webrtc::EventQueueMaker::bindQueue(wrapper::zs_Any_wrapperFromHandle(queue)));
}

//------------------------------------------------------------------------------
zs_Any_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webrtc_EventQueueMaker_extractQueue(org_webrtc_EventQueue_t queue)
{
  return wrapper::zs_Any_wrapperToHandle(wrapper::org::webrtc::EventQueueMaker::extractQueue(wrapper::org_webrtc_EventQueue_wrapperFromHandle(queue)));
}
#endif //0

namespace wrapper
{
#if 0
  //----------------------------------------------------------------------------
  org_webrtc_EventQueueMaker_t org_webrtc_EventQueueMaker_wrapperToHandle(wrapper::org::webrtc::EventQueueMakerPtr value)
  {
    typedef org_webrtc_EventQueueMaker_t CType;
    typedef wrapper::org::webrtc::EventQueueMakerPtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (!value) return 0;
    return reinterpret_cast<CType>(new WrapperTypePtr(value));
  }

  //----------------------------------------------------------------------------
  wrapper::org::webrtc::EventQueueMakerPtr org_webrtc_EventQueueMaker_wrapperFromHandle(org_webrtc_EventQueueMaker_t handle)
  {
    typedef wrapper::org::webrtc::EventQueueMakerPtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (0 == handle) return WrapperTypePtr();
    return (*reinterpret_cast<WrapperTypePtrRawPtr>(handle));
  }
#endif //0

} /* namespace wrapper */

#ifdef __clang__
#pragma clang diagnostic pop
#endif /* __clang__ */

#endif /* ifndef C_USE_GENERATED_ORG_WEBRTC_EVENTQUEUEMAKER */

