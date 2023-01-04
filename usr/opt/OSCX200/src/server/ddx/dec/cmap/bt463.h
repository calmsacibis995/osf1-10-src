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
#ifndef _BT463_H_
#define _BT463_H_

extern void bt463LoadColormap(ColormapPtr pMap, Bool *doWriteWindowTypeTable);
extern long bt463CmapTypeToIndex(long cmapType);
extern long bt463OverlayVisualIndex(ScreenPtr pScreen);
extern Bool bt463IsOverlayWindow(WindowPtr pWin);
extern long bt463FFBWindowID(WindowPtr pWin);
extern void bt463FFBWriteWindowTypeTable(ScreenPtr pScreen, ColormapPtr pMap, long where);

#endif
/*
 * HISTORY
 */
