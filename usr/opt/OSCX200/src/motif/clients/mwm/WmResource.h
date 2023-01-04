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
/*   $RCSfile: WmResource.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:29:30 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void GetAppearanceGCs ();
extern GC   GetHighlightGC ();
extern void MakeAppearanceResources ();
extern void ProcessClientResources ();
extern void ProcessWmResources ();
extern void ProcessScreenListResource ();
extern void ProcessAppearanceResources ();
void ProcessGlobalScreenResources ();
extern void ProcessScreenResources ();
extern void ProcessWmColors ();
extern void ProcessWorkspaceList ();
extern void ProcessWorkspaceResources ();
extern void SetStdClientResourceValues ();
extern void SetStdGlobalResourceValues ();
extern void SetStdScreenResourceValues ();
extern char *WmRealloc ();
extern char *WmMalloc ();
extern void SetupDefaultResources ();
extern Boolean SimilarAppearanceData ();
#else /* _NO_PROTO */
extern void GetAppearanceGCs (WmScreenData *pSD, Pixel fg, Pixel bg, XFontStruct *font, Pixmap bg_pixmap, Pixel ts_color, Pixmap ts_pixmap, Pixel bs_color, Pixmap bs_pixmap, GC *pGC, GC *ptsGC, GC *pbsGC);
extern GC   GetHighlightGC (WmScreenData *pSD, Pixel fg, Pixel bg, Pixmap pixmap);
extern void MakeAppearanceResources (WmScreenData *pSD, AppearanceData *pAData, Boolean makeActiveResources);
extern void ProcessWmColors (WmScreenData *pSD);
extern void ProcessWmResources (void);
extern void SetStdGlobalResourceValues (void);
extern void ProcessScreenListResource (void);
extern void ProcessAppearanceResources (WmScreenData *pSD);
void ProcessGlobalScreenResources (void);
extern void ProcessScreenResources (WmScreenData *pSD, unsigned char *screenName);
extern void ProcessWorkspaceList (WmScreenData *pSD);
extern void ProcessWorkspaceResources (WmWorkspaceData *pWS);
extern void ProcessClientResources (ClientData *pCD);
extern void SetStdClientResourceValues (ClientData *pCD);
extern void SetStdScreenResourceValues (WmScreenData *pSD);
extern char *WmRealloc (char *ptr, unsigned size);
extern char *WmMalloc (char *ptr, unsigned size);
extern void SetupDefaultResources (WmScreenData *pSD);
extern Boolean SimilarAppearanceData (AppearanceData *pAD1, AppearanceData *pAD2);
#endif /* _NO_PROTO */

extern char builtinSystemMenu[];
extern char builtinKeyBindings[];
extern char builtinRootMenu[];
extern char builtinSystemMenuName[];

#define Monochrome(screen) ( DefaultDepthOfScreen(screen) == 1 )
