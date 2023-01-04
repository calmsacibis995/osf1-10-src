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
/*
 * @(#)$RCSfile: ccfg.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/25 17:47:51 $
 */
#ifndef _CCFG_INCL_
#define _CCFG_INCL_

/* ---------------------------------------------------------------------- */

/* ccfg.h		Version 1.08			Nov. 16, 1991 */

/*  This file contains the definitions and data structures for the 
    configuration driver module, CDrv, in the CAM subsystem.

Modification History

	Version	  Date		Who	Reason

	1.00	12/28/90	jag	Creation date.
	1.01	02/08/91	jag	Added the control struct def. and 
					misc flags.
	1.02	03/15/91	jag	Changed file name, added the specific
					IPL/SMP locks for the structures.
	1.03	04/07/91	jag	Added the SMP lock init macros.
	1.04	06/04/91	jag	Added a label to the struct declar.
	1.05	06/05/91	jag	Added a counter for the XPT in the
					EDT structure.
	1.06	07/03/91	jag	Updated from the code review.
	1.07	09/12/91	jag	Added the defines for the scanning
					timeout code and SDTR handling.
	1.08	11/16/91	jag	Added the CCFG_CTRL_VERS define.
*/

/* ---------------------------------------------------------------------- */

/* Misc defines used within the CDrv code. */

/* ---------------------------------------------------------------------- */

/* The CDrv control structure is used for various functions w/in the CDrv.
It contains module wide flags for the scanning process, and it also contains
the shared space for the INQUIRY data. */

typedef struct ccfg_ctrl
{
    U32 ccfg_flags;		/* controlling flags */
    ALL_INQ_DATA inq_buf;	/* scratch area for the INQUIRY data */
    lock_data_t c_lk_ctrl;	/* for locking on the control struct */
} CCFG_CTRL;

#define CCFG_CTRL_VERS	1	/* please incr if CCFG_CTRL changes */

/* The IPL/SMP locking Macros for the control structure. */

#define CTRL_INIT_LOCK( cp )                           \
{           					       \
   lock_init(&((cp)->c_lk_ctrl), TRUE );  	       \
}

#define CTRL_IPLSMP_LOCK( saveipl, cp )                \
{                                                      \
    saveipl = splbio();                                \
    CAM_LOCK_IT( &((cp)->c_lk_ctrl), LK_RETRY );       \
}

#define CTRL_IPLSMP_UNLOCK( saveipl, cp )              \
{                                                      \
    CAM_UNLOCK_IT( &((cp)->c_lk_ctrl) );               \
    splx(saveipl);  	 	                       \
}

#define CTRL_SMP_SLEEPUNLOCK( chan, pri, cp )          \
{                                                      \
    CAM_SLEEP_UNLOCK_IT( (chan), (pri), &((cp)->c_lk_ctrl) ); \
}

#define CTRL_SMP_LOCK( cp )                            \
{                                                      \
    CAM_LOCK_IT( &((cp)->c_lk_ctrl), LK_RETRY );       \
}

/* ---------------------------------------------------------------------- */

/* The CDrv Queue Header structure.  This structure contains a PDrv
working set structure and a lock structure.  The PDrv working set is used
to allow the Q walking code to use a consistant pointer for the queue. */

typedef struct ccfg_qhead
{
    PDRV_WS qws;		/* the Q head working set */
    lock_data_t c_lk_qhead;	/* for locking on the control struct */
} CCFG_QHEAD;

/* The IPL/SMP locking Macros for the Q Head structure. */

#define QHEAD_INIT_LOCK( qp )                          \
{                                                      \
    lock_init( &((qp)->c_lk_qhead), TRUE ); 	       \
}

#define QHEAD_IPLSMP_LOCK( saveipl, qp )               \
{                                                      \
    saveipl = splbio();				       \
    CAM_LOCK_IT( &((qp)->c_lk_qhead), LK_RETRY );      \
}

#define QHEAD_IPLSMP_UNLOCK( saveipl, qp )             \
{                                                      \
    CAM_UNLOCK_IT( &((qp)->c_lk_qhead) );              \
    splx(saveipl);    	                       	       \
}

