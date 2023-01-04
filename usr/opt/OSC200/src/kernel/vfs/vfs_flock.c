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
static char	*sccsid = "@(#)$RCSfile: vfs_flock.c,v $ $Revision: 4.2.18.6 $ (DEC) $Date: 1993/11/15 21:03:53 $";
#endif 
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */

#include <sys/param.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/mode.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/mount.h>
#include <sys/flock.h>
#include <sys/lock_types.h>
#include <mach/vm_param.h>
#include <kern/zalloc.h>
#include <rpc/rpc.h>
#include <klm/lockmgr.h>
#include <klm/klm_prot.h>
#include <nfs/rnode.h>
#include <sys/vfs_proto.h>

/* region types */
#define	S_BEFORE	010
#define	S_START		020
#define	S_MIDDLE	030
#define	S_END		040
#define	S_AFTER		050
#define	E_BEFORE	001
#define	E_START		002
#define	E_MIDDLE	003
#define	E_END		004
#define	E_AFTER		005

#define	SLEEP(ptr,retval)						\
MACRO_BEGIN								\
		assert_wait((vm_offset_t)&ptr->stat.blkpid, TRUE);	\
		SLEEPLCK_UNLOCK();					\
		retval = tsleep((caddr_t)0, (PZERO+1)|PCATCH, "flock", 0); \
MACRO_END

#define	WAKEUP(ptr)							\
MACRO_BEGIN								\
		thread_wakeup((vm_offset_t)&ptr->stat.blkpid);		\
MACRO_END

#define l_end 		l_len
#define GRANT_LOCK_FLAG 0xf             /* flag in eld.l_xxx */

struct	flckinfo flckinfo;		/* configuration and acct info */
struct	flino	*fids;			/* file id head list */
struct	flino	sleeplcks;		/* head of chain of sleeping locks */
struct	filock	*sleeplcks_tail = NULL;
struct	flino	grantlcks;		/* head of chain of l_xxx granted lcks */
struct	flino	klmlcks;		/* head of chain of klm call locks */

#ifdef i386
struct filock *xenix_blocked();	/* forward declaration */
#endif

int dbg_lock=0;
/*
 * Locking precedence:
 * I.e. if you hold one of these locks, you can take lock below the line,
 * but not locks above it.  E.g. If you hold FIDS_LOCK, you can take
 * FLINO_LOCK.
 *
 *	FICHAIN_LOCK	(blocking lock)
 *	SLEEPLCK_LOCK, GRANTLCK_LOCK, KLMLCK_LOCK (blocking locks)
 *	FIDS_LOCK	(spin lock)
 *	FLCKINFO_LOCK	(spin lock)
 *	FLINO_LOCK	(spin lock)
 *
 * These are the locks associated with the different structures related to
 * the flock package.  The lock with the highest precedence, fichain_lock is
 * a mutex lock contained in the flino structure.  Each flino structure has
 * a chain of file locks associated with it (filock structures).  The
 * fichain_lock protects this chain.  The lock is a blocking lock because it
 * is held across blocking operations.  We want to hold it across these
 * operations to protect against races that can occur in flckadj() and other
 * functions that modify the chain, like delflck().  Using a blocking lock
 * allows us to hold that lock while flckadj() travels down the chain.
 * We chose to use a mutex lock rather than a read/write lock because most
 * of the time we traverse the chain in order to modify it, so a read/write
 * lock would not gain us much additional parallelism.  The parallelism through
 * this code is very coarse.  The fichain_lock also protects the entire filock
 * structure for each filock on its chain.  This is very coarse grain, but
 * only will have contention on multiple fcntl's going on with a single file.
 *
 * The sleeplck_lock is really an fichain_lock that protects the global
 * sleeplck structure.  The same locking holds true here, as it does
 * above.  The sleeplck_lock can be acquired by a thread holding a fichain
 * lock.  The lockmanager uses this list, but the entries are placed on
 * behalf of processes that are sleeping on other systems.
 *
 * The grantlck_lock and klmlck_lock are fichain_locks that protects the
 * global grantlck and klmlck structures.  The same locking holds true here.
 * These locks can be acquired by a thread holding a fichain lock.
 *
 * The fids_lock has the next highest precedence.  This protects the global
 * fids list.  Fids is the head of a list of file ids (flino structs).  All
 * modifications to this list must be done under the fids_lock.  This is a
 * spin lock and is usually held for a relatively short duration, only to
 * traverse the list and do an insertion or deletion.
 *
 * The flckinfo_lock protects flckinfo structures.  The global 'flckinfo'
 * variable is protected by this lock.  This lock is used for very short
 * periods of time, only to increment or decrement counts, and is therefore
 * a spin lock.
 *
 * The flino_lock protects the rest of the flino structure.  That is, it
 * protects the refcnt field.  It is a spin lock.  It is infrequently used
 * and held for very short periods of time.
 *
 * It was necessary to add a new field to the filock structure.
 * The 'flip' field is a backpointer to the flino structure whose file
 * lock chain we are on.  This is necessary because we have to take
 * the fichain_lock when we remove ourselves from that list.
 * When we are in setflck(), we check to see if our new lock overlaps
 * (or is 'blocked') by another lock in the chain.  If it is, blocked()
 * returns that filock structure with the filock_lock LOCKED.  We then
 * record the timestamp and increment the sleepcnt in setflck() before
 * releasing the filock_lock.  There is the potential in setflck() that
 * we will block, waiting for the filock that is 'blocking' us to get
 * deleted or modified in some way.  When that happens, a 'wakeup' is posted
 * to wake us up.  However, in the MP case, it is possible that we are
 * racing with another thread who is doing the wakeup.  Therefore, we may
 * have found the blocking filock, but the other thread may post the wakeup
 * before we actually go to sleep.  The 'timestamp' field protects against
 * this.  The timestamp field is incremented everytime a wakeup is posted
 * on this filock.  Therefore, we record the current timestamp under lock,
 * and if it has changed before we want to go to sleep, we know the wakeup
 * has been posted, and we just continue.  The 'sleepcnt' field protects us
 * from a nasty race between setflck() and delflck().  One of the places
 * where a wakeup is posted is in delflck(), when an filock structure gets
 * deleted from the list and its memory is freed.  If blocked() returns us
 * this structure, and we are using it to check various conditions, and this
 * structure suddenly gets freed in the middle of this, bad things can happen.
 * So, again, when blocked returns us the locked filock structure in setflck(),
 * we increment the sleepcnt field.  In delflck, the other thread checks this
 * field and if it is set, that thread will decrement the count and post the
 * wakeup.  In setflck, when we either awaken or don't sleep due to the
 * timestamp changing, we also call delflck(), to actually complete the deletion
 * of this structure (or to post a wakeup, should another thread have found it
 * in the meantime).
 */
udecl_simple_lock_data(, fids_lock)
#define	FIDS_LOCK()		usimple_lock(&fids_lock)
#define	FIDS_UNLOCK()		usimple_unlock(&fids_lock)
#define	FIDS_LOCKINIT()		usimple_lock_init(&fids_lock)

