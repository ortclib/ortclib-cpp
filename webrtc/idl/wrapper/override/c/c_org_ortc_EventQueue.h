
#ifndef C_USE_GENERATED_ORG_ORTC_EVENTQUEUE

#pragma once

#include <wrapper/generated/c/types.h>


ORTC_WRAPPER_C_PLUS_PLUS_BEGIN_GUARD


/* org_ortc_EventQueue*/

ORTC_WRAPPER_C_EXPORT_API org_ortc_EventQueue_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_wrapperClone(org_ortc_EventQueue_t handle);
ORTC_WRAPPER_C_EXPORT_API void ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_wrapperDestroy(org_ortc_EventQueue_t handle);
ORTC_WRAPPER_C_EXPORT_API instance_id_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_wrapperInstanceId(org_ortc_EventQueue_t handle);
#if 0
ORTC_WRAPPER_C_EXPORT_API org_ortc_EventQueue_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_wrapperCreate_EventQueue(zs_Any_t queue);
#endif /* 0 NOT USED */
ORTC_WRAPPER_C_EXPORT_API org_ortc_EventQueue_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_getDefaultForUi();
ORTC_WRAPPER_C_EXPORT_API org_ortc_EventQueue_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_get_singleton();
ORTC_WRAPPER_C_EXPORT_API void ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_set_singleton(org_ortc_EventQueue_t value);
#if 0
ORTC_WRAPPER_C_EXPORT_API zs_Any_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueue_get_queue(org_ortc_EventQueue_t wrapperThisHandle);
#endif /* 0 NOT USED */


ORTC_WRAPPER_C_PLUS_PLUS_END_GUARD

#ifdef __cplusplus


namespace wrapper
{
  org_ortc_EventQueue_t org_ortc_EventQueue_wrapperToHandle(wrapper::org::ortc::EventQueuePtr value);
  wrapper::org::ortc::EventQueuePtr org_ortc_EventQueue_wrapperFromHandle(org_ortc_EventQueue_t handle);

} /* namespace wrapper */
#endif /* __cplusplus */

#endif /* ifndef C_USE_GENERATED_ORG_ORTC_EVENTQUEUE */
