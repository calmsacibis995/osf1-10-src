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
static char *rcsid = "@(#)$RCSfile: str_support.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/09/02 16:23:54 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

#include <sys/secdefines.h>
#include <sys/param.h>
#include <sys/user.h>
#include <sys/file.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/str_support.h>

#if	SEC_BASE
#include <sys/security.h>
#endif

/*
 * Driver interface for cred passed to open procedures.
 */
drv_priv(cr)
	cred_t *cr;
{
	int error = EACCES;

	if (cr && cr != NOCRED)
#ifdef SEC_BASE
		PRIV_SUSER(cr, &u.u_acflag, SEC_ALLOWDACACCESS, 0, EACCES, error);
#else
		error = suser(cr,&u.u_acflag);
#endif
	return error;
}

int
streams_open_comm(size, q, devp, flag, sflag, cred)
	unsigned int size;
	queue_t *q;
	dev_t *devp;
	int flag, sflag;
	cred_t *cred;
{
	int error;
	dev_t dev;

	if (q == NULL)
		return ENXIO;
	if (q->q_ptr)
		return 0;
	if (sflag & MODOPEN) {
		if (size == 0)
			return 0;
		dev = NODEV;
		devp = &dev;
	} else if (devp == 0)
		return ENXIO;
	if (sflag & CLONEOPEN)
		flag |= O_DOCLONE;
	if (error = cdevsw_open_comm(flag, devp, size, (void **)&q->q_ptr))
		return error;
	OTHERQ(q)->q_ptr = q->q_ptr;
	return 0;
}

int
streams_open_ocomm(otherdev, size, q, devp, flag, sflag, cred)
	dev_t otherdev;
	unsigned int size;
	queue_t *q;
	dev_t *devp;
	int flag, sflag;
	cred_t *cred;
{
	int error;
	dev_t dev;

	if (q == NULL)
		return ENXIO;
	if (q->q_ptr)
		return 0;
	if (sflag & MODOPEN) {
		if (size == 0)
			return 0;
		return ENXIO;
	} else if (devp == 0)
		return ENXIO;
	if (sflag & CLONEOPEN)
		flag |= O_DOCLONE;
	if (error = cdevsw_open_ocomm(otherdev, flag, devp, size, (void **)&q->q_ptr))
		return error;
	OTHERQ(q)->q_ptr = q->q_ptr;
	return 0;
}

int
streams_close_comm(q, flag, cred)
	queue_t *q;
	int flag;
	cred_t *cred;
{
	void *p;

	if ((p = (void *)q->q_ptr) == 0)
		return 0;
	q->q_ptr = OTHERQ(q)->q_ptr = 0;
	return cdevsw_close_comm(p);
}

/*VARARGS1*/
void
mpsprintf(mp,va_alist)
mblk_t *mp;
va_dcl
{
	char *p, buf[512];
	int i, len;

	p = buf;
	mi_sprintf(buf, va_alist);
	len = strlen(buf) + 1;
	while (mp && len > 0) {
		if ((i = (mp->b_datap->db_lim - mp->b_wptr) + 1) > 0) {
			if (i > len) i = len;
			bcopy(p, mp->b_wptr, i);
			mp->b_wptr += i;
			p += i;
			len -= i;
		}
		mp = mp->b_cont;
	}
}

/*
 * tt_timeout_pending_cnt:
 *
 *	Return count of pending jobs (timeout requested, but not yet
 *	occurred) in named class.
 *
 * Arguments:
 *
 *	tmtab		- time table
 *	class		- class
 *
 * Returns:
 *
 *	count of pending jobs
 */

int
tt_timeout_pending_cnt(struct tt_timetab *tmtab, int class)
{
	struct tt_timejob	*jp;
	int			count = 0;
	int			s;

	ASSERT((0 <= class) && (class < tmtab->tb_entcnt));

	s = splstr();
	simple_lock(tmtab->tb_lock);
	jp = tmtab->tb_timents[class].te_jobs; 
	while (jp != (struct tt_timejob *) NULL) {
		++count;
		jp = jp->tj_next;
	}

	simple_unlock(tmtab->tb_lock);
	splx(s);
	return(count);
}

