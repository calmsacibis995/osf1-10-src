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
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_PRINTEXTRACT.C*/
/* *66   25-JAN-1993 15:37:29 RAMSHAW "QAR #44 - fix saved state for print db"*/
/* *65   10-NOV-1992 17:06:24 KLUM "raise iconified lib window"*/
/* *64    5-NOV-1992 17:53:30 KLUM "fix for no print queues, again"*/
/* *63    2-NOV-1992 16:09:59 KLUM "fixup no_printer scenario"*/
/* *62   14-OCT-1992 13:53:10 KLUM "fixes to BKR_ERROR_DIALOG"*/
/* *61    5-OCT-1992 11:28:05 KLUM "rename print widget id constants"*/
/* *60   20-SEP-1992 18:03:04 BALLENGER "Fix problems with printers menus."*/
/* *59   26-AUG-1992 17:11:56 KLUM ""*/
/* *58   19-AUG-1992 16:45:54 KLUM "pagination bug"*/
/* *57    6-AUG-1992 12:42:05 KLUM "change ps prolog fnames"*/
/* *56   27-JUL-1992 14:03:05 KARDON "Change name of PS print file"*/
/* *55   24-JUL-1992 12:24:49 KLUM "repress print if NO_PRINT alt-prod-name"*/
/* *54   10-JUL-1992 17:14:28 ROSE "Capture widget ID's, add watch cursor"*/
/* *53    4-JUL-1992 21:33:20 KLUM "remove freeing of printer_choice on OK"*/
/* *52    3-JUL-1992 14:48:00 KLUM "prt wgt fixes"*/
/* *51   25-JUN-1992 13:30:48 KLUM "mem from textgetstring copied now"*/
/* *50   24-JUN-1992 14:48:53 KLUM "include bkr_image.h"*/
/* *49   24-JUN-1992 14:48:03 KLUM "fixed call to getcwd"*/
/* *48   23-JUN-1992 16:52:06 KLUM "variable papersize, pass 1"*/
/* *47   22-JUN-1992 17:44:39 KLUM "variable papersize, pass 1"*/
/* *46   18-JUN-1992 16:06:50 KLUM "bubble up print format"*/
/* *45   11-JUN-1992 14:04:02 KLUM "string constants moved to UIL"*/
/* *44   11-JUN-1992 13:48:13 KLUM "string constants moved to UIL"*/
/* *43    4-JUN-1992 16:23:18 KLUM "fixes to printing from popup windows, UCX"*/
/* *42    4-JUN-1992 16:09:46 KLUM "fixes to printing from popup windows"*/
/* *41   26-MAY-1992 11:41:27 KLUM "fixed append to new file msg to ""write new file"""*/
/* *40   22-MAY-1992 15:27:21 KLUM "retain append button setting from text mode"*/
/* *39   20-MAY-1992 17:46:47 KLUM "print popup, UI fixes"*/
/* *38   20-MAY-1992 11:25:04 KLUM "UI rework + printing from topic windows"*/
/* *37   11-MAY-1992 11:34:29 KLUM "options pulldown for print queues now gets refreshed with mode change"*/
/* *36   10-MAY-1992 13:59:17 KLUM "EFT fixes"*/
/* *35    8-MAY-1992 16:06:38 KLUM "various bug-fixes"*/
/* *34    7-MAY-1992 16:57:53 KLUM "various bug-fixes"*/
/* *33    7-MAY-1992 14:57:40 KLUM "various bug-fixes"*/
/* *32    6-MAY-1992 17:51:04 KLUM "various bug-fixes"*/
/* *31    6-MAY-1992 17:46:11 KLUM "various bug-fixes"*/
/* *30   27-APR-1992 13:58:00 KLUM "change default to PS printing"*/
/* *29   18-APR-1992 15:16:29 KLUM "fixup copyright"*/
/* *28   18-APR-1992 03:44:03 KLUM "ULTRIX cp prolog, rm first"*/
/* *27   12-APR-1992 12:07:11 KLUM "ULTRIX cp prolog, rm first"*/
/* *26   30-MAR-1992 20:26:30 BALLENGER "Get PS prolog from the correct places."*/
/* *25   30-MAR-1992 17:48:57 KLUM "UCX, fonts, pass 1"*/
/* *24   27-MAR-1992 13:28:34 KLUM "prolog logical changed"*/
/* *23   25-MAR-1992 14:37:09 KLUM "remove linked PS, converted PS"*/
/* *22   23-MAR-1992 15:29:29 KLUM "don't even ask"*/
/* *21   23-MAR-1992 13:08:08 KLUM "fix #endif problem, really"*/
/* *20   23-MAR-1992 12:50:07 KLUM "fix strange pre-processor bug"*/
/* *19   23-MAR-1992 10:57:27 KLUM "fix missing #endif"*/
/* *18   20-MAR-1992 11:02:22 KLUM "converted PS with fragments"*/
/* *17   11-FEB-1992 11:03:08 KLUM "remove sprite_position event handler (testing)"*/
/* *16    6-FEB-1992 10:20:03 KLUM "started converted PS"*/
/* *15   23-JAN-1992 16:28:38 KLUM "post bl1"*/
/* *14   21-JAN-1992 16:51:01 KLUM "bl1"*/
/* *13   21-JAN-1992 14:49:10 KLUM "bl1"*/
/* *12   13-JAN-1992 15:37:30 KLUM "fixed graying out of ""delete file after printing"" toggle"*/
/* *11   10-JAN-1992 10:35:28 KLUM "use title if symbol not found"*/
/* *10    9-JAN-1992 17:01:38 KLUM "various gray and set fixes"*/
/* *9     8-JAN-1992 16:25:54 KLUM "new ui with toggles"*/
/* *8     2-JAN-1992 09:29:16 KLUM "ps print work"*/
/* *7    25-NOV-1991 13:08:43 KLUM "add Xm header files"*/
/* *6    14-NOV-1991 01:02:57 KLUM "widgetlist -> widgets"*/
/* *5    13-NOV-1991 14:10:27 KLUM "Green devel work"*/
/* *4     1-NOV-1991 13:13:30 BALLENGER "Reintegrate memex support"*/
/* *3    17-SEP-1991 17:36:00 BALLENGER "include function prototype headers"*/
/* *2    17-SEP-1991 10:34:26 KLUM "Un-comment spooling of print job"*/
/* *1    16-SEP-1991 12:40:04 PARMENTER "Print Callbacks"*/
/* VAX/DEC CMS REPLACEMENT HISTORY, Element BKR_PRINTEXTRACT.C*/
#ifndef VMS
 /*
#else
#module BKR_PRINTEXTRACT "V03-0000"
#endif
#ifndef VMS
  */
#endif

#ifdef PRINT

/*
***************************************************************
**  Copyright (c) Digital Equipment Corporation, 1988, 1991  **
**  All Rights Reserved.  Unpublished rights reserved	     **
**  under the copyright laws of the United States.  	     **
**  	    	    	    	    	    	    	     **
**  The software contained on this media is proprietary	     **
**  to and embodies the confidential technology of  	     **
**  Digital Equipment Corporation.  Possession, use, 	     **
**  duplication or dissemination of the software and	     **
**  media is authorized only pursuant to a valid written     **
**  license from Digital Equipment Corporation.	    	     **
**  	    	    	    	    	    	    	     **
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 	     **
**  disclosure by the U.S. Government is subject to 	     **
**  restrictions as set forth in Subparagraph (c)(1)(ii)     **
**  of DFARS 252.227-7013, or in FAR 52.227-19, as  	     **
**  applicable.	    	    	    	    	    	     **
***************************************************************
*/


/*
**++
**  FACILITY:
**
**      Bookreader User Interface ( bkr )
**
**  ABSTRACT:
**
**	bkr_printextract - extract and possibly print selected topic(s)
**                         or book in Linked PS, Converted PS, Text,
**                         or Other PS forms.
**
**
**  AUTHORS:
**
**      Frank Klum
**
**  CREATION DATE:     30-May-1991
**
**  MODIFICATION HISTORY:
**
**  V03 0001 David L Ballenger 15-Aug-1991
**
**           Cleanup for integration with main Bookreader code.
**
**--
**/

#include <Xm/XmP.h>
#include <X11/StringDefs.h>
#include <Xm/BulletinBP.h>
#include <Xm/DialogS.h>
#include <Xm/ScrolledW.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/Text.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/ToggleB.h>
#include <DXm/DECspecific.h>
#include <Mrm/MrmPublic.h>
#include <DXm/DXmPrint.h>    /* Print Widget stuff */
#include <stdio.h>
#include <ctype.h>

#ifdef VMS
#include <ssdef.h>
#else
#include <limits.h>
#endif
#include "br_common_defs.h"  /* common BR #defines */
#include "br_meta_data.h"    /* typedefs and #defines for I/O of BR files */
#include "br_typedefs.h"     /* BR high-level typedefs and #defines */
#include "br_globals.h"      /* BR external variables declared here */
#include "br_malloc.h"       /* BKR_MALLOC, etc defined here */
#include "br_printextract.h" /* print/extract specific constants and types */
#include "bkr_book.h"        /* function prototypes for .c module */
#include "bkr_brtoascii.h"   /* function prototypes for .c module */
#include "bkr_brtoconvertps.h" /* function prototypes for .c module */
#include "bkr_fetch.h"       /* function prototypes for .c module */
#include "bkr_cursor.h"      /* function prototypes for .c module */
#include "bkr_error.h"       /* function prototypes for .c module */
#include "bkr_printextract.h" /* function prototypes for .c module */
#include "bkr_window.h"       /* function prototypes for .c module */
#include "bkr_pr_readpscomments.h" /* function prototypes for .c module */
#include "bkr_pr_resolve_range.h" /* function prototypes for .c module */
#include "bkr_pr_createpsfile.h"  /* function prototypes for .c module */
#include "bkr_pr_util.h"     /* function prototypes for .c module */
#include "bkr_topic_data.h"  /* function prototypes for .c module */

#include "bkr_copy_clipboard.h"

#define LINK_PS FALSE
#define PRINT_BOOK  1
#define PRINT_TOPIC 2

#define STARTPAGE   1
#define ENDPAGE     2

#define ASCII       1
#define ASSOC_PS    2
#define OTHER_PS    3
#define CONVERT_PS  4

#define APPEND      1
#define OVERWRITE   2

#define BKR_UI      1
#define OTHERPS_UI  2

#define N_ADDEDRNGS 64

#define K_PRINTER   1
#define K_FILE      2
#define K_BOTH      3

#ifdef VMS
#define PROLOG_FNAME "DECW$SYSTEM_DEFAULTS:DECW$BOOKREADER_PROLOG.PS"
#else
/* for now just copy to home dir */
#define PROLOG_FNAME "/usr/lib/X11/app-defaults/DXBookreader_prolog.ps"
#endif

#define PRTWGT_DIAG FALSE


/*
** LOCAL ROUTINES
*/
static void print_form();       /* top level for print book/topic */
static void display_msg();      /* fetch and manage info dialog widget telling
                                   what actions were performed */
static void gen_next_fname();   /* generate a unique fname */
static void setup_for_fname();  /* check existense, set label and text */
static int  create_asciifile(); /* extracts ascii file from book file */
static int  create_convertpsfile(); /* extracts PS file from book file */
static int  expand_ascii();   /* routine to expand topic children */
static int  expand_convertps(); /* routine to expand topic children */
static void submit_print();     /* submits print job from print widget */
static void create_printers_menu(); /* create options list of printers */
static void set_default_printer(); /* sets default in the printers option menu */
static void reset_printers_menu(); /* unmanage & free options list of printers */
static void setposfromparent(); /* sets position of child_w from parent_w */
static void pe_format_radiobox(); /* handles format change */
static void fetch_ui_strings();
static void init_pagesizes();


/* The BKR_WINDOW which invoked the current print dialog.
 * The externalref is in br_printextract.h
 */
externaldef(bkrdata) BKR_WINDOW *pe_window = NULL;


