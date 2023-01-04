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
static char	*sccsid = "@(#)$RCSfile: au_scrns.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:54:50 $";
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
 * au_scrns.c - audit function screens for the aif
 */

#include <sys/types.h>
#include <sys/secdefines.h>
#include "gl_defs.h"
#include "userif.h"
#include "IfAudit.h"

/***********************************************************

                        AUDIT SCREENS

***********************************************************/


/*
	ENABLE AUDIT MENU
*/

Scrn_desc aue_desc [] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO,
		"Press Enter to Enable Audit"},
};

static int
EnableAudit()
{
	static int first_time = 1;

	if (first_time)	{
		first_time = 0;
		MaintenanceStart();
	}
	EnableAuditDoIt(-1);
	return 1;
}

Menu_scrn aue_menu [] = {
	ROUT_DESC (1, EnableAudit, 0, NULL),
};

uchar aue_title [] = "ENABLE AUDIT";
Scrn_hdrs aue_hdrs = {
	aue_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, MT, cmds_line1
};

SKIP_PARMS (aue_scrn, SCR_MENU, aue_desc, aue_menu, NULL, &aue_hdrs);


/*
	DISABLE AUDIT MENU
*/

Scrn_desc aud_desc [] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO,
		"Press Enter to Disable Audit"},
};

static int
DisableAudit()
{
	static int first_time = 1;

	if (first_time)	{
		first_time = 0;
		MaintenanceStart();
	}
	DisableAuditDoIt();
	return 1;
}

Menu_scrn aud_menu [] = {
	ROUT_DESC (1, DisableAudit, 0, NULL),
};

uchar aud_title [] = "DISABLE AUDIT";
Scrn_hdrs aud_hdrs = {
	aud_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, MT, cmds_line1
};

SKIP_PARMS (aud_scrn, SCR_MENU, aud_desc, aud_menu, NULL, &aud_hdrs);

