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
static char rcs_id[] = "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/dxmail/init.c,v 1.1.6.8 1994/01/11 22:06:17 Michael_Igoe Exp $";
#endif

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
 * $Log: init.c,v $
 * Revision 1.1.6.8  1994/01/11  22:06:17  Michael_Igoe
 * 	Fix MAILDROP.
 * 	[1994/01/11  19:31:51  Michael_Igoe]
 *
 * Revision 1.1.8.2  1994/01/11  19:31:51  Michael_Igoe
 * 	Fix MAILDROP.
 *
 * Revision 1.1.6.7  1993/11/11  19:45:41  Michael_Igoe
 * 	 Derive user resource file path name (DXMail) ... same as Motif.
 * 	 Also, set WM_COMMAND property so "xlsclients" sees "dxmail".
 * 	[1993/11/03  18:56:44  Michael_Igoe]
 *
 * Revision 1.1.8.2  1993/11/03  18:56:44  Michael_Igoe
 * 	 Derive user resource file path name (DXMail) ... same as Motif.
 * 	 Also, set WM_COMMAND property so "xlsclients" sees "dxmail".
 *
 * Revision 1.1.6.6  1993/11/02  14:20:57  Adrienne_Snyder
 * 	Use local for old customfile name (dxMail), global for new name (DXMail).
 * 	[1993/10/21  14:49:16  Adrienne_Snyder]
 *
 * Revision 1.1.8.2  1993/10/21  14:49:16  Adrienne_Snyder
 * 	Use local for old customfile name (dxMail), global for new name (DXMail).
 *
 * Revision 1.1.6.5  1993/09/07  17:41:26  Adrienne_Snyder
 * 	Changed call to XtCreateEDiskSource to have the correct parameters passed in.
 * 	[1993/09/02  19:32:25  Adrienne_Snyder]
 *
 * Revision 1.1.7.2  1993/09/02  19:32:25  Adrienne_Snyder
 * 	Changed call to XtCreateEDiskSource to have the correct parameters passed in.
 *
 * Revision 1.1.6.3  1993/08/03  13:37:19  Adrienne_Snyder
 * 	Recovered from bmerge changes
 * 	[1993/08/03  13:26:07  Adrienne_Snyder]
 *
 * Revision 1.1.7.2  1993/08/03  13:26:07  Adrienne_Snyder
 * 	Recovered from bmerge changes
 *
 * Revision 1.1.2.8  92/11/13  14:49:06  Aju_John
 * 	"DPS library related fixes for XtMainLoop and DispatchEvents"
 * 
 * Revision 1.1.3.2  92/11/13  14:46:13  Aju_John
 * 	DPS library related fixes for XtMainLoop and DispatchEvents
 * 
 * Revision 1.1.2.7  92/10/14  12:54:09  Aju_John
 * 	supress warnings on resource file
 * 	[92/10/14  12:49:48  Aju_John]
 * 
 * 	added custom editor
 * 	[92/07/14  13:45:37  Aju_John]
 * 
 * 	removed editor changes on board's instr
 * 	[92/07/13  15:36:11  Aju_John]
 * 
 * 	editor/renamefolder/PSOrientation/showsubfolders changes
 * 	[92/07/08  12:36:52  Aju_John]
 * 
 * 	Added compatibility with oldstyle config file
 * 	[92/07/01  10:57:17  Aju_John]
 * 
 * Revision 1.1.3.2  92/10/14  12:49:48  Aju_John
 * 	supress warnings on resource file
 * 
 * Revision 1.1.2.6  92/07/14  13:52:11  Aju_John
 * 	"custom compose editor changes"
 * 
 * Revision 1.1.3.2  92/07/14  13:45:37  Aju_John
 * 	added custom editor
 * 
 * Revision 1.1.2.5  92/07/13  15:41:47  Aju_John
 * 	"renamefolder/PSOrientation/showsubfolders changes"
 * 
 * Revision 1.1.3.3  92/07/13  15:36:11  Aju_John
 * 	removed editor changes on board's instr
 * 
 * Revision 1.1.3.2  92/07/08  12:36:52  Aju_John
 * 	editor/renamefolder/PSOrientation/showsubfolders changes
 * 
 * Revision 1.1.2.4  92/07/01  10:58:39  Aju_John
 * 	"Added compatibility with oldstyle config file"
 * 
 * Revision 1.1.3.2  92/07/01  10:57:17  Aju_John
 * 	Added compatibility with oldstyle config file
 * 
 * Revision 1.1.2.3  92/06/29  15:21:02  Dave_Hill
 * 	fixed -DAPPDEFAULTS handling
 * 	[92/06/29  15:19:55  Dave_Hill]
 * 
 * Revision 1.1.3.2  92/06/29  15:19:55  Dave_Hill
 * 	fixed -DAPPDEFAULTS handling
 * 
 * Revision 1.1.2.2  92/06/26  11:44:34  Dave_Hill
 * 	first submittal to pool for Aju John, replaces former ../mail
 * 	[92/06/26  11:26:32  Dave_Hill]
 * 
 * Revision 1.1.1.2  92/06/26  11:26:32  Dave_Hill
 * 	first submittal to pool for Aju John, replaces former ../mail
 * 
 * Revision 1.2.2.2  92/03/04  15:11:47  Aju_John
 * 	added support for folder cache
 * 	[92/03/04  14:51:55  Aju_John]
 * 
 * Revision 1.2.1.2  92/03/04  14:51:55  Aju_John
 * 	added support for folder cache
 * 
 * Revision 1.2  91/12/30  12:48:20  devbld
 * Initial load of project
 * 
 * Revision 1.14  91/11/27  15:28:37  aju
 * bl6: Implemented the -t option
 * 
 * Revision 1.13  91/10/08  03:58:43  rmurphy
 * bl4: Intelligently decide print option for ps
 * 
 * Revision 1.12  91/10/04  19:37:07  rmurphy
 * bl4: Convert to SVN display
 * 
 * Revision 1.11  91/09/16  17:32:12  rmurphy
 * bl3mup3: Change word-wrap limits
 * 
 * Revision 1.10  91/08/20  19:40:02  rmurphy
 * bl3mup1: Delete incorrect free of resource mgr data
 * 
 * Revision 1.9  91/08/17  18:39:26  rmurphy
 * bl3mup1: Remove prop. notice
 * 
 * Revision 1.8  91/08/17  11:03:27  rmurphy
 * bl3mup1: Support for body only printing
 * 
 * Revision 1.7  91/07/03  20:08:39  rmurphy
 * bl3: Fix text source handling
 * 
 * Revision 1.6  91/06/16  06:44:30  rmurphy
 * bl3: Help menu moved to Menus
 * 
 * Revision 1.5  91/06/15  06:31:31  rmurphy
 * bl3: Correct use of mhPath
 *
 * Revision 1.4  91/06/15  17:13:49  rmurphy
 * bl3: Remove leading period in customization file name
 * 
 * Revision 1.3  91/06/14  11:41:30  rmurphy
 * bl3: Convert to Motif
 * 
 * Revision 1.2  90/10/30  09:35:47  murphy
 * Convert to Motif V1.1.1
 * 
 * Revision 1.1  90/10/16  17:26:19  samia
 * Initial revision
 * 
 */

