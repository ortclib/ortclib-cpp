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

namespace ortc
{
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  //-------------------------------------------------------------------------
  #pragma mark
  #pragma mark IRTCDataChannel
  #pragma mark
  
  interaction IRTCDataChannel
  {
    virtual void send(BYTE* data) = 0;
  };
  
  interaction IRTCDataChannelDelegate
  {
    virtual void onRTCDataChannelData(BYTE* data) = 0;
  };
}

ZS_DECLARE_PROXY_BEGIN(ortc::IRTCDataChannelDelegate)
ZS_DECLARE_PROXY_METHOD_1(onRTCDataChannelData, BYTE*)
ZS_DECLARE_PROXY_END()
