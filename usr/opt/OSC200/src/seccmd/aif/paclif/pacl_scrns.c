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
static char *rcsid = "@(#)$RCSfile: pacl_scrns.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:12:54 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	pacl_scrns.c,v $
 * Revision 1.1.1.2  92/11/02  08:44:00  devrcs
 *  *** OSF1_1_1 version ***
 * 
 * Revision 1.1.4.2  1992/06/11  17:05:01  hosking
 * 	bug 6057: ANSI C changes to allow '-pedantic' to be enabled
 * 	[1992/06/11  16:59:47  hosking]
 *
 * Revision 1.1.2.3  1992/04/05  18:20:08  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:53:06  marquard]
 * 
 * $OSF_EndLog$
 */
/*
 * paclif_scrns.c - main POSIX ACL Editor screen
 * @(#)pacl_scrns.c	1.1 11:17:41 11/8/91 SecureWare
 * Copyright (c) 1991, SecureWare, Inc.  All rights reserved.
 */


/* #ident "@(#)pacl_scrns.c	1.1 11:17:41 11/8/91 SecureWare" */

#include <sys/secdefines.h>
#include "If.h"
#include "AIf.h"
#include "scrn_local.h"
#include "paclif.h"

#define MASK_ON   "Mask Auto Calculate On"
#define MASK_OFF  "Mask Auto Calculate Off"

/***********************************************************

	MAIN MENU

***********************************************************/

