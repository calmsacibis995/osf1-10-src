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
*=======================================================================
*
*                  COPYRIGHT (c) 1988, 1989, 1992 BY
*            DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*
* THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND  COPIED
* ONLY  IN  ACCORDANCE  WITH  THE  TERMS  OF  SUCH  LICENSE AND WITH THE
* INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR  ANY  OTHER
* COPIES  THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY
* OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE  IS  HEREBY
* TRANSFERRED. 
*
* THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE  WITHOUT  NOTICE
* AND  SHOULD  NOT  BE  CONSTRUED  AS  A COMMITMENT BY DIGITAL EQUIPMENT
* CORPORATION.
*
* DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE OR  RELIABILITY  OF  ITS
* SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.
*
*=======================================================================
*/

/******************************************************************************/
/*									      */
/*   FACILITY:								      */
/*									      */
/*        SVN -- Structured Visual Navigation Widget 			      */
/*									      */
/*   ABSTRACT:								      */
/*									      */
/*									      */
/*   AUTHORS:								      */
/*									      */
/*									      */
/*   CREATION DATE:							      */
/*									      */
/*   MODIFICATION HISTORY:						      */
/*									      */
/*	020	AN			03-Feb-1993			      */
/*		Fix widget macros, so that when rowcolumn widget are used as  */
/*		components, layout does not get messed up. Fixed INIT_WIDGEt  */
/*		and SHOW_WIDGET macros.					      */
/*      019     CS                      11-Aug-1992                           */
/*              Add declaration for function StructTranslateCoords.           */
/*	018	CS			 2-Jan-1992			      */
/*		Changed and added widget manipulation macros.  Moved some of  */
/*		the dxmSvnWindowWidgetClass definitions here.		      */
/*	017	CS			30-Oct-1991			      */
/*		Changed macros REALWIDTH and REALHEIGHT to return width and   */
/*		height of the svn.primary_window_widget.		      */
/*	016	CS			25-Oct-1991			      */
/*		Added macros PRIMARY_HEIGHT, PTR_HEIGHT, FORM_HEIGHT	      */
/*	015	AN			20-Sep-1990			      */
/*		Add macro svn_distance... which computes the distance between */
/*		two points, used in new mouse action procs, for dragging.     */
/*		new Motif mouse semantics and location cursor		      */
/*	009	SL	 		22-Aug-1990			      */
/*		Integrate the DEC Israel changes into this module - add       */
/*		macro LayoutIsRtoL.                                           */
/*	008	AN			11-Jul-1990			      */
/*		Change names of variables and anything external to be	      */
/*		internationalized for sides or pane windows.  "rhs"->secondary*/
/*		and "lhs" -> primary.					      */
/*	007	AN			28-Jun-1990			      */
/*		Changed all external routines and symbols to 'DXmSvn' for     */
/*		SVN to be included in the DXmtoolkit.			      */
/*	006	AN			24-Apr-1990			      */
/*		Since Motif 1.1 no longer defines Xm_DEFAULT_CHARSET, do it   */
/*		in this file.						      */
/*      005	WW			26-Jan-1990			      */
/*		Ultrix compatibility.					      */
/*	004	SL	 		24-Jan-1990			      */
/*		Change SvnComp* constants to SvnKcomp* since the naming	      */
/*		convention changed.					      */
/*	003	WW			23-Jan-1990			      */
/*		Convert Svn* constants to SvnK*.			      */
/*	002	WW			23-Jan-1990			      */
/*		Convert SvnWidget to svn_widget.			      */
/*	001	SL	 		05-Jan-1990			      */
/*		Run this module through the Motif converter DXM_PORT.COM      */
/*		and add a modification history.				      */
/*									      */
/*									      */
/******************************************************************************/


/*
** Alpha 1.1 of Motif does not define this default charset anymore.. so we have too.
*/
#if defined (VMS) || defined (__VMS)
#include <X11/apienvset.h>
#endif

#ifdef VMS
#endif


/*
**
**  Data structures for the SVN Window Class.  Note that nobody actually looks 
**  into the instance record.
**
*/
typedef struct
    {                        
    XtPointer dummy;
    } SvnWindowClassPart;

