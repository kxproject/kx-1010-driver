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

#include "util.h"

class PcDevice;
class Wave;
class WaveStream;
class Topology;
class Hal;

#include "parameters.h"

#pragma code_seg()
DECLARE_INTERFACE_(IAdapterCommon,IUnknown)
{
    DEFINE_ABSTRACT_UNKNOWN()           // For IUnknown

    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
	    PRESOURCELIST ResourceList, PcDevice *device_, PIRP Irp
    )   PURE;
};

typedef IAdapterCommon *PADAPTERCOMMON;

#pragma code_seg()
class Adapter : public IAdapterCommon,
				public CUnknown,
				public IAdapterPowerManagement,
				public IAdapterPowerManagement2
{
    enum { object_magic=0xadcecc22 };
	int magic;

	static NTSTATUS InterruptServiceRoutine(IN PINTERRUPTSYNC  InterruptSync,IN PVOID DynamicContext);

	KDPC dpc;
	static VOID dpc_func(IN PKDPC Dpc,IN PVOID DeferredContext,IN PVOID SystemArgument1,IN PVOID SystemArgument2);

	// current power state
	DEVICE_POWER_STATE      PowerState;

public:
	SAFE_DESTRUCTORS;
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(Adapter);

    PINTERRUPTSYNC InterruptSync;

    bool sampling_rate_locked;
    bool volume_locked;
    int max_sampling_rate;
    bool test_sine_wave;

    virtual ~Adapter();

  	Topology *topology;	// note: these two pointers are simple copies, reference count was not updated; these are zeroed by class destructors
  	Wave *wave;

  	Hal *hal;

    PcDevice *device;

    STDMETHOD_(NTSTATUS,Init)(THIS_ PRESOURCELIST ResourceList, PcDevice *device_, PIRP Irp);

    static NTSTATUS Create(OUT PUNKNOWN *Unknown,IN REFCLSID,IN PUNKNOWN UnknownOuter OPTIONAL,IN POOL_TYPE PoolType);

    NTSTATUS PropertyPrivate(IN PPCPROPERTY_REQUEST   PropertyRequest, Wave *wave, WaveStream *wave_stream);

    void setup_device_name_in_registry(void);	// dynamically change device friendly names in the registry
    void save_hardware_settings(void);

    void apply_volume(LONG left,LONG right, bool force);
    int can_change_hardware_settings(hardware_parameters_t *old_params,hardware_parameters_t *new_params);
    	// returns 0 if succeeded
    	// is only called by Wave::VerifyFormat

    bool verify_magic(void) { return (magic==object_magic); };

    IMP_IAdapterPowerManagement2;
};
