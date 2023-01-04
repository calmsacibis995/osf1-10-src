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
static char	*sccsid = "@(#)$RCSfile: aif_scrns.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 23:57:22 $";
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
 * aif_scrns.c - main & miscellaneous aif screen data structures
 */


#define GL_ALLOCATE

#include	<sys/types.h>
#include	"userif.h"
/*
 * the next two must stay so for global variable allocation
 */
#include	"valid.h"
#include	"logging.h"


/***********************************************************

                        HELP SCREENS

***********************************************************/

/*
	FUNDAMENTALS HELP SCREEN
*/

char *helpf_text[] = {
	"The Trusted Facility Management Package for this trusted system",
	"allows you to administer your system without learning the",
	"complexities of control file setup.  With this system, you can",
	"perform most aspects of system setup with minimal expertise in",
	"the UNIX system.  All control tasks are menu driven and simplify",
	"the complex task of performing day-to-day security maintenance",
	"and administration.  Please select 'Help on Keys' to view a",
	"screeen that describes the key sequences understood by this program.",
	"For more information, consult your 'Trusted Facility Manual'",
	"supplied as part of your system's security documentation.",
	NULL
};

uchar helpf_title [] = "PACKAGE FUNDAMENTALS";
Scrn_hdrs helpf_hdrs = {
	helpf_title, NULL, NULL, NULL,
	NULL, NULL, NULL,
	MT, MT, cmds_anykey
};

SCRN_PARMS (helpf_scrn, SCR_TEXT, NULL, NULL, helpf_text, &helpf_hdrs);

/*
	HELP WITH HELP SCREEN
*/

char *helph_text[] = {
	"You may select context-sensitivity help on any screen or field",
	"by pressing the help key '^Y'.  This will bring up a brief",
	"explanatory message describing the actions that you may perform",
	"at the current spot in the menu hierarchy and on the current screen.",
	NULL
};

uchar helph_title [] = "HOW HELP WORKS";
Scrn_hdrs helph_hdrs = {
	helph_title, NULL, NULL, NULL,
	NULL, NULL, NULL,
	MT, MT, cmds_anykey
};

SCRN_PARMS (helph_scrn, SCR_TEXT, NULL, NULL, helph_text, &helph_hdrs);

/*
	KEY HELP SCREEN
*/

char *helpk_text[] = {
	"The key sequences you use are constant across all screens.",
	"To execute any screen, press the 'Enter' or 'Return' key.",
	"Within a menu screen, use the arrow keys or 'Tab' to move",
	"around within the screen.  In other screens, 'Tab' and '^T'",
	"move forwards and backwards within fields.  To exit any screen,",
	"type the Escape key.  To quit the program entirely, type '^B'.",
	"Other keys are described in the Trusted Facility Manual.",
	NULL
};

uchar helpk_title [] = "USING THE KEYBOARD";
Scrn_hdrs helpk_hdrs = {
	helpk_title, NULL, NULL, NULL,
	NULL, NULL, NULL,
	MT, MT, cmds_anykey
};

SCRN_PARMS (helpk_scrn, SCR_TEXT, NULL, NULL, helpk_text, &helpk_hdrs);

/*
	MAIN HELP MENU
*/

Scrn_desc he_desc [] = {
/* row, col, type, len, inout, required, prompt, help */
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Fundamentals"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Help"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Keys"},
};

Menu_scrn he_menu [] = {
	MENU_DESC (1, &helpf_scrn, NULL),
	MENU_DESC (2, &helph_scrn, NULL),
	MENU_DESC (3, &helpk_scrn, NULL),
};

uchar he_title [] = "HELP MENU";
Scrn_hdrs he_hdrs = {
	he_title, NULL, NULL, NULL,
	NULL, NULL, NULL,
	MT, MT, cmds_line1
};

SCRN_PARMS (he_scrn, SCR_MENU, he_desc, he_menu, NULL, &he_hdrs);

/***********************************************************

                        ABOUT SCREENS

***********************************************************/

char *ab_text [] = {
	"System Management Package",
	"",
	"Version:  0.1 Alpha",
	"Released: October, 1990",
	"",
	"Copyright SecureWare, Inc. 1990",
	"Atlanta, Ga, USA. All rights reserved.",
	NULL
};

uchar ab_title [] = "ABOUT THIS PACKAGE";
Scrn_hdrs ab_hdrs = {
	ab_title, NULL, NULL, NULL,
	NULL, NULL, NULL,
	MT, MT, cmds_anykey
};

SCRN_PARMS (ab_scrn, SCR_TEXT, NULL, NULL, ab_text, &ab_hdrs);

/***********************************************************

	MAIN MENU

***********************************************************/

Scrn_desc	pm1_isso_desc[] = {
/* row, col, type, len, inout, required, prompt, help */
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Audit",
		"aif,audit"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Accounts",
		"aif,accounts"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Devices",
		"aif,devices"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Help",
		"aif,help"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "About",
		"aif,about"},
};

extern Scrn_parms au_scrn, ac_scrn, de_scrn;

Menu_scrn	pm1_isso_menu[] = {
/* choice, type, next_menu, next_routine, next_program */
	MENU_DESC (1, &au_scrn, NULL),
	MENU_DESC (2, &ac_scrn, NULL),
	MENU_DESC (3, &de_scrn, NULL),
	MENU_DESC (4, &he_scrn, NULL),
	MENU_DESC (5, &ab_scrn, NULL),
};

SCRN_PARMS (pm1_isso_scrn, SCR_MENU, pm1_isso_desc, pm1_isso_menu,
	NULL, &main_hdrs);

Scrn_parms *TopScrn;

Scrn_desc	pm1_sysadmin_desc[] = {
/* row, col, type, len, inout, required, prompt, help */
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Accounts",
		"aif,accounts"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "Help",
		"aif,help"},
	{ -1, -1, FLD_SKIP, 0, FLD_INPUT,  NO, NULL},
	{ -1, -1, FLD_CHOICE, 0, FLD_INPUT,  NO, "About",
		"aif,about"},
};

Menu_scrn	pm1_sysadmin_menu[] = {
/* choice, type, next_menu, next_routine, next_program */
	MENU_DESC (1, &ac_scrn, NULL),
	MENU_DESC (2, &he_scrn, NULL),
	MENU_DESC (3, &ab_scrn, NULL),
};

SCRN_PARMS (pm1_sysadmin_scrn, SCR_MENU, pm1_sysadmin_desc, pm1_sysadmin_menu,
	NULL, &main_hdrs);

#if defined(DEBUG) || defined(PRINTSCR)
print_if_scrns()
{
	printscreen (&pm1_scrn);

	print_au_scrns ();
	print_ac_scrns ();
	print_de_scrns ();

	printscreen (&he_scrn);
	printscreen (&helpf_scrn);
	printscreen (&helph_scrn);
	printscreen (&helpk_scrn);

	printscreen (&ab_scrn);
}
#endif /* DEBUG || PRINTSCR */
