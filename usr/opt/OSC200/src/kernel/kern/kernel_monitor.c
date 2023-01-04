/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: kernel_monitor.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/08 17:54:26 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 *	File:	kern/kernel_monitor.c
 *	Author:	Ted Lehr
 *
 *	Copyright (C) 1987, Ted Lehr
 *
 * 	Kernel Monitoring system calls.
 */

#include <mach/boolean.h>
#include <sys/types.h>
#include <kern/thread.h>
#include <kern/lock.h>
#include <mach/machine.h>
#include <kern/zalloc.h>
#include <kern/parallel.h>
#include <machine/machparam.h>
#include <machine/cpu.h>
#include <kern/kern_port.h>
#include <kern/kern_mon.h>
#include <mach/vm_param.h>

struct zone *monitor_zone;

queue_head_t	 	all_monitors;		/* queue of all monitors  */
decl_simple_lock_data(,	all_monitors_lock)	/* lock for monitor queue */


/****************************************************************************
 *	Routine:	monitor_old_thread()
 *
 *	Function:  	Context switch sensor for old thread.  
 *
 *        A flag is set to indicate that the new_thread will probably
 *        have to be checked.  The macro tests old_thread to see if it
 *        has a pointer to a monitor.  
 *
 *        If yes, then first a local pointer is set equal to that monitor 
 *        object.  The sensor checks to see where the monitor's write pointer 
 *        for the buffer of this cpu is pointing and branches to the 
 *        appropriate code.  Now a test is performed to determine whether the 
 *        buffer is full.  If it is, the monitor's read pointer for this cpus
 *        buffer is incremented in order to keep the buffer up-to-date even 
 *        though an event will be lost.  In addition, a warning is issued 
 *        indicating that the buffer overflowed.  The local_event pointer 
 *        is set equal to the next address pointed to by the global write 
 *        pointer.  Then the events are stored.  
 *
 *        Before a check is made to see whether new_thread has the same 
 *	  monitor associated with it, we check to see whether new_thread is 
 *        real thread.  If yes to both of these checks, the thread id is 
 *	  stored in the buffer and check_new_thread is set to MONITOR_NO and 
 *        returned.  Finally the new value of the global write pointer is set.
 *
 *
 *    NOTE-0:
 *         It is not guaranteed that the data collected in a single call 
 *         will contain unaltered events if an overflow has occurred.
 *    NOTE-1:
 *        The write pointer points to the last written location.  
 *        After a buffer is read, the read pointer points to the 
 *        location immediately after the last written location.
 */
 

int
monitor_old_thread(old_thread,new_thread,thiscpu)
int			thiscpu;
register thread_t	old_thread;
thread_t		new_thread;
{    
  register kern_mon_event_t	local_event;	
  register monitor_t		ot_monitor;	
  register monitor_buffer_t	ot_buffer;
  int				check_new_thread;

  ot_monitor = old_thread->monitor_obj;
  check_new_thread = MONITOR_YES;	
  switch (ot_monitor->global_state) {
    case MONITOR_PAUSE:
    case MONITOR_OFF:
  	break;
								
    case MONITOR_SHUTDOWN:
	thread_lock(old_thread);
	if (old_thread-> monitor_obj != MONITOR_NULL) {
	    old_thread-> monitor_obj = MONITOR_NULL;
	    old_thread-> monitor_id = MONITOR_NO_ID;
	    queue_remove(&ot_monitor->monitored_threads, old_thread,
		thread_t, monitored_threads);
	    thread_unlock(old_thread);
	    monitor_deallocate(ot_monitor);
	}
	else {
	    thread_unlock(old_thread);
	}
	break;							
								
    case MONITOR_RUN:					
	/*
	 *	Find the place in the buffer for this event.
	 */
	ot_buffer = &(ot_monitor->buffer[thiscpu]);
	local_event = ot_buffer->write_ptr;
	if (local_event == ot_buffer->buffer_end)
	    local_event = ot_buffer->buffer_head;
	else
	    local_event++;

	/*
	 *	Overflow check.  This is suboptimal when the local event
	 *	has been reset to the buffer front above, but that should
	 *	be rare given reasonable buffer sizes.
	 */
	local_event->third_element = thiscpu;
		    
	if (ot_buffer->read_ptr == ot_buffer->buffer_head) {
	    if (local_event == ot_buffer->buffer_end) {
	        (ot_buffer->read_ptr)++;
	    	local_event->third_element |= MONITOR_OVFLOW;
	    }
	}							
	else {
	    /*
	     *	Overflow check.
	     */
	    if (local_event + 1 == ot_buffer->read_ptr) {
		if (local_event + 1 == ot_buffer->buffer_end) {
		    ot_buffer->read_ptr = ot_buffer->buffer_head;
		    local_event->third_element |= MONITOR_OVFLOW;
		}
		else {						
		    (ot_buffer->read_ptr)++;
		    local_event->third_element |= MONITOR_OVFLOW;
		}
	    }
	}

	/*
	 *	Now fill out the rest of the element.
	 */
	local_event->first_element = (unsigned) old_thread->monitor_id;
	if (new_thread != THREAD_NULL) {
	    if (ot_monitor == new_thread->monitor_obj) {
		local_event->second_element =
		    (unsigned) new_thread->monitor_id;
		check_new_thread = MONITOR_NO;			
	    }
	    else { 
		local_event->second_element = MONITOR_NO_ID;
	    }
	}
	else {
	    local_event->second_element = MONITOR_NO_ID;
	    check_new_thread = MONITOR_NO;			
	}
	kern_mon_store_times(local_event);	
    	ot_buffer->write_ptr = local_event;
	break;							
								
    default:							
	printf("monitor_old_thread sensor: Bad monitor state: %d, cpu %d\n",
			ot_monitor->global_state, thiscpu);	
	panic("KERNEL MONITOR:  Bad monitor state");		
	break;							
    }								
    return(check_new_thread);
}


