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

#include <stdarg.h>
#include <stdio.h>

#pragma code_seg()

#if DBG
void debug(const char *__format, ... )
{
    if(!*drv_debug_level)
     return;

    char my_internal_buf[2048];
    va_list ap;
    va_start(ap, __format);
    
    if(/*_vsnprintf*/RtlStringCbVPrintfA(my_internal_buf, sizeof(my_internal_buf), __format, ap)==STATUS_SUCCESS)
    {
     if((*drv_debug_level==2) ||
        ( (*drv_debug_level==1) && strstr(my_internal_buf,"!!")) )
     {
      DbgPrint(my_internal_buf);
     }
    } else DbgPrint("!!! too long debug string\n");
    va_end(ap);
}
#endif