/*
 * tt_timeout:	
 *
 *	Arrange for a function to be called at a later time.
 *
 * Arguments:
 *
 *	tmtab		- timeout control table
 *	class		- index into timeout table (function argument for
 *			timeout(), among other things, is available there)
 *	arg		- argument for the function
 *	tmo		- time from now (in ticks) when function should happen
 *
 * Returns:
 *
 *	A positive integer that can be used in a later call to tt_untimeout().
 */

int
tt_timeout(struct tt_timetab *tmtab, int class, void *arg, int tmo)
{
	struct tt_timejob	*jp; 
	int			id, s;

	s = splstr();
	simple_lock(tmtab->tb_lock);

	jp = tt_alloc_job(tmtab);
	if (jp == (struct tt_timejob *) NULL) {
		simple_unlock(tmtab->tb_lock);
		splx(s);
		return(0);
	}
		
	jp->tj_func = tmtab->tb_timents[class].te_func;
	jp->tj_arg = arg;
	tt_insert_job(tmtab, class, jp);
	id = jp->tj_id = timeout(tt_qenable, jp, tmo);
	if (id == 0)
		if (tmtab->tb_timeout_panic)
			panic("tt_timeout: can't timeout");

	simple_unlock(tmtab->tb_lock);
	splx(s);
	return(id);
}

/*
 * tt_bufcall:	
 *
 *	Arrange for a function to be called when memory becomes available.
 *
 * Arguments:
 *
 *	tmtab		- timeout control table
 *	size		- size of memory request
 *	pri		- priority request
 *	class		- index into timeout table (function argument for
 *			bufcall(), among other things, is available there)
 *	arg		- argument for the function
 *
 * Returns:
 *
 *	A positive integer that can be used in a later call to tt_unbufcall().
 */

int
tt_bufcall(	struct tt_timetab *tmtab, 
		unsigned int size, 
		int pri, 
		int class, 
		void *arg
	  )
{
	struct tt_timejob	*jp; 
	int			id, s;

	s = splstr();
	simple_lock(tmtab->tb_lock);

	jp = tt_alloc_job(tmtab);
	if (jp == (struct tt_timejob *) NULL) {
		simple_unlock(tmtab->tb_lock);
		splx(s);
		return(0);
	}
		
	jp->tj_func = tmtab->tb_timents[class].te_func;
	jp->tj_arg = arg;
	tt_insert_job(tmtab, class, jp);
	id = jp->tj_id = bufcall(size, pri, tt_qenable, jp);
	if (id == 0)
		if (tmtab->tb_timeout_panic)
			panic("tt_bufcall: can't bufcall");

	simple_unlock(tmtab->tb_lock);
	splx(s);
	return(id);
}

/*
 * tt_cancel:	
 *
 *	Cancel a job previously arranged for callback on a queue.
 *
 * Arguments:
 *
 *	tmtab		- timeout control table
 *	jid		- job id (as returned by tt_timeout()).  If jid is zero,
 *			then it is ignored, and the class arg. takes effect
 *	class		- class number for which every outstanding request
 *			should be cancelled -- only applies when jid arg. 
 *			is zero.  If class is -1, then cancel every request
 *			in every class.
 *	proc		- procedure to do the actual cancelling
 *
 * Returns:
 *
 *	void
 */