static BKR_BOOK_CTX *book;
static Widget     fileselect1_w = (Widget)0;
static Widget     pr_form_w = (Widget)0;
static Widget     print_form_w = (Widget)0;
static Widget     *top_w_ptr;
static Widget     psprint_form_w = (Widget)0;
static Widget     addrange_form_w = (Widget)0;
static Widget     printdialog_w = (Widget)0;
static Widget     *pd_w_ptr = (Widget *)0;
static Widget     pd_s_w = (Widget)0;
static Widget     pd_l_w = (Widget)0;
static Widget     *option_w_ptr = (Widget *)0;
static Widget     option_s_w = (Widget)0;
static Widget     option_l_w = (Widget)0;
static Widget     printers_rc_w;
static Widget     *printers_buttons = (Widget *)0;
static char       **text1_fname = (char **)0;
static char       **text2_fname = (char **)0;
static char       *assoc_psfname = (char *)0;
static char       *other_psfname = "";
static char       *out_assocpsfname = (char *)0;
static char       *out_convertpsfname = (char *)0;
static char       *out_asciifname = (char *)0;
static char       *out_otherpsfname = (char *)0;
static int        gen_assocpsfname = TRUE;
static int        gen_convertpsfname = TRUE;
static int        gen_asciifname = TRUE;
static int        gen_otherpsfname = TRUE;
static int        gen_no = FALSE;
static int        deletefile = TRUE;
static int        print_file = TRUE;
static int        append_mode = FALSE;
static int        save_append_mode = FALSE;
static int        *generated_fname;
static int        format = CONVERT_PS;
static int        new_file;
static int        num_selected = 0;
static int        entirefile;  /* true if print/extract entire book,
                                   false if print/extract topic/s */
static int        otherps_ui;  /* true if long form, false if short form */
static int        addrange_applied; /* true only just after apply */
static int        see_ranges = FALSE; /* if true, show linked PS folio ranges */
static int        n_addedranges;  /* number ranges added while ar window up */
static XmString   *addedranges;   /* list of ranges added */
static int        max_addedranges = N_ADDEDRNGS; /* size of above list */
static int        n_printers;
static int        prev_n_printers;
static XmString   text_printer_choice;
static XmString   ps_printer_choice;
static XmString   *printer_choice;
static XmString   pr_format_list[5];
static XmString   format_choice;
static XmString   *printer_list;
static int        time_count = 1;
static XmString   *selected_strings = (XmString *)0;
static int        n_selected = 0;

static pglist_t  *first_page = (pglist_t *)0; /* ptr to first page struct */
static topic_t   *first_topic = (topic_t *)0; /* ptr to first topic struct */
static cdp_t     cdp;          /* cdp info about input ps file */
static range_t   *first_range; /* ptr to first page-range in list */
static range_t   *range;       /* ptr to a page-range */
static range_t   *f_range;     /* ptr to a page-range for freeing */
static int       n_ranges;     /* number of good range structs in list */
static int       range_type=FOLIO;  /* FOLIO, ORDINAL, or TOPIC */
static FILE      *in_psfp;     /* PS file to read structured comments
                                  from */
static int       ps_logical_def = FALSE;
static int       text_logical_def = FALSE;
static int       from_topic_window = FALSE;
#define GEN_FNAME_LEN 256
static char      gen_filename_string[GEN_FNAME_LEN];
static char      *working_dir;


static char *ps_prt_choice;
static char *text_prt_choice;
static char *fmt_choice;

static long j_stat;
static long j_byte_cnt;

/*--- strings to be fetched from UIL ---*/
static XmString  text_format_string;
static XmString  ps_format_string;
static char      *extract_from;
static char      *submitted_for_print;
static char      *created;
static char      *appended_to;
static char      *and_submitted_for_print;
static char      *slash_delete;
static char      *slash_save;
static char      *dot;
static char      *linked_ps_not_found;
static char      *print_linked_ps;
static char      *write_new;
static char      *append_to_existing;
static char      *write_to_existing;
static char      *overwrite_existing;
static char      *ps_page_range_file;
static char      *linked_ps_file;
static char      *ps_file;
static char      *text_file;
static char      *and_print;
static char      *colon;
static char      *copyright_notice;

#define MAX_PAGE_SIZES 20
typedef struct coord_s
  {
  int     w;
  int     h;
  } coord_t;

static coord_t   pagesize[MAX_PAGE_SIZES];

static int       page_width = 637;
static int       page_height = 825;

externalref Cursor wait_cursor_id;


/* static XmString  pr_printer_str, pr_printer_str_new; */

/*------------------------- fetch_ui_strings ----------------------------
Description: fetches all needed strings from UIL and gets working
             directory (used in gen_next_fname to generate filename)
*-----------------------------------------------------------------------*/
static void fetch_ui_strings()
  {
  char    *cp;

#ifdef VMS
  cp = (char *)getenv("PATH");
  working_dir = (char *)BKR_CALLOC(1,strlen(cp) + 1);
  strcpy(working_dir,cp);
#else
  cp = (char *)getcwd((char *)NULL,PATH_MAX+4);
  working_dir = (char *)BKR_CALLOC(1,strlen(cp) + 4);
  strcpy(working_dir,cp);
  strcat(working_dir,"/");
#endif

  /*---*/
  text_format_string = (XmString)bkr_fetch_literal("text_format_string",
                                                    MrmRtypeCString);
  ps_format_string = (XmString)bkr_fetch_literal("ps_format_string",
                                                  MrmRtypeCString);
  /*---*/

  /*---
  text_format_string = (XmString)XmStringCreate("Text",
                                   XmSTRING_DEFAULT_CHARSET);
  ps_format_string = (XmString)XmStringCreate("PostScript(R)",
                                   XmSTRING_DEFAULT_CHARSET);
  ---*/

  copyright_notice = (char *)bkr_fetch_literal("copyright",MrmRtypeChar8);
  slash_delete = (char *)bkr_fetch_literal("slash_delete",MrmRtypeChar8);
  slash_save = (char *)bkr_fetch_literal("slash_save",MrmRtypeChar8);
  dot = (char *)bkr_fetch_literal("dot",MrmRtypeChar8);
  write_new = (char *)bkr_fetch_literal("write_new",MrmRtypeChar8);
  append_to_existing = (char *)bkr_fetch_literal("append_to_existing",
                                                  MrmRtypeChar8);
  write_to_existing = (char *)bkr_fetch_literal("write_to_existing",
                                                 MrmRtypeChar8);
  overwrite_existing = (char *)bkr_fetch_literal("overwrite_existing",
                                                  MrmRtypeChar8);
  ps_file = (char *)bkr_fetch_literal("ps_file",MrmRtypeChar8);
  text_file = (char *)bkr_fetch_literal("text_file",MrmRtypeChar8);
  and_print = (char *)bkr_fetch_literal("and_print",MrmRtypeChar8);
  colon = (char *)bkr_fetch_literal("colon",MrmRtypeChar8);
  /***
  extract_from = (char *)bkr_fetch_literal("extract_from",
  linked_ps_not_found = (char *)bkr_fetch_literal("linked_ps_not_found",
                                                  MrmRtypeChar8);
  print_linked_ps = (char *)bkr_fetch_literal("print_linked_ps",MrmRtypeChar8);
                                                   MrmRtypeChar8);
  ps_page_range_file = (char *)bkr_fetch_literal("ps_page_range_file",
                                                  MrmRtypeChar8);
  linked_ps_file = (char *)bkr_fetch_literal("linked_ps_file",MrmRtypeChar8);
  ***/

  submitted_for_print = (char *)bkr_fetch_literal("submitted_for_print",
                                                   MrmRtypeChar8);
  created = (char *)bkr_fetch_literal("created",MrmRtypeChar8);
  appended_to = (char *)bkr_fetch_literal("appended_to",MrmRtypeChar8);
  and_submitted_for_print = (char *)bkr_fetch_literal("and_submitted_for_print"
                                    ,MrmRtypeChar8);

  return;
  }


/*------------------------- init_pagesizes -----------------------------
Description: fetches all needed strings from UIL and gets working
             directory (used in gen_next_fname to generate filename)
*-----------------------------------------------------------------------*/
static void init_pagesizes()
  {
  pagesize[DXmSIZE_DEFAULT].w = 637;
  pagesize[DXmSIZE_DEFAULT].h = 825;
  pagesize[DXmSIZE_LETTER].w = 637;
  pagesize[DXmSIZE_LETTER].h = 825;
  pagesize[DXmSIZE_LEDGER].w = 825;
  pagesize[DXmSIZE_LEDGER].h = 1275;
  pagesize[DXmSIZE_LEGAL].w = 637;
  pagesize[DXmSIZE_LEGAL].h = 1050;
  pagesize[DXmSIZE_EXECUTIVE].w = 562;
  pagesize[DXmSIZE_EXECUTIVE].h = 750;
  pagesize[DXmSIZE_A5].w = 435;
  pagesize[DXmSIZE_A5].h = 622;
  pagesize[DXmSIZE_A4].w = 622;
  pagesize[DXmSIZE_A4].h = 877;
  pagesize[DXmSIZE_A3].w = 877;
  pagesize[DXmSIZE_A3].h = 1237;
  pagesize[DXmSIZE_B5].w = 540;
  pagesize[DXmSIZE_B5].h = 757;
  pagesize[DXmSIZE_B4].w = 757;
  pagesize[DXmSIZE_B4].h = 1072;
  pagesize[DXmSIZE_7X9].w = 525;
  pagesize[DXmSIZE_7X9].h = 675;
  pagesize[DXmSIZE_C4_ENVELOPE].w = 675;
  pagesize[DXmSIZE_C4_ENVELOPE].h = 960;
  pagesize[DXmSIZE_C5_ENVELOPE].w = 480;
  pagesize[DXmSIZE_C5_ENVELOPE].h = 675;
  pagesize[DXmSIZE_C56_ENVELOPE].w = 480;
  pagesize[DXmSIZE_C56_ENVELOPE].h = 675;
  pagesize[DXmSIZE_10X13_ENVELOPE].w = 750;
  pagesize[DXmSIZE_10X13_ENVELOPE].h = 975;
  pagesize[DXmSIZE_9X12_ENVELOPE].w = 675;
  pagesize[DXmSIZE_9X12_ENVELOPE].h = 900;
  pagesize[DXmSIZE_BUSINESS_ENVELOPE].w = 309;
  pagesize[DXmSIZE_BUSINESS_ENVELOPE].h = 712;

  return;
  }

/*------------------------- bkr_print_book ----------------------------
Description: Callback for "Print Book" File menu pushbutton.
*-----------------------------------------------------------------------*/
extern void bkr_print_book(widget, tag, data)
  Widget		widget;
  int   		*tag;
  XmAnyCallbackStruct	*data;
  {
  BMD_SHELF_ID          shelf_id;
  BKR_NODE              *node;
  unsigned              entry_id;
  char                  *error_string;


  /*--- give msg and return if don't have license ---*/
  if(!bkrplus_g_allow_print)
    {
    bkr_display_need_license_box ();
    return;
    }

  entirefile = TRUE;

  num_selected = bkr_library->u.library.num_selected;

  /*--- if nothing selected, error dialog box ---*/
  if(num_selected <= 0)
    {
    BKR_ERROR_DIALOG("PE_NO_SELECTION",bkr_library->appl_shell_id);
    return;
    }

  if(num_selected > 1)
    {
    /* printf("\n only a single book may be selected for print: \
select a single book\n"); */
    BKR_ERROR_DIALOG("PE_SELECT_SINGLE_BOOK",bkr_library->appl_shell_id);
    return;
    }

  node = (BKR_NODE_PTR)bkr_library->u.library.selected_entry_tags[0];

  if(node->entry_type != BKR_BOOK_FILE)
    {
    BKR_ERROR_DIALOG("PE_NOT_BOOK",bkr_library->appl_shell_id);
    /* printf("\n selected entry for print must be a book, not a shelf. Expand\
the shelf and select a single book\n"); */
    return;
    }

  shelf_id = node->parent->u.shelf.id;
  entry_id = node->entry_id;

  if(!(book = bkr_book_get(NULL, shelf_id, entry_id)))
    {
    BKR_ERROR_DIALOG("PE_BKR_BOOK_GET_FAIL",bkr_library->appl_shell_id);
    return;
    }

  if(bri_book_no_print(book->book_id))
    {
    BKR_ERROR_DIALOG("PE_BOOK_NO_PRINT",bkr_library->appl_shell_id);
    bkr_book_free(book);
    return;
    }

  bkr_cursor_display_wait(ON);

  otherps_ui = FALSE;

  print_form(bkr_library->appl_shell_id);

  bkr_cursor_display_wait(OFF);

  return;
  }


