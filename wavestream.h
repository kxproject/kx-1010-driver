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

#include "hal.h"

class Wave;

#pragma code_seg()
class WaveStream : //public IMiniportWaveRTStream, // implied by class below
				   public IMiniportWaveRTStreamNotification,
                   public CUnknown,
                   public BufferNotification
{

    enum { object_magic=0xfafecc55 };
	int magic;

	Wave *wave;

	ULONG Pin;
	WAVEFORMATPCMEX current_pin_format;
	KSSTATE State;
	PPORTWAVERTSTREAM PortStream;

public:
	SAFE_DESTRUCTORS;
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(WaveStream);

    ~WaveStream ();

    IMP_IMiniportWaveRTStream;
    IMP_IMiniportWaveRTStreamNotification;

    // Init the Stream object
    NTSTATUS Init(IN Wave *Miniport_,IN PPORTWAVERTSTREAM PortStream,IN ULONG Pin,IN BOOLEAN Capture,IN PKSDATAFORMAT DataFormat);

    static NTSTATUS PropertyPrivate(IN PPCPROPERTY_REQUEST   PropertyRequest);

    bool verify_magic(void) { return (magic==object_magic); };
};
