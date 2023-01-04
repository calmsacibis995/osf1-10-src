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
static char *rcsid = "@(#)$RCSfile: mscp_diskvar.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/13 16:27:25 $";
#endif
/*
 *	mscp_diskvar.c	4.1	(ULTRIX)	7/2/90
 */

/*
 *
 *   Facility:	Systems Communication Architecture
 *		Disk Class Driver
 *
 *   Abstract:	
 *
 *   Author:	David E. Eiche	Creation Date:	March 11, 1988
 *
 *   History:
 *
 *   19-Feb-1991	Tom Tierney
 *	ULTRIX to OSF/1 port:
 *
 *
 *   23-Oct-1989	Tim Burke
 *	Added state tables to allow for a set unit characteristics command.
 *
 *   07-Mar-1989	Todd M. Katz		TMK0001
 *	1. Include header file ../vaxmsi/msisysap.h.
 *	2. Use the ../machine link to refer to machine specific header files.
 *
 *   08-Jul-1988	Pete Keilty
 *	Added accscan state table.
 *
 *   02-Jun-1988     Ricky S. Palmer
 *	Removed inclusion of header file ../vaxmsi/msisysap.h
 *
 *   03-Apr-1988	David E. Eiche		DEE0022
 *	Remove reference to mscp_onlineinit routine.  This is part of
 *	a larger fix to eliminate an open/close race condition.
 *
 *   11-Mar-1988	David E. Eiche
 *	Moved disk- specific MSCP modules from mscp_var.c into (this)
 *	separate module.  Audit trail information is retained in
 *	mscp_var.c.
 */
/**/

/* Libraries and Include Files.
 */
#include	<labels.h>
#include	<sys/types.h>
#include	<dec/binlog/errlog.h>
#include	<io/dec/scs/sca.h>
#include	<io/dec/sysap/sysap.h>
#include	<io/dec/sysap/mscp_bbrdefs.h>

/* External Variables and Routines.
 */
extern  u_long		mscp_accscancm(),
			mscp_accscanem(),
			mscp_alloc_msg(),
			mscp_alloc_rspid(),
			mscp_map_buffer(),
			mscp_availcm(),
			mscp_forcecm(),
			mscp_forceem(),
			mscp_invevent(),
			mscp_markoffline(),
			mscp_markonline(),
			mscp_noaction(),
			mscp_onlgtuntem(),
			mscp_onlinecm(),
			mscp_onlineem(),
			mscp_recovinit(),
			mscp_recovnext(),
			mscp_setunitcm(),
			mscp_setunitem(),
			mscp_transfercm(),
			mscp_transferem();

/**/

/* Bring unit online states.
 */
