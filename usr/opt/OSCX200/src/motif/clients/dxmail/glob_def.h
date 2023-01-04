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
	@(#)$RCSfile: glob_def.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 23:57:12 $
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


typedef struct _resource_list {
  Boolean	debug;
  Boolean	trace;
  Boolean	bgsend;		/* PJS: Backgrounded send, for speed. */
  
   Display	*theDisplay;	/* Display variable. */
  int		theScreen;	/* Which screen we're using. */
   Widget	toplevel;	/* The top level widget (A hack %%%). */
   XrmDatabase	DefaultDB;	/* Database for decxmail data. */
  
  char	*progName;	/* Program name. */
  char	*homeDir;	/* User's home directory. */
  char	*mailDir;	/* mh's mail directory. */
  char	*tempDir;	/* Directory to use for temporary files. */
  char	*initialFolderName; /* Initial folder to use. */
  char	*draftsFolderName;	/* Folder for drafts. */
  char	*wastebasketFolderName;	/* Folder for wastebasket. */
  char        *popHostName;   /* Hostname for POP incorporation. */
  char	*draftFile;		/* Filename of draft. */
  char	*draftScratchFile;	/* Filename for sending. */
  char	*profileFileName; /* Full filename of .mh_profile. */
  Boolean	hasRmmProc;	/* True if the user has a rmmproc. */
  
  char	*editcommand;		/* Command to exec to composing. */
  char	*defMhPath;		/* Path for mh commands. */
  int		defSendLineWidth;	/* How long to break lines on send. */
  int		defBreakSendLineWidth;	/* Minimum length of a line before
					   we'll break it. */
  char	*defPrintCommand; /* Printing command. */
  char	*defPrintStrippedCommand;	/* Stripped print command */
  
  int		defTocWidth;	/* How many characters wide to use in tocs */
  
  Boolean	SkipDeleted;		/* If true, skip over deleted msgs. */
  Boolean	SkipMoved;		/* If true, skip over moved msgs. */
  
  Boolean	defHideBoringHeaders;
  
  Boolean	defNewMailCheck; /* Whether to check for new mail. */
  Boolean	defMakeCheckpoints; /* Whether to create checkpoint files. */
  
  int		defNewMailBorderWidth; /* Amount of border to add. */
  
  Boolean	defOpenSubfolders;    /* Open subfolders when opening drawer */
  
  Boolean	defCloseSubfolders;   /* Close subfolders when opening folder */
  
  int		defInitialState;	/* Initial state */
  
   Toc		*folderList;	/* Array of folders. */
  int		numFolders;	/* Number of entries in above array. */
   Toc		InitialFolder;	/* Toc containing initial folder. */
   Toc		DraftsFolder;	/* Toc containing drafts. */
   Toc		WastebasketFolder; /* Toc for wastebasket. */
  
  char	*unseenSeqName;	/* Sequence for unseen messages. */
  
   Scrn	*scrnList;	/* Array of scrns in use. */
  int		numScrns;	/* Number of scrns in above array. */
/*  
   XmTSource NullSource;
   XmTSource NullTocSource;
  */
   Dimension	rootwidth;	/* Dimensions of root window.  */
   Dimension	rootheight;
  
   Pixmap	NoMailPixmap;	/* Icon pixmap if no new mail. */
   Pixmap	NewMailPixmap;	/* Icon pixmap if new mail. */
   Pixmap	NoMailSmallPixmap; /* Iconify button pixmap if no new mail. */
   Pixmap	NewMailSmallPixmap; /* Iconify button pixmap if new mail. */
   Pixmap	GrayPixmap;	/* Gray pixmap. */
  
   Widget	PopupWidget;	/* Widget last popup menu was created in. */
  
  char	customfile[256]; /* Filename for customization file. */
  
  Boolean 	AutoCommit;	/* Whether to automatically commit changes. */
  Boolean 	AutoPack;	/* Whether to auto-pack on commits. */
  
  char	*ScanFormatFile; /* File containing scan format.  */
  
  char	*ScanCurrentString; /* String to use to indicate message is
			       current. */
  char 	*ScanMovedString; /* String to use to indicate message is
			     moved. */
  char	*ScanDeletedString; /* String to use to indicate message
			       is deleted. */
  
  char	*NormalCursorName; /* Name of cursor to show for normal use. */
  char	*SleepCursorName; /* Name of cursor to show during long ops. */
  
  Boolean	defDefaultViewBackwards; /* Whether to view messages in
					    reverse order by default. */
  Boolean	defUseWastebasket; /* Whether to put deleted messages into a */
  /* wastebasket. */
  int		defWastebasketDaysToKeep; /* How many days to keep old msgs
					     in the wastebasket.  If zero,
					     keep them forever. */
  Boolean	defIncOnShowUnopened; /* Whether to do an "inc" whenever
					 the user hits "show unopened". */
  Boolean	defAffectCurIfNullSelection;
  /* Whether toc buttons should affect the
     current message when there's no selection.*/
  Boolean defBeepIfNoMail;	/* Beep if no mail instead of using a
				   dialog box. */
  Boolean defGrabFocus;	/* Grab focus on new windows. */
  Boolean defAnnotateReplies;	/* Whether to annotate replied-to msgs. */
  Boolean defAnnotateForwards; /* Whether to annotate forwarded msgs. */
  Boolean defDoReadPopups;	/* Whether to make read window popups. */
  
   Radio	ReadWindowsRadio; /* Radio box listing available read
				     windows. */
  
   Msg		LastPopupMsg;	/* Last message selected in a pop-up. */
   Toc		LastPopupToc;	/* Last folder selected in a pop-up. */
  
   XmString EmptyCompoundString; /* A compound string containing "". */
  
  int		defReplyCCAll;	/* yes/no/default:  Whether to cc replies to */
  /* eveyone. */
  int		defReplyCCMe;	/* yes/no/default: Whether to cc replies to */
  /* ourselves. */
  Boolean	defSVNStyle;	/* True if SVN layout */
  
  int		newSVNStyle;	/* Current customize value */
  /* MH command strings */
  char *annoCmd;		/* gets filled in using defMhPath */
  char *compCmd;		/* so that 'mh' no longer needs to be */
  char *folderCmd;		/* in the user's path */
  char *forwCmd;
  char *incCmd;
  char *markCmd;

  char *pickCmd;
  char *refileCmd;
  char *replCmd;
  char *rmfCmd;
  char *rmmCmd;
  char *scanCmd;
  char *sendCmd;
  char *sortmCmd;
} resource_list;
	
char *defChoiceEditor;
	
	
	
	
	
	
	
	
	
	
	
	
	
