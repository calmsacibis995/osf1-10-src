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
/*   $RCSfile: WmImage.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:07:52 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern char  *BitmapPathName ();
extern Pixmap MakeCachedIconPixmap ();
extern Pixmap MakeCachedLabelPixmap ();
extern Pixmap MakeClientIconPixmap ();
extern int GetBitmapIndex ();
extern Pixmap MakeIconPixmap ();
extern Pixmap MakeNamedIconPixmap ();
#else /* _NO_PROTO */
extern char  *BitmapPathName (char *string);
extern int    GetBitmapIndex (WmScreenData *pSD, char *name);
extern Pixmap MakeCachedIconPixmap (ClientData *pCD, int bitmapIndex, Pixmap mask);
extern Pixmap MakeCachedLabelPixmap (WmScreenData *pSD, Widget menuW, int bitmapIndex);
extern Pixmap MakeClientIconPixmap (ClientData *pCD, Pixmap iconBitmap);
extern Pixmap MakeIconPixmap (ClientData *pCD, Pixmap bitmap, Pixmap mask, unsigned int width, unsigned int height, unsigned int depth);
extern Pixmap MakeNamedIconPixmap (ClientData *pCD, String iconName);
#endif /* _NO_PROTO */