typedef struct
    {
    XtPointer dummy;
    } SvnWindowPart;

typedef struct _SvnWindowClassRec
    {
    CoreClassPart             core_class;
    CompositeClassPart	      composite_class;
    ConstraintClassPart	      constraint_class;
    XmManagerClassPart	      manager_class;
    SvnWindowClassPart        svn_window_class;
    } SvnWindowClassRec, *SvnWindowClass;

typedef struct _SvnWindowRec
    {
    CorePart              core;
    CompositePart         composite;
    ConstraintPart	  constraint;
    XmManagerPart	  manager;
    SvnWindowPart	  svn_window;
    } SvnWindowRec, *SvnWindowWidget;

externalref SvnWindowClass  dxmSvnWindowWidgetClass;


/*
**  Redefine XtIsRealized since windows are integer and cannot be standard
**  compared to NULL.
*/
#undef  XtIsRealized
#define XtIsRealized(widget)	((widget)->core.window != 0)


/* 
** Macro to support DEC Israel RtoL code:
*/
#define LayoutIsRtoL(svnw)	((svnw)->manager.dxm_layout_direction \
					== DXmLAYOUT_LEFT_DOWN)


/*
**  Width & height of scroll and nav buttons.
*/
#define button_height		17



/*
**  Width of line used in highlighting entries in tree mode
*/
#define highlight_width	    4
#define mullion_width       5

/*
**  Size to increment the svn.widget_list field
*/
#define WIDGET_LIST_INCREMENT	10

/*
**  Global macros for manipulating sub widgets.
*/
#define INIT_WIDGET(svnw, w)			    \
{						    \
    XtSetMappedWhenManaged(w,FALSE);		    \
    XtManageChild(subw);			    \
    if (LayoutIsRtoL(svnw))			    \
	XtVaSetValues (subw,			    \
	    XmNleftAttachment, XmATTACH_NONE,	    \
	    XmNbottomAttachment, XmATTACH_NONE,	    \
	    XmNrightAttachment, XmATTACH_FORM,	    \
	    XmNtopAttachment, XmATTACH_FORM, NULL); \
    else					    \
	XtVaSetValues (subw,			    \
	    XmNrightAttachment, XmATTACH_NONE,	    \
	    XmNbottomAttachment, XmATTACH_NONE,	    \
	    XmNleftAttachment, XmATTACH_FORM,	    \
	    XmNtopAttachment, XmATTACH_FORM, NULL); \
}
#define HIDE_WIDGET(w) XtUnmanageChild(w)
#define SHOW_WIDGET(svnw,w,attach,newx,newy)							    \
{												    \
    if (LayoutIsRtoL(svnw))									    \
	XtVaSetValues(w, XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET, XmNtopWidget, (Widget) attach, \
	    XmNrightOffset, (int) newx, XmNtopOffset, (int) newy, NULL);			    \
    else											    \
	XtVaSetValues(w, XmNtopAttachment, XmATTACH_OPPOSITE_WIDGET, XmNtopWidget, (Widget) attach, \
	    XmNleftOffset, (int) newx, XmNtopOffset, (int) newy, NULL);				    \
    XtSetMappedWhenManaged(w,TRUE);								    \
    if (!XtIsManaged(w))									    \
	XtManageChild(w);									    \
}
#define MOVE_WIDGET_VERTICAL(w,newy) XtVaSetValues(w, XmNtopOffset, (int) newy, NULL)
#define MOVE_WIDGET_HORIZONTAL(svnw,w,newx)		    \
{							    \
    if (LayoutIsRtoL(svnw))				    \
	 XtVaSetValues(w, XmNrightOffset, (int) newx, NULL);\
    else						    \
	 XtVaSetValues(w, XmNleftOffset, (int) newx, NULL); \
}

