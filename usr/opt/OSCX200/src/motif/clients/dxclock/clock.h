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
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxclock/clock.h,v 1.1.7.2 1993/09/09 17:05:49 Susan_Ng Exp $";
#endif
#endif
/*
static char *BuildSystemHeader =
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxclock/clock.h,v 1.1.7.2 1993/09/09 17:05:49 Susan_Ng Exp $";
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
**	clock.h
**
**  FACILITY:
**      OOTB Clock
**
**  ABSTRACT:
**	Defines and typedefs for OOTB clock
**
**  AUTHORS:
**      Dennis McEvoy, Neal Finnegan
**
**  RELEASER:
**
**  CREATION DATE:     
**	6-OCT-1987
**
**  MODIFICATION HISTORY:
**
**    Version:
**
**	Nov 1990 - V3.0 	(ASP) - Motif conversion.
**      May 1992 - V3.1         (SP)  - I18N and maintenance changes.
**--
**/
#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/CompositeP.h>
#include <X11/ConstrainP.h>
#include <X11/ShellP.h>
#include <X11/VendorP.h>
#include <X11/RectObjP.h>
#include <Xm/Xm.h>
#include <Xm/BulletinBP.h>
#include <Xm/CutPaste.h>
#include <Xm/DrawingAP.h>
#include <Xm/DrawingA.h>
#include <Xm/FileSB.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumnP.h>
#include <Xm/Scale.h>
#include <Xm/TextP.h>
#include <Xm/ToggleB.h>
#include <DXm/DXmHelpB.h>
#include <DXm/DXmCSText.h>
#include <Mrm/MrmAppl.h>

#ifndef NO_XNLS
#ifdef VMS
#include "xnl$def.h"
#else
#include <xnl_def.h>
#endif
#endif

#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

#ifndef NO_AUDIO
#include "diva_def.h"
#endif

/* clock window widget data and class defintions */

#define CLOCK
#define ClockClassName "Clock"

/* jv - added these which were defined in DwtWidget.h but not in MrmWidget.h */
#ifndef external
#ifdef VAXC
#define external globalref
#else
#define external extern
#endif
#endif

typedef struct {
	char	label [20];
	} DateStr;

