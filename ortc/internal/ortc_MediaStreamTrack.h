/*

 Copyright (c) 2015, Hookflash Inc. / Hookflash Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 
 */

#pragma once

#include <ortc/internal/types.h>

#include <ortc/IMediaStreamTrack.h>

#include <openpeer/services/IWakeDelegate.h>
#include <zsLib/MessageQueueAssociator.h>
#include <zsLib/Timer.h>

//#define ORTC_SETTING_SCTP_TRANSPORT_MAX_MESSAGE_SIZE "ortc/sctp/max-message-size"

namespace ortc
{
  namespace internal
  {
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForSettings)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPSender)
    ZS_DECLARE_INTERACTION_PTR(IMediaStreamTrackForRTPReceiver)

    ZS_DECLARE_INTERACTION_PROXY(IMediaStreamTrackAsyncDelegate)

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForSettings
    #pragma mark

    interaction IMediaStreamTrackForSettings
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForSettings, ForSettings)

      static void applyDefaults();

      virtual ~IMediaStreamTrackForSettings() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPSender
    #pragma mark

    interaction IMediaStreamTrackForRTPSender
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPSender, ForSender)

      virtual ~IMediaStreamTrackForRTPSender() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackForRTPReceiver
    #pragma mark

    interaction IMediaStreamTrackForRTPReceiver
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackForRTPReceiver, ForReceiver)

      virtual ~IMediaStreamTrackForRTPReceiver() {}
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackAsyncDelegate
    #pragma mark

    interaction IMediaStreamTrackAsyncDelegate
    {
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraintsPtr, TrackConstraintsPtr)

      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise) = 0;

      virtual void onApplyConstraints(
                                      PromisePtr promise,
                                      TrackConstraintsPtr constraints
                                      ) = 0;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStreamTrack
    #pragma mark
    
    class MediaStreamTrack : public Noop,
                             public MessageQueueAssociator,
                             public SharedRecursiveLock,
                             public IMediaStreamTrack,
                             public IMediaStreamTrackForSettings,
                             public IMediaStreamTrackForRTPSender,
                             public IMediaStreamTrackForRTPReceiver,
                             public IWakeDelegate,
                             public zsLib::ITimerDelegate,
                             public IMediaStreamTrackAsyncDelegate
    {
    protected:
      struct make_private {};

    public:
      friend interaction IMediaStreamTrack;
      friend interaction IMediaStreamTrackFactory;
      friend interaction IMediaStreamTrackForSettings;
      friend interaction IMediaStreamTrackForRTPSender;
      friend interaction IMediaStreamTrackForRTPReceiver;

      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::TrackConstraintsPtr, TrackConstraintsPtr)
      ZS_DECLARE_TYPEDEF_PTR(IMediaStreamTrackTypes::Constraints, Constraints)

    public:
      MediaStreamTrack(
                       const make_private &,
                       IMessageQueuePtr queue
                       );

    protected:
      MediaStreamTrack(Noop) :
        Noop(true),
        MessageQueueAssociator(IMessageQueuePtr()),
        SharedRecursiveLock(SharedRecursiveLock::create())
      {}

      void init();

    public:
      virtual ~MediaStreamTrack();

      static MediaStreamTrackPtr create();

      static MediaStreamTrackPtr convert(IMediaStreamTrackPtr object);
      static MediaStreamTrackPtr convert(ForSettingsPtr object);
      static MediaStreamTrackPtr convert(ForSenderPtr object);
      static MediaStreamTrackPtr convert(ForReceiverPtr object);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark ICETransport => IStatsProvider
      #pragma mark

      virtual PromiseWithStatsReportPtr getStats() const throw(InvalidStateError) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrack
      #pragma mark

      static ElementPtr toDebug(MediaStreamTrackPtr object);

      virtual PUID getID() const override {return mID;}

      virtual IMediaStreamTrackSubscriptionPtr subscribe(IMediaStreamTrackDelegatePtr delegate) override;

      virtual Kinds kind() const override;
      virtual String id() const override;
      virtual String label() const override;
      virtual bool enabled() const override;
      virtual void enabeld(bool enabled) override;
      virtual bool muted() const override;
      virtual bool readOnly() const override;
      virtual bool remote() const override;
      virtual States readyState() const override;

      virtual IMediaStreamTrackPtr clone() const override;

      virtual void stop() override;

      virtual CapabilitiesPtr getCapabilities() const override;
      virtual TrackConstraintsPtr getConstraints() const override;
      virtual SettingsPtr getSettings() const override;

      virtual PromisePtr applyConstraints(const TrackConstraints &constraints) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForSettings
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPSender
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackForRTPReceiver
      #pragma mark

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IWakeDelegate
      #pragma mark

      virtual void onWake() override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => ITimerDelegate
      #pragma mark

      virtual void onTimer(TimerPtr timer) override;

      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => IMediaStreamTrackAsyncDelegate
      #pragma mark

      virtual void onResolveStatsPromise(IStatsProvider::PromiseWithStatsReportPtr promise) override;

      virtual void onApplyConstraints(
                                      PromisePtr promise,
                                      TrackConstraintsPtr constraints
                                      ) override;

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => (internal)
      #pragma mark

      Log::Params log(const char *message) const;
      static Log::Params slog(const char *message);
      Log::Params debug(const char *message) const;
      virtual ElementPtr toDebug() const;

      bool isShuttingDown() const;
      bool isShutdown() const;

      void step();
      bool stepBogusDoSomething();

      void cancel();

      void setState(States state);
      void setError(WORD error, const char *reason = NULL);

    protected:
      //-----------------------------------------------------------------------
      #pragma mark
      #pragma mark MediaStreamTrack => (data)
      #pragma mark

      AutoPUID mID;
      MediaStreamTrackWeakPtr mThisWeak;
      MediaStreamTrackPtr mGracefulShutdownReference;

      IMediaStreamTrackDelegateSubscriptions mSubscriptions;

      States mCurrentState {State_Live};

      WORD mLastError {};
      String mLastErrorReason;
    };

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackFactory
    #pragma mark

    interaction IMediaStreamTrackFactory
    {
      static IMediaStreamTrackFactory &singleton();

      virtual MediaStreamTrackPtr create();
    };

    class MediaStreamTrackFactory : public IFactory<IMediaStreamTrackFactory> {};
  }
}

ZS_DECLARE_PROXY_BEGIN(ortc::internal::IMediaStreamTrackAsyncDelegate)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IStatsProvider::PromiseWithStatsReportPtr, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_TYPEDEF(zsLib::PromisePtr, PromisePtr)
ZS_DECLARE_PROXY_TYPEDEF(ortc::IMediaStreamTrackTypes::TrackConstraintsPtr, TrackConstraintsPtr)
ZS_DECLARE_PROXY_METHOD_1(onResolveStatsPromise, PromiseWithStatsReportPtr)
ZS_DECLARE_PROXY_METHOD_2(onApplyConstraints, PromisePtr, TrackConstraintsPtr)
ZS_DECLARE_PROXY_END()