Scrn_desc	pacl_desc[] = {
/* row, col, type, len, inout, required, prompt, help */
#define PACL_OBJNAME_TITLE_DESC      	0
	{ 1, 2,  FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Name:"},
#define PACL_OBJNAME_DESC		1
	{ 1, 8,  FLD_ALPHA,  40,      FLD_BOTH,   NO, NULL, "paclif,name.hlp"},
#define PACL_GET_ACL_BUTTON_DESC	2	
	{ 3, 2, FLD_CHOICE, 7,        FLD_BOTH,   NO, "Get ACL", 
		  "paclif,get_acl.hlp" },
#define PACL_SET_ACL_BUTTON_DESC	3	
	{ 3, 17, FLD_CHOICE, 7,        FLD_BOTH,   NO, "Set ACL",
	  	  "paclif,set_acl.hlp" },
#define PACL_GET_DFLT_BUTTON_DESC	4	
	{ 4, 2, FLD_CHOICE, 11,        FLD_BOTH,   NO, "Get Default",
		  "paclif,get_default.hlp"},
#define PACL_SET_DFLT_BUTTON_DESC	5	
	{ 4, 17, FLD_CHOICE, 11,        FLD_BOTH,   NO, "Set Default", 
		  "paclif,set_default.hlp" },
#define PACL_DELETE_DFLT_BUTTON_DESC	6	
	{ 4, 32, FLD_CHOICE, 14,       FLD_BOTH,   NO, "Delete Default", 
		  "paclif,delete_default.hlp" },
#define PACL_TEST_ACCESS_BUTTON_DESC	7	
	{ 4, 60, FLD_CHOICE, 16,       FLD_BOTH,   NO, "Test Access",
		  "paclif,test_access.hlp"},
#define PACL_CRE_USER_TITLE_DESC	8
	{ 6, 2,  FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Creator User:"},
#define PACL_CRE_USER_DESC		9	
	{ 6, 16, FLD_ALPHA,  8,       FLD_OUTPUT,   NO },
#define PACL_CRE_GRP_TITLE_DESC		10
	{ 6, 27, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Creator Group:"},
#define PACL_CRE_GRP_DESC		11
	{ 6, 42, FLD_ALPHA,  8,       FLD_OUTPUT,   NO },
#define PACL_OWN_USER_TITLE_DESC	12
	{ 7, 2,  FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Owner User:"},
#define PACL_OWN_USER_DESC		13
	{ 7, 16, FLD_ALPHA,  8,       FLD_BOTH,   NO, NULL,
		  "paclif,owner_user.hlp" },
#define PACL_OWN_GRP_TITLE_DESC		14
	{ 7, 27, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Owner Group:"},
#define PACL_OWN_GRP_DESC		15	
	{ 7, 42, FLD_ALPHA,  8,       FLD_BOTH,   NO, NULL,
		  "paclif,owner_group.hlp" },
#define PACL_SET_OWNER_BUTTON_DESC	16
	{ 6, 54,  FLD_CHOICE, 9,       FLD_BOTH,   NO, "Set Owner",
		  "paclif,set_owner.hlp"},
#define PACL_SET_GROUP_BUTTON_DESC	17
	{ 6, 65, FLD_CHOICE, 9,       FLD_BOTH,   NO,"Set Group",
		  "paclif,set_group.hlp"},
#define PACL_SET_OWNGRP_BUTTON_DESC	18
	{ 7, 58, FLD_CHOICE, 15,      FLD_BOTH,   NO,"Set Owner/Group",
		"paclif,set_both.hlp" },
#define PACL_OWN_PERMS_TITLE_DESC	19	
	{ 9,  2, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Owner Perms:"},
#define PACL_OWN_PERMS_DESC		209	
	{ 9, 15, FLD_ALPHA,  3,       FLD_BOTH,   NO, NULL, 
		  "paclif,owner_perms.hlp" },
#define PACL_MASK_PERMS_TITLE_DESC	21	
	{ 9, 20, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Mask Perms:"},
#define PACL_MASK_PERMS_DESC		22	
	{ 9, 33, FLD_ALPHA,  3,       FLD_BOTH,   NO, NULL,
		  "paclif,mask_perms.hlp"},
#define PACL_OTH_PERMS_TITLE_DESC	23		
	{ 9, 38, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Other Perms:"},
#define PACL_OTH_PERMS_DESC		24			
	{ 9, 51, FLD_ALPHA,  3,       FLD_BOTH,   NO, NULL,
		  "paclif,other_perms.hlp"},
#define PACL_GRP_PERMS_TITLE_DESC	25		
	{10, 20, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Group Perms:"},
#define PACL_GRP_PERMS_DESC		26			
	{10, 33, FLD_ALPHA,  3,       FLD_BOTH,   NO, NULL,
		 "paclif,group_perms.hlp"},
#define PACL_GRP_EFF_TITLE_DESC		27		
	{10, 38, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Eff:"},
#define PACL_GRP_EFF_DESC		28			
	{10, 43, FLD_ALPHA,  3,       FLD_OUTPUT,   NO },
#define PACL_SET_MODE_BUTTON_DESC	29	
	{ 9, 60, FLD_CHOICE, 9,       FLD_BOTH,   NO, "Set Mode",
		 "paclif,set_mode.hlp"},
#define PACL_USER_LIST_TITLE_DESC	30
	{12,  2, FLD_PROMPT, 0,       FLD_OUTPUT, NO, 
           "Username Prm Eff"},
#define PACL_GROUP_LIST_TITLE_DESC	31	
	{12, 31, FLD_PROMPT, 0,       FLD_OUTPUT, NO, 
           "Group    Prm Eff"},
#define PACL_USER_LIST_DESC		32	
	{13,  2, FLD_SCROLL, 16,       FLD_BOTH,   NO, NULL, 
		 "paclif,user.hlp", 3, 0, 1 },
#define PACL_GROUP_LIST_DESC		33	
	{13, 31, FLD_SCROLL, 16,       FLD_BOTH,   NO, NULL, 
		 "paclif,group.hlp", 3, 0, 1 },
#define PACL_SET_BASE_BUTTON		34
	{12, 54, FLD_CHOICE, 20,       FLD_BOTH,   NO, "Restrict to Base ACL",
		 "paclif,set_base_acl.hlp"},
#define PACL_COMPUTE_MASK_BUTTON_DESC	35	
	{13, 54, FLD_CHOICE, 12,       FLD_BOTH,   NO, "Compute Mask",
		 "paclif,compute_mask.hlp"},
#define PACL_PURGE_INEF_BUTTON_DESC	36	
	{14, 54, FLD_CHOICE, 18,       FLD_BOTH,   NO, "Purge Ineffectives",
		 "paclif,purge.hlp"},
#define PACL_MASK_CALC_TOGGLE_DESC	37	
	{15, 54, FLD_TOGGLE, sizeof (MASK_ON), FLD_INPUT, NO, MASK_ON,
		 "paclif,mask_auto.hlp"},
};

#define FIRSTDESC 	PACL_OBJNAME_DESC


int	pacl_valid(), pacl_button(), pacl_exit(), pacl_setup(), 
	pacl_init(), pacl_free();

int	noop_validate(), do_set_owner(), do_set_group(), do_set_owngrp(),
	do_set_mode(), do_set_acl(), do_delete_acl(), do_set_dflt(), 
	do_get_acl(), do_get_dflt(), do_delete_dflt(), do_compute_mask(),
	do_purge_ineff(), do_no_mask(), do_get_grps(), do_get_id(), 
	do_show_perms(), do_set_base();

extern Scrn_parms acc_scrn;
/* 
 * Note that the CHOICE (button) fields do not have scrn_struct spaces
 * allocated automatically, as all of the other fields do.  Thus, 
 * we just mark them as NA, rather than setting their place in the
 * array.  [I'd hate to admit how long it took me to discover this!]
 */


Menu_scrn	pacl_menu[] = {
#define PACL_OBJNAME_STRUCT		0	
	ROUT_DESC (1, noop_validate, 0, NULL),
#define PACL_GET_ACL_BUTTON_STRUCT	NA	
	ROUT_DESC (2, do_get_acl, 0, NULL),
#define PACL_SET_ACL_BUTTON_STRUCT	NA
	ROUT_DESC (3, do_set_acl, 0, NULL),
#define PACL_GET_DFLT_BUTTON_STRUCT	NA	
	ROUT_DESC (4, do_get_dflt, 0, NULL),
#define PACL_SET_DFLT_BUTTON_STRUCT	NA	
	ROUT_DESC (5, do_set_dflt, 0, NULL),
#define PACL_DELETE_DFLT_BUTTON_STRUCT	NA	
	ROUT_DESC (6, do_delete_dflt, 0, NULL),
#define PACL_TEST_ACCESS_BUTTON_STRUCT	NA		
	MENU_DESC (7, &acc_scrn, NULL),
#define PACL_CRE_USER_STRUCT		1		
	ROUT_DESC (8, noop_validate, 0, NULL),
#define PACL_CRE_GRP_STRUCT		2	
	ROUT_DESC (9, noop_validate, 0, NULL),
#define PACL_OWN_USER_STRUCT		3	
	ROUT_DESC (10, noop_validate, 0, NULL),
#define PACL_OWN_GRP_STRUCT		4	
	ROUT_DESC (11, noop_validate, 0, NULL),
#define PACL_SET_OWNER_BUTTON_STRUCT	NA
	ROUT_DESC (12, do_set_owner, 0, NULL),
#define PACL_SET_GROUP_BUTTON_STRUCT	NA
	ROUT_DESC (13, do_set_group, 0, NULL),
#define PACL_SET_OWNGRP_BUTTON_STRUCT	NA
	ROUT_DESC (14, do_set_owngrp, 0, NULL),
#define PACL_OWN_PERMS_STRUCT		5	
	ROUT_DESC (15, noop_validate, 0, NULL),
#define PACL_MASK_PERMS_STRUCT		6	
	ROUT_DESC (16, noop_validate, 0, NULL),
#define PACL_OTH_PERMS_STRUCT		7				
	ROUT_DESC (17, noop_validate, 0, NULL),
#define PACL_GRP_PERMS_STRUCT		8			
	ROUT_DESC (18, noop_validate, 0, NULL),
#define PACL_GRP_EFF_STRUCT		9			
	ROUT_DESC (19, noop_validate, 0, NULL),
#define PACL_SET_MODE_BUTTON_STRUCT	NA			
	ROUT_DESC (20, do_set_mode, 0, NULL),
#define PACL_USER_LIST_STRUCT		10	
	ROUT_DESC (21, noop_validate, 0, NULL),
#define PACL_GROUP_LIST_STRUCT		11	
	ROUT_DESC (22, noop_validate, 0, NULL),
#define PACL_SET_BASE_BUTTON_STRUCT	NA
	ROUT_DESC (23, do_set_base, 0, NULL),
#define PACL_COMPUTE_MASK_BUTTON_STRUCT	NA
	ROUT_DESC (24, do_compute_mask, 0, NULL),
#define PACL_PURGE_INEF_BUTTON_STRUCT	NA	
	ROUT_DESC (25, do_purge_ineff, 0, NULL),
#define PACL_MASK_CALC_TOGGLE_STRUCT 	12
	ROUT_DESC (26, do_no_mask, 0, NULL),
};

#define NSCRNSTRUCT	13


Scrn_hdrs pacl_hdrs= {
	"POSIX ACL Editor",
	cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT,"SPACE=Select",
	"RET=Execute  ESC=Leave Program  ^Y=Item help  ^B=Quit Program",
	"paclif,paclif.main"
};

SCRN_PARMF (pacl_scrn, SCR_FILLIN, pacl_desc, pacl_menu,
	NULL, &pacl_hdrs, pacl_setup, pacl_init, pacl_free);

Scrn_parms *TopScrn = &pacl_scrn;

/*
 * Print screens for role program, depending on whether this is ISSO or
 * System Administrator interface.
 */


struct paclif	pacl_stuff, *pacl_fill = &pacl_stuff;
static char   ToggleValue;

static int pacl_auth();
static int pacl_bfill();
static int pacl_bstruct();
static int pacl_valid();
static int pacl_action();
static void do_free();

#define PARMTEMPLATE	pacl_scrn
#define STRUCTTEMPLATE	pacl_struct
#define DESCTEMPLATE	pacl_desc
#define FILLINSTRUCT	paclif
#define FILLIN		pacl_fill

static Scrn_struct	*pacl_struct;

static int	users_list_action(), groups_list_action(), 
		subattrs_users_list_action(), group_eff_action(),
		mask_action(), user_action(), others_action();

static int
pacl_auth(argv, fill)
	char **argv;
	struct paclif *fill;
{
	return 0; /* okay */
}

/*
 * set the fillin structure with command authorization names and the
 * current state of the default command authorizations from the
 * database
 */

static int
pacl_bfill(fill)
	struct paclif *fill;
{
	fill->filename [0] = 0;
	fill->mask_recalculate = 1;

	init_paclif (fill);

	return 0;
}

/*
 * Set up the fillin structure with the command authorizations
 */

static int
pacl_bstruct(fill, sptemplate)
	struct paclif	*fill;
	struct scrn_struct **sptemplate;
{
	register struct scrn_struct *sp;

	ToggleValue = 0;

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	/* Set the scrn_struct elements; for this screen.          */ 

	sp[PACL_OBJNAME_STRUCT].pointer = fill->filename;
	sp[PACL_OBJNAME_STRUCT].filled = 1;
	sp[PACL_CRE_USER_STRUCT].pointer = fill->creator_name;
	sp[PACL_CRE_USER_STRUCT].filled = 1;
	sp[PACL_CRE_GRP_STRUCT].pointer = fill->creator_group;
	sp[PACL_CRE_GRP_STRUCT].filled = 1;
	sp[PACL_OWN_USER_STRUCT].pointer = fill->owner_name;
	sp[PACL_OWN_USER_STRUCT].filled = 1;
	sp[PACL_OWN_GRP_STRUCT].pointer = fill->owner_group;
	sp[PACL_OWN_GRP_STRUCT].filled = 1;
	sp[PACL_OWN_PERMS_STRUCT].pointer = fill->user_perms;
	sp[PACL_OWN_PERMS_STRUCT].filled = 1;
	sp[PACL_OWN_PERMS_STRUCT].val_act = user_action;
	sp[PACL_MASK_PERMS_STRUCT].pointer = fill->mask_perms;
	sp[PACL_MASK_PERMS_STRUCT].filled = 1;
	sp[PACL_MASK_PERMS_STRUCT].val_act = mask_action;
	sp[PACL_OTH_PERMS_STRUCT].pointer = fill->other_perms;
	sp[PACL_OTH_PERMS_STRUCT].filled = 1;
	sp[PACL_OTH_PERMS_STRUCT].val_act = others_action;
	sp[PACL_GRP_PERMS_STRUCT].pointer = fill->group_perms;
	sp[PACL_GRP_PERMS_STRUCT].filled = 1;
	sp[PACL_GRP_PERMS_STRUCT].val_act = group_eff_action;
	sp[PACL_GRP_EFF_STRUCT].pointer = fill->group_eff;
	sp[PACL_GRP_EFF_STRUCT].filled = 1;
	sp[PACL_USER_LIST_STRUCT].pointer = (char *) fill->user;
	sp[PACL_USER_LIST_STRUCT].filled = fill->number_users;
	sp[PACL_USER_LIST_STRUCT].val_act = users_list_action;
	sp[PACL_GROUP_LIST_STRUCT].pointer = (char *) fill->group;
	sp[PACL_GROUP_LIST_STRUCT].filled = fill->number_groups;
	sp[PACL_GROUP_LIST_STRUCT].val_act = groups_list_action;
	sp[PACL_MASK_CALC_TOGGLE_STRUCT].pointer = &ToggleValue;
	sp[PACL_MASK_CALC_TOGGLE_STRUCT].filled = 1;
	sp[PACL_MASK_CALC_TOGGLE_STRUCT].state = 0;
	sp[PACL_MASK_CALC_TOGGLE_STRUCT].val_act = do_no_mask;

	return 0;
}

/* Validation routine for all of the ALPHA fields */

noop_validate()
{
	return CONTINUE;
}

/*
 * These routines are called when a CHOICE is activated.
 * They should set various fields on the screen, and redisplay.
 */

char	**msg_chown, *msg_chown_text;

do_set_owner()
{
	perform_chown (pacl_fill->filename,
			      pacl_fill->owner_name,
			      NULL);
	return CONTINUE;
}

do_set_group()
{
	perform_chown (pacl_fill->filename,
			      NULL,
			      pacl_fill->owner_group);
	return CONTINUE;
}

do_set_owngrp()
{
	perform_chown (pacl_fill->filename,
			      pacl_fill->owner_name,
			      pacl_fill->owner_group);
	return CONTINUE;
}

/*
 * Set the file permissions using the USER_OBJ, OTHER_OBJ, and either 
 * the MASK_OBJ (if it is present) or the GROUP_OBJ permissions.
 */

do_set_mode()
{
	set_mode (pacl_fill);
	return CONTINUE;
}

char	**msg_acl_read, *msg_acl_read_text;

do_set_acl()
{
	return (write_paclif (pacl_fill, 0));
}

do_set_dflt()
{
	return (write_paclif (pacl_fill, PACLIF_DEFAULT_ENTRY));
}

do_get_acl()
{
	int	i;

	if (read_paclif (pacl_fill, pacl_fill->filename, 0) == 0) {
		for (i = 0; i < NSCRNSTRUCT; i++)
			pacl_struct [i].changed = 1;
	} 

	if (isblank (pacl_fill->mask_perms) && pacl_fill->mask_recalculate) {
		pacl_fill->mask_recalculate = 0;

		pacl_struct [PACL_GROUP_LIST_STRUCT].changed = 1;
		do_no_mask ();
		return 1;
	}

	return 0;
}

do_delete_acl()
{
}

do_get_dflt()
{
	int	i;

	if (read_paclif (pacl_fill, pacl_fill->filename, 
			 PACLIF_DEFAULT_ENTRY) == 0) {
		for (i = 0; i < NSCRNSTRUCT; i++)
			pacl_struct [i].changed = 1;
	} 
	return 0;
}

do_delete_dflt()
{
	return (delete_paclif (pacl_fill, PACLIF_DEFAULT_ENTRY));
}

do_compute_mask()
{
	compute_mask (pacl_fill);

	pacl_struct [PACL_USER_LIST_STRUCT].changed = 1;
	pacl_struct [PACL_GROUP_LIST_STRUCT].changed = 1;
	pacl_struct [PACL_MASK_PERMS_STRUCT].changed = 1;

	return 1;
}


do_purge_ineff()
{
	purge_all_ineffectives (pacl_fill);

	pacl_struct [PACL_USER_LIST_STRUCT].changed = 1;
	pacl_struct [PACL_GROUP_LIST_STRUCT].changed = 1;

	return 1;
}

/*
 * The "Mask Auto Calcualte On/Off" button requires a bit of gymnastics.
 */

do_no_mask()
{
	ToggleValue = 0;

	pacl_fill->mask_recalculate = 
		(strcmp (pacl_desc [PACL_MASK_CALC_TOGGLE_DESC].prompt, 
			 MASK_OFF) == 0);

	pacl_desc [PACL_MASK_CALC_TOGGLE_DESC].prompt = 
			pacl_fill->mask_recalculate ? MASK_ON : MASK_OFF;
	pacl_desc [PACL_MASK_CALC_TOGGLE_DESC].len = 
			pacl_fill->mask_recalculate ? 
				sizeof (MASK_ON) : sizeof (MASK_OFF);

	pacl_struct [PACL_MASK_CALC_TOGGLE_STRUCT].changed = 1;
	pacl_struct [PACL_MASK_CALC_TOGGLE_STRUCT].state = 0;

	return 1;
}

do_set_base ()
{
	perform_set_base (pacl_fill);

	pacl_struct [PACL_USER_LIST_STRUCT].changed = 1;
	pacl_struct [PACL_GROUP_LIST_STRUCT].changed = 1;
	pacl_struct [PACL_GRP_EFF_STRUCT].changed = 1;
	pacl_struct [PACL_MASK_PERMS_STRUCT].changed = 1;

	return 1;
}


/*
 * Action routines.  These are called on each keystroke in a particular
 * field.
 *
 */

/*
 * Handle the groups permissions field.  For each letter entered,
 * make sure that the mask will not be adversely effected.
 */

static int
group_eff_action ()
{
	int	perms = 0;

	correct_permissions (pacl_fill->group_perms,
			     pacl_fill->backing_group_perms);

	if (!isblank (pacl_fill->mask_perms)) {
		if (pacl_fill->mask_recalculate &&
		    check_mask (pacl_fill, 
				pacl_fill->group_perms, 
				pacl_fill->backing_group_perms) == -1) {
			strcpy (pacl_fill->group_perms,
				pacl_fill->backing_group_perms);
		}

		pacl_struct [PACL_USER_LIST_STRUCT].changed = 1;
		pacl_struct [PACL_GROUP_LIST_STRUCT].changed = 1;
		pacl_struct [PACL_GRP_EFF_STRUCT].changed = 1;
		pacl_struct [PACL_MASK_PERMS_STRUCT].changed = 1;

		set_effective (pacl_fill->group_perms, pacl_fill->group_eff,
			       pacl_fill->mask_perms);
	}

	strcpy (pacl_fill->backing_group_perms,
		pacl_fill->group_perms);

	pacl_struct [PACL_GRP_PERMS_STRUCT].changed = 1;
	
	return 1;
}

mask_action()
{
	int	i, perms = 0;

	pacl_struct [PACL_USER_LIST_STRUCT].changed = 1;
	pacl_struct [PACL_GROUP_LIST_STRUCT].changed = 1;

	if (pacl_fill->mask_recalculate) {
		pacl_fill->mask_recalculate = 0;
		do_no_mask ();
	}

	correct_permissions (pacl_fill->mask_perms,
			     pacl_fill->backing_mask_perms);

	set_all_effectives (pacl_fill);

	strcpy (pacl_fill->backing_mask_perms,
		pacl_fill->mask_perms);
	pacl_struct [PACL_MASK_PERMS_STRUCT].changed = 1;

	return 1;
}

user_action()
{
	correct_permissions (pacl_fill->user_perms,
			     pacl_fill->backing_user_perms);
	strcpy (pacl_fill->backing_user_perms, pacl_fill->user_perms);

	pacl_struct [PACL_OWN_PERMS_STRUCT].changed = 1;
	return 1;
}

others_action()
{
	int	perms = 0;

	correct_permissions (pacl_fill->other_perms, 
			     pacl_fill->backing_other_perms);
	strcpy (pacl_fill->backing_other_perms, pacl_fill->other_perms);

	pacl_struct [PACL_OTH_PERMS_STRUCT].changed = 1;
	return 1;
}


/*
 * The action routines for the lists will expand the lists as the
 * user makes additional entries.  This allows an infinitely large 
 * field.
 * 
 * When the user types an entry in the last line of the scrolling region,
 * the region is expanded.  When the user adds a line using the ^R key,
 * we determine that because the last line is written in, and we find
 * a blank line in the middle of the list of fields.   When the user
 * removes a line using ^F, we find a blank line in the middle, and 
 * we remember that we didn't have to make any changes to the length
 * of the list.
 * 
 * The routines will also check to make sure that the user has made an entry
 * in a valid spot.
 */

static int
users_list_action()
{
	int i, j;
	int changed = 0;
	char **temp;

	/* only expand if the user just typed in the last blank */

	if (pacl_stuff.user [pacl_stuff.number_users - 1][0]) {
		int	size = strlen (pacl_stuff.user 
				       [pacl_stuff.number_users - 1]);

		temp = expand_cw_table (pacl_stuff.user, 
					pacl_stuff.number_users,
					pacl_stuff.number_users + 1, 
					ENTRY_WIDTH + 1);
		pacl_stuff.user = temp;

		temp = expand_cw_table (pacl_stuff.backing_user, 
					pacl_stuff.number_users,
					pacl_stuff.number_users + 1, 
					ENTRY_WIDTH + 1);
		pacl_stuff.backing_user = temp;

		strcpy (pacl_stuff.backing_user [pacl_stuff.number_users],
			pacl_stuff.user [pacl_stuff.number_users]);

		pacl_stuff.number_users++;


		/* 
		 * Check to see if the expansion came because of an 
		 * addition to the list in the middle (^R).
		 */

		for (i = 0; i < pacl_stuff.number_users - 2; i++) {
			if (pacl_stuff.user [i][0] == 0) {
				for (j = pacl_stuff.number_users - 1; 
				     j > i; j--)
					strcpy (pacl_stuff.backing_user [j],
						pacl_stuff.backing_user [j-1]);

				strcpy (pacl_stuff.user [i], 
					INITIAL_LINE);
				strcpy (pacl_stuff.backing_user [i],
					INITIAL_LINE);
				changed = 1;
			}
		}

		/*
		 * The change happened because the user typed a value into
		 * the last line of the scrolling region.
		 */

		if (!changed) {
			strcpy (pacl_stuff.user [pacl_stuff.number_users - 2]
				+ size,
				INITIAL_LINE + size);
			strcpy (pacl_stuff.backing_user [pacl_stuff.number_users - 2],
				INITIAL_LINE);
			changed = 1;
		}
	} else {  /* check for removed lines */
		for (i = pacl_stuff.number_users - 2; i >= 0; i--) {
			if (pacl_stuff.user [i][0] == 0) {
				for (j = i; j < pacl_stuff.number_users - 1; 
				     j++) {
					strcpy (pacl_stuff.user [j],
						pacl_stuff.user [j + 1]);
					strcpy (pacl_stuff.backing_user [j],
						pacl_stuff.backing_user [j + 1]);
				}
				changed = 1;
				pacl_stuff.number_users--;
			}
		}
	}

	changed |= check_position (pacl_fill,
				   pacl_stuff.user, 
				   pacl_stuff.backing_user, 
				   pacl_stuff.number_users, 
				   pacl_stuff.mask_perms);

	if (changed) {
		pacl_struct [PACL_USER_LIST_STRUCT].pointer = 
			(char *) pacl_stuff.user;
		pacl_struct [PACL_USER_LIST_STRUCT].filled = 
			pacl_stuff.number_users;

		pacl_struct [PACL_USER_LIST_STRUCT].changed = 1;
		pacl_struct [PACL_GROUP_LIST_STRUCT].changed = 1;
		pacl_struct [PACL_GRP_EFF_STRUCT].changed = 1;
		pacl_struct [PACL_MASK_PERMS_STRUCT].changed = 1;
	}

	return changed;
}

static int
groups_list_action()
{
	int i, j,
	    changed = 0;    /* need to make a screen change */
	char **temp;

	/* only expand if the user just typed in the last blank */

	if (pacl_stuff.group [pacl_stuff.number_groups - 1][0]) {
		int	size = strlen (pacl_stuff.group 
				       [pacl_stuff.number_groups - 1]);
		
		temp = expand_cw_table (pacl_stuff.group, 
					pacl_stuff.number_groups,
					pacl_stuff.number_groups + 1, 
					ENTRY_WIDTH + 1);
		pacl_stuff.group = temp;
		temp = expand_cw_table (pacl_stuff.backing_group, 
					pacl_stuff.number_groups,
					pacl_stuff.number_groups + 1, 
					ENTRY_WIDTH + 1);
		pacl_stuff.backing_group = temp;

		strcpy (pacl_stuff.backing_group [pacl_stuff.number_groups],
			pacl_stuff.group [pacl_stuff.number_groups]);

		pacl_stuff.number_groups++;

		/* 
		 * Check to see if the expansion came because of an 
		 * addition to the list in the middle (^R).
		 */

		for (i = 0; i < pacl_stuff.number_groups - 2; i++) {
			if (pacl_stuff.group [i][0] == 0) {
				for (j = pacl_stuff.number_groups - 1; 
				     j > i; j--)
					strcpy (pacl_stuff.backing_group [j],
						pacl_stuff.backing_group [j-1]);
				strcpy (pacl_stuff.group [i], 
					INITIAL_LINE);
				strcpy (pacl_stuff.backing_group [i],
					INITIAL_LINE);
				changed = 1;
			}
		}

		/*
		 * The change happened because the user typed a value into
		 * the last line of the scrolling region.
		 */

		if (!changed) {
			strcpy (pacl_stuff.group [pacl_stuff.number_groups - 2]
				+ size,
				INITIAL_LINE + size);
			strcpy (pacl_stuff.backing_group [pacl_stuff.number_groups - 2],
				INITIAL_LINE);
			changed = 1;
		}
	} else {  /* check for removed lines */
		for (i = pacl_stuff.number_groups - 2; i >= 0; i--) {
			if (pacl_stuff.group [i][0] == 0) {
				for (j = i; j < pacl_stuff.number_groups - 1; 
				     j++) {
					strcpy (pacl_stuff.group [j],
						pacl_stuff.group [j + 1]);
					strcpy (pacl_stuff.backing_group [j],
						pacl_stuff.backing_group [j + 1]);
					changed = 1;
					pacl_stuff.number_groups--;
				}
			}
		}
	}

	changed |= check_position (pacl_fill,
				   pacl_stuff.group, 
				   pacl_stuff.backing_group, 
				   pacl_stuff.number_groups, 
				   pacl_stuff.mask_perms);

	if (changed) {
		pacl_struct [PACL_GROUP_LIST_STRUCT].pointer = 
			(char *) pacl_stuff.group;
		pacl_struct [PACL_GROUP_LIST_STRUCT].filled = 
			pacl_stuff.number_groups;

		pacl_struct [PACL_USER_LIST_STRUCT].changed = 1;
		pacl_struct [PACL_GROUP_LIST_STRUCT].changed = 1;
		pacl_struct [PACL_GRP_EFF_STRUCT].changed = 1;
		pacl_struct [PACL_MASK_PERMS_STRUCT].changed = 1;
	}

	return changed;
}

/*
 * validate function for the whole screen.
 */

static int
pacl_valid(argv, fill)
	char **argv;
	struct paclif *fill;
{
	fprintf (stderr, "validating the screen!\n");
	
	return 0;
}

/*
 * action function for the whole screen.
 */

static int
pacl_action(fill)
	struct paclif *fill;
{
	char **table;
	int i;
	int ret;
	int count = 0;

	fprintf (stderr, "Action for the screen performed\n");

	return 1;
}

/*
 * routine to free memory.
 */

static void
do_free(argv, fill, nstructs, pp, dp, sp)
	char **argv;
	struct paclif *fill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	
}

#define SETUPFUNC	pacl_setup		/* defined by stemplate.c */
#define AUTHFUNC	pacl_auth
#define BUILDFILLIN	pacl_bfill

#define INITFUNC	pacl_init		/* defined by stemplate.c */
#define BUILDSTRUCT	pacl_bstruct

#define ROUTFUNC	pacl_exit		/* defined by stemplate.c */
#define VALIDATE	pacl_valid
#define SCREENACTION	pacl_action

#define FREEFUNC	pacl_free		/* defined by stemplate.c */
#define FREESTRUCT	do_free

#include "stemplate.c"
