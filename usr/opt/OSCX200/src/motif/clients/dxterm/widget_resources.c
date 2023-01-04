/* #module Widget_Resources.c "X3.0-1" */
/*
 *  Title:	Widget_Resources.c
 *
 *  +------------------------------------------------------------------------+
 *  | Copyright © Digital Equipment Corporation, 1988, 1993 All Rights       |
 *  | Reserved.  Unpublished rights reserved under the copyright laws of     |
 *  | the United States.                                                     |
 *  |                                                                        |
 *  | The software contained on this media is proprietary to and embodies    |
 *  | the confidential technology of Digital Equipment Corporation.          |
 *  | Possession, use, duplication or dissemination of the software and      |
 *  | media is authorized only pursuant to a valid written license from      |
 *  | Digital Equipment Corporation.                                         |
 *  |                                                                        |
 *  | RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure by the      |
 *  | U.S. Government is subject to restrictions as set forth in             |
 *  | Subparagraph (c)(1)(ii) of DFARS 252.227-7013, or in FAR 52.227-19,    |
 *  | as applicable.                                                         |
 *  |                                                                        |
 *  | The information in this software is subject to change  without  notice |
 *  | and  should  not  be  construed  as  a commitment by Digital Equipment |
 *  | Corporation.                                                           |
 *  |                                                                        |
 *  | DIGITAL assumes no responsibility for the use or  reliability  of  its |
 *  | software on equipment which is not supplied by DIGITAL.                |
 *  +------------------------------------------------------------------------+
 *  
 *  Module Abstract:
 *
 *	This module contains routines to handle transfers from Xrm Databases
 *	to widget resource lists and vice versa.
 *
 *	These routines, or something like them, should end up in the Xtoolkit.
 *	They are based on routines in Resources.c.
 *
 *  Procedures contained in this module:
 *
 *	Exported routines:
 *		GetWidgetTreeDatabase
 *		PutWidgetTreeDatabase
 *		GetResourcesFromDatabase
 *		PutResourcesIntoDatabase
 *
 *	Internal routines:
 *		Recursive
 *		GetNamesAndClasses
 *		Convert
 *
 *  Author:	Tom Porcher  10-Mar-1988
 *
 *  Modification history:
 *
 *  Eric Osman		30-Jul-1993	BL-D
 *	- Merge vxt and vms decterm sources.
 *
 * Alfred von Campe     14-Oct-1992     Ag/BL10
 *      - Added typecasts to satisfy Alpha compiler.
 *
 * Alfred von Campe     02-Apr-1992     Ag/BL6.2.1
 *      - Change XrmFreeDatabase() to the supported XrmDestroyDatabase().
 *
 * Alfred von Campe     20-Jan-1992	DEC OSF/1 SSB MUP1
 *      - Align values returned from XtConvert() before using in XtSetArg().
 *
 * Alfred von Campe     06-Oct-1991     Hercules/1 T0.7
 *      - Changed xrm_r_foo to xrm_foo.
 *
 *  Michele Lien    24-Aug-1990 VXT X0.0
 *  - Modify this module to work on VXT platform. Change #ifdef VMS to
 *    #ifdef VMS_DECTERM so that VXT specific code can be compiled under
 *    VMS or ULTRIX development environment.
 *
 * Bob Messenger	25-Jul-1990	X3.0-5
 *	- Fix PutOneResource to avoid consuming massive amounts of memory.
 *	  The rdb parameter is now passed by reference rather than by
 *	  value.
 *
 * Bob Messenger	25-Jul-1990	X3.0-5
 *	- Add PutOneResource to allow a single resource to be modified in
 *	  a resource database.
 *
 * Mark Woodbury	25-May-1990	X3.0-3M
 *	- Motif update
 *
 * Bob Messenger	10-Apr-1990	X3.0-1
 *	- Toolkit no longer supports XtIsComposite.  Merge in changes from
 *	  Destroy.c.
 *
 * Tom Porcher		30-Jun-1988	X0.4-34
 *	- Added "object" support to Recursive().
 *	- Removed recursive calls to GetClassResources and PutClassResources
 *	  since resource lists are now always a superset of the superclass'
 *	  resource list.
 *
 * Tom Porcher		29-Jun-1988	X0.4-33
 *	- Added "Dimension" and "Position" to the save-able types.
 *	- Added debug_save_widget_tree(w) which can be called from the debugger.
 *
 * Tom Porcher		 9-Jun-1988	X0.4-32
 *	- Added reference database parameter to GetWidgetTreeDatabase,
 *	  PutResourcesIntoDatabase, and PutClassResources.
 *	- Changed string list names to be more logical.
 *
 * Tom Porcher          26-May-1988     X0.4-28
 *      - fixed null array for Ultrix pcc.
 *
 * Tom Porcher		 9-May-1988	X0.4-26
 *	- Fixed free_list to make it more understandable.
 *
 * Tom Porcher		 7-Apr-1988	X0.4-7
 *	- Changed resource list handling to use new resource list tables.
 *	- Added list length parameter to XrmQGetSearchLists() call.
 *	- Brought GetNamesAndClasses up-to-date with Resources.c
 *
 */

