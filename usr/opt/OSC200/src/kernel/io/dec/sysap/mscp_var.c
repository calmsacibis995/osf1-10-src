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
static char *rcsid = "@(#)$RCSfile: mscp_var.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/13 16:29:47 $";
#endif
/*
 *	mscp_var.c	4.1	(ULTRIX)	7/2/90
 */

/*
 *
 *   Facility:	Systems Communication Architecture
 *		Disk Class Driver
 *
 *   Abstract:	
 *
 *   Author:	David E. Eiche	Creation Date:	September 30, 1985
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
 *   20-May-1988	David E. Eiche		DEE0037
 *	Fix typo in DEE0036 which caused a state transition to CLOSED
 *	from RESTART upon receipt of an EV_INITIAL event.  Correct history
 *	comment for DEE0036.
 *
 *   02-Jun-1988	Ricky S. Palmer
 *      removed inclusion of header file ../vaxmsi/msisysap.h
 *
 *   16-May-1988	David E. Eiche		DEE0036
 *	Make multiple changes to the connection management state table
 *	to:  handle an connection management end message arriving after
 *	connection restart; substitute a mscp_noaction for mscp_conreturn
 *	so that the latter may be removed;  ignore EV_EXRETRY in restart
 *	and other states in which it is not required to force connection
 *	recovery;  change event which causes transition from RESTART to
 *	CLOSED state:  was EV_INITIAL, is now EV_ERRECOV.
 *
 *   02-Apr-1988	David E. Eiche		DEE0020
 *	Fix connection management state table to correctly process
 *	a path failure event seen in connection restarting state.
 *
 *   07-Mar-1988	David E. Eiche		DEE0015
 *	Changed state tables to accomodate connection recovery.
 *
 *   22-Feb-1988	Robin
 *	Added entries for rd33 and ese20 disk partition sizes.
 *
 *   02-Feb-1988	Robin
 *	Changed the string constants for device types to use the defines
 *	found in devio.h.
 *
 *   26-Jan-1988	Robin Lewis
 *	Changed entries RV80 to be RV20 (the real name)
 *
 *   15-Jan-1988	Todd M. Katz		TMK0001
 *	Include new header file ../vaxmsi/msisysap.h.
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
extern	u_long		mscp_alloc_msg(),
			mscp_alloc_rspid(),
			mscp_map_buffer(),
			mscp_availcm(),
			mscp_concleanup(),
			mscp_concomplete(),
			mscp_condisccmplt(),
			mscp_condealmsg(),
			mscp_conendmsg(),
			mscp_coninit(),
			mscp_conmarkopen(),
			mscp_conrestore(),
			mscp_conresynch(),
			mscp_conreturn(),
			mscp_consetretry(),
			mscp_constconcm(),
			mscp_constconem(),
			mscp_conwatchdog(),
			mscp_invevent(),
			mscp_markoffline(),
			mscp_markonline(),
			mscp_noaction(),
			mscp_onlgtuntem(),
			mscp_onlinecm(),
			mscp_onlineem(),
			mscp_onlineinit(),
			mscp_polinit(),
			mscp_polgtuntem(),
			mscp_recovinit(),
			mscp_recovnext(),
			mscp_transfercm(),
			mscp_transferem();

/**/

/* Connection management states.
 */
