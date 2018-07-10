

#ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENTMAKER

#include <wrapper/generated/cx/cx_Helpers.h>
#include <wrapper/generated/cx/Org_WebRtc_MediaElementMaker.h>
#include <wrapper/generated/cx/Org_WebRtc_MediaElement.h>

#include <wrapper/impl_org_webRtc_MediaElement.h>

//------------------------------------------------------------------------------
::Org::WebRtc::MediaElementMaker^ Org::WebRtc::MediaElementMaker::ToCx(wrapper::org::webRtc::MediaElementMakerPtr value)
{
  if (!value) return nullptr;
  auto result = ref new MediaElementMaker(WrapperCreate{});
  result->native_ = value;
  return result;
}

//------------------------------------------------------------------------------
wrapper::org::webRtc::MediaElementMakerPtr Org::WebRtc::MediaElementMaker::FromCx(::Org::WebRtc::MediaElementMaker^ value)
{
  if (nullptr == value) return wrapper::org::webRtc::MediaElementMakerPtr();
  return value->native_;
}

//------------------------------------------------------------------------------
::Org::WebRtc::MediaElement^ Org::WebRtc::MediaElementMaker::Bind(Windows::UI::Xaml::Controls::MediaElement^ element)
{
  ::Org::WebRtc::MediaElement^ result {};
  result = ::Internal::Helper::ToCx_Org_WebRtc_MediaElement(wrapper::impl::org::webRtc::MediaElement::toWrapper(element));
  return result;
}

//------------------------------------------------------------------------------
Windows::UI::Xaml::Controls::MediaElement^ Org::WebRtc::MediaElementMaker::Extract(::Org::WebRtc::MediaElement^ element)
{
  Windows::UI::Xaml::Controls::MediaElement^ result {};
  result = wrapper::impl::org::webRtc::MediaElement::toNative_cx(::Internal::Helper::FromCx_Org_WebRtc_MediaElement(element));
  return result;
}


#endif //ifndef CX_USE_GENERATED_ORG_WEBRTC_MEDIAELEMENTMAKER
