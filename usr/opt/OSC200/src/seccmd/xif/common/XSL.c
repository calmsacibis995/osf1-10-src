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
static char	*sccsid = "@(#)$RCSfile: XSL.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:05:21 $";
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

#include <sys/secdefines.h>
#if SEC_BASE
#if SEC_MAC

/**************************************************************************
 *
 * Copyright (c) 1989, 1990 SecureWare, Inc.  All rights reserved.
 *
 * This file is based on WmSL.c from the mwm.
 *
 * This module contains routines to implement the user interface for
 * setting program and default sensitivity levels as well as new user
 * clearance values.
 *
 *************************************************************************/


/*
    filename:
        XSL.c
        
    copyright:
        Copyright (c) 1989, 1990 SecureWare Inc.
        ALL RIGHTS RESERVED
    
    function:
        X windows front end for all sensitivity/information label screens
        
    entry points:
*/

/* Common C include files */
#include <stdio.h>
#ifdef AUX
#include <string.h>
#else
#include <strings.h>
#endif
#include <sys/types.h>
#include <sys/security.h>

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/Shell.h>
#include <X11/Xatom.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/MessageB.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/ToggleBG.h>

#include <sys/errno.h>
#include <sys/security.h>
#include <prot.h>

#include <mandatory.h>
#include "XMain.h"

/* Here we cheat, we want the Encodings file definitions to make this code 
   easier to understand, therefore we load in a modified copy of std_labels.h
   This is gruesome but it works. */

#if SEC_MAC_OB
#include "ostd_labels.h"

/* If it is the Orange Book B1 code then we will cheat some more */
/* Prevents lots of undefines - simpler to fix here than the code */
#define MARKWORDS CATWORDS
#endif /* SEC_MAC_OB */

/* Role program  include files */
#include "XMain.h"
#include "XAccounts.h"

/* External routines */
extern char 
	** alloc_table(),
	*Calloc(),
	*Malloc(),
	*Realloc();

/*
 * Routines defined
 */

extern char 	*extract_normal_string();
#if ! SEC_SHW
extern void 	UserClearClose();
extern void 	UserClearOK();
extern void 	DefClearClose();
extern void 	DefClearOK();
extern void 	SinUserClClose();
extern void 	SinUserClOK();
extern void 	DevDefnslClose();
extern void 	DevDefnslOK();
extern void 	DevDefxslClose();
extern void 	DevDefxslOK();
extern void 	DevDefsslClose();
extern void 	DevDefsslOK();
extern void 	DevModnslClose();
extern void 	DevModnslOK();
extern void 	DevModxslClose();
extern void 	DevModxslOK();
extern void 	DevModsslClose();
extern void 	DevModsslOK();
extern void	AudColnslClose();
extern void	AudColnslOK();
extern void	AudColxslClose();
extern void	AudColxslOK();
extern void	AudSelnssClose();
extern void	AudSelnssOK();
extern void	AudSelxssClose();
extern void	AudSelxssOK();
extern void	AudSelnosClose();
extern void	AudSelnosOK();
extern void	AudSelxosClose();
extern void	AudSelxosOK();
#endif /* ! SEC_SHW */
#if SEC_ILB
extern void 	DevDefsilClose();
extern void 	DevDefsilOK();
extern void 	DevModsilClose();
extern void 	DevModsilOK();
#endif /* SEC_ILB */
extern int	CheckAuditEnabled();
extern void	LoadMessage();
extern void	CreateThreeButtons();
#if SEC_MAC_OB
extern void	mac_ob_init();
#endif /* SEC_MAC_OB */

static void	CreateClearanceScreen();
static void	CreateSLScreen();
static void	CreateILScreen();
static void	InitializeSL();
static Widget 	CreateScreen();
static Widget	CreateModes();
static Widget	CreateThisFuture();
static void	CreateSLButtons();
static int	SetScreenFromClear();
static int	SetScreenFromSL();
static void	CreateMessage();
static void	ButtonCB();
static void	ModeCB();
static void	BuildCB();
static void	CheckClear();
static void	CheckSL();
#if SEC_ILB
static int	SetScreenFromIL();
static void	CheckLabel();
#endif
static void	TextModifyCB();
static void	SetSLText();
static char 	*GetSLText();
static void 	SetLabelText();
static char 	*GetLabelText();
static void	SetDfltText();
static char 	*GetDfltText();
static void	SetButton2OK();
static void	SetButton2Check();
static void	ToggleCB();
static void	HelpCB();
static void	SLError();
static void	SLErrorCB();
static void	b_convert();
static void	word_to_string();
static int	WordInDialog();
static void	ToggleButtonSetSensitivity();
static void	ToggleButtonSetState();
static void	RaiseClassificationButtons();
static void	SetToggleNames();
static void	SetToggleButtons();

/*
 * External routines
 */

extern void	MemoryError();
extern char 	*calloc();

/*
 * Definitions
 */

/* Argument array size */

#define DISPLAY		(XtDisplay(main_shell))
#define MAX_ARGS		20

#define	CLASSIFICATION_TOGGLE	1
#define COMPARTMENTS_TOGGLE	2
#define MARKINGS_TOGGLE		3


/* Label mode Callbacks */

#if SEC_ILB
#define LABEL_IL			1
#define LABEL_SL			2
#define LABEL_BOTH			3

/* Label mode callbacks for Mode == LABEL_BOTH */

#define BUILD_IL			1
#define BUILD_SL			2
#endif /* SEC_ILB */


/* Function codes */

#define SL_USER_CLEARANCE		1	/* User clearance */
#define SL_DEFAULT_CLEARANCE		2	/* User's default clearance */
#define SL_SINGLE_USER_SL		3	/* Single user clearance */
#define SL_DEVICES_DEF_MIN_SL		4
#define SL_DEVICES_DEF_MAX_SL		5
#define SL_DEVICES_DEF_SINGLE_LEVEL_SL	6
#define SL_DEVICES_DEF_SINGLE_LEVEL_IL	7
#define SL_DEVICES_MOD_MIN_SL		8
#define SL_DEVICES_MOD_MAX_SL		9
#define SL_DEVICES_MOD_SINGLE_LEVEL_SL	10
#define SL_DEVICES_MOD_SINGLE_LEVEL_IL	11
#define SL_AUDIT_COL_MIN_SL		12
#define SL_AUDIT_COL_MAX_SL		13
#define SL_AUDIT_SEL_SUB_MIN_SL		14
#define SL_AUDIT_SEL_SUB_MAX_SL		15
#define SL_AUDIT_SEL_OBJ_MIN_SL		16
#define SL_AUDIT_SEL_OBJ_MAX_SL		17
#define SL_EXAMPLE_IL_AND_SL		-1	/* Not used. The code had been
						 * put in but was not needed. 
						 * Left in for future use.
						 */
#define SL_MAXIMUM		18

/* Callbacks for ButtonCB() */

#define	BUTTON_OK		1		/* OK button */
#define	BUTTON_CHECK		2		/* Check button */
#define	BUTTON_CANCEL		3		/* Cancel button */
#define	BUTTON_DEFAULT		4		/* Set Default button */
#define	BUTTON_HELP		5		/* Help button */

/*
 * Local Variables
 */

static Boolean	IsTyping = False;	/* Are we typing or not ? */

static int initialized = 0;
static int errorstate = 0;

/* Label build buffer */

static char label_buffer[4096];		/* static buffer for label storage */

/* Error message codes */

#define SL_PARSE_ERROR		1

static char
	**msg_error_1,
	**msg_sl,
	*msg_error_1_text,
	*msg_sl_text;

/*
 * The various CLASSIFICATIONs, COMPARTMENTS, and MARKINGS needed for the
 * main program, where s mean sensitivity label, i means information label,
 * and uc means user clearance.
 */

static CLASSIFICATION ucclass, sclass, iclass, maximum_class = -1;
static COMPARTMENTS *uccomps, *scomps, *icomps, *maximum_comps;
static MARKINGS *smarks, *imarks;

static char	*banner_buffer;
static char	*sparse_table[SL_MAXIMUM];
static char	*iparse_table[SL_MAXIMUM];

static int NumberOfClassifications[SL_MAXIMUM];
static int NumberOfCompartments[SL_MAXIMUM];

static Boolean	ilevelwildcard = False;
static Boolean	slevelwildcard = False;
static short length_words = LONG_WORDS;
static int	max_label_size = 3;
/*
 * Define all the Widgets to be used
 */

static Widget	SLText[SL_MAXIMUM];
static Widget	DfltButton[SL_MAXIMUM];
static Widget	DfltText[SL_MAXIMUM];
static Widget	ClassificationRC[SL_MAXIMUM];
static Widget	CompartmentRC[SL_MAXIMUM];
static Widget	MarkingsW[SL_MAXIMUM];
static Widget	MarkingsLabel[SL_MAXIMUM];
static Widget	MarkingRC[SL_MAXIMUM];
static Widget	*ClassificationButtons[SL_MAXIMUM];
static Widget	*CompartmentButtons[SL_MAXIMUM];
static Widget	ButtonOK[SL_MAXIMUM];
static Widget	ILModeToggle[SL_MAXIMUM];
static Widget	SLModeToggle[SL_MAXIMUM];
static Widget	BothModeToggle[SL_MAXIMUM];
static Widget	ILLabelToggle[SL_MAXIMUM];
static Widget	SLLabelToggle[SL_MAXIMUM];
static Widget	ThisToggle[SL_MAXIMUM];
static Widget	FutureToggle[SL_MAXIMUM];

static int 	CalledFor = 0;		/* call flag */

#if SEC_ILB
static int	file_set_mode = LABEL_BOTH;
static int	saved_file_set_mode = 0;
static int	build_mode = BUILD_IL;
#endif /* SEC_ILB */

void
CreateUserClearance(parent)
	Widget parent;
{
	CreateClearanceScreen(parent, SL_USER_CLEARANCE, True);
}


void
SetUserClearance(use_users_sl, dflt_sl, sl, max_class, max_comps)
	Boolean	use_users_sl;
	char *dflt_sl;
	char *sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_USER_CLEARANCE;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	ucclass = max_class;
	COMPARTMENT_MASK_COPY (uccomps, max_comps);
	XmTextSetString(DfltText[CalledFor], dflt_sl);
	IsTyping = False;
	if (use_users_sl) {
		ToggleButtonSetState(DfltButton[CalledFor], False);
		SetScreenFromClear(sl);
	}
	else {
		ToggleButtonSetState(DfltButton[CalledFor], True);
		SetScreenFromClear(dflt_sl);
	}
}

void
CreateDefClearance(parent)
	Widget parent;
{
	CreateClearanceScreen(parent, SL_DEFAULT_CLEARANCE, False);
}

void
SetDefClearance(dflt_sl, max_class, max_comps)
	char *dflt_sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_DEFAULT_CLEARANCE;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	ucclass = max_class;
	COMPARTMENT_MASK_COPY (uccomps, max_comps);
	IsTyping = False;
	SetScreenFromClear(dflt_sl);
}

void
CreateSingleUserSL(parent)
	Widget parent;
{
	CreateSLScreen(parent, SL_SINGLE_USER_SL, False, False);
}


void
SetSingleUserSL(dflt_sl, max_class, max_comps)
	char *dflt_sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_SINGLE_USER_SL;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	IsTyping = False;
	SetScreenFromSL(dflt_sl);
}

void
CreateDevDefnsl(parent)
	Widget parent;
{
	CreateSLScreen(parent, SL_DEVICES_DEF_MIN_SL, False, False);
}


void
SetDevDefnsl(dflt_sl, max_class, max_comps)
	char *dflt_sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_DEVICES_DEF_MIN_SL;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	IsTyping = False;
	SetScreenFromSL(dflt_sl);
}

void
CreateDevDefxsl(parent)
	Widget parent;
{
	CreateSLScreen(parent, SL_DEVICES_DEF_MAX_SL, False, False);
}


void
SetDevDefxsl(dflt_sl, max_class, max_comps)
	char *dflt_sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_DEVICES_DEF_MAX_SL;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	IsTyping = False;
	SetScreenFromSL(dflt_sl);
}

void
CreateDevDefssl(parent)
	Widget parent;
{
	CreateSLScreen(parent, SL_DEVICES_DEF_SINGLE_LEVEL_SL, False, False);
}


void
SetDevDefssl(dflt_sl, max_class, max_comps)
	char *dflt_sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_DEVICES_DEF_SINGLE_LEVEL_SL;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	IsTyping = False;
	SetScreenFromSL(dflt_sl);
}

#if SEC_ILB
void
CreateDevDefsil(parent)
	Widget parent;
{
	CreateILScreen(parent, SL_DEVICES_DEF_SINGLE_LEVEL_IL, False);
}


void
SetDevDefsil(dflt_il, dflt_sl, max_class, max_comps)
	char *dflt_il;
	char *dflt_sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_DEVICES_DEF_SINGLE_LEVEL_IL;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	IsTyping = False;
	/* Example IL
	XmToggleButtonGadgetSetState(BothModeToggle[CalledFor], True, True);
	XmToggleButtonGadgetSetState(ILLabelToggle[CalledFor], True, True);
	*/
	file_set_mode = LABEL_IL;
	build_mode = BUILD_IL;

	SetScreenFromIL(dflt_il);
}
#endif /* SEC_ILB */

void
CreateDevModnsl(parent)
	Widget parent;
{
	CreateSLScreen(parent, SL_DEVICES_MOD_MIN_SL, True, False);
}


void
SetDevModnsl(use_users_sl, dflt_sl, sl, max_class, max_comps)
	Boolean	use_users_sl;
	char *dflt_sl;
	char *sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_DEVICES_MOD_MIN_SL;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	XmTextSetString(DfltText[CalledFor], dflt_sl);
	IsTyping = False;
	if (use_users_sl) {
		ToggleButtonSetState(DfltButton[CalledFor], False);
		SetScreenFromSL(sl);
	}
	else {
		ToggleButtonSetState(DfltButton[CalledFor], True);
		SetScreenFromSL(dflt_sl);
	}
}
void
CreateDevModxsl(parent)
	Widget parent;
{
	CreateSLScreen(parent, SL_DEVICES_MOD_MAX_SL, True, False);
}


void
SetDevModxsl(use_users_sl, dflt_sl, sl, max_class, max_comps)
	Boolean	use_users_sl;
	char *dflt_sl;
	char *sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_DEVICES_MOD_MAX_SL;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	XmTextSetString(DfltText[CalledFor], dflt_sl);
	IsTyping = False;
	if (use_users_sl) {
		ToggleButtonSetState(DfltButton[CalledFor], False);
		SetScreenFromSL(sl);
	}
	else {
		ToggleButtonSetState(DfltButton[CalledFor], True);
		SetScreenFromSL(dflt_sl);
	}
}
void
CreateDevModssl(parent)
	Widget parent;
{
	CreateSLScreen(parent, SL_DEVICES_MOD_SINGLE_LEVEL_SL, True, False);
}


void
SetDevModssl(use_users_sl, dflt_sl, sl, max_class, max_comps)
	Boolean	use_users_sl;
	char *dflt_sl;
	char *sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_DEVICES_MOD_SINGLE_LEVEL_SL;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	XmTextSetString(DfltText[CalledFor], dflt_sl);
	IsTyping = False;
	if (use_users_sl) {
		ToggleButtonSetState(DfltButton[CalledFor], False);
		SetScreenFromSL(sl);
	}
	else {
		ToggleButtonSetState(DfltButton[CalledFor], True);
		SetScreenFromSL(dflt_sl);
	}
}

#if SEC_ILB
void
CreateDevModsil(parent)
	Widget parent;
{
	CreateILScreen(parent, SL_DEVICES_MOD_SINGLE_LEVEL_IL, True);
}


void
SetDevModsil(use_users_il, dflt_il, il, max_class, max_comps)
	Boolean	use_users_il;
	char *dflt_il;
	char *il;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_DEVICES_MOD_SINGLE_LEVEL_IL;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	file_set_mode = LABEL_IL;
	build_mode = BUILD_IL;
	XmTextSetString(DfltText[CalledFor], dflt_il);
	IsTyping = False;
	if (use_users_il) {
		ToggleButtonSetState(DfltButton[CalledFor], False);
		SetScreenFromIL(il);
	}
	else {
		ToggleButtonSetState(DfltButton[CalledFor], True);
		SetScreenFromIL(dflt_il);
	}
}
#endif /* SEC_ILB */

void
CreateAudColnsl(parent)
	Widget parent;
{
	CreateSLScreen(parent, SL_AUDIT_COL_MIN_SL, False, True);
}


void
SetAudColnsl(dflt_sl, max_class, max_comps)
	char *dflt_sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_AUDIT_COL_MIN_SL;
	XmToggleButtonGadgetSetState(ThisToggle[CalledFor], False, False);
	XtSetSensitive(ThisToggle[CalledFor], CheckAuditEnabled() );
	XmToggleButtonGadgetSetState(FutureToggle[CalledFor], True, False);
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	IsTyping = False;
	SetScreenFromSL(dflt_sl);
}

void
CreateAudColxsl(parent)
	Widget parent;
{
	CreateSLScreen(parent, SL_AUDIT_COL_MAX_SL, False, True);
}


void
SetAudColxsl(dflt_sl, max_class, max_comps)
	char *dflt_sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_AUDIT_COL_MAX_SL;
	XmToggleButtonGadgetSetState(ThisToggle[CalledFor], False, False);
	XtSetSensitive(ThisToggle[CalledFor], CheckAuditEnabled() );
	XmToggleButtonGadgetSetState(FutureToggle[CalledFor], True, False);
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	IsTyping = False;
	SetScreenFromSL(dflt_sl);
}

void
CreateAudSelnss(parent)
	Widget parent;
{
	CreateSLScreen(parent, SL_AUDIT_SEL_SUB_MIN_SL, False, False);
}