#define OFFSCREENY(w)      (XtHeight(w) * 2)
#define REALHEIGHT(w)	   (int)(XtHeight((w)->svn.primary_window_widget))
#define REALWIDTH(w)       (int)(XtWidth((w)->svn.primary_window_widget))
#define IDXW_WIDTH(svnw)   (REALWIDTH(svnw) >> 1)
#define IDXW_HEIGHT(svnw)  (REALHEIGHT(svnw) >> 1) 
#define FORM_HEIGHT(w)	   (int)(XtHeight((w)->svn.primary_form))		    /* return height of form widget */
#define PRIMARY_HEIGHT(w)  (int)(XtHeight((w)->svn.primary_window_widget))	    /* return height of primary window widget */
#define PTR_HEIGHT(w)	   (int)(XtHeight((w)->svn.primary_ptr_widget))		    /* return height of path-to-root widget */


/*
**  Slider Size = svnw->svn.display_count - svnw->svn.num_path
**  Slider Max Value = svnw->svn.num_entries + 1
*/
#define DRAG_TO_BOTTOM( value ) (((value) > svnw->svn.num_entries + 1 - (svnw->svn.display_count - svnw->svn.num_path)) && \
				(svnw->svn.internal_value - svnw->svn.scroll_value >= svnw->svn.display_count))

#define DRAG_TO_TOP( value )	(((value) == 1 && svnw->svn.num_path != 0) && \
				(svnw->svn.scroll_value - svnw->svn.internal_value >= svnw->svn.display_count))


/* 
** Convert a world coordinate into a window coordinate
*/
#define X_TO_WINDOW(svnw, x)	    ((x) - svnw->svn.mapx) 
#define Y_TO_WINDOW(svnw, y)	    ((y) - svnw->svn.mapy)


/* 
** Convert a window coordinate into a world coordinate
*/
#define X_TO_WORLD(svnw, x)	    ((x) + svnw->svn.mapx)
#define Y_TO_WORLD(svnw, y)	    ((y) + svnw->svn.mapy)

/* 
** get the Y position of an entry in world coordinates
*/
#define X_POS(svnw, svnentry1)				\
    ((svnw->svn.tree_style == DXmSvnKtopTree) ?		\
        (svnentry1->x) :				\
	((svnw->svn.tree_style == DXmSvnKhorizontalTree) ||	\
	 (svnw->svn.tree_style == DXmSvnKoutlineTree) ?	\
	    ((*svnw->svn.levely)[svnentry1->level]) :	\
	    (svnentry1->px)))

#define Y_POS(svnw, svnentry1)				\
    ((svnw->svn.tree_style == DXmSvnKtopTree) ?		\
	((*svnw->svn.levely)[svnentry1->level]) :	\
	((svnw->svn.tree_style == DXmSvnKhorizontalTree) ||	\
	 (svnw->svn.tree_style == DXmSvnKoutlineTree) ?	\
	    (svnentry1->x) :				\
	    (svnentry1->py)))

#define PX_POS(w, svnentry1)				\
    ((w->svn.tree_style == DXmSvnKtopTree) ?		\
	(svnentry1->px) :				\
	((svnentry1->level == 0) ?			\
	    ((w->svn.tree_style != DXmSvnKoutlineTree) ? 0 : (svnw->svn.level_spacing/2)) : \
	    ((*w->svn.levely)[svnentry1->level-1] + svnentry1->py)))	    

#define PY_POS(svnw, svnentry1)				\
    ((svnw->svn.tree_style == DXmSvnKtopTree) ?		\
	((svnentry1->level == 0)? 0 : ((*svnw->svn.levely)[svnentry1->level-1] + svnentry1->py)) : \
	(svnentry1->px))


/*#pragma nostandard*/
#define RECT_COORDS(svnw, svnentry, ex, ey, ew, eh, x1, y1, x2, y2)	    \
{                                                                           \
    x1 = ex;                                                                \
    y1 = ey;		                                                    \
    x2 = ex + ew;							    \
    y2 = ey + eh;							    \
}

#define FILL_RECT_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, width, height) \
{                                                                           \
    x = ex;                                                                 \
    y = ey;                                                                 \
    width = ew;                                                             \
    height = eh;                                                            \
	                                                                    \
    if ((svnw->svn.show_highlighting) && (svnentry->highlighted)) {	    \
	y += highlight_width + 1;					    \
	width -= 2 * highlight_width;					    \
	x += highlight_width;						    \
	height -= (2 * highlight_width) + 1;				    \
	}								    \
									    \
    if (svnw->svn.arc_width != 0) {					    \
	x += svnw->svn.arc_width;					    \
	width -= svnw->svn.arc_width * 2;				    \
	}								    \
									    \
}


