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
#include "emu.h"

#include <stdarg.h>

extern size_t emu1010b_netlist_size;
extern unsigned char emu1010b_netlist[];

#define TIMEOUT 16384

#pragma code_seg()
void Hal::writefn0(dword reg, dword data)
{
    if(reg & 0xff000000) 
    {
        dword mask;
        byte size, offset;

        size = (byte) ((reg >> 24) & 0x3f);
        offset = (byte) ((reg >> 16) & 0x1f);
        mask = ((1 << size) - 1) << offset;
        data = (data << offset) & mask;
        reg &= 0x7f;

        lock_instance l;
        hw_lock.acquire(&l);
        data |= inpd(port + (word)reg) & ~mask;
        outpd(port + reg,data);
        hw_lock.release(&l);
    } 
    else 
    {
        outpd(port + reg,data);
    }
}

#pragma code_seg()
dword Hal::readfn0(dword reg)
{
    dword val;

    if(reg & 0xff000000) 
    {
        dword mask;
        byte size, offset;

        size = (byte) ((reg >> 24) & 0x3f);
        offset = (byte) ((reg >> 16) & 0x1f);
        mask = ((1 << size) - 1) << offset;
        reg &= 0x7f;

        val = inpd(port + reg);

        return (val & mask) >> offset;
    }
    else 
    {
        return inpd(port + reg);
    }
}

#pragma code_seg()
void Hal::writefn0w(dword reg, word data)
{
        outpw(port + reg,data);
}

#pragma code_seg()
void Hal::writefn0b(dword reg, byte data)
{
        outp(port + reg,data);
}

#pragma code_seg()
word Hal::readfn0w(dword reg)
{
        return inpw(port + reg);
}

#pragma code_seg()
byte Hal::readfn0b(dword reg)
{
        return inp(port + reg);
}

#pragma code_seg()
void Hal::writeptrw(dword reg, dword channel, word data)
{
        dword regptr;

        regptr = ((reg << 16) & PTR_ADDRESS_MASK) | (channel & PTR_CHANNELNUM_MASK);

        lock_instance l;
        hw_lock.acquire(&l);
        outpd(port + PTR,regptr);
        outpw(port + DATA,data);
        hw_lock.release(&l);
}

#pragma code_seg()
word Hal::readptrw(dword reg, dword channel)
{
        dword regptr;
        word val;

        regptr = ((reg << 16) & PTR_ADDRESS_MASK) | (channel & PTR_CHANNELNUM_MASK);
        lock_instance l;
        hw_lock.acquire(&l);
        outpd(port + PTR,regptr);
        val = inpw(port + DATA);
        hw_lock.release(&l);

        return val;
}

#pragma code_seg()
void Hal::writeptrb(dword reg, dword channel, byte data)
{
        dword regptr;

        regptr = ((reg << 16) & PTR_ADDRESS_MASK) | (channel & PTR_CHANNELNUM_MASK);

        lock_instance l;
        hw_lock.acquire(&l);
        outpd(port + PTR,regptr);
        outp(port + DATA,data);
        hw_lock.release(&l);
}

#pragma code_seg()
byte Hal::readptrb(dword reg, dword channel)
{
        dword regptr;
        byte val;

        regptr = ((reg << 16) & PTR_ADDRESS_MASK) | (channel & PTR_CHANNELNUM_MASK);

        lock_instance l;
        hw_lock.acquire(&l);
        outpd(port + PTR,regptr);
        val = inp(port + DATA);
        hw_lock.release(&l);

        return val;
}

#pragma code_seg()
void Hal::writeptr(dword reg, dword channel, dword data)
{
    dword regptr;

    regptr = ((reg << 16) & PTR_ADDRESS_MASK) | (channel & PTR_CHANNELNUM_MASK);

    if(reg & 0xff000000) 
    {
        dword mask;
        byte size, offset;

        size = (byte) ((reg >> 24) & 0x3f);
        offset = (byte) ((reg >> 16) & 0x1f);
        mask = ((1 << size) - 1) << offset;
        data = (data << offset) & mask;

        lock_instance l;
        hw_lock.acquire(&l);
        outpd(port + PTR,regptr);
        data |= inpd(port + DATA) & ~mask;
        outpd(port + DATA,data);
        hw_lock.release(&l);
    }
     else
    {
    	lock_instance l;
        hw_lock.acquire(&l);
        outpd(port + PTR,regptr);
        outpd(port + DATA,data);
        hw_lock.release(&l);
    }
}

#pragma code_seg()
void Hal::writeptr_multiple(dword channel, ...)
{
    va_list args;

	dword reg;

    va_start(args, channel);

    lock_instance l;
    hw_lock.acquire(&l);
    while((reg = va_arg(args, dword)) != REGLIST_END)
    {
        dword data = va_arg(args, dword);
        dword regptr = (((reg << 16) & PTR_ADDRESS_MASK) | (channel & PTR_CHANNELNUM_MASK));

        outpd(port + PTR,regptr);

        if(reg & 0xff000000) 
        {
            int size = (reg >> 24) & 0x3f;
            int offset = (reg >> 16) & 0x1f;
            dword mask = ((1 << size) - 1) << offset;
            data = (data << offset) & mask;

            data |= inpd(port + DATA) & ~mask;
        }
        outpd(port + DATA,data);
    }
    hw_lock.release(&l);

    va_end(args);
}

#pragma code_seg()
dword Hal::readptr(dword reg, dword channel)
{
    dword regptr, val;

    regptr = ((reg << 16) & PTR_ADDRESS_MASK) | (channel & PTR_CHANNELNUM_MASK);

    if(reg & 0xff000000)
    {
        dword mask;
        byte size, offset;

        size = (byte) ((reg >> 24) & 0x3f);
        offset = (byte) ((reg >> 16) & 0x1f);
        mask = ((1 << size) - 1) << offset;

        lock_instance l;
        hw_lock.acquire(&l);
        outpd(port + PTR,regptr);
        val = inpd(port + DATA);
        hw_lock.release(&l);

        return (val & mask) >> offset;
    }
     else 
    {
    	lock_instance l;
        hw_lock.acquire(&l);
        outpd(port + PTR,regptr);
        val = inpd(port + DATA);
        hw_lock.release(&l);

        return val;
    }
}

#pragma code_seg()
dword Hal::readptr_fast(dword reg, dword channel)
{
        dword regptr;
        dword prev;
        dword val;

        regptr = ((reg << 16) & PTR_ADDRESS_MASK) | (channel & PTR_CHANNELNUM_MASK);
        prev=inpd(port + PTR);
        outpd(port + PTR,regptr);
        val = inpd(port + DATA);
        outpd(port + PTR, prev);

        return val;
}


#pragma code_seg()
int Hal::writefpga(dword reg, dword value)
{
    if (reg > 0x3f)
        return -1;

    reg += 0x40; // 0x40 upwards are registers
    if (value > 0x3f) // 0 to 0x3f are values
        return -2;

	lock_instance l;
    hw_lock.acquire(&l);

    outpd(port+HCFG_K2,reg);
    usleep(10);
    outpd(port+HCFG_K2,reg | 0x80);  // High bit clocks the value into the fpga.
    usleep(10);
    outpd(port+HCFG_K2,value);
    usleep(10);
    outpd(port+HCFG_K2,value | 0x80);  // High bit clocks the value into the fpga

    hw_lock.release(&l);

    return 0;
}

#pragma code_seg()
dword Hal::readfpga(dword reg)
{
    if (reg > 0x3f)
        return (dword)-1;

    reg += 0x40; /// 0x40 upwards are registers

    lock_instance l;
    hw_lock.acquire(&l);

    outpd(port+HCFG_K2,reg);
    usleep(10);
    outpd(port+HCFG_K2,reg | 0x80);  // High bit clocks the value into the fpga
    usleep(10);
    dword ret = ((inpd(port+HCFG_K2) >> 8) & 0x7f);

    hw_lock.release(&l);

    return ret;
}