/* Init.c - Handle start-up initialization. */
#include "decxmail.h"
#include <stdlib.h>
#include <Xproto.h>
#include <DPS/dpsXclient.h> /* for DPS initialization -aju */
/* #include <Convert.h> */
#include "radio.h"
#include "toc.h"
#include "actionprocs.h"
#include "EDiskSrc.h" 


#define TITLESIZE       20  /* AJ 01 Length of the title (max) */
#define RIntegerRange	"IntegerRange"
 
/* Decxmail-specific resources. */ 

typedef struct {
  Boolean debug;
  Boolean trace;
  Boolean bgsend;		/* PJS: Backgrounded send, for speed. */
  char	  *tempDir;	/* Directory to use for temporary files. */
  char	  *defMhPath;		
  char	  *initialFolderName; /* Initial folder to use. */
  char	  *draftsFolderName;	/* Folder for drafts. */
  char	  *wastebasketFolderName;
  char    *popHostName;   /* Hostname for POP incorporation. */
  int	  defSendLineWidth;	/* How long to break lines on send. */
  int	  defBreakSendLineWidth; /* Minimum length of a line before break */
  char    *defPrintCommand; /* Printing command. */
  char	  *defPrintStrippedCommand;	/* Stripped print command */
  int	  defTocWidth;	/* How many characters wide to use in tocs */
  Boolean SkipDeleted;		/* If true, skip over deleted msgs. */
  Boolean SkipMoved;		/* If true, skip over moved msgs. */
  Boolean defHideBoringHeaders;
  Boolean defNewMailCheck; /* Whether to check for new mail. */
  Boolean defMakeCheckpoints; /* Whether to create checkpoint files. */
  Boolean 	AutoCommit;	/* Whether to automatically commit changes. */
  Boolean 	AutoPack;	/* Whether to auto-pack on commits. */
  Boolean     defOpenSubfolders;    /* Open subfolders when opening drawer */
  Boolean     defCloseSubfolders;   /* Close subfolders when opening folder */
  char	*ScanFormatFile; /* File containing scan format.  */
  char	*ScanCurrentString; /* String to use to indicate message is current. */
  char 	*ScanMovedString; /* String to use to indicate message is  moved. */
  char	*ScanDeletedString; /* String to use to indicate message is deleted. */
  char  *ScanIDCols;     /* Which columns in scan contain msg id. */
  char  *ScanCurrentCols; /* Which columns in scan indicate if this */
  char  *ScanMoveCols;   /* Which columns in scan indicate this */
  char  *ScanDeleteCols; /* Which columns in scan indicate this */
  char	*NormalCursorName; /* Name of cursor to show for normal use. */
  char	*SleepCursorName; /* Name of cursor to show during long ops. */
  Boolean	defDefaultViewBackwards; /* Whether to view messages in 
					    reverse order by default. */
  Boolean	defUseWastebasket; /* Whether to put deleted messages into a */
  /* wastebasket. */
  int		defWastebasketDaysToKeep; /* How many days to keep old msgs
			in the wastebasket.  If zero,  keep them forever. */
  Boolean	defIncOnShowUnopened; /* Whether to do an "inc" whenever
					 the user hits "show unopened". */
  Boolean	defAffectCurIfNullSelection;  /* Whether toc buttons should 
		    affect the  current message when there's no selection.*/
  int		defNewMailBorderWidth; /* Amount of border to add. */
  Boolean defBeepIfNoMail; /* Beep if no mail instead of using a dialog box. */
  Boolean defGrabFocus;	/* Grab focus on new windows. */
  Boolean defAnnotateReplies;	/* Whether to annotate replied-to msgs. */
  Boolean defAnnotateForwards; /* Whether to annotate forwarded msgs. */
  Boolean defDoReadPopups;	/* Whether to make read window popups. */
  int		defReplyCCAll;	/* yes/no/default:  Whether to cc replies to */
  /* eveyone. */
  int		defReplyCCMe;	/* yes/no/default: Whether to cc replies to */
  /* ourselves. */
  Boolean	defSVNStyle;	/* True if SVN layout */
  char * defChoiceEditor;       /* user specified editor */
} resource_list;

resource_list ResourceList;

