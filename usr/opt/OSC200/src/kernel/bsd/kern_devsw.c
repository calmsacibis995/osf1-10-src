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
static char *rcsid = "@(#)$RCSfile: kern_devsw.c,v $ $Revision: 4.3.7.2 $ (DEC) $Date: 1993/05/03 20:58:27 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#include <sys/conf.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <kern/lock.h>
#include <sys/lock_types.h>

extern int	nodev();			/* entry returning ENODEV */

/*
 * Null devsw entry(s) used to initialize table entries and remove entries.
 */
struct  cdevsw  null_chrent[] = {

        nodev,                                  /* int  (*d_open)() */
        nodev,                                  /* int  (*d_close)() */
        nodev,                                  /* int  (*d_read)() */
        nodev,                                  /* int  (*d_write)() */
        nodev,                                  /* int  (*d_ioctl)() */
        nodev,                                  /* int  (*d_stop)() */
        nodev,                                  /* int  (*d_reset)() */
        0,                                      /* struct tty *d_ttys */
        nodev,                                  /* int  (*d_select)() */
        nodev,                                  /* int  (*d_mmap)() */
	NULL,					/* int  d_funnel    */
	nodev,					/* int  (*d_segmap)() */
	0					/* int  d_flags */
};

struct  bdevsw  null_blkent[] = {
        nodev,                                  /* int  (*d_open)() */
        nodev,                                  /* int  (*d_close)() */
        nodev,                                  /* int  (*d_strategy)() */
        nodev,                                  /* int  (*d_dump)() */
        nodev,                                  /* int  (*d_psize)() */
        0,                                      /* int  d_flags */
        nodev,                                  /* int  (*d_ioctl)() */
	NULL					/* int  d_funnel    */
};


/*
 * NAME:	devsw_init()
 *
 * FUNCTION:  	Initialize the device switch table(s) for dynamic assignment.
 *
 * DESCRIPTION: This routine, called in init_main:
 *		Initializes all UNUSED table entries, by: 
 *	   	 	1) setting devsw entry inuse flag to DEVSW_FREE
 *	    		2) setting devsw entry equal to that of null_???ent
 *	    		3) initialing the devsw entry reader/writer lock
 *		Initializes all USED table entries, by: 
 *	    		1) setting devsw inuse flag to DEVSW_INUSE
 *	    		2) initialing the devsw entry reader/writer lock
 *
 * EXEC ENV:	This routine must get called to initialize the devsw table(s) 
 *		before non-static initialized entries are accessed or dynamic 
 *		configuration routines are used.
 * 		Each devsw entry has an inuse flag and a reader/writer lock 
 *		associated with it. Any time an open is in progress, the reader 
 *		lock must be held.  Anytime the entry is going to be modified, 
 *		via cdevsw_add() or cdevsw_del(), the writer lock must be held. 
 *		It is the responsibility of the device driver for rejecting 
 *		deconfiguration requests while the device has outstanding 
 *		references (e.g. "opened").  This implies that the device 
 *		driver, in order to be deconfigured, must keep its own internal
 *		open/close reference count.
 *
 * NOTES:	Entries are determined to be UNUSED if the entries d_open 
 *		field is equal to NULL.  Therefore, any statically initialized
 *		entries in conf.c whose d_open field is equal to nodev() or
 *		nulldev() will be considered USED.
 *
 * RETURN VALUES: None.
 *	
 */

