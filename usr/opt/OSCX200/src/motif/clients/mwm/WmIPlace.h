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
/*   $RCSfile: WmIPlace.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 21:43:25 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void InitIconPlacement ();
extern int GetNextIconPlace ();
extern void CvtIconPlaceToPosition ();
extern int FindIconPlace ();
extern int CvtIconPositionToPlace ();
extern void PackRootIcons ();
extern void MoveIconInfo ();
#else /* _NO_PROTO */
extern void InitIconPlacement (WmWorkspaceData *pWS);
extern int GetNextIconPlace (IconPlacementData *pIPD);
extern void CvtIconPlaceToPosition (IconPlacementData *pIPD, int place, int *pX, int *pY);
extern int FindIconPlace (ClientData *pCD, IconPlacementData *pIPD, int x, int y);
extern int CvtIconPositionToPlace (IconPlacementData *pIPD, int x, int y);
extern void PackRootIcons (void);
extern void MoveIconInfo (IconPlacementData *pIPD, int p1, int p2);
#endif /* _NO_PROTO */