void
tt_cancel(struct tt_timetab *tmtab, int jid, int class, void (*proc)())
{
	struct tt_timejob	*jp;
	int			s;

	s = splstr();
	simple_lock(tmtab->tb_lock);

	if (jid == 0) {
		int	i, start, end;

		if (class == -1) {
			start = 0;
			end = tmtab->tb_entcnt - 1;
		} else {
			start = class;
			end = class;
		}

		for (i = start; i <= end; ++i) {
			while (jp = tt_remove_job(tmtab, i, NULL)) {
				int	id = jp->tj_id;

				/* Null out id in case callback comes up 
				 * before we complete the cancel -- we 
				 * check for the zero job id in tt_qenable() 
				 * and act accordingly.
				 */
				jp->tj_id = 0;
				simple_unlock(tmtab->tb_lock);
				splx(s);

				(*proc)(id);

				s = splstr();
				simple_lock(tmtab->tb_lock);
				tt_free_job(tmtab, jp);
			}
		}
	} else {
		if (jp = tt_find_job(tmtab, jid)) {
			int	id;

			(void) tt_remove_job(tmtab, 0, jp);
			id = jp->tj_id;
			jp->tj_id = 0;	/* see comment above */
			simple_unlock(tmtab->tb_lock);
			splx(s);

			(*proc)(id);

			s = splstr();
			simple_lock(tmtab->tb_lock);
			tt_free_job(tmtab, jp);
		}
	}

	simple_unlock(tmtab->tb_lock);
	splx(s);
}

/* tt_qenable:
 *
 *	Post a job to a queue, then enable the queue.
 *
 * Argument:
 *
 *	jp		- the job to post
 *
 * Returns:
 *
 *	void
 */

PRIVATE_STATIC void
tt_qenable(struct tt_timejob *jp)
{
	struct tt_timetab	*tmtab;
	struct tt_timejob	*jp1;
	int			s;
	queue_t			*q;

	s = splstr();
	simple_lock(jp->tj_lock);
	
	tmtab = jp->tj_ent->te_tab;
	(void) tt_remove_job(tmtab, 0, jp);

	if (jp->tj_id == 0) {
		/* untimeout or unbufcall in progress -- just return */

		simple_unlock(jp->tj_lock);
		splx(s);
	} else {
		jp->tj_next = *tmtab->tb_ready;
		*tmtab->tb_ready = jp;
		q = jp->tj_ent->te_q;

		simple_unlock(jp->tj_lock);
		splx(s);
		qenable(q);
	}
}

/* tt_insert_job: 
 *
 *	Insert a job into a tt_timeout table.
 *
 * Arguments:
 *
 *	tmtab		- the time table
 *	class		- the class in which to insert
 *	jp		- the job to insert
 *
 * Returns:
 *
 *	void
 *
 * Assumes:
 *
 *	We are called at splstr() with tmtab->tb_lock already held.
 */

void
tt_insert_job(struct tt_timetab *tmtab, int class, struct tt_timejob *jp)
{
	struct tt_timent	*ent;

	jp->tj_ent = ent = &tmtab->tb_timents[class];
	jp->tj_next = ent->te_jobs;
	if (jp->tj_next)
		jp->tj_next->tj_prev = jp;
	jp->tj_prev = (struct tt_timejob *) NULL;
	ent->te_jobs = jp;
}

/* tt_remove_job:
 *
 *	Remove a job from a timeout table.  If "jp" is non-NULL, then
 *	remove that job.  Otherwise, remove the first job from the named
 *	class.  Return a pointer to the removed job in either case (the
 *	job is removed from its linked list, but not freed).
 *
 * Arguments:
 *
 *	tmtab		- the timeout table
 *	class		- class from which to remove the first job, only
 *			applicable if jp is NULL
 *	jp		- job to remove, if non-NULL
 *
 * Returns:
 *
 *	Pointer to removed job, or NULL if argument jp is NULL and there
 *	are no jobs in the argument class.
 *
 * Assumes:
 *
 *	We are called at splstr() with tmtab->tb_lock already held.
 */
	
PRIVATE_STATIC struct tt_timejob *
tt_remove_job(struct tt_timetab *tmtab, int class, struct tt_timejob *jp)
{
	struct tt_timent	*ent;

	if (jp == (struct tt_timejob *) NULL) { 
		/* remove first job in class */

		ent = &tmtab->tb_timents[class];
		jp = ent->te_jobs;
		if (jp != (struct tt_timejob *) NULL) {
			ent->te_jobs = jp->tj_next;
			if (ent->te_jobs)
				ent->te_jobs->tj_prev = 
						(struct tt_timejob *) NULL;
		}
		return(jp);
	} else {
		/* remove jp */

		if (jp->tj_prev)
			jp->tj_prev->tj_next = jp->tj_next;
		else {
			ent = jp->tj_ent;
			ent->te_jobs = jp->tj_next;
		}
		if (jp->tj_next)
			jp->tj_next->tj_prev = jp->tj_prev;
		return(jp);
	}
}
		
