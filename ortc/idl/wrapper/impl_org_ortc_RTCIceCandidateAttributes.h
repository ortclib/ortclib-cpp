
#pragma once

#include "types.h"
#include "generated/org_ortc_RTCIceCandidateAttributes.h"

#include <ortc/IStatsReport.h>

namespace wrapper {
  namespace impl {
    namespace org {
      namespace ortc {

        struct RTCIceCandidateAttributes : public wrapper::org::ortc::RTCIceCandidateAttributes
        {
          ZS_DECLARE_TYPEDEF_PTR(::ortc::IStatsReportTypes::ICECandidateAttributes, NativeStats);
          RTCIceCandidateAttributesWeakPtr thisWeak_;
          NativeStatsPtr native_;

          RTCIceCandidateAttributes();
          virtual ~RTCIceCandidateAttributes();

          // methods RTCStats
          virtual wrapper::org::ortc::JsonPtr toJson() override;
          virtual String hash() override;

          // properties RTCStats
          virtual ::zsLib::Time get_timestamp() override;
          virtual Optional< wrapper::org::ortc::RTCStatsType > get_statsType() override;
          virtual String get_statsTypeOther() override;
          virtual String get_id() override;

          // methods RTCIceCandidateAttributes
          virtual void wrapper_init_org_ortc_RTCIceCandidateAttributes() override;
          virtual void wrapper_init_org_ortc_RTCIceCandidateAttributes(wrapper::org::ortc::RTCIceCandidateAttributesPtr source) override;
          virtual void wrapper_init_org_ortc_RTCIceCandidateAttributes(wrapper::org::ortc::JsonPtr json) override;

          // properties RTCIceCandidateAttributes
          virtual String get_relatedId() override;
          virtual String get_ipAddress() override;
          virtual unsigned long get_portNumber() override;
          virtual String get_transport() override;
          virtual wrapper::org::ortc::RTCIceCandidateType get_candidateType() override;
          virtual unsigned long get_priority() override;
          virtual String get_addressSourceUrl() override;

          static RTCIceCandidateAttributesPtr toWrapper(NativeStatsPtr native);
          static NativeStatsPtr toNative(wrapper::org::ortc::RTCIceCandidateAttributesPtr wrapper);
        };

      } // ortc
    } // org
  } // namespace impl
} // namespace wrapper

