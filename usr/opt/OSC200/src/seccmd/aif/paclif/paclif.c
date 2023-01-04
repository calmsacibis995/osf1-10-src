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
static char *rcsid = "@(#)$RCSfile: paclif.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/04/01 20:13:05 $";
#endif
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * $OSF_Log:	paclif.c,v $
 * Revision 1.1.1.1  92/05/12  01:45:00  devrcs
 *  *** OSF1_1B29 version ***
 * 
 * Revision 1.1.2.3  1992/04/05  18:20:12  marquard
 * 	paclif POSIX ACL interface program.
 * 	[1992/04/05  11:53:12  marquard]
 *
 * $OSF_EndLog$
 */
/*
 * paclif.c - Posix ACL Interface screen builder and processor 
 * @(#)paclif.c	1.1 11:17:46 11/8/91 SecureWare
 * 
 * Copyright (c) 1991, SecureWare, Inc.  All rights reserved.
 */


#include <sys/secdefines.h>
#include "If.h"
#include "AIf.h"
#include "scrn_local.h"
#include "IfAccounts.h"


/* Creates pacl_buf and pacl_fill */

static struct pacl_edit_struct pacl_buf;
struct pacl_edit_struct *pacl_fill = &pacl_buf:



 /* static routine definitions */

static int pacl_auth();
static int pacl_bfill();
static int pacl_bstruct();
static int pacl_valid();
static int pacl_action();
static void pacl_free();
#ifdef PRINTSCR
void pacl_print();
#endif


#define PARMTEMPLATE	pacl_scrn
#define STRUCTTEMPLATE	pacl_struct
#define DESCTEMPLATE	pacl_desc
#define FILLINSTRUCT	pacl_edit_struct
#define FILLIN		pacl_fill

#define PACL_OBJECT_TITLE_DESC		0
#define PACL_OBJNAME_TITLE__DESC      	1	
#define PACL_OBJNAME_DESC		2
#define PACL_CRE_USER_TITLE__DESC	3
#define PACL_CRE_USER__DESC		4	
#define PACL_CRE_GRP_TITLE__DESC	5
#define PACL_CRE_GRP_DESC		6	
#define PACL_OWN_USER_TITLE__DESC	7
#define PACL_OWN_USER_DESC		8	
#define PACL_OWN_GRP_TITLE__DESC	9
#define PACL_OWN_GRP_DESC		10	
#define PACL_SET_OWNER_BUTTON_DESC	11
#define PACL_SET_GROUP_BUTTON_DESC	12
#define PACL_SET_OWNGRP_BUTTON_DESC	13
#define PACL_DASH_DESC			14
#define PACL_OWN_PERMS_TITLE__DESC	15	
#define PACL_OWN_PERMS_DESC		16	
#define PACL_MASK_PERMS_TITLE__DESC	17	
#define PACL_MASK_PERMS_DESC		18	
#define PACL_OTH_PERMS_TITLE__DESC	19		
#define PACL_OTH_PERMS_DESC		20			
#define PACL_GRP_PERMS_TITLE__DESC	21		
#define PACL_GRP_PERMS_DESC		22			
#define PACL_SET_MODE_BUTTON_DESC	23	
#define PACL_TOGGLE1_TITLE_DESC		24	
#define PACL_TOGGLE2_TITLE_DESC		25	
#define PACL_TOGGLE1_DESC		26	
#define PACL_TOGGLE2_DESC		27	
/* Subject Attributes Section of Screen */
#define PACL_SUBJECT_TITLE_DESC		28			
#define PACL_SUB_UNAME_TITLE__DESC     	29
#define PACL_SUB_UNAME_DESC		30
#define PACL_TOGGLE3_TITLE_DESC		31
#define PACL_TOGGLE3_DESC		32
#define PACL_SUB_PERMS_TITLE__DESC     	33
#define PACL_SUB_PERMS_DESC		34
#define PACL_SET_ACL_BUTTON_DESC	35	
#define PACL_SET_DFLT_BUTTON_DESC	36	
#define PACL_GET_ACL_BUTTON_DESC	37	
#define PACL_GET_DFLT_BUTTON_DESC	38	
#define PACL_DELETE_DFLT_BUTTON_DESC	39	
#define PACL_COMPUTE_MASK_BUTTON_DESC	40	
#define PACL_PURGE_INEF_BUTTON_DESC	41	
#define PACL_NO_MASK_CALC_BUTTON_DESC	42	
#define PACL_GET_GRPS_BUTTON_DESC	43	
#define PACL_GET_ID_BUTTON_DESC		44	
#define PACL_SHOW_PERMS_BUTTON_DESC	45	

