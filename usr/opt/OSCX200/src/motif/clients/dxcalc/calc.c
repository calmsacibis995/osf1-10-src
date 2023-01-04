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
#ifdef OSF1
/*
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint
static char *BuildSystemHeader =
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxcalc/calc.c,v 1.1.7.2 1993/06/22 21:52:07 Susan_Ng Exp $";
#endif
#endif
/* Header from VMS source pool
static char *BuildSystemHeader =
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxcalc/calc.c,v 1.1.7.2 1993/06/22 21:52:07 Susan_Ng Exp $";
*/
/*
**++

  Copyright (c) Digital Equipment Corporation,
  1987, 1988, 1989, 1990, 1991, 1992
  All Rights Reserved.  Unpublished rights reserved
  under the copyright laws of the United States.

  The software contained on this media is proprietary
  to and embodies the confidential technology of
  Digital Equipment Corporation.  Possession, use,
  duplication or dissemination of the software and
  media is authorized only pursuant to a valid written
  license from Digital Equipment Corporation.

  RESTRICTED RIGHTS LEGEND   Use, duplication, or
  disclosure by the U.S. Government is subject to
  restrictions as set forth in Subparagraph (c)(1)(ii)
  of DFARS 252.227-7013, or in FAR 52.227-19, as
  applicable.

**--
**/

/*
**++
**  MODULE NAME:
**	calc.c
**
**  FACILITY:
**      OOTB Calculator
**
**  ABSTRACT:
**
**  AUTHORS:
**      Dennis McEvoy, Neal Finnegan
**
**  RELEASER:
**
**  CREATION DATE:     6-OCT-1987
**
**  MODIFICATION HISTORY:
**
**    Version:
**
**	Nov 1990 - V3.0 	(ASP) - Motif conversion and added memex support.
**      May 1992 - V3.1         (SP)  - I18N and maintenance changes.
**--
**/

#include "calcdefs.h"
#include "calc.h"
#include "calclogos.h"
#include <math.h>
#include <time.h>

#ifdef VMS
#include <types.h>
#include "dwi18n_lib.h"
#else
#include <strings.h>
#include <unistd.h>
#include <pwd.h>
#include <dwi18n_lib.h>
#endif

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif

#ifdef VMS
#include <decw$cursor.h>
#else
#include <X11/decwcursor.h>
#endif

#include <X11/cursorfont.h>
#include <DXm/DECspecific.h>

#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

/* Make this external. */
extern Widget toplevel;

/* Used for displaying of the Watch Cursor. */
Cursor watch_cursor = 0;
Cursor WorkCursorCreate();
void WorkCursorDisplay();
void WorkCursorRemove();

/* Public routines entry points. */
CalcWidget DWT$MY_CALC();
CalcWidget DWT$MY_CALC_CREATE();

CalcWidget DwtMyCalc();
CalcWidget DwtMyCalcCreate();

/* Private routines. */
/* In this module */
static void Realize();
static void Destroy();
static XtGeometryResult geometry_manager();

static void button_pressed();
static void button_released();
static Boolean accept_focus();
static void key_pressed();
static void key_released();
static void Help();
static void CallMenu();
void save_state();
void swap_state();
void send_type_char();
void calc_DisplayHelp();
void calc_serror();
void calc_display_message();
void ParseCS();
void UpdateTranslation();
XtGeometryResult CheckGeometryManager();
int CheckWindowAttributes();
static void find_menubar_height();
static void find_file();

void DrawKey();
void DrawString();
void HighlightKey();
void UnHighlightKey();
void calc_get_uil_string();

static void managed_set_changed();
static void Layout();
static void ClassInitialize();
static void Initialize();
static void InitializeStrings();
static Boolean SetValues();
static void Resize();
static void add_kid();
static void remove_child();

static void Repaint();
static void logo_repaint();
static void DetermineKey();
static void CalcSelectFont();
static void CalcSelectSqrtFont();

static unsigned int copy_proc();
static unsigned int create_proc();
static unsigned int exit_proc();
static unsigned int help_done_proc();
static unsigned int main_help_proc();
static unsigned int message_done_proc();
static unsigned int mode_proc();
static void on_context_activate_proc();
static void tracking_help();
static unsigned int paste_proc();
static unsigned int restore_settings_proc();
static unsigned int save_settings_proc();
static unsigned int undo_proc();

static void popup_procedure();

#ifdef VMS
static char *VMSDescToNull(struct dsc$descriptor_s *desc);
#else
static char *createHomeDirSpec();
#endif

#ifndef NO_HYPERHELP
#ifndef VMS
#include <DXm/bkr_api.h>
#endif
Opaque hyper_help_context;
#endif

/* In calcutils.c */
extern void calc_initialize();
extern void typechar();
extern void ReverseKey();
extern CalcWidget calc_FindMainWidget();

extern void error();
extern void mem_error();
extern void display_in_window();
extern void mem_value_dis();
extern void getnumstring();
extern void CalcLoadFontFamilies();
extern void CalcLoadSqrtFontFamilies();

#ifdef NO_USE
static char translations[] = "Any<KeyDown>:		key_pressed()\n\
	 Any<KeyUp>:		key_released()\n\
	 ~Help<Btn1Down>:	button_pressed()\n\
	 ~Help<Btn1Up>:		button_released()\n\
	 <Leave>:		button_released()\n\
	 <Btn3Down>:	 	MenuCalled()\n\
   	 Help<Btn1Down>:	Help()";
#endif

/* New Composite Class Extension Record for V1.1 Motif. */
static CompositeClassExtensionRec compositeClassExtRec = {NULL, NULLQUARK,
  XtCompositeExtensionVersion, sizeof(CompositeClassExtensionRec), TRUE,};


/* transfer vector from translation manager action names to address of routines */
static XtActionsRec actions[] = {
    {"key_pressed", (XtActionProc) key_pressed},
    {"key_released", (XtActionProc) key_released},
    {"button_pressed", (XtActionProc) button_pressed},
    {"button_released", (XtActionProc) button_released},
    {"MenuCalled", (XtActionProc) CallMenu},
    {"Help", (XtActionProc) Help},
    {NULL, NULL}
};

/* widget resources */
static int resource_0 = 0;
static int resource_1 = 1;
static int resource_5 = 5;
static int resource_190 = 190;
static int resource_240 = 240;

static XtResource resources[] =
{
    /* main window specific resources */
    {
	XmNx, 				/* field name */
	XtCX, 				/* resource class name */
	XtRPosition, 			/* code for type of resource */
	sizeof(Position), 		/* size of resource in bytes */
	XtOffset(CalcWidget, core. x), 	/* offset in bytes of field in widget */
	XtRPosition, 			/* default resource type */
	(caddr_t) & resource_5 		/* addresss of the default value */
    },

    {
	XmNy,
	XtCY,
	XtRPosition,
	sizeof(Position),
	XtOffset(CalcWidget, core.y),
	XtRPosition,
	(caddr_t) & resource_5
    },

    {
	XmNwidth,
	XtCWidth,
	XtRDimension,
	sizeof(Dimension),
	XtOffset(CalcWidget,
	core.width),
	XtRDimension,
	(caddr_t) & resource_190
    },

    {
	XmNheight,
	XtCHeight,
	XtRDimension,
	sizeof(Dimension),
	XtOffset(CalcWidget, core.height),
	XtRDimension,
	(caddr_t) & resource_240
    },

    {
	DwtNkeyFontFamily,
	XtCKeyFontFamily,
	XtRString,
	sizeof(char *),
	XtOffset(CalcWidget, calc.key_font_family),
	XtRString,
	"-Adobe-Times-Bold-R-Normal--*-*-*-*-P-*-ISO8859-1"
    },

    {
	DwtNsqrtFontFamily,
	XtCSqrtFontFamily,
	XtRString,
	sizeof(char *),
	XtOffset(CalcWidget, calc.sqrt_font_family),
	XtRString,
	"-Adobe-Symbol-Medium-R-Normal--*-*-*-*-P-*-ADOBE-FONTSPECIFIC"
    },

    {
	XmNtranslations,
	XmCTranslations,
	XmRTranslationTable,
	sizeof(XtTranslations),
	XtOffset(CalcWidget, calc.translations),
	XmRTranslationTable,
	NULL
    },

    {
	DwtNlanguage,
	XtCLanguage,
	XtRString,
	sizeof(char *),
	XtOffset(CalcWidget, calc.language),
	XtRString,
	"en_US.88591"
    },

    {
	DwtNnumberFormat,
	XtCNumberFormat,
	XtRString,
	sizeof(char *),
	XtOffset(CalcWidget, calc.number_format),
	XtRString,
	""
    }
};

externaldef(calcwidgetclassrec)
    CalcClassRec calcwidgetclassrec =
{
    {
	(WidgetClass) & xmManagerClassRec,	/* superclass*/
	CalcClassName,				/* class_name */
	sizeof(CalcWidgetRec),			/* widget_size */
	ClassInitialize,			/* class_initialize */
	NULL,					/* chained class part init */
	FALSE,					/* class_inited */
	Initialize,				/* initialize */
	NULL,					/* init hook proc */
	Realize,				/* realize */
	actions,				/* actions */
	XtNumber(actions),			/* num_actions */
	resources,				/* resources */
	XtNumber(resources),			/* num_resources */
	NULLQUARK,				/* xrm_class */
	TRUE,					/* compress_motion */
	TRUE,					/* compress_exposure */
	TRUE,					/* compress enter-leave */
	TRUE,					/* visible_interest */
	Destroy,				/* destroy */
	Layout,					/* resize */
	(XtExposeProc)Repaint,			/* expose */
	(XtSetValuesFunc)SetValues,		/* set_values */
	NULL,					/* set values hook proc */
	XtInheritSetValuesAlmost,		/* set values almost proc */
	NULL,					/* get values hook proc */
	(XtAcceptFocusProc)accept_focus,	/* accept_focus */
	XtVersion,				/* current version */
	NULL,					/* callback offset */
#ifdef NO_USE
	translations,				/* default trans */
#else
	NULL,					/* default trans */
#endif
	NULL,					/* Query geom proc */
	NULL,					/* Display Accelerator  */
	NULL					/* extension */
    },

    {						/* composite class record */
	geometry_manager,			/* childrens geo mgr proc */
	managed_set_changed,			/* set changed proc */
	add_kid,	 			/* add a child */
	remove_child,				/* remove a child */
	(XtPointer) &compositeClassExtRec,	/* extension */
    },

    {						/* Constraint class Init */
	NULL,					/* subresource list */
	0,					/* subresource count */
	0,					/* size of constraint record */
	NULL,					/* initialization */
	NULL,					/* destroy proc */
	NULL,					/* set_values proc */
	NULL					/* pointer to extension */
    },

    /*
    ** Manager Class
    */
    {
	XtInheritTranslations,			/* default translations */
	NULL,					/* get resources */
	0,					/* num get_resources */
	NULL,					/* get_cont_resources */
	0,					/* num_get_cont_resources */
	XmInheritParentProcess,			/* parent_process */
	NULL					/* extension */
    },
    {						/* calculator class record */
	NULL,
	0					/* just a dummy field */
    }
};

externaldef(calcwidgetclass)
    CalcWidgetClass calcwidgetclass = (CalcWidgetClass) & calcwidgetclassrec;

/*
**++
**  ROUTINE NAME:
**	add_kid
**
**  FUNCTIONAL DESCRIPTION:
**	add a child to this widget
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void add_kid(w, args, num_args)
    Widget w;
    ArgList args;
    Cardinal *num_args;
{
    (*((CompositeWidgetClass) compositeWidgetClass)->
/*    composite_class.insert_child)(w, args, num_args);*/
      composite_class.insert_child)(w);

    /* If the child is a subclass of dialog, set defaultpositioning to FALSE,
     * so that the subwidget don't try to position themselves */
    if (XtIsSubclass(w, xmBulletinBoardWidgetClass)) {

	XmBulletinBoardWidget d = (XmBulletinBoardWidget) w;

	d->bulletin_board.default_position = FALSE;
    }
/*    return (TRUE);*/
}

