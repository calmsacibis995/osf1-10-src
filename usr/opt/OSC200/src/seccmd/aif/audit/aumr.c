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
static char	*sccsid = "@(#)$RCSfile: aumr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:16 $";
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
 * Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved
 */



/*
 * Routines to implement restoration of audit sessions
 */

#include <sys/secdefines.h>

#if SEC_BASE

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include "gl_defs.h"
#include "IfAudit.h"
#include "userif.h"
#include "UIMain.h"
#include "valid.h"
#include "logging.h"

/* static routine definitions */

static int aumr_auth();
static int aumr_bfill();
static int aumr_valid();
static int aumr_exit();

/* structures defined in au_scrns.c */

extern Scrn_parms	aumr_scrn;
extern Scrn_desc	aumr_desc[];

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	aumr_scrn
#define STRUCTTEMPLATE	aumr_struct
#define DESCTEMPLATE	aumr_desc
#define FILLINSTRUCT	ls_fillin
#define FILLIN		aumr_fill
#define TRAVERSERW	TRAV_RW

#define RST_TITLE_DESC	0
#define RST_RSTDEV_DESC	1

#define RST_DEV_STRUCT	0
#define NSCRNSTRUCT	1

#define FIRSTDESC	RST_RSTDEV_DESC

static Scrn_struct	*aumr_struct;
static struct ls_fillin au_buf, *aumr_fill = &au_buf;

/* Device name */

char DeviceName[21];

static int IsISSO;

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

aumr_auth(argv, aufill)
	char **argv;
	struct ls_fillin *aufill;
{
	static int first_time = 1;

	ENTERFUNC("aumr_auth");
	if (first_time) {
		RestoreStart();
		first_time = 0;
		IsISSO = authorized_user("isso");
	}
	EXITFUNC("aumr_auth");
	if (IsISSO)
		return 0;
	else
		return 1;
}

/*
 * There is only one field.  Nothing to do.
 */

static int
aumr_bfill(aufill)
	struct ls_fillin *aufill;
{
	int ret;

	ENTERFUNC("aumr_bfill");

	memset(DeviceName, '\0', sizeof DeviceName);

	EXITFUNC("aumr_bfill");
	return 0;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
aumr_bstruct(aufill, sptemplate)
	struct ls_fillin *aufill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;

	ENTERFUNC("aumr_bstruct");

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	sp[RST_DEV_STRUCT].pointer = DeviceName;
	sp[RST_DEV_STRUCT].filled = 1;
	sp[RST_DEV_STRUCT].validate = NULL;

	EXITFUNC("aumr_bstruct");
	return 0;
}

/*
 * action routine.
 * Perform restore of audit session.
 */

static int
aumr_action(aufill)
	struct ls_fillin *aufill;
{
	int ret;

	ENTERFUNC("aumr_action");

	ret = RestoreFiles(DeviceName);

	if (ret == 0)
		ret = QUIT;
	else
		ret = INTERRUPT;

	EXITFUNC("aumr_action");
	return ret;
}


static void
mr_free(argv, aufill, nstructs, pp, dp, sp)
	char **argv;
	struct ls_fillin *aufill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("mr_free");
	EXITFUNC("mr_free");
	return;
}

/*
 * validate the structure
 * -- at least one of backup/delete set
 * -- at least one session is selected
 * -- the device is in the device assignment database (SEC_ARCH).
 * -- not trying to delete the current session.
 */

static int
aumr_valid(argv, aufill)
	char **argv;
	struct ls_fillin *aufill;
{
	int ret = 0;
	int i;
	struct dev_asg *dv, *getdvagent();
	int found = 0;

	ENTERFUNC("aumr_valid");

#if SEC_ARCH

	/* Check that the device is in the device assignment database */

	setdvagent();
	while ((dv = getdvagent()) && !found) {
		int i;
		char *cp;

		if (strcmp(dv->ufld.fd_name, DeviceName) == 0)
			break;

		if (dv->uflg.fg_devs)
			for (i = 0; cp = dv->ufld.fd_devs[i]; i++)
				if (strcmp(cp, DeviceName) == 0) {
					found = 1;
					break;
				}
	}
	if (dv == (struct dev_asg *) 0) {
		pop_msg(
		  "The device you specified is not listed in",
		  "the device assignment database.");
		ret = 1;
	}
#endif

	EXITFUNC("aumr_valid");
	return ret;
}

#define SETUPFUNC	aumr_setup	/* defined by stemplate.c */
#define AUTHFUNC	aumr_auth
#define BUILDFILLIN	aumr_bfill

#define INITFUNC	aumr_init		/* defined by stemplate.c */
#define BUILDSTRUCT	aumr_bstruct

#define ROUTFUNC	aumr_exit		/* defined by stemplate.c */
#define VALIDATE	aumr_valid
#define SCREENACTION	aumr_action

#define FREEFUNC	aumr_free		/* defined by stemplate.c */
#define FREESTRUCT	mr_free

#include "stemplate.c"

#endif /* SEC_BASE */
