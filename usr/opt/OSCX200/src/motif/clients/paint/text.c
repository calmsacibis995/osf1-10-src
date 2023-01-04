#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Id: text.c,v 1.1.2.3 92/12/11 08:36:08 devrcs Exp $";
#endif		/* BuildSystemHeader */
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
**   DECpaint - DECwindows paint program
**
**  AUTHOR
**
**   Daniel Latham, October 1987
**
**  ABSTRACT:
**
**   This module handles text entry into the picture
**
**  ENVIRONMENT:
**
**   User mode, executable image.
**
**  MODIFICATION HISTORY:
**
**  dp - 7-dec-93  Fix traversal bug and crash when destroying text widget,
**	 	   ootb_bug  447
**                     
**--
**/           
#include "paintrefs.h" 

#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <DXm/DXmCSText.h>
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif

static int margin_wd, margin_ht;
int font_ht, font_wd;
int ascent;
int descent;
int cursor_on;

/*
 *
 * ROUTINE:  End_Text
 *
 * ABSTRACT: 
 *
 *  Terminate current text editing 
 *
 */
void End_Text()
{
    int i;
    int xoff, yoff;
    GC gc_text;
    Pixmap tmp_pix;
    XmFontList font_list;
    XmStringDirection string_dir;
    XmString str;

/* before copying the text into the picture, save state */
    gc_text = Get_GC (GC_PD_SOLID);
    XSetForeground (disp, gc_text, picture_fg);
    XSetBackground (disp, gc_text, picture_bg);

    Get_Attribute (text_widget, XmNfontList, &font_list);
    Get_Attribute (text_widget, XmNstringDirection, &string_dir);
    str = DXmCSTextGetString (text_widget);

    if (str) {
#ifdef I18N_MULTIBYTE
	XSetFont (disp, gc_text, cur_font[0]->fid);
#else
	XSetFont (disp, gc_text, cur_font->fid);
#endif /* I18N_MULTIBYTE */
	shape_xmin = text_x;
	shape_ymin = text_y;
	shape_wd = XmStringWidth (font_list, str);
	shape_ht = XmStringHeight (font_list, str);

	Save_Picture_State();

	if (opaque_fill) 
	    XmStringDrawImage (disp, picture, font_list, str, gc_text,
			       text_x, text_y, shape_wd,
			       XmALIGNMENT_BEGINNING, string_dir, 0);
	else
	    XmStringDraw (disp, picture, font_list, str, gc_text,
			  text_x, text_y, shape_wd,
			  XmALIGNMENT_BEGINNING, string_dir, 0);


	if (UG_num_used > 1) {
	    Set_Cursor_Watch (pwindow);
	    for (i = 1; i <= 4; i++) {
		if (UG_image[UG_last + i] != 0) {
		    tmp_pix = XCreatePixmap (disp, DefaultRootWindow(disp),
					     UG_extra[i].wd, UG_extra[i].ht,
					     pdepth);
		    XPutImage (disp, tmp_pix, Get_GC (GC_PD_COPY),
			       UG_image[UG_last + i], 0, 0, 0, 0,
			       UG_extra[i].wd, UG_extra[i].ht);

		    xoff = picture_x - UG_extra[i].x;
		    yoff = picture_y - UG_extra[i].y;

		    if (opaque_fill) 
			XmStringDrawImage (disp, picture, font_list, str,
					   gc_text, text_x + xoff,
					   text_y + yoff, shape_wd,
					   XmALIGNMENT_BEGINNING,
					   string_dir, 0);
		    else
			XmStringDraw (disp, picture, font_list, str, gc_text,
				      text_x + xoff, text_y + yoff, shape_wd,
				      XmALIGNMENT_BEGINNING, string_dir, 0);

		    MY_XGetSubImage (disp, tmp_pix, 0, 0, UG_extra[i].wd,
				     UG_extra[i].ht, bit_plane, img_format,
				     picture_image,  UG_extra[i].x,
				     UG_extra[i].y, ImgK_Src);

		    XFreePixmap (disp, tmp_pix);
		}
	    }
	    Set_Cursor (pwindow, current_action);
	}
	XmStringFree (str);
	Increase_Change_Pix (undo_x, undo_y, undo_width, undo_height);
	Refresh_Picture (undo_x, undo_y, undo_width, undo_height);
    }
    else {
	if (undo_available)
	    XtSetSensitive (widget_ids[EDIT_UNDO_BUTTON], SENSITIVE);
    }

    XtUnmanageChild (text_widget);
    XmRemoveTabGroup (text_widget);
    XtDestroyWidget (text_widget);

    /* Moved the following call from above XtUnmanageChild to here.
     * Paint was crashing with this call up there. The argument is
     * that freeing the fontlist leaves text widget with dangling
     * pointer for its font resource and that might be causing the
     * crash. It crashed now (as opposed to with earlier versions of
     * motif) as text widget has also changed and hence it might be
     * managing its resources differently.
     */
    XmFontListFree (font_list);

    text_widget = 0;

    entering_text = FALSE;
}            


