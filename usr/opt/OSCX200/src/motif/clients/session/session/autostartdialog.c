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
static char *BuildSystemHeader= "$Header: /usr/sde/osf1/rcs/x11/src/motif/clients/session/session/autostartdialog.c,v 1.1.4.2 1993/06/25 18:20:20 Paul_Henderson Exp $";	/* BuildSystemHeader */
#endif		/* BuildSystemHeader */
/*
! Include files
*/
#include "smdata.h"
#include "smresource.h"
#include "smconstants.h"
#include <Xm/Xm.h>

char **ConvertAutoResourceToList();

void AutoStartOkButton();
void AutoStartDismissButton();
void AutoStartApplyButton();
void AutoStartAddButton();
void AutoStartRemoveButton();
char **ConvertDefResourceToList();



void create_autostart_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is called when the user selects Application Menu
**	from the Session Manager customize menu.  It creates a modeless dialog
**	box which allows the user to modify the contents of the Application
**	pull down menu.
**
**  FORMAL PARAMETERS:
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

static MrmRegisterArg reglist[] = {
        {"AutoStartAddCallback", (caddr_t) AutoStartAddButton},
        {"AutoStartDismissCallback", (caddr_t) AutoStartDismissButton},
        {"AutoStartApplyCallback", (caddr_t) AutoStartApplyButton},
        {"AutoStartOkCallback", (caddr_t) AutoStartOkButton},
        {"AutoStartRemoveCallback", (caddr_t) AutoStartRemoveButton},
        {"autostart_def_listbox_id", (caddr_t) &autostartsetup.def_list_id},
        {"autostart_menu_listbox_id", (caddr_t) &autostartsetup.menu_list_id},
};

static int reglist_num = (sizeof reglist / sizeof reglist [0]);

    MrmRegisterNames (reglist, reglist_num);

    /* build the dialog using UIL */
    MrmFetchWidget(s_DRMHierarchy, "CustomizeAutoStart", smdata.toplevel,
                        &autostartsetup.menu_attr_id,
                        &drm_dummy_class);
}

InitAutoStartListboxContents()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Fill the listbox with the currently defined apps
**	and the current menu list
**	Gets the resource value from the resource file, 
**	converts the strings to compound strings, and stores them
**	in the listbox.
**
**  FORMAL PARAMETERS:
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
    int i;
    int nhosts;
    char **hostlist;
    XmString *cslist;
    Arg arglist[20];

    /* Read the current settings from the resource file */
    hostlist = ConvertAutoResourceToList(&nhosts);

    /* No defined applications*/
    if (hostlist == NULL) 
	{
	XtSetArg (arglist[0], XmNitems, NULL);
	XtSetArg (arglist[1], XmNitemCount, 0);
	XtSetValues (autostartsetup.menu_list_id, arglist, 2);
    	}
    else
	{
	/* Convert each string to a compound string */
	cslist = (XmString *) XtMalloc (nhosts * sizeof (XmString *));
	for (i=0; i<nhosts; i++) {
	    cslist[i] = XmStringCreate(hostlist[i], 
				def_char_set);
	}
    
	/* Put the strings in the list box */
	for (i=0; i<nhosts; i++)
	    AddSortedItem(autostartsetup.menu_list_id, cslist[i]);

	/* Free all of the memory allocated by this routine */
	for (i=0; i<nhosts; i++) {
	    XmStringFree (cslist[i]);
	    XtFree (hostlist[i]);
	}
	XtFree ((char *)cslist);
	XtFree ((char *)hostlist);
	}

    /* Read the current settings from the resource file */
    hostlist = ConvertDefResourceToList(display_id,
		xrmdb.xrmlist[system_color_type][rdb_merge],
		&nhosts, 1);

    /* No defined applications*/
    if (hostlist == NULL) 
	{
	XtSetArg (arglist[0], XmNitems, NULL);
	XtSetArg (arglist[1], XmNitemCount, 0);
	XtSetValues (autostartsetup.def_list_id, arglist, 2);
    	}
    else
	{
	/* Convert each string to a compound string */
	cslist = (XmString *) XtMalloc (nhosts * sizeof (XmString *));
	for (i=0; i<nhosts; i++) {
	    cslist[i] = XmStringCreate(hostlist[i], 
				def_char_set);
	    /* only add it if it is not in Application Menu list box */
	    AddSortedItem(autostartsetup.def_list_id, cslist[i]);
	}
	/* Free all of the memory allocated by this routine */
	for (i=0; i<nhosts; i++) {
	    XmStringFree (cslist[i]);
	    XtFree (hostlist[i]);
	}
	XtFree ((char *)cslist);
	XtFree ((char *)hostlist);
	}
