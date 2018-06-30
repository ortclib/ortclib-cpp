
#ifndef C_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE

#pragma once

#include <wrapper/generated/c/types.h>


WEBRTC_WRAPPER_C_PLUS_PLUS_BEGIN_GUARD


/* org_webRtc_EventQueue*/

WEBRTC_WRAPPER_C_EXPORT_API org_webRtc_EventQueue_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_EventQueue_wrapperClone(org_webRtc_EventQueue_t handle);
WEBRTC_WRAPPER_C_EXPORT_API void WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_EventQueue_wrapperDestroy(org_webRtc_EventQueue_t handle);
WEBRTC_WRAPPER_C_EXPORT_API instance_id_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_EventQueue_wrapperInstanceId(org_webRtc_EventQueue_t handle);
#if 0
WEBRTC_WRAPPER_C_EXPORT_API org_webRtc_EventQueue_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_EventQueue_wrapperCreate_EventQueue(zs_Any_t queue);
#endif /* 0 NOT USED */
WEBRTC_WRAPPER_C_EXPORT_API org_webRtc_EventQueue_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_EventQueue_getDefaultForUi();
WEBRTC_WRAPPER_C_EXPORT_API org_webRtc_EventQueue_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_EventQueue_get_singleton();
WEBRTC_WRAPPER_C_EXPORT_API void WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_EventQueue_set_singleton(org_webRtc_EventQueue_t value);
#if 0
WEBRTC_WRAPPER_C_EXPORT_API zs_Any_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_EventQueue_get_queue(org_webRtc_EventQueue_t wrapperThisHandle);
#endif /* 0 NOT USED */


WEBRTC_WRAPPER_C_PLUS_PLUS_END_GUARD

#ifdef __cplusplus


namespace wrapper
{
  org_webRtc_EventQueue_t org_webRtc_EventQueue_wrapperToHandle(wrapper::org::webRtc::EventQueuePtr value);
  wrapper::org::webRtc::EventQueuePtr org_webRtc_EventQueue_wrapperFromHandle(org_webRtc_EventQueue_t handle);

} /* namespace wrapper */
#endif /* __cplusplus */

#endif /* ifndef C_USE_GENERATED_ORG_WEBRTC_EVENTQUEUE */