#if	UNIX_LOCKS
#define SLEEPLCK_LOCKINIT()        lock_init2(&sleeplcks.fichain_lock, TRUE, LTYPE_FICHAIN)
#define SLEEPLCK_LOCK()            lock_write(&sleeplcks.fichain_lock)
#define SLEEPLCK_UNLOCK()          lock_write_done(&sleeplcks.fichain_lock)
#define SLEEPLCK_LOCKED()          (sleeplcks.fichain_lock.want_write == TRUE)
#define SLEEPLCK_LOCK_THREAD()     ((thread_t)sleeplcks.fichain_lock.lthread)
#define SLEEPLCK_LOCK_OWNER()       (SLEEPLCK_LOCK_THREAD() == current_thread())
#define SLEEPLCK_LOCK_HOLDER()      (SLEEPLCK_LOCKED()&&SLEEPLCK_LOCK_OWNER())
#define GRANTLCK_LOCKINIT()        lock_init2(&grantlcks.fichain_lock, TRUE, LTYPE_FICHAIN)
#define GRANTLCK_LOCK()            lock_write(&grantlcks.fichain_lock)
#define GRANTLCK_UNLOCK()          lock_write_done(&grantlcks.fichain_lock)
#define GRANTLCK_LOCKED()          (grantlcks.fichain_lock.want_write == TRUE)
#define GRANTLCK_LOCK_THREAD()     ((thread_t)grantlcks.fichain_lock.lthread)
#define GRANTLCK_LOCK_OWNER()       (GRANTLCK_LOCK_THREAD() == current_thread())
#define GRANTLCK_LOCK_HOLDER()      (GRANTLCK_LOCKED()&&GRANTLCK_LOCK_OWNER())
#define KLMLCK_LOCKINIT()        lock_init2(&klmlcks.fichain_lock, TRUE, LTYPE_FICHAIN)
#define KLMLCK_LOCK()            lock_write(&klmlcks.fichain_lock)
#define KLMLCK_UNLOCK()          lock_write_done(&klmlcks.fichain_lock)
#define KLMLCK_LOCKED()          (klmlcks.fichain_lock.want_write == TRUE)
#define KLMLCK_LOCK_THREAD()     ((thread_t)klmlcks.fichain_lock.lthread)
#define KLMLCK_LOCK_OWNER()       (KLMLCK_LOCK_THREAD() == current_thread())
#define KLMLCK_LOCK_HOLDER()      (KLMLCK_LOCKED()&&KLMLCK_LOCK_OWNER())
#else
#define SLEEPLCK_LOCKINIT()      
#define SLEEPLCK_LOCK()           
#define SLEEPLCK_UNLOCK()          
#define SLEEPLCK_LOCK_HOLDER()      
#define GRANTLCK_LOCKINIT()      
#define GRANTLCK_LOCK()           
#define GRANTLCK_UNLOCK()          
#define GRANTLCK_LOCK_HOLDER()
#define KLMLCK_LOCKINIT()      
#define KLMLCK_LOCK()           
#define KLMLCK_UNLOCK()          
#define KLMLCK_LOCK_HOLDER()
#endif

#if	MACH
/*
 * Zones used by lock package for dynamic data structures
 */
zone_t lockfile_zone;
zone_t lockrec_zone;
#endif

/*
 * Patchable configuration variables from vfs_conf.c and param.c.
 */
extern int nlock_record;
extern int nvnode;		/* Used to allow all files to be locked */

static void purge_fs_list(struct flino *, fsid_t *);

/* find file id */

struct flino *
findfid(vp)
struct vnode *vp;
{
	register struct flino *flip;

	FIDS_LOCK();
	flip = fids;
	while (flip != NULL) {
		if (flip->vp == vp) {
			FLINO_LOCK(flip);
			flip->fl_refcnt++;
			FLINO_UNLOCK(flip);
			break;
		}
		flip = flip->next;
	}
	FIDS_UNLOCK();
	return (flip);
}

struct flino *
allocfid(vp)
struct vnode *vp;
{
	struct flino *flip, *flinop;

	ZALLOC(lockfile_zone, flip, struct flino *);
	if (flip != NULL) {
		FLCKINFO_LOCK(&flckinfo);
		if (++flckinfo.filcnt > flckinfo.filmax)
			flckinfo.filmax = flckinfo.filcnt;
		++flckinfo.filtot;
		FLCKINFO_UNLOCK(&flckinfo);

		/* set up file identifier info */
		flip->fl_refcnt = 1;
		flip->fl_flck = NULL;
		FLINO_LOCKINIT(flip);
		FICHAIN_LOCKINIT(flip);

		/* insert into allocated file identifier list */
		FIDS_LOCK();
#if	UNIX_LOCKS
		/*
		 * Someone else may have already inserted a flino entry onto
		 * the list with the same handle information.  Recheck the
		 * list to make sure it wasn't inserted while we were setting
		 * ours up.  If it was, deallocate ours and return the other
		 * one.
		 */
		
		flinop = fids;
		while (flinop != NULL) {
			if (flinop->vp == vp) {
				FLINO_LOCK(flinop);
				flinop->fl_refcnt++;
				FLINO_UNLOCK(flinop);
				FLCKINFO_LOCK(&flckinfo);
				--flckinfo.filcnt;
				--flckinfo.filtot;
				FLCKINFO_UNLOCK(&flckinfo);
				ZFREE(lockfile_zone, flip);
				FIDS_UNLOCK();
				return(flinop);

			}
			flinop = flinop->next;
		}
#endif
		if (fids != NULL)
			fids->prev = flip;
		flip->vp = vp;
		VREF(vp);
		flip->next = fids;
		flip->prev = NULL;
		fids = flip;
		FIDS_UNLOCK();

	}
	return (flip);
}

void freefid(flip)
struct flino *flip;
{
	FLINO_LOCK(flip);
	if (flip->fl_refcnt == 1 && flip->fl_flck == NULL) {
		FLINO_UNLOCK(flip);
		FIDS_LOCK();
		if (flip->fl_refcnt != 1 || flip->fl_flck) {
			FIDS_UNLOCK(); /* Someone snuck in, can't release */
			return;
		}
		if (flip->prev == NULL)
			fids = flip->next;
		if (flip->next != NULL)
			flip->next->prev = flip->prev;
		if (flip->prev != NULL)
			flip->prev->next = flip->next;
		FIDS_UNLOCK();
		FLCKINFO_LOCK(&flckinfo);
		--flckinfo.filcnt;
		FLCKINFO_UNLOCK(&flckinfo);
		vrele(flip->vp); /* Matches allocfid() */
		ZFREE(lockfile_zone, flip);
	} else {
		--flip->fl_refcnt;
		FLINO_UNLOCK(flip);
	}
}
	

/* build file lock free list */

flckinit()
{
	lockfile_zone = zinit(sizeof(struct flino),
			nvnode * sizeof(struct flino),
			PAGE_SIZE, "lockfile");
	if (lockfile_zone == (zone_t) NULL)
		panic("flckinit: no file zone");
	/* set lockfile_zone !pageable, !sleepable, exhaustible, collectable */
	zchange(lockfile_zone, FALSE, FALSE, TRUE, TRUE);

	flckinfo.filcnt = 0;

	lockrec_zone = zinit(sizeof(struct filock),
			nlock_record * sizeof(struct filock),
			PAGE_SIZE, "lockrec");
	if (lockrec_zone == (zone_t) NULL)
		panic("flckinit: no rec zone");
	/* set lockrec_zone !pageable, !sleepable, exhaustible, collectable */
	zchange(lockrec_zone, FALSE, FALSE, TRUE, TRUE);

	flckinfo.reccnt = 0;
	FLCKINFO_LOCKINIT(&flckinfo);
	FIDS_LOCKINIT();
	SLEEPLCK_LOCKINIT();
	GRANTLCK_LOCKINIT();
	KLMLCK_LOCKINIT();
}

/* insert lock after given lock using locking data */

#if	UNIX_LOCKS
/*
 * lckdat is a new lock.  No one else can know about it, therefore, it doesn't
 * require any locking in the MP case.
 */
