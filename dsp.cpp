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

#include "dsp.h"
#include "hal.h"

#include <math.h>

#pragma code_seg()
DSP::DSP()
{
	hal=NULL;
	magic=object_magic;
	bypass_mode=DSP_BYPASS_NONE;
}

#pragma code_seg()
DSP::~DSP()
{
	hal=NULL;
	magic=0;
}

#pragma code_seg()
int DSP::start(void)
{
	if(hal) hal->writeptr(DBG_10K2, 0, 0); else { debug("!! DSP::start: invalid sequence\n"); return STATUS_INVALID_PARAMETER; }
	return 0;
}

#pragma code_seg()
int DSP::stop(void)
{
	if(hal) hal->writeptr(DBG_10K2,0,DBG_10K2_SINGLE_STEP); else { debug("!! DSP::stop: invalid sequence\n"); return STATUS_INVALID_PARAMETER; }
	return 0;
}

#pragma code_seg()
void DSP::hwOP(dword at, dword op, dword z, dword w, dword x, dword y)
{
	hal->writeptr(first_instruction+(at)*2, 0, ((x) << high_operand_shift) | (y));
    hal->writeptr(first_instruction+(at)*2+1, 0, ((op) << opcode_shift ) | ((z) << high_operand_shift) | (w));
}

#pragma code_seg()
int DSP::clear(void)
{
	if(!hal) { debug("!! DSP::clear: invalid sequence\n"); return STATUS_INVALID_PARAMETER; }

	stop();

    // clear microcode
    for(int i = 0; i < microcode_size; i++)
    {
    	hwOP(i,ACC3,C_0-DSP_CONST+(hal->is_10k2?0x80:0),
    	            C_0-DSP_CONST+(hal->is_10k2?0x80:0),
    	            C_0-DSP_CONST+(hal->is_10k2?0x80:0),
    	            C_0-DSP_CONST+(hal->is_10k2?0x80:0));
    }


    // clear GPRs, TRAMs...
    for(int i = 0; i < 256; i++)
    {
    	hal->writeptr(E10K1_GPR_BASE+i,0,0);
    	hal->writeptr(TANKMEMADDRREGBASE+i,0,0);
    	hal->writeptr(E10K1_GPR_BASE+i,0,0);
    	hal->writeptr(TANKMEMADDRREGBASE+i,0,0);
    }


    for(int i=0; i<512; i++)
      hal->writeptr(E10K2_GPR_BASE, 0, 0);

	return 0;
}

