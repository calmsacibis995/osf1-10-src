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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: bkr_cbrresults.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/08/24 15:36:44 $";
#endif
/****************************************************************************/
/*                                                                          */  
/*  Copyright (c) Digital Equipment Corporation, 1990                       */
/*  All Rights Reserved.  Unpublished rights reserved                       */
/*  under the copyright laws of the United States.                          */
/*                                                                          */  
/*  The software contained on this media is proprietary                     */
/*  to and embodies the confidential technology of                          */
/*  Digital Equipment Corporation.  Possession, use,                        */
/*  duplication or dissemination of the software and                        */
/*  media is authorized only pursuant to a valid written                    */
/*  license from Digital Equipment Corporation.                             */
/*                                                                          */  
/*  RESTRICTED RIGHTS LEGEND   Use, duplication, or                         */
/*  disclosure by the U.S. Government is subject to                         */
/*  restrictions as set forth in Subparagraph (c)(1)(ii)                    */
/*  of DFARS 252.227-7013, or in FAR 52.227-19, as                          */
/*  applicable.                                                             */
/*                                                                          */  
/****************************************************************************/
/*.
**. Name:       bkr_cbrresults.c
**.
**. Abstract:   This module contains callbacks for Bookreader's use
**.             of the CBR results widget. Bookreader modifies the
**.             result widget to use XmScrolled list instead of Vlist
**.             for result entries. This is because Bookreader/Char-cell
**.             needs keyboard traversal, which is not currently supported
**.             by vlist. This module somewhat parallels the Dxmcbrresults.c
**.             module.
**.
*/


/*--------------------------------------------------*/
/* Include files                                    */
/*--------------------------------------------------*/

#define	    RESULTS
#define     s_results_box_help	"search_res_db"

#ifdef VMS
#include    <decw$cursor.h>
#else
#include    <X11/decwcursor.h>
#endif

#include    <Xm/Xm.h>
#include    <Xm/XmP.h>
#include    <X11/IntrinsicP.h>
#include    <X11/ShellP.h>
#include    <X11/StringDefs.h>
#include    <X11/Vendor.h>
#include    <X11/VendorP.h>
#include    <X11/Core.h>
#include    <X11/CoreP.h>
#include    <X11/Composite.h>
#include    <X11/CompositeP.h>
#include    <X11/ConstrainP.h>
#include    <Xm/BulletinB.h>
#include    <Xm/BulletinBP.h>
#include    <Xm/PushB.h>
#include    <Xm/Label.h>
#include    <Xm/LabelP.h>
#include    <Xm/PushBP.h>
#include    <Xm/Form.h>
#include    <Xm/FormP.h>
#include    <Xm/MenuShellP.h>
#include    <Xm/TextP.h>
#include    <Mrm/MrmPublic.h>

#include    "br_common_defs.h"
#include    "br_meta_data.h"
#include    "br_typedefs.h"
#include    "br_globals.h"
#include    "br_prototype.h"
#include    "bkr_cbrresults.h"
#include    "bkr_window.h"

static int     selected_position = 1;


/*------------------------------------------------------*/
/* BkrCbrOK_button 				        */
/*------------------------------------------------------*/
extern void 
BkrCbrOK_button( w, tag, data )
    Widget	         w;
    Opaque               tag;
    XmAnyCallbackStruct  *data;
{
    BKR_WINDOW *window = (BKR_WINDOW *)tag;
    Widget     results_dialog = XtParent(window->widgets[W_SEARCH_RESULTS_BOX]) ;

    bkr_simple_results_select(window,selected_position);

    if(bkrplus_g_search_in_progress)
    {
        return;
    }

    XtUnmapWidget(results_dialog);

    return;
}


/*------------------------------------------------------*/
/* BkrCbrApply_button 				        */
/*------------------------------------------------------*/
extern void 
BkrCbrApply_button( w, tag, data )
    Widget	       w;
    Opaque             tag;
    XmAnyCallbackStruct  *data;
{
    if ( ! bkrplus_g_search_in_progress)
    {
        bkr_simple_results_select((BKR_WINDOW *)tag,selected_position);
    }
}

/*------------------------------------------------------*/
/* BkrCbr_double_click 				        */
/*------------------------------------------------------*/
extern void 
BkrCbr_double_click( w, tag, data )
  Widget	       w;
  Opaque               tag;
  XmAnyCallbackStruct  *data;
{
    if ( ! bkrplus_g_search_in_progress)
    {
        bkr_simple_results_select((BKR_WINDOW *)tag,selected_position);
    }
}


/*------------------------------------------------------*/
/* BkrCbrCancel_button 				        */
/*------------------------------------------------------*/
extern void 
BkrCbrCancel_button( w, tag, data )
    Widget	         w;
    Opaque               tag;
    XmAnyCallbackStruct  *data;
{
    BKR_WINDOW           *window = (BKR_WINDOW *)tag;
    Widget               results_dialog = XtParent(window->widgets[W_SEARCH_RESULTS_BOX]);
    
    if ( ! bkrplus_g_search_in_progress)
    {
        XtUnmapWidget(results_dialog);
        
        XmListDeleteAllItems(window->widgets[W_SEARCH_R_LIST]);
        
        window->search.n_results = 0;
        
        selected_position = 1;
    }
}

