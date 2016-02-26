/* 1010 PCI/PCIe Audio Driver
   Copyright (c) Eugene Gavrilov, 2002-2016

   * This program is free software; you can redistribute it and/or modify
   * it under the terms of the GNU General Public License as published by
   * the Free Software Foundation; either version 2 of the License, or
   * (at your option) any later version.
   * 
   * This program is distributed in the hope that it will be useful,
   * but WITHOUT ANY WARRANTY; without even the implied warranty of
   * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   * GNU General Public License for more details.
   *
   * You should have received a copy of the GNU General Public License
   * along with this program; if not, write to the Free Software
   * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#pragma once

#pragma code_seg()
class Property
{
public:

	// helper functions:
    static NTSTATUS ValidatePropertyParams(IN PPCPROPERTY_REQUEST PropertyRequest, IN ULONG cbValueSize, IN ULONG cbInstanceSize = 0);
    static NTSTATUS PropertyHandler_BasicSupport(IN PPCPROPERTY_REQUEST PropertyRequest,IN ULONG Flags,IN DWORD PropTypeSetId,BOOL Multichannel);
    static NTSTATUS PropertyHandlerBasicSupportVolume(IN  PPCPROPERTY_REQUEST     PropertyRequest);

    // handlers:
    static NTSTATUS PropertyCpuResources(IN PPCPROPERTY_REQUEST   PropertyRequest);
    static NTSTATUS PropertyVolume(IN PPCPROPERTY_REQUEST   PropertyRequest);
    static NTSTATUS PropertyMute(IN PPCPROPERTY_REQUEST   PropertyRequest);

    // implementation
    /*
    virtual NTSTATUS SetMute(ULONG node,LONG channel,BOOL value)=0;
    virtual NTSTATUS GetMute(ULONG node,LONG channel,BOOL *value)=0;

    virtual NTSTATUS SetVolume(ULONG node,LONG channel,ULONG volume)=0;
    virtual NTSTATUS GetVolume(ULONG node,LONG channel,ULONG *volume)=0;
    */
};
