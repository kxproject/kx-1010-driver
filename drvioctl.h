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

/// @name Sample rates
//@{
enum // ceapi_sr_e
{
    CEAPI_SR_NONE=0,
    CEAPI_SR_32000=1,
    CEAPI_SR_44100=2,
    CEAPI_SR_48000=4,
    CEAPI_SR_64000=8,
    CEAPI_SR_88200=0x10,
    CEAPI_SR_96000=0x20,
    CEAPI_SR_176400=0x40,
    CEAPI_SR_192000=0x80
};

typedef int ceapi_sr_e;
//@}

/// @name Bits per sample
//@{
enum // ceapi_bps_e
{
    CEAPI_BPS_NONE=0,
    CEAPI_BPS_LSB_16=1,
    CEAPI_BPS_LSB_24=2,
    CEAPI_BPS_LSB_24PD=4,
    CEAPI_BPS_LSB_32=8
};

typedef int ceapi_bps_e;
//@}

enum // ceapi_format_e
{
    CEAPI_FORMAT_NONE=0,
    CEAPI_FORMAT_MIDI=1,

    CEAPI_FORMAT_PCM=2,
};

typedef int ceapi_format_e;

enum device_type_e
{
    CEAPI_DEVICE_AUDIO=1,
    CEAPI_DEVICE_MIDI=2,
    CEAPI_DEVICE_VIRTUAL=4
};

/// @name Defines and data types shared between kernel and user-levels. 
//@{
#define MAX_GUID    64  ///< in bytes / symbols; refers to any kernel-level strings
#define MAX_INPUTS  256 
#define MAX_OUTPUTS 256
//@}

/// Device description. This structure describes audio device.
struct device_info
{
        device_type_e device_type; ///< CEAPI_DEVICE_ above

        union
        {
         struct
         {
           int n_ins;
           int n_outs;
           ceapi_sr_e         sr_support;       ///< see CEAPI_SR_ above
           ceapi_bps_e        bps_support;      ///< see CEAPI_BPS_ above
           ceapi_format_e     format_support;   ///< see CEAPI_FORMAT_ above
         }audio;
         struct
         {
           int ext_caps; // bitfield, currently unused
         }midi;
        }caps;

    char device_guid[MAX_GUID];     ///< 'CECE-[bus]-[vendor]-[model]', e.g. 'CECE-BUSx-00001234-0000abcd'
    char friendly_name[MAX_GUID];   ///< device name retrieved from the hardware/driver
    char unique_id[MAX_GUID];       ///< '[unique_id]//[device/bus-dependent information - plug #, plug type, channel # etc.]'
    char vendor_name[MAX_GUID];     ///< vendor name retrieved from the hardware/driver
    char device_serial[MAX_GUID];   ///< device serial number, if present
    char firmware_version[MAX_GUID];///< user-friendly firmware version number, if present
};

typedef device_info ceapi_func_get_info_t;

/** Device format.
    This structure is used for allocate_buffers. Set desired format, it is safe to use 0 for "current" sr, bps, format if necessary.
*/
struct device_format
{
    // desired device format:
    ceapi_sr_e      cur_device_sr;          // CEAPI_SR* constant (single bit set), can be CEAPI_SR_NONE for current
    ceapi_bps_e     cur_device_bps;         // CEAPI_BPS* constant (single bit set), can be CEAPI_BPS_NONE for current
    ceapi_format_e  cur_device_format;      // CEAPI_FORMAT*, can be CEAPI_FORMAT_NONE for current
    int             cur_n_ins;
    int             cur_n_outs;
    
    DWORD           in_channels[MAX_INPUTS/32];
    DWORD           out_channels[MAX_OUTPUTS/32];
};


#if !defined(DRIVER_API_USAGE)
	// Private Property for kernel-level driver I/O
	// {57E55D2E-692D-4e40-81CD-8D6190542172}
	#define STATIC_KSPROPSETID_DRIVER_Private 0x57e55d2e, 0x692d, 0x4e40, 0x81, 0xcd, 0x8d, 0x61, 0x90, 0x54, 0x21, 0x72
	DEFINE_GUIDSTRUCT("57E55D2E-692D-4e40-81CD-8D6190542172", KSPROPSETID_DRIVER_Private);
	#define KSPROPSETID_DRIVER_Private DEFINE_GUIDNAMED(KSPROPSETID_DRIVER_Private)

	#define KSPROPERTY_DRIVER_TOPOLOGY	100
	#define KSPROPERTY_DRIVER_WAVE		101
#endif

// structure
#pragma pack(push,8)
struct drvioctl_t
{
	int command;
		#define DRVIOCTL_GET_HW_PARAMETERS	1		// get all parameters
		#define DRVIOCTL_SET_HW_PARAMETER	2		// set individual parameter
		#define DRVIOCTL_GET_DEVICE_FORMAT	3		// ASIOX API: get device format
		#define DRVIOCTL_GET_DEVICE_INFO	4		// ASIOX API: get device info
		#define DRVIOCTL_SET_ASIO_X			5		// ASIOX API: enable non-interleaved buffers
		#define DRVIOCTL_GET_HW_PARAMETER	6		// get individual parameter
		#define DRVIOCTL_GET_ASIO_BUFFERS	7		// get ASIO buffers for specific sampling rate
	union
	{
		struct
		{
			int id;
			int value;
		}set_hw_parameter;

		struct
		{
			int id;
			int value;
		}get_hw_parameter;

