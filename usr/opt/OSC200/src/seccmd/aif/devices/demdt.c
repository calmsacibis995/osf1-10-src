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
static char	*sccsid = "@(#)$RCSfile: demdt.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:36 $";
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
 * Routines to implement Modify Terminal screen
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

static int demdt_auth();
static int demdt_bfill();
static int demdt_valid();
static int demdt_exit();
static int demdt_setup();
static int demdt_init();
static int demdt_free();

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	detrm_scrn
#define STRUCTTEMPLATE	demdt_struct
#define DESCTEMPLATE	detrm_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		demdt_fill

/* structures defined in de_scrns.c */

extern Scrn_parms	detrm_scrn;
extern Scrn_desc	detrm_desc[];

#define FIRSTDESC	DEVLIST_DESC
#define NSCRNSTRUCT	TRM_NSCRNSTRUCT

static Scrn_struct	*demdt_struct;
static struct device_fillin de_buf, *demdt_fill = &de_buf;

/* called when a Terminal was selected and the user specifies Modify */

int
demdt_doit()
{
	int i;

	detrm_scrn.setup = demdt_setup;
	detrm_scrn.init = demdt_init;
	detrm_scrn.free = demdt_free;

	for (i = 0; i < NSCRNSTRUCT; i++)
		detrm_scrn.ms[i].next_routine = demdt_exit;

	return traverse(&detrm_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Send the fillin structure and the mode to the lower level routine.
 */

int
demdt_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	int ret;

	ENTERFUNC("demdt_auth");

	ret = device_auth(defill, DeviceUpdate);

	EXITFUNC("demdt_auth");

	return ret;
}

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
demdt_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("demdt_bfill");

	ret = detrm_bfill(defill);

	EXITFUNC("demdt_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
demdt_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("demdt_bstruct");

	ret = detrm_bstruct(defill, sptemplate);

	EXITFUNC("demdt_bstruct");

	return ret;
}

/*
 * action routine.
 * Call lower level routine
 */

int
demdt_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("demdt_action");

	ret = device_action(defill);

	EXITFUNC("demdt_action");

	return ret;
}


void
mdt_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("mdt_free");

	device_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("mdt_free");
	return;
}

/*
 * validate the structure
 */

int
demdt_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret = 0;

	ENTERFUNC("demdt_valid");

	ret = device_valid(argv, defill);

	EXITFUNC("demdt_valid");
	return ret;
}

#define SETUPFUNC	demdt_setup		/* defined by stemplate.c */
#define AUTHFUNC	demdt_auth
#define BUILDFILLIN	demdt_bfill

#define INITFUNC	demdt_init		/* defined by stemplate.c */
#define BUILDSTRUCT	demdt_bstruct

#define ROUTFUNC	demdt_exit		/* defined by stemplate.c */
#define VALIDATE	demdt_valid
#define SCREENACTION	demdt_action

#define FREEFUNC	demdt_free		/* defined by stemplate.c */
#define FREESTRUCT	mdt_free

#include "stemplate.c"

#endif /* SEC_BASE */