void
SetAudSelnss(dflt_sl, max_class, max_comps)
	char *dflt_sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_AUDIT_SEL_SUB_MIN_SL;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	IsTyping = False;
	SetScreenFromSL(dflt_sl);
}

void
CreateAudSelxss(parent)
	Widget parent;
{
	CreateSLScreen(parent, SL_AUDIT_SEL_SUB_MAX_SL, False, False);
}


void
SetAudSelxss(dflt_sl, max_class, max_comps)
	char *dflt_sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_AUDIT_SEL_SUB_MAX_SL;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	IsTyping = False;
	SetScreenFromSL(dflt_sl);
}

void
CreateAudSelnos(parent)
	Widget parent;
{
	CreateSLScreen(parent, SL_AUDIT_SEL_OBJ_MIN_SL, False, False);
}


void
SetAudSelnos(dflt_sl, max_class, max_comps)
	char *dflt_sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_AUDIT_SEL_OBJ_MIN_SL;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	IsTyping = False;
	SetScreenFromSL(dflt_sl);
}

void
CreateAudSelxos(parent)
	Widget parent;
{
	CreateSLScreen(parent, SL_AUDIT_SEL_OBJ_MAX_SL, False, False);
}


void
SetAudSelxos(dflt_sl, max_class, max_comps)
	char *dflt_sl;
	CLASSIFICATION max_class;
	COMPARTMENTS *max_comps;
{
	CalledFor = SL_AUDIT_SEL_OBJ_MAX_SL;
	maximum_class = max_class;
	COMPARTMENT_MASK_COPY (maximum_comps, max_comps);
	sclass = max_class;
	COMPARTMENT_MASK_COPY (scomps, max_comps);
	IsTyping = False;
	SetScreenFromSL(dflt_sl);
}

static void
CreateSLScreen(parent, option, default_button, this_future)
	Widget parent;
	int	option;
	Boolean	default_button;
	Boolean this_future;    /* Draw This/Future buttons. Used by audit */
{
	Widget 	w;
	int	error;
	int	num_class, num_comps;

	/* Make sure we have initialized everything */
	if(!initialized) 
		InitializeSL();

	CalledFor = option;
	maximum_class = mand_syshi->class;
	COMPARTMENT_MASK_COPY (maximum_comps, mand_syshi->cat);
	sclass = maximum_class;
	COMPARTMENT_MASK_COPY (scomps, maximum_comps);
	w = CreateScreen(parent, default_button);
	XtManageChild(w);

	/* Put the names in */
	error = l_parse(mand_ir_to_er(mand_syslo), &sclass, scomps, smarks,
			l_sensitivity_label_tables,
			l_lo_sensitivity_label->l_classification,
			l_lo_sensitivity_label->l_compartments,
			maximum_class, maximum_comps);
	if(error != -1) {
		SLError(SL_PARSE_ERROR);
		return;
	}

	/* Get the banner */

	sparse_table[CalledFor] = Calloc(
		l_sensitivity_label_tables->l_num_entries, 1);

	NumberOfClassifications[CalledFor] = 0;
	NumberOfCompartments[CalledFor] = 0;

	/*
	 * The starting sensitivity level for the SL screen is
	 * always system low. Any valid sensitivity level can
	 * be constructed up to the user's clearance. Thus, only
	 * make the buttons once and insure that the buttons
	 * that can not be chosen by the user are greyed out.
	 */

	num_class = 0;
	num_comps = 0;
	SetToggleNames( l_sensitivity_label_tables,
		l_lo_sensitivity_label->l_classification,
		l_lo_sensitivity_label->l_compartments,
		maximum_class, maximum_comps,
		sparse_table[CalledFor],
		ClassificationButtons[CalledFor], CompartmentButtons[CalledFor],
		&num_class, &num_comps,
		ClassificationRC[CalledFor], CompartmentRC[CalledFor], 
		MarkingRC[CalledFor]);

	/* If in audit we need This/Future buttons 
	 * 	    w  = Form,
	 * XtParent(w) = Frame for the SL widgets */
	if (this_future)
		w = CreateThisFuture(XtParent(w));

	/* Create the buttons */
	CreateSLButtons(XtParent(w));
}

#if SEC_ILB
static void
CreateILScreen(parent, option, default_button)
	Widget parent;
	int	option;
	Boolean	default_button;
{
	Widget 	w;
	int	error;
	int	num_class, num_comps;

	/* Make sure we have initialized everything */
	if(!initialized) 
		InitializeSL();

	CalledFor = option;
	maximum_class = mand_syshi->class;
	COMPARTMENT_MASK_COPY (maximum_comps, mand_syshi->cat);
	sclass = maximum_class;
	COMPARTMENT_MASK_COPY (scomps, maximum_comps);
	w = CreateScreen(parent, default_button);
	XtManageChild(w);

	/* Put the names in */
	error = l_parse(mand_ir_to_er(mand_syslo),
			&iclass, icomps, imarks,
			l_information_label_tables,
			l_lo_sensitivity_label->l_classification,
			l_lo_sensitivity_label->l_compartments,
			maximum_class, maximum_comps);
	if(error != -1) {
		SLError(SL_PARSE_ERROR);
		return;
	}

	/* Get the banner */

	sparse_table[CalledFor] = Calloc(
		l_sensitivity_label_tables->l_num_entries, 1);

	iparse_table[CalledFor] = Calloc(
		l_information_label_tables->l_num_entries, 1);

	NumberOfClassifications[CalledFor] = 0;
	NumberOfCompartments[CalledFor] = 0;

	/*
	 * The starting sensitivity level for the SL screen is
	 * always system low. Any valid sensitivity level can
	 * be constructed up to the user's clearance. Thus, only
	 * make the buttons once and insure that the buttons
	 * that can not be chosen by the user are greyed out.
	 */

	num_class = 0;
	num_comps = 0;
	SetToggleNames( l_information_label_tables,
		l_lo_sensitivity_label->l_classification,
		l_lo_sensitivity_label->l_compartments,
		maximum_class, maximum_comps,
		iparse_table[CalledFor],
		ClassificationButtons[CalledFor], CompartmentButtons[CalledFor],
		&num_class, &num_comps,
		ClassificationRC[CalledFor], CompartmentRC[CalledFor],
		MarkingRC[CalledFor]);

	/* Create the buttons */
	CreateSLButtons(XtParent(w));
}
#endif /* SEC_ILB */

static void
CreateClearanceScreen(parent, option, default_button)
	Widget parent;
	int	option;
	Boolean	default_button;
{
	Widget 	w;
	int	error;
	int	num_class, num_comps;

	/* Make sure we have initialized everything */
	if(!initialized) 
		InitializeSL();

	CalledFor = option;
	maximum_class = mand_syshi->class;
	COMPARTMENT_MASK_COPY (maximum_comps, mand_syshi->cat);
	sclass = maximum_class;
	COMPARTMENT_MASK_COPY (scomps, maximum_comps);
	ucclass = maximum_class;
	COMPARTMENT_MASK_COPY (uccomps, maximum_comps);
	w = CreateScreen(parent, default_button);
	XtManageChild(w);

	/* Put the names in */
	error = l_parse(mand_ir_to_er(mand_syslo), &ucclass, uccomps, imarks,
			l_clearance_tables,
			l_lo_clearance->l_classification,
			l_lo_clearance->l_compartments,
			maximum_class, maximum_comps);
	if(error != -1) {
		SLError(SL_PARSE_ERROR);
		return;
	}

	/* Get the banner */

	sparse_table[CalledFor] = Calloc(l_clearance_tables->l_num_entries, 1);

	NumberOfClassifications[CalledFor] = 0;
	NumberOfCompartments[CalledFor] = 0;

	/*
	 * The starting sensitivity level for the SL screen is
	 * always system low. Any valid sensitivity level can
	 * be constructed up to the user's clearance. Thus, only
	 * make the buttons once and insure that the buttons
	 * that can not be chosen by the user are greyed out.
	 */

	num_class = 0;
	num_comps = 0;
	SetToggleNames(l_clearance_tables, 
		l_lo_clearance->l_classification,
		l_lo_clearance->l_compartments,
		maximum_class, maximum_comps,
		sparse_table[CalledFor],
		ClassificationButtons[CalledFor], CompartmentButtons[CalledFor],
		&num_class, &num_comps,
		ClassificationRC[CalledFor], CompartmentRC[CalledFor],
		MarkingRC[CalledFor]);

	/* Create the buttons */
	CreateSLButtons(XtParent(w));
}

/*
 * Create memory to store all buttons/ toggles etc.
 */
void
InitializeSL()
{
	int i;
	int num_entries;

	/* Allocate buffers first time through */
	if(!initialized) {

#if SEC_MAC_OB
		mac_ob_init();
#endif /* SEC_MAC_OB */
		/* space for IL [SL] banner */
		max_label_size = 
#if SEC_ILB
			l_information_label_tables->l_max_length +
#endif /* SEC_ILB */
 		  l_sensitivity_label_tables->l_max_length + 3;
		if (max_label_size < 5) max_label_size = 5;
		banner_buffer = Calloc(max_label_size, 1);	
		if(banner_buffer ==(char *) NULL)
			MemoryError();

		/* Find the largest table and allocate enough space on each
		   screen to hold this. Slightly wasteful but not by too much */
		num_entries = l_sensitivity_label_tables->l_num_entries;
		if (num_entries < l_clearance_tables->l_num_entries)
			num_entries = l_clearance_tables->l_num_entries;
#if SEC_ILB
		if (num_entries < l_information_label_tables->l_num_entries)
			num_entries = l_information_label_tables->l_num_entries;
#endif /* SEC_ILB */
		num_entries++;

		/* Buttons should be able to hold the largest table */
		for (i=0; i<= SL_MAXIMUM; i++) {
			CompartmentButtons[i] =(Widget *) Calloc(
				num_entries, sizeof (Widget) );
			if(CompartmentButtons[i] ==(Widget *) NULL)
				MemoryError();

			ClassificationButtons[i] =(Widget *) Calloc(
				num_entries, sizeof (Widget) );
			if(ClassificationButtons[i] ==(Widget *) NULL)
				MemoryError();
		}

		uccomps = (COMPARTMENTS *) Calloc(CATWORDS , sizeof(mask_t));
		if (uccomps == (COMPARTMENTS *) NULL)
			MemoryError();

		scomps  = (COMPARTMENTS *) Calloc(CATWORDS , sizeof(mask_t));
		if (scomps == (COMPARTMENTS *) NULL)
			MemoryError();

		icomps  = (COMPARTMENTS *) Calloc(CATWORDS , sizeof(mask_t));
		if (icomps == (COMPARTMENTS *) NULL)
			MemoryError();

		maximum_comps  = (COMPARTMENTS *) Calloc(CATWORDS , 
			sizeof(mask_t));
		if (maximum_comps == (COMPARTMENTS *) NULL)
			MemoryError();

		smarks  = (MARKINGS *) Calloc(MARKWORDS, sizeof(mask_t));
		if (smarks == (MARKINGS *) NULL)
			MemoryError();

		imarks  = (MARKINGS *) Calloc(MARKWORDS, sizeof(mask_t));
		if (imarks == (MARKINGS *) NULL)
			MemoryError();

		/* Load messages in */
		LoadMessage ("msg_sensitivity_label", &msg_sl, &msg_sl_text);

		/* Mark as initialized now */
		initialized = 1;
	}
}

/*
 * CreateScreen()-create the sensitivity label constructor screen
 * including the text widget and the scrolled selector widgets.
 */

static Widget 
CreateScreen(parent, default_button) 
	Widget		parent;		/*  parent widget	*/
	Boolean		default_button;
{
	XmString	xmstring;
	Widget		w;
	Widget		Form;
	Widget 		ScrolledW;
	Widget		NewLabel;	/* General purpose label */
	Widget		Label;		/* General purpose label */
	Arg		args[MAX_ARGS];	/*  arg list		*/
	register int	n;		/*  arg count		*/

	/* Make sure we have initialized everything */
	if(!initialized) 
		InitializeSL();

	n = 0;
	Form = XmCreateForm(parent, "SLForm", args, n);

	/* Create the top-most label */

	if (default_button) {
		if (CalledFor == SL_USER_CLEARANCE)
			xmstring = XmStringLtoRCreate(msg_sl[5], charset);
		else if (CalledFor == SL_DEVICES_MOD_MIN_SL)
			xmstring = XmStringLtoRCreate(msg_sl[22], charset);
		else if (CalledFor == SL_DEVICES_MOD_MAX_SL)
			xmstring = XmStringLtoRCreate(msg_sl[22], charset);
		else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_SL)
			xmstring = XmStringLtoRCreate(msg_sl[22], charset);
		else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL)
			xmstring = XmStringLtoRCreate(msg_sl[22], charset);

		if (! xmstring)
			MemoryError();
		n = 0;
		XtSetArg(args[n], XmNleftAttachment	,XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNtopAttachment	,XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNalignment		,XmALIGNMENT_BEGINNING); n++;
		XtSetArg(args[n], XmNlabelString, xmstring); n++;

		DfltButton[CalledFor] = XmCreateToggleButtonGadget(Form,"DfltButton",args,n);
		XmStringFree(xmstring);
		XtManageChild(DfltButton[CalledFor]);
		XtAddCallback(DfltButton[CalledFor], XmNvalueChangedCallback, ButtonCB,
							BUTTON_DEFAULT);

		/* Create display for current default level-not editable */

		n = 0;
		XtSetArg(args[n], XmNeditMode      	,XmMULTI_LINE_EDIT); n++;
		XtSetArg(args[n], XmNwordWrap	      	,True); n++;
		XtSetArg(args[n], XmNscrollHorizontal	,False); n++;
		XtSetArg(args[n], XmNeditable		,False); n++;
		DfltText[CalledFor] = XmCreateScrolledText(Form,"DfltText",args,n);

		/* Must attach the scrolled text widget */

		n = 0;
		XtSetArg(args[n], XmNleftAttachment	,XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNrightAttachment	,XmATTACH_FORM); n++;
		XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNtopWidget     	,DfltButton[CalledFor]); n++;
		XtSetValues(XtParent(DfltText[CalledFor]),args,n);

		XtManageChild(DfltText[CalledFor]);
	}

	/* Create the default level label */
	if (CalledFor == SL_USER_CLEARANCE)
		xmstring = XmStringLtoRCreate(msg_sl[6], charset);
	else if (CalledFor == SL_DEFAULT_CLEARANCE)
		xmstring = XmStringLtoRCreate(msg_sl[7], charset);
	else if (CalledFor == SL_SINGLE_USER_SL)
		xmstring = XmStringLtoRCreate(msg_sl[8], charset);
	else if (CalledFor == SL_DEVICES_DEF_MIN_SL)
		xmstring = XmStringLtoRCreate(msg_sl[9], charset);
	else if (CalledFor == SL_DEVICES_DEF_MAX_SL)
		xmstring = XmStringLtoRCreate(msg_sl[10], charset);
	else if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_SL)
		xmstring = XmStringLtoRCreate(msg_sl[11], charset);
	else if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_IL)
		xmstring = XmStringLtoRCreate(msg_sl[12], charset);
	else if (CalledFor == SL_DEVICES_MOD_MIN_SL)
		xmstring = XmStringLtoRCreate(msg_sl[13], charset);
	else if (CalledFor == SL_DEVICES_MOD_MAX_SL)
		xmstring = XmStringLtoRCreate(msg_sl[14], charset);
	else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_SL)
		xmstring = XmStringLtoRCreate(msg_sl[15], charset);
	else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL)
		xmstring = XmStringLtoRCreate(msg_sl[16], charset);
	else if (CalledFor == SL_AUDIT_COL_MIN_SL)
		xmstring = XmStringLtoRCreate(msg_sl[28], charset);
	else if (CalledFor == SL_AUDIT_COL_MAX_SL)
		xmstring = XmStringLtoRCreate(msg_sl[29], charset);
	else if (CalledFor == SL_AUDIT_SEL_SUB_MIN_SL)
		xmstring = XmStringLtoRCreate(msg_sl[30], charset);
	else if (CalledFor == SL_AUDIT_SEL_SUB_MAX_SL)
		xmstring = XmStringLtoRCreate(msg_sl[31], charset);
	else if (CalledFor == SL_AUDIT_SEL_OBJ_MIN_SL)
		xmstring = XmStringLtoRCreate(msg_sl[32], charset);
	else if (CalledFor == SL_AUDIT_SEL_OBJ_MAX_SL)
		xmstring = XmStringLtoRCreate(msg_sl[33], charset);
	else
		xmstring = XmStringLtoRCreate(msg_sl[7], charset);

	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNwidth		,(Dimension) 580); n++;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_FORM); n++;
	if (default_button) {
		XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
		XtSetArg(args[n], XmNtopWidget		,XtParent(DfltText[CalledFor])); n++;
	}
	else {
		XtSetArg(args[n], XmNtopAttachment	,XmATTACH_FORM); n++;
	}
	XtSetArg(args[n], XmNrightAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNalignment		,XmALIGNMENT_BEGINNING); n++;

	/* Set the label string depending on function */

	XtSetArg(args[n], XmNlabelString, xmstring); n++;
	NewLabel = XmCreateLabel(Form,"Newlabel",args,n);
	XmStringFree(xmstring);
	XtManageChild(NewLabel);

	/* Create the text input widget */
	/* Really this is a single-line - It automatically wraps !!! */

	n = 0;
	XtSetArg(args[n], XmNeditMode      	,XmMULTI_LINE_EDIT); n++;
	XtSetArg(args[n], XmNwordWrap	      	,True); n++;
	XtSetArg(args[n], XmNrows		,2); n++;
	XtSetArg(args[n], XmNscrollHorizontal	,False); n++;
	SLText[CalledFor] = XmCreateScrolledText(Form,"SLText",args,n);

	/* Must attach the scrolled text widget */
	n = 0;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget     	,NewLabel); n++;
	XtSetValues(XtParent(SLText[CalledFor]),args,n);

	/* Switch text input on */

	XtAddCallback(SLText[CalledFor], XmNmodifyVerifyCallback, TextModifyCB, NULL);
	XtManageChild(SLText[CalledFor]);

