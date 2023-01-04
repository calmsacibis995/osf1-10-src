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
static char	*sccsid = "@(#)$RCSfile: acdlc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:03 $";
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
 *  Copyright (c) 1990 SecureWare, Inc.
 *   All rights reserved
 */



#include <sys/secdefines.h>

#if SEC_BASE /*{*/

#include <stdio.h>
#include <ctype.h>
#include <sys/security.h>
#include <prot.h>
#include <mandatory.h>
#include "userif.h"
#include "valid.h"
#include "logging.h"
#include "UIMain.h"
#include "Accounts.h"

/*
 * Static routine declarations
 */

int acdlc_setup(), acdlc_init(), acdlc_free(), acdlc_exit();

/*
 * Static global variables
 */

extern Scrn_parms acdlc_scrn;
extern Scrn_desc  acdlc_desc[];
extern Menu_scrn  *acdlc_menu;
static Scrn_struct *acdlc_struct;
extern Scrn_hdrs acdlc_hdrs;

#define PARMTEMPLATE	acdlc_scrn
#define STRUCTTEMPLATE	acdlc_struct
#define DESCTEMPLATE	acdlc_desc
#define FILLINSTRUCT	acdlc_fillin
#define FILLIN		acdlcP

#define LOGINS		0
#define INACTIVITY	1
#define NICE		2
#define AUTHENTICATE	3
#define LOCK		4

#define FIRSTDESC	1
#define NSCRNSTRUCT	5

/*
 * External declarations
 */

extern char *Malloc(), *Calloc();

/*
 * Fillin structure for levels.
 */

static struct acdlc_fillin {
	int max_tries;
	int inactivity_timeout;
	int nice;
	char boot_authenticate;
	char lock;
	int f1, f2, f3, f4, f5; /* flags for above */
	int nstructs;
} acdlc, *acdlcP = &acdlc;

static struct sdef_if sds;	/* system defaults superstructure */

static struct pr_default *sd = &sds.df;	/* system defaults */


/*
 * Fill in the scrn_struct
 */

static int
acdlc_bstruct(lf, sptemplate)
	struct acdlc_fillin *lf;
	struct scrn_struct **sptemplate;
{
	register struct scrn_struct *sp;

	ENTERFUNC("acdlc_bstruct");
	lf->nstructs = NSCRNSTRUCT;
	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;
	if (sp) {
		sp[LOGINS].pointer = (char *) &lf->max_tries;
		sp[LOGINS].filled = lf->f1;
		sp[INACTIVITY].pointer = (char *) &lf->inactivity_timeout;
		sp[INACTIVITY].filled = lf->f2;
		sp[NICE].pointer = (char *) &lf->nice;
		sp[NICE].filled = lf->f3;
		sp[AUTHENTICATE].pointer = &lf->boot_authenticate;
		sp[AUTHENTICATE].filled = lf->f4;
		sp[LOCK].pointer = &lf->lock;
		sp[LOCK].filled = lf->f5;
	}
	EXITFUNC("acdlc_bstruct");
	return 0;
}


/*
 * Build the fillin structure.
 * The requirements for this routine were completed at the top level.
 */

static int
acdlc_bfill(lf)
	struct acdlc_fillin *lf;
{
	int ret;

	ENTERFUNC("acdlc_bfill");

	ret = XGetSystemInfo(&sds);

	if (ret == 0) {
		lf->f1 = sd->prg.fg_max_tries;
		lf->max_tries = lf->f1 ? sd->prd.fd_max_tries : 0;

		lf->f2 = sd->sflg.fg_inactivity_timeout;
		lf->inactivity_timeout = lf->f2 ?
			sd->sfld.fd_inactivity_timeout/60 : 0;

		lf->f3 = sd->prg.fg_nice;
		lf->nice = lf->f3 ? sd->prd.fd_nice : 0;

		lf->f4 = sd->sflg.fg_boot_authenticate;
		lf->boot_authenticate =
			lf->f4 ? sd->sfld.fd_boot_authenticate : 0;

		lf->f5 = sd->prg.fg_lock;
		lf->lock = lf->f5 ? sd->prd.fd_lock : 0;
	}
	EXITFUNC("acdlc_bfill");
	return ret;
}

/*
 * Decide whether the user is authorized to perform the function.
 * Nothing to decide here.
 */

static int
acdlc_auth(argv, lf)
	char **argv;
	struct acdlc_fillin *lf;
{
	return 0;
}

/*
 * validate function for the whole screen.
 */

static int
acdlc_valid(argv, lf)
	char **argv;
	struct acdlc_fillin *lf;
{
	int ret = 0;

	ENTERFUNC("acdlc_valid");
	EXITFUNC("acdlc_valid");
	return ret;
}

/*
 * action function for the whole screen.
 */

static int
acdlc_action(lf)
	struct acdlc_fillin *lf;
{
	register Scrn_struct *asp = acdlc_struct;

	ENTERFUNC("acdlc_valid");

	if (asp[LOGINS].filled || asp[LOGINS].changed)
		sd->prg.fg_max_tries  = 1;
	if (sd->prg.fg_max_tries)
		sd->prd.fd_max_tries = lf->max_tries;

	if (asp[INACTIVITY].filled || asp[INACTIVITY].changed)
			sd->sflg.fg_inactivity_timeout = 1;
	if (sd->sflg.fg_inactivity_timeout)
		sd->sfld.fd_inactivity_timeout = 
			lf->inactivity_timeout * 60;

	if (asp[NICE].filled || asp[NICE].changed)
		sd->prg.fg_nice = 1;
	if (sd->prg.fg_nice)
		sd->prd.fd_nice = lf->nice;

	if (asp[AUTHENTICATE].filled || asp[AUTHENTICATE].changed)
		sd->sflg.fg_boot_authenticate = 1;
	if (sd->sflg.fg_boot_authenticate)
		sd->sfld.fd_boot_authenticate = lf->boot_authenticate;

	if (asp[LOCK].filled || asp[LOCK].changed)
		sd->prg.fg_lock = 1;
	if (sd->prg.fg_lock)
		sd->prd.fd_lock = lf->lock;

	XWriteSystemInfo(&sds);

	EXITFUNC("acdlc_valid");
	return CONTINUE;
}

/*
 * routine to free memory.
 * since clearance levels may be re-used, keep memory allocated.
 */

static int
do_free()
{
	ENTERFUNC("do_free");
	EXITFUNC("do_free");
	return 1;
}

#define SETUPFUNC	acdlc_setup	/* defined by stemplate.c */
#define AUTHFUNC	acdlc_auth
#define BUILDFILLIN	acdlc_bfill

#define INITFUNC	acdlc_init		/* defined by stemplate.c */
#define BUILDSTRUCT	acdlc_bstruct

#define ROUTFUNC	acdlc_exit		/* defined by stemplate.c */
#define VALIDATE	acdlc_valid
#define SCREENACTION	acdlc_action

#define FREEFUNC	acdlc_free		/* defined by stemplate.c */
#define FREESTRUCT	do_free

#include "stemplate.c"

#endif /*} SEC_BASE */
