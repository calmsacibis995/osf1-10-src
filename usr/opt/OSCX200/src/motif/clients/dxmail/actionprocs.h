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
	@(#)$RCSfile: actionprocs.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 23:55:15 $
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


/* util.c */

extern void ExecOnDouble();

/* folder.c */

extern void ExecQuit();
extern void ExecCloseScrn();
extern void ExecOpenFolder();
extern void ExecCloseFolder();
extern void ExecOpenFolderInNewWindow();
extern void ExecCreateFolder();
extern void ExecShowFolder();
extern void ExecHideFolder();
extern void ExecDeleteFolder();
extern void ExecDumpScrnWidgetHierarchy();
extern void ExecHideSubfolders();
extern void ExecShowSubfolders();

/* popup.c */

extern void ExecPopupMenu();
extern void PopupSvnMenu();

/* viewfuncs.c */

extern void ExecViewBefore();
extern void ExecViewAfter();
extern void ExecNextSelected();
extern void ExecPrevSelected();
extern void ExecThisReply();
extern void ExecThisForward();
extern void ExecThisInDefault();
extern void ExecThisInNew();
extern void ExecThisUseAsComposition();
extern void ExecEditView();
extern void ExecSaveView();
extern void ExecThisPrintSelective();
extern void ExecThisPrintStripped();
extern void ExecThisPrint();
extern void ExecThisMove();
extern void ExecThisCopy();
extern void ExecThisMoveDialog();
extern void ExecThisCopyDialog();
extern void ExecThisDelete();
extern void ExecThisUnmark();
extern void ExecNewMailInView();
extern void ExecMsgAddToSeq();
extern void ExecMsgRemoveFromSeq();
extern void ExecMakeDefaultView();
extern void ExecCut();
extern void ExecCopy();
extern void ExecPaste();

/* compfuncs.c */

extern void ExecCompReset();
extern void ExecComposeMessage();
extern void ExecSaveDraft();
extern void ExecSendDraft();
extern void ExecComposeUsingFile();

/* tocfuncs.c */

extern void ExecIncorporate();
extern void ExecReadNewMail();
extern void ExecReadNewMailInNew();
extern void ExecShowUnseen();
extern void ExecNextView();
extern void ExecPrevView();
extern void ExecMarkDelete();
extern void ExecMarkMove();
extern void ExecMarkCopy();
extern void ExecMarkMoveDialog();
extern void ExecMarkCopyDialog();
extern void ExecMarkUnmarked();
extern void ExecViewNew();
extern void ExecViewInSpecified();
extern void ExecViewInDefault();
extern void ExecInsertFile();
extern void ExecExtractMsg();
extern void ExecExtractMsgStrip();
extern void ExecTocReply();
extern void ExecTocForward();
extern void ExecTocUseAsComposition();
extern void ExecCommitChanges();
extern void ExecEmptyWastebasket();
extern void ExecOpenSeq();
extern void ExecAddToSeq();
extern void ExecRemoveFromSeq();
extern void ExecCreateSeq();
extern void ExecPick();
extern void ExecPickInSelected();
extern void ExecDeleteSeq();
extern void ExecPrintMessages();
extern void ExecPrintStripped();
extern void ExecPrintWidget();
extern void ExecPrintWidgetStripped();
extern void ExecPack();
extern void ExecSort();
extern void ExecForceRescan();
extern void ExecSelectAll();
extern void ExecRenameFolder();
/* tocout.c */

extern void ExecSelectThisMsg();
extern void ExecExtendThisMsg();
extern void ExecExtendThisToc();

/* screen.c */

extern void ExecViewInDDIFViewer();

/* menus.c */

extern void ExecPopupHelpMenu();
extern void ExecCreateHelpMenu();
extern void ExecOnContext();


/* toc.c */

extern void TocResetAll();

/* msg.c */

extern void ExecCustomEditor();

 
 
