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
static char	*sccsid = "@(#)$RCSfile: lv_defect.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:19:02 $";
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

/*
 * This file is derived from:

 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - hd_phys.c
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * lv_defect.c
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */

#include <lvm/lvmd.h>

#include <sys/errno.h>
#include <sys/conf.h>
#include <sys/specdev.h>

extern void lv_bbdirend();
extern void lv_fixup();
extern void lv_deletedefect();
extern void lv_defecthash();
extern void lv_updatebbdir();

/*
 * NAME:         lv_validblk
 *
 * FUNCTION:
 *	Validate specified block.  If relocation is required, take the
 *	appropriate action depending on mirroring, etc.
 *
 * PARAMETERS:
 *	pb - a physical struct buf
 *
 * RETURN VALUE:
 *	TRUE  - proceed with operation
 *	FALSE - unable to relocate bad block
 *
 */
int
lv_validblk(pb)
register struct pbuf *pb;
{
    register lv_defect_t *bad;		/* defect list pointer */
    register lv_bblk_t	 *defect;	/* located defect */
    register uint_t	 defect_no;	/* located defect number */

    /* Search defect directory hash table */
    pb->pb_bad = NULL;
    bad = BBHASHPTR(pb);
    for(;;) {
	if (bad == NULL)	/* no bad blocks in this request */
	    return(TRUE);
	/*
	 * If beginning of request is > BB address, then BB can't be 
	 * 	in request area (badblk list in ascending order).
	 */
	if (pb->pb.b_blkno > BB_DEFECT(bad->defect))
	    bad = bad->next;
	else
	    break;
    }

    defect    = bad->defect;
    defect_no = BB_DEFECT(defect);

    /* See if the defect is within the range of this request */
    if (pb->pb.b_blkno + BYTE2BLK(pb->pb.b_bcount) <= defect_no) {
	if (IS_FIXUP(pb)) {
	    pb->pb.b_bcount = 0;
	    pb->pb.b_blkno  = pb->pb_start
				+ BYTE2BLK(pb->pb_endaddr - pb->pb_startaddr);
	}
	return(TRUE);
    }

    /*
     * At this point, it is known that there is a bad block somewhere in
     * the request.  Process any blocks preceding the defect first.
     */
    if (pb->pb.b_blkno < defect_no)
	if (IS_FIXUP(pb))
	    pb->pb.b_blkno = defect_no;
	else {
	    pb->pb.b_bcount = BLK2BYTE(defect_no - pb->pb.b_blkno);
	    return(TRUE);
	}

    /*
     * At this point, pb->pb.blkno is the first bad block in the range of
     * this request.  If this logical volume doesn't allow relocation, return
     * failure.  Otherwise, handle that block by itself. 
     */
    if (pb->pb_options & LVM_NORELOC)
	return(FALSE);

    pb->pb_bad = defect;
    pb->pb.b_bcount = DEV_BSIZE;

    switch(BB_STATUS(defect)) {
      case REL_DONE:
	/*
	 * If software relocation has been done, substitute that block
	 * number for the original.
	 */
	pb->pb.b_blkno = BB_ALTERNATE(defect);
	break;

      case REL_PENDING:
      case REL_DEVICE:
	/* If relocation is in progress, we're in trouble  */
	panic("lv_validblk: invalid relocation status");
	break;

      case REL_DESIRED:
	/* Relocation cannot be done on reads or read-only volumes */
	if ((pb->pb.b_flags & B_READ) ||
	   (pb->pb_pvol->pv_flags & LVM_PVRORELOC)) {
	    return(FALSE);
	}

	/* Attempt hardware relocation */
	SET_BB_STATUS(defect, REL_DEVICE);
	pb->pb.b_flags |= B_HWRELOC;
	pb->pb_op = FIX_READ_ERROR;
	break;

      default:
	panic("lvvalidblk: unrecognized relocation status");
	break;
    }  /* end -- switch on bad block status */

    return(TRUE);
}