#ifdef LATER
	/* CreateModes is not called by anything at the moment ... */
	if (CalledFor == SL_EXAMPLE_IL_AND_SL)
		w = CreateModes(Form, XtParent(SLText[CalledFor]) );
	else
#endif /* LATER */
		w = XtParent(SLText[CalledFor]);

	/* 
	 * Create the labels 
	 * First label is attached to the Text label. The List for this
	 * is attached to the bottom. The other List widgets are attached
	 * horizontally with their labels attached above them. For this
	 * reason we only create the first label now, and create the
	 * others later.
	 * Note we have to attach to the parent of SLText because the
	 * parent is the scrolled window.
	 */
	xmstring = XmStringLtoRCreate(msg_sl[1], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget     	,w); n++;
	XtSetArg(args[n], XmNlabelString	,xmstring); n++;
	Label = XmCreateLabel(Form,"label",args,n);
	XtManageChild(Label);
	XmStringFree(xmstring);

	/* For all scrolled window widgets we allow the sys admin guy to
	 * specify sizes in a resource file */
	n = 0;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget     	,Label); n++;
	XtSetArg(args[n], XmNscrollingPolicy	,XmAUTOMATIC);  n++;
	XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmSTATIC);  n++;
	if (default_button) {
		XtSetArg(args[n], XmNheight	,(Dimension) 160); n++;
	}
	else {
		XtSetArg(args[n], XmNheight	,(Dimension) 200); n++;
	}
	if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_IL ||
	    CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL ) {
		XtSetArg(args[n], XmNwidth		,(Dimension) 190); n++;
	} 
	else {
		XtSetArg(args[n], XmNwidth		,(Dimension) 255); n++;
	}
	ScrolledW = XmCreateScrolledWindow(Form, "Classification", args, n);
	XtManageChild(ScrolledW);

	n = 0;
	XtSetArg(args[n], XmNpacking		,XmPACK_COLUMN);  n++;
	XtSetArg(args[n], XmNnumColumns		,(short) 1);  n++;
	ClassificationRC[CalledFor] = XmCreateRadioBox(ScrolledW, "row_column", args, n);
	XtManageChild(ClassificationRC[CalledFor]);

	/* Make the compartments */
	xmstring = XmStringLtoRCreate(msg_sl[2], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget     	,ScrolledW); n++;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget     	,w); n++;
	XtSetArg(args[n], XmNlabelString	,xmstring); n++;
	Label = XmCreateLabel(Form,"label",args,n);
	XmStringFree(xmstring);
	XtManageChild(Label);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget     	,Label); n++;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget     	,ScrolledW); n++;
	XtSetArg(args[n], XmNscrollingPolicy	,XmAUTOMATIC);  n++;
	XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmSTATIC);  n++;
	if (default_button) {
		XtSetArg(args[n], XmNheight	,(Dimension) 160); n++;
	}
	else {
		XtSetArg(args[n], XmNheight	,(Dimension) 200); n++;
	}
	if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_IL ||
	    CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL ) {
		XtSetArg(args[n], XmNwidth		,(Dimension) 190); n++;
	} 
	else {
		XtSetArg(args[n], XmNwidth		,(Dimension) 255); n++;
	}
	ScrolledW = XmCreateScrolledWindow(Form, "Compartments", args, n);
	XtManageChild(ScrolledW);

	n = 0;
	XtSetArg(args[n], XmNpacking		,XmPACK_COLUMN);  n++;
	XtSetArg(args[n], XmNnumColumns		,(short) 1);  n++;

	CompartmentRC[CalledFor] = XmCreateRowColumn(ScrolledW, "row_column", args, n);
	XtManageChild(CompartmentRC[CalledFor]);

#if SEC_ILB
	/* Make the markings */
	/* Only do this for information labels */
	if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_IL ||
	    CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL ) {
	xmstring = XmStringLtoRCreate(msg_sl[21], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget     	,ScrolledW); n++;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget     	,w); n++;
	XtSetArg(args[n], XmNlabelString	,xmstring); n++;
	MarkingsLabel[CalledFor] = XmCreateLabel(Form,"label",args,n);
	XmStringFree(xmstring);
	XtManageChild(MarkingsLabel[CalledFor]);

	n = 0;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget     	,MarkingsLabel[CalledFor]); n++;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget     	,ScrolledW); n++;
	XtSetArg(args[n], XmNscrollingPolicy	,XmAUTOMATIC);  n++;
	XtSetArg(args[n], XmNscrollBarDisplayPolicy, XmSTATIC);  n++;
	if (default_button) {
		XtSetArg(args[n], XmNheight	,(Dimension) 160); n++;
	}
	else {
		XtSetArg(args[n], XmNheight	,(Dimension) 200); n++;
	}
	XtSetArg(args[n], XmNwidth		,(Dimension) 190); n++;
	MarkingsW[CalledFor] = XmCreateScrolledWindow(Form, "Compartments", 
		args, n);
	XtManageChild(MarkingsW[CalledFor]);

	n = 0;
	XtSetArg(args[n], XmNpacking		,XmPACK_COLUMN);  n++;
	XtSetArg(args[n], XmNnumColumns		,(short) 1);  n++;

	MarkingRC[CalledFor] = XmCreateRowColumn(MarkingsW[CalledFor], 
		"row_column", args, n);
	XtManageChild(MarkingRC[CalledFor]);

	} /* End of creation of Markings RC - only for IL screens */
#endif /* SEC_ILB */

	/* 
	 * Return Form.
	 */
	return(Form);
}

#if SEC_ILB
/* Create mode switches for IL/SL screens */
static Widget
CreateModes(parent, topw)
	Widget parent;
	Widget	topw;
{
	XmString xmstring;
	XmString wildcard;
	Widget	Label, Sep;
	Widget 	ConfirmToggle;
	Widget 	WildcardToggle;
	Widget 	LabelRC;
	Widget	ModeRC;
	Arg		args[MAX_ARGS];	/*  arg list		*/
	register int	n;		/*  arg count		*/

#ifdef OLD_CODE
	/* Create the options and buttons label */

	n = 0;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget		,topw); n++;
	XtSetArg(args[n], XmNlabelString,
		XmStringLtoRCreate(OPTIONS,charset)); n++;

	Label = XmCreateLabel(parent,"label",args,n);
	XtManageChild(Label);
#endif

	/* Create a separator between the buttons and text widget */

	n = 0;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget     	,topw); n++;
#if SEC_SHW
	XtSetArg(args[n], XmNmargin		, 6); n++;

	XtSetArg(args[n], XmNlabelString,
		XmStringLtoRCreate(" ",charset)); n++;

	Sep = XmCreateLabel(parent,"label",args,n);
	XtManageChild(Sep);
#else
	XtSetArg(args[n], XmNmargin		, 3); n++;

	Sep = XmCreateSeparator(parent, "Sep", args, n);
	XtManageChild(Sep);
#endif /* SEC_SHW */

#ifdef OLD_CODE
	/* Create the WILDCARD toggle */
	wildcard = XmStringLtoRCreate("Wildcard Label",charset);
	n = 0;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget     	,Sep); n++;
	XtSetArg(args[n], XmNlabelString	,wildcard); n++;

	WildcardToggle = XmCreateToggleButtonGadget(parent, "Toggle", args, n);

	XmStringFree(wildcard);
	XtManageChild(WildcardToggle);

	/* OLD CODE
	XtAddCallback(WildcardToggle, XmNvalueChangedCallback, WildcardCB,
									NULL);
			*/

	/* Place the confirm label toggle below WILDCARD */

	n = 0;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget     	,WildcardToggle); n++;
	XtSetArg(args[n], XmNset	     	,True); n++;
	XtSetArg(args[n], XmNlabelString	,
	    XmStringCreateLtoR("Confirm Label Change ",charset)); n++;

	ConfirmToggle = XmCreateToggleButtonGadget(parent, "Toggle", args, n);

	XtManageChild(ConfirmToggle);
#endif

#if ! SEC_SHW
	/* Create a radio box to contain mode toggle buttons */

	n = 0;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_FORM); n++;
#ifdef OLD_CODE
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget		,ConfirmToggle); n++;
#endif
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget     	,Sep); n++;
	XtSetArg(args[n], XmNpacking		,XmPACK_COLUMN);  n++;
	XtSetArg(args[n], XmNnumColumns		,(short) 1);  n++;

	ModeRC = XmCreateRadioBox(parent, "row_column", args, n);
	XtManageChild(ModeRC);

	/* Create and manage three mode buttons in the radio box */

	xmstring = XmStringLtoRCreate(msg_sl[23], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString	,xmstring); n++;
	ILModeToggle[CalledFor] = XmCreateToggleButtonGadget(
		ModeRC,"Toggle",args,n);
	XtAddCallback(ILModeToggle[CalledFor], XmNvalueChangedCallback, 
		ModeCB, LABEL_IL);
	XtManageChild(ILModeToggle[CalledFor]);

	xmstring = XmStringLtoRCreate(msg_sl[24], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString	,xmstring); n++;

	SLModeToggle[CalledFor] = XmCreateToggleButtonGadget(
		ModeRC,"Toggle",args,n);
	XtAddCallback(SLModeToggle[CalledFor], XmNvalueChangedCallback, 
		ModeCB, LABEL_SL);
	XtManageChild(SLModeToggle[CalledFor]);

	xmstring = XmStringLtoRCreate(msg_sl[25], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNset, True); n++;
	XtSetArg(args[n], XmNlabelString	,xmstring); n++;
	BothModeToggle[CalledFor] = XmCreateToggleButtonGadget(
		ModeRC,"Toggle",args,n);
	XtAddCallback(BothModeToggle[CalledFor],XmNvalueChangedCallback,
		ModeCB,LABEL_BOTH);
	XtManageChild(BothModeToggle[CalledFor]);

	/* Create a radio box to contain two label mode toggle buttons */

	n = 0;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNleftWidget		,ModeRC); n++;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget     	,Sep); n++;
	XtSetArg(args[n], XmNpacking		,XmPACK_COLUMN);  n++;
	XtSetArg(args[n], XmNnumColumns		,(short) 1);  n++;

	LabelRC = XmCreateRadioBox(parent, "row_column", args, n);
	XtManageChild(LabelRC);

	/* Create and manage two mode buttons in the radio box */

	xmstring = XmStringLtoRCreate(msg_sl[26], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNset, True); n++;
	XtSetArg(args[n], XmNlabelString	,xmstring); n++;
	ILLabelToggle[CalledFor] = XmCreateToggleButtonGadget(
		LabelRC,"Toggle",args,n);

	XtAddCallback(ILLabelToggle[CalledFor], XmNvalueChangedCallback, 
		BuildCB, BUILD_IL);

	XtManageChild(ILLabelToggle[CalledFor]);

	xmstring = XmStringLtoRCreate(msg_sl[27], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString	,xmstring); n++;
	SLLabelToggle[CalledFor] = XmCreateToggleButtonGadget(
		LabelRC,"Toggle",args,n);

	XtAddCallback(SLLabelToggle[CalledFor], XmNvalueChangedCallback, 
		BuildCB, BUILD_SL);

	XtManageChild(SLLabelToggle[CalledFor]);
#endif /* ! SEC_SHW */

	/* Create a separator between the buttons and lists */

	n = 0;
	XtSetArg(args[n], XmNleftAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment	,XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment	,XmATTACH_WIDGET); n++;
#if SEC_SHW
	XtSetArg(args[n], XmNtopWidget     	,Sep); n++;
	XtSetArg(args[n], XmNmargin		, 6); n++;

	XtSetArg(args[n], XmNlabelString,
		XmStringLtoRCreate(" ",charset)); n++;

	Sep = XmCreateLabel(parent,"label",args,n);
	XtManageChild(Sep);
#else
	XtSetArg(args[n], XmNtopWidget     	,ModeRC); n++;
	XtSetArg(args[n], XmNmargin		, 3); n++;

	Sep = XmCreateSeparator(parent, "Sep", args, n);
	XtManageChild(Sep);
#endif /* SEC_SHW */
	return (Sep);
}
#endif /* SEC_ILB */


static Widget
CreateThisFuture(topw)
	Widget topw;
{
	XmString xmstring;
	Widget	this_or_next_widget;
	Arg		args[MAX_ARGS];	/*  arg list		*/
	register int	n;		/*  arg count		*/
	/* Row column widget to handle this/next buttons */
	n = 0;
	XtSetArg(args[n], XmNpacking,                   XmPACK_COLUMN); n++;
	XtSetArg(args[n], XmNorientation,                XmHORIZONTAL); n++;
	XtSetArg(args[n], XmNtopAttachment,           XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNtopWidget,                          topw); n++;
	XtSetArg(args[n], XmNleftAttachment,            XmATTACH_FORM); n++;
	this_or_next_widget = XmCreateRowColumn(XtParent(topw), "CurrentFuture",
	        args, n);
	XtManageChild(this_or_next_widget);

	/* Button to confirm for this session */
	n = 0;
	xmstring = XmStringCreate(msg_sl[17], charset);
	if (! xmstring)
	MemoryError();
	XtSetArg(args[n], XmNlabelString, xmstring); n++; 
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
	ThisToggle[CalledFor] = XmCreateToggleButtonGadget(this_or_next_widget,
	                 "ToggleButton", args, n);
	XtManageChild(ThisToggle[CalledFor]);
	XmStringFree(xmstring);
#ifdef TRAVERSAL
	XmAddTabGroup(confirm_this_session_widget);
#endif
	
	/* Button to confirm for next session */
	n = 0;
	xmstring = XmStringCreate(msg_sl[18], charset);
	if (! xmstring)
	MemoryError();
	XtSetArg(args[n], XmNlabelString, xmstring); n++; 
#ifdef TRAVERSAL
	XtSetArg(args[n], XmNtraversalOn,                       True); n++;
#endif
	FutureToggle[CalledFor] = XmCreateToggleButtonGadget(
	        this_or_next_widget, "ToggleButton", args, n);
	XtManageChild(FutureToggle[CalledFor]);
	XmStringFree(xmstring);
#ifdef TRAVERSAL
	XmAddTabGroup(confirm_next_session_widget);
#endif
	/* This is a little weird. We return one of the buttons because
	 * CreateThreeButtons assumes there is a frame around the SL widgets
	 */
	return (FutureToggle[CalledFor]);

}


/*
 * CreateSLButtons()-create the SL screen buttons for Check/Use, Help,
 * and Cancel.
 */

static void
CreateSLButtons(topw)
	Widget topw;
{
	Widget 	help_button, cancel_button;

	/* parent is the form containing the SL widgets. It's parent is
	 * the frame. The parent of that is the main form 
	 */
	CreateThreeButtons (XtParent(topw), topw, 
		&ButtonOK[CalledFor], &cancel_button, &help_button);
	XtAddCallback(ButtonOK[CalledFor], XmNactivateCallback, ButtonCB, 
						BUTTON_OK);
	XtAddCallback(cancel_button, XmNactivateCallback, ButtonCB, 
						BUTTON_CANCEL);
	XtAddCallback(help_button, XmNactivateCallback, ButtonCB, 
						BUTTON_HELP);
	return;
}

/*
 * SetScreenFromClear()-set the screen text widget and the scrolled lists
 * of toggles based on the passed sensitivity label value.
 */

static int
SetScreenFromClear(clearance)
	char *clearance;
{
	int		error;

	/* Parse the SL */

	error = l_parse(clearance, &ucclass, uccomps, imarks,
			l_clearance_tables,
			l_lo_clearance->l_classification,
			l_lo_clearance->l_compartments,
			maximum_class, maximum_comps);

	if(error != -1) {
		SLError(SL_PARSE_ERROR);
		return(0);
	}

	l_convert(banner_buffer, ucclass, l_long_classification,
		uccomps, l_t_markings, l_clearance_tables,
		sparse_table[CalledFor], LONG_WORDS, ALL_ENTRIES);
						   
	SetSLText(banner_buffer);
	
	SetToggleButtons(l_clearance_tables, sparse_table[CalledFor],
				ucclass, uccomps, imarks,
				maximum_class, maximum_comps,
				maximum_class, maximum_comps,
				l_lo_clearance->l_classification,
				l_lo_clearance->l_compartments,
				ClassificationButtons[CalledFor],
				CompartmentButtons[CalledFor]);
	
	return(1);
}


/*
 * SetScreenFromSL()-set the screen text widget and the scrolled lists
 * of toggles based on the passed sensitivity label value.
 */

static int
SetScreenFromSL(slabel)
	char *slabel;
{
	int		error;

	/* Parse the SL */

	error = l_parse(slabel, &sclass, scomps, smarks,
			l_sensitivity_label_tables,
			l_lo_sensitivity_label->l_classification,
			l_lo_sensitivity_label->l_compartments,
			maximum_class, maximum_comps);
	if(error != -1) {
		SLError(SL_PARSE_ERROR);
		return(0);
	}


	l_convert(banner_buffer, sclass, l_short_classification,
		scomps, l_t_markings, l_sensitivity_label_tables,
		sparse_table[CalledFor], SHORT_WORDS, ALL_ENTRIES);
						   
	SetSLText(banner_buffer);
	
	SetToggleButtons(l_sensitivity_label_tables, sparse_table[CalledFor],
				sclass, scomps, imarks,
				maximum_class, maximum_comps,
				maximum_class, maximum_comps,
				l_lo_sensitivity_label->l_classification,
				l_lo_sensitivity_label->l_compartments,
				ClassificationButtons[CalledFor],
				CompartmentButtons[CalledFor]);
	
	return(1);
}

#if SEC_ILB
/* 
 * SetScreenFromIL()-this routine validates the information/sensitivity label
 * combination typed in by the user. If the build mode is both, both labels
 * may be entered or a portion of a label may be edited. Wildcard labels
 * for either is handled. In a single label build mode, both labels may
 * be entered but if only a single label is typed, it must be the label
 * of the current mode type.
 *
 * Made from CheckLabel();
 */

