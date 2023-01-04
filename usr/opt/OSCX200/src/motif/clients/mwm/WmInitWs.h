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
/*   $RCSfile: WmInitWs.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:16:30 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void InitWmGlobal ();
#ifdef DEC_MOTIF_EXTENSION
extern void WmInitResUserGet();
extern void WmInitResSysGet();
#endif
extern void InitWmScreen ();
extern void InitWmWorkspace ();
extern void ProcessMotifWmInfo ();
extern void SetupWmWorkspaceWindows ();
extern void MakeWorkspaceCursors ();
extern void MakeWmFunctionResources ();
extern void MakeXorGC ();
extern void CopyArgv ();
extern void InitScreenNames ();
#else /* _NO_PROTO */
extern void InitWmGlobal (int argc, char *argv [], char *environ []);
#ifdef DEC_MOTIF_EXTENSION
extern void WmInitResSysGet( String name, String *filename );
extern void WmInitResUserGet( String name, String *filename );
#endif
extern void InitWmScreen (WmScreenData *pSD, int sNum);
extern void InitWmWorkspace (WmWorkspaceData *pWS, WmScreenData *pSD);
extern void ProcessMotifWmInfo (Window rootWindowOfScreen);
extern void SetupWmWorkspaceWindows (void);
extern void MakeWorkspaceCursors (void);
extern void MakeWmFunctionResources (WmScreenData *pSD);
extern void MakeXorGC (WmScreenData *pSD);
extern void CopyArgv (int argc, char *argv []);
extern void InitScreenNames (void);
#endif /* _NO_PROTO */