STATE mscp_onl_states[] = {

    /* Unit online initial state.
     */
    { ST_ON_INITIAL,	mscp_invevent },		/* EV_NULL	      */
    { ST_ON_INITIAL,	mscp_alloc_rspid },		/* EV_INITIAL	      */
    { ST_ON_INITIAL,	mscp_alloc_msg },		/* EV_RSPID	      */
    { ST_ON_INITIAL,	mscp_onlinecm },		/* EV_MSGBUF	      */
    { ST_ON_INITIAL,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_ON_ONLIN,	mscp_onlineem },		/* EV_ENDMSG	      */
    { ST_ON_INITIAL,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_ON_INITIAL,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_ON_INITIAL,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_ON_INITIAL,	mscp_invevent },		/* 		      */
    { ST_ON_INITIAL,	mscp_invevent },		/* 		      */
    { ST_ON_INITIAL,	mscp_invevent },		/*		      */
    { ST_ON_INITIAL,	mscp_invevent },		/*		      */
    { ST_ON_INITIAL,	mscp_invevent },		/*		      */
    { ST_ON_INITIAL,	mscp_invevent },		/* EV_ONLCOMPLETE     */
    { ST_ON_INITIAL,	mscp_invevent },		/* EV_ONLERROR	      */

    /* Unit online - online end message processing
     */
    { ST_ON_ONLIN,	mscp_invevent },		/* EV_NULL	      */
    { ST_ON_ONLIN,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_ON_ONLIN,	mscp_invevent },		/* EV_RSPID	      */
    { ST_ON_ONLIN,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_ON_ONLIN,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_ON_GTUNT,	mscp_onlgtuntem },		/* EV_ENDMSG	      */
    { ST_ON_ONLIN,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_ON_ONLIN,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_ON_ONLIN,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_ON_ONLIN,	mscp_invevent },		/*		      */
    { ST_ON_ONLIN,	mscp_invevent },		/*		      */
    { ST_ON_ONLIN,	mscp_invevent },		/*		      */
    { ST_ON_ONLIN,	mscp_invevent },		/*		      */
    { ST_ON_AVAIL,	mscp_availcm },			/* EV_ONLERRAVAIL     */
    { ST_ON_ONLIN,	mscp_invevent },		/* EV_ONLCOMPLETE     */
    { ST_ON_AVAIL,	mscp_markoffline },		/* EV_ONLERROR	      */

    /* Unit online available end message processing.
     */
    { ST_ON_AVAIL,	mscp_invevent },		/* EV_NULL	      */
    { ST_ON_AVAIL,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_ON_AVAIL,	mscp_invevent },		/* EV_RSPID	      */
    { ST_ON_AVAIL,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_ON_AVAIL,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_ON_AVAIL,	mscp_markoffline },		/* EV_ENDMSG	      */
    { ST_ON_AVAIL,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_ON_AVAIL,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_ON_AVAIL,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_ON_AVAIL,	mscp_invevent },		/*		      */
    { ST_ON_AVAIL,	mscp_invevent },		/*		      */
    { ST_ON_AVAIL,	mscp_invevent },		/*		      */
    { ST_ON_AVAIL,	mscp_invevent },		/*		      */
    { ST_ON_AVAIL,	mscp_invevent },		/* EV_ONLERRAVAIL     */
    { ST_ON_AVAIL,	mscp_invevent },		/* EV_ONLCOMPLETE     */
    { ST_ON_AVAIL,	mscp_noaction },		/* EV_ONLERROR	      */

    /* Unit online get unit status end message processing.
     */
    { ST_ON_GTUNT,	mscp_invevent },		/* EV_NULL	      */
    { ST_ON_GTUNT,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_ON_GTUNT,	mscp_invevent },		/* EV_RSPID	      */
    { ST_ON_GTUNT,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_ON_GTUNT,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_ON_GTUNT,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_ON_GTUNT,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_ON_GTUNT,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_ON_GTUNT,	mscp_markonline },		/* EV_ERRECOV	      */
    { ST_ON_GTUNT,	mscp_invevent },		/*		      */
    { ST_ON_GTUNT,	mscp_invevent },		/*		      */
    { ST_ON_GTUNT,	mscp_invevent },		/*		      */
    { ST_ON_GTUNT,	mscp_invevent },		/*		      */
    { ST_ON_GTUNT,	mscp_invevent },		/* EV_ONLERRAVAIL     */
    { ST_ON_GTUNT,	mscp_markonline },		/* EV_ONLCOMPLETE     */
    { ST_ON_INITIAL,	mscp_onlinecm },		/* EV_ONLERROR	      */
};

/**/

/* Make unit available states.
 */
STATE mscp_avl_states[] = {

    /* Unit available initial state.
     */
    { ST_AV_INITIAL,	mscp_invevent },		/* EV_NULL	      */
    { ST_AV_INITIAL,	mscp_alloc_rspid },		/* EV_INITIAL	      */
    { ST_AV_INITIAL,	mscp_alloc_msg },		/* EV_RSPID	      */
    { ST_AV_INITIAL,	mscp_availcm },			/* EV_MSGBUF	      */
    { ST_AV_INITIAL,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_AV_INITIAL,	mscp_markoffline },		/* EV_ENDMSG	      */
    { ST_AV_INITIAL,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_AV_INITIAL,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_AV_INITIAL,	mscp_invevent },		/*		      */
    { ST_AV_INITIAL,	mscp_invevent },		/*		      */
    { ST_AV_INITIAL,	mscp_invevent },		/*		      */
    { ST_AV_INITIAL,	mscp_invevent },		/*		      */
    { ST_AV_INITIAL,	mscp_invevent },		/*		      */
    { ST_AV_INITIAL,	mscp_invevent },		/*		      */
    { ST_AV_INITIAL,	mscp_invevent },		/* EV_AVLCOMPLETE     */
    { ST_AV_INITIAL,	mscp_noaction },		/* EV_AVLERROR	      */
};

/**/

/* Forced replacement states.
 */
STATE mscp_repl_states[] = {

    /* Forced replacement initial state.
     */
    { ST_RPL_INITIAL,	mscp_invevent },		/* EV_NULL	      */
    { ST_RPL_INITIAL,	mscp_alloc_msg },		/* EV_INITIAL	      */
    { ST_RPL_INITIAL,	mscp_invevent },		/* EV_RSPID	      */
    { ST_RPL_INITIAL,	mscp_forcecm },			/* EV_MSGBUF	      */
    { ST_RPL_INITIAL,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_RPL_INITIAL,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_RPL_INITIAL,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_RPL_INITIAL,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_RPL_INITIAL,	mscp_forceem },			/* EV_ERRECOV	      */
    { ST_RPL_INITIAL,	mscp_invevent },		/*		      */
    { ST_RPL_INITIAL,	mscp_invevent },		/*		      */
    { ST_RPL_INITIAL,	mscp_invevent },		/*		      */
    { ST_RPL_INITIAL,	mscp_invevent },		/*		      */
    { ST_RPL_INITIAL,	mscp_invevent },		/*		      */
    { ST_RPL_INITIAL,	mscp_invevent },		/* EV_AVLCOMPLETE     */
    { ST_RPL_INITIAL,	mscp_invevent },		/* EV_AVLERROR	      */
};

