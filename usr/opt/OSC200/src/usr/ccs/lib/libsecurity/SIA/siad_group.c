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
#include <sia.h>
#include <siad.h>
#include "sia_mech.h"

static struct group grp;
static char buf[SIABUFSIZ];
static FILE *context=NULL;

struct group *getgrent()
{
	union sia_get_params pg;
	pg.group.buffer = buf;
	pg.group.result = &grp;
	pg.group.len = sizeof buf;
	pg.group.name = (char *) &context;
	if (sia_getgroup(G_ENT, REENTRANT, &pg) == SIASUCCESS)
		return &grp;
	else
		return (struct group *) 0;
}

struct group *getgrnam(name)
const char *name;
{
	union sia_get_params pg;
	pg.group.buffer = buf;
	pg.group.result = &grp;
	pg.group.len = sizeof buf;
	pg.group.name = (char *) name;
	if (sia_getgroup(G_NAM, REENTRANT, &pg) == SIASUCCESS)
		return &grp;
	else
		return (struct group *) 0;
}

struct group *getgrgid(gid)
gid_t gid;
{
	union sia_get_params pg;
	pg.group.buffer = buf;
	pg.group.result = &grp;
	pg.group.len = sizeof buf;
	pg.group.gid = gid;
	if (sia_getgroup(G_GID, REENTRANT, &pg) == SIASUCCESS)
		return &grp;
	else
		return (struct group *) 0;
}

int setgrent()
{
	union sia_get_params pg;
	pg.group.buffer = buf;
	pg.group.result = &grp;
	pg.group.len = sizeof buf;
	pg.group.name = (char *) &context;
	if(sia_getgroup(G_SET, REENTRANT, &pg) == SIAFAIL)
		return 0;
	else
		return 1;
}

void endgrent()
{
	union sia_get_params pg;
	pg.group.buffer = buf;
	pg.group.result = &grp;
	pg.group.len = sizeof buf;
	pg.group.name = (char *) &context;
	(void) sia_getgroup(G_END, REENTRANT, &pg);
}
