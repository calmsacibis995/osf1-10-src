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
static char	*sccsid = "@(#)$RCSfile: aumm.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:12 $";
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
 * Routines to implement modification of audit parameters
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

static int aumm_auth();
static int aumm_bfill();
static int aumm_valid();
static int aumm_exit();

/* structures defined in au_scrns.c */

extern Scrn_parms	aumm_scrn;
extern Scrn_desc	aumm_desc[];

/* definitions for the screen infrastructure */

#define PARMTEMPLATE	aumm_scrn
#define STRUCTTEMPLATE	aumm_struct
#define DESCTEMPLATE	aumm_desc
#define FILLINSTRUCT	audit_parm_struct
#define FILLIN		aumm_fill
#define TRAVERSERW	TRAV_RW

#define PRM_DMN_READ_TITLE_DESC	0
#define PRM_DMN_READ_DESC	1
#define PRM_BUF_MEM_TITLE_DESC	2
#define PRM_BUF_MEM_DESC	3
#define PRM_CSWITCH_TITLE_DESC	4
#define PRM_CSWITCH_DESC	5
#define PRM_COMPACT_TITLE_DESC	6
#define PRM_COMPACT_DESC	7
#define PRM_ENABLE_TITLE_DESC	8
#define PRM_ENABLE_DESC		9
#define PRM_SHUT_TITLE_DESC	10
#define PRM_SHUT_DESC		11
#define PRM_THIS_DESC		12
#define PRM_FUTURE_DESC		13
#define FIRSTDESC	PRM_DMN_READ_DESC

#define PRM_DMN_READ_STRUCT	0
#define PRM_BUF_MEM_STRUCT	1
#define PRM_CSWITCH_STRUCT	2
#define PRM_COMPACT_STRUCT	3
#define PRM_ENABLE_STRUCT	4
#define PRM_SHUT_STRUCT		5
#define PRM_THIS_STRUCT		6
#define PRM_FUTURE_STRUCT	7
#define NSCRNSTRUCT		8

static Scrn_struct	*aumm_struct;
static struct audit_parm_struct au_buf, *aumm_fill = &au_buf;

static int IsISSO;

/*
 * Initialization for this screen.
 * Returns 0 if the user is authorized for "isso", otherwise returns 1.
 */

aumm_auth(argv, aufill)
	char **argv;
	audparm_fillin *aufill;
{
	static int first_time = 1;

	ENTERFUNC("aumm_auth");
	if (first_time) {
		AuditParametersStart();
		first_time = 0;
		IsISSO = authorized_user("isso");
	}
	EXITFUNC("aumm_auth");
	if (IsISSO)
		return 0;
	else
		return 1;
}

/*
 * Grab the current audit parameters from the parameters file and the
 * system defaults database.
 */

static int
aumm_bfill(aufill)
        audparm_fillin *aufill;
{
	int ret;

	ENTERFUNC("aumm_bfill");

	ret = AuditParametersGet(aufill);

	/* max memory on screen is in units of 1K */

	aufill->au.max_memory /= 1024;

	EXITFUNC("aumm_bfill");
	return ret;
}

/*
 * Builds the scrn_struct for this screen.
 */

static int
aumm_bstruct(aufill, sptemplate)
	audparm_fillin *aufill;
	Scrn_struct    **sptemplate;
{
	struct scrn_struct *sp;

	ENTERFUNC("aumm_bstruct");

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	sp[PRM_DMN_READ_STRUCT].pointer = (char *) &aufill->au.read_count;
	sp[PRM_DMN_READ_STRUCT].filled = 1;

	sp[PRM_BUF_MEM_STRUCT].pointer = (char *) &aufill->au.max_memory;
	sp[PRM_BUF_MEM_STRUCT].filled = 1;

	sp[PRM_CSWITCH_STRUCT].pointer = (char *) &aufill->au.caf_maxsize;
	sp[PRM_CSWITCH_STRUCT].filled = 1;

	sp[PRM_COMPACT_STRUCT].pointer = &aufill->compacted;
	sp[PRM_COMPACT_STRUCT].filled = 1;

	sp[PRM_ENABLE_STRUCT].pointer = &aufill->audit_on_startup;
	sp[PRM_ENABLE_STRUCT].filled = 1;

	sp[PRM_SHUT_STRUCT].pointer = &aufill->shut_or_panic;
	sp[PRM_SHUT_STRUCT].filled = 1;

	sp[PRM_THIS_STRUCT].pointer = &aufill->this;
	sp[PRM_THIS_STRUCT].filled = 1;
	
	sp[PRM_FUTURE_STRUCT].pointer = &aufill->future;
	sp[PRM_FUTURE_STRUCT].filled = 1;

	if (CheckAuditEnabled() == 1)
		aumm_desc[PRM_THIS_DESC].inout = FLD_BOTH;
	else
		aumm_desc[PRM_THIS_DESC].inout = FLD_OUTPUT;

	EXITFUNC("aumm_bstruct");
	return 0;
}

/*
 * action routine.
 * Perform restore of audit session.
 */

static int
aumm_action(aufill)
	audparm_fillin *aufill;
{
	int ret;

	ENTERFUNC("aumm_action");

	/* max memory on screen is in units of 1K */

	aufill->au.max_memory *= 1024;

	ret = AuditParametersPut(aufill);

	aufill->au.max_memory /= 1024;

	EXITFUNC("aumm_action");

	return !ret;
}


static void
mm_free(argv, aufill, nstructs, pp, dp, sp)
	char **argv;
	audparm_fillin *aufill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	ENTERFUNC("mm_free");
	EXITFUNC("mm_free");
	return;
}

/*
 * validate the structure -- the logic is in AuditParametersCheck().
 */

static int
aumm_valid(argv, aufill)
	char **argv;
	audparm_fillin *aufill;
{
	int ret;

	ENTERFUNC("aumm_valid");

	/* max memory is in units of 1K on the screen */

	aufill->au.max_memory *= 1024;

	ret = AuditParametersCheck(aufill);

	aufill->au.max_memory /= 1024;

	EXITFUNC("aumm_valid");
	return ret;
}

#define SETUPFUNC	aumm_setup	/* defined by stemplate.c */
#define AUTHFUNC	aumm_auth
#define BUILDFILLIN	aumm_bfill

#define INITFUNC	aumm_init		/* defined by stemplate.c */
#define BUILDSTRUCT	aumm_bstruct

#define ROUTFUNC	aumm_exit		/* defined by stemplate.c */
#define VALIDATE	aumm_valid
#define SCREENACTION	aumm_action

#define FREEFUNC	aumm_free		/* defined by stemplate.c */
#define FREESTRUCT	mm_free

#include "stemplate.c"

#endif /* SEC_BASE */