static int
SetScreenFromIL(label)
	char *label;
{
	int error;

#if SEC_SHW
	error = l_parse(label, &iclass, icomps, imarks,
		l_information_label_tables,
		l_lo_sensitivity_label->l_classification,
		l_lo_sensitivity_label->l_compartments,
		sclass, scomps);
#else
	if(build_mode == LABEL_IL)
		error = l_parse(label, &iclass, icomps, imarks,
			l_information_label_tables,
			l_lo_sensitivity_label->l_classification,
			l_lo_sensitivity_label->l_compartments,
			sclass, scomps);
	else
		error = l_parse(label, &sclass, scomps, smarks,
			l_sensitivity_label_tables,
			l_lo_sensitivity_label->l_classification,
			l_lo_sensitivity_label->l_compartments,
			maximum_class, maximum_comps);
#endif /* SEC_SHW */

	/* Reflect a parse error with a popup message */

	if(error != -1) {
		SLError(SL_PARSE_ERROR);
		return (0);
	}

	l_b_convert(banner_buffer, iclass, 
		icomps, imarks, iparse_table[CalledFor],
		sclass, scomps, sparse_table[CalledFor], length_words);

	/* Place the new label into the text widget and set buttons */

#if SEC_SHW
	SetSLText(banner_buffer);

	SetToggleButtons(l_information_label_tables, 
		 iparse_table[CalledFor],
		 iclass, icomps, imarks,
		 sclass, scomps,
		 maximum_class, maximum_comps,
		 *l_min_classification,
		 l_in_compartments[*l_min_classification],
		 ClassificationButtons[CalledFor],
		 CompartmentButtons[CalledFor]);
#else
	if(file_set_mode == LABEL_BOTH)
		SetLabelText(banner_buffer,LABEL_BOTH);
	else if(file_set_mode == LABEL_IL)
		SetLabelText(banner_buffer,LABEL_IL);
	else
		SetLabelText(banner_buffer,LABEL_SL);

	if(build_mode == BUILD_IL)
		SetToggleButtons(l_information_label_tables, 
			 iparse_table[CalledFor],
			 iclass, icomps, imarks,
			 sclass, scomps,
			 maximum_class, maximum_comps,
			 *l_min_classification,
			 l_in_compartments[*l_min_classification],
			 ClassificationButtons[CalledFor],
			 CompartmentButtons[CalledFor]);

	else	/* Build mode is BUILD_SL */

		SetToggleButtons(l_sensitivity_label_tables, 
			 sparse_table[CalledFor],
			 sclass, scomps, smarks,
			 maximum_class, maximum_comps,
			 maximum_class, maximum_comps,
			 iclass, icomps,	/* IL is minimum */
			 ClassificationButtons[CalledFor],
			 CompartmentButtons[CalledFor]);
#endif /* SEC_SHW */
	return (1);
}
#endif /* SEC_ILB */


/* 
 * CheckClear()-when the user types in a label value into the text widget,
 * the Use button changes to a Check button that must be used to invoke
 * this function. The label is checked for validity before the OK
 * button is restored.
 */

static void
CheckClear(w)
	Widget w;
{
	char * sl_text;
	register char	*c;
	int error;
	register int	n;
	Arg 		args[MAX_ARGS];
	XmString	xmstring;
	Display		*display;
	XmTextPosition where;

	sl_text = GetSLText();

	/* Parse the SL */

	error = l_parse(sl_text, &ucclass, uccomps, imarks,
			l_clearance_tables,
			l_lo_clearance->l_classification,
			l_lo_clearance->l_compartments,
			maximum_class, maximum_comps);

	if(error == -1) {

		/* Now we should parse the input. */

		l_convert(banner_buffer, ucclass, l_long_classification,
			uccomps, l_t_markings, l_clearance_tables,
			sparse_table[CalledFor], LONG_WORDS, ALL_ENTRIES);
	
		/* Display the text */

		SetSLText(banner_buffer);

		/* Set the toggle buttons from the tables */

		SetToggleButtons(l_clearance_tables, sparse_table[CalledFor],
				 ucclass, uccomps, imarks,
				 maximum_class, maximum_comps,
				 maximum_class, maximum_comps,
 				 l_lo_clearance->l_classification,
 				 l_lo_clearance->l_compartments,
				 ClassificationButtons[CalledFor],
				 CompartmentButtons[CalledFor]);

		IsTyping = False;

		/* Change the Check label to the Use label */
		SetButton2OK ();
	}
	else {
		/* Error in setting the SL */

		n = 0;
		XtSetArg(args[n], XmNcursorPosition, &where); n++;
		XtGetValues(SLText[CalledFor], args, n);

		n = 0;
		XtSetArg(args[n], XmNcursorPosition,(XmTextPosition)error); n++;
		XtSetValues(SLText[CalledFor], args, n);
		display = XtDisplay(w);
		XBell(XtDisplay(w), 100);
	}

}

/* 
 * CheckSL()-when the user types in a label value into the text widget,
 * the Use button changes to a Check button that must be used to invoke
 * this function. The label is checked for validity before the USe
 * button is restored.
 */

static void
CheckSL(w)
	Widget w;
{
	char * sl_text;
	register char	*c;
	int error;
	register int	n;
	Arg 		args[MAX_ARGS];
	XmString	xmstring;
	XmTextPosition where;

	sl_text = GetSLText();

	/* Parse the SL */

	error = l_parse(sl_text, &sclass, scomps, smarks,
			l_sensitivity_label_tables,
			l_lo_sensitivity_label->l_classification,
			l_lo_sensitivity_label->l_compartments,
			maximum_class, maximum_comps);

	if(error == -1) {

		/* Now we should parse the input. */

		l_convert(banner_buffer, sclass, l_short_classification,
			scomps, l_t_markings, l_sensitivity_label_tables,
			sparse_table[CalledFor], SHORT_WORDS, ALL_ENTRIES);
	
		/* Display the text */

		SetSLText(banner_buffer);

		/* Set the toggle buttons from the tables */

		SetToggleButtons(l_sensitivity_label_tables, sparse_table[CalledFor],
			 sclass, scomps, imarks,
			 maximum_class, maximum_comps,
			 maximum_class, maximum_comps,
 			 l_lo_sensitivity_label->l_classification,
 			 l_lo_sensitivity_label->l_compartments,
			 ClassificationButtons[CalledFor],
			 CompartmentButtons[CalledFor]);

		IsTyping = False;

		/* Change the Check label to the Use label */

		SetButton2OK();
	}
	else {
		/* Error in setting the SL */

		n = 0;
		XtSetArg(args[n], XmNcursorPosition, &where); n++;
		XtGetValues(SLText[CalledFor], args, n);

		n = 0;
		XtSetArg(args[n], XmNcursorPosition,(XmTextPosition)error); n++;
		XtSetValues(SLText[CalledFor], args, n);
		/*
		XBell(XtDisplay(w), 100);
		*/
	}

}

#if SEC_ILB
/* 
 * CheckLabel()-this routine validates the information/sensitivity label
 * combination typed in by the user. If the build mode is both, both labels
 * may be entered or a portion of a label may be edited. Wildcard labels
 * for either is handled. In a single label build mode, both labels may
 * be entered but if only a single label is typed, it must be the label
 * of the current mode type.
 */

static void
CheckLabel()
{
	char *il_text = NULL;
	char *text = NULL;
	char *sl_text = NULL;
	Boolean swild = False;
	Boolean		iwild = False;
	register char	*c;
	int error;
	register int	n;
	Arg 		args[MAX_ARGS];

	/* Determine current label mode and process accodingly */

	switch(file_set_mode) {

	/* Must enter both labels for full IL/SL specification */

	    case LABEL_BOTH:

		il_text = GetLabelText(LABEL_IL, &iwild);
		sl_text = GetLabelText(LABEL_SL, &swild);

		/*
		 * If both strings are not present, then the only label that
		 * may be changed is the one specified by the current build
		 * mode. If that label is not present but the other is, it
		 * is an error.
		 */

		if((!il_text && !sl_text) ||
		   ((build_mode == BUILD_IL) && !il_text) ||
		   ((build_mode == BUILD_SL) && !sl_text)) {
			error = 0;
			break;
		}

		/* Form combined string if both present */

		if(il_text && sl_text && !iwild && !swild) {
			text = (char *) Malloc(strlen(il_text) +
							strlen(sl_text) + 2);
			if(!text) {
				error = 0;
				if(il_text)
					free(il_text);
				if(sl_text)
					free(sl_text);
				break;
			}

			strcpy(text,il_text);
			strcat(text," ");
			strcat(text,sl_text);
		}

		/*
		 * If an IL was typed in, then check to see if it is a
		 * wildcard label. If so, set the class/comps/marks per
		 * wildcard. Otherwise, parse the new IL value. If there
		 * is not an IL, leave the current class/comps/marks alone.
		 */

		if(il_text && iwild) {

			/* Set IL to minimum allowed */

			iclass = *l_min_classification;
			COMPARTMENTS_COPY(icomps, l_in_compartments[iclass]);
			MARKINGS_COPY(imarks, l_in_markings[iclass]);
			error = -1;
		}

		/*
		 * If an SL was typed in, then check to see if it is a
		 * wildcard label. If so, set the class/compartments per
		 * wildcard. Otherwise, parse the new SL value. If there
		 * is not an SL, leave the current class/comps alone.
		 */

		if(sl_text && swild) {

			/* Set SL to maximum allowed */

			sclass = maximum_class;
			COMPARTMENTS_COPY(scomps, maximum_comps);
			error = -1;
		}

		/*
		 * If both labels were entered, then validate the new label
		 * using the l_b_parse() routine. Otherwise, the only valid
		 * label is determined by the current label display mode.
		 */

		if(il_text && sl_text && !iwild && !swild)
		{
			error = l_b_parse(text, 
				&iclass, icomps, imarks,
				&sclass, scomps,
				maximum_class, maximum_comps,
				TRUE);
		}
		else if(il_text && !iwild && sl_text && swild)
		{
			error = l_parse(il_text, 
				&iclass, icomps, imarks,
				l_information_label_tables,
				l_lo_sensitivity_label->l_classification,
				l_lo_sensitivity_label->l_compartments,
				sclass, scomps);
		}
		else if(sl_text && !swild && il_text && iwild)
		{
		    char *bracket;

		    /*
		     * If the combined string contains an IL wildcard,
		     * then the SL must be parsed alone. Remove the
		     * brackets from the string to do this.
		     */

		    if(iwild) {

			bracket = strchr(sl_text, '[');
			if(bracket) {
				bracket++;

				text = (char *) Malloc(strlen(sl_text) + 1);
				strcpy(text,bracket);
				bracket = strrchr(text,']');
				if(bracket)
					*bracket = '\0';
			}
			else
				text = sl_text;

		    }
		    else
			text = sl_text;

		    /* Parse the label */

		    error = l_parse(text, 
				&sclass, scomps, smarks,
				l_sensitivity_label_tables,
				l_lo_sensitivity_label->l_classification,
				l_lo_sensitivity_label->l_compartments,
				maximum_class, maximum_comps);

		    /* Release buffer if allocated for bracket parse */

		    if(text && (text != sl_text))
			free(text);
		}

		break;

	    case LABEL_IL:

		/* Get the typed IL value */
		il_text = GetLabelText(LABEL_IL, &iwild);

		/*
		 * Check to see if the label is a wildcard. If so, handle
		 * it as a special case. Otherwise, parse the IL label.
		 */

		if(iwild) {

			/* Set IL to minimum allowed */

			iclass = *l_min_classification;
			COMPARTMENTS_COPY(icomps, l_in_compartments[iclass]);
			MARKINGS_COPY(imarks, l_in_markings[iclass]);
			error = -1;
		}
		else  {
			error = l_parse(il_text, 
				&iclass, icomps, imarks,
				l_information_label_tables,
				l_lo_sensitivity_label->l_classification,
				l_lo_sensitivity_label->l_compartments,
				sclass, scomps);
		}
		break;

	    case LABEL_SL:
	    {
		char *bracket;

		sl_text = GetLabelText(LABEL_SL, &swild);

		/* Make copy of string to eliminate the brackets for parse */

		bracket = strchr(sl_text, '[');
		if(bracket) {
			bracket++;

			text = (char *) Malloc(strlen(sl_text) + 1);
			strcpy(text,bracket);
			bracket = strrchr(text,']');
			if(bracket)
				*bracket = '\0';
		}
		else
			text = sl_text;

		/*
		 * Check to see if the label is a wildcard. If so, handle
		 * it as a special case. Otherwise, parse the SL label.
		 */

		if(swild) {

			/* Set SL to maximum allowed */

			sclass = maximum_class;
			COMPARTMENTS_COPY(scomps, maximum_comps);
			error = -1;
		}
		else  {
			error = l_parse(text, 
				&sclass, scomps, smarks,
				l_sensitivity_label_tables,
				l_lo_sensitivity_label->l_classification,
				l_lo_sensitivity_label->l_compartments,
				maximum_class, maximum_comps);
		}

		/* Delete text buffer if allocated for brackets */

		if(text && (text != sl_text))
			free(text);

		break;

	    }

	    default:

		error = 0;
		break;
	}

	if(error == -1) {

		/* Convert to string */

		b_convert(banner_buffer,
				iclass, icomps, imarks, iwild, iparse_table,
		       		sclass, scomps, swild, sparse_table, 
				length_words);
	
		/* Display the text */

		if(file_set_mode == LABEL_BOTH)
			SetLabelText(banner_buffer,LABEL_BOTH);
		else if(file_set_mode == LABEL_IL)
			SetLabelText(banner_buffer,LABEL_IL);
		else
			SetLabelText(banner_buffer,LABEL_SL);

		/* Set wildcard global flags if applicable */

		if(swild)
			slevelwildcard = True;
		else if(sl_text)
			slevelwildcard = False;

		if(iwild)
			ilevelwildcard = True;
		else if(il_text)
			slevelwildcard = False;

		/* Adjust sensitivities and push buttons */

		if((build_mode == BUILD_IL) && iwild) {

		    /*
		    XmToggleButtonGadgetSetState(WildcardToggle, True, False);
		    */

		    /* De-sensitize the compartments and markings */

		    XtSetSensitive(CompartmentRC[CalledFor], False);
		    XtSetSensitive(MarkingRC[CalledFor], False);

		    /* Set the toggle buttons to reflect label */

		    SetToggleButtons(l_information_label_tables, 
				 iparse_table[CalledFor],
				 iclass, icomps, imarks,
				 sclass, scomps,
				 maximum_class, maximum_comps,
				 *l_min_classification,
				 l_in_compartments[*l_min_classification],
				 ClassificationButtons[CalledFor],
				 CompartmentButtons[CalledFor]);

		    /* Raise the classification buttons */

		    RaiseClassificationButtons(ClassificationButtons[CalledFor],
			NumberOfClassifications[CalledFor]);
		}
		else if((build_mode == BUILD_SL) && swild) {

		    /*
		    XmToggleButtonGadgetSetState(WildcardToggle, True, False);
		    */

		    /* De-sensitize the compartments and markings */

		    XtSetSensitive(CompartmentRC[CalledFor], False);
		    XtSetSensitive(MarkingRC[CalledFor], False);

		    /* Set the toggle buttons to reflect label */

		    SetToggleButtons(l_sensitivity_label_tables, 
				 sparse_table[CalledFor],
				 sclass, scomps, smarks,
				 maximum_class, maximum_comps,
				 maximum_class, maximum_comps,
				 iclass, icomps,	/* IL is mimimum */
				 ClassificationButtons[CalledFor],
				 CompartmentButtons[CalledFor]);

		    /* Raise the classification buttons */

		    RaiseClassificationButtons(ClassificationButtons[CalledFor],
			NumberOfClassifications[CalledFor]);
		}
		else {
			/*
			if(XmToggleButtonGadgetGetState(WildcardToggle)) {
				XmToggleButtonGadgetSetState(WildcardToggle,
								False, False);
				XtSetSensitive(ClassificationRC[CalledFor], True);
				XtSetSensitive(CompartmentRC[CalledFor], True);
				XtSetSensitive(MarkingRC[CalledFor], True);
			}
			*/

			if(build_mode == BUILD_IL)
				SetToggleButtons(l_information_label_tables, 
				    iparse_table[CalledFor],
				    iclass, icomps, imarks,
				    sclass, scomps,
				    maximum_class, maximum_comps,
				    *l_min_classification,
				    l_in_compartments[*l_min_classification],
				    ClassificationButtons[CalledFor],
				    CompartmentButtons[CalledFor]);
			else
				SetToggleButtons(l_sensitivity_label_tables, 
				    sparse_table[CalledFor],
				    sclass, scomps, smarks,
				    maximum_class, maximum_comps,
				    maximum_class, maximum_comps,
				    iclass, icomps,	/* IL is minimum */
				    ClassificationButtons[CalledFor],
				    CompartmentButtons[CalledFor]);
		}

		/* Typing mode is disabled for a valid label */

		if(IsTyping == True) {

			IsTyping = False;

			/* Change Check button to Use new label button */

			/*
			n = 0;
			XtSetArg(args[n], XmNlabelString,
			   XmStringCreateLtoR(" Use \nNew Label",charset)); n++;
			XtSetValues(ButtonApply, args, n);
			*/
			SetButton2OK();

			/* Modify the callback associated with the button */

			/*
			XtRemoveCallback(ButtonApply, XmNactivateCallback,
					ButtonCB, BUTTON_CHECK);
			XtRemoveCallback(FileSB, XmNokCallback, ButtonCB,
					BUTTON_CHECK);
			XtAddCallback(ButtonApply, XmNactivateCallback,
					ButtonCB, BUTTON_APPLY_INTERNAL);
			XtAddCallback(FileSB, XmNokCallback, ButtonCB,
					BUTTON_OK);
			*/
		}

	}
	else {
		/* Error in setting the label */

		XmTextPosition where;

		n = 0;
		XtSetArg(args[n], XmNcursorPosition, &where); n++;
		XtGetValues(SLText[CalledFor], args, n);

		n = 0;
		XtSetArg(args[n],XmNcursorPosition,(XmTextPosition) error); n++;
		XtSetValues(SLText[CalledFor], args, n);

		XBell(DISPLAY, 100);
	}

	/* Release any string space */

	if(il_text && sl_text && text)
		free(text);

	if(il_text)
		free(il_text);

	if(sl_text)
		free(sl_text);
}
#endif /* SEC_ILB */


