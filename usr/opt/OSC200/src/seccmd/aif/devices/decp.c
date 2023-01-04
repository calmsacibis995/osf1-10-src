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
static char	*sccsid = "@(#)$RCSfile: decp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:06 $";
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
 * Routines to implement device control parameters screen
 */

#include <sys/secdefines.h>

#if SEC_BASE

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include "gl_defs.h"
#include "IfDevices.h"
#include "userif.h"
#include "UIMain.h"
#include "valid.h"
#include "logging.h"

/* static routine definitions */

static int decp_auth();
static int decp_bfill();
static int decp_valid();
static int decp_exit();

/* structures defined in de_scrns.c */

extern Scrn_parms	decp_scrn;
extern Scrn_desc	decp_desc[];

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	decp_scrn
#define STRUCTTEMPLATE	decp_struct
#define DESCTEMPLATE	decp_desc
#define FILLINSTRUCT	def_device_fillin
#define FILLIN		decp_fill

#define UNSUC_LOG_TITLE_DESC	0
#define UNSUC_LOG_DESC		1
#define LOG_TIMEOUT_TITLE_DESC	2
#define LOG_TIMEOUT_DESC	3
#define LOG_DELAY_TITLE_DESC	4
#define LOG_DELAY_DESC		5
#define CONTROL_TITLE_DESC	6
#define SET_TITLE_DESC		7
#define CONTROL_UNDER_DESC	8
#define SET_UNDER_DESC		9
#define LOCK_TITLE_DESC		10
#define LOCK_DESC		11

#define UNSUC_LOG_STRUCT	0
#define LOG_TIMEOUT_STRUCT	1
#define LOG_DELAY_STRUCT	2
#define LOCK_STRUCT		3
#define NSCRNSTRUCT		4

#define FIRSTDESC	UNSUC_LOG_DESC

static Scrn_struct	*decp_struct;
static struct def_device_fillin de_buf, *decp_fill = &de_buf;

static int IsISSO;

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

decp_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	static int first_time = 1;

	ENTERFUNC("decp_auth");
	if (first_time) {
		first_time = 0;
		IsISSO = authorized_user("isso");
	}
	EXITFUNC("decp_auth");
	if (IsISSO)
		return 0;
	else
		return 1;
}

/*
 * There is only one field.  Nothing to do.
 */

static int
decp_bfill(defill)
	struct def_device_fillin *defill;
{
	int ret;

	ENTERFUNC("decp_bfill");

	ret = DevGetDefFillin(defill);

	EXITFUNC("decp_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
decp_bstruct(defill, sptemplate)
	struct def_device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;

	ENTERFUNC("decp_bstruct");

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	sp[UNSUC_LOG_STRUCT].pointer = (char *) &defill->max_ulogins;
	sp[UNSUC_LOG_STRUCT].filled = 1;

	sp[LOG_TIMEOUT_STRUCT].pointer = (char *) &defill->login_timeout;
	sp[LOG_TIMEOUT_STRUCT].filled = 1;

	sp[LOG_DELAY_STRUCT].pointer = (char *) &defill->login_delay;
	sp[LOG_DELAY_STRUCT].filled = 1;

	sp[LOCK_STRUCT].pointer = &defill->locked;
	sp[LOCK_STRUCT].filled = 1;

	EXITFUNC("decp_bstruct");
	return 0;
}

/*
 * action routine.
 * Rewrite default database
 */

static int
decp_action(defill)
	struct def_device_fillin *defill;
{
	int ret;

	ENTERFUNC("decp_action");

	ret = DevChangeDefControlParams(defill);

	EXITFUNC("decp_action");
	return INTERRUPT;
}


static void
cp_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct def_device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("cp_free");

	DevFreeDefFillin(defill);

	EXITFUNC("cp_free");
	return;
}

/*
 * validate the structure
 */

static int
decp_valid(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	int ret = 0;
	int i;
	struct dev_asg *dv, *getdvagent();
	int found = 0;

	ENTERFUNC("decp_valid");

	ret = DevValidateDefControlParams(defill);

	EXITFUNC("decp_valid");
	return ret;
}

#define SETUPFUNC	decp_setup	/* defined by stemplate.c */
#define AUTHFUNC	decp_auth
#define BUILDFILLIN	decp_bfill

#define INITFUNC	decp_init		/* defined by stemplate.c */
#define BUILDSTRUCT	decp_bstruct

#define ROUTFUNC	decp_exit		/* defined by stemplate.c */
#define VALIDATE	decp_valid
#define SCREENACTION	decp_action

#define FREEFUNC	decp_free		/* defined by stemplate.c */
#define FREESTRUCT	cp_free

#include "stemplate.c"

#endif /* SEC_BASE */
