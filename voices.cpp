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
#include "debug.h"
#include "hal.h"
#include "voices.h"

#pragma code_seg()
int Voices::hw_cache_size(void)
{
	return 24+4;	// cache size for 16-bit engine: 24 samples + 4 interpolations
}

#pragma code_seg()
Voices::Voices()
{
	magic=object_magic;
	hal=0;
	voice_usage=0;
}

#pragma code_seg()
Voices::~Voices()
{
	close();

	magic=0;
}

#pragma code_seg()
int Voices::close(void)
{
	if(hal)
	{
    	hal=0;
    	voice_usage=0;

    	debug("Voices::close: terminated\n");
	}

	return 0;
}

#pragma code_seg()
int Voices::init(Hal *hal_)
{
	hal=hal_;
	voice_usage=0;

	debug("Voices::init: initialized\n");

	return 0;
}

#pragma code_seg()
void Voices::setup_playback(int voice)
{
    dword start = (hal->hw_buffer.page_index<<(12-2)) + voice*hal->hw_full_buffer_in_samples;	// in =samples=
    // dword cra = hw_cache_size(); // 16-bit engine latency
    dword end = start + hal->hw_full_buffer_in_samples;
    dword cur = end-28;
    dword unfiltered=0x80808080;	// required in order to get bit-accurate playback

    // debug("Voice: %d, hw_voice: %d, fxrt: %08x, start: %08x, end: %08x\n",voice,voice*2+0,(0x3f3f0000 | (voice*2) | ((voice*2+1)<<8) | unfiltered),start,end);
    
    int i=0;
    for(int i=0;i<2;i++)
	hal->writeptr_multiple(voice*2+i, 
        		IFATN, 0xffff,
        		DCYSUSV, 0x0,
        		VTFT, calc_vtft(!i),
        		CVCF, calc_vtft(!i),
        		PTRX, calc_ptrx(!i),	// PITCHTARGET is 0
        		CPF,  CPF_STEREO_MASK,
                FXRT1_K2, (0x3f3f3f00 | (voice*2+i) | unfiltered), // SendA is lowest byte FXBUS
                FXRT2_K2, (0x3f3f3f3f | unfiltered),
                // send e,f,g,h
                FXAMOUNT_K2, 0x00000000,
				// CSL, ST, CA
			    DSL, end,
			    PSST, start,
			    CCCA, cur|CCCA_INTERPROM_0,		// implies CCCA_INTERPROM_0
			    								// set to -48 samples to trigger stop-on-loop
			    // Clear filter delay memory
			    Z1, 0,
			    Z2, 0,
			    // Invalidate maps
			    MAPA, MAP_PTI_MASK | (hal->hw_buffer.silent_page.physical << 1),
			    MAPB, MAP_PTI_MASK | (hal->hw_buffer.silent_page.physical << 1),
				// modulation envelope
			    LFOVAL1, 0x0,
			    FMMOD, 0x0,
			    TREMFRQ, 0x0,
			    FM2FRQ2, 0x0,
			    ENVVAL, 0x0,
				// volume envelope 
			    ATKHLDV, 0,
			    ENVVOL, 0,
			    ATKHLDM, 0,
			    DCYSUSM, 0,
			    PEFE, 0,
				// initial pitch
			    IP,0xe000,
			    REGLIST_END);
}

#pragma code_seg()
void Voices::init_cache(int voice)
{
	unsigned ccis=hw_cache_size(); // 24+4 for 16 bit

	int i=0;
	for(int i=0;i<2;i++)
    	hal->writeptr_multiple(voice*2+i,
        	CD0 + 0, 0x00000000,			// fill-in cache
        	CD0 + 1, 0x00000000,
        	CD0 + 2, 0x00000000,
        	CD0 + 3, 0x00000000,
        	CD0 + 4, 0x00000000,
        	CD0 + 5, 0x00000000,
        	CD0 + 6, 0x00000000,
        	CD0 + 7, 0x00000000,
        	CD0 + 8, 0x00000000,
        	CD0 + 9, 0x00000000,
        	CD0 + 10, 0x00000000,
        	CD0 + 11, 0x00000000,
        	CD0 + 12, 0x00000000,
        	CD0 + 13, 0x00000000,
        	CD0 + 14, 0x00000000,
        	CD0 + 15, 0x00000000,
        	emuCCR, 0,						// Reset cache 
        	CCR_CACHEINVALIDSIZE, ccis,		// init cache: cra=0; ccis=ccis
        	REGLIST_END);
}

#pragma code_seg()
int Voices::start_multiple(dword mask)
{
    if(voice_usage&mask)
    {
    	debug("!! Voices::start_multiple: start: busy [%08x vs %08x]\n",mask,voice_usage);
    	return STATUS_DEVICE_BUSY;
    }
    voice_usage|=mask;

    debug("Voices::start_multiple: start (x%x)\n",mask);

    hal->hw_buffer.voice_buffer.clear();

    // translate virtual mask to physical 16/48
    dword hw_mask=0;
    for(int i=0;i<hal->NUMBER_OF_VOICES;i++)
    {
     if(mask&(1<<i))
      hw_mask|=(3<<(i<<1));
    }

    // enable 'stop on loop' event
    hal->writeptr_multiple(0,
    	SOLEL,hw_mask,	
    	SOLEH,0x00,
    	REGLIST_END);

    for(int what=0;what<3;what++)
    {
       for(int i=0;i<hal->NUMBER_OF_VOICES;i++)
       {
           if(mask&(1<<i))
           {
               switch(what)
               {
               	case 0:
               			setup_playback(i);
               			break;
               	case 1: init_cache(i); break;
               	case 2:
               			for(int ch=0;ch<2;ch++)
               			hal->writeptr_multiple(i*2+ch,
							PTRX,(0x4000<<16) | calc_ptrx(!ch),	// low and high
							CPF,(0x4000<<16) | CPF_STEREO_MASK,
							REGLIST_END);
						// we start-up audio playback, however, we will stop on the next loop
						// we will resume later synchronously by clearing SOLEL/SOLEH registers
                       	break;
               }
           } // mask& compare
       } // all voices
    }	// what

    return 0;
}

#pragma code_seg()
int Voices::stop_multiple(dword mask)
{
    if(!(voice_usage&mask))
    {
    	debug("!! Voices::stop_multiple: not started [%08x vs %08x]\n",mask,voice_usage);
    	return STATUS_INVALID_PARAMETER;
    }
    voice_usage&=(~mask);

    debug("Voices::stop_multiple: stopped (x%x)\n",mask);

    for(int i=0;i<hal->NUMBER_OF_VOICES;i++)
    {
    	if(mask&(1<<i))
       	{
       		for(int ch=0;ch<2;ch++)
       		hal->writeptr_multiple(i*2+ch,
        	 	PTRX, 0,
	         	CPF,  0, 
	         	VTFT, 0,
	         	CVCF, 0,
	        	REGLIST_END);
	    }
    }

    // disable 'stop on loop' event
    hal->writeptr_multiple(0,
    	SOLEL,0x0,
    	SOLEH,0x0,
    	REGLIST_END);

    return 0;
}