/****************************************************************************
 *	Routine:	monitor_new_thread()
 *
 *	Function:  	Context switch sensor for new thread.  
 *
 *	  The macro tests new_thread to see if it has a pointer to a monitor.  
 *        If yes, then first a local pointer is set equal to that monitor 
 *        object.  The sensor checks to see where the monitor's write pointer 
 *        for the buffer of this cpu is pointing and branches to the 
 *        appropriate code.  Now a test is performed to determine whether the 
 *        buffer is full.  If it is, the monitor's read pointer for this cpus
 *        buffer is incremented in order to keep the buffer up-to-date even 
 *        though an event will be lost.  In addition, a warning is issued 
 *        indicating that the buffer overflowed.  The local_event pointer 
 *        is set equal to the next address pointed to by the global write 
 *        pointer.  Then the events are stored.  Finally the new value
 *        of the global write pointer is set.
 *
 *
 *    NOTE-0:
 *         It is not guaranteed that the data collected in a single call 
 *         will contain unaltered events if an overflow has occurred.
 *    NOTE-1:
 *        The write pointer points to the last written location.  
 *        After a buffer is read, the read pointer points to the 
 *        location immediately after the last written location.
 */

void
monitor_new_thread(new_thread,thiscpu)
register thread_t	new_thread;	
int		thiscpu;
{    
  register kern_mon_event_t	local_event;	
  register monitor_t		nt_monitor;
  register monitor_buffer_t	nt_buffer;

  nt_monitor = new_thread->monitor_obj;
  switch (nt_monitor->global_state) {
    case MONITOR_PAUSE:					
    case MONITOR_OFF:					
  	break;							
								
    case MONITOR_SHUTDOWN:					
	thread_lock(new_thread);
	if (new_thread-> monitor_obj != MONITOR_NULL) {
	    new_thread-> monitor_obj = MONITOR_NULL;
	    new_thread-> monitor_id = MONITOR_NO_ID;
	    queue_remove(&nt_monitor->monitored_threads, new_thread,
		thread_t, monitored_threads);
	    thread_unlock(new_thread);
	    monitor_deallocate(nt_monitor);
	}
	else {
	   thread_unlock(new_thread);
	}
	break;

    case MONITOR_RUN:
	/*
	 *	Find the place in the buffer for this event.
	 */
	nt_buffer = &(nt_monitor->buffer[thiscpu]);
	local_event = nt_buffer->write_ptr;
	if (local_event == nt_buffer->buffer_end)
	    local_event = nt_buffer->buffer_head;
	else
	    local_event++;

	/*
	 *	Overflow check.  This is suboptimal when the local event
	 *	has been reset to the buffer front above, but that should
	 *	be rare given reasonable buffer sizes.
	 */
	local_event->third_element = thiscpu;
		    
	if (nt_buffer->read_ptr == nt_buffer->buffer_head) {
	    if (local_event == nt_buffer->buffer_end) {
	        (nt_buffer->read_ptr)++;
	    	local_event->third_element |= MONITOR_OVFLOW;
	    }
	}							
	else {
	    /*
	     *	Overflow check.
	     */
	    if (local_event + 1 == nt_buffer->read_ptr) {
		if (local_event + 1 == nt_buffer->buffer_end) {
		    nt_buffer->read_ptr = nt_buffer->buffer_head;
		    local_event->third_element |= MONITOR_OVFLOW;
		}
		else {		
		    (nt_buffer->read_ptr)++;
		    local_event->third_element |= MONITOR_OVFLOW;
		}
	    }
	}

	/*
	 *	Now fill out the rest of the element.
	 */
	local_event->first_element = MONITOR_NO_ID;
	local_event->second_element = (unsigned) new_thread->monitor_id;
	kern_mon_store_times(local_event);
    	nt_buffer->write_ptr = local_event;
	break;					
						
    default:							
	printf("monitor_new_thread sensor: Bad monitor state: %d, cpu %d\n",
			nt_monitor->global_state, thiscpu);	
	panic("KERNEL MONITOR:  Bad monitor state");		
	break;							
  }
}


/****************************************************************************
 *	Routine:	monitor_init(m)
 *
 *	Function:
 *		Initializes various global monitor structures.
 *		To be placed in bsd/init_main.c (somewhere).
 */
 
void
monitor_init()
{
  monitor_zone = zinit(
		      sizeof(monitor),
		      MONITOR_MAX * sizeof(monitor),
		      MONITOR_CHUNK * sizeof(monitor),
		      "monitors");

  queue_init(&all_monitors);
  simple_lock_init(&all_monitors_lock);
}

/****************************************************************************
 *	Routine:	all_monitor_remqueue(m)
 *
 *	Function:
 *		Removes the monitor from the global all_monitors queue.
 *              Caller cannot hold a thread lock or monitor lock.
 *
 *      Callers: (perhaps incomplete)
 *              monitor_deallocate()
 */

void
all_monitor_remqueue(m)
register monitor_t	m;
{
    if (m == MONITOR_NULL)
    	return;

    simple_lock(&all_monitors_lock);
    queue_remove(&all_monitors, m, monitor_t, all_monitors);
    simple_unlock(&all_monitors_lock);
}


/****************************************************************************
 *	Routine:	all_monitor_enqueue(m)
 *
 *	Function:
 *		Adds the monitor to the global all_monitors queue.
 *              Caller cannot hold a thread lock or monitor lock.
 *
 *      Callers: (perhaps incomplete)
 *              monitor_create()
 */

void
all_monitor_enqueue(m)
register monitor_t	m;
{
    if (m == MONITOR_NULL)
    	return;

    simple_lock(&all_monitors_lock);
    queue_enter(&all_monitors, m, monitor_t, all_monitors);
    simple_unlock(&all_monitors_lock);
}

/****************************************************************************
 *	Routine:	monitor_reference(m)
 *
 *	Function:
 *		Atomically increments monitor reference counter.
 *              Caller cannot hold a thread lock.
 *
 *      Callers: (perhaps incomplete)
 *              convert_port_to_monitor()
 */

void
monitor_reference(m)
monitor_t 	m;
{
  register int     s;

  if (m == MONITOR_NULL)
      return;

  s = splsched();
  monitor_lock(m);
  m->ref_count++;
  monitor_unlock(m);
  (void) splx(s);
}


