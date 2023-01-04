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
static char *rcsid = "@(#)$RCSfile: siad_s_rel.c,v $ $Revision: 1.1.11.5 $ (DEC) $Date: 1993/10/19 22:24:19 $";
#endif
/*****************************************************************************
*
*	int	siad_ses_release()
*
* Description: The purpose of this routine is to do a system initialization
* once at boottime for the security mechanism.
*
* Returns: 
*		SIADSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
*				Configuration Showstopper
******************************************************************************/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_SHARED_LIBRARIES) || (!defined(_SHARED_LIBRARIES) && !defined(_THREAD_SAFE))
#pragma weak siad_ses_release = __siad_ses_release
#endif
#endif
#include "siad.h"
#include "siad_bsd.h"

int
  siad_ses_release (SIAENTITY *entity, int pkgind)
{
	uid_t saveuid = geteuid();
	uid_t savruid = getuid();
	uchar_t amask[AUDIT_MASK_LEN];
	int i;

	if (setreuid(0,0) < 0)
		return SIADSUCCESS; /* do the best we can */
	bzero((void *)amask, sizeof amask);
	(void) audcntl(SET_PROC_AMASK, (char *)amask, sizeof amask, 0L, 0L, 0L);
	(void) audcntl(SET_PROC_ACNTL, (char *)0, 0L, AUDIT_OR, 0L, 0L);
	(void) setreuid(savruid, saveuid);

	return SIADSUCCESS;
}
