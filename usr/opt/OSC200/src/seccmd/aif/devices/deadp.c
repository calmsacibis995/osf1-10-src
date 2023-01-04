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
static char	*sccsid = "@(#)$RCSfile: deadp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:52 $";
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
 * Routines to implement Add Printer screen
 * Veneer routines for those in deptr.c
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
#include "dedevs.h"

/* static routine definitions */

static int deadp_auth();
static int deadp_bfill();
static int deadp_valid();
static int deadp_exit();
static int deadp_setup();
static int deadp_init();
static int deadp_free();

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	deptr_scrn
#define STRUCTTEMPLATE	deadp_struct
#define DESCTEMPLATE	deptr_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		deadp_fill

/* structures defined in de_scrns.c */

extern Scrn_parms	deptr_scrn;
extern Scrn_desc	deptr_desc[];

#define FIRSTDESC	NAME_DESC
#define NSCRNSTRUCT	PTR_NSCRNSTRUCT

static Scrn_struct	*deadp_struct;
static struct device_fillin de_buf, *deadp_fill = &de_buf;

/* called when a printer is selected and the user specifies Add */

int
deadp_doit()
{
	int i;

	deptr_scrn.setup = deadp_setup;
	deptr_scrn.init = deadp_init;
	deptr_scrn.free = deadp_free;

	for (i = 0; i < NSCRNSTRUCT; i++)
		deptr_scrn.ms[i].next_routine = deadp_exit;

	return traverse(&deptr_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Send the fillin structure and the mode to the lower level routine.
 */

int
deadp_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	int ret;

	ENTERFUNC("deadp_auth");

	ret = device_auth(defill, DeviceCreate);

	EXITFUNC("deadp_auth");

	return ret;
}

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
deadp_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("deadp_bfill");

	ret = deptr_bfill(defill);

	EXITFUNC("deadp_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
deadp_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("deadp_bstruct");

	ret = deptr_bstruct(defill, sptemplate);

	EXITFUNC("deadp_bstruct");

	return ret;
}

/*
 * action routine.
 * Call lower level routine
 */

int
deadp_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("deadp_action");

	ret = device_action(defill);

	EXITFUNC("deadp_action");

	return ret;
}


void
adp_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("adp_free");

	device_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("adp_free");
	return;
}

/*
 * validate the structure
 */

int
deadp_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret = 0;

	ENTERFUNC("deadp_valid");

	ret = device_valid(argv, defill);

	EXITFUNC("deadp_valid");
	return ret;
}

#define SETUPFUNC	deadp_setup		/* defined by stemplate.c */
#define AUTHFUNC	deadp_auth
#define BUILDFILLIN	deadp_bfill

#define INITFUNC	deadp_init		/* defined by stemplate.c */
#define BUILDSTRUCT	deadp_bstruct

#define ROUTFUNC	deadp_exit		/* defined by stemplate.c */
#define VALIDATE	deadp_valid
#define SCREENACTION	deadp_action

#define FREEFUNC	deadp_free		/* defined by stemplate.c */
#define FREESTRUCT	adp_free

#include "stemplate.c"

#endif /* SEC_BASE */
