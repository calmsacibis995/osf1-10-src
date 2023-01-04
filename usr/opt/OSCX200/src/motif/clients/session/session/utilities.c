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
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "smdata.h"
#include "smresource.h"


int get_win_position(w,x,y)
Widget	w;
int *x;
int *y;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a windows x and y position
**
**  FORMAL PARAMETERS:
**
**	w - The widget that we want to get the position of
**	x - Pointer to return value for x position
**	y - Pointer to return value for y position
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
XtTranslateCoords(w, 0, 0, (Position *)x, (Position *)y);
}

int convert_list(char_list, dwt_list, numelement)
char	*char_list[];
XmString	dwt_list[];
unsigned    int	numelement;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Take a list of input null terminated strings, and create
**	a list of Compound strings
**
**  FORMAL PARAMETERS:
**
**	char_list - A list of null terminated strings
**	dwt_list - Returned list of compound strings
**	numelement - The number of strings in the list
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
unsigned    int	 i;

for (i = 0; i<numelement; i++)
    {
    /* Convert each string */
    dwt_list[i] = XmStringCreate(char_list[i], 
				def_char_set);
    }
}

char *CSToLatin1 (cs)
    XmString cs;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Convert a compound string to a null terminated string
**
**  FORMAL PARAMETERS:
**
**	cs - compound string
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**	Returns a pointer to a compound string
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
    long dont_care;
    XmStringContext context;
    char *tmp = NULL, *old, *new;
    int result;

    XmStringInitContext(&context, cs);
    result = XmStringGetNextSegment(&context, &tmp, &dont_care,
				    &dont_care, &dont_care);

    while (result != FALSE) {
      XtFree((char *)result);
      old = tmp;
      result = XmStringGetNextSegment (&context, &tmp, &dont_care,
				       &dont_care, &dont_care);
      if (result == FALSE) {
	if (tmp) XtFree(tmp);
	tmp = NULL;
      }
      if (!tmp) {
	tmp = (char *)malloc(1);
	strcpy(tmp, "");
      }
      new = (char *)malloc(strlen(old)+strlen(tmp)+1);
      strcpy(new, old);
      strcat(new, tmp);
      XtFree(old);
      XtFree(tmp);
      tmp = new;
    }

    XtFree((char *)result);
    return (tmp);
}


void widget_create_proc (w, tag, reason)
    Widget w;
    Widget *tag;
    unsigned int *reason;
/*---
!
!
!       This routine is called when a UIL widget is created
!       We need to fill in our global pointers to these widgets
!
! Inputs:
!       w           The widget id of the widget that is being created
!       tag         Pointer to data where widget id should be stored
!       reason      The reason for this callback
!
! Outputs:
!       *tag - The widget IDs of the widgets being created are stored
!                   in this address.
!---
*/
{
    *tag = w;
}

/*---
!       This routine figures out the x and y coordinates to use to
!	center the widget "dialog_id" in the middle of "in_widget"
!       If "in_widget is null we use the whole screen
!
! Inputs:
!	display - display of widget
!	in_widget - widget to center in
!	dialog_id - widget to center
!	dix - return x position
!	diy - return y position
!---
*/
get_center_coor(display, in_widget, dialog_id, dix, diy)
Display	*display;
Widget	in_widget;
Widget	dialog_id;
Position *dix, *diy;
{
    Arg     arglist[4];
    Position srcx = 0, srcy = 0;
    Dimension width, height;
    Dimension scrwidth, scrheight;

    /* get the width and height of the dialog box */
    XtSetArg (arglist[0], XmNwidth, &width);
    XtSetArg (arglist[1], XmNheight, &height);
    XtGetValues (dialog_id, arglist, 2);
    
    /* if in_widget is null we want to center in the whole screen
     * otherwise we will center in the middle of "in_widget"
     */
    if (in_widget == (Widget)0)  {
    	scrwidth = XDisplayWidth(display, XDefaultScreen(display));
    	scrheight = XDisplayHeight(display, XDefaultScreen(display));
    }
    else {
      XtSetArg (arglist[0], XmNx, &srcx);
      XtSetArg (arglist[1], XmNy, &srcy);
      XtSetArg (arglist[2], XmNwidth, &scrwidth);
      XtSetArg (arglist[3], XmNheight, &scrheight);
      XtGetValues (in_widget, arglist, 4);
    }
    
    *dix = srcx + ((scrwidth/2) - (width/2));
    *diy = srcy + ((scrheight/2) - (height/2));
}