/*
	MODIFY AUDIT EVENT SCREEN
*/
Scrn_desc aucpe_desc [] = {
	{ 0,  3, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Event Type" },
	{ 0, 41, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Event Type" },
	{ 1,  3, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----- ----" },
	{ 1, 41, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "----- ----" },
	{ 2,  3, FLD_SCRTOG, 34, FLD_BOTH,    NO, NULL, NULL,
	  8,  4, 2, 0 },
	{14,  6, FLD_TOGGLE, 17, FLD_BOTH,   NO, "Current Session"},
	{14, 40, FLD_TOGGLE, 17, FLD_BOTH,   NO, "Future Sessions"},
};

extern int aucpe_setup(), aucpe_init(), aucpe_free(), aucpe_exit();

Menu_scrn aucpe_menu [] = {
	ROUT_DESC ( 1, aucpe_exit, 0, NULL),
	ROUT_DESC ( 2, aucpe_exit, 0, NULL),
	ROUT_DESC ( 3, aucpe_exit, 0, NULL),
};

uchar aucpe_title [] = "MODIFY AUDIT EVENTS";
Scrn_hdrs aucpe_hdrs = {
	aucpe_title, cur_date, runner, cur_time,
	NULL, cur_user, NULL,
	MT, cmds_titem, cmds_line1
};

SKIP_PARMF (aucpe_scrn, SCR_FILLIN, aucpe_desc, aucpe_menu, NULL, &aucpe_hdrs,
	aucpe_setup, aucpe_init, aucpe_free);


/*
	AUDIT MODIFY USERS/GROUPS SCREEN
*/

Scrn_desc aucpm_desc [] = {
	{ 0, 19, FLD_PROMPT, 0,  FLD_OUTPUT, NO, "Users"},
	{ 0, 50, FLD_PROMPT, 0,  FLD_OUTPUT, NO, "Groups"},
	{ 1, 19, FLD_PROMPT, 0,  FLD_OUTPUT, NO, "-----"},
	{ 1, 50, FLD_PROMPT, 0,  FLD_OUTPUT, NO, "------"},
	{ 2, 18, FLD_SCRTOG, 10, FLD_BOTH,   NO, NULL, NULL,
	   10, 0, 1, 0 },
	{ 2, 49, FLD_SCRTOG, 11, FLD_BOTH,   NO, NULL, NULL,
	   10, 0, 1, 0 },
	{14, 19, FLD_TOGGLE, 17, FLD_BOTH,   NO, "Current Session"},
	{14, 49, FLD_TOGGLE, 17, FLD_BOTH,   NO, "Future Sessions"},
};

extern int aucpm_setup(), aucpm_init(), aucpm_free(), aucpm_exit();

Menu_scrn aucpm_menu [] = {
	ROUT_DESC (1, aucpm_exit, 0, NULL),
	ROUT_DESC (2, aucpm_exit, 0, NULL),
	ROUT_DESC (3, aucpm_exit, 0, NULL),
	ROUT_DESC (4, aucpm_exit, 0, NULL),
};

uchar aucpm_title [] = "MODIFY AUDIT USERS/GROUPS";
Scrn_hdrs aucpm_hdrs = {
	aucpm_title, cur_date, runner, cur_time,
	NULL, cur_user, NULL,
	MT, cmds_titem, cmds_line1
};

SKIP_PARMF (aucpm_scrn, SCR_FILLIN, aucpm_desc, aucpm_menu, NULL, &aucpm_hdrs,
	aucpm_setup, aucpm_init, aucpm_free);

uchar aucpncl_title [] = "MODIFY AUDIT COLLECTION MINIMUM SENSITIVITY LEVEL";

uchar aucpxcl_title [] = "MODIFY AUDIT COLLECTION MAXIMUM SENSITIVITY LEVEL";

/*
	AUDIT COLLECTION PARAMETERS MENU
*/

Scrn_desc aucp_desc [] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Modify Audit Events"},
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Modify Audit Users/Groups"},
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
#if SEC_MAC
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Modify Audit Minimum SL"},
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Modify Audit Maximum SL"},
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
#endif
};

#if SEC_MAC
extern int aucpncl(), aucpxcl();
#endif

Menu_scrn aucp_menu [] = {
	MENU_DESC (1, &aucpe_scrn, NULL),
	MENU_DESC (2, &aucpm_scrn, NULL),
#if SEC_MAC
	ROUT_DESC (3, aucpncl, 0, NULL),
	ROUT_DESC (4, aucpxcl, 0, NULL),
#endif
};

uchar aucp_title [] = "AUDIT COLLECTION PARAMETERS";
Scrn_hdrs aucp_hdrs = {
	aucp_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, MT, cmds_line1
};

SCRN_PARMS (aucp_scrn, SCR_MENU, aucp_desc, aucp_menu, NULL, &aucp_hdrs);


/*
	AUDIT MAINTENANCE BACKUP/DELETE SCREEN
*/

Scrn_desc aumb_desc [] = {
	{ 0,  3, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
"Number Start                    Stop                       Recs    Bytes"},
	{ 1,  3, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
"------ ------------------------ ------------------------ ------ --------"},
	{ 2,  3, FLD_SCRTOG, 72, FLD_BOTH,    NO, NULL, NULL, 8, 0, 1},
	{12, 18, FLD_TOGGLE,  8, FLD_BOTH,    NO, "Backup"},
	{12, 48, FLD_TOGGLE,  8, FLD_BOTH,    NO, "Delete"},
	{14,  2, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Backup Output Device:"},
	{14, 25, FLD_ALPHA,  20, FLD_BOTH,    NO, NULL},
};

extern int aumb_setup(), aumb_init(), aumb_free(), aumb_exit();

Menu_scrn aumb_menu [] = {
	ROUT_DESC (1, aumb_exit, 0, NULL),
	ROUT_DESC (2, aumb_exit, 0, NULL),
	ROUT_DESC (3, aumb_exit, 0, NULL),
};

uchar aumb_title [] = "BACKUP/DELETE COMPACTION FILES";
Scrn_hdrs aumb_hdrs = {
	aumb_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, cmds_titem, cmds_line1
};

SCRN_PARMF (aumb_scrn, SCR_FILLIN, aumb_desc, aumb_menu, NULL, &aumb_hdrs,
	    aumb_setup, aumb_init, aumb_free);

/*
	AUDIT MAINTENANCE RESTORE SCREEN
*/

Scrn_desc aumr_desc [] = {
	{0, 19, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Restore Input Device:"},
	{0, 42, FLD_ALPHA,  20, FLD_BOTH,    NO, NULL},
};

extern int aumr_setup(), aumr_init(), aumr_free(), aumr_exit();

Menu_scrn aumr_menu [] = {
	ROUT_DESC (1, aumr_exit, 0, NULL),
};

uchar aumr_title [] = "RESTORE COMPACTION FILES";
Scrn_hdrs aumr_hdrs = {
	aumr_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, cmds_titem, cmds_line1
};

SCRN_PARMF (aumr_scrn, SCR_FILLIN, aumr_desc, aumr_menu, NULL, &aumr_hdrs,
	    aumr_setup, aumr_init, aumr_free);

/*
	AUDIT MAINTENANCE MODIFY PARAMETERS SCREEN
*/

Scrn_desc aumm_desc [] = {
	{ 0, 10, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"Number of bytes before daemon wakes up:"},
	{ 0, 59, FLD_NUMBER,  6, FLD_BOTH,    NO, NULL},
	{ 1, 10, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"Maximum amount of buffer memory (1K bytes):"},
	{ 1, 59, FLD_NUMBER,  3, FLD_BOTH,    NO, NULL},
	{ 2, 10, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"Number of bytes before compaction file switched:"},
	{ 2, 59, FLD_NUMBER,  8, FLD_BOTH,    NO, NULL},

	{ 4, 10, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"Compact audit output files:"},
	{ 4, 50, FLD_TOGGLE,  1, FLD_INPUT,   NO, NULL},
	{ 5, 10, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"Enable audit on system startup:"},
	{ 5, 50, FLD_TOGGLE,  1, FLD_INPUT,   NO, NULL},
	{ 6, 10, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"Shut down gracefully if disk full:"},
	{ 6, 50, FLD_TOGGLE,  1, FLD_INPUT,   NO, NULL},

	{ 8, 10, FLD_TOGGLE, 17, FLD_INPUT,   NO, "Current Session"},
	{ 8, 30, FLD_TOGGLE, 17, FLD_INPUT,   NO, "Future Sessions"},
};

extern int aumm_init(), aumm_setup(), aumm_exit(), aumm_free();

Menu_scrn aumm_menu [] = {
	ROUT_DESC (1, aumm_exit, 0, NULL),
	ROUT_DESC (2, aumm_exit, 0, NULL),
	ROUT_DESC (3, aumm_exit, 0, NULL),
	ROUT_DESC (4, aumm_exit, 0, NULL),
	ROUT_DESC (5, aumm_exit, 0, NULL),
	ROUT_DESC (6, aumm_exit, 0, NULL),
	ROUT_DESC (7, aumm_exit, 0, NULL),
	ROUT_DESC (8, aumm_exit, 0, NULL),
};

uchar aumm_title [] = "MODIFY AUDIT PARAMETERS";
Scrn_hdrs aumm_hdrs = {
	aumm_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, cmds_titem, cmds_line1
};

SKIP_PARMF (aumm_scrn, SCR_FILLIN, aumm_desc, aumm_menu, NULL, &aumm_hdrs,
	aumm_setup, aumm_init, aumm_free);

/*
	AUDIT MAINTENANCE MODIFY DIRECTORY SCREEN
*/

Scrn_desc aumd_desc [] = {
	{0,  9, FLD_PROMPT,  0, FLD_OUTPUT, NO,
		"Single-User Audit Directory:"},
	{1,  9, FLD_ALPHA,  AUDIRWIDTH, FLD_BOTH,   NO, NULL},

	{3, 23, FLD_PROMPT,  0, FLD_OUTPUT, NO,
		"Multi-User Audit Directory List:"},
	{4, 21, FLD_SCROLL, AUDIRWIDTH, FLD_BOTH,   NO, NULL, NULL, 8, 0, 1},
};

extern int aumd_init(), aumd_setup(), aumd_exit(), aumd_free();

Menu_scrn aumd_menu [] = {
	ROUT_DESC (1, aumd_exit, 0, NULL),
	ROUT_DESC (2, aumd_exit, 0, NULL),
};

uchar aumd_title [] = "MODIFY AUDIT DIRECTORY LIST";
Scrn_hdrs aumd_hdrs = {
	aumd_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, cmds_titem, cmds_line1
};

SCRN_PARMF (aumd_scrn, SCR_FILLIN, aumd_desc, aumd_menu, NULL, &aumd_hdrs,
	aumd_setup, aumd_init, aumd_free);

/*
	AUDIT MAINTENANCE SCREEN
*/

Scrn_desc aum_desc [] = {
	{-1, -1, FLD_CHOICE,  0, FLD_INPUT,   NO,
		"Backup/Delete Compaction Files"},
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
	{-1, -1, FLD_CHOICE,  0, FLD_INPUT,   NO, "Restore Compaction Files"},
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
	{-1, -1, FLD_CHOICE,  0, FLD_INPUT,   NO, "Modify Audit Parameters"},
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
	{-1, -1, FLD_CHOICE,  0, FLD_INPUT,   NO, "Modify Directory List"},
};


Menu_scrn aum_menu [] = {
	MENU_DESC (1, &aumb_scrn, NULL),
	MENU_DESC (2, &aumr_scrn, NULL),
	MENU_DESC (3, &aumm_scrn, NULL),
	MENU_DESC (4, &aumd_scrn, NULL),
};

uchar aum_title [] = "AUDIT MAINTENANCE";
Scrn_hdrs aum_hdrs = {
	aum_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, cmds_titem, cmds_line1
};

SCRN_PARMS (aum_scrn, SCR_MENU, aum_desc, aum_menu, NULL, &aum_hdrs);

/*
	AUDIT CREATE/MODIFY/DELETE REPORT SELECTION FILES
 */

Scrn_desc aurs_desc[] = {
	{ 0, 15, FLD_TOGGLE,    6,  FLD_BOTH,   NO, "Create" },
	{ 0, 36, FLD_TOGGLE,    6,  FLD_BOTH,   NO, "Modify" },
	{ 0, 55, FLD_TOGGLE,    6,  FLD_BOTH,   NO, "Delete" },
	{ 3,  7, FLD_PROMPT,    0,  FLD_OUTPUT, NO,
	  "Create Selection File:" },
	{ 3, 40, FLD_ALPHA, ENTSIZ, FLD_BOTH, NO },
	{ 5,  7, FLD_PROMPT,    0, FLD_OUTPUT,  NO,
	  "Modify/Delete Selection File:" },
	{ 5, 40, FLD_SCRTOG,ENTSIZ, FLD_BOTH,   NO, NULL, NULL,
	  5, 0, 1},
};

extern int aurs_init(), aurs_setup(), aurs_exit(), aurs_free();

Menu_scrn aurs_menu[] = {
	ROUT_DESC(1, aurs_exit, 0, NULL),
	ROUT_DESC(2, aurs_exit, 0, NULL),
	ROUT_DESC(3, aurs_exit, 0, NULL),
	ROUT_DESC(4, aurs_exit, 0, NULL),
	ROUT_DESC(5, aurs_exit, 0, NULL),
};

uchar aurs_title[] = "CREATE/MODIFY/DELETE REPORT SELECTION FILES";
Scrn_hdrs aurs_hdrs = {
	aurs_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, cmds_titem, cmds_line1
};

SCRN_PARMF (aurs_scrn, SCR_FILLIN, aurs_desc, aurs_menu, NULL, &aurs_hdrs,
	aurs_setup, aurs_init, aurs_free);

/*
	AUDIT DISPLAY, PRINT, OR DELETE EXISTING REPORTS
 */

Scrn_desc aurd_desc[] = {
	{ 0, 15, FLD_TOGGLE,    7,  FLD_BOTH,   NO, "Display" },
	{ 0, 36, FLD_TOGGLE,    5,  FLD_BOTH,   NO, "Print" },
	{ 0, 55, FLD_TOGGLE,    6,  FLD_BOTH,   NO, "Delete" },
	{ 2,  7, FLD_PROMPT,    0,  FLD_OUTPUT, NO,
	  "Existing Report Names:" },
	{ 4, 40, FLD_SCRTOG,ENTSIZ, FLD_BOTH,   NO, NULL, NULL,
	  8, 0, 1},
};

extern int aurd_init(), aurd_setup(), aurd_exit(), aurd_free();

Menu_scrn aurd_menu[] = {
	ROUT_DESC(1, aurd_exit, 0, NULL),
	ROUT_DESC(2, aurd_exit, 0, NULL),
	ROUT_DESC(3, aurd_exit, 0, NULL),
	ROUT_DESC(4, aurd_exit, 0, NULL),
};

uchar aurd_title[] = "DISPLAY, PRINT, OR DELETE EXISTING REPORTS";

Scrn_hdrs aurd_hdrs = {
	aurd_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, cmds_titem, cmds_line1
};

SCRN_PARMF (aurd_scrn, SCR_FILLIN, aurd_desc, aurd_menu, NULL, &aurd_hdrs,
	aurd_setup, aurd_init, aurd_free);

/*
	CREATE NEW REPORTS
 */

Scrn_desc aurr_desc[] = {
	{ 0,  3, FLD_PROMPT,    0, FLD_OUTPUT,  NO, "Audit Sessions:" },
	{ 2,  3, FLD_PROMPT,    0, FLD_OUTPUT,  NO,
"Number Start                    Stop                       Recs    Bytes"},
	{ 3,  3, FLD_PROMPT,    0, FLD_OUTPUT,  NO,
"------ ------------------------ ------------------------ ------ --------"},
	{ 4,  3, FLD_SCRTOG,   LSFILLWIDTH, FLD_BOTH,    NO, NULL, NULL,
		3, 0, 1 },
	{ 8,  3, FLD_PROMPT,    0, FLD_OUTPUT,  NO, "Report Selection Files:" },
	{ 8, 47, FLD_PROMPT,    0, FLD_OUTPUT,  NO, "Report Name:" },
	{ 9,  3, FLD_SCRTOG, ENTSIZ, FLD_BOTH,  NO, NULL, NULL,
		5, 3, 2 },
	{ 9, 47, FLD_ALPHA,  ENTSIZ, FLD_BOTH,  NO },
};

extern int aurr_init(), aurr_setup(), aurr_exit(), aurr_free();

Menu_scrn aurr_menu[] = {
	ROUT_DESC(1, aurr_exit, 0, NULL),
	ROUT_DESC(2, aurr_exit, 0, NULL),
	ROUT_DESC(3, aurr_exit, 0, NULL),
};

uchar aurr_title[] = "CREATE NEW REPORTS";

Scrn_hdrs aurr_hdrs = {
	aurr_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, cmds_titem, cmds_line1
};

SCRN_PARMF (aurr_scrn, SCR_FILLIN, aurr_desc, aurr_menu, NULL, &aurr_hdrs,
	aurr_setup, aurr_init, aurr_free);

/*
	CREATE REPORT SELECTION FILE
 */

Scrn_desc aurc_desc[] = {
	{ 0, 18, FLD_PROMPT,      0, FLD_OUTPUT, NO, "Report Selection File:" },
	{ 0, 42, FLD_ALPHA,  ENTSIZ, FLD_OUTPUT, NO },
	{ 2,  6, FLD_PROMPT,      0, FLD_OUTPUT, NO, "Start Time:" },
	{ 2, 19, FLD_ALPHA,       2, FLD_BOTH,   NO },
	{ 2, 23, FLD_ALPHA,       3, FLD_BOTH,   NO },
	{ 2, 28, FLD_ALPHA,       2, FLD_BOTH,   NO },
	{ 2, 32, FLD_ALPHA,       2, FLD_BOTH,   NO },
	{ 2, 35, FLD_PROMPT,      0, FLD_OUTPUT, NO, ":" },
	{ 2, 37, FLD_ALPHA,       2, FLD_BOTH,   NO },
	{ 2, 43, FLD_PROMPT,      0, FLD_OUTPUT, NO, "Stop Time:" },
	{ 2, 55, FLD_ALPHA,       2, FLD_BOTH,   NO },
	{ 2, 59, FLD_ALPHA,       3, FLD_BOTH,   NO },
	{ 2, 64, FLD_ALPHA,       2, FLD_BOTH,   NO },
	{ 2, 68, FLD_ALPHA,       2, FLD_BOTH,   NO },
	{ 2, 71, FLD_PROMPT,      0, FLD_OUTPUT, NO, ":" },
	{ 2, 73, FLD_ALPHA,       2, FLD_BOTH,   NO },
	{ 3, 19, FLD_PROMPT,      0, FLD_OUTPUT, NO,
		"dd  mmm  yy  hh   mm" },
	{ 3, 55, FLD_PROMPT,      0, FLD_OUTPUT, NO,
		"dd  mmm  yy  hh   mm" },
	{ 5,  6, FLD_PROMPT,      0, FLD_OUTPUT, NO, "Audit Events:" },
	{ 5, 43, FLD_PROMPT,      0, FLD_OUTPUT, NO, "Files Selected:" },
	{ 6,  6, FLD_SCRTOG,     34, FLD_BOTH,   NO, NULL, NULL,
		3, 0, 1 },
	{ 6, 43, FLD_SCROLL,     32, FLD_BOTH,   NO, NULL, NULL,
		3, 0, 1 },
	{11,  6, FLD_PROMPT,      0, FLD_OUTPUT, NO, "Users:" },
	{11, 20, FLD_PROMPT,      0, FLD_OUTPUT, NO, "Groups:" },
#if SEC_MAC
	{11, 43, FLD_PROMPT,      0, FLD_OUTPUT, NO,
		"Sensitivity Level Ranges" },
#endif
	{12,  6, FLD_SCRTOG,      8, FLD_BOTH,   NO, NULL, NULL,
		3, 0, 1 },
	{12, 20, FLD_SCRTOG,      8, FLD_BOTH,   NO, NULL, NULL,
		3, 0, 1 },
#if SEC_MAC
	{13, 34, FLD_PROMPT,      0, FLD_OUTPUT, NO, "Subject" },
	{13, 44, FLD_TOGGLE,     10, FLD_BOTH,   NO, "Minimum SL" },
	{13, 57, FLD_TOGGLE,     10, FLD_BOTH,   NO, "Maximum SL" },
	{14, 34, FLD_PROMPT,      0, FLD_OUTPUT, NO, "Object" },
	{14, 44, FLD_TOGGLE,     10, FLD_BOTH,   NO, "Minimum SL" },
	{14, 57, FLD_TOGGLE,     10, FLD_BOTH,   NO, "Maximum SL" },
#endif
};

extern int aurc_init(), aurc_setup(), aurc_exit(), aurc_free();
#if SEC_MAC
extern int aurc_nssl(), aurc_xssl(),  aurc_nosl(), aurc_xosl();
#endif

Menu_scrn aurc_menu[] = {
	ROUT_DESC( 1, aurc_exit, 0, NULL),
	ROUT_DESC( 2, aurc_exit, 0, NULL),
	ROUT_DESC( 3, aurc_exit, 0, NULL),
	ROUT_DESC( 4, aurc_exit, 0, NULL),
	ROUT_DESC( 5, aurc_exit, 0, NULL),
	ROUT_DESC( 6, aurc_exit, 0, NULL),
	ROUT_DESC( 7, aurc_exit, 0, NULL),
	ROUT_DESC( 8, aurc_exit, 0, NULL),
	ROUT_DESC( 9, aurc_exit, 0, NULL),
	ROUT_DESC(10, aurc_exit, 0, NULL),
	ROUT_DESC(11, aurc_exit, 0, NULL),
	ROUT_DESC(12, aurc_exit, 0, NULL),
	ROUT_DESC(13, aurc_exit, 0, NULL),
	ROUT_DESC(14, aurc_exit, 0, NULL),
	ROUT_DESC(15, aurc_exit, 0, NULL),
#if SEC_MAC
	ROUT_DESC(16, aurc_nssl, 0, NULL),
	ROUT_DESC(17, aurc_xssl, 0, NULL),
	ROUT_DESC(18, aurc_nosl, 0, NULL),
	ROUT_DESC(19, aurc_xosl, 0, NULL),
#endif
};

uchar aurc_create_title[] = "CREATE REPORT SELECTION FILE";
uchar aurc_modify_title[] = "MODIFY REPORT SELECTION FILE";

Scrn_hdrs aurc_hdrs = {
	aurc_create_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, cmds_titem, cmds_line1
};

SCRN_PARMF (aurc_scrn, SCR_FILLIN, aurc_desc, aurc_menu, NULL, &aurc_hdrs,
	aurc_setup, aurc_init, aurc_free);

/*
	AUDIT REPORTS MENU
*/

Scrn_desc aur_desc [] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO,
		"Create/Modify/Delete Selection Files"},
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO,
		"Display/Print/Delete Existing Reports"},
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO,
		"Create New Reports"},
};