STATE mscp_con_states[] = {

    /* Connection uninitialized state.
     */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_NULL	      */
    { ST_CN_CLOSED,	mscp_coninit },			/* EV_INITIAL	      */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_RSPID	      */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_TIMEOUT	      */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_CONACTIVE	      */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_POLLCOMPLETE    */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_EXRETRY	      */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_DISCOMPLETE     */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_CONCOMPLETE     */
    { ST_CN_INITIAL,	mscp_invevent },		/* EV_PATHFAILURE     */

    /* Connection closed state.
     */
    { ST_CN_CLOSED,	mscp_invevent },		/* EV_NULL	      */
    { ST_CN_CLOSED,	mscp_noaction },		/* EV_INITIAL	      */
    { ST_CN_CLOSED,	mscp_invevent },		/* EV_RSPID	      */
    { ST_CN_CLOSED,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_CN_CLOSED,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_CN_CLOSED,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_CN_CLOSED,	mscp_concomplete },		/* EV_TIMEOUT	      */
    { ST_CN_CLOSED,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_CN_CLOSED,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_CN_CLOSED,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_CN_RESOURCE,	mscp_alloc_rspid },		/* EV_CONACTIVE	      */
    { ST_CN_CLOSED,	mscp_invevent },		/* EV_POLLCOMPLETE    */
    { ST_CN_DEAD,	mscp_consetretry },		/* EV_EXRETRY	      */
    { ST_CN_CLOSED,	mscp_invevent },		/* EV_DISCOMPLETE     */
    { ST_CN_CLOSED,	mscp_concomplete },		/* EV_CONCOMPLETE     */
    { ST_CN_CLOSED,	mscp_invevent },		/* EV_PATHFAILURE     */

    /* Resource wait state.
     */
    { ST_CN_RESOURCE,	mscp_invevent },		/* EV_NULL	      */
    { ST_CN_RESOURCE,	mscp_noaction },		/* EV_INITIAL	      */
    { ST_CN_RESOURCE,	mscp_alloc_msg },		/* EV_RSPID	      */
    { ST_CN_RESOURCE,	mscp_constconcm },		/* EV_MSGBUF	      */
    { ST_CN_RESOURCE,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_CN_STCON1,	mscp_constconem },		/* EV_ENDMSG	      */
    { ST_CN_RESTART,	mscp_conresynch },		/* EV_TIMEOUT	      */
    { ST_CN_RESOURCE,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_CN_RESOURCE,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_CN_RESOURCE,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_CN_RESOURCE,	mscp_invevent },		/* EV_CONACTIVE	      */
    { ST_CN_RESOURCE,	mscp_invevent },		/* EV_POLLCOMPLETE    */
    { ST_CN_RESTART,	mscp_conresynch },		/* EV_EXRETRY	      */
    { ST_CN_RESOURCE,	mscp_invevent },		/* EV_DISCOMPLETE     */
    { ST_CN_RESOURCE,	mscp_invevent },		/* EV_CONCOMPLETE     */
    { ST_CN_RESTART,	mscp_concleanup },		/* EV_PATHFAILURE     */

    /* First set controller characteristics wait state.
     */
    { ST_CN_STCON1,	mscp_invevent },		/* EV_NULL	      */
    { ST_CN_STCON1,	mscp_noaction },		/* EV_INITIAL	      */
    { ST_CN_STCON1,	mscp_invevent },		/* EV_RSPID	      */
    { ST_CN_STCON1,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_CN_STCON1,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_CN_STCON2,	mscp_constconem },		/* EV_ENDMSG	      */
    { ST_CN_RESTART,	mscp_conresynch },		/* EV_TIMEOUT	      */
    { ST_CN_STCON1,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_CN_STCON1,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_CN_STCON1,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_CN_OPEN,	mscp_conmarkopen },		/* EV_CONACTIVE	      */
    { ST_CN_STCON1,	mscp_invevent },		/* EV_POLLCOMPLETE    */
    { ST_CN_RESTART,	mscp_conresynch },		/* EV_EXRETRY	      */
    { ST_CN_STCON1,	mscp_invevent },		/* EV_DISCOMPLETE     */
    { ST_CN_STCON1,	mscp_invevent },		/* EV_CONCOMPLETE     */
    { ST_CN_RESTART,	mscp_concleanup },		/* EV_PATHFAILURE     */

    /* Second set controller characteristics wait state.
     */
    { ST_CN_STCON2,	mscp_invevent },		/* EV_NULL	      */
    { ST_CN_STCON2,	mscp_noaction },		/* EV_INITIAL	      */
    { ST_CN_STCON2,	mscp_invevent },		/* EV_RSPID	      */
    { ST_CN_STCON2,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_CN_STCON2,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_CN_STCON2,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_CN_RESTART,	mscp_conresynch },		/* EV_TIMEOUT	      */
    { ST_CN_STCON2,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_CN_STCON2,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_CN_STCON2,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_CN_OPEN,	mscp_conmarkopen },		/* EV_CONACTIVE	      */
    { ST_CN_STCON2,	mscp_invevent },		/* EV_POLLCOMPLETE    */
    { ST_CN_RESTART,	mscp_conresynch },		/* EV_EXRETRY	      */
    { ST_CN_STCON2,	mscp_invevent },		/* EV_DISCOMPLETE     */
    { ST_CN_STCON2,	mscp_invevent },		/* EV_CONCOMPLETE     */
    { ST_CN_RESTART,	mscp_concleanup },		/* EV_PATHFAILURE     */

    /* Connection open state.
     */
    { ST_CN_OPEN,	mscp_invevent },		/* EV_NULL	      */
    { ST_CN_OPEN,	mscp_noaction },		/* EV_INITIAL	      */
    { ST_CN_OPEN,	mscp_invevent },		/* EV_RSPID	      */
    { ST_CN_OPEN,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_CN_OPEN,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_CN_OPEN,	mscp_conendmsg },		/* EV_ENDMSG	      */
    { ST_CN_OPEN,	mscp_conwatchdog },		/* EV_TIMEOUT	      */
    { ST_CN_RESTART,	mscp_conresynch },		/* EV_NOCREDITS	      */
    { ST_CN_OPEN,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_CN_OPEN,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_CN_OPEN,	mscp_invevent },		/* EV_CONACTIVE	      */
    { ST_CN_OPEN,	mscp_conrestore },		/* EV_POLLCOMPLETE    */
    { ST_CN_RESTART,	mscp_conresynch },		/* EV_EXRETRY	      */
    { ST_CN_OPEN,	mscp_invevent },		/* EV_DISCOMPLETE     */
    { ST_CN_OPEN,	mscp_invevent },		/* EV_CONCOMPLETE     */
    { ST_CN_RESTART,	mscp_concleanup },		/* EV_PATHFAILURE     */

    /* Connection restarting state.
     */
    { ST_CN_RESTART,	mscp_invevent },		/* EV_NULL	      */
    { ST_CN_RESTART,	mscp_noaction },		/* EV_INITIAL	      */
    { ST_CN_RESTART,	mscp_invevent },		/* EV_RSPID	      */
    { ST_CN_RESTART,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_CN_RESTART,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_CN_RESTART,	mscp_condealmsg },		/* EV_ENDMSG	      */
    { ST_CN_RESTART,	mscp_conresynch },		/* EV_TIMEOUT	      */
    { ST_CN_RESTART,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_CN_CLOSED,	mscp_coninit },			/* EV_ERRECOV	      */
    { ST_CN_RESTART,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_CN_RESTART,	mscp_invevent },		/* EV_CONACTIVE	      */
    { ST_CN_RESTART,	mscp_invevent },		/* EV_POLLCOMPLETE    */
    { ST_CN_RESTART,	mscp_noaction },		/* EV_EXRETRY	      */
    { ST_CN_RESTART,	mscp_condisccmplt },		/* EV_DISCOMPLETE     */
    { ST_CN_RESTART,	mscp_invevent },		/* EV_CONCOMPLETE     */
    { ST_CN_RESTART,	mscp_concleanup },		/* EV_PATHFAILURE     */

    /* Connection dead state.
     */
    { ST_CN_DEAD,	mscp_invevent },		/* EV_NULL	      */
    { ST_CN_DEAD,	mscp_noaction },		/* EV_INITIAL	      */
    { ST_CN_DEAD,	mscp_invevent },		/* EV_RSPID	      */
    { ST_CN_DEAD,	mscp_invevent },		/* EV_MSGBUF	      */
    { ST_CN_DEAD,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_CN_DEAD,	mscp_invevent },		/* EV_ENDMSG	      */
    { ST_CN_CLOSED,	mscp_coninit },			/* EV_TIMEOUT	      */
    { ST_CN_DEAD,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_CN_DEAD,	mscp_invevent },		/* EV_ERRECOV	      */
    { ST_CN_DEAD,	mscp_invevent },		/* EV_SPARE2	      */
    { ST_CN_DEAD,	mscp_invevent },		/* EV_CONACTIVE	      */
    { ST_CN_DEAD,	mscp_invevent },		/* EV_POLLCOMPLETE    */
    { ST_CN_DEAD,	mscp_noaction },		/* EV_EXRETRY	      */
    { ST_CN_DEAD,	mscp_invevent },		/* EV_DISCOMPLETE     */
    { ST_CN_DEAD,	mscp_noaction },		/* EV_CONCOMPLETE     */
    { ST_CN_DEAD,	mscp_invevent },		/* EV_PATHFAILURE     */
};

