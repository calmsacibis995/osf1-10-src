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
static char	*sccsid = "@(#)$RCSfile: de_scrns.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:55:49 $";
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
 * de_scrns.c - device screens for aif
 */



#include <sys/types.h>
#include "userif.h"
#include "gl_defs.h"
#include "IfDevices.h"

/***********************************************************

                        DEVICES SCREENS

***********************************************************/

/*
	DEVICE DEFAULT CONTROL PARAMETERS SCREEN
*/

Scrn_desc decp_desc [] = {
	{  0, 13, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"Maximum number of unsuccessful logins (terminal)"},
	{  0, 62, FLD_NUMBER,  3, FLD_BOTH,    NO, NULL},
	{  1, 13, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Login timeout (seconds)"},
	{  1, 62, FLD_NUMBER,  3, FLD_BOTH,    NO, NULL},
	{  2, 13, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"Delay between unsuccessful logins (seconds)"},
	{  2, 62, FLD_NUMBER,  3, FLD_BOTH,    NO, NULL},

        {  4, 13, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Controls" },
        {  4, 62, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Set" },
        {  5, 13, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "--------" },
        {  5, 62, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "---" },

	{  7, 13, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Lock terminal"},
	{  7, 62, FLD_TOGGLE,  1, FLD_BOTH,    NO, NULL},
};

extern int decp_setup(), decp_init(), decp_exit(), decp_free();

Menu_scrn decp_menu [] = {
	ROUT_DESC (1, decp_exit, 0, NULL),
	ROUT_DESC (2, decp_exit, 0, NULL),
	ROUT_DESC (3, decp_exit, 0, NULL),
	ROUT_DESC (4, decp_exit, 0, NULL),
};

uchar decp_title [] = "DEVICE DEFAULT CONTROL PARAMETERS";
Scrn_hdrs decp_hdrs = {
	decp_title, cur_date, runner, cur_time,
	NULL, cur_dev, NULL,
	MT, cmds_titem, cmds_line1
};

SKIP_PARMF(decp_scrn, SCR_FILLIN, decp_desc, decp_menu, NULL, &decp_hdrs,
	decp_setup, decp_init, decp_free);

#if SEC_MAC
/*
	DEFAULT DEVICE SENSITIVITY LABEL SCREENS
*/

uchar dedmnsl_title[] =
	"SET DEFAULT MULTILEVEL DEVICE MINIMUM SENSITIVITY LABEL";
uchar dedmxsl_title[] =
	"SET DEFAULT MULTILEVEL DEVICE MAXIMUM SENSITIVITY LABEL";
uchar dedssl_title[] =
	"SET DEFAULT SINGLE-LEVEL DEVICE SENSITIVITY LABEL";

uchar demnsl_title[] =
	"MODIFY MULTILEVEL DEVICE MINIMUM SENSITIVITY LABEL";
uchar demxsl_title[] =
	"MODIFY MULTILEVEL DEVICE MAXIMUM SENSITIVITY LABEL";
uchar dessl_title[] =
	"MODIFY SNIGLE-LEVEL DEVICE SENSITIVITY LABEL";

extern int dedmnsl(), dedmxsl(), dedssl();
#endif

#if SEC_ILB
/*
	DEFAULT DEVICE INFORMATION LABEL SCREENS
*/

uchar desli_title[] = "SET DEFAULT SINGLE-LEVEL DEVICE INFORMATION LABEL";

extern int dedsil();

#endif /* SEC_ILB */

/*
	DEVICE DEFAULTS SCREEN
*/

Scrn_desc dede_desc [] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Control Parameters"},
#if SEC_MAC
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Multilevel Minimum SL"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Multilevel Maximum SL"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Single-level SL"},
#endif
#if SEC_ILB
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Single-level IL"},
#endif /* SEC_ILB */
};

Menu_scrn dede_menu [] = {
	MENU_DESC (1, &decp_scrn, NULL),
#if SEC_MAC
	ROUT_DESC (2, dedmnsl, 0, NULL),
	ROUT_DESC (3, dedmxsl, 0, NULL),
	ROUT_DESC (4, dedssl,  0, NULL),
#endif /* SEC_MAC */
#if SEC_ILB
	ROUT_DESC (5, dedsil,  0, NULL),
#endif /* SEC_ILB */
};

uchar dede_title [] = "DEVICE DEFAULTS";
Scrn_hdrs dede_hdrs = {
	dede_title, cur_date, runner, cur_time,
	NULL, cur_dev, NULL,
	MT, MT, cmds_line1
};

SCRN_PARMS (dede_scrn, SCR_MENU, dede_desc, dede_menu, NULL, &dede_hdrs);

/*
	SELECT DEVICE MENU
 */

uchar desdm_title[] = "SELECT DEVICE";

Scrn_desc desdm_desc[] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Printer"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Removable Device"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Terminal"},
#if SEC_NET_TTY
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Host"},
#endif
};

extern int desdp_doit(), desdr_doit(), desdt_doit();
#if SEC_NET_TTY
extern int desdh_doit();
#endif

Menu_scrn desdm_menu [] = {
	ROUT_DESC(1, desdp_doit, 0, NULL),
	ROUT_DESC(2, desdr_doit, 0, NULL),
	ROUT_DESC(3, desdt_doit, 0, NULL),
#if SEC_NET_TTY
	ROUT_DESC(3, desdh_doit, 0, NULL),
#endif
};

Scrn_hdrs desdm_hdrs = {
	desdm_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, MT, cmds_line1
};

SKIP_PARMS(desdm_scrn, SCR_MENU, desdm_desc, desdm_menu, NULL, &desdm_hdrs);

/*
	SELECT DEVICE SCREEN
*/

uchar desd_terminal[] = "Select Terminal from the following list";
uchar desd_printer[] = "Select Printer from the following list";
uchar desd_removable[] = "Select Import/Export device from the following list:";

#if SEC_NET_TTY
char desd_host[] = "Select Host from the following list:";
#endif

Scrn_desc desd_desc [] = {
	{ 0, 15, FLD_PROMPT, 0, FLD_OUTPUT,  NO, NULL },
	{ 2, 24, FLD_SCRTOG, 14, FLD_BOTH,   NO, NULL, NULL,
		8, 4, 2 },
};

Menu_scrn desd_menu [] = {
	ROUT_DESC (1, NULL, 1, NULL),
};

uchar desd_title [] = "SELECT DEVICE";
Scrn_hdrs desd_hdrs = {
	desd_title, cur_date, runner, cur_time,
	NULL, cur_dev, NULL,
	MT, MT, cmds_line1
};

SKIP_PARMF(desd_scrn, SCR_FILLIN, desd_desc, desd_menu, NULL, &desd_hdrs,
	NULL, NULL, NULL);

/*
	DEVICES ADD/MODIFY/DISPLAY/REMOVE PRINTER SCREEN
*/

Scrn_desc deptr_desc [] = {
	{ 0, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Printer name:"},
	{ 0, 44, FLD_ALPHA,  14, FLD_BOTH,    NO, NULL},

	{ 2, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Device list:"},
	{ 2, 44, FLD_SCROLL, DEVICEWIDTH, FLD_BOTH,    NO, NULL, NULL,
		4, 0, 1},

#if SEC_MAC
	{ 7, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Device labels"},
	{ 7, 43, FLD_PROMPT,  1, FLD_OUTPUT,  NO, "SL" },
	{ 8, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "------ ------"},
	{ 8, 43, FLD_PROMPT,  1, FLD_OUTPUT,  NO, "--" },
	{ 9, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Single-level"},
	{ 9, 43, FLD_TOGGLE,  1, FLD_BOTH,    NO},
	{10, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Multilevel"},
	{10, 43, FLD_TOGGLE,  1, FLD_BOTH,    NO},
#endif /* SEC_MAC */
};

Menu_scrn deptr_menu [] = {
	ROUT_DESC (1, NULL, 0, NULL),
	ROUT_DESC (2, NULL, 0, NULL),
#if SEC_MAC
	ROUT_DESC (3, NULL, 0, NULL),
	ROUT_DESC (4, NULL, 0, NULL),
#endif /* SEC_MAC */
};

uchar deadp_title[] = "ADD PRINTER";
uchar demdp_title[] = "MODIFY PRINTER";
uchar dermp_title[] = "REMOVE PRINTER";
uchar dedsp_title[] = "DISPLAY PRINTER";

Scrn_hdrs deptr_hdrs = {
	deadp_title, cur_date, runner, cur_time,
	NULL, cur_dev, NULL,
	cmds_screen, cmds_titem, cmds_line1
};

SKIP_PARMS (deptr_scrn, SCR_FILLIN, deptr_desc, deptr_menu, NULL, &deptr_hdrs);


/*
	DEVICES ADD/DISPLAY/DELETE/MODIFY REMOVABLE DEVICE SCREEN
*/

Scrn_desc dermv_desc [] = {
	{ 0, 30, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Device name:"},
	{ 0, 44, FLD_ALPHA,  14, FLD_BOTH,    NO, NULL},

	{ 2, 30, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Device list:"},
	{ 2, 44, FLD_SCROLL, 16, FLD_BOTH,    NO, NULL, NULL,
		4, 0, 1},

#if SEC_MAC
	{ 7, 30, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Device labels"},
	{ 7, 49, FLD_PROMPT,  1, FLD_OUTPUT,  NO, "SL"},
	{ 8, 30, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "------ ------"},
	{ 8, 49, FLD_PROMPT,  1, FLD_OUTPUT,  NO, "--"},
	{ 9, 30, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Single-level"},
	{ 9, 49, FLD_TOGGLE,  1, FLD_BOTH,    NO},
	{10, 30, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Multilevel"},
	{10, 49, FLD_TOGGLE,  1, FLD_BOTH,    NO},
#endif

#if SEC_ARCH
	{12, 30, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Control parameters"},
	{12, 50, FLD_PROMPT,  1, FLD_OUTPUT,  NO, "Set"},
	{13, 30, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "------- ----------"},
	{13, 50, FLD_PROMPT,  1, FLD_OUTPUT,  NO, "---"},
	{14, 30, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Enable for import"},
	{14, 51, FLD_TOGGLE,  1, FLD_BOTH,    NO},
	{15, 30, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Enable for export"},
	{15, 51, FLD_TOGGLE,  1, FLD_BOTH,    NO},
#endif
};

Menu_scrn dermv_menu [] = {
	ROUT_DESC (1, NULL, 0, NULL),
	ROUT_DESC (2, NULL, 0, NULL),
#if SEC_MAC
	ROUT_DESC (3, NULL, 0, NULL),
	ROUT_DESC (4, NULL, 0, NULL),
#endif /* SEC_MAC */
#if SEC_ARCH
	ROUT_DESC (5, NULL, 0, NULL),
	ROUT_DESC (6, NULL, 0, NULL),
#endif /* SEC_ARCH */
};

uchar deadr_title [] = "ADD REMOVABLE DEVICE";
uchar demdr_title [] = "MODIFY REMOVABLE DEVICE";
uchar dedsr_title [] = "DISPLAY REMOVABLE DEVICE";
uchar dermr_title [] = "REMOVE REMOVABLE DEVICE";

Scrn_hdrs dermv_hdrs = {
	deadr_title, cur_date, runner, cur_time,
	NULL, cur_dev, NULL,
	cmds_screen, cmds_titem, cmds_line1
};

SKIP_PARMS(dermv_scrn, SCR_FILLIN, dermv_desc, dermv_menu, NULL, &dermv_hdrs);



/*
	DEVICES ADD/REMOVE/MODIFY/DISPLAY TERMINAL SCREEN
*/

Scrn_desc detrm_desc [] = {
	{ 0, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Terminal name:"},
	{ 0, 44, FLD_ALPHA,  14, FLD_BOTH,    NO, NULL},

	{ 2, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Device list:"},
	{ 2, 44, FLD_SCROLL, 16, FLD_BOTH,    NO, NULL, NULL,
		4, 0, 1},

	{ 7,  6, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"Maximum number of unsuccessful logins" },
	{ 7, 51, FLD_NUMBER,  3, FLD_BOTH,    NO},
	{ 7, 56, FLD_TOGGLE,  7, FLD_BOTH,    NO, "Default" },
	{ 7, 65, FLD_NUMBER,  3, FLD_OUTPUT,  NO },
	{ 8,  6, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"Login timeout (seconds)"},
	{ 8, 51, FLD_NUMBER,  3, FLD_BOTH,    NO},
	{ 8, 56, FLD_TOGGLE,  7, FLD_BOTH,    NO, "Default"},
	{ 8, 65, FLD_NUMBER,  3, FLD_OUTPUT,  NO },
	{ 9,  6, FLD_PROMPT,  0, FLD_OUTPUT,  NO,
		"Delay between unsuccessful logins (seconds)"},
	{ 9, 51, FLD_NUMBER,  3, FLD_BOTH,    NO},
	{ 9, 56, FLD_TOGGLE,  7, FLD_BOTH,    NO, "Default"},
	{ 9, 65, FLD_NUMBER,  3, FLD_OUTPUT,  NO },

#if SEC_MAC
	{11, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Device labels"},
	{12, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "------ ------"},
	{11, 44, FLD_PROMPT,  1, FLD_OUTPUT,  NO, "SL"},
	{12, 44, FLD_PROMPT,  1, FLD_OUTPUT,  NO, "--"},
	{13, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Single-level"},
	{13, 44, FLD_TOGGLE,  1, FLD_BOTH,    NO},
	{14, 29, FLD_PROMPT,  0, FLD_OUTPUT,  NO, "Multilevel"},
	{14, 44, FLD_TOGGLE,  1, FLD_BOTH,    NO},
#endif /* SEC_MAC */
};

Menu_scrn detrm_menu [] = {
	ROUT_DESC (1, NULL, 0, NULL),
	ROUT_DESC (2, NULL, 0, NULL),
	ROUT_DESC (3, NULL, 0, NULL),
	ROUT_DESC (4, NULL, 0, NULL),
	ROUT_DESC (5, NULL, 0, NULL),
	ROUT_DESC (6, NULL, 0, NULL),
	ROUT_DESC (7, NULL, 0, NULL),
	ROUT_DESC (8, NULL, 0, NULL),
#if SEC_MAC
	ROUT_DESC (9, NULL, 0, NULL),
	ROUT_DESC (10, NULL, 0, NULL),
#endif
};

uchar deadt_title[] = "ADD TERMINAL";
uchar demdt_title[] = "MODIFY TERMINAL";
uchar dermt_title[] = "REMOVE TERMINAL";
uchar dedst_title[] = "DISPLAY TERMINAL";

Scrn_hdrs detrm_hdrs = {
	deadt_title, cur_date, runner, cur_time,
	NULL, cur_dev, NULL,
	cmds_screen, cmds_titem, cmds_line1
};

SKIP_PARMS (detrm_scrn, SCR_FILLIN, detrm_desc, detrm_menu, NULL, &detrm_hdrs);

/*
	DEVICE AUTHORIZED USER LIST SCREEN
 */

Scrn_desc deaus_desc[] = {
	{  0, 20, FLD_TOGGLE, 17, FLD_BOTH, NO, "All Users Allowed" },
	{  2, 20, FLD_PROMPT,  0, FLD_OUTPUT, NO, "Authorized User List:" },
	{  4, 20, FLD_SCRTOG,  9, FLD_BOTH, NO, NULL, NULL,
		6, 4, 4 },
};

extern int deaus_exit(), deaus_setup(), deaus_init(), deaus_free();

Menu_scrn deaus_menu[] = {
	ROUT_DESC(1, deaus_exit, 0, NULL),
	ROUT_DESC(2, deaus_exit, 0, NULL),
};

uchar deaus_title[] = "DEVICE AUTHORIZED USER LIST";

Scrn_hdrs deaus_hdrs = {
	deaus_title, cur_date, runner, cur_time,
	NULL, cur_dev, NULL,
	cmds_screen, cmds_titem, cmds_line1
};

SKIP_PARMF (deaus_scrn, SCR_FILLIN, deaus_desc, deaus_menu, NULL, &deaus_hdrs,
	deaus_setup, deaus_init, deaus_free);

/*
	ADD DEVICES MENU
*/

Scrn_desc dead_desc [] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Printer"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Removable Device"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Terminal"},
};

extern int deadp_doit(), deadr_doit(), deadt_doit();

Menu_scrn dead_menu [] = {
	ROUT_DESC (1, deadp_doit, 0, NULL),
	ROUT_DESC (2, deadr_doit, 0, NULL),
	ROUT_DESC (3, deadt_doit, 0, NULL),
};

uchar dead_title [] = "ADD DEVICES";
Scrn_hdrs dead_hdrs = {
	dead_title, cur_date, runner, cur_time,
	NULL, cur_dev, NULL,
	MT, MT, cmds_line1
};

SCRN_PARMS (dead_scrn, SCR_MENU, dead_desc, dead_menu, NULL, &dead_hdrs);

#if SEC_MAC
/*
	DEVICE SENSITIVITY LABELS MENU
*/

Scrn_desc desl_desc [] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Multilevel Minimum SL"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Multilevel Maximum SL"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Single-level SL"},
};

extern int demnsl(), demxsl(), dessl();

Menu_scrn desl_menu [] = {
	ROUT_DESC (1, demnsl, 0, NULL),
	ROUT_DESC (2, demxsl, 0, NULL),
	ROUT_DESC (3, dessl, 0, NULL),
};

uchar desl_title [] = "DEVICE SENSITIVITY LABELS";
Scrn_hdrs desl_hdrs = {
	desl_title, cur_date, runner, cur_time,
	NULL, NULL, NULL,
	MT, MT, cmds_line1
};

SCRN_PARMS (desl_scrn, SCR_MENU, desl_desc, desl_menu, NULL, &dead_hdrs);

#endif /* SEC_MAC */

/*
	MAIN DEVICES MENU
*/

Scrn_desc de_desc [] = {
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Defaults"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Select Device"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Add Device"},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Modify Device"},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Remove Device"},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Display Device"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Lock Terminal"},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Unlock Terminal"},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Authorized User List"},
#if SEC_MAC
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Sensitivity Labels"},
#endif /* SEC_MAC */
};

extern int demd_doit(), derd_doit(), dedd_doit(), delck_doit(), deunlck_doit();

Menu_scrn de_menu [] = {
	MENU_DESC (1, &dede_scrn, NULL),
	MENU_DESC (2, &desdm_scrn, NULL),
	MENU_DESC (3, &dead_scrn, NULL),
	ROUT_DESC (4, demd_doit, 0, NULL),
	ROUT_DESC (5, derd_doit, 0, NULL),
	ROUT_DESC (6, dedd_doit, 0, NULL),
	ROUT_DESC (7, delck_doit,   0, NULL),
	ROUT_DESC (8, deunlck_doit, 0, NULL),
	MENU_DESC (9, &deaus_scrn, NULL),
#if SEC_MAC
	MENU_DESC (10, &desl_scrn, NULL),
#endif /* SEC_MAC */
};

uchar de_title [] = "DEVICES";
Scrn_hdrs de_hdrs = {
	de_title, cur_date, runner, cur_time,
	NULL, cur_dev, NULL,
	MT, MT, cmds_line1
};

SCRN_PARMS (de_scrn, SCR_MENU, de_desc, de_menu, NULL, &de_hdrs);

#if defined(DEBUG) || defined(PRINTSCR)
print_de_scrns()
{
	printscreen (&de_scrn);
	printscreen (&dede_scrn);
	printscreen (&decp_scrn);
	printscreen (&desmi_scrn);
	printscreen (&desma_scrn);
	printscreen (&desd_scrn);
	printscreen (&dead_scrn);
	printscreen (&deadp_scrn);
	printscreen (&deadr_scrn);
	printscreen (&deadt_scrn);
	printscreen (&derd_scrn);
	printscreen (&derdp_scrn);
	printscreen (&derdr_scrn);
	printscreen (&derdt_scrn);
}
#endif /* DEBUG || PRINTSCR */
