
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

namespace wrapper { namespace impl { namespace org { namespace ortc { ZS_DECLARE_SUBSYSTEM(ortc_wrapper); } } } }

using namespace wrapper::impl::org::ortc;

ZS_DECLARE_TYPEDEF_PTR(::ortc::services::ILogger, UseLogger);

//------------------------------------------------------------------------------
wrapper::impl::org::ortc::Logger::~Logger()
{
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::setDefaultLogLevel(wrapper::org::ortc::log::Level level)
{
  UseLogger::setLogLevel(Helper::toNative(level));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::setLogLevel(
  wrapper::org::ortc::log::Component component,
  wrapper::org::ortc::log::Level level
  )
{
  UseLogger::setLogLevel(Helper::toNative(component), Helper::toNative(level));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::setLogLevel(
  String component,
  wrapper::org::ortc::log::Level level
  )
{
  UseLogger::setLogLevel(component, Helper::toNative(level));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::installStdOutLogger(bool colorizeOutput)
{
  UseLogger::installStdOutLogger(colorizeOutput);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::installFileLogger(
  String fileName,
  bool colorizeOutput
  )
{
  UseLogger::installFileLogger(fileName, colorizeOutput);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::installTelnetLogger(
  uint16_t listenPort,
  ::zsLib::Seconds maxWaitForSocketToBeAvailable,
  bool colorizeOutput
  )
{
  UseLogger::installTelnetLogger(listenPort, maxWaitForSocketToBeAvailable, colorizeOutput);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::installOutgoingTelnetLogger(
  String serverHostWithPort,
  bool colorizeOutput,
  String sendStringUponConnection
  )
{
  UseLogger::installOutgoingTelnetLogger(serverHostWithPort, colorizeOutput, sendStringUponConnection);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::installDebuggerLogger()
{
  UseLogger::installDebuggerLogger();
}

//------------------------------------------------------------------------------
bool wrapper::org::ortc::Logger::isTelnetLoggerListening()
{
  return UseLogger::isTelnetLoggerListening();
}

//------------------------------------------------------------------------------
bool wrapper::org::ortc::Logger::isTelnetLoggerConnected()
{
  return UseLogger::isTelnetLoggerConnected();
}

//------------------------------------------------------------------------------
bool wrapper::org::ortc::Logger::isOutgoingTelnetLoggerConnected()
{
  return UseLogger::isOutgoingTelnetLoggerConnected();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::uninstallStdOutLogger()
{
  UseLogger::uninstallStdOutLogger();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::uninstallFileLogger()
{
  UseLogger::uninstallFileLogger();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::uninstallTelnetLogger()
{
  UseLogger::uninstallTelnetLogger();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::uninstallOutgoingTelnetLogger()
{
  UseLogger::uninstallOutgoingTelnetLogger();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::uninstallDebuggerLogger()
{
  UseLogger::uninstallDebuggerLogger();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::setDefaultEventingLevel(wrapper::org::ortc::log::Level level)
{
  UseLogger::setEventingLevel(Helper::toNative(level));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::setEventingLevel(
  wrapper::org::ortc::log::Component component,
  wrapper::org::ortc::log::Level level
  )
{
  UseLogger::setEventingLevel(Helper::toNative(component), Helper::toNative(level));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::setEventingLevel(
  String component,
  wrapper::org::ortc::log::Level level
  )
{
  UseLogger::setEventingLevel(component, Helper::toNative(level));
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::installEventingListener(
  String sharedSecret,
  uint16_t listenPort,
  ::zsLib::Seconds maxWaitForSocketToBeAvailable
  )
{
  UseLogger::installEventingListener(listenPort, sharedSecret, maxWaitForSocketToBeAvailable);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::connectToEventingServer(
  String sharedSecret,
  String serverHostWithPort
  )
{
  UseLogger::connectToEventingServer(serverHostWithPort, sharedSecret);
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::uninstallEventingListener()
{
  UseLogger::uninstallEventingListener();
}

//------------------------------------------------------------------------------
void wrapper::org::ortc::Logger::disconnectEventingServer()
{
  UseLogger::disconnectEventingServer();
}
