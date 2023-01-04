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
static char	*sccsid = "@(#)$RCSfile: dedsp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:16 $";
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
 * Routines to implement Display Printer screen
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

/* static routine definitions */

static int dedsp_auth();
static int dedsp_bfill();
static int dedsp_valid();
static int dedsp_exit();
static int dedsp_setup();
static int dedsp_init();
static int dedsp_free();

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	deptr_scrn
#define STRUCTTEMPLATE	dedsp_struct
#define DESCTEMPLATE	deptr_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		dedsp_fill

/* structures defined in de_scrns.c */

extern Scrn_parms	deptr_scrn;
extern Scrn_desc	deptr_desc[];

#define PTR_TITLE_DESC		0
#define PTR_NAME_DESC		1
#define PTR_DEVLIST_TITLE_DESC	2
#define PTR_DEVLIST_DESC	3
#if SEC_MAC
#define PTR_DEVLABEL_TITLE_DESC	4
#define PTR_SL_TITLE_DESC	5
#define PTR_DEVLABEL_UNDER_DESC	6
#define PTR_SL_UNDER_DESC	7
#define PTR_SINGLE_TITLE_DESC	8
#define PTR_SINGLE_SL_DESC	9
#define PTR_MULTI_TITLE_DESC	10
#define PTR_MULTI_SL_DESC	11
#endif /* SEC_MAC */

#define FIRSTDESC	PTR_NAME_DESC

#define PTR_NAME_STRUCT		0
#define PTR_DEVLIST_STRUCT	1
#if SEC_MAC
#define PTR_SINGLE_SL_STRUCT	2
#define PTR_MULTI_SL_STRUCT	3
#define NSCRNSTRUCT		4
#else /* SEC_MAC */
#define NSCRNSTRUCT		2
#endif /* SEC_MAC */

static Scrn_struct	*dedsp_struct;
static struct device_fillin de_buf, *dedsp_fill = &de_buf;

/*
 * Called from the top-level screen when a printer device is selected
 * and the user selects 'Display Device'
 */

int
dedsp_doit()
{
	int i;

	deptr_scrn.setup = dedsp_setup;
	deptr_scrn.init = dedsp_init;
	deptr_scrn.free = dedsp_free;

	for (i = 0; i < NSCRNSTRUCT; i++)
		deptr_scrn.ms[i].next_routine = dedsp_exit;

	return traverse(&deptr_scrn, FIRSTDESC);
}

/*
 * Initialization for this screen.
 * Send the fillin structure and the mode to the lower level routine.
 */

int
dedsp_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	int ret;

	ENTERFUNC("dedsp_auth");

	ret = device_auth(defill, DeviceDisplay);

	EXITFUNC("dedsp_auth");

	return ret;
}

/*
 * To build the fillin structure, grab the device entry for the printer.
 * Set up the desc table for the display appropriate for the screen mode.
 */

int
dedsp_bfill(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("dedsp_bfill");

	ret = deptr_bfill(defill);

	EXITFUNC("dedsp_bfill");

	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

int
dedsp_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	int ret;

	ENTERFUNC("dedsp_bstruct");

	ret = deptr_bstruct(defill, sptemplate);

	EXITFUNC("dedsp_bstruct");

	return ret;
}

/*
 * action routine.
 * Call lower level routine
 */

int
dedsp_action(defill)
	struct device_fillin *defill;
{
	int ret;

	ENTERFUNC("dedsp_action");

	ret = device_action(defill);

	EXITFUNC("dedsp_action");

	return ret;
}


void
dsp_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("dsp_free");

	device_free(argv, defill, nstructs, pp, dp, sp);

	EXITFUNC("dsp_free");
	return;
}

/*
 * validate the structure
 */

int
dedsp_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	int ret = 0;

	ENTERFUNC("dedsp_valid");

	ret = device_valid(argv, defill);

	EXITFUNC("dedsp_valid");
	return ret;
}

#define SETUPFUNC	dedsp_setup		/* defined by stemplate.c */
#define AUTHFUNC	dedsp_auth
#define BUILDFILLIN	dedsp_bfill

#define INITFUNC	dedsp_init		/* defined by stemplate.c */
#define BUILDSTRUCT	dedsp_bstruct

#define ROUTFUNC	dedsp_exit		/* defined by stemplate.c */
#define VALIDATE	dedsp_valid
#define SCREENACTION	dedsp_action

#define FREEFUNC	dedsp_free		/* defined by stemplate.c */
#define FREESTRUCT	dsp_free

#include "stemplate.c"

#endif /* SEC_BASE */
