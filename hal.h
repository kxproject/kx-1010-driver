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

enum { NUM_VOICES=2*2*2 };	// stereo, [2x for 24/32 bit is implied], 2x for 96kHz, 2x for 192kHz
							// note: it used to be 16/48 voice, now it is 32/48 (hardware physical stereo pair)
							// that is, NUM_VOICES and voice.cpp/voice.h accept 'logical' 32-bit voices
							// which are internally translated into 16/48 stereo pairs

#include "util.h"
#include "io.h"
#include "8010.h"
#include "hwbuffer.h"
#include "dsp.h"
#include "voices.h"
#include "parameters.h"
#include "audiobuffer.h"

#define REGLIST_END (dword)(0xffffffff)

struct sync_data_t;

enum sync_func_e
{
	SYNC_ADD_NOTIFICATION=1,
	SYNC_REMOVE_NOTIFICATION=2
};

enum { MAX_EVENTS=10 };

#pragma code_seg()
class Hal
{
    enum { object_magic=0xa1fcce77 };
	int magic;
	void *owner;

	kTimer timing;

	word port;
	dword pci_id_ven,pci_id_dev,pci_id_subsys;

	lock hw_lock;
	mutex settings_mutex;
	atomic ipr_value;	// interrupt pending register

	dword fxbs;					// pre-calculated size of rec buffer (as h/w constant)
	int is_streaming;			// counter (number of streaming requests), 0 - streaming is stopped
	bool is_buffer_cleared;		// flag is set if playback buffer has been zeroed due to 'no clients' condition

	// pre-calculated buffer pointers and offsets:
	int total_hw_pb_voices;
	int total_hw_rec_voices;
	dword *pb_buffers_0[NUM_VOICES];		// pre-calculated voice buffers
	dword *pb_buffers_1[NUM_VOICES];		// pre-calculated voice buffers
	dword *rec_buffer_0;
	dword *rec_buffer_1;

	int current_sampling_rate;
	int current_n_channels;

	// power management: device state
	struct device_state_t
	{
		int was_streaming;
		bool is_sleeping;
		hardware_parameters_t settings;
	}device_state;

	struct list buffer_list;

	// notification events
	volatile PKEVENT events[MAX_EVENTS];

	public:
		enum { NUMBER_OF_VOICES=32 };		// hardware supports 64 voices, but we will only handle 32 (this saves dword<->qword conversion)

		Hal(word port,void *owner_);
		~Hal();

		HwBuffer hw_buffer;
		Voices voices;
		DSP dsp;

		bool is_10k2;
		bool is_10k8;

		void msleep(unsigned milliseconds) { LARGE_INTEGER delay; delay.QuadPart=-10000*milliseconds; KeDelayExecutionThread(KernelMode,FALSE,&delay); };
		   // wait 10000*100us relative
		void usleep(unsigned microseconds) { KeStallExecutionProcessor(microseconds);  };

		void writefn0(dword reg, dword data);
		dword readfn0(dword reg);
		void writefn0w(dword reg, word data);
		word readfn0w(dword reg);
		void writefn0b(dword reg, byte data);
		byte readfn0b(dword reg);
		void writeptrw(dword reg, dword channel, word data);
		word readptrw(dword reg, dword channel);
		void writeptrb(dword reg, dword channel, byte data);
		byte readptrb(dword reg, dword channel);
		void writeptr(dword reg, dword channel, dword data);
		void writeptr_multiple(dword channel, ...);
		dword readptr(dword reg, dword channel);
		dword readptr_fast(dword reg, dword channel);
		int writefpga(dword reg, dword value);
		dword readfpga(dword reg);
		int upload_fpga_firmware(byte *data,int size,bool force=false);
		int reload_fpga_firmware(bool force=false);
		int fpga_link_src2dst(dword src, dword dst);
		int init_fpga(void);
		
		void wcwait(dword wait);
		int mute(void);
		int unmute(void);

		void init_spdif(int mode);
		int reset_voice(int voice);
		int reset_voices(void);
		void init_pagetable_and_tram(void);

		int init(void);
		int close(void);

			#define HW_SET_FPGA_FIRMWARE	1
			#define HW_SET_SAMPLING_RATE	2
			#define HW_SET_SPDIF_IO			4
			#define HW_SET_CLOCK			8
			#define HW_SET_ALL				(-1)
		int set_hardware_settings(hardware_parameters_t *params,int flags=HW_SET_ALL);

		int get_hardware_settings(hardware_parameters_t *params);
		int will_accept_hardware_settings(hardware_parameters_t *old_params,hardware_parameters_t *new_params);
				// is only called by Adapter::can_change_hardware_settings
				// returns =0= if succeeded or error otherwise

		int set_spdif_adat_options(bool is_spdif_pro,bool spdif_tx_no_copy_bit,bool is_optical_in_adat,bool is_optical_out_adat,bool mute=true);
		int set_sample_rate(int frequency,clocksource_t source,bool mute=true);
		int set_fpga_routings(bool is_optical_in_adat,bool is_optical_out_adat,bool mute=true);

		// mtr/adc recording support:
		int adjust_rec_buffer_size(int rec_size);	// hw size in bytes
		dword calc_pb_mask(void);
		dword calc_rec_mask(void);

		int get_buffer_sizes(int *max_buffer_size,int *min_buffer_size,int *buffer_size_gran,int *hw_in_latency,int *hw_out_latency,int for_sampling_rate=0);
			// returned values are in samples, 'full' buffer for single channel

		int start_recording(void);
		int stop_recording(void);
		int start_playback(void);
		int stop_playback(void);

		int sync_start(bool from_power_event=false);	// start playback and recording synchronously; if called from power event, restore is_streaming
		int sync_stop(bool from_power_event=false);		// stop playback and recording synchronously; if called from power event, save is_streaming and force stop all streams
		void arrange_sync_call(sync_func_e f,void *p1);	// asks Adapter to call corresponding function via KeSynchronizeExecution

		int add_notification(BufferNotification *n);
		int remove_notification(BufferNotification *n);

		int add_notification_protected(BufferNotification *n);
		int remove_notification_protected(BufferNotification *n);

		#if 0
		// debugging only
		void test_sync(void);
		#endif

		// synchronization routines
		int interrupt_critical(void);	// returns -1 if interrupt is not ours, sets this->ipr_value if DPC is required and returns >0; returns 0 if no DPC is required
		void interrupt_deferred(void);

		void irq_enable(dword inte,bool at_dpc=false);
		void irq_disable(dword inte,bool at_dpc=false);

		bool verify_magic(void) { return (magic==object_magic); };

		int get_current_n_channels(void) { return current_n_channels; };
		int get_current_sampling_rate_fast(void) { return current_sampling_rate; };

		int set_dsp_bypass(dsp_bypass_e bypass);
		int get_clock_frequency(clocksource_t clock);

		enum device_info_e
		{
			PCI_ID_DEV=1,
            PCI_ID_VEN=2,
            PCI_ID_SUBSYS=3,
            PCI_PORT=4
		};

		dword get_device_info(device_info_e nfo)
		{
			switch(nfo)
			{
				case PCI_ID_DEV: return pci_id_dev;
				case PCI_ID_VEN: return pci_id_ven;
				case PCI_ID_SUBSYS: return pci_id_subsys;
				case PCI_PORT: return port;
				default: return 0xffffffff;
			}
		};

		// power management
		int set_power_mode(bool sleep);		// true: sleep, false: wake-up

		int hw_full_buffer_in_samples;		// this is full buffer size in (32-bit) samples for 16/48(!) buffer
};
