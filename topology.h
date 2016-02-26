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

class Adapter;

// Topology MiniportPins
enum
{
    TOPO_WAVEOUT_SOURCE = 0,
    TOPO_WAVEIN_SOURCE,
    TOPO_WAVEOUT_DEST,
    TOPO_WAVEIN_DEST
};

#pragma code_seg()
class Topology : public IMiniportTopology,
				 public CUnknown,
				 public IPowerNotify
{
    enum { object_magic=0x7060cc33 };
	int magic;

	Adapter *adapter;

	#define MAX_CHANNELS	8
	BOOL WaveOutMute[MAX_CHANNELS];
	ULONG WaveOutVolume[MAX_CHANNELS];

	// current power state
	DEVICE_POWER_STATE      PowerState;

public:
	SAFE_DESTRUCTORS;
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(Topology);

    int get_n_channels(void);

    virtual ~Topology();

    IMP_IMiniportTopology;
    /*
    STDMETHODIMP Init(IN PUNKNOWN UnknownAdapter,IN PRESOURCELIST ResourceList,IN PPORTTOPOLOGY Port);
    STDMETHODIMP GetDescription(OUT PPCFILTER_DESCRIPTOR *  OutFilterDescriptor);
    STDMETHODIMP DataRangeIntersection
    					(   IN      ULONG           PinId
                        ,   IN      PKSDATARANGE    DataRange
                        ,   IN      PKSDATARANGE    MatchingDataRange
                        ,   IN      ULONG           OutputBufferLength
                        ,   OUT     PVOID           ResultantFormat     OPTIONAL
                        ,   OUT     PULONG          ResultantFormatLength);

    */

    static NTSTATUS Create(OUT PUNKNOWN *Unknown,IN REFCLSID,IN PUNKNOWN UnknownOuter OPTIONAL,IN POOL_TYPE PoolType);

    NTSTATUS SetMute(ULONG node,LONG channel,BOOL value);
    NTSTATUS GetMute(ULONG node,LONG channel,BOOL *value);
    NTSTATUS SetVolume(ULONG node,LONG channel,ULONG volume);
    NTSTATUS GetVolume(ULONG node,LONG channel,ULONG *volume);

    void update_volume(void);

    static NTSTATUS PropertyPrivate(IN PPCPROPERTY_REQUEST   PropertyRequest);

    friend class Property;

    bool verify_magic(void) { return (magic==object_magic); };

    IMP_IPowerNotify;
};
