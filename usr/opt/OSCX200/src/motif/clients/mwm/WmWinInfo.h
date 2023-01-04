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
/*   $RCSfile: WmWinInfo.h,v $ $Revision: 1.1.4.3 $ $Date: 1993/07/16 22:00:15 $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO

extern void FindClientPlacement ();
extern void FixWindowConfiguration ();
extern void FixWindowSize ();
extern ClientData *GetClientInfo ();
extern ClientData *GetWmClientInfo ();
extern void CalculateGravityOffset ();
extern Boolean InitClientPlacement ();
extern void InitCColormapData ();
extern void MakeSystemMenu ();
extern void PlaceFrameOnScreen ();
extern void PlaceIconOnScreen ();
extern void ProcessMwmHints ();
extern void ProcessWmClass ();
extern void ProcessWmHints ();
extern void ProcessWmIconTitle ();
extern void ProcessWmNormalHints ();
#ifdef DEC_MOTIF_EXTENSION
extern Boolean WmCvtNamePropToXmString ();
#endif /* DEC_MOTIF_EXTENSION */
extern void ProcessWmTransientFor ();
extern void ProcessWmWindowTitle ();
extern Boolean SetupClientIconWindow ();
extern Boolean WmGetWindowAttributes ();

#else /* _NO_PROTO */

extern void FindClientPlacement (ClientData *pCD);
extern void FixWindowConfiguration (ClientData *pCD, unsigned int *pWidth, 
				    unsigned int *pHeight, 
				    unsigned int widthInc, 
				    unsigned int heightInc);
extern void FixWindowSize (ClientData *pCD, unsigned int *pWidth, 
			   unsigned int *pHeight, unsigned int widthInc, 
			   unsigned int heightInc);
extern ClientData *GetClientInfo (WmScreenData *pSD, Window clientWindow, 
				  long manageFlags);
extern ClientData *GetWmClientInfo (WmWorkspaceData *pWS, ClientData *pCD, 
				    long manageFlags);
extern void CalculateGravityOffset (ClientData *pCD, int *xoff, int *yoff);
#ifdef DEC_MOTIF_BUG_FIX
extern Boolean InitClientPlacement (ClientData *pCD, long manageFlags,
                                    Boolean *changeFlag );
#else
extern Boolean InitClientPlacement (ClientData *pCD, long manageFlags);
#endif
extern void InitCColormapData (ClientData *pCD);
extern void MakeSystemMenu (ClientData *pCD);
extern void PlaceFrameOnScreen (ClientData *pCD, int *pX, int *pY, int w, 
				int h);
extern void PlaceIconOnScreen (ClientData *pCD, int *pX, int *pY);
extern void ProcessMwmHints (ClientData *pCD);
extern void ProcessWmClass (ClientData *pCD);
extern void ProcessWmHints (ClientData *pCD, Boolean firstTime);
extern void ProcessWmIconTitle (ClientData *pCD, Boolean firstTime);
extern void ProcessWmNormalHints (ClientData *pCD, Boolean firstTime, 
				  long manageFlags);
#ifdef DEC_MOTIF_EXTENSION
extern Boolean WmCvtNamePropToXmString( XTextProperty wmNameProp, 
                    			XmString *wmXmString);
#endif /* DEC_MOTIF_EXTENSION */
extern void ProcessWmTransientFor (ClientData *pCD);
extern void ProcessWmWindowTitle (ClientData *pCD, Boolean firstTime);
extern Boolean SetupClientIconWindow (ClientData *pCD, Window window);
extern Boolean WmGetWindowAttributes (Window window);

#endif /* _NO_PROTO */