void
#ifdef _NO_PROTO
devsw_init( )
#else
devsw_init( void )
#endif
{
	register int 	i;
	register int	chrcnt;
	register int	blkcnt;
	struct cdevsw 	*chrent;
	struct bdevsw 	*blkent;

	chrcnt = 0;
	blkcnt = 0;

	for (i = 0; i < nchrdev; i++) {

	    CDEVSW_LOCK_INIT(i);
	    cdevlock[i].dsw_flags &= ~DSW_INUSE;

	    chrent = &cdevsw[i];
	    /* 
	     *	A cdevsw[i].d_open == NULL indicates an uninitialized entry
	     *	- thus we initialize the entry (i.e. d_open = nodev, ...)
	     */
	    if (chrent->d_open == NULL)
		bcopy((caddr_t) null_chrent, (caddr_t) chrent,
		    sizeof(struct cdevsw));

	    /* 
	     *	A cdevsw[i].d_open != NULL (NOT including those inited above!)
	     *	indicates an entry statically initialized and in use.
	     */
	    else {
		cdevlock[i].dsw_flags |= DSW_INUSE;
		chrcnt++;
	    }
	}
#ifdef DEBUG_DEVSW
	printf("cdevsw:	%d/%d in use\n", nchrdev, chrcnt);
#endif
	
	for (i = 0; i < nblkdev; i++) {

	    BDEVSW_LOCK_INIT(i);
	    bdevlock[i].dsw_flags &= ~DSW_INUSE;

	    blkent = &bdevsw[i];
	    if (blkent->d_open == NULL)
		bcopy((caddr_t) null_blkent, (caddr_t) blkent, 
		    sizeof(struct bdevsw));
	    else {
		bdevlock[i].dsw_flags |= DSW_INUSE;
		blkcnt++;
	    }
	}
#ifdef DEBUG_DEVSW
	printf("bdevsw:	%d/%d in use\n", nblkdev, blkcnt);
#endif

	return;
}



/*
 * NAME: 	cdevsw_try() and bdevsw_try()
 *
 * FUNCTION:  	Routine called by devsw_add() to test and set a devsw entry.
 *
 * DESCRIPTION: Given a dev_t this function tests whether the desired entry is
 *		available.   If the entry is free it sets it to the supplied 
 *		device switch entry.  The algorithm checks to insure proper 
 *		dev_t is supplied, then acquires a read lock, and tests the 
 *		read locked INUSE flag.  If INUSE NODEV is returned, else an 
 *		attempt to upgraded the read lock to write is performed.  If 
 *		the write fails, another devsw_try() must be adding an entry, 
 *		thus NODEV is returned, else the entry is added, and the lock
 *		is removed.
 *
 * RETURN VALUES: Returns NODEV on failure and dev_t on success.
 *	
 */

dev_t
#ifdef _NO_PROTO
cdevsw_try( devno, chrent )
	dev_t		devno;
	struct cdevsw*	chrent;
#else
cdevsw_try( dev_t devno, struct cdevsw *chrent )
#endif
{
	register int maj;

	if (((maj = major(devno)) < 0) || (maj >= nchrdev))
	    return(NODEV);
	
	CDEVSW_READ_LOCK(maj);	

	if (cdevlock[maj].dsw_flags & DSW_INUSE) {
	    CDEVSW_READ_UNLOCK(maj);	
	    return(NODEV);
	} else {
		/*
		 * Improve the read-only lock to one with write permission.  
		 * If another reader has already requested an upgrade to the
		 * write lock, no lock is held upon return (by definition).
		 */
	    if (lock_read_to_write(&cdevlock[maj].dsw_lock))
		return(NODEV);
	    bcopy((caddr_t) chrent, (caddr_t) &cdevsw[maj], 
			sizeof(struct cdevsw));
	    cdevlock[maj].dsw_flags |= DSW_INUSE;

	    CDEVSW_WRITE_UNLOCK(maj);	
	    return(devno);
	}
}


dev_t
#ifdef _NO_PROTO
bdevsw_try( devno, blkent )
	dev_t		devno;
	struct bdevsw *	blkent;
#else
bdevsw_try( dev_t devno, struct bdevsw *blkent )
#endif
{
	register int maj;

	if (((maj = major(devno)) < 0) || (maj >= nblkdev))
	    return(NODEV);
	
	BDEVSW_READ_LOCK(maj);	

	if (bdevlock[maj].dsw_flags & DSW_INUSE) {
	    BDEVSW_READ_UNLOCK(maj);	
	    return(NODEV);
	} else {
		/*
		 * Improve the read-only lock to one with write permission.  
		 * If another reader has already requested an upgrade to the
		 * write lock, no lock is held upon return.
		 */
	    if (lock_read_to_write(&bdevlock[maj].dsw_lock))
		return(NODEV);
	    bcopy((caddr_t) blkent, (caddr_t) &bdevsw[maj], 
		sizeof(struct bdevsw));
	    bdevlock[maj].dsw_flags |= DSW_INUSE;

	    BDEVSW_WRITE_UNLOCK(maj);	
	    return(devno);
	}
}