static XtResource resources[] = {
    {"backgroundsend", "BackgroundSend", XtRBoolean, sizeof(Boolean),
	XtOffsetOf(resource_list, bgsend) , XtRString, "off"},
    {"debug", "Debug", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf(resource_list,debug), XtRString, "off"},
    {"trace", "Trace", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf(resource_list,trace), XtRString, "off"},
    {"tempdir", "tempDir", XtRString,  sizeof(char *),
	 XtOffsetOf(resource_list, tempDir) , XtRString,  "/tmp"},
    {"mhpath", "MhPath",  XtRString, sizeof(char *),
	 XtOffsetOf(resource_list, defMhPath), XtRString,  "/usr/bin/mh"},
    {"initialfolder", "InitialFolder", XtRString, sizeof(char *),
	 XtOffsetOf(resource_list, initialFolderName), XtRString,  "inbox"},
    {"draftsfolder", "DraftsFolder", XtRString, sizeof(char *),
	 XtOffsetOf(resource_list, draftsFolderName), XtRString,  "drafts"},
    {"wastebasketfolder", "WastebasketFolder", XtRString, sizeof(char *),
	 XtOffsetOf(resource_list, wastebasketFolderName), XtRString, "wastebasket"},
     {"popHost", "PopHost", XtRString, sizeof(char *),
          XtOffsetOf(resource_list, popHostName), XtRString,  NULL},
    {"sendwidth", "SendWidth", XtRInt, sizeof(int),
	 XtOffsetOf(resource_list, defSendLineWidth), XtRString, "985"},
    {"sendbreakwidth", "SendBreakWidth", XtRInt, sizeof(int),
	 XtOffsetOf(resource_list, defBreakSendLineWidth), XtRString,  "998"},
    {"printcommand", "PrintCommand", XtRString, sizeof(char *),
	 XtOffsetOf(resource_list, defPrintCommand), XtRString, PRINTDEFAULT },
    {"strippedprintcommand", "StrippedPrintCommand", XtRString, sizeof(char *),
	 XtOffsetOf(resource_list, defPrintStrippedCommand), XtRString, 
	  STRIPPEDPRINTDEFAULT },
    {"tocwidth", "TocWidth", XtRInt, sizeof(int),
	 XtOffsetOf( resource_list,  defTocWidth), XtRString, "100"},
    {"skipdeleted", "SkipDeleted", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list,  SkipDeleted), XtRString, "True"},
    {"skipmoved", "SkipMoved", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list,  SkipMoved), XtRString, "True"},
    {"hideboringheaders", "HideBoringHeaders", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list,  defHideBoringHeaders), XtRString, "True"},
    {"checknewmail", "CheckNewMail", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list,  defNewMailCheck), XtRString, "True"},
    {"makecheckpoints", "MakeCheckPoints", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list,  defMakeCheckpoints), XtRString, "False"},
    {"autocommit" , "AutoCommit", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list,  AutoCommit), XtRString, "True"},
    {"autoPack", "AutoPack", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list,  AutoPack), XtRString, "False"},
    {"scanformatfile", "ScanFormatFile", XtRString, sizeof(char *),
	 XtOffsetOf( resource_list,  ScanFormatFile), XtRString, ""},
    {"scanidcols", "ScanIDCols",XtRString, sizeof(IntRange),
	 XtOffsetOf( resource_list,  ScanIDCols), XtRString, "0-3"},
    {"scancurrentcols", "ScanCurrentCols",XtRString, sizeof(IntRange),
	 XtOffsetOf( resource_list,  ScanCurrentCols), XtRString, "4-4"},
    {"scanmovecols", "ScanMoveCols",XtRString, sizeof(IntRange),
	 XtOffsetOf( resource_list, ScanMoveCols), XtRString, "5-5"},
    {"scandeletecols", "ScanDeleteCols",XtRString, sizeof(IntRange),
	 XtOffsetOf( resource_list, ScanDeleteCols), XtRString, "5-5"},
    {"scancurrentstring", "ScanCurrentString", XtRString, sizeof(char *),
	 XtOffsetOf( resource_list, ScanCurrentString), XtRString, "+"},
    {"scanmovedstring", "ScanMovedString", XtRString, sizeof(char *),
	 XtOffsetOf( resource_list, ScanMovedString), XtRString, "^"},
    {"scandeletedstring", "ScanDeletedString", XtRString, sizeof(char *),
	 XtOffsetOf( resource_list, ScanDeletedString), XtRString, "D"},
    {"normalcursorname", "NormalCursorName", XtRString, sizeof(char *),
	 XtOffsetOf( resource_list, NormalCursorName), XtRString, "left_ptr"},
    {"sleepcursorname", "SleepCursorName", XtRString, sizeof(char *),
	 XtOffsetOf( resource_list, SleepCursorName), XtRString, "watch"},
    {"newMailBorderWidth", "NewMailBorderWidth", XtRInt, sizeof(int),
	 XtOffsetOf( resource_list, defNewMailBorderWidth), XtRString, "2"},
    {"defaultViewBackwards", "DefaultViewBackwards", XtRBoolean,
	 sizeof(Boolean), XtOffsetOf( resource_list, defDefaultViewBackwards),
	 XtRString, "False"},
    {"useWastebasket", "UseWastebasket", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list, defUseWastebasket), XtRString, "True"},
    {"wastebasketDaysToKeep", "WastebasketDaysToKeep", XtRInt, sizeof(int),
	 XtOffsetOf( resource_list, defWastebasketDaysToKeep), XtRString, "0"},
    {"incOnShowUnopened", "IncOnShowUnopened", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list, defIncOnShowUnopened), XtRString, "True"},
    {"affectCurrentMsgIfNullSelection", "AffectCurrentMsgIfNullSelection",
	 XtRBoolean, sizeof(Boolean), XtOffsetOf( resource_list, defAffectCurIfNullSelection),
	 XtRString, "False"},
    {"openSubfoldersWithDrawer", "OpenSubfoldersWithDrawer",
	 XtRBoolean, sizeof(Boolean), XtOffsetOf( resource_list, defOpenSubfolders),
	 XtRString, "True"},
    {"autoCloseSubfolders", "AutoCloseSubfolders",
	 XtRBoolean, sizeof(Boolean), XtOffsetOf( resource_list, defCloseSubfolders),
	 XtRString, "True"},
    {"beepIfNoMail", "BeepIfNoMail", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list, defBeepIfNoMail), XtRString, "False"},
    {"grabFocus", "GrabFocus", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list, defGrabFocus), XtRString, "False"},
    {"annotateReplies", "AnnotateReplies", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list, defAnnotateReplies), XtRString, "False"},
    {"annotateForwards", "AnnotateForwards", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list, defAnnotateForwards), XtRString, "False"},
    {"readPopups", "ReadPopups", XtRBoolean, sizeof(Boolean),
	 XtOffsetOf( resource_list, defDoReadPopups), XtRString, "True"},
    {"replyCCAll", "ReplyCCAll", RYesNoDefault, sizeof(int),
	 XtOffsetOf( resource_list, defReplyCCAll), XtRString, "default"},
    {"replyCCMe", "ReplyCCMe", RYesNoDefault, sizeof(int),
	 XtOffsetOf( resource_list, defReplyCCMe), XtRString, "default"},
    {"layoutStyle", "LayoutStyle", RLayoutStyle, sizeof(char *),
	 XtOffsetOf( resource_list, defSVNStyle), XtRString, "paned"},
     {"choiceEditor", "ChoiceEditor", XtRString, sizeof(char *),
         XtOffsetOf(resource_list, defChoiceEditor), XtRString,"dxnotepad %s"},
  };