#define FIRSTDESC 	PACL_OBJNAME_DESC


#define PACL_OBJNAME_STRUCT		0	
#define PACL_CRE_USER__STRUCT		1		
#define PACL_CRE_GRP_STRUCT		2	
#define PACL_OWN_USER_STRUCT		3	
#define PACL_OWN_GRP_STRUCT		4	
#define PACL_SET_OWNER_BUTTON_STRUCT	5		
#define PACL_SET_GROUP_BUTTON_STRUCT	6	
#define PACL_SET_OWNGRP_BUTTON_STRUCT	7	
#define PACL_OWN_PERMS_STRUCT		8	
#define PACL_MASK_PERMS_STRUCT		9	
#define PACL_OTH_PERMS_STRUCT		10				
#define PACL_GRP_PERMS_STRUCT		11			
#define PACL_SET_MODE_BUTTON_STRUCT	12			
#define PACL_TOGGLE1_STRUCT		13	
#define PACL_TOGGLE2_STRUCT		14	
#define PACL_SUB_UNAME_STRUCT		15	
#define PACL_TOGGLE3_STRUCT		16	
#define PACL_SUB_PERMS_STRUCT		17
#define PACL_SET_ACL_BUTTON_STRUCT	18	
#define PACL_SET_DFLT_BUTTON_STRUCT	19	
#define PACL_GET_ACL_BUTTON_STRUCT	20	
#define PACL_GET_DFLT_BUTTON_STRUCT	21	
#define PACL_DELETE_DFLT_BUTTON_STRUCT	22	
#define PACL_COMPUTE_MASK_BUTTON_STRUCT	23	
#define PACL_PURGE_INEF_BUTTON_STRUCT	24	
#define PACL_NO_MASK_CALC_BUTTON_STRUCT 25	
#define PACL_GET_GRPS_BUTTON_STRUCT	26	
#define PACL_GET_ID_BUTTON_STRUCT	27 		
#define PACL_SHOW_PERMS_BUTTON_STRUCT	28		

#define NSCRNSTRUCT	29	

static Scrn_struct	*pacl_struct;


/***********************************************************

		POSIX ACL EDITOR SCREEN 

***********************************************************/

