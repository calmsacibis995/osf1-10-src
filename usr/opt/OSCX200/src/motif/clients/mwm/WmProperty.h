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
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: WmProperty.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 21:52:00 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern SizeHints * GetNormalHints ();
extern void ProcessWmProtocols ();
extern void ProcessMwmMessages ();
extern void SetMwmInfo ();
extern PropWMState * GetWMState ();
extern void SetWMState ();
extern PropMwmHints * GetMwmHints ();
extern PropMwmInfo * GetMwmInfo ();
extern void ProcessWmColormapWindows ();
extern Colormap FindColormap ();
extern MenuItem * GetMwmMenuItems ();
extern Status GetWmCommand ();
#else /* _NO_PROTO */
extern SizeHints * GetNormalHints (ClientData *pCD);
extern void ProcessWmProtocols (ClientData *pCD);
extern void ProcessMwmMessages (ClientData *pCD);
extern void SetMwmInfo (Window propWindow, long flags, Window wmWindow);
extern PropWMState * GetWMState (Window window);
extern void SetWMState (Window window, int state, Window icon);
extern PropMwmHints * GetMwmHints (ClientData *pCD);
extern PropMwmInfo * GetMwmInfo (Window rootWindowOfScreen);
extern void ProcessWmColormapWindows (ClientData *pCD);
extern Colormap FindColormap (ClientData *pCD, Window window);
extern MenuItem * GetMwmMenuItems (ClientData *pCD);
extern Status GetWmCommand (Display *display, Window window, int *pArgc, char ***pArgv);
#endif /* _NO_PROTO */