#define FILL_RECT_COORDS(svnw, svnentry, ex, ey, ew, eh, x1, y1, x2, y2)    \
{                                                                           \
    FILL_RECT_DATA (svnw, svnentry, ex, ey, ew, eh, x1, y1, x2, y2)         \
    x2 += x1;                                                               \
    y2 += y1;                                                               \
}


#define FILL_LEFT_ARC_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, width, height, start_angle, end_angle) \
{                                                                           \
    if ((svnw->svn.show_highlighting) && (svnentry->highlighted)) {	    \
	width = (2 * svnw->svn.arc_width);				    \
	height = svnentry->height - (2 * highlight_width);		    \
	x = ex + highlight_width;					    \
	y = ey + highlight_width;					    \
	start_angle = -90*64;                                               \
	end_angle = -180*64;                                                \
	}								    \
    else {								    \
	LEFT_ARC_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, width, height, start_angle, end_angle); \
	}								    \
}


#define FILL_RIGHT_ARC_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, width, height, start_angle, end_angle) \
{                                                                           \
    if ((svnw->svn.show_highlighting) && (svnentry->highlighted)) {	    \
	width = (2 * svnw->svn.arc_width);			   	    \
	height = eh - (2 * highlight_width);				    \
	x = ex + ew - (2 * svnw->svn.arc_width) - highlight_width;	    \
	y = ey + highlight_width;					    \
	start_angle = 90*64;                                                \
	end_angle = -180*64;                                                \
	}								    \
    else {								    \
	RIGHT_ARC_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, width, height, start_angle, end_angle); \
	}								    \
}


#define HIGHLIGHT_RECT_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, width, height)      \
{                                                                           \
    x = ex + highlight_width/2;						    \
    y = ey + (highlight_width/2);					    \
    width = ew - highlight_width + 1;					    \
    height = eh - highlight_width + 1;					    \
}                                                                           
                                                                            
#define HIGHLIGHT_RECT_COORDS(svnw, svnentry, ex, ey, ew, eh, x1, y1, x2, y2)         \
{                                                                           \
    GET_HIGHLIGHT_RECT_DATA (svnw, svnentry, ex, ey, ew, eh, x1, y1, x2, y2)\
    x2 += x1;                                                               \
    y2 += y1;                                                               \
}                                                                           


#define HIGHLIGHT_TOP_COORDS(svnw, svnentry, ex, ey, ew, eh, x1, y1, x2, y2)\
{                                                                           \
    x1 = ex + highlight_width/2 + svnw->svn.arc_width;			    \
    y1 = ey + (highlight_width/2);					    \
    x2 = ex + ew - highlight_width/2 - svnw->svn.arc_width;		    \
    y2 = ey + (highlight_width/2);					    \
}                                                                           


#define HIGHLIGHT_BOT_COORDS(svnw, svnentry, ex, ey, ew, eh, x1, y1, x2, y2)\
{                                                                           \
    x1 = ex + highlight_width/2 + svnw->svn.arc_width;			    \
    y1 = ey + eh - (highlight_width/2) + 1;				    \
    x2 = ex + ew - highlight_width/2 - svnw->svn.arc_width;		    \
    y2 = ey + eh - (highlight_width/2);					    \
}                                                                           

                                                                            
#define HIGHLIGHT_LEFT_ARC_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, width, height, start_angle, end_angle) \
{                                                                           \
    x = ex + (highlight_width/2);					    \
    y = ey + (highlight_width/2) - 1;					    \
    width = svnw->svn.arc_width * 2;                                        \
    height = svnentry->height - (highlight_width/2);			    \
    start_angle = -90*64;                                                   \
    end_angle = -180*64;                                                    \
}


#define HIGHLIGHT_RIGHT_ARC_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, width, height, start_angle, end_angle) \
{                                                                           \
    width = svnw->svn.arc_width * 2;                                        \
    height = eh - (highlight_width/2);					    \
    x = ex + ew - (2 * svnw->svn.arc_width) - (highlight_width/2);	    \
    y = ey + (highlight_width/2) - 1;					    \
    start_angle = 90*64;                                                    \
    end_angle = -180*64;                                                    \
}
                                                                            

