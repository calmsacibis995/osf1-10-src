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
static char	*sccsid = "@(#)$RCSfile: lv_phys.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:20:09 $";
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
 *
 * COMPONENT_NAME: (SYSXLVM) Logical Volume Manager Device Driver - hd_phys.c
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 *  Modification History:  lv_phys.c
 *
 *  24-Apr-91     Terry Carruthers
 *	Modified lv_end function to check for end of media condition
 *
 */

#include <sys/types.h>
#include <sys/errno.h>
#include <sys/conf.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/specdev.h>

#include <lvm/lvmd.h>

extern void lv_fixup(); /* Used in IS_FIXUP macro */
extern void lv_end();	/* Completion routine */
extern void lv_startpv();

#define ERELOCATED	125

/*
 * NAME:         lv_begin
 *
 * FUNCTION:
 *	Begin physical I/O operations.
 *
 * PARAMETERS:
 *	pb - a physical struct buf that describes the I/O to be
 *	performed.
 *
 * This routine is never called from within an interrupt service routine.
 */
void
lv_begin(pb)
register struct pbuf *pb;
{
struct pvol *pv;

    /* fill in physical request parameters */
    pv = pb->pb_pvol;
    pb->pb.b_dev    = pv->pv_vp->v_rdev;
    pb->pb.b_blkno  = pb->pb_start;
    pb->pb_swretry  = pb->pb_op = NULL;

    if (lv_validblk(pb)) {		/* check for bad block relocation */
	if (pb->pb.b_bcount) {
	    pb->pb.b_iodone = lv_end;

	    LV_QUEUEIO(pv,&pb->pb);

	    lv_startpv(pv);	/* Initiate I/O */
	} else {
	    LV_SCHED_DONE(pb);		/* Inform scheduler */
	}
    } else  {				/* incorrigible bad block? */
	if (IS_FIXUP(pb))
	    panic("lv_begin: lv_validblk returned false on fixup!\n");
	pb->pb.b_flags |= B_ERROR;
	pb->pb.b_error  = EMEDIA;	/* simulate a media error */
	LV_SCHED_DONE(pb);		/* inform scheduler */
    }
}

/*
 *  NAME:	lv_end 
 * 
 *  FUNCTION:	Physical operation iodone() routine
 *
 *  NOTES:	This routine is scheduled to run as an offlevel interrupt 
 *	  	handler when the physical device driver calls iodone().
 *		input:  physical buf for a completed request;
 *		b_resid and  b_error set by physical device driver if
 *		b_flags&B_ERROR.
 *		b_error = EIO - non-media I/O error
 *			  EMEDIA - newly grown media error
 *			  ESOFT - request succeeded, but needs 
 *				relocation (may not work next time)
 *
 *  PARAMETERS:   pb - physical buf struct 
 */