// Each Destination has one and only one Source,
// but one Source can feed any number of Destinations simultaneously.

#pragma code_seg()
int Hal::fpga_link_src2dst(dword src, dword dst)
{
    writefpga( 0x00, ((dst >> 8) & 0x3f) );
    writefpga( 0x01, (dst & 0x3f) );
    writefpga( 0x02, ((src >> 8) & 0x3f) );
    writefpga( 0x03, (src & 0x3f) );

    return 0;
}

#pragma code_seg()
void Hal::wcwait(dword wait)
{
    volatile unsigned cnt;
    dword newtime=0, curtime=0;

    curtime = readfn0(WC_SAMPLECOUNTER);
    for(;wait;wait--) 
    {
        for(cnt=0;cnt<TIMEOUT;cnt++)
        {
            newtime = readfn0(WC_SAMPLECOUNTER);
            if(newtime != curtime)
                break;
        }

        if(cnt >= TIMEOUT) // timeout
            break;

        curtime = newtime;
    }
}

#pragma code_seg()
void Hal::init_spdif(int mode)
{
	 return;	// for E-DSP SPDIF outputs are not used

     // init value 0x02109204:

     // Clock accuracy    = 00    (1000ppm)
     // Sample Rate       = 0010  (48kHz)
     // Audio Channel     = 0001  (Left of 2)
     // Source Number     = 0000  (Unspecified)
     // Generation Status = 1     (Original for Cat Code 12)
     // Cat Code          = 0x12  (0010010) (Digital Signal Mixer)
     // Mode              = 00    (Mode 0)
     // Emphasis          = 000   (None)
     // CP                = 1     (Copyright unasserted)
     // AN                = 0     (Digital audio)
     // P                 = 0     (Consumer)

        dword cat_code=0x00001200;

        dword emph=0x0;

        if(mode!=EHC_SPDIF_44)
         emph|=SPCS_SAMPLERATE_48;

        dword bits=(SPCS_CLKACCY_1000PPM | 
                    SPCS_CHANNELNUM_LEFT | SPCS_SOURCENUM_UNSPEC | SPCS_GENERATIONSTATUS 
                    | cat_code | emph | SPCS_COPYRIGHT);

    writeptr_multiple(0,
                // SPDIF0
                SPCS0, bits,
                // SPDIF1
                SPCS1, bits,
                // SPDIF2 & SPDIF3 (10k1)
                SPCS2, bits,
                REGLIST_END);

     switch(mode)
     {
      case EHC_SPDIF_44:
      		bits=0xf1+0xa;
        break;
      case EHC_SPDIF_48:
      		bits=0xb1+0xa;
        break;
      case EHC_SPDIF_96:
      		bits=0xa1+0xa;
        break;
     }

        writeptr_multiple(1,
                SPCS0, bits,
                SPCS1, bits,
                SPCS2, bits,
                REGLIST_END);
}

#pragma code_seg()
int Hal::reset_voices(void)
{
    // init the envelope engine
    for(int i = 0; i < NUMBER_OF_VOICES; i++) 
    {
    	reset_voice(i);
	}

	return 0;
}

#pragma code_seg()
int Hal::reset_voice(int voice)
{
        	writeptr_multiple(voice,
                    DCYSUSV, 0,
                    IP, 0,
                    PTRX, 0,
                    CPF, 0,
                    emuCCR, 0,
                    REGLIST_END);

            wcwait(2);

            writeptr(CCCA, voice, 0);

            writeptr_multiple(voice,
                                    CD0,0,
                                    CD1,0,
                                    CD2,0,
                                    CD3,0,
                                    CD4,0,
                                    CD5,0,
                                    CD6,0,
                                    CD7,0,
                                    CD8,0,
                                    CD9,0,
                                    CDA,0,
                                    CDB,0,
                                    CDC,0,
                                    CDD,0,
                                    CDE,0,
                                    CDF,0,

                                    MAPA, 0xffffffff,
                                    MAPB, 0xffffffff,

                                    CSBA,0x0,
                        			CSDC,0x0,
			                        CSFE,0x0,
			                        CSHG,0x0,
			                        FXRT2_K2,0xbfbfbfbf,
			                        FXRT1_K2,0xbfbfbfbf,
			                        FXAMOUNT_K2,0x0,

                    VTFT, 0x0,
                    CVCF, 0x0,

                    Z1, 0,
                    Z2, 0,

                    ATKHLDM, 0,
                    DCYSUSM, 0,
                    IP, 0,
                    IFATN, 0,
                    PEFE, 0,
                    FMMOD, 0,
                    TREMFRQ, 0,
                    FM2FRQ2, 0,
                    TEMPENV, 0,

                    // These should be last
                    LFOVAL2, 0,
                    LFOVAL1, 0,
                    ATKHLDV, 0,
                    ENVVOL, 0,
                    ENVVAL, 0,
                    PSST, 0x40,
                    DSL, 0xffffff,
                    emuCCR, 0x0,

					REGLIST_END);

         wcwait(1);

    return 0;
}

#pragma code_seg()
int Hal::init(void)
{
    int ret=0;

    is_10k2=true;
    is_10k8=true;
    
    is_streaming=0;
    is_buffer_cleared=false;
    fxbs=0;

    total_hw_pb_voices=0;
    total_hw_rec_voices=0;
    memset(pb_buffers_0,0,sizeof(pb_buffers_0));
    memset(pb_buffers_1,0,sizeof(pb_buffers_1));
    rec_buffer_0=NULL;
    rec_buffer_1=NULL;

    hw_full_buffer_in_samples=DRIVER_DEF_FULL_BUFFER_IN_SAMPLES; 

    pci_id_ven=0;
    pci_id_dev=0;
    pci_id_subsys=0;

    debug("hal: initializing\n");

    // special sequence for E-DSP
    debug("hal: init e-dsp\n");
    outpd(port+HCFG_K1,0x0005a00c);
    outpd(port+HCFG_K1,0x0005a004);
    outpd(port+HCFG_K1,0x0005a000);
    outpd(port+HCFG_K1,0x0005a000);

    // disable all IRQs
    writefn0(INTE,0);

    // mute all, disable any IO
    dword tmp_hcfg_k1=0;
    tmp_hcfg_k1=readfn0(HCFG_K1);
    tmp_hcfg_k1|=(HCFG_LOCKSOUNDCACHE | HCFG_LOCKTANKCACHE_MASK);
    tmp_hcfg_k1&=(~HCFG_AUDIOENABLE);
    writefn0( HCFG_K1, tmp_hcfg_k1); // HCFG_AUDIOENABLED is 0

    // Reset recording buffers
    writeptr_multiple(0,
                MICBS, ADCBS_BUFSIZE_NONE,
                MICBA, 0,
                FXBS, ADCBS_BUFSIZE_NONE,
                FXBA, 0,
                ADCBS, ADCBS_BUFSIZE_NONE,
                ADCBA, 0,
                REGLIST_END);

    writeptr(SPBYPASS,0x0,SPDIF_FORMAT); // bypass is disabled; 24bit mode

    writeptr_multiple(0,
                SPRA,0,
                SPRC,0,
                SPRI,0,
                REGLIST_END);

    // Disable channel interrupts
    writeptr_multiple( 0,
                CLIEL, 0,
                CLIEH, 0,
                SOLEL, 0,
                SOLEH, 0,
                HLIEL, 0,
                HLIEH, 0,
                REGLIST_END);

    reset_voices();

    init_pagetable_and_tram();

    // Enable Vol_Ctrl irqs
    dword inte=0 /*INTE_VOLINCRENABLE | INTE_VOLDECRENABLE | INTE_MUTEENABLE | INTE_FXDSPENABLE*/; // we don't support these anyway
    writefn0(INTE,inte);

    ret=dsp.init(this); // implies dsp.stop();
    if(ret)
    {
    	debug("hal: !!! DSP init failed\n");
        return ret;
    }

    // read current HCFG_K1 and set new bits
    tmp_hcfg_k1=readfn0(HCFG_K1);

    // preserve these bits:
    tmp_hcfg_k1&=(HCFG_44K_K2|HCFG_CODECFORMAT_K2|HCFG_CODECFORMAT2_K2|HCFG_CLOCKSYNTH_K2|HCFG_SLOWRAMPRATE_K2);
    tmp_hcfg_k1|=HCFG_AUTOMUTE_K2 | HCFG_CLOCKSYNTH_K2|HCFG_CODECFORMAT_K2|(HCFG_I2SASRC0_K2|HCFG_I2SASRC1_K2|HCFG_I2SASRC2_K2);
    tmp_hcfg_k1|=((hw_buffer.tram.size==0)?HCFG_LOCKTANKCACHE_MASK:0);

    writefn0( HCFG_K1, tmp_hcfg_k1);

    // defaults are: 1a071 and 1700 / 1704 (when timer is enabled)
    //   CL also uses: HCFG_XMM_K2 (0x40000)
    //	 |HCFG_44K_K2 for 44.1

    debug("hal: hcfg: %08x inte: %08x\n",tmp_hcfg_k1,inte);

    voices.init(this);

    reload_fpga_firmware(true); // force fpga reload

	return ret;
}

