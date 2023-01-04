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
static char	*sccsid = "@(#)$RCSfile: acmupp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:32 $";
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
/*  Copyright (c) 1988, 1990 SecureWare, Inc.
 *   All rights reserved
 */



#include <sys/secdefines.h>

/*
 *	acmupp.c - modify account default password parameters
 */


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

#define HOUR_SEC	3600
#define DAY_SEC		(24 * HOUR_SEC)
#define WEEK_SEC	(7 * DAY_SEC)
/*
 * action routine declarations
*/
int acmupp_setup(), acmupp_init(), acmupp_free(), acmupp_exit();

/*
 * Static global variables
 */

extern Scrn_parms acmupp_scrn;
extern Scrn_desc  acmupp_desc[];
extern Menu_scrn  *acmupp_menu;
Scrn_struct *acmupp_struct;
extern Scrn_hdrs acmupp_hdrs;

#define PARMTEMPLATE	acmupp_scrn
#define STRUCTTEMPLATE	acmupp_struct
#define DESCTEMPLATE	acmupp_desc
#define FILLINSTRUCT	acmupp_fillin
#define FILLIN		acmuppP

#define Z_MIN		0
#define Z_EXPIRE	1
#define Z_LIFETIME	2
#define Z_MAXLEN	3
#define Z_NULLPW	4
#define Z_PICKPWD	5
#define Z_GENPWD	6
#define Z_GENCHARS	7
#define Z_GENLETTERS	8
#define Z_RESTRICT	9

#define FIRSTDESC	1

#define NSCRNSTRUCT	10


/*
 * Fillin structure - most fields named for pr_default fields
 */

struct acmupp_fillin {
	int min;
	int expire;
	int lifetime;
	int maxlen;
	char nullpw;
	char pick_pwd;
	char gen_pwd;		/* pronounceable password generator */
	char gen_chars;		/* character password generator */
	char gen_letters;	/* letter password generator */
	char restrict;
	char f1, f2, f3, f4, f5, f6, f7, f8, f9, f10; /* flags for above */
	int nstructs;
} acmupp, *acmuppP = &acmupp;

static struct prpw_if pru;	/* prpw superstructure */

static struct pr_passwd *pd = &pru.prpw;	/* prpw */

/*
 * Fill in the scrn_struct
 */

int
acmupp_bstruct(lf, sptemplate)
	struct acmupp_fillin *lf;
	struct scrn_struct **sptemplate;
{
	register struct scrn_struct *sp;

	ENTERFUNC("acmupp_bstruct");
	lf->nstructs = NSCRNSTRUCT;
	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;
	if (sp) {
		sp[Z_MIN].pointer = (char *) &lf->min;
		sp[Z_MIN].filled = lf->f1;
		sp[Z_EXPIRE].pointer = (char *) &lf->expire;
		sp[Z_EXPIRE].filled = lf->f2;
		sp[Z_LIFETIME].pointer = (char *) &lf->lifetime;
		sp[Z_LIFETIME].filled = lf->f3;
		sp[Z_MAXLEN].pointer = (char *) &lf->maxlen;
		sp[Z_MAXLEN].filled = lf->f4;
		sp[Z_NULLPW].pointer = &lf->nullpw;
		sp[Z_NULLPW].filled = lf->f5;
		sp[Z_PICKPWD].pointer = &lf->pick_pwd;
		sp[Z_PICKPWD].filled = lf->f6;
		sp[Z_GENPWD].pointer = &lf->gen_pwd;
		sp[Z_GENPWD].filled = lf->f7;
		sp[Z_GENCHARS].pointer = &lf->gen_chars;
		sp[Z_GENCHARS].filled = lf->f8;
		sp[Z_GENLETTERS].pointer = &lf->gen_letters;
		sp[Z_GENLETTERS].filled = lf->f9;
		sp[Z_RESTRICT].pointer = &lf->restrict;
		sp[Z_RESTRICT].filled = lf->f10;
	}
	EXITFUNC("acmupp_bstruct");
	return 0;
}


/*
 * Build the fillin structure.
 * The requirements for this routine were completed at the top level.
 */

int
acmupp_bfill(lf)
	struct acmupp_fillin *lf;
{
	ENTERFUNC("acmupp_bfill");
	XGetUserInfo(gl_user, &pru);
	lf->f1 = pd->uflg.fg_min;
	lf->min = lf->f1 ? pd->ufld.fd_min / WEEK_SEC : 0;

	lf->f2 = pd->uflg.fg_expire;
	lf->expire = lf->f2 ? pd->ufld.fd_expire / WEEK_SEC : 0;

	lf->f3 = pd->uflg.fg_lifetime;
	lf->lifetime = lf->f3 ? pd->ufld.fd_lifetime / WEEK_SEC : 0;

	lf->f4 = pd->uflg.fg_maxlen;
	lf->maxlen = lf->f4 ? pd->ufld.fd_maxlen : 0;

	/*
	 * system stores whether null password allowed.
	 * Screen specifies whether system requires password.
	 */

	lf->f5 = pd->uflg.fg_nullpw;
	lf->nullpw = lf->f5 ? !pd->ufld.fd_nullpw : 1;

	lf->f6 = pd->uflg.fg_pick_pwd;
	lf->pick_pwd = lf->f6 ? pd->ufld.fd_pick_pwd : 0;