#if defined (VMS_DECTERM) || defined (VXT_DECTERM)
#include "MrmAppl.h"
#include "IntrinsicP.h"
#include "ShellP.h"
#else
#include <Mrm/MrmAppl.h>
#include <X11/IntrinsicP.h>
#include <X11/ShellP.h>
#endif

#define SEARCH_LIST_LENGTH 100

#define NULL_ARRAY 1

typedef struct _string_list_item {
    struct _string_list_item *link;
    char data[NULL_ARRAY];
} string_list_item, *string_list;

/*
 * get_string( listp, size )
 * get a pointer to a new string and put it on the specified string list
 */
char*
get_string( listp, size )
    string_list	*listp;
    int		size;
{
    string_list	new_list;

    new_list = (string_list) XtMalloc( sizeof(string_list_item)+size );
    new_list->link = *listp;
    (*listp) = new_list;

    return (new_list->data);
}


/* free_string -- free a string list */

free_string( list )
    string_list	list;
{
    string_list	this, next;

    this = list;

    while ( this != NULL ) {
	next = this->link;
	XtFree( (char *)this );
	this = next;
    }
}

/* Recursive -- from Destroy.c */
/*	altered:  Added args	*/

static void
Recursive(widget, proc, arg1, arg2)
    Widget       widget;
    void	 proc();
    caddr_t	 arg1, arg2;
{
    register int    i;
    CompositePart   *cwp;

    /* Recurse down normal children */
    if (XtIsComposite(widget)) {
	cwp = &(((CompositeWidget) widget)->composite);
	for (i = 0; i < cwp->num_children; i++) {
	    Recursive(cwp->children[i], proc, arg1, arg2);
	}
    }

    /* Recurse down popup children */
    if (XtIsWidget(widget)) {
	for (i = 0; i < widget->core.num_popups; i++) {
	    Recursive(widget->core.popup_list[i], proc, arg1, arg2);
	}
    }

    /* Finally, apply procedure to this widget */
    (*proc) (widget, arg1, arg2);  
} /* Recursive */


/* GetNamesAndClasses -- from Resources.c */
/*	unaltered		*/

static Cardinal
GetNamesAndClasses(w, names, classes)
    register Widget	  w;
    register XrmNameList  names;
    register XrmClassList classes;
{
    register Cardinal length, j;
    register XrmQuark t;
    WidgetClass class;

    for (length = 0; w != NULL; w = (Widget) w->core.parent) {
	names[length] = w->core.xrm_name;
	class = XtClass(w);
	/* KLUDGE KLUDGE KLUDGE KLUDGE */
	if (w->core.parent == NULL && class == applicationShellWidgetClass) {
	    classes[length] =
		    ((ApplicationShellWidget) w)->application.xrm_class;
	} else classes[length] = class->core_class.xrm_class;
	length++;
     }
    /* They're in backwards order, flop them around */
    for (j = 0; j < length/2; j++) {
	t = names[j];
	names[j] = names[length-j-1];
	names[length-j-1] = t;
        t = classes[j];
	classes[j] = classes[length-j-1];
	classes[length-j-1] = t;
    }
    return length;
}


