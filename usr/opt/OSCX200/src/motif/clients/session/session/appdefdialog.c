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
static char *BuildSystemHeader= "$Id: appdefdialog.c,v 1.1.4.2 1993/06/25 18:15:02 Paul_Henderson Exp $";
#endif		/* BuildSystemHeader */
#include <stdio.h>
#include "smdata.h"
#include "smresource.h"
#include "smconstants.h"
#include <Xm/Xm.h>
#include <Xm/Text.h>

char **ConvertDefResourceToList();

void AppDefOkButton();
void AppDefDismissButton();
void AppDefApplyButton();
void AppDefAddButton();
void AppDefRemoveButton();
void AppDefListSelect();
int count_data();
int store_data();
char **add_app_commands();


void create_appdef_attrs()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine is called when the user selects Application Definition
**	from the Session Manager customize menu.  It creates a modeless dialog
**	box which allows the user to modify the contents of the Application
**	Definition list box.
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
        {"AppDefAddButton", (caddr_t) AppDefAddButton},
        {"AppDefDismissButton", (caddr_t) AppDefDismissButton},
        {"AppDefApplyButton", (caddr_t) AppDefApplyButton},
        {"AppDefOkButton", (caddr_t) AppDefOkButton},
        {"AppDefRemoveButton", (caddr_t) AppDefRemoveButton},
        {"AppDefListSelect", (caddr_t) AppDefListSelect},
        {"appdef_listbox_id", (caddr_t) &appdefsetup.def_list_id},
        {"appdef_command_field_id", (caddr_t) &appdefsetup.command_text_id},
        {"appdef_name_field_id", (caddr_t) &appdefsetup.menu_text_id},
        {"appdef_remove_id", (caddr_t) &appdefsetup.menu_remove_id},
};

static int reglist_num = (sizeof reglist / sizeof reglist [0]);

    MrmRegisterNames (reglist, reglist_num);

    /* build the dialog using UIL */
    MrmFetchWidget(s_DRMHierarchy, "CustomizeAppDef", smdata.toplevel,
                        &appdefsetup.menu_attr_id,
                        &drm_dummy_class);
}

InitAppDefListboxContents()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Fill the listbox with the currently defined apps
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
    hostlist = ConvertDefResourceToList(display_id,
		xrmdb.xrmlist[system_color_type][rdb_merge],
		&nhosts, 1);

    /* No defined applications*/
    if (hostlist == NULL) 
	{
	XtSetArg (arglist[0], XmNitems, NULL);
	XtSetArg (arglist[1], XmNitemCount, 0);
	XtSetValues (appdefsetup.def_list_id, arglist, 2);
    	}
    else
	{
	/* Convert each string to a compound string */
	cslist = (XmString *) XtMalloc (nhosts * sizeof (XmString *));
	for (i=0; i<nhosts; i++) {
	    cslist[i] = XmStringCreate(hostlist[i],
				def_char_set);
	    AddSortedItem(appdefsetup.def_list_id, cslist[i]);
	}
	/* Free all of the memory allocated by this routine */
	for (i=0; i<nhosts; i++) {
	    XmStringFree (cslist[i]);
	    XtFree (hostlist[i]);
	}
	XtFree ((char *)cslist);
	XtFree ((char *)hostlist);
	}
appdefsetup.changed = False; 
XmTextSetString(appdefsetup.menu_text_id, "");
XmTextSetString(appdefsetup.command_text_id, "");
}

void AppDefApplyButton()
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
    if (appdefsetup.changed == True) 
	{
	PutAppDefResources();
	CleanUpData();
	get_customized_menu(smdata.create_menu);
	}
}

void AppDefOkButton()
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
if (appdefsetup.managed == ismanaged)
    {
    appdefsetup.managed = notmanaged;
    XtUnmanageChild (appdefsetup.menu_attr_id);
    AppDefApplyButton();
    DeleteAList(appdefsetup.def_list_id);
    }
}

void AppDefDismissButton()
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
if (appdefsetup.managed == ismanaged)
    {
    appdefsetup.managed = notmanaged;
    XtUnmanageChild (appdefsetup.menu_attr_id);
    CleanUpData();
    DeleteAList(appdefsetup.def_list_id);
    }
}

