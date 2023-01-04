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
/*LINTLIBRARY*/
/*
 * SCCS_data:    @(#) Table.c	1.6 92/06/10 06:14:42
 *
 * XpTable -	Forms-based composite widget/geometry manager
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
 * Completely re-implemented by:
 *	David.Smyth@SniAp.MchP.SNI.De
 */

/*
Edit History

01Feb92		david	Re-Implementation

*/

#include <X11/Xp/COPY>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <X11/Xp/TableP.h>

/* For backward compatibility with old Xt releases
**=================================================**
*/
#ifndef XtIsWidget
#ifdef XtSpecificationRelease
#define XtIsWidget(obj) XtIsSubclass(obj,(WidgetClass)coreWidgetClass)
#else
#define XtIsWidget(obj) XtIsSubclass(obj,(WidgetClass)widgetClass)
#endif
#endif

#ifndef XtSpecificationRelease
#if NeedFunctionPrototypes
typedef void*   XtPointer;
#else
typedef char*   XtPointer;
#endif
#endif


/* Resources
**===========**
*/
#ifdef XtOffsetOf
#define OFFSET(field) XtOffsetOf(XpTableRec,table.field)
#else
define OFFSET(field) XtOffset(XpTableWidget,table.field)
#endif

static XtResource resources[] = {
 { XtNdefaultOptions, XtCDefaultOptions, XtRXpTableOpts, sizeof(XpTableOpts),
   OFFSET(default_options), XtRImmediate, (XtPointer)0 },
 { XtNlayout, XtCLayout, XtRXpTableLoc, sizeof(XpTableLoc),
   OFFSET(default_layout), XtRXpTableLoc, (XtPointer)0 },
 { XtNforceShrink, XtCForceShrink, XtRBoolean, sizeof(Boolean),
   OFFSET(force_shrink), XtRImmediate, (XtPointer)True },
 { XtNshrinkSimple, XtCShrinkSimple, XtRBoolean, sizeof(Boolean),
   OFFSET(shrink_simple), XtRImmediate, (XtPointer)True },
 { XtNmarginWidth, XtCMargins, XtRInt, sizeof(int),
   OFFSET(margin_width), XtRImmediate, (XtPointer)0 },
 { XtNmarginHeight, XtCMargins, XtRInt, sizeof(int),
   OFFSET(margin_height), XtRImmediate, (XtPointer)0 },
 { XtNcolumnSpacing, XtCSpacing, XtRInt, sizeof(int),
   OFFSET(col_spacing), XtRImmediate, (XtPointer)0 },
 { XtNrowSpacing, XtCSpacing, XtRInt, sizeof(int),
   OFFSET(row_spacing), XtRImmediate, (XtPointer)0 },
};

#undef OFFSET

/* Core Class Methods
**====================**
*/
static void		XpTableClassInitialize ();
static void		XpTableInitialize _((	Widget, Widget,
						ArgList, Cardinal*	));
static void		XpTableDestroy _((	Widget			));
static void		XpTableResize _((	Widget			));
#ifdef XtSpecificationRelease
static Boolean		XpTableSetValues _((	Widget, Widget, Widget,
						ArgList, Cardinal*	));
#else
static Boolean		XpTableSetValues _((	Widget, Widget, Widget	));
#endif
static XtGeometryResult	XpTableQueryGeometry _(( Widget,
						XtWidgetGeometry*,
						XtWidgetGeometry*	));

/* Composite class methods
**=========================**
*/
static XtGeometryResult	XpTableGeometryManager _(( Widget /*child*/,
						XtWidgetGeometry*,
						XtWidgetGeometry*	));
static void		XpTableChangeManaged _(( Widget		));



XpTableClassRec xpTableClassRec = {
  { /* core_class fields		*/
    /* superclass			*/	(WidgetClass)&compositeClassRec,
    /* class_name			*/	"XpTable",
    /* widget_size			*/	sizeof(XpTableRec),
    /* class_initialize			*/	XpTableClassInitialize,
    /* class_part_initialize		*/	NULL,
    /* class_inited			*/	FALSE,
    /* initialize			*/	XpTableInitialize,
    /* initialize_hook			*/	NULL,
    /* realize				*/	XtInheritRealize,
    /* actions				*/	NULL,
    /* num_actions			*/	0,
    /* resources			*/	resources,
    /* num_resources			*/	XtNumber(resources),
    /* xrm_class			*/	NULLQUARK,
    /* compress_motion			*/	False,
    /* compress_exposure		*/	TRUE,
    /* compress_enterleave		*/	False,
    /* visible_interest			*/	FALSE,
    /* destroy				*/	XpTableDestroy,
    /* resize				*/	XpTableResize,
    /* expose				*/	XtInheritExpose,
    /* set_values			*/	XpTableSetValues,
    /* set_values_hook			*/	NULL,
    /* set_values_almost		*/	XtInheritSetValuesAlmost,
    /* get_values_hook			*/	NULL,
    /* accept_focus			*/	NULL,
    /* version				*/	XtVersion,
    /* callback_private			*/	NULL,
    /* tm_table				*/	XtInheritTranslations,
    /* query_geometry			*/	XpTableQueryGeometry,
    /* display_accelerator		*/	NULL,
    /* extension			*/	NULL
  },
  { /* composite_class fields		*/
    /* geometry_manager			*/	XpTableGeometryManager,
    /* change_managed			*/	XpTableChangeManaged,
    /* insert_child			*/	XtInheritInsertChild,
    /* delete_child			*/	XtInheritDeleteChild,
    /* extension			*/	NULL
  },
  { /* table_class fields		*/
    /* extension			*/	NULL
  }
};

