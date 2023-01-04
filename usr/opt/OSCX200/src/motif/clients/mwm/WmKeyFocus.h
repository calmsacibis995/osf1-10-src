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
/*   $RCSfile: WmKeyFocus.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:23:53 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
void		AutoResetKeyFocus ();
void		ClearFocusIndication ();
Boolean		CheckForKeyFocus ();
void		DoExplicitSelectGrab ();
ClientData *	FindLastTransientTreeFocus ();
ClientData *	FindNextTFocusInSeq ();
ClientData *	FindPrevTFocusInSeq ();
Boolean		FocusNextTransient ();
Boolean         FocusNextWindow ();
Boolean		FocusPrevTransient ();
Boolean		FocusPrevWindow ();
ClientData *	GetClientUnderPointer ();
void		InitKeyboardFocus ();
void		RepairFocus ();
void		ResetExplicitSelectHandling ();
void		SetFocusIndication ();
void		SetKeyboardFocus ();


#else /* _NO_PROTO */

extern void AutoResetKeyFocus (ClientData *pcdNoFocus, Time focusTime);
extern void ClearFocusIndication (ClientData *pCD, Boolean refresh);
extern Boolean CheckForKeyFocus (ClientListEntry *pNextEntry, 
				 unsigned long type, Boolean focusNext, 
				 Time focusTime);
extern void DoExplicitSelectGrab (Window window);
extern ClientData *FindLastTransientTreeFocus (ClientData *pCD, 
					       ClientData *pcdNoFocus);
extern ClientData *FindNextTFocusInSeq (ClientData *pCD, 
					unsigned long startAt);
extern ClientData *FindPrevTFocusInSeq (ClientData *pCD, 
					unsigned long startAt);
extern Boolean FocusNextTransient (ClientData *pCD, unsigned long type, 
				   Boolean initiate, Time focusTime);
extern Boolean FocusNextWindow (unsigned long type, Time focusTime);
extern Boolean FocusPrevTransient (ClientData *pCD, unsigned long type, 
				   Boolean initiate, Time focusTime);
extern Boolean FocusPrevWindow (unsigned long type, Time focusTime);
extern ClientData *GetClientUnderPointer (Boolean *pSameScreen);
extern void InitKeyboardFocus (void);
extern void RepairFocus (void);
extern void ResetExplicitSelectHandling (ClientData *pCD);
extern void SetFocusIndication (ClientData *pCD);
extern void SetKeyboardFocus (ClientData *pCD, long focusFlags);

#endif /* _NO_PROTO */