/*------------------------- bkr_printpopup -----------------------------
Description: Callback for "Print Topic" File menu pushbutton from formal
             topic window.
*-----------------------------------------------------------------------*/
extern void bkr_printpopup(widget, tag, data)
  Widget		widget;
  int   		*tag;
  XmAnyCallbackStruct	*data;
  {
  char                  *error_string;

  /*--- give msg and return if don't have license ---*/
  if(!bkrplus_g_allow_print)
    {
    bkr_display_need_license_box ();
    return;
    }

  entirefile = FALSE;
  pe_window = bkr_window_find(widget);
  book = pe_window->shell->book;

  if(bri_book_no_print(book->book_id))
    {
    BKR_ERROR_DIALOG("PE_BOOK_NO_PRINT",pe_window->appl_shell_id);
    return;
    }

  otherps_ui = FALSE;
  from_topic_window = TRUE;

  bkr_cursor_display_wait(ON);

  print_form(pe_window->appl_shell_id);

  bkr_cursor_display_wait(OFF);
                
  return;
  }


/*------------------------- bkr_printtopic_window ------------------------
Description: Callback for "Print Topic" File menu pushbutton.
*-----------------------------------------------------------------------*/
extern void bkr_printtopic_window(widget, tag, data)
  Widget		widget;
  int   		*tag;
  XmAnyCallbackStruct	*data;
  {
  char                  *error_string;

  /*--- give msg and return if don't have license ---*/
  if(!bkrplus_g_allow_print)
    {
    bkr_display_need_license_box ();
    return;
    }

  entirefile = FALSE;
  pe_window = bkr_window_find(widget);
  book = pe_window->shell->book;

  if(bri_book_no_print(book->book_id))
    {
    BKR_ERROR_DIALOG("PE_BOOK_NO_PRINT",pe_window->appl_shell_id);
    return;
    }

  otherps_ui = FALSE;
  from_topic_window = TRUE;

  bkr_cursor_display_wait(ON);

  print_form(pe_window->appl_shell_id);

  bkr_cursor_display_wait(OFF);
                
  return;
  }


/*------------------------- bkr_print_topic ----------------------------
Description: Callback for "Print Topic" File menu pushbutton.
*-----------------------------------------------------------------------*/
extern void bkr_print_topic(widget, tag, data)
  Widget		widget;
  int   		*tag;
  XmAnyCallbackStruct	*data;
  {
  char                  *error_string;

  /*--- give msg and return if don't have license ---*/
  if(!bkrplus_g_allow_print)
    {
    bkr_display_need_license_box ();
    return;
    }

  entirefile = FALSE;

  /* Since only one print/extract operation is active at one time,
   * we can just store the pointer to the bookreader window doing the
   * print/extract in pe_window.
   */
  pe_window = bkr_window_find(widget);

  book = pe_window->shell->book;
  num_selected = pe_window->u.selection.num_selected;

  /*--- if nothing selected, error dialog box ---*/
  if(num_selected <= 0)
    {
    BKR_ERROR_DIALOG("PE_NO_SELECTION",pe_window->appl_shell_id);
    return;
    }

  if(bri_book_no_print(book->book_id))
    {
    BKR_ERROR_DIALOG("PE_BOOK_NO_PRINT",pe_window->appl_shell_id);
    return;
    }

  bkr_cursor_display_wait(ON);

  otherps_ui = FALSE;
  from_topic_window = FALSE;

  print_form(pe_window->appl_shell_id);

  bkr_cursor_display_wait(OFF);

  return;
  }


/*------------------------- bkr_print_otherps ----------------------------
Description: Callback for "Print Topic" File menu pushbutton.
*-----------------------------------------------------------------------*/
extern void bkr_print_otherps(widget, tag, data)
  Widget		widget;
  int   		*tag;
  XmAnyCallbackStruct	*data;
  {
  char                  *error_string;

  bkr_cursor_display_wait(ON);

  otherps_ui = TRUE;

  print_form(bkr_library->appl_shell_id);

  bkr_cursor_display_wait(OFF);

  return;
  }


/*------------------------- print_form ----------------------------------
Description: "Print Book"/"Print Topic" top level
*-----------------------------------------------------------------------*/
static void print_form(parent_w)
  Widget                parent_w;
  {
  unsigned int          argcnt;
  Arg                   arglist[8];
  XmString              fname;
  unsigned	        status;
  MrmType               dummy_class;
  long                  byte_cnt, l_status;
  char                  *error_string;
  Widget                filename_text_w;
  Widget                w_on;
  Widget                w_off1;
  Widget                w_off2;
  int                   file_type;
  char                  *book_fname;
  char                  *top_w_string;
  char                  *cp;    
  int                   value;
  FILE                  *fp;
  XmString              prt_choice;

#ifdef VMS
  ps_logical_def = getenv("DECW$PRINTER_FORMAT_PS");
  text_logical_def = getenv("DECW$PRINTER_FORMAT_TEXT");
#else
  ps_logical_def = getenv("DECwPrinterFormatPS");
  text_logical_def = getenv("DECwPrinterFormatText");
#endif

  first_range = (range_t *)0;

  /*--- setup ptrs to the appropriate widgets for long or short forms ---*/
  if(otherps_ui)
    {
    top_w_ptr = &print_form_w;
    top_w_string = "print_form";
    option_w_ptr = &option_l_w;
    pd_w_ptr = &pd_l_w;
    assoc_psfname = (char *)0;
    cdp.n_pages = 0;
    }
  else
    {
    top_w_ptr = &pr_form_w;
    top_w_string = "pr_form";
    option_w_ptr = &option_s_w;
    pd_w_ptr = &pd_s_w;

    /*--- allocate and setup linked PS filename --s-*/
    book_fname = book->filename;
    assoc_psfname = (char *) BKR_CALLOC(1,strlen(book_fname)
                             + strlen("BKR_PSDIR:") + 4);
    strcpy(assoc_psfname,"BKR_PSDIR:");
    strcat(assoc_psfname,book_fname);
    if(cp = strrchr(assoc_psfname,'.'))
      *cp = '\0';
    strcat(assoc_psfname,".ps");
    }
  /*--- if BR Print main print widget doesn't already exist... ---*/
  if(!*top_w_ptr)
    {
    MRM_FETCH(top_w_string,bkr_library->appl_shell_id,top_w_ptr);  /*--- fetch it ---*/

    /* Store the widget ID of the toplevel print box with the library window,
       since there is only one per session */
    bkr_library->widgets[W_PR_BOX] = *top_w_ptr;

    fetch_ui_strings();
    init_pagesizes();

    /*--- set up defaults ---*/
    /* set default to "assoc ps" */
    if(!otherps_ui)
      {
#if LINK_PS
      format = ASSOC_PS;
      w_on = bkr_library->widgets[W_PR_LINKPS_TB];   /* "Linked PS" */
      w_off1 = bkr_library->widgets[W_PR_TEXT_TB];  /* "Text" */
      w_off2 = bkr_library->widgets[W_PR_CONVERTPS_TB];  /* "Convert PS" */
      argcnt = 0;
      SET_ARG(XmNset,TRUE);
      XtSetValues(w_on,arglist,argcnt);
      argcnt = 0;
      SET_ARG(XmNset,FALSE);
      XtSetValues(w_off1,arglist,argcnt);
      XtSetValues(w_off2,arglist,argcnt);
#else
      format = CONVERT_PS;
      argcnt = 0;
      SET_ARG(XmNset,TRUE);
      XtSetValues(bkr_library->widgets[W_PR_CONVERTPS_TB],arglist,argcnt);
      argcnt = 0;
      SET_ARG(XmNset,FALSE);
      XtSetValues(bkr_library->widgets[W_PR_TEXT_TB],arglist,argcnt);
#endif
      }
    }  /* end if !*top_w_ptr */

  if(otherps_ui)
    {
    text1_fname = &out_otherpsfname;
    text2_fname = &other_psfname;
    generated_fname = &gen_otherpsfname;
    append_mode = FALSE;
    }
  else
    {
    if(format == ASSOC_PS)
      {
      if(!(fp = fopen(assoc_psfname,"r"))) /* try to open for read */
        {  /*  file not found, set default to Text */
        format = ASCII;
        w_on =  bkr_library->widgets[W_PR_TEXT_TB];   /* "Text" */
        w_off1 = bkr_library->widgets[W_PR_LINKPS_TB];  /* "Linked PS" */
        w_off2 = bkr_library->widgets[W_PR_CONVERTPS_TB];  /* "Convert PS" */
        argcnt = 0;
        SET_ARG(XmNset,TRUE);
        XtSetValues(w_on,arglist,argcnt);
        argcnt = 0;
        SET_ARG(XmNset,FALSE);
        XtSetValues(w_off1,arglist,argcnt);
        XtSetValues(w_off2,arglist,argcnt);
        }
      else
        fclose(fp);
      }

    if(format == ASCII)
      {
      text1_fname = &out_asciifname;
      generated_fname = &gen_asciifname;
      }
    else if(format == CONVERT_PS)
      {
      text1_fname = &out_convertpsfname;
      generated_fname = &gen_convertpsfname;
      }
    else if(format == ASSOC_PS)
      {
      if(!entirefile)
        {
        text1_fname = &out_assocpsfname;
        text2_fname = &assoc_psfname;
        generated_fname = &gen_assocpsfname;
        }
      else
        {
        text1_fname = &assoc_psfname;
        generated_fname = &gen_no;
        }
      }

    if(entirefile && (format == ASSOC_PS))
    /*--- file shown in text1 gets spooled, can only be "Send to Printer" ---*/
      {
      /*--- form is short form to get entirefile ---*/
      /*--- gray out "Delete File After Printing" and "Append" options ---*/
      deletefile = FALSE;
      print_file = TRUE;
      argcnt = 0; /* set "Send to Printer" to be sensitive, on */
      SET_ARG(XmNsensitive,TRUE);
      SET_ARG(XmNset,TRUE);
      XtSetValues(bkr_library->widgets[W_PR_SEND_TO_PRINTER_TB],arglist,argcnt);
      argcnt = 0; /* gray and set off "Save to File", "Both" */
      SET_ARG(XmNsensitive,FALSE);
      SET_ARG(XmNset,FALSE);
      XtSetValues(bkr_library->widgets[W_PR_SAVE_TO_FILE_TB],arglist,argcnt);
      XtSetValues(bkr_library->widgets[W_PR_BOTH_TB],arglist,argcnt);
      }

#if LINK_PS
    if(!entirefile)
      {  /* "Save to File" and "Both" may have
            been grayed out from previous, set to be sensitive */
      argcnt = 0;
      SET_ARG(XmNsensitive,TRUE);
      XtSetValues(bkr_library->widgets[W_PR_SAVE_TO_FILE_TB],arglist,argcnt);
      XtSetValues(bkr_library->widgets[W_PR_BOTH_TB],arglist,argcnt);
      }

    /*--- set sensitivity of "Display PS Ranges" toggle ---*/
    value = ((format == ASSOC_PS) && !entirefile)? TRUE: FALSE;
    argcnt = 0;
    SET_ARG(XmNsensitive,value);
    SET_ARG(XmNset,FALSE);
    XtSetValues(bkr_library->widgets[W_PR_32],arglist,argcnt);
#endif
    }  /* end if !otherps_ui */

  /*--- fetch the print dialog widget ---*/
  if(!printdialog_w)
    {
    MRM_FETCH("printdialog",*top_w_ptr,&printdialog_w);

    /* Store the widget ID of the Print widget with the library window,
       since there is only one per session */
    bkr_library->widgets[W_PR_OPTIONS_BOX] = printdialog_w;

    /*--- setup format list and format choice in print widget ---*/
    pr_format_list[0] = format_choice = ps_format_string;
    pr_format_list[1] = text_format_string;

    argcnt = 0;
    SET_ARG(DXmNprintFormatList, pr_format_list);
    SET_ARG(DXmNprintFormatCount, 2);
    SET_ARG(DXmNprintFormatChoice, format_choice);
    XtSetValues (printdialog_w, arglist, argcnt);

    /*--- get ps printer choice ---*/
    argcnt = 0;
    SET_ARG(DXmNprinterList, &printer_list);
    SET_ARG(DXmNprinterCount, &n_printers);
    SET_ARG(DXmNprinterChoice, &prt_choice);
    XtGetValues (printdialog_w, arglist, argcnt);
    ps_printer_choice = (XmString)XmStringCopy(prt_choice);

    /*--- set format choice to text and get text printer choice ---*/
    argcnt = 0;
    SET_ARG(DXmNprintFormatChoice, text_format_string);
    XtSetValues (printdialog_w, arglist, argcnt);

    argcnt = 0;
    SET_ARG(DXmNprinterList, &printer_list);
    SET_ARG(DXmNprinterCount, &n_printers);
    SET_ARG(DXmNprinterChoice, &prt_choice);
    XtGetValues (printdialog_w, arglist, argcnt);
    text_printer_choice = (XmString)XmStringCopy(prt_choice);

    /*--- set format choice back to what it should be ---*/
    argcnt = 0;
    SET_ARG(DXmNprintFormatChoice, format_choice);
    XtSetValues(printdialog_w,arglist,argcnt);

    argcnt = 0;
    SET_ARG(DXmNprinterList, &printer_list);
    SET_ARG(DXmNprinterCount, &n_printers);
    SET_ARG(DXmNprinterChoice, &prt_choice);
    XtGetValues (printdialog_w, arglist, argcnt);

    printer_choice = (format == ASCII)? &text_printer_choice:
                                        &ps_printer_choice;
#if PRTWGT_DIAG
    /*------*/
    ps_prt_choice = (char *)DXmCvtCStoOS(ps_printer_choice,&j_byte_cnt,&j_stat);
    text_prt_choice = (char *)DXmCvtCStoOS(text_printer_choice,&j_byte_cnt,&j_stat);
    fmt_choice = (char *)DXmCvtCStoOS(format_choice,&j_byte_cnt,&j_stat);
    printf("\nprint_form:\n");
    if(printer_choice == &text_printer_choice)
      printf("  format=TEXT\n");
    else if(printer_choice == &ps_printer_choice)
      printf("  format=PS\n");
    printf("  ps_printer_choice = %s\n",ps_prt_choice);
    printf("  text_printer_choice = %s\n",text_prt_choice);
    printf("  fmt_choice = %s\n",fmt_choice);
    /*------*/
#endif
    }

  if(*generated_fname || !*text1_fname)
    gen_next_fname();  /*--- generate a filename ---*/

  /*--- setup label and filename text widget in print dialog widget ---*/
  setup_for_fname();

  /*--- set up to get print queue choice - if VMS and logicals are
        defined, use and retain settings for print queue list based 
        on PS and Text formats
  ---*/

  /*--- manage top level widget ---*/
  RAISE_WINDOW(bkr_library->appl_shell_id);

  setposfromparent(parent_w,*top_w_ptr,80,30);
  XtManageChild(*top_w_ptr);

  /*--- create option menu of all the print queue names ---*/
  if ((*pd_w_ptr == (Widget)0) || (*option_w_ptr == NULL))
  {
      create_printers_menu();
  }

  return;
  }