/****************************************************************************
 *	Routine:	thread_monitor_dequeue(t,m)
 *
 *	Function:
 *		Dequeues the thread from the associated monitor's queue of
 *              threads.
 *
 *      Conditions:
 *              Caller cannot hold the monitor or thread locks.  MUST be
 *              called at splsched().
 *
 *      Callers: (perhaps incomplete)
 *              thread_terminate(), thread_force_terminate()
 */

void
thread_monitor_dequeue(t,m)
register thread_t	t;
register monitor_t 	m;
{

    if (m == MONITOR_NULL)
    	return;

    if (t == THREAD_NULL)
    	return;

    monitor_lock(m);
    thread_lock(t);
    queue_remove(&m->monitored_threads, t, thread_t, monitored_threads);
    thread_unlock(t);
    monitor_unlock(m);
}


/****************************************************************************
 *	Routine:	monitor_deallocate(m)
 *
 *	Function:
 *		Atomically decrements monitor reference counter.  If the
 *		count is zero, the monitor data structures are deallocated.
 *
 *      Callers: (perhaps incomplete)
 *              monitor_terminate(), thread_unmonitor(), thread_terminate(),
 *              thread_force_terminate(), MIG interface calls, 
 */

void
monitor_deallocate(m)
register monitor_t 	m;
{
  register unsigned int          loc_count;
  register int	s;

  if (m == MONITOR_NULL)
    return;

  s = splsched();
  monitor_lock(m);
  loc_count = --(m->ref_count);
  monitor_unlock(m);
  (void) splx(s);

  if (loc_count == 0)		
    {
      all_monitor_remqueue(m);
      kmem_free((vm_map_t)kernel_map,
		(caddr_t)m->ptr_to_all_bufs,
		(vm_size_t)m->bytes_in_all_bufs);
      kmem_free((vm_map_t)kernel_map,
		(caddr_t)m->ptr_to_reader_buf,
		(vm_size_t)m->bytes_in_all_bufs);
      zfree(monitor_zone, (vm_offset_t) m);
    }					
}


/***********************************************************************
 *	Routine:	monitor_cpu_out(cpu_num)
 *
 *	Function:
 *		Modifies the state variables of monitoring to handle
 *		one less processor.  The all_monitors queue is traversed so
 *		that the processor_state of cpu_num is set to MONITOR_CPU_OFF
 *		if a monitor is running.  The MONITOR_CPU_OFF state tells the
 *		reader that that although cpu_num is dead, its monitor buffer
 *		may still contain something useful.  If a buffer state is
 *		MONITOR_CPU_OFF,  the reader sets the state of the 
 *		buffer to MONITOR_OFF before it reads.  If the buffer
 *		is not read before the store call is made, the state
 *		is set to MONITOR_OFF by the remove_monitor call.
 *
 *		If the state of a monitor is either MONITOR_SHUTDOWN or
 *		MONITOR_OFF, ignore this call since in either case, the 
 *		states of all the cpus are MONITOR_OFF no events are allowed 
 *		to be read even if they are still in the buffers.
 *
 *      Callers: (perhaps incomplete)
 *		cpu_down()
 */

void
monitor_cpu_out(cpu_num)
register int	cpu_num;
{									
    register monitor_t 	    m,next_monitor;
    register unsigned int   s; 
									
    simple_lock(&all_monitors_lock);	
    m = (monitor_t) queue_first(&all_monitors);	
    while (!queue_end(&all_monitors, (queue_entry_t) m)) {
      next_monitor = (monitor_t) queue_next(&m->all_monitors);
      s = splsched();
      monitor_lock(m);	
      /*
       *	Wait out any monitor_read call in progress.
       */
      while (m->read_flag == MONITOR_READING) {
	monitor_unlock(m);
        while (m->read_flag == MONITOR_READING);
	monitor_lock(m);
      }
      switch(m->global_state){	
	case  MONITOR_RUN:	
	      m->processor_state[cpu_num] = MONITOR_CPU_OFF;	
	      break;	
	      			
	case  MONITOR_SHUTDOWN:
	case  MONITOR_OFF:
	      break;
	     
	default:							
	      printf("monitor_cpu_out: Bad monitor state, monitor %d\n",
						m);
	      panic("KERNEL MONITOR:  Bad monitor state");
	      break;	
      }  
      monitor_unlock(m);
      (void) splx(s);
      m = next_monitor;	
    }	
    simple_unlock(&all_monitors_lock);
}

/***********************************************************************/



/***********************************************************************
 *	Routine:	monitor_cpu_in(cpu_num)
 *
 *	Function:
 *		Modifies the state variables of monitoring to handle
 *		one more processor.  The all_monitors queue is traversed so
 *		that the processor_state of cpu_num is set to MONITOR_RUN if 
 *		a monitor is running.  If the state of a monitor is either 
 *		MONITOR_SHUTDOWN or MONITOR_OFF, set the state of of cpu_num
 *		to MONITOR_OFF.  This is redundant since the processor_state 
 *		should be MONITOR_OFF in either of these monitor states.
 *
 *      Callers: (perhaps incomplete)
 * 		cpu_up()
 */

void
monitor_cpu_in(cpu_num)
register int	cpu_num;
{									
    register monitor_t 	m, next_monitor;
    register unsigned int   s; 

    simple_lock(&all_monitors_lock);					
    m = (monitor_t) queue_first(&all_monitors);			
    while (!queue_end(&all_monitors, (queue_entry_t) m)) {	
      next_monitor = (monitor_t) queue_next(&m->all_monitors);	
      s = splsched();
      monitor_lock(m);
      /*
       * Wait out any monitor_read call in progress
       */
      while (m->read_flag == MONITOR_READING) {
	monitor_unlock(m);
        while (m->read_flag == MONITOR_READING);
	monitor_lock(m);
      }
      switch(m->global_state){					
	case  MONITOR_RUN:						
	      m->processor_state[cpu_num] = MONITOR_RUN;
	      break;	
	      
	case  MONITOR_SHUTDOWN:
	case  MONITOR_OFF:
	      m->processor_state[cpu_num] = MONITOR_OFF;
	      break;	
	      
	default:	
	      printf("monitor_cpu_in: Bad monitor state, monitor %d\n",m);
	      panic("KERNEL MONITOR:  Bad monitor state");
	      break;	
      }  				
      monitor_unlock(m);
      (void) splx(s);
      m = next_monitor;	
    }		
    simple_unlock(&all_monitors_lock);
}
/***********************************************************************/



