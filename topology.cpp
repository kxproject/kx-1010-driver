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
#include "topology.h"
#include "guids.h"
#include "property.h"
#include "topo_descriptors.h"
#include "hal.h"

#pragma code_seg("PAGE")
NTSTATUS Topology::Create(OUT PUNKNOWN *Unknown,IN REFCLSID,IN PUNKNOWN UnknownOuter OPTIONAL,IN POOL_TYPE PoolType)
{
    PAGED_CODE();

    ASSERT(Unknown);

    STD_CREATE_BODY_(Topology,Unknown,UnknownOuter,PoolType,PMINIPORTTOPOLOGY);
}   

#pragma code_seg("PAGE")
Topology::~Topology()
{
	PAGED_CODE();

	ASSERT(valid_object(this,Topology));

    debug("Topology::~Topology\n");

    magic=NULL;

    if(adapter)
    {
    	adapter->topology=NULL;

    	adapter->Release();
    	adapter=NULL;
    }
}

#pragma code_seg("PAGE")
STDMETHODIMP Topology::NonDelegatingQueryInterface(IN REFIID Interface,OUT PVOID * Object)
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PMINIPORTTOPOLOGY(this)));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IMiniport))
    {
        *Object = PVOID(PMINIPORT(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IMiniportTopology))
    {
        *Object = PVOID(PMINIPORTTOPOLOGY(this));
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
        //
        // We reference the interface for the caller.
        //
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

#pragma code_seg("PAGE")
STDMETHODIMP Topology::Init(IN PUNKNOWN UnknownAdapter,IN PRESOURCELIST ResourceList,IN PPORTTOPOLOGY Port_)
{
    PAGED_CODE();

    ASSERT(UnknownAdapter);
    ASSERT(ResourceList);
    ASSERT(Port_);

    debug("Topology::Init [%p]\n",this);

    adapter=NULL;
    magic=object_magic;
    PowerState=PowerDeviceD0;

    UnknownAdapter->QueryInterface( IID_IAdapterCommon,(PVOID *)&adapter );

    adapter->topology=this;

    for(int i=0;i<MAX_CHANNELS;i++)
    {
    	WaveOutMute[i]=false;
    	WaveOutVolume[i]=0UL;
    }

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
STDMETHODIMP Topology::GetDescription(OUT PPCFILTER_DESCRIPTOR *  OutFilterDescriptor)
{
    PAGED_CODE();

    ASSERT(OutFilterDescriptor);

    if(OutFilterDescriptor)
    	*OutFilterDescriptor = &MiniportFilterDescriptor;
    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
STDMETHODIMP Topology::DataRangeIntersection
    (   IN      ULONG           /*PinId*/
    ,   IN      PKSDATARANGE    /*DataRange*/
    ,   IN      PKSDATARANGE    /*MatchingDataRange*/
    ,   IN      ULONG           /*OutputBufferLength*/
    ,   OUT     PVOID           /*ResultantFormat*/     OPTIONAL
    ,   OUT     PULONG          /*ResultantFormatLength*/
    )
{
    PAGED_CODE();
    return STATUS_NOT_IMPLEMENTED;
}

#pragma code_seg("PAGE")
NTSTATUS Topology::SetMute(ULONG /*node*/,LONG channel,BOOL value)
{
	PAGED_CODE();

	// debug("Topo:SetMute: node: %d, chn: %d, vol: %d\n",node,channel,value);

	if(channel>=0 && channel<MAX_CHANNELS) WaveOutMute[channel]=value; else return STATUS_INVALID_PARAMETER;

	update_volume();

	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
NTSTATUS Topology::GetMute(ULONG /*node*/,LONG channel,BOOL *value)
{
	PAGED_CODE();

	// debug("Topo:GetMute: node: %d, chn: %d\n",node,channel);

	if(channel>=0 && channel<MAX_CHANNELS && value) *value=WaveOutMute[channel]; else return STATUS_INVALID_PARAMETER;

	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
NTSTATUS Topology::SetVolume(ULONG /*node*/,LONG channel,ULONG volume)
{
	PAGED_CODE();

	// debug("Topo:SetVol: node: %d, chn: %d, vol: %d\n",node,channel,(LONG)volume);

	if(channel>=0 && channel<MAX_CHANNELS) WaveOutVolume[channel]=volume; else return STATUS_INVALID_PARAMETER;

	update_volume();

	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
NTSTATUS Topology::GetVolume(ULONG /*node*/,LONG channel,ULONG *volume)
{
	PAGED_CODE();

	// debug("Topo:GetVolume: node: %d, chn: %d\n",node,channel);

	if(channel>=0 && channel<MAX_CHANNELS && volume) *volume=WaveOutVolume[channel]; else return STATUS_INVALID_PARAMETER;

	return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
void Topology::update_volume(void)
{
	PAGED_CODE();

	adapter->apply_volume(WaveOutMute[0]?0x80000000:WaveOutVolume[0],WaveOutMute[1]?0x80000000:WaveOutVolume[1],false);
}	


#pragma code_seg("PAGE")
NTSTATUS Topology::PropertyPrivate(IN PPCPROPERTY_REQUEST   PropertyRequest)
{
	PAGED_CODE();

	ASSERT(PropertyRequest);

	Topology *topo=(Topology *)PMINIPORTTOPOLOGY(PropertyRequest->MajorTarget);

	if(valid_object(topo,Topology))
	{
		return topo->adapter->PropertyPrivate(PropertyRequest,NULL,NULL);
	}

    debug("!! Topology::PropertyPrivate: invalid target: %p\n",topo);

	return STATUS_INVALID_PARAMETER;
}

#pragma code_seg("PAGE")
int Topology::get_n_channels(void)
{
	PAGED_CODE();

	if(valid_object(adapter,Adapter) && valid_object(adapter->hal,Hal))
		return adapter->hal->get_current_n_channels();
	else
	{
		debug("!! Topology::get_n_channels: objects not yet constructed\n");
		return 0;
	}
}

#pragma code_seg()
STDMETHODIMP_(void) Topology::PowerChangeNotify(IN      POWER_STATE     NewState)
{
    debug("Topology::PowerChangeState - do nothing\n");
    PowerState = NewState.DeviceState;
}