/*
**++
**  ROUTINE NAME:
**	remove_child
**
**  FUNCTIONAL DESCRIPTION:
**	Delete a single widget from a parent widget
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void remove_child(child)
    Widget child;
{
    (*((CompositeWidgetClass) compositeWidgetClass)->
      composite_class.delete_child)(child);
}

/*
**++
**  ROUTINE NAME:
**	Initialize
**
**  FUNCTIONAL DESCRIPTION:
**	This routine will initialize the widget instance
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void Initialize(request, new)
    Widget request, 			/* as build from arglist */
      new;				/* after superclasses init */
{
    CalcWidget mw = (CalcWidget) new;
    Dpy(mw) = XtDisplay(mw);
    HelpWidget(mw) = 0;
}

/*
**++
**  ROUTINE NAME:
**	ClassInitialize
**
**  FUNCTIONAL DESCRIPTION:
**	This routine will initialize the widget class
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void ClassInitialize ()
{
    XmResolvePartOffsets
	(calcwidgetclass, &calcwidgetclassrec.calc_class.calcoffsets);
}

#ifndef VMS
/*
**++
**  ROUTINE NAME:
**	createHomeDirSpec
**
**  FUNCTIONAL DESCRIPTION:
** 	Returns a file spec for the named file in the users home directory
** 	space needs to be freed with XtFree
**
**  FORMAL PARAMETERS:
**	char *simple	File name of the desired file in the users
**			home directory
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static char *createHomeDirSpec(simple)
    char *simple;
{
    char *dir = NULL;
    char *spec;
    char *user;
    int count = 0;
    struct passwd *pw;

    while (count < 3) {
	switch (count) {
	    case 0:
		dir = (char *) getenv("HOME");
		break;
	    case 1:
		if ((user = (char *) getenv("USER")) == NULL) {
		    break;
		}
		if ((pw = getpwnam(user)) == NULL) {
		    break;
		}
		dir = pw->pw_dir;
		break;
	    case 2:
		if ((pw = getpwuid(getuid())) == NULL) {
		    break;
		}
		dir = pw->pw_dir;
		break;
	}
	count++;
	if (dir == NULL)
	    continue;
	if (!access(dir, F_OK))
	    break;
    }
    spec = (char *) XtMalloc(strlen(dir) + strlen(simple) + 2);
    strcpy(spec, dir);
    strcat(spec, "/");
    strcat(spec, simple);
    return (spec);
}
#endif

/*
**++
**  ROUTINE NAME:
**	InitializeStrings
**
**  FUNCTIONAL DESCRIPTION:
**	This routine will initialize the widget instance
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void InitializeStrings(widget)
    CalcWidget widget;
{
    char *tmp, *status_str, base_defaults_name[256], appl_title[256],
      radix_str[10];
    int i, j, xtnumber, char_set, dont_care, array_offset, hex_array_offset;
    long byte_count, cvt_status;
    Arg args[8];
    Cardinal status;
    CalcWidget mw = widget;
    MrmCode *type;
    XmString *array_buffer, *hex_array_buffer, *CSstr, appl_title_cs;
    XmStringContext CScontext;
    /* RGMTextVectorPtr         vector; */

#ifdef TEST_XNLS
    status_str = xnl_winsetlocale(LC_ALL, "en_US.88591", &lan_id, Dpy(mw));
    if (status_str == 0) {
	calc_serror(BadLocale, FATAL);
    }
    CSstr = xnl_langinfo(lan_id, RADIXCHAR, 0);
    DumpCS(&CSstr);
    ParseCS(radix_str, CSstr);
#endif

    calc_get_uil_string(CalcHierarchy(mw), Dpy(mw), "CALC_APPLICATION_TITLE",
      appl_title);
    calc_get_uil_string(CalcHierarchy(mw), Dpy(mw), "CALC_APPLICATION_NAME",
      AppName(mw));
    calc_get_uil_string(CalcHierarchy(mw), Dpy(mw), "CALC_ERROR",
      ErrorString(mw));

#ifdef VMS
    calc_get_uil_string(CalcHierarchy(mw), Dpy(mw), "CALC_VMS_DEFAULTS_NAME",
      base_defaults_name);
    DefaultsName(mw) = (char *) XtMalloc(strlen(base_defaults_name) + 2);
    strcpy(DefaultsName(mw), base_defaults_name);
    calc_get_uil_string(CalcHierarchy(mw), Dpy(mw), "CALC_VMS_SYSTEM_DEF_NAME",
      base_defaults_name);
#else
    calc_get_uil_string(CalcHierarchy(mw), Dpy(mw), "CALC_UNIX_DEFAULTS_NAME",
      base_defaults_name);
    DefaultsName(mw) = (char *) createHomeDirSpec(base_defaults_name);
    calc_get_uil_string(CalcHierarchy(mw), Dpy(mw),
      "CALC_UNIX_SYSTEM_DEF_NAME", base_defaults_name);
#endif

    SystemDefaultsName(mw) = (char *) XtMalloc(strlen(base_defaults_name) + 2);
    strcpy(SystemDefaultsName(mw), base_defaults_name);

#ifndef NO_XNLS
    /* get radix point from locale */
    CSstr = xnl_langinfo(LanguageId(mw), RADIXCHAR, 0);
    ParseCS(radix_str, CSstr);
#endif

    /* Set CSnumber_format to 15:15 */
    CSNumberFormat(mw) = XmStringCreate("%z:15:15", XmSTRING_DEFAULT_CHARSET);

    /* Load decimal mode strings into keys */
    status = MrmFetchLiteral(CalcHierarchy(mw), "CALC_KEYS", Dpy(mw),
      &array_buffer, &type);

    /* Load Hex strings into keys */
    status = MrmFetchLiteral(CalcHierarchy(mw), "CALC_HEX_KEYS", Dpy(mw),
      &hex_array_buffer, &type);

    hex_array_offset = 0;
    array_offset = 0;
    for (i = 0; i < NUMMEMS; i++) {
	ParseCS(Mems(mw)[i].label, array_buffer[array_offset + i]);
	strcpy(Mems(mw)[i].hex_label, Mems(mw)[i].label);
	strcpy(Mems(mw)[i].oct_label, Mems(mw)[i].hex_label);
    }

    array_offset = array_offset + NUMMEMS;
    for (i = 0; i < NUMCLEARS; i++) {
	ParseCS(Clears(mw)[i].label, array_buffer[array_offset + i]);
	strcpy(Clears(mw)[i].hex_label, Clears(mw)[i].label);
	strcpy(Clears(mw)[i].oct_label, Clears(mw)[i].hex_label);
    }

    array_offset = array_offset + NUMCLEARS;
    for (i = 0; i < NUMKEYS; i++) {
	ParseCS(Keys(mw)[i].label, array_buffer[array_offset + i]);
	strcpy(Keys(mw)[i].hex_label, Keys(mw)[i].label);
	strcpy(Keys(mw)[i].oct_label, Keys(mw)[i].hex_label);
    }

    strcpy(Keys(mw)[k_key_pi - DIGIT_BASE].hex_label, "");
    strcpy(Keys(mw)[k_key_period - DIGIT_BASE].hex_label, "");
    strcpy(Keys(mw)[k_key_pi - DIGIT_BASE].oct_label, "");
    strcpy(Keys(mw)[k_key_period - DIGIT_BASE].oct_label, "");
    strcpy(Keys(mw)[k_key_8 - DIGIT_BASE].oct_label, "");
    strcpy(Keys(mw)[k_key_9 - DIGIT_BASE].oct_label, "");

    calc_get_uil_string(CalcHierarchy(mw), Dpy(mw), "CALC_POINT",
      Keys(mw)[1].label);

    ChrZero(mw) = Keys(mw)[k_key_0 - DIGIT_BASE].label[0];
    strcpy(ZeroPoint(mw), Keys(mw)[k_key_0 - DIGIT_BASE].label);

#ifdef NO_XNLS
    ChrPoint(mw) = Keys(mw)[k_key_period - DIGIT_BASE].label[0];
    strcat(ZeroPoint(mw), Keys(mw)[k_key_period - DIGIT_BASE].label);
#else
    ChrPoint(mw) = radix_str[0];
    strcat(ZeroPoint(mw), radix_str);
    /* Make decimal char display the same as radix_str of locale. */
    strcpy(Keys(mw)[1].label, radix_str);
#endif

    array_offset = array_offset + NUMKEYS;
    for (i = 0; i < NUMOPS; i++) {
	ParseCS(Ops(mw)[i].label, array_buffer[array_offset + i]);
	strcpy(Ops(mw)[i].hex_label, Ops(mw)[i].label);
	strcpy(Ops(mw)[i].oct_label, Ops(mw)[i].hex_label);
    }

    strcpy(Ops(mw)[k_key_plus_minus - OPS_BASE].hex_label, "");
    strcpy(Ops(mw)[k_key_percent - OPS_BASE].hex_label, "");
    strcpy(Ops(mw)[k_key_plus_minus - OPS_BASE].oct_label, "");
    strcpy(Ops(mw)[k_key_percent - OPS_BASE].oct_label, "");

    array_offset = array_offset + NUMOPS;
    for (i = 0; i < NUMFUNC; i++) {
	ParseCS(Func(mw)[i].label, array_buffer[array_offset + i]);
	ParseCS(Func(mw)[i].hex_label, hex_array_buffer[hex_array_offset + i]);
	strcpy(Func(mw)[i].oct_label, Func(mw)[i].hex_label);
    }

    hex_array_offset = hex_array_offset + NUMFUNC;
    array_offset = array_offset + NUMFUNC;
    for (i = 0; i < NUMINVFUNC; i++) {
	ParseCS(InvFunc(mw)[i].label, array_buffer[array_offset + i]);
	ParseCS(InvFunc(mw)[i].hex_label,
	  hex_array_buffer[hex_array_offset + i]);
	strcpy(InvFunc(mw)[i].oct_label, InvFunc(mw)[i].hex_label);
    }

    InvFunc(mw)[k_key_square_root - INV_FUNC_BASE].label[1] = 0;
    Keys(mw)[k_key_pi - DIGIT_BASE].label[1] = 0;
    strcpy(InvFunc(mw)[k_key_a - HEX_BASE].oct_label, "");
    strcpy(InvFunc(mw)[k_key_b - HEX_BASE].oct_label, "");
    strcpy(InvFunc(mw)[k_key_c - HEX_BASE].oct_label, "");
    strcpy(InvFunc(mw)[k_key_d - HEX_BASE].oct_label, "");
    strcpy(InvFunc(mw)[k_key_e - HEX_BASE].oct_label, "");
    strcpy(InvFunc(mw)[k_key_f - HEX_BASE].oct_label, "");

    appl_title_cs = DXmCvtFCtoCS(appl_title, &byte_count, &cvt_status);
    DWI18n_SetTitle(XtParent(mw), appl_title_cs);
    DWI18n_SetIconName(XtParent(mw), appl_title_cs);

    UserDatabase(mw) = NULL;
    SystemDatabase(mw) = NULL;
    MergedDatabase(mw) = XtDatabase(Dpy(mw));
}

/*
**++
**  ROUTINE NAME:
**	calc_get_uil_string
**
**  FUNCTIONAL DESCRIPTION:
**	Fetches a UIL string using DRM.  The UI utility routine cannot
**	be used since a context block does not exist at this point.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void calc_get_uil_string(hierarchy, display, name_str, buffer)
    MrmHierarchy hierarchy;
    Display *display;
    char *name_str;
    char *buffer;
{
    Cardinal status;
    caddr_t *value;
    MrmCode *type;

    status = MrmFetchLiteral(hierarchy, name_str, display, &value, &type);

    strcpy(buffer, value);
/*  XtFree(value); */
}

/*
**++
**  ROUTINE NAME:
**	managed_set_changed
**
**  FUNCTIONAL DESCRIPTION:
**	Our children changed their managed state, layout everyone that is
**	left
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void managed_set_changed(w)
    Widget w;
{
    CalcWidget mw = (CalcWidget) w;
}

/*
**++
**  ROUTINE NAME:
**	accept_focus
**
**  FUNCTIONAL DESCRIPTION:
**	Routine to accept input focus.  It will attempt to give it to
**	its work area widget, and then its command window widget.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
/*
static void accept_focus (vw)
    CalcWidget vw;
{
    if ((Work (vw) != NULL) && XtIsManaged (Work (vw)) &&
	((Work (vw))->core.widget_class->core_class.accept_focus != NULL))
	(* (Work (vw))->core.widget_class->core_class.accept_focus) ((Work (vw)));
}
*/

