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
static char *rcsid = "@(#)$RCSfile: mscp_bbrstates.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/13 16:24:56 $";
#endif
/*
 *	mscp_bbrstates.c	4.1	(ULTRIX)	7/2/90
 */

/*
 *
 *   Facility:	Systems Communication Architecture
 *		Disk Class Driver
 *
 *   Abstract:	This module contains the state tables associated with
 *		the bad block relacement portion of the disk class
 *		driver.
 *
 *   Author:	David E. Eiche	Creation Date:	October 15, 1987
 *
 *   History:
 *
 *   19-Feb-1991	Tom Tierney
 *	ULTRIX to OSF/1 port:
 *
 *
 *   07-Mar-1989	Todd M. Katz		TMK0002
 *	1. Include header file ../vaxmsi/msisysap.h.
 *	2. Use the ../machine link to refer to machine specific header files.
 *
 *   27-Jul-1988	Pete Keilty
 *	1. Changed state table states for step15, step15a now uses BBRSUCCESS
 *	   event.
 *	2. Changed step online & step0b on BBRERROR goto step12e
 *
 *   02-Jun-1988     Ricky S. Palmer
 *	Removed inclusion of header file ../vaxmsi/msisysap.h
 *
 *   17-Apr-1988	Ricky S. Palmer
 *	Include header file "../vaxmsi/msisysap.h".
 *
 */

/**/

/* Libraries and Include Files.
 */
#include	<labels.h>
#include	<sys/types.h>
#include	<dec/binlog/errlog.h>
#include	<sys/disklabel.h>
#include	<io/common/pt.h>
#include	<io/dec/scs/sca.h>
#include	<io/dec/sysap/sysap.h>
#include	<io/dec/sysap/mscp_bbrdefs.h>

/* External Variables and Routines.
 */
extern	u_long		mscp_alloc_msg();
extern	u_long		mscp_alloc_rspid();
extern	u_long		mscp_map_buffer();
extern	u_long		mscp_invevent();
extern	u_long		mscp_noaction();
extern	u_long		mscp_bbr_step0();
extern	u_long		mscp_bbr_step0a();
extern	u_long		mscp_bbr_step0b();
extern	u_long		mscp_bbr_step0c();
extern	u_long		mscp_bbr_step1();
extern	u_long		mscp_bbr_step4();
extern	u_long		mscp_bbr_step4a();
extern	u_long		mscp_bbr_step5();
extern	u_long		mscp_bbr_step6();
extern	u_long		mscp_bbr_step6a();
extern	u_long		mscp_bbr_step7();
extern	u_long		mscp_bbr_step7a();
extern	u_long		mscp_bbr_step7b();
extern	u_long		mscp_bbr_step7c();
extern	u_long		mscp_bbr_step8();
extern	u_long		mscp_bbr_step9();
extern	u_long		mscp_bbr_step10();
extern	u_long		mscp_bbr_step11();
extern	u_long		mscp_bbr_step11a();
extern	u_long		mscp_bbr_step11b();
extern	u_long		mscp_bbr_step11c();
extern	u_long		mscp_bbr_step12();
extern	u_long		mscp_bbr_step12a();
extern	u_long		mscp_bbr_step12b();
extern	u_long		mscp_bbr_step12c();
extern	u_long		mscp_bbr_step12d();
extern	u_long		mscp_bbr_step12e();
extern	u_long		mscp_bbr_step13();
extern	u_long		mscp_bbr_step14();
extern	u_long		mscp_bbr_step15();
extern	u_long		mscp_bbr_step15a();
extern	u_long		mscp_bbr_step16();
extern	u_long		mscp_bbr_step17();
extern	u_long		mscp_bbr_step18();
extern	u_long		mscp_bbr_step18a();
extern	u_long		mscp_rct_searcha();
extern	u_long		mscp_rct_searchb();
extern	u_long		mscp_rct_searchc();
extern	u_long		mscp_multi_read_cont();
extern	u_long		mscp_multi_write_cont();
extern	u_long		mscp_bbr_rwcont();
extern	u_long		mscp_bbr_rwfin();


/* Bad block replacement states
 */

