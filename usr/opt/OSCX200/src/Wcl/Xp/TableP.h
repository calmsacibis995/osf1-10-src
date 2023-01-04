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
#ifndef _XpTableP_h
#define _XpTableP_h
#include <X11/Xp/COPY>

/*
 * SCCS_data:    @(#) TableP.h	1.5 92/06/10 06:14:44
 *
 * XpTable -	Forms-based composite widget/geometry manager.
 *		Class heirarchy:
 *			Core
 *			Composite
 *			XpTable
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

#include <X11/Xp/Table.h>
#include <X11/Shell.h>

/* Method definitions
**====================**
   No new methods.  I can't imagine how anyone is going to sub-class
   THIS Widget, as the coupling between the behavior and the instance
   members is rather intense!
*/

/* XpTable Class Part: Information kept in class record
**=======================================================**
*/

typedef struct _XpTableClassPart {
    caddr_t			extension;
} XpTableClassPart;

/* Class hierarchy
**=================**
*/

typedef struct _XpTableClassRec {
    CoreClassPart		core_class;
    CompositeClassPart		composite_class;
    XpTableClassPart		table_class;
} XpTableClassRec;

extern XpTableClassRec xpTableClassRec;


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
typedef struct _XpTableLoc {
    XrmQuark		w_quark;		/* Widget name quark	*/
    Widget		w;			/* actual widget	*/
    int			col, row;		/* Position in table	*/
    int			col_span, row_span;	/* Positions spanned	*/
    int			orig_width, orig_height;/* Childs orig size	*/
    XpTableOpts		options;		/* Child layout options	*/
} XpTableLocRec;				/* 	*XpTableLoc	*/

/* XpTableLoc Methods
**=====================**
*/
extern XpTableLoc XpTableLocNew         _(( int /*num*/ ));
extern XpTableLoc XpTableLocGrow        _(( XpTableLoc ));
extern XpTableLoc XpTableLocCopy        _(( XpTableLoc ));
extern XpTableLoc XpTableLocParse       _(( String /*layout*/ ));
extern XpTableLoc XpTableLocFind        _(( XpTableLoc, Widget /*toFind*/ ));
extern XpTableLoc XpTableLocFindDefault _(( XpTableLoc, Widget /*toFind*/ ));
extern XpTableLoc XpTableLocFindAtPosition _((XpTableLoc,int/*col*/,int/*ro*/));
extern int  XpTableLocLen             _(( XpTableLoc ));
extern int  XpTableLocPreferredWidth  _(( XpTableLoc, int /*shrink*/ ));
extern int  XpTableLocPreferredHeight _(( XpTableLoc, int /*shrink*/ ));
extern int  XpTableLocNumCols         _(( XpTableLoc ));
extern int  XpTableLocNumRows         _(( XpTableLoc ));
extern int  XpTableLocCompareColSpan  _(( XpTableLoc, XpTableLoc ));
extern int  XpTableLocCompareRowSpan  _(( XpTableLoc, XpTableLoc ));
extern void XpTableLocFree _(( XpTableLoc ));

/* Table Vector Structs
**======================**
   A table has two of these vectors: one for columns, and one for rows.
*/
typedef int XpTableVectorOpts;
#define	TBL_VEC_MINIMIZE	0x01
#define	TBL_VEC_LOCK		0x02
#define	TBL_VEC_NOGROW		(TBL_VEC_MINIMIZE | TBL_VEC_LOCK)

typedef struct _XpTableVector {
    XpTableVectorOpts	options;	/* Apply to entire col or row	*/
    int			value;		/* width of col, hieght of row	*/
    int			pref_value;	/* minimum or preferred value	*/
    int			offset;		/* of upper left corner of cell	*/
} XpTableVectorRec, *XpTableVector;

/* XpTableVector Methods
**========================**
*/
#define TABLE XpTableWidget
#define DO_COL (int)1
#define DO_ROW (int)0

extern XpTableVector XpTableVectorNew _(( int, TABLE, int ));
extern void XpTableVectorFree _(( XpTableVector ));
extern void XpTableVectorMinimize _(( XpTableVector, int, TABLE, int));
extern int  XpTableVectorTotalSize _(( XpTableVector, int, TABLE, int));
extern int  XpTableVectorPreferredSize _(( XpTableVector, int, TABLE, int));
extern void XpTableVectorAdjust _(( XpTableVector, int /*len*/, int /*amt*/));
extern void XpTableVectorComputeOffsets _(( XpTableVector, int, int, int ));

#undef TABLE

typedef enum _ResizeStatus { RSinit, RSdone, RSdueToRequest } ResizeStatus;

/* XpTable Part: Information kept in instance record
**====================================================**
*/

typedef struct _XpTablePart {
    /* controlling members, set by SetValues or from resource database
    */
    Boolean		force_shrink;	/* Shrink smaller than pref'd	*/
    Boolean		shrink_simple;	/* Shrink simple widgets	*/
    int			margin_width;	/* to left and right of kids	*/
    int			margin_height;	/* above and below table kids	*/
    int			col_spacing;	/* Space between columns	*/
    int			row_spacing;	/* Space between rows		*/
    XpTableOpts		default_options;/* Default child layout options	*/
    XpTableLoc		default_layout;	/* Layout spec (orig from xrdb)	*/

    /* internally computed members
    */
    XpTableLoc		real_layout;	/* Computed current layout	*/
    int			num_cols;	/* Number of columns		*/
    XpTableVector	cols;		/* Widths and opts of each col	*/
    int			num_rows;	/* Number of rows		*/
    XpTableVector	rows;		/* Heights and opts of each row	*/

    /* State indications
    */
    ResizeStatus	resizeStatus;	/* if Resize method invoked	*/

} XpTablePart;

/* Instance hierarchy
**====================**
*/

typedef struct _XpTableRec {
    CorePart		core;
    CompositePart	composite;
    XpTablePart		table;
} XpTableRec;

/* Geometry Management Support Methods
**=====================================**
*/
extern void XpTableNewLayout _(( XpTableWidget ));
extern void XpTableRecomputeLayout _(( XpTableWidget ));
extern void XpTableResizeLayout _(( XpTableWidget ));

extern void XpTableNewRealLayout _(( XpTableWidget ));
extern void XpTableAppendToDefaultLayout _(( XpTableWidget, XpTableLoc ));
extern void XpTableNewColsAndRows _(( XpTableWidget ));
extern void XpTableMakeColsFitWidth _(( XpTableWidget ));
extern void XpTableMakeRowsFitHeight _(( XpTableWidget ));
extern int  XpTablePreferredWidth _(( XpTableWidget ));
extern int  XpTablePreferredHeight _(( XpTableWidget ));
extern void XpTableRequestResize _(( XpTableWidget ));
extern void XpTableSetGeometryOfChildren _(( XpTableWidget ));

#endif /* _XpTableP_h */
