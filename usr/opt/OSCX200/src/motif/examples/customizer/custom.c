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
static char *rcsid = "@(#)$RCSfile: custom.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 15:41:14 $";
#endif
#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "custom.c";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1993 DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  ALL RIGHTS RESERVED
*  
*******************************************************************************
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xatom.h>
#include <Mrm/MrmAppl.h>     
#include <DXm/DXmColor.h>
#include <DXm/DXmHelpB.h>
#include "customimage.dat"

typedef void (*CmixProc) ();

static void create_cb();
static void button_cb();
static void help_cb();
static void auto_shadow_changed();
static void list_select();
static void cmix_ok();
static void cmix_cancel();
static void my_set_new_color_proc();
static void popup_add_palette_box();
static void add_or_modify_palette();
static void delete_palette();
static void save_palettes();
static void color_button_activate();
static void arrow_arm();
static void arrow_disarm();
static void image_input();
static void UpdateShadowColors();
static void exit_cb();
static void exit_ok();
static void exit_no_save();
static void exit_cancel();
static XtEventHandler text_popup_vis_handler ();

static MrmHierarchy	s_MrmHierarchy;	
static char		*vec[]={"custom.uid"};
static MrmCode		class ;
static MrmRegisterArg	regvec[] = {
	{"create_cb",(XtCallbackProc)create_cb},
	{"button_cb",(XtCallbackProc)button_cb},
	{"auto_shadow_changed",(XtCallbackProc)auto_shadow_changed},
	{"list_select",(XtCallbackProc)list_select},
	{"cmix_ok",(XtCallbackProc)cmix_ok},
	{"cmix_cancel",(XtCallbackProc)cmix_cancel},
	{"save_palette_as",(XtCallbackProc)popup_add_palette_box},
	{"add_or_modify_palette_cb",(XtCallbackProc)add_or_modify_palette},
	{"delete_palette",(XtCallbackProc)delete_palette},
	{"exit_cb",(XtCallbackProc)exit_cb},
	{"exit_ok_cb",(XtCallbackProc)exit_ok},
	{"exit_cancel_cb",(XtCallbackProc)exit_cancel},
	{"exit_no_save_cb",(XtCallbackProc)exit_no_save},
	{"help_cb",(XtCallbackProc)help_cb}
};

static MrmCount	regnum = XtNumber (regvec);

Arg al[20];
MrmType mrmtype;
Pixel *image_cmap;
Cursor menu_cursor;
Display *display;
Screen  *def_screen, **screens;
int def_screen_num, num_screens, def_index;
Colormap *cmaps;
XtAppContext app_context;
Pixmap icon_pixmap;
int num_palettes;
unsigned short **palettes;
char *palette_colors_string, **palette_name_list;
XmString *palette_names;
XImage *image;
XmString no_need_to_save_label, cancel_label, ok_label=NULL;
XmString need_to_save_label, no_label,yes_label=NULL;
Boolean auto_shadow = TRUE;
Boolean need_to_save = FALSE;
Boolean	need_to_write_palette_colors = FALSE;
Boolean pending_grab_release = FALSE;
Boolean text_just_managed = FALSE;
Boolean multiscreen = FALSE;
CmixProc real_set_new_color_proc;
Widget toplevel, mainw, colormix, text_popup, text, picture, list, color_menu;
Widget pict_label, shadow_menu, picture_frame, dummy;
Widget exit_popup, current_image_popup = NULL, help_dialog = NULL;

#define CUST_ICON_17_width 17
#define CUST_ICON_17_height 17
static char CUST_ICON_17_bits[] = {
   0x00, 0x1f, 0x00, 0xc0, 0x75, 0x00, 0xa0, 0x6e, 0x00, 0x50, 0xd5, 0x00,
   0xa8, 0xeb, 0x00, 0x54, 0xd5, 0x00, 0xaa, 0xea, 0x00, 0x76, 0xd5, 0x00,
   0xab, 0x6e, 0x00, 0x55, 0x7b, 0x00, 0xab, 0x6e, 0x00, 0x5d, 0x75, 0x00,
   0xab, 0x6a, 0x00, 0x55, 0x75, 0x00, 0xaa, 0x3a, 0x00, 0x54, 0x1d, 0x00,
   0xf8, 0x0f, 0x00};

#define CUST_ICON_32_width 32
#define CUST_ICON_32_height 32
static char CUST_ICON_32_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x01, 0x00, 0x80, 0xaf, 0x0e,
   0x00, 0xe0, 0x75, 0x1d, 0x00, 0xb8, 0xfa, 0x3a, 0x00, 0x56, 0x77, 0x75,
   0x00, 0xab, 0xaf, 0x6a, 0x00, 0x55, 0x57, 0x75, 0xc0, 0xba, 0xaa, 0xea,
   0x60, 0x7d, 0x55, 0xd5, 0xa0, 0xba, 0xaa, 0xea, 0x58, 0x55, 0x55, 0xf5,
   0xa8, 0xaa, 0xaa, 0x6a, 0xd4, 0x55, 0x55, 0x75, 0xea, 0xab, 0xaa, 0x3a,
   0xd6, 0x55, 0xd5, 0x35, 0xaa, 0xaa, 0x2a, 0x3b, 0x55, 0x55, 0x15, 0x1e,
   0xab, 0xaa, 0x0a, 0x1a, 0x75, 0x55, 0x15, 0x3e, 0xfb, 0xaa, 0x2a, 0x3b,
   0x75, 0x55, 0xd5, 0x35, 0xab, 0xaa, 0xaa, 0x3a, 0x56, 0x55, 0x55, 0x35,
   0xaa, 0xaa, 0xaa, 0x3a, 0x54, 0x55, 0x55, 0x3d, 0xac, 0xaa, 0xaa, 0x1e,
   0x58, 0x55, 0x55, 0x0f, 0xa0, 0xaa, 0xaa, 0x07, 0xc0, 0x55, 0xd5, 0x01,
   0x00, 0xff, 0xff, 0x00, 0x00, 0xfe, 0x3f, 0x00};

#define CUST_ICON_50_width 49
#define CUST_ICON_50_height 49
static char CUST_ICON_50_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x1f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x5c, 0xf5, 0x00, 0x00, 0x00, 0x00, 0xc0,
   0xab, 0xaa, 0x07, 0x00, 0x00, 0x00, 0x70, 0x55, 0x55, 0x1f, 0x00, 0x00,
   0x00, 0xac, 0xaa, 0xab, 0x1e, 0x00, 0x00, 0x80, 0x57, 0xd5, 0x57, 0x3d,
   0x00, 0x00, 0xc0, 0xaa, 0xee, 0xaf, 0x7a, 0x00, 0x00, 0x60, 0x55, 0xdf,
   0x57, 0xf5, 0x00, 0x00, 0xb8, 0xaa, 0xbf, 0xab, 0xea, 0x01, 0x00, 0x54,
   0x55, 0x5f, 0x55, 0xd5, 0x01, 0x00, 0xaa, 0xae, 0xae, 0xaa, 0xea, 0x01,
   0x00, 0x55, 0x5f, 0x55, 0x55, 0xd5, 0x01, 0x80, 0xaa, 0xbf, 0xaa, 0xaa,
   0xea, 0x01, 0x40, 0x55, 0x5f, 0x55, 0x55, 0xd5, 0x01, 0xa0, 0xaa, 0xae,
   0xaa, 0xaa, 0xea, 0x01, 0x70, 0x55, 0x55, 0x55, 0x55, 0xd5, 0x01, 0xb8,
   0xaa, 0xaa, 0xaa, 0xaa, 0xea, 0x01, 0x58, 0x5d, 0x55, 0x55, 0x55, 0xd5,
   0x01, 0xac, 0xbe, 0xaa, 0xaa, 0xaa, 0xea, 0x01, 0x56, 0x7f, 0x55, 0x55,
   0x55, 0xd5, 0x01, 0xaa, 0xbe, 0xaa, 0xaa, 0xbe, 0xea, 0x01, 0x56, 0x5d,
   0x55, 0x55, 0x7f, 0xf5, 0x01, 0xab, 0xaa, 0xaa, 0xaa, 0xf1, 0xfa, 0x00,
   0x55, 0x55, 0x55, 0xd5, 0xe0, 0x7d, 0x00, 0xab, 0xaa, 0xaa, 0xaa, 0xe0,
   0x7e, 0x00, 0x55, 0x55, 0x55, 0xd5, 0xe0, 0x3d, 0x00, 0xeb, 0xab, 0xaa,
   0xaa, 0xe0, 0x1e, 0x00, 0xf5, 0x57, 0x55, 0xd5, 0x71, 0x3d, 0x00, 0xfb,
   0xaf, 0xaa, 0xaa, 0xbf, 0x7e, 0x00, 0xf5, 0x57, 0x55, 0x55, 0x5f, 0x7d,
   0x00, 0xfb, 0xaf, 0xaa, 0xaa, 0xaa, 0x7a, 0x00, 0xf5, 0x57, 0x55, 0x55,
   0x55, 0x7d, 0x00, 0xeb, 0xab, 0xaa, 0xaa, 0xaa, 0x7a, 0x00, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x7d, 0x00, 0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0x7a, 0x00,
   0x56, 0x55, 0x55, 0x55, 0x55, 0x7d, 0x00, 0xae, 0xaa, 0xaa, 0xaa, 0xaa,
   0x7e, 0x00, 0x54, 0x55, 0x55, 0x55, 0x55, 0x7f, 0x00, 0xac, 0xaa, 0xaa,
   0xaa, 0xaa, 0x3f, 0x00, 0x58, 0x55, 0x55, 0x55, 0xd5, 0x1f, 0x00, 0xb0,
   0xaa, 0xaa, 0xaa, 0xea, 0x07, 0x00, 0x60, 0x55, 0x55, 0x55, 0xf5, 0x03,
   0x00, 0xc0, 0xaa, 0xaa, 0xaa, 0xfa, 0x01, 0x00, 0x80, 0x55, 0x55, 0x55,
   0xfd, 0x01, 0x00, 0x00, 0xae, 0xaa, 0xaa, 0xff, 0x00, 0x00, 0x00, 0xf8,
   0xff, 0xff, 0x7f, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0x3f, 0x00, 0x00,
   0x00, 0xe0, 0xff, 0xff, 0x07, 0x00, 0x00};

