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
static char	*sccsid = "@(#)$RCSfile: acauca.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:52:35 $";
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
 *	acauca.c - set/mod/disp user account command auths
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

/*
 * action routine declarations
*/
int acauca_setup(), acauca_init(), acauca_free(), acauca_exit();

/*
 * acauca screen variables
 */

extern Scrn_parms acauca_scrn;
extern Scrn_desc  acauca_desc[];
extern Menu_scrn  *acauca_menu;
Scrn_struct *acauca_struct, *asp;
extern Scrn_hdrs acauca_hdrs;

#define PARMTEMPLATE	acauca_scrn
#define STRUCTTEMPLATE	acauca_struct
#define DESCTEMPLATE	acauca_desc
#define FILLINSTRUCT	acauca_fillin
#define FILLIN		acaucaP

#define FIRSTDESC	19
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
 * Fillin structure
 */

struct FILLINSTRUCT {
	char **auths;		/* list of auth names */
	char *set_st;		/* "Set" state - 1 or 0 */
	char *def_st;		/* "Def" state - 1 or 0 */
	int nstructs;		/* total auths in list */
	int npercol1;		/* auths per col 1 */
	int npercol2;		/* auths per col 2 */
	int npercol3;		/* auths per col 3 */
} acauca, *acaucaP = &acauca;

static struct prpw_if pru;	/* protected passwd entry if */

static struct pr_passwd *prpwd = &pru.prpw;	/* prpw entry */


/*
 * Fill in the scrn_struct
 */

int
acauca_bstruct(af, sptemplate)
	struct FILLINSTRUCT *af;
	struct scrn_struct **sptemplate;
{
	register struct scrn_struct *sp;
	register int i;
	register char *p;

	ENTERFUNC("acauca_bstruct");
	sp = PARMTEMPLATE.ss;
	*sptemplate = PARMTEMPLATE.ss;
	if (sp) {
		asp = sp;
		sp[0].pointer = (char *) af->auths;
		sp[0].filled = af->npercol1;
		sp[1].state = af->set_st;
		sp[1].filled = af->npercol1;
		sp[2].state = af->def_st;
		sp[2].filled = af->npercol1;

		sp[3].pointer = (char *) &af->auths[af->npercol1];
		sp[3].filled = af->npercol2;
		sp[4].state = &af->set_st[af->npercol1];
		sp[4].filled = af->npercol2;
		sp[5].state = &af->def_st[af->npercol1];
		sp[5].filled = af->npercol2;

		sp[6].pointer = (char *) &af->auths[af->npercol1 +
			af->npercol2];
		sp[6].filled = af->npercol3;
		sp[7].state = &af->set_st[af->npercol1 + af->npercol2];
		sp[7].filled = af->npercol3;
		sp[8].state = &af->def_st[af->npercol1 + af->npercol2];
		sp[8].filled = af->npercol3;
	}
	EXITFUNC("acauca_bstruct");
	return 0;
}


/*
 * Build the fillin structure.
 * The requirements for this routine were completed at the top level.
 */

int
acauca_bfill(af)
	struct FILLINSTRUCT *af;
{
	register int i;
	int ret;
	int extra;

	ENTERFUNC("acauca_bfill");
	if (!gl_user || pw_nametoid (gl_user) == (uid_t) -1) {
		nosuchuser (gl_user);
		return 1;
	}
	if (ret = XGetUserInfo (gl_user, &pru))
		return ret;

	/* Read the subsytem names in */

	build_cmd_priv ();
	af->nstructs = total_auths();
	af->nstructs = MIN (42, af->nstructs); 	/* FOR NOW */
	/* Can't read tables */
	if (af->nstructs == -1) {
		if (! msg_error)
			LoadMessage("msg_accounts_subsystems_error", 
				&msg_error, &msg_error_text);
		SystemErrorMessageOpen(-1, msg_error, 0, NULL);
	}
	if (!(af->auths =
		(char **) alloc_table (cmd_priv, af->nstructs)))
			MemoryError ();

	for (i = 0; i< af->nstructs; i++) {
		strcpy (af->auths[i], cmd_priv[i].name);
	}


	/* now build the state tables */

	if (!(af->set_st = Calloc (af->nstructs,
		sizeof (char))))
			MemoryError ();
	if (!(af->def_st = Calloc (af->nstructs,
		sizeof (char))))
			MemoryError ();

