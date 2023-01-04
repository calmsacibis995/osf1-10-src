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
	@(#)$RCSfile: toc.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/03 00:01:38 $"
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

#ifndef _toc_h
#define _toc_h

extern void TocInit();
extern Toc TocCreateFolder();
extern Toc TocShowFolder();
extern Toc TocIncludeFolderName();
extern void TocHideFolder();
extern void TocDeleteFolder();
extern void TocCheckForNewMail();
extern void TocSetScrn();
extern void TocRemoveMsg();
extern void TocRecheckValidity();
extern void TocMakeVisible();
extern void TocSetCurMsg();
extern Msg TocGetCurMsg();
extern Msg TocGetFirstMsg();
extern Msg TocGetLastMsg();
extern Msg TocMsgAfter();
extern Msg TocMsgBefore();
extern void TocForceRescan();
extern void TocCheckSeqButtons();
extern void TocReloadSeqLists();
extern int TocHasSequences();
extern void TocChangeViewedSeq();
extern Sequence TocViewedSequence();
extern Sequence TocGetSeqNamed();
extern Boolean TocCreateNullSequence();
extern Boolean TocDeleteNullSequence();
extern MsgList TocCurMsgList();
extern Boolean TocHasSelection();
extern void TocUnsetSelection();
extern Msg TocMakeNewMsg();
extern void TocStopUpdate();
extern void TocStartUpdate();
extern void TocSetCacheValid();
extern char *TocGetFolderName();
extern Toc TocGetNamed();
extern int TocConfirmCataclysm();
extern void TocCommitChanges();
extern void TocCopyMessages();
extern int TocCanIncorporate();
extern Msg TocIncorporate();
extern void TocMsgChanged();
extern Msg TocMsgFromId();
extern void TocSaveCurMsg();
extern void TocEmptyWastebasket();
extern Boolean TocNeedsPacking();
extern void TocPack();
extern int TocSubFolderCount();

/* Functions defined in tocout.c. */

extern void TocDisableRedisplay();
extern void TocEnableRedisplay();
#endif _toc_h