#pragma code_seg()
int DSP::reset(int new_sampling_rate)
{
	int ret=0;

	clear();

	int n_voices=0;
	switch(new_sampling_rate)
	{
		case 44100: 
		case 48000:
				n_voices=2; break;
		case 88200:
		case 96000:
				n_voices=4; break;
		case 176400:
		case 192000:
				n_voices=8; break;

		default:
			debug("!! DSP::reset: invalid sampling rate: %d\n",new_sampling_rate);
			return -1;
	}


	// load default microcode

	int idx=0;

	const int tmp1=gpr(0);
	const int tmp2=gpr(1);

	const int tst1=gpr(2);
	const int tst2=gpr(3);
	const int tst3=gpr(4);
	const int tst4=gpr(5);
	const int tst5=gpr(6);
	const int tst6=gpr(7);
	const int tst7=gpr(8);
	const int tst8=gpr(9);

	const int mask=gpr(10);
	const int andmask=gpr(11);
	const int shl16=gpr(12);
	const int and24mask=gpr(13);

	const int vol_l=gpr(20);
	const int vol_r=gpr(21);

	int order_rec[8] = { 0,4,1,5,2,6,3,7 };
	int order_pb[8] = { 0,4,1,5,2,6,3,7 };

	hal->writeptr_multiple(0,

		tmp1,0,
		tmp2,0,

		tst1,0,
		tst2,0,
		tst3,0,
		tst4,0,
		tst5,0,
		tst6,0,
		tst7,0,
		tst8,0,

		mask,0x3fff,
		andmask,0xffff0000,
		shl16,0x8000,
		and24mask,0xffffffff,		// it might be a good idea to mask lowest byte, but this is questionable

		vol_l,0x80000000,
		vol_r,0x80000000,
		
		REGLIST_END);

    if(bypass_mode&DSP_BYPASS_EXT_LOOPBACK) // mirror inputs to outputs
    {
    	for(int v=0;v<n_voices;v++)
    	{
    			// cannot send inputs to outputs directly
            	hwOP(idx++, ACC3, 			tmp1,				r(C_0),		r(DSP_E32IN(v)),		r(C_0));
            	hwOP(idx++, ACC3, 			r(DSP_E32OUT(v)),	r(C_0),		tmp1,					r(C_0));
        }
    }
    else
    if(bypass_mode==DSP_BYPASS_INT_LOOPBACK)
    {
    	for(int v=0;v<n_voices;v++)
    	{
                // tmp2 = 32-bit f(FX_BUS(0)+FX_BUS(1))
            	hwOP(idx++, MACS, 			tmp1,				r(C_0),		r(DSP_FX(v*2+0)),		r(C_10000));
            	hwOP(idx++, ANDXOR, 		tmp2,				tmp1, 		mask, 					r(DSP_FX(v*2+1)));

            	// apply volume
                hwOP(idx++, MACS1,			tmp1,				r(C_0),		tmp2,					(v>=4?vol_r:vol_l));
        
                // increase output volume (*4 => <<2)
                hwOP(idx++, MACINTS,		tmp2,				r(C_0),		tmp1,					r(C_4));

                // now send it to rec
                // andxor is not needed in this case

            	hwOP(idx++,		ACC3,		tmp1,				tmp2,		r(C_0),					r(C_0));
            	hwOP(idx++,		MACINTS,	r(DSP_FX2(v*2+1)),	tmp1,		r(C_0),					r(C_0));

            	hwOP(idx++,		MACINTW,	tmp1,				r(C_0),		tmp1,					shl16);
            	hwOP(idx++,		MACW1,		r(DSP_FX2(v*2+0)),	tmp1,		tmp1,					r(C_80000000));
         }
    }
    else
	{
    	// Total 8 hardware outputs, 4+4 for left and right, each 4 handle 48*2*2 data for 192kHz
    	// Interleaving:
    	//      FxBus0,1   -> lowLeft48, highLeft48			->		0
    	//      FxBus2,3   -> lowRight48, highRight48		->		4
    	//      FxBus4,5   -> lowLeft96, highLeft96			->		1
    	//      FxBus6,7   -> lowRight96, highRight96		->		5
    	//		FxBus8,9   -> lowLeft192.1, highLeft192.1 	->		2
    	//		FxBus10,11 -> lowRight192.1, highRight192.2 ->		6
    	//      FxBus12,13 -> lowLeft192.2, highLeft192.2   ->		3
    	//		FxBus14,15 -> lowRight192.2, highRight192.2 ->		7
    	
    	for(int v=0;v<n_voices;v++)
    	{
    			//  lowest:  send: 0xc0, vol: 0x8000 -> ((data)>>1)    -> need to >>15 in DSP
                //  highest: send: 0xe0, vol: 0x8000 -> ((data<<1)>>1) -> nop
                //  Convert 16+16 -> 32-bit:
                //   macs   tmp1,   0,      inl,    C_10000
                //	 andxor	out,	tmp1,	mask,	inh

                // tmp2 = 32-bit f(FX_BUS(0)+FX_BUS(1))
            	hwOP(idx++, 	MACS, 		tmp1,						r(C_0),			r(DSP_FX(v*2+0)),		r(C_10000));
            	hwOP(idx++, 	ANDXOR, 	tmp2,						tmp1, 			mask, 					r(DSP_FX(v*2+1)));

            	// apply volume
                hwOP(idx++, 	MACS1,		tmp1,						r(C_0),			tmp2,					(v>=4?vol_r:vol_l));
        
                // increase output volume (*4 => <<2)
				hwOP(idx++, 	MACINTS,	tmp2,						r(C_0),			tmp1,					r(C_4));

				// test LSB+MSB samples, debug only
				#if 0
					hwOP(idx++, 	ACC3,		tst1+v,						tmp2,			r(C_0),					r(C_0));
				#endif

                if(bypass_mode&DSP_BYPASS_OUT_CONST)
                	hwOP(idx++, ACC3,		r(DSP_E32OUT(order_pb[v])), r(C_4f1bbcdc), 	r(C_0),					r(C_0));
                else
                	hwOP(idx++, ANDXOR,	    r(DSP_E32OUT(order_pb[v])),	tmp2,			and24mask,				r(C_0)); // mask lower bits
        }

        // test FXBUS outputs (debug only):
        #if 0
            for(int v=0;v<8;v++)
            {
             hwOP(idx++, 	MACS, 		tmp1,						r(C_0),			r(DSP_FX(v*2+0)),		r(C_10000));
             hwOP(idx++, MACINTS,			tst1+v,	r(C_0),		tmp1,			r(C_4));
            }
        #endif

        // Recording:
        //
        // output	out_h, out_l
        // static	shl16=0x8000
        // temp		tmp1
        // acc3		tmp1,	in,	0,		0
        // macints	out_h,	tmp1,  0,	0
        // macintw	tmp1,	0,	tmp1,	shl16
        // macwn	out_l,	tmp1,		tmp1,	0x80000000

        // Total 8 hardware inputs, 4+4 for left and right, each 4 handle 48*2*2 data for 192kHz
        // Interleaving:
        //      lowLeft48 highLeft48 lowRight48 highRight48 lowLeft96 highLeft96 lowRight96 highRight96 ....
        
        for(int v=0;v<n_voices;v++)
        {
        	if(bypass_mode&DSP_BYPASS_IN_CONST)
        		hwOP(idx++,	ACC3,		tmp1,				r(C_4f1bbcdc),				r(C_0),			r(C_0));
        	else
        		hwOP(idx++,	ACC3,		tmp1,				r(DSP_E32IN(order_rec[v])),	r(C_0),			r(C_0));

        	hwOP(idx++,		ANDXOR,		tmp1,				tmp1,						r(and24mask),	r(C_0));
        	
        	hwOP(idx++,		MACINTS,	r(DSP_FX2(v*2+1)),	tmp1,						r(C_0),			r(C_0));

        	hwOP(idx++,		MACINTW,	tmp1,				r(C_0),						tmp1,			shl16);
        	
        	hwOP(idx++,		MACW1,		r(DSP_FX2(v*2+0)),	tmp1,						tmp1,			r(C_80000000));
        }
    }

    // zero unused outputs
    for(int v=n_voices;v<8;v++)
    {
    	hwOP(idx++, ACC3,		r(DSP_E32OUT(order_pb[v])),  r(C_0),  r(C_0),				r(C_0));
    }

	start();

	return ret;
}

