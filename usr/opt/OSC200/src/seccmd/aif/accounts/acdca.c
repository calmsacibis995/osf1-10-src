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
static char	*sccsid = "@(#)$RCSfile: acdca.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:52:51 $";
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
 *	acdca.c - set account default command auths
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
int acdca_setup(), acdca_init(), acdca_free(), acdca_exit();

/*
 * acdca screen variables
 */

extern Scrn_parms acdca_scrn;
extern Scrn_desc  acdca_desc[];
extern Menu_scrn  *acdca_menu;
static Scrn_struct *acdca_struct, *asp;
extern Scrn_hdrs acdca_hdrs;

#define PARMTEMPLATE	acdca_scrn
#define STRUCTTEMPLATE	acdca_struct
#define DESCTEMPLATE	acdca_desc
#define FILLINSTRUCT	acdca_fillin
#define FILLIN		acdcaP


#define FIRSTDESC	2
#define NSCRNSTRUCT	1

/*
 * External declarations
 */

extern char *Malloc(), *Calloc();
extern char **alloc_table();

extern void LoadMessage();

static char
	**msg_error,
	*msg_error_text;


/*
 * Fillin structure - most fields named for pr_default fields
 */

static struct acdca_fillin {
	char **auths;
	char *states;
	int nstructs;
} acdca, *acdcaP = &acdca;

static struct sdef_if sds;	/* system defaults superstructure */

static struct pr_default *sd = &sds.df;	/* system defaults */


/*
 * Fill in the scrn_struct
 */

static int
acdca_bstruct(lf, sptemplate)
	struct acdca_fillin *lf;
	struct scrn_struct **sptemplate;
{
	register struct scrn_struct *sp;
	register int i;
	register char *p;

	ENTERFUNC("acdca_bstruct");
	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;
	if (sp) {
		asp = sp;
		sp->pointer = (char *) acdcaP->auths;
		sp->state = acdcaP->states;
		sp->filled = acdcaP->nstructs;
	}
	EXITFUNC("acdca_bstruct");
	return 0;
}


/*
 * Build the fillin structure.
 * The requirements for this routine were completed at the top level.
 */

static int
acdca_bfill(lf)
	struct acdca_fillin *lf;
{
	register int i;

	ENTERFUNC("acdca_bfill");
	XGetSystemInfo(&sds);

	/* Read the subsytem names in */

	build_cmd_priv();
	acdcaP->nstructs = total_auths();

	/* Can't read tables */

	if (acdcaP->nstructs == -1) {
		if (! msg_error)
			LoadMessage("msg_accounts_subsystems_error", 
				&msg_error, &msg_error_text);
		SystemErrorMessageOpen(-1, msg_error, 0, NULL);
	}

	if (!(acdcaP->auths =
		(char **) alloc_table (cmd_priv, acdcaP->nstructs)))
			MemoryError ();

	for (i = 0; i< acdcaP->nstructs; i++) {
		strcpy (acdcaP->auths[i], cmd_priv[i].name);
	}

	/* now build the state table */

	acdcaP->states = Calloc(acdcaP->nstructs, sizeof (char));
	if (acdcaP->states == NULL)
			MemoryError();

	if (sd->prg.fg_cprivs)
		for (i=0;i<acdcaP->nstructs;i++)
			if (ISBITSET (sd->prd.fd_cprivs, cmd_priv[i].value))
			        acdcaP->states[i] = 1;

	EXITFUNC("acdca_bfill");
	return 0;
}

/*
 * Decide whether the user is authorized to perform the function.
 * Nothing to decide here.
 */

static int
acdca_auth(argv, lf)
	char **argv;
	struct acdca_fillin *lf;
{
	return 0;
}

/*
 * validate function for the whole screen.
 */

static int
acdca_valid(argv, lf)
	char **argv;
	struct acdca_fillin *lf;
{
	int ret = 0;

	ENTERFUNC("acdca_valid");
	EXITFUNC("acdca_valid");
	return ret;
}

/*
 * action function for the whole screen.
 */

static int
acdca_action(lf)
	struct acdca_fillin *lf;
{
	char **table;
	int i;
	int ret;
	int count = 0;

	ENTERFUNC("acdca_action");

	for (i = 0; i < acdcaP->nstructs; i++)
		if (acdcaP->states[i])
			count++;
	table = alloc_cw_table(count, widest_auth() + 1);

	if (table == (char **) 0)
		MemoryError();

	sd->prg.fg_cprivs = 1;

	count = 0;
	for (i = 0; i < acdcaP->nstructs; i++) {
		if (acdcaP->states[i]) {
			ADDBIT (sd->prd.fd_cprivs, cmd_priv[i].value);
			strcpy (table[count++], cmd_priv[i].name);
		}
		else
			RMBIT (sd->prd.fd_cprivs, cmd_priv[i].value);
	}

	/* Write information */
	ret = XWriteSystemInfo(&sds);

	/* For authorizations we must also write to authorizations file */

	if (ret == SUCCESS)
		ret = write_authorizations ((char *) 0, table, count);

	free_cw_table(table);

	return (ret);

	EXITFUNC("acdca_action");
	return CONTINUE;
}

/*
 * routine to free memory.
 */

static void
do_free()
{
	ENTERFUNC("do_free");
	if (acdcaP->auths != (char **) 0) {
		free_cw_table(acdcaP->auths);
		acdcaP->auths = (char **) 0;
	}

	if (acdcaP->states != NULL) {
		free(acdcaP->states);
		acdcaP->states = NULL;
	}
	EXITFUNC("do_free");
}

#define SETUPFUNC	acdca_setup	/* defined by stemplate.c */
#define AUTHFUNC	acdca_auth
#define BUILDFILLIN	acdca_bfill

#define INITFUNC	acdca_init		/* defined by stemplate.c */
#define BUILDSTRUCT	acdca_bstruct

#define ROUTFUNC	acdca_exit		/* defined by stemplate.c */
#define VALIDATE	acdca_valid
#define SCREENACTION	acdca_action

#define FREEFUNC	acdca_free		/* defined by stemplate.c */
#define FREESTRUCT	do_free

#include "stemplate.c"

#endif /* } SEC_BASE */
