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
static char *rcsid = "@(#)$RCSfile: chk_user.c,v $ $Revision: 1.1.7.2 $ (DEC) $Date: 1993/04/01 20:20:23 $";
#endif
/*****************************************************************************
*
*	int	siad_chk_user(char *loginname, int  chkflg)
*
* Description: The purpose of this routine is to check the calling routine
* for appropriate privaleges, ussually uid==0.
*
* Returns: 
*		SIADSUCCESS => CONTINUE
*
*		SIADFAIL => DO NOT CONTINUE 
*				Configuration Showstopper
******************************************************************************/
#include <sys/secdefines.h>
#include <sia.h>
#include <siad.h>
#include "sia_mech.h"
#include <sys/security.h>
#include <prot.h>

extern struct passwd siad_bsd_passwd;
extern char siad_bsd_getpwbuf[];
extern struct passwd *bsd_siad_getpwnam(), *getpwnam_local();

int
siad_chk_user(loginname,chkflg)
const char *loginname;
int  chkflg;
{
	struct passwd pwbuf;
	char dbuf[SIABUFSIZ];

	if (chkflg & ~(CHGPASSWD | CHGSHELL | CHGFINGER))
		return SIADFAIL;	/* I don't understand other bits */

	if (chkflg & CHGPASSWD) {
		if (bsd_siad_getpwnam(loginname, &pwbuf, dbuf, sizeof dbuf)
				== SIADFAIL)
			return SIADFAIL;
		/* unfortunately, I don't have the argc/argv for setting
		 * up the auth stuff yet, so I have to stop here.
		 */
		return SIADSUCCESS;
	}
	/* disallow NIS for finger & shell */
	/* This may change--I'm not sure whether we have an extended
	 * yppasswdd yet.
	 */
	if (getpwnam_local(loginname) == NULL)
		return SIADFAIL;
	return SIADSUCCESS;
}
