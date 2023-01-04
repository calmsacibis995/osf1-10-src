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
/*   $RCSfile: WmCDecor.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:09:58 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern Boolean FrameWindow ();
extern void FrameExposureProc ();
extern void BaseWinExposureProc ();
extern void SetFrameShape ();
extern Boolean ConstructFrame ();
extern void GenerateFrameDisplayLists ();
extern void AdoptClient ();
extern void GetTextBox ();
extern void DrawWindowTitle ();
extern void CreateStretcherWindows ();
extern void CountFrameRectangles ();
extern Boolean AllocateFrameDisplayLists ();
extern void InitClientDecoration ();
extern Boolean AllocateGadgetRectangles ();
extern void ComputeGadgetRectangles ();
extern void GetSystemMenuPosition ();
extern void ShowActiveClientFrame ();
extern void ShowInactiveClientFrame ();
extern void RegenerateClientFrame ();
extern void BevelSystemButton ();
extern void BevelMinimizeButton ();
extern void BevelMaximizeButton ();
extern Boolean DepressGadget ();
extern void PushGadgetIn ();
extern void PopGadgetOut ();
#else /* _NO_PROTO */
extern Boolean FrameWindow (ClientData *pcd);
extern void FrameExposureProc (ClientData *pcd);
extern void BaseWinExposureProc (ClientData *pcd);
extern void SetFrameShape (ClientData *pcd);
extern Boolean ConstructFrame (ClientData *pcd);
extern void GenerateFrameDisplayLists (ClientData *pcd);
extern void AdoptClient (ClientData *pcd);
extern void GetTextBox (ClientData *pcd, XRectangle *pBox);
extern void DrawWindowTitle (ClientData *pcd, Boolean eraseFirst);
extern void CreateStretcherWindows (ClientData *pcd);
extern void CountFrameRectangles (WmScreenData *pSD);
extern Boolean AllocateFrameDisplayLists (ClientData *pcd);
extern void InitClientDecoration (WmScreenData *pSD);
extern Boolean AllocateGadgetRectangles (ClientData *pcd);
extern void ComputeGadgetRectangles (ClientData *pcd);
extern void GetSystemMenuPosition (ClientData *pcd, int *px, int *py, unsigned int height, Context context);
extern void ShowActiveClientFrame (ClientData *pcd);
extern void ShowInactiveClientFrame (ClientData *pcd);
extern void RegenerateClientFrame (ClientData *pcd);
extern void BevelSystemButton (RList *prTop, RList *prBot, int x, int y, unsigned int width, unsigned int height);
extern void BevelMinimizeButton (RList *prTop, RList *prBot, int x, int y, unsigned int height);
extern void BevelMaximizeButton (RList *prTop, RList *prBot, int x, int y, unsigned int height);
extern Boolean DepressGadget (ClientData *pcd, int gadget, Boolean depressed);
extern void PushGadgetIn (ClientData *pcd, int gadget);
extern void PopGadgetOut (ClientData *pcd, int gadget);
#endif /* _NO_PROTO */
