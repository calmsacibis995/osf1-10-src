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
static char	*sccsid = "@(#)$RCSfile: desdt.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:01 $";
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
 * Routines to implement terminal selection screen.
 * Merely calls into Device Selection helper routines specifying Terminals.
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

static int desdt_auth();
static int desdt_bfill();
static int desdt_valid();
static int desdt_exit();
static int desdt_setup();
static int desdt_init();
static int desdt_free();

/* structures defined in de_scrns.c */

extern Scrn_parms	desd_scrn;
extern Scrn_desc	desd_desc[];
extern Menu_scrn	desd_menu[];
extern uchar		desd_terminal[];
extern Scrn_hdrs	desd_hdrs;

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	desd_scrn
#define STRUCTTEMPLATE	desd_struct
#define DESCTEMPLATE	desd_desc
#define FILLINSTRUCT	device_fillin	/* not used */
#define FILLIN		desdt_fill

#define SELECT_TITLE_DESC	0
#define SELECT_DESC		1

#define SELECT_STRUCT		0
#define NSCRNSTRUCT		1

#define FIRSTDESC	SELECT_DESC

extern Scrn_struct	*desd_struct;
static struct device_fillin de_buf, *desdt_fill = &de_buf;

int
desdt_doit()
{
	desd_menu[0].next_routine = desdt_exit;
	desd_scrn.init = desdt_init;
	desd_scrn.setup = desdt_setup;
	desd_scrn.free = desdt_free;

	return traverse(&desd_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

desdt_auth(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("desdt_auth");

	ret = desd_auth(TerminalDevice);

	EXITFUNC("desdt_auth");

	return ret;
}

/*
 * Work for this rotuine is accomplished by desd_bfill
 */

static int
desdt_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("desdt_bfill");

	ret = desd_bfill(defill);
	desd_desc[SELECT_TITLE_DESC].prompt = (char *) desd_terminal;
	desd_desc[SELECT_TITLE_DESC].col = (COLS - strlen(desd_terminal)) / 2;

	EXITFUNC("desdt_bfill");

	return 0;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
desdt_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("desdt_bstruct");

	ret = desd_bstruct(defill, sptemplate);

	EXITFUNC("desdt_bstruct");

	return ret;
}

/*
 * action routine.
 * Store the device that was selected
 */

static int
desdt_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("desdt_action");

	ret = desd_action(defill);

	EXITFUNC("desdt_action");

	return ret;
}


static void
sdt_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	int ret;

	ENTERFUNC("sdt_free");

	ret = sd_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("sdt_free");
	return;
}

/*
 * There is no validation routine.
 */

static int
desdt_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("desdt_valid");

	ret = desd_valid(argv, defill);

	EXITFUNC("desdt_valid");

	return ret;
}

#define SETUPFUNC	desdt_setup	/* defined by stemplate.c */
#define AUTHFUNC	desdt_auth
#define BUILDFILLIN	desdt_bfill

#define INITFUNC	desdt_init		/* defined by stemplate.c */
#define BUILDSTRUCT	desdt_bstruct

#define ROUTFUNC	desdt_exit		/* defined by stemplate.c */
#define VALIDATE	desdt_valid
#define SCREENACTION	desdt_action

#define FREEFUNC	desdt_free		/* defined by stemplate.c */
#define FREESTRUCT	sdt_free

#include "stemplate.c"

#endif /* SEC_BASE */