void   AppDefAddButton(widget, tag, data)
Widget  widget;
caddr_t tag;
XmListCallbackStruct    *data;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the add button is hit.  We need to add the current
**	contents of the text field to the list box.
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
    XmString copy;
    int i;
    int count;
    int len;
    char    *text = NULL;
    char    *command = NULL;
    char    *command_request = NULL;
    char    *text_ptr;
    int	issys = 0;
    Arg	arglist[4];
    Time    time = CurrentTime;

    text = XmTextGetString(appdefsetup.menu_text_id);
    i = strlen(text);
    for (text_ptr = text + i - 1; i; i--, text_ptr--)
	if (!(isalnum(*text_ptr) || strchr("_- \t", *text_ptr)))
	    break;
    if (i)
	{
	put_error(0, k_appdef_invalid_msg);
	if (text != NULL) XtFree(text);
	if (command != NULL) XtFree(command);
	return;
	}
    command = XmTextGetString(appdefsetup.command_text_id);
    if ((*text == NULL) || (*command == NULL))
	{
	put_error(0, k_appdef_missing_msg);
	if (text != NULL) XtFree(text);
	if (command != NULL) XtFree(command);
	return;
	}
    copy = XmStringCreate(text , def_char_set);
    if (!XmListItemExists(appdefsetup.def_list_id, copy))
	{
	AddAppDef(text,command);
	/* add it and make it visible */
	AddSortedItem(appdefsetup.def_list_id, copy);
	XmListDeselectAllItems(appdefsetup.def_list_id);
	XmListSelectItem(appdefsetup.def_list_id, copy, 1);
	}
    else
	{
	get_menu_command(text, &command_request, &issys);
	if (issys == 1)
	    {
	    put_error(0, k_appdef_system_msg);
	    }
	else
	    {
	    AddAppDef(text,command);
	    XmListDeselectAllItems(appdefsetup.def_list_id);
	    XmListSelectItem(appdefsetup.def_list_id, copy, 1);
	    }
	}
    if (command_request != NULL)
	XtFree(command_request);

    XtFree(command);
    XtFree(text);
    XmStringFree(copy);
    appdefsetup.changed = True;
}

int AddAppDef(text,command)
char	*text;
char	*command;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Add a resource to our temporary database.  For every menu
**	command entered, we need to have the command string stored
**	in the database.  If the user hits cancel we just free the
**	database.  Otherwise, we can merge this database with the
**	old one so that new commands exist in the normal session
**	manager database.
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
char	*fullcommand;

get_app_def_command(text, &fullcommand);
XrmPutStringResource(&appdefsetup.temp_db_id, fullcommand, command);
/* get it off of the remove list if it is there */
if (appdefsetup.remove_list != NULL)
    {
    int	i;
    for (i=0; i< appdefsetup.num_allocated; i++)
	{
	if (appdefsetup.remove_list[i] != 0)
	    {
	    if (strcmp(appdefsetup.remove_list[i], fullcommand) == 0)
		{
		XtFree(appdefsetup.remove_list[i]);
		appdefsetup.remove_list[i] = 0;
		}
            }
        }
    }
XtFree(fullcommand);
}

#define allocinc    10
void AppDefRemoveButton()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when the Remove button is hit.
**	Get the selected item from the Application Definition
**	list box and remove the item from the list box
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
    XtGetValues (appdefsetup.def_list_id, arglist, 2);

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
    add_remove_list(count);
    for (i=0; i<count; i++)
	{
	char	*temp;
	char	*command;
	/* temp = CSToLatin1 (copy[i]);*/
        XmStringGetLtoR(copy[i], def_char_set, &temp);
	get_app_def_command(temp, &command);
	appdefsetup.remove_list[appdefsetup.num_used++] = command;
	XmListDeleteItem(appdefsetup.def_list_id, copy[i]);
	XtFree((char *)copy[i]);
	XtFree(temp);
	}
    XtFree((char *)copy);    
    appdefsetup.changed = True;
    XmTextSetString(appdefsetup.menu_text_id, "");
    XmTextSetString(appdefsetup.command_text_id, "");
}