#endif
struct filock *
insflck(flip, lckdat, fl, vp, class, upid)
struct	flino	*flip;
struct	filock	*fl;
struct	eflock	*lckdat;
struct  vnode   *vp;
int class;  /* FILE_LOCK or LOCKMGR */
pid_t upid;
{
	register struct filock *new;
	struct	filock	*f;
	int error;

	LASSERT(FICHAIN_LOCK_HOLDER(flip));
	ZALLOC(lockrec_zone, new, struct filock *);
	if (new != NULL) {
		FLCKINFO_LOCK(&flckinfo);
		if (++flckinfo.reccnt > flckinfo.recmax)
			flckinfo.recmax = flckinfo.reccnt;
		++flckinfo.rectot;
		FLCKINFO_UNLOCK(&flckinfo);
		new->set = *lckdat;
		if (class != LOCKMGR)
			new->set.l_pid = upid;
		new->class = class;
		new->stat.wakeflg = 0;
		new->vp = vp; /* for sleeplcks and klmlcks chains */
		new->flip = flip;
		if (flip == &sleeplcks) {  /* add to end of sleep q */
			new->next = NULL;
			new->prev = sleeplcks_tail;
			if (sleeplcks.fl_flck == NULL)
				sleeplcks.fl_flck = new;
			else
				sleeplcks_tail->next = new;
			sleeplcks_tail = new;
		} else if (fl == NULL) {  /* add to beginning of q */
			new->next = flip->fl_flck;
			if (flip->fl_flck != NULL)
				flip->fl_flck->prev = new;
			flip->fl_flck = new;
			new->prev = NULL;
		} else {     /* add to specified location within vp q */
			new->next = fl->next;
			if (fl->next != NULL)
				fl->next->prev = new;
			fl->next = new;
			new->prev = fl;
		}
		if (dbg_lock) {
			printf("*** insflck added to ");
			if (flip == &sleeplcks) 
				printf("sleep q ***\n");
			else if (flip == &grantlcks)
				printf("grant q ***\n");
			else if (flip == &klmlcks)
				printf("klm q ***\n");
			else printf("vp=0x%x ***\n", vp);
			print_flino(flip);
		}
		/*
		 * Check for mandatory lock on vnode if it is local
		 * and has been granted. 
		 */
		if ((flip != &sleeplcks) && (class != LOCKMGR)) {
			VOP_LOCKCTL(vp, NULL, ENFFLCK, NULL, 0, (off_t)0, error);
#ifdef i386
			if (lckdat->l_type & F_ENFRCD)
				set_vxenix(vp);
#endif
		}
	}
	return (new);
}

/* delete lock */

delflck(flip, fl, vp)
struct flino *flip;
struct filock *fl;
struct vnode  *vp;
{
	LASSERT(FICHAIN_LOCK_HOLDER(flip));

	if (fl == sleeplcks_tail)
		sleeplcks_tail = fl->prev;

	if (fl->prev != NULL)
		fl->prev->next = fl->next;
	else
		flip->fl_flck = fl->next;
	if (fl->next != NULL)
		fl->next->prev = fl->prev;
	/*
	 * Don't touch vnode if only dealing with sleeplcks
	 */
	if ((flip != &sleeplcks) && (flip != &grantlcks) &&
	    (flip != &klmlcks) && (flip->fl_flck == NULL)) {
		/* no locks on file */
	        clear_vlocks(vp);
#ifdef i386
		clear_vxenix(vp);
#endif
	}

	if (dbg_lock) {
		printf("### delflck removed from ");
		if (flip == &sleeplcks) 
			printf("sleep q ###\n");
		else if (flip == &grantlcks)
			printf("grant q ###\n");
		else if (flip == &klmlcks)
			printf("klm q ###\n");
		else printf("vp=0x%x ###\n", vp);
		print_flino(flip);
	}
	FLCKINFO_LOCK(&flckinfo);
	--flckinfo.reccnt;
	FLCKINFO_UNLOCK(&flckinfo);

	ZFREE(lockrec_zone, fl);
}

/*
 * Routine to remove a pid from the sleep list.  This is called when SLEEP is
 * interrupted, not when chk_granted does a WAKEUP.  However, there is a race
 * between the two that tries to lead to removing the sleep list entry twice:
 *
 *  Process:				Others:
 *  Attempt to take a lock, find it is
 *  used elsewhere, add us to sleeplcks
 *  and goto sleep.
 *
 *					psignal: Interrupt sleep
 *
 *					setflck/flckadj/chk_granted:
 *					someone does unlock, chk_granted
 *					gives lock to Process, takes it
 *					off sleeplcks, and does wakeup.
 *					(The wakeup is a nop since the
 *					process was woken by psignal.)
 *
 *  Resume after interrupted SLEEP(),
 *  believe we need to take us off
 *  sleeplcks, but we already are off.
 *
 * Therefore, we can't simply delete the filock we put on the sleep list
 * because it may be gone.  (Worse, it could be reallocated and *on* the
 * sleep list again!)  We need to search the list to see if our pid is on
 * the list, and then we can remove that structure.
 *
 * One might suggest that chk_granted() leave the sleep list alone, and have
 * the process take itself off the list after SLEEP() returns.  However, that
 * would lead to chk_granted() putting duplicate filock structures on the
 * vnode list so we'd have to check for that or add a field to filock to
 * say not to add to the flino list.
 *
 * It all seems easiest to to deal with this relatively rare case in the
 * relatively rare code path.  Therefore we search the sleep list after
 * interrupted sleeps.
 */
void delsleeplck(pid_t pid) {
	register struct filock *sf;

	SLEEPLCK_LOCK();
	for (sf=sleeplcks.fl_flck; sf != NULL; sf=sf->next)
		if ((sf->set.l_pid == pid) && (sf->class == FILE_LOCK)) {
			if (dbg_lock)
				printf("delsleeplck found pid on sleep list.\n");
			delflck(&sleeplcks, sf, NULL);
			break;
		}
	SLEEPLCK_UNLOCK();
	if (!sf && dbg_lock)
		printf("delsleeplck did not find pid on sleep list.\n");
}

/*
 * regflck sets the type of span of this (un)lock relative to the specified
 * already existing locked section.
 * There are five regions:
 *
 *  S_BEFORE        S_START         S_MIDDLE         S_END          S_AFTER
 *     010            020             030             040             050
 *  E_BEFORE        E_START         E_MIDDLE         E_END          E_AFTER
 *      01             02              03              04              05
 * 			|-------------------------------|
 *
 * relative to the already locked section.  The type is two octal digits,
 * the 8's digit is the start type and the 1's digit is the end type.
 */
/*
 * ld is a new lock.  No one else can know about it, therefore, it doesn't
 * require any locking in the MP case.
 */
int
regflck(ld, flp)
struct eflock *ld;
struct filock *flp;
{
	register int regntype;

	LASSERT(FICHAIN_LOCK_HOLDER(flp->flip));
	if (ld->l_start > flp->set.l_start) {
		if (ld->l_start > flp->set.l_end) {
			return(S_AFTER|E_AFTER);
		} else if (ld->l_start == flp->set.l_end) {
			return(S_END|E_AFTER);
		} else
			regntype = S_MIDDLE;
	} else if (ld->l_start == flp->set.l_start)
		regntype = S_START;
	else
		regntype = S_BEFORE;

	if (ld->l_end > flp->set.l_start) {
		if (ld->l_end > flp->set.l_end)
			regntype |= E_AFTER;
		else if (ld->l_end == flp->set.l_end)
			regntype |= E_END;
		else
			regntype |= E_MIDDLE;
	} else if (ld->l_end == flp->set.l_start)
		regntype |= E_START;
	else
		regntype |= E_BEFORE;

	return (regntype);
}

/* Adjust file lock from region specified by 'ld' starting at lock 'insrtp' */

/*
 * ld is a new lock.  No one else can know about it, therefore, it doesn't
 * require any locking in the MP case.
 */
