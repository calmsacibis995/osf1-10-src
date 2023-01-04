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
/*   $RCSfile: WmFeedback.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:02:16 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
extern void        ConfirmAction ();
extern void        HideFeedbackWindow ();
extern void        InitCursorInfo ();
extern void        PaintFeedbackWindow();
extern void        ShowFeedbackWindow();
extern void        ShowWaitState ();
extern void        UpdateFeedbackInfo();
extern void        UpdateFeedbackText();

#else /* _NO_PROTO */

extern void ConfirmAction (WmScreenData *pSD, int nbr);
extern void HideFeedbackWindow (WmScreenData *pSD);
extern void InitCursorInfo (void);
extern void PaintFeedbackWindow (WmScreenData *pSD);
extern void ShowFeedbackWindow (WmScreenData *pSD, int x, int y, 
				unsigned int width, unsigned int height, 
				unsigned long style);
extern void ShowWaitState (Boolean flag);
extern void UpdateFeedbackInfo (WmScreenData *pSD, int x, int y, 
				unsigned int width, unsigned int height);
extern void UpdateFeedbackText (WmScreenData *pSD, int x, int y, 
				unsigned int width, unsigned int height);

#endif /* _NO_PROTO */