/**/

/* Unit polling states.
 */
STATE mscp_pol_states[] = {

    /* Unit polling initial state.
     */
    { ST_UP_INITIAL,	mscp_invevent },		/* EV_NULL	      */
    { ST_UP_INITIAL,	mscp_alloc_rspid},		/* EV_INITIAL	      */
    { ST_UP_INITIAL,	mscp_alloc_msg },		/* EV_RSPID	      */
    { ST_UP_INITIAL,	mscp_polinit },			/* EV_MSGBUF	      */
    { ST_UP_INITIAL,	mscp_invevent },		/* EV_MAPPING	      */
    { ST_UP_INITIAL,	mscp_polgtuntem },		/* EV_ENDMSG	      */
    { ST_UP_INITIAL,	mscp_polinit },			/* EV_TIMEOUT	      */
    { ST_UP_INITIAL,	mscp_invevent },		/* EV_NOCREDITS	      */
    { ST_UP_INITIAL,	mscp_invevent },		/*		      */
    { ST_UP_INITIAL,	mscp_invevent },		/* 		      */
    { ST_UP_INITIAL,	mscp_invevent },		/* 		      */
    { ST_UP_INITIAL,	mscp_invevent },		/* EV_POLLCOMPLETE    */
    { ST_UP_INITIAL,	mscp_invevent },		/* 		      */
    { ST_UP_INITIAL,	mscp_invevent },		/* 		      */
    { ST_UP_INITIAL,	mscp_invevent },		/*		      */
    { ST_UP_INITIAL,	mscp_invevent },		/*		      */
};

/**/

/* Class driver "master" control blocks
 */
CLASSB	mscp_classb =  { 0 }, tmscp_classb =  { 0 };

/* Global flags common to disk and tape drivers.
 */
u_long	mscp_gbl_flags;
