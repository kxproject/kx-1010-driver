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

typedef unsigned long 	dword;
typedef unsigned short 	word;
typedef unsigned char 	byte;

#if !defined(InterlockedOr)
    FORCEINLINE LONG InterlockedAnd (
        __inout __drv_interlocked LONG volatile *Target,
        __in LONG Set
        )
    {
        LONG i;
        LONG j;

        j = *Target;
        do {
            i = j;
            j = InterlockedCompareExchange(Target,
                                           i & Set,
                                           i);

        } while (i != j);

        return j;
    }

    FORCEINLINE LONG InterlockedOr (
        __inout __drv_interlocked LONG volatile *Target,
        __in LONG Set
        )
    {
        LONG i;
        LONG j;

        j = *Target;
        do {
            i = j;
            j = InterlockedCompareExchange(Target,
                                           i | Set,
                                           i);

        } while (i != j);

        return j;
    }

#endif


extern "C" NTKERNELAPI PHYSICAL_ADDRESS MmGetPhysicalAddress (__in PVOID BaseAddress);

#pragma code_seg()
struct memory_handle_t
{
	memory_handle_t() { size=0; addr=NULL; physical=0; };

	size_t  size;
	void    *addr;		// virtual
	dword   physical;	// physical	(we are using 32-bit, since E-DSP can only address 2Gb of RAM anyway)

	static size_t pool_size;

	__int64 get_physical(size_t offset) { if(!addr) return 0; PHYSICAL_ADDRESS tmp=MmGetPhysicalAddress(((byte *)addr)+offset); return tmp.QuadPart; };
	int		alloc_contiguous(size_t size);
	void	free_contiguous(void);
	void	clear(void)					{ memset(addr,0,size); };
};

typedef KLOCK_QUEUE_HANDLE lock_instance;

#pragma code_seg()
class lock
{
	KSPIN_LOCK spin_lock;

	public:
		inline void init(void) 									{ KeInitializeSpinLock(&spin_lock); }
		inline void acquire(lock_instance *lock_handle)			{ KeAcquireInStackQueuedSpinLock(&spin_lock,lock_handle);  	 		}
		inline void release(lock_instance *lock_handle)			{ KeReleaseInStackQueuedSpinLock(lock_handle);							}

		inline void acquire_at_dpc(lock_instance *lock_handle) 	{ KeAcquireInStackQueuedSpinLockAtDpcLevel(&spin_lock,lock_handle); 	}
		inline void release_at_dpc(lock_instance *lock_handle) 	{ KeReleaseInStackQueuedSpinLockFromDpcLevel(lock_handle);	}
};

#pragma code_seg()
class mutex
{
	KMUTEX kmutex;

	public:
		void init(void)						{ KeInitializeMutex(&kmutex,0);													}
		void release(void)					{ KeReleaseMutex(&kmutex,FALSE);												}
		bool acquire(void)					{ return !!KeWaitForSingleObject(&kmutex,Executive,KernelMode,FALSE,NULL);		}
};

#pragma code_seg()
class atomic
{
	volatile LONG value;

	public:
		atomic()								{ value=0; };

		inline bool test_and_reset(LONG bit)	{ return !!InterlockedBitTestAndReset(&value,bit);		};
		inline void _or(LONG mask)				{ InterlockedOr(&value,mask);							};
		inline void _and(LONG mask)				{ InterlockedAnd(&value,mask); 							};
		inline LONG get(void)					{ return value; 										};
};
		


#if DBG
	// timer
	#pragma code_seg()
	class kTimer
	{
	    private:
	     	LARGE_INTEGER res;
	     	LARGE_INTEGER prev;
	     	int disabled;
	     	int cnt;
	
	    public:
	     	kTimer();
	     	void enable(void) { disabled=0; }
	     	void disable(void) { disabled=1; }
	     	void print(int resolution,char *txt);
	     	void warn(int min_,int max_,char *warn);
	
	     	ULONGLONG time(void);
	     	ULONGLONG ping(void);
	};
#else
	// timer
	#pragma code_seg()
	class kTimer
	{
	    public:
	     	kTimer() {};
	     	inline void enable(void) {};
	     	inline void disable(void) {};
	     	inline void print(int ,char *) { ; }
	     	inline void warn(int ,int ,char *) { ; }
	
	     	inline ULONGLONG time(void) { return 0; }
	     	inline ULONGLONG ping(void) { return 0; }
	};
#endif