/****************************************************************************
 *	Routine:	monitor_create(this_task,new_monitor, requested_size)
 *
 *	Function:
 *		Memory for a new kernel monitor is created and initialized.  
 *		The parameter, requested_size, is the size of the monitoring 
 *		buffer in number of events.  The size of the buffer is
 *		adjusted to fill out all touched pages.  A monitor port is
 *		returned to the caller.
 *
 *      Callers: (perhaps incomplete)
 * 		the user
 */


kern_return_t
monitor_create(this_task, new_monitor, requested_size)
	 task_t			this_task;
	 monitor_t		*new_monitor;
register int 			*requested_size;
{
#ifdef 	lint
    this_task++;
#endif	lint
    register monitor_t	loc_monitor_ptr;	 /* For readability purposes */
    register monitor_buffer_t loc_buffer_ptr;	 /*   Ditto            */
    register int	allocated_size;	  	 /* Total buf size in bytes */
    register int	cpu_num; 	 	 /* index for inits	    */


    /*
     * allocate memory for a new monitor and assign a pointer to it.
     */

    *new_monitor    = (monitor_t) zalloc(monitor_zone);
		       
    loc_monitor_ptr = *new_monitor;
    /*
     * First initialize the monitor lock, then lock the monitor,
     * initialize and set  reference count to 1.
     */

    simple_lock_init(&loc_monitor_ptr->lock);

    loc_monitor_ptr->ref_count = 1;	/* reference to self */

    /*
     *  Allocate a port to the monitor by calling ipc_monitor_init().
     *  Then set the monitor state to MONITOR_PAUSE.
     */
    ipc_monitor_init(loc_monitor_ptr);
    loc_monitor_ptr->global_state = MONITOR_PAUSE;

    /*
     *  Enable the monitor port. The reference count is incremented in
     *  the ipc_monitor_enable() to indicate the port has been enabled.
     */
    ipc_monitor_enable(loc_monitor_ptr);
    
    /*
     * Begin allocating buffers for the new kernel monitor.
     * Determine how many bytes have been requested and allocated
     * enough page bytes to accomdate it.  The default allocation
     * is the smallest number of page bytes sufficient to hold
     * MONITOR_MIG_BUF_SIZE events.
     */
    if (*requested_size != 0) 
      allocated_size = round_page(*requested_size * sizeof(kern_mon_event));
    else 
      allocated_size = round_page(MONITOR_MIG_BUF_SIZE *
		sizeof(kern_mon_event));

    /*
     *  Remember the total size of all the buffers.
     */
    loc_monitor_ptr->bytes_in_all_bufs = allocated_size;    
		/* in bytes */
    *requested_size = allocated_size / sizeof(kern_mon_event);
    loc_monitor_ptr->max_events_cpu_buf = *requested_size/NCPUS;
		 /* in events */

    /*
     *  Allocate memory to ptr_to_reader_buf to hold all records 
     *  from every completely filled buffer.  Also allocate memory for
     *  all the per-cpu buffers.  Then assign the per-cpu buffers
     *  blocks in that memory.
     */
    loc_monitor_ptr->ptr_to_reader_buf = (kern_mon_event_t) 
		kmem_alloc(kernel_map,loc_monitor_ptr->bytes_in_all_bufs);
    loc_monitor_ptr->ptr_to_all_bufs = (kern_mon_event_t) 
		kmem_alloc(kernel_map,loc_monitor_ptr->bytes_in_all_bufs);
    loc_buffer_ptr = loc_monitor_ptr->buffer;
    for (cpu_num = 0 ; cpu_num < NCPUS; cpu_num++, loc_buffer_ptr++)
    {
      	loc_buffer_ptr->buffer_head =  loc_monitor_ptr->ptr_to_all_bufs +
			cpu_num * (loc_monitor_ptr->max_events_cpu_buf);
	loc_buffer_ptr->buffer_end = loc_buffer_ptr->buffer_head +
			loc_monitor_ptr->max_events_cpu_buf - 1;
 	/*
	 * Set the inital read and write pointers values.  
	 */
	loc_buffer_ptr->write_ptr = loc_buffer_ptr->buffer_head;
	loc_buffer_ptr->read_ptr = loc_buffer_ptr->buffer_head + 1;
    }

    /*
     *  Initialize the overflow flag.
     */
    loc_monitor_ptr->read_flag = MONITOR_NOT_READING;

    /*
     * Also set the appropriate cpu states indicating a status of
     * this kernel monitor.  These states are used by the read_monitor
     * call to determine which processors have active buffers.
     */
    for (cpu_num = 0; cpu_num < NCPUS; cpu_num++){
      if ((machine_slot[cpu_num].is_cpu)  &&
          (machine_slot[cpu_num].running)    ){
	loc_monitor_ptr->processor_state[cpu_num] = MONITOR_RUN;
      } else {
	/*
	 *  Otherwise, set flags indicating kernel monitoring is 
	 *  not on these cpus.
	 */
	loc_monitor_ptr->processor_state[cpu_num] = MONITOR_OFF;
      }
    }

    /*
     *  Initialize the monitored_threads queue of the new monitor
     */
    queue_init(&loc_monitor_ptr->monitored_threads); 

    /*
     *  Add the new monitor to the global all_monitors queue.
     */
    all_monitor_enqueue(loc_monitor_ptr);
    return(KERN_SUCCESS);
}