	lf->f7 = pd->uflg.fg_pick_pwd;
	lf->gen_pwd = lf->f7 ? pd->ufld.fd_gen_pwd : 0;

	lf->f8 = pd->uflg.fg_gen_chars;
	lf->gen_chars = lf->f8 ? pd->ufld.fd_gen_chars : 0;

	lf->f9 = pd->uflg.fg_gen_letters;
	lf->gen_letters = lf->f9 ? pd->ufld.fd_gen_letters : 0;

	lf->f10 = pd->uflg.fg_restrict;
	lf->restrict = lf->f10 ? pd->ufld.fd_restrict : 0;

	EXITFUNC("acmupp_bfill");
	return 0;
}

/*
 * Decide whether the user is authorized to perform the function.
 * Nothing to decide here.
 */

int
acmupp_auth(argv, lf)
	char **argv;
	struct acmupp_fillin *lf;
{
	return 0;
}

/*
 * validate function for the whole screen.
 */

int
acmupp_valid(argv, lf)
	char **argv;
	struct acmupp_fillin *lf;
{
	int ret = 0;

	ENTERFUNC("acmupp_valid");
	EXITFUNC("acmupp_valid");
	return ret;
}

/*
 * action function for the whole screen.
 */

static int
acmupp_action(lf)
	struct acmupp_fillin *lf;
{
	register Scrn_struct *asp = acmupp_struct;

	ENTERFUNC("acmupp_action");

	if (asp[Z_MIN].filled || asp[Z_MIN].changed)
		pd->uflg.fg_min  = 1;
	if (pd->uflg.fg_min)
		pd->ufld.fd_min = lf->min * WEEK_SEC;

	if (asp[Z_EXPIRE].filled || asp[Z_EXPIRE].changed)
			pd->uflg.fg_expire = 1;
	if (pd->uflg.fg_expire)
		pd->ufld.fd_expire = lf->expire * WEEK_SEC;

	if (asp[Z_LIFETIME].filled || asp[Z_LIFETIME].changed)
		pd->uflg.fg_lifetime = 1;
	if (pd->uflg.fg_lifetime)
		pd->ufld.fd_lifetime = lf->lifetime * WEEK_SEC;

	if (asp[Z_MAXLEN].filled || asp[Z_MAXLEN].changed)
		pd->uflg.fg_maxlen = 1;
	if (pd->uflg.fg_maxlen)
		pd->ufld.fd_maxlen = lf->maxlen;

	if (asp[Z_NULLPW].filled || asp[Z_NULLPW].changed)
		pd->uflg.fg_nullpw = 1;
	if (pd->uflg.fg_nullpw)
		pd->ufld.fd_nullpw = !lf->nullpw;

	if (asp[Z_PICKPWD].filled || asp[Z_PICKPWD].changed)
		pd->uflg.fg_pick_pwd = 1;
	if (pd->uflg.fg_pick_pwd)
		pd->ufld.fd_pick_pwd = lf->pick_pwd;

	if (asp[Z_GENPWD].filled || asp[Z_GENPWD].changed)
		pd->uflg.fg_gen_pwd = 1;
	if (pd->uflg.fg_gen_pwd)
		pd->ufld.fd_gen_pwd = lf->gen_pwd;

	if (asp[Z_GENCHARS].filled || asp[Z_GENCHARS].changed)
		pd->uflg.fg_gen_chars = 1;
	if (pd->uflg.fg_gen_chars)
		pd->ufld.fd_gen_chars = lf->gen_pwd;

	if (asp[Z_GENLETTERS].filled || asp[Z_GENLETTERS].changed)
		pd->uflg.fg_gen_letters = 1;
	if (pd->uflg.fg_gen_letters)
		pd->ufld.fd_gen_letters = lf->gen_letters;

	if (asp[Z_RESTRICT].filled || asp[Z_RESTRICT].changed)
		pd->uflg.fg_restrict = 1;
	if (pd->uflg.fg_restrict)
		pd->ufld.fd_restrict = lf->restrict;

	XWriteUserInfo(&pru);

	EXITFUNC("acmupp_action");
	return INTERRUPT;
}

/*
 * routine to free memory.
 * since clearance levels may be re-used, keep memory allocated.
 */

int
acmupp_do_free(argv, af, nstructs, pp, dp, sp)
char **argv;
struct FILLINSTRUCT *af;
int nstructs;
Scrn_parms *pp;
Scrn_desc *dp;
Scrn_struct *sp;
{
	ENTERFUNC("acmupp_do_free");
	EXITFUNC("acmupp_do_free");
	return 1;
}

#define SETUPFUNC	acmupp_setup	/* defined by stemplate.c */
#define AUTHFUNC	acmupp_auth
#define BUILDFILLIN	acmupp_bfill

#define INITFUNC	acmupp_init		/* defined by stemplate.c */
#define BUILDSTRUCT	acmupp_bstruct

#define ROUTFUNC	acmupp_exit		/* defined by stemplate.c */
#define VALIDATE	acmupp_valid
#define SCREENACTION	acmupp_action

#define FREEFUNC	acmupp_free		/* defined by stemplate.c */
#define FREESTRUCT	acmupp_do_free

#include "stemplate.c"

