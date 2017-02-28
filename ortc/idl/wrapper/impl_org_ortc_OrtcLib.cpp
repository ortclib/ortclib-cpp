
#include "impl_org_ortc_OrtcLib.h"
#include "impl_org_ortc_MessageQueue.h"
#include "impl_org_ortc_Helper.h"

#include <ortc/services/IBackgrounding.h>

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::OrtcLib::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::OrtcLib::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::OrtcLib::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::OrtcLib::~OrtcLib()
{
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::OrtcLib::setup()
{
  setup(wrapper::org::ortc::MessageQueue::getDefaultForUi());
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::OrtcLib::setup(wrapper::org::ortc::MessageQueuePtr queue)
{
  NativeType::setup(wrapper::impl::org::ortc::MessageQueue::toNative(queue));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::OrtcLib::startMediaTracing()
{
  NativeType::startMediaTracing();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::OrtcLib::stopMediaTracing()
{
  NativeType::stopMediaTracing();
}

//------------------------------------------------------------------------------
bool wrapper::org::ortc::OrtcLib::isMediaTracing()
{
  return NativeType::isMediaTracing();
}

//------------------------------------------------------------------------------
bool wrapper::org::ortc::OrtcLib::saveMediaTrace(String filename)
{
  return NativeType::saveMediaTrace(filename);  
}

//------------------------------------------------------------------------------
bool wrapper::org::ortc::OrtcLib::saveMediaTrace(
  String host,
  int port
  )
{
  return NativeType::saveMediaTrace(host, port);
}

//------------------------------------------------------------------------------
PromisePtr wrapper::org::ortc::OrtcLib::notifyGoingToBackground()
{
  struct Notified : public Any,
                    public ::ortc::services::IBackgroundingCompletionDelegate
  {
    virtual void onBackgroundingReady(::ortc::services::IBackgroundingQueryPtr query)
    {
      promise_->resolve();
      promise_.reset();
    }

    PromisePtr promise_;
    ::ortc::services::IBackgroundingQueryPtr query_;
  };

  auto observer = make_shared<Notified>();
  observer->promise_ = Promise::create(wrapper::impl::org::ortc::Helper::getGuiQueue());
  observer->promise_->setReferenceHolder(observer);
  observer->query_ = ::ortc::services::IBackgrounding::notifyGoingToBackground();
  return observer->promise_;
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::OrtcLib::notifyGoingToBackgroundNow()
{
  ::ortc::services::IBackgrounding::notifyGoingToBackgroundNow();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::OrtcLib::notifyReturningFromBackground()
{
  ::ortc::services::IBackgrounding::notifyReturningFromBackground();
}

//------------------------------------------------------------------------------
::zsLib::Milliseconds wrapper::org::ortc::OrtcLib::get_ntpServerTime()
{
  return NativeType::ntpServerTime();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::OrtcLib::set_ntpServerTime(::zsLib::Milliseconds value)
{
  NativeType::ntpServerTime(value);
}
