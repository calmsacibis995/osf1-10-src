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
static char	*sccsid = "@(#)$RCSfile: acdka.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:53:00 $";
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
 *	acdka.c - set def kernel auths
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

/*
 * action routine declarations
*/
int acdka_setup(), acdka_init(), acdka_free(), acdka_exit();

/*
 * acdka screen variables
 */

extern Scrn_parms acdka_scrn;
extern Scrn_desc  acdka_desc[];
extern Menu_scrn  *acdka_menu;
static Scrn_struct *acdka_struct, *asp;
extern Scrn_hdrs acdka_hdrs;

#define PARMTEMPLATE	acdka_scrn
#define STRUCTTEMPLATE	acdka_struct
#define DESCTEMPLATE	acdka_desc
#define FILLINSTRUCT	acdka_fillin
#define FILLIN		acdkaP

#define FIRSTDESC	1
#define NSCRNSTRUCT	9

/*
 * External declarations
 */

extern char *Malloc(), *Calloc();

extern void LoadMessage();

static char
	**msg_error,
	*msg_error_text;


/*
 * Fillin structure - most fields named for pr_default fields
 */

static struct FILLINSTRUCT {
	char **auths;		/* list of kernel auths names */
	char *ker_st;		/* "Ker" state - 1 or 0 */
	char *bas_st;		/* "Base" state - 1 or 0 */
	int nstructs;		/* total auths in list */
	int npercol1;		/* auths in col 1 */
	int npercol2;		/* auths in col 2 */
	int npercol3;		/* auths in col 3 */
} acdka, *acdkaP = &acdka;

static struct sdef_if sds;      /* system defaults superstructure */

static struct pr_default *sd = &sds.df; /* system defaults */


/*
 * Fill in the scrn_struct
 */

static int
acdka_bstruct(af, sptemplate)
	struct FILLINSTRUCT *af;
	struct scrn_struct **sptemplate;
{
	register struct scrn_struct *sp;

	ENTERFUNC("acdka_bstruct");
	sp = PARMTEMPLATE.ss;
	*sptemplate = PARMTEMPLATE.ss;
	if (sp) {
		asp = sp;
		sp[0].pointer = (char *) af->auths;
		sp[0].filled = af->npercol1;
		sp[1].state = af->ker_st;
		sp[1].filled = af->npercol1;
		sp[2].state = af->bas_st;
		sp[2].filled = af->npercol1;

		sp[3].pointer = (char *) &af->auths[af->npercol1];
		sp[3].filled = af->npercol2;
		sp[4].state = &af->ker_st[af->npercol1];
		sp[4].filled = af->npercol2;
		sp[5].state = &af->bas_st[af->npercol1];
		sp[5].filled = af->npercol2;

		sp[6].pointer = (char *) &af->auths[af->npercol1 +
			af->npercol2];
		sp[6].filled = af->npercol3;
		sp[7].state = &af->ker_st[af->npercol1 + af->npercol2];
		sp[7].filled = af->npercol3;
		sp[8].state = &af->bas_st[af->npercol1 + af->npercol2];
		sp[8].filled = af->npercol3;
	}
	EXITFUNC("acdka_bstruct");
	return 0;
}


/*
 * Build the fillin structure.
 * The requirements for this routine were completed at the top level.
 */

static int
acdka_bfill(af)
	struct FILLINSTRUCT *af;
{
	register int i;
	int ret;
	int extra;

	ENTERFUNC("acdka_bfill");

	if (ret = XGetSystemInfo (&sds))
		return ret;

	/* Read the subsytem names in */

	af->nstructs = MIN (42, SEC_MAX_SPRIV); 	/* FOR NOW */
	if (!(af->auths = (char **)alloc_table (sys_priv, af->nstructs + 1)))
		MemoryError ();		/* dies */


	for (i = 0; i< af->nstructs; i++) {
		strcpy (af->auths[i], sys_priv[i].name);
	}


	/* now build the state tables */

	if (!(af->ker_st = Calloc (af->nstructs, sizeof (char))))
		MemoryError ();

