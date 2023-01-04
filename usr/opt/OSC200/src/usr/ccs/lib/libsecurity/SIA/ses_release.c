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
static char *rcsid = "@(#)$RCSfile: ses_release.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/12/16 23:55:58 $";
#endif
/*****************************************************************************
*
*	int	siad_ses_release(ENTITY *entityhdl,
*				int pkgind)
*
* Description: Free's any C2 mechanism specific data attached to the
* entity structure.  Called from sia_ses_release.
*
******************************************************************************/
#include <sia.h>
#include <siad.h>
#include "sia_mech.h"
#ifndef	SET_PROC_ACNTL
#include <sys/audit.h>
#endif
#ifndef	PRIO_PROCESS
#include <sys/resource.h>
#endif

siad_ses_release(entity, pkgind)
SIAENTITY *entity;
int pkgind;
{
	register uid_t savuid;
	register C2_MECH *mp = EN_MECH(entity, pkgind);

	if (mp) {
		if (mp->setup_done) {
			savuid = geteuid();
			if (setreuid(0, 0) < 0)
				goto skipit;	/* login still broken */
			if (audcntl(SET_PROC_AMASK, (char *) mp->audmask,
			    sizeof (mp->audmask), 0L, 0L, 0L) < 0) {
				show_perror("audcntl: SET_PROC_AMASK");
			}
			if (audcntl(SET_PROC_ACNTL, (char *)0, 0L,
			    mp->audit_cntl, 0L, 0L) < 0) {
				show_perror("audcntl: SET_PROC_ACNTL");
			}
			if (mp->new_nice <= PRIO_MAX) {
				if (setpriority(PRIO_PROCESS, 0,
				    mp->new_nice) < 0) {
					show_perror("setpriority");
				}
			}
		skipit:
			(void) seteuid(savuid);
		}
		free(mp);
		entity->mech[pkgind] = (int *) 0;
	}
	return SIADSUCCESS;
}