WidgetClass xpTableWidgetClass = (WidgetClass) &xpTableClassRec;


/* Converters
**============**
*/

/*ARGSUSED*/
static void XpCvtStrToXpTableOpts(args, num_args, from, to)
XrmValue *args;			/* Arguments to converter */
Cardinal *num_args;		/* Number of arguments    */
XrmValue *from;			/* From type              */
XrmValue *to;			/* To type                */
/*
 * Converts a string representation into a XpTableOpts, which is
 * which is small enough to fit entirely into the to->addr.
 */
{
    static XpTableOpts opts;

    if (*num_args != 0) {
	XtErrorMsg("XpCvtStrToXpTableOpts", "wrongParameters",
		   "XtToolkitError",
		   "String to options takes no additional arguments",
		   (String *) NULL, (Cardinal *) NULL);
    }

    opts = XpTableOptsParse( (String) from->addr );

    if (opts == NULL)
	XtStringConversionWarning( (String) from->addr, XtRXpTableOpts );

    to->addr = (caddr_t) &opts;
    to->size = sizeof( XpTableOpts );
}

/*ARGSUSED*/
static void XpCvtStrToXpTableLoc(args, num_args, from, to)
XrmValue *args;			/* Arguments to converter */
Cardinal *num_args;		/* Number of arguments    */
XrmValue *from;			/* From type              */
XrmValue *to;			/* To type                */
/*
 * Converts a string representation into an array of TableLocRec
 * structures (ie., TableLocRec*).  This XtCalloc'd array is kept
 * by the resource database: a COPY must be made by the widget.
 */
{
    static XpTableLoc defLocs;

    if (*num_args != 0) {
	XtErrorMsg("XpCvtStrToXpTableLoc", "wrongParameters",
		   "XtToolkitError",
		   "String to layout takes no additional arguments",
		   (String *) NULL, (Cardinal *) NULL);
    }

    defLocs = XpTableLocParse( (String) from->addr );

    if (defLocs == NULL)
	XtStringConversionWarning( (String) from->addr, XtRXpTableLoc );

    to->addr = (caddr_t) &defLocs;
    to->size = sizeof(caddr_t);
}


/* Initialization Methods
**========================**
   Note that no class part initialization is needed, as there are
   no inherited methods (yet?).
*/

static void XpTableClassInitialize()
{
    XtAddConverter(	XtRString, XtRXpTableOpts,
			XpCvtStrToXpTableOpts, NULL, 0 );
    XtAddConverter(	XtRString, XtRXpTableLoc,
			XpCvtStrToXpTableLoc, NULL, 0 );
}

/*ARGSUSED*/
static void XpTableInitialize( requestWidget, newWidget, args, num_args )
    Widget	requestWidget;		/* as already set by Xt		*/
    Widget	newWidget;		/* set up by this method	*/
    ArgList	args;
    Cardinal*	num_args;
{
    XpTableWidget tw = (XpTableWidget) newWidget;

    /* Copy values specified by pointer
    */
    tw->table.default_layout = XpTableLocCopy(tw->table.default_layout);

    tw->table.real_layout = (XpTableLoc)0;
    tw->table.num_cols = tw->table.num_rows = 0;
    tw->table.cols = tw->table.rows = (XpTableVector)0;
    tw->table.resizeStatus = RSinit;
}
 


/* Destroy Method
**================**
   Free any instance data allocated for the XpTablePart members.  
*/
static void XpTableDestroy( w )
    Widget w;			/* Widget to destroy */
{
    XpTableWidget tw = (XpTableWidget) w;

    XpTableLocFree( tw->table.default_layout );
    XpTableLocFree( tw->table.real_layout );
    XpTableVectorFree( tw->table.cols );
    XpTableVectorFree( tw->table.rows );
}



/* Geometry Management and Negotiation
**=====================================**
   The following methods are involved in geometry management and
   negotiation:

   XpTableResize() method:  The parent can issue a resize demand by
   invoking XpTableResize() via XtResizeWidget().  This can be due to the
   parent being resized from above (perhaps the user changed the shell
   size via the window manager) or it may be due to the Table itself
   asking to be made a different size for any of several reasons.

   XpTableSetValues() method:  Geometry management gets involved when a
   change occurs to any Table resource, or any of several superclass
   resources (width, height, margin_width).

   XpTableQueryGeometry() method:  A parent of a Table asks the table for
   its preferred geometry using this method, invoked via
   XtQueryGeometry().

   XpTableGeometryManager() method:  This method is invoked when a child
   wishes to become a different size.

   XpTableChangeManaged() method: this method is invoked when the set of
   managed children changes.  This triggers the initial layout of the
   Table widget, and causes the existing layout to be re-computed.
*/

/* Recompute Layout Due To Parent Demand
**=======================================**
   This gets called when a parent tells the table it must re-size.  We
   certainly have real_layout, rows, cols, already done (or null if no
   managed children), but they must be adjusted to fit within the XpTable
   widgets size as specified in tw->core.width, tw->core.height.

   Cause the background of the Table widget to be re-displayed if there
   is an expose proc.
*/