#define CUST_ICON_75_width 75
#define CUST_ICON_75_height 75
static char CUST_ICON_75_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xf0, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xf0, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe,
   0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0x3f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xaa, 0xaa, 0xea, 0x07, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x7c, 0x55, 0x55, 0xd5, 0x07, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xaf, 0xaa, 0xaa, 0xaa, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xc0,
   0x57, 0x55, 0x75, 0x55, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xaa, 0xaa,
   0xfe, 0xab, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x7e, 0xd5, 0x55, 0xff, 0x57,
   0x7d, 0x00, 0x00, 0x00, 0x00, 0xaf, 0xfa, 0xaf, 0xfe, 0xab, 0xfa, 0x00,
   0x00, 0x00, 0xe0, 0x55, 0xfd, 0x5f, 0xff, 0x57, 0xf5, 0x01, 0x00, 0x00,
   0xf8, 0xaa, 0xfa, 0xaf, 0xfe, 0xab, 0xea, 0x03, 0x00, 0x00, 0x5c, 0x55,
   0xfd, 0x5f, 0xfd, 0x55, 0xd5, 0x07, 0x00, 0x00, 0xae, 0xaa, 0xfa, 0xaf,
   0xfa, 0xaa, 0xaa, 0x07, 0x00, 0x00, 0x57, 0x55, 0xf5, 0x57, 0x55, 0x55,
   0xd5, 0x07, 0x00, 0xc0, 0xab, 0xbe, 0xea, 0xab, 0xaa, 0xaa, 0xaa, 0x07,
   0x00, 0xe0, 0x55, 0x7f, 0x55, 0x55, 0x55, 0x55, 0xd5, 0x07, 0x00, 0xf0,
   0xaa, 0xff, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x07, 0x00, 0x58, 0xd5, 0xff,
   0x55, 0x55, 0x55, 0x55, 0xd5, 0x07, 0x00, 0xac, 0xaa, 0xff, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0x07, 0x00, 0x5e, 0xd5, 0xff, 0x55, 0x55, 0x55, 0x55,
   0xd5, 0x07, 0x00, 0xaf, 0xaa, 0xff, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x07,
   0x80, 0x57, 0x55, 0x5d, 0x55, 0x55, 0x55, 0x55, 0xd5, 0x07, 0xc0, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x07, 0xe0, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x55, 0xd5, 0x07, 0xe0, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0xaa, 0x07, 0x60, 0xd5, 0x57, 0x55, 0x55, 0x55, 0x55, 0x55,
   0xd5, 0x07, 0xb8, 0xea, 0xaf, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x07,
   0x58, 0xf5, 0x5f, 0x55, 0x55, 0x55, 0x55, 0x55, 0xd5, 0x07, 0xae, 0xfa,
   0xbf, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x07, 0x56, 0xf5, 0x5f, 0x55,
   0x55, 0x55, 0x55, 0x55, 0xd5, 0x07, 0xae, 0xfa, 0xbf, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0xea, 0x03, 0x56, 0xf5, 0x5f, 0x55, 0x55, 0x55, 0xf5, 0x55,
   0xf5, 0x01, 0xae, 0xaa, 0xab, 0xaa, 0xaa, 0xaa, 0xfe, 0xaf, 0xfa, 0x01,
   0x57, 0x55, 0x55, 0x55, 0x55, 0x55, 0x9d, 0x5f, 0xfd, 0x01, 0xab, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0x06, 0xbe, 0xfe, 0x00, 0x57, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x07, 0x5c, 0x7f, 0x00, 0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0x06, 0xbc, 0x1f, 0x00, 0x57, 0x55, 0x55, 0x55, 0x55, 0x55, 0x07, 0x5c,
   0x1f, 0x00, 0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x06, 0xbe, 0x0f, 0x00,
   0x57, 0x55, 0x55, 0x55, 0x55, 0x55, 0x07, 0x5f, 0x0f, 0x00, 0xab, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xee, 0xaf, 0x1f, 0x00, 0x57, 0x75, 0x55, 0x55,
   0x55, 0x55, 0xfd, 0x57, 0x7f, 0x00, 0xab, 0xfe, 0xab, 0xaa, 0xaa, 0xaa,
   0xfa, 0xab, 0x7e, 0x00, 0x57, 0xff, 0x57, 0x55, 0x55, 0x55, 0x55, 0x55,
   0xfd, 0x00, 0xab, 0xff, 0xaf, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xfa, 0x00,
   0x57, 0xff, 0x57, 0x55, 0x55, 0x55, 0x55, 0x55, 0xfd, 0x00, 0xab, 0xff,
   0xaf, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xfa, 0x00, 0x57, 0xff, 0x57, 0x55,
   0x55, 0x55, 0x55, 0x55, 0xf5, 0x00, 0xab, 0xfe, 0xab, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0xfa, 0x00, 0x57, 0x75, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
   0xf5, 0x00, 0xaf, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xfa, 0x00,
   0x57, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xf5, 0x00, 0xae, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xfa, 0x00, 0x5e, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x55, 0xfd, 0x00, 0xae, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0xaa, 0x7a, 0x00, 0x58, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
   0x3d, 0x00, 0xb8, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x3e, 0x00,
   0x58, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x3d, 0x00, 0xb8, 0xaa,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0x1e, 0x00, 0x70, 0x55, 0x55, 0x55,
   0x55, 0x55, 0x55, 0xd5, 0x0f, 0x00, 0xe0, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
   0xaa, 0xea, 0x07, 0x00, 0x80, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xf5,
   0x03, 0x00, 0x80, 0xab, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xfa, 0x03, 0x00,
   0x00, 0x5f, 0x55, 0x55, 0x55, 0x55, 0x55, 0xfd, 0x01, 0x00, 0x00, 0xbc,
   0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xfa, 0x00, 0x00, 0x00, 0xf0, 0x55, 0x55,
   0x55, 0x55, 0x55, 0x7f, 0x00, 0x00, 0x00, 0xe0, 0xab, 0xaa, 0xaa, 0xaa,
   0xaa, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x55, 0x55, 0x55, 0x55, 0xd5, 0x0f,
   0x00, 0x00, 0x00, 0x00, 0xae, 0xaa, 0xaa, 0xaa, 0xfa, 0x07, 0x00, 0x00,
   0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00,
   0xf0, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static XtCallbackRec color_button_activate_cb[2] =
{
    {(XtCallbackProc) color_button_activate, NULL},
    {NULL, NULL}
};

static XtCallbackRec arrow_arm_cb[2] =
{
    {(XtCallbackProc) arrow_arm, NULL},
    {NULL, NULL}
};

static XtCallbackRec arrow_disarm_cb[2] =
{
    {(XtCallbackProc) arrow_disarm, NULL},
    {NULL, NULL}
};

static XtCallbackRec image_input_cb[2] =
{
    {(XtCallbackProc) image_input, NULL},
    {NULL, NULL}
};

/* 
 * This keeps track of the stuff associated with each pixel allocated
 * by the customizer (one record / pixel).  In multiscreen mode one
 * pixel for each screen is stored in this record, these pixels will
 * always have the same RGB values.
 */

typedef struct _ValueRec {
    char *value_name;
    XmString xm_value_name;
    Pixel *pixel;
    Widget box;
    Widget button;
    Widget arrow;
    Widget popup;
    Widget popup_label;
    struct _ValueRec *top_shadow;
    struct _ValueRec *bottom_shadow;
    struct _ValueRec *select_color;
    int num_resources;
    XmString *resources;
    Boolean is_in_palette;
} ValueRec, *ValueList;

typedef struct _ResTranslationRec {
    char *resource_name;
    char *translation;
} ResTranslationRec, *ResTranslations;

/*
 * This table maps some common resource specifications into more
 * readable strings
 */