// param is controlled using KX_MAX_VOLUME.KX_MIN_VOLUME
// ret is from 0 to max linear
#pragma code_seg()
dword DSP::calc_volume(LONG value)
{
    // 0x80000000 - DSound/WDM mute
    if((ULONG)value==0x80000000U || value==MIN_VOLUME)
    {
    	return 0x0;
    }
    else if(value==MAX_VOLUME)
    {
    	return DSP_MAX_VOLUME;
    }
    else
    {
        if(value<MIN_VOLUME)
          value=MIN_VOLUME;
        if(value>MAX_VOLUME)
          value=MAX_VOLUME;

        dword ret=0;

        KFLOATING_SAVE state;
        KeSaveFloatingPointState(&state);

        // val=-10^((val/0x10000)/20):
        // (negative, because we are using MACS1 above)
        ret=(dword)(long)(-pow(10.0,(((double)value)*(7.62939453125e-7)))*(double)DSP_MAX_VOLUME);

        KeRestoreFloatingPointState(&state);

        return ret;
    }
}


#pragma code_seg()
void DSP::apply_volume(LONG left,LONG right)	// in dB/0x10000 units
{
	dword dsp_left=calc_volume(left),
		  dsp_right=calc_volume(right);

	// typical volume range: -144*0x10000...0dB in 0.5dB*0x10000 steps
	hal->writeptr_multiple(0,
		gpr(20), // vol_l
		dsp_left,
		gpr(21), // vol-r
		dsp_right,
		REGLIST_END);

   debug("Adapter::apply_volume: Left: %ddB Right: %ddB -> %08x : %08x\n",left/0x10000,right/0x10000,dsp_left,dsp_right);
}

#pragma code_seg()
int DSP::close(void)
{
	return 0;
}

#pragma code_seg()
int DSP::init(Hal *hal_)
{
	hal=hal_;

	if(hal->is_10k2)
	{
		first_instruction=E10K2_MICROCODE_BASE;
		high_operand_shift=E10K2_OP_SHIFT_HI;
		opcode_shift=E10K2_OP_SHIFT_LOW;
		microcode_size=E10K2_MAX_INSTRUCTIONS;
	}
	else
	{
		first_instruction=E10K1_MICROCODE_BASE;
		high_operand_shift=E10K1_OP_SHIFT_HI;
		opcode_shift=E10K1_OP_SHIFT_LOW;
		microcode_size=E10K1_MAX_INSTRUCTIONS;
	}
	is_10k2=hal->is_10k2;
	is_10k8=hal->is_10k8;

	return 0;
}
