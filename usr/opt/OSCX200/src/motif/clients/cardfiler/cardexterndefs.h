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
/*
************************************************************************** 
**                   DIGITAL EQUIPMENT CORPORATION                      ** 
**                         CONFIDENTIAL                                 ** 
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   ** 
************************************************************************** 
*/
/* $Id: cardexterndefs.h,v 1.1.4.2 1993/09/09 17:04:50 Susan_Ng Exp $ */
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

#if defined (VMS) && !defined (__DECC)
#pragma nostandard
#endif
#include <DXm/DXmHelpB.h>            
#include <DXm/DXmCSText.h>                 
#include <DXm/DXmCSTextP.h>             
#include <Xm/ArrowB.h>
#include <Xm/BulletinB.h>            
#include <Xm/PushB.h>                
#include <Xm/FileSB.h>               
#include <Xm/LabelP.h>               
#include <Xm/ScrolledWP.h>           
#include <Xm/MainWP.h>               
#include <Xm/MessageB.h>             
#if defined (VMS) && !defined (__DECC)
#pragma standard
#endif

#ifndef NO_XNLS
extern Locale language_id;
#endif

/* All the procedures */

/* HyperInformation Support   (MEMEX) */
extern void CreateConnectionMenu();
#ifndef NO_MEMEX
extern void FreeSurrogates();
extern void CreateDwUi();
extern void MMXexit();
extern void RedoLink();
#endif 
/* routine to test if XUI WM is running. */
extern Boolean DXIsXUIWMRunning();

extern void delproc();
extern int addproc();
extern int opencard();
extern int retrievecards();
extern void exitproc();
extern int storecards();
extern int storeascards();
extern int NoFunction();
extern void mergeproc();
extern int duplicate();
extern int previousproc();
extern int nextproc();
extern int closeproc();
extern int gotoproc();
extern int findproc();
extern int findnextproc();
extern int clearfunc();
extern int undeleteproc();
extern int undoproc();
extern int cutproc();
extern int copyproc();
extern int pasteproc();
extern int printproc();
extern int restoreproc();
extern int restore_settings_proc();
extern int save_settings_proc();
extern int opengraphicproc();
extern int gcloseproc();
extern int makewindows();
extern int initpixmaps();
extern void resize_index();
extern void create_button();
extern void create_dialog();
extern void create_text();
extern void dialog_mapped();
extern void redrawbitmap();
extern void set_focus();
extern void set_focus_now();

extern Boolean parent_realized;
extern Boolean use_c_sort;

extern Display *dpy;
extern XtAppContext app_context;
extern XtIntervalId button_timerid;
extern Boolean timer_went_off;

extern unsigned int highest_card_id_used;
/*#ifndef NO_MEMEX*/
extern Dimension highlight_icon_width, highlight_icon_height;
extern Pixmap memex_highlight_icon;
/*#endif*/
extern Pixmap icon_pixmap;
extern Pixmap sm_icon_pixmap;

extern XrmDatabase user_database;
extern XrmDatabase system_database;
extern XrmDatabase merged_database;

extern FILE *tempfile;

extern MrmHierarchy card_hierarchy;

/* All the widgets */
extern Widget indexparent, cardparent;
extern XmMainWindowWidget indexmainwindow, cardmainwindow;
extern Widget indexworkarea;
extern Widget cardworkarea;
extern Widget cardimagearea;
extern DXmCSTextWidget valuewindow;
extern Widget listbox, svnlist, help_widget;
extern XmMessageBoxWidget exit_dialog, clear_dialog, enter_fname_dialog;
extern XmMessageBoxWidget open_caution, delete_dialog;
extern XmBulletinBoardWidget index_dialog, card_index_dialog;
extern DXmCSTextWidget index_dialog_text, card_index_dialog_text;
extern XmBulletinBoardWidget goto_dialog, card_goto_dialog, find_dialog;
extern DXmCSTextWidget goto_text, find_text;
extern DXmCSTextWidget card_goto_text, card_find_text;
extern XmFileSelectionBoxWidget file_select_dialog;
extern XmMessageBoxWidget message_dialog, card_message_dialog;
extern XmMessageBoxWidget error_message_dialog;
extern Widget print_widget_id, card_print_widget_id;
extern XmPushButtonWidget print_button, print_as_button;
extern XmPushButtonWidget undelete_button, restore_button;
extern XmPushButtonWidget goto_button, card_goto_button;
extern XmPushButtonWidget find_button, find_next_button;
extern XmPushButtonWidget card_find_button, card_find_next_button;
extern XmPushButtonWidget undo_button, cut_button, copy_button;
extern XmPushButtonWidget paste_button, select_graphic_button,
  deselect_graphic_button;
extern XmPushButtonWidget bb_close;
extern XmArrowButtonWidget bb_button1, bb_button2;
extern Widget buttonbox;

extern char find_target[];
extern char goto_target[];
extern char selection_buff[];
extern char text[], bitmap[], undo_bitmap[];
extern int bmp_bytes;
extern Dimension bmp_height, bmp_width, prev_bmp_height, prev_bmp_width;
extern Dimension bitmap_pm_wid, bitmap_pm_hyt;
extern Dimension undo_bmp_height, undo_bmp_width;
extern Dimension prev_height, prev_width, total_prev_height, total_prev_width;
extern int item_id;

/* State variables */
extern GC pixmap_gc;
extern GC bitmap_gc;
extern GC bitmap_clear_gc;
extern Pixel bitmap_fg;
extern Pixel bitmap_bg;
extern Pixmap bitmap_pm;
extern XImage *bitmap_image;

extern Cursor watch_cursor;
extern Cursor card_watch_cursor;

extern int card_mapped;
extern Boolean GrabFocus, CardDeIconified, IconState;
extern Boolean GraphicSelected;
extern Boolean JustCutPasted;
/*extern Boolean JustSelectedGraphic;*/
/*extern Boolean DrawReversed;*/
#ifndef NO_MEMEX
extern int FromIndex;			/* MEMEX */
#endif
extern Boolean PrintFromIndex;
extern int first_time_in;
extern int card_deleted;
extern int file_changed;
extern int bitmap_changed;
extern int find_position;
extern int oldselect;
extern int index_count;
extern int undotype;
extern char undostring[];
extern int undoposition;
extern int file_select_type;

/* Card pointers */
extern card_instance onecard, deleted_card;
extern card_pointer first, last, card_displayed;
extern card_pointer currentselection;
extern card_pointer addcard();
extern struct card_contents_rec temp_card1;
extern XmString printfilename[];
extern XmString us_supplied_data_syntax[];
extern unsigned int delete_flag;

extern int NoFunction();

extern card_pointer tag_array[];
extern XmFontList svnfontlist;
extern int numcards;
extern char cardfiler[];
extern char card_name[];
extern char filename[];
extern char global_fname[];		/* MEMEX */
extern int has_file_name;		/* MEMEX */
extern char default_name[];
extern char *defaults_name;
extern char *system_defaults_name;
extern char print_title[];
extern char clip_label[];
extern char pid_print_file[];
extern char temp_file_name[];
extern char printfile_buf[];
extern Time lasttime;


