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
static char *rcsid = "@(#)$RCSfile: ses_init.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/08/04 21:23:25 $";
#endif
/*****************************************************************************
*
*	int	siad_ses_init(SIAENTITY *entity)
*
* Description: The purpose of this routine is to do initialization per
* session. This routine is called before session establish by the sia
* library routines.
*
* Returns: 
*		SIADSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
*				Configuration Showstopper
******************************************************************************/
#include <sia.h>
#include <siad.h>
#include "sia_mech.h"

siad_ses_init(entity, pkgind)
SIAENTITY *entity;
int pkgind;
{
	AUDIT_MASK_TYPE amask[AUDIT_MASK_LEN];
	int i;

	set_auth_parameters(entity->argc, entity->argv);
	initprivs();
	memset(amask, 0, sizeof amask);
	for (i = MIN_TRUSTED_EVENT;  i <= MAX_TRUSTED_EVENT;  i++) {
		A_PROCMASK_SET(amask, i, 1, 1);
	}
	/* add error checking here? */
	(void) audcntl(SET_PROC_AMASK, (char *)amask, sizeof amask, 0L, 0L, 0L);
	(void) audcntl(SET_PROC_ACNTL, (char *)0, 0L, AUDIT_AND, 0L, 0L);
	if (pkgind >=0 && pkgind < SIASWMAX) {
		entity->mech[pkgind] = NULL;
	}
	return SIADSUCCESS;
}