static void XpTableResize( w )
    Widget w;
{
    XpTableResizeLayout( (XpTableWidget)w );
    if ( XtIsRealized( w ) && (XtExposeProc)0 != XtClass(w)->core_class.expose )
    {
	XClearWindow( XtDisplay( w ), XtWindow( w ) );
	XtClass(w)->core_class.expose( w, (XEvent*)0, (Region)0 );
    }
}



/* Set Values
**============**
   This method gets called via XtSetVales().  If any table members or
   margin width have been changed, then geometry management must be done.

   We do not have to worry about geometry changes (core.width etc) because
   Xt takes care of invoking XpTableResize directly.
*/

#ifdef XtSpecificationRelease
/*ARGSUSED*/
static Boolean XpTableSetValues(currentWidget, ignoreRequestWidget, newWidget,
				 ignored, notUsed )
Widget currentWidget;			/* Before call to XtSetValues */
Widget ignoreRequestWidget;		/* After call to XtSetValues  */
Widget newWidget;			/* Final version of widget    */
ArgList ignored;
Cardinal* notUsed;
#else
static Boolean XpTableSetValues(currentWidget, ignoreRequestWidget, newWidget)
Widget currentWidget;			/* Before call to XtSetValues */
Widget ignoreRequestWidget;		/* After call to XtSetValues  */
Widget newWidget;			/* Final version of widget    */
#endif
{
    XpTableWidget current = (XpTableWidget) currentWidget;
    XpTableWidget new     = (XpTableWidget) newWidget;

    if ( current->table.force_shrink	!= new->table.force_shrink
      || current->table.shrink_simple	!= new->table.shrink_simple
      || current->table.col_spacing	!= new->table.col_spacing
      || current->table.row_spacing	!= new->table.row_spacing
      || current->table.default_options	!= new->table.default_options
      || current->table.margin_width	!= new->table.margin_width
      || current->table.margin_height	!= new->table.margin_height)
    {
	/* We need to do some re-layout
	*/
	if ( current->table.default_layout != new->table.default_layout )
	{
	    /* values set by pointers require special handling:
	     * free old value, copy and alloc new value.
	     */
	    XpTableLocFree( current->table.default_layout );
	    new->table.default_layout = XpTableLocCopy(
						new->table.default_layout );
	    /* We need to do a complete recomputation and placement
	    */
	    XpTableNewLayout( new );
	}
	else
	{
	    /* we only need to change some things in the existing real_layout
	    */
	    XpTableRecomputeLayout( new );
	}
	/* Let Xt know exposure is needed
	*/
	return True;
    }
    /* No exposure needed due to Table resources
    */
    return False;
}



/* Provide Preferred Geometry To Parent
**======================================**
   If the parent asks if the table can grow, the answer is always yes.  If
   the parent asks for the table to shrink to a size smaller than the
   preferred width or height, then the answer is almost, with the preferred
   width and height provided.
*/
static XtGeometryResult XpTableQueryGeometry(w, request, geo_return)
    Widget w;				/* XpTable widget	*/
    XtWidgetGeometry *request;		/* Parent intended size	*/
    XtWidgetGeometry *geo_return;	/* preferred size	*/
{
    XpTableWidget tw = (XpTableWidget) w;
    int pref;

    /* First check for queries which would not result in a resize.
    ** According to the spec, "No" means "Don't bother."
    */
    if (  request->request_mode & CWWidth
     && !(request->request_mode & CWHeight)
     &&   request->width == tw->core.width )
	return XtGeometryNo;
    if (  request->request_mode & CWHeight
     && !(request->request_mode & CWWidth)
     &&   request->height == tw->core.height )
	return XtGeometryNo;
    if (  request->request_mode & CWWidth
     &&   request->request_mode & CWHeight
     &&   request->width  == tw->core.width
     &&   request->height == tw->core.height )
	return XtGeometryNo;

    if ( request->request_mode == (XtGeometryMask)0 )
    {
	/* Parent is asking for preferred size:
	** XtGeometryNo means already at preferred (minimum) size
	** XtGeometryAlmost means has a preferred size different from current
	** XtGeometryYes is never returned, as Table *does* have preferred size 
	*/
	geo_return->request_mode = CWWidth|CWHeight;
	geo_return->width  = XpTablePreferredWidth(  tw );
	geo_return->height = XpTablePreferredHeight( tw );
	if ( geo_return->width  == tw->core.width
	  && geo_return->height == tw->core.height )
	    return XtGeometryNo;
	else
	    return XtGeometryAlmost;
    }
    if ( request->request_mode & CWWidth )
    {
	pref = XpTablePreferredWidth( tw );
	if ( request->width < (Dimension)pref )
	{
	    geo_return->width = (Dimension)pref;
	    geo_return->request_mode |= CWWidth;
	}
	else
	{
	    geo_return->width = request->width;
	}
    }
    if ( request->request_mode & CWHeight )
    {
	pref = XpTablePreferredHeight( tw );
	if ( request->height < (Dimension)pref )
	{
	    geo_return->height = (Dimension)pref;
	    geo_return->request_mode |= CWHeight;
	}
	else
	{
	    geo_return->height = request->height;
	}
    }

    /* Return No if no resize required, Almost if one or more pref is
    ** provided (i.e, one request smaller than a pref), Yes otherwise.
    */

    if ( geo_return->width  == tw->core.width
     &&  geo_return->height == tw->core.height )
	return XtGeometryNo;

    if ( geo_return->request_mode & (CWWidth|CWHeight) )
	return XtGeometryAlmost;

    return XtGeometryYes;
}