flckadj(flip, insrtp, ld, vp, class, upid)
struct flino	*flip;
struct filock	*insrtp;
struct eflock	*ld;
struct vnode    *vp;
int class; /* Lock class: FILE or LOCKMGR */
pid_t	upid;
{
	struct	eflock	td;			/* lock data for severed lock */
	struct	filock	*flp, *nflp, *tdi, *tdp;
	int	insrtflg, rv = 0;
	int	regtyp;
	pid_t	rpid=0;
	u_int	rsys=0;

	insrtflg = (ld->l_type != F_UNLCK) ? 1 : 0;

	LASSERT(FICHAIN_LOCK_HOLDER(flip));
	nflp = (insrtp == NULL) ? flip->fl_flck : insrtp;
	if (class == LOCKMGR) {
		upid = ld->l_pid;
		rpid = ld->l_rpid;
		rsys = ld->l_rsys;
	}
	while (flp = nflp) {
		nflp = flp->next;
		if ((flp->set.l_pid == upid) && (flp->set.l_rpid == rpid) &&
		    (flp->set.l_rsys == rsys)) {

			regtyp = regflck(ld, flp);

			/* release already locked region if necessary */

			switch (regtyp) {
			case S_BEFORE|E_BEFORE:
				nflp = NULL;
				break;
			case S_BEFORE|E_START:
				if (ld->l_type == flp->set.l_type) {
					ld->l_end = flp->set.l_end;
					if (insrtp == flp)
						insrtp = flp->prev;
					delflck(flip, flp, vp);
				} else
				nflp = NULL;
				break;
			case S_START|E_END:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type) 
					return(rv);
				/* fall thru */
			case S_START|E_AFTER:
				insrtp = flp->prev;
				delflck(flip, flp, vp);
				/*
				 * let setflck know it should call 
				 * chk_granted to do potential 
				 * wakeups 
				 */
				rv = 1;
				break;
			case S_BEFORE|E_END:
				if (ld->l_type == flp->set.l_type)
					nflp = NULL;
				/* fall thru */
			case S_BEFORE|E_AFTER:
				if (insrtp == flp) 
					insrtp = flp->prev;
				delflck(flip, flp, vp);
				/*
				 * let setflck know it should call 
				 * chk_granted to do potential 
				 * wakeups 
				 */
				rv = 1;
				break;
			case S_BEFORE|E_MIDDLE:
				if (ld->l_type == flp->set.l_type)
					ld->l_end = flp->set.l_end;
				else {
					/* setup piece after end of (un)lock */
					td = flp->set;
					td.l_start = ld->l_end;
					tdp = tdi = flp;
					do {
						if (tdp->set.l_start < ld->l_start)
							tdi = tdp;
						else
							break;
					} while (tdp = tdp->next);
					if (insflck(flip, &td, tdi, vp, class, upid) == NULL) {
						return(ENOLCK);
					}
				}
				if (insrtp == flp)
					insrtp = flp->prev;
				delflck(flip, flp, vp);
				/*
				 * let setflck know it should call 
				 * chk_granted to do potential 
				 * wakeups 
				 */
				rv = 1;
				nflp = NULL;
				break;
			case S_START|E_MIDDLE:
			case S_MIDDLE|E_MIDDLE:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return(rv);
				/* setup piece after end of (un)lock */
				td = flp->set;
				td.l_start = ld->l_end;
				tdp = tdi = flp;
				do {
					if (tdp->set.l_start < ld->l_start)
						tdi = tdp;
					else
						break;
				} while (tdp = tdp->next);
				if (insflck(flip, &td, tdi, vp, class, upid) == NULL) {
					return(ENOLCK);
				}
				if (regtyp == (S_MIDDLE|E_MIDDLE)) {
					/* setup piece before (un)lock */
					flp->set.l_end = ld->l_start;
					insrtp = flp;
				} else {
					insrtp = flp->prev;
					delflck(flip, flp, vp);
				}
				/*
				 * let setflck know it should 
				 * call chk_granted to do 
				 * potential wakeups 
				 */
				rv = 1;
				nflp = NULL;
				break;
			case S_MIDDLE|E_END:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return(rv);
				flp->set.l_end = ld->l_start;
				/*
				 * let setflck know it should call 
				 * chk_granted to do potential wakeups 
				 */
				rv = 1;
				insrtp = flp;
				break;
			case S_MIDDLE|E_AFTER:
			case S_END|E_AFTER:
				if (ld->l_type == flp->set.l_type) {
					ld->l_start = flp->set.l_start;
					insrtp = flp->prev;
					delflck(flip, flp, vp);
				} else {
					flp->set.l_end = ld->l_start;
					/*
				 	 * let setflck know it should 
				 	 * call chk_granted to do 
					 * potential wakeups 
				 	 */
					rv = 1;
					insrtp = flp;
				}
				break;
			case S_AFTER|E_AFTER:
				insrtp = flp;
				break;
			}
		} else {
			if (flp->set.l_start > ld->l_end)
				nflp = NULL;
		}
	}

	if (insrtflg) {
		if (flp = insrtp) {
			do {
				if (flp->set.l_start < ld->l_start)
					insrtp = flp;
				else
					break;
			} while (flp = flp->next);
		}
		if (insflck(flip, ld, insrtp, vp, class, upid) == NULL)
			rv = ENOLCK;
	}

	if (dbg_lock) {
		printf("@@@ flckadj: rv=%d @@@\n", rv);
	}
	return (rv);
}

/*
 * blocked checks whether a new lock (lckdat) would be
 * blocked by a previously set lock owned by another process.
 * When blocked is called, 'flp' should point
 * to the record from which the search should begin.
 * Insrt is set to point to the lock before which the new lock
 * is to be placed.
 */
#if	UNIX_LOCKS
/*
 * blocked returns with the lock LOCKED on the filock structure.
 * It is up to the calling routine to unlock the structure.
 *
 * blocked is called only from locked, getflck or setflck, with lckdat being a
 * new lock.  Since no one else can know of lckdat, it requires no locking.
 */
#endif
struct filock *
blocked(flp, lckdat, insrt, class, upid)
struct filock *flp;
struct eflock *lckdat;
struct filock **insrt;
int class; /* Lock class: FILE or LOCKMNGR */
pid_t upid;
{
	struct filock *f;
	pid_t	rpid=0;
	u_int   rsys=0;

	if (class == LOCKMGR) {
		upid = lckdat->l_pid;
		rpid = lckdat->l_rpid;
		rsys = lckdat->l_rsys;
	}

	*insrt = NULL;
	for (f = flp; f != NULL; ) {
		if (f->set.l_start < lckdat->l_start)
			*insrt = f;
		else {
			break;
		}
		if ((f->set.l_pid == upid) && (f->set.l_rpid == rpid) &&
		    (f->set.l_rsys == rsys)) {
			if (lckdat->l_start <= f->set.l_end
			    && lckdat->l_end >= f->set.l_start) {
				*insrt = f;
				break;
			}
		} else	if (lckdat->l_start < f->set.l_end
			    && lckdat->l_end > f->set.l_start
			    && (f->set.l_type == F_WRLCK
				|| (f->set.l_type == F_RDLCK
				    && lckdat->l_type == F_WRLCK))) {
				return(f);
		}
		f = f->next;
	}

	for ( ; f != NULL; ) {
		if (lckdat->l_start < f->set.l_end
		    && lckdat->l_end > f->set.l_start
		    && (f->set.l_pid != upid || f->set.l_rpid != rpid ||
		        f->set.l_rsys != rsys)
		    && (f->set.l_type == F_WRLCK
			|| (f->set.l_type == F_RDLCK && lckdat->l_type == F_WRLCK))) 
			return(f);
		if (f->set.l_start > lckdat->l_end) {
			break;
		}
		f = f->next;
	}

	return(NULL);
}

/* locate overlapping file locks */

#if	UNIX_LOCKS
/*
 * lckdat is a new lock, no one else can know about it, therefore, it doesn't
 * require any locking in the MP case.
 */