/*-------------------- bkr_pe_ok_button ---------------------------------
Description: BR Print main widget "Ok" pushbutton callback. If Text
             print, create ASCII file. If PostScript print, create
             converted PS file.
------------------------------------------------------------------------*/
extern void bkr_pe_ok_button(widget,tag,reason)
  Widget                widget;
  int			*tag;
  XmAnyCallbackStruct   *reason;
  {
  unsigned int          argcnt;
  Arg                   arglist[4];
  int    	        status;
  MrmType               dummy_class;
  Position              xpos,ypos,x,y;
  /*^^ char                  buf[256]; ^^*/
  int                   n_ranges;
  int                   i,j;
  long                  l_status;
  long                  byte_cnt;
  char                  *psfname;
  XmString              string;
  BMD_BOOK_ID           bkid;
  BKR_DIR_ENTRY         *dir_ptr;
  char                  *symbol;
  char                  *title;
  char                  *value;
  char                  *book_fname;
  char                  *cp;
  int                   err_already;
  int                   error;
  int                   y_off;
  char                  *error_string;
  Widget                parent_w;
/* FILE  *v_fp; */

  parent_w = *top_w_ptr;

  if(!**text1_fname)
    {
    BKR_ERROR_DIALOG("CS_IS_NULL_ERROR",parent_w);
    return;
    }

  /*--- put wait cursor in this window ---*/
  XDefineCursor(bkr_display,XtWindow(*top_w_ptr),wait_cursor_id);

  XFlush(bkr_display);

  /***
  widget = otherps_ui? bkr_library->widgets[W_PR_1]:
                      bkr_library->widgets[W_PR_22];
  ***/

  widget = bkr_library->widgets[W_PR_F_NAME_TEXT];
  *text1_fname = XmTextGetString(widget);

  num_selected = (entirefile)? bkr_library->u.library.num_selected:
                                pe_window->u.selection.num_selected;


#if FALSE /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
  if(otherps_ui || (format == ASSOC_PS))
    {  /*--- maybe read structured comments from psfile, maybe resolve
             bookreader ranges, maybe display range selection widget ---*/

    psfname = (otherps_ui)? other_psfname: assoc_psfname;

    /*--- get other_psfname from text widget ---*/

    /*--- try to open ps file for read ---*/
    if(!(in_psfp = fopen(psfname,"r")))
      {
      if(error_string = (char *) bkr_fetch_literal("PE_CANT_OPEN_READ",ASCIZ))
        {
        sprintf( errmsg, error_string, psfname );
        bkr_error_modal( errmsg,  pe_window->appl_shell_id);
        XtFree( error_string );
        }
      /* printf("\n could not open %s for read\n",psfname); */
      XUndefineCursor(bkr_display,XtWindow(*top_w_ptr));
      return;
      }

    /*--- psfile exists ---*/
    if(!otherps_ui && entirefile)   /* "Print Book...", linked PS */
      {
      fclose(in_psfp);
      /*--- print an entire existing file, give message, and done ---*/
      submit_print(psfname,deletefile);
      display_msg(psfname);
      return;
      }

    /* other_psfname opened successfully for read */
    /*--- put wait cursor in this window ---*/
    XDefineCursor(bkr_display,XtWindow(*top_w_ptr),wait_cursor_id);

    /*--- read structured comments into structs ---*/
    first_page = (pglist_t *)0;
    first_topic = (topic_t *)0;
    readpscomments(in_psfp,&cdp,&first_page,&first_topic);
    fclose(in_psfp);

    /*--- fetch psprint_form_w if needed ---*/
    if(!psprint_form_w && (otherps_ui || see_ranges))
      {
      MRM_FETCH("psprint_form",*top_w_ptr,&psprint_form_w);
      }

    /*--- resolve and setup selected bkr_topics into list elements ---*/
    if(!otherps_ui && num_selected && (format == ASSOC_PS))
      {
      bkid = book->book_id;
      err_already = FALSE;

      /* v_fp = fopen("symbols.txt","w"); */

      for(i=0; i < num_selected; ++i,++dir_ptr)
        {
        dir_ptr = (BKR_DIR_ENTRY *)
                   pe_window->u.selection.selected_entry_tags[i];
        value = (char *)0;
        if(title = dir_ptr->title)
          {
          if(*title)
            {
            value = title;
            /* fprintf(v_fp,"\n title = %s\n",title); */
            /* printf("\n title = %s\n",title); */
            }
          }
        if(symbol = (char *)bri_page_chunk_symbol(bkid,dir_ptr->u.entry.target))
          {
          if(*symbol)
            {
            /* printf(" symbol = %s\n",symbol); */
            /* fprintf(v_fp," symbol = %s\n",symbol); */
            value = symbol;
            }
          }
        if(value)  /* if no symbol, but title, use title */
          {
          if(!(range = resolve_range(&cdp,&first_range,
                                     first_page,first_topic,value,
                                     value,TOPIC,&status)))
            {
            if(!err_already)
              {
              err_already = TRUE;
              BKR_ERROR_DIALOG("PE_BAD_RANGE",parent_w);
              }
            }
          }
        }    /* end for loop on num_selected */

      /* fclose(v_fp); */

      /*--- sort ranges on ord_start field ---*/
      rangesort(first_range);

      /*--- condense contiguous ranges ---*/
      condense_ranges(&cdp,&first_range);

      /*--- add condensed list of ranges to list widget ---*/
      if(see_ranges)
        {
        for(range=first_range; range; range=range->next)
          addrange_strings(range);
        }                
      }  /* end if(num_selected && (format == ASSOC_PS)) */

    if(otherps_ui || see_ranges)
      {
      /*--- set up and manage range selection widget ---*/

      /*--- set up label to have psfname ---*/
      strcpy(buf,extract_from);
      strcat(buf,psfname);
      string = DXmCvtOStoCS(buf,&byte_cnt,&l_status);
      argcnt = 0;
      SET_ARG(XmNlabelString,string);
      /* XtSetValues(bkr_library->widgets[W_PR_17],arglist,argcnt); */
      XmStringFree(string);

      /*--- number of pages selected to print ---*/
      sprintf(buf,"%d",cdp.n_pages);
      string = DXmCvtOStoCS(buf,&byte_cnt,&l_status);
      argcnt = 0;
      SET_ARG(XmNlabelString,string);
      /* XtSetValues(bkr_library->widgets[W_PR_18],arglist,argcnt); */
      XmStringFree(string);

      /*--- total number of pages in book ---*/
      sprintf(buf,"%d",cdp.max_ordinal);
      string = DXmCvtOStoCS(buf,&byte_cnt,&l_status);
      argcnt = 0;
      SET_ARG(XmNlabelString,string);
      /* XtSetValues(bkr_library->widgets[W_PR_19],arglist,argcnt); */
      XmStringFree(string);

      y_off = (otherps_ui)? 160: 125;

      setposfromparent(*top_w_ptr,psprint_form_w,0,y_off);

      XtManageChild(psprint_form_w);

      }
    else  /*--- don't display PS ranges widget, create file and give msg ---*/
      {
      cdp.ps_fname = (format == ASSOC_PS)? assoc_psfname: other_psfname;
      cdp.cdp_fname = (format == ASSOC_PS)? out_assocpsfname: out_otherpsfname;
      if(error = createpsfile(&cdp,first_range))
        {
        BKR_ERROR_DIALOG("PE_ASSOC_PSFILE_CREATE",parent_w);
        }
      free_range_mem(&first_range,&first_page,&first_topic,
                     (Widget)0);
      if(!error)
        display_msg(cdp.cdp_fname);
      }
    }    /* end if(otherps_ui || (format == ASSOC_PS)) */
#endif /*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

  /*^^ else ^^*/ if(format == ASCII)
    {  /* extract to text and print book or topic(s) */

    /*--- create the ascii extraction file ---*/
    if(error = create_asciifile())
      {
      BKR_ERROR_DIALOG("PE_TEXTPRINT_ERROR",parent_w);
      XUndefineCursor(bkr_display,XtWindow(*top_w_ptr));
      return;
      }

    /*--- submit print job ---*/
    if(print_file)
      submit_print(out_asciifname,deletefile);

    display_msg(out_asciifname);
    }

  else if(format == CONVERT_PS)  /* extract to PS and print book or topic(s) */
    {
    /*--- create the converted PS file ---*/
    if(error = create_convertpsfile())
      {
      BKR_ERROR_DIALOG("PE_PSPRINT_ERROR",parent_w)
      XUndefineCursor(bkr_display,XtWindow(*top_w_ptr));
      return;
      }

    /*--- submit print job ---*/
    if(print_file)
      submit_print(out_convertpsfname,deletefile);

    display_msg(out_convertpsfname);
    }

  return;
  }