/*------------------------------------------------------*/
/* BkrCbrSelect_list 				        */
/*------------------------------------------------------*/
extern void 
BkrCbrSelect_list( w, tag, data )
  Widget 	        w;
  Opaque                tag;
  XmListCallbackStruct  *data;
  {

  if(bkrplus_g_search_in_progress)
    {
    return;
    }

  selected_position = data->item_position;

  return;
  }



/*------------------------------------------------------*/
/* Add results to the list 				*/
/*------------------------------------------------------*/

extern int 
BKrCbrResultsAddItem (w, item, watch_cursor_flag)
  Widget	      w;
  XmString            item;
  Boolean	      watch_cursor_flag;
  {

  /* Only display watch cursor if flag is set */
  /* if (watch_cursor_flag)
     DisplayWaitCursor (cbr); */

  XmListAddItem(w, item, 0);

  return(0);
  }




/*------------------------------------------------------*/
/* delete all items					*/
/*------------------------------------------------------*/
extern void
BKrCbrResultsDeleteAllItems (scrolled_list_w, n_list_items)
  Widget    scrolled_list_w;
  int       n_list_items;
  {

  if(!scrolled_list_w)
    return;

  while(n_list_items)
    {
    XmListDeletePos(scrolled_list_w,n_list_items);
    --n_list_items;
    }

  return;
  }



/*--------------------------------------------------*/
/*
/*--------------------------------------------------*/
int 
BKrCbrResultSetTitle(w, string)
    Widget	    	w;
    XmString    	string;
{
    Arg		arglist[2];
    int		argcnt;

    argcnt = 0;
    SET_ARG( XmNtitle, string ); 
    XtSetValues( XtParent(w), arglist, argcnt );
    return;
}


/*--------------------------------------------------*/
/*   
/*--------------------------------------------------*/
int 
BKrCbrResultSetQuery(w, string)
  Widget	    	w;
  XmString       	string;
  {

  DXmCSTextSetString( w, string );

  return(0);
  }



/*------------------------------------------------------------------*/
/* The following two routines are stolen from Dick Schoeller's	    */
/* Calendar implementation.  They are an improvement on 	        */   
/* DXmSpaceButtonsEqually() function.  The usage is as follows:	    */
/*                                                                  */
/* BkrWidthButtonsEqually( widget-list, number );                   */
/*                                                                  */
/* XtManage( whole widget );                                        */
/*                                                                  */
/* BkrSpaceButtonsEqually( widget-list, number );                   */
/*------------------------------------------------------------------*/

void 
BkrWidthButtonsEqually( widgets, num_widgets)

Widget              *widgets;
int                 num_widgets ;
{
    Widget	        w;
    Cardinal	    i;
    Dimension	    max_width = 0;
    
    /*
    ** Traverse the list getting the width.  We save a little time by using
    ** XtWidth instead of XtGetValues.
    */

    for (i = 0; i < num_widgets; i++)
        {
    	w = widgets[i];
    	if (XtWidth(w) > max_width) 
            max_width = XtWidth(w);
        }

    /*
    ** Change the sizes.  We need to use set values here.
    */

    for (i = 0; i < num_widgets; i++)
        {
    	w = widgets[i];
    	XtVaSetValues (w, XmNwidth, max_width, NULL);
        }
}

void 
BkrSpaceButtonsEqually (widgets, num_widgets	)

Widget		*widgets;
int     	num_widgets	;
{
    Widget	parent = XtParent(widgets[0]);
    int		i;
    int		fraction_base;
    int		position;
    int		offset;
    int		width;

    /*
    ** Have to have something.
    */
    if (!widgets || (num_widgets == 0) || !XmIsForm(parent)) 
        return;

    /*
    ** Get the form's fraction base, so that we know how to modify the
    ** positions of the widgets.
    */

    XtVaGetValues (parent, XmNfractionBase, &fraction_base, NULL);

    /*
    ** Get the width of one of the widgets.
    */

    width = (int)XtWidth(widgets[0]);

    /*
    ** Change each one to be centered over the appropriate fraction point.
    */

    for (i = 0; i < num_widgets; i++)
        {
    	/*
    	** The "fraction" for each step is the space between widgets.  This
    	** will be 1 more than the number of widgets, dividing the fraction
    	** base.
    	*/

    	position = ((i + 1) * fraction_base) / (num_widgets + 1);

    	/*
    	** The offset is calculated so that you will have a decreasing
    	** fraction of the widget to the left of the attacment point.  It
    	** makes it so that each section contains the same amount of button.
    	*/

    	offset = (width * (i-(int)num_widgets)) / ((int)num_widgets+1);

    	/*
    	** Reset the horizontal placement completely.
    	*/

    	XtVaSetValues(
        	    widgets[i],
        	    XmNleftAttachment, XmATTACH_POSITION,
        	    XmNleftPosition, position,
        	    XmNleftOffset, offset,
        	    XmNrightAttachment, XmATTACH_NONE,
        	    NULL );
    }

}