#define LEFT_ARC_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, width, height, start_angle, end_angle) \
{                                                                           \
    x = ex;			                                            \
    y = ey;					                            \
    width = svnw->svn.arc_width * 2;                                        \
    height = eh;					                    \
    start_angle = -90*64;                                                   \
    end_angle = -180*64;                                                    \
}


#define RIGHT_ARC_DATA(svnw, svnentry, ex, ey, ew, eh, x, y, width, height, start_angle, end_angle) \
{                                                                           \
    width = svnw->svn.arc_width * 2;                                        \
    height = eh;		                                            \
    x = ex + ew - (2 * svnw->svn.arc_width);				    \
    y = ey;					                            \
    start_angle = 90*64;                                                    \
    end_angle = -180*64;                                                    \
}


#define TOP_LINE_COORDS(svnw, svnentry, ex, ey, eh, ew, x1, y1, x2, y2) \
{                                                                           \
    x1 = ex +  svnw->svn.arc_width;					    \
    y1 = ey;								    \
    x2 = ex + ew - svnw->svn.arc_width;					    \
    y2 = ey;								    \
}                                                                           


#define BOT_LINE_COORDS(svnw, svnentry, ex, ey, eh, ew, x1, y1, x2, y2)	    \
{                                                                           \
    x1 = ex +  svnw->svn.arc_width;					    \
    y1 = ey + eh;							    \
    x2 = ex + ew - svnw->svn.arc_width;					    \
    y2 = ey + eh;							    \
}                                                                           
/*#pragma standard*/


/*
**  calculate connection points
*/
#define CONNECTION_DATA(svnw, points, num_points, x1, y1, x2, y2, tl)	    \
{									    \
if (svnw->svn.tree_connections) {					    \
    if (svnw->svn.perpendicular_lines) {				    \
	int xm1, xm2, ym1, ym2;  /* middle points of connection line */	    \
									    \
	if (svnw->svn.tree_style == DXmSvnKoutlineTree) {			    \
	    points[0].x = x1;						    \
	    points[0].y = y1;						    \
	    points[1].x = x1;						    \
	    points[1].y = y2;						    \
	    points[2].x = x2;						    \
	    points[2].y = y2;						    \
	    num_points = 3;						    \
	    }								    \
	else {								    \
	    if (svnw->svn.tree_style == DXmSvnKtopTree) {			    \
		ym1 = y2 - tl;						    \
		ym2 = ym1;						    \
		xm1 = x1;						    \
		xm2 = x2;						    \
		}							    \
	    else { /* for Left tree points are reversed */		    \
		xm1 = x2 - tl;						    \
		xm2 = xm1;						    \
		ym1 = y1;						    \
		ym2 = y2;						    \
		}							    \
									    \
	    points[0].x = x1;						    \
	    points[0].y = y1;						    \
	    points[1].x = xm1;						    \
	    points[1].y = ym1;						    \
	    points[2].x = xm2;						    \
	    points[2].y = ym2;						    \
	    points[3].x = x2;						    \
	    points[3].y = y2;						    \
	    num_points = 4;						    \
	    }								    \
	}								    \
    else {								    \
	points[0].x = x1;						    \
	points[0].y = y1;						    \
	points[1].x = x2;						    \
	points[1].y = y2;						    \
	num_points = 2;							    \
	}								    \
    }									    \
else									    \
    num_points = 0;							    \
}


/* 
** Clear the screen for a redraw operation
*/
#define CLEAR_SCREEN(svnw)									\
{												\
    DXmSvnEntryPtr svnentry;									\
    int i, j;											\
												\
    if (svnw->svn.sub_widgets_used)								\
	for (i = 1; i <= svnw->svn.num_entries; i++) {						\
	    svnentry = StructGetEntryPtr (svnw,i);						\
	    for (j = 0;  j <= svnentry->num_components - 1;  j++) {	                        \
		if (svnentry->entrycompPtr[j].type == DXmSvnKcompWidget)			\
		    HIDE_WIDGET(svnentry->entrycompPtr[j].var.is_widget.readwrite_text);	\
		};										\
	    };											\
												\
    XClearArea (XtDisplay(svnw), XtWindow(svnw->svn.primary_window_widget), 0, 0, XtWidth(svnw), XtHeight(svnw), FALSE);   \
}