		struct
		{
			// output:
			int sampling_rate;
			// buffer_size is not used
			int clock_source;
			int is_optical_out_adat;
			int is_optical_in_adat;
			int sampling_rate_locked;
			int volume_locked;
			int is_spdif_pro;
			int n_channels;
			int is_not_audio;
			int spdif_tx_no_copy_bit;
			int max_sampling_rate;
			int dsp_bypass;
            // fpga_reload is not used
            int spdif_in_frequency;
            int adat_in_frequency;
            int bnc_in_frequency;
            int spdif2_in_frequency;
            int lock_status;
            int max_buffer_size;
            int min_buffer_size;
            int buffer_size_gran;
            int hw_in_latency;
            int hw_out_latency;
            int test_sine_wave;
		}get_hw_parameters;

		#if !defined(DRIVER_API_USAGE)
			device_format 	d_format;
			device_info 	d_info;
		#endif

        struct
        {
        	union
        	{
        		__int64 padding;
        	};
        }set_asio_x;

        struct
        {
        	int sampling_rate;
        	int min_buffer;			// in samples, for 1 channel, full buffer
        	int max_buffer;
        	int buffer_gran;
        }get_asio_buffers;

	};
};
#pragma pack(pop)

#if !defined(CLOCKSOURCE_T_DEFINED_)
 #define CLOCKSOURCE_T_DEFINED_

enum clocksource_t
{
	UnknownClock=0,
    InternalClock=1,
    SPDIF=2,
    ADAT=3,
    BNC=4,
    Dock=5
};

#endif // CLOCKSOURCE_T_DEFINED_

enum ParameterId
{
    SamplingRate = 1,			// current sampling rate
    BufferSize = 2,				// ASIO buffer size (managed by ASIO DLL, not kernel driver)
    ClockSource = 3,			// see clocksource_t enumeration
    OpticalOutADAT = 4,			// if true, optical output operates in ADAT mode, else - S/PDIF
    OpticalInADAT = 5,			// if true, optical input operates in ADAT (coaxial input is enabled), else optical input operates in S/PDIF (coaxial input is disabled)
    LockSampleRate = 6,			// if true, ASIO/WASAPI cannot change sampling rate
    LockVolume = 7,				// if true, hardware volume is always at 100%
    SpdifMode = 8,				// if true, output AES/EBU S/PDIF signal, otherwise - consumer
    NumberOfChannels = 9,		// 'get' only, currently unsupported
    AC3Mode = 10,				// 'get' only, currently unsupported
    SpdifNoCopy = 11,			// 'get' only, currently unsupported
    MaxSamplingRate = 12,		// if 0, no limitations
    DSPBypass = 13,				// 0: normal operation; 1: ext. loopback; 2: output constant; 4: input constant; 8: int. loopback
    FPGAReload = 14,			// 'set' only (forces FPGA reload)
    SPDIFInFrequency = 15,		// 'get' only, in Hz (valid only if ClockSource is SPDIF)
    ADATInFrequency = 16,		// 'get' only, in Hz (valid only if ClockSource is ADAT)
    BNCInFrequency = 17,		// 'get' only, in Hz (valid only if ClockSource is BNC)
    SPDIF2InFrequency = 18,		// 'get' only, in Hz (valid only if ClockSource is Dock - SPDIF2 source)
    LockStatus = 19,			// FGPA registers 0x38 and 0x39 (undocumented): 6 bits + 6 bits
    MaxBufferSize = 20,			// Max. ASIO buffer size (r/o)
    MinBufferSize = 21,			// Min. ASIO buffer size (r/o)
    BufferSizeGran = 22,		// ASIO buffer size granularity (r/o)
    HwInLatency = 23,			// HW Input latency
    HwOutLatency = 24,			// HW Output latency
    TestSineWave = 25,			// generate sine wave instead of actual audio
    ResetFPGA = 26,				// (w/o) Reset FPGA, reload FPGA microcode
    ResetDSP = 27				// (w/o) Reset DSP, reload DSP microcode
};

// prototypes:

#if defined(DRIVER_API_IMPLEMENTATION)
	#define API_CALLING_CONVENTION	__declspec(dllexport)
#elif defined(DRIVER_API_USAGE)
	#define API_CALLING_CONVENTION	__declspec(dllimport)
#endif

#if defined(DRIVER_API_IMPLEMENTATION) || defined(DRIVER_API_USAGE)
	extern "C" int API_CALLING_CONVENTION SetHardwareParameter(int param_id,int param_value);
	extern "C" int API_CALLING_CONVENTION GetHardwareParameters(int *sampling_rate,
																	 int *buffer_size, 
                                                                     int *clock_source,
                                                                     bool *is_optical_out_adat, 
                                                                     bool *is_optical_in_adat,
                                                                     bool *sampling_rate_locked,
                                                                     bool *volume_locked,
                                                                     bool *is_spdif_pro,
                                                                     int *n_channels,
                                                                     bool *is_not_audio,
                                                                     bool *spdif_tx_no_copy_bit,
                                                                     int *max_sampling_rate,
                                                                     int *dsp_bypass,
                                                                     int *spdif_in_frequency,
                                                                     int *adat_in_frequency,
                                                                     int *bnc_in_frequency,
                                                                     int *spdif2_in_frequency,
                                                                     int *lock_status);
	extern "C" int API_CALLING_CONVENTION GetHardwareParameter(int param_id, int *param_value);
#endif