#pragma code_seg()
int Hal::get_buffer_sizes(int *max_buffer_size,int *min_buffer_size,int *buffer_size_gran,int *hw_in_latency,int *hw_out_latency,int for_sampling_rate)
{
	// return ASIO buffer size restrictions:
	// values are in samples, 'full' buffer, single channel
	int div=1;

	if(for_sampling_rate==0)
	 for_sampling_rate=current_sampling_rate;

	switch(for_sampling_rate)
	{
		case 44100: break;
		case 48000: break;
		case 88200: div=2; break;
		case 96000: div=2; break;
		case 192000: div=4; break;
		case 176400: div=4; break;
		default:
			debug("!! Hal::get_buffer_size: invalid sampling rate [%d]\n",current_sampling_rate);
			return -1;
	}

	if(max_buffer_size) *max_buffer_size=hw_full_buffer_in_samples*div*16;
	if(min_buffer_size) *min_buffer_size=hw_full_buffer_in_samples*div;			// full buffer, samples
	if(buffer_size_gran) *buffer_size_gran=hw_full_buffer_in_samples*div/2;     // granularity is 1/2 of 'full buffer'

	if(max_buffer_size && min_buffer_size && buffer_size_gran)
		debug("Hal::get_buffer_sizes: min: %d max: %d, gran: %d for %d\n",*min_buffer_size,*max_buffer_size,*buffer_size_gran,for_sampling_rate);

	// FIXME: change h/w latencies accordingly

	// in samples:
	if(hw_in_latency) *hw_in_latency=1;
	if(hw_out_latency) *hw_out_latency=voices.hw_cache_size()+1; // typical

	return 0;
}

#pragma code_seg()
int Hal::reload_fpga_firmware(bool force)
{
	return upload_fpga_firmware(emu1010b_netlist,emu1010b_netlist_size,force);
}

#pragma code_seg()
int Hal::set_dsp_bypass(dsp_bypass_e bypass)
{
	settings_mutex.acquire();
		dsp.set_bypass(bypass);	
		dsp.reset(current_sampling_rate);
	settings_mutex.release();
	return 0;
}

#pragma code_seg()
int Hal::set_hardware_settings(hardware_parameters_t *params,int flag)
{
	timing.ping();

    settings_mutex.acquire();

	mute();

	current_n_channels=params->n_channels;

	if(flag&HW_SET_FPGA_FIRMWARE)
		init_fpga();

	if(flag&(HW_SET_SAMPLING_RATE|HW_SET_CLOCK))
		set_sample_rate(params->sampling_rate,params->clock_source,false); // mute=false

	if(flag&HW_SET_SPDIF_IO)
		set_spdif_adat_options(params->is_spdif_pro,params->spdif_tx_no_copy_bit,params->is_optical_in_adat,params->is_optical_out_adat,false); // mute=false

	if((flag&HW_SET_SPDIF_IO) || (flag&HW_SET_SAMPLING_RATE))
		set_fpga_routings(params->is_optical_in_adat,params->is_optical_out_adat,false); // mute=false

	if(flag&HW_SET_SAMPLING_RATE/*|HW_SET_SPDIF_IO*/) // SPDIF I/O parameters do not currently influence the DSP microcode
		dsp.reset(params->sampling_rate);

	unmute();

	settings_mutex.release();

	params->print_settings("Hal::set_hardware_settings");
	timing.print(1,"Hal::set_hardware_settings: took %d mS\n");

	return 0;
}

#pragma code_seg()
int Hal::get_hardware_settings(hardware_parameters_t *params)
{
    settings_mutex.acquire();

		params->n_channels=current_n_channels;

		// detect current sampling rate
		dword wclock = readfpga(EMU_HANA_WCLOCK);
		dword def_clock = readfpga(EMU_HANA_DEFCLOCK);
		dword spdif_mode=readfpga(EMU_HANA_SPDIF_MODE);
		dword optical_type=readfpga(EMU_HANA_OPTICAL_TYPE);
		dword reg38=readfpga(0x38);
		dword reg39=readfpga(0x39);

		int current_sampling_rate_=current_sampling_rate;

	settings_mutex.release();

	// work with cached values below:

	bool is_44k=!!(def_clock&EMU_HANA_DEFCLOCK_44_1K);

	params->sampling_rate = (is_44k?44100:48000)
					*((wclock&EMU_HANA_WCLOCK_2X)?2:1)
					*((wclock&EMU_HANA_WCLOCK_4X)?4:1);

	if(current_sampling_rate_!=params->sampling_rate)
		debug("!! Hal::get_hardware_settings: sampling rates mismatch: hal cur: %d vs hw: %d [%x, %x]\n",current_sampling_rate_,params->sampling_rate,wclock,def_clock);

	switch(wclock&EMU_HANA_WCLOCK_SRC_MASK)
	{
		case EMU_HANA_WCLOCK_INT_48K:
		case EMU_HANA_WCLOCK_INT_44_1K:	params->clock_source=InternalClock; break;
		case EMU_HANA_WCLOCK_HANA_SPDIF_IN: params->clock_source=SPDIF; break;
		case EMU_HANA_WCLOCK_HANA_ADAT_IN: params->clock_source=ADAT; break;
		case EMU_HANA_WCLOCK_SYNC_BNCN: params->clock_source=BNC; break;
		case EMU_HANA_WCLOCK_2ND_HANA: params->clock_source=Dock; break;
		default:
			debug("!! Hal::get_hardware_settings: invalid clock source (%x)\n",wclock);
			params->clock_source=UnknownClock;
			break;
	}

	params->is_spdif_pro=(spdif_mode&EMU_HANA_SPDIF_MODE_TX_PRO)?true:false;
	params->spdif_tx_no_copy_bit=(spdif_mode&EMU_HANA_SPDIF_MODE_TX_NOCOPY)?true:false;
	params->is_optical_in_adat=(optical_type&EMU_HANA_OPTICAL_IN_ADAT)?true:false;
	params->is_optical_out_adat=(optical_type&EMU_HANA_OPTICAL_OUT_ADAT)?true:false;

	// params->print_settings("Hal::get_hardware_settings");
	params->lock_status = (reg38&0x3f)|((reg39&0x3f)<<6);

	return 0;
}