/*
 * TextModifyCB()-this callback is invoked when the user first types into
 * the text widget so that the button and callback list can be changed.
 */

static void
TextModifyCB(w, client_data, call_data) 
Widget		w;		/*  widget id		*/
caddr_t		client_data;	/*  data from application   */
caddr_t		call_data;	/*  data from widget class  */
{
	Arg args[2];
	register int n;
	XmString	xmstring;

	/* 
	 * If first time in then change sensitivty of OK and CHECK
	 */

	if(!IsTyping) {

		IsTyping = True;

		/* Turn the default button off */
		if (CalledFor == SL_USER_CLEARANCE 		|| 
		    CalledFor == SL_DEVICES_MOD_MIN_SL 		||
		    CalledFor == SL_DEVICES_MOD_MAX_SL 		||
		    CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_SL	||
		    CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL	     )
			ToggleButtonSetState(DfltButton[CalledFor], False);

		/* Change the Use label to the Check label */

		SetButton2Check();
	}
}

/*
 * SetSLText()-set the sensitivity label portion of the text string into
 * the text widget. Update the default level value or the current label
 * for clearance depending on the invocation mode.
 */

static void
SetSLText(string)
	char *string;
{
	/* Set the sensitivity level text string into widget */
	XtRemoveCallback(SLText[CalledFor], XmNmodifyVerifyCallback, TextModifyCB,NULL);
	XmTextSetString(SLText[CalledFor], string); 
	XtAddCallback(SLText[CalledFor], XmNmodifyVerifyCallback, TextModifyCB,NULL);
}


/*
 * GetSLText()-extract the sensitivity level component from the text
 * widget string and return its pointer.
 */

static char *
GetSLText()
{
	char * sl_text;
	char * c;

#ifdef OLD_CODE
	sl_text = XmTextGetString(SLText[CalledFor]);
	strcpy(label_buffer,sl_text);
	XtFree(sl_text);

	/* Remove any unwanted trailing characters */

	c = label_buffer;
	while(*c) {
		if(*c == '\n')
			*c = '\0';
		c++;
	}
	return(label_buffer);
#endif
	c = sl_text = XmTextGetString(SLText[CalledFor]);

	/* Remove any unwanted trailing characters */
	while(*c) {
		if(*c == '\n')
			*c = '\0';
		c++;
	}

	return(sl_text);
}


#if SEC_ILB
/*
 * GetLabelText()-return the text label from the text widget with an indicator
 * of whether the label is a wildcard value or not. The label type argument
 * indicates whether the caller wants both labels or just the IL or SL.
 *
 * This is from WmFIL.c. Changed so that WILDCARD is not allowed.
 */

static char *
GetLabelText(label, wild)
	int label;
	Boolean	*wild;
{
	char *text, *il_text;
	char *sl_text;
	char *c;

	c = text = XmTextGetString(SLText[CalledFor]);

	/* If combined label desired, return both portions */

	if(label == LABEL_BOTH) {
		text = (char *) Malloc(strlen(c) + 1);
		if(!text) {
			XtFree(c);
			return(NULL);
		}

		strcpy(text,c);
		XtFree(c);
		return(text);
	}

	/* Locate the start of the sensitivity label */

	while(*c != '[' && *c != '\n' && *c != '\0') 
		c++;

	/*
	 * Check for Wildcard label. This is a '*' character with no other
	 * characters in the label.
	 */

	if(label == LABEL_SL) {

		/* If SL character is not '[', there is no SL in this label */

		if(*c != '[') {
			XtFree(text);
			*wild = False;
			return(NULL);
		}

		/* Process sensitivity level */

		*wild = False;

		/* Allocate space for the SL label */

		sl_text = (char *) Malloc(strlen(c) + 1);
		if(!sl_text) {
			XtFree(text);
			return(NULL);
		}
		else {
			strcpy(sl_text,c);
			XtFree(text);
		}

		/*
		 * For SL processing, c points to the '[' character. Bump
		 * the search to start at the next character.
		 */

		for(c++; *c != '\0'; *c++) {
			if(*c == '*') {
				if(*wild) {
					*wild = False;
					break;
				}
				/* Not allowed */
				/*
				*wild = True;
				*/
				*wild = False;
			}
			if(*c != ']' && *c != '*') {
				*wild = False;
				break;
			}
		}
		return(sl_text);
	}
	else if(label == LABEL_IL) {

		*wild = False;

		/* Make sure there is an IL before the SL label */

		for(il_text = text; il_text < c; il_text++) {
			if(*il_text != ' ')
				break;
		}

		if(il_text == c) {	/* no IL label */
			XtFree(text);
			return(NULL);
		}

		/* Set the first character of SL to Null */

		*c = '\0';

		/* Allocate space for the label */

		il_text = (char *) Malloc(strlen(text) + 1);
		if(!il_text) {
			XtFree(text);
			return(NULL);
		}
		else {
			strcpy(il_text,text);
			XtFree(text);
		}

		/* Check for Wildcard IL label */

		*wild = False;
		for(c = il_text;*c != '\0'; *c++) {
			if(*c == '*') {
				if(*wild) {
					*wild = False;
					break;
				}
				/* Not allowed */
				/*
				*wild = True;
				*/
				*wild = False;
			}
			if(*c != ' ' && *c != '*') {
				*wild = False;
				break;
			}
		}
		return(il_text);
	}
}

/*
 * SetLabelText()-set the label text(s) into the text widget based on the
 * mode argument. This will display both, or only an IL or SL label.
 * This is from WmFIL.c
 */

static void
SetLabelText(string, label)
	char *string;
	int label;
{
	Arg args[2];
	int n;
	char *sl;

	/* Setup string based on label type */

	if(label != LABEL_BOTH) {

		/* Locate the start of the sensitivity label */

		sl = string;
		while(*sl && (*sl != '[') && (*sl != '\n'))
			sl++;

		if(*sl != '[')
			*sl = '\0';

		if(label == LABEL_IL)
			*sl = '\0';
		else
			string = sl;
	}

	XtRemoveCallback(SLText[CalledFor], XmNmodifyVerifyCallback, TextModifyCB,NULL);
	XmTextSetString(SLText[CalledFor], string); 
	XtAddCallback(SLText[CalledFor], XmNmodifyVerifyCallback, TextModifyCB,NULL);

	/* Disable typing mode and restore the button and callbacks */

	if(IsTyping) {
		IsTyping = False;

		/* Change Check button to Use new label button */
		SetButton2OK();

		/*
		n = 0;
		XtSetArg(args[n], XmNlabelString,
		    XmStringCreateLtoR(" Use \nNew Label",charset)); n++;
		XtSetValues(ButtonApply, args, n);
		*/

		/* Modify the callback associated with the button */

		/*
		XtRemoveCallback(ButtonApply, XmNactivateCallback, ButtonCB,
								BUTTON_CHECK);
		XtRemoveCallback(FileSB, XmNokCallback, ButtonCB, BUTTON_CHECK);
		XtAddCallback(ButtonApply, XmNactivateCallback, ButtonCB,
							BUTTON_APPLY_INTERNAL);
		XtAddCallback(FileSB, XmNokCallback, ButtonCB, BUTTON_APPLY);
		*/

	}
}
#endif /* SEC_ILB */



/*
 * SetDfltText()-set the sensitivity label portion of the text string into
 * the text widget. Update the default level value or the current label
 * for clearance depending on the invocation mode.
 * Pretty much of a NOP right now, but may be useful later.
 */

static void
SetDfltText(string)
	char *string;
{
	/* Set the default sensitivity level text string into widget */
	XmTextSetString(DfltText[CalledFor], string); 
}

/*
 * GetDfltText()-extract the sensitivity level component from the text
 * widget string and return its pointer.
 */

static char *
GetDfltText()
{
	char * dflt_text;
	char * c;

	dflt_text = XmTextGetString(DfltText[CalledFor]);
	strcpy(label_buffer,dflt_text);
	XtFree(dflt_text);

	/* Remove any unwanted trailing characters */

	c = label_buffer;
	while(*c) {
		if(*c == '\n')
			*c = '\0';
		c++;
	}

	return(label_buffer);
}

/* Converts the button from Check to OK */
static void 
SetButton2OK() 
{
	XmString	xmstring;
	Arg		args[MAX_ARGS];	/*  arg list		*/
	register int	n;		/*  arg count		*/

	/* Change the Check label to the Use label */

	xmstring = XmStringCreateLtoR(msg_sl[3], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString	,xmstring); n++;
	XtSetValues(ButtonOK[CalledFor], args, n);
	XmStringFree(xmstring);

	/* Reset the callbacks */
	XtRemoveCallback(ButtonOK[CalledFor], XmNactivateCallback, ButtonCB,
		BUTTON_CHECK);
	XtAddCallback(ButtonOK[CalledFor], XmNactivateCallback, ButtonCB,
		BUTTON_OK);
}

/* Converts the button from OK to CHECK */
static void 
SetButton2Check() 
{
	XmString	xmstring;
	Arg		args[MAX_ARGS];	/*  arg list		*/
	register int	n;		/*  arg count		*/

	/* Change the Check label to the Use label */

	xmstring = XmStringCreateLtoR(msg_sl[4], charset);
	if (! xmstring)
		MemoryError();
	n = 0;
	XtSetArg(args[n], XmNlabelString	,xmstring); n++;
	XtSetValues(ButtonOK[CalledFor], args, n);
	XmStringFree(xmstring);

	/* Reset the callbacks */
	XtRemoveCallback(ButtonOK[CalledFor], XmNactivateCallback, ButtonCB,
		BUTTON_OK);
	XtAddCallback(ButtonOK[CalledFor], XmNactivateCallback, ButtonCB,
		BUTTON_CHECK);
}

/*
 * ToggleCB()-this function is invoked as the callback routine when any
 * toggle button from a scrolled list widget is toggled. This routine
 * will terminate typing mode and set the new label in the widget
 * after parsing to account for the change and then set the buttons on
 * the screen accordingly.
 */

static void
ToggleCB(w, client_data, call_data) 
Widget		w;		/*  widget id		*/
caddr_t		client_data;	/*  data from application   */
caddr_t		call_data;	/*  data from widget class  */
{
	register int	n;
	Arg 		args[MAX_ARGS];
	XmToggleButtonCallbackStruct *ct = 
				(XmToggleButtonCallbackStruct *) call_data;
	XmString	TextString;
	char		word[BUFSIZ];	/* Should be big enough */
	register char *c;
	int		error;

	/* Lets find out the actual label */
	n = 0;
	XtSetArg(args[n], XmNlabelString,	&TextString); n++;
	XtGetValues(w, args, n);

	/* 
	 * If we are typing then we do not allow ANY changes to take
	 * effect therefore reset toggle value to what it was before.
	 */
	if(IsTyping) {
		ToggleButtonSetState(w, !ct->set);
		/* Don't know why but the next line causes a core dump
		 * so it is left commented out
		 *
		XBell(XtDisplay(w), 100);
		 */
		return;
	}

	/* Make sure the default button is off */

	if (CalledFor == SL_USER_CLEARANCE 		|| 
	    CalledFor == SL_DEVICES_MOD_MIN_SL 		||
	    CalledFor == SL_DEVICES_MOD_MAX_SL 		||
	    CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_SL	||
	    CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL	     )
		ToggleButtonSetState(DfltButton[CalledFor], False);

	/*
	 * Get the long form of the text of the button pressed 
	 */

	c = extract_normal_string(TextString);
	n = 1;
	while(*c != '\0' && *c != '(')
		word[n++] = *c++;
	word[n] = '\0';
	word[0] =(ct->set) ? '+' : '-';
	
	/*
	 * No further for -[CLASSIFICATION](doesn't make sense)
	 */
	if((client_data !=(caddr_t) CLASSIFICATION_TOGGLE) ||(ct->set) )
	{
		if (CalledFor == SL_USER_CLEARANCE)
			error = SetScreenFromClear(word);
		else if (CalledFor == SL_DEFAULT_CLEARANCE)
			error = SetScreenFromClear(word);
		else if (CalledFor == SL_SINGLE_USER_SL)
			error = SetScreenFromSL(word);
		else if (CalledFor == SL_DEVICES_DEF_MIN_SL)
			error = SetScreenFromSL(word);
		else if (CalledFor == SL_DEVICES_DEF_MAX_SL)
			error = SetScreenFromSL(word);
		else if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_SL)
			error = SetScreenFromSL(word);
#if SEC_ILB
		else if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_IL)
			error = SetScreenFromIL(word);
#endif
		else if (CalledFor == SL_DEVICES_MOD_MIN_SL)
			error = SetScreenFromSL(word);
		else if (CalledFor == SL_DEVICES_MOD_MAX_SL)
			error = SetScreenFromSL(word);
		else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_SL)
			error = SetScreenFromSL(word);
#if SEC_ILB
		else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL)
			error = SetScreenFromIL(word);
#endif
		else if (CalledFor == SL_AUDIT_COL_MIN_SL)
			error = SetScreenFromSL(word);
		else if (CalledFor == SL_AUDIT_COL_MAX_SL)
			error = SetScreenFromSL(word);
		else if (CalledFor == SL_AUDIT_SEL_SUB_MIN_SL)
			error = SetScreenFromSL(word);
		else if (CalledFor == SL_AUDIT_SEL_SUB_MAX_SL)
			error = SetScreenFromSL(word);
		else if (CalledFor == SL_AUDIT_SEL_OBJ_MIN_SL)
			error = SetScreenFromSL(word);
		else if (CalledFor == SL_AUDIT_SEL_OBJ_MAX_SL)
			error = SetScreenFromSL(word);
		else
			error = SetScreenFromSL(word);
	}
}

/*
 * SLError()-display an error message in system modal popup box
 */

static void
SLError(msgid)
int msgid;
{
	switch (msgid) {
		case SL_PARSE_ERROR:

			if (! msg_error_1)
				LoadMessage("msg_sensitivity_label_parse_error",
				&msg_error_1, &msg_error_1_text);
			ErrorMessageOpen(-1, msg_error_1, 0, NULL);
			break;

		default:
			printf ("Unknown error number in SL code\n");
			exit(1);
	}
}

/*
 * ButtonCB()-this is the main callback driver that handles the button
 * function callbacks for OK, Help, Check, and Cancel.
 */

static void
ButtonCB(w, client_data, call_data)
	Widget		w;		/*  widget id  */
	caddr_t		client_data;	/*  data from application   */
	caddr_t		call_data;	/*  data from widget class  */
{
	int		n;
	Arg		args[20];
	XmString	xmstring;
	int		error;
	char		*er, *ir, *new_er;
	char		*il, *sl;
	Boolean		iwild = False;
	Boolean		swild = False;
	Boolean		dflt_on;

	switch((int) client_data)
	{

	    case BUTTON_CHECK:		/* User hit the check key */

		if (CalledFor == SL_USER_CLEARANCE)
			CheckClear(w);
		else if (CalledFor == SL_DEFAULT_CLEARANCE)
			CheckClear(w);
		else if (CalledFor == SL_SINGLE_USER_SL)
			CheckSL(w);
		else if (CalledFor == SL_DEVICES_DEF_MIN_SL)
			CheckSL(w);
		else if (CalledFor == SL_DEVICES_DEF_MAX_SL)
			CheckSL(w);
		else if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_SL)
			CheckSL(w);
#if SEC_ILB
		else if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_IL)
			CheckLabel(w);
#endif
		else if (CalledFor == SL_DEVICES_MOD_MIN_SL)
			CheckSL(w);
		else if (CalledFor == SL_DEVICES_MOD_MAX_SL)
			CheckSL(w);
		else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_SL)
			CheckSL(w);
#if SEC_ILB
		else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL)
			CheckLabel(w);
#endif
		else if (CalledFor == SL_AUDIT_COL_MIN_SL)
			CheckSL(w);
		else if (CalledFor == SL_AUDIT_COL_MAX_SL)
			CheckSL(w);
		else if (CalledFor == SL_AUDIT_SEL_SUB_MIN_SL)
			CheckSL(w);
		else if (CalledFor == SL_AUDIT_SEL_SUB_MAX_SL)
			CheckSL(w);
		else if (CalledFor == SL_AUDIT_SEL_OBJ_MIN_SL)
			CheckSL(w);
		else if (CalledFor == SL_AUDIT_SEL_OBJ_MAX_SL)
			CheckSL(w);
		else
			CheckSL(w);
		break;

	    case BUTTON_CANCEL:		/* Exit without change */

		/* Force the first condition to fail */
		if (0 == 1)
			printf (" ***** Do not delete this line *****\n");
