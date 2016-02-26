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
#include "topology.h"
#include "wave.h"
#include "wavestream.h"

#include "hal.h"
#include "aud_utils.h"

#pragma code_seg("PAGE")
NTSTATUS Adapter::Create(OUT PUNKNOWN *Unknown,IN REFCLSID,IN PUNKNOWN UnknownOuter OPTIONAL,IN POOL_TYPE PoolType)
{
    PAGED_CODE();

    ASSERT(Unknown);

    STD_CREATE_BODY_(Adapter,Unknown,UnknownOuter,PoolType,PADAPTERCOMMON);
}   

#pragma code_seg("PAGE")
NTSTATUS Adapter::Init(IN      PRESOURCELIST   ResourceList,IN      PcDevice *device_, PIRP Irp)
{
    PAGED_CODE();

    debug("Adapter::Init [%p]\n",this);

    PowerState=PowerDeviceD0;

    magic=object_magic;
    device=device_;
	topology=NULL;
	wave=NULL;

	InterruptSync=NULL;

	KeInitializeDpc(&dpc,dpc_func,this);
	KeSetImportanceDpc(&dpc,HighImportance);

	NTSTATUS ntStatus = STATUS_SUCCESS;

	PUNKNOWN u_topology = NULL;
	PUNKNOWN u_wave = NULL;

    if(NT_SUCCESS(ntStatus))
    {
        ntStatus = device->InstallSubdevice(device->GetDeviceObject(),
                                    Irp,
                                    L"topology",
                                    CLSID_PortTopology, // port class
                                    CLSID_PortTopology, // unused miniport class id
                                    Topology::Create, // miniport creation func
                                    PUNKNOWN((PADAPTERCOMMON)this),
                                    ResourceList,
                                    &u_topology
                                    ); 
    }

    if(NT_SUCCESS(ntStatus))
    {
        ntStatus = device->InstallSubdevice(device->GetDeviceObject(),
                                    Irp,
                                    L"wave",
                                    CLSID_PortWaveRT, // port class
                                    CLSID_PortWaveRT, // unused miniport class id
                                    Wave::Create, // miniport creation func
                                    PUNKNOWN((PADAPTERCOMMON)this),
                                    ResourceList,
                                    &u_wave
                                    ); 
    }

    // register connections
    if (u_topology && u_wave)
    {
    	ntStatus = PcRegisterPhysicalConnection( device->GetDeviceObject(), u_topology, TOPO_WAVEIN_DEST, u_wave, WAVE_WAVEIN_DEST);
    	if(NT_SUCCESS(ntStatus))
    		ntStatus = PcRegisterPhysicalConnection( device->GetDeviceObject(), u_wave, WAVE_WAVEOUT_DEST, u_topology, TOPO_WAVEOUT_SOURCE);

    	if(!NT_SUCCESS(ntStatus))
    		debug("!! Adapter::Init: failed to register connections [%x]\n",ntStatus);
    }
    else
    	debug("!! Adapter::Init: failed to create wave/topology [%p / %p]\n",u_topology,u_wave);

    if (u_topology)
        u_topology->Release();

    if (u_wave)
    {
    	if(wave)
    	{
    		NTSTATUS r=u_wave->QueryInterface (IID_IPortClsSubdeviceEx, (PVOID *)&wave->update_pin_descriptor_p);
    		if(!NT_SUCCESS(r) || !wave->update_pin_descriptor_p)
    			debug("!! Adapter::init: subdeviceex failed: %08x, %p\n",r,wave->update_pin_descriptor_p);
    	}

        u_wave->Release();
    }

    if (!NT_SUCCESS(ntStatus)) return ntStatus;

    setup_device_name_in_registry();

    // now setup hardware
    hal = new (NonPagedPool) Hal((word)(ResourceList->FindTranslatedPort(0)->u.Port.Start.LowPart),this);

    if(hal)
    {
    	// initialize interrupt subsystem
        ntStatus = PcNewInterruptSync(&InterruptSync,NULL,ResourceList,0,InterruptSyncModeNormal);     // Run ISRs once until we get SUCCESS
        if (NT_SUCCESS(ntStatus) && InterruptSync)
        {
            ntStatus = InterruptSync->RegisterServiceRoutine(InterruptServiceRoutine,PVOID(this),TRUE); //  run this ISR first
            if(NT_SUCCESS(ntStatus))
            {
                ntStatus = InterruptSync->Connect();

                if(NT_SUCCESS(ntStatus))
                {
                	// Process settings: set up defaults, load preferreds from the registry

                	hardware_parameters_t params;

                	// default settings:
                    sampling_rate_locked=false;
                    volume_locked=false;
                    max_sampling_rate=0;
                    test_sine_wave=false;

                	params.sampling_rate=44100;
                	params.n_channels=2;
                	params.clock_source=InternalClock;

                	params.is_spdif_pro=false;
                	params.spdif_tx_no_copy_bit=false;
                	params.is_optical_in_adat=false; // spdif
                	params.is_optical_out_adat=false; // spdif

                	params.is_not_audio=false; // regular PCM, not AC3

                	// get settings from the registry
                	DWORD value=0; int out_type=0; int out_size=0;

                	if(device->get_device_param("sampling_rate",CE_PARAM_DWORD,&value,sizeof(value),&out_type,&out_size)==0)
                		params.sampling_rate=(int)value;
                	if(device->get_device_param("n_channels",CE_PARAM_DWORD,&value,sizeof(value),&out_type,&out_size)==0)
                		params.n_channels=(int)value;
                	if(device->get_device_param("clock_source",CE_PARAM_DWORD,&value,sizeof(value),&out_type,&out_size)==0)
                		params.clock_source=(clocksource_t)value;

                	if(device->get_device_param("is_spdif_pro",CE_PARAM_DWORD,&value,sizeof(value),&out_type,&out_size)==0)
                		params.is_spdif_pro=!!value;
                	if(device->get_device_param("spdif_tx_no_copy_bit",CE_PARAM_DWORD,&value,sizeof(value),&out_type,&out_size)==0)
                		params.spdif_tx_no_copy_bit=!!value;
                	if(device->get_device_param("is_optical_in_adat",CE_PARAM_DWORD,&value,sizeof(value),&out_type,&out_size)==0)
                		params.is_optical_in_adat=!!value;
                	if(device->get_device_param("is_optical_out_adat",CE_PARAM_DWORD,&value,sizeof(value),&out_type,&out_size)==0)
                		params.is_optical_out_adat=!!value;

                	// "is_not_audio" is not restored by default
					//if(device->get_device_param("is_not_audio",CE_PARAM_DWORD,&value,sizeof(value),&out_type,&out_size)==0)
                	//	params.is_not_audio=!!value;

                	// "lock_status" is read-only

                	// software-only parameters managed by Adapter class (not 'Hal'):

                	if(device->get_device_param("sampling_rate_locked",CE_PARAM_DWORD,&value,sizeof(value),&out_type,&out_size)==0)
                		sampling_rate_locked=!!value;
                	if(device->get_device_param("volume_locked",CE_PARAM_DWORD,&value,sizeof(value),&out_type,&out_size)==0)
                		volume_locked=!!value;
                	if(device->get_device_param("max_sampling_rate",CE_PARAM_DWORD,&value,sizeof(value),&out_type,&out_size)==0)
                		max_sampling_rate=(int)value;

                	// init hardware with these settings:

                	if(hal->init()==0) // OK?
                	{
                		debug("Adapter::Init: initialization OK; driver allocated: %d bytes\n",memory_handle_t::pool_size);
                		debug("Adapter::Init: hal: %p, wave: %p topo: %p dev: %p; ada: %p\n",hal, wave, topology, device, this);

                		hal->set_hardware_settings(&params);

                        // register with PortCls for power-managment services
                        if(NT_SUCCESS(ntStatus))
                        {
                        	ntStatus = PcRegisterAdapterPowerManagement( (PUNKNOWN)(PADAPTERCOMMON)this, device->GetDeviceObject() );
            	            if(!NT_SUCCESS(ntStatus))
            	               debug("!!! Adapter::Init: Error registering with Power Management %08x\n",ntStatus);
            	        }
            	        else
            	        	debug("Adapter::Init: registered power adapter notifications\n");

            	        // update max_sampling_rate
            	        if(max_sampling_rate)
            	        {
            	        	wave->update_dataranges(max_sampling_rate);
            	        	wave->generate_format_change_event();
            	        }

                        return STATUS_SUCCESS;
                    }
                    else
                    {
                    	debug("!! Adapter::Init: hal initialization failed\n");
                    	ntStatus=STATUS_INVALID_PARAMETER;
                    }
                } // Connect OK?
                else
                	debug("!! Adapter::Init: failed to connect interrupt [%x]\n",ntStatus);

                // it seems there is no 'unregister' command

            } // Register OK?
            else
             debug("!! Adapter::Init: failed to register interrupt [%x]\n",ntStatus);

        } // NewInterruptSync OK?
        else
         debug("!! Adapter::Init: failed to create interrupt [%x]\n",ntStatus);

  		delete hal; hal=NULL;
    }
    else
    {
			debug("!! Adapter::Init: hal allocation failed\n");
    		ntStatus=STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

#pragma code_seg()
void Adapter::save_hardware_settings(void)
{
	hardware_parameters_t params;
	memset(&params,0,sizeof(params));

	if(hal->get_hardware_settings(&params)==0)
	{
         // save settings into the registry
         DWORD value;

         value=params.sampling_rate; 		device->set_device_param("sampling_rate",CE_PARAM_DWORD,&value,sizeof(value));
         value=params.n_channels; 			device->set_device_param("n_channels",CE_PARAM_DWORD,&value,sizeof(value));
         value=params.clock_source; 		device->set_device_param("clock_source",CE_PARAM_DWORD,&value,sizeof(value));

         value=params.is_spdif_pro; 		device->set_device_param("is_spdif_pro",CE_PARAM_DWORD,&value,sizeof(value));
         value=params.spdif_tx_no_copy_bit; device->set_device_param("spdif_tx_no_copy_bit",CE_PARAM_DWORD,&value,sizeof(value));
         value=params.is_optical_in_adat; 	device->set_device_param("is_optical_in_adat",CE_PARAM_DWORD,&value,sizeof(value));
         value=params.is_optical_out_adat; 	device->set_device_param("is_optical_out_adat",CE_PARAM_DWORD,&value,sizeof(value));

         // stored, but not restored:
         value=params.is_not_audio; 		device->set_device_param("is_not_audio",CE_PARAM_DWORD,&value,sizeof(value));
         // lock_status is not saved, because it is read-only

         // three software parameters managed by Adapter, not 'Hal'
         value=sampling_rate_locked;		device->set_device_param("sampling_rate_locked",CE_PARAM_DWORD,&value,sizeof(value));
         value=volume_locked;				device->set_device_param("volume_locked",CE_PARAM_DWORD,&value,sizeof(value));
         value=max_sampling_rate; 			device->set_device_param("max_sampling_rate",CE_PARAM_DWORD,&value,sizeof(value));
	}
}

#pragma code_seg()
Adapter::~Adapter()
{
	ASSERT(valid_object(this,Adapter));

    debug("Adapter::~Adapter [%p]\n",this);

    // FIXME: this fails:
    // PcUnregisterAdapterPowerManagement( device->GetDeviceObject() );
    // debug("Adapter::~Adapter: unregistered powere management\n");

    // these should have been destroyed already
    ASSERT(wave==0);
    ASSERT(topology==0);

    if(hal)
    {
    	delete hal;	// implies close()
    	hal=NULL;
    }

    if (InterruptSync)
    {
        InterruptSync->Disconnect();
        InterruptSync->Release();
        InterruptSync = NULL;
    }

    // need to remove associated PcDevice
    if(device)
    {
    	// device is not COM-based, no need to call device->Release()
    	delete device;
    	device=NULL;
    }

    if(memory_handle_t::pool_size)
    {
    	debug("!! Adapter::~Adapter: driver leaked %d bytes of contiguous memory\n",memory_handle_t::pool_size);
    }

    magic=NULL;
}

#pragma code_seg("PAGE")
STDMETHODIMP Adapter::NonDelegatingQueryInterface(REFIID  Interface,PVOID * Object)
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PADAPTERCOMMON(this)));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IAdapterCommon))
    {
        *Object = PVOID(PADAPTERCOMMON(this));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IAdapterPowerManagement2))
    {
        *Object = PVOID(PADAPTERPOWERMANAGMENT2(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IAdapterPowerManagment))
    {
        *Object = PVOID(PADAPTERPOWERMANAGMENT(this));
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

#pragma code_seg("PAGE")
void Adapter::setup_device_name_in_registry(void)
{
    PAGED_CODE();

    CONST GUID *device_guids[]=
    {
     &KSCATEGORY_AUDIO,
     &KSCATEGORY_RENDER,
     &KSCATEGORY_CAPTURE,
     &KSCATEGORY_TOPOLOGY,
     &KSCATEGORY_REALTIME
    };

    // try getting device interfaces
    for(int i=0;i<sizeof(device_guids)/sizeof(device_guids[0]);i++)
    {
        PWSTR list;

        NTSTATUS st=IoGetDeviceInterfaces(device_guids[i],device->GetPhysicalDeviceObject(),DEVICE_INTERFACE_INCLUDE_NONACTIVE,&list);
        if(NT_SUCCESS(st))
        {
         unsigned short *p=list;
         while(*p)
         {
            UNICODE_STRING interface_unicode;
            RtlInitUnicodeString(&interface_unicode, p);

            HANDLE reg;
            st=IoOpenDeviceInterfaceRegistryKey(&interface_unicode,KEY_ALL_ACCESS,&reg);
            if(NT_SUCCESS(st))
            {
                  wchar_t tmp_buffer[64]; memset(&tmp_buffer,0,sizeof(tmp_buffer));
                  bool to_skip=false;

                  if(wcsstr(p,L"\\wave")!=0)
                  	RtlStringCbPrintfW(tmp_buffer,sizeof(tmp_buffer),L"1010 Wave");
                  else
                   if(wcsstr(p,L"\\topology")!=0)
                      RtlStringCbPrintfW(tmp_buffer,sizeof(tmp_buffer),L"1010 Topology");
                     else to_skip=true;

                  if(!to_skip)
                  {
                  	  UNICODE_STRING key_name;
                  	  RtlInitUnicodeString(&key_name,L"FriendlyName");

                  	  UNICODE_STRING key_value;
                  	  RtlInitUnicodeString(&key_value,tmp_buffer); // FriendlyName -> tmp_name

                      ZwSetValueKey(reg,&key_name,0,REG_SZ,key_value.Buffer,(ULONG)(wcslen(key_value.Buffer)*2+2));

                      RtlInitUnicodeString(&key_name,L"CLSID");
                      RtlInitUnicodeString(&key_value,L"{17CCA71B-ECD7-11D0-B908-00A0C9223196}");

                      ZwSetValueKey(reg,&key_name,0,REG_SZ,key_value.Buffer,(ULONG)(wcslen(key_value.Buffer)*2+2));

                      // enable 'pull' mode on Win7 and later
                      // HKR,"EP\\0",%PKEY_AudioEndpoint_Supports_EventDriven_Mode%,%REG_DWORD%,0x00000001
                      // PKEY_AudioEndpoint_Supports_EventDriven_Mode = "{1DA5D803-D492-4EDD-8C23-E0C0FFEE7F0E},7"

                      {
                          OBJECT_ATTRIBUTES ObjectAttributes;

                          HANDLE subreg=NULL;

                          UNICODE_STRING subkey_name; RtlInitUnicodeString(&subkey_name,L"EP");
                          InitializeObjectAttributes(&ObjectAttributes,&subkey_name,OBJ_KERNEL_HANDLE,reg,NULL);

                          if(NT_SUCCESS(ZwCreateKey(&subreg,KEY_ALL_ACCESS,&ObjectAttributes,NULL,NULL,REG_OPTION_NON_VOLATILE,NULL)))
                          {
                          		HANDLE subsubreg=NULL;

                          		UNICODE_STRING subsubkey_name; RtlInitUnicodeString(&subsubkey_name,L"0");
                                InitializeObjectAttributes(&ObjectAttributes,&subsubkey_name,OBJ_KERNEL_HANDLE,subreg,NULL);

                                if(NT_SUCCESS(ZwCreateKey(&subsubreg,KEY_ALL_ACCESS,&ObjectAttributes,NULL,NULL,REG_OPTION_NON_VOLATILE,NULL)))
                                {
                                    RtlInitUnicodeString(&key_name,L"{1DA5D803-D492-4EDD-8C23-E0C0FFEE7F0E},7");
                                    DWORD dword_value=0x1;
                                    ZwSetValueKey(subsubreg,&key_name,0,REG_DWORD,&dword_value,sizeof(DWORD));

                                	ZwClose(subsubreg);
                                }
                          		ZwClose(subreg);
                          }
                      }

                      debug("Adaper::setup_device_name_in_registry: updated key: %S\n",p);
                  }

                  ZwClose(reg);
            } 
             else debug("!! Adapter::setup_device_name_in_registry: failed to open device interfaces registry key! [%x]\n",st);

            // skip current and goto next
            while(*p)
             p++;
            p++;
         } // while(*p)

         ExFreePool(list);

        }
         else debug("!! Adapter::setup_device_name_in_registry: failed getting device interfaces! [%x]\n",st);
    } // for each category
}

#pragma code_seg("PAGE")
NTSTATUS Adapter::PropertyPrivate(IN PPCPROPERTY_REQUEST   PropertyRequest, Wave * /*wave_target*/, WaveStream *wave_stream)
{
	PAGED_CODE();

	int result=0;

	drvioctl_t *in=(drvioctl_t *)PropertyRequest->Instance;
	drvioctl_t *out=(drvioctl_t *)PropertyRequest->Value;

	if(in && out && PropertyRequest->ValueSize==sizeof(drvioctl_t) && PropertyRequest->InstanceSize==sizeof(drvioctl_t))
	{
		hardware_parameters_t hw_params; memset(&hw_params,0,sizeof(hw_params));

		if(hal->get_hardware_settings(&hw_params))
		{
			debug("!! Adapter::PropertyPrivate: get_hardware_settings failed\n");
			return STATUS_UNSUCCESSFUL;
		}

		switch(in->command)
		{
			case DRVIOCTL_GET_HW_PARAMETERS:
				out->get_hw_parameters.sampling_rate=hw_params.sampling_rate;
				// buffer_size is unused
				out->get_hw_parameters.clock_source=(int)hw_params.clock_source;
				out->get_hw_parameters.is_optical_out_adat=hw_params.is_optical_out_adat;
				out->get_hw_parameters.is_optical_in_adat=hw_params.is_optical_in_adat;
			    out->get_hw_parameters.sampling_rate_locked=sampling_rate_locked;
			    out->get_hw_parameters.volume_locked=volume_locked;
			    out->get_hw_parameters.is_spdif_pro=hw_params.is_spdif_pro;
				out->get_hw_parameters.n_channels=hw_params.n_channels;
				out->get_hw_parameters.is_not_audio=(int)hw_params.is_not_audio;
				out->get_hw_parameters.spdif_tx_no_copy_bit=(int)hw_params.spdif_tx_no_copy_bit;
				out->get_hw_parameters.max_sampling_rate=max_sampling_rate;
				out->get_hw_parameters.dsp_bypass=(int)hal->dsp.get_bypass();
				// fpga_reload is not used
                out->get_hw_parameters.spdif_in_frequency = hal->get_clock_frequency(SPDIF);
                out->get_hw_parameters.adat_in_frequency = hal->get_clock_frequency(ADAT);
                out->get_hw_parameters.bnc_in_frequency = hal->get_clock_frequency(BNC);
                out->get_hw_parameters.spdif2_in_frequency = hal->get_clock_frequency(Dock);
                out->get_hw_parameters.lock_status = hw_params.lock_status;
                hal->get_buffer_sizes(&out->get_hw_parameters.max_buffer_size,
                					  &out->get_hw_parameters.min_buffer_size,
                					  &out->get_hw_parameters.buffer_size_gran,
                					  &out->get_hw_parameters.hw_in_latency,
                					  &out->get_hw_parameters.hw_out_latency);

                out->get_hw_parameters.test_sine_wave=test_sine_wave;

				result=0;
				break;

			case DRVIOCTL_GET_HW_PARAMETER:

				switch((ParameterId)in->set_hw_parameter.id)
				{
					case SamplingRate: 		out->get_hw_parameter.value=hw_params.sampling_rate; break;
                    /* case BufferSize: */
                    case ClockSource: 		out->get_hw_parameter.value=(int)hw_params.clock_source;
                    case OpticalOutADAT:	out->get_hw_parameter.value=hw_params.is_optical_out_adat; break;
                    case OpticalInADAT:		out->get_hw_parameter.value=hw_params.is_optical_in_adat; break;
                    case LockSampleRate:	out->get_hw_parameter.value=sampling_rate_locked; break;
                    case LockVolume:		out->get_hw_parameter.value=volume_locked; break;
                    case SpdifMode:			out->get_hw_parameter.value=hw_params.is_spdif_pro; break;
                    case NumberOfChannels:	out->get_hw_parameter.value=hw_params.n_channels; break;
                    case AC3Mode:			out->get_hw_parameter.value=hw_params.is_not_audio; break;
                    case SpdifNoCopy:		out->get_hw_parameter.value=hw_params.spdif_tx_no_copy_bit; break;
                    case MaxSamplingRate:	out->get_hw_parameter.value=max_sampling_rate; break;
                    case DSPBypass:			out->get_hw_parameter.value=(int)hal->dsp.get_bypass(); break;
                    /* case FPGAReload:	*/                    
                    case SPDIFInFrequency:	out->get_hw_parameter.value=hal->get_clock_frequency(SPDIF); break;
                    case ADATInFrequency:	out->get_hw_parameter.value=hal->get_clock_frequency(ADAT); break;
                    case BNCInFrequency:	out->get_hw_parameter.value=hal->get_clock_frequency(BNC); break;
                    case SPDIF2InFrequency:	out->get_hw_parameter.value=hal->get_clock_frequency(Dock); break;
                    case LockStatus:		out->get_hw_parameter.value=hw_params.lock_status; break;
                    case MaxBufferSize:		hal->get_buffer_sizes(&out->get_hw_parameter.value,NULL,NULL,NULL,NULL); break;
                    case MinBufferSize:		hal->get_buffer_sizes(NULL,&out->get_hw_parameter.value,NULL,NULL,NULL); break;
                    case BufferSizeGran:	hal->get_buffer_sizes(NULL,NULL,&out->get_hw_parameter.value,NULL,NULL); break;
                    case HwInLatency:		hal->get_buffer_sizes(NULL,NULL,NULL,&out->get_hw_parameter.value,NULL); break;
                    case HwOutLatency:		hal->get_buffer_sizes(NULL,NULL,NULL,NULL,&out->get_hw_parameter.value); break;
                    case TestSineWave:		out->get_hw_parameter.value=test_sine_wave; break;
                    /* case ResetDSP: */
                    /* case ResetFPGA: */

                    default:
						debug("!! Adapter::PropertyPrivate: invalid parameter id: %d [set]\n",in->set_hw_parameter.id);
						return STATUS_INVALID_PARAMETER;
				}
				result=0;
				break;

			case DRVIOCTL_SET_HW_PARAMETER:
				int flag;
				bool need_sr_notification;
				bool need_datarange_update;

				flag=0;
				need_sr_notification=false;
				need_datarange_update=false;

				switch((ParameterId)in->set_hw_parameter.id)
				{
					case MaxSamplingRate:

							if(max_sampling_rate!=in->set_hw_parameter.value)
							{
								need_datarange_update=true;	// update dataranges
							}

							max_sampling_rate=in->set_hw_parameter.value;

							if(in->set_hw_parameter.value!=0)	// set maximum sampling rate limit?
							{
								if(hw_params.sampling_rate>max_sampling_rate)
								{
									// reset sampling rate, because current one is higher than the maximum
                                    in->set_hw_parameter.value=max_sampling_rate;
									// NOTE: fall through to case SamplingRate: below
								}
								else
									break;
							}
							else // remove limit
								break;
					case SamplingRate:

							hardware_parameters_t new_hw_params;
							memcpy(&new_hw_params,&hw_params,sizeof(hardware_parameters_t));
							new_hw_params.sampling_rate=in->set_hw_parameter.value; 

							if(new_hw_params.sampling_rate!=hw_params.sampling_rate) // need changes?
							{
    							// verify that we can change the rate
                                if(hal->will_accept_hardware_settings(&hw_params,&new_hw_params)==0) // ok?
                                {
                                	flag=HW_SET_ALL;
                                	if(new_hw_params.sampling_rate!=hw_params.sampling_rate)
    									need_sr_notification=true;

    								hw_params.sampling_rate=new_hw_params.sampling_rate;
                                }
                                else
                                {
                                	return STATUS_DEVICE_BUSY;
                                }
                            } // else: nop

							break; // sampling rate
					/* case BufferSize: break; */ // buffer size 
					case ClockSource: flag=HW_SET_CLOCK; hw_params.clock_source=(clocksource_t)in->set_hw_parameter.value; break; // clock source
					case OpticalOutADAT: flag=HW_SET_SPDIF_IO; hw_params.is_optical_out_adat=!!in->set_hw_parameter.value; break; // optical out adat
					case OpticalInADAT: flag=HW_SET_SPDIF_IO; hw_params.is_optical_in_adat=!!in->set_hw_parameter.value; break; // optical in adat
					case LockSampleRate: sampling_rate_locked=!!in->set_hw_parameter.value; break; // lock sample rate
					case LockVolume: volume_locked=!!in->set_hw_parameter.value; 
							if(volume_locked)
								apply_volume(0,0,true); // force volume change
							else
								topology->update_volume();
							break; // lock volume
					case SpdifMode: flag=HW_SET_SPDIF_IO; hw_params.is_spdif_pro=!!in->set_hw_parameter.value; break; // spdif pro vs consumer (is_spdif_pro)
					case FPGAReload: 
							debug("Adapter::PropertyPrivate: reload FPGA firmware\n");
							hal->reload_fpga_firmware(true); 
							flag=HW_SET_ALL;
							break; // force reload
					case DSPBypass: debug("Adapter:PropertyPrivate: setting DSP mode to %d\n",(dsp_bypass_e)in->set_hw_parameter.value); hal->set_dsp_bypass((dsp_bypass_e)in->set_hw_parameter.value); break;
					case TestSineWave: test_sine_wave=!!in->set_hw_parameter.value; break;
					case ResetDSP: debug("Adapter::PropertyPrivate: reset DSP\n"); hal->dsp.reset(hw_params.sampling_rate); break;
					case ResetFPGA: debug("Adapter::PropertyPrivate: reset FPGA\n"); flag=HW_SET_FPGA_FIRMWARE|HW_SET_SPDIF_IO|HW_SET_CLOCK; break; 

					case NumberOfChannels:
					case AC3Mode:
					case SpdifNoCopy:
					case SPDIFInFrequency:
					case ADATInFrequency:
					case BNCInFrequency:
					case SPDIF2InFrequency:
					case LockStatus:
					case MaxBufferSize:
					case MinBufferSize:
					case BufferSizeGran:
					case HwInLatency:
					case HwOutLatency:
						debug("!! Adapter::PropertyPrivate: parameter is read-only! %d [set]\n",in->set_hw_parameter.id);
						return STATUS_INVALID_PARAMETER;
					default:
						debug("!! Adapter::PropertyPrivate: invalid parameter id: %d [set]\n",in->set_hw_parameter.id);
						return STATUS_INVALID_PARAMETER;
				}

				if(flag) // need to change hardware setting?
				{
					result=hal->set_hardware_settings(&hw_params,flag);
				}

				if(result==0)
				{
  					if(need_datarange_update)
  					{
  						if(valid_object(wave,Wave))
  							wave->update_dataranges(max_sampling_rate);
  						else
  							debug("!! Adapter::Private: invalid 'wave' object [%p]\n",wave);
  					}

					if(need_sr_notification)
  					{
  						if(valid_object(wave,Wave))
  							wave->generate_format_change_event();
  						else
  							debug("!! Adapter::Private: invalid 'wave' object [%p]\n",wave);
  					}

					// if ok, save settings into the registry
					save_hardware_settings();
				}
				break;

			case DRVIOCTL_GET_DEVICE_FORMAT:
				{
                     out->d_format.cur_n_outs=hw_params.n_channels;
                     out->d_format.cur_n_ins=hw_params.n_channels;
                     out->d_format.cur_device_sr=convRateNum2Ce(hw_params.sampling_rate);
                     out->d_format.cur_device_bps=CEAPI_BPS_LSB_24PD;
				}
				break;

			case DRVIOCTL_GET_DEVICE_INFO:
				{
                     out->d_info.device_type=CEAPI_DEVICE_AUDIO;
                     out->d_info.caps.audio.n_ins=hw_params.n_channels;
                     out->d_info.caps.audio.n_outs=hw_params.n_channels;
                     out->d_info.caps.audio.sr_support=CEAPI_SR_44100|CEAPI_SR_48000|CEAPI_SR_88200|CEAPI_SR_96000|CEAPI_SR_176400|CEAPI_SR_192000;
                     out->d_info.caps.audio.bps_support=CEAPI_BPS_LSB_24PD;
                     out->d_info.caps.audio.format_support=CEAPI_FORMAT_PCM;

                     RtlStringCchPrintfA(out->d_info.device_guid,sizeof(out->d_info.device_guid),"CECE-PCI-%08x-%08x",
                     	((hal->get_device_info(Hal::PCI_ID_VEN)&0xffff)<<16)|hal->get_device_info(Hal::PCI_ID_DEV),
                     	hal->get_device_info(Hal::PCI_ID_SUBSYS));
                     RtlStringCchPrintfA(out->d_info.friendly_name,sizeof(out->d_info.friendly_name),"1010");
                     RtlStringCchPrintfA(out->d_info.vendor_name,sizeof(out->d_info.vendor_name),"E-mu");
                     RtlStringCchPrintfA(out->d_info.unique_id,sizeof(out->d_info.unique_id),"%04x%04x%08x%04x",
                     	hal->get_device_info(Hal::PCI_ID_VEN),
                     	hal->get_device_info(Hal::PCI_ID_DEV),
                     	hal->get_device_info(Hal::PCI_ID_SUBSYS),
                     	hal->get_device_info(Hal::PCI_PORT));
				}
				break;

			case DRVIOCTL_SET_ASIO_X:
				{
					if(valid_object(wave_stream,WaveStream))
					{
                        wave_stream->is_asio=true;
                    }
                    else
                    {
                    	debug("!! Adapter::PropertyPrivate: invalid target [%p]\n",wave_stream);
                    	result=STATUS_INVALID_PARAMETER;
                    }
				}
				break;

			case DRVIOCTL_GET_ASIO_BUFFERS:
				{
					hal->get_buffer_sizes(&out->get_asio_buffers.max_buffer,&out->get_asio_buffers.min_buffer,&out->get_asio_buffers.buffer_gran,
							NULL,NULL,in->get_asio_buffers.sampling_rate);
					out->get_asio_buffers.sampling_rate=in->get_asio_buffers.sampling_rate;
				}
				break;

			default:
				debug("!! Adapter::PropertyPrivate: invalid command [%d]\n",in->command);
				return STATUS_INVALID_PARAMETER;
		}
	}
	else
	{
		debug("!! Adapter::PropertyPrivate: invalid size %d/%d\n",PropertyRequest->ValueSize,PropertyRequest->InstanceSize);
		result=STATUS_INVALID_PARAMETER;
	}

	return result;
}

#pragma code_seg("PAGE")
void Adapter::apply_volume(LONG left,LONG right,bool force)
{
	PAGED_CODE();

	if(force || !volume_locked)
	{
		hal->dsp.apply_volume(left,right);
	}
}

#pragma code_seg("PAGE")
int Adapter::can_change_hardware_settings(hardware_parameters_t *old_params,hardware_parameters_t *new_params)
{
	PAGED_CODE();

	// is only called by Wave::VerifyFormat

	int ret=0;

    ret=hal->will_accept_hardware_settings(old_params,new_params);
   	if(ret) // failed?
   		return ret;

   	// verify that sampling_rate_locked is OK
    if(sampling_rate_locked)
    {
    	if(old_params->sampling_rate != new_params->sampling_rate ||
    	   old_params->n_channels != new_params->n_channels)
    	{
    		debug("Adapter::can_change_hardware_settings: sample rate locked, won't change %d->%d / %d->%d\n",
    			old_params->sampling_rate,new_params->sampling_rate,
    			old_params->n_channels,new_params->n_channels);

    		return STATUS_DEVICE_BUSY;
    	}
    }

	return 0;
}

#pragma code_seg()
STDMETHODIMP_(NTSTATUS) Adapter::QueryPowerChangeState(IN      POWER_STATE     NewState)
{
    debug("Adapter::QueryPowerChangeState\n");

    // Check here to see of a legitimate state is being requested
    // based on the device state and fail the call if the device/driver
    // cannot support the change requested.  Otherwise, return STATUS_SUCCESS.
    // Note: A QueryPowerChangeState() call is not guaranteed to always preceed
    // a PowerChangeState() call.
    // is this actually a state change
    // switch on new state
    switch( NewState.DeviceState )
    {
            case PowerDeviceD0:
            case PowerDeviceD1:
            case PowerDeviceD2:
            case PowerDeviceD3:
            	return STATUS_SUCCESS;
            default:
                debug("!!! Adapter::QueryPowerChangeState: Unknown Device Power State\n");
    }

    return STATUS_INVALID_PARAMETER;
}

#pragma code_seg()
STDMETHODIMP_(NTSTATUS) Adapter::QueryDeviceCapabilities(IN      PDEVICE_CAPABILITIES    /*PowerDeviceCaps*/)
{
    debug("Adapter::QueryDeviceCapabilities\n");

    return STATUS_SUCCESS;
}

#pragma code_seg()
STDMETHODIMP_(void) Adapter::PowerChangeState(IN      POWER_STATE     NewState)
{
    debug("Adapter::PowerChangeState: do nothing\n");

    PowerState = NewState.DeviceState;
}

#pragma code_seg()
STDMETHODIMP_(void) Adapter::PowerChangeState2(__in DEVICE_POWER_STATE _NewDeviceState, __in SYSTEM_POWER_STATE _NewSystemState)
{
    debug("Adapter::PowerChangeState2: do nothing (%d->%d)\n",_NewDeviceState,_NewSystemState);

    PowerState = _NewDeviceState;

    return;
}
