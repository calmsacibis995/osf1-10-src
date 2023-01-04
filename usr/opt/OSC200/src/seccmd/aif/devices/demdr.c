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
static char	*sccsid = "@(#)$RCSfile: demdr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:32 $";
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
 * Routines to implement Modify Removable screen
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

static int demdr_auth();
static int demdr_bfill();
static int demdr_valid();
static int demdr_exit();
static int demdr_setup();
static int demdr_init();
static int demdr_free();

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	dermv_scrn
#define STRUCTTEMPLATE	demdr_struct
#define DESCTEMPLATE	dermv_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		demdr_fill

/* structures defined in de_scrns.c */

extern Scrn_parms	dermv_scrn;
extern Scrn_desc	dermv_desc[];

#define FIRSTDESC	DEVLIST_DESC
#define NSCRNSTRUCT	RMV_NSCRNSTRUCT

static Scrn_struct	*demdr_struct;
static struct device_fillin de_buf, *demdr_fill = &de_buf;

/* called when a Removable is selected and the user specifies Modify */

demdr_doit()
{
	int i;

	dermv_scrn.setup = demdr_setup;
	dermv_scrn.init = demdr_init;
	dermv_scrn.free = demdr_free;

	for (i = 0; i < NSCRNSTRUCT; i++)
		dermv_scrn.ms[i].next_routine = demdr_exit;

	return traverse(&dermv_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Send the fillin structure and the mode to the lower level routine.
 */

int
demdr_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	int ret;
	int i;

	ENTERFUNC("demdr_auth");

	ret = device_auth(defill, DeviceUpdate);

	EXITFUNC("demdr_auth");

	return ret;
}

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
demdr_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("demdr_bfill");

	ret = dermv_bfill(defill);

	EXITFUNC("demdr_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
demdr_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("demdr_bstruct");

	ret = dermv_bstruct(defill, sptemplate);

	EXITFUNC("demdr_bstruct");

	return ret;
}

/*
 * action routine.
 * Call lower level routine
 */

int
demdr_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("demdr_action");

	ret = device_action(defill);

	EXITFUNC("demdr_action");

	return ret;
}


void
mdr_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("mdr_free");

	device_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("mdr_free");
	return;
}

/*
 * validate the structure
 */

int
demdr_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret = 0;

	ENTERFUNC("demdr_valid");

	ret = device_valid(argv, defill);

	EXITFUNC("demdr_valid");
	return ret;
}

#define SETUPFUNC	demdr_setup		/* defined by stemplate.c */
#define AUTHFUNC	demdr_auth
#define BUILDFILLIN	demdr_bfill

#define INITFUNC	demdr_init		/* defined by stemplate.c */
#define BUILDSTRUCT	demdr_bstruct

#define ROUTFUNC	demdr_exit		/* defined by stemplate.c */
#define VALIDATE	demdr_valid
#define SCREENACTION	demdr_action

#define FREEFUNC	demdr_free		/* defined by stemplate.c */
#define FREESTRUCT	mdr_free

#include "stemplate.c"

#endif /* SEC_BASE */