#define QHEAD_SMP_SLEEPUNLOCK( chan, pri, qp )         \
{                                                      \
    CAM_SLEEP_UNLOCK_IT( (chan), (pri), &((qp)->c_lk_qhead) );\
}

#define QHEAD_SMP_LOCK( qp )                           \
{                                                      \
    CAM_LOCK_IT( &((qp)->c_lk_qhead), LK_RETRY );      \
}

/* ---------------------------------------------------------------------- */

/* The EDT structure is used by the CDrv layer to keep the device inquiry
information.  The information is stored in an 8x8 array, based on the
number of targets and the number of LUNs per target. */

typedef struct edt
{
    CAM_EDT_ENTRY edt[ NDPS ][ NLPT ];		/* a layer for targets/LUNs */
    U32 edt_flags;				/* flags for EDT access */
    U32 edt_scan_count;				/* # of XPT ASYNC CB readers */
    lock_data_t c_lk_edt			/* for locking per bus */
} EDT;

/* The IPL/SMP locking Macros for the EDT structure. */

#define EDT_INIT_LOCK( ep )                            \
{                                                      \
    lock_init( &((ep)->c_lk_edt), TRUE );   	       \
}

#define EDT_IPLSMP_LOCK( saveipl, ep )                 \
{                                                      \
    saveipl = splbio();    	                       \
    CAM_LOCK_IT( &((ep)->c_lk_edt), LK_RETRY );        \
}

#define EDT_IPLSMP_UNLOCK( saveipl, ep )               \
{                                                      \
    CAM_UNLOCK_IT( &((ep)->c_lk_edt) );                \
    splx(saveipl);                                     \
}
 
#define EDT_SMP_SLEEPUNLOCK( chan, pri, ep )           \
{                                                      \
    CAM_SLEEP_UNLOCK_IT( (chan), (pri), &((ep)->c_lk_edt) );  \
}

#define EDT_SMP_LOCK( ep )                             \
{                                                      \
    CAM_LOCK_IT( &((ep)->c_lk_edt), LK_RETRY );        \
}

/* ---------------------------------------------------------------------- */

/* Flags used by the CDrv in it's module control structure flags field.  The
control structure contains the working area for the EDT scanning. */

#define EDT_INSCAN	0x00000010	/* signal an EDT scan in progress */
#define INQ_INPROG	0x00000020	/* signal an INQUIRY I/O in prog. */

/* Flags used by the CDrv in the PDrv working set flags field.  These
flags are an overlay of the bits used by the PDrv.  The CDrv is only
using the the same working set defined by the PDrv, there is no attempt
to use all the possible bit settings. */

#define CCB_RECEIVED	0x00000100	/* simple signal flag from callback */
#define ISSUE_WAKEUP	0x00000200	/* the CDrv is sleeping on the WSET */

#define ABORT_SENT	0x00010000	/* timeout code has sent an ABORT */
#define RESET_SENT	0x00020000	/* timeout code has sent a BUS RESET */

#define NO_SDTR		0x00100000	/* Try the CCB w/out SDTR enabled */
#define CCFG_RETRY	0x00800000	/* This CCD is in RETRY state */

/* ---------------------------------------------------------------------- */

/* Scanning arguments for the CDrv scanning code, ccfg_edtscan(). */

#define EDT_FULLSCAN	0x00		/* run through a full scan */
#define EDT_PARTSCAN	0x01		/* run through a partical scan */
#define EDT_SINGLESCAN	0x02		/* do a single nexus scan */

/* ---------------------------------------------------------------------- */

/* Flags used by the XPT and CDrv async handling code.  Each EDT structure
contains a flags field.  This flag is used by the XPT to inform the CDrv
that an Async Callback is in progress and that the async links can not
be disturbed.*/

#define ASYNC_CB_INPROG	0x00000001	/* Used by the XPT async call code */

/* ---------------------------------------------------------------------- */

/* Defines for the Delaying/Loop code. */

#define WAIT_DELAY	1000		/* delay # for 1 msec */
#define WAIT_DELAY_LOOP	2000		/* loop # for 2 seconds */
#define WAIT_RESET_LOOP 250		/* loop # for 250 msec */

#define WAIT_PRIORITY	0

#endif /* _CCFG_INCL_ */