/*
 *  NAME:         lv_swreloc
 * 
 *  FUNCTION:
 *	Performs software relocation when hardware relocation fails.  An
 *	alternate block is found from the relocation blk pool at the end of
 *	the disk and the request is resent to the disk device driver to
 *	write to this new block. 
 *
 *  NOTES:
 *	input:	physical device buf structure.
 *	output:	request is put on ready queue to be sent to the disk
 *			device driver
 *
 *  PARAMETERS:
 *	pb - physical buf struct 
 */
void
lv_swreloc(pb)
register struct pbuf *pb;	/* physical request to process */
{
    register lv_bblk_t *bad = pb->pb_bad;
    register int badhash;           /* index to the bblk hash chain */

    /*
     * If pv is in Read Only Relocate state, nothing more can be done.
     * Reset the defect entry, indicate the error in the pb and return.
     */
    if (pb->pb_pvol->pv_flags & LVM_PVRORELOC) {
	/*
	 * For ESOFT errors, just continue processing the remainder of
	 * the request.
	 */
	if (pb->pb_op == FIX_ESOFT) {
	    pb->pb_addr += pb->pb.b_resid;
	    pb->pb.b_error = pb->pb.b_resid = 0;
	    pb->pb.b_flags &= ~B_ERROR;
	}
	else 
	    pb->pb.b_error = EIO;
	SET_BB_STATUS(pb->pb_bad, REL_DESIRED);
	return;
    }

    /*
     * First try at SW relocation or SW reloc failed w/ EMEDIA.  Get an
     * alternate block and write the old block to the alternate.
     */
    if (BB_STATUS(bad) != REL_PENDING ||
	(BB_STATUS(bad) == REL_PENDING && pb->pb.b_error == EMEDIA)) { 
	if (++pb->pb_swretry < MAX_SWRETRY) {
	    if (pb->pb_pvol->altpool_next < pb->pb_pvol->altpool_end) {
		SET_BB_ALTERNATE(bad, pb->pb_pvol->altpool_next++);
		SET_BB_STATUS(bad, REL_PENDING);

		/* turn off error flag and HW relocate flag */
		pb->pb.b_flags &= ~(B_ERROR | B_HWRELOC);
		pb->pb.b_blkno = BB_ALTERNATE(bad);
		return;
	    }
	}
	/*
	 * SW relocation isn't possible.  Either retry count was exceeded, or
	 * the alternate block supply is exhausted.  Set the PV state to Read
	 * Only Relocation, and pass the bad news to the scheduler.
	 */
	if (pb->pb_op != FIX_ESOFT) {
	    SET_BB_STATUS(pb->pb_bad, REL_DESIRED);
	    SET_BB_ALTERNATE(pb->pb_bad, NULL);
	    pb->pb.b_flags |= B_ERROR;
	    pb->pb.b_error = EIO;
	}
	else {
	    /*
	     * Well, except for soft errors.  The original request succeeded,
	     * so just pretend nothing bad happened.  Update the pbuf, recover
	     * the bad block alternate and continue.
	     */
	    pb->pb.b_flags |= B_READ;
	    pb->pb.b_flags &= ~B_ERROR;
	    pb->pb.b_error  = NULL;
	    pb->pb_op       = NULL;
	    pb->pb_addr     = pb->pb_endaddr;
	    lv_deletedefect(pb->pb_pvol, pb->pb_bad);
	    pb->pb_bad = NULL;
	}

	/* But either way, the pvol is now Read Only Relocate. */
	pb->pb_pvol->pv_flags |= LVM_PVRORELOC;
	return;
    }

    /*
     * An error occurred during relocation which was not a media
     * failure.  Just reset the defect entry and let the scheduler do
     * the rest.
     */
    if (pb->pb_op != FIX_ESOFT) {
	SET_BB_STATUS(pb->pb_bad, REL_DESIRED);
	SET_BB_ALTERNATE(pb->pb_bad, NULL);
	return;
    }

    /*
     * Special case:  a soft error was detected and relocation attempted.
     * The original request worked, so continue as if nothing bad happened.
     * Update the pbuf address so the request can complete.  If the block
     * goes bad later, we'll handle it then.
     */
    pb->pb.b_flags |= B_READ;
    pb->pb.b_flags &= ~B_ERROR;
    pb->pb.b_error = NULL;
    pb->pb_op = NULL;
    pb->pb_addr = pb->pb_endaddr;

    /*
     * Delete kernel bad_blk struct
     */
    lv_deletedefect(pb->pb_pvol, pb->pb_bad);
    pb->pb_bad = NULL;
    return;
}

