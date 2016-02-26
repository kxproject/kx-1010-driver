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

extern "C"
{
	byte __inbyte (word Port);
	word __inword (word Port);
	dword __indword (word Port);
	void __outbyte (word Port,byte Data);
	void __outword (word Port,word Data);
	void __outdword (word Port,dword Data);
};

#pragma intrinsic(__inbyte)
#pragma intrinsic(__inword)
#pragma intrinsic(__indword)
#pragma intrinsic(__outbyte)
#pragma intrinsic(__outword)
#pragma intrinsic(__outdword)

inline byte inp(dword __port)
{
        return __inbyte((word)__port);
}

inline word inpw(dword __port)
{
        return __inword((word)__port);
}
inline dword inpd(dword __port)
{
        return __indword((word)__port);
}

inline void outp(dword __port, byte value)
{
        __outbyte((word)__port, value);
}

inline void outpw(dword __port,word value)
{
        __outword((word)__port, value);
}

inline void outpd(dword __port, dword value)
{
        __outdword((word)__port, value);
}