autostartsetup.changed = False; 
}

void AutoStartApplyButton()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the apply button is hit.  It calls a routine which
**	builds the new menu bar.  It then stores the
**	resource in the resource database.
**
**  FORMAL PARAMETERS:
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
    if (autostartsetup.changed == True) 
	{
	PutAutoStartResources();
	}
}

void AutoStartOkButton()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the OK button is hit.  It calls a routine which
**	builds the new menu bar.  It then stores the
**	resource in the resource database.
**
**  FORMAL PARAMETERS:
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
/* this call back can be called multiple times if the user hits the
    button quickly several times with either mouse, or CR.  Check
    to see if we have already unmanaged the widget before we do
    anything */
if (autostartsetup.managed == ismanaged)
    {
    autostartsetup.managed = notmanaged;
    XtUnmanageChild (autostartsetup.menu_attr_id);
    AutoStartApplyButton();
    DeleteASLists();
    }
}

void AutoStartDismissButton()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the dismiss button is hit.  It calls a routine which
**	frees the contents of the list box.
**
**  FORMAL PARAMETERS:
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

/* this call back can be called multiple times if the user hits the
    button quickly several times with either mouse, or CR.  Check
    to see if we have already unmanaged the widget before we do
    anything */
if (autostartsetup.managed == ismanaged)
    {
    autostartsetup.managed = notmanaged;
    XtUnmanageChild (autostartsetup.menu_attr_id);
    DeleteASLists();
    }
}

void   AutoStartAddButton(widget, tag, data)
Widget  widget;
caddr_t tag;
XmListCallbackStruct    *data;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when an item is selected from Application Definition
**	list box. We get the name from the
**	application definition listbox and add it to the application
**	menu list box.  We also remove it from the definition list
**	box.
**
**  FORMAL PARAMETERS:
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
    XmString *items;
    XmString *copy;
    int i;
    int count;
    int len;
    char    *text;
    Arg	arglist[4];

    /* Get the items selected in the listbox */
    XtSetArg (arglist[0], XmNselectedItemCount, &count);
    XtSetArg (arglist[1], XmNselectedItems, &items);
    XtGetValues (autostartsetup.def_list_id, arglist, 2);
    if (count != 0)
	{
	copy = (XmString *)XtMalloc(count * sizeof(XmString));
	for (i=0; i<count; i++)
	    {
	    copy[i]  = XmStringCopy(items[i]);
	    }
	for (i=0; i<count; i++)
	    {
	    /* add it and make it visible */
	    AddSortedItem(autostartsetup.menu_list_id, items[i]);
	    if (i==0)
		XmListSetItem (autostartsetup.menu_list_id, copy[0]);
	    XmStringFree(copy[i]);
	    }
	 XtFree((char *)copy);    
	}
    autostartsetup.changed = True;
}

void AutoStartRemoveButton()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when an item is selected from Application Menu
**	list box. We get the name from the
**	application menu listbox and add it to the application
**	definition list box.  We also remove it from the menu list
**	box.
**
**  FORMAL PARAMETERS:
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
    int i;
    int count;
    XmString *items;
    Arg	arglist[4];
    XmString *copy;

    /* Get the items selected in the listbox */
    XtSetArg (arglist[0], XmNselectedItemCount, &count);
    XtSetArg (arglist[1], XmNselectedItems, &items);
    XtGetValues (autostartsetup.menu_list_id, arglist, 2);

/* put entries on 'removelist' */
/*
 * just look at first selection until workaround is found for
 * multi-selection list
 */
    if (count == 0) return;
    copy = (XmString *)XtMalloc(count * sizeof(XmString));
    for (i=0; i<count; i++)
	{
	copy[i]  = XmStringCopy(items[i]);
	}
    for (i=0; i<count; i++)
	{
	/* remove it */
	XmListDeleteItem(autostartsetup.menu_list_id, copy[i]);
	XmStringFree(copy[i]);
	}
     XtFree((char *)copy);    
    autostartsetup.changed = True;
}