#endif
getflck(vp, lckdat, offset, pid, class)
	struct vnode *vp;
	struct eflock *lckdat;
	off_t offset;
	pid_t pid;   /* pid of current proc */
	int class; /* Lock class: FILE or LOCKMNGR */
{
	register struct flino *flip;
	struct filock *found, *insrt = NULL;
	register int retval = 0;
	struct eflock inflock;		/* original flock structure */

	/* get file identifier and file lock list pointer if there is one */
	flip = findfid(vp);
	if (flip == NULL) {
		lckdat->l_type = F_UNLCK;
		return (0);
	}

	/* Calculate end marker of locked region */
	if (lckdat->l_len == 0)
		lckdat->l_end = MAXEND;
	else
		lckdat->l_end += lckdat->l_start;

	/* find overlapping lock */

	FICHAIN_LOCK(flip);
	found = blocked(flip->fl_flck, lckdat, &insrt, class, pid);
	LASSERT(FICHAIN_LOCK_HOLDER(flip));
	if (found != NULL) {
		*lckdat = found->set;
	} else {
		lckdat->l_type = F_UNLCK;
	}
	FICHAIN_UNLOCK(flip);
	freefid(flip);

	/* restore length */
	if (lckdat->l_end == MAXEND)
		lckdat->l_len = 0;
	else
		lckdat->l_len -= lckdat->l_start;
	return (retval);
}

/* clear and set file locks */

#if	UNIX_LOCKS
/*
 * lckdat is a new lock, no one else can know about it, therefore, it doesn't
 * require any locking in the MP case.
 */
#endif
setflck(vp, lckdat, slpflg, offset, pid, class)
	struct vnode *vp;
	struct eflock *lckdat;
	int slpflg;
	off_t offset;
	pid_t pid;   /* pid of current proc */
	int class; /* Lock class: FILE or LOCKMGR */
{
	register struct flino *flip;
	register struct filock *found, *sf, *nsf;
	struct filock *insrt = NULL;
	register int retval = 0;
	pid_t	upid, rpid;
	u_int	rsys;

	/* Could be the lockmgr asking us to grant locks */
	if ((class == LOCKMGR) && (lckdat->l_xxx == GRANT_LOCK_FLAG)) {
		if (dbg_lock)
			printf("setflck: calling do_grant_locks\n");

		return (do_grant_locks(lckdat));
	}

	/* Calculate end marker of locked region */
	if (lckdat->l_len == 0)
		lckdat->l_end = MAXEND;
	else
		lckdat->l_end += lckdat->l_start;

	/* get or create a file record lock header */
	flip = findfid(vp);
	if (flip == NULL) {
		if (lckdat->l_type == F_UNLCK)
			return (0);
		if ((flip=allocfid(vp)) == NULL)
			return (ENOLCK);	/* was EMFILE */
	}

	switch (lckdat->l_type) {
	case F_RDLCK:
	case F_WRLCK:
		FICHAIN_LOCK(flip);
		if ((found=blocked(flip->fl_flck, lckdat, &insrt, class, pid)) == NULL) {
			if (dbg_lock) {
				printf("\nsetflck %d: lock request not blocked\n", pid);
				print_flock(lckdat);
			}
			LASSERT(FICHAIN_LOCK_HOLDER(flip));
			retval = flckadj(flip, insrt, lckdat, vp, class, pid);
			/* 
			 * might be some wakeups to do 
			 */
			if ((retval == 1) && (lckdat->l_type == F_RDLCK)) { 
			    /* 
			     * chk_granted returns GRANT_LOCK_FLAG if 
			     * there are remote locks to grant because
			     * of this downgrade
			     */
			    retval = chk_granted(flip, vp, class);
			    if (class == LOCKMGR) {
				if (retval == GRANT_LOCK_FLAG) {
			    		lckdat->l_xxx = retval;
			    		retval = 0;
				} else 
					lckdat->l_xxx = -1;
			    } /* else do klm callbacks below */
			}
			FICHAIN_UNLOCK(flip);
			
			/* Now that we're not holding the FICHAIN lock
			 * we don't have to worry about deadlock when 
			 * making klm calls. 
			 */ 
			if ((class==FILE_LOCK) && (retval==GRANT_LOCK_FLAG))
				retval = klm_callbacks();
			if (retval == 1) retval = 0;
			break;
		} else {
			if (dbg_lock) {
				printf("\nsetflck %d: lock request blocked\n", pid);
				print_flock(lckdat);
			}
			LASSERT(FICHAIN_LOCK_HOLDER(flip));
			if (slpflg) {
				SLEEPLCK_LOCK();
				if (class == LOCKMGR) {
				    /* 
				     * This could be a duplicate request 
 				     * from the lock manager.  Make sure 
				     * ld is not already on sleep q.
				     */
				    for (sf=sleeplcks.fl_flck; sf != NULL; 
				         	     sf=sf->next) {
					/* a proc can only be blocked on
					 * 1 lock at a time
					 */
					if ((sf->set.l_pid == lckdat->l_pid) &&
				    	   (sf->set.l_rpid == lckdat->l_rpid) &&
				           (sf->set.l_rsys == lckdat->l_rsys)) {
						SLEEPLCK_UNLOCK();
						FICHAIN_UNLOCK(flip);
						if (dbg_lock)
						    printf("setflck: dup\n");
						/* send the answer again */
						retval = EINTR;
						goto out;
					}
				    }
				}
				/* deadlock detection */
				if (class != LOCKMGR) {
					upid = u.u_procp->p_pid;
					rpid = 0;
					rsys = 0;
				} else {
					upid = lckdat->l_pid;
					rpid = lckdat->l_rpid;
					rsys = lckdat->l_rsys;
				}
				if (deadflck(found, upid, rpid, rsys)) {
					SLEEPLCK_UNLOCK();
					FICHAIN_UNLOCK(flip);
					if (dbg_lock)
					    printf("setflck: EDEADLK\n");
					retval = EDEADLK;
					goto out;
				} else if ((sf=insflck(&sleeplcks, lckdat, NULL, vp, class, pid)) == NULL) {
					SLEEPLCK_UNLOCK();
					FICHAIN_UNLOCK(flip);
					retval = ENOLCK;
					goto out;
				} else {
					sf->stat.blkpid = found->set.l_pid;
					sf->blkrpid = found->set.l_rpid;
					sf->blkrsys = found->set.l_rsys;
					FICHAIN_UNLOCK(flip);
					if (class == LOCKMGR) {
						/* 
						 * lockmgr interprets EINTR
						 * as "lock isn't avail,
						 * I'll let you know when
						 * it is".
						 */
				if (dbg_lock)
				    printf("setflck: LM proc blocked, rtning EINTR\n");
						SLEEPLCK_UNLOCK();
						retval = EINTR;
						goto out;
					}
				if (dbg_lock)
				    printf("setflck: local proc going to sleep, pid=%d\n", u.u_procp->p_pid);
					/* a local process goes to sleep */
					SLEEP(sf, retval);
					if (retval != 0) {
						/* 
						 * local blocker cancelled
						 * lock request
						 */
			if (dbg_lock)
			    printf("setflck: local proc canceled, pid=%d\n", u.u_procp->p_pid);
						delsleeplck(u.u_procp->p_pid);
					} else { /* local lock granted */
				if (dbg_lock)
				    printf("setflck: local proc woke up, pid=%d\n", u.u_procp->p_pid);
					}
					goto out;
				}
			} else {
				retval = EACCES;
				FICHAIN_UNLOCK(flip);
			}
		}
		break;
	case F_UNLCK:
		/* removing a file record lock */
		if (class == LOCKMGR) {
			/* could be a cancel request, check slp q */
			SLEEPLCK_LOCK();
			for (sf=sleeplcks.fl_flck; sf != NULL; sf=nsf) {
				nsf = sf->next;
				if ((sf->set.l_pid == lckdat->l_pid) &&
				    (sf->set.l_rpid == lckdat->l_rpid) &&
				    (sf->set.l_rsys == lckdat->l_rsys)) {
				if (dbg_lock)
				    printf("setflck: LM cancelled sleep\n");
				        delflck(&sleeplcks, sf, NULL);
					SLEEPLCK_UNLOCK();
					goto out;
				}
			}
			SLEEPLCK_UNLOCK();
		}
		FICHAIN_LOCK(flip);
		if (dbg_lock) {
		    printf("\nsetflck: unlock request\n");
		    print_flock(lckdat);
		}
		retval = flckadj(flip, flip->fl_flck, lckdat, vp, class, pid);
		if (retval == 1) { /* might be some wakeups to do */
			/* 
			 * chk_granted returns GRANT_LOCK_FLAG if 
			 * there are remote locks to grant because
			 * of this unlock
			 */
			retval = chk_granted(flip, vp, class);
			if (class == LOCKMGR) {
				if (retval == GRANT_LOCK_FLAG) {
			    		lckdat->l_xxx = retval;
			    		retval = 0;
				} else 
					lckdat->l_xxx = -1;
			}   /* else do klm callbacks below */
		}
		FICHAIN_UNLOCK(flip);
		
		/* Now that we're not holding the FICHAIN lock
		 * we don't have to worry about deadlock when 
		 * making klm calls. 
		 */ 
		if ((class==FILE_LOCK) && (retval==GRANT_LOCK_FLAG))
			retval = klm_callbacks();

		break;
	default:
		retval = EINVAL;	/* invalid lock type */
		break;
	}

out:
	freefid(flip); 
	return(retval);
}

