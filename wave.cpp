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
#include "guids.h"

#include "wavestream.h"
#include "wave_descriptors.h"

#include "hal.h"

#pragma code_seg("PAGE")
NTSTATUS Wave::Create(OUT PUNKNOWN *Unknown,IN REFCLSID,IN PUNKNOWN UnknownOuter OPTIONAL,IN POOL_TYPE PoolType)
{
    PAGED_CODE();

    ASSERT(Unknown);

    STD_CREATE_BODY_(Wave,Unknown,UnknownOuter,PoolType,PMINIPORTWAVERT);
}   

#pragma code_seg("PAGE")
Wave::~Wave()
{
	PAGED_CODE();

	ASSERT(valid_object(this,Wave));

    debug("Wave::~Wave [%p]\n",this);

    if (PortEvents)
    {
        PortEvents->Release ();
        PortEvents = NULL;
    }

    magic=NULL;

    update_pin_descriptor_p=NULL;

    if(adapter)
    {
    	adapter->wave=NULL;

    	adapter->Release();
    	adapter=NULL;
    }
}

#pragma code_seg("PAGE")
STDMETHODIMP Wave::NonDelegatingQueryInterface(IN REFIID Interface,OUT PVOID *Object)
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PMINIPORTWAVERT(this)));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IMiniport))
    {
        *Object = PVOID(PMINIPORT(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IMiniportWaveRT))
    {
        *Object = PVOID(PMINIPORTWAVERT(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IPowerNotify))
    {
        *Object = PVOID(PPOWERNOTIFY(this));
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
STDMETHODIMP Wave::Init(IN      PUNKNOWN        UnknownAdapter,
                     	IN      PRESOURCELIST   ResourceList,
                     	IN      PPORTWAVERT   	Port_)
{
    PAGED_CODE();

    ASSERT(UnknownAdapter);
    ASSERT(ResourceList);
    ASSERT(Port_);

    debug("Wave::Init [%p]\n",this);

    adapter=NULL;
    magic=object_magic;
    PortEvents=NULL;
    PowerState=PowerDeviceD0;
    update_pin_descriptor_p=NULL;

    UnknownAdapter->QueryInterface( IID_IAdapterCommon,(PVOID*)&adapter );

    adapter->wave=this;

    NTSTATUS ret=Port_->QueryInterface (IID_IPortEvents, (PVOID *)&PortEvents);
    if(!NT_SUCCESS(ret) || PortEvents==NULL)
    {
    	debug("!! Wave::Init: failed to create PortEvent [%08x]\n",ret);
    	return STATUS_UNSUCCESSFUL;
    }
    else
    	debug("Wave::Init: PortEvents created\n");

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
STDMETHODIMP Wave::GetDescription(OUT     PPCFILTER_DESCRIPTOR *  OutFilterDescriptor)
{
    PAGED_CODE();

    ASSERT(OutFilterDescriptor);

    debug("Wave::GetDescription\n");

    if(OutFilterDescriptor)
    	*OutFilterDescriptor = &MiniportFilterDescriptor;

    return STATUS_SUCCESS;
}


#pragma code_seg("PAGE")
STDMETHODIMP Wave::GetDeviceDescription(OUT  PDEVICE_DESCRIPTION DeviceDescription)
{
	PAGED_CODE();

    RtlZeroMemory (DeviceDescription, sizeof (DEVICE_DESCRIPTION));
    
    DeviceDescription->Version = DEVICE_DESCRIPTION_VERSION2;
    DeviceDescription->Master = TRUE;
    DeviceDescription->ScatterGather = FALSE;
    DeviceDescription->Dma32BitAddresses = TRUE;
    DeviceDescription->Dma64BitAddresses = FALSE;
    DeviceDescription->InterfaceType = InterfaceTypeUndefined; //PCIBus;
    DeviceDescription->MaximumLength = 0x80000;

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
STDMETHODIMP Wave::DataRangeIntersection
(
    IN      ULONG           PinId,
    IN      PKSDATARANGE    ClientDataRange,
    IN      PKSDATARANGE    MyDataRange,
    IN      ULONG           OutputBufferLength,
    OUT     PVOID           ResultantFormat,
    OUT     PULONG          ResultantFormatLength
)
{
	PAGED_CODE();

    if (IsEqualGUIDAligned(ClientDataRange->SubFormat, 
            KSDATAFORMAT_SUBTYPE_AC3_AUDIO))
    {
    	return DataRangeIntersectionAC3(PinId,ClientDataRange,MyDataRange,OutputBufferLength,ResultantFormat,ResultantFormatLength);
    }
    else
    if (!IsEqualGUIDAligned(ClientDataRange->SubFormat,
            KSDATAFORMAT_SUBTYPE_PCM) &&
        !IsEqualGUIDAligned(ClientDataRange->SubFormat,
            KSDATAFORMAT_SUBTYPE_WILDCARD))
    {
        return STATUS_NO_MATCH;
    }

    if (!IsEqualGUIDAligned(ClientDataRange->MajorFormat, 
            KSDATAFORMAT_TYPE_AUDIO) &&
        !IsEqualGUIDAligned(ClientDataRange->MajorFormat, 
            KSDATAFORMAT_TYPE_WILDCARD))
    {
        return STATUS_NO_MATCH;
    }

    // Check the size of output buffer. We are returning WAVEFORMATEXTENSIBLE = WAVEFORMATPCMEX 
    //
    if (!OutputBufferLength) 
    {
        *ResultantFormatLength = sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATPCMEX);
        return STATUS_BUFFER_OVERFLOW;
    } 
    
    if (OutputBufferLength < (sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATPCMEX ))) 
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    // Fill in the structure the datarange structure.
    //
    RtlCopyMemory(ResultantFormat, MyDataRange, sizeof(KSDATAFORMAT));

    // Modify the size of the data format structure to fit the WAVEFORMATEXTENSIBLE
    // structure.
    //
    ((PKSDATAFORMAT)ResultantFormat)->FormatSize = sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATPCMEX);

    // Append the WAVEFORMATPCMEX structure
    //
    PWAVEFORMATPCMEX pWfxExt = (PWAVEFORMATPCMEX)((PKSDATAFORMAT)ResultantFormat + 1);

    // Always allocate =all= channels
    pWfxExt->Format.nChannels = (WORD)((PKSDATARANGE_AUDIO) MyDataRange)->MaximumChannels;

    // Ensure that the returned sample rate falls within the supported range
    // of sample rates from our data range.
    if((((PKSDATARANGE_AUDIO) ClientDataRange)->MaximumSampleFrequency <
        ((PKSDATARANGE_AUDIO) MyDataRange)->MinimumSampleFrequency) ||
       (((PKSDATARANGE_AUDIO) ClientDataRange)->MinimumSampleFrequency >
        ((PKSDATARANGE_AUDIO) MyDataRange)->MaximumSampleFrequency))
    {
        debug("!! Wave::Intersection: sample rate unsupported [%d..%d vs %d..%d]\n",
        		((PKSDATARANGE_AUDIO) ClientDataRange)->MinimumSampleFrequency,
        		((PKSDATARANGE_AUDIO) ClientDataRange)->MaximumSampleFrequency,
        		((PKSDATARANGE_AUDIO) MyDataRange)->MinimumSampleFrequency,
        		((PKSDATARANGE_AUDIO) MyDataRange)->MaximumSampleFrequency);
        return STATUS_NO_MATCH;
    }

    pWfxExt->Format.nSamplesPerSec = 
        min(((PKSDATARANGE_AUDIO) ClientDataRange)->MaximumSampleFrequency,
            ((PKSDATARANGE_AUDIO) MyDataRange)->MaximumSampleFrequency);

    // Ensure that the returned bits per sample is in the supported
    // range of bit depths from our data range.
    if((((PKSDATARANGE_AUDIO) ClientDataRange)->MaximumBitsPerSample <
        ((PKSDATARANGE_AUDIO) MyDataRange)->MinimumBitsPerSample) ||
       (((PKSDATARANGE_AUDIO) ClientDataRange)->MinimumBitsPerSample >
        ((PKSDATARANGE_AUDIO) MyDataRange)->MaximumBitsPerSample))
    {
        debug("!! Wave::Intersection: bits per sample unsupported [%d..%d vs %d..%d]\n",
        	((PKSDATARANGE_AUDIO) ClientDataRange)->MinimumBitsPerSample,
        	((PKSDATARANGE_AUDIO) ClientDataRange)->MaximumBitsPerSample,
        	((PKSDATARANGE_AUDIO) MyDataRange)->MinimumBitsPerSample,
        	((PKSDATARANGE_AUDIO) MyDataRange)->MaximumBitsPerSample);
        return STATUS_NO_MATCH;
    }

    pWfxExt->Format.wBitsPerSample = 
        (WORD)min(((PKSDATARANGE_AUDIO) ClientDataRange)->MaximumBitsPerSample,
                  ((PKSDATARANGE_AUDIO) MyDataRange)->MaximumBitsPerSample);

    // Fill in the rest of the format
    pWfxExt->Format.nBlockAlign = 
        (pWfxExt->Format.wBitsPerSample * pWfxExt->Format.nChannels) / 8;
    pWfxExt->Format.nAvgBytesPerSec = 
        pWfxExt->Format.nSamplesPerSec * pWfxExt->Format.nBlockAlign;
    pWfxExt->Format.cbSize = 22;
    pWfxExt->Samples.wValidBitsPerSample = pWfxExt->Format.wBitsPerSample;

    // one special check: use 24/32 instead of 24/24
    if(pWfxExt->Format.wBitsPerSample==24)
    {
    	pWfxExt->Format.wBitsPerSample=32;
    }

    pWfxExt->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    pWfxExt->Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;

    // Determine the appropriate channel config to use.
    switch(pWfxExt->Format.nChannels)
    {
        case 1:
            pWfxExt->dwChannelMask = KSAUDIO_SPEAKER_MONO;
            break;
        case 2:
            pWfxExt->dwChannelMask = KSAUDIO_SPEAKER_STEREO;
            break;
        case 4:
            pWfxExt->dwChannelMask = KSAUDIO_SPEAKER_QUAD;
            break;
        case 6:
            pWfxExt->dwChannelMask = KSAUDIO_SPEAKER_5POINT1;
            break;
        case 8:
        	pWfxExt->dwChannelMask = KSAUDIO_SPEAKER_7POINT1_SURROUND; // = DSSPEAKER_7POINT1_SURROUND
            break;
        default:
            // Unsupported channel count.
            return STATUS_NO_MATCH;
    }

    // Now overwrite also the sample size in the ksdataformat structure.
    ((PKSDATAFORMAT)ResultantFormat)->SampleSize = pWfxExt->Format.nBlockAlign;

    // need to verify format by calling VerifyFormat(...)
    if(VerifyFormat(VFIntersection,PinId,(PKSDATAFORMAT)ResultantFormat))
    {
	    return STATUS_NO_MATCH;
    }
    
    *ResultantFormatLength = sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATPCMEX);

    // /*
    debug("Wave::Intersection: selected: %d/%d bps, %dHz, %d chn, %x mask\n",
    	pWfxExt->Format.wBitsPerSample,
    	pWfxExt->Samples.wValidBitsPerSample,
    	pWfxExt->Format.nSamplesPerSec,
    	pWfxExt->Format.nChannels,
    	pWfxExt->dwChannelMask);
    // */
    
    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
STDMETHODIMP Wave::NewStream(
  					OUT  PMINIPORTWAVERTSTREAM *Stream,
                    IN   PPORTWAVERTSTREAM PortStream,
                    IN   ULONG Pin,
                    IN   BOOLEAN Capture,
                    IN   PKSDATAFORMAT DataFormat)
{
	PAGED_CODE();

    ASSERT (Stream);
    ASSERT (PortStream);
    ASSERT (DataFormat);

    NTSTATUS ntStatus=STATUS_SUCCESS;

    *Stream=NULL;

    if(Pin!=WAVE_WAVEOUT_SOURCE && Pin!=WAVE_WAVEIN_SOURCE && Pin!=WAVE_SPDIF_AC3)
    {
    	debug("!! Wave::NewStream: invalid pin number %d\n",Pin);
    	return STATUS_INVALID_PARAMETER;
    }

    // create WaveRT stream
	WaveStream *stream= new (NonPagedPool, 'strW') WaveStream(NULL);
    if (stream)
    {
        stream->AddRef();

        // Init the stream
        ntStatus = stream->Init (this,PortStream,Pin,Capture,DataFormat);
        if (!NT_SUCCESS (ntStatus))
        {
            // Release the stream and clean up
            debug("!! Wave::NewStream: Failed to init stream\n");
            stream->Release ();
            *Stream = NULL;
            return ntStatus;
        }
        // Save the pointer
    	*Stream = (PMINIPORTWAVERTSTREAM)stream;

    	return STATUS_SUCCESS;
    }

    return STATUS_INSUFFICIENT_RESOURCES;
}

#pragma code_seg("PAGE")
NTSTATUS Wave::VerifyFormat(VFClient_e client,ULONG PinId, IN PKSDATAFORMAT pDataFormat,hardware_parameters_t *new_params/*=NULL*/,bool *need_format_change/*=NULL*/)
{
	PAGED_CODE();

	NTSTATUS ntStatus=STATUS_INVALID_PARAMETER;

	WAVEFORMATPCMEX *wave_format = GetWaveFormat(pDataFormat);
	bool is_ac3=false;

	hardware_parameters_t new_params_; memset(&new_params_,0,sizeof(new_params_));
	hardware_parameters_t old_params_; memset(&old_params_,0,sizeof(old_params_));
	hardware_parameters_t *old_params=&old_params_;
	if(new_params==NULL) new_params=&new_params_;

	// get old_params
	adapter->hal->get_hardware_settings(old_params);
	memcpy(new_params,old_params,sizeof(hardware_parameters_t));

	if(wave_format)
	{
		// verify format tag
		if(wave_format->Format.wFormatTag!=WAVE_FORMAT_PCM)
		{
			if(wave_format->Format.wFormatTag!=WAVE_FORMAT_EXTENSIBLE)
			{
				if(wave_format->Format.wFormatTag==WAVE_FORMAT_DOLBY_AC3_SPDIF)
					is_ac3=true;
				else
					goto END;
			}
			else // wave_format is WAVE_FORMAT_EXTENSIBLE
			{
				if(wave_format->SubFormat == KSDATAFORMAT_SUBTYPE_IEC61937_DOLBY_DIGITAL)
					is_ac3=true;
				else
					if(wave_format->SubFormat != KSDATAFORMAT_SUBTYPE_PCM)
						goto END;
			}
		} // else: PCM

		new_params->is_not_audio=is_ac3;
		new_params->n_channels=wave_format->Format.nChannels;
		new_params->sampling_rate=wave_format->Format.nSamplesPerSec;
		// new_params->bits_per_sample=wave_format->Format.wBitsPerSample;
		// new_params->valid_bits_per_sample=wave_format->Format.wBitsPerSample; // will override a bit later below if applicable
		
		if(wave_format->Format.nChannels==old_params->n_channels)
		{
    		if(  ( wave_format->Format.nSamplesPerSec==44100 || wave_format->Format.nSamplesPerSec==48000 || wave_format->Format.nSamplesPerSec==88200 ||
    		       wave_format->Format.nSamplesPerSec==96000 || wave_format->Format.nSamplesPerSec==176400 || wave_format->Format.nSamplesPerSec==192000 ) &&
    		     (wave_format->Format.wBitsPerSample==16 || wave_format->Format.wBitsPerSample==32))
    		{
    			// check for 'extensible'
    			if(wave_format->Format.cbSize==22)
    			{
    				// new_params->valid_bits_per_sample=wave_format->Samples.wValidBitsPerSample;

    				if( (wave_format->Samples.wValidBitsPerSample==16 && wave_format->Format.wBitsPerSample==16) ||
    					(wave_format->Samples.wValidBitsPerSample==24 && wave_format->Format.wBitsPerSample==32) ||
    					(wave_format->Samples.wValidBitsPerSample==32 && wave_format->Format.wBitsPerSample==32) )
    				{
    					ntStatus=STATUS_SUCCESS;
    				}
    			}
    			else
    				if(wave_format->Format.cbSize==0)
    					ntStatus=STATUS_SUCCESS;

    			if(is_ac3)
    			{
    				if(wave_format->Format.nSamplesPerSec!=48000 || wave_format->Format.wBitsPerSample!=16 || PinId!=WAVE_SPDIF_AC3)
                    	ntStatus=STATUS_INVALID_PARAMETER;    					
    			}
    		}
    	}
	}

	if(NT_SUCCESS(ntStatus))
	{
		// verify if this is dynamic format change
		if(adapter->can_change_hardware_settings(old_params,new_params)) // failed?
		{
			ntStatus=STATUS_NO_MATCH;
			goto END;
		}

		// disable any recording-initiated format change
		if(PinId==WAVE_WAVEIN_SOURCE && old_params->sampling_rate!=new_params->sampling_rate)
		{
			ntStatus=STATUS_NO_MATCH;
			goto END;
		}

		// test for max_sampling_rate
		if(adapter->max_sampling_rate && new_params->sampling_rate>adapter->max_sampling_rate)
		{
			// do not allow this rate
			ntStatus=STATUS_NO_MATCH;
			goto END;
		}

		if(need_format_change)
		{
			if(old_params->sampling_rate!=new_params->sampling_rate ||
			   old_params->n_channels!=new_params->n_channels ||
			   old_params->is_not_audio!=new_params->is_not_audio)
			{	
				*need_format_change=true;
			}
		}
	}
END:
	if(!NT_SUCCESS(ntStatus))
	{
		if(client!=VFProposedFormat && client!=VFIntersection)
	    	debug("!! Wave::VerifyFormat: [for %s]: VerifyFormat failed check for pin %d: %d/%d bps, %dHz, %d chn, %x mask [current: %dHz %dchn]\n",
		    		(client==VFIntersection)?"Intersection":
		    		 (client==VFSetFormat)?"SetFormat":
		    		  (client==VFProposedFormat)?"Proposed":"??",
			    	PinId,
			    	wave_format->Format.wBitsPerSample,
			    	wave_format->Samples.wValidBitsPerSample,
			    	wave_format->Format.nSamplesPerSec,
			    	wave_format->Format.nChannels,
			    	wave_format->dwChannelMask,
			    	old_params->sampling_rate,
			    	old_params->n_channels);
	}

	return ntStatus;
}

#pragma code_seg("PAGE")
WAVEFORMATPCMEX *Wave::GetWaveFormat(IN PKSDATAFORMAT pDataFormat)
{
    PAGED_CODE();

    PWAVEFORMATPCMEX           pWfx = NULL;
    
    // If this is a known dataformat extract the waveformat info.
    //
    if( pDataFormat &&
        ( IsEqualGUIDAligned(pDataFormat->MajorFormat, 
                KSDATAFORMAT_TYPE_AUDIO)             &&
          ( IsEqualGUIDAligned(pDataFormat->Specifier, 
                KSDATAFORMAT_SPECIFIER_WAVEFORMATEX) ||
            IsEqualGUIDAligned(pDataFormat->Specifier, 
                KSDATAFORMAT_SPECIFIER_DSOUND) ) ) )
    {
        pWfx = PWAVEFORMATPCMEX(pDataFormat + 1);

        if (IsEqualGUIDAligned(pDataFormat->Specifier, 
                KSDATAFORMAT_SPECIFIER_DSOUND))
        {
            PKSDSOUND_BUFFERDESC    pwfxds;

            pwfxds = PKSDSOUND_BUFFERDESC(pDataFormat + 1);
            pWfx = (PWAVEFORMATPCMEX)&pwfxds->WaveFormatEx;
        }
    }

    return pWfx;
}


#pragma code_seg("PAGE")
STDMETHODIMP Wave::DataRangeIntersectionAC3
(
    IN      ULONG           PinId,
    IN      PKSDATARANGE    ClientDataRange,
    IN      PKSDATARANGE    /*MyDataRange*/,
    IN      ULONG           OutputBufferLength,
    OUT     PVOID           ResultantFormat,
    OUT     PULONG          ResultantFormatLength
)
{
	PAGED_CODE();
    WORD    wFormatTag = WAVE_FORMAT_DOLBY_AC3_SPDIF;

    // AC3 is supported on AC3-capable host pins only
    if(PinId!=WAVE_SPDIF_AC3)
    	return STATUS_NO_MATCH;

    if (!IsEqualGUIDAligned(ClientDataRange->MajorFormat, 
            KSDATAFORMAT_TYPE_AUDIO) &&
        !IsEqualGUIDAligned(ClientDataRange->MajorFormat, 
            KSDATAFORMAT_TYPE_WILDCARD))
    {
        return STATUS_NO_MATCH;
    }

    if (IsEqualGUIDAligned(ClientDataRange->Specifier, 
            KSDATAFORMAT_SPECIFIER_WAVEFORMATEX) ||
        IsEqualGUIDAligned(ClientDataRange->Specifier,
            KSDATAFORMAT_SPECIFIER_WILDCARD))
    {
        *ResultantFormatLength = sizeof(KSDATAFORMAT_WAVEFORMATEX);
    }
    else if (IsEqualGUIDAligned(ClientDataRange->Specifier, 
        KSDATAFORMAT_SPECIFIER_DSOUND))
    {
        *ResultantFormatLength = sizeof(KSDATAFORMAT_DSOUND);
    }
    else
    {
        return STATUS_NO_MATCH;
    }

    // Validate return buffer size, if the request is only for the
    // size of the resultant structure, return it now.
    //
    if (!OutputBufferLength) 
    {
        *ResultantFormatLength = sizeof(KSDATAFORMAT_WAVEFORMATEX);
        return STATUS_BUFFER_OVERFLOW;
    } 
    else if (OutputBufferLength < sizeof(KSDATAFORMAT_WAVEFORMATEX)) 
    {
        return STATUS_BUFFER_TOO_SMALL;
    }
    
    PKSDATAFORMAT_WAVEFORMATEX  resultantFormatWFX;
    PWAVEFORMATEX               pWaveFormatEx;

    resultantFormatWFX = (PKSDATAFORMAT_WAVEFORMATEX) ResultantFormat;

    // Return the best (only) available format.
    //
    resultantFormatWFX->DataFormat.FormatSize   = *ResultantFormatLength;
    resultantFormatWFX->DataFormat.Flags        = 0;
    resultantFormatWFX->DataFormat.SampleSize   = 4; // must match nBlockAlign
    resultantFormatWFX->DataFormat.Reserved     = 0;

    resultantFormatWFX->DataFormat.MajorFormat  = KSDATAFORMAT_TYPE_AUDIO;
    INIT_WAVEFORMATEX_GUID(&resultantFormatWFX->DataFormat.SubFormat,WAVE_FORMAT_DOLBY_AC3_SPDIF );

    // Extra space for the DSound specifier
    //
    if (IsEqualGUIDAligned(ClientDataRange->Specifier,KSDATAFORMAT_SPECIFIER_DSOUND))
    {
        PKSDATAFORMAT_DSOUND        resultantFormatDSound;
        resultantFormatDSound = (PKSDATAFORMAT_DSOUND)    ResultantFormat;

        resultantFormatDSound->DataFormat.Specifier = 
            KSDATAFORMAT_SPECIFIER_DSOUND;

        // DSound format capabilities are not expressed 
        // this way in KS, so we express no capabilities. 
        //
        resultantFormatDSound->BufferDesc.Flags = 0 ;
        resultantFormatDSound->BufferDesc.Control = 0 ;

        pWaveFormatEx = &resultantFormatDSound->BufferDesc.WaveFormatEx;
    }
    else  // WAVEFORMATEX or WILDCARD (WAVEFORMATEX)
    {
        resultantFormatWFX->DataFormat.Specifier = 
            KSDATAFORMAT_SPECIFIER_WAVEFORMATEX;

        pWaveFormatEx = (PWAVEFORMATEX)((PKSDATAFORMAT)resultantFormatWFX + 1);
    }

    pWaveFormatEx->wFormatTag      = wFormatTag;
    pWaveFormatEx->nChannels       = 2;
    pWaveFormatEx->nSamplesPerSec  = 48000;
    pWaveFormatEx->wBitsPerSample  = 16;
    pWaveFormatEx->cbSize          = 0;
    pWaveFormatEx->nBlockAlign     = 4;
    pWaveFormatEx->nAvgBytesPerSec = 192000;

    if(VerifyFormat(VFIntersection,PinId,(PKSDATAFORMAT)ResultantFormat))
    {
	    return STATUS_NO_MATCH;
    }

    return STATUS_SUCCESS;
}

#pragma code_seg()
NTSTATUS Wave::FormatChangeHandler(IN PPCEVENT_REQUEST EventRequest)
{
	// note: can be called at DISPATCH_LEVEL

	Wave *that = (Wave *)(PMINIPORTWAVERT)EventRequest->MajorTarget;

	if(valid_object(that,Wave) && that->PortEvents)
	{
    	switch(EventRequest->EventItem->Id)
    	{
    		case KSEVENT_PINCAPS_FORMATCHANGE:
    			{
    				if(EventRequest->Verb==PCEVENT_VERB_SUPPORT || EventRequest->Verb==PCEVENT_VERB_REMOVE)
    					return STATUS_SUCCESS;

    				if(EventRequest->Verb==PCEVENT_VERB_ADD && that->PortEvents)
    				{
    					that->PortEvents->AddEventToEventList(EventRequest->EventEntry);
    					return STATUS_SUCCESS;
    				}

    				debug("!! Wave::FormatChangeHandler: invalid verb [%d]\n",EventRequest->Verb);
    				return STATUS_INVALID_PARAMETER;
    			}
    			break;
    		default:
    			debug("!! Wave::FormatChangeHandler: invalid event id [%d]\n",EventRequest->EventItem->Id);
    	}
    }
    else
    {
    	debug("!! Wave::FormatChangeHandler: invalid property targets: %p\n",EventRequest->MajorTarget);
    	return STATUS_INVALID_PARAMETER;
    }

	return STATUS_INVALID_PARAMETER;
}

#pragma code_seg("PAGE")
void Wave::generate_format_change_event(void)
{
	if(PortEvents)
	{
		debug("Wave::generate_format_change_event\n");

        GUID gEventSet = KSEVENTSETID_PinCapsChange;
        PortEvents->GenerateEventList(&gEventSet,   		// event set
                       KSEVENT_PINCAPS_FORMATCHANGE,   	// event id
                       TRUE,                           	// this is a pin event
                       WAVE_WAVEOUT_SOURCE,        		// pin id for the wave data pin                                  
                       FALSE,                          	// not a node event
                       ULONG(-1));                     	// an invalid node id
#if defined(WAVE_WAVEIN_SOURCE)
        PortEvents->GenerateEventList(&gEventSet,   		// event set
                       KSEVENT_PINCAPS_FORMATCHANGE,   	// event id
                       TRUE,                           	// this is a pin event
                       WAVE_WAVEIN_SOURCE,        			// pin id for the wave data pin                                  
                       FALSE,                          	// not a node event
                       ULONG(-1));                     	// an invalid node id
#endif
#if defined(WAVE_SPDIF_AC3) && defined(WAVE_SPDIF)
        PortEvents->GenerateEventList(&gEventSet,   		// event set
                       KSEVENT_PINCAPS_FORMATCHANGE,   	// event id
                       TRUE,                           	// this is a pin event
                       WAVE_SPDIF_AC3,        				// pin id for the wave data pin                                  
                       FALSE,                          	// not a node event
                       ULONG(-1));                     	// an invalid node id
#endif
    }
    else
    	debug("!! Wave::generate_format_change_event: port events unregistered\n");
}

#pragma code_seg("PAGE")
NTSTATUS Wave::ProposeFormatHandler(IN PPCPROPERTY_REQUEST PropertyRequest)
{
    PAGED_CODE();

    NTSTATUS ntStatus = STATUS_INVALID_DEVICE_REQUEST;

	Wave *that = (Wave *)(PMINIPORTWAVERT)PropertyRequest->MajorTarget;

	// debug("Wave::ProposeFormatHandler\n");

	if(valid_object(that,Wave))
	{
            ULONG cbMinSize = sizeof(KSDATAFORMAT)+sizeof(WAVEFORMATEXTENSIBLE);

            if (PropertyRequest->ValueSize == 0)
            {
                PropertyRequest->ValueSize = cbMinSize;
                ntStatus = STATUS_BUFFER_OVERFLOW;

                // debug("Wave::ProposeFormatHandler: overflow\n");
            }
            else if (PropertyRequest->ValueSize < cbMinSize)
            {
                ntStatus = STATUS_BUFFER_TOO_SMALL;
                // debug("Wave::ProposeFormatHandler: too small [%d vs %d, %d]\n",PropertyRequest->ValueSize,cbMinSize,sizeof(KSDATAFORMAT));
            }
            else
            {
            	// get PinId:
            	if(!PropertyRequest->Instance || PropertyRequest->InstanceSize<sizeof(ULONG))
            	{
            		debug("!! Wave::ProposeFormatHandler: invalid instance [%p, %d]\n",PropertyRequest->Instance,PropertyRequest->InstanceSize);
            		return ntStatus;
            	}

            	ULONG PinId = *(PULONG(PropertyRequest->Instance));
            	KSDATAFORMAT_WAVEFORMATEX *DataFormat = (KSDATAFORMAT_WAVEFORMATEX*)PropertyRequest->Value;
            	
            	if (PropertyRequest->Verb & KSPROPERTY_TYPE_GET)
            	{
            		// debug("Wave::ProposeFormatHandler: will check for pin %d, Get [%x]\n",PinId,PropertyRequest->Verb);

                    hardware_parameters_t cur_params; memset(&cur_params,0,sizeof(cur_params));
                    that->adapter->hal->get_hardware_settings(&cur_params);

                    // fill-in format information
            		DataFormat->DataFormat.FormatSize=sizeof(KSDATAFORMAT) + sizeof(WAVEFORMATPCMEX);
                    DataFormat->DataFormat.Flags=0;
                    DataFormat->DataFormat.Reserved=0;
                    DataFormat->DataFormat.MajorFormat=KSDATAFORMAT_TYPE_AUDIO;
                    DataFormat->DataFormat.SubFormat=KSDATAFORMAT_SUBTYPE_PCM;
                    DataFormat->DataFormat.Specifier=KSDATAFORMAT_SPECIFIER_WAVEFORMATEX;

                    WAVEFORMATEXTENSIBLE *waveEx = (WAVEFORMATEXTENSIBLE *)&DataFormat->WaveFormatEx;

                    waveEx->Format.wFormatTag=WAVE_FORMAT_EXTENSIBLE;
                    waveEx->Format.nChannels=(WORD)cur_params.n_channels;
                    waveEx->Format.nSamplesPerSec=cur_params.sampling_rate;
                    
                    waveEx->Format.wBitsPerSample=32;
                    waveEx->Format.cbSize=22;

                    waveEx->Format.nBlockAlign=(waveEx->Format.wBitsPerSample * waveEx->Format.nChannels) / 8;
                    waveEx->Format.nAvgBytesPerSec=waveEx->Format.nSamplesPerSec * waveEx->Format.nBlockAlign;;
                    DataFormat->DataFormat.SampleSize=waveEx->Format.nBlockAlign;

                    waveEx->Samples.wValidBitsPerSample=24;
                    waveEx->SubFormat=KSDATAFORMAT_SUBTYPE_PCM;

                    // Determine the appropriate channel config to use.
                    switch(waveEx->Format.nChannels)
                    {
                        case 1:
                            waveEx->dwChannelMask = KSAUDIO_SPEAKER_MONO;
                            break;
                        case 2:
                            waveEx->dwChannelMask = KSAUDIO_SPEAKER_STEREO;
                            break;
                        case 4:
                            waveEx->dwChannelMask = KSAUDIO_SPEAKER_SURROUND;
                            break;
                        case 6:
                            waveEx->dwChannelMask = KSAUDIO_SPEAKER_5POINT1_SURROUND;
                            break;
                        case 8:
                        	waveEx->dwChannelMask = KSAUDIO_SPEAKER_7POINT1_SURROUND; // = DSSPEAKER_7POINT1_SURROUND
                            break;
                    }


            		return ntStatus;
            	}
            	else
                if (PropertyRequest->Verb & KSPROPERTY_TYPE_SET)
                {
                    // debug("Wave::ProposeFormatHandler: will check for pin %d, Set [%x]\n",PinId,PropertyRequest->Verb);

            		if(NT_SUCCESS(that->VerifyFormat(VFProposedFormat,PinId,(PKSDATAFORMAT)DataFormat)))
            			ntStatus = STATUS_SUCCESS;
            		else
                    	ntStatus = STATUS_NO_MATCH;
                }
                else
                {
                	// debug("Wave::ProposeFormatHandler: unhandled verb %x for pin %d, Set\n",PinId,PropertyRequest->Verb);
                    ntStatus = STATUS_INVALID_PARAMETER;
                }
            }
    }
    else
    	debug("!! Wave::ProposedFormatHandler: invalid 'that'\n");

    return ntStatus;
}

#pragma code_seg()
STDMETHODIMP_(void) Wave::PowerChangeNotify(IN      POWER_STATE     NewState)
{
     debug("Wave::PowerChangeState\n");

     if( NewState.DeviceState != PowerState )
     {
        // activate new state
        switch( NewState.DeviceState )
        {
            case PowerDeviceD0:
                PowerState = NewState.DeviceState;
                adapter->InterruptSync->Connect();
                adapter->hal->set_power_mode(false); // wake-up
                break;

            case PowerDeviceD1:
            case PowerDeviceD2:
                PowerState = NewState.DeviceState;
                adapter->InterruptSync->Disconnect();
                adapter->hal->set_power_mode(true); // sleep
                break;
                
            case PowerDeviceD3:
                PowerState = NewState.DeviceState;
                adapter->InterruptSync->Disconnect();
                adapter->hal->set_power_mode(true); // sleep
                break;

            default:
                debug("!!! Wave::PowerChangeNotify: Unknown Device Power State\n");
                break;
        }
     }
}

#pragma code_seg("PAGE")
void Wave::update_dataranges(int max_sampling_rate)
{
	PAGED_CODE();

	NTSTATUS status=0;

	debug("Wave::update_dataranges: new limit is: %d\n",max_sampling_rate);

	if(update_pin_descriptor_p)
	{
    	// update pin descriptors with new data ranges
    	int cnt=0;

    	#if defined(WAVE_WAVEOUT_SOURCE)
    	if(max_sampling_rate!=0)
    	{
        	for(cnt=0;cnt<sizeof(PinDataRangesStreamPlayback)/sizeof(PinDataRangesStreamPlayback[0]);cnt++)
        	{
        		if(((KSDATARANGE_AUDIO *)MiniportPins[WAVE_WAVEOUT_SOURCE].KsPinDescriptor.DataRanges[cnt])->MaximumSampleFrequency > (ULONG)max_sampling_rate)
        			break;
        	}
            MiniportPins[WAVE_WAVEOUT_SOURCE].KsPinDescriptor.DataRangesCount = cnt;
        }
        else
        	MiniportPins[WAVE_WAVEOUT_SOURCE].KsPinDescriptor.DataRangesCount = SIZEOF_ARRAY(PinDataRangePointersPlayback);

        // Give Portcls the new pin descriptor
        status = update_pin_descriptor_p->UpdatePinDescriptor(WAVE_WAVEOUT_SOURCE,PCUPDATE_PIN_DESC_FLAG_DATARANGES, &MiniportPins[WAVE_WAVEOUT_SOURCE]);
        if(!NT_SUCCESS(status))
        	debug("!! Wave::update_dataranges: UpdatePinDescriptor failed: %08x\n",status);
        #endif

        #if defined(WAVE_WAVEIN_SOURCE)
    	if(max_sampling_rate!=0)
    	{
        	for(cnt=0;cnt<sizeof(PinDataRangesStreamRecording)/sizeof(PinDataRangesStreamRecording[0]);cnt++)
        	{
        		if(((KSDATARANGE_AUDIO *)MiniportPins[WAVE_WAVEIN_SOURCE].KsPinDescriptor.DataRanges[cnt])->MaximumSampleFrequency > (ULONG)max_sampling_rate)
        			break;
        	}
            MiniportPins[WAVE_WAVEIN_SOURCE].KsPinDescriptor.DataRangesCount = cnt;
        }
        else
        	MiniportPins[WAVE_WAVEIN_SOURCE].KsPinDescriptor.DataRangesCount = SIZEOF_ARRAY(PinDataRangePointersRecording);

       	// Give Portcls the new pin descriptor
        status = update_pin_descriptor_p->UpdatePinDescriptor(WAVE_WAVEIN_SOURCE,PCUPDATE_PIN_DESC_FLAG_DATARANGES, &MiniportPins[WAVE_WAVEIN_SOURCE]);
        if(!NT_SUCCESS(status))
        	debug("!! Wave::update_dataranges: UpdatePinDescriptor failed: %08x\n",status);
        #endif
    }
    else
    	debug("!! Wave::update_dataranges: no interface IID_IPortClsSubdeviceEx found\n");
}
