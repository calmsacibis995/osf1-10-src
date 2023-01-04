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
static char	*sccsid = "@(#)$RCSfile: dermp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:39 $";
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
 * Routines to implement Remove Printer screen
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

static int dermp_auth();
static int dermp_bfill();
static int dermp_valid();
static int dermp_exit();
static int dermp_setup();
static int dermp_init();
static int dermp_free();

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	deptr_scrn
#define STRUCTTEMPLATE	dermp_struct
#define DESCTEMPLATE	deptr_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		dermp_fill

/* structures defined in de_scrns.c */

extern Scrn_parms	deptr_scrn;
extern Scrn_desc	deptr_desc[];

#define FIRSTDESC	NAME_DESC
#define NSCRNSTRUCT	PTR_NSCRNSTRUCT

static Scrn_struct	*dermp_struct;
static struct device_fillin de_buf, *dermp_fill = &de_buf;

/* called when the user has selected a printer and requests "Remove Device" */

int
dermp_doit()
{
	int i;

	deptr_scrn.setup = dermp_setup;
	deptr_scrn.init = dermp_init;
	deptr_scrn.free = dermp_free;

	for (i = 0; i < NSCRNSTRUCT; i++)
		deptr_scrn.ms[i].next_routine = dermp_exit;

	return traverse(&deptr_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Send the fillin structure and the mode to the lower level routine.
 */

int
dermp_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	int ret;

	ENTERFUNC("dermp_auth");

	ret = device_auth(defill, DeviceRemove);

	EXITFUNC("dermp_auth");

	return ret;
}

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
dermp_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("dermp_bfill");

	ret = deptr_bfill(defill);

	EXITFUNC("dermp_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
dermp_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("dermp_bstruct");

	ret = deptr_bstruct(defill, sptemplate);

	EXITFUNC("dermp_bstruct");

	return ret;
}

/*
 * action routine.
 * Call lower level routine
 */

int
dermp_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("dermp_action");

	ret = RemoveDevice(defill->device_name);

	/* if successfully remove the device, remove any current selection */

	if (ret == 0)
		DeselectDevice();

	EXITFUNC("dermp_action");

	return INTERRUPT;
}


void
rmp_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("rmp_free");

	device_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("rmp_free");
	return;
}

/*
 * validate the structure -- nothing to do because no input fields
 */

int
dermp_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret = 0;

	ENTERFUNC("dermp_valid");

	EXITFUNC("dermp_valid");

	return ret;
}

#define SETUPFUNC	dermp_setup		/* defined by stemplate.c */
#define AUTHFUNC	dermp_auth
#define BUILDFILLIN	dermp_bfill

#define INITFUNC	dermp_init		/* defined by stemplate.c */
#define BUILDSTRUCT	dermp_bstruct

#define ROUTFUNC	dermp_exit		/* defined by stemplate.c */
#define VALIDATE	dermp_valid
#define SCREENACTION	dermp_action

#define FREEFUNC	dermp_free		/* defined by stemplate.c */
#define FREESTRUCT	rmp_free

#include "stemplate.c"

#endif /* SEC_BASE */