/* Handle Geometry Requests from Children
**========================================**
   Position changes are always rejected.  Size changes are accepted in the
   following circumstances:  A request to become smaller is always OK, a
   request to become larger is OK unless the col/row is locked and the
   requested size is greater than the locked (current) col/row size.

   Accepted changes cause a re-layout, which eventually results in moving
   children about.
*/

static XtGeometryResult XpTableGeometryManager( child, request, reply )
    Widget child;		/* Widget                    */
    XtWidgetGeometry *request;	/* Requested geometry change */
    XtWidgetGeometry *reply;	/* Actual reply to request   */
{
    Widget		parent	= child->core.parent;
    XpTableWidget	tw	= (XpTableWidget) parent;
    Dimension		width, height, border_width;
    XtGeometryResult	result;
    XpTableLoc		loc;

    if ( !parent || !XtIsSubclass( parent, xpTableWidgetClass ) )
	XtErrorMsg("XpTableGeometryManager", "badParent", "XtToolkitError",
		   "Parent of widget is not an XpTableWidget",
		   (String *) NULL, (Cardinal *) NULL);

    /* First cut at result: size change OK, others not.
    */
    if (request->request_mode & (CWWidth|CWHeight|CWBorderWidth))
    {
	if (request->request_mode & (CWX|CWY|CWSibling|CWStackMode))
	    result = XtGeometryAlmost;
	else
	    result = XtGeometryYes;
    }
    else
	return XtGeometryNo;

    /* Always need at least this much:
    */
    if ( request->request_mode & CWWidth )
	width = request->width;
    else
	width = child->core.width;

    if ( request->request_mode & CWHeight )
	height = request->height;
    else
	height = child->core.height;

    if ( request->request_mode & CWBorderWidth )
	border_width = request->border_width;
    else
	border_width = child->core.border_width;

    /* Life is simple if the child wants to become smaller:
    */
    if ( width <= child->core.width
     &&  height <= child->core.height
     &&  border_width <= child->core.border_width )
    {
	reply->request_mode = (CWWidth|CWHeight|CWBorderWidth);
	reply->width = width;
	reply->height = height;
	reply->border_width = border_width;

	if ( result == XtGeometryAlmost )
	{
	    /* just respond as a query, do NOT change
	    */
	    return XtGeometryAlmost;
	}
	/* resize child, then do re-layout
	*/
	XtResizeWidget( child, width, height, border_width );
	XpTableRecomputeLayout( tw );
	return XtGeometryDone;
    }

    /* Child wants to become bigger, this may or may not be OK.
    */
    loc = XpTableLocFind( tw->table.real_layout, child );

    if ( NULL == loc )
	XtErrorMsg("XpTableGeometryManager", "bogusChild", "XtToolkitError",
		   "Could not find managed child in real_layout",
		   (String *) NULL, (Cardinal *) NULL);

    if ( width > child->core.width
     && (tw->table.cols[ loc->col ].options & TBL_VEC_MINIMIZE)
     &&  tw->table.cols[ loc->col ].value < (int)(width + 2*border_width) )
    {
	result = XtGeometryAlmost;
	reply->request_mode |= CWWidth;
	reply->width = tw->table.cols[ loc->col ].value - 2*border_width;
    }
    if ( height > child->core.height
     && (tw->table.rows[ loc->row ].options & TBL_VEC_MINIMIZE) 
     &&  tw->table.rows[ loc->row ].value < (int)(height + 2*border_width) )
    {
	result = XtGeometryAlmost;
	reply->request_mode |= CWHeight;
	reply->height = tw->table.cols[ loc->col ].value - 2*border_width;
    }
    if ( border_width > child->core.border_width 
     &&(( (tw->table.cols[ loc->col ].options & TBL_VEC_MINIMIZE) 
       &&  tw->table.cols[ loc->col ].value < (int)(width + 2*border_width) )
      ||( (tw->table.rows[ loc->row ].options & TBL_VEC_MINIMIZE)
       &&  tw->table.rows[ loc->row ].value < (int)(height + 2*border_width) )))
    {
	result = XtGeometryAlmost;
	reply->request_mode |= CWBorderWidth;
	reply->border_width = child->core.border_width;
    }
    if ( result == XtGeometryAlmost )
	return result;

    /* Looks OK: resize child, then do re-layout
    */
    XtResizeWidget( child, width, height, border_width );
    XpTableRecomputeLayout( tw );
    return XtGeometryDone;
}

/* Handle Increase or Decrease in Managed Children
**=================================================**
   Called when a child or when children are managed or unmanaged via
   XtManageChild(), XtUnmanageChild() etc.
*/
static void XpTableChangeManaged( w )
    Widget w;
{
    XpTableNewLayout( (XpTableWidget)w );
}

/*===========================**
** End Of Xt Invoked Methods **
**===========================*/

/* Internal Table Methods
**========================**
   There are 3 ways the Table may need to be recomputed:
 1 Resize Method: Command to change to specific size from above.
 2 ChangedManage and SetVales: Compute new layout, ask parent for new size.
 3 GeometryRequest from child: Recompute layout, ask parent for new size.
*/