#pragma code_seg()
int Hal::will_accept_hardware_settings(hardware_parameters_t *old_params,hardware_parameters_t *new_params)
{
	int ret=0;

	// this function is only called by Adapter::can_change_hardware_settings

	// use cached value or retrieve from hardware
	hardware_parameters_t old_params_; memset(&old_params_,0,sizeof(old_params_));
	if(old_params==NULL)
	{
		old_params=&old_params_;
        ret=get_hardware_settings(&old_params_);
        if(ret)
        	return ret;
    }

    settings_mutex.acquire();

    if(old_params->sampling_rate!=new_params->sampling_rate)
    {
    	if(is_streaming || !is_list_empty(&buffer_list))
    	{
    		// verify that there are no ASIO clients currently running
            struct list *pos;

    	    for_each_list_entry(pos,&buffer_list)
    	    {
    	    	BufferNotification *buff = list_item(pos, BufferNotification, list);

    	    	if(buff && buff->is_asio)
    	    	{
            		debug("!! Hal::will_accept_hardware_settings: device in use [ASIO] while trying to change sampling rate: %d -> %d [%s, %s]\n",
            			old_params->sampling_rate,new_params->sampling_rate,
            			is_streaming?"streaming":"paused",
            			is_list_empty(&buffer_list)?"no buffers":"buffers allocated");

            		ret=STATUS_DEVICE_BUSY;
            		break;
    	    	}
            }
    	}
    }

    settings_mutex.release();

    return ret;
}

#pragma code_seg()
void Hal::init_pagetable_and_tram(void)
{
    // Init page table & tank memory base register

    dword trbs=0;
    switch(hw_buffer.tram.size)
    {
           case 16*1024: trbs=TCBS_BUFFSIZE_16K; break;
           case 32*1024: trbs=TCBS_BUFFSIZE_32K; break;
           case 64*1024: trbs=TCBS_BUFFSIZE_64K; break;
           case 128*1024: trbs=TCBS_BUFFSIZE_128K; break;
           case 256*1024: trbs=TCBS_BUFFSIZE_256K; break;
           case 512*1024: trbs=TCBS_BUFFSIZE_512K; break;
           case 1024*1024: trbs=TCBS_BUFFSIZE_1024K; break;
           case 2048*1024: trbs=TCBS_BUFFSIZE_2048K; break;
           case 0: // disable TRAM
           default:
               trbs=0x0;
               break;
    }

    writeptr_multiple( 0,
               PTB, hw_buffer.virtual_pagetable.physical,
               TCB, trbs?hw_buffer.tram.physical:0,
               TBLSZ, trbs, 
               REGLIST_END);

    for(int chn = 0; chn < NUMBER_OF_VOICES; chn++) 
    {
       writeptr_multiple( chn,
                   MAPA, MAP_PTI_MASK | (hw_buffer.silent_page.physical * 2),
                   MAPB, MAP_PTI_MASK | (hw_buffer.silent_page.physical * 2),
                   REGLIST_END);
    }
}

#pragma code_seg()
Hal::~Hal()
{
	close();
}

#pragma code_seg()
int Hal::close(void)
{
	debug("hal: close\n");

	mute();

	// disable interrupts
    writefn0(INTE, 0);

    // disable audio
    dword tmp_hcfg_k1=0;
    tmp_hcfg_k1=readfn0(HCFG_K1);
    tmp_hcfg_k1|=(HCFG_LOCKSOUNDCACHE | HCFG_LOCKTANKCACHE_MASK);
    tmp_hcfg_k1&=(~HCFG_AUDIOENABLE);
    writefn0(HCFG_K1, tmp_hcfg_k1);

    dsp.stop();

    voices.close();

    reset_voices();

    // disable all buffers and interrupts
    writeptr_multiple( 0,
               PTB, 0,

               // recording buffers
               MICBS, ADCBS_BUFSIZE_NONE,
               MICBA, 0,
               FXBS, ADCBS_BUFSIZE_NONE,
               FXBA, 0,
               FXWCL, 0,
               FXWCH, 0,
               ADCBS, ADCBS_BUFSIZE_NONE,
               ADCBA, 0,
               TCBS, 0,
               TCB, 0,

               // voice interrupt
               CLIEL, 0,
               CLIEH, 0,
               SOLEL, 0,
               SOLEH, 0,
               REGLIST_END);

    writeptr(FXWCL, 0, 0);
    writeptr(FXWCH, 0, 0);

    dsp.close();

	return 0;
}

#pragma code_seg()
int Hal::mute(void)
{
	writefpga(EMU_HANA_UNMUTE,EMU_MUTE);
	writefn0(HCFG_K1,readfn0(HCFG_K1)&(~HCFG_AUDIOENABLE));
	
	return 0;
}

#pragma code_seg()
int Hal::unmute(void)
{
	writefn0(HCFG_K1,readfn0(HCFG_K1)|HCFG_AUDIOENABLE);
	writefpga(EMU_HANA_UNMUTE,EMU_UNMUTE);

	return 0;
}

#pragma code_seg()
int Hal::upload_fpga_firmware(byte *data,int size,bool force)
{
	dword val=readfpga(EMU_HANA_ID);
	if(((val & 0x3f) == 0x15) && !force)
	{
		debug("!! Hal::upload_fpga_firmware: already uploaded [%x]\n",val);
		// keep current FPGA firmware and continue

		goto DUMP;
	}
	else
		debug("Hal::upload_fpga_firmware: hana_id: %x (%s)\n",val,force?"force upload":"not uploaded");

   // FPGA netlist already present so clear it
   // Return to programming mode
   // just in case - clear FPGA
   writefpga(EMU_HANA_FPGA_CONFIG, EMU_HANA_FPGA_CONFIG_HANA);

   lock_instance l;
   hw_lock.acquire(&l);

   dword tmp;

   outpd(port+HCFG_K2, 0x00); 	// Set PGMN low for 1uS
   tmp = inpd(port+HCFG_K2);
   usleep(100);
   outpd(port+HCFG_K2, 0x80); 	// Leave bit 7 set during netlist setup
   tmp = inpd(port+HCFG_K2);
   usleep(100); 				// Allow FPGA memory to clean

   while(size--) 
   {
     byte value=*data; data++;

     for(int i = 0; i < 8; i++) 
     {
        byte reg = 0x80;
        if(value & 0x1)
           reg|=0x20;

        value = value >> 1;   
        outpd(port+HCFG_K2, reg);
        tmp = inpd(port+HCFG_K2);
        outpd(port+HCFG_K2, reg | 0x40);
        tmp = inpd(port+HCFG_K2);
     }
   }

   // After programming, set GPIO bit 4 high again
   outpd(port+HCFG_K2, 0x10);
   tmp = inpd(port+HCFG_K2);

   hw_lock.release(&l);

   writefpga(EMU_HANA_FPGA_CONFIG,0);

DUMP:
   // dump FPGA state
   debug("Hal::upload_fpga_firmware: Option: %x ID: %x, Versions: %d.%d, %d.%d, %d; DockPwr: %x\n",
	   	readfpga(EMU_HANA_OPTION_CARDS),
   		readfpga(EMU_HANA_ID),
   		readfpga(EMU_HANA_MAJOR_REV),
   		readfpga(EMU_HANA_MINOR_REV),
   		readfpga(EMU_DOCK_MAJOR_REV),
   		readfpga(EMU_DOCK_MINOR_REV),
   		readfpga(EMU_DOCK_BOARD_ID),
   		readfpga(EMU_HANA_DOCK_PWR));

   debug("Hal::upload_fpga_firmware: WClock: %x, Def: %x, Mute: %x, IRQ: %x, 38: %x, 39: %x, Cfg: %x\n",
   		readfpga(EMU_HANA_WCLOCK),
   		readfpga(EMU_HANA_DEFCLOCK),
   		readfpga(EMU_HANA_UNMUTE),
   		readfpga(EMU_HANA_IRQ_ENABLE),
   		readfpga(0x38),
   		readfpga(0x39),
   		readfpga(EMU_HANA_FPGA_CONFIG));

   return 0;
}