/*
#define ST_BB_ONLINIT			0
#define ST_BB_REPINIT			1
*/
STATE mscp_bbr_states[] = {

    /* BBR Online processing initial state.
     */
    { ST_BB_ONLINIT,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_ONLINIT,	mscp_alloc_rspid },		/* EV_INITIAL	      */
    { ST_BB_ONLINIT,	mscp_alloc_msg },		/* EV_RSPID	      */
    { ST_BB_ONLINIT,	mscp_bbr_step0 },		/* EV_MSGBUF	      */
    { ST_BB_ONLINIT,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_ONLINIT,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_ONLINIT,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_ONLINIT,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_ONLINIT,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_ONLINIT,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_ONLINIT,	mscp_invevent },		/*		      */
    { ST_BB_ONLINIT,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_ONLINIT,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_ONLINIT,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP0A,	mscp_bbr_step0a },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP12E,	mscp_bbr_step12e },		/* EV_BBRERROR	      */


/*	BBR replacement started
 */
    { ST_BB_REPINIT,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_REPINIT,	mscp_alloc_rspid },		/* EV_INITIAL	      */
    { ST_BB_REPINIT,	mscp_alloc_msg },		/* EV_RSPID	      */
    { ST_BB_STEP4,	mscp_bbr_step4 },		/* EV_MSGBUF	      */
    { ST_BB_REPINIT,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_REPINIT,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_REPINIT,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_REPINIT,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_REPINIT,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_REPINIT,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_REPINIT,	mscp_invevent },		/*		      */
    { ST_BB_REPINIT,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_REPINIT,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_REPINIT,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_REPINIT,	mscp_invevent },		/* EV_BBRSUCCESS      */
    { ST_BB_REPINIT,	mscp_invevent },		/* EV_BBRERROR	      */

/*	Step 0 continuation
 */
    { ST_BB_STEP0A,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP0A,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP0A,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP0A,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP0A,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP0A,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP0A,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP0A,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP0A,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP0A,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP0A,	mscp_invevent },		/*		      */
    { ST_BB_STEP0A,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP0A,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP0A,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP0B,	mscp_bbr_step0b },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP0B,	mscp_bbr_step0b },		/* EV_BBRERROR	      */

/*	Step 0 continuation
 */
    { ST_BB_STEP0B,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP0B,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP0B,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP0B,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP0B,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP0B,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP0B,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP0B,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP0B,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP0B,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP0B,	mscp_invevent },		/*		      */
    { ST_BB_STEP0B,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP0B,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP0B,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP0C,	mscp_bbr_step0c },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP12E,	mscp_bbr_step12e },		/* EV_BBRERROR	      */

/*	Step 0 continuation
 */
    { ST_BB_STEP0C,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP0C,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP0C,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP0C,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP0C,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP0C,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP0C,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP0C,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP0C,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP0C,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP0C,	mscp_invevent },		/*		      */
    { ST_BB_STEP0C,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP0C,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP0C,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP1,	mscp_bbr_step1 },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP1,	mscp_bbr_step1 },		/* EV_BBRERROR	      */

/*	Step 1 - Check if BBR in progress
 */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP1,	mscp_invevent },		/*		      */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP1,	mscp_invevent },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP18,	mscp_bbr_step18 },		/* EV_BBRERROR	      */

/*	Step 4 - Attempt to read original lbn
 */
    { ST_BB_STEP4,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP4,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP4,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP4,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP4,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP4,	mscp_bbr_step4a },		/* EV_ENDMSG	      */
    { ST_BB_STEP4,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP4,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP4,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP4,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP4,	mscp_invevent },		/*		      */
    { ST_BB_STEP4,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP4,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP4,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP5,	mscp_bbr_step5 },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP4,	mscp_invevent },		/* EV_BBRERROR	      */


/*	Step 5 - Save Bad Block data in RCT sector 1
 */
    { ST_BB_STEP5,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP5,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP5,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP5,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP5,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP5,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP5,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP5,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP5,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP5,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP5,	mscp_invevent },		/*		      */
    { ST_BB_STEP5,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP5,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP5,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP6,	mscp_bbr_step6 },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP18,	mscp_bbr_step18 },		/* EV_BBRERROR	      */


/*	Step 6 - Read RCT block 0 for update
 */
    { ST_BB_STEP6,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP6,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP6,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP6,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP6,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP6,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP6,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP6,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP6,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP6,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP6,	mscp_invevent },		/*		      */
    { ST_BB_STEP6,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP6,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP6,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP6A,	mscp_bbr_step6a },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP18,	mscp_bbr_step18 },		/* EV_BBRERROR	      */


/*	Step 6a - Update RCT block 0 and write it out
 */
    { ST_BB_STEP6A,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP6A,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP6A,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP6A,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP6A,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP6A,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP6A,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP6A,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP6A,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP6A,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP6A,	mscp_invevent },		/*		      */
    { ST_BB_STEP6A,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP6A,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP6A,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP7,	mscp_bbr_step7 },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP17,	mscp_bbr_step17 },		/* EV_BBRERROR	      */


