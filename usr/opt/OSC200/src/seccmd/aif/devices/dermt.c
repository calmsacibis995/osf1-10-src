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
static char	*sccsid = "@(#)$RCSfile: dermt.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:46 $";
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
 * Routines to implement Remove Terminal screen
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

static int dermt_auth();
static int dermt_bfill();
static int dermt_valid();
static int dermt_exit();
static int dermt_setup();
static int dermt_init();
static int dermt_free();

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	detrm_scrn
#define STRUCTTEMPLATE	dermt_struct
#define DESCTEMPLATE	detrm_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		dermt_fill

/* structures defined in de_scrns.c */

extern Scrn_parms	detrm_scrn;
extern Scrn_desc	detrm_desc[];

#define FIRSTDESC	NAME_DESC
#define NSCRNSTRUCT	TRM_NSCRNSTRUCT

static Scrn_struct	*dermt_struct;
static struct device_fillin de_buf, *dermt_fill = &de_buf;

int
dermt_doit()
{
	int i;

	detrm_scrn.setup = dermt_setup;
	detrm_scrn.init = dermt_init;
	detrm_scrn.free = dermt_free;

	for (i = 0; i < NSCRNSTRUCT; i++)
		detrm_scrn.ms[i].next_routine = dermt_exit;

	return traverse(&detrm_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Send the fillin structure and the mode to the lower level routine.
 */

int
dermt_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	int ret;

	ENTERFUNC("dermt_auth");

	ret = device_auth(defill, DeviceRemove);

	EXITFUNC("dermt_auth");

	return ret;
}

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
dermt_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("dermt_bfill");

	ret = detrm_bfill(defill);

	EXITFUNC("dermt_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
dermt_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("dermt_bstruct");

	ret = detrm_bstruct(defill, sptemplate);

	EXITFUNC("dermt_bstruct");

	return ret;
}

/*
 * action routine.
 * Call lower level routine
 */

int
dermt_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("dermt_action");

	ret = RemoveDevice(defill->device_name);

	/* if successfully remove the device, remove any current selection */

	if (ret == 0)
		DeselectDevice();

	EXITFUNC("dermt_action");

	return INTERRUPT;
}


void
rmt_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("rmt_free");

	device_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("rmt_free");
	return;
}

/*
 * validate the structure -- nothing to do because no input fields
 */

int
dermt_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret = 0;

	ENTERFUNC("dermt_valid");

	EXITFUNC("dermt_valid");

	return ret;
}

#define SETUPFUNC	dermt_setup		/* defined by stemplate.c */
#define AUTHFUNC	dermt_auth
#define BUILDFILLIN	dermt_bfill

#define INITFUNC	dermt_init		/* defined by stemplate.c */
#define BUILDSTRUCT	dermt_bstruct

#define ROUTFUNC	dermt_exit		/* defined by stemplate.c */
#define VALIDATE	dermt_valid
#define SCREENACTION	dermt_action

#define FREEFUNC	dermt_free		/* defined by stemplate.c */
#define FREESTRUCT	rmt_free

#include "stemplate.c"

#endif /* SEC_BASE */
