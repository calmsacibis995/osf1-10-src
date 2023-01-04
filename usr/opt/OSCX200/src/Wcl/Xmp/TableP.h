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
#ifndef _XmpTableP_h
#define _XmpTableP_h
#include <X11/Xmp/COPY>

/*
 * SCCS_data:    @(#) TableP.h	1.7 92/06/10 06:13:59
 *
 * XmpTable -	Forms-based composite widget/geometry manager derived from
 *		Motif manager widgets.  Class heirarchy:
 *			Core
 *			Composite
 *			Constraint
 *			XmManager
 *			XmBulletinBoard
 *			XmpTable
 *
 * Originally implemented by:
 *	David Harrison
 *	University of California, Berkeley
 *	1989
 *
 * Many bug fixes and enhancements provided by
 *	marbru@auto-trol.com	Martin Brunecky
 *	nazgul@alphalpha.com	Kee Hinckley
 *	pastor@PRC.Unisys.COM	Jon A. Pastor
 *
 * Completely re-implemented by:
 *	David.Smyth@SniAP.MchP.SNI.De
 */

#include <X11/Xmp/Table.h>
#include <X11/Xm/XmP.h>
#include <X11/Xm/BulletinBP.h>

/* Method definitions
**====================**
   No new methods.  I can't imagine how anyone is going to sub-class
   THIS Widget, as the coupling between the behavior and the instance
   members is rather intense!
*/

/* XmpTable Class Part: Information kept in class record
**=======================================================**
*/

typedef struct _XmpTableClassPart {
    caddr_t			extension;
} XmpTableClassPart;

/* Class hierarchy
**=================**
*/

typedef struct _XmpTableClassRec {
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    ConstraintClassPart		constraint_class;
    XmManagerClassPart		manager_class;
    XmBulletinBoardClassPart	bulletin_class;
    XmpTableClassPart		table_class;
} XmpTableClassRec;

extern XmpTableClassRec xmpTableClassRec;


/* Private data types
**====================**
   All of these structures are often used as elements of null terminated arrays.
*/

/* Table Location structs.
**========================**
   These are used to describe each widget location, in both the
   default_layout and the real_layout.  In the default_layout, the
   location structs contain a quark, in the real_layout they contain the
   actual widget and the orig* fields are set.
*/
typedef struct _XmpTableLoc {
    XrmQuark		w_quark;		/* Widget name quark	*/
    Widget		w;			/* actual widget	*/
    int			col, row;		/* Position in table	*/
    int			col_span, row_span;	/* Positions spanned	*/
    int			orig_width, orig_height;/* Childs orig size	*/
    XmpTableOpts	options;		/* Child layout options	*/
} XmpTableLocRec;				/* 	*XmpTableLoc	*/

/* XmpTableLoc Methods
**=====================**
*/
extern XmpTableLoc XmpTableLocNew	  _(( int /*num*/ ));
extern XmpTableLoc XmpTableLocGrow	  _(( XmpTableLoc ));
extern XmpTableLoc XmpTableLocCopy	  _(( XmpTableLoc ));
extern XmpTableLoc XmpTableLocParse	  _(( char* /*layout*/ ));
extern XmpTableLoc XmpTableLocFind	  _(( XmpTableLoc, Widget /*toFind*/ ));
extern XmpTableLoc XmpTableLocFindDefault _(( XmpTableLoc, Widget /*toFind*/ ));
extern XmpTableLoc XmpTableLocFindAtPosition _((XmpTableLoc,int/*c*/,int/*r*/));
extern int  XmpTableLocLen		_(( XmpTableLoc ));
extern int  XmpTableLocPreferredWidth	_(( XmpTableLoc, int /*shrink*/ ));
extern int  XmpTableLocPreferredHeight	_(( XmpTableLoc, int /*shrink*/ ));
extern int  XmpTableLocNumCols		_(( XmpTableLoc ));
extern int  XmpTableLocNumRows		_(( XmpTableLoc ));
extern int  XmpTableLocCompareColSpan	_(( XmpTableLoc, XmpTableLoc ));
extern int  XmpTableLocCompareRowSpan	_(( XmpTableLoc, XmpTableLoc ));
extern void XmpTableLocFree _(( XmpTableLoc ));

