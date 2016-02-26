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
#include "guids.h"
#include "adapter.h"

#pragma code_seg()
PcDevice *PcDevice::GetObject(DEVICE_OBJECT *fdo)
{
	if(fdo)
	{
		if(fdo->DeviceExtension)
		{
			PcDevice **extension = (PcDevice **)((PCHAR)fdo->DeviceExtension + PORT_CLASS_DEVICE_EXTENSION_SIZE);
			if(extension && *extension)
				return *extension;
		}
	}

	return NULL;
}

#pragma code_seg("PAGE")
NTSTATUS PcDevice::Add(PDRIVER_OBJECT  DriverObject, PDEVICE_OBJECT  PhysicalDeviceObject)
{
	PAGED_CODE();
	debug("PcDevice::Add: drvo: %p pdo: %p; next: %p\n",DriverObject,PhysicalDeviceObject,DriverObject->DeviceObject);

	NTSTATUS ret = PcAddAdapterDevice(DriverObject, PhysicalDeviceObject, PcDevice::Start, MAX_MINIPORTS, sizeof(void *) + PORT_CLASS_DEVICE_EXTENSION_SIZE);

	PDEVICE_OBJECT FunctionalDeviceObject=DriverObject->DeviceObject;

	while(FunctionalDeviceObject)
	{
		PcDevice **extension = (PcDevice **)((PCHAR)FunctionalDeviceObject->DeviceExtension + PORT_CLASS_DEVICE_EXTENSION_SIZE);
		if(extension && *extension==NULL)
		{
			PcDevice *device = new (NonPagedPool) PcDevice(FunctionalDeviceObject,PhysicalDeviceObject);
			if(device)
			{
				*extension = device;
				break;
			}
		}

		FunctionalDeviceObject=FunctionalDeviceObject->NextDevice;
	}

	return ret;
}

#pragma code_seg("PAGE")
NTSTATUS PcDevice::InstallSubdevice(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp,IN PWCHAR Name,IN REFGUID PortClassId,IN REFGUID MiniportClassId,
									IN PFNCREATEINSTANCE MiniportCreate,IN PUNKNOWN UnknownAdapter,
                                    IN PRESOURCELIST ResourceList,OUT PUNKNOWN *OutPortUnknown OPTIONAL)
{
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);
    ASSERT(Name);
    ASSERT(ResourceList);

    PPORT port=NULL;

    NTSTATUS ntStatus=PcNewPort(&port,PortClassId);

    if (NT_SUCCESS(ntStatus))
    {
        PUNKNOWN miniport=NULL;

        if (MiniportCreate)   // a function to create a proprietary miniport
      	{
        	ntStatus = MiniportCreate((PUNKNOWN *)&miniport, MiniportClassId, NULL, NonPagedPool);
      	}
      	else   // Ask PortCls for one of its built-in miniports.
		{
        	ntStatus = PcNewMiniport((PMINIPORT*)&miniport,MiniportClassId);
 		}

        if (NT_SUCCESS(ntStatus))
        {
            //
            //  Bind the port, miniport, and resources.
            //
            ntStatus = port->Init( DeviceObject,Irp,miniport,UnknownAdapter,ResourceList);

            if (NT_SUCCESS(ntStatus))
            {
                //
                // Register the subdevice (port/miniport combination).
                //
                ntStatus = PcRegisterSubdevice( DeviceObject, Name, port );
                if ( !NT_SUCCESS(ntStatus) )
                {
                    debug("!! PcDevice::InstallSubdevice: PcRegisterSubdevice failed\n");
                }
                else
                {
                    if (OutPortUnknown)
                    {
                    	// FIXME: strange, IID_IUnknown does not work...
                        NTSTATUS result=port->QueryInterface(IID_IPort,(PVOID *) OutPortUnknown);
                        if (!NT_SUCCESS(result))
                        	debug("!! PcDevice::InstallSubdevice: failed to query for unknown interface: %p, %p [%x]\n",port,miniport,result);
                        else
                        	debug("PcDevice::InstallSubdevice: port: %p, mini: %p, out: %p\n",port,miniport,*OutPortUnknown);
                    }
                }
            }
            else
            {
                debug("!! PcDevice::InstallSubdevice: port->Init failed [%x]\n",ntStatus);
            }
        }
        else
        {
            debug("!! PcDevice::InstallSubdevice: PcNewMiniport failed\n");
        }

        if(miniport)
        {
        	miniport->Release();
        	miniport=NULL;
        }

        port->Release();
    }
    else
    {
        debug("!! PcDevice::InstallSubdevice: PcNewPort failed\n");
    }

    return ntStatus;
}

#pragma code_seg("PAGE")
NTSTATUS PcDevice::Start(PDEVICE_OBJECT DeviceObject,PIRP Irp,PRESOURCELIST ResourceList)
{
	PAGED_CODE();
	PcDevice *device=PcDevice::GetObject(DeviceObject);
	if(valid_object(device,PcDevice))
		return device->Start(Irp,ResourceList);

    debug("!! PcDevice::Start: invalid object\n");
    return STATUS_UNSUCCESSFUL;
}