/*
**++
**  ROUTINE NAME:
** 	SetValues (old, request, new)
**
**  FUNCTIONAL DESCRIPTION:
** 	This routine detects differences in two versions
** 	of a widget, when a difference is found the
** 	appropriate action is taken.  It is called when the
**	user calls the public routine XtSetvalues on the widget;
**
**  FORMAL PARAMETERS:
**	Widget old, new;
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static Boolean SetValues(current, request, new_one)
    Widget current, 			/* original widget */
      request, 				/* as modif by arglist */
      new_one;				/* as modif by superclasses */
{
    CalcWidget old = (CalcWidget) current;
    CalcWidget new = (CalcWidget) new_one;

    return (FALSE);
}

/*
**++
**  ROUTINE NAME:
**	Help
**
**  FUNCTIONAL DESCRIPTION:
**	This routine will be called from the widget's
**	main event handler via the translation manager.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void Help(w, ev, params, num_params)
    Widget w;
    XEvent *ev;
    char **params;
    int num_params;
{
    CalcWidget mw = (CalcWidget) w;
    XButtonPressedEvent *but_event = (XButtonPressedEvent *) ev;
    int *key;
    keyinfo *key_struct;
    XmString Frame;

    DetermineKey(mw, but_event->x, but_event->y, &key, &key_struct);

    /* printf ("Key was number %d \n", key); */

    Frame = XmStringCreate("Overview", XmSTRING_DEFAULT_CHARSET);
    calc_DisplayHelp(mw, Frame);
}

/*
**++
**  ROUTINE NAME:
**	CallMenu
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void CallMenu(w, ev, params, num_params)
    Widget w;
    XEvent *ev;
    char **params;
    int num_params;
{
    CalcWidget mw = (CalcWidget) w;

    XmMenuPosition(Menu(mw), ev);
    XtManageChild((Widget) Menu(mw));
}

/*
**++
**  ROUTINE NAME:
**	button_pressed
**
**  FUNCTIONAL DESCRIPTION:
**	Routine to called from the translation manager
**	when a button is pressed
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void button_pressed(w, event, params, num_params)
    Widget w;
    XEvent *event;
    char **params;
    int num_params;
{
    CalcWidget mw = (CalcWidget) w;
    XButtonPressedEvent *but_event = (XButtonPressedEvent *) event;
    int *key;
    keyinfo *key_struct;
    int selection_changed = FALSE;

    accept_focus(mw);
    DetermineKey(mw, but_event->x, but_event->y, &key, &key_struct);
    if ((int) key > k_memory) {
	if (key == (int *) k_key_inverse)
	    if (InvsEnabled(mw))
		UnHighlightKey(mw, key_struct);
	    else
		HighlightKey(mw, key_struct);
	else {
	    ButtonPressed(mw) = key_struct;
	    HighlightKey(mw, key_struct);
	}
	key_hit(mw, key, FALSE);
    }

    if ((int) key == k_memory)
	if (DisplaySelected(mw)) {
	    DisplaySelected(mw) = FALSE;
	    selection_changed = TRUE;
	}

    if ((int) key == k_display)
	if (!DisplaySelected(mw)) {
	    DisplaySelected(mw) = TRUE;
	    selection_changed = TRUE;
	}

    if (selection_changed) {
	display_in_window(mw, BufferCopy(mw), &Disp(mw));
	getnumstring(mw, MemValue(mw));
	display_in_window(mw, Numstr(mw), &Memory(mw));
    }
}

/*
**++
**  ROUTINE NAME:
**	button_released
**
**  FUNCTIONAL DESCRIPTION:
**	Routine to called from the translation manager
**	when a button is released
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void button_released(w, event, params, num_params)
    Widget w;
    XEvent *event;
    char **params;
    int num_params;
{
    CalcWidget mw = (CalcWidget) w;

    if (ButtonPressed(mw) != NULL) {
	UnHighlightKey(mw, ButtonPressed(mw));
	ButtonPressed(mw) = NULL;
    }
}

/*
**++
**  ROUTINE NAME:
**	key_pressed
**
**  FUNCTIONAL DESCRIPTION:
**	Routine to called from the translation manager when a key is pressed
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void key_pressed(w, event, params, num_params)
    CalcWidget w;
    XEvent *event;
    char **params;
    int *num_params;
{
    XKeyPressedEvent *key_event = (XKeyPressedEvent *) event;
    int num = *num_params;
    char input[10];

    memcpy(input, params[0], 10);

    if (KeyPressed(w) != NULL)
	UnHighlightKey(w, KeyPressed(w));

    typechar(w, input, FALSE);
}

/*
**++
**  ROUTINE NAME:
**	key_released
**
**  FUNCTIONAL DESCRIPTION:
**	Routine to called from the translation manager
**	when a key is released
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void key_released(w, event, params, num_params)
    CalcWidget w;
    XEvent *event;
    char **params;
    int num_params;
{
    XKeyReleasedEvent *key_event = (XKeyReleasedEvent *) event;

    /* ignore modifier keys */
    if ((key_event->keycode != SHFT_KEY) && (key_event->keycode != CTRL_KEY) &&
      (key_event->keycode != LOCK_KEY) && (key_event->keycode != COMP_KEY)) {

	if (KeyPressed(w) != NULL) {
	    UnHighlightKey(w, KeyPressed(w));
	    KeyPressed(w) = NULL;
	}
    }
}

/*
**++
**  ROUTINE NAME:
**	Destroy
**
**  FUNCTIONAL DESCRIPTION:
**	Destroy the widget specific data structs
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void Destroy(w)
    Widget w;
{
    CalcWidget mw = (CalcWidget) w;

/* free the callback lists. One remove call per callback */

}

/*
**++
**  ROUTINE NAME:
**	Realize
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void Realize(w, window_mask, window_atts)
    Widget w;
    Mask *window_mask;
    XSetWindowAttributes *window_atts;

{
    CalcWidget mw = (CalcWidget) w;
    Widget menu;
    MrmType *dummy_class;
    char *status_str;

    if (XtIsRealized( (Widget) mw))
	return;

    XtCreateWindow((Widget) mw, InputOutput, CopyFromParent, *window_mask, window_atts);

#ifndef NO_XNLS
    /* Load the language from XNLS */
    status_str = xnl_winsetlocale(0, 0, &LanguageId(mw), Dpy(mw));

    /* attempt to load US locale instead if the above fails */
    if (status_str == 0) {
	/* calc_serror(BadLanguage, WARNING); */
	status_str =
	  xnl_winsetlocale(LC_ALL, "en_US.88591", &LanguageId(mw), Dpy(mw));
    }


    /* if that doesn't work then bad locale */
    if (status_str == 0)
	calc_serror(BadLocale, FATAL);

    CSNumberFormat(mw) =
      XmStringCreate(NumberFormat(mw), XmSTRING_DEFAULT_CHARSET);
#endif

    if (MrmFetchWidget(CalcHierarchy(mw), "opt_popup_menu", mw, &menu,
      &dummy_class) != MrmSUCCESS) {
	calc_serror(NoPopup, FATAL);
    }

    Menu(mw) = (XmRowColumnWidget) menu;

    InitializeStrings(mw);
    calc_initialize(mw);

#ifdef SYNCH
    XSynchronize(XtDisplay(mw), 1);
#endif

    CalcLoadFontFamilies(mw);
    CalcLoadSqrtFontFamilies(mw);

    if (NumSqrtFonts(mw) != 0) {

	/* The DECterm technical character set changed the encoding for the pi
	 * and square root characters between V1 and V2. Determine which
	 * encoding to use by the number of characters  in the font */
	if (SqrtFonts(mw)[0]->max_char_or_byte2 > MAX_V1_FONT_CHARS) {
	    InvFunc(mw)[k_key_square_root - INV_FUNC_BASE].label[0] =
	      V2_SQRT_KEY;
	    Keys(mw)[k_key_pi - DIGIT_BASE].label[0] = V2_PI_KEY;
	}				/* else */
	  else {
	    InvFunc(mw)[k_key_square_root - INV_FUNC_BASE].label[0] =
	      V1_SQRT_KEY;
	    Keys(mw)[k_key_pi - DIGIT_BASE].label[0] = V1_PI_KEY;
	}				/* else */
    }					/* if */

    Layout(mw);
    XtInstallAllAccelerators((Widget) mw, (Widget) mw);
}