/*
 *  NAME:         lv_hwreloc
 * 
 *  FUNCTION:
 *	For WRITE requests only, set up request for HW relocation for the bad
 *	block.
 *
 *  NOTES:
 *	input:	physical device buf, operation completed with error.
 *	output:	pbuf set up for HW relocation and put on readyQ to be sent
 * 		back to disk device driver
 *
 *  PARAMETERS:
 *	pb - physical buf struct 
 */
void
lv_hwreloc(pb)
struct pbuf *pb;			/* physical request to process */
{
    lv_bblk_t   *defect = NULL;		/* bad block structure	       */
    int         i;			/* index to defect hash chain  */
    struct pvol *pv;			/* pbuf's pvol structure       */

    pv = pb->pb_pvol;

    /*
     * If this isn't the second attempt to fix and ESOFT error, then
     * make sure relocation is allowed, and allocate a defect entry in the
     * defect directory.  If this is the second attempt, the entry is
     * already allocated.
     */
    if (pb->pb_op == FIX_ESOFT)
	    defect = pb->pb_bad;
    else {
	/*
	 * A new defect entry is needed.  Exception: if pb_bad is not
	 * NULL, the defect entry is already available.  This is the
	 * case of an error accessing an alternate pool block.  If no
	 * pb_bad exists,  find the first open entry in the bad block
	 * directory.
	 */
	if (pb->pb_bad) {
	    defect = pb->pb_bad;
	} else {
	    for (i = 1; i < pv->pv_maxdefects; i++) {
		if (!BB_REASON(&pv->pv_bbdir[i])) {
		    defect = &pv->pv_bbdir[i];
		    break;
		}
	    }

	    /*
	     * there are no slots available in the bad block directory,
	     */
	    if (defect == NULL) {
		/*
		 * Return and continue processing, just don't allow any new HW
		 * or SW relocation.  For ESOFT errors, just continue
		 * processing the remainder of the request.
		 */
		if (pb->pb.b_error == ESOFT) {
		    pb->pb_addr += pb->pb.b_resid;
		    pb->pb.b_error = pb->pb.b_resid = 0;
		    pb->pb.b_flags &= ~B_ERROR;
		    pb->pb_bad = NULL;
		} else
		    pb->pb.b_error = EIO;
		return;
	    }
	}

	/*
	 * Assign a lv_bblk struct for the relocated defect (if necessary)
	 * and fill in the defect structure.
	 */
	if (pb->pb_bad == NULL) {
	    lv_defecthash(pv, defect);
	    SET_BB_DEFECT(defect, pb->pb.b_blkno + BYTE2BLK(pb->pb.b_bcount -
							    pb->pb.b_resid));
	} else
	    if (pb->pb.b_error == ESOFT)
		pb->pb_op = FIX_ESOFT;
	SET_BB_STATUS(defect, REL_DESIRED);
	SET_BB_REASON(defect, DEFECT_SYS);
	pb->pb_bad = defect;
    }
    