Scrn_desc	pacl_desc[] = {
/* row, col, type, len, inout, required, prompt, help */
	{ 0, 17, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Object Attributes"},
	{ 0, 60, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Subject Attributes"},
	{ 3, 2,  FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Name:"},
	{ 3, 7,  FLD_ALPHA,  8,       FLD_BOTH,   NO },
	{ 5, 2,  FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Creator User:"},
	{ 5, 16, FLD_ALPHA,  8,       FLD_BOTH,   NO },
	{ 5, 27, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Creator Group:"},
	{ 5, 42, FLD_ALPHA,  8,       FLD_BOTH,   NO },
	{ 6, 2,  FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Owner User:"},
	{ 6, 16, FLD_ALPHA,  8,       FLD_BOTH,   NO },
	{ 6, 27, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Owner Group:"},
	{ 6, 42, FLD_ALPHA,  8,       FLD_BOTH,   NO },
	{ 8, 2,  FLD_CHOICE, 9,       FLD_BOTH,   NO,"Set Owner" },
	{ 8, 19, FLD_CHOICE, 9,       FLD_BOTH,   NO,"Set Group" },
	{ 8, 34, FLD_CHOICE, 15,      FLD_BOTH,   NO,"Set Owner/Group" },
	{ 9, 1,  FLD_PROMPT, 0,       FLD_OUTPUT, NO, 
          "--------------------------------------------------------"},
	{10,  2, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Owner Perms:"},
	{10, 15, FLD_ALPHA,  3,       FLD_BOTH,   NO },
	{10, 20, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Group Perms:"},
	{10, 33, FLD_ALPHA,  3,       FLD_BOTH,   NO },
	{10, 38, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Other Perms:"},
	{10, 51, FLD_ALPHA,  3,       FLD_BOTH,   NO },
	{11, 20, FLD_PROMPT, 0,       FLD_OUTPUT, NO, "Mask Perms:"},
	{11, 33, FLD_ALPHA,  3,       FLD_BOTH,   NO },
	{11, 38, FLD_CHOICE, 9,       FLD_BOTH,   NO,"Set Mode" },
	{13,  2, FLD_PROMPT, 0,       FLD_OUTPUT, NO, 
           "User Name  Perms  Eff.:"},
	{13, 31, FLD_PROMPT, 0,       FLD_OUTPUT, NO, 
           "Group Name  Perms  Eff.:"},
	{14,  2, FLD_SCRTOG, 21,       FLD_BOTH,   NO, NULL, NULL, 3,0,1 },
	{14, 31, FLD_SCRTOG, 21,       FLD_BOTH,   NO, NULL, NULL, 3,0,1 },
	{ 3, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "| User Name   Grp Nms"},
	{ 4, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "|"},
	{ 4, 59, FLD_ALPHA,  8,        FLD_BOTH,   NO },
	{ 4, 71, FLD_SCRTOG, 8,        FLD_BOTH,   NO, NULL, NULL, 3,0,1 },
	{ 5, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "|"},
	{ 6, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "| Perms:"},
	{ 6, 65, FLD_ALPHA,  3,        FLD_BOTH,   NO },
	{ 7, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "|"},
	{ 8, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "|"},
	{ 9, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "|"},
	{ 9, 60, FLD_CHOICE, 7,        FLD_BOTH,   NO, "Set ACL" },
	{ 9, 70, FLD_CHOICE, 8,        FLD_BOTH,   NO, "Set Dflt" },
	{10, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "|"},
	{10, 60, FLD_CHOICE, 7,        FLD_BOTH,   NO, "Get ACL" },
	{10, 70, FLD_CHOICE, 8,        FLD_BOTH,   NO, "Get Dflt" },
	{11, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "|"},
	{11, 60, FLD_CHOICE, 11,       FLD_BOTH,   NO, "Delete Dflt" },
	{12, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "|"},
	{12, 60, FLD_CHOICE, 12,       FLD_BOTH,   NO, "Compute Mask" },
	{13, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "|"},
	{13, 60, FLD_CHOICE, 18,       FLD_BOTH,   NO, "Purge Ineffectives" },
	{14, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "|"},
	{14, 60, FLD_CHOICE, 17,       FLD_BOTH,   NO, "No Mask Calculate" },
	{15, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "|"},
	{15, 60, FLD_CHOICE, 8,        FLD_BOTH,   NO, "Get Grps" },
	{15, 71, FLD_CHOICE, 6,        FLD_BOTH,   NO, "Get Id" },
	{16, 57, FLD_PROMPT, 0,        FLD_OUTPUT, NO, "|"},
	{16, 60, FLD_CHOICE, 16,       FLD_BOTH,   NO, "Show Permissions" },
};

Menu_scrn	pacl_menu[] = {
	MENU_DESC (1, &au_scrn, NULL),
	ROUT_DESC (1, do_field_valid, 0, NULL),
	ROUT_DESC (2, do_field_valid, 0, NULL),
	ROUT_DESC (3, do_field_valid, 0, NULL),
	ROUT_DESC (4, do_field_valid, 0, NULL),
	ROUT_DESC (5, do_field_valid, 0, NULL),
	ROUT_DESC (6, do_set_owner, 0, NULL),
	ROUT_DESC (7, do_set_group, 0, NULL),
	ROUT_DESC (8, do_set_owngrp, 0, NULL),
	ROUT_DESC (9, do_field_valid, 0, NULL),
	ROUT_DESC (10, do_field_valid, 0, NULL),
	ROUT_DESC (11, do_field_valid, 0, NULL),
	ROUT_DESC (12, do_field_valid, 0, NULL),
	ROUT_DESC (13, do_set_mode, 0, NULL),
	ROUT_DESC (14, do_field_valid, 0, NULL),
	ROUT_DESC (15, do_field_valid, 0, NULL),
	ROUT_DESC (16, do_field_valid, 0, NULL),
	ROUT_DESC (17, do_field_valid, 0, NULL),
	ROUT_DESC (18, do_field_valid, 0, NULL),
	ROUT_DESC (19, do_set_ACL, 0, NULL),
	ROUT_DESC (20, do_set_dflt, 0, NULL),
	ROUT_DESC (21, do_get_ACL, 0, NULL),
	ROUT_DESC (22, do_get_dflt, 0, NULL),
	ROUT_DESC (23, do_delete_dflt, 0, NULL),
	ROUT_DESC (24, do_compute_mask, 0, NULL),
	ROUT_DESC (25, do_purge_ineff, 0, NULL),
	ROUT_DESC (26, do_no_mask, 0, NULL),
	ROUT_DESC (27, do_get_grps, 0, NULL),
	ROUT_DESC (28, do_get_id, 0, NULL),
	ROUT_DESC (29, do_show_perms, 0, NULL),
};

Scrn_hdrs pacl_hdrs = {
	"POSIX ACL EDITOR",
	cur_date, NULL, cur_time,
	NULL, NULL, NULL,
	MT,NULL,
	"RET=Execute  ESC=Leave Program  ^Y=Item help",
	"paclif"
};

SCRN_PARMF (pacl_scrn, SCR_FILLIN, pacl_desc, pacl_menu,
	NULL, &pacl_hdrs,pacl_setup, pacl_init, pacl_free);

Scrn_parms *TopScrn;


/*
 * No need for an authorization function; user authorization has been checked. 
 */

static int
pacl_auth(argv, pafill)
	char **argv;
	pacl_edit_struct *pafill;
{
	return 0;

}

/*
 * Setup the screen with the initial field values.
 */

static int
pacl_bfill(pafill)
	struct pacl_edit_struct *pafill;
{
	/* Initialize all screen fields to spaces.		*/
	/* Fillin the Subject area user name, group region, and permissions. */
	/* If no object name on command line, Prompt the user for the */ 
	/* object name			*/	
	/* Get the object file name from the screen. 	*/
	/* Do stat command on the object		*/
	/* Fill in the Creator User, Creator Group, Owner User, Owner Group
	 * Owner Perms, Mask Perms, Other Perms, Group Perms, User scroll
	 * region, Group scroll region, subject User Name, subject Group
	 * scroll region, and subject perms.                 		  */
	 
	return 0;

}
/*
 * Set up the fillin structure with the command authorizations
 */

static int
pacl_bstruct(pafill, sptemplate)
	struct pacl_edit_struct *pafill;
	Scrn_struct **sptemplate;
{
	struct scrn_struct *sp;

	sp = PARMTEMPLATE.ss;
	*sptemplate = sp;

	if (sp == (struct scrn_struct *) 0)
		MemoryError();

	/* Set the scrn_struct elements; for this screen.          */ 
	sp[PACL_OBJNMAE_STRUCT].pointer = pafill->obj_name;
	sp[PACL_OBJNMAE_STRUCT].filled = 1;
	sp[PACL_CRE_USER_STRUCT].pointer = pafill->obj_cruser;
	sp[PACL_CRE_USER_STRUCT].filled = 1;
	sp[PACL_CRE_GRP_STRUCT].pointer = pafill->obj_crgrp;
	sp[PACL_CRE_GRP_STRUCT].filled = 1;
	sp[PACL_OWN_USER_STRUCT].pointer = pafill->obj_ownuser;
	sp[PACL_OWN_USER_STRUCT].filled = 1;
	sp[PACL_OWN_GRP_STRUCT].pointer = pafill->obj_owngrp;
	sp[PACL_OWN_GRP_STRUCT].filled = 1;
	sp[PACL_SET_OWNER_BUTTON_STRUCT].pointer = &do_set_owner;
	sp[PACL_SET_OWNER_BUTTON_STRUCT].filled = 1;
	sp[PACL_SET_OWNER_BUTTON_STRUCT].val_act = ?????;
	sp[PACL_SET_GROUP_BUTTON_STRUCT].pointer = &do_set_group;
	sp[PACL_SET_GROUP_BUTTON_STRUCT].filled = 1;
	sp[PACL_SET_GROUP_BUTTON_STRUCT].val_act = ?????;
	sp[PACL_SET_OWNGRP_BUTTON_STRUCT].pointer = &do_set_owngrp;
	sp[PACL_SET_OWNGRP_BUTTON_STRUCT].filled = 1;
	sp[PACL_SET_OWNGRP_BUTTON_STRUCT].val_act = ?????;
	sp[PACL_SET_OWN_PERMS_STRUCT].pointer = pafill->acl_own_perms;
	sp[PACL_SET_OWN_PERMS_STRUCT].filled = 1;
	sp[PACL_SET_MASK_PERMS_STRUCT].pointer = pafill->acl_mask_perms;
	sp[PACL_SET_MASK_PERMS_STRUCT].filled = 1;
	sp[PACL_SET_OTH_PERMS_STRUCT].pointer = pafill->acl_other_perms;
	sp[PACL_SET_OTH_PERMS_STRUCT].filled = 1;
	sp[PACL_SET_GRP_PERMS_STRUCT].pointer = pafill->acl_group_perms;
	sp[PACL_SET_GRP_PERMS_STRUCT].filled = 1;
	sp[PACL_SET_MODE_BUTTON_STRUCT].pointer = &do_set_mode;
	sp[PACL_SET_MODE_BUTTON_STRUCT].filled = 1;
	sp[PACL_SET_MODE_BUTTON_STRUCT].val_act = ?????;
	sp[PACL_SET_TOGGLE1_STRUCT].pointer = (char *) User_Table;
	sp[PACL_SET_TOGGLE1_STRUCT].filled = Nusers;
	sp[PACL_SET_TOGGLE1_STRUCT].state = UserState; 
	sp[PACL_SET_TOGGLE2_STRUCT].pointer = (char *) Group_Table;
	sp[PACL_SET_TOGGLE2_STRUCT].filled = Ngroups;
	sp[PACL_SET_TOGGLE2_STRUCT].state = GroupState; 
	sp[PACL_SUB_UNAME_STRUCT].pointer = pafill->sbj_uname;
	sp[PACL_SUB_UNAME_STRUCT].filled = 1;
	sp[PACL_SET_TOGGLE3_STRUCT].pointer = (char *) SbjGroup_Table;
	sp[PACL_SET_TOGGLE3_STRUCT].filled = NsbjGroups;
	sp[PACL_SET_TOGGLE3_STRUCT].state = sbj_groupState; 
	sp[PACL_SUB_PERMS_STRUCT].pointer = pafill->sbj_uperms;
	sp[PACL_SUB_UNAME_STRUCT].filled = 1;
	sp[PACL_SET_ACL_BUTTON_STRUCT].pointer = &do_set_acl;
	sp[PACL_SET_ACL_BUTTON_STRUCT].filled = 1;
	sp[PACL_SET_ACL_BUTTON_STRUCT].val_act = ?????;
	sp[PACL_SET_DFLT_BUTTON_STRUCT].pointer = &do_set_dflt;
	sp[PACL_SET_DFLT_BUTTON_STRUCT].filled = 1;
	sp[PACL_SET_DFLT_BUTTON_STRUCT].val_act = ?????;
	sp[PACL_GET_ACL_BUTTON_STRUCT].pointer = &do_get_acl;
	sp[PACL_GET_ACL_BUTTON_STRUCT].filled = 1;
	sp[PACL_GET_ACL_BUTTON_STRUCT].val_act = ?????;
	sp[PACL_GET_DFLT_BUTTON_STRUCT].pointer = &do_get_dflt;
	sp[PACL_GET_DFLT_BUTTON_STRUCT].filled = 1;
	sp[PACL_GET_DFLT_BUTTON_STRUCT].val_act = ?????;
	sp[PACL_DELETE_DFLT_BUTTON_STRUCT].pointer = &do_delete_dflt;
	sp[PACL_DELETE_DFLT_BUTTON_STRUCT].filled = 1;
	sp[PACL_DELETE_DFLT_BUTTON_STRUCT].val_act = ?????;
	sp[PACL_COMPUTE_MASK_BUTTON_STRUCT].pointer = &do_compute_mask;
	sp[PACL_COMPUTE_MASK_BUTTON_STRUCT].filled = 1;
	sp[PACL_COMPUTE_MASK_BUTTON_STRUCT].val_act = ?????;
	sp[PACL_PURGE_INEFF_BUTTON_STRUCT].pointer = &do_purge_ineff;
	sp[PACL_PURGE_INEFF_BUTTON_STRUCT].filled = 1;
	sp[PACL_PURGE_INEFF_BUTTON_STRUCT].val_act = ?????;
	sp[PACL_NO_MASK_CALC_BUTTON_STRUCT].pointer = &do_no_mask;
	sp[PACL_NO_MASK_CALC_BUTTON_STRUCT].filled = 1;
	sp[PACL_NO_MASK_CALC_BUTTON_STRUCT.val_act = ?????;
	sp[PACL_GET_GRPS_BUTTON_STRUCT].pointer = &do_get_grps;
	sp[PACL_GET_GRPS_BUTTON_STRUCT].filled = 1;
	sp[PACL_GET_GRPS_BUTTON_STRUCT].val_act = ?????;
	sp[PACL_GET_ID_BUTTON_STRUCT].pointer = &do_get_id;
	sp[PACL_GET_ID_BUTTON_STRUCT].filled = 1;
	sp[PACL_GET_ID_BUTTON_STRUCT].val_act = ?????;
	sp[PACL_SHOW_PERMS_BUTTON_STRUCT].pointer = &do_show_perms;
	sp[PACL_SHOW_PERMS_BUTTON_STRUCT].filled = 1;
	sp[PACL_SHOW_PERMS_BUTTON_STRUCT].val_act = ?????;
	
	return 0;
}

/*
 * validate function for the whole screen.
 */

static int
pacl_valid(argv, fill)
	char **argv;
	acct_def_cmd_auth *fill;
{
	return 0;
}

/*
 * action function for the whole screen.
 */

static int
pacl_action(fill)
	acct_def_cmd_auth *fill;
{
	char **table;
	int i;
	int ret;
	int count = 0;

	SetDefCmdAuths(fill);
	return 1;
}

/*
 * routine to free memory.
 */

static void
do_free(argv, fill, nstructs, pp, dp, sp)
	char **argv;
	acct_def_cmd_auth *fill;
	int nstructs;
	Scrn_parms *pp;
	Scrn_desc *dp;
	Scrn_struct *sp;
{
	AcctFreeDefCmdAuths(fill);
}

#ifdef PRINTSCR

/* routine to print the screen contents */

void
pacl_print()
{
	Scrn_struct *sp, *buildscrn_struct();

	/* call the auth and bfill routines to load in command authorizations */

	if (pacl_auth(pacl_fill)) {
		fprintf(stderr,
                    "Auth function failed for default cmd auths screen\n");
		return;
	}

	if (pacl_bfill(pacl_fill)) {
		fprintf(stderr,
                    "Could not print Default command auths screen\n");
		return;
	}
	PARMTEMPLATE.ss = buildscrn_struct(&pacl_scrn);
	if (PARMTEMPLATE.ss == (Scrn_struct *) 0) {
		fprintf(stderr,
          "Could not build scrn struct for default command auths screen\n");
		return;
	}
	if (pacl_bstruct(pacl_fill, &sp)) {
		fprintf(stderr,
              "Bstruct routine failed for default command auths screen\n");
		return;
	}
	printscreen(&pacl_scrn);
	printscreen (&he_scrn);
}
#endif /* PRINTSCR */

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