char *defChoiceEditor;

static XrmOptionDescRec table[] = {
    {"-trace",	"*trace",	XrmoptionNoArg, "on"},
    {"-debug",	"*debug",	XrmoptionNoArg,	"on"}
};



static XtActionsRec actions[] = {
    {"help-menu", 		 (XtActionProc)   ExecCreateHelpMenu},
    {"help-oncontext", 		 (XtActionProc)   ExecOnContext},
    {"do-prompt-yes", 		 (XtActionProc)   DoPromptYes},
    {"do-pick-yes",		 (XtActionProc)   DoPickYes},
    {"quit",			 (XtActionProc)   ExecQuit},
    {"close-scrn",		 (XtActionProc)   ExecCloseScrn},
    {"reply-msg",		 (XtActionProc)   ExecThisReply},
    {"forward-msg",		 (XtActionProc)   ExecThisForward},
    {"viewindefault-msg",	 (XtActionProc)   ExecThisInDefault},
    {"viewinnew-msg",		 (XtActionProc)   ExecThisInNew},
    {"useascomp-msg",		 (XtActionProc)   ExecThisUseAsComposition},
    {"comp-using-file",		 (XtActionProc)   ExecComposeUsingFile},
    {"edit-msg",		 (XtActionProc)   ExecEditView},
    {"save-msg",		 (XtActionProc)   ExecSaveView},
    {"print-msg-selective", 	 (XtActionProc)   ExecThisPrintSelective},
    {"print-msg",		 (XtActionProc)   ExecThisPrint},
    {"print-msg-stripped",	 (XtActionProc)   ExecThisPrintStripped},
    {"move-msg",		 (XtActionProc)   ExecThisMove},
    {"copy-msg",		 (XtActionProc)   ExecThisCopy},
    {"move-msg-dialog",		 (XtActionProc)   ExecThisMoveDialog},
    {"copy-msg-dialog",		 (XtActionProc)   ExecThisCopyDialog},
    {"delete-msg",		 (XtActionProc)   ExecThisDelete},
    {"unmark-msg",		 (XtActionProc)   ExecThisUnmark},
    {"reset-comp",		 (XtActionProc)   ExecCompReset},
    {"new-comp",		 (XtActionProc)   ExecComposeMessage},
    {"save-comp",		 (XtActionProc)   ExecSaveDraft},
    {"send-comp",		 (XtActionProc)   ExecSendDraft},
    {"open-folder",		 (XtActionProc)   ExecOpenFolder},
    {"close-folder",		 (XtActionProc)   ExecCloseFolder},
    {"openinnew-folder",	 (XtActionProc)   ExecOpenFolderInNewWindow},
    {"create-folder",		 (XtActionProc)   ExecCreateFolder},
    {"show-folder",		 (XtActionProc)   ExecShowFolder},
    {"hide-folder",		 (XtActionProc)   ExecHideFolder},
    {"delete-folder",		 (XtActionProc)   ExecDeleteFolder},
    {"incorporate",		 (XtActionProc)   ExecIncorporate},
    {"read-new-mail",		 (XtActionProc)   ExecReadNewMail},
    {"read-new-mail-in-new",	 (XtActionProc)   ExecReadNewMailInNew},
    {"show-unseen",		 (XtActionProc)   ExecShowUnseen},
    {"toc-next-msg",		 (XtActionProc)   ExecNextView},
    {"toc-prev-msg",		 (XtActionProc)   ExecPrevView},
    {"view-next-msg",		 (XtActionProc)   ExecViewAfter},
    {"view-prev-msg",		 (XtActionProc)   ExecViewBefore},
    {"view-next-selected",	 (XtActionProc)   ExecNextSelected},
    {"view-prev-selected",	 (XtActionProc)   ExecPrevSelected},
    {"view-new-mail",		 (XtActionProc)   ExecNewMailInView},
    {"addtoseq-msg",		 (XtActionProc)   ExecMsgAddToSeq},
    {"removefromseq-msg",	 (XtActionProc)   ExecMsgRemoveFromSeq},
    {"make-default-window",	 (XtActionProc)   ExecMakeDefaultView},
    {"insert-file",		 (XtActionProc)   ExecInsertFile},
/* PJS */
    {"extract-msg",		 (XtActionProc)   ExecExtractMsg},
    {"extract-selected",	 (XtActionProc)   ExecExtractMsg},
    {"extract-msg-strip",	 (XtActionProc)   ExecExtractMsgStrip},
    {"delete-selected",		 (XtActionProc)   ExecMarkDelete},
    {"move-selected",		 (XtActionProc)   ExecMarkMove},
    {"copy-selected",		 (XtActionProc)   ExecMarkCopy},
    {"move-selected-dialog",	 (XtActionProc)   ExecMarkMoveDialog},
    {"copy-selected-dialog",	 (XtActionProc)   ExecMarkCopyDialog},
    {"unmark-selected",		 (XtActionProc)   ExecMarkUnmarked},
    {"viewinnew-selected",	 (XtActionProc)   ExecViewNew},
    {"viewindefault-selected",	 (XtActionProc)   ExecViewInDefault},
    {"viewinspecified-selected", (XtActionProc)   ExecViewInSpecified},
    {"reply-selected",		 (XtActionProc)   ExecTocReply},
    {"forward-selected",	 (XtActionProc)   ExecTocForward},
    {"useascomp-selected",	 (XtActionProc)   ExecTocUseAsComposition},
    {"commit",			 (XtActionProc)   ExecCommitChanges},
    {"empty-wastebasket",	 (XtActionProc)   ExecEmptyWastebasket},
    {"create-seq",		 (XtActionProc)   ExecCreateSeq},
    {"open-seq",		 (XtActionProc)   ExecOpenSeq},
    {"addto-seq",		 (XtActionProc)   ExecAddToSeq},
    {"removefrom-seq",		 (XtActionProc)   ExecRemoveFromSeq},
    {"pick",			 (XtActionProc)   ExecPick},
    {"pick-selected-folder",	 (XtActionProc)   ExecPickInSelected},
    {"delete-seq",		 (XtActionProc)   ExecDeleteSeq},
/* PJS - The next line was missing... What should it be? */
    {"print-setup",		 (XtActionProc)   NoOp},
    {"print-selected",		 (XtActionProc)   ExecPrintMessages},
    {"print-widget",		 (XtActionProc)   ExecPrintWidget},
    {"print-selected-strip",	 (XtActionProc)   ExecPrintStripped},
    {"print-widget-strip",	 (XtActionProc)   ExecPrintWidgetStripped},
    {"pack",			 (XtActionProc)   ExecPack},
    {"sort",			 (XtActionProc)   ExecSort},
    {"rescan",			 (XtActionProc)   ExecForceRescan},
    {"select-this-msg",		 (XtActionProc)   ExecSelectThisMsg},
    {"extend-this-msg",		 (XtActionProc)   ExecExtendThisMsg},
    {"extend-this-toc",		 (XtActionProc)   ExecExtendThisToc},
    {"cut",			 (XtActionProc)   ExecCut},
/* PJS - no action entry for 'copy-to-clipboard': is this right...? */
    {"copy-to-clipboard",	 (XtActionProc)   ExecCopy},
    {"copy",			 (XtActionProc)   ExecCopy},
    {"paste",			 (XtActionProc)   ExecPaste},
    {"select-all",		 (XtActionProc)   ExecSelectAll},
    {"customize",		 (XtActionProc)   CustomizeCreate},
    {"on-double",		 (XtActionProc)   ExecOnDouble},
    {"do-popup",		 (XtActionProc)   ExecPopupMenu},
    {"read-in-ddif-viewer",	 (XtActionProc)   ExecViewInDDIFViewer},
    {"dump-widget-hierarchy",	 (XtActionProc)   ExecDumpScrnWidgetHierarchy},
    {"rescan-folders-update",	 (XtActionProc)   TocResetAll},
    {"no-op",			 (XtActionProc)   NoOp},
    {"resize-folder-box",	 (XtActionProc)   ResizeFolderBox},
    {"hide-subfolders",		 (XtActionProc)   ExecHideSubfolders},
    {"show-subfolders",		 (XtActionProc)   ExecShowSubfolders},
    {"custom-editor",		 (XtActionProc)   ExecCustomEditor},
    {"ps-orient",                (XtActionProc)   ExecPSOrient},
    {"rename-folder",            (XtActionProc)   ExecRenameFolder},
    {"custom-editor",            (XtActionProc)   ExecCustomEditor},
};				
/* ajs */
static XtAppContext app_context;    /* Application context from XtAppInitialize
*/