void
lv_end(pb)
register struct pbuf *pb;	/* physical device buf struct */
{
struct pvol *pv;

    /* update buffer address by number of bytes processed */
    pb->pb_addr += pb->pb.b_bcount - pb->pb.b_resid;
    pv = pb->pb_pvol;
    LOCK_INTERRUPT(&pv->pv_intlock);
    pv->pv_curxfs--;
    UNLOCK_INTERRUPT(&pv->pv_intlock);

    /*
     * If the operation just performed required relocation, determine
     * whether it succeeded.  If not, attempt software relocation; otherwise
     * update the defect directory to reflect changes.
     */
    if (pb->pb_bad && !(pb->pb.b_flags & B_READ) &&
	BB_STATUS(pb->pb_bad) != REL_DONE) {
	/*
	 * If the B_WRITEV flag is still set, then an ESOFT fix via write
	 * verification failed.  Try hardware relocation.
	 */
	if (pb->pb.b_flags & B_WRITEV) {
	    pb->pb.b_flags |= B_ERROR | B_READ;
	    pb->pb.b_flags &= ~B_WRITEV;
	    pb->pb.b_error  = ESOFT;
	} else {
	    /*
	     * Successful hardware relocation will set B_ERROR and return
	     * ERELOCATED; any other return indicates software relocation
	     * should be attempted.
	     */
	    if (pb->pb.b_flags & B_ERROR && pb->pb.b_error != ERELOCATED) {
		lv_swreloc(pb);
	    } else {
		lv_relocdone(pb);
	    }
	}
    }

    /*
     * If the operation detected either a media surface error or a soft
     * error from the physical driver, attempt hardware relocation.
     */
    if (pb->pb.b_flags & B_ERROR  &&
	((pb->pb.b_error == EMEDIA) || (pb->pb.b_error == ESOFT)))
	lv_hwreloc(pb);

    /*
     * If we aren't updating the defect directory, determine if I/O is
     * complete or if some other kind of error occured, and call the
     * scheduler completion routine.  Otherwise, if the entire I/O is
     * incomplete, continue with the rest of the buffer.
     */
    if (pb->pb_op != BBDIR_UPDATE_PENDING) {

	/* ULTRIX/OSF:  Added code to check for end-of-media
	 *
	 *         If not doing a relocation operation then 
	 *         all physical I/O operations should complete
	 *         (even if it is with an error).  However if
	 *         the operation returns with residual data and no error
	 *         then the I/O reached the physical end-of-media.
	 *         This really should never happen, but better check ...
	 */
	if ( !pb->pb_bad                  && 
	     !(pb->pb.b_flags & B_ERROR)  && 
	     (pb->pb.b_resid > 0)           )
	   pb->pb.b_flags |= B_ERROR;

	if ((pb->pb.b_flags & B_ERROR) || (pb->pb_addr == pb->pb_endaddr)) {
	    LV_SCHED_DONE(pb);
	} else {
	    if (pb->pb_bad && BB_STATUS(pb->pb_bad) != REL_DONE) {
		pb->pb.b_iodone = lv_end;

		LV_QUEUEIO(pv,&pb->pb);

		if (thread_call_one(&lv_threadq, lv_startpv, pv) == FALSE) {
			panic("lv_end thread_call");
		}
	    } else {
		lv_resume(pb);
	    }
	}
    }
}

/*
 * NAME:	lv_startpv
 *
 * FUNCTION:	Initiate all I/O pending for a given physical volume.
 *		Must not be called from within an interrupt
 *		service routine (biodone) to prevent recursion
 *		and reentry of the physical disk driver.
 */
void
lv_startpv(pv)
struct pvol *pv;
{
register struct buf *bp;

	LOCK_INTERRUPT(&(pv->pv_intlock));
	while (LV_QUEUE_FETCH(&(pv->pv_ready_Q), bp),bp) {
		UNLOCK_INTERRUPT(&(pv->pv_intlock));
		event_clear(&(bp->b_iocomplete));
		(*bdevsw[major(bp->b_dev)].d_strategy)(bp);
		LOCK_INTERRUPT(&(pv->pv_intlock));
	}
	UNLOCK_INTERRUPT(&(pv->pv_intlock));
	return;
}

/*
 *  NAME:        lv_resume 
 * 
 *  FUNCTION:    Resume physical I/O operation.         
 *
 *  NOTES:     	 input:   physical buf struct contents:
 *		    pb_start = physical address translation for lb->b_blkno.
 *		    b_un.b_addr = next address in buffer.
 *
 *		 output:  if successful, physical request added to ready_Q;
 *		    otherwise, calls scheduler's operation end routine
 *		    with simulated I/O error status.
 *
 *  PARAMETERS:  pb - physical buf struct 
 */
void
lv_resume(pb)
struct pbuf *pb;		/* physical device buf struct */
{
    struct pvol *pv;
    int bytes_done;		/* bytes already processed */

    /* compute next physical block number and remaining byte count */
    bytes_done	    = pb->pb_addr - pb->pb_startaddr;
    pb->pb.b_bcount = pb->pb_endaddr - pb->pb_addr;
    pb->pb.b_blkno  = pb->pb_start + BYTE2BLK(bytes_done);

    if (lv_validblk(pb)) {		/* check for bad block relocation */
	if (pb->pb.b_bcount) {
	    pv = pb->pb_pvol;
	    pb->pb.b_iodone = lv_end;

	    LV_QUEUEIO(pv,&pb->pb);

	    if (thread_call_one(&lv_threadq, lv_startpv, pv) == FALSE) {
		panic("lv_resume thread_call");
	    }
	} else {
	    LV_SCHED_DONE(pb);		/* Inform scheduler */
	}
    } else {				/* incorrigible bad block? */
	if (IS_FIXUP(pb))
	    panic("lv_begin: lv_validblk returned false on fixup!\n");
	pb->pb.b_flags |= B_ERROR;
	pb->pb.b_error  = EMEDIA;	/* simulate a media error */
	LV_SCHED_DONE(pb);		/* inform scheduler */
    }
}
