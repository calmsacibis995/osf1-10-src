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
#include "smdata.h"
#include "smresource.h"
#include "smconstants.h"
#include <Xm/Xm.h>

char **ConvertDefResourceToList();
char **ConvertMenuResourceToList();

void AppMenuOkButton();
void AppMenuDismissButton();
void AppMenuApplyButton();
void AppMenuAddButton();
void AppMenuRemoveButton();



void create_appmenu_attrs()
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
        {"AppMenuAddCallback", (caddr_t) AppMenuAddButton},
        {"AppMenuDismissCallback", (caddr_t) AppMenuDismissButton},
        {"AppMenuApplyCallback", (caddr_t) AppMenuApplyButton},
        {"AppMenuOkCallback", (caddr_t) AppMenuOkButton},
        {"AppMenuRemoveCallback", (caddr_t) AppMenuRemoveButton},
        {"appmenu_def_listbox_id", (caddr_t) &appmenusetup.def_list_id},
        {"appmenu_menu_listbox_id", (caddr_t) &appmenusetup.menu_list_id},
};

static int reglist_num = (sizeof reglist / sizeof reglist [0]);

    MrmRegisterNames (reglist, reglist_num);

    /* build the dialog using UIL */
    MrmFetchWidget(s_DRMHierarchy, "CustomizeAppMenu", smdata.toplevel,
                        &appmenusetup.menu_attr_id,
                        &drm_dummy_class);
}

InitListboxContents()
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
    hostlist = ConvertMenuResourceToList(&nhosts);

    /* No defined applications*/
    if (hostlist == NULL) 
	{
	XtSetArg (arglist[0], XmNitems, NULL);
	XtSetArg (arglist[1], XmNitemCount, 0);
	XtSetValues (appmenusetup.menu_list_id, arglist, 2);
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
	    AddSortedItem(appmenusetup.menu_list_id, cslist[i]);

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
	XtSetValues (appmenusetup.def_list_id, arglist, 2);
    	}
    else
	{
	/* Convert each string to a compound string */
	cslist = (XmString *) XtMalloc (nhosts * sizeof (XmString *));
	for (i=0; i<nhosts; i++) {
	    cslist[i] = XmStringCreate(hostlist[i], 
				def_char_set);
	    /* only add it if it is not in Application Menu list box */
	    if (!XmListItemExists(appmenusetup.menu_list_id, cslist[i]))
		{
		AddSortedItem(appmenusetup.def_list_id, cslist[i]);
		}		
	}
	/* Free all of the memory allocated by this routine */
	for (i=0; i<nhosts; i++) {
	    XmStringFree (cslist[i]);
	    XtFree (hostlist[i]);
	}
	XtFree ((char *)cslist);
	XtFree ((char *)hostlist);
	}
appmenusetup.changed = False; 
}

void AppMenuApplyButton()
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
    if (appmenusetup.changed == True) 
	{
	PutAppMenuResources();
	get_customized_menu(smdata.create_menu);
	}
}

void AppMenuOkButton()
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
if (appmenusetup.managed == ismanaged)
    {
    appmenusetup.managed = notmanaged;
    XtUnmanageChild (appmenusetup.menu_attr_id);
    AppMenuApplyButton();
    DeleteLists();
    }
}

void AppMenuDismissButton()
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
if (appmenusetup.managed == ismanaged)
    {
    appmenusetup.managed = notmanaged;
    XtUnmanageChild (appmenusetup.menu_attr_id);
    DeleteLists();
    }
}

void   AppMenuAddButton(widget, tag, data)
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
    XtGetValues (appmenusetup.def_list_id, arglist, 2);
    if (count != 0) {
	copy = (XmString *)XtMalloc(count * sizeof(XmString));
	for (i=0; i<count; i++) {
	    copy[i]  = XmStringCopy(items[i]);
	}
	for (i=0; i<count; i++) {
	    /* add it and make it visible */
	    AddSortedItem(appmenusetup.menu_list_id, items[i]);
	    if (i==0)
		XmListSetItem (appmenusetup.menu_list_id, copy[0]);
	    /* remove it from definition list box */
	    XmListDeleteItem(appmenusetup.def_list_id, copy[i]);
	    XmStringFree(copy[i]);
	}
	XtFree((char *)copy);
    }
    appmenusetup.changed = True;
}

void AppMenuRemoveButton()
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
    XtGetValues (appmenusetup.menu_list_id, arglist, 2);

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
	/* add it and make it visible */
	AddSortedItem(appmenusetup.def_list_id, items[i]);
	XmListDeleteItem(appmenusetup.menu_list_id, copy[i]);
	if (i==0)
	    XmListSetItem (appmenusetup.def_list_id, copy[0]);
	XmStringFree(copy[i]);
	}
     XtFree((char *)copy);    
    appmenusetup.changed = True;
}