ResTranslationRec res_translations [] = {
    {"sm.screenBackground", "Screen background"},
    {"sm.screenForeground", "Screen foreground"},
    {"*background", "Window background"},
    {"*foreground", "Window foreground"},
    {"*topShadowColor", "Window top shadow"},
    {"*bottomShadowColor", "Window bottom shadow"},
    {"Mwm*background", "Border background"},
    {"Mwm*foreground", "Border foreground"},
    {"Mwm*topShadowColor", "Border top shadow"},
    {"Mwm*bottomShadowColor", "Border bottom shadow"},
    {"Mwm*activeBackground", "Active border background"},
    {"Mwm*activeForeground", "Active border foreground"},
    {"Mwm*activeTopShadowColor", "Active border top shadow"},
    {"Mwm*activeBottomShadowColor", "Active border bottom shadow"},
    {"Mwm*matteBackground", "Matte background"},
    {"Mwm*matteForeground", "Matte foreground"},
    {"Mwm*matteTopShadowColor", "Matte top shadow"},
    {"Mwm*matteBottomShadowColor", "Matte bottom shadow"},
    {"Mwm*menu*background", "Menu background"},
    {"Mwm*menu*foreground", "Menu foreground"},
    {"Mwm*menu*topShadowColor", "Menu top shadow"},
    {"Mwm*menu*bottomShadowColor", "Menu bottom shadow"},
    {"Mwm*iconImageBackground", "Icon image background"},
    {"Mwm*iconImageForeground", "Icon image foreground"},
    {"Mwm*iconImageTopShadowColor", "Icon image top shadow"},
    {"Mwm*iconImageBottomShadowColor", "Icon image bottom shadow"},
    {"*highlightColor", "Highlight color"},
    {"*troughColor", "Trough color"},
    {"*armColor", "Arm color"},
    {"*selectColor", "Select color"},
    {"sm.pointer_foreground", "Pointer foreground"},
    {"sm.pointer_background", "Pointer background"}
};

int rt_num = XtNumber (res_translations);

typedef struct _PaletteResources {
    char *value_name;
    ValueList value_rec;
} PaletteResourceRec, *PaletteResourceList;

PaletteResourceList pr_list = NULL;
int pr_num = 0;
ValueList value_list = NULL;
int num_values = 0;
int current_palette = 0;
ValueList current_color = NULL;
Widget current_button;
#ifdef VMS
char *app_defaults_name = "DECW$USER_DEFAULTS:CUSTOM.DAT";
char *dxm_defaults_name = "DECW$USER_DEFAULTS:DXMDEFAULTS.DAT";
#else
/* Default value determined at runtime for Unix */
char *app_defaults_name;
char *dxm_defaults_name;
#endif

/* This function goes through the list of resource values settable */
/* by palette switches and points each entry to it's corresponding */
/* record in the value_list.					   */

static void CreatePaletteLinks()
{
    int i,j;
    PaletteResourceList prl;
    ValueList vl;
    int new_pixels = 0;

    for (i=0, prl = pr_list; i < pr_num; i++, prl++)
    {
	for (j=0, vl=value_list; ((j<num_values) && (prl->value_rec == NULL)); j++, vl++)
	{
	    if (strcmp(prl->value_name, vl->value_name) == 0)     
	    {
		prl->value_rec = vl;
		vl->is_in_palette = TRUE;
		break;
	    }
	}	
    }
}

/*
 * Writes the current set of palette colors to the resource database
 */

static void write_palette_colors (db)
    XrmDatabase db;
{
    int i;
    XrmValue value;
    char *pcbuff, temp[200];

    /* Allocate a buffer big enough to handle the paletteColors string */

    pcbuff = XtMalloc (pr_num * 50 + 1000);
    strcpy (pcbuff, "");

    for (i=0; i<pr_num; i++)
    {
	sprintf (temp, "%s\n", pr_list[i].value_name);
	strcat (pcbuff, temp);
    }

    value.addr = pcbuff;
    value.size = strlen (pcbuff) + 1;
    XrmPutResource (&db, "Custom.paletteColors", XtRString, &value);

    need_to_write_palette_colors = FALSE;
}

/* This function finds all the dynamic color cells that aren't     */
/* included in the list of colors affected by palette switches.    */
/* These are then added to the palette resource list and the       */
/* list is stored in the resource database as Custom.paletteColors */
/*								   */
/* This process is needed to cause color names newly added to      */
/* the DXmDefaults file by the user to be integrated into the  	   */
/* exisiting palette scheme.					   */

static int AddNewPixelsToPalette()
{
    int i,j;
    ValueList vl;
    int num_new_pixels = 0;

    for (i=0, vl=value_list; i<num_values; i++, vl++)
	    if (!vl->is_in_palette)
		num_new_pixels++;

    if (num_new_pixels)
    {
	pr_list = (PaletteResourceList) XtRealloc ((char *) pr_list,
		    (pr_num + num_new_pixels) * sizeof (PaletteResourceRec));

	for (i=0, vl=value_list; i<num_values; i++, vl++)
	{
	    if (!vl->is_in_palette)
	    {
		pr_list[pr_num].value_name = strcpy (XtMalloc(strlen(vl->value_name)+1), vl->value_name);
		pr_list[pr_num].value_rec = vl;
		pr_num++;
	    }
	}

	need_to_write_palette_colors = TRUE;
    }

    return (num_new_pixels);
}

/* This function fills in the pointers to the top and bottom shadow */
/* records for each background color record			    */

static void CreateShadowLinks()
{
    ValueList vl, vl1;
    int i,j, len;
    char ts_str[100], bs_str[100], sel_str[100];

    for (i=0, vl=value_list; i < num_values; i++, vl++)
    {
	if (strstr(vl->value_name, "Background") != NULL)
	{
	    len = strlen(vl->value_name) - strlen("Background");

	    strncpy (ts_str, vl->value_name, len);
	    ts_str[len] = '\0';
	    strcat (ts_str, "TopShadow");

	    strncpy (bs_str, vl->value_name, len);
	    bs_str[len] = '\0';
	    strcat (bs_str, "BottomShadow");

	    strncpy (sel_str, vl->value_name, len);
	    sel_str[len] = '\0';
	    strcat (sel_str, "SelectColor");

	    for (j=0, vl1=value_list; j < num_values; j++, vl1++)
	    {
		if (strcmp (vl1->value_name, ts_str) == 0)
		    vl->top_shadow = vl1;

		if (strcmp (vl1->value_name, bs_str) == 0)
		    vl->bottom_shadow = vl1;

		if (strcmp (vl1->value_name, sel_str) == 0)
		    vl->select_color = vl1;
	    }
	}
    }	    
}

/* This routine takes a resource name (like "Mwm*background") and checks */
/* the resource name translation table for a more readable translation,  */
/* returning the translation string if one exists or the original        */
/* otherwise.  An intelligent resource name parser should eventually     */
/* replace this crude method.						 */

static char *TranslateResourceName (name)
    char *name;
{
    int i;
    ResTranslations rt;

    for (i=0, rt=res_translations; i<rt_num; i++, rt++)
    {
	if (strcmp(rt->resource_name, name) == 0)
	    return (rt->translation);
    }

    /* No translation found, return original string */

    return (name);
}

/* This function adds a record for the given resource value to the value */
/* list, unless one already exists.  Then the resource name associated   */
/* with this value is added to that resource value's list of associated  */
/* resource names							 */

static void AddResourceValueToList (value, name)
    char *value, *name;
{
    ValueList vl, match;
    int i;
    char *string;
    Boolean found = FALSE;
	
    for (i=0, vl=value_list; i<num_values && !found; i++, vl++)
	if (strcmp(value, vl->value_name) == 0)
	{
	    found = TRUE;
	    match = vl;
	}

    if (!found)   /* Not found, add a new record to the list */
    {
	value_list = (ValueList) XtRealloc ((char *) value_list, sizeof(ValueRec) * (num_values + 1));
	match = &value_list[num_values];
	match->value_name = strcpy (XtMalloc(strlen(value)+1), value);
	match->xm_value_name = XmStringCreateSimple (match->value_name);
	match->pixel = (Pixel *) XtMalloc(sizeof(Pixel) * num_screens);
	match->top_shadow = NULL;
	match->bottom_shadow = NULL;
	match->select_color = NULL;
	match->num_resources = 0;
	match->resources = NULL;
	match->is_in_palette = FALSE;
	num_values++;
    }

    /* Add the resource name to the list of compound strings */

    string = TranslateResourceName (name);

    match->resources = (XmString *) XtRealloc ((char *) match->resources,
			    (sizeof (XmString *))*(match->num_resources+1));
    match->resources[match->num_resources] = XmStringCreateSimple(string);
    match->num_resources++;
}

/* This routine loads our data structures with the contents of the   */
/* DXmDefaults resource file.  It also writes the file to a property */
/* which the toolkit will merge in with the other resource databases */
/* upon each application startup.				     */

static void ReadDXmDefaults ()
{
    int i;
    Atom atom;
    XrmDatabase rdb, db;
    FILE *dxm_defaults_file;
    char dxm_defaults[20000], line[500], *name, *value;

    dxm_defaults_file = fopen (dxm_defaults_name, "r");

    if (!dxm_defaults_file)
    {
	printf ("Couldn't open %s\nExiting....\n", dxm_defaults_name);
	exit(1);
    }

    fread(dxm_defaults, 1, 20000, dxm_defaults_file);    
    fclose (dxm_defaults_file);

    atom = XInternAtom (display, "DXM_DEFAULTS", FALSE);

    for (i=0; i<num_screens; i++)
    {
	XChangeProperty (display, RootWindowOfScreen(screens[i]), atom, XA_STRING, 8,
			 PropModeReplace, (unsigned char *) dxm_defaults, strlen(dxm_defaults)+1);
    }

    /* Merge with current database so this application (the customizer)  */
    /* will also pick up the dynamic colors.				 */

    db = XrmGetStringDatabase (dxm_defaults);
    rdb = XtDatabase (display);
    XrmMergeDatabases (db, &rdb);

    /* Parse the dxmdefaults file.  Add all the resource values encountered */
    /* to our list							    */

    name = strtok(dxm_defaults, " :\t\n");
    while (name != NULL && name[0] == (char) "!")
	name = strtok (NULL, " :\t\n");

    while (name != NULL)
    {
	value = strtok (NULL, " :\t\n");
	AddResourceValueToList (value, name);
	name = strtok (NULL, " :\t\n");
	while (name != NULL && name[0] == (char) "!")
	    name = strtok (NULL, " :\t\n");
    }    

    CreateShadowLinks ();
}