/*
**  Buffer macros for lines (segments)
*/
#define BUFF_SEG_INIT(name,size)				    \
struct {							    \
    int bmax;							    \
    int bnum;							    \
    GC bgc;							    \
    Window bw;							    \
    svn_widget bsvnw;						    \
    XSegment bdata[size];					    \
    } name


#define BUFF_SEG_SET_ATTRS(name,svnw,w,gc)		\
{							\
    name.bsvnw = svnw;					\
    name.bw= w;						\
    name.bgc = gc;					\
    name.bnum = 0;					\
    name.bmax = sizeof(name.bdata)/sizeof(XSegment);	\
}    


#define BUFF_SEG_FLUSH(name)				\
{							\
    if (name.bnum != 0)					\
	XDrawSegments(XtDisplay(name.bsvnw),		\
	    name.bw, name.bgc, name.bdata, name.bnum);	\
    name.bnum = 0;					\
}    

#define BUFF_SEG(name,ix1,iy1,ix2,iy2)			\
{							\
    if (name.bnum >= name.bmax)				\
	BUFF_SEG_FLUSH(name);				\
							\
    name.bdata[name.bnum].x1 = ix1;			\
    name.bdata[name.bnum].y1 = iy1;			\
    name.bdata[name.bnum].x2 = ix2;			\
    name.bdata[name.bnum++].y2 = iy2;			\
}



/*
**  Buffer Macros for Rectangles
*/
#define BUFF_RECT_INIT(name,size)				    \
struct {							    \
    int bnum;							    \
    int bmax;							    \
    int bfill;							    \
    GC bgc;				    			    \
    Window bw;							    \
    svn_widget bsvnw;						    \
    XRectangle bdata[size];					    \
    } name


#define BUFF_RECT_SET_ATTRS(name,svnw,w,gc,fill)	\
{							\
    name.bsvnw = svnw;					\
    name.bw= w;						\
    name.bgc = gc;					\
    name.bnum = 0;					\
    name.bfill = fill;					\
    name.bmax = sizeof(name.bdata)/sizeof(XRectangle);	\
}    


#define BUFF_RECT_FLUSH(name)				\
{							\
    if (name.bnum != 0) {				\
      if (name.bfill) { 				\
	XFillRectangles(XtDisplay(name.bsvnw),		\
	    name.bw, name.bgc, name.bdata, name.bnum);	\
	}						\
      else {						\
	XDrawRectangles(XtDisplay(name.bsvnw),		\
	    name.bw, name.bgc, name.bdata, name.bnum);	\
	}						\
      }							\
							\
    name.bnum = 0;					\
}    


#define BUFF_RECT(name,x1,y1,w,h)			\
{							\
    if (name.bnum >= name.bmax)				\
	BUFF_RECT_FLUSH(name);				\
							\
    name.bdata[name.bnum].x = x1;			\
    name.bdata[name.bnum].y = y1;			\
    name.bdata[name.bnum].width = w;			\
    name.bdata[name.bnum++].height = h;			\
}



/*
**  Buffer macros for Arcs
*/
#define BUFF_ARC_INIT(name,size)				    \
struct {							    \
    int bnum;							    \
    int bmax;							    \
    int bfill;							    \
    GC bgc;							    \
    Window bw;							    \
    svn_widget bsvnw;						    \
    XArc bdata[size];						    \
    } name


#define BUFF_ARC_SET_ATTRS(name,svnw,w,gc,fill)		\
{							\
    name.bsvnw = svnw;					\
    name.bw= w;						\
    name.bgc = gc;					\
    name.bnum = 0;					\
    name.bfill = fill;					\
    name.bmax = sizeof(name.bdata)/sizeof(XArc);	\
}    


