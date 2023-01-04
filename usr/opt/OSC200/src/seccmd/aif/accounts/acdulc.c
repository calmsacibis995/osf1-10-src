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
static char	*sccsid = "@(#)$RCSfile: acdulc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:16 $";
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

/*
 *	acdulc.c - accounts login control screen routines
 */

#include <stdio.h>
#include <ctype.h>
#include <sys/security.h>
#include <prot.h>
#include "userif.h"
#include "valid.h"
#include "logging.h"
#include "UIMain.h"
#include "Utils.h"
#include "Accounts.h"

/*
 * Static routine declarations
 */

int acdulc_setup(), acdulc_init(), acdulc_free(), acdulc_exit();

/*
 * Static global variables
 */

extern Scrn_parms acdulc_scrn;
extern Scrn_desc  acdulc_desc[];
extern Menu_scrn  *acdulc_menu;
static Scrn_struct *acdulc_struct;
extern Scrn_hdrs acdulc_hdrs;

static
int max_act(), nice_act(), lock_act(), lode_act();

#define PARMTEMPLATE	acdulc_scrn
#define STRUCTTEMPLATE	acdulc_struct
#define DESCTEMPLATE	acdulc_desc
#define FILLINSTRUCT	acdulc_fillin
#define FILLIN		acdulcP

#define LOGINS		0	/* max unsucc. login tries */
#define LOG_BTN		1		/* defalt button */
#define LOG_DEF		2		/* default for above */
#define NICE		3	/* initial nice value */
#define NICE_BTN	4		/* defalt button */
#define NICE_DEF	5		/* default for above */
#define LOCK_BTN	6		/* defalt button */
#define LOCK		7	/* account locked to start with */
#define LODE_BTN	8		/* defalt button */
#define LOCK_DEF	9		/* default for above */

#define FIRSTDESC	1
#define NSCRNSTRUCT	10

/*
 * External declarations
 */


/*
 * Fillin structure for levels.
 */

struct acdulc_fillin {
	int max_tries;
	char max_btn;
	int max_def;
	int nice;
	char nice_btn;
	int nice_def;
	char lock_btn;
	char lock;
	char lode_btn;
	char lock_def;
	int f1, f2, f3, f4, f5, f6; /* flags for above */
	int nstructs;
} acdulc, *acdulcP = &acdulc;

static struct prpw_if pru;	/* prpw superstructure */

static struct pr_passwd *pd = &pru.prpw;	/* prpw */


/*
 * Fill in the scrn_struct
 */

static
int
acdulc_bstruct(lf, sptemplate)
	struct acdulc_fillin *lf;
	struct scrn_struct **sptemplate;
{
	register struct scrn_struct *sp;

	ENTERFUNC("acdulc_bstruct");
	lf->nstructs = NSCRNSTRUCT;
	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;
	if (sp) {
		sp[LOGINS].pointer = (char *) &lf->max_tries;
		sp[LOGINS].filled = lf->f1;
		sp[LOG_BTN].pointer = (char *) &lf->max_btn;
		sp[LOG_BTN].filled = lf->f1;
		sp[LOG_BTN].val_act = max_act;
		sp[LOG_DEF].pointer = (char *) &lf->max_def;
		sp[LOG_DEF].filled = lf->f2;

		sp[NICE].pointer = (char *) &lf->nice;
		sp[NICE].filled = lf->f3;
		sp[NICE_BTN].pointer = (char *) &lf->nice_btn;
		sp[NICE_BTN].filled = lf->f3;
		sp[NICE_BTN].val_act = nice_act;
		sp[NICE_DEF].pointer = (char *) &lf->nice_def;
		sp[NICE_DEF].filled = lf->f5;

		sp[LOCK_BTN].pointer = (char *) &lf->lock_btn;
		sp[LOCK_BTN].filled = lf->f4;
		sp[LOCK_BTN].val_act = lock_act;
		sp[LOCK].pointer = &lf->lock;
		sp[LOCK].filled = lf->f5;
		sp[LODE_BTN].pointer = (char *) &lf->lode_btn;
		sp[LODE_BTN].filled = lf->f6;
		sp[LODE_BTN].val_act = lode_act;
		sp[LOCK_DEF].pointer = &lf->lock_def;
		sp[LOCK_DEF].filled = lf->f6;
	}
	EXITFUNC("acdulc_bstruct");
	return 0;
}


