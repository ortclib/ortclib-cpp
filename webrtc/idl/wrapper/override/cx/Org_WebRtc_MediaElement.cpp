

#ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT

#include <wrapper/generated/cx/cx_Helpers.h>
#include <wrapper/generated/cx/Org_WebRtc_MediaElement.h>

//------------------------------------------------------------------------------
::Org::WebRtc::MediaElement^ Org::WebRtc::MediaElement::ToCx(wrapper::org::webRtc::MediaElementPtr value)
{
  if (!value) return nullptr;
  auto result = ref new MediaElement(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementPtr Org::WebRtc::MediaElement::FromCx(::Org::WebRtc::MediaElement^ value)
{
  if (nullptr == value) return wrapper::org::webRtc::MediaElementPtr();
  return value->native_;
}

#if 0
//------------------------------------------------------------------------------
Org::WebRtc::MediaElement::MediaElement(Platform::Object^ element)
 : native_(wrapper::org::webRtc::MediaElement::wrapper_create())
{
  if (!native_) {throw ref new Platform::NullReferenceException();}
  native_->wrapper_init_org_webRtc_MediaElement(::Internal::Helper::FromCx(element));
}

//------------------------------------------------------------------------------
Platform::Object^ Org::WebRtc::MediaElement::Element::get()
{
  if (!native_) {throw ref new Platform::NullReferenceException();}
  return ::Internal::Helper::ToCx(native_->get_element());
}
#endif //0

#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENT
