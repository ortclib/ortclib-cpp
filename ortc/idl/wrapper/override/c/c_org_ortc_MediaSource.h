
#ifndef C_USE_GENERATED_ORG_ORTC_MEDIASOURCE

#pragma once

#include <wrapper/generated/types.h>


ORTC_WRAPPER_C_PLUS_PLUS_BEGIN_GUARD


/* org_ortc_MediaSource*/

ORTC_WRAPPER_C_EXPORT_API org_ortc_MediaSource_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_wrapperCreate_MediaSource();
ORTC_WRAPPER_C_EXPORT_API org_ortc_MediaSource_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_wrapperClone(org_ortc_MediaSource_t handle);
ORTC_WRAPPER_C_EXPORT_API void ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_wrapperDestroy(org_ortc_MediaSource_t handle);
ORTC_WRAPPER_C_EXPORT_API instance_id_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_wrapperInstanceId(org_ortc_MediaSource_t handle);
ORTC_WRAPPER_C_EXPORT_API zs_Any_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_get_source(org_ortc_MediaSource_t wrapperThisHandle);
ORTC_WRAPPER_C_EXPORT_API void ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_set_source(org_ortc_MediaSource_t wrapperThisHandle, zs_Any_t value);
ORTC_WRAPPER_C_EXPORT_API zs_Any_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_get_track(org_ortc_MediaSource_t wrapperThisHandle);


ORTC_WRAPPER_C_PLUS_PLUS_END_GUARD

#ifdef __cplusplus


namespace wrapper
{
  org_ortc_MediaSource_t org_ortc_MediaSource_wrapperToHandle(wrapper::org::ortc::MediaSourcePtr value);
  wrapper::org::ortc::MediaSourcePtr org_ortc_MediaSource_wrapperFromHandle(org_ortc_MediaSource_t handle);

} /* namespace wrapper */
#endif /* __cplusplus */

#endif /* ifndef C_USE_GENERATED_ORG_ORTC_MEDIASOURCE */