/********************************************************************************  
 *	Routine:	monitor_terminate(monitor_port)
 *
 *	Function:
 *		Goes through linked-list of threads and sets the state of 
 *		monitor to MONITOR_SHUTDOWN in each of the threads with 
 *		pointers to the monitor.  It grabs the write lock before 
 *		changing the state in each thread.  
 *
 *		Thus, in other code, when the threads either exit or context
 *		switch, they set their monitor state to MONITOR_OFF, 
 *		decrement the threads counter in the monitor structure and 
 *		monitor pointers to NULL.  The last thread to set its state 
 *	        to MONITOR_OFF de-allocates the monitor.
 *
 *      Callers: (perhaps incomplete)
 * 		the user
 */


kern_return_t
monitor_terminate(this_monitor)
register monitor_t	this_monitor;	     /* this monitor	    */
{    
    register int		cpu_num;	     /* index for inits	    */
    register kern_port_t	myport;
    register port_t		self;
    register unsigned int	s;

    /*
     * Check to see if the monitor is a valid monitor.
     */
    if (this_monitor == MONITOR_NULL)
      return(KERN_INVALID_ARGUMENT);

    /*
     * Now lock the monitor and check two things:
     *	1.  If the monitor port doesn't exist, the monitor is being
     *		terminated.  Return KERN_INVALID_ARGUMENT.
     *  2.  If someone is reading the monitor, wait for them to finish.
     *
     * Have to check both of these in a loop because it is necessary
     * to drop the lock.
     */
    s = splsched();
    monitor_lock(this_monitor);

    while ((this_monitor->read_flag == MONITOR_READING) ||
	   ((self = this_monitor->monitor_self) == PORT_NULL)) {
		/*
		 *	Termination check.
		 */
		if ((self = this_monitor->monitor_self) == PORT_NULL) {
			monitor_unlock(this_monitor);
			(void) splx(s);
			return(KERN_INVALID_ARGUMENT);
		}

		/*
		 *	read_flag must be set.  Wait for it.
		 */
		thread_sleep((vm_offset_t)&this_monitor->read_flag,
			&this_monitor->lock, TRUE);
		monitor_lock(this_monitor);
    }

    /*
     * Disable the monitor port (the existence of the port
     * was checked above).  The monitor data structures are 
     * disconnected from the monitor port.  Inside the
     * ipc_monitor_disable() call, the port has been 
     * separated from the monitor and the reference counter is
     * decremented.
     */
    ipc_monitor_disable(this_monitor);

    /*
     *	Release kernel right to the monitor port.
     */
    ipc_monitor_terminate(this_monitor);

    /*
     *  Set the monitor state to MONITOR_SHUTDOWN.
     */
    this_monitor->global_state = MONITOR_SHUTDOWN;

    /*
     *  Unlock the monitor and then loop through the cpu flags and 
     *  setprocessor_states to MONITOR_OFF.  grab and release the
     *  the monitor lock before and after each iteration.  NO danger
     *  races here because 1) any futher calls to monitor terminate
     *  will fail the port tests above; and 2) The monitor_read() call
     *  may interleave with the assignment of MONITOR_OFF to the
     *  processor states, but none of the buffers will go away while
     *  it is reading them.
     */
    for (cpu_num = 0 ; cpu_num < NCPUS; cpu_num++)
    {
      this_monitor->processor_state[cpu_num] = MONITOR_OFF;
    }
    monitor_unlock(this_monitor);
    (void) splx(s);

    /*
     *	Deallocate monitor's reference to itself.
     */
    monitor_deallocate(this_monitor);

    return(KERN_SUCCESS);
}


/*****************************************************************************
 *	Routine:	monitor_resume(this_monitor)
 *
 *	Function:
 *		Enable the passed kernel monitor for monitoring.  The global
 *		state of the monitor is set to MONITOR_RUN.  The monitor 
 *		stateof any thread associated with the monitor is also set to 
 *		MONITOR_RUN.
 *
 *      Callers: (perhaps incomplete)
 * 		the user 
 */


kern_return_t
monitor_resume(this_monitor)
register monitor_t	this_monitor;		     /* this monitor	    */
{    
    register port_t		self;
    register unsigned int	s;

    /*
     * Check to see if the monitor is a valid monitor.
     */
    if (this_monitor == MONITOR_NULL)
      return(KERN_INVALID_ARGUMENT);

    s = splsched();
    monitor_lock(this_monitor);
    /*
     * If the monitor port does not exist, unlock the monitor
     * and return KERN_INVALID_ARGUMENT.
     */
    if ((self = this_monitor->monitor_self) == PORT_NULL) {
	monitor_unlock(this_monitor);
	(void) splx(s);
	return(KERN_INVALID_ARGUMENT);
    }

    /*
     *  If the monitor is being shut down, do NOT set the 
     *  monitor state to MONITOR_RUN.  Instead, unlock the 
     *  monitor and return that the call failed.
     */
    if (this_monitor->global_state == MONITOR_SHUTDOWN) {
	monitor_unlock(this_monitor);
	(void) splx(s);
	return(KERN_FAILURE);
    }
   
    /*
     *  Set the monitor state to MONITOR_RUN.
     */
    this_monitor->global_state = MONITOR_RUN;

    monitor_unlock(this_monitor);
    (void) splx(s);
    return(KERN_SUCCESS);
}


/****************************************************************************
 *	Routine:	monitor_suspend(this_monitor)
 *
 *	Function:
 *		Disable the passed kernel monitor for monitoring.  The global
 *		state of the monitor is set to MONITOR_PAUSE.  The monitor 
 *		state of any thread associated with the monitor is also set 
 *		to MONITOR_PAUSE.  
 *
 *      Callers: (perhaps incomplete)
 * 		the user 
 */