	/* set the defaults to the system defaults */

	for (i=0;i<af->nstructs;i++)
		if (ISBITSET (prpwd->sfld.fd_cprivs, cmd_priv[i].value))
		        af->def_st[i] = 1;
		else
			af->def_st[i] = 0;

	/* set the user auths (def if none) */
	if (prpwd->uflg.fg_cprivs)
		for (i=0;i<af->nstructs;i++)
			if (ISBITSET (prpwd->ufld.fd_cprivs,cmd_priv[i].value))
			        af->set_st[i] = 1;
			else
				af->set_st[i] = 0;
	else
		for (i=0;i<af->nstructs;i++)
			af->set_st[i] = af->def_st[i];

	/* determine the number of entries per column */

	af->npercol1 = af->npercol2 = af->npercol3 = (af->nstructs + 2) / 3;
	extra = af->nstructs % 3;
	if (extra) {
		af->npercol1++;
		af->npercol2 += (extra - 1);
	}


	EXITFUNC("acauca_bfill");
	return 0;
}

/*
 * Decide whether the user is authorized to perform the function.
 * Nothing to decide here.
 */

int
acauca_auth(argv, af)
	char **argv;
	struct FILLINSTRUCT *af;
{
	return 0;
}

/*
 * validate function for the whole screen.
 */

int
acauca_valid(argv, af)
	char **argv;
	struct FILLINSTRUCT *af;
{
	int ret = 0;

	ENTERFUNC("acauca_valid");
	EXITFUNC("acauca_valid");
	return ret;
}

/*
 * action function for the whole screen.
 */

static int
acauca_action(af)
	struct FILLINSTRUCT *af;
{
	char **table;
	int i,j,k;

	ENTERFUNC("acauca_action");

	/* Allocate space to store subsystem names */

	k = af->nstructs;
	if (!(table = (char **)alloc_table (cmd_priv, k)))
		MemoryError();

	/* Loop through all cmd privs. If all default then set user = default */

	j = 0;
	for (i=0; i<af->nstructs; i++) {
		if (af->def_st[i])
			j ++;
	}

	/* All defaults */
	if (j == af->nstructs) {
		prpwd->uflg.fg_cprivs = 0;
		strcpy (table[0], AUTH_DEFAULT);
		k = 1;
	}
	else {
		k = 0;
		prpwd->uflg.fg_cprivs = 1;

		for (i =0; i<af->nstructs; i++) {
			if (af->set_st[i]) {
			    ADDBIT (prpwd->ufld.fd_cprivs, cmd_priv[i].value);
			    strcpy (table[k++], cmd_priv[i].name);
		}
			else
			    RMBIT (prpwd->ufld.fd_cprivs, cmd_priv[i].value);
		}
	}

	/* Write information */

	j = XWriteUserInfo (&pru);

	/* For authorizations we must also write to authorizations file */

	if (j == SUCCESS)
		write_authorizations (prpwd->ufld.fd_name, table, k);

	free_cw_table (table);

	EXITFUNC("acauca_action");
	return j;
}


/*
 * routine to free memory.
 */

int
acauca_do_free(argv, af, nstructs, pp, dp, sp)
char **argv;
struct FILLINSTRUCT *af;
int nstructs;
Scrn_parms *pp;
Scrn_desc *dp;
Scrn_struct *sp;
{
	ENTERFUNC("acauca_free");
	free_cw_table (af->auths);
	Free (af->set_st);
	Free (af->def_st);
	EXITFUNC("acauca_free");
	return 1;
}

#define SETUPFUNC	acauca_setup	/* defined by stemplate.c */
#define AUTHFUNC	acauca_auth
#define BUILDFILLIN	acauca_bfill

#define INITFUNC	acauca_init		/* defined by stemplate.c */
#define BUILDSTRUCT	acauca_bstruct

#define ROUTFUNC	acauca_exit		/* defined by stemplate.c */
#define VALIDATE	acauca_valid
#define SCREENACTION	acauca_action

#define FREEFUNC	acauca_free		/* defined by stemplate.c */
#define FREESTRUCT	acauca_do_free

#include "stemplate.c"


#endif /* } SEC_BASE */