/*------------------ bkr_pe_options_button ------------------------------
Description: BR Print main widget "Options" pushbutton callback -
             manages print dialog widget to set queue, etc.
------------------------------------------------------------------------*/
extern void bkr_pe_options_button(widget,tag,reason)
  Widget                widget;
  int			*tag;
  XmAnyCallbackStruct   *reason;
  {

  /*--- manage print widget ---*/
  XtManageChild(printdialog_w);

  return;
  }


/*-------------------- display_msg --------------------------------------
Description: BR Print - fetch and manage information dialog widget which
             describes what actions were performed on completion of the
             extract/print operation.
------------------------------------------------------------------------*/
static void display_msg(fname)
  char                      *fname;
  {
  unsigned int              argcnt;
  Arg                       arglist[8];
  XmString                  filecreatedmsg;
  char                      msg_buf[512];
  long                      stat;
  long                      byte_cnt;
  unsigned int              status;
  MrmType                   dummy_class;
  Widget                    cancel_button;
  Widget                    help_button;
  char                      workbuf[128];
  char                      *printer_string;
  static Widget             infodialog_w = (Widget)0;

  if(n_printers <= 0)
    {
    print_file = deletefile = FALSE;
    }

  /*--- fetch info dialog widget ---*/
  if(!infodialog_w)
    {
    MRM_FETCH("infodialog",*top_w_ptr,&infodialog_w);
    }

  /*--- Get rid of the Cancel and Help buttons ---*/
  if(cancel_button = XmMessageBoxGetChild(infodialog_w,
                                          XmDIALOG_CANCEL_BUTTON))
    XtUnmanageChild(cancel_button);
  if(help_button = XmMessageBoxGetChild(infodialog_w,
                                        XmDIALOG_HELP_BUTTON))
    XtUnmanageChild(help_button);

  /*--- set up message telling what was done ---*/
  strcpy(msg_buf,fname);

  strcat(msg_buf,"\n");

  if(entirefile && ((format == ASSOC_PS) || (format == OTHER_PS)))
    strcat(msg_buf,submitted_for_print);
  else
    {
    if(new_file || !append_mode)
      strcat(msg_buf,created);
    else if(append_mode)
      strcat(msg_buf,appended_to);
    if(print_file)
      {
      strcpy(workbuf,and_submitted_for_print);
      if(deletefile)
        strcat(workbuf,slash_delete);
      else
        strcat(workbuf,slash_save);
      strcat(msg_buf,workbuf);

      printer_string = (char *)DXmCvtCStoOS(*printer_choice,&byte_cnt,&stat);

      if(strlen(printer_string) < 120)
        {
        sprintf(workbuf," on %s",printer_string);
        strcat(msg_buf,workbuf);
        }
      XtFree(printer_string);
      }
    }
  strcat(msg_buf,dot);

  /*--- convert C string to compound string ---*/
  filecreatedmsg = DXmCvtOStoCS(msg_buf,&byte_cnt,&stat);

  /*--- set msg arg ---*/
  argcnt = 0;
  SET_ARG(XmNmessageString,filecreatedmsg);
  XtSetValues(infodialog_w, arglist, argcnt);
  XmStringFree(filecreatedmsg);

  /*--- manage widget ---*/
  /* setposfromparent(*top_w_ptr,infodialog_w,0,130); */

  XUndefineCursor(bkr_display,XtWindow(*top_w_ptr));

  if(assoc_psfname)
    {
    BKR_CFREE(assoc_psfname);
    }
  assoc_psfname = (char *)0;

  XtUnmanageChild(*top_w_ptr);

  XtManageChild(infodialog_w);

  return;
  }


/*-------------------- bkr_pe_infodialog_ok -----------------------------
Description: BR Print - information dialog box "Ok" pushbutton callback
------------------------------------------------------------------------*/
extern void bkr_pe_infodialog_ok(widget,tag,reason)
  Widget                widget;
  int			*tag;
  XmAnyCallbackStruct   *reason;
  {
  int                   i;

  /* XmStringFree(text_printer_choice); */
  /* XmStringFree(ps_printer_choice); */

  if(entirefile)
    bkr_book_free(book);

  bkr_cursor_display_wait(OFF);

  return;
  }


/*-------------------- bkr_pe_cancel_button -----------------------------
Description: BR Print - main widget "Cancel" pushbutton callback.
------------------------------------------------------------------------*/
extern void bkr_pe_cancel_button(widget,tag,reason)
  Widget                widget;
  int			*tag;
  XmAnyCallbackStruct   *reason;
  {

  XUndefineCursor(bkr_display,XtWindow(*top_w_ptr));

  XtUnmanageChild(*top_w_ptr);

  if(assoc_psfname)
    {
    BKR_CFREE(assoc_psfname);
    }
  assoc_psfname = (char *)0;

  bkr_cursor_display_wait(OFF);

  return;
  }


/*--------------------- bkr_pe_printdialog_ok -----------------------
Description: BR Print - print dialog widget "Ok" pushbutton callback
--------------------------------------------------------------------*/
extern void bkr_pe_printdialog_ok(w, tag, reason)
  Widget		w;
  int			*tag;
  XmAnyCallbackStruct   *reason;
  {
  int                   argcnt;
  int                   i;
  int                   new_format;
  Widget                on_widget;
  Widget                off_widget;
  int                   size_k;
  Arg                   arglist[4];
  XmString              pr_format;
  XmString              prt_choice;
  long                  bc, status;


char *p_format,*ps_format;

  /* bkr_cursor_display_wait( ON ); */

  argcnt = 0;
  SET_ARG(DXmNpageSize, &size_k);
  XtGetValues (printdialog_w, arglist, argcnt);

  page_width = pagesize[size_k].w;
  page_height = pagesize[size_k].h;

#if PRTWGT_DIAG
  /*------*/
  fmt_choice = (char *)DXmCvtCStoOS(format_choice,&j_byte_cnt,&j_stat);
  ps_prt_choice = (char *)DXmCvtCStoOS(ps_printer_choice,&j_byte_cnt,&j_stat);
  text_prt_choice = (char *)DXmCvtCStoOS(text_printer_choice,&j_byte_cnt,&j_stat);
  fmt_choice = (char *)DXmCvtCStoOS(format_choice,&j_byte_cnt,&j_stat);
  printf("\nprintdialog_ok start:\n");
  if(printer_choice == &text_printer_choice)
    printf("  format=TEXT\n");
  else if(printer_choice == &ps_printer_choice)
    printf("  format=PS\n");
  printf("  ps_printer_choice = %s\n",ps_prt_choice);
  printf("  text_printer_choice = %s\n",text_prt_choice);
  printf("  fmt_choice = %s\n",fmt_choice);
  /*------*/
#endif

  /*--- check to see if format was changed from the print widget
        (as opposed to changing format from the radiobox, top-level)
        If it was, update the radiobox setting and do all the other
        stuff we do when the format changes ---*/
  argcnt = 0;
  SET_ARG(DXmNprintFormatChoice, &pr_format);
  XtGetValues(printdialog_w,arglist,argcnt);

  /*--- get printer choice ---*/
  prev_n_printers = n_printers;
  argcnt = 0;
  SET_ARG(DXmNprinterList, &printer_list);
  SET_ARG(DXmNprinterCount, &n_printers);
  SET_ARG(DXmNprinterChoice, &prt_choice);
  XtGetValues (printdialog_w, arglist, argcnt);

  if(XmStringCompare(pr_format,ps_format_string))
    {
    new_format = CONVERT_PS;
    format_choice = ps_format_string;
    printer_choice = &ps_printer_choice;
    }
  else
    {
    new_format = ASCII;
    format_choice = text_format_string;
    printer_choice = &text_printer_choice;
    }

  XmStringFree(*printer_choice);
  *printer_choice = (XmString) XmStringCopy(prt_choice);

  if(new_format != format)  /* format has been changed */
    { /*--- update format radiobox setting ---*/
    if(new_format == ASCII)
      {
      on_widget = bkr_library->widgets[W_PR_TEXT_TB];
      off_widget = bkr_library->widgets[W_PR_CONVERTPS_TB];
      }
    else
      {
      on_widget = bkr_library->widgets[W_PR_CONVERTPS_TB];
      off_widget = bkr_library->widgets[W_PR_TEXT_TB];
      }

    argcnt = 0;
    SET_ARG(XmNset,TRUE);
    XtSetValues(on_widget,arglist,argcnt);
    argcnt = 0;
    SET_ARG(XmNset,FALSE);
    XtSetValues(off_widget,arglist,argcnt);

    /*--- update everything else that has to do with format ---*/
    pe_format_radiobox(new_format);
    }
  else
    {  /*--- format not changed, just update the printer options menu ---*/
    set_default_printer();
    }

#if PRTWGT_DIAG
  /*------*/
  fmt_choice = (char *)DXmCvtCStoOS(format_choice,&j_byte_cnt,&j_stat);
  ps_prt_choice = (char *)DXmCvtCStoOS(ps_printer_choice,&j_byte_cnt,&j_stat);
  text_prt_choice = (char *)DXmCvtCStoOS(text_printer_choice,&j_byte_cnt,&j_stat);
  printf("\nprintdialog_ok end:\n");
  if(printer_choice == &text_printer_choice)
    printf("  format=TEXT\n");
  else if(printer_choice == &ps_printer_choice)
    printf("  format=PS\n");
  printf("  ps_printer_choice = %s\n",ps_prt_choice);
  printf("  text_printer_choice = %s\n",text_prt_choice);
  printf("  fmt_choice = %s\n",fmt_choice);
  /*------*/
#endif

  return;
  }


/*--------------------- gen_next_fname ----------------------------
Purpose: Generate a (hopefully) unique filename based on filetype,
         timestamp, and process id.
------------------------------------------------------------------*/
static void gen_next_fname()
  {
  char      buf[512];
  char      num_buf[128];
  char      *cp;
  time_t    time_val;
  long      time_l;
  int       pid;
  int       len;

  static long  prev_time = -1L;

  /* printf("\n gen_next_fname\n"); */
  buf[511] = num_buf[127] = '\0';  /* safety */

  *generated_fname = TRUE;
    
  pid = getpid();
    
  time(&time_val);
  time_l = (long)time_val;
    
  if(time_l == prev_time) 
    time_l += (long)time_count;

  prev_time = time_l;

  if(deletefile)
    {
#ifdef VMS
    strcpy(buf,"SYS$SCRATCH:bkr_");
#else
    strcpy(buf,"/usr/tmp/bkr_");
#endif
    }
  else
    {
    strcpy(buf,working_dir);
    strcat(buf,"bkr_");
    }

  /***
  if(format == ASCII)
    strcat(buf,"ascii");
  else
    strcat(buf,"ps");
  ***/

  /*--- take last 3 digits of process id ---*/
  sprintf(num_buf,"%d",pid);
  if(strlen(num_buf) < 3)
    sprintf(num_buf,"%03d",pid);

  if(strlen(num_buf) > 3)
    cp = num_buf + strlen(num_buf) - 3;
  else
    cp = num_buf;

  strcat(buf,cp);  /*--- append to name ---*/

  /*--- take last 5 digits of time ---*/
  sprintf(num_buf,"%ld",time_l);
  if(strlen(num_buf) < 5)
    sprintf(num_buf,"%05ld",time_l);

  if(strlen(num_buf) > 5)
    cp = num_buf + strlen(num_buf) - 5;
  else
    cp = num_buf;

  strcat(buf,cp);  /*--- append to name ---*/
  if(format == ASCII)
    {
    strcat(buf,".txt");
    }
  else
    {
    strcat(buf,".ps");
    }

  if(*text1_fname)
    {
    BKR_CFREE(*text1_fname);
    }

  len = strlen(buf)+1;
  if(len >= GEN_FNAME_LEN - 1)  /* safety, this should not happen */
    {
    len = GEN_FNAME_LEN - 2;
    *(buf + len - 1) = '\0';
    }

  *text1_fname = (char *)BKR_CALLOC(1,strlen(buf)+1);
    
  strcpy(*text1_fname,buf);

  /* keep a copy of this generated fname string - we use this
     to compare typed filename on losing focus callback from
     text widget - if they are the same, then "generated" remains
     true even though the text widget had and lost focus */

  strcpy(gen_filename_string,buf);
    
  return;
  }