void XpTableNewLayout( tw )
    XpTableWidget tw;
{
    XpTableNewRealLayout(	tw );
    XpTableNewColsAndRows(	tw );
    XpTableRequestResize(	tw );
}

void XpTableRecomputeLayout( tw )
    XpTableWidget	tw;
{
    XpTableNewColsAndRows(	tw );
    XpTableRequestResize(	tw );
}

/* Called due to XpTableRequestResize()
*/
void XpTableResizeLayout( tw )
    XpTableWidget tw;
{
    XpTableMakeColsFitWidth(	   tw );
    XpTableMakeRowsFitHeight(	   tw );
    XpTableSetGeometryOfChildren( tw );
    tw->table.resizeStatus = RSdone;
}


/* Build a new real_layout for all managed children.
**==================================================**
   Each location is a function of the real_layout, default_layout, 
   default_options, and automatic positioning.

   The list of managed children in traversed: if a managed child already
   appears in the current real_layout, that layout is copied.  Otherwise,
   location data comes from default_layout, default_options, and automatic
   positioning members of the parent table widget.

   Since both the default_layout and real_layout are changed by 
   XpTablePosition(), XpTableResize(), XpTableOptions(), and
   XpTableConfig(), changes to the children remain in effect when the table
   layout is re-computed, and when children become managed and unmanaged
   multiple times.  However, if the default_layout is changed by a
   XtSetValues, all the positioning stuff in default_layout is lost.
   OK, since both are done by the client program: if the programmer
   wants to change the layout, the programmer will also need to reposition
   children.
*/

void XpTableNewRealLayout( tw )
    XpTableWidget tw;
{
    int		num_children	= tw->composite.num_children;
    WidgetList	children	= tw->composite.children;
    XpTableLoc	result		= XpTableLocNew( num_children );
    XpTableLoc	loc		= result;
    XpTableLoc	found;

    int		child	= 0;	/* index into list of all children */

    for ( ;  child < num_children  ;  child++ )
    {
	Widget w = children[child];

	if ( XtIsManaged( w ) )
	{
	    if ( found = XpTableLocFind( tw->table.real_layout, w ) )
	    {
		/* This widget was in previous layout, copy all fields
		 */
		*loc = *found;
	    }
	    else if ( found = XpTableLocFind( tw->table.default_layout, w ) )
	    {
		/* This child has been laid out before, so copy everything.
		 */
		*loc = *found;
	    }
	    else if ( found = XpTableLocFindDefault( tw->table.default_layout,
							w ) )
	    {
		/* Never laid out this child, but default layout provides
		 * some information.  Copy everything, fill in the blanks
		 * (col,row,col_span,row_span already have defaults).
		 * No problem if tw->table.default_options is zero.
		 */
		*loc = *found;
		loc->w = w;
		loc->orig_width  = w->core.width  + 2*w->core.border_width;
		loc->orig_height = w->core.height + 2*w->core.border_width;
		if ( !loc->options )
		    loc->options = tw->table.default_options;

		XpTableAppendToDefaultLayout( tw, loc );
	    }
	    else
	    {
		/* Never laid out this child, not in default layout.  Fill
		 * in everything with default values.
		 */
		loc->w = w;
		loc->w_quark = w->core.xrm_name;
		loc->col = loc->row = 0;
		loc->col_span = loc->row_span = 1;
		loc->orig_width  = w->core.width  + 2*w->core.border_width;
		loc->orig_height = w->core.height + 2*w->core.border_width;
		if ( !loc->options )
		    loc->options = tw->table.default_options;

		XpTableAppendToDefaultLayout( tw, loc );
	    }
	    loc++;	/* loc only incremented for MANAGED children */
	}
    }
    XpTableLocFree( tw->table.real_layout );
    tw->table.real_layout = result;
}

/* Append loc to default_layout
**==============================**
*/
void XpTableAppendToDefaultLayout( tw, loc )
    XpTableWidget	tw;
    XpTableLoc		loc;
{
    int inx;

    tw->table.default_layout = XpTableLocGrow( tw->table.default_layout );
    inx = XpTableLocLen( tw->table.default_layout );
    tw->table.default_layout[inx] = *loc;
}

/* Create New Cols and Rows Vectors
**==================================**
   This must be done whenever a new real_layout is created, or when
   the existing real_layout has been changed.
*/
void XpTableNewColsAndRows( tw )
    XpTableWidget tw;
{
    XpTableVectorFree( tw->table.cols );
    XpTableVectorFree( tw->table.rows );

    tw->table.num_cols = XpTableLocNumCols( tw->table.real_layout );
    tw->table.num_rows = XpTableLocNumRows( tw->table.real_layout );

    if ( tw->table.num_cols && tw->table.num_rows )
    {
	tw->table.cols = XpTableVectorNew( tw->table.num_cols, tw, DO_COL );
	tw->table.rows = XpTableVectorNew( tw->table.num_rows, tw, DO_ROW );
    }
    else
    {
	tw->table.num_cols = tw->table.num_rows = 0;
	tw->table.cols     = tw->table.rows     = (XpTableVector)0;
    }
}

