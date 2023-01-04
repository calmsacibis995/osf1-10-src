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
static char	*sccsid = "@(#)$RCSfile: desdh.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:52 $";
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
 * Routines to implement host selection screen.
 * Merely calls into Device Selection helper routines specifying Hosts.
 */

#include <sys/secdefines.h>

#if SEC_NET_TTY

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

static int desdh_auth();
static int desdh_bfill();
static int desdh_valid();
static int desdh_exit();
static int desdh_setup();
static int desdh_init();
static int desdh_free();

/* structures defined in de_scrns.c */

extern Scrn_parms	desd_scrn;
extern Scrn_desc	desd_desc[];
extern Menu_scrn	desd_menu[];
extern uchar		desd_host[];
extern Scrn_hdrs	desd_hdrs;

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	desd_scrn
#define STRUCTTEMPLATE	desd_struct
#define DESCTEMPLATE	desd_desc
#define FILLINSTRUCT	device_fillin	/* not used */
#define FILLIN		desdh_fill

#define SELECT_TITLE_DESC	0
#define SELECT_DESC		1

#define SELECT_STRUCT		0
#define NSCRNSTRUCT		1

#define FIRSTDESC	SELECT_DESC

extern Scrn_struct	*desd_struct;
static struct device_fillin de_buf, *desdh_fill = &de_buf;

int
desdh_doit()
{
	desd_menu[0].next_routine = desdh_exit;
	desd_scrn.init = desdh_init;
	desd_scrn.setup = desdh_setup;
	desd_scrn.free = desdh_free;

	return traverse(&desd_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

desdh_auth(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("desdh_auth");

	ret = desd_auth(HostDevice);

	EXITFUNC("desdh_auth");

	return ret;
}

/*
 * Work for this rotuine is accomplished by desd_bfill
 */

static int
desdh_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("desdh_bfill");

	ret = desd_bfill(defill);

	desd_desc[SELECT_TITLE_DESC].prompt = (char *) desd_host;

	EXITFUNC("desdh_bfill");

	return 0;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
desdh_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("desdh_bstruct");

	ret = desd_bstruct(defill, sptemplate);

	EXITFUNC("desdh_bstruct");

	return ret;
}

/*
 * action routine.
 * Store the device that was selected
 */

static int
desdh_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("desdh_action");

	ret = desd_action(defill);

	EXITFUNC("desdh_action");

	return ret;
}


static void
sdh_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	int ret;

	ENTERFUNC("sdh_free");

	ret = sd_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("sdh_free");
	return;
}

/*
 * There is no validation routine.
 */

static int
desdh_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("desdh_valid");

	ret = desd_valid(argv, defill);

	EXITFUNC("desdh_valid");

	return ret;
}

#define SETUPFUNC	desdh_setup	/* defined by stemplate.c */
#define AUTHFUNC	desdh_auth
#define BUILDFILLIN	desdh_bfill

#define INITFUNC	desdh_init		/* defined by stemplate.c */
#define BUILDSTRUCT	desdh_bstruct

#define ROUTFUNC	desdh_exit		/* defined by stemplate.c */
#define VALIDATE	desdh_valid
#define SCREENACTION	desdh_action

#define FREEFUNC	desdh_free		/* defined by stemplate.c */
#define FREESTRUCT	sdh_free

#include "stemplate.c"

#endif /* SEC_NET_TTY */