/*	Step 7 - Start stress test of suspected bad block
 */
    { ST_BB_STEP7,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP7,	mscp_bbr_step7 },		/* EV_INITIAL	      */
    { ST_BB_STEP7,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP7,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP7,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP7,	mscp_bbr_step7a },		/* EV_ENDMSG	      */
    { ST_BB_STEP7,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP7,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP7,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP7,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP7,	mscp_invevent },		/*		      */
    { ST_BB_STEP7,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP7,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP7,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP7B,	mscp_bbr_step7b },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP8,	mscp_bbr_step8 },		/* EV_BBRERROR	      */


/*	Step 7b - Write saved data and reread up to 4 times
 */
    { ST_BB_STEP7B,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP7B,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP7B,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP7B,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP7B,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP7B,	mscp_bbr_step7b },		/* EV_ENDMSG	      */
    { ST_BB_STEP7B,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP7B,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP7B,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP7B,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP7B,	mscp_invevent },		/*		      */
    { ST_BB_STEP7B,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP7B,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP7B,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP7C,	mscp_bbr_step7c },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP8,	mscp_bbr_step8 },		/* EV_BBRERROR	      */


/*	Step 7c - write inverse data and reread it
 */
    { ST_BB_STEP7C,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP7C,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP7C,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP7C,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP7C,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP7C,	mscp_bbr_step7c },		/* EV_ENDMSG	      */
    { ST_BB_STEP7C,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP7C,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP7C,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP7C,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP7C,	mscp_invevent },		/*		      */
    { ST_BB_STEP7C,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP7C,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP7C,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP7B,	mscp_bbr_step7b },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP8,	mscp_bbr_step8 },		/* EV_BBRERROR	      */


/*	Step 8 - Write saved data back to original block
 */
    { ST_BB_STEP8,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP8,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP8,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP8,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP8,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP8,	mscp_bbr_step8 },		/* EV_ENDMSG	      */
    { ST_BB_STEP8,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP8,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP8,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP8,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP8,	mscp_invevent },		/*		      */
    { ST_BB_STEP8,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP8,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP8,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP13,	mscp_bbr_step13 },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP9,	mscp_bbr_step9 },		/* EV_BBRERROR	      */


/*	Step 9 - Start search of RCT for replacement block
 */
    { ST_BB_STEP9,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP9,	mscp_bbr_step9 },		/* EV_INITIAL	      */
    { ST_BB_STEP9,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP9,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP9,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP9,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP9,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP9,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP9,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP9,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP9,	mscp_invevent },		/*		      */
    { ST_BB_STEP9,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP16,	mscp_bbr_step16 },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP16,	mscp_bbr_step16 },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP10,	mscp_bbr_step10 },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP16,	mscp_bbr_step16 },		/* EV_BBRERROR	      */


/*	Step 10 - Update RCT sector 0 to indicate phase 2
 */
    { ST_BB_STEP10,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP10,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP10,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP10,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP10,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP10,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP10,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP10,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP10,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP10,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP10,	mscp_invevent },		/*		      */
    { ST_BB_STEP10,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP10,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP10,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP11,	mscp_bbr_step11 },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP16,	mscp_bbr_step16 },		/* EV_BBRERROR	      */


/*	Step 11 - Update descriptors to record replacement
 */
    { ST_BB_STEP11,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP11,	mscp_bbr_step11 },		/* EV_INITIAL	      */
    { ST_BB_STEP11,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP11,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP11,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP11,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP11,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP11,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP11,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP11,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP11,	mscp_invevent },		/*		      */
    { ST_BB_STEP11,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP11,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP11,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP11A,	mscp_bbr_step11a },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP16,	mscp_bbr_step16 },		/* EV_BBRERROR	      */


/*	Step 11a - Process second RCT descriptor block
 */
    { ST_BB_STEP11A,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP11A,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP11A,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP11A,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP11A,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP11A,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP11A,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP11A,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP11A,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP11A,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP11A,	mscp_invevent },		/*		      */
    { ST_BB_STEP11C,	mscp_bbr_step11c },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP11A,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP11A,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP11B,	mscp_bbr_step11b },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP16,	mscp_bbr_step16 },		/* EV_BBRERROR	      */


/*	Step 11b - Write out RCT descriptor block
 */
    { ST_BB_STEP11B,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP11B,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP11B,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP11B,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP11B,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP11B,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP11B,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP11B,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP11B,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP11B,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP11B,	mscp_invevent },		/*		      */
    { ST_BB_STEP11B,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP11B,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP11B,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP11C,	mscp_bbr_step11c },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP15,	mscp_bbr_step15 },		/* EV_BBRERROR	      */


