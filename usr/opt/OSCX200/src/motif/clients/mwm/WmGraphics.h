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
/*   $RCSfile: WmGraphics.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:23:13 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern RList *AllocateRList ();
extern void BevelDepressedRectangle ();
extern void BevelRectangle ();
extern void DrawStringInBox ();
extern Boolean ExtendRList ();
extern void FreeRList ();
extern void StretcherCorner ();
extern void WmDrawString ();
extern void WmDrawXmString ();
extern GC WmGetGC ();

#else /* _NO_PROTO */

extern RList *AllocateRList (unsigned int amt);
extern void BevelDepressedRectangle (RList *prTop, RList *prBot, int x, 
				     int y, unsigned int width, 
				     unsigned int height, unsigned int top_wid,
 				     unsigned int right_wid, 
				     unsigned int bot_wid, 
				     unsigned int left_wid, 
				     unsigned int in_wid);
extern void BevelRectangle (RList *prTop, RList *prBot, int x, int y, 
			    unsigned int width, unsigned int height, 
			    unsigned int top_wid, unsigned int right_wid, 
			    unsigned int bot_wid, unsigned int left_wid);
extern void DrawStringInBox (Display *dpy, Window win, GC gc, 
			     XFontStruct *pfs, XRectangle *pbox, String str);
extern Boolean ExtendRList (RList *prl, unsigned int amt);
extern void FreeRList (RList *prl);
extern void StretcherCorner (RList *prTop, RList *prBot, int x, int y, 
			     int cnum, unsigned int swidth, 
			     unsigned int cwidth, unsigned int cheight);
extern void WmDrawString (Display *dpy, Drawable d, GC gc, int x, int y, 
			  char *string, unsigned int length);
extern void WmDrawXmString (Display *dpy, Window w, XmFontList xmfontlist, 
			    XmString xmstring, GC gc, Position x, Position y, 
			    Dimension width, XRectangle *pbox);

extern GC WmGetGC (WmScreenData *pSD, unsigned long gc_mask, XGCValues *pGcv);

#endif /* _NO_PROTO */

