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
static char	*sccsid = "@(#)$RCSfile: initgroups.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/09/23 18:29:47 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCADM) Standard C Library System Admin Functions 
 *
 * FUNCTIONS: initgroups 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * initgroups.c	1.6  com/lib/c/adm,3.1,8943 9/13/89 10:32:11
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak initgroups = __initgroups
#endif
#include <stdio.h>
#include <sys/param.h>
#include <grp.h>
#include "libc_msg.h"

/*
 * NAME: isagroup
 *
 * FUNCTION: check for existing group membership
 *
 * RETURN VALUE DESCRIPTIONS:
 *	1 - group is already present
 *	0 - group is not already present
 */

static int
isagroup (int groups[], int ngroups, gid_t group)
{
	int	i;

	for (i = 0;i < ngroups;i++)
		if (groups[i] == group)
			return 1;

	return 0;
}

/*                                                                    
 * NAME: initgroups
 *
 * FUNCTION: initialize group membership list
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     - 0 on success
 *	     - 1 on failure
 */

initgroups(uname, agroup)
	char *uname;
	int agroup;
{
	register struct group *grp;
	register int i;
	int groups[NGROUPS];
	int ngroups = 0;
#ifdef _THREAD_SAFE
	char line[BUFSIZ+1];
	struct group group;
	FILE *grf;

	grp = &group;
#endif
	if (agroup >= 0)
		groups[ngroups++] = agroup;

#ifdef _THREAD_SAFE
	setgrent_r(&grf);
	while (getgrent_r(grp, line, sizeof(line) - 1, &grf) == 0) {
#else
	setgrent();
	while (grp = getgrent()) {
#endif
		if (grp->gr_gid == agroup ||
		    isagroup (groups, ngroups, grp->gr_gid))
			continue;
		for (i = 0; grp->gr_mem[i]; i++)
			if (!strcmp(grp->gr_mem[i], uname)) {
				if (ngroups == NGROUPS) {
					nl_catd	catd = catopen(MF_LIBC, NL_CAT_LOCALE);
					fprintf(stderr,
						catgets(catd, MS_LIBC, M_INITG,
				"initgroups: %s is in too many groups\n"),
						uname);
					catclose(catd);
					goto toomany;
				}
				groups[ngroups++] = grp->gr_gid;
			}
	}
toomany:
#ifdef _THREAD_SAFE
	endgrent_r(&grf);
#else
	endgrent();
#endif
	return setgroups(ngroups, groups);
}