#pragma code_seg()
Hal::Hal(word port_,void *owner_)
{
	magic=object_magic;
	owner=owner_;
	port=port_;
	init_list(&buffer_list); 
	memset(&device_state,0,sizeof(device_state));
	timing.enable();
	memset((void *)events,0,sizeof(events));

	debug("hal: init - port: %04x\n",port);
	
	hw_lock.init();
	settings_mutex.init();
}

#pragma code_seg()
int Hal::set_sample_rate(int frequency,clocksource_t source,bool mute)
{
	bool is_44;
	dword mask;

	switch(frequency)
	{
		case 44100:  is_44=true;  mask=EMU_HANA_WCLOCK_1X; break;
		case 48000:  is_44=false; mask=EMU_HANA_WCLOCK_1X; break;
		case 88200:  is_44=true;  mask=EMU_HANA_WCLOCK_2X; break;
		case 96000:  is_44=false; mask=EMU_HANA_WCLOCK_2X; break;
		case 176400: is_44=true;  mask=EMU_HANA_WCLOCK_4X; break;
		case 192000: is_44=false; mask=EMU_HANA_WCLOCK_4X; break;
		default: debug("!! Hal::set_sample_rate: invalid rate %d\n",frequency); return STATUS_INVALID_PARAMETER;
	}

    dword val=readfn0(HCFG_K1);

    if(mute) writefpga(EMU_HANA_UNMUTE,EMU_MUTE);

	if(is_44)
	{
		val|=HCFG_44K_K2;
        writefpga(EMU_HANA_DEFCLOCK,EMU_HANA_DEFCLOCK_44_1K);
	}
	else
	{
        val&=(~HCFG_44K_K2);
        writefpga(EMU_HANA_DEFCLOCK,EMU_HANA_DEFCLOCK_48K);
	}

	switch(source)
	{
		case InternalClock: writefpga(EMU_HANA_WCLOCK,(is_44?EMU_HANA_WCLOCK_INT_44_1K:EMU_HANA_WCLOCK_INT_48K) | mask); break;
		case SPDIF: 		writefpga(EMU_HANA_WCLOCK,EMU_HANA_WCLOCK_HANA_SPDIF_IN | mask); break;
		case ADAT: 			writefpga(EMU_HANA_WCLOCK,EMU_HANA_WCLOCK_HANA_ADAT_IN | mask); break;
		case BNC: 			writefpga(EMU_HANA_WCLOCK,EMU_HANA_WCLOCK_SYNC_BNCN | mask); break;
		case Dock: 			writefpga(EMU_HANA_WCLOCK,EMU_HANA_WCLOCK_2ND_HANA | mask); break;
		default:
			debug("!! Hal::set_sample_rate: invalid clock source [%d]\n",source);
	}

	writefn0(HCFG_K1,val);

	msleep(20);

    if(mute) writefpga(EMU_HANA_UNMUTE,EMU_UNMUTE);

    current_sampling_rate=frequency;

    // set-up 10kx (legacy, unused on E-DSP) SPDIF registers:
    switch(frequency)
    {
    	default:
    	case 48000: init_spdif(EHC_SPDIF_48); break;
    	case 44100: init_spdif(EHC_SPDIF_44); break;
    	case 96000: init_spdif(EHC_SPDIF_96); break;
    }

    return 0;
}

#pragma code_seg()
int Hal::set_fpga_routings(bool is_optical_in_adat,bool /*is_optical_out_adat*/,bool mute)
{
	if(mute) writefpga(EMU_HANA_UNMUTE,EMU_MUTE);

	// default connections: SPDIF and analog

	switch(current_sampling_rate)
	{
		case 44100:
		case 48000:
			// spdif
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+0,EMU_DST_HANA_SPDIF_LEFT1);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+4,EMU_DST_HANA_SPDIF_RIGHT1);
	
	        // analog
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+0,EMU_DST_HAMOA_DAC_LEFT1);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+4,EMU_DST_HAMOA_DAC_RIGHT1);

			// adat
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+0,EMU_DST_HANA_ADAT+0);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+4,EMU_DST_HANA_ADAT+1);
			
			break;

		case 88200:
		case 96000:
			// spdif
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+0,EMU_DST_HANA_SPDIF_LEFT1);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+4,EMU_DST_HANA_SPDIF_RIGHT1);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+1,EMU_DST_HANA_SPDIF_LEFT2);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+5,EMU_DST_HANA_SPDIF_RIGHT2);

			// analog
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+0,EMU_DST_HAMOA_DAC_LEFT1);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+4,EMU_DST_HAMOA_DAC_RIGHT1);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+1,EMU_DST_HAMOA_DAC_LEFT2);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+5,EMU_DST_HAMOA_DAC_RIGHT2);

			// adat
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+0,EMU_DST_HANA_ADAT+0);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+1,EMU_DST_HANA_ADAT+1);
            fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+4,EMU_DST_HANA_ADAT+2);
            fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+5,EMU_DST_HANA_ADAT+3);

			break;

		case 176400:
		case 192000:

			// spdif
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+0,EMU_DST_HANA_SPDIF_LEFT1);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+4,EMU_DST_HANA_SPDIF_RIGHT1);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+1,EMU_DST_HANA_SPDIF_LEFT2);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+5,EMU_DST_HANA_SPDIF_RIGHT2);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+2,EMU_DST_HANA_SPDIF_LEFT3);
    		fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+3,EMU_DST_HANA_SPDIF_LEFT4);
    		fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+6,EMU_DST_HANA_SPDIF_RIGHT3);
    		fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+7,EMU_DST_HANA_SPDIF_RIGHT4);

    		// analog
    		fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+0,EMU_DST_HAMOA_DAC_LEFT1);
    		fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+4,EMU_DST_HAMOA_DAC_RIGHT1);
    		fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+1,EMU_DST_HAMOA_DAC_LEFT2);
    		fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+5,EMU_DST_HAMOA_DAC_RIGHT2);
		    fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+2,EMU_DST_HAMOA_DAC_LEFT3);
    		fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+3,EMU_DST_HAMOA_DAC_LEFT4);
			fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+6,EMU_DST_HAMOA_DAC_RIGHT3);
    		fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+7,EMU_DST_HAMOA_DAC_RIGHT4);

    		// adat
            fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+0,EMU_DST_HANA_ADAT+0);
            fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+1,EMU_DST_HANA_ADAT+1);
            fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+2,EMU_DST_HANA_ADAT+2);
            fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+3,EMU_DST_HANA_ADAT+3);
            fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+4,EMU_DST_HANA_ADAT+4);
            fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+5,EMU_DST_HANA_ADAT+5);
            fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+6,EMU_DST_HANA_ADAT+6);
            fpga_link_src2dst(EMU_SRC_ALICE_EMU32A+7,EMU_DST_HANA_ADAT+7);

    		break;

		default:
			debug("!! Hal::set_fpga_routings: invalid sampling rate [%d]\n",current_sampling_rate);
			return -1;
	}
    
	// inputs: EMU_DST_ALICE2_EMU32_0 .. EMU_DST_ALICE2_EMU32_F
	int table[16] = { 0xf,0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe };

	if(!is_optical_in_adat) // regular SPDIF input? (optical or coaxial)?
	{
    	fpga_link_src2dst(EMU_SRC_HANA_SPDIF_LEFT1,table[0]);
    	fpga_link_src2dst(EMU_SRC_HANA_SPDIF_LEFT2,table[1]);
    	fpga_link_src2dst(EMU_SRC_HANA_SPDIF_LEFT3,table[2]);
    	fpga_link_src2dst(EMU_SRC_HANA_SPDIF_LEFT4,table[3]);
    	fpga_link_src2dst(EMU_SRC_HANA_SPDIF_RIGHT1,table[4]);
    	fpga_link_src2dst(EMU_SRC_HANA_SPDIF_RIGHT2,table[5]);
    	fpga_link_src2dst(EMU_SRC_HANA_SPDIF_RIGHT3,table[6]);
    	fpga_link_src2dst(EMU_SRC_HANA_SPDIF_RIGHT4,table[7]);
    }
    else
	switch(current_sampling_rate)
	{
		case 48000:
		case 44100:
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+0,table[0]);
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+1,table[4]);
             	break;
		case 96000:
		case 88200:
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+0,table[0]);
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+1,table[1]);
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+2,table[4]);
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+3,table[5]);
             	break;
		case 192000:
		case 176400:
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+0,table[0]);
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+1,table[1]);
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+2,table[2]);
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+3,table[3]);
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+4,table[4]);
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+5,table[5]);
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+6,table[6]);
             	fpga_link_src2dst(EMU_SRC_HANA_ADAT+7,table[7]);
             	break;
	}

	if(mute) writefpga(EMU_HANA_UNMUTE,EMU_UNMUTE);

    return 0;
}