/* Convert -- missing conversions to string */

static void
Convert( widget, from_type, from, to_type, to )
    Widget	widget;
    XrmQuark	from_type, to_type;
    XrmValuePtr	from, to;
{
    static char	value[100];
    char	*value_addr;

    value[0] = '\0';
    to->addr = NULL;
    to->size = 0;

    if (from_type == XrmStringToQuark(XtRString)) {
	(*to) = *from;

    } else if (from_type == XrmStringToQuark(XtRBoolean)) {
	if (*((Boolean *)from->addr)) {
	    strcpy( value, "on" );
	} else {
	    strcpy( value, "off" );
	}

    } else if (from_type == XrmStringToQuark(XtRInt)) {
	sprintf( value, "%d", *((int *)from->addr) );

    } else if (from_type == XrmStringToQuark(XtRDimension)) {
	sprintf( value, "%d", *((Dimension *)from->addr) );

    } else if (from_type == XrmStringToQuark(XtRPosition)) {
	sprintf( value, "%d", *((Position *)from->addr) );

    } else if (from_type == XrmStringToQuark(XtRPixel)) {
	XColor	color;

	color.pixel = *((Pixel *)from->addr);
	XQueryColor( XtDisplay(widget), widget->core.colormap, &color );
	sprintf( value, "#%04x%04x%04x", color.red, color.green, color.blue );
    }


    if ( value[0] != '\0' ) {
	to->addr = value;
	to->size = strlen( value )+1;
    }
}

/*
 * GetClassResources( widgetClass, widget, rdb, names, classes, length,
 *		      arglist, argmax, stringlistp, aligned )
 *	Get resources for this widget's class or superclass
 *	     from the specified resource database.
 */

static Cardinal
GetClassResources( widgetClass, widget, rdb, names, classes, length,
		   arglist, argmax, stringlistp, aligned )
    WidgetClass	    widgetClass;    /* Widget class resources are in	    */
    Widget	    widget;	    /* Widget resources are associated with */
    XrmDatabase     rdb;            /* Xrm database to get resource values from */
    XrmNameList     names;	    /* Full inheritance name of widget      */
    XrmClassList    classes;	    /* Full inheritance class of widget     */
    Cardinal	    length;	    /* Number of entries in names, classes  */
    ArgList	    arglist;
    Cardinal	    argmax;
    string_list	    *stringlistp;
    char	    *aligned[];	    /* Holds aligned values for XtSetArg()  */
{
    XrmResourceList* table;          /* The list of resources required.      */
    Cardinal	    num_resources;  /* number of items in resource list     */
    register 	XrmResourceList* res;
    		XrmValue	value;
    register 	int		j;
    		int		i;
    		XrmHashTable	searchList[SEARCH_LIST_LENGTH];
		XrmQuark	rawType;
		XrmValue	rawValue;
		XrmQuark	QString;
		Cardinal	argcount;

/* No longer needed since class is a superset of superclass' resources */
/*
    if (widgetClass->core_class.superclass != NULL) {
        argcount = GetClassResources(widgetClass->core_class.superclass,
				     widget, rdb, names, classes, length,
				     arglist, argmax, stringlistp );
    } else {
	argcount = 0;
    }
*/
    argcount = 0;

    QString = XrmStringToQuark(XtRString);
    table = (XrmResourceList*)widgetClass->core_class.resources;
    num_resources = widgetClass->core_class.num_resources;

    /* terminate name and class lists */
    names[length] = NULLQUARK;
    classes[length] = NULLQUARK;

    if (num_resources != 0) {

	/* Ask resource manager for a list of database levels that we can
	   do a single-level search on each resource */

	XrmQGetSearchList(rdb, names, classes, searchList, SEARCH_LIST_LENGTH);
	
	for (res = table, j = 0;
	     j < num_resources && argcount < argmax;
	     j++, res++) {

		rawValue.addr = NULL;

		if (XrmQGetSearchResource(searchList,
			(*res)->xrm_name,
			(*res)->xrm_class, &rawType, &rawValue)
			&& rawType != (XrmQuark)(*res)->xrm_type) {

		    XtConvert( widget,
			       XrmQuarkToString(rawType), &rawValue,
			       XrmQuarkToString((XrmQuark)(*res)->xrm_type),
			       &value );
		} else {
		    value = rawValue;
		}

/* XtConvert sometimes returns an unaligned address in value.addr which
   was causing annoying "fixed unaligned access" messages on startup.
   To avoid this, I allocate local memory and bcopy the values before
   using them.  The aligned[] array needs to be freed in this functions'
   caller (GetResourcesFromDatabase() ).
 */

		if (value.addr != NULL) {
		    aligned[argcount] = XtMalloc(value.size);
		    memcpy(aligned[argcount], value.addr, value.size);

		    if ((XrmQuark)(*res)->xrm_type == QString) {
			XtSetArg( arglist[argcount],
				  XrmQuarkToString((*res)->xrm_name),
				  strcpy( get_string( stringlistp,
						      strlen(value.addr)+1),
					  aligned[argcount] ) );

		    } else {
			XtSetArg( arglist[argcount],
				  XrmQuarkToString((*res)->xrm_name),
			          *((caddr_t *)(aligned[argcount])) );
 		    }
		    argcount++;
		}
	}
    }

    return argcount;
}

