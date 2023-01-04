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
static char	*sccsid = "@(#)$RCSfile: isr_env.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:03 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 * Copyright 1990 Encore Computer Corporation
 *
 * ALL RIGHTS RESERVED. Licensed Material - Property of Encore Computer
 * Corporation. This software is made available solely pursuant to the
 * terms of a software license agreement which governs its use. 
 * Unauthorized duplication, distribution or sale are strictly prohibited.
 *
 * Module Function:
 *	Interrupt service environment, supports loadable drivers, deferred
 *		interrupt handling, non-parallel drivers, etc.
 */
/*
 * OSF/1 Release 1.0
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysconfig.h"
#include "sys/lock_types.h"
#include "kern/lock.h"
#include "kern/queue.h"
#include "kern/thread.h"
#include "mmax/MP_queue.h"
#include "mmax/isr_env.h"
#include "mmaxio/crqdefs.h"
#include "mmax/boot.h"
#include "mmax/cpu.h"
#include "mmaxio/io.h"

extern union boot	Boot;

struct intr_list	itbl[NUMSYSITBL];

/*
 * The following counter arrays are used for per interrupt vector
 * counts of interrupts, interrupts with no handlers, and/or interrupts
 * with all disabled handlers.
*/
long	istray_cnt[NUMSYSITBL];
long	disintr_cnt[NUMSYSITBL];

struct isr_que	*isr_q_slots;
struct MP_q_hd	isr_free_q, isr_req_q;

void
itbl_init()
{
	register long	i;

	/*
	 * Zero the slot and initialize the simple
	 * lock element for each slot in the
	 * itbl[] array.
	 * Also zero the interrupt counters.
	*/
	for(i = 0; i < NUMSYSITBL; i++) {
		bzero(itbl[i], sizeof(struct intr_list));
		/*
		 * All the interrupt table locks are initialized
		 * to be NON-BLOCKING for use in an interrupt
		 * context.
		*/
		lock_init2(&itbl[i].it_rwlock, FALSE, LTYPE_SWINTR);
		itbl[i].it_nextid = 1;
		istray_cnt[i] = 0;
		disintr_cnt[i] = 0;
	}
	handler_init();
	return;
}

isr_env_init()
{
	extern void isr_thread();
	register struct isr_que	*i;
	register long		n;

	/*
	 * Initialize the Global ISR queue environment.
	 *
	 * isr_q_slots is the global array allocated in startup(), file
	 * mmax/machdep.c.
	*/
#ifndef ISR_THREADS_TEST
	ASSERT(Boot.boot_num_isrthreads != 0);
# ifdef ISR_THREADS_DEBUG
	printf("boot_num_isrthreads = %d\n", Boot.boot_num_isrthreads);
# endif
#endif
	queue_init(&isr_req_q.head);
	MPQ_SLOCK_INIT(&isr_req_q.lock);
	isr_req_q.cnt = 0;
	queue_init(&isr_free_q.head);
	MPQ_SLOCK_INIT(&isr_free_q.lock);
	isr_free_q.cnt = 0;
#ifndef ISR_THREADS_TEST
	for(i = isr_q_slots; i < &isr_q_slots[Boot.boot_size_isrqueue]; i++)
#else
	for(i = isr_q_slots; i < &isr_q_slots[B_SIZE_ISRQUEUE]; i++)
#endif
		MP_enqueue(&isr_free_q, i);

	/*
	 * Initialize the Global ISR thread pool.
	*/
#ifndef ISR_THREADS_TEST
	for(n = 0; n < Boot.boot_num_isrthreads; n++)
#else
	for(n = 0; n < B_NUM_ISRTHREADS; n++)
#endif
		(void)kernel_thread(current_task(), isr_thread);
}

void
isr_thread()
{
	register thread_t	thread;
	register ihandler_t	*ihp;
	register struct MP_q_hd	*hdptr;
	register struct isr_que	*isrq, *i;
	struct isr_que		*isr_pkt;

	hdptr = &isr_free_q;
	thread = current_thread();
	thread->priority = thread->sched_pri = 0;
	thread_swappable(thread, FALSE);

	/*
	 * In a loop:
	 * 1- get a request packet ( we may sleep waiting for one).
	 * 2- setup execution environment (via the control word).
	 * 3- execute ISR.
	 * 4- return to calling environment.
	 * 5- free the request packet.
	 * 6- lock the queue and check if allocation of more slots
	 *	is needed; if so, do it and unlock the queue.
	*/
	for(;;) {
		MP_dequeue(&isr_req_q, &isr_pkt, MP_QWAIT);
		ihp = isr_pkt->handle;
		FUNNEL(ihp->ih_funnel);
		(*ihp->ih_handler)(ihp);
		UNFUNNEL(ihp->ih_funnel);
		MP_enqueue(&isr_free_q, isr_pkt);

		/* Check if memory is needed. */
		MPQ_SLOCK(&hdptr->lock);
		if(hdptr->cnt <= ISR_Q_LWM && hdptr->alloc != TRUE) {
			/* Allocate some memory into the free queue */
			hdptr->alloc = TRUE;
			MPQ_SULOCK(&hdptr->lock); /* We might sleep */
			/*
			 * NOTE: The following NULL test is
			 * an unimplemented error return from
			 * kalloc() and SHOULD NOT HAPPEN!
			*/
			isrq = (struct isr_que *)
				kalloc(sizeof(struct isr_que) * ISR_Q_ALC);
			if(isrq != (struct isr_que *)NULL) {
				for (i = isrq; i < &isrq[ISR_Q_ALC]; i++)
					MP_enqueue(&isr_free_q, i);
			}

			MPQ_SLOCK(&hdptr->lock);
			hdptr->alloc = FALSE;
		}
		MPQ_SULOCK(&hdptr->lock);
	}
	/* Should never get here from forever loop */
}

