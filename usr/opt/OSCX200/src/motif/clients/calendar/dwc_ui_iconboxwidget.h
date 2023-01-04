#if !defined(_iconboxwidget_h_)
#define _iconboxwidget_h_ 1
/* $Header$ */
/* #module DWC$UI_ICONBOXWIDGET.H "V1-001"				    */
/*
**  Copyright (c) Digital Equipment Corporation, 1990
**  All Rights Reserved.  Unpublished rights reserved
**  under the copyright laws of the United States.
**  
**  The software contained on this media is proprietary
**  to and embodies the confidential technology of 
**  Digital Equipment Corporation.  Possession, use,
**  duplication or dissemination of the software and
**  media is authorized only pursuant to a valid written
**  license from Digital Equipment Corporation.
**
**  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
**  disclosure by the U.S. Government is subject to
**  restrictions as set forth in Subparagraph (c)(1)(ii)
**  of DFARS 252.227-7013, or in FAR 52.227-19, as
**  applicable.
**++
**  FACILITY:
**
**	DECwindows Calendar; user interface routines
**
**  AUTHOR:
**
**	Marios Cleovoulou, December-1988
**
**  ABSTRACT:
**
**	This include file contains the public structures etc, for the iconbox
**	widget. 
**
**  ENVIRONMENT:
**
**	User mode, executable image.
**
**
**  MODIFICATION HISTORY:
**
**	V1-001  Marios Cleovoulou				Dec-1988
**		Initial version.
**--
**/


#include "dwc_compat.h"

#if !defined(_iconboxwidgetp_h_)
typedef WidgetClass IconboxWidgetClass;
typedef Widget IconboxWidget;
extern IconboxWidgetClass  iconboxWidgetClass;
#endif

typedef enum
{
    DwcIbwSingleSelection,
    DwcIbwMultipleSelection,
    DwcIbwOrderList
} DwcIbwIconboxStyle;

#define DwcIbwNforeground	     "foreground"
#define DwcIbwNbackground	     "background"
#define DwcIbwNmarginWidth           "ibwMarginWidth"
#define DwcIbwNmarginHeight          "ibwMarginHeight"
#define DwcIbwNiconWidth	     "ibwIconWidth"
#define DwcIbwNiconHeight	     "ibwIconHeight"
#define DwcIbwNspacing               "ibwSpacing"
#define DwcIbwNnumberOfIcons         "ibwNumberOfIcons"
#define DwcIbwNoffIcons              "ibwOffIcons"
#define DwcIbwNnumberOfSelected      "ibwNumberOfSelected"
#define DwcIbwNselectedIcon          "ibwselectedIcon"
#define DwcIbwNselectedIcons         "ibwselectedIcons"
#define DwcIbwNcolumns               "ibwColumns"
#define DwcIbwNrows                  "ibwRows"
#define DwcIbwNiconboxStyle	     "ibwIconboxStyle"
#define DwcIbwNeditable		     "ibwEditable"
#define DwcIbwNvalueChangedCallback  "ibwValueChangedCallback"
#define DwcIbwNhelpCallback          "helpCallback"

#define DwcIbwCMarginWidth           "IbwMarginWidth"
#define DwcIbwCMarginHeight          "IbwMarginHeight"
#define DwcIbwCIconWidth             "IbwIconWidth"
#define DwcIbwCIconHeight	     "IbwIconHeight"
#define DwcIbwCSpacing               "IbwSpacing"
#define DwcIbwCNumberOfIcons         "IbwNumberOfIcons"
#define DwcIbwCOffIcons              "IbwOffIcons"
#define DwcIbwCNumberOfSelected      "IbwNumberOfSelected"
#define DwcIbwCSelectedIcon          "IbwSelectedIcon"
#define DwcIbwCSelectedIcons         "IbwSelectedIcons"
#define DwcIbwCColumns               "IbwColumns"
#define DwcIbwCRows                  "IbwRows"
#define DwcIbwCIconboxStyle	     "IbwIconboxStyle"
#define DwcIbwCEditable		     "IbwEditable"
#define DwcIbwCValueChangedCallback  "IbwValueChangedCallback"
#define DwcIbwCHelpCallback          "IbwHelpCallback"

/*
**  Standard low level create routine (there is no high level one)
*/

Widget DwcIbwIconboxCreate PROTOTYPE ((
	Widget	parent,
	char	*name,
	Arg	*arglist,
	int	argcount));

/*
**  For use with style = SingleSelection
*/

void DwcIbwSetSelectedIcon PROTOTYPE ((Widget w, unsigned char icon));

unsigned char DwcIbwGetSelectedIcon PROTOTYPE ((Widget w));

/*
**  For use with style = MultipleSelection or OrderList
*/

void DwcIbwSetSelectedIcons PROTOTYPE ((
	Widget		w,
	unsigned char	*icons,
	Cardinal	num_icons));

void DwcIbwGetSelectedIcons PROTOTYPE ((
	Widget		w,
	unsigned char	**ret_icons,
	Cardinal	*ret_num_icons));

/*
** Register the widget with Mrm so it can be used as a user_defined from UIL.
*/
int DwcIbwInitializeForMrm PROTOTYPE ((void));

typedef struct _DwcIbwCallbackStruct {
    int			    reason ;
    XEvent		    *event ;
    DwcIbwIconboxStyle	    style ;
    Cardinal		    number_selected ;
    unsigned char	    selected_icon ;
    unsigned char	    *selected_icons ;
    Boolean		    selected_icon_on ;
} DwcIbwCallbackStruct, *DwcIbwCallbackStructPtr ;

#endif /* _iconboxwidget_h_ */
