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
static char *BuildSystemHeader= "$Header: /usr/sde/x11/rcs/x11/src/./motif/clients/paint/font.c,v 1.1.2.3 92/12/11 08:34:52 devrcs Exp $";	/* BuildSystemHeader */
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
**   DECpaint - VMS DECwindows paint program
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
**                     
**--
**/           
#include "paintrefs.h"

#define MEDIUM_WEIGHT 1
#define BOLD_WEIGHT 2

#define ROMAN_SLANT 1
#define OBLIQUE_SLANT 2

#ifdef I18N_MULTIBYTE
/* We need to know the number of charset */
static int num_charset;
static char **char_set;
static int font_local = 1;
#endif /* I18N_MULTIBYTE */
static int font_family = 1;
static int font_size = 3;
static int font_slant = 1;
static int font_weight = 2;


void Init_Font_Attributes ()
{
    MrmCode data_type;
    caddr_t value;
#ifdef I18N_MULTIBYTE
/* We have some more attributes                            */
/* num_charset is the number of charset, it is more than 1 */
/* we have also a default local font                       */

    if (MrmFetchLiteral (s_DRMHierarchy, "num_charset",
	     XtDisplay (toplevel), &value, &data_type)	== MrmSUCCESS)
	num_charset = (int)*value;

    if (MrmFetchLiteral (s_DRMHierarchy, "char_set",
	    XtDisplay (toplevel), (caddr_t *) &char_set, &data_type)
							!= MrmSUCCESS) {
	char_set = (char **)XtMalloc(sizeof(char *));
	char_set[0] = "ISO8859-1";
    }
    if (num_charset > 1)
	if (MrmFetchLiteral (s_DRMHierarchy, "initial_font_local",
	    XtDisplay (toplevel), &value, &data_type) 	== MrmSUCCESS) {
	    font_local = (int)*value;
	}
#endif /* I18N_MULTIBYTE */
    if (MrmFetchLiteral (s_DRMHierarchy, "initial_font_family",
	    XtDisplay (toplevel), &value, &data_type) 	== MrmSUCCESS) {
	font_family = (int)*value;
    }
    if (MrmFetchLiteral (s_DRMHierarchy, "initial_font_size",
	    XtDisplay (toplevel), &value, &data_type) 	== MrmSUCCESS) {
	font_size = (int)*value;
    }
    if (MrmFetchLiteral (s_DRMHierarchy, "initial_font_slant",
	    XtDisplay (toplevel), &value, &data_type) 	== MrmSUCCESS) {
	font_slant = (int)*value;
    }
    if (MrmFetchLiteral (s_DRMHierarchy, "initial_font_weight",
	    XtDisplay (toplevel), &value, &data_type) 	== MrmSUCCESS) {
	font_weight = (int)*value;
    }
}


int Get_Power_Of_10 (in)
    int in;
{
    int i, out = 1;
    for (i = in; i >= 10; i /= 10) {
        out *= 10;
    }
    return (out);
}


/*
 *
 * ROUTINE:  Set_Font
 *
 * ABSTRACT: 
 *
 *  UnLoad current font and load a new font.  Inputs are:
 *  font_family
 *  font_size
 *  font_slant
 *  font_weight
 *
 */
