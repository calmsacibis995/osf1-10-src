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
	@(#)$RCSfile: msg.h,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/08/02 23:59:14 $
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


#ifndef _msg_h
#define _msg_h

extern char *MsgGetName();
extern char *MsgFileName();
extern MsgType MsgGetMsgType(); 
extern void MsgCreateSource();
extern void MsgDestroySource();
extern void MsgSaveChanges();
extern int MsgSetScrn();
extern int MsgSetScrnForComp();
extern void MsgSetScrnForce();
extern void MsgRepaintLabels();
extern void MsgSetFate();
extern void MsgBatchSetFate();
extern FateType MsgGetFate();
extern void MsgSetTemporary();
extern Boolean MsgGetTemporary();
extern void MsgSetPermanent();
extern int MsgGetId();
extern char *MsgGetScanLine();
extern Toc MsgGetToc();
extern void MsgSetReapable();
extern void MsgClearReapable();
extern Boolean MsgGetReapable();
extern void MsgSetEditable();
extern void MsgClearEditable();
extern Boolean MsgGetEditable();
extern Boolean MsgChanged();
extern void MsgSetCallOnChange();
extern void MsgClearCallOnChange();
extern void MsgSend();
extern void MsgLoadComposition();
extern void MsgLoadReply();
extern void MsgLoadForward();
extern void MsgLoadCopy();
extern Boolean MsgLoadFromFile();
extern void MsgCheckPoint();
extern Msg MsgMalloc();
extern void MsgFree();
extern void MsgRemoveFromSeq();
extern XmTextSource MsgGetSource();
extern MsgHandle MsgGetHandle();
extern void MsgFreeHandle();
extern Msg MsgFromHandle();
extern char *MsgGetDDIFFile();
extern void InitMsg();

#endif _msg_h