/* Create a push button and arrow button for each of the dynamic     */
/* resource values encountered.  Also create a popup for each that   */
/* describes which resources it's associated with.		     */

static void CreateColorButtons ()
{
    int n=0, i, j;
    ValueList vl;
    XmString label_string, temp, sep=XmStringSeparatorCreate();

    for (i=0, vl=value_list; i<num_values; i++, vl++)
    {

	/* Create a rowcolumn to hold the color button and arrow */

	XtSetArg (al[0], XmNorientation, XmHORIZONTAL);
	XtSetArg (al[1], XmNspacing, 0);

	if (strstr(vl->value_name, "Shadow") != NULL)
	    vl->box = (Widget) XmCreateRowColumn (shadow_menu, "colorbox", al, 2);
	else
	    vl->box = (Widget) XmCreateRowColumn (color_menu, "colorbox", al, 2);

	color_button_activate_cb[0].closure = (XtPointer) vl;
    
	XtSetArg (al[0], XmNwidth, 30); 
	XtSetArg (al[1], XmNheight, 30);
	XtSetArg (al[2], XmNrecomputeSize, FALSE); 
	XtSetArg (al[3], XmNlabelString, XmStringCreateSimple(" "));
	XtSetArg (al[4], XmNactivateCallback, color_button_activate_cb);
	XtSetArg (al[5], XmNbackground, vl->pixel[def_index]);
	XtSetArg (al[6], XmNtopShadowColor, WhitePixelOfScreen (XtScreen(vl->box)));
	XtSetArg (al[7], XmNbottomShadowColor, BlackPixelOfScreen (XtScreen(vl->box)));

	vl->button = (Widget) XmCreatePushButton (vl->box, "colorbutton", al, 8);
	XtManageChild (vl->button);

	/* Create the arrow button */

	arrow_arm_cb[0].closure = (XtPointer) vl;
	arrow_disarm_cb[0].closure = (XtPointer) vl;

	XtSetArg (al[0], XmNarrowDirection, XmARROW_RIGHT); 
	XtSetArg (al[1], XmNarmCallback, arrow_arm_cb); 
	XtSetArg (al[2], XmNdisarmCallback, arrow_disarm_cb); 
	XtSetArg (al[3], XmNshadowThickness, 0); 
	XtSetArg (al[4], XmNforeground, vl->pixel[def_index]);

	vl->arrow = (Widget) XmCreateArrowButton (vl->box, "colorarrow", al, 5);
	XtManageChild (vl->arrow);

        /* Create a popup bulletin board with no window manager borders */

        XtSetArg (al[0], XmNdefaultPosition, FALSE);
        XtSetArg (al[1], XmNshadowThickness, 2);
        XtSetArg (al[2], XmNmarginWidth, 3);
        XtSetArg (al[3], XmNmarginHeight, 3);
        vl->popup = (Widget) XmCreateBulletinBoardDialog (vl->box, "colorpopup", al, 4);

        XtSetArg (al[0], XmNoverrideRedirect, TRUE);
        XtSetValues (XtParent (vl->popup), al, 1);

	/* Create a label displaying the resources tied to this pixel */

	label_string = XmStringCreateSimple ("");
	for (j=0; j<vl->num_resources; j++)
	{		
	    label_string = XmStringConcat (label_string, vl->resources[j]);
	    if (j < vl->num_resources-1)
		label_string = XmStringConcat (label_string, sep);
	}
	XtSetArg (al[0], XmNlabelString, label_string);
        XtSetArg (al[1], XmNmarginWidth, 1);
        XtSetArg (al[2], XmNmarginHeight, 1);
        XtSetArg (al[3], XmNalignment, XmALIGNMENT_BEGINNING);
	vl->popup_label = (Widget) XmCreateLabelGadget (vl->popup, "popup_label", al, 4);
	XtManageChild (vl->popup_label);

	XmStringFree (label_string);
    }

    XtManageChildren ((WidgetList) DXmChildren(color_menu), DXmNumChildren(color_menu));
    XtManageChildren ((WidgetList) DXmChildren(shadow_menu), DXmNumChildren(shadow_menu));

    XtSetArg (al[0], XmNnumColumns, DXmNumChildren(shadow_menu)/2);
    XtSetValues (shadow_menu, al, 1);
}

/* Initializes the list of image colors.  This is done by querying the   */
/* resource database for the pixel that corresponds to each section of   */
/* the image.  The image data is later modified to reflect these pixels. */

static void init_image_colormap()
{
    int         i;
    Boolean     status;
    char *	reptype;
    XrmValue	source, dest;
    XrmDatabase rdb = XtDatabase (display);

    image_cmap = (Pixel *) XtMalloc((icolors + 1) * sizeof(Pixel));
    dest.size = sizeof(Pixel);

    for (i = 0; i < icolors; i++) 
    {
	if (XrmGetResource (rdb, inames[i], iclasses[i], &reptype, &source))
	{		    
	    dest.addr = (XPointer) &image_cmap[i];
	    if (!XtConvertAndStore ( toplevel, XtRString, &source, 
				    XtRPixel, &dest))
		image_cmap[i] = BlackPixelOfScreen(XtScreen(toplevel));
	}
	else
	    image_cmap[i] = BlackPixelOfScreen(XtScreen(toplevel));
    }
}

/* This function creates the screen facsimile image from the stored data */
/* and merges it into a pixmap which is displayed as the background of a */
/* drawing area widget.							 */

static void CreateImage ()
{
    int i;
    char *d;
    Pixmap pixmap;
    GC gc;

    init_image_colormap();

    for (i=0, d=iraster; i < iwidth * iheight; i++, d++)
	*d = (char) image_cmap[*d];

    image = XCreateImage (display, DefaultVisual (display, def_screen_num),
			  DefaultDepthOfScreen (def_screen), ZPixmap, 0,
			  iraster, iwidth, iheight, 8, iwidth);

    pixmap = XCreatePixmap (display, RootWindow (display, def_screen_num),
			    iwidth, iheight, DefaultDepthOfScreen (def_screen));

    gc = XCreateGC (display, RootWindow (display, def_screen_num), 0, NULL);

    XPutImage (display, pixmap, gc, image, 0, 0, 0, 0, iwidth, iheight);

    XtSetArg(al[0], XmNwidth, iwidth);
    XtSetArg(al[1], XmNheight, iheight);
    XtSetArg(al[2], XmNbackgroundPixmap, pixmap);
    XtSetArg(al[3], XmNinputCallback, image_input_cb);
    
    pict_label =  (Widget) XmCreateDrawingArea (picture, "ilabel", al, 4);

    XtManageChild (pict_label);    

}

/* This function creates a list of allocated pixels indexed by the atomized  */
/* color value name for each pixel and stores this list on a property on the */
/* root window.  When applications are later invoked, the toolkit knows to   */
/* look in this property for dynamic color specifications.		     */

static void InitializeColors ()
{
    int i, j;
    ValueList vl;
    Atom atom, type;
    unsigned long *prop, *temp;

    atom = XInternAtom (display, "DXM_DYNAMIC_COLORS", FALSE);
    type = XInternAtom (display, "DXM_DYNAMIC_COLORS_TYPE", FALSE);

    for (i=0; i<num_screens; i++)
    {
	if ((DefaultVisualOfScreen(screens[i])->class == StaticColor) ||
	    (DefaultVisualOfScreen(screens[i])->class == TrueColor) ||
	    (DefaultVisualOfScreen(screens[i])->class == StaticGray))
	{
	    printf ("Default visual of screen %d doesn't support read/write colormap\n", i);
	    printf ("Exiting\n");
	    exit(1);
	}
	prop = temp = (unsigned long *) XtMalloc (sizeof(unsigned long) * num_values * 2);
	for (j=0, vl=value_list; j<num_values; j++, vl++)
	{
	    if (!XAllocColorCells (display, cmaps[i], FALSE, 0, 0, &vl->pixel[i], 1))
	    {
		printf ("Could only allocate %d out of %d color cells on screen %d\n", j, num_values, i );
		printf ("Try paring down your DXmDefaults file\n");		
		printf ("Exiting\n");
		exit(1);
	    }		
	    *temp = (unsigned long) XInternAtom(display, vl->value_name, FALSE); temp++;
	    *temp = (unsigned long) vl->pixel[i]; temp++;
	    if (strcmp(vl->value_name, "DXmDynamicScreenBackground") == 0)
	    {
		XSetWindowBackground (display, RootWindowOfScreen(screens[i]),
		    vl->pixel[i]); 
		XClearWindow (display, RootWindowOfScreen(screens[i]));
	    }
	}
	
	XChangeProperty (display, RootWindowOfScreen(screens[i]), atom, type, 32,
		     PropModeReplace, (unsigned char *) prop, num_values * 2);
    }		 
}

