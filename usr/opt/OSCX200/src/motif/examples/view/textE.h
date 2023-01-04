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
 * @(#)$RCSfile: textE.h,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/11/05 20:40:43 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: textE.h,v $ $Revision: 1.1.2.2 $ $Date: 1993/11/05 20:40:43 $ */

#if ( defined text_h )
#define extern 
#endif

#ifdef _NO_PROTO
extern void FileOKCallback();
extern void NewPaneCallback();
extern void KillPaneCallback();
extern void FindCallback();
#else
extern void FileOKCallback(Widget fsb, ViewPtr this,
			   XmFileSelectionBoxCallbackStruct *call_data);

extern void FindCallback(Widget button, ViewPtr this,
			  XmPushButtonCallbackStruct *call_data);

extern void NewPaneCallback(Widget fsb, ViewPtr this,
			    XmPushButtonCallbackStruct *call_data);

extern void KillPaneCallback(Widget button, ViewPtr this,
			    XmPushButtonCallbackStruct *call_data);

#endif

#if ( defined extern )
#undef extern 
#endif