Menu_scrn aur_menu [] = {
	MENU_DESC(1, &aurs_scrn, NULL),
	MENU_DESC(2, &aurd_scrn, NULL),
	MENU_DESC(3, &aurr_scrn, NULL),
};

uchar aur_title [] = "AUDIT REPORTS";
Scrn_hdrs aur_hdrs = {
	aur_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, MT, cmds_line1
};

SCRN_PARMS (aur_scrn, SCR_MENU, aur_desc, aur_menu, NULL, &aur_hdrs);


/*
	MAIN AUDIT MENU
*/

char PromptAuditEnable[] = "Enable Audit";
char PromptAuditDisable[] = "Disable Audit";

Scrn_desc au_desc [] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, PromptAuditEnable },
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Collection Parameters"},
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Maintenance"},
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Reports"},
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Statistics"},
	{ -1, -1, FLD_SKIP,   0, FLD_INPUT,  NO, NULL},
};

extern Scrn_parms austats_scrn;

Menu_scrn au_menu [] = {
	MENU_DESC (1, &aue_scrn, NULL),
	MENU_DESC (2, &aucp_scrn, NULL),
	MENU_DESC (3, &aum_scrn, NULL),
	MENU_DESC (4, &aur_scrn, NULL),
	MENU_DESC (5, &austats_scrn, NULL),
};

