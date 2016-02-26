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

#if !defined(DRIVER_API_IMPLEMENTATION)

struct hardware_parameters_t
{
    int sampling_rate;
    int n_channels;
    clocksource_t clock_source;

    bool is_spdif_pro;
    bool spdif_tx_no_copy_bit;
    bool is_optical_in_adat;
    bool is_optical_out_adat;

    // these two are not stored in the registry
    bool is_not_audio;
    int lock_status;

    inline void print_settings(const char *prefix)
    {
    	debug("%s: %dHz, %d chn, clock: %s, SPDIF: %s%s, In:%s, Out:%s %s; lock: %x\n",
    		prefix,sampling_rate,n_channels,
    		(clock_source==InternalClock)?"Internal":
    			(clock_source==SPDIF)?"SPDIF":
    			(clock_source==ADAT)?"ADAT":
    			(clock_source==BNC)?"BNC":
    			(clock_source==Dock)?"Dock":
    			"???",
    		is_spdif_pro?"Pro":"Consumer",
            spdif_tx_no_copy_bit?" no_copy":"",
            is_optical_in_adat?"ADAT":"SPDIF",
            is_optical_out_adat?"ADAT":"SPDIF",
            is_not_audio?"[AC3]":"",
            lock_status);
    }
};

#endif

// various driver options

// DRIVER_MEM_CACHE:
//      hardware buffer, do not cache it
//      Microsoft recommends MmWriteCombined for HD Audio buffers
//      due to PCI bus snooping,
//      which might be disabled for HD Audio, thus, causing cache issues
// DRIVER_DEF_FULL_BUFFER_IN_SAMPLES: maximum is (65536/2stereo/4bps/4(192multiple)) = 2048 samples
//      this is hw 16/48 buffer size in samples
// DRIVER_OPTIMIZE_BUFFERS
//      optimizes buffer fill-in/fill-out procedures by using sampling rate specific functions

// 0.9.5:
/*
#define DRIVER_MEM_CACHE						MmNonCached
#define DRIVER_BUFF_CACHE						MmCached
#define DRIVER_START_REC_FIRST					1
#define DRIVER_DEF_FULL_BUFFER_IN_SAMPLES		1024
#define DRIVER_OPTIMIZE_BUFFERS					0
*/

// 0.9.6:
// #define DRIVER_MEM_CACHE						MmWriteCombined

// 0.9.7
/*
#define DRIVER_MEM_CACHE						MmNonCached
#define DRIVER_BUFF_CACHE						MmNonCached
#define DRIVER_START_REC_FIRST					0
#define DRIVER_DEF_FULL_BUFFER_IN_SAMPLES		256
#define DRIVER_OPTIMIZE_BUFFERS					1
*/

// 0.9.8
/*
#define DRIVER_MEM_CACHE						MmWriteCombined
#define DRIVER_BUFF_CACHE						MmNonCached
#define DRIVER_START_REC_FIRST					0
#define DRIVER_DEF_FULL_BUFFER_IN_SAMPLES		128
#define DRIVER_OPTIMIZE_BUFFERS					1
*/

// 0.9.8.1: caching only
/*
#define DRIVER_MEM_CACHE						MmNonCached
#define DRIVER_BUFF_CACHE						MmCached
#define DRIVER_START_REC_FIRST					0
#define DRIVER_DEF_FULL_BUFFER_IN_SAMPLES		128
#define DRIVER_OPTIMIZE_BUFFERS					1
 // PCIe: clicks
*/
/*
// 0.9.8.2: buffer size only
#define DRIVER_MEM_CACHE						MmWriteCombined
#define DRIVER_BUFF_CACHE						MmNonCached
#define DRIVER_START_REC_FIRST					0
#define DRIVER_DEF_FULL_BUFFER_IN_SAMPLES		1024
#define DRIVER_OPTIMIZE_BUFFERS					1
*/

/*
// 0.9.8.3: optimizations only
#define DRIVER_MEM_CACHE						MmWriteCombined
#define DRIVER_BUFF_CACHE						MmNonCached
#define DRIVER_START_REC_FIRST					0
#define DRIVER_DEF_FULL_BUFFER_IN_SAMPLES		128
#define DRIVER_OPTIMIZE_BUFFERS					0
 // PCIe: clicks
*/

/*
// 0.9.8.4: rec/pb sync
#define DRIVER_MEM_CACHE						MmWriteCombined
#define DRIVER_BUFF_CACHE						MmNonCached
#define DRIVER_START_REC_FIRST					1
#define DRIVER_DEF_FULL_BUFFER_IN_SAMPLES		128
#define DRIVER_OPTIMIZE_BUFFERS					1
 // PCIe: clicks
*/

// 0.9.8.5: all
/*
#define DRIVER_MEM_CACHE						MmNonCached
#define DRIVER_BUFF_CACHE						MmCached
#define DRIVER_START_REC_FIRST					1
#define DRIVER_DEF_FULL_BUFFER_IN_SAMPLES		1024
#define DRIVER_OPTIMIZE_BUFFERS					0
*/

// 0.9.9.1

#define DRIVER_MEM_CACHE						MmWriteCombined
#define DRIVER_BUFF_CACHE						MmNonCached
#define DRIVER_START_REC_FIRST					0
#define DRIVER_DEF_FULL_BUFFER_IN_SAMPLES		512
#define DRIVER_OPTIMIZE_BUFFERS					1