/*
**++
**  ROUTINE NAME:
**	Layout
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void Layout(p)
    Widget p;
{
    Position vert_pad, hor_pad;
    int i, j, k;
    CalcWidget mw = (CalcWidget) p;
    Dimension wid, hyt;
    Dimension disp_w, disp_h, mem_w, mem_h;
    Position disp_x, disp_y, mem_x, mem_y;
    Dimension membox_w, membox_h;
    Position membox_x, membox_y;
    Dimension clearbox_w, clearbox_h, keybox_w, keybox_h, opsbox_w, opsbox_h;
    Position clearbox_x, clearbox_y, keybox_x, keybox_y, opsbox_x, opsbox_y;
    Dimension funcbox_w, funcbox_h;
    Position funcbox_x;
    Dimension inv_funcbox_w, inv_funcbox_h;
    Position inv_funcbox_x;
    Dimension top_key_wid, bot_key_wid, key_hyt;
    Position key_x, key_y;
    Dimension temp;
    Dimension MB_h;
    Dimension margin;
    XtWidgetGeometry g, *gp;

    if (!XtIsRealized(mw))
	return;

    /* Get the current width and height of the workarea */
    wid = Width(mw);
    hyt = Height(mw);

    /* Determine the menu bar height */
    gp = &g;
    GeoMode(gp) = CWX | CWY | CWWidth | CWHeight;

    gp->width = wid;
    gp->height = Height(TopMenuBar(mw));
    gp->x = 0;
    gp->y = 0;
    find_menubar_height(TopMenuBar(mw), gp);

    calc_ChangeWindowGeometry(TopMenuBar(mw), gp, 0);
    (*(XtCoreProc(TopMenuBar(mw), resize)))(TopMenuBar(mw));

    MB_h = gp->height;

    /*
    ** Determine the key widths and key height spacing
    **
    ** The bottom key widths are determined by ¼ key width margin and spacing,
    ** 5 normal width keys and 1x1½ width key.  This results in 7½ key widths
    ** across the window.
    **
    ** The top key widths are determined by ¼ of a bottom key for margins and
    ** 4 equal sized keys across.
    **
    ** The key height is determined by 10 equal height "keys", 4x¼ height
    ** spaces, 1x½ height space and the ¼ height margins.
    */
    bot_key_wid = (wid * 2) / 15;
    top_key_wid = (wid - (bot_key_wid / 2)) / 4;
    key_hyt = (hyt - MB_h) / 12;

    vert_pad = key_hyt * VERTPADDINGRATIO;
    hor_pad = bot_key_wid * HORPADDINGRATIO;

    /* Determine the fonts based on the key width and height */
    CalcSelectFont (mw, key_hyt - 4, bot_key_wid - 4);
    CalcSelectSqrtFont (mw, key_hyt - 4, bot_key_wid - 4);

    /* Clear the area */
    if (CalcGC(mw) != 0)
	XFillRectangle(Dpy(mw), Win(mw), CalcClearGC(mw), 0, 0, wid, hyt);

    /* Determine the results display size */
    disp_x = hor_pad;
    disp_w = wid - disp_x - hor_pad;
    disp_y = 2 * vert_pad + MB_h;
    disp_h = key_hyt;

    /* Certain bounding boxes have same x and width as the display */
    membox_x = funcbox_x = inv_funcbox_x = clearbox_x = disp_x;
    membox_w = funcbox_w = inv_funcbox_w = disp_w;

    /* Determine the bounding boxes size and position */

    /* Membox for the logo, memory, and mem operators */
    membox_y = disp_y + disp_h + vert_pad;
    membox_h = key_hyt;

    /* Funcbox for degs, xfact, 1/x and random */
    funcbox_h = key_hyt;

    /* Inverse Funtions box for all invertable funcs */
    inv_funcbox_h = 2 * key_hyt;

    /* Clear box for clear and plus/minus */
    clearbox_h = opsbox_h = keybox_h = 4 * key_hyt;
    clearbox_y = keybox_y = opsbox_y = hyt - clearbox_h - vert_pad;
    clearbox_w = (bot_key_wid * 3) / 2;

    /* Ops box for operators PI and equals */
    opsbox_w = 2 * bot_key_wid;
    opsbox_x = wid - hor_pad - opsbox_w;

    /* Key box for key pad and decimal point */
    keybox_w = 3 * bot_key_wid;
    keybox_x = (((opsbox_x + (clearbox_x + clearbox_w)) / 2) - (keybox_w / 2));

    /* Load the display structure size */
    gp->x = disp_x;
    gp->y = disp_y;
    gp->width = disp_w;
    gp->height = disp_h;
    margin = gp->height - DisplayFontHeight(mw);
    LayoutKey (&Disp(mw), gp, margin, True);

    mem_h = disp_h - 4;

    /* Load the logo structure */
    gp->x = membox_x;
    gp->width = logo_width;
    gp->height = logo_height;
    /*
    ** temp is used for aligning the logo and the membox.
    */
    temp = (mem_h - logo_height) / 2;
    if (temp < 0) temp = 0;
    gp->y = membox_y + temp;
    LayoutKey (&LogoWindow(mw), gp, 0, False);

    /* Load the memory structure */
    gp->x = membox_x + logo_width + hor_pad;
    gp->y = membox_y;
    gp->width = membox_w - logo_width - hor_pad;
    gp->height = mem_h;
    margin = gp->height - MemoryFontHeight(mw);
    LayoutKey (&Memory(mw), gp, margin, True);

    /* Load the memory keys structures */
    gp->width = top_key_wid;
    gp->height = key_hyt;

    gp->x = membox_x;
    gp->y += key_hyt + vert_pad / 2;
    margin = gp->height - KeyFontHeight(mw);
    for (i = 0; i < 3; i++)
    {
	LayoutKey (&Mems(mw)[i], gp, margin, False);
	gp->x += gp->width;
    }
    gp->width = membox_w + membox_x - gp->x;
    LayoutKey (&Mems(mw)[i], gp, margin, False);

    /* Load the function keys structures */
    gp->width = top_key_wid;
    gp->x = funcbox_x;
    gp->y += key_hyt + ((vert_pad * 3) / 2);
    for (i = 0; i < 3; i++)
    {
	LayoutKey (&Func(mw)[i], gp, margin, False);
	gp->x += gp->width;
    }
    gp->width = funcbox_w + funcbox_x - gp->x;
    LayoutKey (&Func(mw)[i], gp, margin, False);

    /* Load the invertable functions keys structures */
    gp->y += key_hyt + vert_pad;
    for (k = 0, i = 0; i < 2; i++)
    {
	gp->width = top_key_wid;
	gp->x = inv_funcbox_x;
	for (j = 0; j < 3; j++, k++)
	{
	    LayoutKey (&InvFunc(mw)[k], gp, margin, False);
	    gp->x += gp->width;
	}
	gp->width = inv_funcbox_w + inv_funcbox_x - gp->x;
	LayoutKey (&InvFunc(mw)[k], gp, margin, False);
	k++;
	gp->y += key_hyt;
    }

    /* Load the clear keys structures */
    gp->x = clearbox_x;

    gp->width = (bot_key_wid * 3) / 2;
    gp->height = 2 * key_hyt;

    margin = gp->height - KeyFontHeight(mw);
    gp->y = clearbox_y;
    for (i = 0; i < 2; i++)
    {
	LayoutKey (&Clears(mw)[i], gp, margin, False);
	gp->y += (key_hyt * 2);
    }

    /*
    ** Load the keypad keys structures
    */
    gp->width = bot_key_wid;
    gp->height = key_hyt;
    margin = gp->height - KeyFontHeight(mw);
    for (k = 0, i = 0; i < 4; i++)
    {
	gp->y = (3 - i) * key_hyt + keybox_y;
	for (j = 0; j < 3; j++, k++)
	{
	    gp->x = j * bot_key_wid + keybox_x;
	    LayoutKey (&Keys(mw)[k], gp, margin, False);
	}
    }

    /* Load the operator keys structures */
    gp->width = bot_key_wid;
    gp->height = key_hyt;
    gp->x = opsbox_x;
    gp->y = opsbox_y;
    for (i = 0; i < 4; i++)
    {
	LayoutKey (&Ops(mw)[i], gp, margin, False);
	gp->y += key_hyt;
    }

    gp->x = bot_key_wid + opsbox_x;
    gp->y = opsbox_y;
    LayoutKey (&Ops(mw)[4], gp, margin, False);

    gp->y += key_hyt;
    LayoutKey (&Ops(mw)[5], gp, margin, False);

    gp->y += key_hyt;
    gp->height = 2 * key_hyt;
    margin = gp->height - KeyFontHeight(mw);
    LayoutKey (&Ops(mw)[6], gp, margin, False);

}

/*
**++
**  ROUTINE NAME:
**	find_menubar_height
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void find_menubar_height(m, g)
    Widget m;
    XtWidgetGeometry *g;
{
    if (m == NULL)
	return;

    if (!XtIsSubclass(m, xmRowColumnWidgetClass)) {
	g->height = XtHeight(m);	/* it's not a menu */
    } else {
	XtWidgetGeometry intended, reply;

	intended.request_mode = CWWidth;
					/* ask about this width */
	intended.width = g->width;	/* if it affects the size */
					/* he'll tell us */

	switch (XtQueryGeometry(m, &intended, &reply)) {
	    case XtGeometryAlmost: 	/* he wants to compromise */

		if (reply.request_mode & CWHeight) {
		    g->height = reply.height;
					/* take height he suggests */
		    return;
		}			/* else just use existing */

	    /* no break */
	    case XtGeometryYes: 	/* he agrees, no problem w/ */
					/* current height */

	    /* no break */
	    case XtGeometryNo: 		/* wants to be his current */
		g->height = XtHeight(m);
					/* size */
		break;
	}
    }
}

/*
**++
**  ROUTINE NAME:
**	CalcSelectFont
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void CalcSelectFont(mw, key_hyt, key_wid)
    CalcWidget mw;
    int key_hyt, key_wid;
{
    XGCValues gcv;
    int i;
    int hyt_select_id, wid_select_id;
    int use_key_hyt, use_key_wid;

    if (NumFonts(mw) == 0)
	return;

    use_key_hyt = key_hyt;
    use_key_wid = key_wid;

    if (use_key_hyt <= FontHeights(mw)[0])
	hyt_select_id = 0;
    else if (use_key_hyt >= FontHeights(mw)[NumFonts(mw) - 1])
	hyt_select_id = NumFonts(mw) - 1;
    else
	for (i = 0; i < NumFonts(mw) - 1; i++)
	    if ((use_key_hyt >= FontHeights(mw)[i]) &&
	      (use_key_hyt < FontHeights(mw)[i + 1]))
		hyt_select_id = i;

    if (use_key_wid <= FontWidths(mw)[0])
	wid_select_id = 0;
    else if (use_key_wid >= FontWidths(mw)[NumFonts(mw) - 1])
	wid_select_id = NumFonts(mw) - 1;
    else
	for (i = 0; i < NumFonts(mw) - 1; i++)
	    if ((use_key_wid >= FontWidths(mw)[i]) &&
	      (use_key_wid < FontWidths(mw)[i + 1]))
		wid_select_id = i;

    if (wid_select_id < hyt_select_id)
	CFont(mw) = CFonts(mw)[wid_select_id];
    else
	CFont(mw) = CFonts(mw)[hyt_select_id];

    if (CalcGC(mw) != 0) {
	gcv. font = CFont(mw)->fid;

	XChangeGC(Dpy(mw), CalcGC(mw), GCFont, &gcv);
	XChangeGC(Dpy(mw), CalcClearGC(mw), GCFont, &gcv);
    }

    DisplayFontHeight(mw) =
      (CFont(mw)->max_bounds.ascent + CFont(mw)->max_bounds.descent);

    MemoryFontHeight(mw) = DisplayFontHeight(mw);
    KeyFontHeight(mw) = DisplayFontHeight(mw);
}

/*
**++
**  ROUTINE NAME:
**	CalcSelectSqrtFont
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void CalcSelectSqrtFont(mw, key_hyt, key_wid)
    CalcWidget mw;
    int key_hyt, key_wid;
{
    XGCValues gcv;
    int i;
    int hyt_select_id, wid_select_id;
    int use_key_hyt, use_key_wid;

    if (NumSqrtFonts(mw) == 0)
	return;

    use_key_hyt = key_hyt;
    use_key_wid = key_wid;

    if (use_key_hyt <= SqrtFontHeights(mw)[0])
	hyt_select_id = 0;
    else if (use_key_hyt >= SqrtFontHeights(mw)[NumSqrtFonts(mw) - 1])
	hyt_select_id = NumSqrtFonts(mw) - 1;
    else
	for (i = 0; i < NumSqrtFonts(mw) - 1; i++)
	    if ((use_key_hyt >= SqrtFontHeights(mw)[i]) &&
	      (use_key_hyt < SqrtFontHeights(mw)[i + 1]))
		hyt_select_id = i;

    if (use_key_wid <= SqrtFontWidths(mw)[0])
	wid_select_id = 0;
    else if (use_key_wid >= SqrtFontWidths(mw)[NumSqrtFonts(mw) - 1])
	wid_select_id = NumSqrtFonts(mw) - 1;
    else
	for (i = 0; i < NumSqrtFonts(mw) - 1; i++)
	    if ((use_key_wid >= SqrtFontWidths(mw)[i]) &&
	      (use_key_wid < SqrtFontWidths(mw)[i + 1]))
		wid_select_id = i;

    if (wid_select_id < hyt_select_id)
	SqrtFont(mw) = SqrtFonts(mw)[wid_select_id];
    else
	SqrtFont(mw) = SqrtFonts(mw)[hyt_select_id];

    if (CalcGC(mw) != 0) {
	gcv. font = SqrtFont(mw)->fid;

	XChangeGC(Dpy(mw), SqrtGC(mw), GCFont, &gcv);
	XChangeGC(Dpy(mw), SqrtClearGC(mw), GCFont, &gcv);
    }

    SqrtFontHeight(mw) =
      (SqrtFont(mw)->max_bounds.ascent + SqrtFont(mw)->max_bounds.descent);
}

/*
**++
**  ROUTINE NAME:
**	Repaint
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void Repaint(p)
    Widget p;
{
    int i, direction, ascent, descent, pixel_count, rc = 0, ac = 0;
    unsigned long ScreenNumber;
    Arg al[5];
    CalcWidget mw = (CalcWidget) p;
    Display *dpy;
    GC gc, gc1;
    Pixel fgpixel, bgpixel;
    XGCValues gcv;
    XRectangle rects[45];
    XmFontList FontList;
    XCharStruct overall;
    XFontStruct *font_struct;

    if (!XtIsRealized(mw))
	return;

    dpy = Dpy(mw);
    ScreenNumber = XDefaultScreen(dpy);

    if (CalcGC(mw) == 0) {

	fgpixel = Foreground(mw);
	bgpixel = Background(mw);

	gcv. foreground = fgpixel;
	gcv. background = bgpixel;

	CalcGC(mw) =
	  XCreateGC(dpy, XtWindow(mw), GCForeground | GCBackground, &gcv);

	SqrtGC(mw) =
	  XCreateGC(dpy, XtWindow(mw), GCForeground | GCBackground, &gcv);

	gcv. foreground = bgpixel;
	gcv. background = fgpixel;

	CalcClearGC(mw) =
	  XCreateGC(dpy, XtWindow(mw), GCForeground | GCBackground, &gcv);

	SqrtClearGC(mw) =
	  XCreateGC(dpy, XtWindow(mw), GCForeground | GCBackground, &gcv);

	gc = XDefaultGC(dpy, ScreenNumber);
	XCopyGC(dpy, gc, GCFont, CalcGC(mw));
	XCopyGC(dpy, gc, GCFont, CalcClearGC(mw));
	XCopyGC(dpy, gc, GCFont, SqrtGC(mw));
	XCopyGC(dpy, gc, GCFont, SqrtClearGC(mw));

	/* CFont (mw) = XLoadQueryFont (dpy, mw -> calc. key_font_family); */
	/* SqrtFont (mw) = XLoadQueryFont (dpy, mw -> calc. sqrt_font_family);
	 */

	if (NumFonts(mw) == 0) {
	    calc_display_message(NoKeyFont, ERROR);
	    CFont(mw) = XQueryFont(dpy, XGContextFromGC(gc));
	} else {
	    gcv. font = CFont(mw)->fid;

	    XChangeGC(dpy, CalcGC(mw), GCFont, &gcv);
	    XChangeGC(dpy, CalcClearGC(mw), GCFont, &gcv);
	}

	if (NumSqrtFonts(mw) == 0) {
	    calc_display_message(NoSqrtFont, ERROR);
	    SqrtFont(mw) = XQueryFont(dpy, XGContextFromGC(gc));
	} else {
	    gcv. font = SqrtFont(mw)->fid;

	    XChangeGC(dpy, SqrtGC(mw), GCFont, &gcv);
	    XChangeGC(dpy, SqrtClearGC(mw), GCFont, &gcv);
	}
    }

    DrawKey(mw, &Disp(mw), &rects[rc], &rc, FALSE);
    display_in_window(mw, BufferCopy(mw), &Disp(mw));

    DrawKey(mw, &Memory(mw), &rects[rc], &rc, FALSE);
    getnumstring(mw, MemValue(mw));
    display_in_window(mw, Numstr(mw), &Memory(mw));

    logo_repaint(mw);

    for (i = 0; i < NUMMEMS; i++)
	DrawKey(mw, &Mems(mw)[i], &rects[rc], &rc, TRUE);

    for (i = 0; i < NUMFUNC; i++)
	DrawKey(mw, &Func(mw)[i], &rects[rc], &rc, TRUE);

    if (InvsEnabled(mw))
	HighlightKey(mw, &InvFunc(mw)[0]);
    else
	UnHighlightKey(mw, &InvFunc(mw)[0]);
    for (i = 1; i < NUMINVFUNC; i++)
	DrawKey(mw, &InvFunc(mw)[i], &rects[rc], &rc, TRUE);

    for (i = 0; i < NUMCLEARS; i++)
	DrawKey(mw, &Clears(mw)[i], &rects[rc], &rc, TRUE);

    for (i = 0; i < NUMOPS; i++)
	DrawKey(mw, &Ops(mw)[i], &rects[rc], &rc, TRUE);

    for (i = 0; i < NUMKEYS; i++)
	DrawKey(mw, &Keys(mw)[i], &rects[rc], &rc, TRUE);

}