uchar au_title [] = "AUDIT";
Scrn_hdrs au_hdrs = {
	au_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, MT, cmds_line1
};

static int au_init();

SCRN_PARMF (au_scrn, SCR_MENU, au_desc, au_menu, NULL, &au_hdrs,
	NULL, au_init, NULL);

/*
 * Simple function that is called before the main audit screen is
 * displayed.  If audit is enabled, it sets up to disable audit; otherwise,
 * it sets up to enable it.
 */

static int
au_init()
{
	if (CheckAuditEnabled() == 1) {
		au_desc[0].prompt = PromptAuditDisable;
		au_menu[0].next_menu = &aud_scrn;

		/* restore statistics item */

		au_scrn.ndescs = NUMDESCS(au_desc);

	} else {
		au_desc[0].prompt = PromptAuditEnable;
		au_menu[0].next_menu = &aue_scrn;

		/* don't allow statistics item to appear */

		au_scrn.ndescs = NUMDESCS(au_desc) - 2;
	}
	return 0;
}
		



#if defined(DEBUG) || defined(PRINTSCR)
print_au_scrns()
{
	printscreen (&au_scrn);
	printscreen (&aue_scrn);
	printscreen (&aes_scrn);
	printscreen (&aud_scrn);
	printscreen (&ads_scrn);
	printscreen (&aucp_scrn);
	printscreen (&aucpe_scrn);
	printscreen (&aucpm_scrn);
	printscreen (&aucpncl_scrn);
	printscreen (&aucpxcl_scrn);
	printscreen (&aum_scrn);
	printscreen (&aumb_scrn);
	printscreen (&aumr_scrn);
	printscreen (&aumm_scrn);
	printscreen (&aumd_scrn);
	printscreen (&aur_scrn);
	printscreen (&aus_scrn);
}
#endif /* DEBUG || PRINTSCR */
