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
static char	*sccsid = "@(#)$RCSfile: delck.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:56:25 $";
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



/* screen to implement terminal lock */

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

static int delck_auth();
static int delck_bfill();
static int delck_valid();
static int delck_exit();
static int delck_setup();
static int delck_init();
static int delck_free();

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	delck_scrn
#define STRUCTTEMPLATE	delck_struct
#define DESCTEMPLATE	delck_desc
#define FILLINSTRUCT	device_fillin
#define FILLIN		delck_fill

static DeviceFillin de_buf, *delck_fill = &de_buf;
static Scrn_struct *delck_struct;
static char dummy[2];

static Scrn_desc delck_desc[] = {
	{  0, 20, FLD_PROMPT,  0, FLD_OUTPUT, NO,
		"Press return to lock terminal" },
	{  0, 51, FLD_ALPHA,   1, FLD_BOTH,  NO }
};

#define FIRSTDESC	1
#define NSCRNSTRUCT	1

static Menu_scrn delck_menu[] = {
	ROUT_DESC(1, delck_exit, 0, NULL),
};

static uchar delck_title[] = "LOCK TERMINAL";

static Scrn_hdrs delck_hdrs = {
	delck_title, cur_date, runner, cur_time,
		NULL, cur_dev, NULL,
		MT, NULL, cmds_line1
};

SKIP_PARMF(delck_scrn, SCR_FILLIN, delck_desc, delck_menu, NULL, &delck_hdrs,
	delck_setup, delck_init, delck_free);

delck_doit()
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

	return traverse(&delck_scrn, 1);
}

static int
delck_auth(argv, defill)
	char **argv;
	struct def_device_fillin *defill;
{
	ENTERFUNC("delck_auth");
	EXITFUNC("delck_auth");
	return 0;
}

static int
delck_bfill(defill)
	struct device_fillin *defill;
{
	int ret;
	ENTERFUNC("delck_bfill");
	ret = DevGetFillin(DevSelected, defill);
	EXITFUNC("delck_bfill");
	return ret;
}

static int
delck_bstruct(defill, sptemplate)
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
delck_valid(argv, defill)
	char **argv;
	struct device_fillin *defill;
{
	ENTERFUNC("delck_valid");
	EXITFUNC("delck_valid");
	return 0;
}

static int
delck_action(defill)
	struct device_fillin *defill;
{
	ENTERFUNC("delck_action");
	DevLock(defill);
	EXITFUNC("delck_action");
	return INTERRUPT;
}

static void
lck_free(argv, defill, nstructs, pp, dp, sp)
	char **argv;
	struct device_fillin *defill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("lck_free");

	DevFreeFillin(defill);

	EXITFUNC("lck_free");
	return;
}

#define SETUPFUNC	delck_setup		/* defined by stemplate.c */
#define AUTHFUNC	delck_auth
#define BUILDFILLIN	delck_bfill

#define INITFUNC	delck_init		/* defined by stemplate.c */
#define BUILDSTRUCT	delck_bstruct

#define ROUTFUNC	delck_exit		/* defined by stemplate.c */
#define VALIDATE	delck_valid
#define SCREENACTION	delck_action

#define FREEFUNC	delck_free		/* defined by stemplate.c */
#define FREESTRUCT	lck_free

#include "stemplate.c"

#endif /* SEC_BASE */
