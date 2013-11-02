/*
 * Copyright (C) 2013 SMB Phone Inc. / Hookflash Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include <ortc/types.h>
#include <ortc/IRTCTrack.h>

namespace ortc
{
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTCDTMFTrack
  #pragma mark
  
  interaction IRTCDTMFTrack : public IRTCTrack
  {
    void playTones(String tones, ULONG duration = 100, ULONG interToneGap = 70);
  };
  
  interaction IRTCDTMFTrackDelegate
  {
    virtual void onRTCDTMFTrackTone(String tone, ULONG duration) = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IRTCDTMFTrackDelegate)
ZS_DECLARE_PROXY_METHOD_2(onRTCDTMFTrackTone, String, ULONG)
ZS_DECLARE_PROXY_END()
