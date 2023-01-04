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
static char	*sccsid = "@(#)$RCSfile: aumd.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:07 $";
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
 * Routines to implement modification of single-user directory and
 * multi-user audit directory list.
 */

#include <sys/secdefines.h>

#if SEC_BASE

#include <sys/security.h>
#include <sys/audit.h>
#include <prot.h>
#include "gl_defs.h"
#include "IfAudit.h"
#include "userif.h"
#include "UIMain.h"
#include "valid.h"
#include "logging.h"

/* static routine definitions */

static int aumd_auth();
static int aumd_bfill();
static int aumd_valid();
static int aumd_exit();
static int aumd_expand();
static void expand_table();

/* structures defined in au_scrns.c */

extern Scrn_parms	aumd_scrn;
extern Scrn_desc	aumd_desc[];

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	aumd_scrn
#define STRUCTTEMPLATE	aumd_struct
#define DESCTEMPLATE	aumd_desc
#define FILLINSTRUCT	audir_fillin
#define FILLIN		aumd_fill
#define TRAVERSERW	TRAV_RW

#define MD_SINGLE_TITLE_DESC	0
#define MD_SINGLE_DESC		1
#define MD_MULTI_TITLE_DESC	2
#define MD_MULTI_DESC		3
#define FIRSTDESC	MD_SINGLE_DESC

#define MD_SINGLE_STRUCT	0
#define MD_MULTI_STRUCT		1
#define NSCRNSTRUCT		2

static Scrn_struct	*aumd_struct;
static struct audir_fillin au_buf, *aumd_fill = &au_buf;

static int IsISSO;

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

aumd_auth(argv, aufill)
	char **argv;
	struct audir_fillin *aufill;
{
	static int first_time = 1;

	ENTERFUNC("aumd_auth");
	if (first_time) {
		AuditDirListStart();
		first_time = 0;
		IsISSO = authorized_user("isso");
	}
	EXITFUNC("aumd_auth");
	if (IsISSO)
		return 0;
	else
		return 1;
}

/*
 * Grab the root directory and directory list from the parameters file
 * Allocate a structure for the screen that is one larger than the
 * current table.  Expand dynamically as the user adds new directories.
 */

static int
aumd_bfill(aufill)
        struct audir_fillin *aufill;
{
	int ret;

	ENTERFUNC("aumd_bfill");

	ret = AuditDirListGet(aufill);

	expand_table(aufill);

	return ret;
}

/*
 * Expand the directory list by one
 */

static void
expand_table(aufill)
        struct audir_fillin *aufill;
{
	char **new_table;

	/* expand by one the directory table to allow for expansion */

	new_table = expand_cw_table(aufill->dirs,
			aufill->ndirs, aufill->ndirs+1, AUDIRWIDTH + 1);
	if (new_table == (char **) 0)
		MemoryError();

	aufill->dirs = new_table;
	aufill->ndirs++;

	EXITFUNC("aumd_bfill");
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
aumd_bstruct(aufill, sptemplate)
	struct audir_fillin *aufill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;

	ENTERFUNC("aumd_bstruct");

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	sp[MD_SINGLE_STRUCT].pointer = aufill->root_dir;
	sp[MD_SINGLE_STRUCT].filled = 1;

	sp[MD_MULTI_STRUCT].pointer = (char *) aufill->dirs;
	sp[MD_MULTI_STRUCT].filled = aufill->ndirs;
	sp[MD_MULTI_STRUCT].val_act = aumd_expand;

	EXITFUNC("aumd_bstruct");
	return 0;
}

/*
 * Entry validation routine.  If the user is typing on the last line
 * of the scrolling region, expand the table by one.
 */

static int
aumd_expand()
{
	/* User typed on last line if first character of last line non-zero */

	if (aumd_fill->dirs[aumd_fill->ndirs-1][0] != '\0') {
		struct scrn_struct *sp;

		expand_table(aumd_fill);
		sp = PARMTEMPLATE.ss;
		sp[MD_MULTI_STRUCT].pointer = (char *) aumd_fill->dirs;
		sp[MD_MULTI_STRUCT].filled = aumd_fill->ndirs;

		return 1;  /* redraw screen and reload scrn_rep */
	} else
		return 0;
}

/*
 * action routine.
 * Perform restore of audit session.
 */

static int
aumd_action(aufill)
	struct audir_fillin *aufill;
{
	int ret;

	ENTERFUNC("aumd_action");

	ret = AuditDirListPut(aufill);

	EXITFUNC("aumd_action");

	return((ret == 0) ? 1 : 0);
}


static void
md_free(argv, aufill, nstructs, pp, dp, sp)
	char **argv;
	struct audir_fillin *aufill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("md_free");
	if (aufill->dirs != (char **) 0) {
		free_cw_table(aufill->dirs);
		aufill->dirs = (char **) 0;
	}
	EXITFUNC("md_free");
	return;
}

/*
 * validate the structure -- the logic is in AuditParametersCheck().
 */

static int
aumd_valid(argv, aufill)
	char **argv;
	struct audir_fillin *aufill;
{
	int ret;

	ENTERFUNC("aumd_valid");

	ret = AuditDirListCheck(aufill);

	EXITFUNC("aumd_valid");
	return ret;
}

#define SETUPFUNC	aumd_setup	/* defined by stemplate.c */
#define AUTHFUNC	aumd_auth
#define BUILDFILLIN	aumd_bfill

#define INITFUNC	aumd_init		/* defined by stemplate.c */
#define BUILDSTRUCT	aumd_bstruct

#define ROUTFUNC	aumd_exit		/* defined by stemplate.c */
#define VALIDATE	aumd_valid
#define SCREENACTION	aumd_action

#define FREEFUNC	aumd_free		/* defined by stemplate.c */
#define FREESTRUCT	md_free

#include "stemplate.c"

#endif /* SEC_BASE */
