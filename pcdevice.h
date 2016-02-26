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
class PcDevice
{
  	static const MAX_MINIPORTS=2;
  	DEVICE_OBJECT *DeviceObject;			// fdo
  	DEVICE_OBJECT *PhysicalDeviceObject;

    enum { object_magic=0xdececc11 };
	int magic;

	public:

		DEVICE_OBJECT *GetDeviceObject(void) { return DeviceObject; };
		DEVICE_OBJECT *GetPhysicalDeviceObject(void) { return PhysicalDeviceObject; };

		// object:
		PcDevice(DEVICE_OBJECT *fdo, DEVICE_OBJECT *pdo);
		virtual ~PcDevice();
		static PcDevice *GetObject(DEVICE_OBJECT *fdo);

		// static members:
		static DRIVER_ADD_DEVICE Add; // static NTSTATUS Add(PDRIVER_OBJECT  DriverObject,PDEVICE_OBJECT  PhysicalDeviceObject);
		static NTSTATUS Start(PDEVICE_OBJECT DeviceObject,PIRP Irp,PRESOURCELIST ResourceList);
		static NTSTATUS InstallSubdevice(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irq,IN PWCHAR Name,IN REFGUID PortClassId,IN REFGUID MiniportClassId,
								  IN PFNCREATEINSTANCE MiniportCreate,IN PUNKNOWN UnknownAdapter,
                                  IN PRESOURCELIST ResourceList,OUT PUNKNOWN *OutPortUnknown OPTIONAL);

		// handlers:
		NTSTATUS Start(PIRP Irp,PRESOURCELIST ResourceList);

		// hardware parameters stored in the registry
	    int get_device_param(char *param,int type,void *data,int size,int *out_type,int *out_size);
	    int set_device_param(char *param,int type,void *data,int size);

	    bool verify_magic(void) { return (magic==object_magic); };
};

#if !defined(CE_PARAM_DWORD)
	// for get_device_param and set_device_param
	#define CE_PARAM_DWORD  1
	#define CE_PARAM_CHAR   2
	#define CE_PARAM_BIN    4
#endif
