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

#include "common.h"
#include "debug.h"
#include "pcdriver.h"
#include "pcdevice.h"

#pragma data_seg()
UNICODE_STRING PcDriver::registry_path={0,0,NULL};
PDRIVER_OBJECT PcDriver::DriverObject=NULL;
VOID (*PcDriver::UnloadDriverPointer)(IN PDRIVER_OBJECT  DriverObject_)=NULL;

#pragma code_seg()
VOID PcDriver::DriverUnload(IN PDRIVER_OBJECT DriverObject_)
{
 debug("PCIAudio: driver unloaded\n");
 if(UnloadDriverPointer)
  UnloadDriverPointer(DriverObject_);

 if(registry_path.Buffer)
 {
  ExFreePool(registry_path.Buffer);
  registry_path.Buffer=NULL;
 }

 DriverObject=0;
}

#pragma code_seg("INIT")
void PcDriver::InitRegistryPath(UNICODE_STRING *RegistryPath)
{
    registry_path.Length=registry_path.MaximumLength=RegistryPath->Length;
    registry_path.Buffer=(PWSTR)ExAllocatePoolWithTag(NonPagedPool,registry_path.Length+2,'10rg');
    if(registry_path.Buffer)
    {
     memset(registry_path.Buffer,0,registry_path.Length+2);
     memcpy(registry_path.Buffer,RegistryPath->Buffer,registry_path.Length);

     debug("PCIAudio: RegistryPath: '%ws'\n",registry_path.Buffer);
    }
}