int 
do_grant_locks(ld)
	struct eflock *ld;
{
	struct filock *filockp;

	if (grantlcks.fl_flck == NULL)
		return(ENOLCK);
	GRANTLCK_LOCK();
	/*
	 * Look for entries in queue that have same callback address
	 * as eflock passed.
	 */
	filockp = grantlcks.fl_flck;
	while (filockp) {
		if (filockp->set.l_cb == ld->l_cb)
			break;
		filockp = filockp->next;
	}
	if (filockp == NULL)
		return(ENOLCK);
	ld->l_type = filockp->set.l_type;
	ld->l_whence = filockp->set.l_whence;
	ld->l_start = filockp->set.l_start;
	/* restore length so lock manager will recognize lock */
	ld->l_len = filockp->set.l_len;
	if (ld->l_end == MAXEND)
		ld->l_len = 0;
	else
		ld->l_len -= ld->l_start;
	ld->l_pid = filockp->set.l_pid;
	ld->l_rpid = filockp->set.l_rpid;
	ld->l_rsys = filockp->set.l_rsys;

	if (dbg_lock)
	   printf("do_grant_locks granting pid %d %d rsys 0x%x\n",
			ld->l_pid, ld->l_rpid, ld->l_rsys);
	delflck(&grantlcks, filockp, NULL);

	/*
	 * Scan again to see if a lock associated with this callback
	 * address in on the queue.
	 */
	filockp = grantlcks.fl_flck;
	while (filockp) {
		if (filockp->set.l_cb == ld->l_cb)
			break;
		filockp = filockp->next;
	}
	if (filockp == NULL)
		ld->l_xxx = -1;
	GRANTLCK_UNLOCK();
	return(0);
}

int
klm_callbacks()
{
	struct eflock ld;
	lockhandle_t lh;
	struct filock *ptr, *nptr;
	int error=0;

	KLMLCK_LOCK();
	lh.lh_servername = hostname;
	for (ptr=klmlcks.fl_flck; ptr != NULL; ptr=nptr) {
		nptr = ptr->next;
		ld.l_type = ptr->set.l_type;
		ld.l_whence = 0;
		ld.l_start = ptr->set.l_start;
		/* restore length so lock mngr will recognize lock */
		ld.l_len = ptr->set.l_len;
		if (ld.l_end == MAXEND)
			ld.l_len = 0;
		else ld.l_len -= ld.l_start;
		ld.l_pid = ptr->set.l_pid;
		ld.l_rpid = ptr->set.l_rpid;
		ld.l_rsys = ptr->set.l_rsys;
		ld.l_cb = ptr->set.l_cb;

		if (dbg_lock)
		   printf("klm_callbacks granting pid %d %d rsys 0x%x\n",
				ld.l_pid, ld.l_rpid, ld.l_rsys);
		lh.lh_vp = ptr->vp;
		bcopy((caddr_t)vtofh(ptr->vp), (caddr_t)&lh.lh_id, NFS_FHSIZE);
		/* pid is irrelevant in this call */
		error = klm_lockctl(&lh, &ld, KLM_GRANTED, u.u_cred, 
					u.u_procp->p_pid, 1);

		delflck(&klmlcks, ptr, NULL);
	}
	KLMLCK_UNLOCK();
	return(error);
}

int
chk_granted(flip, vp, class)
	struct flino	*flip;
	struct vnode    *vp;
	int class; /* class of lock being downgraded or unlocked */
{
	struct filock *found, *sf, *nsf;
	struct filock *insrt = NULL;
	int retval = 0;
	int nfs_locks_to_grant = 0;

	SLEEPLCK_LOCK();
	for (sf=sleeplcks.fl_flck; sf != NULL; sf=nsf) {
		nsf = sf->next;
		if (sf->vp == vp) { 
			if ((found=blocked(flip->fl_flck, &(sf->set), &insrt, 
					       sf->class, sf->set.l_pid)) == NULL) {
				/* Can wake this guy up */
				if (dbg_lock)
		    		    printf("chk_granted: found an ld to grant\n");
				/* 
				 * Don't add it to the list of locks for the
				 * vnode if it was just a read() or write()
				 * blocked on a mandatory lock.
				 */
				if (sf->class != IO_LOCK)
				      retval = flckadj(flip, insrt, &(sf->set),
						 vp, sf->class, sf->set.l_pid);

				if (retval == ENOLCK) {
					SLEEPLCK_UNLOCK();
					return(retval);
				}

				/* local user being woken up? */
				if (sf->class != LOCKMGR) {
					if (dbg_lock)
		    		            printf("chk_granted: calling WAKEUP\n");
					WAKEUP(sf);
				} else { 
					/* The lock mngr could be servicing
					 * the unlock-downgrade request 
					 * that caused this wakeup or it
					 * could be blocked waiting to get 
					 * the FICHAIN or SLEEPLCK lock.
					 * To prevent deadlock we just 
					 * queue this granted request 
					 * instead of trying to send a
					 * KLM_GRANTED message.  If the 
					 * unlock or downgrade causing this 
					 * wakeup is a local lock, we'll
					 * send the KLM_GRANTED msgs to the
					 * lockmgr after letting go of 
					 * the FICHAIN and sleep q; o.w.,
					 * the lm will ask for all granted 
					 * locks after it's done with the 
					 * current request by doing another
					 * fcntl and setting l_xxx.
					 */
					if (class == FILE_LOCK) {
					    if (dbg_lock)
		    		                printf("chk_granted: adding to KLM q\n");
						KLMLCK_LOCK();
						insflck(&klmlcks, &(sf->set), 
						    NULL, vp, sf->class, 
						    sf->set.l_pid);
						KLMLCK_UNLOCK();
					} else {
					    if (dbg_lock)
		    		                printf("chk_granted: adding to GRANT q\n");
						GRANTLCK_LOCK();
						insflck(&grantlcks, &(sf->set),
						        NULL, vp, sf->class,
							sf->set.l_pid);
						GRANTLCK_UNLOCK();
					}
					nfs_locks_to_grant = GRANT_LOCK_FLAG;
				}
		        	delflck(&sleeplcks, sf, NULL);
			} else {
				/* still blocked */
				sf->stat.blkpid = found->set.l_pid;
				sf->blkrpid = found->set.l_rpid;
				sf->blkrsys = found->set.l_rsys;
			}
		} /* same vp */
	} /* for */
	SLEEPLCK_UNLOCK();
	return(nfs_locks_to_grant);
}

/* deadflck does the deadlock detection for the given record 
 * Note: Although NFS client locks are checked as well, the lack of
 * globally shared locking information among the lock mngrs prevents
 * this algorithm from being totally effective in other than the
 * local environment.
 */

