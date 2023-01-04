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
/*   $RCSfile: WmCDInfo.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:01:13 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern int FrameX ();
extern int FrameY ();
extern unsigned int FrameWidth ();
extern unsigned int FrameHeight ();
extern unsigned int TitleTextHeight ();
extern unsigned int UpperBorderWidth ();
extern unsigned int LowerBorderWidth ();
extern unsigned int CornerWidth ();
extern unsigned int CornerHeight ();
extern int BaseWindowX ();
extern int BaseWindowY ();
extern unsigned int BaseWindowWidth ();
extern unsigned int BaseWindowHeight ();
extern Boolean GetFramePartInfo ();
extern int IdentifyFramePart ();
extern int GadgetID ();
extern void FrameToClient ();
extern void ClientToFrame ();
extern Boolean GetDepressInfo ();
extern void SetFrameInfo ();
extern void SetClientOffset ();
extern Boolean XBorderIsShowing ();
#else /* _NO_PROTO */
extern int FrameX (ClientData *pcd);
extern int FrameY (ClientData *pcd);
extern unsigned int FrameWidth (ClientData *pcd);
extern unsigned int FrameHeight (ClientData *pcd);
extern unsigned int TitleTextHeight (ClientData *pcd);
extern unsigned int UpperBorderWidth (ClientData *pcd);
extern unsigned int LowerBorderWidth (ClientData *pcd);
extern unsigned int CornerWidth (ClientData *pcd);
extern unsigned int CornerHeight (ClientData *pcd);
extern int BaseWindowX (ClientData *pcd);
extern int BaseWindowY (ClientData *pcd);
extern unsigned int BaseWindowWidth (ClientData *pcd);
extern unsigned int BaseWindowHeight (ClientData *pcd);
extern Boolean GetFramePartInfo (ClientData *pcd, int part, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight);
extern int IdentifyFramePart (ClientData *pCD, int x, int y);
extern int GadgetID (int x, int y, GadgetRectangle *pgadget, unsigned int count);
extern void FrameToClient (ClientData *pcd, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight);
extern void ClientToFrame (ClientData *pcd, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight);
extern Boolean GetDepressInfo (ClientData *pcd, int part, int *pX, int *pY, unsigned int *pWidth, unsigned int *pHeight, unsigned int *pInvertWidth);
extern void SetFrameInfo (ClientData *pcd);
extern void SetClientOffset (ClientData *pcd);
extern Boolean XBorderIsShowing (ClientData *pcd);
#endif /* _NO_PROTO */

/*
 * TitleBarHeight() is now a simple macro instead of a procedure.
 */

#define TitleBarHeight(pcd) ((pcd)->frameInfo.titleBarHeight)
