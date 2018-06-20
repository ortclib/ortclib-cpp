
#include "impl_org_ortc_Logger.h"
#include "impl_org_ortc_Helper.h"

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

#include <ortc/services/ILogger.h>

namespace wrapper { namespace impl { namespace org { namespace ortc { ZS_DECLARE_SUBSYSTEM(org_ortc_wrapper); } } } }

using namespace wrapper::impl::org::ortc;

ZS_DECLARE_TYPEDEF_PTR(::ortc::services::ILogger, UseLogger);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::Logger::~Logger() noexcept
{
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::setLogLevel(
  wrapper::org::ortc::log::Component component,
  wrapper::org::ortc::log::Level level
  ) noexcept
{
  UseLogger::setLogLevel(Helper::toNative(component), Helper::toNative(level));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::setLogLevel(
  String component,
  wrapper::org::ortc::log::Level level
  ) noexcept
{
  UseLogger::setLogLevel(component, Helper::toNative(level));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::installStdOutLogger(bool colorizeOutput) noexcept
{
  UseLogger::installStdOutLogger(colorizeOutput);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::installFileLogger(
  String fileName,
  bool colorizeOutput
  ) noexcept
{
  UseLogger::installFileLogger(fileName, colorizeOutput);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::installTelnetLogger(
  uint16_t listenPort,
  ::zsLib::Seconds maxWaitForSocketToBeAvailable,
  bool colorizeOutput
  ) noexcept
{
  UseLogger::installTelnetLogger(listenPort, maxWaitForSocketToBeAvailable, colorizeOutput);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::installOutgoingTelnetLogger(
  String serverHostWithPort,
  bool colorizeOutput,
  String sendStringUponConnection
  ) noexcept
{
  UseLogger::installOutgoingTelnetLogger(serverHostWithPort, colorizeOutput, sendStringUponConnection);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::installDebuggerLogger() noexcept
{
  UseLogger::installDebuggerLogger();
}

//------------------------------------------------------------------------------
bool wrapper::org::ortc::Logger::isTelnetLoggerListening() noexcept
{
  return UseLogger::isTelnetLoggerListening();
}

//------------------------------------------------------------------------------
bool wrapper::org::ortc::Logger::isTelnetLoggerConnected() noexcept
{
  return UseLogger::isTelnetLoggerConnected();
}

//------------------------------------------------------------------------------
bool wrapper::org::ortc::Logger::isOutgoingTelnetLoggerConnected() noexcept
{
  return UseLogger::isOutgoingTelnetLoggerConnected();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::uninstallStdOutLogger() noexcept
{
  UseLogger::uninstallStdOutLogger();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::uninstallFileLogger() noexcept
{
  UseLogger::uninstallFileLogger();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::uninstallTelnetLogger() noexcept
{
  UseLogger::uninstallTelnetLogger();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::uninstallOutgoingTelnetLogger() noexcept
{
  UseLogger::uninstallOutgoingTelnetLogger();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::uninstallDebuggerLogger() noexcept
{
  UseLogger::uninstallDebuggerLogger();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::setDefaultEventingLevel(
  wrapper::org::ortc::log::Component component,
  wrapper::org::ortc::log::Level level
  ) noexcept
{
  UseLogger::setDefaultEventingLevel(Helper::toNative(component), Helper::toNative(level));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::setDefaultEventingLevel(
  String component,
  wrapper::org::ortc::log::Level level
  ) noexcept
{
  UseLogger::setDefaultEventingLevel(component, Helper::toNative(level));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::setEventingLevel(
  wrapper::org::ortc::log::Component component,
  wrapper::org::ortc::log::Level level
  ) noexcept
{
  UseLogger::setEventingLevel(Helper::toNative(component), Helper::toNative(level));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::setEventingLevel(
  String component,
  wrapper::org::ortc::log::Level level
  ) noexcept
{
  UseLogger::setEventingLevel(component, Helper::toNative(level));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::installEventingListener(
  String sharedSecret,
  uint16_t listenPort,
  ::zsLib::Seconds maxWaitForSocketToBeAvailable
  ) noexcept
{
  UseLogger::installEventingListener(listenPort, sharedSecret, maxWaitForSocketToBeAvailable);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::connectToEventingServer(
  String sharedSecret,
  String serverHostWithPort
  ) noexcept
{
  UseLogger::connectToEventingServer(serverHostWithPort, sharedSecret);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::uninstallEventingListener() noexcept
{
  UseLogger::uninstallEventingListener();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::disconnectEventingServer() noexcept
{
  UseLogger::disconnectEventingServer();
}