int
deadflck(flp, upid, rpid, rsys)
	struct filock *flp;
	pid_t upid, rpid;
	u_int rsys;
{
	register struct filock *blck, *sf;
	pid_t blckpid, blckrpid;
	u_int blckrsys;

	LASSERT(SLEEPLCK_LOCK_HOLDER());
	blck = flp;	/* current blocking lock pointer */
	blckpid = blck->set.l_pid;
	blckrpid = blck->set.l_rpid;
	blckrsys = blck->set.l_rsys;
	do {
		if ((blckpid == upid) && (blckrpid == rpid) &&
		    (blckrsys == rsys)) {
			return(1);
		}
		/* if the blocking process is sleeping on a locked region,
		 * change the blocked lock to this one.
		 */
		for (sf = sleeplcks.fl_flck; sf != NULL; sf = sf->next) {
			if ((blckpid == sf->set.l_pid) &&
			    (blckrpid == sf->set.l_rpid) &&
			    (blckrsys == sf->set.l_rsys)) {
				blckpid = sf->stat.blkpid;
				blckrpid = sf->blkrpid;
				blckrsys = sf->blkrsys;
				break;
			}
		}
		blck = sf;
	} while (blck != NULL);
	return(0);
}

/* Clean up record locks left around by process (called in closef) */

cleanlocks(vp)
struct vnode *vp;
{
	register struct filock *flp, *nflp;
	register struct flino *flip;
	int call = FALSE;	/* True if worth calling chk_granted */
	int retval = 0;

	flip = findfid(vp);
	if (flip == NULL)
		return retval;

	FICHAIN_LOCK(flip);
	if (dbg_lock)
		printf("\nin cleanlocks\n");
#ifdef i386
	xenix_sem_cleanup(flip);       /* for xenix compatibility */
#endif
	for (flp=flip->fl_flck; flp!=NULL;) {
		if ((flp->set.l_pid == u.u_procp->p_pid) &&
		    (flp->set.l_rpid == 0) && (flp->set.l_rsys == 0)) {
			delflck(flip, flp, vp);
			call = TRUE;
			flp = flip->fl_flck;
		} else
			flp = flp->next;
	}
	/* There might be some wakeups to do. 
	 * chk_granted returns GRANT_LOCK_FLAG if
	 * there are remote locks to grant because
	 * of these local unlocks.
	 */
	if (call) {
		if (dbg_lock)
			printf("cleanlocks: calling chk_granted\n");
		retval = chk_granted(flip, vp, FILE_LOCK); 
	}

	FICHAIN_UNLOCK(flip);

	/* Now that we're not holding the FICHAIN lock, we can grant
	 * klm locks without worrying about deadlock.  Local wakeups
	 * were done in chk_granted.
	 */
	if (retval == GRANT_LOCK_FLAG)
		retval = klm_callbacks();
	freefid(flip);
	return(retval);
}

/*
 * locked() checks for enforcement mode blocking locks.  If check_wlck is set,
 * only write locks will block the operation (i.e., we are reading the file).
 * For writes, read or write locks will be blocking locks.
 */
int
locked(vp, lckdat, flag)
	struct vnode *vp;
	struct eflock *lckdat;
	int flag;
{
	register struct filock *found, *sf;
	struct filock *insrt = NULL;
	struct flino *flip;
	int retval = 0;
	u_int v_flag;

        lckdat->l_whence = L_SET;
	if (flag & VRDFLCK)               
	      lckdat->l_type = F_RDLCK;   /* we will not be blocked by other
					    read locks */
	else
	      lckdat->l_type = F_WRLCK;

	if (dbg_lock) {
		printf("\nin locked(), pid=%d, vp=0x%x\n", u.u_procp->p_pid, vp);
		print_flock(lckdat);
	}

	flip = findfid (vp);
	if (flip == NULL)
	      return (0);
	/*
	 * We start from the same file offset, regardless of whether
	 * we block and someone else changes the f_offset on us.
	 * This effectively makes this an atomic (seek,read|write).
	 */

	/* find overlapping lock */
	FICHAIN_LOCK(flip);
	if (flag & VMANFLCK)
		found = blocked(flip->fl_flck, lckdat, &insrt, IO_LOCK, 
				u.u_procp->p_pid);
#ifdef i386
	else {
		found = xenix_blocked(flip->fl_flck, lckdat, &insrt);
		if (found) {
			BM(VN_LOCK(vp));
			v_flag = vp->v_flag;
			BM(VN_UNLOCK(vp));
			/* if a SVR3 process is blocked by a Xenix lock
			   on a file that does not have enforcement
			   mode file locking turned on, sleep even 
			   though FNDELAY is set (pg 55 of
			   Locus Final Design Doc) */
			if ((!(v_flag & VENF_LOCK)) && 
			    (u.u_procp->cxenix == NULL))
				flag &= ~SLPFLCK;
		}
	}
#endif
	LASSERT(FICHAIN_LOCK_HOLDER(flip));
	if (found == NULL)
	      goto out;
	if (flag & SLPFLCK) {
	      retval = EAGAIN;
	      goto out;
	}

	/* do deadlock detection here */
	SLEEPLCK_LOCK();
	if (deadflck(found, u.u_procp->p_pid, 0L, 0L))
		retval = EDEADLK;
	else if ((sf=insflck(&sleeplcks, lckdat, NULL,
		     vp, IO_LOCK, u.u_procp->p_pid)) == NULL)
		retval = ENOLCK;
	else {
		sf->stat.blkpid = found->set.l_pid;	
		sf->blkrpid = found->set.l_rpid;	
		sf->blkrsys = found->set.l_rsys;
		FICHAIN_UNLOCK(flip);

		if (dbg_lock)
		   printf("process going to sleep on mandatory lock pid %d\n",
				u.u_procp->p_pid);

		SLEEP(sf,retval);
		if (retval)	/* read or write has been cancelled */
			delsleeplck(u.u_procp->p_pid);
		freefid(flip);
		return(retval);
	}
	SLEEPLCK_UNLOCK();

out:
	FICHAIN_UNLOCK(flip);
	freefid(flip);
        return(retval);
}

print_flino(fl)
	struct flino *fl;
{
	struct filock *ptr;

	for (ptr=fl->fl_flck; ptr!=NULL; ptr= ptr->next) {
	     printf("  type=%d start=%d end=%d pid=%d rpid=%d rsys=0x%x\n",
			ptr->set.l_type, ptr->set.l_start, 
			ptr->set.l_end, ptr->set.l_pid, ptr->set.l_rpid,
			ptr->set.l_rsys);
	     printf("  blkpid=%d blkrpid=%d blkrsys=0x%x\n", 
			ptr->stat.blkpid, ptr->blkrpid, ptr->blkrsys); 
	     printf("  class=%d vp=0x%x\n", ptr->class, ptr->vp);
	     printf("\n");
	} 
}

print_flock(ld)
	struct eflock *ld;
{
	printf("type=%d start=%d end=%d pid=%d rpid=%d rsys=0x%x\n",
		ld->l_type, ld->l_start, ld->l_end, ld->l_pid, 
		ld->l_rpid, ld->l_rsys);
}

/*
 * This function will kill all locks associated with processes on the
 * remote system specified by rsys.  Called by ufs_lockctl() in ufs_vnops.c.
 */
void
kill_proc_locks(ld)
	struct eflock *ld;
{
	struct flino *flip, *nflip;
	struct filock *t;	/* For walking the lock chain.	 */
	struct filock *nt;
	int call;		/* True if worth calling chk_granted */
	int grant_flag = 0;	/* Becomes when locks to grant */

	if (dbg_lock) {
		printf("...in kill_proc_locks\n");
		print_flock(ld);
	}

	/* Purge remote entries from the sleep list for the locks in waiting. */
	SLEEPLCK_LOCK();
	flip = &sleeplcks;
	for (t = flip->fl_flck; t != NULL; t = nt) {
		nt = t->next; /* Next lock in current (flip) chain */
		if (t->set.l_rsys == ld->l_rsys) {
			delflck(flip, t, NULL);
		}
	}
	SLEEPLCK_UNLOCK();

