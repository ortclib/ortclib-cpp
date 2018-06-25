

#ifndef C_USE_GENERATED_ORG_ORTC_MEDIASOURCE

#include <wrapper/generated/c/c_helpers.h>
#include <zsLib/types.h>
#include <zsLib/eventing/types.h>
#include <zsLib/SafeInt.h>

#include <wrapper/generated/c/c_org_ortc_MediaSource.h>
#include <wrapper/generated/org_ortc_MediaSource.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wheader-hygiene"
#endif /* __clang__ */

using namespace wrapper;

//------------------------------------------------------------------------------
org_ortc_MediaSource_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_wrapperCreate_MediaSource()
{
  typedef org_ortc_MediaSource_t CType;
  typedef wrapper::org::ortc::MediaSourcePtr WrapperTypePtr;
  auto result = wrapper::org::ortc::MediaSource::wrapper_create();
  result->wrapper_init_org_ortc_MediaSource();
  return reinterpret_cast<CType>(new WrapperTypePtr(result));
}

//------------------------------------------------------------------------------
org_ortc_MediaSource_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_wrapperClone(org_ortc_MediaSource_t handle)
{
  typedef wrapper::org::ortc::MediaSourcePtr WrapperTypePtr;
  typedef WrapperTypePtr * WrapperTypePtrRawPtr;
  if (0 == handle) return 0;
  return reinterpret_cast<org_ortc_MediaSource_t>(new WrapperTypePtr(*reinterpret_cast<WrapperTypePtrRawPtr>(handle)));
}

//------------------------------------------------------------------------------
void ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_wrapperDestroy(org_ortc_MediaSource_t handle)
{
  typedef wrapper::org::ortc::MediaSourcePtr WrapperTypePtr;
  typedef WrapperTypePtr * WrapperTypePtrRawPtr;
  if (0 == handle) return;
  delete reinterpret_cast<WrapperTypePtrRawPtr>(handle);
}

//------------------------------------------------------------------------------
instance_id_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_wrapperInstanceId(org_ortc_MediaSource_t handle)
{
  typedef wrapper::org::ortc::MediaSourcePtr WrapperTypePtr;
  typedef WrapperTypePtr * WrapperTypePtrRawPtr;
  if (0 == handle) return 0;
  return reinterpret_cast<instance_id_t>((*reinterpret_cast<WrapperTypePtrRawPtr>(handle)).get());
}

//------------------------------------------------------------------------------
zs_Any_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_get_source(org_ortc_MediaSource_t wrapperThisHandle)
{
  auto wrapperThis = wrapper::org_ortc_MediaSource_wrapperFromHandle(wrapperThisHandle);
  return wrapper::zs_Any_wrapperToHandle(wrapperThis->get_source());
}

//------------------------------------------------------------------------------
void ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_set_source(org_ortc_MediaSource_t wrapperThisHandle, zs_Any_t value)
{
  auto wrapperThis = wrapper::org_ortc_MediaSource_wrapperFromHandle(wrapperThisHandle);
  wrapperThis->set_source(wrapper::zs_Any_wrapperFromHandle(value));
}

//------------------------------------------------------------------------------
zs_Any_t ORTC_WRAPPER_C_CALLING_CONVENTION org_ortc_MediaSource_get_track(org_ortc_MediaSource_t wrapperThisHandle)
{
  auto wrapperThis = wrapper::org_ortc_MediaSource_wrapperFromHandle(wrapperThisHandle);
  return wrapper::zs_Any_wrapperToHandle(wrapperThis->get_track());
}


namespace wrapper
{
  //----------------------------------------------------------------------------
  org_ortc_MediaSource_t org_ortc_MediaSource_wrapperToHandle(wrapper::org::ortc::MediaSourcePtr value)
  {
    typedef org_ortc_MediaSource_t CType;
    typedef wrapper::org::ortc::MediaSourcePtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (!value) return 0;
    return reinterpret_cast<CType>(new WrapperTypePtr(value));
  }

  //----------------------------------------------------------------------------
  wrapper::org::ortc::MediaSourcePtr org_ortc_MediaSource_wrapperFromHandle(org_ortc_MediaSource_t handle)
  {
    typedef wrapper::org::ortc::MediaSourcePtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (0 == handle) return WrapperTypePtr();
    return (*reinterpret_cast<WrapperTypePtrRawPtr>(handle));
  }


} /* namespace wrapper */

#ifdef __clang__
#pragma clang diagnostic pop
#endif /* __clang__ */

#endif /* ifndef C_USE_GENERATED_ORG_ORTC_MEDIASOURCE */