/*
 * NAME: 	cdevsw_add() and bdevsw_add()
 *
 * FUNCTION:  	Add an entry into the Device Switch Table
 *
 * DESCRIPTION: Given a dev_t and devsw entry this function tests whether the 
 *		desired entry is available.   If the entry is UNUSED it copies 
 *		the supplied device switch entry.  If NODEV is supplied as the
 *		dev_t, the routine finds the first UNUSED entry and copies
 *		in the supplied device switch entry.  If successful, the dev_t
 *		representing the device switch table entry is returned.
 *		If the table is full or invalid data was passed in, NODEV is 
 *		returned.
 *
 * EXEC ENV:  	Called by device driver subsystems' configuration routine to 
 *		register their entry points in the device switch table.
 *
 * RETURN VALUES: Returns the device switch entry number (dev_t) on success or
 *		NODEV on failure.
 *	
 */

dev_t 
#ifdef _NO_PROTO
cdevsw_add( devno, chrent )
	dev_t		devno;
	struct cdevsw *	chrent;
#else
cdevsw_add( dev_t devno, struct cdevsw * chrent )
#endif
{
	register int i;

	if (chrent == (struct cdevsw *) NULL)
	    return(NODEV);

					/* Directive: use supplied dev_t */
	if (devno != NODEV)
	    return(cdevsw_try(devno, chrent));

					/* Directive: find first free dev_t */
	for (i= 0; i < nchrdev; i++) {
	    if ((devno = cdevsw_try(makedev(i,0), chrent)) != NODEV)
		return(devno);
	}
	return(NODEV);
}


dev_t 
#ifdef _NO_PROTO
bdevsw_add( devno, blkent )
	dev_t		devno;
	struct bdevsw *	blkent;
#else
bdevsw_add( dev_t devno, struct bdevsw * blkent )
#endif
{
	register int i;

	if (blkent == (struct bdevsw *) NULL)
	    return(NODEV);

					/* Directive: use supplied dev_t */
	if (devno != NODEV)
	    return(bdevsw_try(devno, blkent));

					/* Directive: find first free dev_t */
	for (i= 0; i < nblkdev; i++) {
	    if ((devno = bdevsw_try(makedev(i,0), blkent)) != NODEV)
		return(devno);
	}
	return(NODEV);
}



/*
 * NAME : 	cdevsw_del(), bdevsw_del() and dualdevsw_del
 *
 * FUNCTION: 	Deletes an entry from the device switch table.
 *
 * DESCRIPTION: Given a valid dev_t the function copies in the null_???ent 
 *		into the device switch table, thus unregistering the device
 *		driver entry points.  
 *
 * EXEC ENV:	This routine is called by a device driver subsystems'
 *		unconfiguration routine to remove its exported entry points
 *		from the device switch.
 *
 * RETURN VALUES: 0 on success, -1 on failure.
 */

int
#ifdef _NO_PROTO
cdevsw_del( devno )
	dev_t		devno;
#else
cdevsw_del( dev_t devno )
#endif
{
	register int maj;
	extern void aio_cdevsw_del();

	if (devno == NODEV)
	    return(-1);

	if ((maj=major(devno)) < 0 || maj >= nchrdev)
	    return(-1);

	CDEVSW_WRITE_LOCK(maj);
	if (cdevlock[maj].dsw_flags & DSW_INUSE) {

	    bcopy((caddr_t) null_chrent, (caddr_t) &cdevsw[maj], 
		sizeof(struct cdevsw));
	    aio_cdevsw_del(maj);
	    cdevlock[maj].dsw_flags &= ~DSW_INUSE;

	    CDEVSW_WRITE_UNLOCK(maj);
	    return(0);
	} 
	CDEVSW_WRITE_UNLOCK(maj);
	return(-1);
}

int
#ifdef _NO_PROTO
bdevsw_del( devno )
	dev_t		devno;
#else
bdevsw_del( dev_t devno )
#endif
{
	register int maj;

	if (devno == NODEV)
	    return(-1);

	if ((maj=major(devno)) < 0 || maj >= nblkdev)
	    return(-1);

	BDEVSW_WRITE_LOCK(maj);
	if (bdevlock[maj].dsw_flags & DSW_INUSE) {

	    bcopy((caddr_t) null_blkent, (caddr_t) &bdevsw[maj], 
		sizeof(struct bdevsw));
	    bdevlock[maj].dsw_flags &= ~DSW_INUSE;

	    BDEVSW_WRITE_UNLOCK(maj);
	    return(0);
	}
	BDEVSW_WRITE_UNLOCK(maj);
	return(-1);
}
int
#ifdef _NO_PROTO
dualdevsw_del( devno )
	dev_t		devno;
