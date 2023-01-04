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
static char *rcsid = "@(#)$RCSfile: sysap_start.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/07/13 16:31:08 $";
#endif
/*
 *	sysap_start.c	2.3	(ULTRIX)	10/12/89
 */

/*
 *
 *   Facility:	Systems Communication Architecture
 *
 *   Abstract:	This module contains all of the routine(s) to initialize
 *		and or start System Applications (SYSAPS).
 *
 *   scs_start_sysaps		Attach and/or start System Applications
 *
 *   Creator:	Larry Cohen	Creation Date:  March 13, 1987
 *
 *   History:
 *
 *   19-Feb-1991	Tom Tierney
 *	ULTRIX to OSF/1 port:
 *
 *   Aug 1990	Matthew Sacks
 *	Added calls to lockinit macros to initialize the SMP locks in
 *      the classb data structures.  This is done here so that they
 *	can used by the blah_init_driver routines.
 *
 */
/**/


#include	<labels.h>
#include	<io/dec/sysap/sysap_start.h>
#include	<sys/dk.h>
#include	<sys/types.h>
#include	<dec/binlog/errlog.h>
#include	<io/dec/scs/sca.h>
#include	<io/dec/sysap/sysap.h>
#include	<io/dec/sysap/mscp_bbrdefs.h>

extern CLASSB	mscp_classb, tmscp_classb;

/*
 *   Name:	scs_start_sysaps - Attach and/or start System Applications
 *
 *   Abstract:	This subroutine scans a function array "sysaps" for sysaps to
 *		attach and/or start.  
 *
 *   Inputs:
 *
 *   sysaps			- function array of sysaps to start
 *
 *   Outputs:
 *
 *   SMP:
 *
 *	The Sysaps's class block data structures have smp locks, and
 *	those locks are inited here, before calling the start routines.
 *	Also, the rspid database lock is initialized.
 *
 *   none
 *
 */

void
scs_start_sysaps()
{
	int i;
	extern Sysap_start sysaps[];
	CLASSB	*mscp_clp = &mscp_classb, *tmscp_clp = &tmscp_classb;
	
	Init_classb_lock(mscp_clp);
	Init_classb_lock(tmscp_clp);
	Init_rspid_db_lock();

	for (i=0; sysaps[i] != SYSAP_LAST && i<MAX_SYSAPS; i++)
		if (sysaps[i]) 
			(*sysaps[i])();

}
