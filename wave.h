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

// Wave Miniport Pins
#define WAVE_WAVEOUT_SOURCE 	0
#define WAVE_WAVEOUT_DEST		1
#define WAVE_WAVEIN_SOURCE		2
#define WAVE_WAVEIN_DEST		3
#define WAVE_SPDIF_AC3			4
// #define WAVE_SPDIF				5

enum VFClient_e
{
	VFIntersection=1,
	VFSetFormat=2,
	VFProposedFormat=3
};

#pragma code_seg()
class Wave : public IMiniportWaveRT,
			 public CUnknown,
			 public IPowerNotify
{
	Adapter *adapter;

    enum { object_magic=0xfafecc55 };
	int magic;

	DEVICE_POWER_STATE      PowerState;

public:
	SAFE_DESTRUCTORS;
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(Wave);

    PPORTEVENTS	PortEvents;

    PPORTCLSSubdeviceEx update_pin_descriptor_p;

    ~Wave();

    Adapter *GetAdapter(void) { return adapter; };

    IMP_IMiniportWaveRT;
    /*
    STDMETHODIMP Init(IN PUNKNOWN UnknownAdapter,IN PRESOURCELIST ResourceList,IN PPORTWAVERT Port);
    STDMETHODIMP GetDescription(OUT PPCFILTER_DESCRIPTOR *  OutFilterDescriptor);
    STDMETHODIMP DataRangeIntersection
    					(   IN      ULONG           PinId
                        ,   IN      PKSDATARANGE    DataRange
                        ,   IN      PKSDATARANGE    MatchingDataRange
                        ,   IN      ULONG           OutputBufferLength
                        ,   OUT     PVOID           ResultantFormat     OPTIONAL
                        ,   OUT     PULONG          ResultantFormatLength);
    STDMETHODIMP NewStream(
  					OUT  PMINIPORTWAVERTSTREAM *Stream,
                    IN   PPORTWAVERTSTREAM PortStream,
                    IN   ULONG Pin,
                    IN   BOOLEAN Capture,
                    IN   PKSDATAFORMAT DataFormat);
	STDMETHODIMP GetDeviceDescription(OUT  PDEVICE_DESCRIPTION DeviceDescription);
    */

    NTSTATUS VerifyFormat(VFClient_e client,ULONG PinId, IN PKSDATAFORMAT DataFormat,hardware_parameters_t *new_params=NULL,bool *need_format_change=NULL);
    	// is called by NewStream, DataRangeIntersection and WaveStream::SetFormat

    STDMETHODIMP DataRangeIntersectionAC3
    					(   IN      ULONG           PinId
                        ,   IN      PKSDATARANGE    DataRange
                        ,   IN      PKSDATARANGE    MatchingDataRange
                        ,   IN      ULONG           OutputBufferLength
                        ,   OUT     PVOID           ResultantFormat     OPTIONAL
                        ,   OUT     PULONG          ResultantFormatLength);

    static NTSTATUS Create(OUT PUNKNOWN *Unknown,IN REFCLSID,IN PUNKNOWN UnknownOuter OPTIONAL,IN POOL_TYPE PoolType);
    static WAVEFORMATPCMEX *GetWaveFormat(IN PKSDATAFORMAT pDataFormat);

    bool verify_magic(void) { return (magic==object_magic); };

    static NTSTATUS FormatChangeHandler(IN PPCEVENT_REQUEST EventRequest);
    static NTSTATUS ProposeFormatHandler(IN PPCPROPERTY_REQUEST PropertyRequest);

    // dynamic sampling rate and data ranges notifications
    void generate_format_change_event(void);
    void update_dataranges(int new_max_sampling_rate);

    IMP_IPowerNotify;
};
