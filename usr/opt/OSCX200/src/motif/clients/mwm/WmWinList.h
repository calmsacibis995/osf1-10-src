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
/*   $RCSfile: WmWinList.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:09:28 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO

extern void			AddClientToList ();
extern void			AddEntryToList ();
extern void			AddTransient ();
extern Boolean			CheckIfClientObscuredByAny ();
extern Boolean			CheckIfClientObscuring ();
extern Boolean			CheckIfClientObscuringAny ();
extern Boolean			CheckIfObscuring ();
extern int			CountTransientChildren ();
extern void			DeleteClientFromList ();
extern void			DeleteEntryFromList ();
extern void			DeleteFullAppModalChildren();
extern void			DeleteTransient();
extern ClientListEntry *	FindClientNameMatch ();
extern ClientData *		FindTransientFocus ();
extern ClientData *		FindTransientOnTop ();
extern ClientData *		FindTransientTreeLeader ();
extern void			FixupFullAppModalCounts();
extern Window *		        MakeTransientWindowList ();
extern void			MarkModalSubtree ();
extern void			MarkModalTransient ();
extern void			MoveEntryInList ();
extern Boolean			PutTransientBelowSiblings ();
extern Boolean			PutTransientOnTop ();
extern void			RestackTransients ();
extern void			RestackTransientsAtWindow ();
extern void			SetupSystemModalState ();
extern void			StackTransientWindow ();
extern void			StackWindow ();
extern void			UnMarkModalTransient ();
extern void			UndoSystemModalState ();
extern void			AddLostTransient ();

#else /* _NO_PROTO */

extern void AddClientToList (WmWorkspaceData *pWS, ClientData *pCD, 
			     Boolean onTop);
extern void AddEntryToList (WmWorkspaceData *pWS, ClientListEntry *pEntry, 
			    Boolean onTop, ClientListEntry *pStackEntry);
extern void AddTransient (WmWorkspaceData *pWS, ClientData *pCD);
extern Boolean CheckIfClientObscuredByAny (ClientData *pcd);
extern Boolean CheckIfClientObscuring (ClientData *pcdTop, ClientData *pcd);
extern Boolean CheckIfClientObscuringAny (ClientData *pcd);
extern Boolean CheckIfObscuring (ClientData *pcdA, ClientData *pcdB);
extern int CountTransientChildren (ClientData *pcd);
extern void DeleteClientFromList (WmWorkspaceData *pWS, ClientData *pCD);
extern void DeleteEntryFromList (WmWorkspaceData *pWS, 
				 ClientListEntry *pListEntry);
extern void DeleteFullAppModalChildren (ClientData *pcdLeader, 
					ClientData *pCD);
extern void DeleteTransient (ClientData *pCD);
extern ClientListEntry *FindClientNameMatch (ClientListEntry *pEntry, 
					     Boolean toNext, 
					     String clientName,
					     unsigned long types);
extern ClientData *FindTransientFocus (ClientData *pcd);
extern ClientData *FindTransientOnTop (ClientData *pcd);
extern ClientData *FindTransientTreeLeader (ClientData *pcd);
extern void FixupFullAppModalCounts (ClientData *pcdLeader, 
				     ClientData *pcdDelete);
extern Window *MakeTransientWindowList (Window *windows, ClientData *pcd);
extern void MarkModalSubtree (ClientData *pcdTree, ClientData *pcdAvoid);
extern void MarkModalTransient (ClientData *pcdLeader, ClientData *pCD);
extern void MoveEntryInList (WmWorkspaceData *pWS, ClientListEntry *pEntry, 
			     Boolean onTop, ClientListEntry *pStackEntry);
extern Boolean PutTransientBelowSiblings (ClientData *pcd);
extern Boolean PutTransientOnTop (ClientData *pcd);
extern void RestackTransients (ClientData *pcd, Boolean doTop);
extern void RestackTransientsAtWindow (ClientData *pcd);
extern void SetupSystemModalState (ClientData *pCD);
extern void StackTransientWindow (ClientData *pcd);
extern void StackWindow (WmWorkspaceData *pWS, ClientListEntry *pEntry, 
			 Boolean onTop, ClientListEntry *pStackEntry);
extern void UnMarkModalTransient (ClientData *pcdModee, int modalCount, 
				  ClientData *pcdModal);
extern void UndoSystemModalState (void);
extern void AddLostTransient (ClientData *pCD);

#endif /* _NO_PROTO */