#if ! SEC_SHW
		else if (CalledFor == SL_USER_CLEARANCE)
			UserClearClose();
		else if (CalledFor == SL_DEFAULT_CLEARANCE)
			DefClearClose();
		else if (CalledFor == SL_SINGLE_USER_SL)
			SinUserClClose();
		else if (CalledFor == SL_DEVICES_DEF_MIN_SL)
			DevDefnslClose();
		else if (CalledFor == SL_DEVICES_DEF_MAX_SL)
			DevDefxslClose();
		else if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_SL)
			DevDefsslClose();
		else if (CalledFor == SL_DEVICES_MOD_MIN_SL)
			DevModnslClose();
		else if (CalledFor == SL_DEVICES_MOD_MAX_SL)
			DevModxslClose();
		else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_SL)
			DevModsslClose();
		else if (CalledFor == SL_AUDIT_COL_MIN_SL)
			AudColnslClose();
		else if (CalledFor == SL_AUDIT_COL_MAX_SL)
			AudColxslClose();
		else if (CalledFor == SL_AUDIT_SEL_SUB_MIN_SL)
			AudSelnssClose();
		else if (CalledFor == SL_AUDIT_SEL_SUB_MAX_SL)
			AudSelxssClose();
		else if (CalledFor == SL_AUDIT_SEL_OBJ_MIN_SL)
			AudSelnosClose();
		else if (CalledFor == SL_AUDIT_SEL_OBJ_MAX_SL)
			AudSelxosClose();
#endif /* ! SEC_SHW */
#if SEC_ILB
		else if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_IL)
			DevDefsilClose();
		else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL)
			DevModsilClose();
#endif /* SEC_ILB */
		break;

	    case BUTTON_OK:	/* Make the label change */

		/* Get the sensitivity label portion of string */

		/* Even if default button is on the SLText widget contains
		 * the current value */

		/* Force the first condition to fail */
		if (0 == 1)
			printf (" ***** Do not delete this line *****\n");
#if ! SEC_SHW
		else if (CalledFor == SL_USER_CLEARANCE) {
			dflt_on = XmToggleButtonGadgetGetState(
					DfltButton[CalledFor]);
			sl = GetSLText();
			UserClearOK (w, client_data, call_data, sl, !dflt_on);
		}
		else if (CalledFor == SL_DEFAULT_CLEARANCE) {
			sl = GetSLText();
			DefClearOK (w, client_data, call_data, sl);
		}
		else if (CalledFor == SL_SINGLE_USER_SL) {
			sl = GetSLText();
			SinUserClOK (w, client_data, call_data, sl);
		}
		else if (CalledFor == SL_DEVICES_DEF_MIN_SL) {
			sl = GetSLText();
			DevDefnslOK (w, client_data, call_data, sl);
		}
		else if (CalledFor == SL_DEVICES_DEF_MAX_SL) {
			sl = GetSLText();
			DevDefxslOK (w, client_data, call_data, sl);
		}
		else if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_SL) {
			sl = GetSLText();
			DevDefsslOK (w, client_data, call_data, sl);
		}
		else if (CalledFor == SL_DEVICES_MOD_MIN_SL) {
			dflt_on = XmToggleButtonGadgetGetState(
					DfltButton[CalledFor]);
			sl = GetSLText();
			DevModnslOK (w, client_data, call_data, sl, !dflt_on);
		}
		else if (CalledFor == SL_DEVICES_MOD_MAX_SL) {
			dflt_on = XmToggleButtonGadgetGetState(
					DfltButton[CalledFor]);
			sl = GetSLText();
			DevModxslOK (w, client_data, call_data, sl, !dflt_on);
		}
		else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_SL) {
			dflt_on = XmToggleButtonGadgetGetState(
					DfltButton[CalledFor]);
			sl = GetSLText();
			DevModsslOK (w, client_data, call_data, sl, !dflt_on);
		}
		else if (CalledFor == SL_AUDIT_COL_MIN_SL) {
			sl = GetSLText();
			AudColnslOK (w, client_data, call_data, sl, 
			XmToggleButtonGadgetGetState(ThisToggle[CalledFor]),
			XmToggleButtonGadgetGetState(FutureToggle[CalledFor]) );
		}
		else if (CalledFor == SL_AUDIT_COL_MAX_SL) {
			sl = GetSLText();
			AudColxslOK (w, client_data, call_data, sl, 
			XmToggleButtonGadgetGetState(ThisToggle[CalledFor]),
			XmToggleButtonGadgetGetState(FutureToggle[CalledFor]) );
		}
		else if (CalledFor == SL_AUDIT_SEL_SUB_MIN_SL) {
			sl = GetSLText();
			AudSelnssOK (w, client_data, call_data, sl);
		}
		else if (CalledFor == SL_AUDIT_SEL_SUB_MAX_SL) {
			sl = GetSLText();
			AudSelxssOK (w, client_data, call_data, sl);
		}
		else if (CalledFor == SL_AUDIT_SEL_OBJ_MIN_SL) {
			sl = GetSLText();
			AudSelnosOK (w, client_data, call_data, sl);
		}
		else if (CalledFor == SL_AUDIT_SEL_OBJ_MAX_SL) {
			sl = GetSLText();
			AudSelxosOK (w, client_data, call_data, sl);
		}
#endif /* ! SEC_SHW */
#if SEC_ILB
		else if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_IL) {
			switch (file_set_mode) {
				case LABEL_BOTH:
					il = GetLabelText(LABEL_IL, &iwild);
					sl = GetLabelText(LABEL_SL, &swild);
					break;

				case LABEL_IL:
					il = GetLabelText(LABEL_IL, &iwild);
					sl = (char *) Malloc(1);
					if (! sl)
						MemoryError();
					break;

				case LABEL_SL:
					il = (char *) Malloc(1);
					if (! il)
						MemoryError();
					sl = GetLabelText(LABEL_SL, &swild);
					break;
			}
			DevDefsilOK (w, client_data, call_data, file_set_mode, 
				il, sl);
			if (il)
				free(il);
		}
		else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL) {
			dflt_on = XmToggleButtonGadgetGetState(
					DfltButton[CalledFor]);
			switch (file_set_mode) {
				case LABEL_BOTH:
					il = GetLabelText(LABEL_IL, &iwild);
					sl = GetLabelText(LABEL_SL, &swild);
					break;

				case LABEL_IL:
					il = GetLabelText(LABEL_IL, &iwild);
					sl = (char *) Malloc(1);
					if (! sl)
						MemoryError();
					break;

				case LABEL_SL:
					il = (char *) Malloc(1);
					if (! il)
						MemoryError();
					sl = GetLabelText(LABEL_SL, &swild);
					break;
			}
			DevModsilOK (w, client_data, call_data, file_set_mode,
				il, sl, !dflt_on);
			/* NN1
			if (il)
				free(il);
				*/
		}
#endif /* SEC_ILB */
		/* NN1
		if (sl)
			free(sl);
		*/

		break;

		/* Drop through to handle default setting */

	    case BUTTON_DEFAULT:

		/* If turning button on then turn typing off and reset the
		 * screen. If turning button off then nothing to do */
		if (CalledFor == SL_USER_CLEARANCE 		|| 
		    CalledFor == SL_DEVICES_MOD_MIN_SL 		||
		    CalledFor == SL_DEVICES_MOD_MAX_SL 		||
		    CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_SL	||
		    CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL	     ) {
			XmToggleButtonCallbackStruct *ct = 
				(XmToggleButtonCallbackStruct *) call_data;

			if (ct->set) {

			/* If we were typing then we need to set button to OK */
				if (IsTyping) {
					SetButton2OK();
				}

				IsTyping = False;
				if (CalledFor == SL_USER_CLEARANCE)
					SetScreenFromClear(GetDfltText());
		    		else if 
				   (CalledFor == SL_DEVICES_MOD_MIN_SL ||
		    		    CalledFor == SL_DEVICES_MOD_MAX_SL ||
			 	    CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_SL)
					 SetScreenFromSL(GetDfltText());
#if SEC_ILB
				else if 
		    		   (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL)
					 SetScreenFromIL(GetDfltText());
#endif /* SEC_ILB */
			}
		}

		break;


	    case BUTTON_HELP:

		if (CalledFor == SL_USER_CLEARANCE)
			HelpDisplayOpen (w, "accounts,UserClear", call_data);
		else if (CalledFor == SL_DEFAULT_CLEARANCE)
			HelpDisplayOpen (w, "accounts,DefClear", call_data);
		else if (CalledFor == SL_SINGLE_USER_SL)
			HelpDisplayOpen (w, "accounts,SinUserCl", call_data);
		else if (CalledFor == SL_DEVICES_DEF_MIN_SL)
			HelpDisplayOpen (w, "devices,DevDefnsl", call_data);
		else if (CalledFor == SL_DEVICES_DEF_MAX_SL)
			HelpDisplayOpen (w, "devices,DevDefxsl", call_data);
		else if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_SL)
			HelpDisplayOpen (w, "devices,DevDefssl", call_data);
		else if (CalledFor == SL_DEVICES_DEF_SINGLE_LEVEL_IL)
			HelpDisplayOpen (w, "devices,DevDefsil", call_data);
		else if (CalledFor == SL_DEVICES_MOD_MIN_SL)
			HelpDisplayOpen (w, "devices,DevModnsl", call_data);
		else if (CalledFor == SL_DEVICES_MOD_MAX_SL)
			HelpDisplayOpen (w, "devices,DevModxsl", call_data);
		else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_SL)
			HelpDisplayOpen (w, "devices,DevModssl", call_data);
		else if (CalledFor == SL_DEVICES_MOD_SINGLE_LEVEL_IL)
			HelpDisplayOpen (w, "devices,DevModsil", call_data);
		else if (CalledFor == SL_AUDIT_COL_MIN_SL)
			HelpDisplayOpen (w, "audit,AudColnsl", call_data);
		else if (CalledFor == SL_AUDIT_COL_MAX_SL)
			HelpDisplayOpen (w, "audit,AudColxsl", call_data);
		else if (CalledFor == SL_AUDIT_SEL_SUB_MIN_SL)
			HelpDisplayOpen (w, "audit,AudSelnss", call_data);
		else if (CalledFor == SL_AUDIT_SEL_SUB_MAX_SL)
			HelpDisplayOpen (w, "audit,AudSelxss", call_data);
		else if (CalledFor == SL_AUDIT_SEL_OBJ_MIN_SL)
			HelpDisplayOpen (w, "audit,AudSelnos", call_data);
		else if (CalledFor == SL_AUDIT_SEL_OBJ_MAX_SL)
			HelpDisplayOpen (w, "audit,AudSelxos", call_data);
		break;

	}
}


#if SEC_CMW

/*
 * ModeCB()-callback that is invoked when the user toggles the label build
 * buttons. This allows the label build mode to be switched between IL, SL,
 * or both labels. This also effect the scrolled lists displayed as well
 * as the button choices.
 */

static void
ModeCB(w, client_data, call_data) 
Widget		w;		/*  widget id		*/
long		client_data;	/*  data from application   */
long		call_data;	/*  data from widget class  */
{
	XmToggleButtonCallbackStruct *ct = 
				(XmToggleButtonCallbackStruct *) call_data;

	/* Process the callback type */

	switch(client_data) {

	    case LABEL_IL:

		if(ct->set)
			file_set_mode = LABEL_IL;

		/* De-Sensitize the IL=SL button */

		/*
		XtSetSensitive(ButtonSet, False);
		*/

		/* Set the IL to system low, SL to maximum */

		sclass = maximum_class;
		COMPARTMENTS_COPY(scomps, maximum_comps);

		iclass = *l_min_classification;
		COMPARTMENTS_COPY(icomps, l_in_compartments[iclass]);
		MARKINGS_COPY(imarks, l_in_markings[iclass]);

		/* Force toggle off/on to generate build mode callback */

		XmToggleButtonGadgetSetState(ILLabelToggle[CalledFor],False,False);
		XmToggleButtonGadgetSetState(ILLabelToggle[CalledFor],True,True);

		/* De-sensitize the SL build mode toggle */

		XtSetSensitive(ILLabelToggle[CalledFor], True);
		XtSetSensitive(SLLabelToggle[CalledFor], False);
		break;

	    case LABEL_SL:

		if(ct->set)
			file_set_mode = LABEL_SL;

		/* De-Sensitize the IL=SL button */

		/*
		XtSetSensitive(ButtonSet, False);
		*/

		/* Set the IL to system low, SL to maximum */

		sclass = maximum_class;
		COMPARTMENTS_COPY(scomps, maximum_comps);

		iclass = *l_min_classification;
		COMPARTMENTS_COPY(icomps, l_in_compartments[iclass]);
		MARKINGS_COPY(imarks, l_in_markings[iclass]);

		/* Force IL mode toggle off/on to generate build callback */

		XmToggleButtonGadgetSetState(SLLabelToggle[CalledFor],False,False);
		XmToggleButtonGadgetSetState(SLLabelToggle[CalledFor],True,True);

		/* De-sensitize the IL build mode toggle */

		XtSetSensitive(SLLabelToggle[CalledFor], True);
		XtSetSensitive(ILLabelToggle[CalledFor], False);
		break;

	    case LABEL_BOTH:

		if(ct->set && (file_set_mode == LABEL_BOTH))
			break;
		else
			file_set_mode = LABEL_BOTH;

		/* Sensitize the IL=SL button */

		/*
		XtSetSensitive(ButtonSet, True);
		*/

		if(build_mode == BUILD_IL) {

			/* Re-manage the label and markings scrolled window */

			XtManageChild(MarkingsLabel[CalledFor]);
			XtManageChild(MarkingsW[CalledFor]);
		}

		/* Set the IL to system low, SL to maximum */

		sclass = maximum_class;
		iclass = *l_min_classification;

		COMPARTMENTS_COPY(scomps, maximum_comps);
		COMPARTMENTS_COPY(icomps, l_in_compartments[iclass]);
		MARKINGS_COPY(imarks, l_in_markings[iclass]);

		/* Raise the Wildcard toggle button and reset flags */

		/*
		XmToggleButtonGadgetSetState(WildcardToggle[CalledFor], False, False);
		*/

		/*
		ilevelwildcard = False;
		slevelwildcard = False;
		*/

		/* Update text string to reflect both labels */

		b_convert(banner_buffer,
			iclass, icomps, imarks, False, iparse_table,
			sclass, scomps, False, sparse_table,
			length_words);

		SetLabelText(banner_buffer, LABEL_BOTH);

		/* Sensitize the build mode choices */

		XtSetSensitive(ILLabelToggle[CalledFor], True);
		XtSetSensitive(SLLabelToggle[CalledFor], True);

		/* Set the toggles to reflect new label */

		if(build_mode == BUILD_IL)
			SetToggleButtons(l_information_label_tables,
				 iparse_table[CalledFor],
				 iclass, icomps, imarks,
				 sclass, scomps,
				 maximum_class, maximum_comps,
				 *l_min_classification,
				 l_in_compartments[*l_min_classification],
				 ClassificationButtons[CalledFor],
				 CompartmentButtons[CalledFor]);

		else	/* Build mode is BUILD_SL */

			SetToggleButtons(l_sensitivity_label_tables,
				 sparse_table[CalledFor],
				 sclass, scomps, smarks,
				 maximum_class, maximum_comps,
				 maximum_class, maximum_comps,
				 iclass, icomps,	/* IL is minimum */
				 ClassificationButtons[CalledFor],
				 CompartmentButtons[CalledFor]);

		break;

	}
}

/*
 * BuildCB()-this routine is called when the user changes the label display
 * mode from SL to IL. This controls the label that is displayed and which
 * label the buttons on the screen are set from.
 */

static void
BuildCB(w, client_data, call_data) 
Widget		w;		/*  widget id		*/
long		client_data;	/*  data from application   */
long		call_data;	/*  data from widget class  */
{
	XmToggleButtonCallbackStruct *ct = 
				(XmToggleButtonCallbackStruct *) call_data;

	/* Set the label build mode based on callback set */

	switch(client_data) {

	    case BUILD_IL:

		if(ct->set)
			build_mode = BUILD_IL;
		else
			return;

		/* Re-manage the label and markings scrolled window */

		XtManageChild(MarkingsLabel[CalledFor]);
		XtManageChild(MarkingsW[CalledFor]);

		/*
		if(ilevelwildcard) {
			XtSetSensitive(CompartmentRC, False);
			XtSetSensitive(MarkingRC, False);
			XmToggleButtonGadgetSetState(WildcardToggle, True,
									False);
		}
		else {
		*/
			XtSetSensitive(CompartmentRC[CalledFor], True);
			XtSetSensitive(MarkingRC[CalledFor], True);
			/*
			XmToggleButtonGadgetSetState(WildcardToggle, False,
									False);
									*/
		/*
		}
		*/

		/* Convert the current settings */

		b_convert(banner_buffer,
			iclass, icomps, imarks, ilevelwildcard, iparse_table,
			sclass, scomps, slevelwildcard, sparse_table,
			length_words);

		if(file_set_mode == LABEL_BOTH)
			SetLabelText(banner_buffer,LABEL_BOTH);
		else if(file_set_mode == LABEL_IL)
			SetLabelText(banner_buffer,LABEL_IL);

		SetToggleButtons(l_information_label_tables, 
			 iparse_table[CalledFor],
			 iclass, icomps, imarks,
			 sclass, scomps,
			 maximum_class, maximum_comps,
			 *l_min_classification,
			 l_in_compartments[*l_min_classification],
			 ClassificationButtons[CalledFor],
			 CompartmentButtons[CalledFor]);

		if(ilevelwildcard)
		    RaiseClassificationButtons(ClassificationButtons[CalledFor],
			NumberOfClassifications[CalledFor]);

		break;

	    case BUILD_SL:

		if(ct->set)
			build_mode = BUILD_SL;
		else
			return;

		/* Unmanage the label and markings widgets for SL mode */

		XtUnmanageChild(MarkingsLabel[CalledFor]);
		XtUnmanageChild(MarkingsW[CalledFor]);

		/* Force compartments insensitive in Wildcard mode */

		/*
		if(slevelwildcard) {
			XtSetSensitive(CompartmentRC, False);
			XmToggleButtonGadgetSetState(WildcardToggle, True,
									False);
		}
		else {
			XtSetSensitive(CompartmentRC, True);
			XmToggleButtonGadgetSetState(WildcardToggle, False,
									False);
		}
		*/

		/* Convert the current settings */

		b_convert(banner_buffer,
			iclass, icomps, imarks, ilevelwildcard, iparse_table,
			sclass, scomps, slevelwildcard, sparse_table,
			length_words);

		if(file_set_mode == LABEL_BOTH)
			SetLabelText(banner_buffer,LABEL_BOTH);
		else if(file_set_mode == LABEL_SL)
			SetLabelText(banner_buffer,LABEL_SL);

		/* Set the toggle buttons */

		SetToggleButtons(l_sensitivity_label_tables, 
			 sparse_table[CalledFor],
			 sclass, scomps, smarks,
			 maximum_class, maximum_comps,
			 maximum_class, maximum_comps,
			 iclass, icomps,	/* IL is minimum */
			 ClassificationButtons[CalledFor],
			 CompartmentButtons[CalledFor]);

		if(slevelwildcard)
		    RaiseClassificationButtons(ClassificationButtons[CalledFor],
			NumberOfClassifications[CalledFor]);

		break;

	}
}

