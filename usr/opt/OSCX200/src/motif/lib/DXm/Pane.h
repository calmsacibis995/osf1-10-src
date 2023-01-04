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
* $Header: /usr/sde/osf1/rcs/x11/src/motif/lib/DXm/Pane.h,v 1.1.4.2 1993/05/07 01:37:30 Andy_DAmore Exp $
*/

/*
 * Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.
 * 
 *                         All Rights Reserved
 * 
 * Permission to use, copy, modify, and distribute this software and its 
 * documentation for any purpose and without fee is hereby granted, 
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in 
 * supporting documentation, and that the name of Digital Equipment
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.  
 * 
 * 
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */
/*
 * V1.0a 17-Mar-1988	RDB
 *	fix major bugs and enhancements
 * V1.4	 03-Nov-1988	RDB
 *	updates and revisions
 * V2.0	 04-Apr-1989	RDB
 *	updates and revisions
 * V2.1	 11-Apr-1989	RDB
 *	add leftmullion
 *       09-Jan-1990	SDL
 *	Run through the Motif converter DXM_PORT.COM
 *	 10-Jan-1990 	WDW
 *	Add VMSDescToNull which was supplied by J. Vangilder.
 *	Make sure shorts, Dimensions, and Positions were not
 *	intermixed.
 *	 11-Jan-1990 	WDW
 *	Update documentation.
 *	 23-Jan-1990	WDW
 *	Remove DWT references.  Make resource names similar to NetEd
 *	and GObE's.
 *	 23-Jan-1990	WDW
 *      Make constants and masks similar to GObE and NetEd's.
 *       26-Jan-1990 	WDW
 *	Ultrix compatibility.
 *       31-Jan-1990 	WDW
 *	Modify header files so private part is in panep.h
 *	 01-Feb-1990	SDL
 *	Add widget class definition (get from panep.h)
 *	 02-Feb-1990	SDL
 *	To compile cleanly with /STANDARD=PORT, add #pragma standard
 *	lines around #include files and externalrefs, fix up #endif. 
 *       03-Aug-1992    CS
 *      Add typedef for pane_widget.
 */

#ifndef _Pane_h
#define _Pane_h
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef VMS
/*#pragma nostandard*/
#include "PanedW.h"		/* gives XmNminimum and XmNmaximum */
#include "XmP.h"
/*#pragma standard*/
#else
#include <Xm/PanedW.h>
#include <Xm/XmP.h>
#endif


#ifndef PANE
/*#pragma nostandard*/
externalref WidgetClass panewidgetclass;
/*#pragma standard*/
#endif

typedef struct _PaneRec * pane_widget;

#define PaneMpbNotShared	0L
#define	PaneMpbShared		(1L<<0)
#define	PaneMpbViewableInPane	(1L<<1)

/***********************************************************************
 *
 * Pane Widget (subclass of CompositeClass)
 *
 ***********************************************************************/

/* New fields */
#define PaneNmullionSize	"mullionSize"
#define PaneNoverrideText	"overrideText"
#define PaneNposition		"position"
#define PaneNsharedFlag		"sharedFlag"

#define PaneCMullionSize	"MullionSize"
#define PaneCOverrideText	"OverrideText"
#define PaneCPosition		"Position"
#define PaneCSharedFlag		"SharedFlag"

#ifdef _NO_PROTO

extern Widget PaneCreateWidget ( );
extern Widget PaneWidget ( );
extern void PaneAddWidget ( );
extern void PaneGetMinMax ( );
extern void PaneSetMinMax ( );
extern void PaneSetMin ( );
extern void PaneSetMax ( );
extern void PaneAllowResizing ( );
extern void PaneMakeViewable ( );
extern unsigned int PaneInitializeForMRM ( );
#ifdef VMS
extern char *VMSDescToNull ( );
#endif

#else

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern Widget PaneCreateWidget ( Widget p , char *name , ArgList al , Cardinal ac );
extern Widget PaneWidget ( Widget p , char *name , Position x , Position y , Dimension width , Dimension height , int orientation , XtCallbackList mapcallback , XtCallbackList helpcallback );
extern void PaneAddWidget ( Widget subwidget , Cardinal position , Dimension min , Dimension max , Boolean resizable , Cardinal sharedflag );
extern void PaneGetMinMax ( Widget subwidget , Dimension *min , Dimension *max );
extern void PaneSetMinMax ( Widget subwidget , Dimension min , Dimension max );
extern void PaneSetMin ( Widget subwidget , Dimension min );
extern void PaneSetMax ( Widget subwidget , Dimension max );
extern void PaneAllowResizing ( pane_widget pane , int allowtype );
extern void PaneMakeViewable ( Widget subwidget );
extern unsigned int PaneInitializeForMRM ( void );
#ifdef VMS
extern char *VMSDescToNull ( struct dsc$descriptor_s *desc );
#endif
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO undefined */


#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _Pane_h */
/* DON'T ADD STUFF AFTER THIS #endif */
