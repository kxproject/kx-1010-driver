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

#include "adapter.h"
#include "wave.h"
#include "wavestream.h"
#include "guids.h"

#pragma code_seg("PAGE")
WaveStream::~WaveStream()
{
	PAGED_CODE();

	ASSERT(valid_object(this,WaveStream));

    debug("WaveStream[%d]::~WaveStream [%p]\n",Pin,this);

    enabled=false;

    if(NotificationEvent)
    {
    	debug("!! WaveStream[%d]::~WaveStream: notification event %p still allocated\n",Pin,NotificationEvent);
    	NotificationEvent=NULL;
    }

    if(wave)
    {
        // just in case
        if(valid_object(wave->GetAdapter()->hal,Hal))
        	wave->GetAdapter()->hal->remove_notification(this);

    	wave=NULL;
    }

    memset(&current_pin_format,0,sizeof(current_pin_format));

    magic=NULL;
}

#pragma code_seg("PAGE")
STDMETHODIMP WaveStream::NonDelegatingQueryInterface(IN REFIID Interface,OUT PVOID *Object)
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PMINIPORTWAVERTSTREAM(this)));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IMiniportWaveRTStream))
    {
        *Object = PVOID(PMINIPORTWAVERTSTREAM(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IMiniportWaveRTStreamNotification))
    {
        *Object = PVOID(PMINIPORTWAVERTSTREAMNOTIFICATION(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

#pragma code_seg("PAGE")
NTSTATUS WaveStream::Init(IN Wave *Miniport_,IN PPORTWAVERTSTREAM PortStream_,IN ULONG Pin_,IN BOOLEAN Capture_,IN PKSDATAFORMAT DataFormat)
{
	PAGED_CODE();

	debug("WaveStream[%d]::Init: [%p, %p]\n",Pin_,this,Miniport_);

	magic=object_magic;
	wave=Miniport_;
	Pin=Pin_;
	State=KSSTATE_STOP;
	PortStream=PortStream_;

	AudioPosition=0;
	NotificationCount=0;
	AudioBuffer=NULL;
	NotificationEvent=NULL;

	is_asio=false;
	is_16bit=false;
	is_recording=!!Capture_;
	enabled=false;

	NTSTATUS ntStatus=SetFormat(DataFormat);

	return ntStatus;
}

#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS) WaveStream::SetFormat(IN  PKSDATAFORMAT   Format)
{
	PAGED_CODE();

	bool need_format_change=false;
    hardware_parameters_t new_params;  memset(&new_params,0,sizeof(new_params));

	if(NT_SUCCESS(wave->VerifyFormat(VFSetFormat,Pin,Format,&new_params,&need_format_change)))
	{
		if(need_format_change)
		{
    		// apply format changes
            if(wave->GetAdapter()->hal->set_hardware_settings(&new_params)) // failed?
      		{
      			new_params.print_settings("!! WaveStream::SetFormat: h/w failed to switch to");
      			return STATUS_INVALID_PARAMETER;
      		}

      		// save registry settings
      		wave->GetAdapter()->save_hardware_settings();

      		// signal format change (if pb pin is opened, rec pin needs to update its format)
      		wave->generate_format_change_event();
      	}

  		memcpy(&current_pin_format,wave->GetWaveFormat(Format),Format->FormatSize<sizeof(current_pin_format)?Format->FormatSize:sizeof(current_pin_format));

  		if(current_pin_format.Format.wBitsPerSample==16)
  			is_16bit=true;
  		else
  			is_16bit=false;

  		sampling_rate=current_pin_format.Format.nSamplesPerSec;
  		n_channels=current_pin_format.Format.nChannels;

  		return STATUS_SUCCESS;
	}

	return STATUS_INVALID_PARAMETER;
}

#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS) WaveStream::AllocateAudioBuffer(
    IN  ULONG               RequestedSize,
    OUT PMDL                *AudioBufferMdl,
    OUT ULONG               *ActualSize_,
    OUT ULONG               *OffsetFromFirstPage,
    OUT MEMORY_CACHING_TYPE *cacheType)
{
	PAGED_CODE();
	
    PHYSICAL_ADDRESS    high;

    high.HighPart = MAXULONG;
    high.LowPart = MAXULONG;

    // verify buffer size is properly aligned with regards to Hal's buffer quant
    ULONG ActualSize=RequestedSize;
    
    int buff_mult=current_pin_format.Format.nChannels*current_pin_format.Format.wBitsPerSample/8;		// use WASAPI format to handle 16-bit software buffers properly
    int max_buff=0,min_buff=0,buff_gran=0;

    // get hal requirements
    wave->GetAdapter()->hal->get_buffer_sizes(&max_buff,&min_buff,&buff_gran,NULL,NULL);
    // returned values are in samples for double ASIO buffer (one channel)
    max_buff=max_buff*buff_mult;
    min_buff=min_buff*buff_mult;
    buff_gran=buff_gran*buff_mult;

    debug("WaveStream[%d]::AllocateAudioBuffer: buffer size restrictions: min:%d max:%d (x8) gran:%d (bytes, full buffer) hal: %d (16/48 samples, full buffer), requested: %d\n",
          Pin,min_buff,max_buff,buff_gran,wave->GetAdapter()->hal->hw_full_buffer_in_samples,RequestedSize);

    if((int)RequestedSize<min_buff)
    	ActualSize=min_buff;
    else if((int)RequestedSize>max_buff)
    		ActualSize=max_buff;
    	 else
    	 	if(buff_gran && RequestedSize%buff_gran)
    	 	{
    	 		ActualSize=RequestedSize-(RequestedSize%buff_gran)+buff_gran;
    	 	}
    	 	else
                ActualSize=RequestedSize;

	if(ActualSize!=RequestedSize)
    {
    	debug("!! WaveStream[%d]::AllocateAudioBuffer: buffer size corrected from %d to %d bytes (hal reports: %d samples for 16/48 full buffer)\n",
    		Pin,RequestedSize,ActualSize,wave->GetAdapter()->hal->hw_full_buffer_in_samples);
    }
    
    AudioBuffer = PortStream->AllocatePagesForMdl (high, ActualSize);

    if (!AudioBuffer)
    {
        debug("!! WaveStream[%d]::AllocateAudioBuffer: allocation failed [size: %d/%d]\n",Pin,ActualSize,RequestedSize);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *AudioBufferMdl = AudioBuffer;
    *ActualSize_ = ActualSize;
    *OffsetFromFirstPage = 0;
    *cacheType = DRIVER_BUFF_CACHE; // this is software buffer, not hardware, still need to disable cache on it in order to avoid clicks
    // MSND:  Miniports for Intel High Definition Audio Codecs must specify a CacheType of MmWriteCombined to ensure cache coherency.
    // This is because the Intel High Definition Audio Controller may be configured for non-snoop operation.

    AudioBufferPointer=PortStream->MapAllocatedPages(AudioBuffer,*cacheType);

    if(!AudioBufferPointer)
    	debug("!! WaveStream[%d]::AllocateAudioBuffer: failed to mmap buffer\n",Pin);

    // initialize variables:
    NotificationCount=0;
    AudioPosition=0;
    BufferSize=ActualSize;

    debug("WaveStream[%d]::AllocateAudioBuffer: allocated: %d bytes\n",Pin,ActualSize);

	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
STDMETHODIMP_(VOID) WaveStream::FreeAudioBuffer(IN PMDL Mdl,IN ULONG Size)
{
	PAGED_CODE();

	if(State==KSSTATE_RUN)
	{
		debug("!! WaveStream[%d]::FreeAudioBuffer: still running\n",Pin);
		wave->GetAdapter()->hal->sync_stop();
	}

	if(AudioBufferPointer)
	{
		PortStream->UnmapAllocatedPages(AudioBufferPointer,AudioBuffer);
		AudioBufferPointer=NULL;
	}

	PortStream->FreePagesFromMdl (Mdl);

	NotificationCount=0;
	AudioBuffer=NULL;

	debug("WaveStream[%d]::FreeAudioBuffer: freed: %p / %d bytes\n",Pin,Mdl,Size);
}

#pragma code_seg("PAGE")
NTSTATUS WaveStream::AllocateBufferWithNotification(ULONG NotificationCount_,ULONG RequestedSize,PMDL *AudioBufferMdl,ULONG *ActualSize,ULONG *OffsetFromFirstPage,MEMORY_CACHING_TYPE *CacheType)
{
	PAGED_CODE();

	NTSTATUS ret = AllocateAudioBuffer(RequestedSize,AudioBufferMdl,ActualSize,OffsetFromFirstPage,CacheType);
	if(!NT_SUCCESS(ret)) return ret;

	NotificationCount=NotificationCount_;
	debug("WaveStream[%d]::AllocateAudioBufferWithNotification: notification count: %d\n",Pin,NotificationCount);

	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
VOID WaveStream::FreeBufferWithNotification(PMDL AudioBufferMdl,ULONG SizeWritten)
{
	PAGED_CODE();

	return FreeAudioBuffer(AudioBufferMdl,SizeWritten);
}

#pragma code_seg("PAGE")
NTSTATUS WaveStream::RegisterNotificationEvent(PKEVENT NotificationEvent_)
{
	PAGED_CODE();

	NotificationEvent=NotificationEvent_;
	debug("WaveStream[%d]::RegisterNotificationEvent: registered\n",Pin);

	return STATUS_SUCCESS;
}


#pragma code_seg("PAGE")
NTSTATUS WaveStream::UnregisterNotificationEvent(PKEVENT NotificationEvent_)
{
	PAGED_CODE();

	if(NotificationEvent==NotificationEvent_)
	{
    	NotificationEvent=NULL;
    	debug("WaveStream[%d]::UnregisterNotificationEvent: unregistered\n",Pin);
    }	
    else
    {
    	debug("!! WaveStream[%d]::UnregisterNotificationEvent: invalid parameter: %p vs %p\n",Pin,NotificationEvent,NotificationEvent_);
    	return STATUS_INVALID_PARAMETER;
    }

	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
STDMETHODIMP_(void) WaveStream::GetHWLatency(OUT PKSRTAUDIO_HWLATENCY hwLatency)
{
	PAGED_CODE();

	int in=0,out=0,min_buff=0;

	wave->GetAdapter()->hal->get_buffer_sizes(NULL,NULL,&min_buff,&in,&out);

    // pre-0.9.9.1: FifoSize = min_buff*(is_16bit?2:4)*n_channels;
    hwLatency->FifoSize = 0;                                            // in bytes
    hwLatency->ChipsetDelay = 0;    									// in 100-nanosecond units
    hwLatency->CodecDelay = (is_recording?in:out)*10000000LL/48000LL;	// in 100-nanosecond units

    debug("WaveStream[%d]::GetHWLatency: calculate latencies: %d,%d,%d -> Fifo: %d bytes, Chipset: %d, Codec:%d\n",
    	Pin,in,out,min_buff,hwLatency->FifoSize,hwLatency->ChipsetDelay,hwLatency->CodecDelay);
}

#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS) WaveStream::GetPositionRegister(OUT PKSRTAUDIO_HWREGISTER   hwRegister)
{
	PAGED_CODE();

	// need to fill-in only Register, Width and Accuracy

	hwRegister->Width=32;

	int hw_buffer_size=wave->GetAdapter()->hal->hw_full_buffer_in_samples; // in samples, 48/16 buffer, full size
	switch(wave->GetAdapter()->hal->get_current_sampling_rate_fast())
	{
		case 44100:
		case 48000: break; // nop
		case 88200:
		case 96000: hw_buffer_size*=2; break;
		case 176400:
		case 192000: hw_buffer_size*=4; break;
		default:
			debug("!!WaveStream[%d]::GetPositionRegister: %s, sampling rate undefined!\n",Pin,is_recording?"recording":"playback");
	}
	hwRegister->Accuracy=(hw_buffer_size/2)*current_pin_format.Format.nChannels*current_pin_format.Format.wBitsPerSample/8;
	  // in bytes, divisible by frame size (e.g. 4 for 2ch/16)
	  // hw_full_buffer_in_samples is full buffer in samples, accuracy is 1/2 of such buffer
	  // note: measured in source format bit depth (e.g. 16 for 16-bit pcm playback)
	  // software buffer is x2 or x4 larger than 16/48 hw buffer
      // MSDN: For example, the audio frame size for a 2-channel, 16-bit PCM stream is 4 bytes. If the position register increments
      // (by two times the frame size) once every second tick of the sample clock, the accuracy value is 8 bytes. If the position register
      // increments (by four times the frame size) once every fourth tick of the sample clock, the accuracy value is 16 bytes, and so on
    
	hwRegister->Register=&AudioPosition;

	debug("WaveStream[%d]::GetPositionRegister: %s, accuracy: %d\n",Pin,is_recording?"recording":"playback",hwRegister->Accuracy);

	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS) WaveStream::GetClockRegister(OUT PKSRTAUDIO_HWREGISTER   /*hwRegister*/)
{
	PAGED_CODE();

	// we do not support clock register

	return STATUS_UNSUCCESSFUL;
}

#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS) WaveStream::GetPosition(OUT PKSAUDIO_POSITION   Position)
{
	PAGED_CODE();

	debug("WaveStream[%d]::GetPosition\n",Pin);

	Position->PlayOffset=AudioPosition;
    
    int hw_buffer_size=wave->GetAdapter()->hal->hw_full_buffer_in_samples; // in samples, 48/16 buffer, full size
	switch(wave->GetAdapter()->hal->get_current_sampling_rate_fast())
	{
            case 44100:
            case 48000: break; // nop
            case 88200:
            case 96000: hw_buffer_size*=2; break;
            case 176400:
            case 192000: hw_buffer_size*=4; break;
		default:
			debug("!!WaveStream[%d]::GetPositionRegister: %s, sampling rate undefined!\n",Pin,is_recording?"recording":"playback");
	}
    
#if defined(_NTDDK_)
    ULONGLONG offset;
#else
    DWORDLONG offset;
#endif
    
    offset=AudioPosition;
	offset+=(hw_buffer_size/2)*current_pin_format.Format.nChannels*current_pin_format.Format.wBitsPerSample/8;
    if(offset>=BufferSize)
        offset=0;

	Position->WriteOffset=offset;
    
	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS) WaveStream::SetState(IN  KSSTATE State_)
{
	PAGED_CODE();

	// debug("WaveStream::SetState: %d\n",State_);

	if(State==KSSTATE_PAUSE && State_==KSSTATE_RUN) // start
	{
		update_buffer_callbacks();
		enabled=true;
		wave->GetAdapter()->hal->add_notification(this);
    	wave->GetAdapter()->hal->sync_start();
	}
	else
	 if(State==KSSTATE_RUN && State_==KSSTATE_PAUSE) // Stop
	 {
	 	enabled=false;
	 	wave->GetAdapter()->hal->remove_notification(this);
		wave->GetAdapter()->hal->sync_stop();
	 }

	State=State_;

	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
NTSTATUS WaveStream::PropertyPrivate(IN PPCPROPERTY_REQUEST   PropertyRequest)
{
	PAGED_CODE();

	ASSERT(PropertyRequest);

    Wave *wave = (Wave *)(PMINIPORTWAVERT)PropertyRequest->MajorTarget;
    WaveStream *wave_stream = (WaveStream *)PMINIPORTWAVERTSTREAM(PropertyRequest->MinorTarget);

    if(valid_object(wave,Wave) || valid_object(wave_stream,WaveStream))
    {
    	return wave->GetAdapter()->PropertyPrivate(PropertyRequest, wave, wave_stream);
    }
    else
    {
    	debug("!! WaveStream::Private: invalid property targets: %p / %p\n",wave,wave_stream);
    	return STATUS_INVALID_PARAMETER;
    }
}