/*
 * b_convert()-convert both labels using the internal representations unless
 * one or both is a wildcard value. Handle wildcards as special cases which
 * l_b_convert() routine can not do.
 */

static void
b_convert(string, iclass, icomps_ptr, imarks_ptr, ilabel_wild, iparse_table,
		  sclass, scomps_ptr, slabel_wild, sparse_table, use_short_names)
char * string;			/* the place to return the output string */
CLASSIFICATION iclass;		/* the information label classification */
COMPARTMENTS *icomps_ptr;	/* the information label compartments */
MARKINGS *imarks_ptr;		/* the information label markings */
Boolean  ilabel_wild;		/* True if IL is a wildcard */
char *iparse_table;		/* the information label parse table */
CLASSIFICATION sclass;		/* the sensitivity label classification */
COMPARTMENTS *scomps_ptr;	/* the sensitivity label compartments */
Boolean  slabel_wild;		/* True if SL is a wildcard */
char *sparse_table;		/* the sensitivity label parse table */
int use_short_names;		/* flag sez use short names for output */
{
	register char *sp;		/* fast ptr into output string */
	register char *dummy = NULL;
	
	if(!l_encodings_initialized())
		return;		/* error return if encodings not in place */

	/* Allocate dummy for wildcard handling on parse table return */

	if((dummy = (char *) Malloc(max_label_size)) == NULL)
		return;

	sp = string;

	if(ilabel_wild) {
		strcpy(sp, "*");
		sp += strlen("*");

		/* Update the parse table but ignore string return */

		l_convert(dummy, iclass, l_long_classification, 
				icomps_ptr, imarks_ptr,
				l_information_label_tables, iparse_table, 
				use_short_names, ALL_ENTRIES);
	
	} else {
		l_convert(sp, iclass, l_long_classification, 
				icomps_ptr, imarks_ptr,
				l_information_label_tables, iparse_table, 
				use_short_names, ALL_ENTRIES);
	}
	
	sp += strlen(sp);	/* sp now ptr after IL in string */
	
	strcpy(sp, " [");	/* add starting SL delimiter */
	
	sp += 2;		/* sp now ptr to place for sensitivity label */

	if(slabel_wild) {
		strcpy(sp, "*");
		sp += strlen("*");

		/* Update parse table but ignore string return */

		l_convert(dummy, sclass, l_short_classification, 
				scomps_ptr, l_t_markings,
				l_sensitivity_label_tables, sparse_table, 
				use_short_names, ALL_ENTRIES);
	} else {
		l_convert(sp, sclass, l_short_classification, 
				scomps_ptr, l_t_markings,
				l_sensitivity_label_tables, sparse_table, 
				use_short_names, ALL_ENTRIES);
	}
	
	strcat(sp, "]");		/* add closing delimiter */

	/* Free the dummy string for wildcard use */

	if(dummy)
		free(dummy);
}
#endif /* SEC_CMW */


/*
 * The internal subroutine word_to_string puts the indicated word from the
 * passed word table into the passed string, putting the short
 * version of the name in parenthesis after the long version, if the short
 * version is different than the long version.  word_to_string is used to
 * produce each button in the ChangeLabelDialog on which a user can click to
 * change the label.
 */

static void
word_to_string (s, l_tables, i)
register char *s;
register struct l_tables *l_tables;
register int i;
{
	int do_short = FALSE;
	char *short_prefix;
	char *short_word;
	char *short_suffix;
	
#ifdef DEBUG
printf ("New version of Word to string\n");
printf ("Word to string\n");
printf ("i = %d\n", i);
printf ("table prefix %d\n", (int) l_tables->l_words[i].l_w_prefix );
#endif
/*
 * First output the prefix if any, setting do_short if there is a short prefix,
 * and setting short_prefix to the short prefix if present, else the output one.
 */
 
	if (l_tables->l_words[i].l_w_prefix >= 0)
	{
		sprintf (s, "%s ", l_tables->l_words[l_tables->l_words[i].l_w_prefix].l_w_output_name);

#if SEC_ENCODINGS
		if (l_tables->l_words[l_tables->l_words[i].l_w_prefix].l_w_short_name)
		{
			do_short = TRUE;
			short_prefix = l_tables->l_words[l_tables->l_words[i].l_w_prefix].l_w_short_name;
		}
		else short_prefix = l_tables->l_words[l_tables->l_words[i].l_w_prefix].l_w_output_name;
#endif /* SEC_ENCODINGS */
	}
	else *s = '\0';		/* make s null unless a prefix was put there */

/*
 * Next output the word itself, setting do_short if there is a short word,
 */ 
	strcat (s, l_tables->l_words[i].l_w_output_name);

#if SEC_ENCODINGS
	if (l_tables->l_words[i].l_w_short_name)
	{
		do_short = TRUE;
		short_word = l_tables->l_words[i].l_w_short_name;
	}
	else short_word = l_tables->l_words[i].l_w_output_name;

#endif /* SEC_ENCODINGS */
	
/*
 * Next output the suffix if any, setting do_short if there is a short suffix,
 * and setting short_suffix to the short suffix if present, else the output one.
 */
 
	if (l_tables->l_words[i].l_w_suffix >= 0)
	{
		sprintf (&s[strlen(s)], " %s",
			l_tables->l_words[l_tables->l_words[i].l_w_suffix].l_w_output_name);

#if SEC_ENCODINGS
		if (l_tables->l_words[l_tables->l_words[i].l_w_suffix].l_w_short_name)
		{
			do_short = TRUE;
			short_suffix = l_tables->l_words[l_tables->l_words[i].l_w_suffix].l_w_short_name;
		}
		else short_suffix = l_tables->l_words[l_tables->l_words[i].l_w_suffix].l_w_output_name;
#endif /* SEC_ENCODINGS */
	}

/*
 * Now, if there are short versions of the word, its prefix, or its suffix,
 * put the shortened version of this word in parentheses after the long
 * version (put in string above).
 */

#if SEC_ENCODINGS
	if (do_short)
	{
		if (l_tables->l_words[i].l_w_prefix >= 0)
			sprintf (&s[strlen(s)], " (%s %s", short_prefix, short_word);
		else
			sprintf (&s[strlen(s)], " (%s", short_word);
			
		if (l_tables->l_words[i].l_w_suffix >= 0)
			sprintf (&s[strlen(s)], " %s)", short_suffix);
		else
			strcat (s, ")");
	}
#endif /* SEC_ENCODINGS */
}

/*
 * The subroutine WordInDialog determines whether a given word should be displayed
 * in the ChangeLabelDialog box.  The word should not be in, and therefore WordInDialog
 * returns FALSE iff:
 *
 *		1)  The word is not visible given the passed max class and comps;
 *
 *		2)	The word, if turned on, would force the label below the passed
 *			min_comps;
 *
 *		3)	The word is part of an invalid combination, and the OTHER word in the
 *			combination is an inverse compartment that is forced on because of the
 *			max class and comps.
 *
 *		4)  The word requires another word that falls into one of the above two
 *			categories (determined recursively).
 */

static int
WordInDialog (l_tables, index, min_comps_ptr, max_class, max_comps_ptr, 
		parse_table)
struct l_tables *l_tables;		/* the l_tables in question */
int index;				/* index of word in l_words table */
COMPARTMENTS *min_comps_ptr;		/* the minimum valid compartments */
CLASSIFICATION max_class;		/* the maximum classification */
COMPARTMENTS *max_comps_ptr;		/* the maximum valid compartments */
char *parse_table;			/* the parse table */
{
	register struct l_word_pair *wp;
	register short *lp;	
	register int i;
	struct l_constraints *cp;
	short *invalid_list;
	short *end_invalid_list;
	short dont_output;
	
/*
 * If word is not visible given the max class and compartments, return FALSE.
 */
 
#define WORD_VISIBLE(i,cl,cm) ((cl)>=l_tables->l_words[i].l_w_min_class \
      && COMPARTMENTS_DOMINATE (cm, l_tables->l_words[i].l_w_cm_value))

#ifdef DEBUG
printf ("WORD IN DIALOG %d %d %d\n", index,index , (int) max_class);
printf ("State %d\n", (int) max_class);
printf ("State %d\n", (int) l_tables->l_words[index].l_w_min_class );
printf ("Dom %x\n", *max_comps_ptr);
printf ("Dom %d\n", (int) COMPARTMENTS_DOMINATE 
	(max_comps_ptr, l_tables->l_words[index].l_w_cm_value));
#endif

	if (!WORD_VISIBLE (index, max_class, max_comps_ptr))	/* if word is not visible */
		return (FALSE);

/*
 * If word would force label below minimum compartments, return FALSE.
 */
 
	if (COMPARTMENT_MASK_ANY_BITS_MATCH (l_tables->l_words[index].l_w_cm_mask,
										 min_comps_ptr)
	 && COMPARTMENTS_EQUAL (l_tables->l_words[index].l_w_cm_value, l_0_compartments))
		return (FALSE);

/*
 * Next check the list of constraints to see whether any constraint prevents this
 * word from being put in the dialog.  Scan the list of constraints, processing each
 * entry depending on its type: NOT_WITH or ONLY_WITH.
 */
 
	for (cp = l_tables->l_constraints; cp->l_c_type != L_END; ++cp)
	{

/*
 * If this is an ONLY_WITH constraint, then this word should NOT be put in the
 * dialog if some word that this one cannot be combined with (i.e. does NOT
 * appear in the second list) is an inverse compartment forced on because of the
 * max class and comps.
 */

#define L_OK 4

		if (cp->l_c_type==ONLY_WITH)
		{
			for (lp = cp->l_c_first_list; lp != cp->l_c_second_list; lp++)
			 if (index==*lp)
			{
				for (lp = cp->l_c_second_list; lp < cp->l_c_end_second_list; lp++)
					parse_table[*lp] |= L_OK;	/* mark word OK to be on */
				dont_output = FALSE;
				for (i=l_tables->l_first_main_entry; i<l_tables->l_num_entries; i++)
				{
					if (parse_table[i]&L_OK)
						parse_table[i] &= ~L_OK;						
					else
					{
						if (!COMPARTMENT_MASK_EQUAL (l_tables->l_words[i].l_w_cm_mask,
													 l_0_compartments)	/* if this is a compartment word */
						 && COMPARTMENTS_EQUAL (l_tables->l_words[i].l_w_cm_value,
						 						l_0_compartments)	/* for an inverse compartment */
						 && COMPARTMENTS_IN (max_comps_ptr,
						 					 l_tables->l_words[i].l_w_cm_mask,
								 			 l_tables->l_words[i].l_w_cm_value))
								 			 	/* and indicated by the max */
							dont_output = TRUE;
					}
				}
				if (dont_output) return (FALSE);
				break;
			}
		}

/*
 * Else if this is a NOT_WITH constraint, then check whether this word is part
 * of an invalid combination, and any OTHER word in the combination is an
 * inverse compartmented forced on because of the max class and comps.  If so,
 * then don't display this word.
 */

		else		/* must be a NOT_WITH constraint */
		{
			invalid_list = 0;
			for (lp = cp->l_c_first_list; lp != cp->l_c_second_list; lp++)
			 if (index==*lp)
			{
				invalid_list = cp->l_c_second_list;
				end_invalid_list = cp->l_c_end_second_list;
				break;
			}
			
			if (!invalid_list) for (lp = cp->l_c_second_list;
			 lp != cp->l_c_end_second_list;
			 lp++) if (index==*lp)
			{
				invalid_list = cp->l_c_first_list;
				end_invalid_list = cp->l_c_second_list;
				break;
			}
				
			if (invalid_list) for (lp=invalid_list; lp!=end_invalid_list; lp++)
			{
				if (!COMPARTMENT_MASK_EQUAL (l_tables->l_words[*lp].l_w_cm_mask,
											 l_0_compartments)	/* if this is a compartment word */
				 && COMPARTMENTS_EQUAL (l_tables->l_words[*lp].l_w_cm_value,
				 						l_0_compartments)	/* for an inverse compartment */
				 && COMPARTMENTS_IN (max_comps_ptr,
				 					 l_tables->l_words[*lp].l_w_cm_mask,
						 			 l_tables->l_words[*lp].l_w_cm_value))
						 			 	/* and indicated by the max */
					return (FALSE);
			}
		}
	}
	
/*
 * Now, if this requires another word, check (recursively) whether THAT word can
 * be in the dialog.
 */
 
	for (wp=l_tables->l_required_combinations; wp->l_word1 != L_END; wp++)
		if (index==wp->l_word1)	/* if this word requires another... */
			return (WordInDialog (l_tables, wp->l_word2, min_comps_ptr, max_class,
				    max_comps_ptr, parse_table));
			
	return (TRUE);	/* if all of the above tests passed */
}


static Widget
AddToggle(Parent, string, client_data)
	Widget	Parent;
	char	*string;
	caddr_t	client_data;
{
	Arg		args[MAX_ARGS];	/*  arg list		*/
	register int	n;		/*  arg count		*/
	Widget		w;

#ifdef DEBUG
printf ("Add Toggle %s\n", string);
#endif /* DEBUG */
	n = 0;
	XtSetArg(args[n], XmNlabelString,
			XmStringLtoRCreate(string,charset)); n++;
	w = XmCreateToggleButtonGadget(Parent,"Toggle",args,n);
	/* 
	 * We are only interested in the valueChanged call back because
	 * The user can Arm (or Disarm) the toggle by holding the mouse
	 * button down. If he then moves the mouse away from the toggle
	 * and releases it the toggle is not changed. We receive both
	 * an ARM and DISARM callback but not the Value Changed
	 */
	XtAddCallback(w, XmNvalueChangedCallback,  ToggleCB, client_data);
	XtManageChild(w);
#ifdef DEBUG
printf ("Add Toggle return\n");
#endif /* DEBUG */
	return (w);
}

/* Set the button sensitivity. Check to make sure this is a change */

static void
ToggleButtonSetSensitivity(w, state)
	Widget	w;
	Boolean	state;
{
	Arg		args[MAX_ARGS];	/*  arg list		*/
	register int	n;		/*  arg count		*/

	if (XtIsSensitive(w) == state) return;
	n = 0;
	XtSetArg(args[n], XmNsensitive, state); n++;
	XtSetValues(w,args,n);
	return;
}

/* Set the button state. Check to make sure this is a change */

static void
ToggleButtonSetState(w, state)
	Widget	w;
	Boolean	state;
{
	if (XmToggleButtonGadgetGetState(w) == state) return;
	XmToggleButtonGadgetSetState(w, state, False);
}


/*
 * RaiseClassificationButtons()-raise all classification buttons to show
 * them as valid choices. Used for Wildcard representation.
 */

static void
RaiseClassificationButtons(class_buttons, num_class)
	Widget *class_buttons;
	int	num_class;
{
	int i;

	for(i=0; i < num_class; i++)
		ToggleButtonSetState(class_buttons[i], False);
}
	
/*
 * The subroutine SetToggleNames initializes the work area with the
 * proper names of classifications and words, as taken from the
 * label tables, including only those words visible given the passed maximum
 * classification and compartments, and those words that can be selected given
 * the passed minimum classification and compartments.
 */
 
#define FindFirstCompartment 1
#define FoundFirstCompartment 2
#define FoundFirstMarking 3

static void
SetToggleNames (l_tables, min_class, min_comps_ptr, max_class,
			max_comps_ptr, parse_table,
		 	class_buttons, comp_buttons,
			class_num, comp_num,
			class_rc, comp_rc, marks_rc)