void Set_Text_Color ()
{
    Arg args[5];
    int argcnt = 0;

    if (text_widget != 0) {
	XtSetArg (args[argcnt], XmNforeground, colormap[paint_color].pixel);
	++argcnt;
	XtSetArg (args[argcnt], XmNbackground, colormap[paint_bg_color].pixel);
	++argcnt;
	XtSetValues (text_widget, args, argcnt);
    }
}


/*
 *
 * ROUTINE:  Begin_Text
 *
 * ABSTRACT:               
 *
 *  Initialize insertion of text into the picture
 *
 */
void Begin_Text()          
{
    Arg args[20];
    int argcnt;                                  
    int ncols, nrows;
#ifdef I18N_MULTIBYTE
    XmFontList cur_font_list;
    extern XmFontList Create_Font_List();
#endif /* I18N_MULTIBYTE */

    if (entering_text)
	End_Text();

    XtSetSensitive (widget_ids[EDIT_UNDO_BUTTON], INSENSITIVE);
#ifdef I18N_MULTIBYTE
/* Create_Font_List will automatically calls Set_Font */
    cur_font_list = Create_Font_List();
#else
    Set_Font(); /* jj-port */
#endif /* I18N_MULTIBYTE */

    entering_text = TRUE;                                      
#ifdef I18N_MULTIBYTE
    ascent = cur_font[0]->max_bounds.ascent;
    descent = cur_font[0]->max_bounds.descent; 
    font_ht = ascent + descent;
    font_wd = cur_font[0]->max_bounds.width;
#else
    ascent = cur_font->max_bounds.ascent;
    descent = cur_font->max_bounds.descent; 
    font_ht = ascent + descent;
    font_wd = cur_font->max_bounds.width;
#endif /* I18N_MULTIBYTE */
                                                                  
    text_x = points[0].x;
    text_y = points[0].y - ascent;
    ncols = 1;
    nrows = 1;
    margin_wd = 2;
    margin_ht = 2;

    argcnt = 0;       
    XtSetArg (args[argcnt], XmNx, IX_TO_PWX(text_x));
    ++argcnt;
    XtSetArg (args[argcnt], XmNy, IY_TO_PWY(text_y));
    ++argcnt;
    XtSetArg (args[argcnt], XmNcolumns, ncols);
    ++argcnt;                            
    XtSetArg (args[argcnt], XmNrows, nrows);
    ++argcnt;
    XtSetArg (args[argcnt], XmNresizeHeight, TRUE);
    ++argcnt;
    XtSetArg (args[argcnt], XmNresizeWidth, TRUE);
    ++argcnt;
    XtSetArg (args[argcnt], XmNeditable, TRUE);
    ++argcnt;
    XtSetArg (args[argcnt], XmNeditMode, XmMULTI_LINE_EDIT);
    ++argcnt;
/*
    XtSetArg (args[argcnt], XmNvalue, "");
    ++argcnt;
*/
    XtSetArg (args[argcnt], XmNfontList,
#ifdef I18N_MULTIBYTE
/* FontList has already been created */
	  cur_font_list);
#else
    	  XmFontListCreate (cur_font, XmSTRING_DEFAULT_CHARSET));
#endif /* I18N_MULTIBYTE */
    ++argcnt;
    XtSetArg (args[argcnt], XmNmarginWidth, margin_wd);
    ++argcnt;
    XtSetArg (args[argcnt], XmNmarginHeight, margin_ht);
    ++argcnt;
    XtSetArg (args[argcnt], XmNborderWidth, 0);
    ++argcnt;
/* Motif
    XtSetArg (args[argcnt], XmNhalfBorder, FALSE);
    ++argcnt;
*/
    XtSetArg (args[argcnt], XmNshadowThickness, 0);
    ++argcnt;
/*
    XtSetArg( args[argcnt], XmNbackgroundPixmap, None );
    ++argcnt;
*/
    XtSetArg (args[argcnt], XmNforeground, colormap[paint_color].pixel);
    ++argcnt;
    XtSetArg (args[argcnt], XmNbackground, colormap[paint_bg_color].pixel);
    ++argcnt;

    text_widget = DXmCreateCSText (picture_widget, "", args, argcnt);
    XmAddTabGroup (text_widget);

    XtManageChild (text_widget);
    XSetInputFocus (disp, XtWindow(text_widget), RevertToParent,
		    CurrentTime);

    /* dp:
     * XmProcessTraversal determines which component of the hierarchy
     * receives keyboard events when the hierarchy that contains the
     * give widget has keyboard focus. XmTRAVERSE_CURRENT finds the 
     * hierarchy that contains the widget and the tab group that 
     * contains the widget. If this tab group is not the active tab
     * group, make it the active tab group. If widget is an item in 
     * the tab group, make it the active item. If widget is the active
     * tab group, make the first traversable item in the tab group
     * the active item. While XmTRAVERSE_NEXT_TAB_GROUP finds the 
     * active tab group and sets the next tab group to be the active
     * tab group in the hierarchy. This was preventing the text widget
     * from being the active item (and tab group) every alternate time
     * it was created and added to the list of tab groups.
     */

    XmProcessTraversal (text_widget, XmTRAVERSE_CURRENT);
}
