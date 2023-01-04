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
/*   $RCSfile: TextInP.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:53:07 $ */
/*
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmTextInP_h
#define _XmTextInP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/Text.h>
#include <Xm/TextStrSoP.h>

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
 *
 * Definitions for modules implementing text input modules.
 *
 ****************************************************************/

typedef struct {
    int x;
    int y;
} SelectionHint;

typedef struct _InputDataRec {
    XmTextWidget widget;		/* Back-pointer to widget record. */
    XmTextScanType *sarray;	/* Description of what to cycle through on */
				/* selections. */
    int sarraycount;		/* Number of elements in above. */
    int new_sel_length;		/* New selection length for selection moves. */
    int threshold;		/* number of pixels crossed -> drag */
    SelectionHint selectionHint; /* saved coords of button down */
    SelectionHint Sel2Hint;	/* saved the coords of button down */
    XtIntervalId select_id;
    XmTextScanType stype;	/* Current selection type. */
    XmTextScanDirection extendDir;
    XmTextScanDirection Sel2ExtendDir;
    XmTextPosition origLeft, origRight;
    XmTextPosition Sel2OrigLeft, Sel2OrigRight;
    XmTextPosition stuffpos;
    XmTextPosition sel2Left, sel2Right; /* secondary selection */
    XmTextPosition anchor;	/* anchor point of the primary selection */
    Position select_pos_x;	/* x position for timer-based scrolling */
    Position select_pos_y;	/* y position for timer-based scrolling */
    Boolean pendingdelete;	/* TRUE if we're implementing pending delete */
    Boolean syncing;		/* If TRUE, then we've multiple keystrokes */
    Boolean extending;      /* true if we are extending */
    Boolean Sel2Extending;	/* true if we are extending */
    Boolean hasSel2;   		/* has secondary selection */
    Boolean has_destination;  	/* has destination selection */
    Boolean selectionMove;	/* delete selection after stuff */
    Boolean cancel;		/* indicates that cancel was pressed */
    Boolean overstrike;     	/* overstrike */
    Boolean sel_start;		/* indicates that a btn2 was pressed */
    Time dest_time;		/* time of destination selection ownership */
    Time sec_time;		/* time of secondary selection ownership */
    Time lasttime;		/* Time of last event. */
} InputDataRec, *InputData;


/* 
 * Create a new instance of an input object.  By the time this is called,
 * the widget context has been saved.
 */

#ifdef _NO_PROTO
typedef void (*InputCreateProc)(); /* widget, args, num_args */
#else
typedef void (*InputCreateProc)(
			Widget,
			ArgList,
			Cardinal) ;
#endif

/*
 * Get values out of the input object.
 */
#ifdef _NO_PROTO
typedef void (*InputGetValuesProc)(); /* widget, args, num_args */
#else
typedef void (*InputGetValuesProc)(
			Widget,
			ArgList,
			Cardinal) ;
#endif

/*
 * Set values in the input object.
 */

#ifdef _NO_PROTO
typedef void (*InputSetValuesProc)(); /* oldw, reqw, new_w, args, num_args */
#else
typedef void (*InputSetValuesProc)(
			Widget,
			Widget,
			Widget,
			ArgList,
			Cardinal *) ;
#endif

/*
 * Inform input of invalidated positions.
 */
#ifdef _NO_PROTO
typedef void (*InputInvalidateProc)(); /* ctx, position, topos, delta */
#else
typedef void (*InputInvalidateProc)(
			XmTextWidget,
			XmTextPosition,
			XmTextPosition,
			long) ;
#endif

/*
 * Get secondary resources.
 */
#ifdef _NO_PROTO
typedef void (*InputGetSecResProc)(); /* secResDataRtn */
#else
typedef void (*InputGetSecResProc)(
			XmSecondaryResourceData *) ;
#endif


typedef struct _InputRec {
    struct _InputDataRec *data;	/* Input-specific data; opaque type. */
    InputInvalidateProc Invalidate;
    InputGetValuesProc  GetValues;
    InputSetValuesProc	SetValues;
    XtWidgetProc	destroy;
    InputGetSecResProc  GetSecResData;
} InputRec;


externalref XtPointer _XmdefaultTextActionsTable;

#ifdef DEC_MOTIF_BUG_FIX
externalref Cardinal _XmdefaultTextActionsTableSize;
#else
extern Cardinal _XmdefaultTextActionsTableSize;
#endif

/********    Private Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget _XmTextGetDropReciever() ;
extern Boolean _XmTextHasDestination() ;
extern Boolean _XmTextSetDestinationSelection() ;
extern Boolean _XmTextSetSel2() ;
extern Boolean _XmTextGetSel2() ;
extern void _XmTextInputGetSecResData() ;
extern XmTextPosition _XmTextGetAnchor() ;
extern void _XmTextInputCreate() ;

#else

extern Widget _XmTextGetDropReciever( 
                        Widget w) ;
extern Boolean _XmTextHasDestination( 
                        Widget w) ;
extern Boolean _XmTextSetDestinationSelection( 
                        Widget w,
                        XmTextPosition position,
#if NeedWidePrototypes
                        int disown,
#else
                        Boolean disown,
#endif /* NeedWidePrototypes */
                        Time set_time) ;
extern Boolean _XmTextSetSel2( 
                        XmTextWidget tw,
                        XmTextPosition left,
                        XmTextPosition right,
                        Time set_time) ;
extern Boolean _XmTextGetSel2( 
                        XmTextWidget tw,
                        XmTextPosition *left,
                        XmTextPosition *right) ;
extern void _XmTextInputGetSecResData( 
                        XmSecondaryResourceData *secResDataRtn) ;
extern XmTextPosition _XmTextGetAnchor( 
                        XmTextWidget tw) ;
extern void _XmTextInputCreate( 
                        Widget wid,
                        ArgList args,
                        Cardinal num_args) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmTextInP_h */
/*DON'T ADD ANYTHING AFTER THIS #endif */