#ifdef I18N_MULTIBYTE
/* We have to set num_charset number of fonts                       */
/* We will return a comma-separated XLFD names for creating FontSet */
char *Set_Font()
#else
XFontStruct *Set_Font()
#endif /* I18N_MULTIBYTE */
{
    char uil_fname[80];
    char fname[80];
    int i, j, k;
#ifdef I18N_MULTIBYTE
    MrmCode data_type;
    int len;
    char **lname, *flname;

    for (i=0; i < num_charset; i++)
	if (cur_font[i] != NULL)
	    XUnloadFont( disp, cur_font[i]->fid );
#else
/* unload current font */
    if (cur_font != NULL)
	XUnloadFont( disp, cur_font->fid );
#endif /* I18N_MULTIBYTE */

/* Build font name */
    i = 0;
    strncpy (&uil_fname[i], "font_", strlen("font_"));
    i += strlen("font_");

/* append font family name */
    k = font_family;
    for (j = Get_Power_Of_10 (k); j > 0; j /= 10) {
        uil_fname[i] = '0' + (k / j);
        i++;
    }

    uil_fname[i] = '_';
    i++;

/* append weight */
    k = font_weight;
    for (j = Get_Power_Of_10 (k); j > 0; j /= 10) {
        uil_fname[i] = '0' + (k / j);
        i++;
    }

    uil_fname[i] = '_';
    i++;
 
/* append slant */
    k = font_slant;
    for (j = Get_Power_Of_10 (k); j > 0; j /= 10) {
        uil_fname[i] = '0' + (k / j);
        i++;
    }

    uil_fname[i] = '_';
    i++;

/* append size */
    k = font_size;
    for (j = Get_Power_Of_10 (k); j > 0; j /= 10) {
        uil_fname[i] = '0' + (k / j);
        i++;
    }

    uil_fname[i] = '\0';

    if (Get_UIL_Value (fname, uil_fname) != SUCCESS) {
    }

#ifdef I18N_MULTIBYTE
    cur_font[0] = XLoadQueryFont (disp, fname);
    if (cur_font[0] == NULL) {
    /* font not found, use whatever available */
	strcpy(fname, "-*-");
	strcat(fname, char_set[0]);
	cur_font[0] = XLoadQueryFont (disp, fname);
    }

    len = strlen(fname) + num_charset;

/* create local uil font name */
    if (num_charset > 1) {
	i = 0;
	strncpy (&uil_fname[i], "lfont_", strlen("lfont_"));
	i += strlen("lfont_");

	k = font_local;
	for (j = Get_Power_Of_10 (k); j > 0; j /= 10) {
            uil_fname[i] = '0' + (k / j);
            i++;
	}

	uil_fname[i] = '_';
	i++;

	k = font_size;
	for (j = Get_Power_Of_10 (k); j > 0; j /= 10) {
            uil_fname[i] = '0' + (k / j);
            i++;
	}

	uil_fname[i] = '\0';

	MrmFetchLiteral (s_DRMHierarchy, uil_fname,
	    XtDisplay (toplevel), (caddr_t *)&lname, &data_type);

	for (i=1; i < num_charset; i++) {
	    cur_font[i] = XLoadQueryFont (disp, lname[i-1]);
	    if (cur_font[i] == NULL) {
	    /* font not found, use whatever available */
		strcpy(lname[i-1], "-*-");
		strcat(lname[i-1], char_set[i]);
		cur_font[i] = XLoadQueryFont (disp, lname[i-1]);
	    }
	    len += strlen(lname[i-1]);
	}
    } else
	lname = NULL;
    flname = XtMalloc(len);
    strcpy(flname, fname);
    for (i=1; i < num_charset; i++) {
	strcat(flname, ",");
	strcat(flname, lname[i-1]);
    }

    if (lname)
	XtFree((char *)lname);

    return (flname);
#else
    cur_font = XLoadQueryFont (disp, fname);
    return (cur_font);
#endif /* I18N_MULTIBYTE */
}

#ifdef I18N_MULTIBYTE
/*
 *
 * ROUTINE:  Create_Font_List
 *
 * ABSTRACT: 
 *
 *  Free current font list and current font set.
 *  Recreate current font list and current font set.
 *  We use Set_Font to query the requested FontStruct and
 *  we need the return value for creating FontSet if
 *  num_charset > 1 
 *
 */
XmFontList Create_Font_List ()
{
    static XmFontList cur_font_list = NULL;
    XFontSet cur_font_set;
    char *base_font_name_list;
    char **missing_charset_list;
    int missing_charset_count;
    char *def_string;
    int i;

    if (cur_font_list != NULL) {
	XmFontListFree(cur_font_list);
	cur_font_list = NULL;
    }
    base_font_name_list = Set_Font();
    for (i=0; i < num_charset; i++)
	cur_font_list = XmFontListAppendEntry( cur_font_list,
	    XmFontListEntryCreate (
		char_set[i], XmFONT_IS_FONT, (XtPointer)cur_font[i]) );
    if (num_charset > 1) {
	cur_font_set = XCreateFontSet(disp, base_font_name_list,
	    &missing_charset_list, &missing_charset_count, &def_string);
	cur_font_list = XmFontListAppendEntry( cur_font_list,
	    XmFontListEntryCreate ( XmFONTLIST_DEFAULT_TAG,
		XmFONT_IS_FONTSET, (XtPointer)cur_font_set) );
	if (missing_charset_list)
	    XFreeStringList(missing_charset_list);
    }
    XtFree(base_font_name_list);
    return (cur_font_list);
}