#pragma code_seg()
int Hal::set_spdif_adat_options(bool is_spdif_pro,bool spdif_tx_no_copy_bit,bool is_optical_in_adat,bool is_optical_out_adat,bool mute)
{
	if(mute) writefpga(EMU_HANA_UNMUTE,EMU_MUTE);

	writefpga(EMU_HANA_SPDIF_MODE,(is_spdif_pro?EMU_HANA_SPDIF_MODE_TX_PRO:EMU_HANA_SPDIF_MODE_TX_COMSUMER)|
								  (spdif_tx_no_copy_bit?EMU_HANA_SPDIF_MODE_TX_NOCOPY:0));
	writefpga(EMU_HANA_OPTICAL_TYPE,(is_optical_in_adat?EMU_HANA_OPTICAL_IN_ADAT:EMU_HANA_OPTICAL_IN_SPDIF)|
								    (is_optical_out_adat?EMU_HANA_OPTICAL_OUT_ADAT:EMU_HANA_OPTICAL_OUT_SPDIF));

	msleep(20);

	if(mute) writefpga(EMU_HANA_UNMUTE,EMU_UNMUTE);

	return 0;
}

#pragma code_seg()
int Hal::get_clock_frequency(clocksource_t clock)
{
	switch(clock)
	{
		case SPDIF: return 0x1770000 / (((readfpga(EMU_HANA_WC_SPDIF_HI) << 5) | readfpga(EMU_HANA_WC_SPDIF_LO))+1);
		case ADAT: 	return 0x1770000 / (((readfpga(EMU_HANA_WC_ADAT_HI) << 5) | readfpga(EMU_HANA_WC_ADAT_LO))+1);
		case BNC: 	return 0x1770000 / (((readfpga(EMU_HANA_WC_BNC_HI) << 5) | readfpga(EMU_HANA_WC_BNC_LO))+1);
		case Dock: 	return 0x1770000 / (((readfpga(EMU_HANA2_WC_SPDIF_HI) << 5) | readfpga(EMU_HANA2_WC_SPDIF_LO))+1);
		default: 
			return 0;
	}
}


#pragma code_seg()
int Hal::init_fpga(void)
{
	writefpga(EMU_HANA_IRQ_ENABLE,0 /*EMU_HANA_IRQ_WCLK_CHANGED|EMU_HANA_IRQ_ADAT|EMU_HANA_IRQ_DOCK|EMU_HANA_IRQ_DOCK_LOST*/); // we currently do not support FPGA interrupts
	writefpga(EMU_HANA_MIDI_IN,0);
	writefpga(EMU_HANA_MIDI_OUT,0);
	
	writefpga(EMU_HANA_DOCK_LEDS_1,0x0);
	writefpga(EMU_HANA_DOCK_LEDS_2,0x0);
	writefpga(EMU_HANA_DOCK_LEDS_3,0x0);

    writefpga(EMU_HANA_ADC_PADS,0x0 /* EMU_HANA_DOCK_ADC_PAD1|EMU_HANA_DOCK_ADC_PAD2|EMU_HANA_DOCK_ADC_PAD3|EMU_HANA_0202_ADC_PAD1*/ ); // +4dB PRO
	writefpga(EMU_HANA_DAC_PADS,EMU_HANA_0202_DAC_PAD1 /* EMU_HANA_DOCK_DAC_PAD1|EMU_HANA_DOCK_DAC_PAD2|EMU_HANA_DOCK_DAC_PAD3|EMU_HANA_DOCK_DAC_PAD4|EMU_HANA_0202_DAC_PAD1*/); // +4dB PRO
	debug("Hal::init_fpga: DAC pads set to -12dB for 0202\n");

	return 0;
}

#pragma data_seg()
int Hal::adjust_rec_buffer_size(int preferred_size)
{
	if(preferred_size==384) { fxbs=ADCBS_BUFSIZE_384; } else
	if(preferred_size==448) { fxbs=ADCBS_BUFSIZE_448; } else
	if(preferred_size==512) { fxbs=ADCBS_BUFSIZE_512; } else
	if(preferred_size==640) { fxbs=ADCBS_BUFSIZE_640; } else
	if(preferred_size==768) { fxbs=ADCBS_BUFSIZE_768; } else
	if(preferred_size==896) { fxbs=ADCBS_BUFSIZE_896; } else
	if(preferred_size==1024) { fxbs=ADCBS_BUFSIZE_1024; } else
	if(preferred_size==1280) { fxbs=ADCBS_BUFSIZE_1280; } else
	if(preferred_size==1536) { fxbs=ADCBS_BUFSIZE_1536; } else
	if(preferred_size==1792) { fxbs=ADCBS_BUFSIZE_1792; } else
	if(preferred_size==2048) { fxbs=ADCBS_BUFSIZE_2048; } else
	if(preferred_size==2560) { fxbs=ADCBS_BUFSIZE_2560; } else
	if(preferred_size==3072) { fxbs=ADCBS_BUFSIZE_3072; } else
	if(preferred_size==3584) { fxbs=ADCBS_BUFSIZE_3584; } else
	if(preferred_size==4096) { fxbs=ADCBS_BUFSIZE_4096; } else
	if(preferred_size==5120) { fxbs=ADCBS_BUFSIZE_5120; } else
	if(preferred_size==6144) { fxbs=ADCBS_BUFSIZE_6144; } else
	if(preferred_size==7168) { fxbs=ADCBS_BUFSIZE_7168; } else
	if(preferred_size==8192) { fxbs=ADCBS_BUFSIZE_8192; } else
	if(preferred_size==10240) { fxbs=ADCBS_BUFSIZE_10240; } else
	if(preferred_size==12288) { fxbs=ADCBS_BUFSIZE_12288; } else
	if(preferred_size==14366) { fxbs=ADCBS_BUFSIZE_14366; } else
	if(preferred_size==16384) { fxbs=ADCBS_BUFSIZE_16384; } else
	if(preferred_size==20480) { fxbs=ADCBS_BUFSIZE_20480; } else
	if(preferred_size==24576) { fxbs=ADCBS_BUFSIZE_24576; } else
	if(preferred_size==28672) { fxbs=ADCBS_BUFSIZE_28672; } else
	if(preferred_size==32768) { fxbs=ADCBS_BUFSIZE_32768; } else
	if(preferred_size==40960) { fxbs=ADCBS_BUFSIZE_40960; } else
	if(preferred_size==49152) { fxbs=ADCBS_BUFSIZE_49152; } else
	if(preferred_size==57344) { fxbs=ADCBS_BUFSIZE_57344; } else
	if(preferred_size==65536) { fxbs=ADCBS_BUFSIZE_65536; } else 
	{
		debug("!! Hal::adjust_rec_buffer_size: invalid size %d\n",preferred_size);
		return STATUS_INVALID_PARAMETER;
	}

	return 0;
}

