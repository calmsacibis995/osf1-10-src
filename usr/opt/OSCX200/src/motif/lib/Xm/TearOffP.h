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
 * @(#)$RCSfile: TearOffP.h,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/06 15:50:49 $
 */
/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $RCSfile: TearOffP.h,v $ $Revision: 1.1.4.2 $ $Date: 1993/05/06 15:50:49 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
#ifndef _XmTearOffP_h
#define _XmTearOffP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _XmExcludedParentPaneRec
{
   short pane_list_size;
   Widget *pane;
   short num_panes;
} XmExcludedParentPaneRec;

#ifdef DEC_MOTIF_BUG_FIX
externalref XmExcludedParentPaneRec _XmExcludedParentPane;
#else
extern XmExcludedParentPaneRec _XmExcludedParentPane;
#endif

/********    Private Function Declarations    ********/
#ifdef VMS
/* VMS limit of 31 characters in extern function names */
#define _XmLowerTearOffObscuringPoppingDownPanes    _XmLowerTearOffObscurPopDwnPane
#define	_XmRestoreExcludedTearOffToToplevelShell    _XmRestExcludedTearOffToTopShel
#define	_XmRestoreTearOffToToplevelShell	    _XmRestoreTearOffToToplevelShel
#endif

#ifdef _NO_PROTO

extern void _XmTearOffBtnDownEventHandler() ;
extern void _XmTearOffBtnUpEventHandler() ;
extern void _XmDestroyTearOffShell() ;
extern void _XmDismissTearOff() ;
extern void _XmTearOffInitiate() ;
extern void _XmAddTearOffEventHandlers() ;
extern Boolean _XmIsTearOffShellDescendant() ;
extern void _XmLowerTearOffObscuringPoppingDownPanes() ;
extern void _XmRestoreExcludedTearOffToToplevelShell() ;
extern void _XmRestoreTearOffToToplevelShell() ;
extern void _XmRestoreTearOffToMenuShell() ;

#else

extern void _XmTearOffBtnDownEventHandler( 
                        Widget reportingWidget,
                        XtPointer data,
                        XEvent *event,
                        Boolean *cont) ;
extern void _XmTearOffBtnUpEventHandler( 
                        Widget reportingWidget,
                        XtPointer data,
                        XEvent *event,
                        Boolean *cont) ;
extern void _XmDestroyTearOffShell( 
                        Widget wid) ;
extern void _XmDismissTearOff( 
                        Widget shell,
                        XtPointer closure,
                        XtPointer call_data) ;
extern void _XmTearOffInitiate( 
                        Widget wid,
                        XEvent *event) ;
extern void _XmAddTearOffEventHandlers( 
                        Widget wid) ;
extern Boolean _XmIsTearOffShellDescendant( 
                        Widget wid) ;
extern void _XmLowerTearOffObscuringPoppingDownPanes(
			Widget ancestor,
			Widget tearOff ) ;
extern void _XmRestoreExcludedTearOffToToplevelShell( 
                        Widget wid,
                        XEvent *event) ;
extern void _XmRestoreTearOffToToplevelShell( 
                        Widget wid,
                        XEvent *event) ;
extern void _XmRestoreTearOffToMenuShell( 
                        Widget wid,
                        XEvent *event) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif  /* _XmTearOffP_h */
/* DON'T ADD STUFF AFTER THIS #endif */