/*
 * PutClassResources( widget, rdbp, names, classes, length, ref_rdb,
 *		      use_appl_class )
 *	Put resources for this widget's class or superclass into the
 *          specified resource database.
 */
 
static void
PutClassResources( widgetClass, widget, rdbp, names, classes, length, ref_rdb,
		   use_appl_class )
    WidgetClass	    widgetClass;    /* Widget class resources are in	    */
    Widget	    widget;	    /* Widget resources are associated with */
    XrmDatabase     *rdbp;          /* Xrm database to store resource values into */
    XrmNameList     names;	    /* Full inheritance name of widget      */
    XrmClassList    classes;	    /* Full inheritance class of widget     */
    Cardinal	    length;	    /* Number of entries in names, classes  */
    XrmDatabase     ref_rdb;        /* Xrm database for reference           */
    Boolean	    use_appl_class; /* use application class if true,
				           application name if false        */
{
    XrmResourceList* table;         /* The list of resources required.      */
    Cardinal	    num_resources;  /* number of items in resource list     */
    XrmBinding	    bindings[100];  /* List of bindings			    */
    register 	XrmResourceList* res;
    register 	int		j;
    		int		i;
    		XrmHashTable	searchList[SEARCH_LIST_LENGTH];
		XrmQuark	QString;
		XrmQuark	rawType;
		XrmValue	value;
		XrmValue	rawValue;
		XrmValue	ref_value;
                char            name_string[400];
		char            *name;
		XrmQuark	name_zero;
		Boolean		value_alloced;

/* No longer needed since class is a superset of superclass' resources */
/*
    if (widgetClass->core_class.superclass != NULL) {
        PutClassResources(widgetClass->core_class.superclass,
	    widget, rdbp, names, classes, length, ref_rdb, use_appl_class );
    }
*/

    QString = XrmStringToQuark(XtRString);
    table = (XrmResourceList*)widgetClass->core_class.resources;
    num_resources = widgetClass->core_class.num_resources;

    /* Create binding list */
    for ( i=0; i < length+1; i++) {
	bindings[i] = XrmBindTightly;
    }

    /* terminate name and class lists for XrmQGetSearchList() */
    /* terminate name list (will use names[length] for resource name) */
    names[length] = NULLQUARK;
    classes[length] = NULLQUARK;
    names[length+1] = NULLQUARK;

    /* if we are using application class name,
       then replace appl name with class */
    name_zero = names[0];
    if (use_appl_class) {
	names[0] = classes[0];
        }

    if (num_resources != 0) {

	/* Ask resource manager for a list of database levels that we can
	   do a single-level search on each resource */

	XrmQGetSearchList(ref_rdb, names, classes, searchList, SEARCH_LIST_LENGTH);
	
	for (res = table, j = 0; j < num_resources; j++, res++) {

	    /* fill in resource name as last name in name list */
	    names[length] = (XrmQuark) (*res)->xrm_name;

	    /* convert resource to a string value */
            rawValue.addr = ((char *)widget) - (*res)->xrm_offset - 1;
	    if ((XrmQuark)(*res)->xrm_type == QString) {
		value.addr = *((caddr_t *)rawValue.addr);
	    } else {
		Convert( widget, 
			 (XrmQuark)(*res)->xrm_type, &rawValue, QString, &value);
	    }
	    if (value.addr != NULL) {
		/* check reference database:
		   If the resource exists, extract it and
		     convert it to String */
		Boolean value_alloced = FALSE;

		rawValue.addr = NULL;

		if (XrmQGetSearchResource(searchList,
			(*res)->xrm_name,
			(*res)->xrm_class, &rawType, &rawValue)
			&& rawType != QString) {
		    value.addr = XtNewString( value.addr );
		    value_alloced = TRUE;
		    Convert( widget,
			       rawType, &rawValue, QString, &ref_value );
		} else {
		    ref_value = rawValue;
		}

		/* store this resource if not in reference db or
                   value differs from value in reference db */
		if ( ref_value.addr == NULL ||
		       strcmp( value.addr, ref_value.addr ) != 0) 
		    XrmQPutStringResource( rdbp, bindings, names, value.addr );
		if ( value_alloced )
		    XtFree( value.addr );
	    }	    
	}
    }

    names[0] = name_zero;
}

