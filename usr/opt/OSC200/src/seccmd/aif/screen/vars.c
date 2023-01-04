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
static char	*sccsid = "@(#)$RCSfile: vars.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:58:52 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */



/*
	vars.c - generic time & date routines
*/

#include <time.h>
#ifdef OSF
#include <sys/types.h>
#include <sys/timeb.h>
typedef time_t Time_type;
#else OSF
typedef long Time_type;
#endif OSF
#include "aif.h"

#define TD_SIZE 13
#define GU_SIZE 32

Time_type now;
struct tm *tm_now;

Time_type MakeDate (Date)
uchar *Date;
{
	now = time ((long *) 0);
	tm_now = localtime (&now);
	sprintf (Date, "%2d/%2d/%2d", tm_now->tm_mon+1, tm_now->tm_mday+1,
		((tm_now->tm_year % 1000) % 100));
}

Time_type MakeTime (Time)
uchar *Time;
{
	now = time ((long *) 0);
	tm_now = localtime (&now);
	sprintf (Time, "%2d:%2.2d", tm_now->tm_hour, tm_now->tm_min);
}

uchar *MakeGroup (Group)
uchar *Group;
{
	sprintf (Group, "Group: %s", gl_group);
}

uchar *MakeGID (GID)
uchar *GID;
{
	sprintf (GID, "GID: %d", gl_gid);
}

uchar *MakeUser (User)
uchar *User;
{
	sprintf (User, "User: %s", gl_user);
}

uchar *MakeUID (UID)
uchar *UID;
{
	sprintf (UID, "UID: %d", gl_uid);
}

uchar *MakeDev (Dev)
uchar *Dev;
{
	sprintf (Dev, "Dev: %s", gl_device);
}

uchar *MakeRunner (Runner)
uchar *Runner;
{
	strcpy (Runner, gl_runner);
}

/*
 * updhfv () - update header & footer variables
 */

uchar *updhfv (s)
uchar *s;
{
	uchar *tdp;
	if (!strcmp (s, "$DATE")) {
		tdp = (uchar *) Malloc (TD_SIZE);
		MakeDate (tdp);
	} else if (!strcmp (s, "$TIME")) {
		tdp = (uchar *) Malloc (TD_SIZE);
		MakeTime (tdp);
	} else if (!strcmp (s, "$USER") && gl_user && *gl_user) {
		tdp = (uchar *) Malloc (GU_SIZE);
		MakeUser (tdp);
	} else if (!strcmp (s, "$UID") && gl_uid != BOGUS_ID) {
		tdp = (uchar *) Malloc (GU_SIZE);
		MakeUID (tdp);
	} else if (!strcmp (s, "$DEV") && gl_device && *gl_device) {
		tdp = (uchar *) Malloc (GU_SIZE);
		MakeDev (tdp);
	} else if (!strcmp (s, "$GROUP") && gl_group && *gl_group) {
		tdp = (uchar *) Malloc (GU_SIZE);
		MakeGroup (tdp);
	} else if (!strcmp (s, "$GID") && gl_gid != BOGUS_ID) {
		tdp = (uchar *) Malloc (GU_SIZE);
		MakeGID (tdp);
	} else if (!strcmp (s, "$RUNNER") && gl_runner && *gl_runner) {
		tdp = (uchar *) Malloc (GU_SIZE);
		MakeRunner (tdp);
	} else
		tdp = s;
	return tdp;
}

/*
 * not_vars (s) - TRUE if string is not a $VAR string, else FALSE
 */

not_vars (s)
uchar *s;
{
	return (strcmp (s, "$DATE") && strcmp (s, "$TIME") &&
		strcmp (s, "$USER") && strcmp (s, "$GROUP") &&
		strcmp (s, "$UID") && strcmp (s, "$GID") &&
		strcmp (s, "$DEV") && strcmp (s, "$RUNNER"));
}

#ifndef UNAMELEN
#include "UIMain.h"
#endif /* UNAMELEN */

/*
 * init_vars() - initialize vars
 */
init_vars()
{
	gl_user = (uchar *) Malloc (UNAMELEN + 1);
	gl_user[0] = NULL;
	gl_uid = BOGUS_ID;
	gl_group = (uchar *) Malloc (GNAMELEN + 1);
	gl_group[0] = NULL;
	gl_gid = BOGUS_ID;
	gl_device = (uchar *) Malloc (DEVICE_NAME_LEN + 1);
	gl_device[0] = NULL;
	gl_host = (uchar *) Malloc (HOST_NAME_LEN + 1);
	gl_host[0] = NULL;
	gl_printer = (uchar *) Malloc (PRINTER_NAME_LEN + 1);
	gl_printer[0] = NULL;
	gl_tape = (uchar *) Malloc (TAPE_NAME_LEN + 1);
	gl_tape[0] = NULL;
	gl_terminal = (uchar *) Malloc (TERMINAL_NAME_LEN + 1);
	gl_terminal[0] = NULL;
	gl_runner = (uchar *) Malloc (GU_SIZE);
	gl_runner[0] = NULL;
}
