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

static PASSWD pwd;
static char buf[SIABUFSIZ];
static FILE *context=NULL;

PASSWD *getpwent()
{
	union sia_get_params pg;
	pg.passwd.buffer = buf;
	pg.passwd.result = &pwd;
	pg.passwd.len = sizeof buf;
	pg.passwd.name = (char *) &context;
	if (sia_getpasswd(P_ENT, REENTRANT, &pg) == SIASUCCESS)
		return &pwd;
	else
		return (PASSWD *) 0;
}

PASSWD *getpwnam(name)
const char *name;
{
	union sia_get_params pg;
	pg.passwd.buffer = buf;
	pg.passwd.result = &pwd;
	pg.passwd.len = sizeof buf;
	pg.passwd.name = (char *) name;
	if (sia_getpasswd(P_NAM, REENTRANT, &pg) == SIASUCCESS)
		return &pwd;
	else
		return (PASSWD *) 0;
}

PASSWD *getpwuid(uid)
uid_t uid;
{
	union sia_get_params pg;
	pg.passwd.buffer = buf;
	pg.passwd.result = &pwd;
	pg.passwd.len = sizeof buf;
	pg.passwd.uid = uid;
	if (sia_getpasswd(P_UID, REENTRANT, &pg) == SIASUCCESS)
		return &pwd;
	else
		return (PASSWD *) 0;
}

int setpwent()
{
	union sia_get_params pg;
	pg.passwd.buffer = buf;
	pg.passwd.result = &pwd;
	pg.passwd.len = sizeof buf;
	pg.passwd.name = (char *) &context;
	if(sia_getpasswd(P_SET, REENTRANT, &pg) == SIAFAIL)
		return 0;
	else
		return 1;
}

void endpwent()
{
	union sia_get_params pg;
	pg.passwd.buffer = buf;
	pg.passwd.result = &pwd;
	pg.passwd.len = sizeof buf;
	pg.passwd.name = (char *) &context;
	(void) sia_getpasswd(P_END, REENTRANT, &pg);
}
