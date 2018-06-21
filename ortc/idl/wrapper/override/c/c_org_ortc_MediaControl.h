
#ifndef C_USE_GENERATED_ORG_ORTC_MEDIACONTROL

#pragma once

#include <wrapper/generated/types.h>


ORTC_WRAPPER_C_PLUS_PLUS_BEGIN_GUARD


/* org_ortc_MediaControl*/

ORTC_WRAPPER_C_EXPORT_API zs_Any_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaControl_get_displayOrientation();
ORTC_WRAPPER_C_EXPORT_API void ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaControl_set_displayOrientation(zs_Any_t value);


ORTC_WRAPPER_C_PLUS_PLUS_END_GUARD

#ifdef __cplusplus


namespace wrapper
{
  org_ortc_MediaControl_t org_ortc_MediaControl_wrapperToHandle(wrapper::org::ortc::MediaControlPtr value);
  wrapper::org::ortc::MediaControlPtr org_ortc_MediaControl_wrapperFromHandle(org_ortc_MediaControl_t handle);

} /* namespace wrapper */
#endif /* __cplusplus */

#endif /* ifndef C_USE_GENERATED_ORG_ORTC_MEDIACONTROL */