#pragma code_seg("PAGE")
NTSTATUS PcDevice::Start(PIRP Irp,PRESOURCELIST ResourceList)
{
	PAGED_CODE();
	debug("PcDevice::Start [do: %p]\n",DeviceObject);

    PUNKNOWN 		u_adapter  = NULL;
    PADAPTERCOMMON  padapter   = NULL;

    // create a new adapter common object
    NTSTATUS ntStatus = Adapter::Create( &u_adapter,IID_IAdapterCommon,NULL,NonPagedPool );
    if (NT_SUCCESS(ntStatus))
    {
        ASSERT( u_adapter );

        // query for the IAdapterCommon interface
        ntStatus = u_adapter->QueryInterface( IID_IAdapterCommon,(PVOID *)&padapter );
        if (NT_SUCCESS(ntStatus))
        {
            // Initialize the object
            ntStatus = padapter->Init(ResourceList, this, Irp);
        }
        else
        	debug("!! PcDevice::Start: failed to query adapter's interface [%x]\n",ntStatus);

        u_adapter->Release();
    }

	if(padapter)
		padapter->Release();

    return ntStatus;
}

#pragma code_seg("PAGE")
PcDevice::PcDevice(DEVICE_OBJECT *fdo,DEVICE_OBJECT *pdo)
{
	PAGED_CODE();

	magic=object_magic;
	DeviceObject=fdo;
	PhysicalDeviceObject=pdo;

	debug("PcDevice::PcDevice(): fdo: %p, pdo: %p [PcDevice: %p]\n",fdo,pdo,this);
}

#pragma code_seg("PAGE")
PcDevice::~PcDevice()
{
	PAGED_CODE();

	ASSERT(valid_object(this,PcDevice));

	DeviceObject=NULL;
	PhysicalDeviceObject=NULL;
	magic=0;

	debug("PcDevice::~PcDevice()\n");
}

#pragma code_seg()
int PcDevice::set_device_param(char *param,int type,void *data,int size)
{
    HANDLE reg;
    int ret=STATUS_UNSUCCESSFUL;

    if(IoOpenDeviceRegistryKey(PhysicalDeviceObject,PLUGPLAY_REGKEY_DEVICE,KEY_SET_VALUE,&reg)==STATUS_SUCCESS)
    {
                UNICODE_STRING value_name;
                ANSI_STRING ansi_value;
                RtlInitAnsiString(&ansi_value,param);
                RtlAnsiStringToUnicodeString(&value_name,&ansi_value,TRUE);

//              debug("-- set dev param [%s]\n",param);

                void *buff=data;
                UNICODE_STRING string_data; memset(&string_data,0,sizeof(string_data));

                ULONG type_=(ULONG)-1;

                switch(type)
                {
                 case CE_PARAM_DWORD: type_=REG_DWORD; break;
                 case CE_PARAM_BIN: type_=REG_BINARY; break;
                 case CE_PARAM_CHAR: 
                    type_=REG_SZ;
                    // transform ansi->unicode
                    ANSI_STRING value;
                    RtlInitAnsiString(&value,(PCSZ)data);
                    RtlAnsiStringToUnicodeString(&string_data,&value,TRUE);
                    buff=string_data.Buffer;
                    break;
                }

                if(type!=(ULONG)-1)
                 ret=ZwSetValueKey(reg,&value_name,NULL,type_,buff,(ULONG)size);
                else
                 ret=STATUS_INVALID_PARAMETER;

            ZwClose(reg);

            if(string_data.Buffer) // free
             RtlFreeUnicodeString(&string_data);

            RtlFreeUnicodeString(&value_name);

            return ret;
    }
    return ret;
}

#pragma code_seg()
int PcDevice::get_device_param(char *param,int /*type*/,void *data,int size,int *out_type,int *out_size)
{
    HANDLE reg;
    int ret=STATUS_UNSUCCESSFUL;

    if(IoOpenDeviceRegistryKey(PhysicalDeviceObject,PLUGPLAY_REGKEY_DEVICE,KEY_QUERY_VALUE,&reg)==STATUS_SUCCESS)
    {
                UNICODE_STRING value_name;
                ANSI_STRING ansi_value;
                RtlInitAnsiString(&ansi_value,param);

                RtlAnsiStringToUnicodeString(&value_name,&ansi_value,TRUE);

//              debug("-- get dev param [%s]\n",param);

                ULONG result_length=0;

                 ULONG result[40]; // KEY_VALUE_PARTIAL_INFORMATION + 4 -> 16 bytes
                 ret=ZwQueryValueKey(reg,&value_name,KeyValuePartialInformation,result,sizeof(result),&result_length);

                 if(ret==STATUS_SUCCESS)
                 {
                  switch(result[1])
                  {
                   default:
                   case REG_BINARY: *out_type=CE_PARAM_BIN; break;
                   case REG_DWORD: *out_type=CE_PARAM_DWORD; break;
                   case REG_SZ: 
                   case REG_EXPAND_SZ:
                    *out_type=CE_PARAM_CHAR; break;
                  }
                  *out_size=result[2];

                  if(size>=*out_size)
                  {
                   memcpy(data,&result[3],*out_size);
                  }
                 }

            ZwClose(reg);

            RtlFreeUnicodeString(&value_name);

            return ret;
    }
    return ret;
}
