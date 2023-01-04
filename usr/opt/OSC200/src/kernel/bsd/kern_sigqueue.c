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
static char *rcsid = "@(#)$RCSfile: kern_sigqueue.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/22 17:28:21 $";
#endif

#include <sys/signal.h>
#include <sys/proc.h>
#include <sys/siginfo.h>
#include <kern/zalloc.h>
#include <vm/vm_kern.h>
#include <mach/kern_return.h>

/*
 * These are zeroed siginfo structures, suitable for assignment.
 * (Zeroed because static/bss data.)
 */
siginfo_t zero_siginfo;
k_siginfo_t zero_ksiginfo;

/*
 * This is a guess regarding the maximum amount of space needed by this
 * zone for sigqueues at any one time.  More than this can be allocated, 
 * because this zone is set to be NOT exhaustible (see zinit() for more
 * info).
 */
#define SIGQ_MAX_MEM	(32 * nproc * sizeof(struct sigqueue_struct))

/*
 * Initial amount of memory allocated for the sigqueue zone --
 * of course more memory will be allocated for this zone if
 * zalloc() needs more.  This is a pretty arbitrary number.
 */
#define	SIGQ_INITIAL_MEM	(8 * 32  * sizeof(struct sigqueue_struct))

/*
 * The amount of memory allocate each time the zone is expanded.
 */
#define SIGQ_ALLOC_MEM		(4 * 32 * sizeof(struct sigqueue_struct))

struct zone *sigqueue_zone;

/*
 * Create a zone for sigqueue structures.  The zone is by default
 * expandable, collectable, pageable, and NOT exhaustable, NOT sleepable.
 */
void
sigqueue_init()
{
	register vm_offset_t	newmem;

	sigqueue_zone = zinit(sizeof(struct sigqueue_struct),/* element size */
			      SIGQ_MAX_MEM,        /* "max" zone size */
			      SIGQ_ALLOC_MEM,      /* size for expansion */
			      "SVR4 sigqueue zone");

	newmem = kmem_alloc(kernel_map, SIGQ_INITIAL_MEM);
	if (newmem)
		zcram(sigqueue_zone, newmem, SIGQ_INITIAL_MEM);
}

/*
 * Signal queue manipulation routines.
 */

/*
 * Find the next sigqueue in the queue "q" starting at the queue
 * member "qp" which has a signal of type "sig".
 *
 */
sigqueue_t
sigq_find_next_sig(queue_t q, sigqueue_t qp, int sig)
{
	while (!queue_end(q, (queue_entry_t) qp)) {
		if (qp->siginfo.si_signo == sig) {
			return(qp);
		} else {
			qp = (sigqueue_t) queue_next(&qp->sigqueue_list);
		}
	}
	return NULL;
}


/*
 *
 * remove all sigqueue structures from the queue "q" which have
 * the signal type "sig".
 *
 */
int
sigq_remove_all(queue_t q, int sig)
{
	sigqueue_t sigqueue, tmpq;
	int rc;

	tmpq = (sigqueue_t) queue_first(q);

	/* remove all sigqueues in sigqueue list */
	if (sig == ALL_SIGQ_SIGS) {
		while (!queue_end(q, (queue_entry_t) tmpq)) {
			sigq_dequeue(q, tmpq);
			SIGQ_FREE((vm_offset_t) tmpq);
			tmpq = (sigqueue_t) queue_first(q);
		}
		return KERN_SUCCESS;
	}
		
	rc = KERN_FAILURE;	/* assume couldn't find any to remove */

	while ((sigqueue = sigq_find_next_sig(q, tmpq, sig)) != NULL) {
		/*
		 * continue looking starting at one past the one
		 * we're about to remove.
		 */
		tmpq = (sigqueue_t) queue_next(&sigqueue->sigqueue_list);
		sigq_dequeue(q, sigqueue);
		SIGQ_FREE((vm_offset_t) sigqueue);
		rc = KERN_SUCCESS;
	}
	return rc;
}

/* 
 *
 * remove all but the first sigqueue structure in queue "q" which contain
 * a signal of the type "sig".
 *
 * Don't really need this for SVR4, because there should only ever be
 * one of any signal type queued on any queue.
 *
 */
int
sigq_only_one(queue_t q, int sig)
{
	sigqueue_t sigqueue;

	sigqueue = sigq_find_sig(q, sig);
	if (sigqueue) {
		sigq_dequeue(q, sigqueue);
		sigq_remove_all(q, sig);
		enqueue_tail(q, (queue_entry_t) sigqueue);
		/* found at least one of this signal type */
		return KERN_SUCCESS;
	}
	return KERN_FAILURE;	/* didn't find this signal type */
}


#if	SVR4_SIG_DEBUG
/*
* count all sigqueue structures in the queue "q" 
*/
int
sigq_count_all(queue_t q)
{
	sigqueue_t tmpq;
	int count = 0;

	SVR4_SIGDPRINT(5,("sigqueue_count_all(): q = 0x%x\n", q));

	tmpq = (sigqueue_t) queue_first(q);
	while ( !queue_end(q, (queue_entry_t) tmpq) ) {
		count++;
		tmpq = (sigqueue_t) queue_next(&(tmpq->sigqueue_list));
	}
	return count;
}
#endif	/* SVR4_SIG_DEBUG */