/*
 * GetResourcesFromDatabase( widget, rdb )
 *	Get resources for this widget from the specified resource database.
 */
 
void
GetResourcesFromDatabase( widget, rdb )
    Widget	    widget;	    /* Widget resources are associated with */
    XrmDatabase     rdb;            /* Xrm database to get resource values from */
{
    XrmName         names[100];	    /* Full inheritance name of widget      */
    XrmClass        classes[100];   /* Full inheritance class of widget     */
    Cardinal	    length;	    /* Number of entries in names, classes  */
    Arg		    arglist[200];   /* Arglist for storing list for SetValues */
    Cardinal	    argcount;	    /* count of SetValues args              */
    string_list	    free_me;	    /* pointer to malloc'ed data	    */
    int		    i;		    /* loop counter */
    char 	    *aligned[200];  /* Temporary pointers for XtSetArg call */

    free_me = NULL;

    length = GetNamesAndClasses( widget, names, classes );

    argcount = GetClassResources( widget->core.widget_class, widget, rdb,
				  names, classes, length, arglist,
				  XtNumber(arglist), &free_me, aligned );

    if(argcount)
	    XtSetValues( widget, arglist, argcount );

    free_string( free_me );

    for (i = 0; i < argcount; i++)
	XtFree(aligned[i]);
}


/*
 * PutResourcesIntoDatabase( widget, rdbp, ref_rdb )
 *	Put resources for this widget into the specified resource database.
 *
 * If a resource if found in the ref_rdb and has the same value in the
 * ref_rdb, it is not put into the database.
 */

void
PutResourcesIntoDatabase( widget, rdbp, ref_rdb )
    Widget	    widget;	    /* Widget resources are associated with */
    XrmDatabase     *rdbp;          /* Xrm database to store resource values into */
    XrmDatabase     ref_rdb;        /* Xrm database for reference           */
{
    XrmName         names[100];	    /* Full inheritance name of widget      */
    XrmClass        classes[100];   /* Full inheritance class of widget     */
    Cardinal	    length;	    /* Number of entries in names, classes  */

    length = GetNamesAndClasses( widget, names, classes );

    PutClassResources( widget->core.widget_class, widget, rdbp,
		       names, classes, length, ref_rdb, TRUE );
}

/*
 * PutWidgetTreeDatabase( w, rdb )
 *	Loads resources from the specified database into the widget tree
 *	starting with widget w.
 */

void
PutWidgetTreeDatabase( w, rdb )
    Widget w;
    XrmDatabase rdb;
{

    /* Traverse widget tree */

    Recursive( w, GetResourcesFromDatabase, rdb );

}

