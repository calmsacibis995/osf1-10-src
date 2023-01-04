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
static char	*sccsid = "@(#)$RCSfile: ac_scrns.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:52:19 $";
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
 * ac_scrns.c - account screen data structures for aif
 */


#include <sys/types.h>

#include "userif.h"
#include "UIstrlen.h"


/***********************************************************

                        ACCOUNTS SCREENS

***********************************************************/

/*
	SET ACCOUNT DEFAULTS SCREENS
*/

/*
	SET ACCOUNT DEFAULT COMMAND AUTHS SCREEN
*/

Scrn_desc acdca_desc [] = {
	{ 0, 28, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Command Authorizations"},
	{ 1, 28, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "------- --------------"},
	{ 3, 18, FLD_SCRTOG, 12, FLD_BOTH,    NO, NULL, NULL, 12, 15, 2},
};

int acdca_exit(), acdca_free(), acdca_init(), acdca_setup();

Menu_scrn acdca_menu [] = {
	ROUT_DESC (1, acdca_exit, 0, NULL),
};

uchar acdca_title [] = "SET DEFAULT COMMAND AUTHORIZATIONS";
Scrn_hdrs acdca_hdrs = {
	acdca_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, cmds_titem, cmds_line1
};

SKIP_PARMF (acdca_scrn, SCR_FILLIN, acdca_desc, acdca_menu, NULL, &acdca_hdrs,
	acdca_setup, acdca_init, acdca_free);

/*
	SET ACCOUNT DEFAULT KERNEL AUTHS SCREEN
*/

Scrn_desc acdka_desc [] = {
	{ 0,  0, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Authorization"},
	{ 0, 18, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Ker"},
	{ 0, 22, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Base"},
	{ 0, 27, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Authorization"},
	{ 0, 45, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Ker"},
	{ 0, 49, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Base"},
	{ 0, 54, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Authorization"},
	{ 0, 72, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Ker"},
	{ 0, 76, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Base"},

	{ 1,  0, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "----------------"},
	{ 1, 18, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "---"},
	{ 1, 22, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "----"},
	{ 1, 27, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "----------------"},
	{ 1, 45, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "---"},
	{ 1, 49, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "----"},
	{ 1, 54, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "----------------"},
	{ 1, 72, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "---"},
	{ 1, 76, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "----"},

	{ 2,  0, FLD_SCROLL, 16, FLD_OUTPUT,  NO, NULL, NULL, 12, 0, 1},
	{ 2, 18, FLD_SCRTOG,  1, FLD_BOTH,    NO, NULL, NULL, 12, 0, 1},
	{ 2, 22, FLD_SCRTOG,  1, FLD_BOTH,    NO, NULL, NULL, 12, 0, 1},
	{ 2, 27, FLD_SCROLL, 16, FLD_OUTPUT,  NO, NULL, NULL, 12, 0, 1},
	{ 2, 45, FLD_SCRTOG,  1, FLD_BOTH,    NO, NULL, NULL, 12, 0, 1},
	{ 2, 49, FLD_SCRTOG,  1, FLD_BOTH,    NO, NULL, NULL, 12, 0, 1},
	{ 2, 54, FLD_SCROLL, 16, FLD_OUTPUT,  NO, NULL, NULL, 12, 0, 1},
	{ 2, 72, FLD_SCRTOG,  1, FLD_BOTH,    NO, NULL, NULL, 12, 0, 1},
	{ 2, 76, FLD_SCRTOG,  1, FLD_BOTH,    NO, NULL, NULL, 12, 0, 1},
};

int acdka_exit(), acdka_free(), acdka_init(), acdka_setup();

Menu_scrn acdka_menu [] = {
	ROUT_DESC (1, acdka_exit, 0, NULL),
	ROUT_DESC (2, acdka_exit, 0, NULL),
	ROUT_DESC (3, acdka_exit, 0, NULL),
	ROUT_DESC (4, acdka_exit, 0, NULL),
	ROUT_DESC (5, acdka_exit, 0, NULL),
	ROUT_DESC (6, acdka_exit, 0, NULL),
	ROUT_DESC (7, acdka_exit, 0, NULL),
	ROUT_DESC (8, acdka_exit, 0, NULL),
	ROUT_DESC (9, acdka_exit, 0, NULL),
};

uchar acdka_title [] = "SET DEFAULT KERNEL AUTHORIZATIONS";
Scrn_hdrs acdka_hdrs = {
	acdka_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, cmds_titem, cmds_line1
};

SKIP_PARMF (acdka_scrn, SCR_FILLIN, acdka_desc, acdka_menu, NULL, &acdka_hdrs,
	acdka_setup, acdka_init, acdka_free);

/*
	SET ACCOUNT DEFAULT LOGIN CONTROL SCREEN
*/

Scrn_desc acdlc_desc [] = {
	{ 0, 15, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Maximum number of unsuccessful user logins: "},
	{ 0, 62, FLD_NUMBER, 4, FLD_BOTH,  NO, NULL},
	{ 1, 15, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Inactivity timeout (minutes): "},
	{ 1, 62, FLD_NUMBER, 4, FLD_BOTH,  NO, NULL},
	{ 2, 15, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Nice value: "},
	{ 2, 64, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},

	{ 4, 15, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Boot authentication required: "},
	{ 4, 65, FLD_TOGGLE, 1, FLD_BOTH,  NO, NULL},
	{ 5, 15, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Lock account: "},
	{ 5, 65, FLD_TOGGLE, 1, FLD_BOTH,  NO, NULL},
};

int acdlc_exit(), acdlc_free(), acdlc_init(), acdlc_setup();

Menu_scrn acdlc_menu [] = {
	ROUT_DESC (1, acdlc_exit, 0, NULL),
	ROUT_DESC (2, acdlc_exit, 0, NULL),
	ROUT_DESC (3, acdlc_exit, 0, NULL),
	ROUT_DESC (4, acdlc_exit, 0, NULL),
	ROUT_DESC (5, acdlc_exit, 0, NULL),
};

uchar acdlc_title [] = "SET DEFAULT LOGIN CONTROL";
Scrn_hdrs acdlc_hdrs = {
	acdlc_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, cmds_yesno, cmds_line1
};

SKIP_PARMF (acdlc_scrn, SCR_FILLIN, acdlc_desc, acdlc_menu, NULL, &acdlc_hdrs,
	acdlc_setup, acdlc_init, acdlc_free);

/*
	SET ACCOUNT DEFAULT PASSWORD PARAMS SCREEN
*/

Scrn_desc acdpp_desc [] = {
	{ 0, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Minimum change time (weeks)"},
	{ 0, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},
	{ 1, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Password expiration time (weeks)"},
	{ 1, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},
	{ 2, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Password lifetime (weeks)"},
	{ 2, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},
	{ 3, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Expiration warning (weeks)"},
	{ 3, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},
	{ 4, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Maximum password length (characters)"},
	{ 4, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},

	{ 6, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Password required"},
	{ 6, 57, FLD_TOGGLE, 1, FLD_BOTH,  NO, NULL},
	{ 7, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "User can choose own password"},
	{ 7, 57, FLD_TOGGLE, 1, FLD_BOTH,  NO, NULL},
	{ 8, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "System generates password"},
	{ 8, 57, FLD_TOGGLE, 1, FLD_BOTH,  NO, NULL},
	{ 9, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Random password of chars"},
	{ 9, 57, FLD_TOGGLE, 1, FLD_BOTH,  NO, NULL},
	{10, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Random password of letters"},
	{10, 57, FLD_TOGGLE, 1, FLD_BOTH,  NO, NULL},
	{11, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Perform triviality checks"},
	{11, 57, FLD_TOGGLE, 1, FLD_BOTH,  NO, NULL},
};

int acdpp_exit(), acdpp_free(), acdpp_init(), acdpp_setup();

Menu_scrn acdpp_menu [] = {
	ROUT_DESC (1, acdpp_exit, 0, NULL),
	ROUT_DESC (2, acdpp_exit, 0, NULL),
	ROUT_DESC (3, acdpp_exit, 0, NULL),
	ROUT_DESC (4, acdpp_exit, 0, NULL),
	ROUT_DESC (5, acdpp_exit, 0, NULL),
	ROUT_DESC (6, acdpp_exit, 0, NULL),
	ROUT_DESC (7, acdpp_exit, 0, NULL),
	ROUT_DESC (8, acdpp_exit, 0, NULL),
	ROUT_DESC (9, acdpp_exit, 0, NULL),
	ROUT_DESC (10, acdpp_exit, 0, NULL),
	ROUT_DESC (11, acdpp_exit, 0, NULL),
};

uchar acdpp_title [] = "SET DEFAULT PASSWORD PARAMETERS";
Scrn_hdrs acdpp_hdrs = {
	acdpp_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, cmds_titem, cmds_line1
};

SKIP_PARMF (acdpp_scrn, SCR_FILLIN, acdpp_desc, acdpp_menu, NULL, &acdpp_hdrs,
	acdpp_setup, acdpp_init, acdpp_free);


/*
	ACCOUNT DEFAULTS MENU
*/

Scrn_desc acd_desc [] = {
#if SEC_MAC
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Clearance"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
#endif /* SEC_MAC */
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Command Authorizations"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Kernel Authorizations"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Login Controls"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Password Parameters"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
#if SEC_MAC
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO,
		"Single User Shell Sensitivity Level"},
#endif
};

extern int do_acdssl();
#if SEC_MAC
extern int do_acdcl();
#endif

Menu_scrn acd_menu [] = {
#if SEC_MAC
	ROUT_DESC (1, do_acdcl, 0, NULL),
#endif /* SEC_MAC */
	MENU_DESC (2, &acdca_scrn, NULL),
	MENU_DESC (3, &acdka_scrn, NULL),
	MENU_DESC (4, &acdlc_scrn, NULL),
	MENU_DESC (5, &acdpp_scrn, NULL),
#if SEC_MAC
	ROUT_DESC (6, do_acdssl, 0, NULL),
#endif
};

uchar acd_title [] = "ACCOUNT DEFAULTS";
Scrn_hdrs acd_hdrs = {
	acd_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, MT, cmds_line1
};

SCRN_PARMS (acd_scrn, SCR_MENU, acd_desc, acd_menu, NULL, &acd_hdrs);

/*
	SELECT USER (EXISTING) SCREEN
*/

Scrn_desc acsue_desc [] = {
	{ 0,  9, FLD_SCRTOG, UNAMELEN, FLD_BOTH,  NO, NULL, NULL, 13, 5, 5},
};


int acsue_setup(), acsue_init(), acsue_exit(), acsue_free();

Menu_scrn acsue_menu [] = {
	ROUT_DESC (1, acsue_exit, 1, NULL),
};

uchar acsue_title [] = "SELECT EXISTING USER";
Scrn_hdrs acsue_hdrs = {
	acsue_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, cmds_screen, cmds_line1
};

SKIP_PARMF (acsue_scrn, SCR_FILLIN, acsue_desc, acsue_menu, NULL, &acsue_hdrs,
	acsue_setup, acsue_init, acsue_free);


/*
	SELECT USER SCREEN
*/

Scrn_desc acsu_desc [] = {
	{ 0, 22, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Enter user name or ID:"},
	{ 0, 46, FLD_ALPHA, UNAMELEN, FLD_BOTH,  NO, NULL},
};


int acsu_setup(), acsu_init(), acsu_rout(), acsu_free();

Menu_scrn acsu_menu [] = {
	ROUT_DESC (1, acsu_rout, 1, &acsue_scrn),
};

uchar acsu_title [] = "SELECT USER";
Scrn_hdrs acsu_hdrs = {
	acsu_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, MT, cmds_line1
};

SKIP_PARMF (acsu_scrn, SCR_FILLIN, acsu_desc, acsu_menu, NULL, &acsu_hdrs,
	acsu_setup, acsu_init, acsu_free);

/*
	SET ACCOUNT USER COMMAND AUTHS SCREEN
*/

Scrn_desc acauca_desc [] = {
	{ 0,  5, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Command Auths" },
	{ 0, 19, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Set" },
	{ 0, 23, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Dflt" },
	{ 0, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Command Auths" },
	{ 0, 43, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Set" },
	{ 0, 47, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Dflt" },
	{ 0, 53, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Command Auths" },
	{ 0, 67, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Set" },
	{ 0, 71, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Dflt" },
	{ 1,  5, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "------- -----" },
	{ 1, 19, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "---" },
	{ 1, 23, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----" },
	{ 1, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "------- -----" },
	{ 1, 43, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "---" },
	{ 1, 47, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----" },
	{ 1, 53, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "------- -----" },
	{ 1, 67, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "---" },
	{ 1, 71, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----" },
	{ 2,  5, FLD_SCROLL, 13, FLD_OUTPUT,  NO, NULL, NULL, 14, 0, 1},
	{ 2, 19, FLD_SCRTOG,  1, FLD_BOTH,    NO, NULL, NULL, 14, 0, 1},
	{ 2, 23, FLD_SCRTOG,  1, FLD_BOTH,    NO, NULL, NULL, 14, 0, 1},
	{ 2, 29, FLD_SCROLL, 13, FLD_OUTPUT,  NO, NULL, NULL, 14, 0, 1},
	{ 2, 43, FLD_SCRTOG,  1, FLD_BOTH,    NO, NULL, NULL, 14, 0, 1},
	{ 2, 47, FLD_SCRTOG,  1, FLD_BOTH,    NO, NULL, NULL, 14, 0, 1},
	{ 2, 53, FLD_SCROLL, 13, FLD_OUTPUT,  NO, NULL, NULL, 14, 0, 1},
	{ 2, 67, FLD_SCRTOG,  1, FLD_BOTH,    NO, NULL, NULL, 14, 0, 1},
	{ 2, 71, FLD_SCRTOG,  1, FLD_BOTH,    NO, NULL, NULL, 14, 0, 1},
};

int acauca_setup(), acauca_init(), acauca_free(), acauca_exit();

Menu_scrn acauca_menu [] = {
	ROUT_DESC (1, acauca_exit, 0, NULL),
	ROUT_DESC (2, acauca_exit, 0, NULL),
	ROUT_DESC (3, acauca_exit, 0, NULL),
	ROUT_DESC (4, acauca_exit, 0, NULL),
	ROUT_DESC (5, acauca_exit, 0, NULL),
	ROUT_DESC (6, acauca_exit, 0, NULL),
	ROUT_DESC (7, acauca_exit, 0, NULL),
	ROUT_DESC (8, acauca_exit, 0, NULL),
	ROUT_DESC (9, acauca_exit, 0, NULL),
};

uchar acauca_title [] = "SET USER COMMAND AUTHORIZATIONS";
Scrn_hdrs acauca_hdrs = {
	acauca_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, cmds_titem, cmds_line1
};

SKIP_PARMF (acauca_scrn, SCR_FILLIN, acauca_desc, acauca_menu, NULL,
	&acauca_hdrs, acauca_setup, acauca_init, acauca_free);

/*
	SET ACCOUNT USER LOGIN CONTROL SCREEN
*/

Scrn_desc acaulc_desc [] = {
	{ 0,  6, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Maximum number of unsuccessful user logins: "},
	{ 0, 53, FLD_NUMBER, 4, FLD_BOTH,    NO, NULL},
	{ 0, 59, FLD_TOGGLE, 7, FLD_BOTH,    NO, "Default"},
	{ 0, 67, FLD_PROMPT, 1, FLD_OUTPUT,  NO, "("},
	{ 0, 69, FLD_NUMBER, 4, FLD_OUTPUT,  NO, NULL},
	{ 0, 74, FLD_PROMPT, 0, FLD_OUTPUT,  NO, ")"},
	{ 2,  6, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Nice value: "},
	{ 2, 53, FLD_NUMBER, 2, FLD_BOTH,    NO, NULL},
	{ 2, 59, FLD_TOGGLE, 7, FLD_BOTH,    NO, "Default"},
	{ 2, 67, FLD_PROMPT, 1, FLD_OUTPUT,  NO, "("},
	{ 2, 69, FLD_NUMBER, 2, FLD_OUTPUT,  NO, NULL},
	{ 2, 72, FLD_PROMPT, 0, FLD_OUTPUT,  NO, ")"},

	{ 5,  6, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Account Locked: "},
	{ 5, 53, FLD_TOGGLE, 3, FLD_BOTH,    NO, "Set "},
	{ 5, 57, FLD_YN,     1, FLD_OUTPUT,  NO, NULL},
	{ 5, 60, FLD_TOGGLE, 4, FLD_BOTH,    NO, "Dflt "},
	{ 5, 65, FLD_YN,     1, FLD_OUTPUT,  NO, NULL},
};

int acaulc_setup(), acaulc_init(), acaulc_free(), acaulc_exit();

Menu_scrn acaulc_menu [] = {
	ROUT_DESC (1, acaulc_exit, 0, NULL),
	ROUT_DESC (2, acaulc_exit, 0, NULL),
	ROUT_DESC (3, acaulc_exit, 0, NULL),
	ROUT_DESC (4, acaulc_exit, 0, NULL),
	ROUT_DESC (5, acaulc_exit, 0, NULL),
	ROUT_DESC (6, acaulc_exit, 0, NULL),
	ROUT_DESC (7, acaulc_exit, 0, NULL),
	ROUT_DESC (8, acaulc_exit, 0, NULL),
	ROUT_DESC (9, acaulc_exit, 0, NULL),
	ROUT_DESC (10, acaulc_exit, 0, NULL),
};

uchar acaulc_title [] = "SET USER LOGIN CONTROL";
Scrn_hdrs acaulc_hdrs = {
	acaulc_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, cmds_yesno, cmds_line1
};

SKIP_PARMF (acaulc_scrn, SCR_FILLIN, acaulc_desc, acaulc_menu, NULL,
	&acaulc_hdrs, acaulc_setup, acaulc_init, acaulc_free);

/*
	SET ACCOUNT USER PASSWORD PARAMS SCREEN
*/

Scrn_desc acaupp_desc [] = {
	{ 0, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Minimum change time (weeks)"},
	{ 0, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},
	{ 1, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Password expiration time (weeks)"},
	{ 1, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},
	{ 2, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Password lifetime (weeks)"},
	{ 2, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},
	{ 3, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Maximum password length (characters)"},
	{ 3, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},

	{ 5, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Password required"},
	{ 5, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{ 6, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "User can choose own password"},
	{ 6, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{ 7, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "System generates password"},
	{ 7, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{ 8, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Random password of chars"},
	{ 8, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{ 9, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Random password of letters"},
	{ 9, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{10, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Perform triviality checks"},
	{10, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
};

int acaupp_setup(), acaupp_init(), acaupp_free(), acaupp_exit();

Menu_scrn acaupp_menu [] = {
	ROUT_DESC (1,  acaupp_exit, 0, NULL),
	ROUT_DESC (2,  acaupp_exit, 0, NULL),
	ROUT_DESC (3,  acaupp_exit, 0, NULL),
	ROUT_DESC (4,  acaupp_exit, 0, NULL),
	ROUT_DESC (5,  acaupp_exit, 0, NULL),
	ROUT_DESC (6,  acaupp_exit, 0, NULL),
	ROUT_DESC (7,  acaupp_exit, 0, NULL),
	ROUT_DESC (8,  acaupp_exit, 0, NULL),
	ROUT_DESC (9,  acaupp_exit, 0, NULL),
	ROUT_DESC (10, acaupp_exit, 0, NULL),
};

uchar acaupp_title [] = "SET USER PASSWORD PARAMETERS";
Scrn_hdrs acaupp_hdrs = {
	acaupp_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, cmds_titem, cmds_line1
};

SKIP_PARMF (acaupp_scrn, SCR_FILLIN, acaupp_desc, acaupp_menu, NULL,
	&acaupp_hdrs, acaupp_setup, acaupp_init, acaupp_free);


/*
	CHECK USER (EXISTING) SCREEN
*/

Scrn_desc accue_desc [] = {
	{ 0,  9, FLD_SCRTOG, UNAMELEN, FLD_BOTH,  NO, NULL, NULL, 13, 5, 5},
};


int accue_setup(), accue_init(), accue_free();

Menu_scrn accue_menu [] = {
	ROUT_DESC (1, NULL, 1, NULL),
};

uchar accue_title [] = "CURRENT USERS";
Scrn_hdrs accue_hdrs = {
	accue_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, cmds_screen, cmds_line1
};

SKIP_PARMF (accue_scrn, SCR_NOCHANGE, accue_desc, accue_menu, NULL, &accue_hdrs,
	accue_setup, accue_init, accue_free);


/*
	ADD USER SCREEN
*/

Scrn_desc acau_desc [] = {
	{ 0, 11, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "User Name:"},
	{ 0, 22, FLD_ALPHA, UNAMELEN, FLD_BOTH,  YES, NULL},
	{ 0, 34, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "ID:"},
	{ 0, 38, FLD_NUMBER,  5, FLD_BOTH,  NO, NULL},
	{ 0, 45, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Main Group:"},
	{ 0, 57, FLD_ALPHA,  GNAMELEN, FLD_BOTH,  YES, NULL},
	{ 1, 11, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Home Directory:"},
	{ 1, 27, FLD_ALPHA,  NHOMEDIR, FLD_BOTH,  YES, NULL},
	{ 2, 11, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Shell:"},
	{ 2, 27, FLD_ALPHA,  NSHELLNAME, FLD_BOTH,  NO, NULL},
	{ 3, 11, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Comment:"},
	{ 3, 27, FLD_ALPHA,  NGECOS, FLD_BOTH,  NO, NULL},

	{ 5, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Supplementary Groups"},

	{ 6, 16, FLD_SCRTOG, GNAMELEN, FLD_BOTH,  NO, NULL, NULL, 3, 2, 4},

	{10, 24, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"User Security Parameters Menus"},

	{12,  6, FLD_CHOICE, 0,  FLD_INPUT,  NO, "Audit Events",
		NULL, 0, 0, 0, 1},
#if SEC_MAC
	{12, 29, FLD_CHOICE, 0,  FLD_INPUT,  NO, "Clearance",
		NULL, 0, 0, 0, 1},
#endif /* SEC_MAC */
	{12, 52, FLD_CHOICE, 0,  FLD_INPUT,  NO, "Command Authorizations",
		NULL, 0, 0, 0, 1},
	{13,  6, FLD_CHOICE, 0,  FLD_INPUT,  NO, "Kernel Authorizations",
		NULL, 0, 0, 0, 1},
	{13, 29, FLD_CHOICE, 0,  FLD_INPUT,  NO, "Login Controls",
		NULL, 0, 0, 0, 1},
	{13, 52, FLD_CHOICE, 0,  FLD_INPUT,  NO, "Password Parameters",
		NULL, 0, 0, 0, 1},
};

int acau_setup (), acau_init (), acau_exit (), acau_free();

extern int do_acsucl(), do_acauae(), do_acauka();

Menu_scrn acau_menu [] = {
	ROUT_DESC (1,  acau_exit, 0, &accue_scrn),
	ROUT_DESC (2,  acau_exit, 0, NULL),
	ROUT_DESC (3,  acau_exit, 0, NULL),
	ROUT_DESC (4,  acau_exit, 0, NULL),
	ROUT_DESC (5,  acau_exit, 0, NULL),
	ROUT_DESC (6,  acau_exit, 0, NULL),
	ROUT_DESC (7,  acau_exit, 0, NULL),
	ROUT_DESC (8,  do_acauae, 0, NULL),
#if SEC_MAC
	ROUT_DESC (9,  do_acsucl, 0, NULL),
#endif /* SEC_MAC */
	MENU_DESC (10, &acauca_scrn, NULL),
	ROUT_DESC (11, do_acauka, 0, NULL),
	MENU_DESC (12, &acaulc_scrn, NULL),
	MENU_DESC (13, &acaupp_scrn, NULL),
};

uchar acau_title [] = "ADD USER";
Scrn_hdrs acau_hdrs = {
	acau_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	cmds_screen, cmds_tcgroup, cmds_line1
};

SCRN_PARMF (acau_scrn, SCR_FILLIN, acau_desc, acau_menu, NULL, &acau_hdrs,
	acau_setup, acau_init, acau_free);

/*
	MODIFY ACCOUNT USER PASSWORD PARAMS SCREEN
*/

Scrn_desc acmupp_desc [] = {
	{ 0, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Minimum change time (weeks)"},
	{ 0, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},
	{ 1, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Password expiration time (weeks)"},
	{ 1, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},
	{ 2, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Password lifetime (weeks)"},
	{ 2, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},
	{ 3, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Maximum password length (characters)"},
	{ 3, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},

	{ 5, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Password required"},
	{ 5, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{ 6, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "User can choose own password"},
	{ 6, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{ 7, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "System generates password"},
	{ 7, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{ 8, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Random password of chars"},
	{ 8, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{ 9, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Random password of letters"},
	{ 9, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{10, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Perform triviality checks"},
	{10, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
};

int acmupp_setup(), acmupp_init(), acmupp_free(), acmupp_exit();

Menu_scrn acmupp_menu [] = {
	ROUT_DESC (1,  acmupp_exit, 0, NULL),
	ROUT_DESC (2,  acmupp_exit, 0, NULL),
	ROUT_DESC (3,  acmupp_exit, 0, NULL),
	ROUT_DESC (4,  acmupp_exit, 0, NULL),
	ROUT_DESC (5,  acmupp_exit, 0, NULL),
	ROUT_DESC (6,  acmupp_exit, 0, NULL),
	ROUT_DESC (7,  acmupp_exit, 0, NULL),
	ROUT_DESC (8,  acmupp_exit, 0, NULL),
	ROUT_DESC (9,  acmupp_exit, 0, NULL),
	ROUT_DESC (10, acmupp_exit, 0, NULL),
};

uchar acmupp_title [] = "MODIFY USER PASSWORD PARAMETERS";
Scrn_hdrs acmupp_hdrs = {
	acmupp_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, cmds_titem, cmds_line1
};

SKIP_PARMF (acmupp_scrn, SCR_FILLIN, acmupp_desc, acmupp_menu, NULL,
	&acmupp_hdrs, acmupp_setup, acmupp_init, acmupp_free);

/*
	MODIFY USER SCREEN
*/

int acmu_setup(), acmu_init(), acmu_exit(), acmu_free();

Menu_scrn acmu_menu [] = {
	ROUT_DESC (1, acmu_exit, 0, &acsue_scrn),
	ROUT_DESC (2, acmu_exit, 0, NULL),
	ROUT_DESC (3, acmu_exit, 0, NULL),
	ROUT_DESC (4, acmu_exit, 0, NULL),
	ROUT_DESC (5, acmu_exit, 0, NULL),
	ROUT_DESC (6, acmu_exit, 0, NULL),
	ROUT_DESC (7, acmu_exit, 0, NULL),
	ROUT_DESC (8,  do_acauae, 0, NULL),
#if SEC_MAC
	ROUT_DESC (9,  do_acsucl, 0, NULL),
#endif
	MENU_DESC (10, &acauca_scrn, NULL),
	ROUT_DESC (11, do_acauka, 0, NULL),
	MENU_DESC (12, &acaulc_scrn, NULL),
	MENU_DESC (13, &acmupp_scrn, NULL),
};

uchar acmu_title [] = "MODIFY USER";
Scrn_hdrs acmu_hdrs = {
	acmu_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	cmds_screen, cmds_tcgroup, cmds_line1
};

SCRN_PARMF (acmu_scrn, SCR_FILLIN, acau_desc, acmu_menu, NULL, &acmu_hdrs,
	acmu_setup, acmu_init, acmu_free);


/*
	RETIRE USER SCREEN
*/


Scrn_desc acru_desc [] = {
	{ 0, 9, FLD_SCRTOG, UNAMELEN, FLD_BOTH, NO, NULL, NULL, 13, 5, 5},
};

int acru_setup(), acru_init(), acru_exit(), acru_free();

Menu_scrn acru_menu [] = {
	ROUT_DESC (1, acru_exit, 0, &acsue_scrn),
};

uchar acru_title [] = "RETIRE USER";
Scrn_hdrs acru_hdrs = {
	acru_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, cmds_screen, cmds_line1
};

SCRN_PARMF (acru_scrn, SCR_FILLIN, acru_desc, acru_menu, NULL, &acru_hdrs,
	acru_setup, acru_init, acru_free);


/*
	DISPLAY ACCOUNT USER COMMAND AUTHS SCREEN
*/

Scrn_desc acduca_desc [] = {
	{ 0,  5, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Command Auths" },
	{ 0, 19, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Set" },
	{ 0, 23, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Dflt" },
	{ 0, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Command Auths" },
	{ 0, 43, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Set" },
	{ 0, 47, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Dflt" },
	{ 0, 53, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Command Auths" },
	{ 0, 67, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Set" },
	{ 0, 71, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Dflt" },
	{ 1,  5, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "------- -----" },
	{ 1, 19, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "---" },
	{ 1, 23, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----" },
	{ 1, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "------- -----" },
	{ 1, 43, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "---" },
	{ 1, 47, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----" },
	{ 1, 53, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "------- -----" },
	{ 1, 67, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "---" },
	{ 1, 71, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----" },
	{ 2,  5, FLD_SCROLL, 13, FLD_OUTPUT,  NO, NULL, NULL, 14, 0, 1},
	{ 2, 19, FLD_SCRTOG,  1, FLD_INPUT,   NO, NULL, NULL, 14, 0, 1},
	{ 2, 23, FLD_SCRTOG,  1, FLD_INPUT,   NO, NULL, NULL, 14, 0, 1},
	{ 2, 29, FLD_SCROLL, 13, FLD_OUTPUT,  NO, NULL, NULL, 14, 0, 1},
	{ 2, 43, FLD_SCRTOG,  1, FLD_INPUT,   NO, NULL, NULL, 14, 0, 1},
	{ 2, 47, FLD_SCRTOG,  1, FLD_INPUT,   NO, NULL, NULL, 14, 0, 1},
	{ 2, 53, FLD_SCROLL, 13, FLD_OUTPUT,  NO, NULL, NULL, 14, 0, 1},
	{ 2, 67, FLD_SCRTOG,  1, FLD_INPUT,   NO, NULL, NULL, 14, 0, 1},
	{ 2, 71, FLD_SCRTOG,  1, FLD_INPUT,   NO, NULL, NULL, 14, 0, 1},
};

int acduca_setup(), acduca_init(), acduca_free(), acduca_exit();

Menu_scrn acduca_menu [] = {
	ROUT_DESC (1, acduca_exit, 0, NULL),
	ROUT_DESC (2, acduca_exit, 0, NULL),
	ROUT_DESC (3, acduca_exit, 0, NULL),
	ROUT_DESC (4, acduca_exit, 0, NULL),
	ROUT_DESC (5, acduca_exit, 0, NULL),
	ROUT_DESC (6, acduca_exit, 0, NULL),
	ROUT_DESC (7, acduca_exit, 0, NULL),
	ROUT_DESC (8, acduca_exit, 0, NULL),
	ROUT_DESC (9, acduca_exit, 0, NULL),
};

uchar acduca_title [] = "DISPLAY USER COMMAND AUTHORIZATIONS";
Scrn_hdrs acduca_hdrs = {
	acduca_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, cmds_titem, cmds_line1
};

SKIP_PARMF (acduca_scrn, SCR_NOCHANGE, acduca_desc, acduca_menu, NULL,
	&acduca_hdrs, acduca_setup, acduca_init, acduca_free);

/*
	DISPLAY ACCOUNT USER KERNEL AUTHS SCREEN
*/

Scrn_desc acduka_desc [] = {
	{ 0,  0, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Authorization"},
	{ 0, 18, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Ker"},
	{ 0, 22, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Base"},
	{ 0, 27, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Authorization"},
	{ 0, 45, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Ker"},
	{ 0, 49, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Base"},
	{ 0, 54, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Authorization"},
	{ 0, 72, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Ker"},
	{ 0, 76, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Base"},

	{ 1,  0, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "----------------"},
	{ 1, 18, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "---"},
	{ 1, 22, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "----"},
	{ 1, 27, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "----------------"},
	{ 1, 45, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "---"},
	{ 1, 49, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "----"},
	{ 1, 54, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "----------------"},
	{ 1, 72, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "---"},
	{ 1, 76, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "----"},

	{ 2, 18, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "S/D"},
	{ 2, 23, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "S/D"},
	{ 2, 45, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "S/D"},
	{ 2, 50, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "S/D"},
	{ 2, 72, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "S/D"},
	{ 2, 77, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "S/D"},

	{ 3, 18, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "- -"},
	{ 3, 23, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "- -"},
	{ 3, 45, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "- -"},
	{ 3, 50, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "- -"},
	{ 3, 72, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "- -"},
	{ 3, 77, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "- -"},

	{ 2,  0, FLD_SCROLL, 0, FLD_OUTPUT,  NO, NULL, NULL, 11, 0, 1},
	{ 2, 18, FLD_SCRTOG, 1, FLD_BOTH,    NO, NULL, NULL, 11, 0, 1},
	{ 2, 20, FLD_SCRTOG, 1, FLD_BOTH,    NO, NULL, NULL, 11, 0, 1},
	{ 2, 23, FLD_SCRTOG, 1, FLD_BOTH,    NO, NULL, NULL, 11, 0, 1},
	{ 2, 25, FLD_SCRTOG, 1, FLD_BOTH,    NO, NULL, NULL, 11, 0, 1},
	{ 2, 27, FLD_SCROLL, 0, FLD_OUTPUT,  NO, NULL, NULL, 11, 0, 1},
	{ 2, 45, FLD_SCRTOG, 1, FLD_BOTH,    NO, NULL, NULL, 11, 0, 1},
	{ 2, 47, FLD_SCRTOG, 1, FLD_BOTH,    NO, NULL, NULL, 11, 0, 1},
	{ 2, 50, FLD_SCRTOG, 1, FLD_BOTH,    NO, NULL, NULL, 11, 0, 1},
	{ 2, 52, FLD_SCRTOG, 1, FLD_BOTH,    NO, NULL, NULL, 11, 0, 1},
	{ 2, 54, FLD_SCROLL, 0, FLD_OUTPUT,  NO, NULL, NULL, 11, 0, 1},
	{ 2, 72, FLD_SCRTOG, 1, FLD_BOTH,    NO, NULL, NULL, 11, 0, 1},
	{ 2, 74, FLD_SCRTOG, 1, FLD_BOTH,    NO, NULL, NULL, 11, 0, 1},
	{ 2, 77, FLD_SCRTOG, 1, FLD_BOTH,    NO, NULL, NULL, 11, 0, 1},
	{ 2, 79, FLD_SCRTOG, 1, FLD_BOTH,    NO, NULL, NULL, 11, 0, 1},
};


int acduka_exit ();

Menu_scrn acduka_menu [] = {
	ROUT_DESC (1, acduka_exit, 0, NULL),
	ROUT_DESC (2, acduka_exit, 0, NULL),
	ROUT_DESC (3, acduka_exit, 0, NULL),
	ROUT_DESC (4, acduka_exit, 0, NULL),
	ROUT_DESC (5, acduka_exit, 0, NULL),
	ROUT_DESC (6, acduka_exit, 0, NULL),
	ROUT_DESC (7, acduka_exit, 0, NULL),
	ROUT_DESC (8, acduka_exit, 0, NULL),
	ROUT_DESC (9, acduka_exit, 0, NULL),
	ROUT_DESC (10, acduka_exit, 0, NULL),
	ROUT_DESC (11, acduka_exit, 0, NULL),
	ROUT_DESC (12, acduka_exit, 0, NULL),
	ROUT_DESC (13, acduka_exit, 0, NULL),
	ROUT_DESC (14, acduka_exit, 0, NULL),
	ROUT_DESC (15, acduka_exit, 0, NULL),
};

uchar acduka_title [] = "DISPLAY USER KERNEL AUTHORIZATIONS";
Scrn_hdrs acduka_hdrs = {
	acduka_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, cmds_titem, cmds_line1
};

SKIP_PARMS (acduka_scrn, SCR_NOCHANGE, acduka_desc, acduka_menu, NULL, &acduka_hdrs);

/*
	DISPLAY ACCOUNT USER LOGIN CONTROL SCREEN
*/

Scrn_desc acdulc_desc [] = {
	{ 0,  6, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Maximum number of unsuccessful user logins: "},
	{ 0, 53, FLD_NUMBER, 4, FLD_BOTH,  NO, NULL},
	{ 0, 59, FLD_TOGGLE, 7, FLD_BOTH,  NO, "Default"},
	{ 0, 67, FLD_PROMPT, 1, FLD_OUTPUT,  NO, "("},
	{ 0, 69, FLD_NUMBER, 4, FLD_OUTPUT,  NO, NULL},
	{ 0, 74, FLD_PROMPT, 0, FLD_OUTPUT,  NO, ")"},
	{ 2,  6, FLD_PROMPT, 0, FLD_BOTH,  NO, "Nice value: "},
	{ 2, 53, FLD_NUMBER, 2, FLD_OUTPUT,  NO, NULL},
	{ 2, 59, FLD_TOGGLE, 7, FLD_BOTH,  NO, "Default"},
	{ 2, 67, FLD_PROMPT, 1, FLD_OUTPUT,  NO, "("},
	{ 2, 69, FLD_NUMBER, 2, FLD_OUTPUT,  NO, NULL},
	{ 2, 72, FLD_PROMPT, 0, FLD_OUTPUT,  NO, ")"},

	{ 5,  6, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Account Locked: "},
	{ 5, 53, FLD_TOGGLE, 3, FLD_BOTH,  NO, "Set "},
	{ 5, 57, FLD_YN,     1, FLD_OUTPUT,  NO, NULL},
	{ 5, 60, FLD_TOGGLE, 4, FLD_BOTH,  NO, "Dflt "},
	{ 5, 65, FLD_YN,     1, FLD_OUTPUT,  NO, NULL},
};

int acdulc_setup(), acdulc_init(), acdulc_free(), acdulc_exit();

Menu_scrn acdulc_menu [] = {
	ROUT_DESC (1, acdulc_exit, 0, NULL),
	ROUT_DESC (2, acdulc_exit, 0, NULL),
	ROUT_DESC (3, acdulc_exit, 0, NULL),
	ROUT_DESC (4, acdulc_exit, 0, NULL),
	ROUT_DESC (5, acdulc_exit, 0, NULL),
	ROUT_DESC (6, acdulc_exit, 0, NULL),
	ROUT_DESC (7, acdulc_exit, 0, NULL),
	ROUT_DESC (8, acdulc_exit, 0, NULL),
	ROUT_DESC (9, acdulc_exit, 0, NULL),
	ROUT_DESC (10, acdulc_exit, 0, NULL),
};

uchar acdulc_title [] = "DISPLAY USER LOGIN CONTROL";
Scrn_hdrs acdulc_hdrs = {
	acdulc_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, cmds_yesno, cmds_line1
};

SKIP_PARMF (acdulc_scrn, SCR_NOCHANGE, acdulc_desc, acdulc_menu, NULL,
	&acdulc_hdrs, acdulc_setup, acdulc_init, acdulc_free);

/*
	DISPLAY ACCOUNT USER PASSWORD PARAMS SCREEN
*/

Scrn_desc acdupp_desc [] = {
	{ 0, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Minimum change time (weeks)"},
	{ 0, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},
	{ 1, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Password expiration time (weeks)"},
	{ 1, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},
	{ 2, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Password lifetime (weeks)"},
	{ 2, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},
	{ 3, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Maximum password length (characters)"},
	{ 3, 56, FLD_NUMBER, 2, FLD_BOTH,  NO, NULL},

	{ 5, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Password required"},
	{ 5, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{ 6, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "User can choose own password"},
	{ 6, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{ 7, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "System generates password"},
	{ 7, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{ 8, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Random password of chars"},
	{ 8, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{ 9, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Random password of letters"},
	{ 9, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
	{10, 19, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Perform triviality checks"},
	{10, 57, FLD_YN,     1, FLD_BOTH,  NO, NULL},
};

int acdupp_setup(), acdupp_init(), acdupp_free(), acdupp_exit();

Menu_scrn acdupp_menu [] = {
	ROUT_DESC (1, acdupp_exit, 0, NULL),
	ROUT_DESC (2, acdupp_exit, 0, NULL),
	ROUT_DESC (3, acdupp_exit, 0, NULL),
	ROUT_DESC (4, acdupp_exit, 0, NULL),
	ROUT_DESC (5, acdupp_exit, 0, NULL),
	ROUT_DESC (6, acdupp_exit, 0, NULL),
	ROUT_DESC (7, acdupp_exit, 0, NULL),
	ROUT_DESC (8, acdupp_exit, 0, NULL),
	ROUT_DESC (9, acdupp_exit, 0, NULL),
	ROUT_DESC (10, acdupp_exit, 0, NULL),
};

uchar acdupp_title [] = "DISPLAY USER PASSWORD PARAMETERS";
Scrn_hdrs acdupp_hdrs = {
	acdupp_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, cmds_titem, cmds_line1
};

SKIP_PARMF (acdupp_scrn, SCR_NOCHANGE, acdupp_desc, acdupp_menu, NULL,
	&acdupp_hdrs, acdupp_setup, acdupp_init, acdupp_free);


/*
	DISPLAY USER SCREEN
*/

Scrn_desc acdu_desc [] = {
	{ 0, 11, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "User Name:"},
	{ 0, 22, FLD_ALPHA,  UNAMELEN, FLD_BOTH,  NO, NULL},
	{ 0, 34, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "ID:"},
	{ 0, 38, FLD_NUMBER,  5, FLD_BOTH,  NO, NULL},
	{ 0, 45, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Main Group:"},
	{ 0, 57, FLD_ALPHA,  GNAMELEN, FLD_OUTPUT,  NO, NULL},
	{ 1, 11, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Home Directory:"},
	{ 1, 27, FLD_ALPHA,  NHOMEDIR, FLD_OUTPUT,  NO, NULL},
	{ 2, 11, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Shell:"},
	{ 2, 27, FLD_ALPHA,  NSHELLNAME, FLD_OUTPUT,  NO, NULL},
	{ 3, 11, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Comment:"},
	{ 3, 27, FLD_ALPHA,  NGECOS, FLD_OUTPUT,  NO, NULL},

	{ 5, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Supplementary Groups"},

	{ 6, 16, FLD_SCRTOG, GNAMELEN, FLD_BOTH,  NO, NULL, NULL, 3, 2, 4},

	{10, 24, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"User Security Parameters Menus"},
	{12,  6, FLD_CHOICE, 0,  FLD_INPUT,  NO, "Audit Events"},
#if SEC_MAC
	{12, 29, FLD_CHOICE, 0,  FLD_INPUT,  NO, "Clearance"},
#endif /* SEC_MAC */
	{12, 52, FLD_CHOICE, 0,  FLD_INPUT,  NO, "Command Authorizations"},
	{13,  6, FLD_CHOICE, 0,  FLD_INPUT,  NO, "Kernel Authorizations"},
	{13, 29, FLD_CHOICE, 0,  FLD_INPUT,  NO, "Login Controls"},
	{13, 52, FLD_CHOICE, 0,  FLD_INPUT,  NO, "Password Parameters"},
};

int acdu_setup(), acdu_init(), acdu_exit(), acdu_free();
extern int do_acduae(), do_acducl(), do_acduka();

Menu_scrn acdu_menu [] = {
	ROUT_DESC (1,  acdu_exit, 0, &acsue_scrn),
	ROUT_DESC (2,  acdu_exit, 0, NULL),
	ROUT_DESC (3,  acdu_exit, 0, NULL),
	ROUT_DESC (4,  acdu_exit, 0, NULL),
	ROUT_DESC (5,  acdu_exit, 0, NULL),
	ROUT_DESC (6,  acdu_exit, 0, NULL),
	ROUT_DESC (7,  acdu_exit, 0, NULL),
	ROUT_DESC (8,  do_acduae, 0, NULL),
#if SEC_MAC
	ROUT_DESC (9,  do_acducl, 0, NULL),
#endif /* SEC_MAC */
	MENU_DESC (10, &acduca_scrn, NULL),
	ROUT_DESC (11, do_acduka, 0, NULL),
	MENU_DESC (12, &acdulc_scrn, NULL),
	MENU_DESC (13, &acdupp_scrn, NULL),
};
uchar acdu_title [] = "DISPLAY USER";
Scrn_hdrs acdu_hdrs = {
	acdu_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, cmds_screen, cmds_line1
};

SCRN_PARMF (acdu_scrn, SCR_FILLIN, acdu_desc, acdu_menu, NULL, &acdu_hdrs,
	acdu_setup, acdu_init, acdu_free);

/*
	LOCK USER SCREEN
*/

Scrn_desc aclu_desc [] = {
	{ 0, 9, FLD_SCRTOG, UNAMELEN, FLD_BOTH, NO, NULL, NULL, 13, 5, 5},
};

int aclu_exit(), aclu_setup(), aclu_init(), aclu_free();

Menu_scrn aclu_menu [] = {
	ROUT_DESC (1, aclu_exit, 0, NULL),
};

uchar aclu_title [] = "LOCK USER";
Scrn_hdrs aclu_hdrs = {
	aclu_title, cur_date, runner, cur_time,
	NULL, MT, NULL,
	MT, MT, cmds_line1
};

SCRN_PARMF (aclu_scrn, SCR_FILLIN, aclu_desc, aclu_menu, NULL, &aclu_hdrs,
	aclu_setup, aclu_init, aclu_free);

/*
	UNLOCK USER SCREEN
*/

Scrn_desc acuu_desc [] = {
	{ 0, 9, FLD_SCRTOG, UNAMELEN, FLD_BOTH, NO, NULL, NULL, 12, 5, 5},
};

int acuu_exit(), acuu_setup(), acuu_init(), acuu_free();

Menu_scrn acuu_menu [] = {
	ROUT_DESC (1, acuu_exit, 0, NULL),
};

uchar acuu_title [] = "UNLOCK USER";
Scrn_hdrs acuu_hdrs = {
	acuu_title, cur_date, runner, cur_time,
	NULL, MT, NULL,
	MT, MT, cmds_line1
};

SCRN_PARMF (acuu_scrn, SCR_FILLIN, acuu_desc, acuu_menu, NULL, &acuu_hdrs,
	acuu_setup, acuu_init, acuu_free);

/*
	USERS MENU
*/

Scrn_desc acu_desc [] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Select Users"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Add Users"},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Modify Users"},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Retire Users"},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Display Users"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Lock Users"},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Unlock Users"},
};

Menu_scrn acu_menu [] = {
	MENU_DESC (1, &acsu_scrn, NULL),
	MENU_DESC (2, &acau_scrn, NULL),
	MENU_DESC (3, &acmu_scrn, NULL),
	MENU_DESC (4, &acru_scrn, NULL),
	MENU_DESC (5, &acdu_scrn, NULL),
	MENU_DESC (6, &aclu_scrn, NULL),
	MENU_DESC (7, &acuu_scrn, NULL),
};

uchar acu_title [] = "USERS";
Scrn_hdrs acu_hdrs = {
	acu_title, cur_date, runner, cur_time,
	NULL, cur_user, cur_uid,
	MT, MT, cmds_line1
};

SCRN_PARMS (acu_scrn, SCR_MENU, acu_desc, acu_menu, NULL, &acu_hdrs);

/*
	SELECT GROUP (EXISTING) SCREEN
*/

Scrn_desc acsge_desc [] = {
	{ 0,  9, FLD_SCRTOG, GNAMELEN, FLD_BOTH,  NO, NULL, NULL, 13, 5, 5},
};

int acsge_setup(), acsge_init(), acsge_exit(), acsge_free();

Menu_scrn acsge_menu [] = {
	ROUT_DESC (1, acsge_exit, 0, NULL),
};

uchar acsge_title [] = "CURRENT GROUPS";
Scrn_hdrs acsge_hdrs = {
	acsge_title, cur_date, runner, cur_time,
	NULL, cur_group, cur_gid,
	MT, MT, cmds_line1
};

SKIP_PARMF (acsge_scrn, SCR_FILLIN, acsge_desc, acsge_menu, NULL, &acsge_hdrs,
	acsge_setup, acsge_init, acsge_free);

/*
	SELECT GROUP SCREEN
*/

Scrn_desc acsg_desc [] = {
	{ 0, 22, FLD_PROMPT, 0, FLD_OUTPUT,  NO, "Enter group name or ID:"},
	{ 0, 47, FLD_ALPHA, GNAMELEN, FLD_BOTH,  NO, NULL},
};

int acsg_setup(), acsg_init(), acsg_exit(), acsg_free();

Menu_scrn acsg_menu [] = {
	ROUT_DESC (1, acsg_exit, 0, &acsge_scrn),
};

uchar acsg_title [] = "SELECT GROUP";
Scrn_hdrs acsg_hdrs = {
	acsg_title, cur_date, runner, cur_time,
	NULL, cur_group, cur_gid,
	MT, MT, cmds_line1
};

SKIP_PARMF (acsg_scrn, SCR_FILLIN, acsg_desc, acsg_menu, NULL, &acsg_hdrs,
	acsg_setup, acsg_init, acsg_free);

/*
	ADD GROUP SCREEN
*/

Scrn_desc acag_desc [] = {
	{ 0, 28, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Group Name:"},
	{ 0, 40, FLD_ALPHA,  GNAMELEN, FLD_BOTH,  NO, NULL},
	{ 1, 28, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Group ID:"},
	{ 1, 40, FLD_NUMBER,  5, FLD_BOTH,  NO, NULL},

	{ 3, 37, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Users"},

	{ 5, 16, FLD_SCRTOG, UNAMELEN, FLD_BOTH,  NO, NULL, NULL, 8, 2, 4},
};

int acag_exit(), acag_setup(), acag_init(), acag_free();

Menu_scrn acag_menu [] = {
	ROUT_DESC (1, acag_exit, 0, NULL),
	ROUT_DESC (2, acag_exit, 0, NULL),
	ROUT_DESC (3, acag_exit, 0, NULL),
};

uchar acag_title [] = "ADD GROUP";
Scrn_hdrs acag_hdrs = {
	acag_title, cur_date, runner, cur_time,
	NULL, cur_group, cur_gid,
	cmds_screen, cmds_tuser, cmds_line1
};

SCRN_PARMF (acag_scrn, SCR_FILLIN, acag_desc, acag_menu, NULL, &acag_hdrs,
	acag_setup, acag_init, acag_free);

/*
	MODIFY GROUP SCREEN
*/

Scrn_desc acmg_desc [] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, NULL},
};

int acmg_exit(), acmg_setup(), acmg_init(), acmg_free();


Menu_scrn acmg_menu [] = {
	ROUT_DESC (1, acmg_exit, 0, &acsge_scrn),
	ROUT_DESC (2, acmg_exit, 0, &acsge_scrn),
	ROUT_DESC (3, acmg_exit, 0, NULL),
};

uchar acmg_title [] = "MODIFY GROUP";
Scrn_hdrs acmg_hdrs = {
	acmg_title, cur_date, runner, cur_time,
	NULL, cur_group, cur_gid,
	cmds_screen, cmds_tuser, cmds_line1
};

SCRN_PARMF (acmg_scrn, SCR_FILLIN, acag_desc, acmg_menu, NULL, &acmg_hdrs,
	acmg_setup, acmg_init, acmg_free);

/*
	DISPLAY GROUP SCREEN
*/

Scrn_desc acdg_desc [] = {
	{ 0, 28, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Group Name:"},
	{ 0, 40, FLD_ALPHA,  GNAMELEN, FLD_BOTH,  NO, NULL},
	{ 1, 28, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Group ID:"},
	{ 1, 40, FLD_NUMBER,  5, FLD_BOTH,  NO, NULL},

	{ 3, 37, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Users"},

	{ 5, 16, FLD_SCROLL, UNAMELEN, FLD_OUTPUT,  NO, NULL, NULL, 8, 2, 4},
};


int acdg_exit(), acdg_setup(), acdg_init(), acdg_free();

Menu_scrn acdg_menu [] = {
	ROUT_DESC (1, acdg_exit, 0, &acsge_scrn),
	ROUT_DESC (2, acdg_exit, 0, &acsge_scrn),
	ROUT_DESC (3, acdg_exit, 0, NULL),
};

uchar acdg_title [] = "DISPLAY GROUP";
Scrn_hdrs acdg_hdrs = {
	acdg_title, cur_date, runner, cur_time,
	NULL, cur_group, cur_gid,
	MT, MT, cmds_anykey
};

SCRN_PARMF (acdg_scrn, SCR_FILLIN, acdg_desc, acdg_menu, NULL, &acdg_hdrs,
	acdg_setup, acdg_init, acdg_free);

/*
	USERS' GROUPS MENU
*/

Scrn_desc acg_desc [] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Select Groups"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Add Groups"},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Update Groups"},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Display Groups"},
};

Menu_scrn acg_menu [] = {
	MENU_DESC (1, &acsg_scrn, NULL),
	MENU_DESC (2, &acag_scrn, NULL),
	MENU_DESC (3, &acmg_scrn, NULL),
	MENU_DESC (4, &acdg_scrn, NULL),
};

uchar acg_title [] = "GROUPS";
Scrn_hdrs acg_hdrs = {
	acg_title, cur_date, runner, cur_time,
	NULL, cur_group, cur_gid,
	MT, MT, cmds_line1
};

SCRN_PARMS (acg_scrn, SCR_MENU, acg_desc, acg_menu, NULL, &acg_hdrs);

/*
	MAIN ACCOUNTS MENU
*/

Scrn_desc ac_desc [] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Defaults"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Users"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Groups"},
};

Menu_scrn ac_menu [] = {
	MENU_DESC (1, &acd_scrn, NULL),
	MENU_DESC (2, &acu_scrn, NULL),
	MENU_DESC (3, &acg_scrn, NULL),
};

uchar ac_title [] = "ACCOUNTS";
Scrn_hdrs ac_hdrs = {
	ac_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, MT, cmds_line1
};

SCRN_PARMS (ac_scrn, SCR_MENU, ac_desc, ac_menu, NULL, &ac_hdrs);



#if defined(DEBUG) || defined(PRINTSCR)
print_ac_scrns()
{
	printscreen (&ac_scrn);
	printscreen (&acd_scrn);
	printscreen (&acdcl_scrn);
	printscreen (&acdca_scrn);
	printscreen (&acdka_scrn);
	printscreen (&acdlc_scrn);
	printscreen (&acdpp_scrn);
	printscreen (&acu_scrn);
	printscreen (&acsu_scrn);
	printscreen (&acsue_scrn);
	printscreen (&acau_scrn);
	printscreen (&acaucl_scrn);
	printscreen (&acauca_scrn);
	printscreen (&acauka_scrn);
	printscreen (&acaulc_scrn);
	printscreen (&acaupp_scrn);
	printscreen (&acmu_scrn);
	printscreen (&acru_scrn);
	printscreen (&acdu_scrn);
	printscreen (&acducl_scrn);
	printscreen (&acduca_scrn);
	printscreen (&acduka_scrn);
	printscreen (&acdulc_scrn);
	printscreen (&acdupp_scrn);
	printscreen (&aclu_scrn);
	printscreen (&acuu_scrn);
	printscreen (&acg_scrn);
	printscreen (&acsg_scrn);
	printscreen (&acsge_scrn);
	printscreen (&acag_scrn);
	printscreen (&acmg_scrn);
	printscreen (&acdg_scrn);
}
#endif /* DEBUG || PRINTSCR */