/**/

/* Data transfer states.
 */
STATE mscp_xfr_states[] = {

    /* Unit online initial state.
     */
    { ST_XF_INITIAL,	mscp_invevent },		/* EV_NULL	      */
    { ST_XF_INITIAL,	mscp_alloc_rspid },		/* EV_INITIAL	      */
    { ST_XF_INITIAL,	mscp_alloc_msg },		/* EV_RSPID	      */
    { ST_XF_INITIAL,	mscp_map_buffer },		/* EV_MSGBUF	      */
    { ST_XF_INITIAL,	mscp_transfercm },		/* EV_MAPPING	      */
    { ST_XF_INITIAL,	mscp_transferem },		/* EV_ENDMSG	      */
    { ST_XF_INITIAL,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_XF_INITIAL,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_XF_INITIAL,	mscp_transfercm },		/* EV_ERRECOV	      */
    { ST_XF_INITIAL,	mscp_invevent },		/*		      */
    { ST_XF_INITIAL,	mscp_invevent },		/*		      */
    { ST_XF_INITIAL,	mscp_invevent },		/*		      */
    { ST_XF_INITIAL,	mscp_invevent },		/*		      */
    { ST_XF_INITIAL,	mscp_invevent },		/*		      */
    { ST_XF_INITIAL,	mscp_invevent },		/*		      */
    { ST_XF_INITIAL,	mscp_invevent },		/*		      */
};

/**/

/* Set unit characteristics states.
 */
STATE mscp_stu_states[] = {

    /* Unit online initial state.
     */
    { ST_STU_INITIAL,	mscp_invevent },		/* EV_NULL	      */
    { ST_STU_INITIAL,	mscp_alloc_rspid },		/* EV_INITIAL	      */
    { ST_STU_INITIAL,	mscp_alloc_msg },		/* EV_RSPID	      */
    { ST_STU_INITIAL,	mscp_setunitcm },		/* EV_MSGBUF	      */
    { ST_STU_INITIAL,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_STU_INITIAL,	mscp_setunitem },		/* EV_ENDMSG	      */
    { ST_STU_INITIAL,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_STU_INITIAL,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_STU_INITIAL,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_STU_INITIAL,	mscp_invevent },		/*		      */
    { ST_STU_INITIAL,	mscp_invevent },		/*		      */
    { ST_STU_INITIAL,	mscp_invevent },		/*		      */
    { ST_STU_INITIAL,	mscp_invevent },		/*		      */
    { ST_STU_INITIAL,	mscp_invevent },		/*		      */
    { ST_STU_INITIAL,	mscp_invevent },		/*		      */
    { ST_STU_INITIAL,	mscp_invevent },		/*		      */
};

/*^L*/

/* Access scan states.
 */
STATE mscp_accscan_states[] = {

    /* Unit online initial state.
     */
    { ST_ACC_INITIAL,	mscp_invevent },		/* EV_NULL	      */
    { ST_ACC_INITIAL,	mscp_alloc_rspid },		/* EV_INITIAL	      */
    { ST_ACC_INITIAL,	mscp_alloc_msg },		/* EV_RSPID	      */
    { ST_ACC_INITIAL,	mscp_accscancm },		/* EV_MSGBUF	      */
    { ST_ACC_INITIAL,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_ACC_INITIAL,	mscp_accscanem },		/* EV_ENDMSG	      */
    { ST_ACC_INITIAL,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_ACC_INITIAL,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_ACC_INITIAL,	mscp_forceem },			/* EV_ERRECOV	      */
    { ST_ACC_INITIAL,	mscp_invevent },		/*		      */
    { ST_ACC_INITIAL,	mscp_invevent },		/*		      */
    { ST_ACC_INITIAL,	mscp_invevent },		/*		      */
    { ST_ACC_INITIAL,	mscp_invevent },		/*		      */
    { ST_ACC_INITIAL,	mscp_invevent },		/*		      */
    { ST_ACC_INITIAL,	mscp_invevent },		/*		      */
    { ST_ACC_INITIAL,	mscp_invevent },		/*		      */
};

/**/

/* Unit recovery states.
 */
