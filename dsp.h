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
#include "8010.h"

class Hal;

enum e10kx_dsp_opcodes
{
    MACS=0,
    MACS1,
    MACW,
    MACW1,
    MACINTS,
    MACINTW,
    ACC3,
    MACMV,
    ANDXOR,
    TSTNEG,
    LIMIT,
    LIMIT1,
    LOG,
    EXP,
    INTERP,
    SKIP,
    TOTAL_OPCODES
};

#define DSP_CONST 	0x2000

// constants (on 10k2: 0x40-> 0xc0); logical numbering;
#define C_0         0x2040 // 0x0
#define C_1         0x2041 // 0x1
#define C_2         0x2042 // 0x2
#define C_3         0x2043 // 0x3
#define C_4         0x2044 // 0x4
#define C_8         0x2045 // 0x8
#define C_10        0x2046 // 0x10
#define C_20        0x2047 // 0x20
#define C_100       0x2048 // 0x100
#define C_10000     0x2049 // 0x10000
#define C_80000     0x204a // 0x80000
#define C_10000000  0x204b // 0x10000000
#define C_20000000  0x204c // 0x20000000
#define C_40000000  0x204d // 0x40000000
#define C_80000000  0x204e // 0x80000000
#define C_7fffffff  0x204f // 0x7fffffff
#define C_ffffffff  0x2050 // 0xffffffff
#define C_fffffffe  0x2051 // 0xfffffffe
#define C_c0000000  0x2052 // 0xc0000000
#define C_4f1bbcdc  0x2053 // 0x4f1bbcdc
#define C_5a7ef9db  0x2054 // 0x5a7ef9db
#define C_00100000  0x2055 // 0x00100000

#define ACCUM   	0x2056 // ACCUM, accumulator
#define CCR 		0x2057 // CCR, condition code register
#define NOISE1  	0x2058 // noise source
#define NOISE2  	0x2059 // noise source
#define IRQREG  	0x205A // IRQREG, set the MSB to generate a FX interrupt
#define DBAC    	0x205B 

#define C_1F    	0x206B // on 10k2 only

#define DSP_IN(i) 	(0x2200+(i))  // 10k2: 0x40+; 10k1: 0x10+

#define DSP_OUT(i) 	(0x2300+(i))  // 10k2: 0x60+; 10k1: 0x20+

#define DSP_FX(i) 	(0x2400+(i))  // FXBus: 10k2: 0x0..0x3f, 10k1: 0x0..0xf
#define DSP_FX2(i) 	(0x2500+(i))  // Rec FXBus: 0x30..0x3f for 10k1, 0x80.. for 10k2

#define DSP_E32IN(i) 	(0x2600+(i))	// Emu32 inputs
#define DSP_E32OUT(i) 	(0x2700+(i))	// Emu32 outputs

#define DSP_REGLAST  0x27ff

#define MAX_GPR_NAME 12

struct dsp_register_info
{
 char name[MAX_GPR_NAME];

 word num;
 byte type;
 #define GPR_STATIC 	0x1     // a register with or without initial value
 #define GPR_TEMP   	0x3     // register is not saved between samples [shared]
 #define GPR_CONTROL    0x4     // a kind of static
 #define GPR_CONST  	0x5     // constant [shared]
 #define GPR_INPUT  	0x7
 #define GPR_OUTPUT 	0x8
 #define GPR_ITRAM  	0x9
 #define GPR_XTRAM  	0xa
 #define GPR_TRAMA  	0xb
 #define GPR_INTR   	0xf
 #define GPR_MASK   	0xf

 #define TRAM_READ  	0x20
 #define TRAM_WRITE 	0x40

 word translated;
 #define DSP_REG_NOT_TRANSLATED 0xffff

 dword p;
};

struct dsp_code
{
 byte op;
 word r,a,x,y;
};

#ifndef LIST_T_
 #define LIST_T_
 struct list 
 {
    union
    {
     struct list *next;
     __int64 next_padding;
    };

    union
    {
     struct list *prev;
     __int64 prev_padding;
    };
 };
#endif

#if !defined(DSP_MAX_STRING)
	#if defined(KX_MAX_STRING)
		#define DSP_MAX_STRING	KX_MAX_STRING
	#else
    	#define DSP_MAX_STRING 128
    #endif
#endif

struct dsp_microcode
{
 struct list list;

 int pgm;

 char name[DSP_MAX_STRING];
 char copyright[DSP_MAX_STRING];
 char engine[DSP_MAX_STRING];
 char comment[DSP_MAX_STRING];
 char created[DSP_MAX_STRING];
 char guid[DSP_MAX_STRING];

 dword flag;
 #define MICROCODE_TRANSLATED   1
 #define MICROCODE_ENABLED  	2
 #define MICROCODE_BYPASS   	4

 // kX DSP sub-flags
 #define MICROCODE_HIDDEN   	0x80000000  // hide from kX DSP applet
 #define MICROCODE_OPENED   	0x40000000  // microcode applet window is currently opened
 #define MICROCODE_MINIMIZED    0x20000000  // applet window is displayed as a header only

 // all resources
 int offset;
 #define DSP_MICROCODE_NOT_TRANSLATED   -1

 int itramsize;
 int xtramsize;

 int itram_start;
 int xtram_start;

 union
 {
    dsp_code *code;                // not defined for kx_get_microcode()
    __int64 code_padding;
 };