char **ConvertAutoResourceToList(nhosts_return)
    int *nhosts_return;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get the application definition strings out of the resource databases.
**	The string resource will be APPNAME,APPNAME
**
**  FORMAL PARAMETERS:
**
**	nhosts_return - The number of items returned in the list
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
    char *resource;    
    char tmp[80];
    char **hostlist;
    int size;
    int nbytes;
    int nhosts = 0;
    int i;
    char	*the_rep;
    XrmValue	the_value;
    int	value[4];

    sm_get_int_resource(inumautostart, value);
    nhosts = value[0];
    if (nhosts == 0)
	return(NULL);

    /* Get the string out of the user resource file*/
    XrmGetResource(xrmdb.xrmlist[system_color_type][rdb_merge], "DXsession.AutoStart", NULL, 
			&the_rep, &the_value);

    size = the_value.size;
    if (size <= 0)return (NULL);
    resource = the_value.addr;

    /* Allocate memory for the list of names we are going to return */
    hostlist = (char **) XtMalloc (nhosts  * sizeof(char *));

    i = 0;
    walkresource(size, resource, nhosts, hostlist, &i);

    /* return the number of strings and the list */
    *nhosts_return = nhosts;
    return (hostlist);
}

PutAutoStartResources()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get all of the items from the listbox, format them in one
**	long string seperated by commas, and put that resource into
**	the database.  
**
**  FORMAL PARAMETERS:
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
    XmString *cslist;
    int count;
    char *tmp;
    char str[20];
    int i;
    int j;

    char **newdata = NULL;
    Arg	arglist[4];
    char    *listdata = NULL;
    int	size = 0;

    /* mark that there have been changes since the last
	time we saved the setup resources */

    smdata.resource_changed = 1;

    /* Get all of the entries in the listbox */
    XtSetArg (arglist[0], XmNitems, &cslist);
    XtSetArg (arglist[1], XmNitemCount, &count);
    XtGetValues (autostartsetup.menu_list_id, arglist, 2);

    /* If there aren't any, then store zeros for the two resources and return*/
    if (count==0) {
	sm_put_resource (iautostart, "0");
	sm_put_int_resource (inumautostart, 0);
	return;
    }

    /* Convert each item in the list box to a null terminated string, and
       count them as we go along */
    newdata = (char **)XtMalloc(count * sizeof(char *));
    for (i=0; i<count; i++) 
	{
	int len;
	/* tmp = CSToLatin1 (cslist[i]); */
        XmStringGetLtoR(cslist[i], def_char_set, &tmp);
	len = strlen(tmp);
	if (len == 0){
	    XtFree (tmp);
	    count -= 1;
	    continue;
	    }
        else
	    {
	    size = size + len + 1;
	    newdata[i] = tmp;
	    }
	}

    listdata = XtMalloc(size + 1);
    listdata[0] = '\0';
     /* Take each item, allocate some memory and concatanate the
	   string onto the current string seperated by a comma */
    for (i=0; i<count; i++)
	{
	strcat (listdata, newdata[i]);
	if (i != (count - 1))
	    strcat (listdata, ",");
	XtFree (newdata[i]);
	}

    /* Store the count and the list */
    sm_put_int_resource (inumautostart, count);
    sm_put_resource (iautostart, listdata);
    if (newdata != NULL)
	XtFree((char *)newdata);
    if (listdata != NULL)
	XtFree(listdata);
   autostartsetup.changed = False;
}

int DeleteASLists()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Remove all of the items from both list boxes
**
**  FORMAL PARAMETERS:
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
DeleteAList(autostartsetup.menu_list_id);
DeleteAList(autostartsetup.def_list_id);
}

int ResetASListbox()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Delete the current items in the list box and
**	reset it to resource values.   We need this
**	routine when the user switches databases and the
**	dialog box is managed on the screen.
**
**  FORMAL PARAMETERS:
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
DeleteASLists();
InitAutoStartListboxContents();
}

int AutoStart()
{
int i;
int nhosts;
char **hostlist = NULL;

/* Read the current settings from the resource file */
hostlist = ConvertAutoResourceToList(&nhosts);

/* No defined applications*/
if (hostlist == NULL) return;

for (i=0; i<nhosts; i++)
	create_app(hostlist[i], GETSCREEN(screensetup.appl_screennum, display_id));
XtFree ((char *)hostlist);
}
