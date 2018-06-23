

#ifndef C_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER

#pragma once

#include <wrapper/generated/c/types.h>


ORTC_WRAPPER_C_PLUS_PLUS_BEGIN_GUARD


/* org_ortc_EventQueueMaker*/

#if 0
ORTC_WRAPPER_C_EXPORT_API org_ortc_EventQueue_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueueMaker_bindQueue(zs_Any_t queue);
ORTC_WRAPPER_C_EXPORT_API zs_Any_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_EventQueueMaker_extractQueue(org_ortc_EventQueue_t queue);
#endif //0

ORTC_WRAPPER_C_PLUS_PLUS_END_GUARD

#ifdef __cplusplus


namespace wrapper
{
#if 0
  org_ortc_EventQueueMaker_t org_ortc_EventQueueMaker_wrapperToHandle(wrapper::org::ortc::EventQueueMakerPtr value);
  wrapper::org::ortc::EventQueueMakerPtr org_ortc_EventQueueMaker_wrapperFromHandle(org_ortc_EventQueueMaker_t handle);
#endif //0

} /* namespace wrapper */
#endif /* __cplusplus */

#endif /* ifndef C_USE_GENERATED_ORG_ORTC_EVENTQUEUEMAKER */