kern_return_t
monitor_suspend(this_monitor)
register monitor_t	this_monitor;		     /* this monitor	    */
{    
    register port_t		self;
    register unsigned int	s;

    /*
     * Check to see if the monitor is a valid monitor.
     */
    if (this_monitor == MONITOR_NULL)
      return(KERN_INVALID_ARGUMENT);

    s = splsched();
    monitor_lock(this_monitor);

    /*
     * If the monitor port does not exist, unlock the monitor
     * and return KERN_INVALID_ARGUMENT.
     */
    if ((self = this_monitor->monitor_self) == PORT_NULL) {
	monitor_unlock(this_monitor);
  	(void) splx(s);
	return(KERN_INVALID_ARGUMENT);
    }

    /*
     *  If the monitor is being shut down, do NOT set the 
     *  monitor state to MONITOR_PAUSE.  Instead, unlock the 
     *  monitor and return that the call failed.
     */
    if (this_monitor->global_state == MONITOR_SHUTDOWN) {
	monitor_unlock(this_monitor);
  	(void) splx(s);
	return(KERN_FAILURE);
    }

    /*
     *  Set the monitor state to MONITOR_PAUSE.
     */    
    this_monitor->global_state = MONITOR_PAUSE;

    monitor_unlock(this_monitor);
    (void) splx(s);	
    return(KERN_SUCCESS);
}




/****************************************************************************
 *  	Routine:	thread_monitor(this_monitor,monitor_id,this_thread):
 *
 *  	Function:
 *  		Enables kernel monitoring of the thread thread basis.  The 
 *		caller must pass a unique integer for thread identification 
 *		outside kernel.
 *
 *  		Thus, when thread_block() is called, a sensor will fire
 *		only if either the thread switched out or the thread
 *		switched in in set for monitoring.
 *
 *      Callers: (perhaps incomplete)
 * 		the user 
 */

kern_return_t
thread_monitor(this_monitor,monitor_id,this_thread)
register int		monitor_id;
register monitor_t	this_monitor;
register thread_t       this_thread;
{
    register int		s;
    register port_t		self;

    /*
     * Check to see if the thread  and monitor are ok.
     */
    if ((this_thread == THREAD_NULL) || (this_monitor == MONITOR_NULL))
      return(KERN_INVALID_ARGUMENT);

    s = splsched();
    monitor_lock(this_monitor);

    /*
     * If the monitor port does not exist, unlock the monitor
     * and return KERN_INVALID_ARGUMENT.
     */
    if ((self = this_monitor->monitor_self) == PORT_NULL) {
	monitor_unlock(this_monitor);
	(void) splx(s);
	return(KERN_INVALID_ARGUMENT);
    }

    /*
     *  If the monitor is being shut down, do NOT enable 
     *  the thread for monitor.  Instead, unlock the monitor 
     *  and return that the call failed.
     */
    if (this_monitor->global_state == MONITOR_SHUTDOWN) {
	monitor_unlock(this_monitor);
	(void) splx(s);
	return(KERN_FAILURE);
    }

    /*
     * Assign the monitor and a unique id to the thread.  Also enqueue
     * the thread onto the monitored_threads of this_monitor.
     */

    thread_lock(this_thread);
    if (!(this_thread->active)) {
       thread_unlock(this_thread);
       monitor_unlock(this_monitor);
       (void) splx(s);
       return(KERN_FAILURE);
    }
    this_thread-> monitor_obj = this_monitor;
    this_thread-> monitor_id = monitor_id;
    queue_enter(&this_monitor->monitored_threads, this_thread,
		   thread_t, monitored_threads);
    thread_unlock(this_thread);
    this_monitor->ref_count++;
    monitor_unlock(this_monitor);
    (void) splx(s);
    return(KERN_SUCCESS);
}


/****************************************************************************
 *  	Routine:	thread_unmonitor(this_monitor,this_thread):
 *
 *  	Function:
 *  		Disables kernel monitoring of the thread thread basis.  
 *
 *      Callers: (perhaps incomplete)
 * 		the user 
 */

kern_return_t
thread_unmonitor(this_monitor, this_thread)
register monitor_t	this_monitor;
register thread_t	this_thread;
{
    register int		s;
    register int  		return_value = KERN_SUCCESS;
    register port_t		self;

    /*
     * Check to see if the thread and monitor are ok.
     */
    if ((this_thread == THREAD_NULL) || (this_monitor == MONITOR_NULL))
      return(KERN_INVALID_ARGUMENT);

    s = splsched();
    monitor_lock(this_monitor);
    /*
     * If the monitor port does not exist, unlock the monitor
     * and return KERN_INVALID_ARGUMENT.
     */
    if ((self = this_monitor->monitor_self) == PORT_NULL) {
	monitor_unlock(this_monitor);
	(void) splx(s);
	return(KERN_INVALID_ARGUMENT);
    }

    /*
     * If the monitor passed to this call is the monitor
     * assigned to this thread, disable the thread by
     * setting the monitor pointer of the thread to NULL.
     * Set the id to MONITOR_NO_ID and remove the thread
     * from monitored_threads queue of this_monitor.
     *
     * Otherwise, unlock the monitor and return KERN_INVALID_ARGUMENT.
     */

    thread_lock(this_thread);
    if (this_thread->monitor_obj != this_monitor) {
	thread_unlock(this_thread);
        monitor_unlock(this_monitor);
	(void) splx(s);
	return(KERN_INVALID_ARGUMENT);
    }

    this_thread->monitor_obj = MONITOR_NULL;
    this_thread->monitor_id = MONITOR_NO_ID;
    queue_remove(&this_monitor->monitored_threads, this_thread,
				thread_t, monitored_threads);
    thread_unlock(this_thread);
    monitor_unlock(this_monitor);
    (void) splx(s);
    /*
     *  Decrement the reference counter once to show that the thread
     *  is no longer using the monitor.
     */    
    monitor_deallocate(this_monitor);
    return(KERN_SUCCESS);
}


/****************************************************************************
 * 	Routine:	monitor_read(this_monitor, buffer, events_read)
 *
 *	Function:
 * 		The events in the kernel monitor buffer are read.  It is the 
 * 		responsibility of user code to format the event data returned 
 * 		by this call.  Data is aligned around event boundaries.
 *
 *	NOTE-0:
 * 		It is not guaranteed that the data collected in a single call 
 * 		will contain unaltered events if an overflow has occurred.
 *	NOTE-1:
 *		The write pointer points to the last written location.  After
 *		reading a buffer, the read pointer points to the location 
 *		immediately after the last written location.
 *
 *      Callers: (perhaps incomplete)
 * 		the user 
 */