/* Tell the user how to use this program. */
Syntax()
{
    extern void exit();
    (void)fprintf(stderr, "usage:  %s [display] [=geometry]\n", progName);
    exit(2);
}


/*
 * Converter to convert a string of form "num-num" to an integer
 * array of length two. 
 */
/* ARGSUSED */
static void CvtStringToIntRange(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static IntRange i;
    if (*num_args != 0)
        XtWarningMsg("cvtIntToPixmap","wrongParameters","decxmailError",
		 "String to Integer-range conversion needs no extra arguments",
		     NULL,NULL);
    if (sscanf((char *)fromVal->addr, "%u-%u", &(i[0]), &(i[1])) == 2) {
	toVal->size = sizeof(IntRange);
	toVal->addr = (caddr_t) i;
    } else {
	XtStringConversionWarning((char *) fromVal->addr, "Integer-range");
    }
}

/*
 * Converter to convert a string to type XmRResizePolicy.
 */
/* ARGSUSED */
static void CvtStringToResize(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static char result;
    char *ptr;
    if (*num_args != 0)
        XtWarningMsg("cvtStringToResize","wrongParameters","decxmailError",
		     "String to Resize conversion needs no extra arguments",
		     NULL,NULL);
    ptr = (char *) fromVal->addr;
    result = 99;
    if (strncmpIgnoringCase("fixed", ptr, 5) == 0)
	result = XmRESIZE_NONE;
    if (strncmpIgnoringCase("growonly", ptr, 8) == 0)
	result = XmRESIZE_GROW;
    if (strncmpIgnoringCase("shrinkwrap", ptr, 10) == 0)
	result = XmRESIZE_ANY;
    if (result != 99) {
	toVal->size = sizeof(char);
	toVal->addr = (caddr_t) &result;
    } else {
	XtStringConversionWarning((char *) fromVal->addr, "Resize");
    }
}

/*
 * Convert a string to a yes/no/default value. 
 */

/* ARGSUSED */
static void CvtStringToYesNoDefault(args, num_args, fromVal, toVal)
XrmValuePtr args;
Cardinal *num_args;
XrmValuePtr fromVal;
XrmValuePtr toVal;
{
    static int result;
    char *ptr;
    if (*num_args != 0)
        XtWarningMsg("cvtStringToYesNoDefault","wrongParameters",
		     "decxmailError",
		  "String to YesNoDefault conversion needs no extra arguments",
		     NULL, NULL);
    ptr = (char *) fromVal->addr;
    result = 99;
    if (strncmpIgnoringCase("default", ptr, 7) == 0)
	result = DEFAULT;
    if (strncmpIgnoringCase("on", ptr, 2) == 0)
	result = TRUE;
    if (strncmpIgnoringCase("off", ptr, 3) == 0)
	result = FALSE;
    if (result != 99) {
	toVal->size = sizeof(int);
	toVal->addr = (caddr_t) &result;
    } else {
	XtStringConversionWarning((char *) fromVal->addr, "YesNoDefault");
	result = DEFAULT;
    }
}
/*
 * Converter to convert a string to type LayoutStyle.
 */
/* ARGSUSED */
static void CvtStringToLayout(args, num_args, fromVal, toVal)
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
{
    static Boolean result;
    char *ptr;
    if (*num_args != 0)
        XtWarningMsg("cvtStringToLayout","wrongParameters","decxmailError",
		     "String to Layout conversion needs no extra arguments",
		     NULL,NULL);
    ptr = (char *) fromVal->addr;
    result = 99;
    if (strncmpIgnoringCase("paned", ptr, 5) == 0)
	result = False;
    if (strncmpIgnoringCase("svn", ptr, 3) == 0)
	result = True;
    if (strncmpIgnoringCase("outline", ptr, 7) == 0)
	result = True;
    if (result != 99) {
	toVal->size = sizeof(Boolean);
	toVal->addr = (caddr_t) &result;
    } else {
	XtStringConversionWarning((char *) fromVal->addr, "Layout");
    }
}
    