void   AppDefListSelect(widget, tag, data)
Widget  widget;
caddr_t tag;
XmListCallbackStruct    *data;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called when an item is selected.  We need to add the current
**	resource def for the item to the contents of the command text field 
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
    char    *command_request = NULL;
    char    *command = NULL;
    char    *label;
    char    *the_rep;
    XrmValue        the_value;
    int	issys = 0;
    Time    time = CurrentTime;

    /* Get the items selected in the listbox */
    XtSetArg (arglist[0], XmNselectedItemCount, &count);
    XtSetArg (arglist[1], XmNselectedItems, &items);
    XtGetValues (appdefsetup.def_list_id, arglist, 2);

    
    /* If you reselected a selected item, count is 0 and item is
       set to junk.  If you do a XmStringGetLttoR with item being
       junk, you get a core dump.  Very unattractive.
       So, need to check if count is 0.  If it is, set label to
       an empty string.  Else get XmString|GetLtoR and set label
     */
    /* label = CSToLatin1 (items[0]);*/
    if (count == 0) 
      {
	label = "";
      }
    else 
      {
	XmStringGetLtoR(items[0], def_char_set, &label);
      }
    XmTextSetString(appdefsetup.menu_text_id, label, "");
    
    the_value.size = 0;
    if (appdefsetup.temp_db_id != 0)
	    {
	    get_app_def_command(label, &command);
	    if (command != NULL) 
	      {
		XrmGetResource(appdefsetup.temp_db_id, 
			    command, NULL, &the_rep, &the_value);
	      }
	    
	    if (the_value.size != 0)
		    {
		    command_request=(XtMalloc(the_value.size + 1));
		    strncpy(command_request, the_value.addr, the_value.size);
		    command_request[the_value.size] = 0; 
		    }
	    if (command != NULL) 
	      {
		XtFree(command);
	      }
	    
            }
     
    if (command_request == NULL)
	    {
	      get_menu_command(label, &command_request, &issys);
	    }
    if (command_request != NULL)
	 {
	 XmTextSetString(appdefsetup.command_text_id, command_request, "");
	 }
    else
	 {
	 XmTextSetString(appdefsetup.command_text_id, "");
	 }
    XtFree(label);
    if (command_request != NULL)
	XtFree(command_request);
    if (issys == 1)
	{
        XtSetSensitive(appdefsetup.menu_remove_id, FALSE);
	}
    else
	{
        XtSetSensitive(appdefsetup.menu_remove_id, TRUE);
	}
    /*
    (* XtCoreProc (appdefsetup.menu_text_id, accept_focus))
                      (appdefsetup.menu_text_id, &time);
    */
}

