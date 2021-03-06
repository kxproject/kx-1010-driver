/* 1010 PCI/PCIe Audio Driver
   Copyright (c) Eugene Gavrilov, 2002-2016
   Copyright 1999, 2000 Creative Labs, Inc. 

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

#define PTR					0x00		// Indexed register set pointer register	
						// NOTE: The CHANNELNUM and ADDRESS words can	
						// be modified independently of each other.	
#define PTR_CHANNELNUM_MASK	0x0000003f	// For each per-channel register, indicates the	
						// channel number of the register to be		
						// accessed.  For non per-channel registers the	
						// value should be set to zero.			
#define PTR_ADDRESS_MASK	0xffff0000	// Register index (10k2; incl 10k1 (was 7ff)	

#define DATA				0x04		// Indexed register set data register		

#define IPR					0x08		// Global interrupt pending register		
						// Clear pending interrupts by writing a 1 to	
						// the relevant bits and zero to the other bits	
#define IPR_P16V_ENABLE				0x80000000 
#define IPR_WATERMARK_REACHED		0x40000000
#define IPR_GPIO_CHANGE				0x20000000
#define IPR_K2_MIDITRANSBUFEMPTY 	0x10000000
#define IPR_K2_MIDIRECVBUFEMPTY 	0x08000000 
#define IPR_SPDIFBUFHALFFUL_K2		0x04000000
#define IPR_SPDIFBUFFUL_K2			0x02000000
#define IPR_SAMPLERATETRACKER		0x01000000	// Sample rate tracker lock status change	
#define IPR_FXDSP					0x00800000	// Enable FX DSP interrupts			
#define IPR_FORCEINT				0x00400000	// Force Sound Blaster interrupt		
#define IPR_PCIERROR				0x00200000	// PCI bus error				
#define IPR_VOLINCR					0x00100000	// Volume increment button pressed		
#define IPR_VOLDECR					0x00080000	// Volume decrement button pressed		
#define IPR_MUTE					0x00040000	// Mute button pressed				
#define IPR_MICBUFFULL				0x00020000	// Microphone buffer full			
#define IPR_MICBUFHALFFULL			0x00010000	// Microphone buffer half full			
#define IPR_ADCBUFFULL				0x00008000	// ADC buffer full				
#define IPR_ADCBUFHALFFULL			0x00004000	// ADC buffer half full				
#define IPR_EFXBUFFULL				0x00002000	// Effects buffer full				
#define IPR_EFXBUFHALFFULL			0x00001000	// Effects buffer half full			
#define IPR_GPSPDIFSTATUSCHANGE		0x00000800	// GPSPDIF channel status change SPDIF B	
#define IPR_CDROMSTATUSCHANGE		0x00000400	// CD-ROM channel status change SPDIF A		
#define IPR_INTERVALTIMER			0x00000200	// Interval timer terminal count		
#define IPR_MIDITRANSBUFEMPTY		0x00000100	// MIDI UART transmit buffer empty		
#define IPR_MIDIRECVBUFEMPTY		0x00000080	// MIDI UART receive buffer empty		
#define IPR_CHANNELLOOP				0x00000040	// One or more channel loop interrupts pending	
#define IPR_CHANNELNUMBERMASK		0x0000003f	// When IPR_CHANNELLOOP is set, indicates the	
						// Highest set channel in CLIPL or CLIPH.  When	
						// IP is written with CL set, the bit in CLIPL	
						// or CLIPH corresponding to the CIN value 	
						// written will be cleared.			

#define INTE					0x0c		// Interrupt enable register			

#define INTE_K2_GPIO_ENABLE 	0x00040000
#define INTE_K2_MIDITXENABLE 	0x00020000
#define INTE_K2_MIDIRXENABLE 	0x00010000 
#define INTE_SPDIF_BUFFUL_ENABLE_K2 		0x00008000 
#define INTE_SPDIF_HALFBUFFULL_ENABLE_K2	0x00004000 
#define INTE_SAMPLERATETRACKER	0x00002000	// Enable sample rate tracker interrupts	
						// NOTE: This bit must always be enabled       	
#define INTE_FXDSPENABLE		0x00001000	// Enable FX DSP interrupts			
#define INTE_PCIERRORENABLE		0x00000800	// Enable PCI bus error interrupts		
#define INTE_VOLINCRENABLE		0x00000400	// Enable volume increment button interrupts	
#define INTE_VOLDECRENABLE		0x00000200	// Enable volume decrement button interrupts	
#define INTE_MUTEENABLE			0x00000100	// Enable mute button interrupts		
#define INTE_MICBUFENABLE		0x00000080	// Enable microphone buffer interrupts		
#define INTE_ADCBUFENABLE		0x00000040	// Enable ADC buffer interrupts			
#define INTE_EFXBUFENABLE		0x00000020	// Enable Effects buffer interrupts		
#define INTE_GPSPDIFENABLE		0x00000010	// Enable GPSPDIF(a) status interrupts		
#define INTE_CDSPDIFENABLE		0x00000008	// Enable CDSPDIF(b) status interrupts		
#define INTE_INTERVALTIMERENB	0x00000004	// Enable interval timer interrupts		
#define INTE_MIDITXENABLE		0x00000002	// Enable MIDI transmit-buffer-empty interrupts	
#define INTE_MIDIRXENABLE		0x00000001	// Enable MIDI receive-buffer-empty interrupts	


#define WC						0x10		// Wall Clock register				
#define WC_SAMPLECOUNTER_MASK	0x03FFFFC0	// Sample periods elapsed since reset		
#define WC_SAMPLECOUNTER		0x14060010
#define WC_CURRENTCHANNEL		0x0000003F	// Channel [0..63] currently being serviced	
						// NOTE: Each channel takes 1/64th of a sample	
						// period to be serviced.			

#define HCFG_K1						0x14		// Hardware config register			
#define HCFG_CODECFORMAT_MASK_K1	0x00070000	// CODEC format					
#define HCFG_CODECFORMAT_AC97_K1	0x00000000	// AC97 CODEC format -- Primary Output		
#define HCFG_CODECFORMAT_I2S_K1		0x00010000	// I2S CODEC format -- Secondary (Rear) Output	
#define HCFG_GPINPUT0_K1			0x00004000	// External pin112				
#define HCFG_GPINPUT1_K1			0x00002000	// External pin110				
#define HCFG_GPOUTPUT_MASK_K1		0x00001c00	// External pins which may be controlled	
#define HCFG_GPOUT2_K1				0x00001000	// set to enable digital out on 5.1 cards	
#define HCFG_GPOUT1_K1				0x00000800
#define HCFG_GPOUT0_K1				0x00000400
#define HCFG_JOYENABLE_K1	      	0x00000200	// Internal joystick enable    			
#define HCFG_PHASETRACKENABLE_K1	0x00000100	// Phase tracking enable			
						// 1 = Force all 3 async digital inputs to use	
						// the same async sample rate tracker (ZVIDEO)	
#define HCFG_AC3ENABLE_MASK_K1		0x0x0000e0	// AC3 async input control - Not implemented	
#define HCFG_AC3ENABLE_ZVIDEO_K1	0x00000080	// Channels 0 and 1 replace ZVIDEO		
#define HCFG_AC3ENABLE_CDSPDIF_K1	0x00000040	// Channels 0 and 1 replace CDSPDIF		
#define HCFG_AC3ENABLE_GPSPDIF_K1  	0x00000020      // Channels 0 and 1 replace GPSPDIF             
#define HCFG_AUTOMUTE_K1			0x00000010	// When set, the async sample rate convertors	
						// will automatically mute their output when	
						// they are not rate-locked to the external	
						// async audio source  				
#define HCFG_LOCKSOUNDCACHE			0x00000008	// 1 = Cancel bustmaster accesses to soundcache 
						// NOTE: This should generally never be used.  	
#define HCFG_LOCKTANKCACHE_MASK		0x00000004	// 1 = Cancel bustmaster accesses to tankcache	
						// NOTE: This should generally never be used.  	
// #define HCFG_LOCKTANKCACHE	0x01020014
#define HCFG_MUTEBUTTONENABLE		0x00000002	// 1 = Master mute button sets AUDIOENABLE = 0.	
						// NOTE: This is a 'cheap' way to implement a	
						// master mute function on the mute button, and	
						// in general should not be used unless a more	
						// sophisticated master mute function has not	
						// been written.       				
#define HCFG_AUDIOENABLE			0x00000001	// 0 = CODECs transmit zero-valued samples	
						// Should be set to 1 when the EMU10K1 is	
						// completely initialized.

#define HCFG_K2					0x18
#define HCFG_K2_GPINPUT_MASK	0x0000ff00
#define HCFG_K2_GPOUTPUT_MASK	0x000000ff

#define TIMER			0x1a		// Timer terminal count register		
						// NOTE: After the rate is changed, a maximum	
						// of 1024 sample periods should be allowed	
						// before the new rate is guaranteed accurate.	
#define TIMER_RATE_MASK		0x000003ff	// Timer interrupt rate in sample periods	
						// 0 == 1024 periods, [1..4] are not useful	
// #define TIMER_RATE		0x0a00001a

#define AC97DATA		0x1c		// AC97 register set data register (16 bit)	

#define AC97ADDRESS		0x1e		// AC97 register set address register (8 bit)	
#define AC97ADDRESS_READY	0x80		// Read-only bit, reflects CODEC READY signal	
#define AC97ADDRESS_ADDRESS	0x7f		// Address of indexed AC97 register		

//******************************************************************************************************
// 10k1 / 10k2 PTR set
//******************************************************************************************************

#define CPF			0x00		// Current pitch and fraction register			
#define CPF_CURRENTPITCH_MASK	0xffff0000	// Current pitch (linear, 0x4000 == unity pitch shift) 	
#define CPF_CURRENTPITCH	0x10100000
#define CPF_STEREO_MASK		0x00008000	// 1 = Even channel interleave, odd channel locked	
#define CPF_STOP_MASK		0x00004000	// 1 = Current pitch forced to 0			
#define CPF_FRACADDRESS_MASK	0x00003fff	// Linear fractional address of the current channel	

#define PTRX			0x01		// Pitch target and send A/B amounts register		
#define PTRX_PITCHTARGET_MASK	0xffff0000	// Pitch target of specified channel			
#define PTRX_PITCHTARGET	0x10100001
#define PTRX_FXSENDAMOUNT_A_MASK 0x0000ff00	// Linear level of channel output sent to FX send bus A	
#define PTRX_FXSENDAMOUNT_A	0x08080001
#define PTRX_FXSENDAMOUNT_B_MASK 0x000000ff	// Linear level of channel output sent to FX send bus B	
#define PTRX_FXSENDAMOUNT_B	0x08000001
#define PTRX_SENDAB		0x10000001

#define CVCF			0x02		// Current volume and filter cutoff register		
#define CVCF_CURRENTVOL_MASK	0xffff0000	// Current linear volume of specified channel		
#define CVCF_CURRENTVOL		0x10100002
#define CVCF_CURRENTFILTER_MASK	0x0000ffff	// Current filter cutoff frequency of specified channel	
#define CVCF_CURRENTFILTER	0x10000002

#define VTFT			0x03		// Volume target and filter cutoff target register	
#define VTFT_VOLUMETARGET_MASK	0xffff0000	// Volume target of specified channel			
#define VTFT_FILTERTARGET_MASK	0x0000ffff	// Filter cutoff target of specified channel		
#define VTFT_VOLUME		0x10100003

#define Z1			0x05		// Filter delay memory 1 register			

#define Z2			0x04		// Filter delay memory 2 register			

#define PSST			0x06		// Send C amount and loop start address register	
#define PSST_FXSENDAMOUNT_C_MASK 0xff000000	// Linear level of channel output sent to FX send bus C	

#define PSST_FXSENDAMOUNT_C	0x08180006

#define PSST_LOOPSTARTADDR_MASK	0x00ffffff	// Loop start address of the specified channel		
#define PSST_LOOPSTARTADDR	0x18000006

#define DSL			0x07		// Send D amount and loop start address register	
#define DSL_FXSENDAMOUNT_D_MASK	0xff000000	// Linear level of channel output sent to FX send bus D	

#define DSL_FXSENDAMOUNT_D	0x08180007

#define DSL_LOOPENDADDR_MASK	0x00ffffff	// Loop end address of the specified channel		
#define DSL_LOOPENDADDR		0x18000007

#define CCCA			0x08		// Filter Q, interp. ROM, byte size, cur. addr register 
#define CCCA_RESONANCEMASK	0xf0000000	// Lowpass filter resonance (Q) height			
#define CCCA_INTERPROMMASK	0x0e000000	// Selects passband of interpolation ROM		
						// 1 == full band, 7 == lowpass				
						// ROM 0 is used when pitch shifting downward or less	
						// then 3 semitones upward.  Increasingly higher ROM	
						// numbers are used, typically in steps of 3 semitones,	
						// as upward pitch shifting is performed.		
#define CCCA_INTERPROM_0	0x00000000	// Select interpolation ROM 0				
#define CCCA_INTERPROM_1	0x02000000	// Select interpolation ROM 1				
#define CCCA_INTERPROM_2	0x04000000	// Select interpolation ROM 2				
#define CCCA_INTERPROM_3	0x06000000	// Select interpolation ROM 3				
#define CCCA_INTERPROM_4	0x08000000	// Select interpolation ROM 4				
#define CCCA_INTERPROM_5	0x0a000000	// Select interpolation ROM 5				
#define CCCA_INTERPROM_6	0x0c000000	// Select interpolation ROM 6				
#define CCCA_INTERPROM_7	0x0e000000	// Select interpolation ROM 7				
#define CCCA_8BITSELECT		0x01000000	// 1 = Sound memory for this channel uses 8-bit samples	
#define CCCA_CURRADDR_MASK	0x00ffffff	// Current address of the selected channel		
#define CCCA_CURRADDR		0x18000008
#define CCCA_INTERPROM		0x04190008
#define CCCA_RESONANCEQ		0x041c0008

#define emuCCR			0x09		// Cache control register				
#define CCR_CACHEINVALIDSIZE	0x07190009
#define CCR_CACHEINVALIDSIZE_MASK	0xfe000000	// Number of invalid samples cache for this channel    	
#define CCR_CACHELOOPFLAG	0x01000000	// 1 = Cache has a loop service pending			
#define CCR_INTERLEAVEDSAMPLES	0x00800000	// 1 = A cache service will fetch interleaved samples	
#define CCR_WORDSIZEDSAMPLES	0x00400000	// 1 = A cache service will fetch word sized samples	
#define CCR_READADDRESS		0x06100009
#define CCR_READADDRESS_MASK	0x003f0000	// Location of cache just beyond current cache service	
#define CCR_LOOPINVALSIZE	0x0000fe00	// Number of invalid samples in cache prior to loop	
						// NOTE: This is valid only if CACHELOOPFLAG is set	
#define CCR_LOOPFLAG		0x00000100	// Set for a single sample period when a loop occurs	
#define CCR_CACHELOOPADDRHI	0x000000ff	// DSL_LOOPSTARTADDR's hi byte if CACHELOOPFLAG is set	

#define CLP			0x0a		// Cache loop register (valid if CCR_CACHELOOPFLAG = 1) 
						// NOTE: This register is normally not used		
#define CLP_CACHELOOPADDR	0x0000ffff	// Cache loop address (DSL_LOOPSTARTADDR [0..15])	

#define MAPA			0x0c		// Cache map A						

#define MAPB			0x0d		// Cache map B						

#define MAP_PTE_MASK		0xffffe000	// The 19 MSBs of the PTE indexed by the PTI		
#define MAP_PTI_MASK		0x00001fff	// The 13 bit index to one of the 8192 PTE dwords      	

#define ENVVOL			0x10		// Volume envelope register				
#define ENVVOL_MASK		0x0000ffff	// Current value of volume envelope state variable	  
						// 0x8000-n == 666*n usec delay	       			

#define ATKHLDV 		0x11		// Volume envelope hold and attack register		
#define ATKHLDV_PHASE0		0x00008000	// 0 = Begin attack phase				
#define ATKHLDV_HOLDTIME_MASK	0x00007f00	// Envelope hold time (127-n == n*88.2msec)		
#define ATKHLDV_ATTACKTIME_MASK	0x0000007f	// Envelope attack time, log encoded			
						// 0 = infinite, 1 = 10.9msec, ... 0x7f = 5.5msec	

#define DCYSUSV 		0x12		// Volume envelope sustain and decay register		
#define DCYSUSV_PHASE1_MASK	0x00008000	// 0 = Begin attack phase, 1 = begin release phase	
#define DCYSUSV_SUSTAINLEVEL_MASK 0x00007f00	// 127 = full, 0 = off, 0.75dB increments		
#define DCYSUSV_CHANNELENABLE_MASK 0x00000080	// 0 = Inhibit envelope engine from writing values in	
						// this channel and from writing to pitch, filter and	
						// volume targets.					
#define DCYSUSV_DECAYTIME_MASK	0x0000007f	// Volume envelope decay time, log encoded     		
						// 0 = 43.7 sec, 1 = 21.8 sec, 0x7f = 0.022 sec

#define LFOVAL1 		0x13		// Modulation LFO value					
#define LFOVAL_MASK		0x0000ffff	// Current value of modulation LFO state variable	
						// 0x8000-n == 666*n usec delay				

#define ENVVAL			0x14		// Modulation envelope register				
#define ENVVAL_MASK		0x0000ffff	// Current value of modulation envelope state variable 	
						// 0x8000-n == 666*n usec delay				

#define ATKHLDM			0x15		// Modulation envelope hold and attack register		
#define ATKHLDM_PHASE0		0x00008000	// 0 = Begin attack phase				
#define ATKHLDM_HOLDTIME	0x00007f00	// Envelope hold time (127-n == n*42msec)		
#define ATKHLDM_ATTACKTIME	0x0000007f	// Envelope attack time, log encoded			
						// 0 = infinite, 1 = 11msec, ... 0x7f = 5.5msec		

#define DCYSUSM			0x16		// Modulation envelope decay and sustain register	
#define DCYSUSM_PHASE1_MASK	0x00008000	// 0 = Begin attack phase, 1 = begin release phase	
#define DCYSUSM_SUSTAINLEVEL_MASK 0x00007f00	// 127 = full, 0 = off, 0.75dB increments		
#define DCYSUSM_DECAYTIME_MASK	0x0000007f	// Envelope decay time, log encoded			
						// 0 = 43.7msec, 1 = 21.8msec, 0x7f = 22msec		

#define LFOVAL2 		0x17		// Vibrato LFO register					
#define LFOVAL2_MASK		0x0000ffff	// Current value of vibrato LFO state variable 		
						// 0x8000-n == 666*n usec delay				

#define IP			0x18		// Initial pitch register				
#define IP_MASK			0x0000ffff	// Exponential initial pitch shift			
						// 4 bits of octave, 12 bits of fractional octave	
#define IP_UNITY		0x0000e000	// Unity pitch shift					

#define IFATN			0x19		// Initial filter cutoff and attenuation register	
#define IFATN_FILTERCUTOFF_MASK	0x0000ff00	// Initial filter cutoff frequency in exponential units	
						// 6 most significant bits are semitones		
						// 2 least significant bits are fractions		
#define IFATN_FILTERCUTOFF	0x08080019
#define IFATN_ATTENUATION_MASK	0x000000ff	// Initial attenuation in 0.375dB steps			
#define IFATN_ATTENUATION	0x08000019


#define PEFE			0x1a		// Pitch envelope and filter envelope amount register	
#define PEFE_PITCHAMOUNT_MASK	0x0000ff00	// Pitch envlope amount					
						// Signed 2's complement, +/- one octave peak extremes	
#define PEFE_PITCHAMOUNT	0x0808001a
#define PEFE_FILTERAMOUNT_MASK	0x000000ff	// Filter envlope amount				
						// Signed 2's complement, +/- six octaves peak extremes 
#define PEFE_FILTERAMOUNT	0x0800001a
#define FMMOD			0x1b		// Vibrato/filter modulation from LFO register		
#define FMMOD_MODVIBRATO	0x0000ff00	// Vibrato LFO modulation depth				
						// Signed 2's complement, +/- one octave extremes	
#define FMMOD_MOFILTER		0x000000ff	// Filter LFO modulation depth				
						// Signed 2's complement, +/- three octave extremes	


#define TREMFRQ 		0x1c		// Tremolo amount and modulation LFO frequency register	
#define TREMFRQ_DEPTH		0x0000ff00	// Tremolo depth					
						// Signed 2's complement, with +/- 12dB extremes	
#define TREMFRQ_FREQ		0x000000ff	// Modulation LFO frequency				
						// 0.039Hz steps, maximum of 9.85 Hz.			
					

#define FM2FRQ2 		0x1d		// Vibrato amount and vibrato LFO frequency register	
#define FM2FRQ2_DEPTH		0x0000ff00	// Vibrato LFO vibrato depth				
						// Signed 2's complement, +/- one octave extremes	
#define FM2FRQ2_FREQUENCY	0x000000ff	// Vibrato LFO frequency				
						// 0.039Hz steps, maximum of 9.85 Hz.			

#define TEMPENV 		0x1e		// Tempory envelope register				
#define TEMPENV_MASK		0x0000ffff	// 16-bit value						
						// NOTE: All channels contain internal variables; do	
						// not write to these locations.			
// 0x1f unused

#define CD0			0x20		// Cache data 0 register				
#define CD1			0x21		// Cache data 1 register				
#define CD2			0x22		// Cache data 2 register				
#define CD3			0x23		// Cache data 3 register				
#define CD4			0x24		// Cache data 4 register				
#define CD5			0x25		// Cache data 5 register				
#define CD6			0x26		// Cache data 6 register				
#define CD7			0x27		// Cache data 7 register				
#define CD8			0x28		// Cache data 8 register				
#define CD9			0x29		// Cache data 9 register				
#define CDA			0x2a		// Cache data A register				
#define CDB			0x2b		// Cache data B register				
#define CDC			0x2c		// Cache data C register				
#define CDD			0x2d		// Cache data D register				
#define CDE			0x2e		// Cache data E register				
#define CDF			0x2f		// Cache data F register				

#define PTB			0x40		// Page table base register				
#define PTB_MASK		0xfffff000	// Physical address of the page table in host memory	

#define TCB			0x41		// Tank cache base register    				
#define TCB_MASK		0xfffff000	// Physical address of the bottom of host based TRAM	

#define ADCCR			0x42		// ADC sample rate/stereo control register		
#define ADCCR_RCHANENABLE_K2	0x00000020	// Enables right channel for writing to the host       	
#define ADCCR_LCHANENABLE_K2	0x00000010	// Enables left channel for writing to the host		
						// NOTE: To guarantee phase coherency, both channels	
						// must be disabled prior to enabling both channels.	
#define ADCCR_SAMPLERATE_MASK_K2 0x0000000f 	// Sample rate convertor output rate   			
#define ADCCR_SAMPLERATE_48	0x00000000	// 48kHz sample rate					
#define ADCCR_SAMPLERATE_44	0x00000001	// 44.1kHz sample rate					
#define ADCCR_SAMPLERATE_32	0x00000002	// 32kHz sample rate					
#define ADCCR_SAMPLERATE_24	0x00000003	// 24kHz sample rate					
#define ADCCR_SAMPLERATE_22	0x00000004	// 22.05kHz sample rate					
#define ADCCR_SAMPLERATE_16	0x00000005	// 16kHz sample rate					
#define ADCCR_SAMPLERATE_12_K2	0x00000006	// 12 kHz sample rate				
#define ADCCR_SAMPLERATE_11_K2	0x00000007	// 11.025kHz sample rate				
#define ADCCR_SAMPLERATE_8_K2	0x00000008	// 8kHz sample rate					

#define TBLSZ			0x43	// effects tank internal table size

#define TCBS			0x44		// Tank cache buffer size register			
#define TCBS_MASK		0x00000007	// Tank cache buffer size field				
#define TCBS_BUFFSIZE_16K	0x00000000
#define TCBS_BUFFSIZE_32K	0x00000001
#define TCBS_BUFFSIZE_64K	0x00000002
#define TCBS_BUFFSIZE_128K	0x00000003
#define TCBS_BUFFSIZE_256K	0x00000004
#define TCBS_BUFFSIZE_512K	0x00000005
#define TCBS_BUFFSIZE_1024K	0x00000006
#define TCBS_BUFFSIZE_2048K	0x00000007

#define MICBA			0x45		// AC97 microphone buffer address register		
#define MICBA_MASK		0xfffff000	// 20 bit base address					

#define ADCBA			0x46		// ADC buffer address register				
#define ADCBA_MASK		0xfffff000	// 20 bit base address					

#define FXBA			0x47		// FX Buffer Address 
#define FXBA_MASK		0xfffff000	// 20 bit base address					

#define MICBS			0x49		// Microphone buffer size register			

#define ADCBS			0x4a		// ADC buffer size register				

#define FXBS			0x4b		// FX buffer size register				

// The following mask values define the size of the ADC, MIX and FX buffers in bytes 
#define ADCBS_BUFSIZE_NONE	0x00000000
#define ADCBS_BUFSIZE_384	0x00000001
#define ADCBS_BUFSIZE_448	0x00000002
#define ADCBS_BUFSIZE_512	0x00000003
#define ADCBS_BUFSIZE_640	0x00000004
#define ADCBS_BUFSIZE_768	0x00000005
#define ADCBS_BUFSIZE_896	0x00000006
#define ADCBS_BUFSIZE_1024	0x00000007
#define ADCBS_BUFSIZE_1280	0x00000008
#define ADCBS_BUFSIZE_1536	0x00000009
#define ADCBS_BUFSIZE_1792	0x0000000a
#define ADCBS_BUFSIZE_2048	0x0000000b
#define ADCBS_BUFSIZE_2560	0x0000000c
#define ADCBS_BUFSIZE_3072	0x0000000d
#define ADCBS_BUFSIZE_3584	0x0000000e
#define ADCBS_BUFSIZE_4096	0x0000000f
#define ADCBS_BUFSIZE_5120	0x00000010
#define ADCBS_BUFSIZE_6144	0x00000011
#define ADCBS_BUFSIZE_7168	0x00000012
#define ADCBS_BUFSIZE_8192	0x00000013
#define ADCBS_BUFSIZE_10240	0x00000014
#define ADCBS_BUFSIZE_12288	0x00000015
#define ADCBS_BUFSIZE_14366	0x00000016
#define ADCBS_BUFSIZE_16384	0x00000017
#define ADCBS_BUFSIZE_20480	0x00000018
#define ADCBS_BUFSIZE_24576	0x00000019
#define ADCBS_BUFSIZE_28672	0x0000001a
#define ADCBS_BUFSIZE_32768	0x0000001b
#define ADCBS_BUFSIZE_40960	0x0000001c
#define ADCBS_BUFSIZE_49152	0x0000001d
#define ADCBS_BUFSIZE_57344	0x0000001e
#define ADCBS_BUFSIZE_65536	0x0000001f

#define CDCS			0x50		// (SPSA) CD-ROM digital channel status register	

#define GPSCS			0x51		// (SPSB) General Purpose SPDIF channel status register

#define DBG_10K2		0x53		
#define DBG_10K2_SINGLE_STEP		0x00020000
#define DBG_10K2_STEP			0x00010000
#define DBG_10K2_ZC			0x40000000
#define DBG_10K2_STEP_ADDR		0x000003ff
#define DBG_10K2_CONDITION_CODE		0x0000f800
#define DBG_10K2_SATURATION_OCCURED	0x20000000
#define DBG_10K2_SATURATION_ADDR	0x0ffc0000

#define SPCS0			0x54		// SPDIF output Channel Status 0 register	

#define SPCS1			0x55		// SPDIF output Channel Status 1 register	

#define SPCS2			0x56		// SPDIF output Channel Status 2 register	


#define SPCS_CLKACCYMASK	0x30000000	// Clock accuracy				
#define SPCS_CLKACCY_1000PPM	0x00000000	// 1000 parts per million			
#define SPCS_CLKACCY_50PPM	0x10000000	// 50 parts per million				
#define SPCS_CLKACCY_VARIABLE	0x20000000	// Variable accuracy				
#define SPCS_SAMPLERATEMASK	0x0f000000	// Sample rate					
#define SPCS_SAMPLERATE_44	0x00000000	// 44.1kHz sample rate				
#define SPCS_SAMPLERATE_48	0x02000000	// 48kHz sample rate				
#define SPCS_SAMPLERATE_32	0x03000000	// 32kHz sample rate				
#define SPCS_CHANNELNUMMASK	0x00f00000	// Channel number				
#define SPCS_CHANNELNUM_UNSPEC	0x00000000	// Unspecified channel number			
#define SPCS_CHANNELNUM_LEFT	0x00100000	// Left channel					
#define SPCS_CHANNELNUM_RIGHT	0x00200000	// Right channel				
#define SPCS_SOURCENUMMASK	0x000f0000	// Source number				
#define SPCS_SOURCENUM_UNSPEC	0x00000000	// Unspecified source number			
#define SPCS_GENERATIONSTATUS	0x00008000	// Originality flag (see IEC-958 spec)		
#define SPCS_CATEGORYCODEMASK	0x00007f00	// Category code (see IEC-958 spec)		
#define SPCS_MODEMASK		0x000000c0	// Mode (see IEC-958 spec)			
#define SPCS_EMPHASISMASK	0x00000038	// Emphasis					
#define SPCS_EMPHASIS_NONE	0x00000000	// No emphasis					
#define SPCS_EMPHASIS_50_15	0x00000008	// 50/15 usec 2 channel				
#define SPCS_COPYRIGHT		0x00000004	// Copyright asserted flag -- do not modify	
#define SPCS_NOTAUDIODATA	0x00000002	// 0 = Digital audio, 1 = not audio		
#define SPCS_PROFESSIONAL	0x00000001	// 0 = Consumer (IEC-958), 1 = pro (AES3-1992)	

// The 32-bit CLIx and SOLx registers all have one bit per channel control/status      		
#define CLIEL			0x58		// Channel loop interrupt enable low register	

#define CLIEH			0x59		// Channel loop interrupt enable high register	

#define CLIPL			0x5a		// Channel loop interrupt pending low register	

#define CLIPH			0x5b		// Channel loop interrupt pending high register	

#define SOLEL			0x5c		// Stop on loop enable low register		

#define SOLEH			0x5d		// Stop on loop enable high register		

#define SPBYPASS		0x5e		// SPDIF BYPASS mode register			
#define SPDIF_FORMAT	0x000f00	

#define AC97SLOT		0x5f		// additional AC97 slots enable bits 

#define CDSRCS			0x60		// (SRT0-spdifA) CD-ROM Sample Rate Converter status register	

#define GPSRCS			0x61		// (SRT1-spdifB) General Purpose SPDIF sample rate cvt status 

#define ZVSRCS			0x62		// (SRT2-spdifC) ZVideo sample rate converter status		
						// NOTE: This one has no SPDIFLOCKED field	
						// Assumes sample lock

// These three bitfields apply to CDSRCS, GPSRCS, and (except as noted) ZVSRCS.			
#define SRCS_VALIDAUDIO		0x04000000	// valid audio data 
#define SRCS_SPDIFLOCKED	0x02000000	// SPDIF stream locked				
#define SRCS_RATELOCKED		0x01000000	// Sample rate locked				
#define SRCS_ESTSAMPLERATE	0x0007ffff	// Estimated Sample Rate			

#define ADCIDX_K2		0x63
#define MICIDX_MASK             0x0000ffff      // 16-bit value                                 
#define MICIDX_IDX		0x10000063

#define MICIDX_K2               0x64            // Microphone recording buffer index register   

#define ADCIDX_MASK		0x0000ffff	// 16 bit index field				
#define ADCIDX_IDX		0x10000064

#define FXIDX			0x65		// FX recording buffer index register		
#define FXIDX_MASK		0x0000ffff	// 16-bit value					
#define FXIDX_IDX		0x10000065

#define MPUDATA_10K2		0x70
#define MPUSTAT_10K2		0x71
#define MPUCMD_10K2		MPUSTAT_10K2

#define MPUDATA2_10K2		0x72
#define MPUSTAT2_10K2		0x73
#define MPUCMD2_10K2		MPUSTAT_10K2

#define FXWCL			0x74		// Effects Engine Output Write Low Channels (0 - 31) 
#define FXWCH			0x75		// Effects Engine Output Write High Channels (32 - 63) 

#define EHC			0x76

#define EHC_ASYNC_BYPASS	0x80000000
#define EHC_P16V_PB_RATE	0x30000	
#define EHC_SRC48_BYPASS	0x0
#define EHC_SRC48_192		0x2000
#define EHC_SRC48_96		0x4000
#define EHC_SRC48_44		0x8000
#define EHC_SRC48_MUTED		0xC000
#define EHC_SRC48_MASK		0xE000

#define EHC_SPDIF_44		0x80
#define EHC_SPDIF_96		0x40
#define EHC_P17V_SPDIF_192  0x20    // p17v only?..
#define EHC_SPDIF_48		0x00
#define EHC_SRCMULTI_SPDIF_BYPASS	0x0
#define EHC_SRCMULTI_SPDIF_MASK		0xe0
#define EHC_SRCMULTI_SPDIF_MUTED	0xc0

#define EHC_SRCMULTI_I2S_BYPASS		0x0
#define EHC_SRCMULTI_I2S_192		0x200
#define EHC_SRCMULTI_I2S_96		    0x400
#define EHC_SRCMULTI_I2S_44		    0x800
#define EHC_SRCMULTI_I2S_MUTED		0xc00
#define EHC_SRCMULTI_I2S_MASK		0xe00

// additional bits for p17v:
#define EHC_P17V_TVM        0x1     // tank virtual memory mode [p17v only]
#define EHC_P17V_SEL0       0x30000 // 00: 48, 01: 44.1, 10: 96, 11: 192
#define EHC_P17V_SEL1       0xc0000
#define EHC_P17V_SEL2       0x300000
#define EHC_P17V_SEL3       0xc00000

#define FXRT2_K2		0x7c		
#define FXRT1_K2		0x7e		

#define FXAMOUNT_K2		0x7d		

#define FXSENDAMOUNT_H		0x0818007d
#define FXSENDAMOUNT_G		0x0810007d
#define FXSENDAMOUNT_F		0x0808007d
#define FXSENDAMOUNT_E		0x0800007d

#define E10K1_GPR_BASE		0x100		    	
#define E10K2_GPR_BASE		0x400

#define TANKMEMDATAREGBASE	0x200		
#define TANKMEMDATAREG_MASK	0x000fffff	

#define TANKMEMADDRREGBASE	0x300		

#define TRAM_CLEAR_K1	0x00800000	
#define TRAM_ALIGN_K1	0x00400000	
#define TRAM_WRITE_K1	0x00200000	
#define TRAM_READ_K1	0x00100000	

#define HCFG_I2SASRC0_K2			0x0010
#define HCFG_I2SASRC1_K2			0x0020
#define HCFG_I2SASRC2_K2			0x0040
#define HCFG_I2SPT0_K2				0x0100
#define HCFG_I2SPT1_K2				0x0200
#define HCFG_I2SPT2_K2				0x0400
#define HCFG_SLOWRAMPRATE_K2		0x1000 	
#define HCFG_CLOCKSYNTH_K2			0x2000 	
#define HCFG_VBIT_AUTOMUTE_K2		0x4000  
#define HCFG_AUTOMUTE_K2			0x8000 
#define HCFG_CODECFORMAT_K2			0x10000 
#define HCFG_CODECFORMAT2_K2		0x20000 
#define HCFG_XMM_K2					0x40000 
#define HCFG_44K_K2					0x80000 
#define HCFG_PBE_K2					0x100000

#define HWM	0x48	

#define CSBA	0x4c	
#define CSDC	0x4d
#define CSFE	0x4e
#define CSHG	0x4f

#define GP2SCS_10K2		0x52	

#define SPRI	0x6a	
#define SPRA	0x6b	
#define SPRC	0x6c	

#define SRT3	0x77	
#define SRT4	0x78
#define SRT5	0x79

#define SRT_ESTSAMPLERATE	0x1fffff	
#define SRT_RATELOCKED		0x1000000

#define TTB			0x6e		// Tank Table Base 
#define TDOF		0x6f		// Tank Delay Offset
#define TTDA		0x7a		// Tank Table DMA Address 
#define TTDD		0x7b		// Tank Table DMA Data 

#define HLIEL	0x66	// Half Loop Interrupt Enable (low) 
#define HLIEH	0x67	// Half Loop Interrupt Enable (high) 
#define HLIPL	0x68	// Half Loop Interrupt Pending (low) 
#define HLIPH	0x69	// Half Loop Interrupt Pending (high) 
