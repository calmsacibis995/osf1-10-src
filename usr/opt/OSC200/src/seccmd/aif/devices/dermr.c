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
static char	*sccsid = "@(#)$RCSfile: dermr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:42 $";
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
 * Routines to implement Remove Removable screen
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

static int dermr_auth();
static int dermr_bfill();
static int dermr_valid();
static int dermr_exit();
static int dermr_setup();
static int dermr_init();
static int dermr_free();

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	dermv_scrn
#define STRUCTTEMPLATE	dermr_struct
#define DESCTEMPLATE	dermv_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		dermr_fill

/* structures defined in de_scrns.c */

extern Scrn_parms	dermv_scrn;
extern Scrn_desc	dermv_desc[];

#define FIRSTDESC	NAME_DESC
#define NSCRNSTRUCT	RMV_NSCRNSTRUCT

static Scrn_struct	*dermr_struct;
static struct device_fillin de_buf, *dermr_fill = &de_buf;

/* called when the user has selected a Removable and requests Remove */

int
dermr_doit()
{
	int i;

	dermv_scrn.setup = dermr_setup;
	dermv_scrn.init = dermr_init;
	dermv_scrn.free = dermr_free;

	for (i = 0; i < NSCRNSTRUCT; i++)
		dermv_scrn.ms[i].next_routine = dermr_exit;

	return traverse(&dermv_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Send the fillin structure and the mode to the lower level routine.
 */

int
dermr_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	int ret;

	ENTERFUNC("dermr_auth");

	ret = device_auth(defill, DeviceRemove);

	EXITFUNC("dermr_auth");

	return ret;
}

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
dermr_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("dermr_bfill");

	ret = dermv_bfill(defill);

	EXITFUNC("dermr_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
dermr_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("dermr_bstruct");

	ret = dermv_bstruct(defill, sptemplate);

	EXITFUNC("dermr_bstruct");

	return ret;
}

/*
 * action routine.
 * Call lower level routine
 */

int
dermr_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("dermr_action");

	ret = RemoveDevice(defill->device_name);

	/* if successfully remove the device, remove any current selection */

	if (ret == 0)
		DeselectDevice();

	EXITFUNC("dermr_action");

	return INTERRUPT;
}


void
rmr_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("rmr_free");

	device_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("rmr_free");
	return;
}

/*
 * validate the structure -- nothing to do because no input fields
 */

int
dermr_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret = 0;

	ENTERFUNC("dermr_valid");

	EXITFUNC("dermr_valid");

	return ret;
}

#define SETUPFUNC	dermr_setup		/* defined by stemplate.c */
#define AUTHFUNC	dermr_auth
#define BUILDFILLIN	dermr_bfill

#define INITFUNC	dermr_init		/* defined by stemplate.c */
#define BUILDSTRUCT	dermr_bstruct

#define ROUTFUNC	dermr_exit		/* defined by stemplate.c */
#define VALIDATE	dermr_valid
#define SCREENACTION	dermr_action

#define FREEFUNC	dermr_free		/* defined by stemplate.c */
#define FREESTRUCT	rmr_free

#include "stemplate.c"

#endif /* SEC_BASE */
