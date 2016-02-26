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
#include "util.h"
#include "debug.h"
#include "hal.h"
#include "audiobuffer.h"

#define OPTIMIZE_BUFFERS	DRIVER_OPTIMIZE_BUFFERS

// performance: (E6600 CPU, core2duo, 2.4GHz, 'word' (16-bit) processing - LEGACY)
// (CHK build)
//  (Optimized buffers): PB     PB+REC
//  	44.1/16/wdm: 	5mS
//  	48/16/wdm: 		5mS
//  	88.2/16/wdm: 	10mS
//  	96/16/wdm: 		10mS
//  	176/16/wdm: 	19mS
//  	192/16/wdm: 	19mS
//  (Unoptimized buffers):
//  	44.1/16/wdm: 	5mS		12mS
//  	48/16/wdm: 		5mS
//  	88.2/16/wdm: 	10mS    22mS
//  	96/16/wdm: 		10mS
//  	176/16/wdm: 	19mS
//  	192/16/wdm: 	18mS	43mS
// (FRE build)
//  (Optimized buffers):
//  	44.1/16/wdm: 	5mS		11mS
//  	48/16/wdm: 		5mS
//  	88.2/16/wdm: 	10mS	22mS
//  	96/16/wdm: 		10mS	22mS
//  	176/16/wdm: 	18mS
//  	192/16/wdm: 	19mS	42mS
//  (Unoptimized buffers):
//  	44.1/16/wdm: 	5mS		10
//  	48/16/wdm: 		5mS
//  	88.2/16/wdm: 	10mS    22
//  	96/16/wdm: 		10mS
//  	176/16/wdm: 	18mS    42
//  	192/16/wdm: 	19mS
// == Conclusion: no difference ==


// performance: i7 CPU, 3.2GHz: WORD vs DWORD:
// OPTIMIZE_BUFFERS:1
//    44100  / 32768 bytes / 220 mS : 136 mS
//   192000  / 32768 bytes / 910 mS : 540 mS
//
// OPTIMIZE_BUFFERS:0
//   ~3-5mS>> .. down to 2-3mS>>


