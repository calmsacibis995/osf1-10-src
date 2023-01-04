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
static char	*sccsid = "@(#)$RCSfile: hwconf.c,v $ $Revision: 1.2.4.3 $ (DEC) $Date: 1992/09/29 09:16:28 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from hwconf.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif
#include <sys/user.h>
#include <machine/hwconf.h>

struct hw_config hwconf;

int
mipshwconf(p, args, retval)
	void *p;
	void *args;
	int *retval;
{
	char	*pname;
	struct args {
		int option;
		struct hw_config *info;
	} *uap = (struct args *) args;
	
	char *prom_getenv();
	int prom_setenv();
	register int i;
	struct hw_config tmp;
	int error = 0;

	switch (uap->option) {

	case HWCONF_GET:
		for (i=0; i < ENV_ENTRIES; i++) {
			/* check validity of promenv.name pointer before using 
			 * it.
			 */
			pname = prom_getenv(hwconf.promenv[i].name);
			if (pname) {
				strncpy(hwconf.promenv[i].value, 
				    pname, ENV_MAXLEN);
			}
		}
		error = copyout((caddr_t)&hwconf, (caddr_t)uap->info,
				sizeof(struct hw_config));
		break;

	case HWCONF_SET:
#if	SEC_BASE
		if (!privileged(SEC_SYSATTR, 0))
			return EACCES;
#else
		if (suser(u.u_cred, &u.u_acflag))
			return EACCES;
#endif
		error = copyin((caddr_t)uap->info, (caddr_t)&tmp,
				sizeof(struct hw_config));
		if (error)
			return(error);
		for (i=0; i < ENV_ENTRIES; i++) {
		    if (!strcmp(tmp.promenv[i].name, hwconf.promenv[i].name))
			prom_setenv(tmp.promenv[i].name, tmp.promenv[i].value);
		}
		break;

	default:
		return(EINVAL);
	}
	return(error);
}

hwconf_init()
{
	strcpy(hwconf.promenv[0].name, "netaddr");
	strcpy(hwconf.promenv[1].name, "lbaud");
	strcpy(hwconf.promenv[2].name, "rbaud");
	strcpy(hwconf.promenv[3].name, "bootfile");
	strcpy(hwconf.promenv[4].name, "bootmode");
	strcpy(hwconf.promenv[5].name, "console");
}