/*------------------- setup_for_fname -------------------------------------
Description: BR Print - checks file existence, sets label and text widget
             for file, and sets sensitivities.
--------------------------------------------------------------------------*/
static void setup_for_fname()
  {
  Widget           widget;
  Widget           widget1;
  Widget           widget2;
  unsigned int     argcnt;
  Arg              arglist[8];
  FILE             *fp;
  XmString         label;
  long             byte_cnt,l_status;
  int              un_gray;
  int              append_setting;
  char             buf[256];


  if(otherps_ui || (format != ASCII))
    append_mode = FALSE;

  if(entirefile && (format == ASSOC_PS))
    {   /* just spool the entire ps file shown, no writing to a new file */
    /* filename field is input file instead of output file now */
    if(!(fp = fopen(*text1_fname,"r")))
      label = DXmCvtOStoCS(linked_ps_not_found,&byte_cnt,&l_status);
    else
      {
      fclose(fp);
      label = DXmCvtOStoCS(print_linked_ps,&byte_cnt,&l_status);
      }
    }
  else
    {
    /*--- see if file does not already exist ---*/
    if(fp = fopen(*text1_fname,"r"))
      {
      new_file =  FALSE;
      fclose(fp);
      }
    else
      new_file = TRUE;

    if(append_mode)
      {
      if(new_file)
        strcpy(buf,write_new);
      else
        strcpy(buf,append_to_existing);
      }
    else
      {
      if(new_file)
        strcpy(buf,write_new);
      else
        {
#ifdef VMS
        strcpy(buf,write_to_existing);
#else
        strcpy(buf,overwrite_existing);
#endif
        }
      }

    if(format == OTHER_PS)
      strcat(buf,ps_page_range_file);
    else if(format == ASSOC_PS)
      strcat(buf,linked_ps_file);
    else if(format == CONVERT_PS)
      strcat(buf,ps_file);
    else if(format == ASCII)
      strcat(buf,text_file);

    if(print_file)
      {
      strcat(buf,and_print);
      if(deletefile)
        strcat(buf,slash_delete);
      else
        strcat(buf,slash_save);
      }
    else
      strcat(buf,colon);
    label = DXmCvtOStoCS(buf,&byte_cnt,&l_status);
    }

  /*--- set sensitivity of "Append to Existing File" toggle ---*/
  widget1 = bkr_library->widgets[W_PR_APPENDTOFILE_TB];
  un_gray = (format == ASCII)? TRUE: FALSE;
  argcnt = 0;
  SET_ARG(XmNsensitive,un_gray);
  if(!otherps_ui)
    XtSetValues(widget1,arglist,argcnt);
  argcnt = 0;

  /* if grayed, set "Append to Existing File" to off */
  argcnt = 0;
  append_setting = (un_gray)? append_mode: FALSE;
  SET_ARG(XmNset,append_setting);
  if(!otherps_ui)
    XtSetValues(widget1,arglist,argcnt);

  /*--- set label widget ---*/
  argcnt = 0;
  SET_ARG(XmNlabelString,label);
  /*** widget = (otherps_ui)? bkr_library->widgets[W_PR_3]:
                        bkr_library->widgets[W_PR_26];
  ***/
  widget = bkr_library->widgets[W_PR_F_NAME_LABEL];
  XtSetValues(widget,arglist,argcnt);
  XmStringFree(label);

  /*--- convert fname C string to compound string ---*/
  /*--- Xmfname = DXmCvtOStoCS(*text1_fname,&byte_cnt,&l_status); ---*/
  /*--- set filename text widget in BR main widget to fname ---*/
  
  /*** widget = otherps_ui? bkr_library->widgets[W_PR_1]:
                      bkr_library->widgets[W_PR_22]; ***/
  widget = bkr_library->widgets[W_PR_F_NAME_TEXT];
  XmTextSetString(widget,*text1_fname);

  return;
  }


/*------------------------- bkr_pe_format_radiobox -------------------------
Description: BR Print -  "PS, Text" radiobox callback shell
--------------------------------------------------------------------------*/
extern void bkr_pe_format_radiobox(w, tag, reason)
  Widget		w;
  int			*tag;
  XmAnyCallbackStruct   *reason;
  {
  Arg                   arglist[6];
  unsigned int          argcnt;
  XmString              prt_choice;

  /*--- set the format choice and print queue choice in the print widget ---*/


#if PRTWGT_DIAG
  /*------*/
  ps_prt_choice = (char *)DXmCvtCStoOS(ps_printer_choice,&j_byte_cnt,&j_stat);
  text_prt_choice = (char *)DXmCvtCStoOS(text_printer_choice,&j_byte_cnt,&j_stat);
  fmt_choice = (char *)DXmCvtCStoOS(format_choice,&j_byte_cnt,&j_stat);
  printf("\nbkr_pe_format_radiobox begin:\n");
  if(printer_choice == &text_printer_choice)
    printf("  format=TEXT\n");
  else if(printer_choice == &ps_printer_choice)
    printf("  format=PS\n");
  printf("  ps_printer_choice = %s\n",ps_prt_choice);
  printf("  text_printer_choice = %s\n",text_prt_choice);
  printf("  fmt_choice = %s\n",fmt_choice);
  /*------*/
#endif

  if(*tag == ASCII)
    {
    format_choice = text_format_string;
    printer_choice = &text_printer_choice;
    }
  else
    {
    format_choice = ps_format_string;
    printer_choice = &ps_printer_choice;
    }

  argcnt = 0;
  SET_ARG(DXmNprintFormatChoice, format_choice);
  XtSetValues(printdialog_w,arglist,argcnt);

  /*--- get printer choice ---*/
  prev_n_printers = n_printers;
  argcnt = 0;
  SET_ARG(DXmNprinterList, &printer_list);
  SET_ARG(DXmNprinterCount, &n_printers);
  SET_ARG(DXmNprinterChoice, &prt_choice);
  XtGetValues (printdialog_w, arglist, argcnt);
  XmStringFree(*printer_choice);
  *printer_choice = (XmString) XmStringCopy(prt_choice);

#if PRTWGT_DIAG
  /*------*/
  ps_prt_choice = (char *)DXmCvtCStoOS(ps_printer_choice,&j_byte_cnt,&j_stat);
  text_prt_choice = (char *)DXmCvtCStoOS(text_printer_choice,&j_byte_cnt,&j_stat);
  fmt_choice = (char *)DXmCvtCStoOS(format_choice,&j_byte_cnt,&j_stat);
  printf("\nbkr_pe_format_radiobox end:\n");
  if(printer_choice == &text_printer_choice)
    printf("  format=TEXT\n");
  else if(printer_choice == &ps_printer_choice)
    printf("  format=PS\n");
  printf("  ps_printer_choice = %s\n",ps_prt_choice);
  printf("  text_printer_choice = %s\n",text_prt_choice);
  printf("  fmt_choice = %s\n",fmt_choice);
  /*------*/
#endif

  pe_format_radiobox(*tag);

  return;
  }


/*------------------------- pe_format_radiobox -------------------------
Description: BR Print -  "PS, Text" radiobox callback internals
--------------------------------------------------------------------------*/
static void pe_format_radiobox(tag)
  int			tag;
  {
  Arg                   arglist[6];
  unsigned int          argcnt;
  int                   sensitive;
  int                   prev_generated;
  Widget		widget;
  char                  **prev_fname;
  long                  byte_cnt,l_status;
  int                   i;
  char                  *cmp_ext;
  char                  *copy_ext;
  char                  *cp;
  XmString              prt_choice;

#if PRTWGT_DIAG
  /*------*/
  ps_prt_choice = (char *)DXmCvtCStoOS(ps_printer_choice,&j_byte_cnt,&j_stat);
  text_prt_choice = (char *)DXmCvtCStoOS(text_printer_choice,&j_byte_cnt,&j_stat);
  fmt_choice = (char *)DXmCvtCStoOS(format_choice,&j_byte_cnt,&j_stat);
  printf("\npe_format_radiobox begin:\n");
  if(printer_choice == &text_printer_choice)
    printf("  format=TEXT\n");
  else if(printer_choice == &ps_printer_choice)
    printf("  format=PS\n");
  printf("  ps_printer_choice = %s\n",ps_prt_choice);
  printf("  text_printer_choice = %s\n",text_prt_choice);
  printf("  fmt_choice = %s\n",fmt_choice);
  /*------*/
#endif

  if(tag == format)
    return;

  format = tag;

  prev_generated = *generated_fname;
  prev_fname = text1_fname;

  if(format == CONVERT_PS)
    {
    format_choice = ps_format_string;
    printer_choice = &ps_printer_choice;
    save_append_mode = append_mode;
    generated_fname = &gen_convertpsfname;
    text1_fname = &out_convertpsfname;
    cmp_ext = ".txt";
    copy_ext = ".ps";
    }
  else if(format == ASCII)
    {
    format_choice = text_format_string;
    printer_choice = &text_printer_choice;
    append_mode = save_append_mode;
    generated_fname = &gen_asciifname;
    text1_fname = &out_asciifname;
    cmp_ext = ".ps";
    copy_ext = ".txt";
    }

  if(!*text1_fname || *generated_fname)
    {
    if(prev_generated)
      gen_next_fname();
    else   /* the previous format's filename was entered */
      {  /* use it as a basis for this fname rather than generating one */
      if(*prev_fname)
        {
        if(*text1_fname)
          {
          BKR_CFREE(*text1_fname);
          }
        *text1_fname = (char *)BKR_CALLOC(1,strlen(*prev_fname) + 2);
        strcpy(*text1_fname,*prev_fname);
        if(cp = strrchr(*text1_fname,'.'))
          {
          if(!strcmp(cp,cmp_ext)) /* if ended in .ps, use .txt or vice versa */
            {
            strcpy(cp,copy_ext);
            *generated_fname = FALSE;
            }
          }
        }
      }
    }

  setup_for_fname();

  /*--- set up to get print queue choice - use and retain settings
        for print queue list based on PS and Text formats
  ---*/

#if FALSE
  /* set up to get array of print queue names */
  argcnt = 0;
  SET_ARG(DXmNprinterList,&printer_list);
  SET_ARG(DXmNprinterCount,&n_printers);
  SET_ARG(DXmNprinterChoice, &prt_choice);
  XtGetValues(printdialog_w, arglist, argcnt);
  XmStringFree(*printer_choice);
  *printer_choice = (XmString) XmStringCopy(prt_choice);
#endif

  /*--- rebuild printers option menu if we need to ---*/
  if(n_printers > 0)
    {
    if (text_logical_def || ps_logical_def)
      {  
      /* The format changed so do a complete reset of the printers
       * menu in case the available printers / print queues
       * changed based on the format change.
       */
      reset_printers_menu();
      }
    else 
      {
      /* Just set the default printer in the options menu. */
      set_default_printer();
      }
    }
#if PRTWGT_DIAG
  /*------*/
  fmt_choice = (char *)DXmCvtCStoOS(format_choice,&j_byte_cnt,&j_stat);
  ps_prt_choice = (char *)DXmCvtCStoOS(ps_printer_choice,&j_byte_cnt,&j_stat);
  text_prt_choice = (char *)DXmCvtCStoOS(text_printer_choice,&j_byte_cnt,&j_stat);
  printf("\npe_format_radiobox end:\n");
  if(printer_choice == &text_printer_choice)
    printf("  format=TEXT\n");
  else if(printer_choice == &ps_printer_choice)
    printf("  format=PS\n");
  printf("  ps_printer_choice = %s\n",ps_prt_choice);
  printf("  text_printer_choice = %s\n",text_prt_choice);
  printf("  fmt_choice = %s\n",fmt_choice);
  /*------*/
#endif

  return;
  }