/* Fetches the list of palette names and values from the resource database and */
/* adds the palette list box items.					       */

static void SetupPalettes ()
{
    char name[128];
    unsigned short *colors;
    int i, j, red, green, blue, num_new_pixels;    
    PaletteResourceList prl;
    char *temp, *type, *temp_palette;
    XrmValue value;
    XrmDatabase rdb = XrmGetDatabase(display);

    /* Set up the palette resource list */

    pr_list = (PaletteResourceList) XtMalloc (pr_num * sizeof (PaletteResourceRec));

    temp = strtok(palette_colors_string, " :\t\n");

    for (i=0, prl = pr_list; i<pr_num; i++, prl++)
    {
	prl->value_name = strcpy (XtMalloc(strlen(temp)+1), temp);
	prl->value_rec = (ValueList) NULL;
	temp = strtok (NULL, " :\t\n");
    }				   

    CreatePaletteLinks();

    num_new_pixels = AddNewPixelsToPalette();

    palettes = (unsigned short **) XtMalloc (num_palettes * sizeof (unsigned short *));
    palette_names = (XmString *) XtMalloc (num_palettes * sizeof (XmString));

    /* For each palette name, create a compound string for use in the 
     * list box, fetch the color definitions for that palette from
     * the resource database, and parse these into our data structures
     */

    for (i=0; i<num_palettes; i++)
    {
	palette_names[i] = XmStringCreateSimple (palette_name_list[i]);
	strcpy (name, "Custom.");
	strcat (name, palette_name_list[i]);
	XrmGetResource (rdb, name, name, &type, &value);       
	temp_palette = value.addr;
	if (temp_palette == NULL)
	    printf ("Error: %s palette not defined\n", palette_name_list[i]);

	colors = (unsigned short *) XtMalloc ((pr_num * 3) * sizeof (unsigned short));
	temp = strtok(temp_palette, " :\t\n");

	for (j=0; j<pr_num; j++)
	{
	    if (j < pr_num - num_new_pixels)
	    {
		colors[j*3] = atoi (temp);
		temp = strtok (NULL, " :\t\n");
		colors[j*3+1] = atoi (temp);
		temp = strtok (NULL, " :\t\n");
		colors[j*3+2] = atoi (temp);
		temp = strtok (NULL, " :\t\n");
	    }
	    else
	    {
		/* New additions to palette, give them default RGB values */
		if (strstr (pr_list[j].value_name, "Foreground") != NULL)
		    colors[j*3] = colors[j*3+1] = colors[j*3+2] = 65535;
		else
		    colors[j*3] = colors[j*3+1] = colors[j*3+2] = 0;
	    }
	}

	palettes[i] = colors;
    }

    XtSetArg (al[0], XmNitems, palette_names);
    XtSetArg (al[1], XmNitemCount, num_palettes);
    XtSetValues (list, al, 2);

    if (num_new_pixels)
	save_palettes();
}

/* Change Palette */
static void ChangePalette (index)
    int index;
{
    PaletteResourceList prl;
    XColor xc;
    int i, j;

    current_palette = index;

    /* First pass, sets each color that was allocated */

    for (i=0, prl = pr_list; i<pr_num; i++, prl++)
    {
	if (prl->value_rec != NULL)
	{
	    xc.red = palettes[index][i*3];	
	    xc.green = palettes[index][i*3+1];	
	    xc.blue = palettes[index][i*3+2];	
	    xc.flags = DoRed | DoGreen | DoBlue;
	    for (j=0; j<num_screens; j++)
	    {
		xc.pixel = prl->value_rec->pixel[j];
		XStoreColor (display, cmaps[j], &xc);
	    }
	}
    }

    /* Second pass, for auto shadowing */

    if (auto_shadow)
	for (i=0, prl = pr_list; i<pr_num; i++, prl++)
	{
	    if ((prl->value_rec != NULL) &&
		(prl->value_rec->top_shadow || prl->value_rec->bottom_shadow ||
		 prl->value_rec->select_color))
	    {
		xc.pixel = prl->value_rec->pixel[def_index];
		xc.red = palettes[index][i*3];	
		xc.green = palettes[index][i*3+1];	
		xc.blue = palettes[index][i*3+2];	
		xc.flags = DoRed | DoGreen | DoBlue;
		UpdateShadowColors (prl->value_rec, &xc);
	    }
	}
}

/* 
 * Fetches the customizer resources from the resource database.
 * We should be using XtConvertAndStore to convert the resources
 * but it requires a widget argument.  We can't create any widgets
 * until after the color set up is done so we call the converters
 * directly.
 */

static void FetchCustomizerResources ()
{
    char *type, *temp, *temp2, *palette_names_string;
    XrmValue value, dest;
    XtCacheRef cache_ref;
    XrmDatabase rdb = XrmGetDatabase(display);
    int i;

    /* Fetch the autoShadow resource */

    XrmGetResource (rdb, "Custom.autoShadow", "Custom.AutoShadow", &type, &value);       
    if (value.addr)
    {
	dest.size = sizeof (Boolean); dest.addr = (XPointer) &auto_shadow;
	XtCallConverter (display, XtCvtStringToBoolean, NULL, 0, &value,
			 &dest, &cache_ref);
    }

    /* Fetch the multiscreen resource */

    XrmGetResource (rdb, "Custom.multiScreen", "Custom.MultiScreen", &type, &value);       
    if (value.addr)
    {
	dest.size = sizeof (Boolean); dest.addr = (XPointer) &multiscreen;
	XtCallConverter (display, XtCvtStringToBoolean, NULL, 0, &value,
			 &dest, &cache_ref);
    }

    /* Get the list of palette colors */

    XrmGetResource (rdb, "Custom.paletteColors", "Custom.PaletteColors", 
		    &type, &value);       
    
    palette_colors_string = value.addr;

    if (!palette_colors_string)
    {
	printf ("Couldn't get paletteColors resource\nCheck file %s\nExiting...\n", app_defaults_name);
	exit(1);
    }


    /* Figure out the number of colors in a palette */

    pr_num = 0;
    temp2 = strcpy(XtMalloc(strlen(palette_colors_string) + 1), palette_colors_string);
    temp = strtok(temp2, " :\t\n");
    while (temp != NULL)
    {
	pr_num++;
	temp = strtok (NULL, " :\t\n");
    }				   

    /* Get the palette names list */

    XrmGetResource (rdb, "Custom.paletteNames", "Custom.PaletteNames", 
		    &type, &value);       

    palette_names_string = value.addr;

    if (!palette_names_string)
    {
	printf ("Couldn't get paletteNames resource\nCheck file %s\nExiting...\n", app_defaults_name);
	exit(1);
    }

    /* Figure out the number of palette names */

    num_palettes = 0;
    temp2  = strcpy(XtMalloc(strlen(palette_names_string) + 1), palette_names_string);
    temp = strtok(temp2, " :\t\n");
    while (temp != NULL)
    {
	num_palettes++;
	temp = strtok (NULL, " :\t\n");
    }				   
    
    /* Setup the palette name list as an array of strings */

    palette_name_list = (char **) XtMalloc (num_palettes * sizeof (char *));
    temp = strtok(palette_names_string, " :\t\n");
    i = 0;
    while (temp != NULL)
    {
	palette_name_list[i++] = strcpy(XtMalloc(strlen(temp) + 1), temp);
	temp = strtok (NULL, " :\t\n");
    }				   
}

/*
 * Initializes the arrays of screens and colormaps that will be
 * affected by the customizer
 */

static void InitializeScreenData ()
{
    int i;

    def_screen = DefaultScreenOfDisplay(display);
    def_screen_num = XScreenNumberOfScreen(def_screen);

    if (multiscreen)
    {
	def_index = def_screen_num;
	num_screens = ScreenCount(display);
	screens = (Screen **) XtMalloc (sizeof(Screen *) * num_screens);
    	cmaps = (Colormap *) XtMalloc(sizeof(Colormap) * num_screens);
	for (i=0; i<num_screens; i++)
	{
	    screens[i] = ScreenOfDisplay(display, i);
	    cmaps[i] = DefaultColormapOfScreen(screens[i]);
	    _XmGetDefaultThresholdsForScreen(screens[i]);
	}
    }
    else
    {
	def_index = 0;
	num_screens = 1;
	screens = (Screen **) XtMalloc (sizeof(Screen *));
	cmaps = (Colormap *) XtMalloc(sizeof(Colormap));
	screens[0] = def_screen;
	cmaps[0] = DefaultColormapOfScreen(def_screen);
	_XmGetDefaultThresholdsForScreen(screens[0]);
    }
}

/*
 *  Finds the window manager's preferred icon size and creates
 *  an appropriately sized icon.
 */

