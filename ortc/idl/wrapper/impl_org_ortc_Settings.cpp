
#include "impl_org_ortc_Settings.h"
#include "impl_org_ortc_Json.h"

#include <zsLib/IHelper.h>
#include <zsLib/SafeInt.h>

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

ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::Settings::NativeType, NativeType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::Settings::WrapperImplType, WrapperImplType);
ZS_DECLARE_TYPEDEF_PTR(wrapper::impl::org::ortc::Settings::WrapperType, WrapperType);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::Settings::~Settings()
{
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Settings::applyDefaults()
{
  NativeType::applyDefaults();
}

//------------------------------------------------------------------------------
bool wrapper::org::ortc::Settings::apply(wrapper::org::ortc::JsonPtr jsonSettings)
{
  return NativeType::apply(zsLib::IHelper::toString(wrapper::impl::org::ortc::Json::toNative(jsonSettings)));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Settings::setString(
  String key,
  String value
  )
{
  NativeType::setString(key, value);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Settings::setInt(
  String key,
  int64_t value
  )
{
  NativeType::setInt(key, SafeInt<LONG>(value));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Settings::setUInt(
  String key,
  uint64_t value
  )
{
  NativeType::setUInt(key, SafeInt<ULONG>(value));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Settings::setBool(
  String key,
  bool value
  )
{
  NativeType::setBool(key, value);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Settings::setFloat(
  String key,
  float value
  )
{
  NativeType::setFloat(key, value);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Settings::setDouble(
  String key,
  double value
  )
{
  NativeType::setDouble(key, value);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Settings::clear(String key)
{
  NativeType::clear(key);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Settings::clearAll()
{
  NativeType::clearAll();
}