/*------------------------- reset_printers_menu -----------------------------
Description: reset the printers menu
--------------------------------------------------------------------------*/
static void reset_printers_menu()
{
    if (*pd_w_ptr) 
    {
        /* Unmanage the pulldown and then destory it, this should
         * take care of all of its push button children as well.
         */
        if (XtIsManaged(*pd_w_ptr)) 
        {
            XtUnmanageChild(*pd_w_ptr);
        }
        XtDestroyWidget(*pd_w_ptr);
        *pd_w_ptr = NULL;
    }
    if (printers_buttons) 
    {
        /* Finally free the buffer we allocated to keep track of the
         * push buttons.  The push buttons themselves were destroyed
         * when we destroyed the pulldown that contained them.
         */
        XtFree(printers_buttons);
        printers_buttons = NULL;
    }

    /* Now call create_printers_menu() to recreate the pulldown and 
     * push buttons and tell the option menu about them.
     * Create_printers_menu() will only do do as much as necessary.
     */
    create_printers_menu();
}


/*------------------------- bkr_pe_radio2 ---------------------------------
Description: BR Print - "Send to Printer, Save to File, Both" rabiobox
             callback.
--------------------------------------------------------------------------*/
extern void bkr_pe_radio2(w, tag, reason)
  Widget         		w;
  int			       *tag;
  XmAnyCallbackStruct          *reason;
  {
  Arg                   arglist[4];
  unsigned int          argcnt;
  Widget                widget;
  int                   prev_deletefile;

  prev_deletefile = deletefile;

  if(*tag == K_PRINTER)
    {
    deletefile = print_file = TRUE;
    }
  else if(*tag == K_FILE)
    {
    deletefile = print_file = FALSE;
    }    
  else if(*tag == K_BOTH)
    {
    deletefile = FALSE;
    print_file = TRUE;
    }

  if(*generated_fname && (deletefile != prev_deletefile))
    {
    gen_next_fname();
    }

  setup_for_fname();

  return;
  }


/*------------------------- bkr_pe_append_tb ---------------------------------
Description: BR Print -  main widget "Append to Existing File" toggle button
             callback.
--------------------------------------------------------------------------*/
extern void bkr_pe_append_tb(w, tag, toggle)
  Widget         		w;
  int			       *tag;
  XmToggleButtonCallbackStruct *toggle;
  {

  if(toggle->set)
    {
    append_mode = TRUE;
    }
  else
    {
    append_mode = FALSE;
    }

  setup_for_fname();

  return;
  }


/*------------------------- bkr_pe_filename_text --------------------------
Description: BR Print -  filename text widget <CR> callback
--------------------------------------------------------------------------*/
extern void bkr_pe_filename_text(w, tag, reason)
  Widget		w;
  int			*tag;
  XmAnyCallbackStruct   *reason;
  {
  char                  *cp;
  int                   argcnt;
  Arg                   arglist[4];
  Widget                widget;

  BKR_CFREE(*text1_fname);

  /*--- get new text1_fname string from filename text widget ---*/
  /*** widget = (otherps_ui)? bkr_library->widgets[W_PR_1]:
                        bkr_library->widgets[W_PR_22];***/
  widget = bkr_library->widgets[W_PR_F_NAME_TEXT];
  cp = XmTextGetString(widget);
  *text1_fname = (char *)BKR_CALLOC(1,strlen(cp) + 2);
  strcpy(*text1_fname,cp);
  XtFree(cp);


  /* if callback happens because of change in focus but filename
     was not changed, just return */
  if(!strcmp(*text1_fname,gen_filename_string))
    return;

  *generated_fname = FALSE;

  setup_for_fname();

  if(print_file && deletefile)
    {   /*--- set "Delete File After Printing" to FALSE ---*/
    if(otherps_ui)
      {
      deletefile = FALSE;
      argcnt = 0;
      SET_ARG(XmNset,FALSE);
      /* XtSetValues(bkr_library->widgets[W_PR_8],arglist,argcnt); */
      }
    }

  return;
  }
  /*-----------------------------------------------------------------*/


/*------------------------- create_asciifile ----------------------------
Description: Creates ASCII txt file from decw$book file.
------------------------------------------------------------------------*/
static int create_asciifile()
  {
  char                   *pgs_extracted;  /* array of flags */
  FILE                   *fp;             /* ascii output file ptr */
  char                   *open_string;
  int                    i;
  char                   *error_string;
  Widget                 parent_w;
  char                   *copyright;
  int                    copyrt_from_book;
  char                   *out_fname;
  BKR_DIR_ENTRY          *entry;

  parent_w = *top_w_ptr;
  out_fname = out_asciifname;

  /*--- open ascii extraction file for write ---*/
  open_string = (append_mode)? "a": "w";

  if(!(fp = fopen(out_asciifname,open_string)))
    {
    OPEN_ERROR(out_asciifname);
    }

  /*--- get and output copyright string ---*/
  copyrt_from_book = TRUE;
  if(!(copyright = bri_book_copyright_info(book->book_id)))
    {
    copyright = copyright_notice;
    copyrt_from_book = FALSE;
    }
  F_PUTS(copyright);
  F_PUTS("\n\n");

  if(copyrt_from_book) 
    BriFree(copyright,book->book_id);

  if(entirefile) /*--- print book from library window ---*/
    {
    /*--- extract ftext for entire book ---*/
    if(bkr_book_ascii(book,out_asciifname,fp,parent_w))
      {
      return(1);
      }
    }
  else if(from_topic_window)  /*--- print topic from topic window ---*/
    {
    if(bkr_topic_ascii(book,pe_window->u.topic.page_id,
                       out_asciifname,fp,*top_w_ptr))
      return(1);
    }
  else    /*--- print topic(s) from navigation window ---*/
    {
    /* allocate array of flags to tell when a page has been extracted */
    if(!(pgs_extracted = (char *)BKR_CALLOC(1,
                                  (book->n_pages+1) * sizeof(char))))
      {
      BKR_ERROR_DIALOG("PE_MALLOC",parent_w);
      return(1);
      }

    num_selected = (entirefile)? bkr_library->u.library.num_selected:
                                  pe_window->u.selection.num_selected;

    /*--- loop on the number of entries selected (highlighted) ---*/
    for(i=0; i < num_selected; ++i)
      {
      /*--- extract ascii page/s for selected entry including children ---*/
      entry = (BKR_DIR_ENTRY *)pe_window->u.selection.selected_entry_tags[i];
      if((entry->entry_type == DIRECTORY_ENTRY) && entry->u.entry.target)
        {
        if(expand_ascii(book,
              (BKR_DIR_ENTRY *)pe_window->u.selection.selected_entry_tags[i],
               fp, pgs_extracted))
          {
          return(1);
          }
        }
      }
    BKR_CFREE(pgs_extracted);
    }     /* end else print/extract topic */

  fclose(fp);

  return(0);
  }

/*------------------------- create_convertpsfile ----------------------------
Description: Creates Converted PS file from decw$book file.
------------------------------------------------------------------------*/
static int create_convertpsfile()
  {
  char                   *pgs_extracted;  /* array of flags */
  FILE                   *convertps_fp;   /* Converted PS output file ptr */
  char                   *open_string;
  int                    i;
  char                   *error_string;
  Widget                 parent_w;
  BKR_DIR_ENTRY          *entry;

  parent_w = *top_w_ptr;

 /******** #ifdef VMS
  convertps_fp = fopen(out_convertpsfname,open_string,"rat=cr","rfm=var");
 #else
  convertps_fp = fopen(out_convertpsfname,open_string);
 #endif *********/

  convertps_fp = fopen(out_convertpsfname,"w");

  if(!convertps_fp)
    {
    OPEN_ERROR(out_convertpsfname)
    }

  /*--- copy converted PS prolog to out_convertpsfname to start ---*/
  if(includeprolog(NULL, PROLOG_FNAME, out_convertpsfname,
                   convertps_fp,parent_w))
    {
    return(1);
    }

  setup_paper_scaling(page_width,page_height);

  /*--- create document setup area ---*/
  if(bkr_psdocsetup(book,out_convertpsfname,convertps_fp,parent_w))
    {
    return(1);
    }

  if(bkr_ps_begindoc(out_convertpsfname,convertps_fp,parent_w))
    {
    return(1);
    }

  if(entirefile) /*--- print book from library window ---*/
    {
    /*--- extract ftext for entire book ---*/
    if(bkr_book_convertps(book,out_convertpsfname,convertps_fp,parent_w))
      {
      return(1);
      }
    }
  else if(from_topic_window)  /*--- print topic from topic window ---*/
    {
    bkr_topic_convertps(book,(BMD_OBJECT_ID)0,out_convertpsfname,convertps_fp,
                        FALSE,*top_w_ptr,pe_window->u.topic.data);
    }
  else    /*--- print topic(s) from navigation window ---*/
    {
    /* allocate array of flags to tell when a page has been extracted */
    if(!(pgs_extracted = (char *)BKR_CALLOC(1,
                                  (book->n_pages+1) * sizeof(char))))
      {
      BKR_ERROR_DIALOG("PE_MALLOC",parent_w);
      return(1);
      }

    num_selected = (entirefile)? bkr_library->u.library.num_selected:
                                  pe_window->u.selection.num_selected;

    /*--- loop on the number of entries selected (highlighted) ---*/
    for(i=0; i < num_selected; ++i)
      {
      entry = (BKR_DIR_ENTRY *)pe_window->u.selection.selected_entry_tags[i];
      if((entry->entry_type == DIRECTORY_ENTRY) && entry->u.entry.target)
        {
        /*--- extract ascii page/s for selected entry including children ---*/
        if(expand_convertps(book,
                (BKR_DIR_ENTRY *)pe_window->u.selection.selected_entry_tags[i],
               convertps_fp, pgs_extracted))
          {
          return(1);
          }
        }
      }
    BKR_CFREE(pgs_extracted);
    }     /* end else print/extract topic */

  if(bkr_ps_enddoc(out_convertpsfname,convertps_fp,parent_w))
    {
    return(1);
    }

  fclose(convertps_fp);

  return(0);
  }
    

/*----------------------- expand_ascii --------------------------
Purpose: recursively process all ancestors for ascii extraction
------------------------------------------------------------------*/
static int expand_ascii(book,dir,fp,pgs_extracted)
  BKR_BOOK_CTX         *book;
  BKR_DIR_ENTRY        *dir;
  FILE                 *fp;
  char                 pgs_extracted[];
  {
  BMD_OBJECT_ID        pg_id;

  /*--- get the page id that this chunk occurs on ---*/
  pg_id = bri_page_chunk_page_id(book->book_id, dir->u.entry.target);

  /* if we haven't done this page yet, convert the page to ascii */
  if(pg_id >= book->n_pages)     /* safety */
    ;
    /* printf("\n bkr internal error:  pg_id exceeded n_pages\n"); */
  else
    {
    if(!pgs_extracted[pg_id])
      {
      if(bkr_topic_ascii(book,pg_id,out_asciifname,fp,*top_w_ptr))
        {
        return(1);
        }
      pgs_extracted[pg_id] = TRUE;
      }
    }

  /*--- loop and recurse to process children and siblings */
  for(dir = dir->children; dir; dir = dir->sibling)
    expand_ascii(book,dir,fp,pgs_extracted);

  return(0);
  }


/*----------------------- expand_convertps ------------------------
Purpose: recursively process all ancestors for PS extraction
------------------------------------------------------------------*/
static int expand_convertps(book,dir,fp,pgs_extracted)
  BKR_BOOK_CTX         *book;
  BKR_DIR_ENTRY        *dir;
  FILE                 *fp;
  char                 pgs_extracted[];
  {
  BKR_TOPIC_DATA       *topic;
  BMD_OBJECT_ID        pg_id;

  /*--- get the page id that this chunk occurs on ---*/
  pg_id = bri_page_chunk_page_id(book->book_id, dir->u.entry.target);

  /* if we haven't done this page yet, convert the page to PS */
  if(pg_id >= book->n_pages)     /* safety */
    ;
    /* printf("\n bkr internal error:  pg_id exceeded n_pages\n"); */
  else
    {
    if(!pgs_extracted[pg_id])
      {
      if(!(topic = bkr_topic_data_get(book,pg_id)))
        return(1);
      bkr_topic_convertps(book,pg_id,out_convertpsfname,fp,FALSE,*top_w_ptr,
                          topic);
      bkr_topic_data_free(book,pg_id);
      pgs_extracted[pg_id] = TRUE;
      }
    }

  /*--- loop and recurse to process children and siblings */
  for(dir = dir->children; dir; dir = dir->sibling)
    expand_convertps(book,dir,fp,pgs_extracted);

  return(0);
  }


