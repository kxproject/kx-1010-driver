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

#pragma code_seg()
class HwBuffer
{
    enum { object_magic=0xc3fcce88 };
	int magic;

	enum { HW_PAGE_SIZE=4096, MAXPAGES=8192 };

    struct pagetable
    {
        word n_pages;  	// block size in pages
        word usage; 	// number of block uses
        dword physical;	// address
    }pagetable[MAXPAGES];

    int alloc(dword size,dword physical);
    void free(int index);

	public:
		HwBuffer();
		~HwBuffer();

		void silent_all_pages(void);

        memory_handle_t		virtual_pagetable;
        memory_handle_t		tram;
        memory_handle_t		silent_page;
        memory_handle_t		mtr_buffer;
        memory_handle_t		voice_buffer;

        int page_index;

        bool verify_magic(void) { return (magic==object_magic); };
};