    /*
     * For EMEDIA on reads, write out the bad block directory to the physical
     * volume, and notify the scheduler.
     */
    if ((pb->pb.b_flags & B_READ) && (pb->pb.b_error == EMEDIA)) {
	SET_BB_ALTERNATE(defect, NULL);
	lv_updatebbdir(pb);
    } else {
	/* 
	 * Attempt HW relocation on the one bad block on writes and ESOFT
	 * reads.
	 */
	SET_BB_STATUS(defect, REL_DEVICE);
	pb->pb.b_flags &= ~B_ERROR;
	pb->pb.b_blkno  = BB_DEFECT(defect);
	pb->pb.b_bcount = DEV_BSIZE;
	pb->pb.b_resid  = NULL;

	if (BB_ALTERNATE(defect))
	    /*
	     * Error on alternate block.  The original has already been
	     * shown to be bad, and the alternate is just dropped.  Software
	     * relocation will replace it with another.
	     */
	    lv_swreloc(pb);
	else
	    /*
	     * For soft error (read succeeded) attempt to rewrite the block
	     * with write verify set.  Record this as a soft error fix
	     * attempt.  If this has been tried and failed, try hardware
	     * relocation.
	     */
	    if (pb->pb.b_flags & B_READ && pb->pb.b_error == ESOFT
		&& pb->pb_op == NULL) {
		pb->pb.b_flags &= ~B_READ;
		pb->pb.b_flags |= B_WRITEV;
		pb->pb_op = FIX_ESOFT;
	    } else
		pb->pb.b_flags |= B_HWRELOC;
    }
}

/*
 *  NAME:         lv_relocdone 
 * 
 *  FUNCTION:
 *	Successful relocated physical request completion.
 *
 *  NOTES:
 *	input:	physical buf structure after bad block I/O operation completed.
 *	output:	defect status updated, if necessary.
 *
 *  PARAMETERS:
 *	pb - physical buf struct 
 *	bad - pointer to bad block hash chain
 */
void
lv_relocdone(pb)
struct pbuf *pb;	/* physical request to process */
{
struct lv_bblk *bad = pb->pb_bad;	/* bad block structure */

    /* branch on bad block state */
    switch(BB_STATUS(bad))  {
      case REL_DONE:
	break;

      case REL_PENDING:
	/*
	 * S/W relocation completed succesfully.  The entry is in the defect
	 * directory, but needs to be written out to disk.
	 */
	SET_BB_STATUS(bad, REL_DONE);
	if (pb->pb_op == FIX_ESOFT) {
	    /*
	     * For soft errors, reset the B_READ flag, update the pbuf
	     * address, and continue processing the request.
	     */
	    pb->pb.b_flags |= B_READ;
	    pb->pb_addr = pb->pb_endaddr;
	}
	pb->pb_op = pb->pb_swretry = NULL;
	lv_updatebbdir(pb);
	break;

      case REL_DEVICE:
	/*
	 * Successful H/W relocation or rewrite (verified)
	 *
	 * For soft errors, the defect directory on disk does not contain
	 * this entry; only the copy in memory was updated.  Remove the
	 * entry from the table, and return the defect structure to the free
	 * list.
	 */
	if (pb->pb_op == FIX_ESOFT) {
	    /* 
	     * Set request back to READ to complete the original request.  The
	     * original READ request was successful; update the pbuf address
	     * to allow request processing to continue. 
	     */
	    pb->pb.b_flags |= B_READ;
	    pb->pb_addr = pb->pb_endaddr;
	}
	pb->pb.b_flags &= ~B_ERROR;
	pb->pb.b_error = NULL;
	pb->pb_bad = NULL;
	pb->pb_op = pb->pb_swretry = NULL;
	lv_deletedefect(pb->pb_pvol, bad);
	break;

      case REL_DESIRED:
	/* Relocation desired - impossible case */
	panic("lv_relocdone: invalid relocation status");
	break;

    }  /* end -- switch on bad block status */
    return;
}

/*
 *  NAME:         lv_initdefects
 * 
 *  FUNCTION:
 *	This routine initializes the defect hash table for a given pvol's
 *	defect list.  This hash table is used to check block numbers for
 *	residence in the defect list.
 *
 *  NOTES:
 *	input:	pvol structure pointer for the device in question
 *	output:	initialized hash table in that pvol
 *
 *  PARAMETERS:
 *	pv - pvol structure pointer
 */
