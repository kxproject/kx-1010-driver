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

#if DBG

#pragma code_seg()
kTimer::kTimer()
{
 res.QuadPart=0;
 prev.QuadPart=0;
 disabled=1;
 cnt=1;
}

#pragma code_seg()
ULONGLONG kTimer::ping(void)
{
 prev=KeQueryPerformanceCounter(&res);

 return prev.QuadPart*1000000LL/res.QuadPart;
}

#pragma code_seg()
ULONGLONG kTimer::time(void)
{
 if(disabled)
  return 0;

 LARGE_INTEGER cur;
 
 cur=KeQueryPerformanceCounter(&res);

 ULONGLONG ret=cur.QuadPart-prev.QuadPart;
 prev=cur;

 ret=ret*1000000LL/res.QuadPart;

 return ret;
}

#pragma code_seg()
void kTimer::print(int resolution, char *txt)
{
 if(!disabled)
 {
   // int res=(int)(time()/10);

   int result=(int)time();

   if(--cnt==0)
   {
    if(result!=0)
     debug(txt,result);

    cnt=resolution;
   }
 }
}

#pragma code_seg()
void kTimer::warn(int min_,int max_,char *warn)
{
 if(!disabled)
 {
   // int res=(int)(time()/10);

   int result=(int)time();

   if(result>max_ || result<min_)
   {
    debug(warn,result);
   }
 }
}

#endif // DBG
