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
/*   $RCSfile: WmMenu.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 21:38:47 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO

extern void      ActivateCallback ();
extern Widget    CreateMenuWidget ();
extern void      FreeCustomMenuSpec ();
extern void      MWarning ();
extern MenuSpec *MakeMenu ();
extern void      PostMenu ();
extern void      TraversalOff ();
extern void      TraversalOn ();
extern void      UnpostMenu ();
extern void      UnmapCallback ();


#else /* _NO_PROTO */

extern void ActivateCallback (Widget w, XtPointer client_data, 
			      XtPointer call_data);
extern Widget CreateMenuWidget (WmScreenData *pSD, String menuName, 
				Widget parent, Boolean fTopLevelPane, 
				MenuSpec *topMenuSpec, 
				MenuItem *moreMenuItems);
extern void FreeCustomMenuSpec (MenuSpec *menuSpec);
extern void MWarning (char *format, char *message);
extern MenuSpec *MakeMenu (WmScreenData *pSD, String menuName, 
			   Context initialContext, Context accelContext, 
			   MenuItem *moreMenuItems, Boolean fSystemMenu);
extern void PostMenu (MenuSpec *menuSpec, ClientData *pCD, int x, int y, 
		      unsigned int button, Context newContext, long flags, 
		      XEvent *passedInEvent);
extern void TraversalOff (MenuSpec *menuSpec);
extern void TraversalOn (MenuSpec *menuSpec);
extern void UnmapCallback (Widget w, XtPointer client_data, XtPointer call_data);
extern void UnpostMenu (MenuSpec *menuSpec);

#endif /* _NO_PROTO */