/*
 * GetWidgetTreeDatabase( w, ref_rdb )
 *	Returns an XrmDatabase with all the resource values
 *	from the widget tree starting with widget w.
 */

XrmDatabase
GetWidgetTreeDatabase( w, ref_rdb )
    Widget	w;
    XrmDatabase ref_rdb;        /* Xrm database for reference           */
{
    XrmDatabase	rdb;

    rdb = NULL;

    /* Traverse widget tree */

    Recursive( w, PutResourcesIntoDatabase, &rdb, ref_rdb );

    return( rdb );
}

/*
 * PutOneResource
 *
 * Writes the value of a single resource in to a resource database.
 */

Boolean PutOneResource( widget, resource, rdbp )
    Widget widget;
    char *resource;		/* Resource name			*/
    XrmDatabase *rdbp;		/* Resource database; this is both an	*
				 * input parameter and an output	*
				 * parameter				*/
{
    XrmName	names[100];	/* Full inheritance name of widget      */
    XrmClass	classes[100];   /* Full inheritance class of widget     */
    XrmBinding	bindings[100];	/* List of bindings			*/
    Cardinal	length;		/* Number of entries in names, classes  */
    XrmQuark	resource_quark;	/* Resource name in quark form		*/
    WidgetClass	widget_class;	/* Widget class resource is in		*/
    XrmResourceList *table;	/* List of resources for this class	*/
    XrmResourceList *res;	/* An individual resource in the list	*/
    Cardinal	num_resources;	/* Number of resources for this class	*/
    XrmValue	value;		/* Value of the resource (as a string)	*/
    XrmValue	rawValue;	/* Value of the resource before		*/
				/* conversion to string			*/
    XrmQuark	QString;	/* Quark for type String		*/
    int		i, j;		/* General loop indices			*/

    length = GetNamesAndClasses( widget, names, classes );

    /* Create binding list */
    for ( i=0; i < length+1; i++) {
	bindings[i] = XrmBindTightly;
    }

    /* terminate name and class lists for XrmQGetSearchList() */
    /* terminate name list (will use names[length] for resource name) */

    names[length] = NULLQUARK;
    classes[length] = NULLQUARK;
    names[length+1] = NULLQUARK;

    /* replace application name with class name */

    names[0] = classes[0];

    /*
     * Convert the resource name into a quark and search for it in the
     * class structure.  Return if we can't find it.
     */

    resource_quark = XrmStringToQuark( resource );
    widget_class = widget->core.widget_class;
    QString = XrmStringToQuark(XtRString);
    table = (XrmResourceList*)widget_class->core_class.resources;
    num_resources = widget_class->core_class.num_resources;

    for ( res = table, j = 0; j < num_resources; j++, res++ ) {
	if ( (*res)->xrm_name == resource_quark )
	    break;
    }

    if ( j == num_resources )
	return False;		/* no such resource */

    /*
     * Fill in resource name as last name in name list.
     */

    names[length] = (XrmQuark) (*res)->xrm_name;

    /*
     * Convert resource to a string value.
     */
    rawValue.addr = ((char *)widget) - (*res)->xrm_offset - 1;
    if ((XrmQuark)(*res)->xrm_type == QString) {
	value.addr = *((caddr_t *)rawValue.addr);
    } else {
	Convert( widget, (XrmQuark)(*res)->xrm_type, &rawValue, QString,
	  &value );
    }
    if ( value.addr != NULL ) {

	/*
	 * Put the resource into the database.
	 */
	XrmQPutStringResource( rdbp, bindings, names, value.addr );
    }
    return True;
}

void
debug_save_widget_tree( w )
    Widget w;
{
    XrmDatabase rdb;
    XrmDatabase ref_rdb = NULL;

    rdb = GetWidgetTreeDatabase( w, ref_rdb );
#ifdef VXT_DECTERM
    VxtrmPutCurrentDatabase( rdb );
#else
    XrmPutFileDatabase( rdb, "save_all_widgets.dat" );
#endif VXT_DECTERM
    XrmDestroyDatabase( rdb );
}
