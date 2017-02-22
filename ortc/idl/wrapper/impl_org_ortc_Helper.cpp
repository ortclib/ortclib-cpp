
#include "impl_org_ortc_Helper.h"

#include "impl_org_ortc_MediaStreamTrack.h"
#include "impl_org_ortc_RTCStatsTypeSet.h"
#include "impl_org_ortc_RTCStatsReport.h"

#include <ortc/IStatsProvider.h>
#include <zsLib/Log.h>

namespace wrapper { namespace impl { namespace org { namespace ortc { ZS_DECLARE_SUBSYSTEM(ortc_wrapper); } } } }
namespace wrapper { namespace impl { namespace org { namespace ortc { namespace adapter { ZS_DECLARE_SUBSYSTEM(ortc_adapter_wrapper); } } } } }

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {
        namespace adapter {
        } // namespace adapater


        //---------------------------------------------------------------------
        Helper::WrapperMediaStreamTrackListPtr Helper::toWrapper(MediaStreamTrackListPtr tracks)
        {
          auto result = make_shared<WrapperMediaStreamTrackList>();
          for (auto iter = tracks->begin(); iter != tracks->end(); ++iter)
          {
            auto native = wrapper::impl::org::ortc::MediaStreamTrack::createFromNative(*iter);
            if (!native) continue;
            result->push_back(native);
          }
          return result;
        }

        //---------------------------------------------------------------------
        Helper::MediaStreamTrackListPtr Helper::toNative(WrapperMediaStreamTrackListPtr tracks)
        {
          if (!tracks) return MediaStreamTrackListPtr();

          MediaStreamTrackListPtr result = make_shared<MediaStreamTrackList>();
          for (auto iter = tracks->begin(); iter != tracks->end(); ++iter)
          {
            auto native = wrapper::impl::org::ortc::MediaStreamTrack::toNative(*iter);
            if (!native) continue;
            result->push_back(native);
          }
          return result;
        }

        //---------------------------------------------------------------------
        Helper::PromiseWithStatsReportPtr Helper::getStats(IStatsProviderPtr native, wrapper::org::ortc::RTCStatsTypeSetPtr statTypes)
        {
          ZS_THROW_INVALID_ARGUMENT_IF(!native);
          ZS_THROW_INVALID_ARGUMENT_IF(!statTypes);

          auto value = wrapper::impl::org::ortc::RTCStatsTypeSet::toNative(statTypes);
          ZS_THROW_INVALID_ARGUMENT_IF(!value);

          auto result = native->getStats(*value);
          if (!result) return PromiseWithStatsReportPtr();

          auto promise = PromiseWithStatsReport::create();

          result->thenClosure([promise, result]()
          {
            if (result->isRejected())
            {
              promise->reject();
              return;
            }
            promise->resolve(wrapper::impl::org::ortc::RTCStatsReport::createFromNative(result->value()));
          });
        }

      } // namespace ortc
    } // namespace org
  } // namespace impl
} // namespace wrapper
