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
static char *rcsid = "@(#)$RCSfile: acc_scrns.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:11:06 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	acc_scrns.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:52:29  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  17:50:27  hosking]
 *
 * Revision 1.1.2.4  1992/04/05  18:19:16  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:51:39  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * acc_scrns.c - secondary POSIX ACL Editor screen
 *
 * This screen will display the "Test Access" elements.
 */

/* #ident "@(#)acc_scrns.c	1.1 11:15:45 11/8/91 SecureWare" */

#include <sys/secdefines.h>
#include "If.h"
#include "AIf.h"
#include "scrn_local.h"
#include "paclif.h"


/***********************************************************

	MAIN MENU

***********************************************************/

Scrn_desc	acc_desc[] = {
/* row, col, type, len, inout, required, prompt, help */
#define ACC_OBJNAME_TITLE_DESC      	0
	{ 4, 2,  FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Username:"},
#define ACC_OBJNAME_DESC		1
	{ 4, 12,  FLD_ALPHA,  NUSERNAME,      FLD_BOTH,   NO, NULL,
		  "acc_test,username.hlp"},
#define ACC_GROUP_LIST_TITLE_DESC	2	
	{ 4, 30, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Group:"},
#define ACC_GROUP_LIST_DESC		3	
	{ 4, 38, FLD_SCROLL, NGROUPNAME, FLD_BOTH,   NO, NULL, 
		  "acc_test,group.hlp", 8,0,1 },
#define ACC_PERMS_TITLE_DESC		4	
	{ 4, 50, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Permissions:"},
#define ACC_PERMS_DESC			5	
	{ 4, 65, FLD_ALPHA,  3,       FLD_OUTPUT,   NO },
#define ACC_LOAD_GROUPS_BUTTON_DESC	6	
	{14, 10, FLD_CHOICE, 11,       FLD_BOTH,   NO, "Load Groups",
		 "acc_test,load.hlp"},
#define ACC_RESET_ID_BUTTON_DESC	7	
	{14, 30, FLD_CHOICE, 14,       FLD_BOTH,   NO, "Reset Identity",
		 "acc_test,reset.hlp"},
#define ACC_PERMISSIONS_BUTTON_DESC	8	
	{14, 50, FLD_CHOICE, 22,       FLD_BOTH,   NO, 
		 "Determine Permissions", "acc_test,determine.hlp" },
};

#define FIRSTDESC 	ACC_OBJNAME_DESC


int	acc_valid(), acc_button(), acc_exit(), acc_setup(), 
	acc_init(), acc_free();

int	noop_validate(), do_reset_ids(), do_load_groups(), do_permissions();

/* 
 * Note that the CHOICE (button) fields do not have scrn_struct spaces
 * allocated automatically, as all of the other fields do.  Thus, 
 * we just mark them as NA, rather than setting their place in the
 * array.  [I'd hate to admit how long it took me to discover this!]
 */


Menu_scrn	acc_menu[] = {
#define ACC_OBJNAME_STRUCT		0	
	ROUT_DESC (1, noop_validate, 0, NULL),
#define ACC_GROUP_LIST_STRUCT		1
	ROUT_DESC (2, noop_validate, 0, NULL),
#define ACC_PERMS_STRUCT		2
	ROUT_DESC (3, noop_validate, 0, NULL),
#define ACC_LOAD_GROUPS_BUTTON_STRUCT	3
	ROUT_DESC (4, do_load_groups, 0, NULL),
#define ACC_RESET_IDS_BUTTON_STRUCT     4
	ROUT_DESC (5, do_reset_ids, 0, NULL),
#define ACC_PERMISSIONS_BUTTON_STRUCT   5
	ROUT_DESC (6, do_permissions, 0, NULL),
};

#define NSCRNSTRUCT	6


Scrn_hdrs acc_hdrs= {
	"Test POSIX ACL Subject Access",
	cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT,"SPACE=Select",
	"RET=Execute  ESC=Leave Screen  ^Y=Item help  ^B=Quit Program",
	"paclif,acc.main"
};

SCRN_PARMF (acc_scrn, SCR_FILLIN, acc_desc, acc_menu,
	NULL, &acc_hdrs, acc_setup, acc_init, acc_free);

/*
 * Print screens for role program, depending on whether this is ISSO or
 * System Administrator interface.
 */


struct acc_ids	acc_stuff, *acc_fill = &acc_stuff;
struct paclif	*pacl_fill;

static int acc_auth();
static int acc_bfill();
static int acc_bstruct();
static int acc_valid();
static int acc_action();
static void do_free();

#define PARMTEMPLATE	acc_scrn
#define STRUCTTEMPLATE	acc_struct
#define DESCTEMPLATE	acc_desc
#define FILLINSTRUCT	acc_ids
#define FILLIN		acc_fill

static Scrn_struct	*acc_struct;

static int	groups_list_action();

static int
acc_auth(argv, fill)
	char **argv;
	struct acc_ids *fill;
{
	return 0; /* okay */
}

/*
 * set the fillin structure with command authorization names and the
 * current state of the default command authorizations from the
 * database
 */

static int
acc_bfill(fill)
	struct acc_ids *fill;
{
	fill->username [0] = 0;
	fill->number_groups = 1;
	fill->group = alloc_cw_table (1, NGROUPNAME);
	fill->perms [0] = 0;

	reset_ids (fill);

	return 0;
}

/*
 * Set up the fillin structure with the command authorizations
 */

static int
acc_bstruct(fill, sptemplate)
	struct acc_ids	*fill;
	struct scrn_struct **sptemplate;
{
	register struct scrn_struct *sp;

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	/* Set the scrn_struct elements for this screen. */ 

	sp[ACC_OBJNAME_STRUCT].pointer = fill->username;
	sp[ACC_OBJNAME_STRUCT].filled = 1;
	sp[ACC_GROUP_LIST_STRUCT].pointer = (char *) fill->group;
	sp[ACC_GROUP_LIST_STRUCT].filled = fill->number_groups;
	sp[ACC_GROUP_LIST_STRUCT].val_act = groups_list_action;
	sp[ACC_PERMS_STRUCT].pointer = fill->perms;
	sp[ACC_PERMS_STRUCT].filled = 1;

	return 0;
}

/*
 * These routines are called when a CHOICE is activated.
 * They should set various fields on the screen, and redisplay.
 */

/* noop_validate is defined in pacl_scrns.c */

do_reset_ids()
{
	reset_ids (acc_fill);
	acc_struct [ACC_OBJNAME_STRUCT].changed = 1;
	return CONTINUE;
}

do_load_groups()
{
	load_groups (acc_fill);
	acc_struct [ACC_GROUP_LIST_STRUCT].pointer = (char *) acc_stuff.group;
	acc_struct [ACC_GROUP_LIST_STRUCT].filled = acc_stuff.number_groups;
	acc_struct [ACC_GROUP_LIST_STRUCT].changed = 1;
	return CONTINUE;
}

do_permissions ()
{
	determine_access (acc_fill, pacl_fill);
}

/*
 * The action routines for the lists will expand the lists as the
 * user makes additional entries.  This allows an infinitely large 
 * field.
 * 
 * They will also check to make sure that the user has made an entry
 * in a valid spot.
 */

static int
groups_list_action()
{
	int i, j;
	int changed = 0;
	char **temp;


	/* only expand if the user just typed in the last blank */
 
	if (acc_stuff.group [acc_stuff.number_groups - 1][0] != 0) {
		temp = expand_cw_table (acc_stuff.group, 
					acc_stuff.number_groups,
					acc_stuff.number_groups + 1, 
					NGROUPNAME + 1);
		acc_stuff.group = temp;
		acc_stuff.number_groups++;

		for (i = 0; i < acc_stuff.number_groups - 2; i++)
			if (acc_stuff.group [i][0] == 0)
				strcpy (acc_stuff.group [i], "   ");
		changed = 1;
	} else { /* check for removed strings */
		for (i = acc_stuff.number_groups - 2; i >= 0; i--) {
			if (acc_stuff.group [i][0] == 0) {
				for (j = i; j < acc_stuff.number_groups - 1; j++) {
					strcpy (acc_stuff.group [j],
						acc_stuff.group [j + 1]);
				}
				changed = 1;
				acc_stuff.number_groups--;
			}
		}
	}

	if (changed) {
		acc_struct [ACC_GROUP_LIST_STRUCT].pointer = 
			(char *) acc_stuff.group;
		acc_struct [ACC_GROUP_LIST_STRUCT].filled = 
			acc_stuff.number_groups;
		return 1;
	}

	return 0;
}


/*
 * validate function for the whole screen.
 */

static int
acc_valid(argv, fill)
	char **argv;
	struct acc_ids *fill;
{
	return 0;
}

/*
 * action function for the whole screen.
 */

static int
acc_action(fill)
	struct acc_ids *fill;
{
	char **table;
	int i;
	int ret;
	int count = 0;

	return 1;
}

/*
 * routine to free memory.
 */

static void
do_free(argv, fill, nstructs, pp, dp, sp)
	char **argv;
	struct acc_ids *fill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	
}

#define SETUPFUNC	acc_setup		/* defined by stemplate.c */
#define AUTHFUNC	acc_auth
#define BUILDFILLIN	acc_bfill

#define INITFUNC	acc_init		/* defined by stemplate.c */
#define BUILDSTRUCT	acc_bstruct

#define ROUTFUNC	acc_exit		/* defined by stemplate.c */
#define VALIDATE	acc_valid
#define SCREENACTION	acc_action

#define FREEFUNC	acc_free		/* defined by stemplate.c */
#define FREESTRUCT	do_free

#include "stemplate.c"