char **ConvertMenuResourceToList(nhosts_return)
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
    char 	*the_rep;
    XrmValue	the_value;
    int	value[4];

    sm_get_int_resource(inumappmenu, value);
    nhosts = value[0];
    if (nhosts == 0)
    {
        *nhosts_return = nhosts;
	return(NULL);
    }

    /* Get the string out of the user resource file*/
    XrmGetResource(xrmdb.xrmlist[system_color_type][rdb_merge], "DXsession.AppMenu", NULL, 
			&the_rep, &the_value);

    size = the_value.size;
    if (size <= 0)
    {
        *nhosts_return = nhosts;
	return(NULL);
    }
    resource = the_value.addr;

    /* Allocate memory for the list of names we are going to return */
    hostlist = (char **) XtMalloc (nhosts  * sizeof(char *));

    i = 0;
    walkresource(size, resource, nhosts, hostlist, &i);

    /* return the number of strings and the list */
    *nhosts_return = nhosts;
    return (hostlist);
}

PutAppMenuResources()
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
    XtGetValues (appmenusetup.menu_list_id, arglist, 2);

    /* If there aren't any, then store zeros for the two resources and return*/
    if (count==0) {
	sm_put_resource (iappmenu, "0");
	sm_put_int_resource (inumappmenu, 0);
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
    sm_put_int_resource (inumappmenu, count);
    sm_put_resource (iappmenu, listdata);
    if (newdata != NULL)
	XtFree((char *)newdata);
    if (listdata != NULL)
	XtFree(listdata);
   appmenusetup.changed = False;
}

int DeleteLists()
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
DeleteAList(appmenusetup.menu_list_id);
DeleteAList(appmenusetup.def_list_id);
}

int ResetListbox()
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
DeleteLists();
InitListboxContents();
}

int AddSortedItem(list, additem)
Widget	list;
XmString	additem;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Add a compound string to a listbox in sorted
**	alphabetical order.
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
int i;
Arg	arglist[4];

/* Get all of the entries in the listbox */
XtSetArg (arglist[0], XmNitems, &cslist);
XtSetArg (arglist[1], XmNitemCount, &count);
XtGetValues (list, arglist, 2);

/* If there aren't any, then store zeros for the two resources and return*/
if (count==0) 
    {
    XmListAddItem(list, additem, 0);
    return;
    }

/* tmp = CSToLatin1 (additem);*/
XmStringGetLtoR(additem, def_char_set, &tmp);
for (i=0; i<count; i++)
    {
    char    *string1;
    /* string1 = CSToLatin1 (cslist[i]); */
    XmStringGetLtoR(cslist[i], def_char_set, &string1);
    if (strcmp(tmp,string1) < 0)
	{
	XmListAddItem(list, additem, i+1);
	XtFree(string1);
	XtFree(tmp);
	return;
	}
    XtFree(string1);
    }
XmListAddItem(list, additem, 0);
XtFree(tmp);
}

DeleteAList(list_id)
Widget	list_id;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Remove all of the items from a list boxe
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
    Arg	arglist[4];

    /* if we don't delselect all items, the listbox
	code accvios when we reset the list.  */
    XmListDeselectAllItems(list_id);

    XtSetArg (arglist[0], XmNitemCount, 0);
    XtSetValues (list_id, arglist, 1);
}

int verify_menu_resource()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	When the application definition customize menu
**	is invoked, and items are removed from the
**	application definition list, this routine
**	will ensure that those items are also removed
**	from the application menu.
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
int i,j,m;
int  changed,same;
char	*menu_name;
int nhosts;
char **hostlist;
char **newlist = NULL;
int size = 0;
char    *listdata;

if (appdefsetup.remove_list == NULL) return;

/* Read the current settings from the resource file */
hostlist = ConvertMenuResourceToList(&nhosts);
if (nhosts == 0) return;
m=0;
changed = 0;
newlist = (char **)XtMalloc(sizeof(char *) *nhosts);
for (j=0; j<nhosts; j++)
    {
    same = 0;
    for (i=0; i< appdefsetup.num_used; i++)
	{
	menu_name = 0;
	if (appdefsetup.remove_list[i] != 0)
	    {
	    get_app_def_name(appdefsetup.remove_list[i], &menu_name);
	    if (menu_name != 0)
		if (strcmp(menu_name, hostlist[j]) == 0)
			{
			XtFree(menu_name);
			same = 1;
			break;
			}
	    }
        if (menu_name != 0)
	    XtFree(menu_name);
        }
     if (same == 0)
	    {
	    newlist[m++] = hostlist[j];
	    size = size + strlen(hostlist[j]) + 1;
	    }
     else
	    {
	    changed = 1;
	    }
     }  
if (changed == 1)
    {
    listdata = XtMalloc(size + 1);
    listdata[0] = '\0';
     /* Take each item, allocate some memory and concatanate the
	   string onto the current string seperated by a comma */
    for (i=0; i<m; i++)
	{
	strcat (listdata, newlist[i]);
	if (i != (m - 1))
	    strcat (listdata, ",");
	}

    /* Store the count and the list */
    sm_put_int_resource (inumappmenu, m);
    sm_put_resource (iappmenu, listdata);
    if (listdata != NULL)
	XtFree(listdata);
    }        	    		   
for (j=0; j<nhosts; j++)
    XtFree(hostlist[j]);
XtFree((char *)hostlist);
XtFree((char *)newlist);
}