	/*
	 * For all files with locks from that system, release them
	 * and tell the lockmanager if it needs to grant some.
	 *
	 * A note on SMP locking:
	 * We need the fids lock while we walk the chain.  However, we need
	 * the fichain lock before calling delflck.  Fichain is a sleep
	 * lock, so we have to release the fids lock first, because that's a
	 * spin lock.  Since we'll unlock fids, we have to bump the
	 * reference count on the flino to keep that from going away, and
	 * that has to be done with the flino locked.
	 */
	ld->l_xxx = -1;		/* Assume we'll have nothing to grant */
	FIDS_LOCK();
	for (flip = fids; flip != NULL; flip = nflip) {
		FLINO_LOCK(flip);
		flip->fl_refcnt++;
		FLINO_UNLOCK(flip);
		FIDS_UNLOCK();		/* Gotta free spin lock */
		FICHAIN_LOCK(flip);	/* Before taking sleep lock */
		call = FALSE;
		for (t = flip->fl_flck; t != NULL; t = nt) {
			nt = t->next;	/* Next lock in current (flip) chain */
			if (t->set.l_rsys == ld->l_rsys) {
				delflck(flip, t, flip->vp);
				call = TRUE;
			}
		}
		if (call &&
		    (chk_granted(flip, flip->vp, LOCKMGR) == GRANT_LOCK_FLAG))
			ld->l_xxx = GRANT_LOCK_FLAG;
		FICHAIN_UNLOCK(flip);
		nflip = flip->next;
		freefid(flip);
		FIDS_LOCK();
	}
	FIDS_UNLOCK();
	return;
}

/*
 * Routine to purge all lock manager locks associated with a file system.
 * Used by the ASE project when moving filesystem access from one machine
 * to another.  The daemon has been killed or is about to be kill, and
 * the reclaim mechanism will be used to reestablish locks on another
 * system.  Called with a vnode of the file system to purge.
 *
 * Although all locking in ASE systems should be via the lock manager,
 * we try to do the right thing with local locks, like leaving them intact
 * and waking processes waiting on locks we release.
 */
void purge_fs_locks(struct vnode *vp) {
	fsid_t *fsp;
	struct flino *flip, *nflip;
	struct filock *t;	/* For walking the lock chain.	 */
	struct filock *nt;

	fsp = &vp->v_mount->m_stat.f_fsid;

	if (dbg_lock)
		printf("...in purge_fs_locks for %s\n",
		       vp->v_mount->m_stat.f_mntonname);

	/* Purge remote entries from the sleep list for the locks in waiting. */
	purge_fs_list(&sleeplcks, fsp);

	/*
	 * For all files with locks from that system that are managed by
	 * the lock manager, release them.  Call chk_granted to release
	 * any local processes waiting.
	 */
	FIDS_LOCK();
	for (flip = fids; flip != NULL; flip = nflip) {
		nflip = flip->next;
		if (!fsid_equal(&flip->vp->v_mount->m_stat.f_fsid, fsp))
			continue; /* Not our fs */
		FLINO_LOCK(flip);
		flip->fl_refcnt++;
		FLINO_UNLOCK(flip);
		FIDS_UNLOCK();		/* Gotta free spin lock */
		FICHAIN_LOCK(flip);	/* Before taking sleep lock */
		for (t = flip->fl_flck; t != NULL; t = nt) {
			nt = t->next;	/* Next lock in current (flip) chain */
			if (t->class == LOCKMGR)
				delflck(flip, t, flip->vp);
		}
		(void) chk_granted(flip, flip->vp, LOCKMGR);
		FICHAIN_UNLOCK(flip);
		freefid(flip);
		FIDS_LOCK();
	}
	FIDS_UNLOCK();

	/*
	 * Purge the klm and grant lists.  chk_granted() should not have
	 * added anything since we already purged the sleep list, but there
	 * could be stuff leftover from before.
	 */
	purge_fs_list(&klmlcks, fsp);
	purge_fs_list(&grantlcks, fsp);
}

/*
 * Local routine used by purge_fs_locks() to junk lock manager locks
 * on the various paperwork lists.  Note we call the lock/unlock routines
 * instead of using SLEEPLCK_LOCK, GRANTLCK_LOCK, or KLMLCK_LOCK and their
 * UNLOCK counterparts.
 */
static void
purge_fs_list(struct flino *flip, /* List head to scan */
	      fsid_t *fsp)	/* File system of interest */
{
	struct filock *t, *nt;	/* For walking the lock chain. */

#if UNIX_LOCKS
	lock_write(&flip->fichain_lock);
#endif
	if (dbg_lock)
		printf("purge_fs_list: for fs %x/%x\n",
		       fsp->val[0], fsp->val[1]);
	for (t = flip->fl_flck; t != NULL; t = nt) {
		nt = t->next;	/* Next lock in current (flip) chain */
		if (fsid_equal(&t->vp->v_mount->m_stat.f_fsid, fsp) &&
		    t->class == LOCKMGR)
			delflck(flip, t, NULL);
	}
#if UNIX_LOCKS
	lock_write_done(&flip->fichain_lock);
#endif
}

clear_vlocks(vp)
struct vnode *vp;
{
	VN_LOCK(vp);
	vp->v_flag &= ~(VLOCKS|VENF_LOCK);
	VN_UNLOCK(vp);
} 


#ifdef i386
set_vxenix(vp)
struct vnode *vp;
{
	VN_LOCK(vp);
	vp->v_flag |= VXENIX;
	VN_UNLOCK(vp);
}

clear_vxenix(vp)
struct vnode *vp;
{
	VN_LOCK(vp);
	vp->v_flag &= ~(VXENIX);
	VN_UNLOCK(vp);
} 

/*
 * xenix_blocked checks whether a new lock (lckdat) would be
 * blocked by a previously set lock owned by another process.
 * When xenix_blocked is called, 'flp' should point
 * to the record from which the search should begin.
 * Insrt is set to point to the lock before which the new lock
 * is to be placed.
 */
#if	UNIX_LOCKS
/*
 * xenix_blocked returns with the lock LOCKED on the filock structure.
 * It is up to the calling routine to unlock the structure.
 *
 * xenix_blocked is called only from locked, with lckdat being a
 * new lock.  Since no one else can know of lckdat, it requires no locking.
 */
#endif
struct filock *
xenix_blocked(flp, lckdat, insrt)
struct filock *flp;
struct flock *lckdat;
struct filock **insrt;
{
	struct filock *f;

	*insrt = NULL;
	for (f = flp; f != NULL; ) {
		if (f->set.l_start < lckdat->l_start)
			*insrt = f;
		else {
			break;
		}
		if (f->set.l_pid == u.u_procp->p_pid) {
			if (lckdat->l_start <= f->set.l_end
			    && lckdat->l_end >= f->set.l_start) {
				*insrt = f;
				break;
			}
		} else	if (lckdat->l_start < f->set.l_end
			    && lckdat->l_end > f->set.l_start
			    && (f->set.l_type == (F_WRLCK | F_ENFRCD)
				|| (f->set.l_type == (F_RDLCK | F_ENFRCD)
				    && lckdat->l_type == F_WRLCK))) {
				return(f);
		}
		f = f->next;
	}

	for ( ; f != NULL; ) {
		if (lckdat->l_start < f->set.l_end
		    && lckdat->l_end > f->set.l_start
		    && f->set.l_pid != u.u_procp->p_pid
		    && (f->set.l_type == (F_WRLCK | F_ENFRCD)
			|| (f->set.l_type == (F_RDLCK | F_ENFRCD)
			&& lckdat->l_type == F_WRLCK))) 
			return(f);
		if (f->set.l_start > lckdat->l_end) {
			break;
		}
		f = f->next;
	}

	return(NULL);
}

#endif
