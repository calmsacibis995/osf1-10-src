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
 * Motif Release 1.2.1
*/ 
/*   $RCSfile: WmWinState.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:18:12 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void NormalTransientTransition();
extern void SetClientState ();
extern void SetClientStateWithEventMask ();
extern void ConfigureNewState ();
extern void SetClientWMState ();
extern void MapClientWindows ();
extern void ShowIconForMinimizedClient ();
extern void ShowAllIconsForMinimizedClient ();
#else /* _NO_PROTO */
extern void SetClientState (ClientData *pCD, int newState, Time setTime);
extern void SetClientStateWithEventMask (ClientData *pCD, int newState, Time setTime, unsigned int event_mask);
extern void ConfigureNewState (ClientData *pcd);
extern void SetClientWMState (ClientData *pCD, int wmState, int mwmState);
extern void MapClientWindows (ClientData *pCD);
extern void ShowIconForMinimizedClient (WmWorkspaceData *pWS, ClientData *pCD);
extern void ShowAllIconsForMinimizedClient (ClientData *pCD);
#endif /* _NO_PROTO */