typedef	struct {
    XmDrawingAreaWidget	analog_part;
    XmDrawingAreaWidget	digital_part;
    XmDrawingAreaWidget	date_part;
    XmDrawingAreaWidget	alarm_bell;
    Widget		top_menu_bar;
    XmRowColumnWidget		menu;
    XmBulletinBoardWidget	settings, alarm_settings;
    XmToggleButtonWidget	analog_wid, digital_wid, date_wid, alarm_wid, repeat_wid;
    XmToggleButtonWidget	menubar_wid, military_time_wid, am_wid, pm_wid;
    XmToggleButtonWidget	bell_on_wid;
    XmToggleButtonWidget	audio_on_wid; 
    XmPushButtonWidget		speaker_on_wid, headphone_on_wid;
    XmScaleWidget		audio_volume_wid;
    DXmCSTextWidget		audio_fname_text_wid;
    XmFileSelectionBoxWidget	audio_fsbox_wid;
    XmPushButtonWidget		audio_fname_button_wid, audio_play_button_wid, audio_stop_button_wid;
    XmRowColumnWidget		audio_output_menu_wid;
    XmTextWidget		hr_wid, min_wid;
    DXmCSTextWidget		alarm_mes_wid;
    XmPushButtonWidget	settings_ok_wid;
    XmMessageBoxWidget	message_wid;
    XmMessageBoxWidget	error_message_wid;
    Widget		help_widget;
    XrmDatabase		user_database;
    XrmDatabase		system_database;
    XrmDatabase		merged_database;
#ifndef NO_XNLS
    Locale		language_id;
#endif
    int			is_us_locale;
    int			is_12hr_locale;
/*
    char		* defaults_name;
    char		* system_defaults_name;
*/
    char		day_ptr [100],
			num_ptr [100],
			month_ptr [100];
    char		date_ptr [100],
			dig_ptr [100],
			longest_date [100],
			longest_time [100];
    int			current_day;
    DateStr		months [NUM_MONTHS];
    DateStr		days [NUM_DAYS];
    DateStr		day_numbers [DAY_NUMERAL];
    DateStr		hour_numbers [HOUR_NUMERAL];
    DateStr		min_numbers [MIN_NUMERAL];
    DateStr		day_suffix;
    DateStr		hour_suffix;
    DateStr		min_suffix;
    DateStr		amstr;
    DateStr		pmstr;
    int			digital_present;
    int			analog_present;
    int			date_present;
    int			military_time;
    int			menubar_present;
    Dimension		no_mb_minwidth;
    Dimension		no_mb_minheight;
    Dimension		minwidth;
    Dimension		minheight;
    int			clock_type;
    int			alarm_on;
    int			repeat_on;
    char		alarm_hr [4];
    char		alarm_min [4];
    int			alarm_pm;
    char		alarm_mes [800];
    int			audio_capable;
    int			bell_on;
#ifndef NO_AUDIO
    diva_hardware_type	audio_hardware;
    diva_channel	audio_channel;
#endif
    int			audio_on;
    int			speaker_on;
    int			headphone_on;
    int			audio_volume;
    int 		audio_async_active;
    int			audio_state;
    char		audio_fname [256];
    char		audio_dirmask [256];
    float		date_perc_dn;
    float		date_perc_da;
    float		analog_perc_da;
    float		analog_perc_na;
    float		analog_perc_dna;
    int			date_int_dn;
    int			date_int_da;
    int			analog_int_da;
    int			analog_int_na;
    int			analog_int_dna;
    Display		*dpy;
    GC			analog_gc;
    GC			digital_gc;
    GC			date_gc;
    GC			analog_clear_gc;
    GC			digital_clear_gc;
    GC			date_clear_gc;
    GC			bell_gc;
    Pixmap		bell_pixmap;
    int			hand_width;
    struct		tm last_tm;
    int			digital_font_height;
    XFontStruct		*digital_font;
    XFontStruct		*date_font;
    XmFontList		digital_fontlist;
    XmFontList		date_fontlist;
    XmFontList		digital_fontlists[10];
    XmFontList		date_fontlists[10];
    int			date_font_height;
    MrmHierarchy		clock_hierarchy;
#ifdef NO_LONGER_USED
    MrmResourceContextPtr	context_ptr;
    MrmResourceContextPtr	array_context_ptr;
#endif
    XFontStruct		*fonts [10];
    int			font_heights [10];
    int			date_font_widths [10];
    int			digital_font_widths [10];
    int			num_fonts;
    char		*font_family;
    char		*date_font_family;
    char		*digital_font_family;
    XmString		CSdate_format;
    XmString		CStime_format;
    XmString		CSmilitary_format;
    char		*language;
    char		*date_format;
    char		*time_format;
    char		*military_format;
    char		*alarm_hour;
    char		*alarm_minute;
    char		*alarm_message;
    char		*audio_filename;
    char		*audio_dir_mask;
    XmFontList		clock_font;

    } ClockPart;


/* now define the actual clock widget data structure */

typedef struct 
    {
    CorePart	    core;		/* basic widget */
    CompositePart   composite;		/* composite specific data */
    ConstraintPart  constraint;		/* constraint specific data */
    XmManagerPart   manager;		/* manager specific data */
    XmPrimitivePart   dwtcommon;
    ClockPart	    clock;		/* main specific */
    } ClockWidgetRec, * ClockWidget;

typedef struct 
    {
    int		    mumble;		/* nothing special */
    } ClockClassPart;

typedef struct _ClockClassRec 
    {
    CoreClassPart	core_class;
    CompositeClassPart	composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart  manager_class;
    XmPrimitiveClassPart  primitive_class;
    ClockClassPart	clock_class;
    } ClockClassRec, * ClockWidgetClass;


external ClockClassRec     clockwidgetclassrec;
external ClockWidgetClass  clockwidgetclass;
