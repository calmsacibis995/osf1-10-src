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
 * $XConsortium: OSio.h,v 1.2 91/07/16 12:56:49 jap Exp $
 *
 * Copyright IBM Corporation 1987,1988,1989,1990,1991
 *
 * All Rights Reserved
 *
 * License to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of IBM not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * IBM DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS, AND 
 * NONINFRINGEMENT OF THIRD PARTY RIGHTS, IN NO EVENT SHALL
 * IBM BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 *
*/

#ifndef OSIO_H
#define OSIO_H

#define	CURRENT_X()	(AIXCurrentX)
#define	CURRENT_Y()	(AIXCurrentY)

extern	int	AIXDefaultDisplay;
extern	int	AIXCurrentX;
extern	int	AIXCurrentY;
extern	void	AIXBlockHandler();
extern	void	AIXWakeupHandler();
extern	void	AIXInitEventHandlers();
extern	int	AIXProcessArgument();
extern	void	hftTermQueue();
extern	void	NoopDDA() ;

#define	setCursorPosition(x,y)	{ AIXCurrentX= (x); AIXCurrentY= (y); }

#define	OS_BlockHandler			AIXBlockHandler
#define	OS_WakeupHandler		AIXWakeupHandler
#define	OS_MouseProc			AIXMouseProc
#define	OS_KeybdProc			AIXKeybdProc

#define OS_CapsLockFeedback(dir)	NoopDDA()

#define	OS_PreScreenInit()		AIXMachineDependentInit()
#define	OS_PostScreenInit()		NoopDDA()
#define	OS_ScreenStateChange(e)		NoopDDA()

#define OS_AddAndRegisterOtherDevices()	NoopDDA()
#define	OS_GetDefaultScreens()		AIXGetDefaultScreens()

#define	OS_InitInput()			AIXInitEventHandlers()
#define	OS_SaveState()			NoopDDA()
#define	OS_RestoreState()		NoopDDA()

#define	OS_GiveUp()			hftTermQueue()
#define	OS_Abort()			hftTermQueue()

#define	OS_ProcessArgument		AIXProcessArgument

#endif /* OSIO_H */
