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
/*   $RCSfile: WmIDecor.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 21:31:47 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void CreateActiveIconTextWindow ();
extern void DrawIconTitle ();
extern void GetIconDimensions ();
extern void GetIconTitleBox ();
extern void HideActiveIconText ();
extern void IconExposureProc ();
extern void InitIconSize ();
extern Boolean MakeIcon ();
extern void MakeIconShadows ();
extern void MoveActiveIconText ();
extern void PaintActiveIconText ();
extern void PutBoxInIconBox ();
extern void PutBoxOnScreen ();
extern void RedisplayIconTitle ();
extern void ReparentIconWindow ();
extern void ShowActiveIcon ();
extern void ShowActiveIconText ();
extern void ShowInactiveIcon ();

#else /* _NO_PROTO */

extern void CreateActiveIconTextWindow (WmScreenData *pSD);
extern void DrawIconTitle (ClientData *pcd);
extern void GetIconDimensions (WmScreenData *pSD, unsigned int *pWidth, 
			       unsigned int *pLabelHeight, 
			       unsigned int *pImageHeight);
extern void GetIconTitleBox (ClientData *pcd, XRectangle *pBox);
extern void HideActiveIconText (WmScreenData *pSD);
extern void IconExposureProc (ClientData *pcd, Boolean clearFirst);
extern void InitIconSize (WmScreenData *pSD);
extern Boolean MakeIcon (WmWorkspaceData *pWS, ClientData *pcd);
extern void MakeIconShadows (ClientData *pcd, int xOffset, int yOffset);
extern void MoveActiveIconText (ClientData *pcd);
extern void PaintActiveIconText (ClientData *pcd, Boolean erase);
extern void PutBoxInIconBox (ClientData *pCD, int *px, int *py, 
			     unsigned int *width, unsigned int *height);
extern void PutBoxOnScreen (int screen, int *px, int *py, unsigned int width, 
			    unsigned int height);
extern void RedisplayIconTitle (ClientData *pcd);
extern void ReparentIconWindow (ClientData *pcd, int xOffset, int yOffset);
extern void ShowActiveIcon (ClientData *pcd);
extern void ShowActiveIconText (ClientData *pcd);
extern void ShowInactiveIcon (ClientData *pcd, Boolean refresh);

#endif /* _NO_PROTO */

