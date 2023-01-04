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
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxcalc/calc.h,v 1.1.5.2 1993/04/13 22:46:32 Ronald_Hegli Exp $";
#endif
#endif
/*  Header from VMS source pool
static char *BuildSystemHeader =
  "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxcalc/calc.h,v 1.1.5.2 1993/04/13 22:46:32 Ronald_Hegli Exp $";
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
**	calc.h
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

#include <stdio.h>
#ifdef VMS
#include <descrip.h>
#endif

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
#include <Xm/CutPasteP.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumnP.h>
#include <Xm/TextP.h>
#include <DXm/DXmHelpB.h>
#include <Mrm/MrmAppl.h>
#include <DXm/DECspecific.h>

#ifndef NO_XNLS
#ifdef VMS
#include <xnl$def.h>
#else
#include <xnl_def.h>
#endif
#endif

#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

/* calc window widget data and class definitions */

#define CALC
#define CalcClassName "Calc"

#ifndef external
#ifdef VAXC
#define external globalref
#else
#define external extern
#endif
#endif

typedef struct
{
    short x, y;
    Dimension width, height;
    int margin;
    char label[5];
    char hex_label[5];
    char oct_label[5];
    Boolean inv;
} keyinfo;

typedef struct
{
    short x, y;
    Dimension width, height;
    int margin;
    char *label;
} keyinfo2;

typedef char *key_name;
typedef struct
{
    Widget top_menu_bar;
    XmRowColumnWidget menu;

    XrmDatabase user_database;
    XrmDatabase system_database;
    XrmDatabase merged_database;

    char *defaults_name;
    char *system_defaults_name;

#ifndef NO_XNLS
    Locale language_id;
#endif

    keyinfo mem_bound_box;
    keyinfo key_bound_box;
    keyinfo clear_bound_box;
    keyinfo ops_bound_box;
    keyinfo func_bound_box;
    keyinfo inv_func_bound_box;
    keyinfo disp;
    keyinfo memory;
    keyinfo logo_window;
    keyinfo keys[NUMKEYS];
    keyinfo clears[NUMCLEARS];
    keyinfo ops[NUMOPS];
    keyinfo func[NUMFUNC];
    keyinfo inv_func[NUMINVFUNC];
    keyinfo mems[NUMMEMS];
    XmPushButtonWidget p_d_undo_button_wid;
    XmPushButtonWidget p_u_undo_button_wid;

    Widget help_widget;
    XmMessageBoxWidget message_wid;

    Display *dpy;
    XFontStruct *font;
    XFontStruct *fonts[10];
    XFontStruct *sqrt_font;
    XFontStruct *sqrt_fonts[10];

    XtTranslations translations;

    char error_string[MAX_BUFF + 3];
    char zero_point[MAX_BUFF + 3];
    char app_name[MAX_BUFF + 3];
    char chr_point;
    char chr_zero;

    int sqrt_key;
    int pi_key;

    int font_heights[10];
    int font_widths[10];
    int sqrt_font_heights[10];
    int sqrt_font_widths[10];
    int num_fonts;
    int num_sqrt_fonts;
    int display_font_height;
    int memory_font_height;
    int key_font_height;
    int sqrt_font_height;
    GC calc_gc;
    GC calc_clear_gc;
    GC logo_gc;
    GC sqrt_gc;
    GC sqrt_clear_gc;
    Pixmap logo_pixmap;
    Pixel mem_fgpixel, mem_bgpixel;
    Pixel disp_fgpixel, disp_bgpixel;
    MrmHierarchy calc_hierarchy;

/*    Not supported by Motif. 
   MRMResourceContextPtr	context_ptr;
   MRMResourceContextPtr	array_context_ptr;
   MRMResourceContextPtr	hex_array_context_ptr;
*/

    keyinfo *button_pressed;
    keyinfo *key_pressed;

    int undo_enabled;
    int memory_flag;
    int strmax;
    int display_selected;
    int num_stack_count;
    int buffer_count;
    int current_op;
    char buffer[MAX_BUFF + 3];
    char buffer_copy[MAX_BUFF + 3];
    double num_stack[2];
    int error_flag;
    int mem_error_flag;
    double mem_value;
    char numstr[INT_MAX_STR + 1];
    int mode;
    char output_format_str[10];
    char input_format_str[10];

    int trig_type;
    int invs_enabled;

    int save_memory_flag;
    int save_strmax;
    int save_num_stack_count;
    int save_buffer_count;
    int save_current_op;
    char save_buffer[MAX_BUFF + 3];
    double save_num_stack[2];
    int save_error_flag;
    int save_mem_error_flag;
    double save_mem_value;
    char save_numstr[INT_MAX_STR + 1];

    double high_no;
    double low_no;

    char *key_font_family;
    char *sqrt_font_family;
    char *language;
    char *number_format;
    XmString CSnumber_format;
} CalcPart;

/* now define the actual calc widget data structure */

typedef struct
{
    CorePart		core;		/* basic widget */
    CompositePart	composite;	/* composite specific data */
    ConstraintPart	constraint;	/* constraint specific data */
    XmManagerPart	manager;	/* manager specific data */
    CalcPart		calc;		/* main specific */
} CalcWidgetRec, *CalcWidget;

typedef struct
{
    XmOffsetPtr		calcoffsets;
    int			mumble;		/* nothing special */
} CalcClassPart;

typedef struct _CalcClassRec
{
    CoreClassPart	core_class;
    CompositeClassPart	composite_class;
    ConstraintClassPart	constraint_class;
    XmManagerClassPart	manager_class;
    CalcClassPart	calc_class;
} CalcClassRec, *CalcWidgetClass;