static Pixmap CreateCustomIcon ()
{    
    XIconSize *size_list;
    int num_sizes, i, biggest, best_size = 17;
    Pixmap pixmap_rtn;

    if (XGetIconSizes(display, DefaultRootWindow(display), &size_list, &num_sizes))
    {
	biggest = 0;
	for (i = 1; i < num_sizes; i++) 
	{
	    if ((size_list[i].max_width >= size_list[biggest].max_width) &&
		(size_list[i].max_height >= size_list[biggest].max_height))
		biggest = i;
	}

	if ((size_list[biggest].max_width >= 75) && (size_list[biggest].max_height >= 75))
	    best_size = 75;
	else if ((size_list[biggest].max_width >= 50) && (size_list[biggest].max_height >= 50))
	    best_size = 50;
	else if ((size_list[biggest].max_width >= 32) && (size_list[biggest].max_height >= 32))
	    best_size = 32;

	XFree(size_list);
    }		

    switch (best_size) {
	case 75:
	    pixmap_rtn = XCreateBitmapFromData (display, XtWindow (toplevel),
		    CUST_ICON_75_bits, CUST_ICON_75_width, CUST_ICON_75_height);
	    break;
	case 50:
	    pixmap_rtn = XCreateBitmapFromData (display, XtWindow (toplevel),
		    CUST_ICON_50_bits, CUST_ICON_50_width, CUST_ICON_50_height);
	    break;
	case 32:
	    pixmap_rtn = XCreateBitmapFromData (display, XtWindow (toplevel),
		    CUST_ICON_32_bits, CUST_ICON_32_width, CUST_ICON_32_height);
	    break;
	case 17:
	    pixmap_rtn = XCreateBitmapFromData (display, XtWindow (toplevel),
		    CUST_ICON_17_bits, CUST_ICON_17_width, CUST_ICON_17_height);
	    break;
    }

    return (pixmap_rtn);
}

/*
 *  Resets the icon pixmap to the properly sized one on a ReparentNotify
 *  event.  This would normally occur if the window manager starts up
 *  after the color customizer.
 */

static void SetIconPixmap (w, tag, event)
    Widget w;
    XtPointer tag;
    XEvent *event;
{
    Arg al[1];
    Pixmap old, new;

    if (event->type != ReparentNotify)
	return;
    
    new = CreateCustomIcon();

    XtSetArg (al[0], XmNiconPixmap, &old);
    XtGetValues (toplevel, al, 1);

    XtSetArg (al[0], XmNiconPixmap, new);
    XtSetValues (toplevel, al, 1);

    XFreePixmap (XtDisplay(toplevel), old);
}

/*
 *  Main program
 */
int main(argc, argv)
int argc;
char **argv;
{
    int i, n;
    char *temp;

    MrmInitialize ();
    DXmInitialize ();
    XtToolkitInitialize();

    app_context = XtCreateApplicationContext();

    display = XtOpenDisplay(app_context, NULL, "Custom", "Custom",
                            NULL, 0, &argc, argv);

    if (display == NULL) {
        fprintf(stderr, "%s:  Can't open display\n", argv[0]);
        exit(1);
    }

#ifndef VMS
    temp = (char *) _XmOSGetHomeDirName();

    app_defaults_name = XtMalloc (strlen(temp) + strlen("/Custom") + 1);
    strcpy (app_defaults_name, temp);
    strcat (app_defaults_name, "/Custom");

    dxm_defaults_name = XtMalloc (strlen(temp) + strlen("/DXmDefaults") + 1);
    strcpy (dxm_defaults_name, temp);
    strcat (dxm_defaults_name, "/DXmDefaults");
#endif

    FetchCustomizerResources ();
    InitializeScreenData ();
    ReadDXmDefaults();
    InitializeColors ();

    /* 
     * Create the toplevel shell
     */

    n = 0;
    XtSetArg(al[n], XmNallowShellResize, True);  n++;
    XtSetArg(al[n], XmNtitle, "Color Customizer");  n++;
    XtSetArg(al[n], XmNdeleteResponse, XmDO_NOTHING);  n++;
    toplevel = XtAppCreateShell("Custom", "Custom", applicationShellWidgetClass,
                              display, al, n);

    XmAddProtocolCallback (toplevel, 
			   XmInternAtom(display, "WM_PROTOCOLS", FALSE),
			   XmInternAtom(display, "WM_DELETE_WINDOW", FALSE),
			   exit_cb, (XtPointer) 0);

    
    XtAddEventHandler (toplevel, StructureNotifyMask, False,
		       (XtEventHandler) SetIconPixmap, (XtPointer) 0);

    if (MrmOpenHierarchy(1,			    /* number of files	    */
			vec, 			    /* files     	    */
			NULL,			    /* os_ext_list (null)   */
			&s_MrmHierarchy)	    /* ptr to returned id   */
			!= MrmSUCCESS) {
	printf ("can't open hierarchy\n");
	exit(1);
     }

    if (MrmRegisterNames (regvec, regnum)
			!= MrmSUCCESS)
    {
	printf("can't register names\n");
	exit(1);
    }

    if (MrmFetchWidget (s_MrmHierarchy,
			"main",
			toplevel,
			&mainw,
			&class)
			!= MrmSUCCESS)
    {
	printf("can't fetch interface\n");
	exit(1);
    }			    

    SetupPalettes ();
    CreateColorButtons ();
    CreateImage ();

    XtManageChild(mainw);
    
    /*
     *  Realize the toplevel widget.  This will cause the entire "managed"
     *  widget hierarchy to be displayed
     */

    XtRealizeWidget(toplevel);

    XmListSelectPos (list, 1, TRUE); /* Select first palette */

    menu_cursor = XmGetMenuCursor(XtDisplay(toplevel));

    icon_pixmap = CreateCustomIcon();
    XtSetArg (al[0], XmNiconPixmap, icon_pixmap);

    XtSetValues (toplevel, al, 1);

    if (MrmFetchWidget (s_MrmHierarchy,
			"colormix",
			toplevel,
			&colormix,
			&class)
			!= MrmSUCCESS)
    {
	printf("can't fetch colormix\n");
	exit(1);
    }

    XtSetArg (al[0], DXmNsetNewColorProc, &real_set_new_color_proc);
    XtGetValues (colormix, al, 1);
    XtSetArg (al[0], DXmNsetNewColorProc, my_set_new_color_proc );
    XtSetValues (colormix, al, 1);

    if (MrmFetchWidget (s_MrmHierarchy,
			"text_popup",
			toplevel,
			&text_popup,
			&class)
			!= MrmSUCCESS)
    {
	printf("can't fetch text popup\n");
	exit(1);
    }

    XtAddEventHandler (text_popup, VisibilityChangeMask, FALSE,
		       (XtEventHandler) text_popup_vis_handler, (XtPointer) NULL);

    if (MrmFetchWidget (s_MrmHierarchy,
			"exit_popup",
			toplevel,
			&exit_popup,
			&class)
			!= MrmSUCCESS)
    {
	printf("can't fetch exit popup\n");
	exit(1);
    }
    
    XtUnmanageChild ((Widget) XmMessageBoxGetChild (exit_popup, XmDIALOG_HELP_BUTTON));
    
    XtAppMainLoop(app_context);
}

/*
 * Updates the shadow colors for the specified background pixel
 */

static void UpdateShadowColors (vl, background)
    ValueList vl;
    XColor *background;
{
    int i;
    XColor fg, select, top_shadow, bottom_shadow;
    XmColorProc color_proc = XmGetColorCalculation ();

    (*color_proc) (background, &fg, &select, &top_shadow, &bottom_shadow);

    if (vl->top_shadow != NULL)
    {
	top_shadow.flags = DoRed | DoGreen | DoBlue;

	for (i=0; i<num_screens; i++)
	{
	    top_shadow.pixel = vl->top_shadow->pixel[i];
	    XStoreColors (display, cmaps[i], &top_shadow, 1);
	}
    }

    if (vl->bottom_shadow != NULL)
    {
	bottom_shadow.flags = DoRed | DoGreen | DoBlue;
	for (i=0; i<num_screens; i++)
	{
	    bottom_shadow.pixel = vl->bottom_shadow->pixel[i];
	    XStoreColors (display, cmaps[i], &bottom_shadow, 1);
	}
    }

    if (vl->select_color != NULL)
    {
	select.flags = DoRed | DoGreen | DoBlue;
	for (i=0; i<num_screens; i++)
	{
	    select.pixel = vl->select_color->pixel[i];
	    XStoreColors (display, cmaps[i], &select, 1);
	}
    }
}

/*
 * Called when the user clicks on the auto shadowing toggle 
 */

