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
static char	*sccsid = "@(#)$RCSfile: dedsr.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:19 $";
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
 * Routines to implement Display Removable screen
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

static int dedsr_auth();
static int dedsr_bfill();
static int dedsr_valid();
static int dedsr_exit();
static int dedsr_setup();
static int dedsr_init();
static int dedsr_free();

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	dermv_scrn
#define STRUCTTEMPLATE	dedsr_struct
#define DESCTEMPLATE	dermv_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		dedsr_fill

/* structures defined in de_scrns.c */

extern Scrn_parms	dermv_scrn;
extern Scrn_desc	dermv_desc[];

#define FIRSTDESC	NAME_DESC
#define NSCRNSTRUCT	RMV_NSCRNSTRUCT

static Scrn_struct	*dedsr_struct;
static struct device_fillin de_buf, *dedsr_fill = &de_buf;

/*
 * called from the top-level screen when the user selects a removable
 * device and requests "display"
 */

int
dedsr_doit()
{
	int i;

	dermv_scrn.setup = dedsr_setup;
	dermv_scrn.init = dedsr_init;
	dermv_scrn.free = dedsr_free;

	for (i = 0; i < NSCRNSTRUCT; i++)
		dermv_scrn.ms[i].next_routine = dedsr_exit;

	return traverse(&dermv_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Send the fillin structure and the mode to the lower level routine.
 */

int
dedsr_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	int ret;

	ENTERFUNC("dedsr_auth");

	ret = device_auth(defill, DeviceDisplay);

	EXITFUNC("dedsr_auth");

	return ret;
}

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
dedsr_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("dedsr_bfill");

	ret = dermv_bfill(defill);

	EXITFUNC("dedsr_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
dedsr_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("dedsr_bstruct");

	ret = dermv_bstruct(defill, sptemplate);

	EXITFUNC("dedsr_bstruct");

	return ret;
}

/*
 * action routine.
 * Call lower level routine
 */

int
dedsr_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("dedsr_action");

	ret = device_action(defill);

	EXITFUNC("dedsr_action");

	return ret;
}


void
dsr_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("dsr_free");

	device_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("dsr_free");
	return;
}

/*
 * validate the structure
 */

int
dedsr_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret = 0;

	ENTERFUNC("dedsr_valid");

	ret = device_valid(argv, defill);

	EXITFUNC("dedsr_valid");
	return ret;
}

#define SETUPFUNC	dedsr_setup		/* defined by stemplate.c */
#define AUTHFUNC	dedsr_auth
#define BUILDFILLIN	dedsr_bfill

#define INITFUNC	dedsr_init		/* defined by stemplate.c */
#define BUILDSTRUCT	dedsr_bstruct

#define ROUTFUNC	dedsr_exit		/* defined by stemplate.c */
#define VALIDATE	dedsr_valid
#define SCREENACTION	dedsr_action

#define FREEFUNC	dedsr_free		/* defined by stemplate.c */
#define FREESTRUCT	dsr_free

#include "stemplate.c"

#endif /* SEC_BASE */