 int code_size;

 union
 {
    dsp_register_info *info;       // not defined for kx_get_microcode()
    __int64 info_padding;
 };

 int info_size;
};

struct dspconnections
{
 int pgm_id;    // destination microcode id; =-1 for physical;
 word num;  	// destination microcode register number; or '.translated' for physical
 word this_num; // requested microcode register number
};

#define MAX_PGM_NUMBER  0x100       // internal driver limitation; no more than 255 pgms

// 10kX DSP internal structure and constants:

#define E10K2_MAX_INSTRUCTIONS	1024
#define E10K1_MAX_INSTRUCTIONS	512

#define E10K1_MICROCODE_BASE    0x400       // microcode base address
#define E10K2_MICROCODE_BASE    0x600

// instruction consists of two dwords:
#define E10K1_X_MASK_LOW        0x000ffc00  
#define E10K1_Y_MASK_LOW        0x000003ff  
#define E10K1_OP_MASK_HI        0x00f00000  
#define E10K1_R_MASK_HI         0x000ffc00  
#define E10K1_A_MASK_HI         0x000003ff  

#define E10K1_OP_SHIFT_LOW      20
#define E10K1_OP_SHIFT_HI       10

#define E10K2_Y_MASK_LOW        0x000007ff      
#define E10K2_X_MASK_LOW        0x007ff000
#define E10K2_OP_MASK_HI        0x0f000000
#define E10K2_R_MASK_HI         0x007ff000
#define E10K2_A_MASK_HI         0x000007ff

#define E10K2_OP_SHIFT_LOW      24
#define E10K2_OP_SHIFT_HI       12

enum dsp_bypass_e
{
	DSP_BYPASS_NONE=0,
    DSP_BYPASS_EXT_LOOPBACK=1,
	DSP_BYPASS_OUT_CONST=2,
	DSP_BYPASS_IN_CONST=4,
	DSP_BYPASS_INT_LOOPBACK=8
};

#pragma code_seg()
class DSP
{
    enum { object_magic=0xdcbdcb88 };
	int magic;

	Hal *hal;
	bool is_10k2;
	bool is_10k8;

	int first_instruction;
	int high_operand_shift;
	int opcode_shift;
	int microcode_size;

	enum dsp_bypass_e bypass_mode;

	public:
		DSP();
		~DSP();

        int init(Hal *hal_);
        int close(void);
        int start(void);
        int stop(void);
        int reset(int new_sampling_rate);
        int clear(void);	// assumes stop()

        void set_bypass(dsp_bypass_e bypass_mode_)		{ bypass_mode=bypass_mode_; };
        dsp_bypass_e get_bypass(void)					{ return bypass_mode; };

        // DirectSound/WDM/WinMM volumes
        static const LONG MAX_VOLUME=0x0;			//    0 dB
        static const LONG MIN_VOLUME=-144*0x10000;	// -144 dB
        static const LONG VOLUME_STEP=0x8000;		//  0.5 dB
        // DSP volumes
        static const dword DSP_MAX_VOLUME=0x80000000;

        void hwOP(dword at, dword op, dword z, dword w, dword x, dword y);		// every value is physical, card-specific

        // translate DSP_* constants into physical values:
        inline dword dsp_const(dword var)   { return var-DSP_CONST+(is_10k2?0x80:0); 			};	// for 10k2 constants start from 0xC0
        inline dword dsp_in(dword var)		{ return var-DSP_IN(0)+(is_10k2?0x40:0x10);		};
        inline dword dsp_out(dword var)		{ return var-DSP_OUT(0)+(is_10k2?0x60:0x20);		};
        inline dword dsp_fx(dword var)		{ return var-DSP_FX(0);									};
        inline dword dsp_fx2(dword var)	 	{ return var-DSP_FX2(0)+(is_10k2?0x80:0x30);		};
        inline dword dsp_e32_in(dword var)  { return var-DSP_E32IN(0)+(is_10k8?0x160:0x50);  	};
        inline dword dsp_e32_out(dword var) { return var-DSP_E32OUT(0)+(is_10k8?0x1e0:(var<DSP_E32OUT(0x10)?0xb0:0x90)); 		};
        inline dword gpr(dword var)			{ return var+(is_10k2?E10K2_GPR_BASE:E10K1_GPR_BASE); };

        inline dword r(dword var)
        		{
        			if(var>=DSP_CONST && var<DSP_IN(0)) return dsp_const(var);
        			else if(var>=DSP_IN(0) && var<DSP_OUT(0)) return dsp_in(var);
        			else if(var>=DSP_OUT(0) && var<DSP_FX(0)) return dsp_out(var);
        			else if(var>=DSP_FX(0) && var<DSP_FX2(0)) return dsp_fx(var);
        			else if(var>=DSP_FX2(0) && var<DSP_E32IN(0)) return dsp_fx2(var);
        			else if(var>=DSP_E32IN(0) && var<DSP_E32OUT(0)) return dsp_e32_in(var);
        			else if(var>=DSP_E32OUT(0) && var<DSP_REGLAST) return dsp_e32_out(var);
        			else return var;
        		};

        bool verify_magic(void) { return (magic==object_magic); };

        void apply_volume(LONG left,LONG right);	// in dB/0x10000 units
        dword calc_volume(LONG value);
};
