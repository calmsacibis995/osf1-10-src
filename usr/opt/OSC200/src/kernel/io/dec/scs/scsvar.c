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
static char *rcsid = "@(#)$RCSfile: scsvar.c,v $ $Revision: 1.1.8.3 $ (DEC) $Date: 1993/07/16 17:12:42 $";
#endif
#ifndef	lint
static char *sccsid = "@(#)scsvar.c	4.2	(ULTRIX)	8/31/90";
#endif	lint
/*
 * derived from scsvar.c	4.2	(ULTRIX)	8/31/90";
 */
/*
 *   Facility:	Systems Communication Architecture
 *		Systems Communication Services
 *
 *   Abstract:	This module contains Systems Communication Services( SCS )
 *		internal data structures.
 *
 *   Creator:	Todd M. Katz	Creation Date:	June 15, 1985
 *
 *   Modification History:
 *
 *   23-Jul-1991	Brian Nadeau
 *	Added NPORT support.
 *
 *   1-Jul-1991		Pete Keilty
 *	Added np_info routine for new N_PORT CI driver. 
 *
 *   29-May-1991	grist
 *	Added entries for 3max+/bigmax and maxine.
 *
 *   15-Mar-1991	Brian Nadeau
 *	Port to OSF
 *
 *   31-Aug-1990	rafiey (Ali Rafieymehr)
 *	Added entries for VAX9000, Mipsmate, and 3MIN in cpu_types[].
 *
 *   21-May-1989	Pete Keilty
 *	Update cpu_types[] with new cpu values.
 *
 *   08-May-1989	Adrian Thoms
 *	Fixed the cpu_types array for VVAX, CVAX and 6400
 *	Added numbering comments
 *
 *   06-Apr-1989	Pete Keilty
 *	Changed scsa_lk_db to new lock_t struct lk_scadb
 *
 *   10-Feb-1989	Todd M. Katz		TMK0004
 *	1. Add support for SCS event console logging through addition of
 *	   vectors scs_cltab[], scs_cli[], and scs_clw[].
 *	2. Include header file ../vaxmsi/msisysap.h.
 *
 *   10-Jan-1989	Kong
 *	Added VAX6500 to the list.
 *
 *   19-Aug-1988	Todd M. Katz		TMK0003
 *	Split scs_map_pc[] in 2 with the new table called scs_map_spc[].
 *
 *   08-Jul-1988	Todd M. Katz		TMK0002
 *	Bring cpu_types[] up-to-date with new CPU values.
 *
 *   02-Jun-1988	Ricky S. Palmer
 *	Removed inclusion of header file ../vaxmsi/msisysap.h
 *
 *   08-Jan-1988	Todd M. Katz		TMK0001
 *	Formated module, revised comments, increased robustness, restructured
 *	code paths, and added SMP support.
 */

/* Libraries and Include Files.
 */
#include		<sys/types.h>
#include		<sys/param.h>
#include		<dec/binlog/errlog.h>
#include		<io/dec/scs/scs.h>
#include		<io/dec/sysap/sysap.h>


/* Global Variables and Data Structures.
 */
	/* NOTE: cpu_types is only a 4 character field 			*/
