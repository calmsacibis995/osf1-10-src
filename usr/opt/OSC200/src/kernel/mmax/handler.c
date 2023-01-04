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
static char	*sccsid = "@(#)$RCSfile: handler.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:47 $";
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
 *	Interrupt Handler service routines to support loadable drivers.
 */
/*
 * OSF/1 Release 1.0
 */

#include "sys/errno.h"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysconfig.h"
#include "sys/lock_types.h"
#include "kern/lock.h"
#include "kern/queue.h"
#include "mmax/isr_env.h"
#include "mmax/handler.h"
#include "mmaxio/io.h"

/*
 * Interrupt handler function lock definitions
 */
#define	HANDLER_WRITE_LOCK()	lock_write(&intr_func_lock)
#define	HANDLER_WRITE_UNLOCK()	lock_done(&intr_func_lock)
#define	HANDLER_LOCK_INIT()	lock_init2(&intr_func_lock, TRUE, LTYPE_SWINTR);


static lock_data_t    intr_func_lock;


/*
 * Initialize synchronization lock for the handler functions.
 */
void
handler_init()
{
	HANDLER_LOCK_INIT();
}



/*
 * NAME:	handler_add()
 * 
 * FUNCTION:	Register an interrupt handler to the interrupt dispatcher
 * 
 * EXEC ENV: 	Dynamic Device Drivers - This routine is called by a 
 *		dynamically loaded device driver's configuration routine
 *		to register its ISR.
 *
 * RETURNS:	On success zero is returned, else -1 on failure.
 * 
*/
ihandler_id_t *
handler_add(ih)
ihandler_t *	ih;
{
	extern struct MP_q_hd	isr_free_q, isr_req_q;
	int			index;
	ihandler_t *		p;
	ihandler_id_t		id;
	int			s;

	/*
	 * Sanity check
	*/
	if(ih == NULL || ih->ih_handler == NULL)
		return(NULL);

	/*
	 * The following lock results in the serialization of the
	 * interrupt handling funcions.  This is done to hide all
	 * of the locking requirements/semantics from device drivers.
	*/
	HANDLER_WRITE_LOCK();

	/*
	 * Call the resolver routine (if provided) to determine if the
	 * new handler can be added.  
	 * Note: The resolver might change the target vector/level if a
	 * conflict is found, by changing the appropriate information in
	 * the ih structure. 
	*/
	if (ih->ih_resolver != (int(*)())NULL)
	    if ((*ih->ih_resolver)(ih) < 0) {
			HANDLER_WRITE_UNLOCK();
			return (NULL);
		}

	/*
	 * Map the interrupt handler to an interrupt table index
	 * Note: The ih_resolver() may have remapped the targetted level
	*/
	id_set(&id, ih, 0);		/* temporary dummy id mapping */
	if ((index = id_index(&id)) < 0) {
		HANDLER_WRITE_UNLOCK();
		return (NULL);
	}

	/*
	 * Add interrupt handler into the interrupt table 
	*/
	s = splhigh();
	ITABLE_WRITE_LOCK(index);

	/* Insert handler into itbl */
	if(itbl[index].it_intr == NULL) {
		itbl[index].it_intr = ih;
	} else {
#if !MULTIMAX
		for(p = itbl[index].it_intr; p->ih_next != NULL; p=p->ih_next)
		;
		p->ih_next = ih;
#else
		ITABLE_WRITE_UNLOCK(index);
		splx(s);
		HANDLER_WRITE_UNLOCK();
		return (NULL);
#endif
	}
	ih->ih_next = NULL;
#if	SER_COMPAT
	if (ih->ih_funnel != FUNNEL_NULL) {
		if (ih->ih_free_queue == NULL)
			ih->ih_free_queue = &isr_free_q;
		if (ih->ih_req_queue == NULL)
			ih->ih_req_queue = &isr_req_q;
	}
#endif

	/* Create the real handle id */
	id_set(&ih->ih_id, ih, id_unique(index));

	/* Disable interrupt handler */
	ih->ih_state &= ~IH_STATE_ENABLED;

	ITABLE_WRITE_UNLOCK(index);
	splx(s);
	HANDLER_WRITE_UNLOCK();

	return(&ih->ih_id);
}

