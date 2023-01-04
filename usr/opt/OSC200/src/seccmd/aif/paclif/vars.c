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
static char *rcsid = "@(#)$RCSfile: vars.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:14:32 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	vars.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:05:53  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  17:00:35  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:20:58  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:54:33  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1990, SecureWare, Inc.
 *   All rights reserved
 *
 * Based on OSF version:
 *	@(#)vars.c	2.12 16:31:41 5/17/91 SecureWare
 */

/* #ident "@(#)vars.c	1.1 11:19:24 11/8/91 SecureWare" */

/*
	vars.c - generic time & date routines
*/

#include "If.h"
#include "IfDevices.h"
#include "AIf.h"
#include "scrn_local.h"
#include <time.h>

#define TD_SIZE 13	/* time/date buffer size */
#define GU_SIZE 32	/* user/device buffer size */

extern int	no_date_and_time;  /* for debugging */

time_t now;
struct tm *tm_now;

static void
MakeDate(Date)
char *Date;
{
	if (no_date_and_time) {
		sprintf (Date, "        ");
		return;
	}

	now = time((long *) 0);
	tm_now = localtime(&now);
	sprintf(Date, "%2d/%2d/%2d", tm_now->tm_mon+1, tm_now->tm_mday,
		tm_now->tm_year % 100);
}

static void
MakeTime(Time)
char *Time;
{
	char *suffix;

	if (no_date_and_time) {
		sprintf (Time, "        ");
		return;
	}

	now = time((long *) 0);
	tm_now = localtime(&now);
	if (tm_now->tm_hour == 0) {
		tm_now->tm_hour = 12;
		suffix = MSGSTR(VARS_1, "am");
	} else if (tm_now->tm_hour > 12) {
		tm_now->tm_hour -= 12;
		suffix = MSGSTR(VARS_2, "pm");
	} else if (tm_now->tm_hour == 12)
		suffix = MSGSTR(VARS_2, "pm");
	else
		suffix = MSGSTR(VARS_1, "am");

	sprintf(Time, "%2d:%2.2d %s", tm_now->tm_hour, tm_now->tm_min, suffix);
}

static void
MakeGroup(Group)
char *Group;
{
	sprintf(Group, MSGSTR(VARS_3, "Group: %s"), gl_group);
}

static void
MakeGID(GID)
char *GID;
{
	sprintf(GID, MSGSTR(VARS_4, "GID: %d"), gl_gid);
}

static void
MakeUser(User)
char *User;
{
	sprintf(User, MSGSTR(VARS_5, "User: %s"), gl_user);
}

static void
MakeUID(UID)
char *UID;
{
	sprintf(UID, MSGSTR(VARS_6, "UID: %d"), gl_uid);
}

static void
MakeDev(Dev)
char *Dev;
{
	sprintf(Dev, MSGSTR(VARS_7, "Dev: %s"), gl_device);
}

static void
MakeRunner(Runner)
char *Runner;
{
	strcpy(Runner, gl_runner);
}

/*
 * updhfv () - update header & footer variables
 */

char *
updhfv(s)
char *s;
{
	char *tdp;
	if (!strcmp (s, "$DATE")) {
		tdp = Malloc(TD_SIZE);
		MakeDate (tdp);
	} else if (!strcmp (s, "$TIME")) {
		tdp = Malloc(TD_SIZE);
		MakeTime (tdp);
	} else if (!strcmp (s, "$USER") && gl_user && *gl_user) {
		tdp = Malloc(GU_SIZE);
		MakeUser (tdp);
	} else if (!strcmp (s, "$UID") && gl_uid != BOGUS_ID) {
		tdp = Malloc(GU_SIZE);
		MakeUID (tdp);
	} else if (!strcmp (s, "$DEV") && gl_device && *gl_device) {
		tdp = Malloc(GU_SIZE);
		MakeDev (tdp);
	} else if (!strcmp (s, "$GROUP") && gl_group && *gl_group) {
		tdp = Malloc(GU_SIZE);
		MakeGroup (tdp);
	} else if (!strcmp (s, "$GID") && gl_gid != BOGUS_ID) {
		tdp = Malloc(GU_SIZE);
		MakeGID (tdp);
	} else if (!strcmp (s, "$RUNNER") && gl_runner && *gl_runner) {
		tdp = Malloc(GU_SIZE);
		MakeRunner (tdp);
	} else
		tdp = s;
	return tdp;
}

/*
 * not_vars (s) - TRUE if string is not a $VAR string, else FALSE
 */

int
not_vars(s)
char *s;
{
	return (strcmp (s, "$DATE") && strcmp (s, "$TIME") &&
		strcmp (s, "$USER") && strcmp (s, "$GROUP") &&
		strcmp (s, "$UID") && strcmp (s, "$GID") &&
		strcmp (s, "$DEV") && strcmp (s, "$RUNNER"));
}

/*
 * init_vars() - initialize vars
 */
init_vars()
{
	gl_user = Calloc(UNAMELEN + 1, 1);
	gl_uid = BOGUS_ID;
	gl_group = Calloc(GNAMELEN + 1, 1);
	gl_gid = BOGUS_ID;
	gl_device = Calloc(DEVICE_NAME_LEN + 1, 1);
	gl_host = Calloc(HOST_NAME_LEN + 1, 0);
	gl_printer = Calloc(PRINTER_NAME_LEN + 1, 1);
	gl_tape = Calloc(TAPE_NAME_LEN + 1, 1);
	gl_terminal = Calloc(TERMINAL_NAME_LEN + 1, 1);
	gl_runner = Calloc(GU_SIZE, 1);
}
