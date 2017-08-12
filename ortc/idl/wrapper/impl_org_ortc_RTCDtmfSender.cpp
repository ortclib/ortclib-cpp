
#include "impl_org_ortc_RTCDtmfSender.h"
#include "impl_org_ortc_RTCRtpSender.h"
#include "impl_org_ortc_RTCDtmfToneChangeEvent.h"

using ::zsLib::String;
using ::zsLib::Optional;
using ::zsLib::Any;
using ::zsLib::AnyPtr;
using ::zsLib::AnyHolder;
using ::zsLib::Promise;
using ::zsLib::PromisePtr;
using ::zsLib::PromiseWithHolder;
using ::zsLib::PromiseWithHolderPtr;
using ::zsLib::eventing::SecureByteBlock;
using ::zsLib::eventing::SecureByteBlockPtr;
using ::std::shared_ptr;
using ::std::weak_ptr;
using ::std::make_shared;
using ::std::list;
using ::std::set;
using ::std::map;

namespace wrapper { namespace impl { namespace org { namespace ortc { ZS_DECLARE_SUBSYSTEM(ortc_wrapper); } } } }

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtmfSender::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtmfSender::NativeTypeSubscription, NativeTypeSubscription);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtmfSender::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::RTCDtmfSender::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDtmfSender::RTCDtmfSender()
{
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCDtmfSenderPtr wrapper::org::ortc::RTCDtmfSender::wrapper_create()
{
  auto pThis = make_shared<wrapper::impl::org::ortc::RTCDtmfSender>();
  pThis->thisWeak_ = pThis;
  return pThis;
}

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::RTCDtmfSender::~RTCDtmfSender()
{
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtmfSender::wrapper_init_org_ortc_RTCDtmfSender(wrapper::org::ortc::RTCRtpSenderPtr sender)
{
  native_ = NativeType::create(thisWeak_.lock(), RTCRtpSender::toNative(sender));
}

//------------------------------------------------------------------------------
bool wrapper::impl::org::ortc::RTCDtmfSender::canInsertDtmf()
{
  return native_->canInsertDTMF();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtmfSender::insertDtmf(String tones)
{
  native_->insertDTMF(tones);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtmfSender::insertDtmf(
  String tones,
  ::zsLib::Milliseconds duration
  )
{
  native_->insertDTMF(tones, duration);
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtmfSender::insertDtmf(
  String tones,
  ::zsLib::Milliseconds duration,
  ::zsLib::Milliseconds interToneGap
  )
{
  native_->insertDTMF(tones, duration, interToneGap);
}

//------------------------------------------------------------------------------
uint64_t wrapper::impl::org::ortc::RTCDtmfSender::get_objectId()
{
  return native_->getID();
}

//------------------------------------------------------------------------------
wrapper::org::ortc::RTCRtpSenderPtr wrapper::impl::org::ortc::RTCDtmfSender::get_sender()
{
  return RTCRtpSender::toWrapper(native_->sender());
}

//------------------------------------------------------------------------------
String wrapper::impl::org::ortc::RTCDtmfSender::get_toneBuffer()
{
  return native_->toneBuffer();
}

//------------------------------------------------------------------------------
::zsLib::Milliseconds wrapper::impl::org::ortc::RTCDtmfSender::get_duration()
{
  return native_->duration();
}

//------------------------------------------------------------------------------
::zsLib::Milliseconds wrapper::impl::org::ortc::RTCDtmfSender::get_interToneGap()
{
  return native_->interToneGap();
}

//------------------------------------------------------------------------------
void wrapper::impl::org::ortc::RTCDtmfSender::wrapper_onObserverCountChanged(size_t count)
{
  subscriptionCount_ = count;
  subscribe();
}

//------------------------------------------------------------------------------
void WrapperImplType::onDTMFSenderToneChanged(
  IDTMFSenderPtr sender,
  String tone
)
{
  onToneChange(RTCDtmfToneChangeEvent::toWrapper(tone));
}

//------------------------------------------------------------------------------
WrapperImplTypePtr WrapperImplType::toWrapper(NativeTypePtr track)
{
  if (!track) return WrapperImplTypePtr();

  auto result = make_shared<WrapperImplType>();
  result->thisWeak_ = result;
  result->native_ = track;
  result->defaultSubscription_ = false;
  result->subscribe();
  return result;
}

//------------------------------------------------------------------------------
NativeTypePtr WrapperImplType::toNative(WrapperTypePtr wrapper)
{
  if (!wrapper) return NativeTypePtr();
  auto result = std::dynamic_pointer_cast<WrapperImplType>(wrapper);
  if (!result) return NativeTypePtr();
  return result->native_;
}

//------------------------------------------------------------------------------
void WrapperImplType::subscribe()
{
  if (defaultSubscription_) return;
  if (!native_) return;

  zsLib::AutoLock lock(lock_);
  if (subscriptionCount_ < 1) {
    if (!subscription_) return;
    subscription_->cancel();
    return;
  }
  if (subscription_) return;
  subscription_ = native_->subscribe(thisWeak_.lock());
}
