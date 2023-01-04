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
static char	*sccsid = "@(#)$RCSfile: dedst.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:22 $";
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
 * Routines to implement Display Terminal screen
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

static int dedst_auth();
static int dedst_bfill();
static int dedst_valid();
static int dedst_exit();
static int dedst_setup();
static int dedst_init();
static int dedst_free();

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	detrm_scrn
#define STRUCTTEMPLATE	dedst_struct
#define DESCTEMPLATE	detrm_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		dedst_fill

/* structures defined in de_scrns.c */

extern Scrn_parms	detrm_scrn;
extern Scrn_desc	detrm_desc[];

#define FIRSTDESC	NAME_DESC
#define NSCRNSTRUCT	TRM_NSCRNSTRUCT

static Scrn_struct	*dedst_struct;
static struct device_fillin de_buf, *dedst_fill = &de_buf;

/* Called when the user has selected a terminal and requests display */

int
dedst_doit()
{
	int i;

	detrm_scrn.setup = dedst_setup;
	detrm_scrn.init = dedst_init;
	detrm_scrn.free = dedst_free;

	for (i = 0; i < NSCRNSTRUCT; i++)
		detrm_scrn.ms[i].next_routine = dedst_exit;

	return traverse(&detrm_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Send the fillin structure and the mode to the lower level routine.
 */

int
dedst_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	int ret;

	ENTERFUNC("dedst_auth");

	ret = device_auth(defill, DeviceDisplay);

	EXITFUNC("dedst_auth");

	return ret;
}

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
dedst_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("dedst_bfill");

	ret = detrm_bfill(defill);

	EXITFUNC("dedst_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
dedst_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("dedst_bstruct");

	ret = detrm_bstruct(defill, sptemplate);

	EXITFUNC("dedst_bstruct");

	return ret;
}

/*
 * action routine.
 * Call lower level routine
 */

int
dedst_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("dedst_action");

	ret = device_action(defill);

	EXITFUNC("dedst_action");

	return ret;
}


void
dst_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("dst_free");

	device_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("dst_free");
	return;
}

/*
 * validate the structure
 */

int
dedst_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret = 0;

	ENTERFUNC("dedst_valid");

	ret = device_valid(argv, defill);

	EXITFUNC("dedst_valid");
	return ret;
}

#define SETUPFUNC	dedst_setup		/* defined by stemplate.c */
#define AUTHFUNC	dedst_auth
#define BUILDFILLIN	dedst_bfill

#define INITFUNC	dedst_init		/* defined by stemplate.c */
#define BUILDSTRUCT	dedst_bstruct

#define ROUTFUNC	dedst_exit		/* defined by stemplate.c */
#define VALIDATE	dedst_valid
#define SCREENACTION	dedst_action

#define FREEFUNC	dedst_free		/* defined by stemplate.c */
#define FREESTRUCT	dst_free

#include "stemplate.c"

#endif /* SEC_BASE */
