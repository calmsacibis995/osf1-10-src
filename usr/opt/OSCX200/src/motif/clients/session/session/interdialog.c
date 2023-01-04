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
#ifndef lint	/* BuildSystemHeader added automatically */
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/session/session/interdialog.c,v 1.1.4.3 1993/10/20 20:39:36 Peter_Wolfe Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
#include <stdio.h>
#include <stdlib.h>
#include "smdata.h"
#include "smresource.h"
#include "smconstants.h"

/*
! External storage
*/


void	setlanguage_ok_pb_cb();
void	setlanguage_apply_pb_cb();
void	setlanguage_dismiss_pb_cb();
void	setlanguage_list_create_cb();
void	setlanguage_list_single_cb();

/*
 * These are the values the language
 * resource can take.  They must be maintained
 * here in the same order as in the UIL list-box
 * items list.
 */

static char             *languages [] =
                                {
                                "Null",       /* indexing starts at 1 */
                                "Default",     /* Default */
				"da_DK.ISO8859-1",       /* Danish */
                                "nl_NL.ISO8859-1",       /* Dutch */
				"nl_BE.ISO8859-1",       /* Dutch Belgian*/
                                "en_GB.ISO8859-1",       /* GB English */
                                "en_US.ISO8859-1",       /* US English */
				"fi_FI.ISO8859-1",       /* Finnish */
                                "fr_FR.ISO8859-1",       /* French */
                                "fr_CA.ISO8859-1",       /* French-Canadian */
				"fr_BE.ISO8859-1",       /* French-Belgian */
                                "de_DE.ISO8859-1",       /* German */
				"is_IS.ISO8859-1",       /* Icelandic */
                                "it_IT.ISO8859-1",       /* Italian */
                                "no_NO.ISO8859-1",       /* Norwegian*/
				"pt_PT.ISO8859-1",       /* Portugese */
                                "es_ES.ISO8859-1",       /* Spanish */
                                "sv_SE.ISO8859-1",       /* Swedish */
                                "fr_CH.ISO8859-1",       /* Swiss-french*/
                                "de_CH.ISO8859-1"        /* Swiss-German*/
                                };
#define num_languages 19
#define DEFAULTLANG 1

int	create_inter_attrs()

{
static int		reglist_num = 5;
static MrmRegisterArg	reglist[] = {
	{"setlanguage_ok_pb_cb",(caddr_t)setlanguage_ok_pb_cb},
	{"setlanguage_apply_pb_cb",(caddr_t)setlanguage_apply_pb_cb},
	{"setlanguage_dismiss_pb_cb",(caddr_t)setlanguage_dismiss_pb_cb},
	{"setlanguage_list_create_cb",(caddr_t)setlanguage_list_create_cb},
	{"setlanguage_list_single_cb",(caddr_t)setlanguage_list_single_cb}
	};

int x,y;
Arg	arglist[10];

MrmRegisterNames (reglist, reglist_num);

if (MrmFetchWidget (
	s_DRMHierarchy,
	"SETLANGUAGE_BOX",
	smdata.toplevel,
	& intersetup.inter_attr_id,
	&drm_dummy_class) 
	!= MrmSUCCESS)
    {
    fprintf (stderr, "can't fetch international window");
    return(0);
    }
return(1);
}

void	setlanguage_ok_pb_cb()
{
if (intersetup.managed == ismanaged)
    {
    intersetup.managed = notmanaged;
    setlanguage_apply_pb_cb();
    XtUnmanageChild(intersetup.inter_attr_id);
    }
}

void	setlanguage_apply_pb_cb()
{
/*
* The changed list box has been verified by user
*/
if (intersetup.list_item_number != intersetup.list_item_edited)
    {
    intersetup.list_item_number = intersetup.list_item_edited;
    inter_put_attrs();
    }
}

void setlanguage_dismiss_pb_cb()
{
if (intersetup.managed == ismanaged)
    {
    intersetup.managed = notmanaged;
    intersetup.changed = 0;
    XtUnmanageChild(intersetup.inter_attr_id);
    /*
    * Reset the list box selection to what it was, and maintain
    * our internal note of this.
    */
    XmListSelectPos(intersetup.SetLanguageListBox, 
		      intersetup.list_item_number, False);
    intersetup.list_item_edited = intersetup.list_item_number;
    }
}

/*
 * Called when list box is created
 */
void	setlanguage_list_create_cb(widget, tag, list)
Widget				widget;
int 				*tag;
XmListCallbackStruct	*list;
{
intersetup.SetLanguageListBox = widget;
}

/*
 * Called when list box item is selected
 */
void	setlanguage_list_single_cb(widget, tag, list)
Widget				widget;
int 				*tag;
XmListCallbackStruct	*list;
{
/*
 * Store the item-number selected so we can use it to find
 * our resource string if this gets 'applied'.
 */

intersetup.list_item_edited = list->item_position;
intersetup.changed = intersetup.changed | minlanguage;
}

extern int setenv();

/****************************************/
extern	int	interattr_get_values()

{
char	svalue[256];
int	value[4];
int	size;
char *lang_environ;

/* primary language */

intersetup.list_item_number = 0;

lang_environ = getenv("LANG");
if (!lang_environ || !strlen(lang_environ)) {
  size = sm_get_any_resource(ilanguage, svalue, value);
  if (size != 0) {
    setenv("LANG=", svalue);
  }
} else {
  strcpy(svalue, lang_environ);
  size = strlen(svalue);
}
if (size != 0 && strlen(svalue))
    {
    for (size = 1; size<num_languages; size++)
	if (strcmp(languages[size], svalue) == 0)
	    {
	    intersetup.list_item_number = size;
	    break;
	    }
    }
      else {
      intersetup.list_item_number = DEFAULTLANG;
      }
}

extern	int	interattr_set_values()

{
unsigned	int	status,found1, found2,i,j;
Arg	arglist[10];
XmString	temparray[2];

intersetup.changed = 0;

/* list box for language . Check if the index value is one that
    we know of */
if ((intersetup.list_item_number == 0) || 
	    (intersetup.list_item_number > num_languages))
    {
    XtSetArg(arglist[0], XmNselectedItemCount, 0);
    XtSetArg(arglist[1], XmNselectedItems, NULL);
    XtSetValues(intersetup.SetLanguageListBox, arglist, 2);
    }
else
    XmListSelectPos(intersetup.SetLanguageListBox, 
	      intersetup.list_item_number, False);

intersetup.list_item_edited = intersetup.list_item_number;
}


extern	int	inter_put_attrs()

{
char	*value;
unsigned	int	ivalue,status;
char	astring[10];	


/* language */

if ((intersetup.changed & minlanguage) != 0)
	{
	if (intersetup.list_item_number == DEFAULTLANG) {
	    sm_remove_resource(ilanguage);
	    setenv("LANG=", "");
	  } else
	if (intersetup.list_item_number != 0) {
	    sm_put_resource(ilanguage, 
			languages[intersetup.list_item_number]);
	    setenv("LANG=", languages[intersetup.list_item_number]);
	  }
	}

if (intersetup.changed != 0)
	{
	sm_change_property(XtDisplay(smdata.toplevel));
	smdata.resource_changed = 1;
	intersetup.changed = 0;
	}
}