/*
**++
**  ROUTINE NAME:
**	DrawKey
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void DrawKey(w, key, rects, rc, draw_string)
    Widget w;
    keyinfo *key;
    XRectangle *rects;
    int *rc;
    Boolean draw_string;
{
    XmOffsetPtr	    o = CalcOffsetPtr(w);
    CalcWidget mw = (CalcWidget) w;

    rects->x = key->x;
    rects->y = key->y;
    rects->width = key->width;
    rects->height = key->height;
    *rc = *rc + 1;

    if (draw_string)
	if (((key == &InvFunc(mw)[k_key_square_root - INV_FUNC_BASE]) ||
	  (key == &Keys(mw)[k_key_pi - DIGIT_BASE])) && (Mode(mw) == k_decimal))
	    DrawString(mw, key, SqrtGC(mw));
	else
	    DrawString(mw, key, CalcGC(mw));

    _XmDrawShadow
    (
	XtDisplay(w),
	XtWindow(w),
	key->inv ? CalcBottomShadowGC(w,o) : CalcTopShadowGC(w,o),
	key->inv ? CalcTopShadowGC(w,o) : CalcBottomShadowGC(w,o),
	CalcShadowThickness(w,o),
	rects->x, rects->y, rects->width, rects->height
    );
}

/*
**++
**  ROUTINE NAME:
**	DrawString
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void DrawString(w, key, gc)
    Widget w;
    keyinfo *key;
    GC gc;
{
    int len, font_height;
    int direction = NULL;	/* Return direction value */
    int ascent = NULL;		/* Return ascent value */
    int descent = NULL;		/* Return descent value */
    CalcWidget mw = (CalcWidget) w;
    Dimension x, y;
    XCharStruct overall;	/* Return extent values */

    if (Mode(mw) == k_decimal)
	len = strlen(key->label);
    else if (Mode(mw) == k_hexadecimal)
	len = strlen(key->hex_label);
    else
	len = strlen(key->oct_label);

    if (((key == &InvFunc(mw)[k_key_square_root - INV_FUNC_BASE]) ||
      (key == &Keys(mw)[k_key_pi - DIGIT_BASE])) && (Mode(mw) == k_decimal)) {
	XTextExtents(SqrtFont(mw), key->label, len, &direction, &ascent,
	  &descent, &overall);
	font_height = SqrtFontHeight(mw);
	y = key->y + (key->height - ((key->height - font_height) / 2)) -
	  SqrtFont(mw)->max_bounds.descent;
    } else {
	XTextExtents(CFont(mw), key->label, len, &direction, &ascent, &descent,
	  &overall);
	font_height = KeyFontHeight(mw);
	y = key->y + (key->height - ((key->height - font_height) / 2)) -
	  CFont(mw)->max_bounds.descent;
    }

    x = key->x + (key->width - overall.width) / 2;

    if (Mode(mw) == k_decimal)
	XDrawString(Dpy(mw), Win(mw), gc, x, y, key->label, len);
    else if (Mode(mw) == k_hexadecimal)
	XDrawString(Dpy(mw), Win(mw), gc, x, y, key->hex_label, len);
    else
	XDrawString(Dpy(mw), Win(mw), gc, x, y, key->oct_label, len);
}

/*
**++
**  ROUTINE NAME:
**	HighlightKey
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void HighlightKey(w, key)
    Widget w;
    keyinfo *key;
{
    XmOffsetPtr	    o = CalcOffsetPtr(w);
    CalcWidget mw = (CalcWidget) w;

    if (((key == &InvFunc(mw)[k_key_square_root - INV_FUNC_BASE]) ||
      (key == &Keys(mw)[k_key_pi - DIGIT_BASE])) && (Mode(mw) == k_decimal))
	DrawString(mw, key, SqrtGC(mw));
    else
	DrawString(mw, key, CalcGC(mw));

    _XmDrawShadow
    (
	XtDisplay(w),
	XtWindow(w),
	key->inv ? CalcTopShadowGC(w,o) : CalcBottomShadowGC(w,o),
	key->inv ? CalcBottomShadowGC(w,o) : CalcTopShadowGC(w,o),
	CalcShadowThickness(w,o),
	key->x, key->y, key->width, key->height
    );
}

/*
**++
**  ROUTINE NAME:
**	UnHighlightKey
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
*/
void UnHighlightKey(w, key)
    Widget w;
    keyinfo *key;
{
    XmOffsetPtr	    o = CalcOffsetPtr(w);
    CalcWidget mw = (CalcWidget) w;

    XFillRectangle
    (
	Dpy(mw), Win(mw), CalcClearGC(mw),
	key->x, key->y,
	key->width, key->height
    );

    if (((key == &InvFunc(mw)[k_key_square_root - INV_FUNC_BASE]) ||
      (key == &Keys(mw)[k_key_pi - DIGIT_BASE])) && (Mode(mw) == k_decimal))
	DrawString(mw, key, SqrtGC(mw));
    else
	DrawString(mw, key, CalcGC(mw));

    _XmDrawShadow
    (
	XtDisplay(w),
	XtWindow(w),
	key->inv ? CalcBottomShadowGC(w,o) : CalcTopShadowGC(w,o),
	key->inv ? CalcTopShadowGC(w,o) : CalcBottomShadowGC(w,o),
	CalcShadowThickness(w,o),
	key->x, key->y, key->width, key->height
    );
}

/*
**++
**  ROUTINE NAME:
**	DetermineKey
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void DetermineKey(w, x, y, key_found, key)
    Widget w;
    Dimension x;
    Dimension y;
    int *key_found;
    keyinfo **key;
{
    CalcWidget mw = (CalcWidget) w;
    int i;

    *key_found = 0;
    for (i = 0; i < NUMMEMS; i++)
	if ((x > Mems(mw)[i].x) && (x < (Mems(mw)[i].x + Mems(mw)[i].width)) &&
	  (y > Mems(mw)[i].y) && (y < (Mems(mw)[i].y + Mems(mw)[i].height))) {
	    *key = &Mems(mw)[i];
	    *key_found = MEM_BASE + i;
	}

    if (*key_found == 0)
	for (i = 0; i < NUMKEYS; i++) {
	    if ((x > Keys(mw)[i].x) &&
	      (x < (Keys(mw)[i].x + Keys(mw)[i].width)) && (y >
	      Keys(mw)[i].y) && (y < (Keys(mw)[i].y + Keys(mw)[i].height))) {
		*key = &Keys(mw)[i];
		*key_found = DIGIT_BASE + i;
		if (((Mode(mw) == k_hexadecimal) || (Mode(mw) == k_octal)) &&
		  ((*key_found == k_key_period) || (*key_found == k_key_pi)))
		    *key_found = 0;
		if ((Mode(mw) == k_octal) &&
		  ((*key_found == k_key_8) || (*key_found == k_key_9)))
		    *key_found = 0;
	    }
	}

    if (*key_found == 0)
	for (i = 0; i < NUMOPS; i++)
	    if ((x > Ops(mw)[i].x) &&
	      (x < (Ops(mw)[i].x + Ops(mw)[i].width)) &&
	      (y > Ops(mw)[i].y) && (y < (Ops(mw)[i].y + Ops(mw)[i].height))) {
		*key = &Ops(mw)[i];
		*key_found = OPS_BASE + i;
		if (((Mode(mw) == k_hexadecimal) || (Mode(mw) == k_octal)) &&
		  ((*key_found == k_key_plus_minus) ||
		  (*key_found == k_key_percent)))
		    *key_found = 0;
	    }

    if (*key_found == 0)
	for (i = 0; i < NUMFUNC; i++)
	    if ((x > Func(mw)[i].x) &&
	      (x < (Func(mw)[i].x + Func(mw)[i].width)) && (y >
	      Func(mw)[i].y) && (y < (Func(mw)[i].y + Func(mw)[i].height))) {
		*key = &Func(mw)[i];
		if (Mode(mw) == k_decimal)
		    *key_found = FUNC_BASE + i;
		else
		    *key_found = LOGICAL_BASE + i;
	    }

    if (*key_found == 0)
	for (i = 0; i < NUMINVFUNC; i++)
	    if ((x > InvFunc(mw)[i].x) &&
	      (x < (InvFunc(mw)[i].x + InvFunc(mw)[i].width)) &&
	      (y > InvFunc(mw)[i].y) &&
	      (y < (InvFunc(mw)[i].y + InvFunc(mw)[i].height))) {
		*key = &InvFunc(mw)[i];
		if (Mode(mw) == k_decimal)
		    *key_found = INV_FUNC_BASE + i;
		else
		    *key_found = HEX_BASE + i;
		if ((Mode(mw) == k_octal) && (*key_found < k_key_not))
		    *key_found = 0;
	    }

    if (*key_found == 0)
	for (i = 0; i < NUMCLEARS; i++)
	    if ((x > Clears(mw)[i].x) &&
	      (x < (Clears(mw)[i].x + Clears(mw)[i].width)) && (y > Clears(
	      mw)[i].y) && (y < (Clears(mw)[i].y + Clears(mw)[i].height))) {
		*key = &Clears(mw)[i];
		*key_found = CLEAR_BASE + i;
	    }

    if (*key_found == 0)
	if ((x > Disp(mw).x) && (x < (Disp(mw).x + Disp(mw).width)) &&
	  (y > Disp(mw).y) && (y < (Disp(mw).y + Disp(mw).height))) {
	    *key = &Disp(mw);
	    *key_found = k_display;
	}

    if (*key_found == 0)
	if ((x > Memory(mw).x) && (x < (Memory(mw).x + Memory(mw).width)) &&
	  (y > Memory(mw).y) && (y < (Memory(mw).y + Memory(mw).height))) {
	    *key = &Memory(mw);
	    *key_found = k_memory;
	}
}

/*
**++
**  ROUTINE NAME:
**	accept_focus
**
**  FUNCTIONAL DESCRIPTION:
**	Routine to accept focus the calc widget
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static Boolean accept_focus(w)
    CalcWidget w;
{
    Boolean status;

    status = XmProcessTraversal(w, XmTRAVERSE_CURRENT);

/*  Replaced by the above call.
  XSetInputFocus (Dpy (w), Win (w), RevertToPointerRoot, CurrentTime);
  (* w->core.widget_class->core_class.accept_focus) (w);
*/
}