/*	Step 11c - Write out RCT descriptor block
 */
    { ST_BB_STEP11C,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP11C,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP11C,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP11C,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP11C,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP11C,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP11C,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP11C,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP11C,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP11C,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP11C,	mscp_invevent },		/*		      */
    { ST_BB_STEP11C,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP11C,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP11C,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP12,	mscp_bbr_step12 },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP15,	mscp_bbr_step15 },		/* EV_BBRERROR	      */


/*	Step 12
 */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP12A,	mscp_bbr_step12a },		/* EV_ENDMSG	      */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP12,	mscp_invevent },		/*		      */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP12,	mscp_invevent },		/* EV_BBRERROR	      */


/*	Step 12a
 */
    { ST_BB_STEP12A,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP12A,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP12A,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP12A,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP12A,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP12B,	mscp_bbr_step12b },		/* EV_ENDMSG	      */
    { ST_BB_STEP12A,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP12A,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP12A,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP12A,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP12A,	mscp_invevent },		/*		      */
    { ST_BB_STEP12A,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP12A,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP12A,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP12C,	mscp_bbr_step12c },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP12D,	mscp_bbr_step12d },		/* EV_BBRERROR	      */


/*	Step 12b
 */
    { ST_BB_STEP12B,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP12B,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP12B,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP12B,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP12B,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP12B,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP12B,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP12B,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP12B,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP12B,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP12B,	mscp_invevent },		/*		      */
    { ST_BB_STEP9,	mscp_bbr_step9 },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP12B,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP12B,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP12C,	mscp_bbr_step12c },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP12D,	mscp_bbr_step12d },		/* EV_BBRERROR	      */


/*	Step 12c
 */
    { ST_BB_STEP12C,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP12C,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP12C,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP12C,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP12C,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP12C,	mscp_bbr_step12c },		/* EV_ENDMSG	      */
    { ST_BB_STEP12C,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP12C,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP12C,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP12C,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP12C,	mscp_invevent },		/*		      */
    { ST_BB_STEP12C,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP12C,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP12C,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP13,	mscp_bbr_step13 },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP9,	mscp_bbr_step9 },		/* EV_BBRERROR	      */


/*	Step 12d
 */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP12E,	mscp_bbr_step12e },		/* EV_ENDMSG	      */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP12D,	mscp_invevent },		/*		      */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP12D,	mscp_invevent },		/* EV_BBRERROR	      */


/*	Step 12e
 */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP18,	mscp_bbr_step18 },		/* EV_ENDMSG	      */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP12E,	mscp_invevent },		/*		      */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP12E,	mscp_invevent },		/* EV_BBRERROR	      */


/*	Step 13
 */
    { ST_BB_STEP13,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP13,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP13,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP13,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP13,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP13,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP13,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP13,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP13,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP13,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP13,	mscp_invevent },		/*		      */
    { ST_BB_STEP13,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP13,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP13,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP13,	mscp_bbr_step14 },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP17,	mscp_bbr_step17 },		/* EV_BBRERROR	      */


/*	Step 15
 */
    { ST_BB_STEP15,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP15,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP15,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP15,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP15,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP15,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP15,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP15,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP15,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP15,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP15,	mscp_invevent },		/*		      */
    { ST_BB_STEP15A,	mscp_bbr_step15a },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP15,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP15,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP15A,	mscp_bbr_step15a },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP15A,	mscp_bbr_step15a },		/* EV_BBRERROR	      */

/*	Step 15a - Write out descriptor block
 */
    { ST_BB_STEP15A,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP15A,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP15A,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP15A,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP15A,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP15A,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP15A,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP15A,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP15A,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP15A,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP15A,	mscp_invevent },		/*		      */
    { ST_BB_STEP15A,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP15A,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP15A,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP16,	mscp_bbr_step16 },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP16,	mscp_bbr_step16 },		/* EV_BBRERROR	      */

/*	Step 16
 */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP17,	mscp_bbr_step17 },		/* EV_ENDMSG	      */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP16,	mscp_invevent },		/*		      */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP16,	mscp_invevent },		/* EV_BBRERROR	      */

/*	Step 17
 */
    { ST_BB_STEP17,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP17,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP17,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP17,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP17,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP17,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP17,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP17,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP17,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP17,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP17,	mscp_invevent },		/*		      */
    { ST_BB_STEP17,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP17,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP17,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP18,	mscp_bbr_step18 },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP18,	mscp_bbr_step18 },		/* EV_BBRERROR	      */

