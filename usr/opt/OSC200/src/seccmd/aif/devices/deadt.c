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
static char	*sccsid = "@(#)$RCSfile: deadt.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:59 $";
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
 * Routines to implement Add Terminal screen
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

static int deadt_auth();
static int deadt_bfill();
static int deadt_valid();
static int deadt_exit();
static int deadt_setup();
static int deadt_init();
static int deadt_free();

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	detrm_scrn
#define STRUCTTEMPLATE	deadt_struct
#define DESCTEMPLATE	detrm_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		deadt_fill

/* structures defined in de_scrns.c */

extern Scrn_parms	detrm_scrn;
extern Scrn_desc	detrm_desc[];

#define FIRSTDESC	NAME_DESC
#define NSCRNSTRUCT	TRM_NSCRNSTRUCT

static Scrn_struct	*deadt_struct;
static struct device_fillin de_buf, *deadt_fill = &de_buf;

/* called when a terminal is selected and the user specifies Add */

int
deadt_doit()
{
	int i;

	detrm_scrn.setup = deadt_setup;
	detrm_scrn.init = deadt_init;
	detrm_scrn.free = deadt_free;

	for (i = 0; i < NSCRNSTRUCT; i++)
		detrm_scrn.ms[i].next_routine = deadt_exit;

	return traverse(&detrm_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Send the fillin structure and the mode to the lower level routine.
 */

int
deadt_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	int ret;

	ENTERFUNC("deadt_auth");

	ret = device_auth(defill, DeviceCreate);

	EXITFUNC("deadt_auth");

	return ret;
}

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
deadt_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("deadt_bfill");

	ret = detrm_bfill(defill);

	EXITFUNC("deadt_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
deadt_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("deadt_bstruct");

	ret = detrm_bstruct(defill, sptemplate);

	EXITFUNC("deadt_bstruct");

	return ret;
}

/*
 * action routine.
 * Call lower level routine
 */

int
deadt_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("deadt_action");

	ret = device_action(defill);

	EXITFUNC("deadt_action");

	return ret;
}


void
adt_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("adt_free");

	device_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("adt_free");
	return;
}

/*
 * validate the structure
 */

int
deadt_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret = 0;

	ENTERFUNC("deadt_valid");

	ret = device_valid(argv, defill);

	EXITFUNC("deadt_valid");
	return ret;
}

#define SETUPFUNC	deadt_setup		/* defined by stemplate.c */
#define AUTHFUNC	deadt_auth
#define BUILDFILLIN	deadt_bfill

#define INITFUNC	deadt_init		/* defined by stemplate.c */
#define BUILDSTRUCT	deadt_bstruct

#define ROUTFUNC	deadt_exit		/* defined by stemplate.c */
#define VALIDATE	deadt_valid
#define SCREENACTION	deadt_action

#define FREEFUNC	deadt_free		/* defined by stemplate.c */
#define FREESTRUCT	adt_free

#include "stemplate.c"

#endif /* SEC_BASE */