/*
 * Make sure that the given scan string is long enough to fill the given
 * scan columns.  Change the cols arg to be start-length, rather than
 * start-end.
 */

static void FixUpScanString(str, cols)
char **str;
IntRange cols;
{
    char *ptr;
    int length = strlen(*str);
    register int i;
    cols[1] = cols[1] - cols[0] + 1;
    if (length < cols[1]) {
	ptr = XtMalloc((Cardinal) (cols[1] + 1));
	(void) strcpy(ptr, *str);
	for (i=length ; i<cols[1] ; i++)
	    (void) strcat(ptr, " ");
	*str = ptr;
    }
}



/* ARGSUSED */
static int IgnoreFocusErrors(dpy, event)
    Display *dpy;
    XErrorEvent *event;
{
    if (event->request_code == X_SetInputFocus)
	return 0;
#ifdef notdef
    return _XDefaultError(dpy, event);
#else
    return 0;
#endif
}

static void
setupCommandPaths(path)
char *path;
{
    char str[100];

    sprintf(str, "%s/anno", path);
    annoCmd = XtNewString(str);
    sprintf(str, "%s/comp", path);
    compCmd = XtNewString(str);
    sprintf(str, "%s/folder", path);
    folderCmd = XtNewString(str);
    sprintf(str, "%s/forw", path);
    forwCmd = XtNewString(str);
    sprintf(str, "%s/inc", path);
    incCmd = XtNewString(str);
    sprintf(str, "%s/mark", path);
    markCmd = XtNewString(str);
    sprintf(str, "%s/pick", path);
    pickCmd = XtNewString(str);
    sprintf(str, "%s/refile", path);
    refileCmd = XtNewString(str);
    sprintf(str, "%s/repl", path);
    replCmd = XtNewString(str);
    sprintf(str, "%s/rmf", path);
    rmfCmd = XtNewString(str);
    sprintf(str, "%s/rmm", path);
    rmmCmd = XtNewString(str);
    sprintf(str, "%s/scan", path);
    scanCmd = XtNewString(str);
    sprintf(str, "%s/send", path);
    sendCmd = XtNewString(str);
    sprintf(str, "%s/sortm", path);
    sortmCmd = XtNewString(str);
}
/* All the start-up initialization goes here. */
char  user_title[TITLESIZE+1];
char  dxm_maildrop[1028];