/*
 *
 * ROUTINE:  Change_Font_Local
 *
 * ABSTRACT: 
 *
 *  Callback procedure activate when asian font style change.
 *  The corressponding buttons only exist in asian uid,
 *  i.e. This callback will not be called in iso8859 locale.
 *
 */
void Change_Font_Local (w, toggle_id, r)
    Widget   w;
    int	     *toggle_id;
    XmToggleButtonCallbackStruct *r;
{
    if (r->set) {
	font_local = *toggle_id;
	if (entering_text) {
	    Set_Attribute (text_widget, XmNfontList, Create_Font_List());
	}
    }
}
#endif /* I18N_MULTIBYTE */

/*
 * The following are routines that change the current action
 */
void Change_Font_Family (w, toggle_id, r)
    Widget   w;
    int	     *toggle_id;
    XmToggleButtonCallbackStruct *r;
{
    if (r->set) {
	font_family = *toggle_id;
	if (entering_text) {
	    Set_Attribute (text_widget, XmNfontList,
#ifdef I18N_MULTIBYTE
/* We may need a font list of more than one charset */
			   Create_Font_List());
#else
			   XmFontListCreate (Set_Font(),
					     XmSTRING_DEFAULT_CHARSET));
#endif /* I18N_MULTIBYTE */
	}
    }
}

void Change_Font_Size (w, toggle_id, r)
    Widget   w;
    int	     *toggle_id;
    XmToggleButtonCallbackStruct *r;
{
    if (r->set) {
	font_size = *toggle_id;
        if(entering_text) {
	    Set_Attribute (text_widget, XmNfontList,
#ifdef I18N_MULTIBYTE
/* We may need a font list of more than one charset */
			   Create_Font_List());
#else
			   XmFontListCreate (Set_Font(),
					     XmSTRING_DEFAULT_CHARSET));
#endif /* I18N_MULTIBYTE */
	}
    }
}

void Change_Font_Style (w, toggle_id, r)
    Widget   w;
    int	     *toggle_id;
    XmToggleButtonCallbackStruct *r;
{
    switch (*toggle_id) {
	case FONT_STYLE_BOLD_ID :
	    if (font_weight == BOLD_WEIGHT) {
		font_weight = MEDIUM_WEIGHT;
                if (font_slant == ROMAN_SLANT) {
		    Set_Attribute (widget_ids[FONT_STYLE_NORMAL_TOGGLE],
                                   XmNset, TRUE);
		}
	    }
            else {
		font_weight = BOLD_WEIGHT;
                if (font_slant == ROMAN_SLANT) {
		    Set_Attribute (widget_ids[FONT_STYLE_NORMAL_TOGGLE],
                                   XmNset, FALSE);
		}
	    }		
	    break;
	case FONT_STYLE_ITALIC_ID :
	    if (font_slant == OBLIQUE_SLANT) {
		font_slant = ROMAN_SLANT;
		if (font_weight == MEDIUM_WEIGHT) {
		    Set_Attribute (widget_ids[FONT_STYLE_NORMAL_TOGGLE],
				   XmNset, TRUE);
		}
	    }
	    else {
		font_slant = OBLIQUE_SLANT;
		if (font_weight == MEDIUM_WEIGHT) {
		    Set_Attribute (widget_ids[FONT_STYLE_NORMAL_TOGGLE],
				   XmNset, FALSE);
		}
	    }
	    break;
	case FONT_STYLE_NORMAL_ID :
	    if ((font_weight == MEDIUM_WEIGHT) && 
		(font_slant == ROMAN_SLANT)) {
		Set_Attribute (widget_ids[FONT_STYLE_NORMAL_TOGGLE],
			       XmNset, TRUE);
	    }
	    if (font_weight == BOLD_WEIGHT) {
		font_weight = MEDIUM_WEIGHT;
		Set_Attribute (widget_ids[FONT_STYLE_BOLD_TOGGLE],
			       XmNset, FALSE);
	    }
	    if (font_slant == OBLIQUE_SLANT) {
		font_slant = ROMAN_SLANT;
		Set_Attribute (widget_ids[FONT_STYLE_ITALIC_TOGGLE],
			       XmNset, FALSE);
	    }
	    break;
    }

    if (entering_text) {
	Set_Attribute (text_widget, XmNfontList,
#ifdef I18N_MULTIBYTE
/* We may need a font list of more than one charset */
		       Create_Font_List());
#else
		       XmFontListCreate (Set_Font(),
					 XmSTRING_DEFAULT_CHARSET));
#endif /* I18N_MULTIBYTE */
    }
}