PutAppDefResources()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get all of the items from the listbox, format them in one
**	long string seperated by commas, and put that resource into
**	the database.  Also add each of the commands
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
    char *command;
    char *command_request = NULL;
    char *the_rep;
    XrmValue        the_value;
    char str[20];
    int i;
    int j;

    char **newdata = NULL;
    Arg	arglist[4];
    char    *listdata = NULL;
    int	size = 0;
    char    *resource_name = "DXsession.applications";
    char    *temp;

    /* mark that there have been changes since the last
	time we saved the setup resources */

    smdata.resource_changed = 1;

    /* Get all of the entries in the listbox */
    XtSetArg (arglist[0], XmNitems, &cslist);
    XtSetArg (arglist[1], XmNitemCount, &count);
    XtGetValues (appdefsetup.def_list_id, arglist, 2);

    /* If there aren't any, then store zeros for the two resources and return*/
    if (count==0) 
	{
	/* put the DXsession.applications resource on the remove list so that it
	   is deleted */
	add_remove_list(1);
	temp = XtMalloc(strlen(resource_name) + 1);
	strcpy(temp, resource_name);
	appdefsetup.remove_list[appdefsetup.num_used++] = temp;
	if (appdefsetup.temp_db_id != 0)
	    {
	    XrmDestroyDatabase(appdefsetup.temp_db_id);
	    appdefsetup.temp_db_id = 0;
	    }
	}
    else
	{
	/* Convert each item in the list box to a null terminated string, and
	count them as we go along */
	newdata = (char **)XtMalloc(count * sizeof(char *));
	for (i=0,j=0; i<count; i++) 
	    {
	    int len;
	    char    *command;
	    int	issys = 0;

            command = NULL;
	    /* tmp = CSToLatin1 (cslist[i]); */
    	    XmStringGetLtoR(cslist[i], def_char_set, &tmp);
	    get_menu_command(tmp, &command, &issys);
	    if (issys != 1)
		{
		len = strlen(tmp);
		if (len == 0){
		    count -= 1;
		    continue;
		    XtFree (tmp);
		    }
		else
		    {
		    size = size + len + 1;
		    newdata[j++] = tmp;
		    }
		 }
            else
		XtFree (tmp);
            if (command != NULL)
		XtFree(command);
	    }

	if (j == 0)
	    {
	    XtFree((char *)newdata);
	    /* put the DXsession.applications resource on the remove list so that it
	       is deleted */
	    add_remove_list(1);
	    temp = XtMalloc(strlen(resource_name) + 1);
	    strcpy(temp, resource_name);
	    appdefsetup.remove_list[appdefsetup.num_used++] = temp;
	    if (appdefsetup.temp_db_id != 0)
		{
		XrmDestroyDatabase(appdefsetup.temp_db_id);
		appdefsetup.temp_db_id = 0;
		}
	    }
	else
	    {
	    listdata = XtMalloc(size + 1);
	    listdata[0] = '\0';
	     /* Take each item, allocate some memory and concatanate the
		   string onto the current string seperated by a comma */
	    for (i=0; i<j; i++)
		{
		strcat (listdata, newdata[i]);
		if (i != (j - 1))
		    strcat (listdata, ",");

		 /* get the command resource name for newdata[i] */
		get_app_def_command(newdata[i], &command);
		/* get the resource and store it */
		XrmGetResource(appdefsetup.temp_db_id, 
			    command, NULL, &the_rep, &the_value);
	    	if (the_value.size != 0)
		    {
		    command_request=(XtMalloc(the_value.size + 1));
		    strncpy(command_request, the_value.addr, the_value.size);
		    command_request[the_value.size] = 0;

		    /* Only resave the resource if it has been changed */
		    PhilSave( command, command_request, rdb_merge/*generic*/);
		    }

	        if (command != NULL)
		    XtFree(command);
		
		if (command_request != NULL)
		   XtFree(command_request);

		XtFree (newdata[i]);
		}
    /* For ultrix, A.R., put each item into PhilSave.
	For items that need to be removed, need to call a remove to
	PhilSave.  If we do this up here, we can get the DXsession.*.command
	resource and call PhilSave with it. 
	
	A.R.  do this later -- remove  */

	    /* Store the count and the list */
    	    sm_put_int_resource (inumapps, count);
	    sm_put_resource (iapps, listdata);

	    XrmPutStringResource(&appdefsetup.temp_db_id, resource_name, listdata);

	    sm_change_property(XtDisplay(smdata.toplevel));       

	    if (newdata != NULL)
		XtFree((char *)newdata);
	    if (listdata != NULL)
		XtFree(listdata);
	    }
      }
    
    /* Now, for each item in the temporary database, put it in the
       real session manager resource database */
   if (appdefsetup.temp_db_id != 0)
	{
	/* now merge the two */
 	XrmMergeDatabases(appdefsetup.temp_db_id, 
		    &xrmdb.xrmlist[system_color_type][rdb_merge]);
	appdefsetup.temp_db_id = 0;
	}
   /* Now we need to remove the items that are on the remove list.  There is
      no call to remove a resource.  We will call this routine which will
      scan the database and look at each item.  If it is on the remove list
      we will not write it to the buffer.  Then we will reset the merge
      and general databases to be the new database without the items on the
      remove list */
   if (appdefsetup.remove_list != NULL)
    {
    verify_menu_resource();
    for(i=0; i<appdefsetup.num_used; i++)
	if (appdefsetup.remove_list[i] != 0)
	    {
	    ResetDataBase(&xrmdb.xrmlist[system_color_type][rdb_merge]);
	    break;
	    }
   }
   CleanUpData();   
