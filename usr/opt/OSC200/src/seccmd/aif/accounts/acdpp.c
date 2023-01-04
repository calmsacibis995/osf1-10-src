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
static char	*sccsid = "@(#)$RCSfile: acdpp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:07 $";
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
#if SEC_BASE /*{*/

/*
 *	acdpp.c - set account default password parameters
 */


#include <stdio.h>
#include <ctype.h>
#include <sys/security.h>
#include <prot.h>
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
int acdpp_setup(), acdpp_init(), acdpp_free(), acdpp_exit();

/*
 * Static global variables
 */

extern Scrn_parms acdpp_scrn;
extern Scrn_desc  acdpp_desc[];
extern Menu_scrn  *acdpp_menu;
static Scrn_struct *acdpp_struct;
extern Scrn_hdrs acdpp_hdrs;

#define PARMTEMPLATE	acdpp_scrn
#define STRUCTTEMPLATE	acdpp_struct
#define DESCTEMPLATE	acdpp_desc
#define FILLINSTRUCT	acdpp_fillin
#define FILLIN		acdppP

#define Z_MIN		0
#define Z_EXPIRE	1
#define Z_LIFETIME	2
#define Z_PWEXPIRE	3
#define Z_MAXLEN	4
#define Z_NULLPW	5
#define Z_PICKPWD	6
#define Z_GENPWD	7
#define Z_GENCHARS	8
#define Z_GENLETTERS	9
#define Z_RESTRICT	10

#define FIRSTDESC	1

#define NSCRNSTRUCT	11

/*
 * External declarations
 */

extern char *Malloc(), *Calloc();

extern void LoadMessage();

/*
 * Fillin structure - most fields named for pr_default fields
 */

static struct acdpp_fillin {
	int min;
	int expire;
	int lifetime;
	int pw_expire_warning;
	int maxlen;
	char nullpw;
	char pick_pwd;
	char gen_pwd;		/* pronounceable password generator */
	char gen_chars;		/* character password generator */
	char gen_letters;	/* letter password generator */
	char restrict;
	char f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11; /* flags for above */
	int nstructs;
} acdpp, *acdppP = &acdpp;

static struct sdef_if sds;	/* system defaults superstructure */

static struct pr_default *sd = &sds.df;	/* system defaults */


/*
 * Fill in the scrn_struct
 */

static int
acdpp_bstruct(lf, sptemplate)
	struct acdpp_fillin *lf;
	struct scrn_struct **sptemplate;
{
	register struct scrn_struct *sp;

	ENTERFUNC("acdpp_bstruct");
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
		sp[Z_PWEXPIRE].pointer = (char *) &lf->pw_expire_warning;
		sp[Z_PWEXPIRE].filled = lf->f4;
		sp[Z_MAXLEN].pointer = (char *) &lf->maxlen;
		sp[Z_MAXLEN].filled = lf->f5;
		sp[Z_NULLPW].pointer = &lf->nullpw;
		sp[Z_NULLPW].filled = lf->f6;
		sp[Z_PICKPWD].pointer = &lf->pick_pwd;
		sp[Z_PICKPWD].filled = lf->f7;
		sp[Z_GENPWD].pointer = &lf->gen_pwd;
		sp[Z_GENPWD].filled = lf->f8;
		sp[Z_GENCHARS].pointer = &lf->gen_chars;
		sp[Z_GENCHARS].filled = lf->f9;
		sp[Z_GENLETTERS].pointer = &lf->gen_letters;
		sp[Z_GENLETTERS].filled = lf->f10;
		sp[Z_RESTRICT].pointer = &lf->restrict;
		sp[Z_RESTRICT].filled = lf->f11;
	}
	EXITFUNC("acdpp_bstruct");
	return 0;
}


/*
 * Build the fillin structure.
 * The requirements for this routine were completed at the top level.
 */

static int
acdpp_bfill(lf)
	struct acdpp_fillin *lf;
{
	ENTERFUNC("acdpp_bfill");
	XGetSystemInfo(&sds);
	lf->f1 = sd->prg.fg_min;
	lf->min = lf->f1 ? sd->prd.fd_min / WEEK_SEC : 0;

	lf->f2 = sd->prg.fg_expire;
	lf->expire = lf->f2 ? sd->prd.fd_expire / WEEK_SEC : 0;

	lf->f3 = sd->prg.fg_lifetime;
	lf->lifetime = lf->f3 ? sd->prd.fd_lifetime / WEEK_SEC : 0;

	lf->f4 = sd->sflg.fg_pw_expire_warning;
	lf->pw_expire_warning = lf->f4 ?
		sd->sfld.fd_pw_expire_warning / DAY_SEC : 0;

	lf->f5 = sd->prg.fg_maxlen;
	lf->maxlen = lf->f5 ? sd->prd.fd_maxlen : 0;

	/*
	 * system stores whether null password allowed.
	 * Screen specifies whether system requires password.
	 */

	lf->f6 = sd->prg.fg_nullpw;
	lf->nullpw = lf->f6 ? !sd->prd.fd_nullpw : 1;