#pragma code_seg()
void BufferNotification::update_buffer_callbacks(void)
{
	CurrentBufferPointer=(BYTE *)AudioBufferPointer;
	asio_channel_offset=BufferSize/n_channels;
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
PKEVENT BufferNotification::process_pb_buffers(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples)
{
	PKEVENT ret=NULL;
	if(!AudioBufferPointer)
	 return ret;

	ULONG PrevAudioPosition=AudioPosition;
    ULONG NewAudioPosition=AudioPosition;

	if(is_asio)
	{
#if OPTIMIZE_BUFFERS
		switch(sampling_rate)
		{
			case 44100:
			case 48000:
					process_pb_buffers_asio_48(buffers,hw_voices,samples);
					break;
			case 88200:
			case 96000:
					process_pb_buffers_asio_96(buffers,hw_voices,samples);
					break;
			case 176400:
			case 192000:
					process_pb_buffers_asio_192(buffers,hw_voices,samples);
					break;
		}
#else
		process_pb_buffers_asio(buffers,hw_voices,samples);
#endif
		NewAudioPosition+=samples*sizeof(dword)*hw_voices;
	}
	else
	{
		if(is_16bit)
		{
#if OPTIMIZE_BUFFERS
    		switch(sampling_rate)
    		{
    			case 44100:
    			case 48000:
    					process_pb_buffers_wdm_16_48(buffers,hw_voices,samples);
    					break;
    			case 88200:
    			case 96000:
    					process_pb_buffers_wdm_16_96(buffers,hw_voices,samples);
    					break;
    			case 176400:
    			case 192000:
    					process_pb_buffers_wdm_16_192(buffers,hw_voices,samples);
    					break;
    		}
#else
			process_pb_buffers_wdm_16(buffers,hw_voices,samples);
#endif
			NewAudioPosition+=samples*sizeof(word)*hw_voices;
		}
		else
		{
#if OPTIMIZE_BUFFERS
    		switch(sampling_rate)
    		{
    			case 44100:
    			case 48000:
    					process_pb_buffers_wdm_48(buffers,hw_voices,samples);
    					break;
    			case 88200:
    			case 96000:
    					process_pb_buffers_wdm_96(buffers,hw_voices,samples);
    					break;
    			case 176400:
    			case 192000:
    					process_pb_buffers_wdm_192(buffers,hw_voices,samples);
    					break;
    		}
#else
			process_pb_buffers_wdm(buffers,hw_voices,samples);
#endif
			
			NewAudioPosition+=samples*sizeof(dword)*hw_voices;
		}
	}

	if(NewAudioPosition>=BufferSize)
	{
        // by using temporarily NewAudioPosition we avoid a case when AudioPosition becomes BufferSize
        
		AudioPosition=0;
		if(NotificationCount && NotificationEvent) // full buffer
			ret=NotificationEvent; // arrange KeSetEvent(NotificationEvent,0,FALSE); in DPC
	}
	else
    {
        AudioPosition = NewAudioPosition;
        if((PrevAudioPosition<(BufferSize>>1)) && (AudioPosition>=(BufferSize>>1)))
        {
            if(NotificationCount>1 && NotificationEvent) // half buffer
                ret=NotificationEvent; // arrange KeSetEvent(NotificationEvent,0,FALSE); in DPC
        }
    }

	CurrentBufferPointer=(BYTE *)AudioBufferPointer+AudioPosition;

	KeMemoryBarrier();	// flush DMA caches

	return ret;
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
PKEVENT BufferNotification::process_pb_buffers(word * restrict buffers[NUM_VOICES],int hw_voices,int samples)
{
	PKEVENT ret=NULL;
	if(!AudioBufferPointer)
	 return ret;

	ULONG PrevAudioPosition=AudioPosition;
    ULONG NewAudioPosition=AudioPosition;

	if(is_asio)
	{
#if OPTIMIZE_BUFFERS
		switch(sampling_rate)
		{
			case 44100:
			case 48000:
					process_pb_buffers_asio_48(buffers,hw_voices,samples);
					break;
			case 88200:
			case 96000:
					process_pb_buffers_asio_96(buffers,hw_voices,samples);
					break;
			case 176400:
			case 192000:
					process_pb_buffers_asio_192(buffers,hw_voices,samples);
					break;
		}
#else
		process_pb_buffers_asio(buffers,hw_voices,samples);
#endif
		NewAudioPosition+=samples*sizeof(word)*hw_voices; // hw_voices are 16-bit, that's why 'word'
	}
	else
	{
		if(is_16bit)
		{
#if OPTIMIZE_BUFFERS
    		switch(sampling_rate)
    		{
    			case 44100:
    			case 48000:
    					process_pb_buffers_wdm_16_48(buffers,hw_voices,samples);
    					break;
    			case 88200:
    			case 96000:
    					process_pb_buffers_wdm_16_96(buffers,hw_voices,samples);
    					break;
    			case 176400:
    			case 192000:
    					process_pb_buffers_wdm_16_192(buffers,hw_voices,samples);
    					break;
    		}
#else
			process_pb_buffers_wdm_16(buffers,hw_voices,samples);
#endif
			NewAudioPosition+=samples*sizeof(word)*(hw_voices/2);	// hw_voices are 16-bit
		}
		else
		{
#if OPTIMIZE_BUFFERS
    		switch(sampling_rate)
    		{
    			case 44100:
    			case 48000:
    					process_pb_buffers_wdm_48(buffers,hw_voices,samples);
    					break;
    			case 88200:
    			case 96000:
    					process_pb_buffers_wdm_96(buffers,hw_voices,samples);
    					break;
    			case 176400:
    			case 192000:
    					process_pb_buffers_wdm_192(buffers,hw_voices,samples);
    					break;
    		}
#else
			process_pb_buffers_wdm(buffers,hw_voices,samples);
#endif
			
			NewAudioPosition+=samples*sizeof(word)*hw_voices;	// hw_voices are 16-bit
		}
	}	

	if(NewAudioPosition>=BufferSize)
	{
        // by using temporarily NewAudioPosition we avoid a case when AudioPosition becomes BufferSize
        
		AudioPosition=0;
		if(NotificationCount && NotificationEvent) // full buffer or half buffer
			ret=NotificationEvent; // arrange KeSetEvent(NotificationEvent,0,FALSE); in DPC
	}
	else
    {
        AudioPosition = NewAudioPosition;
        
        if((PrevAudioPosition<(BufferSize>>1)) && (AudioPosition>=(BufferSize>>1)))
        {
            if(NotificationCount>1 && NotificationEvent) // full buffer or half buffer
                ret=NotificationEvent; // arrange KeSetEvent(NotificationEvent,0,FALSE); in DPC
        }
    }

	CurrentBufferPointer=(BYTE *)AudioBufferPointer+AudioPosition;

	return ret;
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
PKEVENT BufferNotification::process_rec_buffers(dword * restrict buffer,int hw_voices,int samples)
{
	PKEVENT ret=NULL;

	if(!AudioBufferPointer)
	 return ret;

	ULONG PrevAudioPosition=AudioPosition;
    ULONG NewAudioPosition=AudioPosition;

	if(is_asio)
	{
#if OPTIMIZE_BUFFERS
		switch(sampling_rate)
		{
			case 44100:
			case 48000:
					process_rec_buffers_asio_48(buffer,hw_voices,samples);
					break;
			case 88200:
			case 96000:
					process_rec_buffers_asio_96(buffer,hw_voices,samples);
					break;
			case 176400:
			case 192000:
					process_rec_buffers_asio_192(buffer,hw_voices,samples);
					break;
		}
#else
		process_rec_buffers_asio(buffer,hw_voices,samples);
#endif
		NewAudioPosition+=samples*sizeof(dword)*hw_voices; // hw_voices are 32-bit, that's why 'dword'
	}
	else
	{
		if(is_16bit)
		{
			process_rec_buffers_wdm_16(buffer,hw_voices,samples);
			NewAudioPosition+=samples*sizeof(word)*hw_voices; // hw_voices are 32-bit, that's why 'word' (16-bit)
		}
		else
		{
			process_rec_buffers_wdm(buffer,hw_voices,samples);
			NewAudioPosition+=samples*sizeof(dword)*hw_voices;	// hw_voices are 32-bit
		}
	}	

	if(NewAudioPosition>=BufferSize)
	{
        // by using temporarily NewAudioPosition we avoid a case when AudioPosition becomes BufferSize
        
		AudioPosition=0;
		if(NotificationCount && NotificationEvent) // full buffer or half buffer
			ret=NotificationEvent; // arrange KeSetEvent(NotificationEvent,0,FALSE); in DPC
	}
	else
    {
        AudioPosition = NewAudioPosition;
        
        if((PrevAudioPosition<=(BufferSize>>1)) && (AudioPosition>=(BufferSize>>1)))
        {
            if(NotificationCount>1 && NotificationEvent) // full buffer or half buffer
                ret=NotificationEvent; // arrange KeSetEvent(NotificationEvent,0,FALSE); in DPC
        }
    }

	CurrentBufferPointer=(BYTE *)AudioBufferPointer+AudioPosition;

	return ret;
}

// ASIO buffer layout
//
// linear buffer mapped into separate channels and double-buffer:
//   chn0 buf0, chn0 buf1
//   chn1 buf0, chn1 buf1
// 	 ...
//  AudioPosition is used in WDM mode (linear position in the whole buffer in bytes)

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_asio_48(dword * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	dword * restrict src1 = (dword *)((BYTE *)AudioBufferPointer+AudioPosition/n_channels);
	dword * restrict src2 = src1+(asio_channel_offset>>2); // because of 'dword'

  	for(int i=0;i<samples;i++)
  	{
  		buffers[0][i]=*src1++;
  		buffers[1][i]=*src2++;
  	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_asio_48(word * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	word * restrict src1 = (word *)((BYTE *)AudioBufferPointer+AudioPosition/n_channels);
	word * restrict src2 = src1+(asio_channel_offset>>1); // because of 'word'

  	for(int i=0;i<samples;i++)
  	{
  		buffers[0][i]=*src1++;
  		buffers[1][i]=*src1++;

  		buffers[2][i]=*src2++;
  		buffers[3][i]=*src2++;
  	}
}


#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_asio_96(dword * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	dword * restrict src1 = (dword *)((BYTE *)AudioBufferPointer+AudioPosition/n_channels);
	dword * restrict src2 = src1+(asio_channel_offset>>2); // because of 'dword'
	
  	for(int i=0;i<samples;i++)
  	{
  		buffers[0][i]=*src1++;
        buffers[2][i]=*src1++;
        
  		buffers[1][i]=*src2++;
  		buffers[3][i]=*src2++;
  	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_asio_96(word * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	word * restrict src1 = (word *)((BYTE *)AudioBufferPointer+AudioPosition/n_channels);
	word * restrict src2 = src1+(asio_channel_offset>>1); // because of 'word'
	
  	for(int i=0;i<samples;i++)
  	{
  		buffers[0][i]=*src1++;
  		buffers[1][i]=*src1++;
        buffers[4][i]=*src1++;
  		buffers[5][i]=*src1++;
        
  		buffers[2][i]=*src2++;
  		buffers[3][i]=*src2++;
  		buffers[6][i]=*src2++;
  		buffers[7][i]=*src2++;
  	}
}


#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_asio_192(dword * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	dword * restrict src1 = (dword *)((BYTE *)AudioBufferPointer+AudioPosition/n_channels);
	dword * restrict src2 = src1+(asio_channel_offset>>2); // because of 'word'

  	for(int i=0;i<samples;i++)
  	{
  		buffers[0][i]=*src1++;
        buffers[2][i]=*src1++;
        buffers[4][i]=*src1++;
        buffers[6][i]=*src1++;
        
  		buffers[1][i]=*src2++;
  		buffers[3][i]=*src2++;
  		buffers[5][i]=*src2++;
  		buffers[7][i]=*src2++;
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_asio_192(word * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	word * restrict src1 = (word *)((BYTE *)AudioBufferPointer+AudioPosition/n_channels);
	word * restrict src2 = src1+(asio_channel_offset>>1); // because of 'word'

  	for(int i=0;i<samples;i++)
  	{
  		buffers[0][i]=*src1++;
  		buffers[1][i]=*src1++;
  		buffers[2][i]=*src2++;
  		buffers[3][i]=*src2++;
  		buffers[4][i]=*src1++;
  		buffers[5][i]=*src1++;
  		buffers[6][i]=*src2++;
  		buffers[7][i]=*src2++;
  		buffers[8][i]=*src1++;
  		buffers[9][i]=*src1++;
  		buffers[10][i]=*src2++;
  		buffers[11][i]=*src2++;
  		buffers[12][i]=*src1++;
  		buffers[13][i]=*src1++;
  		buffers[14][i]=*src2++;
  		buffers[15][i]=*src2++;
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_rec_buffers_asio_48(dword * restrict src,int /*hw_voices*/,int samples)
{
	dword * restrict dst1 = (dword *)((BYTE *)AudioBufferPointer+AudioPosition/n_channels);
	dword * restrict dst2 = dst1+(asio_channel_offset>>2); // because of 'dword'

  	for(int i=0;i<samples;i++)
  	{
  		*dst1=*src; dst1++; src++;

  		*dst2=*src; dst2++; src++;
  	}
}


#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_rec_buffers_asio_96(dword * restrict src,int /*hw_voices*/,int samples)
{
	dword * restrict dst1 = (dword *)((BYTE *)AudioBufferPointer+AudioPosition/n_channels);
	dword * restrict dst2 = dst1+(asio_channel_offset>>2); // because of 'dword'

  	for(int i=0;i<samples;i++)
  	{
  		*dst1=*src; dst1++; src++;
  		*dst2=*src; dst2++; src++;
  		*dst1=*src; dst1++; src++;
  		*dst2=*src; dst2++; src++;
  	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_rec_buffers_asio_192(dword * restrict src,int /*hw_voices*/,int samples)
{
	dword * restrict dst1 = (dword *)((BYTE *)AudioBufferPointer+AudioPosition/n_channels);
	dword * restrict dst2 = dst1+(asio_channel_offset>>2); // because of 'dword'

  	for(int i=0;i<samples;i++)
  	{
  		*dst1=*src; dst1++; src++;
  		*dst2=*src; dst2++; src++;
  		*dst1=*src; dst1++; src++;
  		*dst2=*src; dst2++; src++;
  		*dst1=*src; dst1++; src++;
  		*dst2=*src; dst2++; src++;
  		*dst1=*src; dst1++; src++;
  		*dst2=*src; dst2++; src++;
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_wdm_48(dword * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	dword * restrict src = (dword *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		buffers[0][i]=*src++;
		buffers[1][i]=*src++;
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_wdm_48(word * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	word * restrict src = (word *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		buffers[0][i]=*src++;
		buffers[1][i]=*src++;
		buffers[2][i]=*src++;
		buffers[3][i]=*src++;
	}
}


#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

void BufferNotification::process_pb_buffers_wdm_96(dword * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	dword * restrict src = (dword *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		buffers[0][i]=*src++;
		buffers[1][i]=*src++;
		buffers[2][i]=*src++;
		buffers[3][i]=*src++;
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

void BufferNotification::process_pb_buffers_wdm_96(word * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	word * restrict src = (word *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		buffers[0][i]=*src++;
		buffers[1][i]=*src++;
		buffers[2][i]=*src++;
		buffers[3][i]=*src++;
		buffers[4][i]=*src++;
		buffers[5][i]=*src++;
		buffers[6][i]=*src++;
		buffers[7][i]=*src++;
	}
}


#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

void BufferNotification::process_pb_buffers_wdm_192(dword * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	dword * restrict src = (dword *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		buffers[0][i]=*src++;
		buffers[1][i]=*src++;
		buffers[2][i]=*src++;
		buffers[3][i]=*src++;
		buffers[4][i]=*src++;
		buffers[5][i]=*src++;
		buffers[6][i]=*src++;
		buffers[7][i]=*src++;
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

void BufferNotification::process_pb_buffers_wdm_192(word * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	word * restrict src = (word *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		buffers[0][i]=*src++;
		buffers[1][i]=*src++;
		buffers[2][i]=*src++;
		buffers[3][i]=*src++;

		buffers[4][i]=*src++;
		buffers[5][i]=*src++;
		buffers[6][i]=*src++;
		buffers[7][i]=*src++;

		buffers[8][i]=*src++;
		buffers[9][i]=*src++;
		buffers[10][i]=*src++;
		buffers[11][i]=*src++;

		buffers[12][i]=*src++;
		buffers[13][i]=*src++;
		buffers[14][i]=*src++;
		buffers[15][i]=*src++;
	}
}


#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_wdm_16_48(dword * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	word * restrict src = (word *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		buffers[0][i]=(*src)<<16; src++;
		buffers[1][i]=(*src)<<16; src++;
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_wdm_16_48(word * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	word * restrict src = (word *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		buffers[0][i]=0;
		buffers[1][i]=*src++;
		buffers[2][i]=0;
		buffers[3][i]=*src++;
	}
}


#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

void BufferNotification::process_pb_buffers_wdm_16_96(dword * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	word * restrict src = (word *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		buffers[0][i]=(*src)<<16; src++;
		buffers[1][i]=(*src)<<16; src++;
		buffers[2][i]=(*src)<<16; src++;
		buffers[3][i]=(*src)<<16; src++;
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

void BufferNotification::process_pb_buffers_wdm_16_96(word * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	word * restrict src = (word *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		buffers[0][i]=0;
		buffers[1][i]=*src++;
		buffers[2][i]=0;
		buffers[3][i]=*src++;
		buffers[4][i]=0;
		buffers[5][i]=*src++;
		buffers[6][i]=0;
		buffers[7][i]=*src++;
	}
}


#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

void BufferNotification::process_pb_buffers_wdm_16_192(dword * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	word * restrict src = (word *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		buffers[0][i]=(*src)<<16; src++;
		buffers[1][i]=(*src)<<16; src++;
		buffers[2][i]=(*src)<<16; src++;
		buffers[3][i]=(*src)<<16; src++;
		buffers[4][i]=(*src)<<16; src++;
		buffers[5][i]=(*src)<<16; src++;
		buffers[6][i]=(*src)<<16; src++;
		buffers[7][i]=(*src)<<16; src++;
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

void BufferNotification::process_pb_buffers_wdm_16_192(word * restrict buffers[NUM_VOICES],int /*hw_voices*/,int samples)
{
	word * restrict src = (word *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		buffers[0][i]=0;
		buffers[1][i]=*src++;
		buffers[2][i]=0;
		buffers[3][i]=*src++;
		buffers[4][i]=0;
		buffers[5][i]=*src++;
		buffers[6][i]=0;
		buffers[7][i]=*src++;
		buffers[8][i]=0;
		buffers[9][i]=*src++;
		buffers[10][i]=0;
		buffers[11][i]=*src++;
		buffers[12][i]=0;
		buffers[13][i]=*src++;
		buffers[14][i]=0;
		buffers[15][i]=*src++;
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_asio(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples)
{
	dword * restrict src1 = (dword *)((BYTE *)AudioBufferPointer+AudioPosition/n_channels);
	dword * restrict src2 = src1+(asio_channel_offset>>2); // because of 'dword'

  	for(int i=0;i<samples;i++)
  	{
  		for(int j=0;j<hw_voices;)
  		{
	  		buffers[j][i]=*src1++; j++;
	  		buffers[j][i]=*src2++; j++;
	  	}
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_asio(word * restrict buffers[NUM_VOICES],int hw_voices,int samples)
{
	word * restrict src1 = (word *)((BYTE *)AudioBufferPointer+AudioPosition/n_channels);
	word * restrict src2 = src1+(asio_channel_offset>>1); // because of 'word'

  	for(int i=0;i<samples;i++)
  	{
  		for(int j=0;j<hw_voices;)
  		{
	  		buffers[j][i]=*src1++; j++;
	  		buffers[j][i]=*src1++; j++;
	  		buffers[j][i]=*src2++; j++;
	  		buffers[j][i]=*src2++; j++;
	  	}
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_rec_buffers_asio(dword * restrict src,int hw_voices,int samples)
{
	dword * restrict dst1 = (dword *)((BYTE *)AudioBufferPointer+AudioPosition/n_channels);
	dword * restrict dst2 = dst1+(asio_channel_offset>>2); // because of 'dword'

  	for(int i=0;i<samples;i++)
  	{
  		for(int j=0;j<(hw_voices>>1);j++)
  		{
  			*dst1=*src; dst1++; src++;
  			*dst2=*src; dst2++; src++;
  		}
  	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_rec_buffers_wdm(dword * restrict src,int hw_voices,int samples)
{
	dword *dst = (dword *)CurrentBufferPointer;

	// memcpy(dst,src,samples*hw_voices*sizeof(dword));
	for(int i=0;i<(samples*hw_voices);i++)
	{
		*dst=*src; dst++; src++;
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_rec_buffers_wdm_16(dword * restrict src,int hw_voices,int samples)
{
	word * restrict dst = (word *)CurrentBufferPointer;

	for(int i=0;i<(samples*hw_voices);i++)
	{
		*dst++=(word)((*src++)>>16);
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_wdm(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples)
{
	dword * restrict src = (dword *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		for(int j=0;j<hw_voices;j++)
		{
            buffers[j][i]=*src++;
        }
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_wdm(word * restrict buffers[NUM_VOICES],int hw_voices,int samples)
{
	word * restrict src = (word *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		for(int j=0;j<hw_voices;j++)
		{
            buffers[j][i]=*src++;
        }
	}
}


#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_wdm_16(dword * restrict buffers[NUM_VOICES],int hw_voices,int samples)
{
	word * restrict src = (word *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		for(int j=0;j<hw_voices;j++)
		{
			buffers[j][i]=(*src++)<<16;
        }
	}
}

#ifdef CE_OPTIMIZE
#pragma optimize("gty", on)
#pragma inline_depth(16)
#endif

#pragma code_seg()
void BufferNotification::process_pb_buffers_wdm_16(word * restrict buffers[NUM_VOICES],int hw_voices,int samples)
{
	word * restrict src = (word *)CurrentBufferPointer;

	for(int i=0;i<samples;i++)
	{
		for(int j=0;j<hw_voices;j+=2)
		{
			buffers[j][i]=0;
            buffers[j+1][i]=*src++;
        }
	}
}
