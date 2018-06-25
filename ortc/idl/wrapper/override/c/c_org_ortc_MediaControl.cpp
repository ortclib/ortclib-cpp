

#ifndef C_USE_GENERATED_ORG_ORTC_MEDIACONTROL

#include <wrapper/generated/c/c_helpers.h>
#include <zsLib/types.h>
#include <zsLib/eventing/types.h>
#include <zsLib/SafeInt.h>

#include <wrapper/generated/c/c_org_ortc_MediaControl.h>
#include <wrapper/generated/org_ortc_MediaControl.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wheader-hygiene"
#endif /* __clang__ */

using namespace wrapper;

//------------------------------------------------------------------------------
zs_Any_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaControl_get_displayOrientation()
{
  return wrapper::zs_Any_wrapperToHandle(wrapper::org::ortc::MediaControl::get_displayOrientation());
}

//------------------------------------------------------------------------------
void ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaControl_set_displayOrientation(zs_Any_t value)
{
  wrapper::org::ortc::MediaControl::set_displayOrientation(wrapper::zs_Any_wrapperFromHandle(value));
}


namespace wrapper
{
  //----------------------------------------------------------------------------
  org_ortc_MediaControl_t org_ortc_MediaControl_wrapperToHandle(wrapper::org::ortc::MediaControlPtr value)
  {
    typedef org_ortc_MediaControl_t CType;
    typedef wrapper::org::ortc::MediaControlPtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (!value) return 0;
    return reinterpret_cast<CType>(new WrapperTypePtr(value));
  }

  //----------------------------------------------------------------------------
  wrapper::org::ortc::MediaControlPtr org_ortc_MediaControl_wrapperFromHandle(org_ortc_MediaControl_t handle)
  {
    typedef wrapper::org::ortc::MediaControlPtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (0 == handle) return WrapperTypePtr();
    return (*reinterpret_cast<WrapperTypePtrRawPtr>(handle));
  }


} /* namespace wrapper */

#ifdef __clang__
#pragma clang diagnostic pop
#endif /* __clang__ */

#endif /* ifndef C_USE_GENERATED_ORG_ORTC_MEDIACONTROL */