/*
**++
**  ROUTINE NAME:
**	geometry_manager
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static XtGeometryResult geometry_manager(w, request, reply)
    Widget w;
    XtWidgetGeometry *request, *reply;
{
    CalcWidget mw = (CalcWidget) XtParent(w);

    /* Just say no (for now) */
    return (XtGeometryNo);
}

#ifdef DWTVMS
/*
**++
**  ROUTINE NAME:
**	DWT$MY_CALC
**
**  FUNCTIONAL DESCRIPTION:
**	Public entry points for VMS
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
CalcWidget DWT$MY_CALC(parent, name$dsc, x, y, width, height)
    Widget *parent;
    struct dsc$descriptor_s *name$dsc;
    int *x;
    int *y;
    int *width;
    int *height;
{
    char *name;
    int argCount = 0;
    Arg arglist[20];
    CalcWidget mw;

    name = VMSDescToNull(name$dsc);

    XtSetArg(arglist[argCount], XmNx, *x);
    argCount++;
    XtSetArg(arglist[argCount], XmNy, *y);
    argCount++;
    XtSetArg(arglist[argCount], XmNheight, *height);
    argCount++;
    XtSetArg(arglist[argCount], XmNwidth, *width);
    argCount++;

    mw = XtCreateWidget(name, calcwidgetclass, *parent, arglist, argCount);

    return (mw);

}
#endif	/* DWTVMS */

#ifdef DWTVMS
/*
**++
**  ROUTINE NAME:
**	DWT$MY_CALC_CREATE
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
CalcWidget DWT$MY_CALC_CREATE(parent, name$dsc, arglist, argCount)

    Widget *parent;
    struct dsc$descriptor_s *name$dsc;
    Arg *arglist;
    int *argCount;
{
    CalcWidget mw;
    char *name;

    name = VMSDescToNull(name$dsc);

    mw = XtCreateWidget(name, calcwidgetclass, *parent, arglist, *argCount);


    return (mw);

}
#endif	/* DWTVMS */

#ifdef VMS
static char uid_filespec[] = {"DECW$CALC"};
#else
static char APPL_CLASS[] = {"DXcalc"};
#endif

static char uid_default[] = {"DECW$SYSTEM_DEFAULTS:.UID"};

static MrmOsOpenParam os_ext_list;
static MrmOsOpenParamPtr os_ext_list_ptr = &os_ext_list;

static MrmRegisterArg regvec[] = {
    {"copy_proc", (caddr_t) copy_proc},
    {"create_proc", (caddr_t) create_proc},
    {"exit_proc", (caddr_t) exit_proc},
    {"help_done_proc", (caddr_t) help_done_proc},
    {"main_help_proc", (caddr_t) main_help_proc},
    {"message_done_proc", (caddr_t) message_done_proc},
    {"mode_proc", (caddr_t) mode_proc},
    {"on_context_activate_proc", (caddr_t) on_context_activate_proc},
    {"paste_proc", (caddr_t) paste_proc},
    {"restore_settings_proc", (caddr_t) restore_settings_proc},
    {"save_settings_proc", (caddr_t) save_settings_proc},
    {"undo_proc", (caddr_t) undo_proc}
};

/*
**++
**  ROUTINE NAME:
**	DwtMyCalc
**
**  FUNCTIONAL DESCRIPTION:
**	Public entry points for UNIX
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
CalcWidget DwtMyCalc(parent, name, x, y, width, height)
    Widget parent;
    char *name;
    int x, y, width, height;
{
    CalcWidget mw;
    MrmHierarchy calc_hierarchy;	/* DRM database hierarchy id  */
    MrmType *dummy_class;
    Arg arglist[5];
    int argCount = 0;
    MrmCount regnum;
    char *file_array[1];
    FILE *fp;

#ifdef VMS
    file_array[0] = uid_filespec;
    os_ext_list.nam_flg.related_nam = 0;
#else

    /* get_uid_filename looks up the environment variable UIDDIR, and attempts
     * to find a uid file with the correct name in it. */
    char *def_name, *uiddir, *uid_name;
    int uiddir_len, uid_fd;

    /* untested on ultrix */
    os_ext_list.nam_flg.clobber_flg = TRUE;

    file_array[0] =
      XtMalloc(strlen("/usr/lib/X11/uid/.uid") + strlen(CLASS_NAME) + 1);

/*
    sprintf (file_array [0], "/usr/lib/X11/uid/%s.uid", CLASS_NAME);
*/
    sprintf(file_array[0], "%s", CLASS_NAME);

    if ((uiddir = (char *) getenv("UIDDIR")) != NULL) {
	uiddir_len = strlen(uiddir);
	if ((def_name = rindex(file_array[0], '/')) == NULL) {
	    def_name = file_array[0];
	} else
	    def_name++;
	uiddir_len += strlen(def_name) + 2; /* 1 for '/', 1 for '\0' */
	uid_name = (char *) XtMalloc(uiddir_len * sizeof(char));
	strcpy(uid_name, uiddir);
	strcat(uid_name, "/");
	strcat(uid_name, def_name);
	if (access(uid_name, R_OK) != 0) {
	    XtFree(uid_name);
	} else {
	    XtFree(file_array[0]);
	    file_array[0] = uid_name;
	}
    }
#endif

    os_ext_list.version = MrmOsOpenParamVersion;
    os_ext_list.default_fname = (char *) uid_default;

    /* Define the DRM "hierarchy" */
#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
    if (MrmOpenHierarchyPerDisplay(
      XtDisplay(parent),				/* display */
      1,				/* number of files */
      file_array, 			/* files */
      &os_ext_list_ptr, 		/* os_ext_list (null) */
      &calc_hierarchy)			/* ptr to returned id */
      !=MrmSUCCESS)
	calc_serror(NoHierarchy, FATAL);
#else
    if (MrmOpenHierarchy(1, 		/* number of files */
      file_array, 			/* files */
      &os_ext_list_ptr, 		/* os_ext_list (null) */
      &calc_hierarchy)			/* ptr to returned id */
      !=MrmSUCCESS)
	calc_serror(NoHierarchy, FATAL);
#endif

    regnum = sizeof(regvec) / sizeof(MrmRegisterArg);
    MrmRegisterNames(regvec, regnum);

    if (MrmFetchWidget(calc_hierarchy, "calc_widget", parent, &mw,
      &dummy_class) != MrmSUCCESS) {
	calc_serror(NoMain, FATAL);
    }

    XtSetArg(arglist[argCount], XmNx, x);
    argCount++;
    XtSetArg(arglist[argCount], XmNy, y);
    argCount++;
    XtSetArg(arglist[argCount], XmNheight, height);
    argCount++;
    XtSetArg(arglist[argCount], XmNwidth, width);
    argCount++;
    XtSetValues((Widget) mw, arglist, argCount);

    CalcHierarchy(mw) = calc_hierarchy;
    return (mw);
}

/*
**++
**  ROUTINE NAME:
**	DwtMyCalcCreate
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
CalcWidget DwtMyCalcCreate(parent, name, args, argCount)
    Widget parent;
    char *name;
    ArgList args;
    int argCount;
{

    return ((CalcWidget) XtCreateWidget(name, (WidgetClass) calcwidgetclass, parent, args,
      argCount));

}

/*
**++
**  ROUTINE NAME:
**	create_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static unsigned int create_proc(w, tag, reason)
    Widget *w;
    caddr_t *tag;
    unsigned int *reason;
{
    int widget_num = (int) *tag;
    int mem_num;
    CalcWidget mw;
    XtTranslations trans;
    Pixel fgpixel, bgpixel;
    XmFontList FontList;
    XFontStruct *FontInfo;

    mw = calc_FindMainWidget(w);

    switch (widget_num) {
	case k_pull_down_undo_button:
	    PDUndoButtonWid(mw) = (XmPushButtonWidget) w;
	    XtSetSensitive((Widget) w, FALSE);
	    break;

	case k_pop_up_undo_button:
	    PUUndoButtonWid(mw) = (XmPushButtonWidget) w;
	    XtSetSensitive((Widget) w, FALSE);
	    break;

	case k_menu_bar:
	    TopMenuBar(mw) = (Widget) w;
	    break;
    }
}

/*
**++
**  ROUTINE NAME:
**	logo_repaint
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void logo_repaint(w)
    Widget w;
{
    CalcWidget mw = (CalcWidget) w;
    Display *dpy;
    XGCValues gcv;
    Pixel fgpixel, bgpixel;
    unsigned long ScreenNumber;

    if ((LogoWindow(mw).x + logo_width) >= Memory(mw).x)
	return;

    if ((LogoWindow(mw).y + logo_height) >= Mems(mw)[0].y)
	return;

    dpy = Dpy(mw);
    ScreenNumber = XDefaultScreen(dpy);

    if (LogoGC(mw) == 0)
    {
	gcv.foreground = Background(mw);
	gcv.background = Foreground(mw);

	LogoGC(mw) =
	  XCreateGC(dpy, XtWindow(mw), GCForeground | GCBackground, &gcv);

	LogoPixmap(mw) = XCreateBitmapFromData(dpy, XtWindow(mw), logo_bits,
	  logo_width, logo_height);
	if (LogoPixmap(mw) == 0)
	    calc_display_message(NoBellPixmap, ERROR);
    }

    XCopyPlane(Dpy(mw), LogoPixmap(mw), XtWindow(mw), LogoGC(mw), 0, 0,
      logo_width, logo_height, LogoWindow(mw).x, LogoWindow(mw).y, 1);
}

/*
**++
**  ROUTINE NAME:
**	copy_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static unsigned int copy_proc(w, tag, cb, name, parent)
    Widget *w;
    caddr_t *tag;
    XmAnyCallbackStruct *cb;
    String name;
    Widget parent;
{
    CalcWidget mw;
    XKeyEvent *event = (XKeyEvent *) cb->event;
    int status;
    unsigned int id, length, data_id;
    char input_buffer[80];
    XmString clip_label;

    mw = calc_FindMainWidget(w);

    if (DisplaySelected(mw))
	strcpy(input_buffer, BufferCopy(mw));
    else {
	getnumstring(mw, MemValue(mw));
	strcpy(input_buffer, Numstr(mw));
    }

    length = strlen(input_buffer);

    clip_label = XmStringCreate(AppName(mw), XmSTRING_DEFAULT_CHARSET);
    status = XmClipboardStartCopy(Dpy(mw), Win(mw), clip_label, event->time,
      mw, 0, &id);
    XmStringFree(clip_label);
    if (status != ClipboardSuccess)
	calc_display_message(NoBeginClip, INFORMATION);
    else {
	status = XmClipboardCopy(Dpy(mw), Win(mw), id, "STRING", input_buffer,
	  length, 0, &data_id);
	if (status != ClipboardSuccess)
	    calc_display_message(NoClipCopy, INFORMATION);
	status = XmClipboardEndCopy(Dpy(mw), Win(mw), id);
	if (status != ClipboardSuccess)
	    calc_display_message(NoEndClip, INFORMATION);
    }

}

/*
**++
**  ROUTINE NAME:
**	paste_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static unsigned int paste_proc(w, tag, reason)
    Widget *w;
    Opaque tag;
    XmAnyCallbackStruct *reason;
{
    char *str, output_buffer[80];
    int i, id, length, status, char_set, dont_care;
    CalcWidget mw;
    XEvent *event;
    XmString comp_str;
    XmStringContext context;

    mw = calc_FindMainWidget(w);
    event = reason->event;

    status = XmClipboardLock(Dpy(mw), Win(mw));
    if (status != ClipboardSuccess)
	calc_display_message(NoBeginClip, INFORMATION);
    else {
	status = XmClipboardRetrieve(Dpy(mw), Win(mw), "STRING", output_buffer,
	  80, &length, &id);
	if (status != ClipboardSuccess)
	    calc_display_message(NoClipCopy, INFORMATION);
	status = XmClipboardUnlock(Dpy(mw), Win(mw), 1);
	if (status != ClipboardSuccess)
	    calc_display_message(NoEndClip, INFORMATION);
    }

    if (status == ClipboardSuccess) {
	save_state(mw);
	for (i = 0; i < length; i++) {
	    send_type_char(mw, event, output_buffer[i]);
	}
    }
}

/*
**++
**  ROUTINE NAME:
**	mode_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static unsigned int mode_proc(w, tag, reason)
    Widget *w;
    caddr_t *tag;
    unsigned int *reason;
{
    CalcWidget mw;
    int new_mode = (int) *tag;

    mw = calc_FindMainWidget(w);

    if (Mode(mw) != new_mode) {
	flush_buffer(mw);
	Mode(mw) = new_mode;
	switch (new_mode) {
	    case k_decimal:
		strcpy(OutputFormatStr(mw), "%.15lf");
		strcpy(InputFormatStr(mw), "%lf");
		break;
	    case k_hexadecimal:
		strcpy(OutputFormatStr(mw), "%X");
		strcpy(InputFormatStr(mw), "%X");
		break;
	    case k_octal:
		strcpy(OutputFormatStr(mw), "%o");
		strcpy(InputFormatStr(mw), "%o");
		break;
	    case k_binary:
		strcpy(OutputFormatStr(mw), "%.15lf");
		strcpy(InputFormatStr(mw), "%lf");
		break;
	    default:
		strcpy(OutputFormatStr(mw), "%.15lf");
		strcpy(InputFormatStr(mw), "%lf");
		break;
	}  /* switch */

	switch_modes(mw);
    }
}