/* Table Vector Structs
**======================**
   A table has two of these vectors: one for columns, and one for rows.
*/
typedef int XmpTableVectorOpts;
#define	TBL_VEC_MINIMIZE	0x01
#define	TBL_VEC_LOCK		0x02
#define	TBL_VEC_NOGROW		(TBL_VEC_MINIMIZE | TBL_VEC_LOCK)

typedef struct _XmpTableVector {
    XmpTableVectorOpts	options;	/* Apply to entire col or row	*/
    int			value;		/* width of col, hieght of row	*/
    int			pref_value;	/* minimum or preferred value	*/
    int			offset;		/* of upper left corner of cell	*/
} XmpTableVectorRec, *XmpTableVector;

/* XmpTableVector Methods
**========================**
*/
#define TABLE XmpTableWidget
#define DO_COL (int)1
#define DO_ROW (int)0

extern XmpTableVector XmpTableVectorNew _(( int, TABLE, int ));
extern void XmpTableVectorFree _(( XmpTableVector ));
extern void XmpTableVectorMinimize _(( XmpTableVector, int, TABLE, int));
extern int  XmpTableVectorTotalSize _(( XmpTableVector, int, TABLE, int));
extern int  XmpTableVectorPreferredSize _(( XmpTableVector, int, TABLE, int));
extern void XmpTableVectorAdjust _(( XmpTableVector, int /*len*/, int /*amt*/));
extern void XmpTableVectorComputeOffsets _(( XmpTableVector, int, int, int ));

#undef TABLE

typedef enum _ResizeStatus { RSinit, RSdone, RSdueToRequest } ResizeStatus;

/* XmpTable Part: Information kept in instance record
**====================================================**
*/

typedef struct _XmpTablePart {
    /* controlling members, set by SetValues or from resource database
    */
    Boolean		force_shrink;	/* Shrink smaller than pref'd	*/
    Boolean		shrink_simple;	/* Shrink simple widgets	*/
    int			col_spacing;	/* Space between columns	*/
    int			row_spacing;	/* Space between rows		*/
    XmpTableOpts	default_options;/* Default child layout options	*/
    XmpTableLoc		default_layout;	/* Layout spec (orig from xrdb)	*/

    /* internally computed members
    */
    XmpTableLoc		real_layout;	/* Computed current layout	*/
    int			num_cols;	/* Number of columns		*/
    XmpTableVector	cols;		/* Widths and opts of each col	*/
    int			num_rows;	/* Number of rows		*/
    XmpTableVector	rows;		/* Heights and opts of each row	*/

    /* State indications
    */
    ResizeStatus	resizeStatus;	/* if Resize method invoked	*/

} XmpTablePart;

/* Instance hierarchy
**====================**
*/

typedef struct _XmpTableRec {
    CorePart		core;
    CompositePart	composite;
    ConstraintPart	constraint;
    XmManagerPart	manager;
    XmBulletinBoardPart	bulletin_board;
    XmpTablePart	table;
} XmpTableRec;

/* Geometry Management Support Methods
**=====================================**
*/
extern void XmpTableNewLayout _(( XmpTableWidget ));
extern void XmpTableRecomputeLayout _(( XmpTableWidget ));
extern void XmpTableResizeLayout _(( XmpTableWidget ));

extern void XmpTableNewRealLayout _(( XmpTableWidget ));
extern void XmpTableAppendToDefaultLayout _(( XmpTableWidget, XmpTableLoc ));
extern void XmpTableNewColsAndRows _(( XmpTableWidget ));
extern void XmpTableMakeColsFitWidth _(( XmpTableWidget ));
extern void XmpTableMakeRowsFitHeight _(( XmpTableWidget ));
extern int  XmpTablePreferredWidth _(( XmpTableWidget ));
extern int  XmpTablePreferredHeight _(( XmpTableWidget ));
extern void XmpTableRequestResize _(( XmpTableWidget ));
extern void XmpTableSetGeometryOfChildren _(( XmpTableWidget ));

#endif /* _XmpTableP_h */
