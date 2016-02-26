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
#include "pcdriver.h"
#include "pcdevice.h"
#include "guids.h"

#include "adapter.h"

#include "hal.h"
#include "util.h"

#pragma code_seg()
NTSTATUS Adapter::InterruptServiceRoutine(IN PINTERRUPTSYNC  InterruptSync,IN PVOID DynamicContext)
{
    ASSERT(InterruptSync);
    ASSERT(DynamicContext);

    Adapter *that = (Adapter *) DynamicContext;
    if(!valid_object(that,Adapter))
    {
      debug("!! Adapter::Interrupt: Null Dynamic context in adapter::ISR\n");
      return STATUS_UNSUCCESSFUL;
    }

    int ret=that->hal->interrupt_critical();
    if(ret<0)	// not our IRQ?
    	return STATUS_UNSUCCESSFUL;
    else
    	if(ret>0) // still need dpc processing?
           KeInsertQueueDpc(&that->dpc,0,0); // this will call hal->interrupt_deferred() via Adapter::dpc_func()
        // else ret==0 - processed, no DPC needed
           
    return STATUS_SUCCESS;
}

#pragma code_seg()
VOID Adapter::dpc_func(IN PKDPC /*Dpc*/,IN PVOID DeferredContext,IN PVOID /*SystemArgument1*/,IN PVOID /*SystemArgument2*/)
{
	// runs at DISPATCH_LEVEL

	Adapter *that=(Adapter *)DeferredContext;

	if(valid_object(that,Adapter))
	{
		that->hal->interrupt_deferred();
	}
    else
    	debug("!! Adapter::dpc_func: invalid context\n");
}

struct sync_context_t
{
	Hal *that;
	sync_func_e func;
	void *p;
};

#pragma code_seg()
BOOLEAN sync_func(void *context_)
{
	// executed by KeSynchronizeExecution usually at DIRQ level
	// with Hal::hw_lock or Hal::settings_mutex acquired

	sync_context_t *context=(sync_context_t *)context_;

	if(context && valid_object(context->that,Hal))
	{
		switch(context->func)
		{
			case SYNC_ADD_NOTIFICATION:
				context->that->add_notification_protected((BufferNotification *)context->p);
				break;
			case SYNC_REMOVE_NOTIFICATION:
				context->that->remove_notification_protected((BufferNotification *)context->p);
				break;
			default:
				debug("!! sync_func: invalid func\n");
		}
	}
	else
		debug("!! sync_func: invalid context\n");

	return TRUE;
}


#pragma code_seg()
void Hal::arrange_sync_call(sync_func_e func,void *p)
{
	// executed at DPC_LEVEL or DISPATCH_LEVEL,
	// usually with Hal::hw_lock or Hal::settings_mutex acquired
	Adapter *that=(Adapter *)owner;

	if(valid_object(that,Adapter) && that->InterruptSync)
	{
		sync_context_t *context = (sync_context_t *)ExAllocatePoolWithTag(NonPagedPool,sizeof(sync_context_t),'cntx');
		if(context)
		{
			context->that=this;
			context->func=func;
			context->p=p;

			KeSynchronizeExecution(that->InterruptSync->GetKInterrupt(),sync_func,context);
			ExFreePoolWithTag(context,'cntx');
		}
		else
			debug("!! Hal::arrange_sync_func: failed to allocate memory\n");
	}
	else
		debug("!! Hal::arrange_sync_start: invalid adapter object\n");
}