/* Adjust rows and columns to fit
**================================**
   These procedures are called when the Table's parent changes the size of
   the Table.  The XpTableRecomputeLayout() procedure computes the
   preferred size of the table based on the layout and the children, with
   the assumption that the table could be any size.  Now, we have a
   specific size, so we will need to adjust everything to fit.

   If the new size is larger, then its easy: just expand the space available
   to each row and/or column, and change the geometries of all the children.

   If the new size is smaller, then first shrink it to the minimum size,
   and then adjust.

   If the new size is smaller and force_shrink is true (the new default),
   then adjust all children to fit the new size.

   If the new size is smaller and force_shrink is false then we must do
   something like a better behaved version of the old behavior:  If the
   shrink to the preferred size but no smaller.
*/
void XpTableMakeColsFitWidth( tw )
    XpTableWidget tw;
{
    int change;
    int	toFit	= tw->core.width;
    int current = XpTableVectorTotalSize( tw->table.cols, tw->table.num_cols,
					  tw, DO_COL );
    int	min = XpTableVectorPreferredSize( tw->table.cols, tw->table.num_cols,
					  tw, DO_COL );

    if ( toFit < min && 0 == tw->table.force_shrink )
    {
	/* Oh no you don't - smallest size is preferred size.  Excess clipped.
	*/
	change = min - current; /* grow to preferred size */
    }
    else
    {
	change = toFit - current;
    }
    XpTableVectorAdjust( tw->table.cols, tw->table.num_cols, change );
}

void XpTableMakeRowsFitHeight( tw )
    XpTableWidget tw;
{
    int change;
    int toFit = tw->core.height;
    int current = XpTableVectorTotalSize( tw->table.rows, tw->table.num_rows,
					  tw, DO_ROW );
    int min = XpTableVectorPreferredSize( tw->table.rows, tw->table.num_rows,
					  tw, DO_ROW );

    if ( toFit < min && 0 == tw->table.force_shrink )
    {
	/* Oh no you don't - smallest size is preferred size.  Excess clipped.
	*/
	change = min - current; /* shrink or grow to preferred size */
    }
    else
    {
	change = toFit - current;
    }
    XpTableVectorAdjust( tw->table.rows, tw->table.num_rows, change );
}

/* Determine Preferred (Minimum) Size of Table
**=============================================**
*/
int XpTablePreferredWidth( tw )
    XpTableWidget tw;
{
    XpTableVector	vec = tw->table.cols;
    int			num = tw->table.num_cols;

    return XpTableVectorPreferredSize( vec, num, tw, DO_COL );
}

int XpTablePreferredHeight( tw )
    XpTableWidget tw;
{
    XpTableVector	vec = tw->table.rows;
    int			num = tw->table.num_rows;

    return XpTableVectorPreferredSize( vec, num, tw, DO_ROW );
}

/* Request Resize from Parent
**============================**
   This procedure gets called by other XpTable methods when the XpTable
   instance wants to grow or shrink.  Since we cannot yet tell if the
   desired size is OK, the children of the Table have NOT been sized
   or positioned: this is only done by the XpTableResize method.

   Here is when the wonders of Xt Geometry Management come into play.  We
   cannot tell a priori what the hell is going to happen here.  We can ask
   the parent to allow the table to resize based on the computed width and
   height of the cols and rows.

   If the parent says yes, then XpTableResize may, or then again, may
   not, have been called.  Since we don't know, we must keep a bogus
   little flag in the instance to indicate what really happened.
*/
void XpTableRequestResize( tw )
    XpTableWidget tw;
{
    XtGeometryResult	result;
    Dimension		desired_width, desired_height;
    Dimension		approved_width, approved_height;
   
    /* If this is True after the call to XtMakeResizeRequest(), then
    ** we know that XpTableResize() has been invoked.  Otherwise,
    ** we must invoke XpTableResize() directly.
    */
    tw->table.resizeStatus = RSdueToRequest;

    desired_width  = XpTablePreferredWidth( tw );
    desired_height = XpTablePreferredHeight( tw );

    result = XtMakeResizeRequest(	(Widget)tw,
					desired_width, desired_height,
			      		&approved_width, &approved_height );

    /* Nothing special to do if XtGeometryYes or XtGeometryNo
    */
    if ( result == XtGeometryAlmost )
	(void) XtMakeResizeRequest(	(Widget)tw,
					approved_width, approved_height,
					(Dimension*) 0, (Dimension*) 0 );

    /* No matter what the outcome, the Table must be "resized", as this
    ** is where the table looks at its actual width/height and sizes
    ** and positions the children widgets.
    */
    if ( tw->table.resizeStatus == RSdueToRequest )
	XpTableResizeLayout( tw );
}