/*
**++
**  ROUTINE NAME:
**	undo_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static unsigned int undo_proc(w, tag, reason)
    Widget *w;
    caddr_t *tag;
    unsigned int *reason;
{
    CalcWidget mw;

    mw = calc_FindMainWidget(w);
    swap_state(mw);
}

/*
**++
**  ROUTINE NAME:
**	switch_modes
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
switch_modes(mw)
    CalcWidget mw;
{
    Arg translation_args[1];

    if (Mode(mw) == k_decimal) {
	HighNo(mw) = (pow(10.0, (double) MAX_BUFF)) - 1;
	Strmax(mw) = MAX_BUFF;
	XtSetArg(translation_args[0], XmNtranslations, "calc_translations");
    } else if (Mode(mw) == k_hexadecimal) {
	HighNo(mw) = pow(2.0, 32.0);
	Strmax(mw) = MAX_HEX_BUFF;
	XtSetArg(translation_args[0], XmNtranslations,
	  "calc_hex_translations");
    } else {
	HighNo(mw) = pow(2.0, 32.0);
	Strmax(mw) = MAX_OCT_BUFF;
	XtSetArg(translation_args[0], XmNtranslations,
	  "calc_oct_translations");
    }

    XFillRectangle
	(Dpy(mw), Win(mw), CalcClearGC(mw), 0, 0, Width(mw), Height(mw));

    Repaint(mw);

    getnumstring(mw, NumStack(mw)[NumStackCount(mw) - 1]);
    display_in_window(mw, Numstr(mw), &Disp(mw));
    getnumstring(mw, MemValue(mw));
    display_in_window(mw, Numstr(mw), &Memory(mw));

    MrmFetchSetValues(CalcHierarchy(mw), mw, translation_args, 1);
    XtInstallAllAccelerators((Widget) mw, (Widget) mw);
}

/*
**++
**  ROUTINE NAME:
**	save_state
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void save_state(mw)
    CalcWidget mw;
{

    if (!UndoEnabled(mw)) {
	XtSetSensitive((Widget) PDUndoButtonWid(mw), TRUE);
	XtSetSensitive((Widget) PUUndoButtonWid(mw), TRUE);
	UndoEnabled(mw) = TRUE;
    }
    SaveNumStackCount(mw) = NumStackCount(mw);
    SaveNumStack(mw)[0] = NumStack(mw)[0];
    SaveNumStack(mw)[1] = NumStack(mw)[1];

    strcpy(SaveBuffer(mw), Buffer(mw));
    SaveBufferCount(mw) = strlen(SaveBuffer(mw));

    SaveStrmax(mw) = Strmax(mw);
    SaveCurrentOp(mw) = CurrentOp(mw);
    SaveErrorFlag(mw) = ErrorFlag(mw);
    SaveMemValue(mw) = MemValue(mw);
    SaveMemErrorFlag(mw) = MemErrorFlag(mw);

}

/*
**++
**  ROUTINE NAME:
**	swap_state
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void swap_state(mw)
    CalcWidget mw;
{
    int temp_int;
    char temp_str[MAX_BUFF + 2];
    char *str;
    double temp_double;

    temp_int = SaveNumStackCount(mw);
    SaveNumStackCount(mw) = NumStackCount(mw);
    NumStackCount(mw) = temp_int;

    temp_double = SaveNumStack(mw)[0];
    SaveNumStack(mw)[0] = NumStack(mw)[0];
    NumStack(mw)[0] = temp_double;

    temp_double = SaveNumStack(mw)[1];
    SaveNumStack(mw)[1] = NumStack(mw)[1];
    NumStack(mw)[1] = temp_double;

    strcpy(temp_str, SaveBuffer(mw));

    strcpy(SaveBuffer(mw), BufferCopy(mw));
    strcpy(Buffer(mw), temp_str);

    temp_int = SaveBufferCount(mw);
    SaveBufferCount(mw) = strlen(SaveBuffer(mw));
    BufferCount(mw) = temp_int;

    temp_int = SaveStrmax(mw);
    SaveStrmax(mw) = Strmax(mw);
    Strmax(mw) = temp_int;

    temp_int = SaveCurrentOp(mw);
    SaveCurrentOp(mw) = CurrentOp(mw);
    CurrentOp(mw) = temp_int;

    temp_int = SaveErrorFlag(mw);
    SaveErrorFlag(mw) = ErrorFlag(mw);
    ErrorFlag(mw) = temp_int;

    temp_double = SaveMemValue(mw);
    SaveMemValue(mw) = MemValue(mw);
    MemValue(mw) = temp_double;

    temp_int = SaveMemErrorFlag(mw);
    SaveMemErrorFlag(mw) = MemErrorFlag(mw);
    MemErrorFlag(mw) = temp_int;

    if (ErrorFlag(mw))
	error(mw);
    else {
	if (BufferCount(mw) == 0)
	    display_in_window(mw, ZeroPoint(mw), &Disp(mw));
	else
	    display_in_window(mw, Buffer(mw), &Disp(mw));
    }

    if (MemErrorFlag(mw))
	mem_error(mw);
    else
	mem_value_dis(mw);
}

/*
**++
**  ROUTINE NAME:
**	send_type_char
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void send_type_char(mw, ev, inp)
    CalcWidget mw;
    XEvent *ev;
    char inp;
{
    XEvent local_event;
    char inp_string[20];
    KeyCode inp_keycode;
    KeySym inp_keysym;
    XKeyEvent *event = (XKeyEvent *) ev;
    unsigned int button_state;

    button_state = NULL;
    switch (inp) {
	case ' ':
	    strcpy(inp_string, "space");
	    break;
	case '!':
	    strcpy(inp_string, "exclam");
	    button_state = ShiftMask;
	    break;
	case '"':
	    strcpy(inp_string, "quotedbl");
	    button_state = ShiftMask;
	    break;
	case '#':
	    strcpy(inp_string, "numbersign");
	    button_state = ShiftMask;
	    break;
	case '$':
	    strcpy(inp_string, "dollar");
	    button_state = ShiftMask;
	    break;
	case '%':
	    strcpy(inp_string, "percent");
	    button_state = ShiftMask;
	    break;
	case '&':
	    strcpy(inp_string, "ampersand");
	    button_state = ShiftMask;
	    break;
	case '(':
	    strcpy(inp_string, "parenleft");
	    button_state = ShiftMask;
	    break;
	case ')':
	    strcpy(inp_string, "parenright");
	    button_state = ShiftMask;
	    break;
	case '*':
	    strcpy(inp_string, "asterisk");
	    button_state = ShiftMask;
	    break;
	case '+':
	    strcpy(inp_string, "plus");
	    button_state = ShiftMask;
	    break;
	case ',':
	    {
		if (inp == ChrPoint(mw))
		    strcpy(inp_string, "period");
		else
		    strcpy(inp_string, "comma");
		break;
	    }
	case '-':
	    strcpy(inp_string, "minus");
	    break;
	case '.':
	    strcpy(inp_string, "period");
	    break;
	case '/':
	    strcpy(inp_string, "slash");
	    break;
	case ':':
	    strcpy(inp_string, "semicolon");
	    button_state = ShiftMask;
	    break;
	case ';':
	    strcpy(inp_string, "semicolon");
	    break;
	case '<':
	    strcpy(inp_string, "less");
	    break;
	case '=':
	    strcpy(inp_string, "KP_Enter");
	    break;
	case '>':
	    strcpy(inp_string, "greater");
	    button_state = ShiftMask;
	    break;
	case '?':
	    strcpy(inp_string, "question");
	    button_state = ShiftMask;
	    break;
	case '@':
	    strcpy(inp_string, "at");
	    button_state = ShiftMask;
	    break;
	case '[':
	    strcpy(inp_string, "bracketleft");
	    break;
	case ']':
	    strcpy(inp_string, "bracketright");
	    break;
	case '^':
	    strcpy(inp_string, "asciicircum");
	    button_state = ShiftMask;
	    break;
	case '_':
	    strcpy(inp_string, "underscore");
	    button_state = ShiftMask;
	    break;
	case '`':
	    strcpy(inp_string, "quoteleft");
	    break;
	case '{':
	    strcpy(inp_string, "braceleft");
	    button_state = ShiftMask;
	    break;
	case '|':
	    strcpy(inp_string, "bar");
	    break;
	case '}':
	    strcpy(inp_string, "braceright");
	    button_state = ShiftMask;
	    break;
	case '~':
	    strcpy(inp_string, "asciitilde");
	    button_state = ShiftMask;
	    break;
	default:
	    sprintf(inp_string, "%c", inp);
	    break;
    }
    inp_keysym = XStringToKeysym(inp_string);
    inp_keycode = XKeysymToKeycode(Dpy(mw), inp_keysym);

    local_event.xkey.serial = event->serial; /* # of last request processed by
					 * server */
    local_event.xkey.display = event->display;
					/* Display the event was read from */
    local_event.xkey.root = event->root;  /* root window that the event occured
					 * on */
    local_event.xkey.subwindow = event->subwindow;
					/* child window */
    local_event.xkey.time = event->time;	/* milliseconds */
    local_event.xkey.x = event->x;
    local_event.xkey.y = event->y;
					/* pointer x, y coordinates in event
					 * window */
    local_event.xkey.x_root = event->x_root;
    local_event.xkey.y_root = event->y_root; /* coordinates relative to root */
    local_event.xkey.same_screen = event->same_screen;
					/* same screen flag */

    local_event.xkey.type = KeyPress;	/* of event */
    local_event.xkey.send_event = FALSE;  /* true if this came from a SendEvent
					 * request */
    local_event.xkey.state = button_state;	/* key or button mask */
    local_event.xkey.window = XtWindow(mw);    /* "event" window it is reported
					 * relative to */
    local_event.xkey.keycode = inp_keycode;	/* detail (key code) */

    XtDispatchEvent(&local_event);

    local_event.xkey.type = KeyRelease;	/* of event */
    XtDispatchEvent(&local_event);
}

