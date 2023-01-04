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
static char	*sccsid = "@(#)$RCSfile: acdupp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:19 $";
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
 *	acdupp.c - modify account default password parameters
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
int acdupp_setup(), acdupp_init(), acdupp_free(), acdupp_exit();

/*
 * Static global variables
 */

extern Scrn_parms acdupp_scrn;
extern Scrn_desc  acdupp_desc[];
extern Menu_scrn  *acdupp_menu;
Scrn_struct *acdupp_struct;
extern Scrn_hdrs acdupp_hdrs;

#define PARMTEMPLATE	acdupp_scrn
#define STRUCTTEMPLATE	acdupp_struct
#define DESCTEMPLATE	acdupp_desc
#define FILLINSTRUCT	acdupp_fillin
#define FILLIN		acduppP

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

struct acdupp_fillin {
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
} acdupp, *acduppP = &acdupp;

static struct prpw_if pru;	/* prpw superstructure */

static struct pr_passwd *pd = &pru.prpw;	/* prpw */

/*
 * Fill in the scrn_struct
 */

int
acdupp_bstruct(lf, sptemplate)
	struct acdupp_fillin *lf;
	struct scrn_struct **sptemplate;
{
	register struct scrn_struct *sp;

	ENTERFUNC("acdupp_bstruct");
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
	EXITFUNC("acdupp_bstruct");
	return 0;
}


/*
 * Build the fillin structure.
 * The requirements for this routine were completed at the top level.
 */

int
acdupp_bfill(lf)
	struct acdupp_fillin *lf;
{
	ENTERFUNC("acdupp_bfill");
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

	EXITFUNC("acdupp_bfill");
	return 0;
}

/*
 * Decide whether the user is authorized to perform the function.
 * Nothing to decide here.
 */

int
acdupp_auth(argv, lf)
	char **argv;
	struct acdupp_fillin *lf;
{
	return 0;
}

/*
 * validate function for the whole screen.
 */

int
acdupp_valid(argv, lf)
	char **argv;
	struct acdupp_fillin *lf;
{
	int ret = 0;

	return ret;
}

/*
 * action function for the whole screen.
 */

static int
acdupp_action(lf)
	struct acdupp_fillin *lf;
{
	return INTERRUPT;
}

/*
 * routine to free memory.
 * since clearance levels may be re-used, keep memory allocated.
 */

int
acdupp_do_free(argv, af, nstructs, pp, dp, sp)
char **argv;
struct FILLINSTRUCT *af;
int nstructs;
Scrn_parms *pp;
Scrn_desc *dp;
Scrn_struct *sp;
{
	ENTERFUNC("acdupp_do_free");
	EXITFUNC("acdupp_do_free");
	return 1;
}

#define SETUPFUNC	acdupp_setup	/* defined by stemplate.c */
#define AUTHFUNC	acdupp_auth
#define BUILDFILLIN	acdupp_bfill

#define INITFUNC	acdupp_init		/* defined by stemplate.c */
#define BUILDSTRUCT	acdupp_bstruct

#define ROUTFUNC	acdupp_exit		/* defined by stemplate.c */
#define VALIDATE	acdupp_valid
#define SCREENACTION	acdupp_action

#define FREEFUNC	acdupp_free		/* defined by stemplate.c */
#define FREESTRUCT	acdupp_do_free

#include "stemplate.c"