/*	Step 18
 */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_STEP18,	mscp_invevent },		/*		      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_BBRSUCCESS      */
    { ST_BB_STEP18,	mscp_invevent },		/* EV_BBRERROR	      */

/*	RCT search state
 */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/*		      */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_RCTSEARCH,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_RCTSEARCHA,	mscp_rct_searcha },		/* EV_BBRSUCCESS      */
    { ST_BB_RCTSEARCH,	mscp_rct_searchc },		/* EV_BBRERROR	      */


/*	RCT search state a
 */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/*		      */
    { ST_BB_RCTSEARCHB,	mscp_rct_searchb },		/* EV_BBRSUBSTEP      */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_RCTSEARCHA,	mscp_invevent },		/* EV_BBRSUCCESS      */
    { ST_BB_RCTSEARCHA,	mscp_rct_searchc },		/* EV_BBRERROR	      */


/*	RCT search state b
 */
    { ST_BB_RCTSEARCHB,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_RCTSEARCHB,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_RCTSEARCHB,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_RCTSEARCHB,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_RCTSEARCHB,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_RCTSEARCHB,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_BB_RCTSEARCHB,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_RCTSEARCHB,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_RCTSEARCHB,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_RCTSEARCHB,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_RCTSEARCHB,	mscp_invevent },		/*		      */
    { ST_BB_RCTSEARCHB,	mscp_rct_searchb },		/* EV_BBRSUBSTEP      */
    { ST_BB_STEP16,	mscp_bbr_step16 },		/* EV_BBRRCTFULL      */
    { ST_BB_STEP16,	mscp_bbr_step16 },		/* EV_BBRINVRCT	      */
    { ST_BB_RCTSEARCHB,	mscp_rct_searchb },		/* EV_BBRSUCCESS      */
    { ST_BB_RCTSEARCHB,	mscp_rct_searchc },		/* EV_BBRERROR	      */


/* 	Multi-read algorithm
 */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_MULTIREAD,	mscp_multi_read_cont },		/* EV_ENDMSG	      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/*		      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_BBRSUCCESS      */
    { ST_BB_MULTIREAD,	mscp_invevent },		/* EV_BBRERROR	      */


/*	Multi-write algorithm
 */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_MULTIWRITE,	mscp_multi_write_cont },	/* EV_ENDMSG	      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/*		      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_BBRSUCCESS      */
    { ST_BB_MULTIWRITE,	mscp_invevent },		/* EV_BBRERROR	      */


/*	Multi-write algorithm - forced error path
 */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_NULL	      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_MAPPING	      */
    { ST_BB_MULTIWRITE2, mscp_multi_write_cont },	/* EV_ENDMSG	      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/*		      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_BBRSUCCESS      */
    { ST_BB_MULTIWRITE2, mscp_invevent },		/* EV_BBRERROR	      */


/* 	Read in BBR mode
 */
    { ST_BB_READ,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_READ,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_READ,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_READ,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_READ,	mscp_bbr_rwcont },		/* EV_MAPPING	      */
    { ST_BB_READ,	mscp_bbr_rwfin },		/* EV_ENDMSG	      */
    { ST_BB_READ,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_READ,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_READ,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_READ,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_READ,	mscp_invevent },		/*		      */
    { ST_BB_READ,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_READ,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_READ,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_READ,	mscp_invevent },		/* EV_BBRSUCCESS      */
    { ST_BB_READ,	mscp_invevent },		/* EV_BBRERROR	      */


/*	Write in BBR mode
 */
    { ST_BB_WRITE,	mscp_invevent },		/* EV_NULL	      */
    { ST_BB_WRITE,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_BB_WRITE,	mscp_invevent },		/* EV_RSPID	      */
    { ST_BB_WRITE,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_BB_WRITE,	mscp_bbr_rwcont },		/* EV_MAPPING	      */
    { ST_BB_WRITE,	mscp_bbr_rwfin },		/* EV_ENDMSG	      */
    { ST_BB_WRITE,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_BB_WRITE,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_BB_WRITE,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_BB_WRITE,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_BB_WRITE,	mscp_invevent },		/*		      */
    { ST_BB_WRITE,	mscp_invevent },		/* EV_BBRSUBSTEP      */
    { ST_BB_WRITE,	mscp_invevent },		/* EV_BBRRCTFULL      */
    { ST_BB_WRITE,	mscp_invevent },		/* EV_BBRINVRCT	      */
    { ST_BB_WRITE,	mscp_invevent },		/* EV_BBRSUCCESS      */
    { ST_BB_WRITE,	mscp_invevent },		/* EV_BBRERROR	      */

};