int
lv_initdefects(pv, maxdefects)
struct pvol *pv;			/* Physical volume to process	*/
int maxdefects;				/* Max size of defect directory */
{
    struct dirbuf {			/* A convenient structure to	*/
	char b[DEV_BSIZE];		/*     ease address calculation */
    } *endp, *startp;			/* Pointer to pv_bbdir blocks	*/
    int	i,				/* Loop index			*/
	currentdefects = 0,		/* Current defect count		*/
        dirsize,			/* Size of defect directory	*/
        status;				/* Subroutine return status	*/
    lv_bblk_t  *dp,			/* Defect structure pointer	*/
	       *newdir;			/* Replacement defect directory */

    /*
     * Because this routine can be called twice on the same physical volume,
     * free up the bad block directory space and the freelist space on entry.
     */
    KFREE(pv->pv_bbdir, pv->bbdirsize); pv->bbdirsize = 0;
    KFREE(pv->freelist, pv->freelistsize); pv->freelistsize = 0;

    /* Read in the bad block directory */
    if ((status = lv_bbdirinit(pv)) != ESUCCESS)
	return(status);

    /*
     * Determine the appropriate length for the in-memory defect directory.
     * Also count the number of defects it currently contains.  The directory
     * is terminated by a block of nulls.
     */
    startp = (struct dirbuf *)pv->pv_bbdir;
    for (endp = startp; endp < &startp[PVRA_BBDIR_LENGTH]; endp++) {
	if (biszero((char *)endp, DEV_BSIZE))
	    break;
	for (dp = (lv_bblk_t *)endp; dp < (lv_bblk_t *)(endp + 1); dp++)
	    if (BB_REASON(dp))
		currentdefects++;
    }
    /*
     * Adjust maxdefects to be the greater of the requested and existing
     * defect count, but not larger than the size of the bad block directory.
     */
    dirsize    = (PVRA_BBDIR_LENGTH-1) * DEV_BSIZE / sizeof(lv_bblk_t);
    maxdefects = MIN(dirsize, MAX(maxdefects, currentdefects));

    /*
     * Round the maxdefect count up to the next block boundary.  The -1 term
     * is to account for the fact that the first entry in the defect directory
     * is the string "DEFECT01", and therefore unusable.
     */
    maxdefects = (((maxdefects >> DEFECTSHIFT) + 1) << DEFECTSHIFT) - 1;

    endp = &startp[BYTE2BLK((maxdefects+1) * sizeof(lv_bblk_t)) + 1];

    /*
     * If the new directory is smaller than the original, replace it, and save
     * some memory.
     */
    if (endp < &startp[PVRA_BBDIR_LENGTH]) {
	dirsize = (endp - startp) * DEV_BSIZE;
	newdir = (lv_bblk_t *)kalloc(dirsize);
	if (newdir) {
		bcopy(pv->pv_bbdir, newdir, dirsize);
		KFREE(pv->pv_bbdir, pv->bbdirsize);
		pv->bbdirsize = dirsize;
		pv->pv_bbdir = newdir;
	}
    }

    /*
     * Allocate sufficient links for the free pool to handle any new
     * defects.  Currently, this is the same size as the bad block
     * directory, but don't count on this always being so.  Then link them all
     * together.
     */
    pv->freelist = (lv_defect_t *)kalloc(maxdefects * sizeof(lv_defect_t));
    if (pv->freelist) {
        pv->freelistsize = maxdefects * sizeof(lv_defect_t);
	bzero(pv->freelist, maxdefects * sizeof(lv_defect_t));
	for (i = 0; i < maxdefects-1; i++)
	    pv->freelist[i].next = &pv->freelist[i+1];
    }

    /* Initialize the defect relocation pool fields in the pvol */
    pv->altpool_psn = pv->altpool_next = pv->pv_lvmrec->altpool_psn;
    pv->altpool_end = pv->altpool_psn + pv->pv_lvmrec->altpool_len;

    /*
     * Loop through the defect table (skipping the invalid first entry),
     * hashing the entries into the defect hash table in the pvol structure.
     */
    for (dp = &pv->pv_bbdir[1]; dp < &pv->pv_bbdir[maxdefects]; dp++)
	/* If defect entry is valid, add it to the hash table. */
	if (BB_REASON(dp) && pv->freelist) {
	    lv_defecthash(pv, dp);
	    if (BB_ALTERNATE(dp) > pv->altpool_next)
		pv->altpool_next = BB_ALTERNATE(dp);
	}

    /*
     * If there are no free alternates available, or the free list is
     * exhausted, mark the pvol to only allow read-only relocation.
     */
    if (++pv->altpool_next > pv->altpool_end || pv->freelist == NULL)
	pv->pv_flags |= LVM_PVRORELOC;
    return(ESUCCESS);
}

