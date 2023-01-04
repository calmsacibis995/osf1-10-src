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
**************************************************************************
**                   DIGITAL EQUIPMENT CORPORATION                      **
**                         CONFIDENTIAL                                 **
**    NOT FOR MODIFICATION OR REDISTRIBUTION IN ANY MANNER WHATSOEVER   **
**************************************************************************
*/
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/icons.c,v 1.1.2.2 92/12/11 08:34:57 devrcs Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
  #ifndef ULTRIX
  #module ICONS "V1-000"
  #endif
*/
/*
****************************************************************************
**                                                                          *
**  Copyright (c) 1987                                                      *
**  By DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.                        *
**  All Rights Reserved
**                                                                          *
**  This software is furnished under a license and may be used and  copied  *
**  only  in  accordance  with  the  terms  of  such  license and with the  *
**  inclusion of the above copyright notice.  This software or  any  other  *
**  copies  thereof may not be provided or otherwise made available to any  *
**  other person.  No title to and ownership of  the  software  is  hereby  *
**  transferred.                                                            *
**                                                                          *
**  The information in this software is subject to change  without  notice  *
**  and  should  not  be  construed  as  a commitment by DIGITAL EQUIPMENT  *
**  CORPORATION.                                                            *
**                                                                          *
**  DIGITAL assumes no responsibility for the use or  reliability  of  its  *
**  software on equipment which is not supplied by DIGITAL.                 *
**                                                                          *
*****************************************************************************
**++
**  FACILITY:
**
**   DECpaint - VMS DECwindows paint program
**
**  AUTHOR
**
**   Daniel Latham, October 1987
**
**  ABSTRACT:
**
**   ICONS creates the icons used in DECpaint.
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**      dl      10/18/88
**      Put up message if user tries to click on an icon when the current
**      action is not finished.
**
**--
**/           

#include <string.h>
#include "paintrefs.h"
#include "icons.h"

/* icon action look up table */
static int ia_lut[MAX_ACTION] ;
static Widget  icon_buttons[NUM_ICONS];

/*   need to store widget's id in order to reference that widget independantly
 *   later on.
 */
void Create_Icon_Button (w, id)
    Widget w;
    int    *id;
{
    icon_buttons[*id] = w;
}


void Set_Icon_Button_Sensitivity (id, state)
    int id, state;
{
    XtSetSensitive (icon_buttons[id], state);
}

/*
 *
 * ROUTINE:  Unhighlight_Icon
 *
 * ABSTRACT: 
 *
 *  Highlights the given icon cell 
 *
 */
void Unhighlight_Icon (ia)
    int ia;
{
    Fetch_Set_Attribute (icon_buttons[ia], XmNlabelPixmap, icon_names[ia]);
}

/*
 *
 * ROUTINE:  Highlight_Icon
 *
 * ABSTRACT: 
 *
 *  Highlights the given icon cell 
 *
 */
void Highlight_Icon (ia)
    int ia;
{
    char tmp[40];
    strcpy (tmp, icon_names[ia]);
    strcat (tmp, "_neg");
    Fetch_Set_Attribute (icon_buttons[ia], XmNlabelPixmap, tmp);
}


/*
 *
 * ROUTINE:  Set_Icon_Pixmaps
 *
 * ABSTRACT: 
 *
 *  Sets up the icon button pixmaps.
 *
 */
void Set_Icon_Pixmaps ()
{
    int i;
    char *size;
    char tmp[40];

/* If height of screen is not large enough to fit all of the icons, use the
 * small icons.
 */
    if (screen_ht < MIN_HT_FOR_BIG_ICONS) {
	for (i = 0; i < NUM_ICONS; i++) {
	    size = strchr (icon_names[i], (int)'3');
	    *size = '1';
	}
    }

/* Set the label pixmaps and the label insensitive pixmaps */
    for (i = 0; i < NUM_ICONS; i++) {
	Fetch_Set_Attribute (icon_buttons[i], XmNlabelPixmap, icon_names[i]);
	strcpy (tmp, icon_names[i]);
	strcat (tmp, "_ins");
	Fetch_Set_Attribute (icon_buttons[i], XmNlabelInsensitivePixmap, tmp);
    }    
}


/*
 *
 * ROUTINE:  Finished_Action
 *
 * ABSTRACT:
 *
 *  Returns TRUE if the user isn't currently rubberbanding.
 *  If the user is in the middle of rubberbanding, put up a message
 *  telling him to complete the current action and return FALSE.
 *      dl - 10/18/88
 */
int Finished_Action()
{
int retval;

    if( rbanding ){
	switch( current_action ) {
            case TEXT :
		retval = TRUE;
                break;
	    case POLYGON :
/*		Display_Message ("T_FINISH_POLYGON"); 
                retval = FALSE;
                break;
*/
            default :
/*		Display_Message ("T_FINISH_SHAPE"); */
                retval = FALSE;
		XBell (disp, 0);
                break;
        }
    }
    else
	retval = TRUE;
    return( retval );
}



/*
 *
 * ROUTINE:  Set_New_Icon
 *
 * ABSTRACT: 
 *
 *  Set a new icon.
 *
 */
void Set_New_Icon (action)
    int action;
{

/* unhighlight old icon */
	Unhighlight_Icon (ia_lut[current_action]);
	Highlight_Icon (ia_lut[action]);

/* change current action */
	Change_Action(action);
	Set_Cursor (pwindow, current_action);
}


/*
 *
 * ROUTINE:  Set_Select_Icon
 *
 * ABSTRACT: 
 *
 *  Set select icon,  don't worry about deselecting.
 *
 */
void Set_Select_Icon (action)
    int action;
{

/* unhighlight old icon */
    if (action != current_action) {
	Unhighlight_Icon (ia_lut[current_action]);
	Highlight_Icon (ia_lut[action]);

/* change current action */
	prv_action = current_action;
	current_action = action;
	Set_Cursor (pwindow, current_action);
    }
}

/*
 *
 * ROUTINE:  Clicked_On_Icon
 *
 * ABSTRACT: 
 *
 *  Handle Mouse button down events
 *
 */
void Clicked_On_Icon (w, id, r)
	Widget  w;
	int *id;
	XmAnyCallbackStruct *r;
{
int index;

/* dl - 10/18/88 first make sure the user isn't in the middle of rubberbanding*/
    if (Finished_Action())
	Set_New_Icon (icon_action[*id]);
}


Create_Icon_Menu ()
{
    int i;

    for (i = 0; i < NUM_ICONS; i++)
	ia_lut[icon_action[i]] = i; 

/* higlight the brush icon */
    Highlight_Icon (ia_lut[BRUSH]);
}
