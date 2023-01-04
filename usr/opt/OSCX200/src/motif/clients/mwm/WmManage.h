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
/*   $RCSfile: WmManage.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 21:26:19 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void AdoptInitialClients ();
#ifdef DEC_MOTIF_EXTENSION
extern void WmManageModReset();
#endif /* DEC_MOTIF_EXTENSION */
#ifdef DEC_MOTIF_BUG_FIX
extern Boolean ManageWindow ();
#else
extern void ManageWindow ();
#endif
extern void UnManageWindow ();
extern void WithdrawTransientChildren ();
extern void WithdrawWindow ();
extern void ResetWithdrawnFocii ();
extern void FreeClientFrame ();
extern void FreeIcon ();
extern void WithdrawDialog ();
extern void ReManageDialog ();
#else /* _NO_PROTO */
extern void AdoptInitialClients (WmScreenData *pSD);
#ifdef DEC_MOTIF_EXTENSION
extern void WmManageModReset( XEvent *event );
#endif /* DEC_MOTIF_EXTENSION */
#ifdef DEC_MOTIF_BUG_FIX
extern Boolean ManageWindow (WmScreenData *pSD, Window clientWindow, long manageFlags);
#else
extern void ManageWindow (WmScreenData *pSD, Window clientWindow, long manageFlags);
#endif

extern void UnManageWindow (ClientData *pCD);
extern void WithdrawTransientChildren (ClientData *pCD);
extern void WithdrawWindow (ClientData *pCD);
extern void ResetWithdrawnFocii (ClientData *pCD);
extern void FreeClientFrame (ClientData *pCD);
extern void FreeIcon (ClientData *pCD);
extern void WithdrawDialog (Widget dialogboxW);
extern void ReManageDialog (WmScreenData *pSD, Widget dialogboxW);
#endif /* _NO_PROTO */
