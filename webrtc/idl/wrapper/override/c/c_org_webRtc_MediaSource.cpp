

#ifndef C_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE

#include <wrapper/generated/c/c_helpers.h>
#include <zsLib/types.h>
#include <zsLib/eventing/types.h>
#include <zsLib/SafeInt.h>

#include <wrapper/generated/c/c_org_webRtc_MediaSource.h>
#include <wrapper/generated/org_webRtc_MediaSource.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wheader-hygiene"
#endif /* __clang__ */

using namespace wrapper;

//------------------------------------------------------------------------------
org_webRtc_MediaSource_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_MediaSource_wrapperCreate_MediaSource()
{
  typedef org_webRtc_MediaSource_t CType;
  typedef wrapper::org::webRtc::MediaSourcePtr WrapperTypePtr;
  auto result = wrapper::org::webRtc::MediaSource::wrapper_create();
  result->wrapper_init_org_webRtc_MediaSource();
  return reinterpret_cast<CType>(new WrapperTypePtr(result));
}

//------------------------------------------------------------------------------
org_webRtc_MediaSource_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_MediaSource_wrapperClone(org_webRtc_MediaSource_t handle)
{
  typedef wrapper::org::webRtc::MediaSourcePtr WrapperTypePtr;
  typedef WrapperTypePtr * WrapperTypePtrRawPtr;
  if (0 == handle) return 0;
  return reinterpret_cast<org_webRtc_MediaSource_t>(new WrapperTypePtr(*reinterpret_cast<WrapperTypePtrRawPtr>(handle)));
}

//------------------------------------------------------------------------------
void WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_MediaSource_wrapperDestroy(org_webRtc_MediaSource_t handle)
{
  typedef wrapper::org::webRtc::MediaSourcePtr WrapperTypePtr;
  typedef WrapperTypePtr * WrapperTypePtrRawPtr;
  if (0 == handle) return;
  delete reinterpret_cast<WrapperTypePtrRawPtr>(handle);
}

//------------------------------------------------------------------------------
instance_id_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_MediaSource_wrapperInstanceId(org_webRtc_MediaSource_t handle)
{
  typedef wrapper::org::webRtc::MediaSourcePtr WrapperTypePtr;
  typedef WrapperTypePtr * WrapperTypePtrRawPtr;
  if (0 == handle) return 0;
  return reinterpret_cast<instance_id_t>((*reinterpret_cast<WrapperTypePtrRawPtr>(handle)).get());
}

//------------------------------------------------------------------------------
zs_Any_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_MediaSource_get_source(org_webRtc_MediaSource_t wrapperThisHandle)
{
  auto wrapperThis = wrapper::org_webRtc_MediaSource_wrapperFromHandle(wrapperThisHandle);
  return wrapper::zs_Any_wrapperToHandle(wrapperThis->get_source());
}

//------------------------------------------------------------------------------
void WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_MediaSource_set_source(org_webRtc_MediaSource_t wrapperThisHandle, zs_Any_t value)
{
  auto wrapperThis = wrapper::org_webRtc_MediaSource_wrapperFromHandle(wrapperThisHandle);
  wrapperThis->set_source(wrapper::zs_Any_wrapperFromHandle(value));
}

//------------------------------------------------------------------------------
zs_Any_t WEBRTC_WRAPPER_C_CALLING_CONVENTION org_webRtc_MediaSource_get_track(org_webRtc_MediaSource_t wrapperThisHandle)
{
  auto wrapperThis = wrapper::org_webRtc_MediaSource_wrapperFromHandle(wrapperThisHandle);
  return wrapper::zs_Any_wrapperToHandle(wrapperThis->get_track());
}


namespace wrapper
{
  //----------------------------------------------------------------------------
  org_webRtc_MediaSource_t org_webRtc_MediaSource_wrapperToHandle(wrapper::org::webRtc::MediaSourcePtr value)
  {
    typedef org_webRtc_MediaSource_t CType;
    typedef wrapper::org::webRtc::MediaSourcePtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (!value) return 0;
    return reinterpret_cast<CType>(new WrapperTypePtr(value));
  }

  //----------------------------------------------------------------------------
  wrapper::org::webRtc::MediaSourcePtr org_webRtc_MediaSource_wrapperFromHandle(org_webRtc_MediaSource_t handle)
  {
    typedef wrapper::org::webRtc::MediaSourcePtr WrapperTypePtr;
    typedef WrapperTypePtr * WrapperTypePtrRawPtr;
    if (0 == handle) return WrapperTypePtr();
    return (*reinterpret_cast<WrapperTypePtrRawPtr>(handle));
  }


} /* namespace wrapper */

#ifdef __clang__
#pragma clang diagnostic pop
#endif /* __clang__ */

#endif /* ifndef C_USE_GENERATED_ORG_WEBRTC_MEDIASOURCE */