#define BUFF_ARC_FLUSH(name)				\
{							\
    if (name.bnum != 0) {				\
      if (name.bfill) {					\
	XFillArcs(XtDisplay(name.bsvnw),		\
	    name.bw, name.bgc, name.bdata, name.bnum);	\
        }						\
      else {						\
	XDrawArcs(XtDisplay(name.bsvnw),		\
	    name.bw, name.bgc, name.bdata, name.bnum);	\
	}						\
      }							\
							\
    name.bnum = 0;					\
}    


/*
#define BUFF_ARC_FLUSH(name)				\
{							\
    int i;						\
    if (name.bnum != 0) {				\
      if (name.bfill) {					\
	for (i = 0; i < name.bnum; i++)			\
	    XFillArc(XtDisplay(name.bsvnw),		\
		name.bw, name.bgc,			\
		name.bdata[i].x,			\
		name.bdata[i].y,			\
		name.bdata[i].width,			\
		name.bdata[i].height,			\
		name.bdata[i].angle1,			\
		name.bdata[i].angle2);			\
        }						\
      else {						\
	for (i = 0; i < name.bnum; i++)			\
	    XDrawArc(XtDisplay(name.bsvnw),		\
		name.bw, name.bgc,			\
		name.bdata[i].x,			\
		name.bdata[i].y,			\
		name.bdata[i].width,			\
		name.bdata[i].height,			\
		name.bdata[i].angle1,			\
		name.bdata[i].angle2);			\
	}						\
      }							\
							\
    name.bnum = 0;					\
}    
*/

#define BUFF_ARC(name,x1,y1,w,h,a1,a2)			\
{							\
    if (name.bnum >= name.bmax)				\
	BUFF_ARC_FLUSH(name);				\
							\
    name.bdata[name.bnum].x = x1;			\
    name.bdata[name.bnum].y = y1;			\
    name.bdata[name.bnum].width = w;			\
    name.bdata[name.bnum].height = h;			\
    name.bdata[name.bnum].angle1 = a1;			\
    name.bdata[name.bnum++].angle2 = a2;		\
							\
}


/******************************************************************************/
/*                                                                            */
/* Macro that will figure out the distance between two points.. window points */
/*                                                                            */
/******************************************************************************/
#define svn_distance(x1, y1, x2, y2)\
    sqrt(((x1) - (x2)) * ((x1) - (x2)) + ((y1) - (y2)) * ((y1) - (y2)))


/*
**  Routines in SVN
*/
    extern Widget DXmSvnWindow ();


