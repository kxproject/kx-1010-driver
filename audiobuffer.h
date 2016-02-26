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

#include "list.h"

#define restrict __restrict

#pragma code_seg()
class BufferNotification
{
		BYTE *CurrentBufferPointer;
		int asio_channel_offset;

	public:
		struct list list;

     	// stream management
     	ULONG AudioPosition;
     	ULONG NotificationCount;
     	ULONG BufferSize;
     	PMDL  AudioBuffer;
     	PKEVENT NotificationEvent;
     	VOID  *AudioBufferPointer;

		bool is_asio;
		bool is_recording;
		bool is_16bit;
		bool enabled;
		int sampling_rate;
		int n_channels;

		void update_buffer_callbacks(void);

		PKEVENT process_pb_buffers(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples);		// returns NULL if no notification is required or valid kernel event handle
		PKEVENT process_pb_buffers(word * restrict buffers[NUM_VOICES],int hw_voices,int samples);		// returns NULL if no notification is required or valid kernel event handle
		PKEVENT process_rec_buffers(dword * restrict buffer,int hw_voices,int samples);	// --""--

		inline void process_pb_buffers_asio(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_asio_48(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_asio_96(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_asio_192(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_asio(word * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_asio_48(word * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_asio_96(word * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_asio_192(word * restrict buffers[NUM_VOICES],int hw_voices,int samples);

		inline void process_rec_buffers_asio(dword * restrict buffer,int hw_voices,int samples);
		inline void process_rec_buffers_asio_48(dword * restrict buffer,int hw_voices,int samples);
		inline void process_rec_buffers_asio_96(dword * restrict buffer,int hw_voices,int samples);
		inline void process_rec_buffers_asio_192(dword * restrict buffer,int hw_voices,int samples);

		inline void process_pb_buffers_wdm_48(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm_16_48(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm_96(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm_16_96(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm_192(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm_16_192(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm_48(word * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm_16_48(word * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm_96(word * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm_16_96(word * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm_192(word * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm_16_192(word * restrict buffers[NUM_VOICES],int hw_voices,int samples);

		inline void process_rec_buffers_wdm(dword * restrict buffer,int hw_voices,int samples);
		inline void process_rec_buffers_wdm_16(dword * restrict buffer,int hw_voices,int samples);
		inline void process_pb_buffers_wdm(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm_16(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm(word * restrict buffers[NUM_VOICES],int hw_voices,int samples);
		inline void process_pb_buffers_wdm_16(word * restrict buffers[NUM_VOICES],int hw_voices,int samples);
};