	lf->f7 = sd->prg.fg_pick_pwd;
	lf->pick_pwd = lf->f7 ? sd->prd.fd_pick_pwd : 0;

	lf->f8 = sd->prg.fg_pick_pwd;
	lf->gen_pwd = lf->f8 ? sd->prd.fd_gen_pwd : 0;

	lf->f9 = sd->prg.fg_gen_chars;
	lf->gen_chars = lf->f9 ? sd->prd.fd_gen_chars : 0;

	lf->f10 = sd->prg.fg_gen_letters;
	lf->gen_letters = lf->f10 ? sd->prd.fd_gen_letters : 0;

	lf->f11 = sd->prg.fg_restrict;
	lf->restrict = lf->f11 ? sd->prd.fd_restrict : 0;

	EXITFUNC("acdpp_bfill");
	return 0;
}

/*
 * Decide whether the user is authorized to perform the function.
 * Nothing to decide here.
 */

static int
acdpp_auth(argv, lf)
	char **argv;
	struct acdpp_fillin *lf;
{
	return 0;
}

/*
 * validate function for the whole screen.
 */

static int
acdpp_valid(argv, lf)
	char **argv;
	struct acdpp_fillin *lf;
{
	int ret = 0;

	ENTERFUNC("acdpp_valid");
	EXITFUNC("acdpp_valid");
	return ret;
}

/*
 * action function for the whole screen.
 */

static int
acdpp_action(lf)
	struct acdpp_fillin *lf;
{
	register Scrn_struct *asp = acdpp_struct;

	ENTERFUNC("acdpp_action");

	if (asp[Z_MIN].filled || asp[Z_MIN].changed)
		sd->prg.fg_min  = 1;
	if (sd->prg.fg_min)
		sd->prd.fd_min = lf->min * WEEK_SEC;

	if (asp[Z_EXPIRE].filled || asp[Z_EXPIRE].changed)
			sd->prg.fg_expire = 1;
	if (sd->prg.fg_expire)
		sd->prd.fd_expire = lf->expire * WEEK_SEC;

	if (asp[Z_LIFETIME].filled || asp[Z_LIFETIME].changed)
		sd->prg.fg_lifetime = 1;
	if (sd->prg.fg_lifetime)
		sd->prd.fd_lifetime = lf->lifetime * WEEK_SEC;

	if (asp[Z_PWEXPIRE].filled || asp[Z_PWEXPIRE].changed)
		sd->sflg.fg_pw_expire_warning = 1;
	if (sd->sflg.fg_pw_expire_warning)
		sd->sfld.fd_pw_expire_warning = lf->pw_expire_warning * DAY_SEC;

	if (asp[Z_MAXLEN].filled || asp[Z_MAXLEN].changed)
		sd->prg.fg_maxlen = 1;
	if (sd->prg.fg_maxlen)
		sd->prd.fd_maxlen = lf->maxlen;

	if (asp[Z_NULLPW].filled || asp[Z_NULLPW].changed)
		sd->prg.fg_nullpw = 1;
	if (sd->prg.fg_nullpw)
		sd->prd.fd_nullpw = !lf->nullpw;

	if (asp[Z_PICKPWD].filled || asp[Z_PICKPWD].changed)
		sd->prg.fg_pick_pwd = 1;
	if (sd->prg.fg_pick_pwd)
		sd->prd.fd_pick_pwd = lf->pick_pwd;

	if (asp[Z_GENPWD].filled || asp[Z_GENPWD].changed)
		sd->prg.fg_gen_pwd = 1;
	if (sd->prg.fg_gen_pwd)
		sd->prd.fd_gen_pwd = lf->gen_pwd;

	if (asp[Z_GENCHARS].filled || asp[Z_GENCHARS].changed)
		sd->prg.fg_gen_chars = 1;
	if (sd->prg.fg_gen_chars)
		sd->prd.fd_gen_chars = lf->gen_pwd;

	if (asp[Z_GENLETTERS].filled || asp[Z_GENLETTERS].changed)
		sd->prg.fg_gen_letters = 1;
	if (sd->prg.fg_gen_letters)
		sd->prd.fd_gen_letters = lf->gen_letters;

	if (asp[Z_RESTRICT].filled || asp[Z_RESTRICT].changed)
		sd->prg.fg_restrict = 1;
	if (sd->prg.fg_restrict)
		sd->prd.fd_restrict = lf->restrict;

	XWriteSystemInfo(&sds);

	EXITFUNC("acdpp_action");
	return INTERRUPT;
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

#define SETUPFUNC	acdpp_setup	/* defined by stemplate.c */
#define AUTHFUNC	acdpp_auth
#define BUILDFILLIN	acdpp_bfill

#define INITFUNC	acdpp_init		/* defined by stemplate.c */
#define BUILDSTRUCT	acdpp_bstruct

#define ROUTFUNC	acdpp_exit		/* defined by stemplate.c */
#define VALIDATE	acdpp_valid
#define SCREENACTION	acdpp_action

#define FREEFUNC	acdpp_free		/* defined by stemplate.c */
#define FREESTRUCT	do_free

#include "stemplate.c"

#endif /*} SEC_BASE */
