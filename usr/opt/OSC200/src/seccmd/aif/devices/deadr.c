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
static char	*sccsid = "@(#)$RCSfile: deadr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:55 $";
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
 * Routines to implement Add Removable screen
 * Veneer routines for those in dedevs.c
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

static int deadr_auth();
static int deadr_bfill();
static int deadr_valid();
static int deadr_exit();
static int deadr_setup();
static int deadr_init();
static int deadr_free();

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	dermv_scrn
#define STRUCTTEMPLATE	deadr_struct
#define DESCTEMPLATE	dermv_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		deadr_fill

/* structures defined in de_scrns.c */

extern Scrn_parms	dermv_scrn;
extern Scrn_desc	dermv_desc[];

static Scrn_struct	*deadr_struct;
static struct device_fillin de_buf, *deadr_fill = &de_buf;

#define FIRSTDESC NAME_DESC
#define NSCRNSTRUCT RMV_NSCRNSTRUCT

/* called when a removable is selected and the user specifies Add */

int
deadr_doit()
{
	int i;

	dermv_scrn.setup = deadr_setup;
	dermv_scrn.init = deadr_init;
	dermv_scrn.free = deadr_free;

	for (i = 0; i < NSCRNSTRUCT; i++)
		dermv_scrn.ms[i].next_routine = deadr_exit;

	return traverse(&dermv_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Send the fillin structure and the mode to the lower level routine.
 */

int
deadr_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	int ret;

	ENTERFUNC("deadr_auth");

	ret = device_auth(defill, DeviceCreate);

	EXITFUNC("deadr_auth");

	return ret;
}

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
deadr_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("deadr_bfill");

	ret = dermv_bfill(defill);

	EXITFUNC("deadr_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
deadr_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("deadr_bstruct");

	ret = dermv_bstruct(defill, sptemplate);

	EXITFUNC("deadr_bstruct");

	return ret;
}

/*
 * action routine.
 * Call lower level routine
 */

int
deadr_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("deadr_action");

	ret = device_action(defill);

	EXITFUNC("deadr_action");

	return ret;
}


void
adr_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("adr_free");

	device_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("adr_free");
	return;
}

/*
 * validate the structure
 */

int
deadr_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret = 0;

	ENTERFUNC("deadr_valid");

	ret = device_valid(argv, defill);

	EXITFUNC("deadr_valid");
	return ret;
}

#define SETUPFUNC	deadr_setup		/* defined by stemplate.c */
#define AUTHFUNC	deadr_auth
#define BUILDFILLIN	deadr_bfill

#define INITFUNC	deadr_init		/* defined by stemplate.c */
#define BUILDSTRUCT	deadr_bstruct

#define ROUTFUNC	deadr_exit		/* defined by stemplate.c */
#define VALIDATE	deadr_valid
#define SCREENACTION	deadr_action

#define FREEFUNC	deadr_free		/* defined by stemplate.c */
#define FREESTRUCT	adr_free

#include "stemplate.c"

#endif /* SEC_BASE */
