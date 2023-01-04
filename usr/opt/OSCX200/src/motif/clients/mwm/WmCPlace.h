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
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.2
*/ 
/*   $RCSfile: WmCPlace.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:24:58 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
#ifdef DEC_MOTIF_BUG_FIX
extern Boolean PlaceWindowInteractively ();
#else
extern void PlaceWindowInteractively ();
#endif
extern void DoPlacement ();
extern void SetupPlacement ();
extern void HandlePlacementKeyEvent ();
extern void HandlePlacementButtonEvent ();
extern void HandlePlacementMotionEvent ();
extern void StartInteractiveSizing ();
extern Bool IsRepeatedKeyEvent ();
#else /* _NO_PROTO */
#ifdef DEC_MOTIF_BUG_FIX
extern Boolean PlaceWindowInteractively (ClientData *pcd);
#else
extern void PlaceWindowInteractively (ClientData *pcd);
#endif
extern void DoPlacement (ClientData *pcd);
extern void SetupPlacement (ClientData *pcd);
extern void HandlePlacementKeyEvent (ClientData *pcd, XKeyEvent *pev);
extern void HandlePlacementButtonEvent (XButtonEvent *pev);
extern void HandlePlacementMotionEvent (ClientData *pcd, XMotionEvent *pev);
extern void StartInteractiveSizing (ClientData *pcd, Time time);
extern Bool IsRepeatedKeyEvent (Display *dpy, XEvent *pEvent, char *pOldEvent);
#endif /* _NO_PROTO */