/*
**  Routines in display dispatch module and in service modules
*/

    extern void DXmSvnMapPosition ();
    extern void OutlineMapPosition ();
    extern void TopTreeMapPosition ();

    extern int  DXmSvnPositionDisplay ();
    extern int  OutlinePositionDisplay ();
    extern int  TopTreePositionDisplay ();

    extern void DisplayCreateGhost ();
    extern void OutlineCreateGhost ();
    extern void TopTreeCreateGhost ();

    extern void DisplayDeleteGhost ();
    extern void OutlineDeleteGhost ();
    extern void TopTreeDeleteGhost ();

    extern void DisplaySvnInvalidateEntry ();
    extern void OutlineInvalidateEntry    ();
    extern void TopTreeInvalidateEntry    ();

    extern void DisplaySvnSetEntrySensitivity ();
    extern void OutlineSetEntrySensitivity    ();
    extern void TopTreeSetEntrySensitivity    ();

    extern void DisplaySvnAddEntries ();
    extern void OutlineAddEntries    ();
    extern void TopTreeAddEntries    ();

    extern void DisplaySvnDeleteEntries ();
    extern void OutlineDeleteEntries    ();
    extern void TopTreeDeleteEntries    ();

    extern void DisplaySvnEnableDisplay ();
    extern void OutlineEnableDisplay    ();
    extern void TopTreeEnableDisplay    ();

    extern void DisplayHighlightEntry ();
    extern void OutlineHighlightEntry ();
    extern void TopTreeHighlightEntry ();

    extern void DXmSvnAutoScrollDisplay ();
    extern void OutlineAutoScrollDisplay ();
    extern void TopTreeAutoScrollDisplay ();

    extern int DXmSvnAutoScrollCheck ();
    extern int OutlineAutoScrollCheck ();
    extern int TopTreeAutoScrollCheck ();

    extern void DisplayDraw ();
    extern void OutlineDraw ();
    extern void TopTreeDraw ();

    extern void DisplayAdjustHeight ();
    extern void OutlineAdjustHeight ();
    extern void TopTreeAdjustHeight ();

    extern void DisplayResize ();
    extern void OutlineResize ();
    extern void TopTreeResize ();

    extern void DisplaySetApplDragging ();
    extern void OutlineSetApplDragging ();
    extern void TopTreeSetApplDragging ();

    extern int  DisplayChangeMode ();

    extern void DisplayCreate ();
    extern void DisplayDrawEntry ();
    extern void DisplayDrawColumns ();
    extern void DisplayDrawColumnsBox ();
    extern void DisplayGraphicsExpose ();

    extern void	OutlineDrawExposed ();
    extern void	TopTreeDrawExposed ();

    extern void DisplayNavButton ();
    extern void TopTreeNavButton ();

    extern void DisplayAdjustEntrySize ();
    extern void OutlineAdjustEntrySize ();
    extern void TopTreeAdjustEntrySize ();

    extern void DisplaySetRangeHook ();
    extern void OutlineSetRangeHook ();
    extern void TopTreeSetRangeHook ();

    extern int  OutlineGetNumDisplayed ();
    extern void OutlineGetDisplayed ();

    extern void DisplayGetRangeEntries ();
    extern void OutlineGetRangeEntries ();
    extern void TopTreeGetRangeEntries ();

    extern void OutlineFlushEntry ();

    extern void OutlineScrollHelp ();

    extern void OutlineVScroll ();
    extern void TopTreeVScroll ();

    extern void OutlineHScroll ();
    extern void TopTreeHScroll ();

    extern void OutlineScrollButtonDown ();
    extern void OutlineScrollButtonUp ();

    extern void TopTreeScrollButton ();
    extern XFontStruct * DisplayFont ();
    extern void DisplayAdjustEntryHeight();
    extern void DisplayCompData ();

    extern void TopTreeSetCurrentEntry ();
    extern void DisplayExpose ();
    extern void DisplayMergeClips ();
#ifdef _NO_PROTO
    extern void DisplaySetWatch ();
#else
    extern void DisplaySetWatch (svn_widget svnw, Boolean flag);
#endif
    extern void DisplayWorkAreaFixup ();


/*
**  Svn distributed functionality defined in SVN_SELECTION.C
*/
    extern void SelectToggleSelections  ();
    extern void SelectClearSelections   ();
    extern void SelectSelectSet         ();
    extern void SelectClearHighlighting ();
    extern void SelectReportTransitions ();


/*
**  Svn distributed functionality defined in SVN_STRUCTURE.C
*/
    extern DXmSvnEntryPtr StructGetValidEntryPtr   ();
    extern DXmSvnEntryPtr StructGetEntryPtr        ();
    extern void           StructSvnAddEntries      ();
    extern void           StructSvnDeleteEntries   ();
    extern svn_widget     StructFindSvnWidget      ();
    extern void           StructSvnInvalidateEntry ();
    extern void           StructOpenArray          ();
    extern void           StructCloseArray         ();
    extern void           StructTranslateCoords    ();

/*
**  SVN debugging diagnostics
*/
#define SVN_ILLENTNUM    "%SVN-F-ILLENTNUM, Invalid entry number"
#define SVN_NOSUCHENTNUM "%SVN-F-NOSUCHENTRY, The entry number specified does not exist"
#define SVN_DELTOOMANY   "%SVN-F-DELTOOMANY, Attempt to delete a nonexistant entry number"
#define SVN_NOTDIS       "%SVN-W-NOTDIS, SVN should be disabled for this call"
#define SVN_NOTINVAL     "%SVN-W-NOTINVAL, This call should be made only on invalid entry numbers"
#define SVN_TOOENABLED   "%SVN-W-TOOENABLED, More calls to enable then disable"
#define SVN_INSERTHOLE   "%SVN-W-INSERTHOLE, Component number too large in InsertComponent call"
#define SVN_REMOVEMISS   "%SVN-W-REMOVEMISS, Component number not found in RemoveComponent call"

#if defined(VMS) || defined (__VMS)
#include <X11/apienvrst.h>
#endif