/*
 *  NAME:         lv_defecthash
 * 
 *  FUNCTION:
 *	This function enters a defect into the defect hash table for later
 *	hashed lookup.
 *
 *  NOTES:
 *	input:	pvol structure pointer for the device in question
 *		defect structure pointer for a given defect
 *	output:	updated hash table containing that defect
 *
 *  PARAMETERS:
 *	pv - pvol structure pointer
 *	dp - defect structure pointer
 */
void lv_defecthash(pv, dp)
struct pvol *pv;		/* physical volume to process */
lv_bblk_t *dp;			/* Defect structure pointer   */
{
    lv_defect_t *hp, 		/* Hash table pointer	  */
		**prev;		/* Previous entry pointer */

    /*
     * Insert defects into the hash chain in ascending order.  This
     * prevents exhaustive search of the chain during bad block
     * lookup.
     */
    prev = &pv->pv_defects[BBHASHINX(BB_DEFECT(dp))];
    hp = *prev;
    while (hp && BB_DEFECT(dp) > BB_DEFECT(hp->defect)) {
	prev = &hp->next;
	hp = *prev;
    }
    *prev = pv->freelist;
    pv->freelist = (*prev)->next;
    (*prev)->next = hp;
    (*prev)->defect = dp;
}

/*
 *  NAME:         lv_deletedefect
 * 
 *  FUNCTION:
 *	This function deletes a defect entry from the defect table, updating
 *	the on-disk copy if necessary.
 *
 *  NOTES:
 *	input:	pvol structure pointer for the device in question
 *		defect structure pointer for a given defect
 *	output:	updated defect table without that defect
 *
 *  PARAMETERS:
 *	pv - pvol structure pointer
 *	dp - defect structure pointer
 */
void lv_deletedefect(pv, dp)
struct pvol *pv;		/* physical volume to process */
lv_bblk_t *dp;			/* Defect structure pointer   */
{
    lv_defect_t *hp, 		/* Hash table pointer	  */
		**prev;		/* Previous entry pointer */

    /*
     * Remove defect entry from the hash chain.  Return the removed defect
     * node to the free list.
     */
    prev = &pv->pv_defects[BBHASHINX(BB_DEFECT(dp))];
    hp = *prev;
    while (BB_DEFECT(dp) != BB_DEFECT(hp->defect)) {
	prev = &hp->next;
	hp = *prev;
    }
    *prev = hp->next;
    hp->defect->defect_reason = hp->defect->alternate_status = NULL;
    hp->defect = NULL;
    hp->next = pv->freelist;
    pv->freelist = hp;
}

/*
 *  NAME:         lv_bbdirinit
 * 
 *  FUNCTION:
 *	This function reads in the defect directory from the disk to an in
 *	memory copy.
 *
 *  NOTES:
 *	input:	pvol structure pointer for the device in question
 *
 *  PARAMETERS:
 *	pv - pvol structure pointer
 *
 *  RETURN VALUE:
 *	ESUCCESS if the read went OK
 *	the appropriate error code from the read if it failed
 */
int
lv_bbdirinit(pv)
struct pvol *pv;
{
    register struct buf *bp;
    struct volgrp *vg;
    int error, failcount, switchends;