/* tt_find_job:
 *
 *	Find the job with the given job id.
 *
 * Arguments:
 *
 *	tmtab		- the timeout table
 *	jid		- job id
 *
 * Returns:
 *
 *	Pointer to found job, or NULL if job isn't present in tmtab.
 *
 * Assumes:
 *
 *	We are called at splstr() with tmtab->tb_lock already held.
 */

PRIVATE_STATIC struct tt_timejob *
tt_find_job(struct tt_timetab *tmtab, int jid)
{
	struct tt_timent	*ent;
	struct tt_timejob	*jp;
	int			i;

	for (i = 0; i < tmtab->tb_entcnt; ++i) {
		ent = &tmtab->tb_timents[i];
		for (jp = ent->te_jobs; jp != (struct tt_timejob *) NULL; jp = jp->tj_next ) {
			if (jp->tj_id == jid)
				return(jp);
		}
	}

	return((struct tt_timejob *) NULL);
}

/* tt_alloc_job:
 *
 *	Allocate a job from freelist (if supplied), else call supplied
 *	allocator routine.
 *
 * Argument:
 *
 *	tmtab		- timeout control table
 *
 * Returns:
 *
 *	Allocated job, or NULL if none available.
 *
 * Assumes:
 *
 *	We are called at splstr() with jp->tj_lock already held.
 */

struct tt_timejob *
tt_alloc_job(struct tt_timetab *tmtab)
{
	int			i;
	struct tt_timejob	*jp = (struct tt_timejob *) NULL;

	if (tmtab->tb_freelist != (struct tt_timejob *) NULL) {
		for (i = 0; i < tmtab->tb_freesz; ++i)
			if (tmtab->tb_freelist[i].tj_inuse == 0) {
				jp = &tmtab->tb_freelist[i];
				jp->tj_inuse = 1;
				goto out;
			}
	} else {
		/* tb_allocproc had better exist */
		jp = (*tmtab->tb_allocproc)(tmtab);
	}

out:
	if (jp != (struct tt_timejob *) NULL) {
		jp->tj_next = jp->tj_prev = (struct tt_timejob *) NULL;
	} else {
		if (tmtab->tb_alloc_panic)
			panic("tt_alloc_job: nothing to allocate");
	}
	return(jp);
}

/* tt_timeout_pending:
 *
 *	Return indication of whether timeout requests are outstanding in
 *	the named class.
 *
 * Arguments:
 *
 *	tmtab		- timeout control table
 *	class		- class to check
 *
 * Returns:
 *
 *	0 if nothing outstanding, non-zero if some job or jobs outstanding.
 */

tt_timeout_pending(struct tt_timetab *tmtab, int class)
{
	int	ret, s;

	s = splstr();
	simple_lock(tmtab->tb_lock);

	ASSERT((0 <= class) && (class <= tmtab->tb_entcnt));
	ret = (tmtab->tb_timents[class].te_jobs != (struct tt_timejob *) NULL);

	simple_unlock(tmtab->tb_lock);
	splx(s);
	return(ret);
}

/* tt_free_job:
 *
 *	Deallocate a timeout job descriptor.  If a free routine has been
 *	supplied by the driver, then use it, else just clear tj_inuse "bit".
 *
 * Arguments:
 *
 *	tmtab		- timeout control table
 *	jp		- job to free
 *
 * Returns:
 *
 *	void
 *
 * Assumes:
 *
 *	We are called at splstr() with jp->tj_lock already held.
 */

PRIVATE_STATIC void
tt_free_job(struct tt_timetab *tmtab, struct tt_timejob *jp)
{
        if (tmtab->tb_freeproc != (void (*)()) NULL)
                (*tmtab->tb_freeproc)(jp);
        else
                jp->tj_inuse = 0;
}

