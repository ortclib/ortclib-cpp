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

#include <ortc/internal/ortc_MediaStream.h>
#include <ortc/internal/ortc_MediaStreamTrack.h>

#include <openpeer/services/IHelper.h>

#include <zsLib/Log.h>
#include <zsLib/XML.h>

namespace ortc { ZS_DECLARE_SUBSYSTEM(ortclib) }

namespace ortc
{
  typedef openpeer::services::IHelper OPIHelper;
  
  namespace internal
  {
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark IMediaStreamForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    MediaStreamPtr IMediaStreamForMediaManager::create(IMessageQueuePtr queue, IMediaStreamDelegatePtr delegate)
    {
      MediaStreamPtr pThis(new MediaStream(queue, delegate));
      pThis->mThisWeak = pThis;
      return pThis;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStream
    #pragma mark
    
    //-----------------------------------------------------------------------
    MediaStream::MediaStream(IMessageQueuePtr queue, IMediaStreamDelegatePtr delegate) :
      MessageQueueAssociator(queue),
      mID(zsLib::createPUID()),
      mInactive(true),
      mAudioChannel(-1),
      mVideoChannel(-1)
    {
      mAudioTracks = MediaStreamTrackListPtr(new MediaStreamTrackList());
      mVideoTracks = MediaStreamTrackListPtr(new MediaStreamTrackList());
    }
    
    //-----------------------------------------------------------------------
    MediaStream::~MediaStream()
    {
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStream => IMediaStream
    #pragma mark
    
    //-----------------------------------------------------------------------
    String MediaStream::id()
    {
      return String();
    }
    
    //-----------------------------------------------------------------------
    MediaStreamTrackListPtr MediaStream::getAudioTracks()
    {
      return mAudioTracks;
    }
    
    //-----------------------------------------------------------------------
    MediaStreamTrackListPtr MediaStream::getVideoTracks()
    {
      return mVideoTracks;
    }

    //-----------------------------------------------------------------------
    IMediaStreamTrackPtr MediaStream::getTrackById(String trackId)
    {
      return IMediaStreamTrackPtr();
    }
    
    //-----------------------------------------------------------------------
    void MediaStream::addTrack(IMediaStreamTrackPtr track)
    {
      if (typeid(*track) == typeid(LocalAudioStreamTrack))
      {
        if (mAudioChannel != -1)
        {
          LocalAudioStreamTrackPtr localTrack = boost::dynamic_pointer_cast<LocalAudioStreamTrack>(track);
          localTrack->forMediaManager().setChannel(mAudioChannel);
        }
        mAudioTracks->push_back(track);
      }
      else if (typeid(*track) == typeid(RemoteReceiveAudioStreamTrack))
      {
        if (mAudioChannel != -1)
        {
          RemoteReceiveAudioStreamTrackPtr remoteTrack = boost::dynamic_pointer_cast<RemoteReceiveAudioStreamTrack>(track);
          remoteTrack->forMediaManager().setChannel(mAudioChannel);
        }
        mAudioTracks->push_back(track);
      }
      else if (typeid(*track) == typeid(LocalVideoStreamTrack))
      {
        if (mVideoChannel != -1)
        {
          LocalVideoStreamTrackPtr localTrack = boost::dynamic_pointer_cast<LocalVideoStreamTrack>(track);
          localTrack->forMediaManager().setChannel(mVideoChannel);
        }
        mVideoTracks->push_back(track);
      }
      else if (typeid(*track) == typeid(RemoteReceiveVideoStreamTrack))
      {
        if (mVideoChannel != -1)
        {
          RemoteReceiveVideoStreamTrackPtr remoteTrack = boost::dynamic_pointer_cast<RemoteReceiveVideoStreamTrack>(track);
          remoteTrack->forMediaManager().setChannel(mVideoChannel);
        }
        mVideoTracks->push_back(track);
      }
    }
    
    //-----------------------------------------------------------------------
    void MediaStream::removeTrack(IMediaStreamTrackPtr track)
    {
      if (typeid(*track) == typeid(LocalAudioStreamTrack) || typeid(*track) == typeid(RemoteReceiveAudioStreamTrack) ||
          typeid(*track) == typeid(RemoteSendAudioStreamTrack))
      {
        mAudioTracks->remove(track);
      }
      else if (typeid(*track) == typeid(LocalVideoStreamTrack) || typeid(*track) == typeid(RemoteReceiveVideoStreamTrack) ||
               typeid(*track) == typeid(RemoteSendVideoStreamTrack))
      {
        mVideoTracks->remove(track);
      }
    }
    
    //-----------------------------------------------------------------------
    IMediaStreamPtr MediaStream::clone()
    {
      return IMediaStreamPtr();
    }
    
    //-----------------------------------------------------------------------
    bool MediaStream::inactive()
    {
      return false;
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStream => IMediaStreamForMediaManager
    #pragma mark
    
    //-------------------------------------------------------------------------
    String MediaStream::getCNAME()
    {
      return String();
    }
    
    //-------------------------------------------------------------------------
    int MediaStream::getAudioChannel()
    {
      return mAudioChannel;
    }
    
    //-------------------------------------------------------------------------
    void MediaStream::setAudioChannel(int channel)
    {
      mAudioChannel = channel;
      
      for (MediaStreamTrackList::iterator iter = mAudioTracks->begin(); iter != mAudioTracks->end(); iter++)
      {
        if (typeid(**iter) == typeid(LocalAudioStreamTrack))
        {
          LocalAudioStreamTrackPtr localTrack = boost::dynamic_pointer_cast<LocalAudioStreamTrack>(*iter);
          localTrack->forMediaManager().setChannel(channel);
        }
        else if (typeid(**iter) == typeid(RemoteReceiveAudioStreamTrack))
        {
          RemoteReceiveAudioStreamTrackPtr remoteTrack = boost::dynamic_pointer_cast<RemoteReceiveAudioStreamTrack>(*iter);
          remoteTrack->forMediaManager().setChannel(channel);
        }
      }
    }
    
    //-------------------------------------------------------------------------
    int MediaStream::getVideoChannel()
    {
      return mVideoChannel;
    }
    
    //-------------------------------------------------------------------------
    void MediaStream::setVideoChannel(int channel)
    {
      mVideoChannel = channel;
      
      for (MediaStreamTrackList::iterator iter = mVideoTracks->begin(); iter != mVideoTracks->end(); iter++)
      {
        if (typeid(**iter) == typeid(LocalVideoStreamTrack))
        {
          LocalVideoStreamTrackPtr localTrack = boost::dynamic_pointer_cast<LocalVideoStreamTrack>(*iter);
          localTrack->forMediaManager().setChannel(channel);
        }
        else if (typeid(**iter) == typeid(RemoteReceiveVideoStreamTrack))
        {
          RemoteReceiveVideoStreamTrackPtr remoteTrack = boost::dynamic_pointer_cast<RemoteReceiveVideoStreamTrack>(*iter);
          remoteTrack->forMediaManager().setChannel(channel);
        }
      }
    }

    //-------------------------------------------------------------------------
    void MediaStream::setVoiceRecordFile(String fileName)
    {
      AutoRecursiveLock lock(mLock);
      
      //ZS_LOG_DEBUG(log("set voice record file - value: ") + fileName)
      
      mVoiceRecordFile = fileName;
    }
    
    //-------------------------------------------------------------------------
    String MediaStream::getVoiceRecordFile() const
    {
      AutoRecursiveLock lock(mLock);
      
      //ZS_LOG_DEBUG(log("get voice record file - value: ") + mVoiceRecordFile)
      
      return mVoiceRecordFile;
    }
    
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStream => IWakeDelegate
    #pragma mark
    
    //-------------------------------------------------------------------------
    void MediaStream::onWake()
    {
      ZS_LOG_DEBUG(log("wake"))
      
      AutoRecursiveLock lock(getLock());
      step();
    }
    
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStream => (internal)
    #pragma mark
    
    //-------------------------------------------------------------------------
    Log::Params MediaStream::log(const char *message) const
    {
      ElementPtr objectEl = Element::create("ortc::MediaStream");
      OPIHelper::debugAppend(objectEl, "id", mID);
      return Log::Params(message, objectEl);
    }
    
    //-------------------------------------------------------------------------
    Log::Params MediaStream::debug(const char *message) const
    {
      return Log::Params(message, toDebug());
    }
    
    //-------------------------------------------------------------------------
    ElementPtr MediaStream::toDebug() const
    {
      ElementPtr resultEl = Element::create("MediaStream");
      
      OPIHelper::debugAppend(resultEl, "id", mID);
      
      OPIHelper::debugAppend(resultEl, "graceful shutdown", (bool)mGracefulShutdownReference);
      OPIHelper::debugAppend(resultEl, "graceful shutdown", mShutdown);
      
      OPIHelper::debugAppend(resultEl, "error", mLastError);
      OPIHelper::debugAppend(resultEl, "error reason", mLastErrorReason);
      
      return resultEl;
    }
    
    //-------------------------------------------------------------------------
    bool MediaStream::isShuttingDown() const
    {
      return (bool)mGracefulShutdownReference;
    }
    
    //-------------------------------------------------------------------------
    bool MediaStream::isShutdown() const
    {
      if (mGracefulShutdownReference) return false;
      return mShutdown;
    }
    
    //-------------------------------------------------------------------------
    void MediaStream::step()
    {
      ZS_LOG_DEBUG(debug("step"))
      
      AutoRecursiveLock lock(getLock());
      
      if ((isShuttingDown()) ||
          (isShutdown())) {
        ZS_LOG_DEBUG(debug("step forwarding to cancel"))
        cancel();
        return;
      }
      
    }
    
    //-------------------------------------------------------------------------
    void MediaStream::cancel()
    {
      //.......................................................................
      // start the shutdown process
      
      //.......................................................................
      // try to gracefully shutdown
      
      if (!mGracefulShutdownReference) mGracefulShutdownReference = mThisWeak.lock();
      
      if (mGracefulShutdownReference) {
      }
      
      //.......................................................................
      // final cleanup
      
      get(mShutdown) = true;
      
      // make sure to cleanup any final reference to self
      mGracefulShutdownReference.reset();
    }
    
    //-----------------------------------------------------------------------
    void MediaStream::setError(WORD errorCode, const char *inReason)
    {
      String reason(inReason);
      
      if (0 != mLastError) {
        ZS_LOG_WARNING(Detail, debug("error already set thus ignoring new error") + ZS_PARAM("new error", errorCode) + ZS_PARAM("new reason", reason))
        return;
      }
      
      get(mLastError) = errorCode;
      mLastErrorReason = reason;
      
      ZS_LOG_WARNING(Detail, debug("error set") + ZS_PARAM("error", mLastError) + ZS_PARAM("reason", mLastErrorReason))
    }
  }
}