/*
 * NAME:	handler_del()
 * 
 * FUNCTION:	Deregister an interrupt handler from the interrupt dispatcher
 * 
 * EXEC ENV: 	Dynamic Device Drivers - This routine is called by a 
 *		dynamically device driver's deconfiguration routine to 
 *		deregister its ISRs.
 *
 * RETURNS:	On success zero is returned, else -1 on failure.
 * 
 */
int 
handler_del(id)
ihandler_id_t *	id;
{
	int		index;
	int		err;
	ihandler_t *	p;
	int		s;

	HANDLER_WRITE_LOCK();
	/*
	 * Map the interrupt handler to an interrupt table index
	*/
	if((index = id_index(id)) < 0) 
		return(-1);

	/*
	 * Deregister the interrupt handler
	*/
	err = 0;
	s = splhigh();
	ITABLE_WRITE_LOCK(index);

	/* Unlink the handler from the interrupt table */
	p = itbl[index].it_intr;
	if(p == NULL) 
		err = -1;
	else {
		if (!id_cmp(&p->ih_id, id)) {	
			itbl[index].it_intr = p->ih_next;
		} else {
			while (p->ih_next != NULL && id_cmp(&p->ih_next->ih_id, id))
				p = p->ih_next;
			if(p != NULL)
				p->ih_next = p->ih_next->ih_next;
			else
				err = -1;
		}
	}
						/* UNLOCK & RESPL */
	ITABLE_WRITE_UNLOCK(index);
	splx(s);
	HANDLER_WRITE_UNLOCK();

	return(err);
}


/* 
 * NAME:	handler_enable()
 *
 * FUNCTION:	Sets a interrupt handler to an enabled state 
 *
 * EXEC ENV:	This routine must be executed on a interrupt handler before 
 *		the dispatcher will recognize it.
 *
 * RETURNS:	On success zero is returned, else -1 on failure.
*/
int
handler_enable(id)
ihandler_id_t *	id;
{
	return(handler_state(id, IH_STATE_ENABLED, TRUE));
}


/* 
 * NAME:	handler_disable()
 *
 * FUNCTION:	Sets a interrupt handler to an disabled state 
 *
 * EXEC ENV:	This routine is used on a interrupt handler to tell
 *		the dispatcher not to call this handler.
 *
 * RETURNS:	On success zero is returned, else -1 on failure.
*/
int
handler_disable(id) 
ihandler_id_t *	id;
{
	return(handler_state(id, IH_STATE_ENABLED, FALSE));
}


/*
 * Locate a interrupt handler and set/clear bits in the state flag
*/
int
handler_state(id, state, set)
ihandler_id_t *	id;
int	state;
int	set;				/* TRUE==Set, FALSE==Clear */
{
	int		err;
	int		index;
	ihandler_t *	p;
	int		s;

	HANDLER_WRITE_LOCK();

	/*
	 * Map the interrupt handler to an interrupt table index
	*/
	if((index = id_index(id)) < 0) 
		return(-1);

	/*
	 * Find interrupt handler, and set/clear state
	*/
	err = 0;
	s = splhigh();
	ITABLE_WRITE_LOCK(index);

	/*
	 * Unlink the handler from the interrupt table handler chain.
	*/
	for(p = itbl[index].it_intr;
		p && id_cmp(&p->ih_id, id); p = p->ih_next)
		;
	if(p) {
		if(set == FALSE)
			p->ih_state &= ~state;
		else
			p->ih_state |=  state;
	} else
		err = -1;

	ITABLE_WRITE_UNLOCK(index);
	splx(s);
	HANDLER_WRITE_UNLOCK();

	return(err);
}


/*
 * Return the total number of interrupts received of type specified,
 * by scanning itbl for all ISR's of given type.
 */