/*
 * tt_get_job:
 *
 *	Get timeout job off head of ready queue.  The job has already been
 *	removed from its class; we just need to free it here.
 *
 * Argument:
 *
 *	jp		- the job to run.
 *
 * Returns:
 *
 *	void
 *
 * Assumes:
 *
 *	We are called at splstr() with jp->tj_lock already held.
 */

PRIVATE_STATIC void
tt_get_job(struct tt_timejob *jp, void (**func)(), void **arg)
{
	*func = jp->tj_func;
	*arg = jp->tj_arg;

	*jp->tj_ent->te_tab->tb_ready = jp->tj_next;
	tt_free_job(jp->tj_ent->te_tab, jp);
}

/*
 * tt_init_timetab:
 *
 *	Initialize timeout control structure.
 *
 * Arguments:
 *
 *	td		- table descriptor 
 *	unit		- device minor number
 *	wq		- device write queue
 *	lock		- timeout spin lock
 *	ready_loc	- address for recording ready jobs
 *
 * Returns:
 *
 *	The filled in (struct tt_timetab).
 */

struct tt_timetab *
tt_init_timetab(	struct tt_tabdesc 	*td, 
			int 			unit, 
			queue_t 		*wq, 
			simple_lock_t		lock,
			struct tt_timejob	**ready_loc
	)
			
{
	int			i;
	struct tt_timetab	*tmtab;
	

	tmtab = &td->tbd_tabs[unit];
	simple_lock_init(lock);
	tmtab->tb_lock = lock;
	tmtab->tb_timents = &td->tbd_ents[unit * td->tbd_entcnt];
	for (i = 0; i < td->tbd_entcnt; ++i) {
		struct tt_timent	*ent;
		struct tt_entdesc	*entd;

		ent = &tmtab->tb_timents[i];
		entd = &td->tbd_entdesc[i];
		ent->te_tab = tmtab;
		ent->te_func = entd->ted_func;
		ent->te_q = (entd->ted_qtype == TT_TIMEOUT_WQ ? wq : RD(wq));
		ent->te_jobs = (struct tt_timejob *) NULL;
	}
	tmtab->tb_entcnt = td->tbd_entcnt;
	tmtab->tb_ready = ready_loc;

	if (td->tbd_freelist) {
		tmtab->tb_freelist = &td->tbd_freelist[unit * td->tbd_freesz];
		tmtab->tb_freesz = td->tbd_freesz;
		for (i = 0; i < tmtab->tb_freesz; ++i) {
			tmtab->tb_freelist[i].tj_inuse = 0;
			tmtab->tb_freelist[i].tj_lock = lock;
		}
	} else {
		tmtab->tb_allocproc = td->tbd_allocproc;
		tmtab->tb_freeproc = td->tbd_freeproc;
	}

	tmtab->tb_alloc_panic = td->tbd_alloc_panic;
	tmtab->tb_timeout_panic = td->tbd_timeout_panic;

	return(tmtab);
} 

/*
 * tt_run_jobs:
 *
 *	Run ready jobs, if any exist.
 *
 * Argument:
 *
 *	tmtab		- timeout control structure
 *
 * Returns:
 *
 *	count of jobs run
 *
 * Assumes:
 *
 *	We are called by a streams service routine.
 */

tt_run_jobs(struct tt_timetab *tmtab)
{
	int     	s;
	int		count = 0;
	void		(*func)();
	void		*arg;

	s = splstr();
	simple_lock(tmtab->tb_lock);

	while (*tmtab->tb_ready) {
		tt_get_job(*tmtab->tb_ready, &func, &arg);
		simple_unlock(tmtab->tb_lock);
		splx(s);

		++count;
		(*func)(arg);

		s = splstr();
		simple_lock(tmtab->tb_lock);
	}
	simple_unlock(tmtab->tb_lock);
	splx(s);

	return(count);
}

void 
tt_untimeout(struct tt_timetab *tmtab, int jid, int class)
{
	tt_cancel(tmtab, jid, class, untimeout);
}

void
tt_unbufcall(struct tt_timetab *tmtab, int jid, int class)
{
	tt_cancel(tmtab, jid, class, unbufcall);
}
