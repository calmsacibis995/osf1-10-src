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
static char	*sccsid = "@(#)$RCSfile: desdr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:58 $";
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
 * Routines to implement removable device selection screen.
 * Merely calls into Device Selection helper routines specifying Removables.
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

static int desdr_auth();
static int desdr_bfill();
static int desdr_valid();
static int desdr_exit();
static int desdr_setup();
static int desdr_init();
static int desdr_free();

/* structures defined in de_scrns.c */

extern Scrn_parms	desd_scrn;
extern Scrn_desc	desd_desc[];
extern Menu_scrn	desd_menu[];
extern uchar		desd_removable[];
extern Scrn_hdrs	desd_hdrs;

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	desd_scrn
#define STRUCTTEMPLATE	desd_struct
#define DESCTEMPLATE	desd_desc
#define FILLINSTRUCT	device_fillin	/* not used */
#define FILLIN		desdr_fill

#define SELECT_TITLE_DESC	0
#define SELECT_DESC		1

#define SELECT_STRUCT		0
#define NSCRNSTRUCT		1

#define FIRSTDESC	SELECT_DESC

extern Scrn_struct	*desd_struct;
static struct device_fillin de_buf, *desdr_fill = &de_buf;

int
desdr_doit()
{
	desd_menu[0].next_routine = desdr_exit;
	desd_scrn.init = desdr_init;
	desd_scrn.setup = desdr_setup;
	desd_scrn.free = desdr_free;

	return traverse(&desd_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

desdr_auth(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("desdr_auth");

	ret = desd_auth(RemovableDevice);

	EXITFUNC("desdr_auth");

	return ret;
}

/*
 * Work for this rotuine is accomplished by desd_bfill
 */

static int
desdr_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("desdr_bfill");

	ret = desd_bfill(defill);
	desd_desc[SELECT_TITLE_DESC].prompt = (char *) desd_removable;
	desd_desc[SELECT_TITLE_DESC].col = (COLS - strlen(desd_removable)) / 2;

	EXITFUNC("desdr_bfill");

	return 0;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
desdr_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("desdr_bstruct");

	ret = desd_bstruct(defill, sptemplate);

	EXITFUNC("desdr_bstruct");

	return ret;
}

/*
 * action routine.
 * Store the device that was selected
 */

static int
desdr_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("desdr_action");

	ret = desd_action(defill);

	EXITFUNC("desdr_action");

	return ret;
}


static void
sdr_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	int ret;

	ENTERFUNC("sdr_free");

	ret = sd_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("sdr_free");
	return;
}

/*
 * There is no validation routine.
 */

static int
desdr_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("desdr_valid");

	ret = desd_valid(argv, defill);

	EXITFUNC("desdr_valid");

	return ret;
}

#define SETUPFUNC	desdr_setup	/* defined by stemplate.c */
#define AUTHFUNC	desdr_auth
#define BUILDFILLIN	desdr_bfill

#define INITFUNC	desdr_init		/* defined by stemplate.c */
#define BUILDSTRUCT	desdr_bstruct

#define ROUTFUNC	desdr_exit		/* defined by stemplate.c */
#define VALIDATE	desdr_valid
#define SCREENACTION	desdr_action

#define FREEFUNC	desdr_free		/* defined by stemplate.c */
#define FREESTRUCT	sdr_free

#include "stemplate.c"

#endif /* SEC_BASE */