InitializeWorld(argc, argv)
unsigned int argc;
char **argv;
{
    register int l;
    FILE *fid;
    char str[500], str2[500], *ptr, *ptr2, oldcustomfile[500];
    char  *userfilepath, *userfilepathfmt, *appledir;
    char  pathfmtbuf[1024];
    Boolean found;
    Scrn scrn;
    XrmValue value;
    struct stat *buffer;
    void InitDPS();

    ptr = rindex(argv[0], '/');
    if (ptr) progName = ptr + 1;
    else progName = argv[0];


    MrmInitialize();
    DXmInitialize();
    toplevel = XtInitialize(progName, PROGCLASS, table, XtNumber(table),
			   (int *)&argc, argv);

  app_context = XtWidgetToApplicationContext(toplevel);

/* ajs - XtSetLanguageProc set's a locale for the toolkit in Motif 1.2 or later */
#if (((XmVERSION == 1) && (XmREVISION >= 2)) || XmVERSION >= 2)
    XtSetLanguageProc(app_context, NULL, app_context);
#endif

    user_title[0] = 0;

    if (argc > 1)
       {
	 if (strncmp(argv[1],"-t",2) == 0)
	   (void)strncpy (user_title, argv[2], TITLESIZE);
	 else
	   Syntax();
       }
 

/* code to maintain compatibility with earlier dxMail configuration file */

    homeDir = MallocACopy(getenv("HOME"));
    (void) sprintf(oldcustomfile, "%s/%s", homeDir, "dxMail");
 
/*  Need to resolve the user application resource file name same as Xt. */

    customfile[0] = '\0';

    userfilepathfmt = getenv("XUSERFILESEARCHPATH");
    if ( !userfilepathfmt ) {
       appledir = getenv("XAPPLRESDIR");
       if ( appledir ) {
          sprintf( pathfmtbuf, "%s%s%s%s%s%s%s%s", appledir, "/%L/%N:",
                   appledir, "/%1/%N:", appledir, "/%N:", homeDir, "/%N" );
       }
       else {
          sprintf( pathfmtbuf, "%s%s%s%s%s%s", homeDir, "/%L/%N:",
                   homeDir, "/%1/%N:", homeDir, "/%N" );
       }
       userfilepathfmt = pathfmtbuf;
    }

    userfilepath = XtResolvePathname( XtDisplay(toplevel), NULL, NULL,
                                 NULL, userfilepathfmt, NULL, 0, NULL );

    if ( userfilepath ) {
       strcpy( customfile, userfilepath );
       XtFree( userfilepath );
    }
    else {
       (void) sprintf(customfile, "%s/%s", homeDir, PROGCLASS);
    }


    fid = myfopen (customfile , "r");
    if (fid  == NULL) {
       fid = myfopen (oldcustomfile, "r");
          if (fid != NULL){
              printf("Warning: copying %s to %s\n", "dxMail", PROGCLASS);
              myfclose (fid);
             (void) sprintf(str, "cp %s %s", oldcustomfile, customfile );
              system (str);
             str[0] = 0x00;
          }
    }
    else {
      myfclose(fid);
    }

/*    XrmPutFileDatabase(DefaultDB, "junk"); */

    XtAddConverter(XtRString, RIntegerRange, CvtStringToIntRange,
		   (XtConvertArgList) NULL, (Cardinal) 0);
    XtAddConverter(XtRString, XmRResizePolicy, CvtStringToResize,
		   (XtConvertArgList) NULL, (Cardinal) 0);
    XtAddConverter(XtRString, RYesNoDefault, CvtStringToYesNoDefault,
		   (XtConvertArgList) NULL, (Cardinal) 0);
    XtAddConverter(XtRString, RLayoutStyle, CvtStringToLayout,
		   (XtConvertArgList) NULL, (Cardinal) 0);

    theDisplay = XtDisplay(toplevel);
    theScreen = DefaultScreen(theDisplay);
    DefaultDB = XtDatabase(theDisplay);


    sprintf(str2, "%s/.mh_profile", homeDir);
    profileFileName = MallocACopy(str2);
    fid = myfopen(profileFileName, "r");
    if (fid == NULL) {
	fid = myfopen(profileFileName, "w");
	if (fid) {
	    (void) fprintf(fid, "Path: Mail\n");
	    (void) fprintf(fid, "Unseen-Sequence: unseen\n");
	    (void) myfclose(fid);
	    fid = myfopen(profileFileName, "r");
	}
    }
    unseenSeqName = NULL;
    found = hasRmmProc = FALSE;
    if (fid) {
	while (ptr = ReadLine(fid)) {
	    if (strncmpIgnoringCase(ptr, "Path:", 5) == 0) {
		ptr += 5;
		while (*ptr == ' ' || *ptr == '\t')
		    ptr++;
		(void) strcpy(str, ptr);
	      }
	    if (strncmpIgnoringCase(ptr, "Unseen-Sequence:", 16) == 0) {
		found = TRUE;
		ptr += 16;
		while (*ptr == ' ' || *ptr == '\t')
		    ptr++;
		if (*ptr) {
		    ptr2 = ptr;
		    while (*ptr2 && *ptr2 != ' ' && *ptr2 != '\t') ptr2++;
		    *ptr2 = 0;
		    unseenSeqName = MallocACopy(ptr);
		}
	    }
            if (strncmpIgnoringCase(ptr, "MailDrop:", 9) == 0) {
                ptr += 9;
                while (*ptr == ' ' || *ptr == '\t')
                   ptr++;
                strcpy( dxm_maildrop, ptr );
            }
	    if (strncmpIgnoringCase(ptr, "rmmproc:", 8) == 0)
		hasRmmProc = TRUE;
	}
	(void) myfclose(fid);
	if (!found) {
	    fid = myfopen(profileFileName, "a");
	    if (fid) {
		(void) fprintf(fid, "Unseen-Sequence: unseen\n");
		(void) myfclose(fid);
		unseenSeqName = "unseen";
	    }
	}
    } else Punt("Can't read or create .mh_profile!");
    for (l = strlen(str) - 1; l >= 0 && (str[l] == ' ' || str[l] == '\t'); l--)
	str[l] = 0;
    if (str[0] == '/')
	(void) strcpy(str2, str);
    else
	(void) sprintf(str2, "%s/%s", homeDir, str);
    mailDir = MallocACopy(str2);
    (void) sprintf(str, "%s/draft", mailDir);
    draftFile = MallocACopy(str);
    (void) sprintf(str, "%s/decxmaildraft", mailDir);
    draftScratchFile = MallocACopy(str);

    XtGetApplicationResources(toplevel, (XtPointer) &ResourceList,
			      resources, XtNumber(resources),
			      (Arg *)NULL, (Cardinal) 0);
    MapStructureToGlobals();

    FixUpScanString(&ScanCurrentString, ScanIDCols);
	/* Just a cheap way to fix up ScanIDCols. */

    FixUpScanString(&ScanCurrentString, ScanCurrentCols);
    FixUpScanString(&ScanMovedString, ScanMoveCols);
    FixUpScanString(&ScanDeletedString, ScanDeleteCols);

    NullSource = XtCreateEDiskSource("/dev/null", FALSE,NULL,NULL);
    NullTocSource = (XmTextSource) NULL;
    DEBUG(("Created Null Sources %x/%x\n",NullSource,NullTocSource));

    l = strlen(defMhPath) - 1;
    if (l > 0 && defMhPath[l] == '/')
	defMhPath[l] = 0;

    sprintf(str,"%s/inc", defMhPath);
    if ((fid = myfopen(str, "r")) == NULL) {
	fprintf(stderr,"dxmail fatal error:\n");
        fprintf(stderr,"The Rand Mail Handler can't be found using your mhpath of %s\n",
		 defMhPath);
        fprintf(stderr,"Verify that MH is installed and that the path is correct.\n");
	exit(2);
    }
    (void) myfclose(fid);
    setupCommandPaths(defMhPath);
    EmptyCompoundString = XmStringCreateSimple("");

    rootwidth = DisplayWidth(theDisplay, theScreen);
    rootheight = DisplayHeight(theDisplay, theScreen);

    numScrns = 0;
    scrnList = (Scrn *) XtMalloc((unsigned) 1);

    TocInit();
/*    InitPick(); */
    IconInit();

    value.size = sizeof(Pixmap);
    value.addr = (caddr_t) &GrayPixmap;

    XrmPutResource(&DefaultDB, "*innerFolderArea.XmPushbutton.borderPixmap",
		   XtRPixmap, &value);

    InitActions(actions, XtNumber(actions));
    InitMenu();		/* This is an empty function... */
    InitPopup();
    InitMsg();

#ifdef IGNOREDDIF
    dps_exists = FALSE;
#else /*IGNOREDDIF */
    InitDPS();          /* This function is defined in this file (to initialize XDPS */
#endif /*IGNOREDDIF */
     

    XtAddActions(actions, XtNumber(actions));

    DEBUG(("Making screen ... "));

    ReadWindowsRadio = RadioCreate();

DEBUG(("Done radio...\n"));

    XSetErrorHandler(IgnoreFocusErrors);

DEBUG(("Done errors...\n"));

    if (defSVNStyle) 
	scrn = CreateNewScrn("mainoutline");
    else
	scrn = CreateNewScrn("main");

    /* Set WM_COMMAND property so "xlsclients" sees us. */

    XSetCommand( theDisplay, XtWindow(scrn->parent), argv, argc );

    DEBUG((" setting toc ... "));

    TocSetScrn(InitialFolder, scrn);

    if (!defSVNStyle) {
	RadioSetOpened(scrn->folderradio, TocGetFolderName(InitialFolder));
    }

    DEBUG(("done\n"));

/* if (debug) {(void)fprintf(stderr, "Syncing ... "); (void)fflush(stderr); XSync(theDisplay, 0); (void)fprintf(stderr, "done\n");} */

/*
    ptr = GetApplicationResourceAsString("main.InitialState", "InitialState");
    if (ptr) {
	defInitialState = atoi(ptr);
    } else {
	defInitialState = IconicState;
    }
*/
	defInitialState = 0;
    newSVNStyle = defSVNStyle;

    MapScrn(scrn, 0);

}


