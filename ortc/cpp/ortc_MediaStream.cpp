/*
 
 Copyright (c) 2013, SMB Phone Inc. / Hookflash Inc.
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
#include <zsLib/Log.h>

namespace ortc { ZS_IMPLEMENT_SUBSYSTEM(ortclib) }

namespace ortc
{
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
      mDelegate(delegate),
      mError(0),
      mInactive(true)
    {
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
      return MediaStreamTrackListPtr();
    }
    
    //-----------------------------------------------------------------------
    MediaStreamTrackListPtr MediaStream::getVideoTracks()
    {
      return MediaStreamTrackListPtr();
    }

    //-----------------------------------------------------------------------
    IMediaStreamTrackPtr MediaStream::getTrackById(String trackId)
    {
      return IMediaStreamTrackPtr();
    }
    
    //-----------------------------------------------------------------------
    void MediaStream::addTrack(IMediaStreamTrackPtr track)
    {
      
    }
    
    //-----------------------------------------------------------------------
    void MediaStream::removeTrack(IMediaStreamTrackPtr track)
    {
      
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

    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    //-----------------------------------------------------------------------
    #pragma mark
    #pragma mark MediaStream => (internal)
    #pragma mark
    
    //-----------------------------------------------------------------------
    String MediaStream::log(const char *message) const
    {
      return String("MediaStream [") + Stringize<typeof(mID)>(mID).string() + "] " + message;
    }

  }
}