unsigned long
handler_stats(type)
        int     type;
{
	extern   long           istray_cnt[], disintr_cnt[];
	register ihandler_t     *ih;
	register int            index;
	register unsigned long  cnt;
	int			s;

	cnt = 0;
	if (type & INTR_STRAY)
		for (index = 0; index < NUMSYSITBL; ++index)
			cnt += istray_cnt[index];
	if (type & INTR_DISABLED)
		for (index = 0; index < NUMSYSITBL; ++index)
			cnt += disintr_cnt[index];
	if (type & ~(INTR_STRAY|INTR_DISABLED)) {
		for (index = 0; index < NUMSYSITBL; ++index) {
			s = splhigh();
			ITABLE_READ_LOCK(index);
			for (ih = itbl[index].it_intr; ih; ih = ih->ih_next) {
				if (!(ih->ih_state & IH_STATE_ENABLED))
					continue;
				if (ih->ih_stats.ihs_type & type)
					cnt += ih->ih_stats.ihs_count;
			}
			ITABLE_READ_UNLOCK(index);
			splx(s);
		}
	}
	return (cnt);
}


/*
 * Default resolver function, used to find an itbl entry
 * acceptable both to system and to driver
 */
int
sys_resolve(ihp)
register struct ihandler	*ihp;
{
	register struct intr_list	*itblp;
	register long	i;
	register int	iflag = ihp->ih_rparam[0].intparam;
	register int	oflag;

	oflag = 0;
	/*
	 * If the level is out of range, return an error.
	 */
	if(ihp->ih_level >= NUMSYSITBL) {
		oflag = EFAULT;
		goto out;
	}

	itblp = &itbl[ihp->ih_level];

	/*
	 * No locks are required as the handler function
	 * environment has a serialization lock to prevent
	 * race conditions among possible writers.
	 *
	 * Check for no collision, and if the
	 * IH_VEC_MULTIPLE_OK flag is set.  If this is
	 * true, return with an O.K. status.  Otherwise,
	 * check for the IH_VEC_DYNAMIC_OK flag.
	 * If this flag is clear, we have an error.
	 * If VEC_DYNAMICOK is set, redefine the level
	 * to a free slot and reserve the slot.
	*/
	if ((itblp->it_intr == NULL) ||
		(iflag & IH_VEC_MULTIPLE_OK)) {
#if	MULTIMAX
		if (iflag & IH_VEC_MULTIPLE_OK) {
			oflag = EINVAL;
			goto out;
		}
#endif
		/*
		 * For multiple handlers/level, just return as we
		 * will sort the interrupt structure into the chain
		 * later.
		*/
		goto out;
	} else {
		if (!(iflag & IH_VEC_DYNAMIC_OK)) {
			oflag = EBUSY;
			goto out;
		}
		for (itblp = &itblp[0], i = 0; itblp < &itbl[NUMSYSITBL];
			itblp++, i++) {
			if(itblp->it_intr == (ihandler_t *)NULL)
				break;
		}
		if(itblp >= &itblp[NUMSYSITBL]) {	/* Overrun */
			printf("sys_resolve: Out Of Interrupt Table Slots\n");
			oflag = EAGAIN;
			goto out;
		}

		/*
		 * Set the interrupt level (vector) to be used.
		*/
		ihp->ih_level = i;
	}
out:
	ihp->ih_rparam[0].intparam = oflag;
	return ((oflag) ? -1 : 0);
}

#if	!MULTIMAX
/*
 * Implement priority sorting of intr structs used on systems
 * that allow multiple handlers per interrupt level (i.e., per
 * entry in itbl).
 *
 * The example here sorts ihnadler structs in descending order based
 * on the priority element.
 *
 * The sort algorithm used in other implementations is very
 * machine-specific.
 */
void
sort_intr(itblp, ihp)
register struct intr_list	*itblp;
register ihandler_t	*ihp;
{
        register ihandler_t	*curp, *lastp;

	lastp = (ihandler_t *)NULL;
	for(curp = itblp->it_intr; ; lastp = curp, curp = curp->ih_next) {
		/*
		 * Here's the sort: new entry goes after any old
		 * ones of same priority.
		*/
		if(!curp || (curp->ih_priority < ihp->ih_priority)) {
			if(lastp)
				lastp->ih_next = ihp;
			else
				itblp->it_intr = ihp;
			ihp->ih_next = curp;
			break;
		}
	}
	return;
}

#endif /*!MULTIMAX*/