	if (!(af->bas_st = Calloc (af->nstructs, sizeof (char))))
		MemoryError ();

	/* Load the kernel privs */

	if (sd->prg.fg_sprivs)
		for (i=0;i<af->nstructs;i++)
		    if (ISBITSET (sd->prd.fd_sprivs, i))
	        	af->ker_st[i] = 1;
		    else
			af->ker_st[i] = 0;
	else
		for (i=0;i<af->nstructs;i++)
		    af->ker_st[i] = 0;

	/* Load the default base privs */

	if (sd->prg.fg_bprivs)
		for (i=0;i<af->nstructs;i++)
		    if (ISBITSET (sd->prd.fd_bprivs, i))
			af->bas_st[i] = 1;
		    else
			af->bas_st[i] = 0;
	else
		for (i=0;i<af->nstructs;i++)
		    af->bas_st[i] = 0;


	/* determine the number of entries per column */

	af->npercol1 = af->npercol2 = af->npercol3 = af->nstructs / 3;
	extra = af->nstructs % 3;
	if (extra) {
		af->npercol1++;
		af->npercol2 += (extra - 1);
	}
		
	EXITFUNC("acdka_bfill");
	return 0;
}

/*
 * Decide whether the user is authorized to perform the function.
 * Nothing to decide here.
 */

static int
acdka_auth(argv, af)
	char **argv;
	struct FILLINSTRUCT *af;
{
	return 0;
}

/*
 * validate function for the whole screen.
 */

static int
acdka_valid(argv, af)
	char **argv;
	struct FILLINSTRUCT *af;
{
	int ret = 0;

	ENTERFUNC("acdka_valid");
	EXITFUNC("acdka_valid");
	return ret;
}

/*
 * action function for the whole screen.
 */

static int
acdka_action(af)
	struct FILLINSTRUCT *af;
{
	char **table;
	int i, ret;

	ENTERFUNC("acdka_action");

	/* Allocate space to store subsystem names */

	if (!(table = (char **)alloc_table (sys_priv, af->nstructs)))
		MemoryError();

	/* System privileges first */

	sd->prg.fg_sprivs = 1;
	for (i =0; i<af->nstructs; i++)
		if (af->ker_st[i]) {
			ADDBIT (sd->prd.fd_sprivs, sys_priv[i].value);
		} else {
			RMBIT (sd->prd.fd_sprivs, sys_priv[i].value);
		}

	/* Now the base privileges */

	sd->prg.fg_bprivs = 1;
	for (i =0; i<af->nstructs; i++)
		if (af->bas_st[i]) {
			ADDBIT (sd->prd.fd_bprivs, sys_priv[i].value);
		} else {
			RMBIT (sd->prd.fd_bprivs, sys_priv[i].value);
		}

	/* Write information */

	ret = XWriteSystemInfo (&sds);

	free_cw_table (table);

	EXITFUNC("acdka_action");
	return ret;
}


/*
 * routine to free memory.
 */

static int
acdka_do_free(argv, af, nstructs, pp, dp, sp)
char **argv;
struct FILLINSTRUCT *af;
int nstructs;
Scrn_parms *pp;
Scrn_desc *dp;
Scrn_struct *sp;
{
	ENTERFUNC("acdka_free");
	free_cw_table (af->auths);
	Free (af->ker_st);
	Free (af->bas_st);
	EXITFUNC("acdka_free");
	return 1;
}

int acauka_exit ()
{
}

int acduka_exit()
{
}

#define SETUPFUNC	acdka_setup	/* defined by stemplate.c */
#define AUTHFUNC	acdka_auth
#define BUILDFILLIN	acdka_bfill

#define INITFUNC	acdka_init		/* defined by stemplate.c */
#define BUILDSTRUCT	acdka_bstruct

#define ROUTFUNC	acdka_exit		/* defined by stemplate.c */
#define VALIDATE	acdka_valid
#define SCREENACTION	acdka_action

#define FREEFUNC	acdka_free		/* defined by stemplate.c */
#define FREESTRUCT	acdka_do_free

#include "stemplate.c"
