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
	@(#)$RCSfile: decxmail.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 23:56:42 $
*/

/*
 *                     Copyright (c) 1987, 1991 by
 *              Digital Equipment Corporation, Maynard, MA
 *                      All rights reserved.
 *
 *   This software is furnished under a license and may be used and
 *   copied  only  in accordance with the terms of such license and
 *   with the  inclusion  of  the  above  copyright  notice.   This
 *   software  or  any  other copies thereof may not be provided or
 *   otherwise made available to any other person.  No title to and
 *   ownership of the software is hereby transferred.
 *
 *   The information in this software is subject to change  without
 *   notice  and should not be construed as a commitment by Digital
 *   Equipment Corporation.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/*
 */


#ifndef _decxmail_h
#define _decxmail_h
#include <stdio.h>
#include <strings.h>
#include <Xlib.h>
#include <Xutil.h>
#include <Xm/Xm.h>
#include <Xm/Text.h>

#define DDIFVIEWER
#define REALDDIFVIEWER

#include <Xm/XmP.h>

#include "StringDefs.h"

#define RMailPixmap	"MailPixmap"
#define RMailBitmap	"MailBitmap"
#define RYesNoDefault	"YesNoDefault"
#define RLayoutStyle	"LayoutStyle"

/* The following gives the slop allowed between the directory
 * mod date and the last scanfile read. Measured in seconds.
 */
#define SCAN_SLOP 5 * 60
/*
 * PostScript searching gives up after this
 * many lines in the message
 */
#define PSLIMIT 50
#define DEFAULT		2

/* 
 * Default printers for entire message and stripped message AJ01
 */
#define PRINTDEFAULT "lpr >/dev/null 2>/dev/null"
#define STRIPPEDPRINTDEFAULT "lpr >/dev/null 2>/dev/null"

#define DELETEABORTED	-1


#ifndef APPDEFAULT
#define PROGCLASS "dxMail"
#else
#define PROGCLASS APPDEFAULT
#endif

typedef int * dp;		/* For debugging. */

typedef struct _ButtonRec *Button;
typedef struct _TocRec *Toc;
typedef struct _MsgRec *Msg;
typedef struct _MsgHandleRec *MsgHandle;
typedef struct _RadioRec *Radio;
typedef struct _DDIFFileInfoRec *DDIFFileInfo;

typedef int IntRange[2];

typedef enum {
    Fignore, Fmove, Fdelete
} FateType;

typedef enum {
    ADD, REMOVE, DELETE
} TwiddleOperation;

typedef enum {
    MTunknown, MTtext, MTddif, MTdtif, MTps
} MsgType;

/* Definition of SVN tags - 16 bit TOC number and 16 bit message number */
/* However, for Alpha, Xtpointer is  64 bits, so the union elements
   had to be made 32 + 32, so they were changed from unsigned short to
   unsigned int. Before the change, values assigned to tocNumber and
   msgNumber wouldn't add up to form tagValue. A side effect was, if
   a msg was moved or deleted, its status wouldn't change -aju 08/92 */

typedef union {
	XtPointer tagValue;
	struct {
#ifdef __alpha
/* 64 bit architecture specific */
	    unsigned int tocNumber;
	    unsigned int msgNumber;
#else  /* __alpha */
/* 32 bit acrchitecture (MIPS) */
	    unsigned short tocNumber;
	    unsigned short msgNumber;
#endif /* __alpha */
	} tagFields;
} TagRec;


typedef struct _PopupRec {
    char *name;
    Widget widget;
    Boolean fixed;
    struct _PopupRec *next;
} PopupRec, *Popup;


typedef struct _ScrnRec {
   char		*name;		/* Name of the kind of scrn. */
   Widget	parent;		/* The parent widget of the scrn */
   Widget	main;		/* Main window widget for the scrn */
   Widget	menu;		/* Widget for the pull-down menu */
   Widget	widget;		/* The pane widget for the scrn */
   Boolean	mapped;		/* TRUE only if we've mapped this screen. */
   Widget	folderwidget;	/* Folder RowColumn widget */
   Radio	folderradio;	/* Folder buttons. */
   Widget	toclabel;	/* Toc titlebar. */
   Widget	tocwidget;	/* Toc text. */
   int		max_buttons;	/* How many buttons have been created. */
   Radio 	seqradio;	/* Sequence buttons. */
   Widget	viewwidget;	/* View widget. */
   Widget 	ddifpane;	/* Pane containing ddif widgets: */
   Widget	ddifheaders;	/* Widget showing headers of ddif messages. */
   Widget	ddifbody;	/* Widget showing body of ddif message. */
   Boolean	ddifontop;	/* TRUE if the ddif widgets are visible. */
   DDIFFileInfo ddifinfo;	/* Info about the ddif being viewed here. */
   int		readnum;	/* Which # read window this is. */
   Toc		toc;		/* The table of contents. */
   Toc		lasttoc;	/* Last displayed TOC */
   Msg		msg;		/* The message being viewed. */
   Popup	firstpopup;	/* Linked list of popup info. */
   Cursor	normalcursor;	/* Cursor for normal use. */
   Cursor	sleepcursor;	/* Cursor when busy doing something. */
   Boolean	lastwentbackwards; /* TRUE if the last viewing operation was
				     an "ExecViewBefore" */
   Boolean	neednewpixmaps;	/* TRUE if the button pixmaps should be
				   refigured. */
   Boolean	neednewenabled;	/* TRUE if we need to refigure which buttons
				   are enabled on this screen. */
   Boolean	neednewtitle;	/* TRUE if we need to set the title for this
				   screen. */
   Boolean	neednewiconname; /* TRUE if we need to set the icon name for */
				 /* this screen. */
   XContext    actionContext;    /* Context for looking up actions: AJ */ 
} ScrnRec, *Scrn;


typedef struct {
    int nummsgs;
    Msg *msglist;
} MsgListRec, *MsgList;


typedef struct {
   char		*name;		/* Name of this sequence. */
   MsgList	mlist;		/* Messages in this sequence. */
} SequenceRec, *Sequence;


/* Warning and error codes. */

#define	WNoNewMail		"NoNewMail"
#define WMustRestart		"MustRestart"
#define WViewerError		"ViewerError"

#define EFolderCantDelete	"FolderCantDelete"
#define EFolderEmptyName	"FolderEmptyName"
#define EFolderExists		"FolderExists"
#define EFolderCantCreate	"FolderCantCreate"
#define EFolderCantShow 	"FolderCantShow"
#define EFolderCantHide 	"FolderCantHide"
#define EFolderNoSuch		"FolderNoSuch"
#define EFolderCantMoveCopySelf	"FolderCantMoveCopySelf"
#define ESequenceNoSuch		"SequenceNoSuch"
#define ESequenceEmptyName	"SequenceEmptyName"
#define ESequenceNoAll		"SequenceNoAll"
#define ESequenceCantCreate	"SequenceCantCreate"
#define EMessageNoneSpecified	"MessageNoneSpecified"
#define EFileCantOpen		"FileCantOpen"
#define EFileBadDirectory	"FileBadDirectory"
#define ESelectDirectory	"SelectDirectory"
#define ESubFoldersPresent      "Subfolders Present " 
/*SM: Help Stuff*/
#define HELPDIALOG_CLASS	"dxmHelpWidgetClass"
#define HELPDIALOG_CREATE	"DXmCreateHelpDialog"


#include "globals.h"
#include "macros.h"
#include "externs.h"

#endif _decxmail_h
