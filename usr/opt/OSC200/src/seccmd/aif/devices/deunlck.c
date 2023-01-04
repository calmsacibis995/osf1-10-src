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
static char	*sccsid = "@(#)$RCSfile: deunlck.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:09 $";
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



/* screen to implement terminal unlock */

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

static int deunlck_auth();
static int deunlck_bfill();
static int deunlck_valid();
static int deunlck_exit();
static int deunlck_setup();
static int deunlck_init();
static int deunlck_free();

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	deunlck_scrn
#define STRUCTTEMPLATE	deunlck_struct
#define DESCTEMPLATE	deunlck_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		deunlck_fill

static DeviceFillin de_buf, *deunlck_fill = &de_buf;
static Scrn_struct *deunlck_struct;
static char dummy[2];

static Scrn_desc deunlck_desc[] = {
	{  0, 18, FLD_PROMPT,  0, FLD_OUTPUT, NO,
		"Press return to unlock terminal" },
	{  0, 51, FLD_ALPHA,   1, FLD_BOTH,  NO }
};

#define FIRSTDESC	1
#define NSCRNSTRUCT	1

static Menu_scrn deunlck_menu[] = {
	ROUT_DESC(1, deunlck_exit, 0, NULL),
};

static uchar deunlck_title[] = "UNLOCK TERMINAL";

static Scrn_hdrs deunlck_hdrs = {
	deunlck_title, cur_date, runner, cur_time,
		NULL, cur_dev, NULL,
		MT, NULL, cmds_line1
};

SKIP_PARMF(deunlck_scrn, SCR_FILLIN, deunlck_desc, deunlck_menu, NULL,
	&deunlck_hdrs,
	deunlck_setup, deunlck_init, deunlck_free);

deunlck_doit()
{
	if (!IsDeviceSelected) {
		pop_msg("You must select a terminal to lock.",
		  "Please select a terminal using 'Select Device'.");
		return 1;
	}
	if (DeviceType != TerminalDevice) {
		pop_msg("You may only lock a terminal device.",
		  "The device currently selected is not a terminal device.");
		return 1;
	}

	return traverse(&deunlck_scrn, 1);
}

static int
deunlck_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	ENTERFUNC("deunlck_auth");
	EXITFUNC("deunlck_auth");
	return 0;
}

static int
deunlck_bfill(defill)
	struct device_fillin *defill;
{
	int ret;
	ENTERFUNC("deunlck_bfill");
	ret = DevGetFillin(DevSelected, defill);
	EXITFUNC("deunlck_bfill");
	return ret;
}

static int
deunlck_bstruct(defill, sptemplate)
	struct device_fillin *defill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp = PARMTEMPLATE.ss;

	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	dummy[0] = ' ';
	sp[0].pointer = dummy;
	sp[0].filled = 1;
	sp[0].changed = 1;

	return 0;
}

static int
deunlck_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	ENTERFUNC("deunlck_valid");
	EXITFUNC("deunlck_valid");
	return 0;
}

static int
deunlck_action(defill)
	struct device_fillin *defill;
{
	ENTERFUNC("deunlck_action");
	DevUnlock(defill);
	EXITFUNC("deunlck_action");
	return INTERRUPT;
}

static void
unlck_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("unlck_free");

	DevFreeFillin(defill);

	EXITFUNC("unlck_free");
	return;
}

#define SETUPFUNC	deunlck_setup		/* defined by stemplate.c */
#define AUTHFUNC	deunlck_auth
#define BUILDFILLIN	deunlck_bfill

#define INITFUNC	deunlck_init		/* defined by stemplate.c */
#define BUILDSTRUCT	deunlck_bstruct

#define ROUTFUNC	deunlck_exit		/* defined by stemplate.c */
#define VALIDATE	deunlck_valid
#define SCREENACTION	deunlck_action

#define FREEFUNC	deunlck_free		/* defined by stemplate.c */
#define FREESTRUCT	unlck_free

#include "stemplate.c"

#endif /* SEC_BASE */
