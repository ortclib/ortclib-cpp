/*
 
 Copyright (c) 2014, Hookflash Inc. / Hookflash Inc.
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

#include <ortc/internal/ortc_Factory.h>

#include <zsLib/Log.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark (helper)
    #pragma mark

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark Factory
    #pragma mark

    //-------------------------------------------------------------------------
    void Factory::override(FactoryPtr override)
    {
      singleton().mOverride = override;
    }

    //-------------------------------------------------------------------------
    Factory &Factory::singleton()
    {
      static Singleton<Factory, false> factory;
      Factory &singleton = factory.singleton();
      if (singleton.mOverride) return (*singleton.mOverride);
      return singleton;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IDTLSTransportFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IDTLSTransportFactory &IDTLSTransportFactory::singleton()
    {
      return Factory::singleton();
    }

    //-------------------------------------------------------------------------
    DTLSTransportPtr IDTLSTransportFactory::create(
                                                   IDTLSTransportDelegatePtr delegate,
                                                   IICETransportPtr iceTransport
                                                   )
    {
      if (this) {}
      return internal::DTLSTransport::create(delegate, iceTransport);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IICETransportFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IICETransportFactory &IICETransportFactory::singleton()
    {
      return Factory::singleton();
    }

    //-------------------------------------------------------------------------
    ICETransportPtr IICETransportFactory::create(
                                                 IICETransportDelegatePtr delegate,
                                                 IICETransport::ServerListPtr servers
                                                 )
    {
      if (this) {}
      return internal::ICETransport::create(delegate, servers);
    }


    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IRTPSenderFactory
    #pragma mark

    //-------------------------------------------------------------------------
    IRTPSenderFactory &IRTPSenderFactory::singleton()
    {
      return Factory::singleton();
    }

    //-------------------------------------------------------------------------
    RTPSenderPtr IRTPSenderFactory::create(
                                          IDTLSTransportPtr rtpTransport,
                                          IDTLSTransportPtr rtcpTransport
                                          )
    {
      if (this) {}
      return internal::RTPSender::create(rtpTransport, rtcpTransport);
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaManagerFactory
    #pragma mark
    
    //-------------------------------------------------------------------------
    IMediaManagerFactory &IMediaManagerFactory::singleton()
    {
      return Factory::singleton();
    }
    
    //-------------------------------------------------------------------------
    MediaManagerPtr IMediaManagerFactory::create(IMediaManagerDelegatePtr delegate)
    {
      if (this) {}
      return MediaManager::create(IMessageQueuePtr(), delegate);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamFactory
    #pragma mark
    
    //-------------------------------------------------------------------------
    IMediaStreamFactory &IMediaStreamFactory::singleton()
    {
      return Factory::singleton();
    }
    
    //-------------------------------------------------------------------------
    MediaStreamPtr IMediaStreamFactory::create(IMediaStreamDelegatePtr delegate)
    {
      if (this) {}
      return MediaStream::create(IMessageQueuePtr(), delegate);
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamTrackFactory
    #pragma mark
    
    //-------------------------------------------------------------------------
    IMediaStreamTrackFactory &IMediaStreamTrackFactory::singleton()
    {
      return Factory::singleton();
    }
    
    //-------------------------------------------------------------------------
    LocalAudioStreamTrackPtr IMediaStreamTrackFactory::createLocalAudioStreamTrack(IMediaStreamTrackDelegatePtr delegate)
    {
      if (this) {}
      return LocalAudioStreamTrackPtr();
    }
    
    //-------------------------------------------------------------------------
    RemoteReceiveAudioStreamTrackPtr IMediaStreamTrackFactory::createRemoteReceiveAudioStreamTrack(IMediaStreamTrackDelegatePtr delegate)
    {
      if (this) {}
      return RemoteReceiveAudioStreamTrackPtr();
    }
    
    //-------------------------------------------------------------------------
    RemoteSendAudioStreamTrackPtr IMediaStreamTrackFactory::createRemoteSendAudioStreamTrack(IMediaStreamTrackDelegatePtr delegate)
    {
      if (this) {}
      return RemoteSendAudioStreamTrackPtr();
    }
    
    //-------------------------------------------------------------------------
    LocalVideoStreamTrackPtr IMediaStreamTrackFactory::createLocalVideoStreamTrack(IMediaStreamTrackDelegatePtr delegate)
    {
      if (this) {}
      return LocalVideoStreamTrackPtr();
    }
    
    //-------------------------------------------------------------------------
    RemoteReceiveVideoStreamTrackPtr IMediaStreamTrackFactory::createRemoteReceiveVideoStreamTrack(IMediaStreamTrackDelegatePtr delegate)
    {
      if (this) {}
      return RemoteReceiveVideoStreamTrackPtr();
    }
    
    //-------------------------------------------------------------------------
    RemoteSendVideoStreamTrackPtr IMediaStreamTrackFactory::createRemoteSendVideoStreamTrack(IMediaStreamTrackDelegatePtr delegate)
    {
      if (this) {}
      return RemoteSendVideoStreamTrackPtr();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaEngineFactory
    #pragma mark
    
    //-------------------------------------------------------------------------
    IMediaEngineFactory &IMediaEngineFactory::singleton()
    {
      return Factory::singleton();
    }
    
    //-------------------------------------------------------------------------
    MediaEnginePtr IMediaEngineFactory::create(IMediaEngineDelegatePtr delegate)
    {
      if (this) {}
      return MediaEngine::create(delegate);
    }
  }
}
