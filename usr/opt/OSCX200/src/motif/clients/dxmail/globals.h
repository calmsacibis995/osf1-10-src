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
	@(#)$RCSfile: globals.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 23:57:23 $"
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


#ifdef MAIN
#define ext
#else
#define ext extern
#endif

ext Boolean	debug;
ext Boolean	trace;
ext Boolean	bgsend;		/* PJS: Backgrounded send, for speed. */

ext Display	*theDisplay;	/* Display variable. */
ext int		theScreen;	/* Which screen we're using. */
ext Widget	toplevel;	/* The top level widget (A hack %%%). */
ext XrmDatabase	DefaultDB;	/* Database for decxmail data. */

ext char	*progName;	/* Program name. */
ext char	*homeDir;	/* User's home directory. */
ext char	*mailDir;	/* mh's mail directory. */
ext char	*tempDir;	/* Directory to use for temporary files. */
ext char	*initialFolderName; /* Initial folder to use. */
ext char	*draftsFolderName;	/* Folder for drafts. */
ext char	*wastebasketFolderName;	/* Folder for wastebasket. */
ext char        *popHostName;   /* Hostname for POP incorporation. */
ext char	*draftFile;		/* Filename of draft. */
ext char	*draftScratchFile;	/* Filename for sending. */
ext char	*profileFileName; /* Full filename of .mh_profile. */
ext Boolean	hasRmmProc;	/* True if the user has a rmmproc. */

ext char	*editcommand;		/* Command to exec to composing. */
ext char	*defMhPath;		/* Path for mh commands. */
ext int		defSendLineWidth;	/* How long to break lines on send. */
ext int		defBreakSendLineWidth;	/* Minimum length of a line before
					   we'll break it. */
ext char	*defPrintCommand; /* Printing command. */
ext char	*defPrintStrippedCommand;	/* Stripped print command */

ext int		defTocWidth;	/* How many characters wide to use in tocs */

ext Boolean	SkipDeleted;		/* If true, skip over deleted msgs. */
ext Boolean	SkipMoved;		/* If true, skip over moved msgs. */

ext Boolean	defHideBoringHeaders;

ext Boolean	defNewMailCheck; /* Whether to check for new mail. */
ext Boolean	defMakeCheckpoints; /* Whether to create checkpoint files. */

ext int		defNewMailBorderWidth; /* Amount of border to add. */

ext Boolean	defOpenSubfolders;    /* Open subfolders when opening drawer */

ext Boolean	defCloseSubfolders;   /* Close subfolders when opening folder */

ext int		defInitialState;	/* Initial state */

ext Toc		*folderList;	/* Array of folders. */
ext int		numFolders;	/* Number of entries in above array. */
ext Toc		InitialFolder;	/* Toc containing initial folder. */
ext Toc		DraftsFolder;	/* Toc containing drafts. */
ext Toc		WastebasketFolder; /* Toc for wastebasket. */

ext char	*unseenSeqName;	/* Sequence for unseen messages. */

ext Scrn	*scrnList;	/* Array of scrns in use. */
ext int		numScrns;	/* Number of scrns in above array. */

ext XmTextSource NullSource;
ext XmTextSource NullTocSource;

ext Dimension	rootwidth;	/* Dimensions of root window.  */
ext Dimension	rootheight;

ext Pixmap	NoMailPixmap;	/* Icon pixmap if no new mail. */
ext Pixmap	NewMailPixmap;	/* Icon pixmap if new mail. */
ext Pixmap	NoMailSmallPixmap; /* Iconify button pixmap if no new mail. */
ext Pixmap	NewMailSmallPixmap; /* Iconify button pixmap if new mail. */
ext Pixmap	GrayPixmap;	/* Gray pixmap. */

ext Widget	PopupWidget;	/* Widget last popup menu was created in. */

ext char	customfile[256]; /* Filename for customization file. */

ext Boolean 	AutoCommit;	/* Whether to automatically commit changes. */
ext Boolean 	AutoPack;	/* Whether to auto-pack on commits. */

ext char	*ScanFormatFile; /* File containing scan format.  */
ext IntRange	ScanIDCols;	/* Which columns in scan contain msg id. */
ext IntRange	ScanCurrentCols; /* Which columns in scan indicate if this
				    is the current msg. */
ext IntRange	ScanMoveCols;	/* Which columns in scan indicate this
				   message is marked for moving. */
ext IntRange	ScanDeleteCols;	/* Which columns in scan indicate this
				   message is marked for deleting. */
ext char	*ScanCurrentString; /* String to use to indicate message is
				       current. */
ext char 	*ScanMovedString; /* String to use to indicate message is
				     moved. */
ext char	*ScanDeletedString; /* String to use to indicate message
				       is deleted. */

ext char	*NormalCursorName; /* Name of cursor to show for normal use. */
ext char	*SleepCursorName; /* Name of cursor to show during long ops. */

ext Boolean	defDefaultViewBackwards; /* Whether to view messages in
					  reverse order by default. */
ext Boolean	defUseWastebasket; /* Whether to put deleted messages into a */
				   /* wastebasket. */
ext int		defWastebasketDaysToKeep; /* How many days to keep old msgs
					     in the wastebasket.  If zero,
					     keep them forever. */
ext Boolean	defIncOnShowUnopened; /* Whether to do an "inc" whenever
				         the user hits "show unopened". */
ext Boolean	defAffectCurIfNullSelection;
				/* Whether toc buttons should affect the
				 current message when there's no selection.*/
ext Boolean defBeepIfNoMail;	/* Beep if no mail instead of using a
				   dialog box. */
ext Boolean defGrabFocus;	/* Grab focus on new windows. */
ext Boolean defAnnotateReplies;	/* Whether to annotate replied-to msgs. */
ext Boolean defAnnotateForwards; /* Whether to annotate forwarded msgs. */
ext Boolean defDoReadPopups;	/* Whether to make read window popups. */

ext Radio	ReadWindowsRadio; /* Radio box listing available read
				     windows. */

ext Msg		LastPopupMsg;	/* Last message selected in a pop-up. */
ext Toc		LastPopupToc;	/* Last folder selected in a pop-up. */

ext XmString EmptyCompoundString; /* A compound string containing "". */

ext int		defReplyCCAll;	/* yes/no/default:  Whether to cc replies to */
				/* eveyone. */
ext int		defReplyCCMe;	/* yes/no/default: Whether to cc replies to */
				/* ourselves. */
ext Boolean	defSVNStyle;	/* True if SVN layout */

ext int		newSVNStyle;	/* Current customize value */
/* MH command strings */
ext char *annoCmd;		/* gets filled in using defMhPath */
ext char *compCmd;		/* so that 'mh' no longer needs to be */
ext char *folderCmd;		/* in the user's path */
ext char *forwCmd;
ext char *incCmd;
ext char *markCmd;
ext char *pickCmd;
ext char *refileCmd;
ext char *replCmd;
ext char *rmfCmd;
ext char *rmmCmd;
ext char *scanCmd;
ext char *sendCmd;
ext char *sortmCmd;

/* choice editor stuff */

ext char *defChoiceEditor;
ext Boolean dps_exists;


