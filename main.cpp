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

#if DBG
    #include "oem_res.h"

    static int trap_level=0;
    static int debug_level=2;

	int *drv_trap_level=&trap_level;
	int *drv_debug_level=&debug_level;

#endif

// embed copyright signature
//#pragma comment(exestr,"Copyright (c) Eugene Gavrilov")
static const char *unique_copyright_signature="author:$Eugene Gavrilov$ - Copyright (c) Eugene Gavrilov";
__inline const char *get_version() { return unique_copyright_signature; };

extern "C" DRIVER_INITIALIZE DriverEntry;

#pragma code_seg("INIT")
NTSTATUS DriverEntry(IN PDRIVER_OBJECT   DriverObject_,IN PUNICODE_STRING  RegistryPath)
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;

    debug("!! PCIAudio: driver start-up (Build: %s; %s) Revision: %s\n",__TIME__,__DATE__,RC_DRIVER_REVISION);

    get_version();

	if (!IoIsWdmVersionAvailable(6,0))
	{
		debug("!! PCIAudio:WaveRT unsupported\n");
		return STATUS_UNSUCCESSFUL;
	}

    PcDriver::DriverObject=DriverObject_;
    PcDriver::InitRegistryPath(RegistryPath);

    ntStatus=PcInitializeAdapterDriver(DriverObject_, RegistryPath, PcDevice::Add);

    if(!NT_SUCCESS(ntStatus))
     debug("!! PCIAudio: Error initializing PortCls\n");

    PcDriver::UnloadDriverPointer=DriverObject_->DriverUnload;
    DriverObject_->DriverUnload=PcDriver::DriverUnload;

    return(ntStatus);
}