/* Set Geometry Of Children
**==========================**
   Children are placed according to the real_layout, cols, rows, and
   row and column spacing.
*/
void XpTableSetGeometryOfChildren( tw )
    XpTableWidget tw;
{
    XpTableLoc	loc;

    if ( tw->table.real_layout	== (XpTableLoc)0
      || tw->table.cols		== (XpTableVector)0
      || tw->table.num_cols	== 0
      || tw->table.rows		== (XpTableVector)0
      || tw->table.num_rows	== 0 )
	return;

    XpTableVectorComputeOffsets( tw->table.cols, tw->table.num_cols,
				  tw->table.margin_width,
				  tw->table.col_spacing );

    XpTableVectorComputeOffsets( tw->table.rows, tw->table.num_rows,
				  tw->table.margin_height,
				  tw->table.row_spacing );

    for ( loc = tw->table.real_layout  ;  loc->w  ;  loc++ )
    {
	int cell_x, cell_y, cell_w, cell_h, new_x, new_y, new_w, new_h, pad, i;

	/* Upper left corner of where we will place the widget
	*/
	cell_x = tw->table.cols[ loc->col ].offset;
	cell_y = tw->table.rows[ loc->row ].offset;

	/* cell width and height may well span cols and rows and spacing
	*/
	pad = tw->table.col_spacing;
	cell_w = -pad;
	for ( i = 0  ;  i < loc->col_span  ;  i++ )
	    cell_w += tw->table.cols[ loc->col + i ].value + pad;

	pad = tw->table.row_spacing;
	cell_h = -pad;
	for ( i = 0  ;  i < loc->row_span  ;  i++ )
	    cell_h += tw->table.rows[ loc->row + i ].value + pad;

	/* If widget size is SM, then always use existing size.
	 * Otherwise, use value from cols and rows (the value already
	 * reflects the proper size if LK).
	 */
	if (loc->options & TBL_SM_WIDTH)
	    new_w = loc->w->core.width;
	else
	    new_w  = cell_w - 2 * loc->w->core.border_width;

	if (loc->options & TBL_SM_HEIGHT)
	    new_h = loc->w->core.height;
	else
	    new_h  = cell_h - 2 * loc->w->core.border_width;

	/* Be certain that the size does not go to zero, or negative!
	*/
	if ( new_w <= 0 ) new_w = cell_w;
	if ( new_h <= 0 ) new_h = cell_h;

	if (new_w != loc->w->core.width || new_h != loc->w->core.height)
	    XtResizeWidget( loc->w, new_w, new_h, loc->w->core.border_width );

	new_w = loc->w->core.width  + 2 * loc->w->core.border_width;
	new_h = loc->w->core.height + 2 * loc->w->core.border_width;

	if ( loc->options & TBL_LEFT )
	    new_x = cell_x;
	else if ( loc->options & TBL_RIGHT )
	    new_x = cell_x + cell_w - new_w;
	else
	    new_x = cell_x + (cell_w - new_w)/2;

	if ( loc->options & TBL_TOP )
	    new_y = cell_y;
	else if ( loc->options & TBL_BOTTOM )
	    new_y = cell_y + cell_h - new_h;
	else
	    new_y = cell_y + (cell_h - new_h)/2;

	XtMoveWidget( loc->w, new_x, new_y );
    }
}



/* XpTableOpts methods
**======================**
*/

XpTableOpts XpTableOptsParse( optString )
    String		optString;
{
    XpTableOpts opt = 0;

    for ( ;  *optString;  optString++) {
	switch (*optString)
	{
	case 'l':	opt |= TBL_LEFT;	break;
	case 'r':	opt |= TBL_RIGHT;	break;
	case 't':	opt |= TBL_TOP;		break;
	case 'b':	opt |= TBL_BOTTOM;	break;
	case 'w':	opt |= TBL_LK_WIDTH;	break;
	case 'h':	opt |= TBL_LK_HEIGHT;	break;
	case 'W':	opt |= TBL_SM_WIDTH;	break;
	case 'H':	opt |= TBL_SM_HEIGHT;
	default:	break;
	}
    }
    return opt;
}


/* Client Utility Functions
**==========================**
The following functions are used to reconfigure children of XpTable widgets
*/

/* Position Child in Col,Row of Table
**====================================**
*/
#define CHG_POS  0x1
#define CHG_SPAN 0x2
#define CHG_OPTS 0x4
#define CHG_ALL  0x7

static void Change( loc, what, col, row, col_span, row_span, opts )
    XpTableLoc loc;		/* specific loc to change	*/
    int what;			/* What to change		*/
    int col, row;		/* New position in table	*/
    int col_span, row_span;	/* New spans of child in table	*/
    XpTableOpts opts;		/* New size/justification opts	*/
{
    if ( what & CHG_POS )
    {
	if ( 0 <= col ) loc->col = col;
	if ( 0 <= row ) loc->row = row;
    }
    if ( what & CHG_SPAN )
    {
	if ( 1 <= col_span ) loc->col_span = col_span;
	if ( 1 <= row_span ) loc->row_span = row_span;
    }
    if ( what & CHG_OPTS )
    {
	loc->options = opts;
    }
}