    pv->bbdirsize = PVRA_BBDIR_LENGTH*DEV_BSIZE;
    if ((pv->pv_bbdir = (lv_bblk_t *)kalloc(pv->bbdirsize)) == NULL) {
	pv->bbdirsize = 0;
	return(ENOMEM);
    }
    /*
     * Read in the primary bad block directory.  It the read fails, read
     * the secondary bbdir.
     */
    bp = &(VG_LVOL0(pv->pv_vg)->lv_rawbuf);

    BUF_LOCK(bp);

    bp->b_flags = B_BUSY|B_READ;
    bp->b_blkno = PVRA_BBDIR_SN1;
    bp->b_bcount = pv->bbdirsize;
    bp->b_un.b_addr = (caddr_t)pv->pv_bbdir;
    bp->b_vp = NULL;
    bp->b_dev = pv->pv_vp->v_rdev;
    switchends = PVRA_BBDIR_SN2 - PVRA_BBDIR_SN1;

    for (failcount = 0; error = lv_bufio(bp) && (failcount < 2); ) {
	bp->b_flags = B_BUSY|B_READ;
	bp->b_blkno += switchends;
	switchends = -switchends;
	if (bp->b_bcount == bp->b_resid)
	    failcount++;
	else
	    failcount = 1;
    }
    if (error) {
	KFREE(pv->pv_bbdir, pv->bbdirsize);
	pv->bbdirsize = 0;
    }

    BUF_UNLOCK(bp);
    return(error);
}

/*
 *  NAME:         lv_updatebbdir
 * 
 *  FUNCTION:
 *	This function writes out a changed defect directory block to the two
 *	on-disk copies.
 *
 *  NOTES:
 *	input:	pbuf structure pointer for the directory block to be updated.
 *
 *  PARAMETERS:
 *	pb - pbuf structure pointer
 */
void
lv_updatebbdir(pb)
struct pbuf *pb;
{
    int size, locked;
    struct buf *bp;
    struct buf *ep;
    struct pvol *pv;

    pv = pb->pb_pvol;
    LOCK_INTERRUPT(&pv->pv_intlock);

    pb->pb_op = BBDIR_UPDATE_PENDING;
    size = btodb((caddr_t)pb->pb_bad - (caddr_t)pv->pv_bbdir);

    bp = &pv->pv_buf;
    BUF_LOCK_TRY(bp,locked);
    if (!locked) {
	/*
	 * If the buffer is busy, there is someone on the queue.  Add this
	 * buffer to the end of queue. The first pbuf on the queue serves
	 * as the queue head. The queue is formed through the pb.b_forw and
	 * pb.b_back pointers.
	 */
		/* ep points to current end of list */
	ep = bp->b_forw->b_back;
		/* hang this (p)buf off the end */
	ep->b_forw = (struct buf *)pb;
		/* set the end pointer to the new (p)buf */
	bp->b_forw->b_back = (struct buf *)pb;

	UNLOCK_INTERRUPT(&pv->pv_intlock);
    } else {
	UNLOCK_INTERRUPT(&pv->pv_intlock);

	ASSERT(bp->b_forw == NULL);
	/*
	 * The queue is empty.  Set the busy bit and put this pbuf on the
	 * front of the queue.
	 */
	/* This pbuf becomes the waiting pbuf anchor */
	pb->pb.b_forw 	= NULL;	/* last on waiting pbuf list */
	pb->pb.b_back	= (struct buf *)pb;

	bp->b_flags	= B_BUSY|B_WRITE|B_ASYNC;
	bp->b_forw	= (struct buf *)pb;
	bp->b_back	= (struct buf *)pv;
	bp->b_blkno	= PVRA_BBDIR_SN1 + size;
	bp->b_bcount	= DEV_BSIZE;
	bp->b_un.b_addr = ((caddr_t)pb->pb_pvol->pv_bbdir) + dbtob(size);
	bp->b_vp	= NULL;
	bp->b_iodone	= lv_bbdirend;
	bp->b_dev 	= pb->pb.b_dev;

	BUF_GIVE_AWAY(bp);
	LV_QUEUEIO(pv, bp);
	lv_startpv(pv);
    }
    return;
}