#pragma code_seg()
inline dword Hal::calc_rec_mask(void)
{
  	switch(current_sampling_rate)
  	{
  		case 48000:
  		case 44100: return 0xf;
  		case 88200:
  		case 96000: return 0xff;
  		case 176400:
  		case 192000: return 0xffff;
  		default:
  			return 0;
  	}
}

#pragma code_seg()
inline dword Hal::calc_pb_mask(void)
{      
  	switch(current_sampling_rate)
  	{
  		case 48000:
  		case 44100: return 0x3;
  		case 88200:
  		case 96000: return 0xf;
  		case 176400:
  		case 192000: return 0xff;
  		default:
  			return 0;
  	}
}

#pragma code_seg()
int Hal::start_recording(void)
{
	if(fxbs==0) { debug("!! Hal::start_rec: size not specified\n"); return STATUS_INVALID_PARAMETER; }

	dword mask=calc_rec_mask();
	if(mask==0)
	{
		debug("!! Hal::start_rec: invalid sampling rate %d\n",current_sampling_rate);
  		return STATUS_INVALID_PARAMETER;
  	}

	irq_enable(INTE_EFXBUFENABLE);

    // fxbs size = 0, write address, change fxwc bits, set fxbs
    writeptr_multiple(0,
    	FXBS, 0,
    	FXBA, hw_buffer.mtr_buffer.physical,
    	FXWCL, 0x0, // NOTE: we record 'mask' channels, but start counting from 32
    	FXWCH, mask,
    	// FXBS, fxbs, // later
    	REGLIST_END);

    // need to write ((FXBS<<16),fxbs) to start actual recording
    // this is done later to ensure synchronous audio start-up

    debug("Hal::start_rec: prepared, mask: %x, size: %x, addr: %x\n",
    	mask,fxbs,hw_buffer.mtr_buffer.physical);

	return 0;
}

#pragma code_seg()
int Hal::stop_recording(void)
{
	if(fxbs==0) { debug("!! Hal::stop_rec: invalid size\n"); }

   	irq_disable(INTE_EFXBUFENABLE);

   	fxbs=0;

   	// fxbs first
    writeptr_multiple(0,
       	FXBS, 0,
       	FXWCL, 0,
       	FXWCH, 0,
       	REGLIST_END);
    debug("Hal::stop_rec: stopped\n");

	// now it is OK to clean the buffers
	hw_buffer.mtr_buffer.clear();

    return 0;
}

#pragma code_seg()
int Hal::start_playback(void)	// if '0' is specified, function will calculate the mask automatically
{
	dword mask=calc_pb_mask();
	if(mask==0)
	{
   		debug("!! Hal::start_playback: invalid sampling rate %d\n",current_sampling_rate);
   		return STATUS_INVALID_PARAMETER;
   	}

	// set up actual set of voices
	voices.start_multiple(mask);

	// irq_enable(INTE_);
	// (we don't use this interrupt now)

	// audio is started, but with SOLEL/SOLEH set to 1
	// audio will automatically stop after initial buffer loop

	debug("Hal::start_playback: started %dHz, %d channels, mask: %x\n",current_sampling_rate,current_n_channels,mask);

	return 0;
}

#pragma code_seg()
int Hal::stop_playback(void)
{
	dword mask=calc_pb_mask();
	if(mask==0)
	{
 		debug("!! Hal::stop_playback: invalid sampling rate %d\n",current_sampling_rate);
   		return STATUS_INVALID_PARAMETER;
   	}

	// terminate actual set of voices
	voices.stop_multiple(mask);

    // irq_disable(INTE_);
    // (we don't use this interrupt now)

    // clear audio buffers
    hw_buffer.voice_buffer.clear();

	debug("Hal::stop_playback: stopped\n");

    return 0;
}

#pragma code_seg()
int Hal::sync_start(bool from_power_event)		// start playback and recording synchronously
{
	int ret=0;
	debug("Hal::sync_start\n");

    settings_mutex.acquire();

	int mult=0;
  	switch(current_sampling_rate)
  	{
  		case 48000:
  		case 44100: mult=2; break;		// these multipliers are for 2-channel stereo audio, 32-bit (2+2 channels for each stereo stream)
  		case 88200:
  		case 96000: mult=4; break;
  		case 176400:
  		case 192000: mult=8; break;
  		default:
  			debug("!! Hal::sync_start: invalid sampling rate [%d]\n",current_sampling_rate);
  			settings_mutex.release();
  			return 0;
  	}

	// check special case when returning from 'sleep' mode
	if(from_power_event)
	{
		if(device_state.was_streaming)
		{
			is_streaming=device_state.was_streaming;
		}
		else
		{
			// no need to resume: we were not playing back
			is_streaming=0;
			settings_mutex.release();
			debug("Hal::sync_start: nothing to resume\n");
			return 0;
		}
    }
    else
    {
    	if(is_streaming) // already?
    	{
    		is_streaming++;
    		debug("Hal::sync_start: already streaming: %d\n",is_streaming);
    		settings_mutex.release();
    		return 0;
    	}
    	else
    		is_streaming=1;
    }

    // setup buffers and pre-calculate stuff:
	adjust_rec_buffer_size(mult*hw_full_buffer_in_samples*4);
		// hw_full_buffer_in_samples is in =samples=, adjust_rec_buffer needs 'bytes'
		// mult: number of hw 'channels', 32-bit each;
		// will set fxbs
	// pre-calculate playback buffer offsets
	total_hw_pb_voices=mult;
	total_hw_rec_voices=mult;

	// debug("Hal::sync_start: pb: %d, rec: %d, mult: %d, rec_buf: %d\n",total_hw_pb_voices,total_hw_rec_voices,mult,hw_full_buffer_in_samples*4*mult);

	for(int i=0;i<NUM_VOICES;i++)
	{
		pb_buffers_0[i]=(dword *)hw_buffer.voice_buffer.addr+hw_full_buffer_in_samples*i;
		pb_buffers_1[i]=pb_buffers_0[i]+(hw_full_buffer_in_samples>>1);	// because this is double-buffer
	}

    rec_buffer_0=(dword *)hw_buffer.mtr_buffer.addr;
	rec_buffer_1=rec_buffer_0+((mult*hw_full_buffer_in_samples)>>1);	// because rec_buffer_1 is 'dword' and this is double-buffer

	// note: settings_mutex is still acquired

	start_recording();
	start_playback();

	// audio playback has started but will stop on the next loop
	// recording has NOT started yet

	// wait 10 ms
    {
      LARGE_INTEGER delay;
      delay.QuadPart=-100000; // wait 100000*100us (10 ms) relative

      KeDelayExecutionThread(KernelMode,FALSE,&delay);
    }

    // FIXME NOW
    // need to pre-load audio buffers for playback and shift current playback position accordingly

    int retries=0;

    // need to protect this with hw_lock:
    lock_instance l;
    hw_lock.acquire(&l);
    {
        // wait for 'good' moment
        // since we are using 2(32bit)*2(stereo)*2(96)*2(192)=16 channels max, need to wait for chn number 16 to start processing

        while(1)
        {
           dword WC_start=inpd(port + WC);
           if((WC_start&WC_CURRENTCHANNEL)>=16 && (WC_start&WC_CURRENTCHANNEL)<=20)
           	break;

           retries++;
        }

        #if DRIVER_START_REC_FIRST==1
        	// usleep(150); // delay recording in order to 'sync' playback and rec (due to pb cache); also lets PCI mem controller to flush memory
        	// start recording
        	outpd(port + PTR, (FXBS<<16));
        	outpd(port + DATA, fxbs);
        #endif

        // for playback, disable 'stop on loop' condition
        outpd(port + PTR, (SOLEL<<16));
        outpd(port + DATA, 0);
        // (SOLEH is never used anyway for our present channel allocation strategy):
        //outpd(port + PTR, (SOLEH<<16));
        //outpd(port + DATA, 0);    

        #if DRIVER_START_REC_FIRST==0
	        // usleep(150); // delay recording in order to 'sync' playback and rec (due to pb cache); also lets PCI mem controller to flush memory
	        // start recording
	        outpd(port + PTR, (FXBS<<16));
	        outpd(port + DATA, fxbs);
	    #endif
    }
    hw_lock.release(&l);

	settings_mutex.release();

	debug("Hal::sync_start: retries: %d\n",retries);

	return ret;
}


