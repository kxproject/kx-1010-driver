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
#include "hal.h"

#pragma code_seg()
class Voices
{
    enum { object_magic=0x01ceAA77 };
	int magic;

	Hal *hal;
	dword voice_usage;

	public:
		Voices();
		~Voices();	// implies close()

		int init(Hal *hal_);
		int close(void);

		dword calc_vtft(bool low) { return (low)?0x80007fff:0x80007fff; 	};	// volume
		dword calc_ptrx(bool low) { return (low)?0xc000:0xe000;				}; 	// send A amount / send B amount

		// these functions accept 'logical' voice numbers which are translated into 16/48 stereo pairs internally
		int stop_multiple(dword mask);
		int start_multiple(dword mask);

		void init_cache(int voice);
		void setup_playback(int voice);

		bool verify_magic(void) { return (magic==object_magic); };

		int hw_cache_size(void);
};