#pragma code_seg()
int Hal::interrupt_critical(void)
{
	dword ipr=inpd(port + IPR);
	bool need_signal=false;

    if(ipr==0)
     return -1; // not our IRQ

    timing.ping();

    // clear =all= IPR bits
    outpd(port + IPR,ipr);

    // processed DIRQ interrupts:

    bool need_to_clear_buffer=false;

    if(ipr&IPR_EFXBUFFULL)
    {
    	// FXIDX is 0 here
    	ipr&=(~IPR_EFXBUFFULL);

    	need_to_clear_buffer=true;

        struct list *pos;

        if(is_streaming)
	    for_each_list_entry(pos,&buffer_list)
	    {
	    	BufferNotification *buff = list_item(pos, BufferNotification, list);
	    	PKEVENT e=NULL;

	    	if(buff && buff->enabled)
	    	{
	    		if(buff->is_recording)
	    			e=buff->process_rec_buffers(rec_buffer_1,total_hw_rec_voices,hw_full_buffer_in_samples>>1);
	    		else
	    		{
	    			e=buff->process_pb_buffers(pb_buffers_1,total_hw_pb_voices,hw_full_buffer_in_samples>>1);
	    			is_buffer_cleared=false;
	    			need_to_clear_buffer=false;
	    		}
	    	}

	    	if(e)
	    	{
	    		for(int i=0;i<MAX_EVENTS;i++)
	    		{
	    			// check whether the item is NULL (empty), store event if true

	    			#if defined(_WIN64)
	    				if(!InterlockedCompareExchange64((LONG64 *)&events[i],(LONG64)e,NULL))
	    			#else
	    				if(!InterlockedCompareExchange((LONG *)&events[i],(LONG)e,NULL))
	    			#endif

	    				break;
	    		}
	    		need_signal=true;
	    	}
	    }
    }
    if(ipr&IPR_EFXBUFHALFFULL)
    {
    	// FXIDX is buffer_size/2 here
    	ipr&=(~IPR_EFXBUFHALFFULL);

    	need_to_clear_buffer=true;

        struct list *pos;

        if(is_streaming)
	    for_each_list_entry(pos,&buffer_list)
	    {
	    	BufferNotification *buff = list_item(pos, BufferNotification, list);
	    	PKEVENT e=NULL;

	    	if(buff && buff->enabled)
	    	{
	    		if(buff->is_recording)
	    			e=buff->process_rec_buffers(rec_buffer_0,total_hw_rec_voices,hw_full_buffer_in_samples>>1);
	    		else
	    		{
	    			e=buff->process_pb_buffers(pb_buffers_0,total_hw_pb_voices,hw_full_buffer_in_samples>>1);
	    			is_buffer_cleared=false;
	    			need_to_clear_buffer=false;
	    		}
	    	}

	    	if(e)
	    	{
	    		for(int i=0;i<MAX_EVENTS;i++)
	    		{
	    			// check whether the item is NULL (empty), store event if true

	    			#if defined(_WIN64)
	    				if(!InterlockedCompareExchange64((LONG64 *)&events[i],(LONG64)e,NULL))
	    			#else
	    				if(!InterlockedCompareExchange((LONG *)&events[i],(LONG)e,NULL))
	    			#endif

	    				break;
	    		}

	    		need_signal=true;
	    	}
	    }
    }

    if(need_to_clear_buffer && !is_buffer_cleared)
    {
    	// no clients are attached - need to empty playback buffers now
    	ipr|=IPR_MUTE;	// special software interrupt passed to DPC

    	need_to_clear_buffer=false;
    	is_buffer_cleared=true;
    }

    if(need_signal)
    {
    	// send event
    	ipr|=IPR_FORCEINT;	// special software interrupt passed to DPC
    }

    /*
    	// template
        if(ipr&IPR_...)
        {
        	// do something...
        	ipr&=(~IPR_...);
        }
    */

    // we should clear the particular INTE bit to calm down the chip...
    // we have some time to process the DPC (depending on the FIFO size)

    dword mask=0;

    if(ipr&IPR_MIDIRECVBUFEMPTY)
    	mask|=INTE_MIDIRXENABLE;
    if(ipr&IPR_MIDITRANSBUFEMPTY)
    	mask|=INTE_MIDITXENABLE;
    if(ipr&IPR_K2_MIDIRECVBUFEMPTY)
    	mask|=INTE_K2_MIDIRXENABLE;
    if(ipr&IPR_K2_MIDITRANSBUFEMPTY)
    	mask|=INTE_K2_MIDITXENABLE;

    if(mask)
    {
    	// warning: this is unsafe: need to protect irq_enable/irq_disable with KeSynchronizeExecution (arrange_sync_call)
    	// since we don't use MIDI, no need to bother at the moment
    	// warning: MIDI will lock up if interrupts are not cleared
    	debug("!! Hal::interrupt_critical: should never happen [%x]\n",mask);
    	outpd(port+INTE,inpd(port + INTE)&(~mask));
    }

    #if DBG
     timing.print(1000,"Hal::interrupt_critical: typical time: %dmS\n");
    #endif

    // unprocessed IRQs will be passed to deferred routine
    if(ipr)
    {
    	ipr_value._or((LONG)ipr);
    	return (int)ipr;
    }

    return 0;
}

#pragma code_seg()
void Hal::interrupt_deferred(void)
{
	// runs at DISPATCH_LEVEL

	int retry=0;

	while(ipr_value.get())
	{
		// test each bit we wish to try
		if(ipr_value.test_and_reset(22)) // bit 22: IPR_FORCEINT
		{
			PKEVENT e=NULL;
      		for(int i=0;i<MAX_EVENTS;i++)
      		{
      			#if defined(_WIN64)
      				e=(PKEVENT)InterlockedExchange64((LONG64 *)&events[i],NULL);
      			#else
      				e=(PKEVENT)InterlockedExchange((LONG *)&events[i],NULL);
      			#endif

      			if(e)
                {
                	KeSetEvent(e,0,FALSE);
      			}
      		}
		
			retry=1;
		}

		if(ipr_value.test_and_reset(18)) // bit 18: IPR_MUTE
		{
			debug("Hal::interrupt_deferred: Buffer emptied\n");
			hw_buffer.voice_buffer.clear();
			retry=1;
		}

		if(ipr_value.test_and_reset(9)) // bit 9: IPR_INTERVALTIMER
		{
			// do something
			retry=1;
		}

		if(retry) { retry=0; continue; }

		// no interrupts processed this time:
        if(ipr_value.get())
        {
        	debug("!! Hal::deferred: ipr still non-zero [%x]\n",ipr_value.get());
        	ipr_value._and(~ipr_value.get());
        	break;
        }
	}
}
