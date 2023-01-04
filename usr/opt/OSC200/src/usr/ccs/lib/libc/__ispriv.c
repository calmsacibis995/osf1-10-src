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
static char *rcsid = "@(#)$RCSfile: __ispriv.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/06/08 01:21:10 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/* 
 * __ispriv
 *
 * FUNCTION: determine if current program is privileged relative to the
 *           invoking user.
 *
 * PARAMETERS: none
 *
 * RETURN VALUES: 1 -- privileged
 *                0 -- not privileged
 *
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/secdefines.h>

#if SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#include <syscall.h>

extern int syscall();

#else
#include <sys/types.h>
#endif

int
__ispriv()
{

#if SEC_BASE

/* SEC_BASE version checks whether the effective or potential privileges
 * of the process include any privileges not in the user's base privileges.
 */

	privvec_t effective, potential, base;
	obj_t objid;
	int i;

	objid.o_pid = 0;

	/* if the syscall fails for any reason, return TRUE to be on the
         * safe side
	 */

	if (syscall(SYS_security, SEC_STATPRIV, SEC_EFFECTIVE_PRIV,
		    (ulong) effective, OT_PROCESS, &objid, 0L, 0L) == -1)
		return (1);

	if (syscall(SYS_security, SEC_STATPRIV, SEC_POTENTIAL_PRIV,
		    (ulong) potential, OT_PROCESS, &objid, 0L, 0L) == -1)
		return (1);

	if (syscall(SYS_security, SEC_STATPRIV, SEC_BASE_PRIV,
		    (ulong) base, OT_PROCESS, &objid, 0L, 0L) == -1)
		return (1);

	for (i = 0; i < SEC_SPRIVVEC_SIZE; ++i)
		if ((potential[i] | effective[i] | base[i]) != base[i])
			return (1);

	return (0);

#else  /* SEC_PRIV */

	return(!geteuid());

#endif /* SEC_BASE */

}

