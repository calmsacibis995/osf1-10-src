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
/*   $RCSfile: ListP.h,v $ $Revision: 1.1.6.2 $ $Date: 1993/05/06 15:40:54 $ */
/*
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
#ifndef _XmListP_h
#define _XmListP_h
#if defined(VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#include <Xm/List.h>
#include <Xm/PrimitiveP.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  List struct passed to Convert proc for drag and drop */
typedef struct _XmListDragConvertStruct
{
   Widget w;
   XmString * strings;
   int num_strings;
} XmListDragConvertStruct;

/*  List class structure  */

typedef struct _XmListClassPart
{
   int foo;	/*  No new fields needed  */
} XmListClassPart;


/*  Full class record declaration for List class  */

typedef struct _XmListClassRec
{
   CoreClassPart        core_class;
   XmPrimitiveClassPart primitive_class;
   XmListClassPart     list_class;
} XmListClassRec;

externalref XmListClassRec xmListClassRec;

/****************
 *
 * Internal form of the list elements.
 *
 ****************/
 
typedef	struct {
	_XmString	name;
	Dimension	height;
	Dimension	width;
	Dimension	CumHeight;
	Boolean		selected;
	Boolean		last_selected;
	Boolean		LastTimeDrawn;
	unsigned short	NumLines;
	int		length;
} Element, *ElementPtr;

/*  The List instance record  */

typedef struct _XmListPart
{
	Dimension	spacing;
	short           ItemSpacing;
	Dimension       margin_width;
	Dimension    	margin_height;
	XmFontList 	font;
	XmString	*items;
	int		itemCount;
	XmString	*selectedItems;
        int             *selectedIndices;
	int		selectedItemCount;
	int 		visibleItemCount;
	int 		LastSetVizCount;
	unsigned char	SelectionPolicy;
	unsigned char	ScrollBarDisplayPolicy;
	unsigned char	SizePolicy;
        XmStringDirection StrDir;

        Boolean		AutoSelect;
        Boolean		DidSelection;
        Boolean		FromSetSB;
        Boolean		FromSetNewSize;
        Boolean		AddMode;
	unsigned char	LeaveDir;
	unsigned char	HighlightThickness;
	int 		ClickInterval;
        XtIntervalId	DragID;
	XtCallbackList 	SingleCallback;
	XtCallbackList 	MultipleCallback;
	XtCallbackList 	ExtendCallback;
	XtCallbackList 	BrowseCallback;
	XtCallbackList 	DefaultCallback;


	GC		NormalGC;	
	GC		InverseGC;
	GC		HighlightGC;
        Pixmap          DashTile;
	ElementPtr	*InternalList;
	int		LastItem;
	int		FontHeight;
	int		top_position;
	char		Event;
	int		LastHLItem;
	int		StartItem;
	int		OldStartItem;
	int		EndItem;
	int		OldEndItem;
	Position	BaseX;
	Position	BaseY;
	Boolean		MouseMoved;
	Boolean		AppendInProgress;
	Boolean		Traversing;
	Boolean		KbdSelection;
	short		DownCount;
	Time		DownTime;
	int		CurrentKbdItem;
	unsigned char	SelectionType;
	GC		InsensitiveGC;

	int vmin;		  /*  slider minimum coordiate position     */
	int vmax;		  /*  slider maximum coordiate position     */
	int vOrigin;		  /*  slider edge location                  */
	int vExtent;		  /*  slider size                           */

	int hmin;		  /*  Same as above for horizontal bar.     */
	int hmax;
	int hOrigin;
	int hExtent;

	Dimension	MaxWidth;
	Dimension	CharWidth;
	Position	XOrigin;
	
	XmScrollBarWidget   	hScrollBar;
	XmScrollBarWidget   	vScrollBar;
	XmScrolledWindowWidget  Mom;
	Dimension	MaxItemHeight;
	
} XmListPart;


/*  Full instance record declaration  */

typedef struct _XmListRec
{
   CorePart	   core;
   XmPrimitivePart primitive;
   XmListPart	   list;
} XmListRec;


/********    Private Function Declarations    ********/
#ifdef _NO_PROTO


#else


#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
#endif /* _XmListP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