static
int max_act()
{
	register struct scrn_struct *sp;

	sp = PARMTEMPLATE.ss;
	if (FILLIN->max_btn) {
		FILLIN->max_tries = FILLIN->max_def;
		sp[LOGINS].changed = 1;
	}
}



static
int nice_act()
{
	register struct scrn_struct *sp;

	sp = PARMTEMPLATE.ss;
	if (FILLIN->nice_btn) {
		FILLIN->nice = FILLIN->nice_def;
		sp[NICE].changed = 1;
	}
}



static
int lock_act()
{
	register struct scrn_struct *sp;

	sp = PARMTEMPLATE.ss;
	FILLIN->lock = FILLIN->lock_btn;
	sp[LOCK].changed = 1;
	FILLIN->lode_btn = 0;
	sp[LODE_BTN].changed = 1;
}



static
int lode_act()
{
	register struct scrn_struct *sp;

	sp = PARMTEMPLATE.ss;
	if (FILLIN->lode_btn) {
		FILLIN->lock = FILLIN->lock_def;
		sp[LOCK_DEF].changed = 1;
		FILLIN->lock_btn = FILLIN->lock;
		sp[LOCK_BTN].changed = 1;
	}
}




/*
 * Build the fillin structure.
 * The requirements for this routine were completed at the top level.
 */

static
int
acdulc_bfill(lf)
	struct acdulc_fillin *lf;
{
	int ret;

	ENTERFUNC("acdulc_bfill");

	if ((ret = XGetUserInfo (gl_user, &pru)) == SUCCESS) {
		lf->f1 = pd->uflg.fg_max_tries;
		lf->max_tries = lf->f1 ? pd->ufld.fd_max_tries : 0;
		lf->max_btn = lf->max_tries;

		lf->f2 = pd->sflg.fg_max_tries;
		lf->max_def = lf->f2 ? pd->sfld.fd_max_tries : 0;

		lf->f3 = pd->uflg.fg_nice;
		lf->nice = lf->f3 ? pd->ufld.fd_nice : 0;
		lf->nice_btn = lf->nice;

		lf->f4 = pd->sflg.fg_nice;
		lf->nice_def = lf->f4 ? pd->sfld.fd_nice : 0;

		lf->f5 = pd->uflg.fg_lock;
		lf->lock = lf->f5 ? pd->ufld.fd_lock : 0;
		lf->lock_btn = lf->lock;

		lf->f6 = pd->sflg.fg_lock;
		lf->lock_def = lf->f6 ? pd->sfld.fd_lock : 0;
		lf->lode_btn = lf->lock_def;
	}
	EXITFUNC("acdulc_bfill");
	return ret;
}

/*
 * Decide whether the user is authorized to perform the function.
 * Nothing to decide here.
 */

static
int
acdulc_auth(argv, lf)
	char **argv;
	struct acdulc_fillin *lf;
{
	return 0;
}

/*
 * validate function for the whole screen.
 */

static
int
acdulc_valid(argv, lf)
	char **argv;
	struct acdulc_fillin *lf;
{
	int ret = 0;

	ENTERFUNC("acdulc_valid");
	EXITFUNC("acdulc_valid");
	return ret;
}

/*
 * action function for the whole screen.
 */

static int
acdulc_action(lf)
	struct acdulc_fillin *lf;
{
	ENTERFUNC("acdulc_valid");
	EXITFUNC("acdulc_valid");
	return CONTINUE;
}

/*
 * routine to free memory.
 * since clearance levels may be re-used, keep memory allocated.
 */

static
int
acdulc_do_free(argv, af, nstructs, pp, dp, sp)
char **argv;
struct FILLINSTRUCT *af;
int nstructs;
Scrn_parms *pp;
Scrn_desc *dp;
Scrn_struct *sp;
{
	ENTERFUNC("acdulc_do_free");
	EXITFUNC("acdulc_do_free");
	return 1;
}

#define SETUPFUNC	acdulc_setup	/* defined by stemplate.c */
#define AUTHFUNC	acdulc_auth
#define BUILDFILLIN	acdulc_bfill

#define INITFUNC	acdulc_init		/* defined by stemplate.c */
#define BUILDSTRUCT	acdulc_bstruct

#define ROUTFUNC	acdulc_exit		/* defined by stemplate.c */
#define VALIDATE	acdulc_valid
#define SCREENACTION	acdulc_action

#define FREEFUNC	acdulc_free		/* defined by stemplate.c */
#define FREESTRUCT	acdulc_do_free

#include "stemplate.c"

#endif /*} SEC_BASE */
