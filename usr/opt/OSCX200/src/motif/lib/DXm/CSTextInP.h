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
*****************************************************************************
**                                                                          *
**                         COPYRIGHT (c) 1990 BY                            *
**             DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                *
**                         ALL RIGHTS RESERVED                              *
**                                                                          *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED  *
**  ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE  *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER  *
**  COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY  *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY  *
**  TRANSFERRED.                                                            *
**                                                                          *
**  THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE  *
**  AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS  *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**      < to be supplied >
**
**  ABSTRACT:
**
**      < to be supplied >
**
**  ENVIRONMENT:
**
**      < to be supplied >
**
**  MODIFICATION HISTORY:
**
**	Created IN_LOSE_SELECTION to replace -999 constant in CSTextIn.c
**
**
**--
**/
#ifndef _DXmCSTextInP_h
#define _DXmCSTextInP_h
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include "CSTextSrc.h"

#define IN_LOSE_SELECTION -999

#define InputData(w)  ((DXmCSTextInputData)  \
	     ( ((DXmCSTextInput)(Input (w)) )->data))

/****************************************************************
 *
 * Definitions for modules implementing text input modules.
 *
 ****************************************************************/

typedef struct {
    int x;
    int y;
} DXmCSTextSelectionHint;

typedef struct _DXmCSTextInputDataRec {
    DXmCSTextWidget 		widget;	     	/* Back-ptr to widget record. */
    XComposeStatus 		compstatus;  	/* Compose key status. */
    DXmCSTextScanType 		*sarray;  	/* Description of what to cycle
                                                   through on selections. */
    int 			sarraycount;	/* # of elements in above */
    Time 			lasttime;	/* Time of last event. */
    DXmCSTextScanType 		stype;		/* Current selection type. */
    DXmCSTextScanDirection 	extendDir;
    DXmCSTextScanDirection 	Sel2ExtendDir;
    DXmCSTextPosition 		origLeft, 	
				origRight;
    DXmCSTextPosition 		Sel2OrigLeft,	
				Sel2OrigRight;
    DXmCSTextPosition 		stuffpos;
    Boolean 			pendingdelete;	/* TRUE if we're implementing 
						   pending delete */
    Boolean 			syncing;	/* If TRUE, then we've multiple
						   keystrokes */
    Boolean 			extending;	/* true if we are extending */
    Boolean 			Sel2Extending;	/* true if we are extending */
    DXmCSTextSelectionHint 	selectionHint;	/* saved coords of button down*/
    DXmCSTextSelectionHint 	Sel2Hint;	/* saved coords of button down*/
    int 			threshold;	/* # of pixels crossed -> drag*/
    DXmCSTextPosition 		sel2Left, 	
				sel2Right; 	/* secondary selection */
    Boolean 			hasSel2;	/* has secondary selection */
    Boolean 			has_destination; /* has destination selection */
    Boolean 			selectionMove;/* delete selection after stuff */
    Boolean 			cancel;	/* indicates that cancel was pressed */
    Boolean 			pendingoff;	/* TRUE if we shouldn't do 
						   pending delete on the 
						   current selection. */
    DXmCSTextFormat 		pselformat; 	/* format of last imported 
						   primary selection */
    DXmCSTextFormat 		sselformat; 	/* format of last imported 
						   secondary selection*/
    Time 			ppastetime;     /* Time paste event occurred 
						   (primary) */
    Time 			spastetime;	/* Time paste event occurred 
						   (secondary) */
    Boolean 			verticalMoveLast;   
						/* true is last user cmnd was 
						   vertical move */
    Position 			verticalXPosition; 
						/* used for vertical moves */
    Position 			verticalYPosition; 
						/* used for vertical moves */

    Boolean 			quick_key;	/* 2ndary selection via the 
						   keyboard */
    DXmCSTextPosition 		anchor;		/* anchor point of the 
						   primary selection */
    Time dest_time;		/* time of destination selection ownership */
    Time sec_time;		/* time of secondary selection ownership */
    Boolean changed_dest_visible;   /* destination visibility changed */

} DXmCSTextInputDataRec, *DXmCSTextInputData;
  

typedef void (*DXmCSTextInputInvalidateProc)(); /* ctx, position, topos, delta*/
typedef void (*DXmCSTextInputSetValuesProc)(); /* widget, args, num_args */
typedef void (*DXmCSTextInputGetValuesProc)(); /* widget, args, num_args */

typedef struct _DXmCSTextInputRec {
    struct _DXmCSTextInputDataRec 	*data; 
    DXmCSTextInputInvalidateProc 	Invalidate;
    DXmCSTextInputGetValuesProc  	GetValues;
    DXmCSTextInputSetValuesProc		SetValues;
    XtWidgetProc			destroy;
} DXmCSTextInputRec;

typedef struct {
    Widget widget;
    DXmCSTextPosition insert_pos;
    int num_chars;
    Time timestamp;
    Boolean move;
} _DXmCSTextDropTransferRec;

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _DXmCSTextInP_h */
/*DON'T ADD ANYTHING AFTER THIS #endif */