#pragma code_seg()
int Hal::sync_stop(bool from_power_event)		// stop playback and recording synchronously
{
	int ret=0;
	debug("Hal::sync_stop\n");

    settings_mutex.acquire(); // otherwise already acquired

	if(from_power_event)
	{
		if(!is_streaming)
		{
			// nothing is streaming atm
			device_state.was_streaming=0;
			settings_mutex.release();
			debug("Hal::sync_stop: nothing is streaming\n");
			return 0;
		}
		else
		{
			device_state.was_streaming=is_streaming;
			is_streaming=0;
		}
	}
	else
	{
		if(is_streaming)
		{
			is_streaming--;
			if(is_streaming!=0)
			{
				// still streaming smth
				debug("Hal::sync_stop: still streaming [%d]\n",is_streaming);
				settings_mutex.release();
				return 0;
			}
			// else is_streaming became 0
		}
		else
		{
			settings_mutex.release();
			debug("!! Hal::sync_stop: invalid condition: already stopped\n");
			return 0;
		}
	}

	stop_recording();
	stop_playback();

	settings_mutex.release();

	debug("Hal::sync_stopped\n");

	return ret;
}

#pragma code_seg()
void Hal::irq_enable(dword inte,bool at_dpc)
{
	lock_instance l;

	if(!at_dpc) hw_lock.acquire(&l);

    dword val = inpd(port + INTE) | inte;
    outpd(port + INTE,val);

	if(!at_dpc) hw_lock.release(&l);
}

#pragma code_seg()
void Hal::irq_disable(dword inte,bool at_dpc)
{
	lock_instance l;

	if(!at_dpc) hw_lock.acquire(&l);

    dword val = inpd(port + INTE) & (~inte);
    outpd(port + INTE,val);

	if(!at_dpc) hw_lock.release(&l);
}

#pragma code_seg()
int Hal::set_power_mode(bool sleep)		// true: sleep, false: wake-up
{
	if(sleep)
	{
		debug("Hal::set_power_mode: putting device into sleep mode\n");

        settings_mutex.acquire();

    		if(device_state.is_sleeping)
    		{
    			settings_mutex.release();
    			debug("Hal::set_power_mode: already sleeping\n");
    			return 0;
    		}

    		// save current state
            get_hardware_settings(&device_state.settings);

            sync_stop(true); // from_power_event

    		close();

    		device_state.is_sleeping=true;

		settings_mutex.release();
	}
	else
	{
		debug("Hal::set_power_mode: waking updevice\n");

		settings_mutex.acquire();

    		if(device_state.is_sleeping==false)
    		{
    			settings_mutex.release();
    			debug("Hal::set_power_mode: already woke up\n");
    			return 0;
    		}

    		device_state.is_sleeping=false;

    		init();

    		set_hardware_settings(&device_state.settings,HW_SET_ALL);

    		sync_start(true);

		settings_mutex.release();
	}

	return 0;
}

#pragma code_seg()
int Hal::add_notification(BufferNotification *n)
{
   	settings_mutex.acquire();

       	arrange_sync_call(SYNC_ADD_NOTIFICATION,n);

   	settings_mutex.release();

	return 0;
}

#pragma code_seg()
int Hal::remove_notification(BufferNotification *n)
{
   	settings_mutex.acquire();

       	arrange_sync_call(SYNC_REMOVE_NOTIFICATION,n);

   	settings_mutex.release();

	return 0;
}

#pragma code_seg()
int Hal::add_notification_protected(BufferNotification *n)
{
	// executed by KeSynchronizeExecution usually at DIRQ level
	// with Hal::settings_mutex acquired

	list_add(&n->list,&buffer_list);

	return 0;
}

#pragma code_seg()
int Hal::remove_notification_protected(BufferNotification *n)
{
	// executed by KeSynchronizeExecution usually at DIRQ level
	// with Hal::settings_mutex acquired

	// verify item is in the list first
	bool found=false;

    struct list *pos;
    for_each_list_entry(pos,&buffer_list)
    {
    	BufferNotification *buff = list_item(pos, BufferNotification, list);
    	if(buff && buff==n)
    	{
    		found=true;
    		break;
    	}
    }

    if(found)
    {
		list_del(&n->list);
	}
	return 0;
}

#if 0
// for debugging only

#pragma code_seg()
void Hal::test_sync(void)
{
	if(!is_streaming)
	 return;

	dword v1,v2,v3,v4,fxidx;
	int last_voice=0;

	switch(current_sampling_rate)
	{
		case 44100:
		case 48000:
			last_voice=4;
			break;
		case 88200:
		case 96000:
			last_voice=8;
			break;
	    case 176400:
	    case 192000:
	    	last_voice=16;
	    	break;
	}


	if(last_voice==0)
	{
		debug("Hal::test_sync: last_voice is undefined, curr sr: %d\n",current_sampling_rate);
		return;
	}

    lock_instance l;
    hw_lock.acquire(&l);

    outpd(port + PTR,(CCCA << 16) | (0));
    v1 = inpd(port + DATA);
    outpd(port + PTR,(CCCA << 16) | (1));
    v2 = inpd(port + DATA);
    outpd(port + PTR,(CCCA << 16) | (last_voice-2));
    v3 = inpd(port + DATA);
    outpd(port + PTR,(CCCA << 16) | (last_voice-1));
    v4 = inpd(port + DATA);
    outpd(port + PTR,(FXIDX << 16) | (0));
    fxidx = inpd(port + DATA);

	hw_lock.release(&l);

	debug("Hal::test_sync: %x, %x.%x.%x.%x\n",fxidx,v1,v2,v3,v4);

#if 0
	debug("Hal::test_sync: fxbus results: %08x %08x %08x %08x %08x %08x %08x %08x\n",
		readptr(E10K2_GPR_BASE+2,0),readptr(E10K2_GPR_BASE+3,0),
		readptr(E10K2_GPR_BASE+4,0),readptr(E10K2_GPR_BASE+5,0),
		readptr(E10K2_GPR_BASE+6,0),readptr(E10K2_GPR_BASE+7,0),
		readptr(E10K2_GPR_BASE+8,0),readptr(E10K2_GPR_BASE+9,0)
		);
#endif
	
#if 0
	dword ptrx,ccca,vtft,cpf,fxrt,dsl,psst;
	int v=0;

	ptrx = readptr(PTRX,v);
	ccca = readptr(CCCA,v);
	vtft = readptr(VTFT,v);
	cpf  = readptr(CPF,v);
	fxrt = readptr(FXRT1_K2,v);
	dsl  = readptr(DSL,v);
	psst = readptr(PSST,v);
	
	debug("Hal::test_sync: ptrx:%08x ccca:%08x vtft: %08x cpf: %08x fxrt: %08x, dsl: %08x, psst: %08x\n",
		ptrx,ccca,vtft,cpf,fxrt,dsl,psst);
	debug("Hal::test_sync: Mapa: %08x mapb: %08x, silent: %08x, voice: %08x\n",
		readptr(MAPA,v)>>1,readptr(MAPB,v)>>1,hw_buffer.silent_page.physical,hw_buffer.voice_buffer.physical);
#endif
}

#endif