/* we will also reset the lists which define which resources to store as pro
    properties on the root window */
    if (removecount != 0)
	{
	for (i=0; i<removecount; i++)
	    if (removelist[i] != NULL)
		    free(removelist[i]);
	free(removelist);
        removecount = 0;
	}
   removelist = get_remove_list(&removecount);
   removelist = add_app_commands(display_id, 
		xrmdb.xrmlist[system_color_type][rdb_merge],
		removelist, &removecount);
   appdefsetup.changed = False;
}

int ResetAppDefListbox()
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
CleanUpData();
DeleteAList(appdefsetup.def_list_id);
InitAppDefListboxContents();
}

int CleanUpData()
{
int i;

if (appdefsetup.temp_db_id != 0)
	{
	XrmDestroyDatabase(appdefsetup.temp_db_id);
	appdefsetup.temp_db_id = 0;
	}
if (appdefsetup.remove_list != NULL)
    {
    for (i=0; i<appdefsetup.num_used; i++)
	if (appdefsetup.remove_list[i] != 0)
		XtFree(appdefsetup.remove_list[i]);
    XtFree((char *)appdefsetup.remove_list);
    }
appdefsetup.remove_list = NULL;
appdefsetup.num_used = 0;
appdefsetup.num_allocated = 0;
}

ResetDataBase(userResourcesDB)
XrmDatabase *userResourcesDB;
{
unsigned        int     status, i;
char    *buffer;
unsigned int    size = 0;
XrmQuark everything = NULLQUARK;

/* walk the database and get the size of the buffer needed.  the
    routine count_items will be called for every item in the
    database*/

init_static_quark();

XrmEnumerateDatabase(*userResourcesDB, (XrmNameList)&everything, 
		     (XrmClassList)&everything, XrmEnumAllLevels, count_data,
		     (XtPointer)&size);

/* now allocate that much memory for the buffer */
buffer = (char *)malloc(size);
if (buffer == 0)
        {
	fprintf(stderr, "insufficient memory!\n");
	/* A.R.  should we do an exit? or popup a dialog box  */
	return;
        }

/* initialize the first element of buffer to null */
*buffer = 0;

XrmEnumerateDatabase(*userResourcesDB, (XrmNameList)&everything, 
		     (XrmClassList)&everything, XrmEnumAllLevels, store_data,
		     buffer);

/* now we have a buffer which has the necessary items removed.  Free
   the  old database, and reset it to the new buffer */
XrmDestroyDatabase(*userResourcesDB);
*userResourcesDB = XrmGetStringDatabase(buffer);
free(buffer);
}

/**************************************/
int count_data(db, bindings, quarks, type, value, length)
    XrmDatabase		*db;
    XrmBindingList      bindings;
    XrmQuarkList        quarks;
    XrmRepresentation   *type;
    XrmValue            *value;
    unsigned int        *length;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Called once for every line in the database.
**      Call storeentry to construct the ascii representation of that line.
**      Then count the number of items and add it to the previous length.
**
**  FORMAL PARAMETERS:
**
**	db - The database being enumerated
**      bindings - The list of bindings - the binding will either be a * or a .
**      quarks - the list of quarks.  In the resource wm.foreground, there are
**          two quarks - 1. wm and 2. foreground
**      type - could be any of the xlib defined types for resources - string,
**              int, etc...
**      value - the value of the resource.  Contains an ascii string and a size
**      length - the pointer to the integer which is to hold the length of the
**              final resource string
**
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**	Assumes no resource string is greater than 10000 chars.
**--
**/
{
char	name[10000];
int i,j,same;

name[0] = 0;

/* get the actual string representation for a resource and value */
StoreEntry(bindings, quarks, *type, value, name);

/* don't store the resource which is on remove list */
for (i =0; i<appdefsetup.num_used; i++)
    if (appdefsetup.remove_list[i] != 0)
	{
	int len;
	same = 1;
	len = strlen(appdefsetup.remove_list[i]);
	for (j=0; j<len; j++)
	    if (name[j] != appdefsetup.remove_list[i][j])
		{
		same = 0;
		break;
		}
        if (same == 1) return (False);
	}
/* add the new length to the old length.*/
*length = *length+strlen(name);
return (False);
}