void MapStructureToGlobals()
{
debug=ResourceList.debug;
trace=ResourceList.trace;
bgsend=ResourceList.bgsend;
tempDir=ResourceList.tempDir;
defMhPath=ResourceList.defMhPath;
initialFolderName=ResourceList.initialFolderName;
draftsFolderName=ResourceList.draftsFolderName;
wastebasketFolderName=ResourceList.wastebasketFolderName;
popHostName=ResourceList.popHostName;
defSendLineWidth=ResourceList.defSendLineWidth;
defBreakSendLineWidth=ResourceList.defBreakSendLineWidth;
defPrintCommand=ResourceList.defPrintCommand;
defPrintStrippedCommand=ResourceList.defPrintStrippedCommand;
defTocWidth=ResourceList.defTocWidth;
SkipDeleted=ResourceList.SkipDeleted;
SkipMoved=ResourceList.SkipMoved;
defHideBoringHeaders=ResourceList.defHideBoringHeaders;
defNewMailCheck=ResourceList.defNewMailCheck;
defMakeCheckpoints=ResourceList.defMakeCheckpoints;
AutoCommit=ResourceList.AutoCommit;
AutoPack=ResourceList.AutoPack;
defOpenSubfolders=ResourceList.defOpenSubfolders;
defCloseSubfolders=ResourceList.defCloseSubfolders;
ScanFormatFile=ResourceList.ScanFormatFile;
ScanCurrentString=ResourceList.ScanCurrentString;
ScanMovedString=ResourceList.ScanMovedString;
ScanDeletedString=ResourceList.ScanDeletedString;

ScanIDCols[0]=get_first_number(ResourceList.ScanIDCols);
ScanCurrentCols[0]=get_first_number(ResourceList.ScanCurrentCols);
ScanMoveCols[0]=get_first_number(ResourceList.ScanMoveCols);
ScanDeleteCols[0]=get_first_number(ResourceList.ScanDeleteCols);
ScanIDCols[1]=get_last_number(ResourceList.ScanIDCols);
ScanCurrentCols[1]=get_last_number(ResourceList.ScanCurrentCols);
ScanMoveCols[1]=get_last_number(ResourceList.ScanMoveCols);
ScanDeleteCols[1]=get_last_number(ResourceList.ScanDeleteCols);


NormalCursorName=ResourceList.NormalCursorName;
SleepCursorName=ResourceList.SleepCursorName;
defDefaultViewBackwards=ResourceList.defDefaultViewBackwards;
defUseWastebasket=ResourceList.defUseWastebasket;
defWastebasketDaysToKeep=ResourceList.defWastebasketDaysToKeep;
defIncOnShowUnopened=ResourceList.defIncOnShowUnopened;
defAffectCurIfNullSelection=ResourceList.defAffectCurIfNullSelection;
defNewMailBorderWidth=ResourceList.defNewMailBorderWidth;
defBeepIfNoMail=ResourceList.defBeepIfNoMail;
defGrabFocus=ResourceList.defGrabFocus;
defAnnotateReplies=ResourceList.defAnnotateReplies;
defAnnotateForwards=ResourceList.defAnnotateForwards;
defDoReadPopups=ResourceList.defDoReadPopups;
defReplyCCAll=ResourceList.defReplyCCAll;
defReplyCCMe=ResourceList.defReplyCCMe;
defSVNStyle=ResourceList.defSVNStyle;
defChoiceEditor=ResourceList.defChoiceEditor;
}


int get_first_number(range_str)
char *range_str;
{
char in_string[8], *ptr;
int i;

strcpy (in_string, range_str);
ptr = index(in_string, '-');
*ptr = 0;
i = atoi(in_string);
return(i);
}


int get_last_number(range_str)
char *range_str;
{
char in_string[8], *ptr1;
int i;

strcpy (in_string, range_str);
ptr1 = index(in_string, '-');
ptr1++;

if (*ptr1 != ' ' && *ptr1!='\t' && isdigit(*ptr1))
  i = atoi(ptr1);
else
  i = -1;
return(i);
}   

#ifndef IGNOREDDIF   /* flag that turns off all CDA and PS support */

/* InitDPS - checks to see if XDPS library is loaded. If so,
   sets the global variable dps_exists */

void InitDPS()
{

extern Boolean check_for_xdps();

    /*  find out if server has display postscript extension;
     *  allow MIT registered string or previously used DECwindows
     *  string
     */

    dps_exists = check_for_xdps(toplevel);

}

/* 
 * The following function checks to see if the DPS extn library is loaded 
 */

Boolean check_for_xdps (vw)
    Widget   vw;                            /* widget associated with a window      */

{
    char        **list_of_extensions;       /* array of extension name strings      */
    char        **extension_pointer;        /* pointer into the list of extensions  */
    int         nextensions_return;         /* number of extensions returned        */
    int         i;                          /* loop index        */
    Boolean     return_status;              /* return status        */

    /*
     * Initially assume that we weren't able to locate the XDPS extension
     */

    return_status = FALSE;

    /*
     * Ask the server for the list of known extensions available.
     */

    list_of_extensions = XListExtensions(XtDisplay(vw), 
					 /* Display associated with the server   */
					 &nextensions_return
					 /* Number of extensions returned  */
					 );  

    extension_pointer = list_of_extensions;

    for ( i = 0; i < nextensions_return; i++)
        {
            if ((strcmp(*extension_pointer, "Adobe-DPS-Extension") == 0) || 
                (strcmp(*extension_pointer, "DPSExtension") == 0))
                {
                    return_status = TRUE;
                    break;
                };

            extension_pointer++;
        };

    XFreeExtensionList(list_of_extensions);

    if (return_status)
      XDPSSetEventDelivery(XtDisplay(vw),  dps_event_pass_through);
    else
      DEBUG(("No DPS library support.\n"));

    return (return_status);
}



#endif /*  IGNOREDDIF that turns off all CDA and PS support */