register struct l_tables *l_tables;
CLASSIFICATION min_class;
COMPARTMENTS *min_comps_ptr;
CLASSIFICATION max_class;
COMPARTMENTS *max_comps_ptr;
char *parse_table;
Widget	*class_buttons;
Widget	*comp_buttons;
int	*class_num;
int	*comp_num;
Widget	class_rc;
Widget	comp_rc;
Widget	marks_rc;
{
	register int i;
	register struct l_word_pair *wp;
	char buffer[256];
	int mode;
	XmString xmstring;


	/* Debug start */
#ifdef DEBUG
	printf ("Set Toggle Names Leave debug start\n");
	printf ("min_class %d\n", min_class);
	printf ("min_comps_ptr %u %x\n", *min_comps_ptr, *min_comps_ptr);
	printf ("max_class %d\n", max_class);
	printf ("max_comps_ptr %u %x\n", *max_comps_ptr, *max_comps_ptr);
	printf ("num entries %d\n", l_tables->l_num_entries);
	printf ("i par name\n");
	for (i=l_tables->l_first_main_entry; i< l_tables->l_num_entries; i++) {
		printf ("%d %d %s\n", i, parse_table[i], 
			l_tables->l_words[i].l_w_output_name);
	}
#endif
	/* Debug end */

/*
 * Now, loop through the classifications, filling their long and 
 * short names into the dialog box.  Only classifications visible 
 * at the max class and comps are shown.
 */

	for (i=0; i<= *l_max_classification; i++)
	{
 		if (CLASSIFICATION_VISIBLE (i, max_class)
 		 && i >= min_class)
		{
#if SEC_ENCODINGS
			sprintf (buffer, "%s (%s)",
					l_long_classification[i],
					 l_short_classification[i]);
#else
			sprintf (buffer, "%s",
					l_long_classification[i]);
#endif /* SEC_ENCODINGS */
			class_buttons [*class_num] = 
				AddToggle(class_rc, buffer,
					CLASSIFICATION_TOGGLE);
			*class_num = *class_num + 1;
		}
	}

/*
 * Now, loop through the words, filling their long and short names into the
 * dialog box.  Only words visible at the max class and comps are shown.
 */

	/* Missing initialization */
	mode = FindFirstCompartment;

	for (i=l_tables->l_first_main_entry; i< l_tables->l_num_entries; i++) {
		if (WordInDialog (l_tables, i, min_comps_ptr, max_class, 
					max_comps_ptr, parse_table))
		{
			if (mode==FindFirstCompartment)
			{
				if (!COMPARTMENT_MASK_EQUAL (
					l_tables->l_words[i].l_w_cm_mask,
						l_0_compartments))
					mode = FoundFirstCompartment;
				else
					mode = FoundFirstMarking;
			}
			else if (mode == FoundFirstCompartment
			 && COMPARTMENT_MASK_EQUAL (
				l_tables->l_words[i].l_w_cm_mask,
						l_0_compartments))
					mode = FoundFirstMarking;
			 
			word_to_string (buffer, l_tables, i);
			xmstring = XmStringLtoRCreate(buffer, charset);
			if (! xmstring)
				MemoryError();
			/* Have to guard the case of SYSLO and SYSHI which do
			 * not have compartment numbers, therefore the above
			 * code now thinks we are in the Markings section */

			if (mode == FoundFirstCompartment ||
			      ( (CalledFor != SL_DEVICES_DEF_SINGLE_LEVEL_IL) &&
			        (CalledFor != SL_DEVICES_MOD_SINGLE_LEVEL_IL))){
				comp_buttons [*comp_num] = 
					AddToggle(comp_rc, buffer, COMPARTMENTS_TOGGLE);
				*comp_num = *comp_num + 1;
			}
			else {
				/* if (mode == FoundFirstMarking) */
				comp_buttons [*comp_num] = 
					AddToggle(marks_rc, buffer, MARKINGS_TOGGLE);
				*comp_num = *comp_num + 1;
			} 
			XmStringFree (xmstring);
		}
	}
#ifdef DEBUG
	printf ("Set Toggle Names Leave debug end\n");
#endif /* DEBUG */
}

/*
 * The subroutine SetToggleButtons turns the classification and word buttons on
 * or off as required.  It also hilites (disables/dims) buttons whose value
 * cannot be changed by the user at this time for a large number of reasons.
 * class_buttons and comp_buttons are assumed big enough.
 */

static void
SetToggleButtons (l_tables, parse_table, class, comps_ptr,
		 marks_ptr, sclass, scomps_ptr, ucclass, uccomps_ptr,
		 min_class, min_comps_ptr,
		 class_buttons, comp_buttons)
register struct l_tables *l_tables;
char *parse_table;
CLASSIFICATION class;
COMPARTMENTS *comps_ptr;
MARKINGS *marks_ptr;
CLASSIFICATION sclass;
COMPARTMENTS *scomps_ptr;
CLASSIFICATION ucclass;
COMPARTMENTS *uccomps_ptr;
CLASSIFICATION min_class;
COMPARTMENTS *min_comps_ptr;
Widget	*class_buttons;
Widget	*comp_buttons;
{
	register int i,j;
	register struct l_word_pair *wp;
	struct l_constraints *cp;
	short *lp;
	short *invalid_list;
	short *end_invalid_list;
	int item;	/* NJH */
	short itemType;
	CLASSIFICATION words_min_class;	/* the min class required by words on */
	CLASSIFICATION words_max_class;	/* the max class required by words on */

/*
 * If the class is NO_LABEL, meaning that the label has not been set yet, 
 * then no buttons should be set, so just return.
 */

	if (class == NO_LABEL) return;

/*
 * Now, loop through the parse table, turning on the FORCED bit in entries
 * that are FORCED on or off because 1) they are required by some other word that
 * is on, or 2) they are part of an invalid combination.  While looping through
 * the table, also compute words_min_class, the minimum classification required given
 * the words that are on, and words_max_class. the maximum classification required
 * by words that are on.
 */

#define L_OK 4
#define FORCED 2
#define ON 1

	words_min_class = min_class;
	words_max_class = sclass;
#ifdef DEBUG
	printf ("Words 1 min/x cl %d %d\n", words_min_class, words_max_class);
	printf ("Set Toggle Buttons Enter debug start\n");
	printf ("class %d\n", class);
	printf ("comps_ptr %u %x\n", *comps_ptr, *comps_ptr);
	printf ("marks_ptr %u %x\n", *marks_ptr, *marks_ptr);
#endif

	for (i=l_tables->l_first_main_entry; i< l_tables->l_num_entries; i++)
	if (parse_table[i]&ON)		/* if this word is on... */
	{
		words_min_class = L_MAX (words_min_class,
			 l_tables->l_words[i].l_w_min_class);
		words_max_class = L_MIN (words_max_class,
			l_tables->l_words[i].l_w_max_class);

		for (wp=l_tables->l_required_combinations; 
				wp->l_word1 != L_END; wp++)
			if (i==wp->l_word1 
					/* if this word requires another... */
	 		 && WORD_VISIBLE (wp->l_word2, sclass, scomps_ptr)) {
						/* and other visible */
			parse_table[wp->l_word2] |= FORCED; 
#ifdef DEBUG
			printf ("Forced %d\n", wp->l_word2);
#endif
			}
					/* mark it as forced on */
		
		for (cp = l_tables->l_constraints; cp->l_c_type != L_END; ++cp)
		{
			if (cp->l_c_type==ONLY_WITH)
			{
				for (lp = cp->l_c_first_list; 
					lp != cp->l_c_second_list; lp++)
				 if (i==*lp) {
					for (lp = cp->l_c_second_list; 
					lp < cp->l_c_end_second_list; lp++) {
						parse_table[*lp] |= L_OK;
						/* mark word OK to be on */
#ifdef DEBUG
					printf ("ForcedOK2 %d\n", *lp);
#endif
					}
					
					for (j=l_tables->l_first_main_entry; 
					j<l_tables->l_num_entries; j++)
					 if (j != i) {
						if (parse_table[j]&L_OK) {
							parse_table[j] &= ~L_OK;
#ifdef DEBUG
						printf ("ForcedNotOK2 %d\n", j);
#endif
						}
						else {
						parse_table[j] |= FORCED;
#ifdef DEBUG
						printf ("ForcedNot %d\n", j);
#endif
						}
					}
					break;
				}
				
				for (lp = cp->l_c_second_list; 
					lp != cp->l_c_end_second_list; lp++)
				 	if (i==*lp) break;
				 	
				if (lp == cp->l_c_end_second_list)
				{
					for (lp = cp->l_c_first_list; 
						lp < cp->l_c_second_list; lp++)
					if (i != *lp) {
						parse_table[*lp] |= FORCED;
#ifdef DEBUG
						printf ("F4 %d\n", *lp);
#endif
					}
						/* mark word OK to be on */
				}
				
			}
			else
			{
				invalid_list = 0;
				for (lp = cp->l_c_first_list; 
					lp != cp->l_c_second_list; lp++)
				 if (i==*lp)
				{
					invalid_list = cp->l_c_second_list;
					end_invalid_list = 
						cp->l_c_end_second_list;
					break;
				}
				
				if (!invalid_list) 
				for (lp = cp->l_c_second_list;
					 lp != cp->l_c_end_second_list;
				 	lp++) if (i==*lp)
				{
					invalid_list = cp->l_c_first_list;
					end_invalid_list = cp->l_c_second_list;
					break;
				}
					
				if (invalid_list) for 
				(lp=invalid_list; lp!=end_invalid_list; 
					lp++) {
					parse_table[*lp] |= FORCED;
#ifdef DEBUG
					printf ("F5 %d\n", *lp);
#endif
				}
						/* mark it as forced off */ 
			}
		}
	} 
		

/*
 * Now, loop again through the parse table, turning on the FORCED bit in entries
 * that are FORCED off because 1) they are not visible given the passed sclass
 * and scomps; 2) their output minimum classification is below the current
 * classification; 3) they have a min_class above the words_max_class; 4) they
 * have a max_class below the current classification; 4) they are lower in a
 * hierarchy than another word that is on, or 5) they are higher in a hierarchy
 * than some other word that is FORCED on.
 */
 
	for (i=l_tables->l_first_main_entry; i< l_tables->l_num_entries; i++) {
#ifdef DEBUG
		printf ("HH %d %d %d %d %d %d %d\n",
			i, l_tables->l_words[i].l_w_output_min_class,
		 	l_tables->l_words[i].l_w_min_class,
			words_max_class,
		 	l_tables->l_words[i].l_w_max_class,
			class,
			parse_table[i]) ;
#endif
		if (!WORD_VISIBLE (i, sclass, scomps_ptr)
		 || class < l_tables->l_words[i].l_w_output_min_class
		 || l_tables->l_words[i].l_w_min_class > words_max_class
		 || l_tables->l_words[i].l_w_max_class < class) {
			parse_table[i] = FORCED;	/* mark forced off */
#ifdef DEBUG
			printf ("H0 %d\n", i);
#endif
		}
			
		else {
		if (parse_table[i]&ON)	/* if this word is ON */
			for (j=l_tables->l_first_main_entry; 
				j<l_tables->l_num_entries; j++) {
				if (WORD_VISIBLE (j, ucclass, uccomps_ptr)
			  	 && j!=i
			  	 && parse_table[j]==FALSE) {
#ifdef DEBUG
					printf ("H9 %d %d %d %x %x %x %x\n", 
						i,j,parse_table[j],
					   *l_tables->l_words[i].l_w_cm_mask,
			  		   *l_tables->l_words[j].l_w_cm_mask,
					   *l_tables->l_words[i].l_w_cm_value,
				  	   *l_tables->l_words[j].l_w_cm_value);
#endif
					if (COMPARTMENT_MASK_DOMINATE 
					 (l_tables->l_words[i].l_w_cm_mask,
			  		   l_tables->l_words[j].l_w_cm_mask)
					 && COMPARTMENTS_DOMINATE 
					 (l_tables->l_words[i].l_w_cm_value,
				  	   l_tables->l_words[j].l_w_cm_value)
				 	 && MARKING_MASK_DOMINATE 
					 (l_tables->l_words[i].l_w_mk_mask,
				  	   l_tables->l_words[j].l_w_mk_mask)
				 	 && MARKINGS_DOMINATE 
					 (l_tables->l_words[i].l_w_mk_value,
				  	   l_tables->l_words[j].l_w_mk_value)) {
						parse_table[j] = FORCED;
#ifdef DEBUG
					printf ("H1 %d\n", j);
#endif
					}
						/* mark forced off */
						
					if (parse_table[i]&FORCED
			  		 && parse_table[j]==FALSE
					 && COMPARTMENT_MASK_DOMINATE 
					 (l_tables->l_words[j].l_w_cm_mask,
			  		l_tables->l_words[i].l_w_cm_mask)
					 && COMPARTMENTS_DOMINATE 
					 (l_tables->l_words[j].l_w_cm_value,
				  	l_tables->l_words[i].l_w_cm_value)
				 	 && MARKING_MASK_DOMINATE 
					 (l_tables->l_words[j].l_w_mk_mask,
				  	   l_tables->l_words[i].l_w_mk_mask)
				 	 && MARKINGS_DOMINATE 
					 (l_tables->l_words[j].l_w_mk_value,
				  	   l_tables->l_words[i].l_w_mk_value)) {
						parse_table[j] = FORCED;
#ifdef DEBUG
					printf ("H2 %d\n", j);
#endif
						/* mark forced off */
					}
				}
			}
		}
	}
	
/*
 * Now, loop through the words again, marking as FORCED those words that are
 * FORCED off because some word they REQUIRE is FORCED off.
 */

	for (i=l_tables->l_first_main_entry; i< l_tables->l_num_entries; i++)
	{
		if (parse_table[i]==FORCED)	/* if this word is FORCED OFF */
			for (wp=l_tables->l_required_combinations; 
				wp->l_word1 != L_END; wp++)
				if (i==wp->l_word2) {/* if this word is required */
					parse_table[wp->l_word1] = FORCED;
#ifdef DEBUG
					printf ("H3 %d\n", wp->l_word1);
#endif
				}
						/* mark it as forced off */
	}

/*
 * Scan the parse table for words that are on so that the minimum and the
 * maximum classifications can be set based on the words that are enabled.
 */

/*
	words_min_class = min_class;
	words_max_class = ucclass;
#ifdef DEBUG
	printf ("Words3 min/x cl %d %d\n", words_min_class, words_max_class);
#endif
	
	for (i=l_tables->l_first_main_entry; i< l_tables->l_num_entries; i++) {

	    if (parse_table[i]&ON) {
		words_min_class = L_MAX (words_min_class,
			 l_tables->l_words[i].l_w_min_class);
		words_max_class = L_MIN (words_max_class,
			l_tables->l_words[i].l_w_max_class);
#ifdef DEBUG
printf ("Words4 %d min/x cl %d %d\n", i, words_min_class, words_max_class);
#endif
	    }

	}
*/
		
/*
 * Next, loop through the classifications, turning the classification buttons on and
 * off as appropriate, and disabling (dimming) those that are below the 
 * the minimum classification required (computed above), are above the words_max_class
 * (computed above), or above the passed sensitivity level classification.
 */

	j = 0;

	for (i=0; i<= *l_max_classification; i++)
	{
 		if (CLASSIFICATION_VISIBLE (i, ucclass)
 		 && i >= min_class)
		{
			if (class==i) /* if this button represents classification in label */
			{
				ToggleButtonSetState(
					class_buttons[j], True);
			}
			else
			{
				ToggleButtonSetState(
					class_buttons[j], False);
			}

#ifdef DEBUG
			printf ("F6 %d %d %d %d\n",
				i,words_min_class, words_max_class, sclass);
#endif
			if(i < words_min_class ||
			   i > words_max_class ||
			   i > sclass ||
			   words_min_class == sclass) {
				ToggleButtonSetSensitivity(
					class_buttons[j], False);
#ifdef DEBUG
				printf ("F7\n");
#endif
			}
			else {
				ToggleButtonSetSensitivity(
					class_buttons[j], True);
#ifdef DEBUG
				printf ("F8\n");
#endif
			}
			j++;
		}
	}

/*
 * Now loop through the words that should be in the dialog, turning the buttons
 * on or off as appropriate and dimming as needed.
 */

	j = 0;
	for (i=l_tables->l_first_main_entry; i< l_tables->l_num_entries; i++)
		if (WordInDialog (l_tables, i, min_comps_ptr, ucclass, uccomps_ptr,
				  parse_table))
	{
			/* set button on or off */
#ifdef DEBUG
		printf ("G0 %d %d %d\n", j, parse_table[i], parse_table[i]&ON);
#endif
		ToggleButtonSetState(comp_buttons[j], parse_table[i]&ON);

	/* Need to put a check in to see when we start on markings */
/*
 * If the word is an inverse compartment (tested by first two parts of if
 * statement below), and is logically on (bits are zero) in the sensitivity
 * label, then this word will be on, but cannot be turned off because to
 * do so would put the IL above the SL.  Therefore, the word must be dimmed.
 * Likewise if a non-inverse compartment that is on in the minimum compartments.
 */
 
		if (!COMPARTMENT_MASK_EQUAL (l_tables->l_words[i].l_w_cm_mask,
		 l_0_compartments)	/* if this is a compartment word */
		 && COMPARTMENTS_EQUAL (l_tables->l_words[i].l_w_cm_value,
		 l_0_compartments)	/* for an inverse compartment */
		 && COMPARTMENTS_IN (scomps_ptr,
		 l_tables->l_words[i].l_w_cm_mask,
		l_tables->l_words[i].l_w_cm_value))
		/* and indicated by the SL */
			{
			ToggleButtonSetSensitivity(
					comp_buttons[j], False);
#ifdef DEBUG
					printf ("G1 %d\n", j);
#endif
			}
		else if (!COMPARTMENT_MASK_EQUAL 
			(l_tables->l_words[i].l_w_cm_mask,
		  l_0_compartments)	/* if this is a compartment word */
		 && !COMPARTMENTS_EQUAL (l_tables->l_words[i].l_w_cm_value,
		 l_0_compartments)	/* for a non-inverse compartment */
		 && COMPARTMENTS_IN (min_comps_ptr,
		l_tables->l_words[i].l_w_cm_mask,
		l_tables->l_words[i].l_w_cm_value))
			{
#ifdef DEBUG
					printf ("G2 %d\n", j);
#endif
			ToggleButtonSetSensitivity(
					comp_buttons[j], False);
			}

/*
 * Otherwise, the button should be dimmed if it was FORCED (on or off). Otherwise,
 * it is not dimmed.
 */

		else if (parse_table[i]&FORCED)
			{
#ifdef DEBUG
			printf ("F9 %d\n", j);
#endif
			ToggleButtonSetSensitivity(
					comp_buttons[j], False);
			}
		else
			{
#ifdef DEBUG
			printf ("FA %d\n", j);
#endif
			ToggleButtonSetSensitivity(
					comp_buttons[j], True);
			}
		j++;
	}

	/* Debug start */
#ifdef DEBUG
	printf ("Set Toggle Buttons Leave debug start\n");
	for (i=l_tables->l_first_main_entry; i< l_tables->l_num_entries; i++) {
		printf ("%d %d %s\n", i, parse_table[i], 
			l_tables->l_words[i].l_w_output_name);
	}
	printf ("Set Toggle Buttons Leave debug end\n");
#endif
	/* Debug end */

}	

#endif /* SEC_MAC */
#endif /* SEC_BASE */