#if	MMAX_IDEBUG
extern long	last_inttype[], last_intpc[], last_intth[];
#if STAT_TIME
#define	FRAME_PC	10	/* offset of interrupt PC in stack frame */
#else
#define	FRAME_PC	11	/* offset of interrupt PC in stack frame */
#endif
#endif

vecbus_isr(sp, ilvl)
long *sp;
long ilvl;
{
	register long			cpu_id, hand_flag;
	register struct ihandler	*ihp;
	struct isr_que			*isr_pkt;

#if	MMAX_IDEBUG
	cpu_id = cpu_number();
	last_inttype[cpu_id] = ilvl;
	last_intpc[cpu_id] = sp[FRAME_PC];
	last_intth[cpu_id] = (long)(active_threads[cpu_id]);
#endif
	/*
	 * If there is no handler loaded, print a console message,
	 * increment a counter, and return.
	 * Otherwise, walk down the interrupt handler list for this
	 * vector number calling each ISR enabled.
	*/
	ihp = itbl[ilvl].it_intr;
	if(ihp == (ihandler_t *)NULL) {
		printf("vecbus_isr: received intr for unknown driver @ %d\n",
			ilvl);
		istray_cnt[ilvl]++;
		return;
	}

	hand_flag = 0;
	for ( ; ihp; ihp = ihp->ih_next) {
		if ((ihp->ih_state & IH_STATE_ENABLED) == 0)
			continue;
		ITABLE_READ_LOCK(ilvl);
		++hand_flag;
		++ihp->ih_stats.ihs_count;
#if	SER_COMPAT
		if(ihp->ih_funnel == FUNNEL_NULL) {
#endif
			if (!(ihp->ih_flags & IH_VEC_PASS_ISP))
				(*ihp->ih_handler)(ihp);
			else
				(*ihp->ih_handler)(ihp, sp);
#if	SER_COMPAT
		} else {
			/*
			 * We have a special situation;
			 * punt to an ISR thread to synchronize
			 * the driver code execution.
			 */
			MP_dequeue(ihp->ih_free_queue, &isr_pkt,
				MP_QNOWAIT);
			/*
			 * Fill in the isr_que structure
			 * and place it on the request queue.
			*/
			isr_pkt->handle = ihp;
			MP_enqueue(ihp->ih_req_queue, isr_pkt);
		}
#endif
	}
	ITABLE_READ_UNLOCK(ilvl);

	if(!hand_flag) {
		printf("vecbus_isr: no enabled handlers for interrupt @ %d\n", ilvl);
		disintr_cnt[ilvl]++;
	}

	return;
}

void
dump_ihandlers(pih)
ihandler_t *pih;
{
	for (; pih; pih = pih->ih_next) {
		printf("id %x %x handler %x resolver %x flags %x\n",
			pih->ih_id.ihd_index, pih->ih_id.ihd_element, pih->ih_handler,
			pih->ih_resolver, pih->ih_flags);
		printf("  state %x level %d priority %d hparam %x rparam %x\n",
			pih->ih_state, pih->ih_level, pih->ih_priority,
			pih->ih_hparam[0].intparam, pih->ih_rparam[0].intparam);
#if	SER_COMPAT
		printf("  req_queue %x free_queue %x ih_funnel %x\n",
			pih->ih_req_queue, pih->ih_free_queue, pih->ih_funnel);
#else
		printf("  req_queue %x free_queue %x\n",
			pih->ih_req_queue, pih->ih_free_queue);
#endif
	}
}

void
dump_itbl(pitbl)
struct intr_list *pitbl;
{
	int i;

	for (i = 0; i < NUMSYSITBL; ++i, ++pitbl)
		if (pitbl->it_intr) {
			printf("%d:	");
			dump_ihandlers(pitbl->it_intr);
		}
}