static void auto_shadow_changed ( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmToggleButtonCallbackStruct *callback_data;
{
    XColor xc;
    ValueList vl;
    int i, j;

    if (callback_data->set)
    {
	auto_shadow = TRUE;

	for (j=0, vl=value_list; j < num_values; j++, vl++)
	    if (vl->top_shadow || vl->bottom_shadow || vl->select_color)
	    {
		xc.pixel = vl->pixel[def_index];
		XQueryColor (display, cmaps[def_index], &xc);
		UpdateShadowColors (vl, &xc);
	    }
    }
    else
    {
	auto_shadow = FALSE;
    }
}

/* 
 * Does nothing right now 
 */

static void button_cb ( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
}

/* 
 * Registers widget ids as they're created
 */

static void create_cb ( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
    if (*tag == 1)
	list = widget;

    if (*tag == 2)
	color_menu = widget;

    if (*tag == 3)
	shadow_menu = widget;

    if (*tag == 4)
	text = widget;

    if (*tag == 5)
	picture = widget;

    if (*tag == 6)
	picture_frame = widget;

    if (*tag == 7)
    {
	if (auto_shadow)
	    XmToggleButtonGadgetSetState (widget, TRUE, FALSE);
    }
}

/* 
 * Called when the user selects an item in the palette list box
 */

static void list_select ( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmListCallbackStruct *callback_data;
{
    int item_num = callback_data->item_position;

    ChangePalette (item_num - 1);
}

/* 
 * Updates the colors when user clicks OK or Apply on the colormix widget
 * (this is actually overkill since these colors are already updated
 * dynamically, anyway).
 */

static void cmix_ok ( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	DXmColorMixCallbackStruct *callback_data;
{
    int i;
    XColor xc;

    xc.red = callback_data->newred;
    xc.green = callback_data->newgrn;
    xc.blue = callback_data->newblu;
    xc.flags = DoRed | DoGreen | DoBlue;

    for (i=0; i<num_screens; i++)
    {
	xc.pixel = current_color->pixel[i];
	XStoreColors (display, cmaps[i], &xc, 1);
    }

    if (auto_shadow && (current_color->top_shadow || current_color->bottom_shadow ||
			current_color->select_color))
	UpdateShadowColors (current_color, &xc);

    if (*tag == 0)
	XtUnmanageChild (colormix);
}

/*
 * Called when the users clicks Cancel on the colormix widget
 */

static void cmix_cancel ( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
    XtUnmanageChild (widget);
}

/*
 * This routine is called whenever the user modifies the new color in 
 * the colormix widget
 */

static void my_set_new_color_proc ( widget, red, green, blue )
	Widget	widget;
	unsigned short red, green, blue;
{
    XColor xc;
    int i;

    (*real_set_new_color_proc) (widget, red, green, blue);

    xc.red = red;
    xc.green = green;
    xc.blue = blue;
    xc.flags = DoRed | DoGreen | DoBlue;

    for (i=0; i<num_screens; i++)
    {
	xc.pixel = current_color->pixel[i];
	XStoreColors (display, cmaps[i], &xc, 1);
    }

    if (auto_shadow && (current_color->top_shadow || current_color->bottom_shadow || current_color->select_color))
	UpdateShadowColors (current_color, &xc);
}

/* 
 * This event handler is called when the visibility of the text popup 
 * for querying a palette name changes.  In order to move focus to 
 * another shell, XSetInputFocus must be called, and the widget's
 * window must be visible for the call to succeed.  The end result
 * is that focus is set to the text widget whenever the dialog is
 * popped up.
 */

static XtEventHandler text_popup_vis_handler (w, data, event)
    Widget w;
    XtPointer data;
    XEvent *event;
{
    if ((text_just_managed) && 
	(event->xvisibility.state != VisibilityFullyObscured))
    {
	XSetInputFocus (XtDisplay(w),
			XtWindow(XtParent(w)),
			RevertToParent,
			CurrentTime);

	/* Need to call this twice for some reason */
	XmProcessTraversal (text, XmTRAVERSE_CURRENT);
	XmProcessTraversal (text, XmTRAVERSE_CURRENT);
    }

    text_just_managed = FALSE;
}

/*
 * Manages the popup that queries for a palette name
 */

static void popup_add_palette_box ( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
    text_just_managed = TRUE;
    XtManageChild (text_popup);
}

/* 
 * This routine will remove the currently selected palette from the 
 * palette listbox and free up the memory associated with it.	    
 */

static void delete_palette ( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
    int i, j, item_count, pos_count, *pos_list;
    unsigned short ** temp, **new;
    XmString *temp_names, *new_names;

    need_to_save = TRUE;

    if (!XmListGetSelectedPos (list, &pos_list, &pos_count))
	return;
 
    XmListDeletePos (list, pos_list[0]);
    XtSetArg (al[0], XmNwidth, 193);
    XtSetArg (al[1], XmNheight, 149);
    XtSetValues (list, al, 2);

    new = temp = (unsigned short **) XtMalloc ((num_palettes - 1) * sizeof (unsigned short *));
    new_names = temp_names = (XmString *) XtMalloc ((num_palettes - 1) * sizeof (XmString));

    for (i=0; i<num_palettes; i++)
    {
	if (i == pos_list[0]-1)
	{
	    XtFree ((char *) palettes[i]);
	    XmStringFree (palette_names[i]);
	}
	else
	{
	    *temp = palettes[i];
	    temp++;
	    *temp_names = palette_names[i];
	    temp_names++;
	}
    }

    XtFree ((char *) pos_list);
    XtFree ((char *) palettes);
    XtFree ((char *) palette_names);
    palettes = new;
    palette_names = new_names;
    num_palettes--;

    save_palettes();
}

/* 
 * Writes the data for a single palette to the specified buffer
 */

static void write_palette ( buff, index)
    char *buff;
    int index;
{
    int i;
    char temp[200];
    PaletteResourceList prl;
    unsigned short red, green, blue;

    for (i=0, prl = pr_list; i<pr_num; i++, prl++)
    {
	red   = palettes[index][i*3];	
	green = palettes[index][i*3+1];	
	blue  = palettes[index][i*3+2];	
	sprintf (temp, "%d\t%d\t%d\n", red, green, blue);
	strcat (buff, temp);
    }
}

/*
 * Stores the current palette list resources to the application defaults file
 */

static void save_palettes ()
{
    int i;
    Boolean sep;
    XrmValue value;
    XmStringDirection dir;
    XmStringContext context;
    XmStringCharSet charset;
    PaletteResourceList prl;
    char *buffer, *buffer2, *name, temp[200], res_name[100];
    XrmDatabase db = XrmGetFileDatabase (app_defaults_name);

    if (!db)
    {
	printf ("Couldn't open %s\nExiting...\n", app_defaults_name);
	exit(1);
    }

    need_to_save = FALSE;

    if (need_to_write_palette_colors)
	write_palette_colors(db);

    /* 
     * Allocate a buffer that should be big enough to handle a 
     * palette definition string and a buffer that can handle the 
     * paletteNames resource string
     */

    buffer = XtMalloc (pr_num * 50 + 1000);
    buffer2 = XtMalloc (num_palettes * 50);
    strcpy (buffer2, "");

    for (i=0; i<num_palettes; i++)
    {
	XmStringInitContext (&context, palette_names[i]);
	XmStringGetNextSegment (context, &name, &charset, &dir, &sep);  
	XmStringFreeContext (context);

	/* Add the palette name to the paletteNames string buffer */

	sprintf (temp, "%s\n", name);
	strcat (buffer2, temp);

	/* Store the palette definition */

	strcpy (buffer, "");
	write_palette (buffer, i);

	/* Put the palette definition into the resource database */

	sprintf (res_name, "Custom.%s", name);
	value.addr = buffer;
	value.size = strlen (buffer) + 1;
	XrmPutResource (&db, res_name, XtRString, &value);
    }

    /* Put the paletteNames resource into the database */

    value.addr = buffer2;
    value.size = strlen (buffer2) + 1;
    XrmPutResource (&db, "Custom.paletteNames", XtRString, &value);

    /* Write out the database to disk */

    XrmPutFileDatabase (db, app_defaults_name);
}

/* This routine saves the current set of colors as a palette and adds    */
/* the user specified palette name to the list box.  If the name already */
/* exists in the listbox, then the palette represented by this name is   */
/* replaced by the new one.						 */

static void add_or_modify_palette ( widget, tag, callback_data )
	Widget	widget;
	int *tag;
	XmAnyCallbackStruct *callback_data;
{
    PaletteResourceList prl;
    int new_name = (int) *tag;
    XColor xc;
    char *value;
    XmString item;
    unsigned short *colors;
    int i, position;

    need_to_save = TRUE;
    colors = (unsigned short *) XtMalloc ((pr_num * 3) * sizeof (unsigned short));

    for (i=0, prl = pr_list; i<pr_num; i++, prl++)
    {
	if (prl->value_rec == NULL)
	{
	    colors[i*3] = palettes[current_palette][i*3];	
	    colors[i*3+1] = palettes[current_palette][i*3+1];	
	    colors[i*3+2] = palettes[current_palette][i*3+2];	
	}
	else 
	{
	    xc.pixel = prl->value_rec->pixel[def_index];
	    XQueryColor (display, cmaps[def_index], &xc);
	    colors[i*3] = xc.red;	
	    colors[i*3+1] = xc.green;	
	    colors[i*3+2] = xc.blue;
	}
    }

    if (new_name)
	item = (XmString) DXmCSTextGetString (text);
    else
    	item = (XmString) palette_names[current_palette];	

    position = XmListItemPos (list, item);

    /* If a palette by this name doesn't already exist, add one */

    if (position == 0)
    {
	num_palettes++;
	palettes = (unsigned short **) XtRealloc ((char *) palettes, 
						 num_palettes * sizeof (unsigned short *));
	palette_names = (XmString *) XtRealloc ((char *) palette_names, 
						 num_palettes * sizeof (XmString));
	palettes[num_palettes-1] = colors;
	palette_names[num_palettes-1] = item;
	XmListAddItem (list, item, 0);
	XtSetArg (al[0], XmNwidth, 193);
	XtSetArg (al[1], XmNheight, 149);
	XtSetValues (list, al, 2);
    }
    else
    /* Modify the existing palette */
    {
	XtFree ((char *) palettes[position-1]);
	palettes[position-1] = colors;
    }
	    
    XmListSelectItem (list, item, FALSE);    

    save_palettes();
}

/*
 * Brings up the colormix widget whenever a color button is clicked on
 */

static void color_button_activate ( widget, vl, callback_data )
	Widget	widget;
	ValueList vl;
	XmAnyCallbackStruct *callback_data;
{
    XColor xc;

    current_color = vl;

    xc.pixel = vl->pixel[def_index];
    XQueryColor (display, cmaps[def_index], &xc);

    XtSetArg (al[0],DXmNorigRedValue,xc.red);
    XtSetArg (al[1],DXmNorigGreenValue,xc.green);
    XtSetArg (al[2], DXmNorigBlueValue,xc.blue);
    XtSetArg (al[3],DXmNnewRedValue,xc.red);
    XtSetArg (al[4],DXmNnewGreenValue,xc.green);
    XtSetArg (al[5], DXmNnewBlueValue,xc.blue);
    XtSetArg (al[6], XmNdialogTitle, vl->xm_value_name);
    XtSetValues (colormix, al, 7);    

    if (!XtIsManaged (colormix))
	XtManageChild (colormix);
}

/*
 * Pops up a resource info window next to the arrow button clicked on, or
 * pops it down if the window is already popped up (through the keyboard)
 * Also grabs the pointer and keyboard like a regular pulldown menu while
 * the popup is on the screen.
 */

static void arrow_arm (widget, vl, callback_data )
	Widget	widget;
	ValueList vl;
	XmArrowButtonCallbackStruct *callback_data;
{
    Position dx, dy;
    Window window;
    XKeyEvent *event = (XKeyEvent *) callback_data->event;
   
    if (XtIsManaged (vl->popup))
    {
	XtUnmanageChild(vl->popup);
	XtUngrabPointer (widget, event->time);
	XtUngrabKeyboard (widget, event->time);
	return;
    }

    XtTranslateCoords (widget, 0, 0, &dx, &dy);

    XtSetArg (al[0], XmNx, dx + XtWidth(widget));
    XtSetArg (al[1], XmNy, dy);
    XtSetValues (vl->popup, al, 2);
    XtManageChild (vl->popup);

    if (callback_data->event->type == KeyPress)
    {
	XtGrabPointer (widget, FALSE, ButtonReleaseMask, GrabModeAsync, 
		       GrabModeAsync, None, menu_cursor, event->time);
	XtGrabKeyboard (widget, FALSE, GrabModeAsync, GrabModeAsync, event->time);
    }
}

/*
 * Pops down the resource info popup for the specified arrow button
 */
static void arrow_disarm (widget, vl, callback_data )
	Widget	widget;
	ValueList vl;
	XmArrowButtonCallbackStruct *callback_data;
{
     if (callback_data->event->type == ButtonRelease)
     {
	XtUnmanageChild(vl->popup);
	XtUngrabPointer (widget, callback_data->event->xbutton.time);
	XtUngrabKeyboard (widget, callback_data->event->xbutton.time);
     }
}

/*
 * Manages a message box asking for confirmation
 */
static void exit_cb ( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
    if (need_to_save)
    {
	if (!yes_label)
	{
	    yes_label = XmStringCreateSimple ("Yes");	    
	    no_label = XmStringCreateSimple ("No");	    
	    need_to_save_label = XmStringCreateSimple ("Save changes before exiting?");
	}
	XtSetArg (al[0], XmNokLabelString, yes_label);
	XtSetArg (al[1], XmNcancelLabelString, no_label);
	XtSetArg (al[2], XmNmessageString, need_to_save_label);
	XtSetArg (al[3], XmNdefaultButton, XmMessageBoxGetChild (exit_popup, XmDIALOG_OK_BUTTON));
	XtSetValues (exit_popup, al, 4);
	XtManageChild ((Widget) XmMessageBoxGetChild (exit_popup, XmDIALOG_HELP_BUTTON));
    }
    else
    {
	if (!ok_label)
	{
	    ok_label = XmStringCreateSimple ("OK");	    
	    cancel_label = XmStringCreateSimple ("Cancel");	    
	    no_need_to_save_label = XmStringCreateSimple ("Are you sure?");
	}
	XtSetArg (al[0], XmNokLabelString, ok_label);
	XtSetArg (al[1], XmNcancelLabelString, cancel_label);
	XtSetArg (al[2], XmNmessageString, no_need_to_save_label);
	XtSetArg (al[3], XmNdefaultButton, XmMessageBoxGetChild (exit_popup, XmDIALOG_OK_BUTTON));
	XtSetValues (exit_popup, al, 4);
	XtUnmanageChild ((Widget) XmMessageBoxGetChild (exit_popup, XmDIALOG_HELP_BUTTON));
    }
    XtManageChild (exit_popup);
}

/*
 * Removes the dynamic color customization properties from the root window 
 * and exits.
 */

static void exit_ok ( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
    int i;
    Atom def_atom, col_atom;
    Display *display = XtDisplay (widget);

    if (need_to_save)
	save_palettes();

    def_atom = XInternAtom (display, "DXM_DEFAULTS", TRUE);
    col_atom = XInternAtom (display, "DXM_DYNAMIC_COLORS", TRUE);

    for (i=0; i<num_screens; i++)
    {
	XDeleteProperty (display, RootWindowOfScreen (screens[i]), def_atom);
	XDeleteProperty (display, RootWindowOfScreen (screens[i]), col_atom);
    }
    XSync (display, FALSE);

    exit(0);
}

/*
 * If called when changes need to be saved (and the button label is "No",
 * meaning don't save changes), exits the application.  Otherwise, does
 * nothing (this is when the label is "Cancel").
 */

static void exit_no_save ( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
    Atom def_atom, col_atom;
    Display *display = XtDisplay (widget);

    if (need_to_save)
    {
	def_atom = XInternAtom (display, "DXM_DEFAULTS", TRUE);
	col_atom = XInternAtom (display, "DXM_DYNAMIC_COLORS", TRUE);

	XDeleteProperty (display, DefaultRootWindow (display), def_atom);
	XDeleteProperty (display, DefaultRootWindow (display), col_atom);

	XSync (display, FALSE);

	exit(0);
    }
}

/*
 * This is called when the user hits cancel in the exit box when the palettes
 * haven't been saved.  In this case, the cancel button is actually the help
 * button with the label redefined to be "Cancel"
 */

static void exit_cancel ( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
    XtUnmanageChild (exit_popup);    
}

/*
 * Manages a help box
 */
static void help_cb ( widget, tag, callback_data )
	Widget	widget;
	char    *tag;
	XmAnyCallbackStruct *callback_data;
{
    XmString help_spec;

    if (!help_dialog)
    {
	MrmFetchWidget (s_MrmHierarchy, "help_dialog", toplevel, &help_dialog, &class);
#ifdef VMS
	help_spec = XmStringCreateSimple("decw$examples:custom.hlb");
#else
	help_spec = XmStringCreateSimple("Custom_Help");
#endif
	XtSetArg (al[0], DXmNlibrarySpec, help_spec);
	XtSetValues (help_dialog, al, 1);
	XmStringFree (help_spec);
    }

    if (!XtIsManaged(help_dialog))
	XtManageChild (help_dialog);
}

/*
 * Called when the screen fascimile is clicked on.  Looks at the corresponding
 * image to find out what pixel was clicked on, then brings up the proper
 * resource info popup as though user had hit enter while focus was on that
 * arrow button
 */

static void image_input (widget, tag, data )
	Widget	widget;
	char	*tag;
	XmDrawingAreaCallbackStruct *data;
{
    int i;
    ValueList vl;    
    XEvent *event = data->event;
    Pixel pixel;
    Boolean done = FALSE;
    Position dx, dy;
    Window window;
	        
    if (event)
    {
	if ((event->type == ButtonPress) && (event->xbutton.button == Button1))
	{
	    if (event->xbutton.x < 0 || event->xbutton.x >= iwidth || 
		event->xbutton.y < 0 || event->xbutton.y >= iheight)
		return;

	    if (current_image_popup)
	    {
		XtUnmanageChild (current_image_popup);
		XtUngrabPointer (widget, event->xbutton.time);
		XtUngrabKeyboard (widget, event->xbutton.time);
		current_image_popup = NULL;
	    }

	    pixel = XGetPixel (image, event->xbutton.x, event->xbutton.y);

	    for (i=0, vl=value_list; ((i < num_values) && (!done)); i++, vl++)
		if (pixel == vl->pixel[def_index])
		{
		    XmProcessTraversal (vl->arrow, XmTRAVERSE_CURRENT);
		    XtTranslateCoords (vl->arrow, 0, 0, &dx, &dy);
		    XtSetArg (al[0], XmNx, dx+XtWidth(vl->arrow));
		    XtSetArg (al[1], XmNy, dy);
		    XtSetValues (vl->popup, al, 2);
		    XtManageChild (vl->popup);

		    current_image_popup = vl->popup;

		    XtGrabPointer (widget, FALSE, ButtonPressMask|ButtonReleaseMask, GrabModeAsync, 
				   GrabModeAsync, None, menu_cursor, event->xbutton.time);

		    XtGrabKeyboard (widget, FALSE, GrabModeAsync, GrabModeAsync, event->xbutton.time);

		    done = TRUE;
		}
	}

	if ((event->type == ButtonRelease) && (event->xbutton.button == Button1))
	{
	    if (event->xbutton.x < 0 || event->xbutton.x >= iwidth || 
		event->xbutton.y < 0 || event->xbutton.y >= iheight)
		if (current_image_popup)
		{
		    XtUnmanageChild (current_image_popup);
		    XtUngrabPointer (widget, event->xbutton.time);
		    XtUngrabKeyboard (widget, event->xbutton.time);
		    current_image_popup = NULL;
		}
	}
    }
}