STATE mscp_rec_states[] = {

    /* Unit online initial state.
     */
    { ST_RE_INITIAL,	mscp_invevent },		/* EV_NULL	      */
    { ST_RE_INITIAL,	mscp_recovinit },		/* EV_INITIAL	      */
    { ST_RE_INITIAL,	mscp_alloc_msg },		/* EV_RSPID	      */
    { ST_RE_INITIAL,	mscp_onlinecm },		/* EV_MSGBUF	      */
    { ST_RE_INITIAL,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_RE_ONLIN,	mscp_onlineem },		/* EV_ENDMSG	      */
    { ST_RE_INITIAL,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_RE_INITIAL,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_RE_INITIAL,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_RE_INITIAL,	mscp_invevent },		/* 		      */
    { ST_RE_INITIAL,	mscp_invevent },		/* 		      */
    { ST_RE_INITIAL,	mscp_invevent },		/*		      */
    { ST_RE_INITIAL,	mscp_invevent },		/*		      */
    { ST_RE_INITIAL,	mscp_invevent },		/*		      */
    { ST_RE_INITIAL,	mscp_invevent },		/* EV_ONLCOMPLETE     */
    { ST_RE_INITIAL,	mscp_invevent },		/* EV_ONLERROR	      */

    /* Unit recovery - online end message processing
     */
    { ST_RE_ONLIN,	mscp_invevent },		/* EV_NULL	      */
    { ST_RE_ONLIN,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_RE_ONLIN,	mscp_invevent },		/* EV_RSPID	      */
    { ST_RE_ONLIN,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_RE_ONLIN,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_RE_GTUNT,	mscp_onlgtuntem },		/* EV_ENDMSG	      */
    { ST_RE_ONLIN,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_RE_ONLIN,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_RE_ONLIN,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_RE_ONLIN,	mscp_invevent },		/*		      */
    { ST_RE_ONLIN,	mscp_invevent },		/*		      */
    { ST_RE_ONLIN,	mscp_invevent },		/*		      */
    { ST_RE_ONLIN,	mscp_invevent },		/*		      */
    { ST_RE_AVAIL,	mscp_availcm },			/* EV_ONLERRAVAIL     */
    { ST_RE_ONLIN,	mscp_invevent },		/* EV_ONLCOMPLETE     */
    { ST_RE_AVAIL,	mscp_markoffline },		/* EV_ONLERROR	      */

    /* Unit recovery available end message processing.
     */
    { ST_RE_AVAIL,	mscp_invevent },		/* EV_NULL	      */
    { ST_RE_AVAIL,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_RE_AVAIL,	mscp_invevent },		/* EV_RSPID	      */
    { ST_RE_AVAIL,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_RE_AVAIL,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_RE_AVAIL,	mscp_markoffline },		/* EV_ENDMSG	      */
    { ST_RE_AVAIL,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_RE_AVAIL,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_RE_AVAIL,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_RE_AVAIL,	mscp_invevent },		/*		      */
    { ST_RE_AVAIL,	mscp_invevent },		/*		      */
    { ST_RE_AVAIL,	mscp_invevent },		/*		      */
    { ST_RE_AVAIL,	mscp_invevent },		/*		      */
    { ST_RE_AVAIL,	mscp_invevent },		/* EV_ONLERRAVAIL     */
    { ST_RE_AVAIL,	mscp_invevent },		/* EV_ONLCOMPLETE     */
    { ST_RE_GTUNT,	mscp_recovnext },		/* EV_ONLERROR	      */

    /* Unit recovery get unit status end message processing.
     */
    { ST_RE_GTUNT,	mscp_invevent },		/* EV_NULL	      */
    { ST_RE_GTUNT,	mscp_invevent },		/* EV_INITIAL	      */
    { ST_RE_GTUNT,	mscp_invevent },		/* EV_RSPID	      */
    { ST_RE_GTUNT,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_RE_GTUNT,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_RE_GTUNT,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_RE_GTUNT,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_RE_GTUNT,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_RE_GTUNT,	mscp_recovnext },		/* EV_ERRECOV	      */
    { ST_RE_GTUNT,	mscp_invevent },		/*		      */
    { ST_RE_GTUNT,	mscp_invevent },		/*		      */
    { ST_RE_GTUNT,	mscp_invevent },		/*		      */
    { ST_RE_INITIAL,	mscp_onlinecm },		/* EV_ONLDONEXT	      */
    { ST_RE_GTUNT,	mscp_invevent },		/* EV_ONLERRAVAIL     */
    { ST_RE_GTUNT,	mscp_recovnext },		/* EV_ONLCOMPLETE     */
    { ST_RE_INITIAL,	mscp_onlinecm },		/* EV_ONLERROR	      */
};