/*----------------------- submit_print ----------------------------
Purpose: Spool print job using print widget values.
------------------------------------------------------------------*/
static void submit_print(fname,deletefile)
  char               *fname;
  int                deletefile;
  {
  XmString           file_str[1];
  unsigned long      l_status;
  unsigned int       status;
  MrmType            dummy_class;
  static Widget      infodialog_w = (Widget)0;
  int                argcnt;
  /*** char               *filename; ***/
  char               *error_string;
  Widget             parent_w;
  Arg                arglist[4];

  parent_w = *top_w_ptr;

  /*--- fetch print widget ---*/
  if (!printdialog_w)
    {
    MRM_FETCH("printdialog",*top_w_ptr,&printdialog_w);
    }

#ifdef VMS
#define PR_OPTION "/param=data=post"
#else
#define PR_OPTION " -Dpostscript"
#endif

  /*--- create filename string for print widget print job ---*/
  file_str[0] = XmStringCreateLtoR(*text1_fname, XmSTRING_OS_CHARSET);

  if(n_printers <= 0)
    {
    BKR_ERROR_DIALOG("PE_NO_PRINTERS",*top_w_ptr);
    return;
    }

/* #ifdef VMS */
  argcnt = 0;
  SET_ARG(DXmNdeleteFile,deletefile);
  XtSetValues(printdialog_w,arglist,argcnt);
/* #endif */

  /*--- submit print job ---*/
  l_status = DXmPrintWgtPrintJob(printdialog_w, file_str, 1);

#ifndef VMS
  if(deletefile)
    {
#ifndef __osf__
    DELETE_FILE(fname);
#endif
    }
#endif

  /* printf("\n print job spooled\n"); */

  /**********
  BKR_CFREE(filename);
  **********/

  return;
  } 


/*------------------------- set_default_printer -----------------------------
Description: BR Print - set history for the option menu of print queues 
--------------------------------------------------------------------------*/
static void set_default_printer ()
  {
  Arg       arglist[6];
  int       argcnt;
  int       i;
  long      byte_cnt,stat;

#if PRTWGT_DIAG
  /*------*/
  fmt_choice = (char *)DXmCvtCStoOS(format_choice,&j_byte_cnt,&j_stat);
  ps_prt_choice = (char *)DXmCvtCStoOS(ps_printer_choice,&j_byte_cnt,&j_stat);
  text_prt_choice = (char *)DXmCvtCStoOS(text_printer_choice,&j_byte_cnt,&j_stat);
  printf("Set_printers_menu begin:\n");
  if(printer_choice == &text_printer_choice)
    printf("  format=TEXT\n");
  else if(printer_choice == &ps_printer_choice)
    printf("  format=PS\n");
  printf("  ps_printer_choice = %s\n",ps_prt_choice);
  printf("  text_printer_choice = %s\n",text_prt_choice);
  printf("  fmt_choice = %s\n",fmt_choice);
  /*------*/
#endif

  for(i = 0; i < n_printers; ++i)
    {
    if(XmStringCompare(printer_list[i],*printer_choice))
      {
      XtVaSetValues(*option_w_ptr,XmNmenuHistory,printers_buttons[i],NULL);
      break;
      }
    }

#if PRTWGT_DIAG
  /*------*/
  fmt_choice = (char *)DXmCvtCStoOS(format_choice,&j_byte_cnt,&j_stat);
  ps_prt_choice = (char *)DXmCvtCStoOS(ps_printer_choice,&j_byte_cnt,&j_stat);
  text_prt_choice = (char *)DXmCvtCStoOS(text_printer_choice,&j_byte_cnt,&j_stat);
  printf("Set_printers_menu end:\n");
  if(printer_choice == &text_printer_choice)
    printf("  format=TEXT\n");
  else if(printer_choice == &ps_printer_choice)
    printf("  format=PS\n");
  printf("  ps_printer_choice = %s\n",ps_prt_choice);
  printf("  text_printer_choice = %s\n",text_prt_choice);
  printf("  fmt_choice = %s\n",fmt_choice);
  /*------*/
#endif

  return;
  }


/*------------------------- create_printers_menu -------------------------
Description: BR Print - creates the option menu of print queue names
--------------------------------------------------------------------------*/
static void create_printers_menu ()
  {
  Arg       arglist[6];
  int       argcnt;
  int       i;
  int       choice;
  XmString  prt_choice;
  long      status;
  MrmType   dummy_class;

#if FALSE
  /* set up to get the number of print queues available */
  argcnt = 0;
  SET_ARG(DXmNprinterCount, &n_printers);
  /* set up to get array of print queue names */
  SET_ARG(DXmNprinterList, &printer_list);
  XtGetValues (printdialog_w, arglist, argcnt);
#endif

  if(!n_printers)
    return;

  /* Figure out which row-column widget to use and manage it
   * if necessary.
   */
  /*** printers_rc_w = (otherps_ui)? bkr_library->widgets[W_PR_2]:
                               bkr_library->widgets[W_PR_27]; ***/
  printers_rc_w = bkr_library->widgets[W_PR_PRTS_RC];

  if (!XtIsManaged(printers_rc_w)) {
      XtManageChild(printers_rc_w);
  }

  /* Create the pulldown menu if it is not already created, but don't
   * manage it yet.  That will happen when some selects the option
   * menu below
   */
  if (*pd_w_ptr == NULL) 
  {
      *pd_w_ptr = XmCreatePulldownMenu(printers_rc_w,"pd",NULL,0);
  }


  /* XtSetArg (arglist[0], XmNindicatorOn, 0); */
  /* XtSetArg (arglist[1], XmNactivateCallback, bkr_pe_printers_buttons); */

#if FALSE
  /* set up to get default print queue name */
  argcnt = 0;
  SET_ARG(DXmNprinterChoice, &prt_choice);
  XtGetValues (printdialog_w, arglist, argcnt);
  /* XmStringFree(*printer_choice); */
  *printer_choice = (XmString) XmStringCopy(prt_choice);
#endif 

  /* Free the buffer we use to keep track of the printer buttons, if
   * necessary, then allocate a new one for the new buttons.
   */
  if (printers_buttons) {
      XtFree(printers_buttons);
  }
  printers_buttons = (Widget *) XtCalloc (n_printers, sizeof (Widget));

  /* Now create the buttons with the pulldown menu as their parent.
   * During this we see if one of the buttons matches the previous
   * printer choice.  We will use this or the first (0) button as the
   * the default choice for the option menu.
   */
  for (i = choice = 0; i < n_printers; i++)
  {
      XtSetArg (arglist[0], XmNlabelString, printer_list[i]);
      printers_buttons[i] =  XmCreatePushButtonGadget(*pd_w_ptr, "", arglist, 1);
      if (XmStringCompare(printer_list[i],*printer_choice))
      {
          choice = i;
      }
      XtAddCallback(printers_buttons[i],XmNactivateCallback,bkr_pe_printers_buttons,NULL);
  }
  XtManageChildren(printers_buttons,n_printers);

  /* Create the argument list for the options menu values that we are
   * interested in, namely the submenu and the menu history. Then create
   * the options menu and/or set its values, and finally manage it if
   * necessary.
   */
  argcnt = 0;
  SET_ARG(XmNsubMenuId, *pd_w_ptr);
  SET_ARG(XmNmenuHistory, printers_buttons[choice]);
  if (*option_w_ptr == NULL)
  {
      *option_w_ptr = XmCreateOptionMenu(printers_rc_w,"",arglist,argcnt);
  }
  else 
  {
      XtSetValues(*option_w_ptr,arglist,argcnt);
  }
  if ( ! XtIsManaged(*option_w_ptr)) 
  {
      XtManageChild(*option_w_ptr);
  }


#if PRTWGT_DIAG
  /*------*/
  fmt_choice = (char *)DXmCvtCStoOS(format_choice,&j_byte_cnt,&j_stat);
  ps_prt_choice = (char *)DXmCvtCStoOS(ps_printer_choice,&j_byte_cnt,&j_stat);
  text_prt_choice = (char *)DXmCvtCStoOS(text_printer_choice,&j_byte_cnt,&j_stat);
  printf("\ncreate_printers_menu end:\n");
  if(printer_choice == &text_printer_choice)
    printf("  format=TEXT\n");
  else if(printer_choice == &ps_printer_choice)
    printf("  format=PS\n");
  printf("  ps_printer_choice = %s\n",ps_prt_choice);
  printf("  text_printer_choice = %s\n",text_prt_choice);
  printf("  fmt_choice = %s\n",fmt_choice);
  /*------*/
#endif

  return;
  }


/*--------------------- bkr_pe_printers_buttons ---------------------------
Description: BR Print -  printers option menu pushbutton callback for
             value changed - sets chosen pushbutton
             and sets printer choice value in the printdialog_w.
--------------------------------------------------------------------------*/
extern void bkr_pe_printers_buttons(w, tag, reason)
  Widget		w;
  int			*tag;
  XmAnyCallbackStruct   *reason;
  {
  Arg                   arglist[4];
  int                   argcnt;
  XmString              label_pr_choice;
  long                  byte_cnt,stat;

  /*--- get new printer choice value from options pulldown ---*/
  argcnt = 0;
  SET_ARG(XmNlabelString,&label_pr_choice);
  XtGetValues(w,arglist,argcnt);
  /* XmStringFree(*printer_choice); */
  *printer_choice = (XmString) XmStringCopy(label_pr_choice);

#if FALSE
  /*--- set format choice in the print widget ---*/
  argcnt = 0;
  SET_ARG(DXmNprintFormatChoice, format_choice);
  XtSetValues(printdialog_w,arglist,argcnt);
#endif

  /*--- set print queue in the print widget ---*/
  argcnt = 0;
  SET_ARG(DXmNprinterChoice,*printer_choice);
  XtSetValues(printdialog_w, arglist,argcnt);

  /*--- set options button history ---*/
  argcnt = 0;
  SET_ARG(XmNmenuHistory, w);
  XtSetValues(*option_w_ptr,arglist,argcnt);

#if PRTWGT_DIAG
  /*------*/
  fmt_choice = (char *)DXmCvtCStoOS(format_choice,&j_byte_cnt,&j_stat);
  ps_prt_choice = (char *)DXmCvtCStoOS(ps_printer_choice,&j_byte_cnt,&j_stat);
  text_prt_choice = (char *)DXmCvtCStoOS(text_printer_choice,&j_byte_cnt,&j_stat);
  printf("\npe_printers_buttons end:\n");
  if(printer_choice == &text_printer_choice)
    printf("  format=TEXT\n");
  else if(printer_choice == &ps_printer_choice)
    printf("  format=PS\n");
  printf("  ps_printer_choice = %s\n",ps_prt_choice);
  printf("  text_printer_choice = %s\n",text_prt_choice);
  printf("  fmt_choice = %s\n",fmt_choice);
  /*------*/
#endif

  return;
  }


/*-------------------- setposfromparent ---------------------------------
Description: BR Print - sets pos of child widget translated from
             parent widgets origin.
------------------------------------------------------------------------*/
static void setposfromparent(parent_w,child_w,x,y)
  Widget     parent_w;
  Widget     child_w;
  int        x;
  int        y;
  {
  Position   xpos;
  Position   ypos;
  Arg        arglist[4];
  int        argcnt;

  /* get xy pos of parent */
  argcnt = 0;
  SET_ARG(XmNx, &xpos);
  SET_ARG(XmNy, &ypos);
  XtGetValues (parent_w, arglist, argcnt);

  /* translate */
  xpos += (Position) x;
  ypos += (Position) y;

  /* set xy pos of child */
  argcnt = 0;
  SET_ARG(XmNdefaultPosition, FALSE);
  SET_ARG(XmNx, xpos);
  SET_ARG(XmNy, ypos);
  XtSetValues(child_w,arglist,argcnt);

  return;
  }
#endif /* PRINT */