/*
**++
**  ROUTINE NAME:
**	exit_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static unsigned int exit_proc(w, tag, reason)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
{
    CalcWidget mw;

#ifndef NO_HYPERHELP
    if (hyper_help_context)
	DXmHelpSystemClose(hyper_help_context, calc_serror,
	  "Help Close Error");
#endif
    mw = calc_FindMainWidget(w);
    XCloseDisplay(Dpy(mw));
    exit(OK_STATUS);
}

/*
**++
**  ROUTINE NAME:
**	save_settings_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static unsigned int save_settings_proc(w, tag, reason)
    Widget *w;
    caddr_t *tag;
    unsigned int *reason;
{
    CalcWidget mw;
    XrmValue put_value;
    char resource_value[20];
    char resource_name[256];
    Position x, y;
    Dimension width, height;

    mw = calc_FindMainWidget(w);

    if (UserDatabase(mw) == NULL)
	UserDatabase(mw) = XrmGetFileDatabase(DefaultsName(mw));

    width = XtWidth(toplevel);
    height = XtHeight(toplevel);
    XtTranslateCoords(toplevel, 0, 0, &x, &y);
    x = x - XtBorderWidth(toplevel);
    y = y - XtBorderWidth(toplevel);

    sprintf(resource_value, "%dx%d+%d+%d", width, height, x, y);

    put_value. addr = resource_value;
    put_value. size = strlen(resource_value) + 1;

    sprintf(resource_name, "Calc.%s", xrm_geometry);

    XrmPutResource(&UserDatabase(mw), resource_name, XtRString, &put_value);

    XrmPutFileDatabase(UserDatabase(mw), DefaultsName(mw));
}

/*
**++
**  ROUTINE NAME:
**	restore_settings_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static unsigned int restore_settings_proc(w, tag, reason)
    Widget *w;
    caddr_t *tag;
    unsigned int *reason;
{
    CalcWidget mw;
    Arg args[10];
    int ac;
    int status, geometry_mask;
    Position x, y;
    Dimension width, height;
    int intx, inty;
    unsigned int intwidth, intheight;
    char *get_type;
    XrmValue get_value;
    char resource_name[256];
    char resource_class[256];

    mw = calc_FindMainWidget(w);
    if (SystemDatabase(mw) == NULL)
	SystemDatabase(mw) = XrmGetFileDatabase(SystemDefaultsName(mw));
    intx = 75;
    inty = 20;
    intwidth = 255;
    intheight = 335;
    sprintf(resource_name, "Calc.%s", xrm_geometry);
    sprintf(resource_class, "Calc.%s", xrc_geometry);

    status = XrmGetResource(SystemDatabase(mw),
					/* Database. */
      resource_name, 			/* Resource's ASCIZ name. */
      resource_class, 			/* Resource's ASCIZ class. */
      &get_type, 			/* Resource's type (out). */
      &get_value);			/* Address to return value. */
    if (status != NULL) {
	geometry_mask =
	  XParseGeometry(get_value. addr, &intx, &inty, &intwidth, &intheight);
    }

    if (intx < 0)
	intx =
	  XWidthOfScreen(XDefaultScreenOfDisplay(Dpy(mw))) + intx - intwidth;
    if (inty < 0)
	inty =
	  XHeightOfScreen(XDefaultScreenOfDisplay(Dpy(mw))) + inty - intheight;
    if (intwidth < 100)
	intwidth = 100;
    if (intheight < 100)
	intheight = 100;

    x = (Position) intx;
    y = (Position) inty;
    width = (Dimension) intwidth;
    height = (Dimension) intheight;

    ac = 0;
    XtSetArg(args[ac], XmNx, x);
    ac++;
    XtSetArg(args[ac], XmNy, y);
    ac++;
    XtSetValues(XtParent(mw), args, ac);
    ac = 0;
    XtSetArg(args[ac], XmNwidth, width);
    ac++;
    XtSetArg(args[ac], XmNheight, height);
    ac++;
    XtSetValues((Widget) mw, args, ac);
}

/*
**++
**  ROUTINE NAME:
**	message_done_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static unsigned int message_done_proc(w, tag, reason)
    Widget *w;
    caddr_t *tag;
    unsigned int *reason;
{
    CalcWidget mw;

    mw = calc_FindMainWidget(w);

    XtUnmanageChild((Widget) MessageWid(mw));
}

/*
**++
**  ROUTINE NAME:
**	main_help_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static unsigned int main_help_proc(w, tag, reason)
    Widget *w;
    caddr_t *tag;
    unsigned int *reason;
{
    CalcWidget mw;
    XmString Frame;

    mw = calc_FindMainWidget(w);

    Frame = (XmString) tag;
    calc_DisplayHelp(mw, Frame);

}

/*
**++
**  ROUTINE NAME:
**	on_context_activate_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void on_context_activate_proc(w, tag, reason, name, parent)
    Widget w;
    caddr_t *tag;
    unsigned int *reason;
    String name;
    Widget parent;
{
    /* Change to use the DXm's Help On context utilities. */
    DXmHelpOnContext(toplevel, FALSE);
}

/*
**++
**  ROUTINE NAME:
**	tracking_help
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static void tracking_help()
{
    Widget track_widget;
    Cursor cursor;

    track_widget = NULL;

    cursor = XCreateFontCursor(XtDisplay(toplevel), XC_question_arrow);

    track_widget = XmTrackingLocate(toplevel, cursor, FALSE);

    if (track_widget != 0) {
	if (XtHasCallbacks(track_widget, XmNhelpCallback) == XtCallbackHasSome)
	    XtCallCallbacks(track_widget, XmNhelpCallback, NULL);
    } else {
	XtUngrabPointer(toplevel, CurrentTime);
    }
}

/*
**++
**  ROUTINE NAME:
**	help_done_proc
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
static unsigned int help_done_proc(w, tag, reason)
    Widget *w;
    caddr_t *tag;
    unsigned int *reason;
{
    CalcWidget mw;

    mw = calc_FindMainWidget(w);
    XtUnmanageChild(HelpWidget(mw));
}

/*
**++
**  ROUTINE NAME:
**	calc_DisplayHelp
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void calc_DisplayHelp(mw, Frame)
    CalcWidget mw;
    XmString Frame;
{
#ifndef NO_HYPERHELP
    /* Hyperhelp */
    if (!hyper_help_context) {

	WorkCursorDisplay();
	DXmHelpSystemOpen(&hyper_help_context, toplevel, CALC_HELP,
	  calc_display_message, NoHelpWidget);
	WorkCursorRemove();
    }

    WorkCursorDisplay();
    DXmHelpSystemDisplay(hyper_help_context, CALC_HELP, "Topic", Frame,
      calc_display_message, NoHelpWidget);
    WorkCursorRemove();
#else
    int ac;
    Arg args[5];
    XmString help_lib_name_cs;
    MrmType *dummy_class;
    Widget help_widget;

    ac = 0;
    help_lib_name_cs = XmStringCreate("calc", XmSTRING_DEFAULT_CHARSET);
    XtSetArg(args[ac], DXmNlibrarySpec, help_lib_name_cs);
    ac++;

    if (HelpWidget(mw) == 0) {
	WorkCursorDisplay();
	if (MrmFetchWidgetOverride(CalcHierarchy(mw),
					/* hierarchy id                 */
	  "main_help", 			/* Index of widget to fetch     */
	  mw, 				/* Parent of widget             */
	  NULL, 			/* Override name                */
	  args, 			/* Override args                */
	  ac, 				/* Override args count          */
	  &help_widget, 		/* Widget id                    */
	  &dummy_class) != MrmSUCCESS)	/* Widget Class                 */
	  {
	    calc_display_message(NoHelpWidget, WARNING);
	}

	HelpWidget(mw) = help_widget;
	WorkCursorRemove();
    }

    if (HelpWidget(mw) != 0) {
	ac = 0;
	XtSetArg(args[ac], DXmNfirstTopic, Frame);
	ac++;
	XtSetValues(HelpWidget(mw), args, ac);
	XtManageChild(HelpWidget(mw));
    }
#endif
}

/*
**++
**  ROUTINE NAME:
**	calc_serror
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void calc_serror(str, level)
    char *str;
    int level;
{
    printf("%s \n", str);
    if (level == FATAL)
	exit(ERROR_STATUS);
}

/*
**++
**  ROUTINE NAME:
**	calc_display_message
**
**  FUNCTIONAL DESCRIPTION:
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void calc_display_message(str, level)
    char *str;
    int level;
{
    printf("%s \n", str);
    if (level == FATAL)
	exit(ERROR_STATUS);
}

/*
**++
**  ROUTINE NAME:
**	WorkCursorCreate
**
**  FUNCTIONAL DESCRIPTION:
**	This routine creates a "wait" cursor
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
Cursor WorkCursorCreate(wid)
    Widget wid;				/* Widget to be used to create the
					 * cursor.  The fields used are the
					 * display and the colormap.  Any
					 * widget with the same display and
					 * colormap can be used later with this
					 * cursor */
{
/* Create a wait cursor */
    Cursor cursor;
    Font font;
    XColor fcolor, bcolor, dummy;
    int status;

    font = XLoadFont(XtDisplay(wid), "DECw$Cursor");
    status = XAllocNamedColor(XtDisplay(wid), wid->core.colormap, "Black",
      &fcolor, &dummy);
    status = XAllocNamedColor(XtDisplay(wid), wid->core.colormap, "White",
      &bcolor, &dummy);
    cursor = XCreateGlyphCursor(XtDisplay(wid), font, font, decw$c_wait_cursor,
      decw$c_wait_cursor + 1, &fcolor, &bcolor);
    return cursor;
}

/*
**++
**  ROUTINE NAME:
**	WorkCursorDisplay
**
**  FUNCTIONAL DESCRIPTION:
**	This routine displays a watch cursor in the Clock
**	window.
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void WorkCursorDisplay()
{

    /* Display the watch cursor in the clock window */
    if (watch_cursor == NULL) {
	watch_cursor = DXmCreateCursor(toplevel, decw$c_wait_cursor);
    }

    XDefineCursor(XtDisplay(toplevel), XtWindow(toplevel), watch_cursor);

/* Use XtAddGrab to enable toolkit filtering of events */
    XtAddGrab(toplevel, TRUE, FALSE);
    XFlush(XtDisplay(toplevel));
}

/*
**++
**  ROUTINE NAME:
**	WorkCursorRemove
**
**  FUNCTIONAL DESCRIPTION:
**	Return to the normal cursor in both windows
**
**  FORMAL PARAMETERS:
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
void WorkCursorRemove()
{
    XUndefineCursor(XtDisplay(toplevel), XtWindow(toplevel));

    /* Use XtRemoveGrab to disable toolkit filtering of events */
    XtRemoveGrab(toplevel);
}

#ifdef DWTVMS
/*
**++
**  ROUTINE NAME:
**	*VMSDescToNull (desc)
**
**  FUNCTIONAL DESCRIPTION:
**  	Converts a string descriptor to a null terminated string
**
**  FORMAL PARAMETERS:
**	desc - the descriptor
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
#define ReadDesc(source_desc,addr,len)	\
    LIB$ANALYZE_SDESC(source_desc,&len,&addr)

char *VMSDescToNull(desc)
    struct dsc$descriptor_s *desc;
{
    char *nullterm_string;
    char *temp_string;
    unsigned short temp_length;

    if (desc->dsc$b_class <= DSC$K_CLASS_D) {
	temp_length = desc->dsc$w_length;
	temp_string = desc->dsc$a_pointer;
    } else
	ReadDesc(desc, temp_string, temp_length);

    nullterm_string = (char *) XtMalloc(temp_length + 1);

    if (temp_length != 0)
	mybcopy(temp_string, nullterm_string, temp_length);

    /* Make it null-terminated */
    *(nullterm_string + temp_length) = '\0';

    return (nullterm_string);
}
#endif

#ifdef DWTVMS
/*
**++
**  ROUTINE NAME: mybcopy
**
**  FUNCTIONAL DESCRIPTION:
**      Replaces the C bcopy routine. Copies an area of memory to another.
**
**  FORMAL PARAMETERS:
**      b1 - buffer to copy from.
**      b2 - buffer to copy into.
**      n  - number of bytes to copy.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  FUNCTION VALUE:
**
**  SIDE EFFECTS:
**--
**/
mybcopy(b1, b2, n)
    char *b1;
    char *b2;
    int n;
{
    int i;

    for (i = 0; i < n; i++) {
        *b2 = *b1;
        b1++;
        b2++;
    }
}
#endif