static void XpTableChildChange(child, what, col, row, col_span, row_span, opts)
    Widget child;		/* Child widget to change	*/
    int what;			/* What to change		*/
    int col, row;		/* New position in table	*/
    int col_span, row_span;	/* New spans of child in table	*/
    XpTableOpts opts;		/* New size/justification opts	*/
{
    if ( !XtIsSubclass( child->core.parent, xpTableWidgetClass ) )
    {
	Cardinal one = 1;
	char* name = XtName( child );
	XtWarningMsg( "notChildOfTable", "XpTableChildChange", "XpLibError",
		"Widget %s is not a child of an XpTable widget.", &name, &one);
	return;
    }
    else
    {
	XpTableWidget	tw   = (XpTableWidget)child->core.parent;
	XpTableLoc	def  = XpTableLocFind( tw->table.default_layout,child);
	XpTableLoc	real = XpTableLocFind( tw->table.real_layout,   child);

	if ( def == (XpTableLoc)0 )
	{
	    /* Never laid out this child before.  
	    */
	    static XpTableLocRec	nullRec;
	    XpTableLocRec		newRec;
	    XpTableLoc			initDef;

	    newRec = nullRec;
	    def = &newRec;

	    /* Find the initial default for this name, copy what we can.
	    */
	    initDef = XpTableLocFindDefault( tw->table.default_layout, child );
	    if ( initDef != (XpTableLoc)0 )
		*def = *initDef;

	    /* Set up default fields.
	    */
	    def->w_quark = child->core.xrm_name;
	    def->w	 = child;
	    if ( def->col_span <= 0 ) def->col_span = 1;
	    if ( def->row_span <= 0 ) def->row_span = 1;
	    if ( def->options  == 0 ) def->options  = tw->table.default_options;
	    def->orig_width  = child->core.width  + 2*child->core.border_width;
	    def->orig_height = child->core.height + 2*child->core.border_width;

	    /* Append to default_layout, then get pointer to that loc
	    */
	    XpTableAppendToDefaultLayout( tw, def );
	    def  = XpTableLocFind( tw->table.default_layout,child );
	}

	/* Change loc in default_layout to reflect widget position etc.
	*/
	Change( def, what, col, row, col_span, row_span, opts );
	
	if ( real != (XpTableLoc)0 )
	{
	    /* In real_layout: Change child's actual position/span/opt
	    */
	    Change( real, what, col, row, col_span, row_span, opts );

	    /* We do not have to create a new real_layout.  Also, we know
	    ** child is managed, since it is in the real_layout.  Therefore:
	    */
	    XpTableRecomputeLayout( tw );
	}
	/* If the child is not managed, no re-layout needs to be done: it
	** will be done when the child becomes managed (see ChangeManaged)
	*/
    }
}

/* Change Position of Child
**==========================**
*/
void XpTableChildPosition( child, col, row )
    Widget child;			/* Child widget to move		*/
    int    col, row;			/* New position in table	*/
{
    XpTableChildChange( child, CHG_POS, col, row, 0, 0, 0);
}

/* Change Size (Span) of Child
**=============================**
*/
void XpTableChildResize( child, col_span, row_span)
    Widget child;			/* Child widget to resize	*/
    int    col_span, row_span;		/* New widget span		*/
{
    XpTableChildChange( child, CHG_SPAN, 0, 0, col_span, row_span, 0);
}

void XpTableChildOptions( child, opts )
    Widget	 child;		/* Child widget to get new options	*/
    XpTableOpts opts;		/* New option mask			*/
{
    XpTableChildChange( child, CHG_OPTS, 0, 0, 0, 0, opts );
}

void XpTableChildConfig( child, col, row, col_span, row_span, opts )
    Widget child;		/* Child widget to change	*/
    int col, row;		/* New position in table	*/
    int col_span, row_span;	/* New spans of child in table	*/
    XpTableOpts opts;		/* New size/justification opts	*/
{
    XpTableChildChange(child, CHG_ALL , col, row, col_span, row_span, opts);
}

/* Constructors
**==============**
*/

Widget XpCreateTable( parent, name, arglist, argcount )
    Widget   parent;
    char*    name;
    ArgList  arglist;
    Cardinal argcount;
{
   return XtCreateWidget(name, xpTableWidgetClass, parent, arglist, argcount);
}

/* Table Dialog Constructor 
** Date: Fri, 8 Feb 91 12:23:39 EST
** From: pastor@PRC.Unisys.COM (Jon A. Pastor)
*/

#ifndef DIALOG_SUFFIX
#define DIALOG_SUFFIX "_popup"
#define DIALOG_SUFFIX_SIZE strlen(DIALOG_SUFFIX)
#endif

/* Destroy parent dialog shell when the child is destroyed.
*/
/*ARGSUSED*/
static void XpDestroyParentCallback( w, ignored, unused )
    Widget w;
    XtPointer ignored, unused;
{
    XtDestroyWidget( XtParent( w ) );
}

Widget XpCreateTableTransient( parent, name, arglist, argcount )
    Widget   parent;
    char*    name;
    ArgList  arglist;
    Cardinal argcount;
{
    return XpCreateTableDialog( parent, name, arglist, argcount );
}

Widget XpCreateTableDialog( parent, name, arglist, argcount )
    Widget   parent;
    char*    name;
    ArgList  arglist;
    Cardinal argcount;
{
    char*  dsName;
    int    i;
    Arg    shellArgs[2];
    Widget tableShell, table;

    /* Fabricate a name for the dialog shell using Motif 1.1 naming
    */
    dsName = (char*)XtCalloc( strlen(name)+DIALOG_SUFFIX_SIZE+1, sizeof(char) );
    strcpy( dsName, name ); strcat( dsName, DIALOG_SUFFIX );

    /* Create a Transient Shell widget
    */
    i = 0;
    XtSetArg( shellArgs[i], XtNallowShellResize, True);	i++;
#ifdef XtTransientForBugIsFixed
#ifdef XtSpecificationRelease
    XtSetArg( shellArgs[i], XtNtransientFor, parent);	i++;
#endif
#endif
    tableShell = XtCreatePopupShell( dsName, transientShellWidgetClass, 
					parent, shellArgs, i );
    XtFree( dsName );

    /* Create the XpTable widget
    */
    table = XtCreateWidget( name, xpTableWidgetClass, tableShell, 
			    arglist, argcount );
    XtManageChild( table );
    XtAddCallback( table, XtNdestroyCallback, XpDestroyParentCallback, NULL);

    return table;
}