kern_return_t 
monitor_read(this_monitor, out_buffer, events_read)
     register monitor_t	                this_monitor;	/* this monitor	    */
     kern_mon_event_t		 	out_buffer;
     unsigned int		        *events_read;
{
    register kern_mon_event_t	loc_write_ptr;		/* local pointer    */
    register kern_mon_event_t	loc_read_ptr;		/* local pointer    */
    register monitor_buffer_t	loc_buffer_ptr;		/* and one more */
    int				cpu_num;
    int				s;
    int				buf_events1 = 0;        /* partial event sum */
    int				buf_events2 = 0;        /* partial event sum */
    int				total_events = 0;	/* total events sum  */
    port_t			self;
    boolean_t                   mig_buffer_full = FALSE;

    /*
     * Check to see if the monitor is a valid monitor.
     */
    if (this_monitor == MONITOR_NULL)
	return(KERN_INVALID_ARGUMENT);

    /*
     * Now lock the monitor and check two things:
     *	1.  If the monitor port doesn't exist, the monitor is being
     *		terminated.  Return KERN_INVALID_ARGUMENT.
     *  2.  If someone is reading the monitor, wait for them to finish.
     *
     * Have to check both of these in a loop because it is necessary
     * to drop the lock.
     */
    s = splsched();
    monitor_lock(this_monitor);

    while ((this_monitor->read_flag == MONITOR_READING) ||
	   ((self = this_monitor->monitor_self) == PORT_NULL)) {
		/*
		 *	Termination check.
		 */
		if ((self = this_monitor->monitor_self) == PORT_NULL) {
			monitor_unlock(this_monitor);
			(void) splx(s);
			return(KERN_INVALID_ARGUMENT);
		}

		/*
		 *	read_flag must be set.  Wait for it.
		 */
		thread_sleep((vm_offset_t)&this_monitor->read_flag,
			&this_monitor->lock, TRUE);
		monitor_lock(this_monitor);
    }

    /*
     * Before traversing the buffers, set a flag indicating that a read
     * is being performed.  Then unlock the monitor_lock and go to it!
     */
    this_monitor->read_flag = MONITOR_READING;
    monitor_unlock(this_monitor);
    (void) splx(s);

    for ( cpu_num = 0; cpu_num < NCPUS; cpu_num++ ){
	/*
	 * Break out of the FOR loop if the MIG buffer is full
	 */
	if (mig_buffer_full) break;

	switch (this_monitor->processor_state[cpu_num]){

	case MONITOR_OFF:
	    /*
	     *	The MONITOR_CPU_OFF state tells the reader that
	     *	that although mycpu is dead, its monitor buffer may
	     *	still contain something useful.  If a buffer state is
	     *	MONITOR_CPU_OFF,  the reader sets the state of the 
	     *	buffer to MONITOR_OFF before it reads.  
	     */
	    break;
	case MONITOR_CPU_OFF:
	    this_monitor->processor_state[cpu_num] = MONITOR_OFF;

	    /*
	     * ... Fall through.
	     */

	case MONITOR_RUN: 
	    /*  
	     *  Grab the global pointers and set the local pointers to them.
	     *  Later, the read pointer is updated by setting it equal to
	     *  loc_write_ptr + 1 (or similar).
	     *
	     *  NOTE: It is possible that sensors may write the buffer while
	     *  it is being read.  This produces no unexpected results
	     *  except in the case when the buffer is full and more sensor
	     *  firings occur.  The sensors bump of the read_ptr[cpu_num]
	     *  pointer and set a flag when overflowing.  If this happens
	     *  while reading here but after setting loc_read_ptr, the
	     *  read_ptr[cpu_num] pointer will eventually be reset to
	     *  a pointer value less than it was bumped up to.  Thus, in
	     *  read, the reader will see that overflow has occurred and
	     *  that the values contained in this read and last read
	     *  may be suspect.
	     */
	    loc_buffer_ptr = &(this_monitor->buffer[cpu_num]);
	    loc_write_ptr = loc_buffer_ptr->write_ptr;
	    loc_read_ptr = loc_buffer_ptr->read_ptr;

	    /*
	     *  Check to see where everybody points.
	     */
	    if(loc_read_ptr == loc_buffer_ptr->buffer_head) {
		/*
		 * The read pointer is at the beginning of the buffer.
		 * Now, if the write pointer is NOT at the end of the 
		 * buffer then there is something in the buffer  (the write
		 * pointer is constrained to get no closer than 2 events
		 * behind read pointer unless buffer is empty when it is
		 * one behind the read pointer).  Note that if the buffer
		 * is empty, the global read pointer for that buffer
		 * is not updated.
		 */
		if(loc_write_ptr != loc_buffer_ptr->buffer_end) {
		    /*
		     *  Calculate the number of events in the buffer and update
		     *  the global read pointer of this buffer.  If the number
		     *  of new events plus the current total events exceeds the
		     *  size of the MIG buffer, then only grab enough events to
		     *  fill the buffer (saving the others for a later call).
		     *  Special action for MIG is taken only when the size
		     *	limit is exceeded.
		     */
		    buf_events1 = loc_write_ptr - loc_read_ptr + 1;
		    if (buf_events1 + total_events <= MONITOR_MIG_BUF_SIZE) {
			loc_buffer_ptr->read_ptr = loc_write_ptr + 1;
		    }
		    else {
			buf_events1 = MONITOR_MIG_BUF_SIZE - total_events;
			loc_buffer_ptr->read_ptr = loc_read_ptr + buf_events1;
			mig_buffer_full = TRUE;
		    }

		    /*
		     *  Copy the monitor events into the local buffer, indexed
		     *  by total_events.  Then increment the total event count.
		     */
		    bcopy((char *) loc_read_ptr,
			  (char *) this_monitor->ptr_to_reader_buf
			  + (total_events * sizeof(kern_mon_event)),
			  buf_events1 * sizeof(kern_mon_event));
		    total_events = total_events + buf_events1;

		}
	    }
	    else {
		/*
		 * Check if there is something in the buffer (the write
		 * pointer is constrained to get no closer than 2 events
		 * behind read pointer unless buffer is empty).  Note
		 * that if the buffer is empty, the global read pointer
		 * for that buffer is not updated.
		 */
		if(loc_write_ptr != loc_read_ptr - 1) {
		    if (loc_read_ptr <= loc_write_ptr) {
			/*
			 *  Calculate the number of events in  buffer.  
			 *  The + 1 is needed because the read pointer 
			 *  begins at the write pointer + 1 (the write
			 *  pointer points to the last written entry).
			 */
			buf_events1 = (loc_write_ptr - loc_read_ptr + 1);
			if (buf_events1 + total_events <=
			    MONITOR_MIG_BUF_SIZE) {
				if (loc_write_ptr !=
				    loc_buffer_ptr->buffer_end) {
					loc_buffer_ptr->read_ptr =
					    loc_write_ptr + 1;
				}
				else {
				    loc_buffer_ptr->read_ptr =
					loc_buffer_ptr->buffer_head;
				}
			}
			else {
			    buf_events1 = MONITOR_MIG_BUF_SIZE - total_events;
			    loc_buffer_ptr->read_ptr =
				loc_read_ptr + buf_events1;
			    mig_buffer_full = TRUE;
			}
			/*
			 *  Copy the monitor events into the local buffer, 
			 *  indexed by total_events.  Then increment the total
			 *  event count.
			 */
		 	bcopy((char *) loc_read_ptr,
		    		(char *) this_monitor->ptr_to_reader_buf 
			 	+ (total_events * sizeof(kern_mon_event)),
				buf_events1 * sizeof(kern_mon_event));
			total_events = total_events + buf_events1;
		    }
		    else {
			/*
			 *  The read pointer is greater than the write pointer.
			 *  Calculate the number of events in the buffer.
			 *
			 *  NOTE:  In order to understand why the +1 is in the
			 *  next two statments, recall what the pointers point
			 *  to.  Now, if the reader is at the end of the buffer
			 *  and the writer is at the beginning of the buffer,
			 *  then if the +1 was not present we would calculate
			 *  that there was nothing in the buffer.  But,
			 *  actually, there would be two events.  Hence,
			 *  the + 1.
			 */
			buf_events1 = (loc_buffer_ptr->buffer_end -
			    loc_read_ptr) + 1;

			/*
			 * Check to see whether the new events will exceed
			 * the MIG buffer limit.  If not, then set the global
			 * read_ptr to the beginning of the buffer in case
			 * calculations for buf_events2 fail.  If the
			 * addition of buf_events1 will exceed the MIG buffer
			 * size, only collect an amount that will meet
			 * the limit.
	      		 */
			if (buf_events1 + total_events <=
			    MONITOR_MIG_BUF_SIZE) {
				loc_buffer_ptr->read_ptr =
				    loc_buffer_ptr->buffer_head;
			} else {
			    buf_events1 = MONITOR_MIG_BUF_SIZE - total_events;
			    loc_buffer_ptr->read_ptr = loc_read_ptr +
				buf_events1;
			    mig_buffer_full = TRUE;
			}

			/*
			 *  Copy the monitor events into the local buffer, 
			 *  indexed by total_events.  Then increment the total
			 *  event count.
			 */
			bcopy((char *) loc_read_ptr,
			      (char *) this_monitor->ptr_to_reader_buf  
			      + (total_events * sizeof(kern_mon_event)),
			      buf_events1 * sizeof(kern_mon_event));
			total_events = total_events + buf_events1;

	      		/*
			 * Break out of the case statement if the MIG buffer
			 * is full.
			 */
			if (mig_buffer_full) break;

			/*
			 *  Find how many events lie at the other end of
			 *  the buffer.  If the new total exceeds the mig
			 *  limit, then only collect the difference.
			 *
			 *  Copy the monitor events into the local buffer, 
			 *  indexed by total_events.  Then increment the total
			 *  event count.
			 */
			buf_events2 = (loc_write_ptr -
			    loc_buffer_ptr->buffer_head) + 1;
			if (buf_events2 + total_events <=
			    MONITOR_MIG_BUF_SIZE) {
				loc_buffer_ptr->read_ptr = loc_write_ptr + 1;
			}
			else {
			    buf_events2 = MONITOR_MIG_BUF_SIZE - total_events;
			    loc_buffer_ptr->read_ptr =
				loc_buffer_ptr->buffer_head + buf_events2;
			    mig_buffer_full = TRUE;
			}
			bcopy((char *) loc_buffer_ptr->buffer_head,
			      (char *) this_monitor->ptr_to_reader_buf 
			      + (total_events * sizeof(kern_mon_event)),
			      buf_events2 * sizeof(kern_mon_event));
			total_events = total_events + buf_events2;
		    }
		}
	    }
	    break;
	
	    /*
	     *  The cpu state should not be anything but the cases
	     *  shown above.  Thus, we should not get here.
	     */
	default:
	    printf("reader: Bad cpu state, cpu %d, state %d\n",
	       cpu_num,this_monitor->processor_state[cpu_num]);
	    panic("KERNEL MONITOR:  Bad cpu state, can not read");
	}
    }
    /*
     *  Now copy all stuff out to MIG and return the number of events the
     *  the user gets.  Afterwards, grab the lock and reset the read flag.
     *  Then record the number of events read, unlock the monitor and send
     *  a wakeup signal.
     */

    bcopy((char *) this_monitor->ptr_to_reader_buf, (char *) out_buffer,
	total_events * sizeof(kern_mon_event));
    s = splsched();
    monitor_lock(this_monitor);

    this_monitor->read_flag = MONITOR_NOT_READING;
    *events_read = total_events;

    monitor_unlock(this_monitor);
    (void) splx(s);
    thread_wakeup((vm_offset_t)&this_monitor->read_flag);
    return(KERN_SUCCESS);
}    
		



