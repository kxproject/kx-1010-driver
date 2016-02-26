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
#include "hwbuffer.h"

#define n_pages(a) (((a)/HW_PAGE_SIZE)+(((a)%HW_PAGE_SIZE)?1:0))

#pragma code_seg()
int HwBuffer::alloc(dword size,dword physical)
{
	int index = 0;

	word numpages = (word)n_pages(size);

	while(index < MAXPAGES-1) 
	{
		if(pagetable[index].usage) 
		{
			if((pagetable[index].physical==physical) && (pagetable[index].n_pages==numpages))
			{
				// re-use:
				pagetable[index].usage++;

				return index;
			}

			// block is occupied
		} 
		 else
		{
			// found free
			if(pagetable[index].n_pages >= numpages) 
			{
				// Block size is OK
				// if free block is larger than the block requested
                                // - resize the remaining block
				if(pagetable[index].n_pages > numpages)
					pagetable[index + numpages].n_pages = pagetable[index].n_pages - numpages;
                    // pagetable[index + numpages].usage should be '0'!

				pagetable[index].n_pages = (word)numpages;

				pagetable[index].physical=physical;
				pagetable[index].usage=1;

				return index;
			}

			// too small -- skip this block
		}
		index += pagetable[index].n_pages;
	}

	return -1;
}

#pragma code_seg()
void HwBuffer::free(int index)
{
	if(index<0)
	 return;

	if(pagetable[index].usage) 
	{
		pagetable[index].usage--;
		if(pagetable[index].usage==0)
		{
        		// allocated: mark as free -- already done by 'usage--;'
        		word prev_size=pagetable[index].n_pages;
        		pagetable[index].physical=0;

        		// if the next block is free, too, concatenate the blocks
        		if(!(pagetable[index + prev_size].usage))
        			pagetable[index].n_pages += pagetable[index + prev_size].n_pages;
        }
	}
	else
		debug("!! HwBuffer::free: invalid index %d\n",index);
}

#pragma code_seg()
HwBuffer::HwBuffer()
{
	magic=object_magic;
	page_index=-1;

	memset(&pagetable,0,sizeof(pagetable));

    // first block: used; size=1
	pagetable[0].n_pages = 1;
	pagetable[0].usage = 1;

    // second block: unused; size=rest
    pagetable[1].n_pages = MAXPAGES - 1;
    pagetable[1].usage = 0;

    virtual_pagetable.alloc_contiguous(MAXPAGES * sizeof(dword));
    tram.alloc_contiguous(256*1024);
    silent_page.alloc_contiguous(HW_PAGE_SIZE);

    mtr_buffer.alloc_contiguous(65536); // 24/32-bit, stereo, x2 for 96, x2 for 192kHz ==> 65536 max => 2048 samples
    voice_buffer.alloc_contiguous(65536); // 24/32-bit, stereo, x2 for 96, x2 for 192kHz

    {
    	silent_all_pages();

    	int pages=n_pages(voice_buffer.size);
        
        page_index = alloc(pages * HW_PAGE_SIZE, voice_buffer.physical);

        if(page_index<0)
        {
            	debug("!! HwBuffer::alloc_buffer failed [size=%d] (addx failed)\n",voice_buffer.size);
        }
        else
    	{
    	  for(int i=0;i<pages;i++)
    	  {
	    	  	__int64 a=voice_buffer.get_physical(i*HW_PAGE_SIZE);
    			dword physical=(dword)a;
    			((dword *) virtual_pagetable.addr)[page_index+i] = (physical << 1) | (page_index+i);

    			// debug("hw_buffer: pagetable[%d] = %x\n",page_index+i,((dword *) virtual_pagetable.addr)[page_index+i]);
    	  }

    	  debug("hwbuffer: page_index: %d, pages: %d, usage: %d\n",page_index,pages,pagetable[page_index].usage);
    	}
    }


    debug("hwbuffer: allocated: pt: %8x tram: %8x silent: %8x mtr: %8x voice: %8x\n",
    	virtual_pagetable.physical,tram.physical,silent_page.physical,mtr_buffer.physical,voice_buffer.physical);
}

#pragma code_seg()
HwBuffer::~HwBuffer()
{
	if(page_index!=-1)
	{
    	int pages=n_pages(voice_buffer.size);

    	if(pagetable[page_index].usage==1) // last one
    	{
            	for(int i=0;i<pages;i++) 
            	{
            			((dword *) virtual_pagetable.addr)[page_index+i] = (silent_page.physical << 1) | (page_index+i);
            	}
        }

    	free(page_index);
    	page_index = -1;
		
	}

    virtual_pagetable.free_contiguous();
    tram.free_contiguous();
    silent_page.free_contiguous();
    mtr_buffer.free_contiguous();
    voice_buffer.free_contiguous();

    debug("hwbuffer: freed hardware buffers\n");

    magic=0;
}


#pragma code_seg()
void HwBuffer::silent_all_pages(void)
{
	for(int i = 0; i < MAXPAGES; i++)
    	((dword *) virtual_pagetable.addr)[i] = (silent_page.physical<<1) | i;
}

#pragma code_seg()
int memory_handle_t::alloc_contiguous(size_t size_)
{
    PHYSICAL_ADDRESS highest,lowest,boundary;

    highest.LowPart=0x7fffffff; // 2Gb limit
    highest.HighPart=0x0;

    lowest.LowPart=0x0;
    lowest.HighPart=0x0;

    boundary.LowPart=0;
    boundary.HighPart=0;

    size=size_;

    addr=MmAllocateContiguousMemorySpecifyCache(size,lowest,highest,boundary,DRIVER_MEM_CACHE);

    if(addr)
    {
    	physical=(dword)(MmGetPhysicalAddress((void *)addr)).LowPart;
     	RtlZeroMemory(addr,size);
     	pool_size+=size;
     	return 0;
    } 
    else 
    {
	    addr=0;
     	physical=0;
     	size=0;
     	return STATUS_INSUFFICIENT_RESOURCES;
    }
}

#pragma code_seg()
void memory_handle_t::free_contiguous(void)
{
	if(addr)
	{
		MmFreeContiguousMemory(addr);
		pool_size-=size;

		addr=0;
		physical=0;
		size=0;
	}
}

#pragma data_seg()
size_t memory_handle_t::pool_size=0;