/**************************************/
int store_data(db, bindings, quarks, type, value, buffer)
    XrmDatabase         *db;
    XrmBindingList      bindings;
    XrmQuarkList        quarks;
    XrmRepresentation   *type;
    XrmValue            *value;
    char		*buffer;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called once for every line in the database.
**	Call storeentry to construct the ascii representation of that line.
**	Then concatenate this line to the buffer.
**
**  FORMAL PARAMETERS:
**
**	bindings - The list of bindings - the binding will either be a * or a .
**	quarks - the list of quarks.  In the resource wm.foreground, there are
**	    two quarks - 1. wm and 2. foreground
**	type - could be any of the xlib defined types for resources - string,
**		int, etc...
**	value - the value of the resource.  Contains an ascii string and a size
**	buffer - the pointer to the buffer which will contain the database
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**	Assumes no resource string is greater than 10000 chars.
**--
**/
{
char	name[10000];
int i,j,same;

name[0] = 0;

/* get the ascii representation of this resource and value */
StoreEntry(bindings, quarks, *type, value, name);

/* don't store the resource which is on remove list */
for (i =0; i<appdefsetup.num_used; i++)
    if (appdefsetup.remove_list[i] != 0)
	{
	int len;
	same = 1;
	len = strlen(appdefsetup.remove_list[i]);
	for (j=0; j<len; j++)
	    if (name[j] != appdefsetup.remove_list[i][j])
		{
		same = 0;
		break;
		}
        if (same == 1) return (False);
	}

/* add this line to the buffer */
strcat(buffer, name);
return (False);
}

add_remove_list(count)
int count;
{
int i;

if (appdefsetup.remove_list == NULL)
    {
    int size;
    if (count > allocinc)
	size = count;
    else
	size = allocinc;	    
    appdefsetup.remove_list = (char **)XtMalloc(sizeof(char *) * size);
    appdefsetup.num_allocated = size;
    for (i=0; i<size; i++)
	appdefsetup.remove_list[i] = 0;
    appdefsetup.num_used = 0;
    }
else
    {
    if (appdefsetup.num_used == appdefsetup.num_allocated)
	{
	char    **newlist;
	int size;
	if (count > allocinc)
	    size = count;
	else
	    size = allocinc;	    
	newlist = (char **)XtMalloc(sizeof(char *) * 
			    (size + appdefsetup.num_allocated));
	for (i=0; i<appdefsetup.num_allocated; i++)
	    newlist[i] = appdefsetup.remove_list[i];
	for(i=appdefsetup.num_allocated; 
	    i<(size + appdefsetup.num_allocated);
		    i++)
	    newlist[i] = 0;
	appdefsetup.num_allocated = size + appdefsetup.num_allocated;
	XtFree((char *)appdefsetup.remove_list);
	appdefsetup.remove_list = newlist;
	}
     }
}

get_menu_command(label_text, retptr, issysptr)
char	*label_text;
char	**retptr;
int *issysptr;
{
    char	*command_request;
    char	*the_rep;
    XrmValue        the_value;

    /* Get the command of the menu which was selected */
    get_app_def_command(label_text, &command_request);

    *issysptr = 0;
    XrmGetResource(xrmdb.xrmlist[system_color_type][rdb_merge],
	           command_request, NULL, &the_rep, &the_value);
    if (the_value.size == 0) {
        XtFree(command_request);
        *retptr=NULL;
        return;
    }

    XtFree(command_request);
    /* It was there, copy the result to return storage */
    command_request = XtMalloc(the_value.size + 1);
    strncpy(command_request, the_value.addr,the_value.size);
    command_request[the_value.size] = 0;
    *retptr = command_request;
}

get_app_def_name(menu, retptr)
char	*menu;
char	**retptr;
{
char	*command_string = ".command";
char	*sm_string = "DXsession.";
char	*command_request;
char	*part1;
char	*part2;
int len;

/* Get the command of the menu which was selected */

part1 = strchr(menu, '.');
if (part1 == 0)
    {
    *retptr = NULL;
    return;
    }
part2 = strchr(part1+1, '.');
if (part2 == 0)
    {
    *retptr = NULL;
    return;
    }

len = part2-part1-1;
command_request = XtMalloc(len+1); 

strncpy(command_request, part1+ 1, len);
command_request[len] = 0;
*retptr = command_request;
}