/*
 *  NAME:         lv_bbdirend
 * 
 *  FUNCTION:
 *	This function is the completion routine for lv_updatebbdir.
 *
 *  NOTES:
 *	input:	pbuf structure pointer for the directory block updated.
 *
 *  PARAMETERS:
 *	bp - buf structure pointer
 */
void
lv_bbdirend(bp)
struct buf *bp;
{
    int size;
    struct pbuf *pb;
    struct pvol *pv;

    LASSERT(BUF_IS_LOCKED(bp));

    pv = (struct pvol *)bp->b_back;
    LOCK_INTERRUPT(&(pv->pv_intlock));
    pv->pv_curxfs--;

    /*
     * If the write was not in the range of the second defect directory, then
     * this must be the completion of the first copy.  Adjust the block number
     * and write the second copy.
     */
    if (bp->b_blkno < PVRA_BBDIR_SN2) {
	UNLOCK_INTERRUPT(&(pv->pv_intlock));
	bp->b_iodone =  lv_bbdirend;
	bp->b_blkno += PVRA_BBDIR_SN2 - PVRA_BBDIR_SN1;

	LV_QUEUEIO(pv, bp);
	lv_startpv(pv);
    } else {
	/*
	 * The second write completed.  Notify the scheduler of the error,
	 * then begin the whole process again for any subsequent defect
	 * directory updates.
	 */
	pb = (struct pbuf *)bp->b_forw;
	/*
	 * Remove this pbuf from the front of the queue, the next pbuf
	 * (if any) becomes the queue anchor.
	 */
	if ((bp->b_forw = pb->pb.b_forw) != NULL) {
		bp->b_forw->b_back = pb->pb.b_back;
	}
	UNLOCK_INTERRUPT(&(pv->pv_intlock));
	pb->pb_op = NULL;
	pb->pb_bad = NULL;
	if ((pb->pb.b_flags & B_ERROR) || (pb->pb_addr == pb->pb_endaddr)) {
		LV_SCHED_DONE(pb);
	} else {
		lv_resume(pb);
	}

	/*
	 * Now begin bad block updates for any other pbufs waiting for the
	 * pvol buffer.
	 */
	LOCK_INTERRUPT(&(pv->pv_intlock));
	if (pb = (struct pbuf *)bp->b_forw) {
	    UNLOCK_INTERRUPT(&(pv->pv_intlock));
	    size = btodb((caddr_t)pb->pb_bad - (caddr_t)pb->pb_pvol->pv_bbdir);
	    bp->b_iodone = lv_bbdirend;
	    bp->b_blkno = PVRA_BBDIR_SN1 + size;
	    bp->b_un.b_addr = ((caddr_t)pb->pb_pvol->pv_bbdir) + dbtob(size);

	    LV_QUEUEIO(pv, bp);
	    lv_startpv(pv);
	} else {
	    BUF_ACCEPT(bp);
	    BUF_UNLOCK(bp);
	    UNLOCK_INTERRUPT(&(pv->pv_intlock));
	}
    }
    return;
}

/*
 * int biszero(addr, count)
 *
 * Check if a region of memory is all zeroes. Brute force version
 * for machines that can't make all the non-portable assumptions
 * about interconversion of pointer types and longs.
 */
static int
biszero(addr, count)
char *addr;
int count;
{
register int result = 0;
#if NON_PORTABLE
	if (count >= sizeof(long)) {
		switch ((long)addr & ((sizeof(long))-1)) {
		default: panic("long to long: write more code");
			break;
		case 3: result |= *addr++; count--;	/* FALLTHROUGH */
		case 2: result |= *addr++; count--;	/* FALLTHROUGH */
		case 1: result |= *addr++; count--;	/* FALLTHROUGH */
		case 0:
			break;
		}
		/* p is now long-aligned */
		while (count >= sizeof(long)) {
			count -= sizeof(long);
			result |= *(long *)addr;
			addr += sizeof(long);
		}
	}
#endif
	while (count && !result) {
		result |= *addr++; count--;
	}
	return(!result);
}