#else
dualdevsw_del( dev_t devno )
#endif
{
	/* 
	 * This routine has no "value added" it is being provided merely
	 * for completeness.
	 */
	if ((bdevsw_del(devno) != 0) ||
	    (cdevsw_del(devno) != 0)) {
		return(-1);
	}
	return(0);
}

/*
 * NAME:        dualdevsw_try	
 *
 * FUNCTION:  	Routine called by dualdevsw_add() to test and set the same
 *		slot in cdevsw and bdevsw 
 *
 * DESCRIPTION: Given a dev_t this function tests whether the desired entry is
 *		available in both tables. If the entry is free it sets it to 
 *               the supplied device switch entry. 
 */
dev_t
dualdevsw_try( dev_t devno, struct cdevsw *chrent, struct bdevsw *blkent)
{
	register int maj;
 
	if ((maj = major(devno)) < 0 || maj >= nchrdev || maj >= nblkdev) {
		return(NODEV);
	}
 
	CDEVSW_READ_LOCK(maj);
	BDEVSW_READ_LOCK(maj);
 
 	if ((cdevlock[maj].dsw_flags & DSW_INUSE) ||
				(bdevlock[maj].dsw_flags & DSW_INUSE)) {
		BDEVSW_READ_UNLOCK(maj);
		CDEVSW_READ_UNLOCK(maj);
		return(NODEV);
	}

	/*
	 * Improve the read-only lock to one with write permission.
	 * If another reader has already requested an upgrade to the
	 * write lock, no lock is held upon return (by definition).
	 */
	if (lock_read_to_write(&bdevlock[maj].dsw_lock)){
		CDEVSW_WRITE_UNLOCK(maj);
		return(NODEV);
	}
	if (lock_read_to_write(&cdevlock[maj].dsw_lock)){
		BDEVSW_READ_UNLOCK(maj);
		return(NODEV);
	}
	bcopy((caddr_t) chrent, (caddr_t) &cdevsw[maj],
	sizeof(struct cdevsw));
	cdevlock[maj].dsw_flags |= DSW_INUSE;

	bcopy((caddr_t) blkent, (caddr_t) &bdevsw[maj],
	sizeof(struct bdevsw));
	bdevlock[maj].dsw_flags |= DSW_INUSE;

	BDEVSW_WRITE_UNLOCK(maj);
	CDEVSW_WRITE_UNLOCK(maj);
	return(devno);
}
 
/*
 * NAME: 	dualdevsw_add
 *
 * FUNCTION:  	Add entries into the bdev and cdev Device Switch Table
 *
 * DESCRIPTION: Given a dev_t and devsw entry this function tests whether the
 *		desired entries are available.   If the entries are UNUSED it
 *		copies the supplied device switch entry.  If NODEV is supplied
 *		as the dev_t, the routines finds the first 2 UNUSED entries
 *		and copies in the supplied device switch entry.
 *		If successful, the dev_t representing the device switch
 *		table entry is returned.  If the table is full or invalid
 *		data was passed in, NODEV is returned.
 *
 * EXEC ENV:  	Called by device driver subsystems' configuration routine to
 *		register their entry points in the device switch table.
 *
 * RETURN VALUES: Returns the device switch entry number (dev_t) on success or
 *		NODEV on failure.
 */
dev_t
dualdevsw_add( dev_t devno, struct cdevsw * chrent, struct bdevsw *blkent)
{
	register int i, limit;
 
	if (chrent == (struct cdevsw *) NULL || blkent == (struct bdevsw *)NULL)
		return(NODEV);
 
	/* Directive: use supplied dev_t */
	if (devno != NODEV)
		return(dualdevsw_try(devno, chrent, blkent));
 
	/* Directive: find first free dev_t */
	limit = (nblkdev <= nchrdev ? nblkdev : nchrdev);
	for (i = 0;  i < limit;  i++) {
		if ((devno = dualdevsw_try(makedev(i,0), chrent, blkent))
								!= NODEV) {
			return(devno);
		}
	}
	return(NODEV);
}