char		*cpu_types[] = {	/* CPU Types			     */
		    "V780",	/* 1 */
		    "V750",	/* 2 */
		    "V730",	/* 3 */
		    "8600",	/* 4 */
		    "8200",	/* 5 */
		    "8800",	/* 6 */
		    "MV1 ",	/* 7 */
		    "MV2 ",	/* 8 */
		    "VSTR",	/* 9 */
		    "3600",	/* 10 */
		    "6200",	/* 11 */
		    "3400",	/* 12 */
		    "PVAX",	/* 13 */
		    "FFOX",	/* 14 */
		    "3900",	/* 15 */
		    "PMAX",	/* 16 */
		    "8820",	/* 17 */
		    "5400",	/* 18 */
		    "5800",	/* 19 */
		    "5000",	/* 20 */
		    "CMAX",	/* 21 */
		    "6400",	/* 22 */
		    "VVAX",	/* 23 */
		    "5500",	/* 24 */
		    "5100",	/* 25 */
		    "9000",	/* 26 */
		    "3MIN",	/* 27 */
		    "ADU ",	/* 28 */
		    "COBR",	/* 29 */
		    "FLAM",	/* 30 */ 
		    "RUBY",	/* 31 */
		    "BIGM",	/* 32 */
		    "PELI",	/* 33 */
		    "JENS",	/* 34 */
};
sbq		scs_config_db = {	/* System-wide Configuration	     */
		    &scs_config_db,	/*  Database Queue Head		     */
		    &scs_config_db
};
pccbq		scs_lport_db = {	/* System-wide Local Port Database   */
		    &scs_lport_db,	/*  Queue Head			     */
		    &scs_lport_db
};
cbq		scs_listeners = {	/* Listening SYSAP Queue Head	     */
		    &scs_listeners,
		    &scs_listeners
};
pbq		scs_timeoutq = {	/* SCS Protocol Sequence Timeout     */
		    &scs_timeoutq,	/*  Queue Head			     */
		    &scs_timeoutq
};
fbq		dsa_fbq = {		/* DSA Isr Fork Block Queue Head     */
		    &dsa_fbq,		/*  Queue Head			     */
		    &dsa_fbq
};
CBVTDB		*scs_cbvtdb		/* CB Vector Table Database Pointer  */
		    = NULL;
SCSIB		lscs			/* Local System Permanent Information*/
		    = { 0 };
void		( *gvp_info )()		/* GVP Routine to Retrieve GVP	     */
		    = NULL;		/*  Information			     */
void		( *np_info )()		/* N_PORT Routine to Retrieve NP     */
		    = NULL;		/*  Information			     */
struct slock	lk_scadb;		/* SCA Database Lock Structure	     */
u_short		scs_map_pc[] = {	/* SCS Path Crash Mapping Table	     */
	PF_SYSAP,	/* E_SYSAP	  - SYSAP requested failure	     */
	PF_SCSTIMEOUT	/* E_TIMEOUT	  - SCS request timeout occurred     */
},
		scs_map_spc[] = {	/* SCS Severe Path Crash Mapping Tabl*/
	PF_SCSPROTOCOL,	/* SE_BADCONNID	  - Illegal connection identification*/
	PF_SCSPROTOCOL,	/* SE_BADCSTATE   - Illegal connection state	     */
	PF_SCSPROTOCOL,	/* SE_NEGCREDITS  - Unsupported withdrawal of credits*/
	PF_SCSPROTOCOL,	/* SE_BADSCSMTYPE - Unsupported SCS message type     */
	PF_SCSPROTOCOL,	/* SE_NOTRANSFERS - Unexpected block data transfer   */
	PF_SCSPROTOCOL	/* SE_NOCREDITS	  - Message sent without credit	     */
};
static CLFTAB				/* Console Logging Formating Tables  */
	scs_cli[] = {			/* SCS Informational Event Table     */
    { CF_NONE, "new listener established" },
    { CF_NONE, "new connection established" }
},
	scs_clw[] = {			/* SCS Warning Event Table	     */
    { CF_NONE, "listener terminated" },
    { CF_NONE, "connection terminated" },
    { CF_NONE, "connection failed: path failure" },
    { CF_NONE, "connection establishment aborted: requested by remote scs" },
    { CF_NONE, "connection establishment aborted: path failure" },
    { CF_NONE, "connection establishment aborted: requested by remote sysap" },
    { CF_NONE, "connection establishment aborted: requested by local sysap" }
};
CLSTAB					/* SCS Console Logging Table	     */
	scs_cltab[ ES_W + 1 ] = {
    { 2,	scs_cli	   },		/* Severity == ES_I		     */
    { 7,	scs_clw	   },		/* Severity == ES_W		     */
};
