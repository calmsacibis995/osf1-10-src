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
#include <stdio.h>
#include <ctype.h>    /* needed for isgraph */
#define globalref extern
#define globaldef
#define noshare
#define SS_NORMAL 1
#include <sys/file.h>
#include "smshare.h"
#include <X11/Intrinsic.h>

char **ConvertDefResourceToList();
char **add_app_commands();

/*******************************************************/
int     sm_set_property(filename, pDisplay, userResourcesDB, rm_data)
char    *filename;
Display *pDisplay;
XrmDatabase *userResourcesDB;
struct  resourcedata    *rm_data;
{
FreeSaveList();                 /* A.R.  should this be here if reset fails */
FreeRemoveList();
*userResourcesDB = NULL;

if (reset_property(pDisplay, filename) == 0) return (0);
sm_put_property(pDisplay,*userResourcesDB, rm_data);
return(1);
}


store_properties(display, userResourcesDB, removelist, removecount)
Display	*display;
struct	resourcelist	userResourcesDB;
char	**removelist;
unsigned	int removecount;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Calls routines which read the correct resource files in and then
**	calls routines to put the resource database as a property on the
**	root window.
**
**  FORMAL PARAMETERS:
**
**	display - Pointer to the open display connection
**	userResourcesDB - an array of xrmDatabase pointers.  As we read in
**		   each database, the pointer to that database will be saved
**		   in this array.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**      Memory is allocated.
**	File are open/read/closed
**	A property is put on the root window
**--
**/
{
unsigned	int	status,i,numscreens,j,system_type;
XrmDatabase tempDB;
Window	root;

/* Get the number of screens on the system */
numscreens = XScreenCount(display);

/* For each screen, store the property*/
for (i=0; i < numscreens; i++)
    {
    root = XRootWindow(display, i);	
    system_type = determine_system_color(display,i);
    sm_put_property(display, 
			userResourcesDB.xrmlist[system_type][rdb_merge], 
			root, removelist, removecount);
    }
}

int	determine_system_color(display_id,screen_num)
Display	*display_id;
unsigned int screen_num;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Looks at the depth of the screen and determines if this is a
**	black/white, color, or gray scale machine.
**
**  FORMAL PARAMETERS:
**
**	display - Pointer to the open display connection
**	screen_num - Screen number to look at.
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**      none
**
**  SIDE EFFECTS:
**
**--
**/
{
Visual  *type;
Screen  *screen;

screen = XScreenOfDisplay(display_id,screen_num);
type = XDefaultVisualOfScreen(screen);

switch (type->class)
    {
    case   StaticGray:
        {
        if (XDefaultDepthOfScreen(screen) == 1)
	    {
	    return(black_white_system);
	    }
	else
	    {
	    return(black_white_system);
	    }
	break;
	}
    case    GrayScale:
	{
	return(gray_system);
	break;
	}
    case    StaticColor:
    case    PseudoColor:
    case    DirectColor:
    case    TrueColor:
	{
	return(color_system);
	break;
	}
    default:
	{
	return(black_white_system);
	}
    }
return(0);
}

char	**add_app_commands(display_id,xrm, resourcelist, count)
Display	*display_id;
XrmDatabase xrm;
char	**resourcelist;
unsigned    int	*count;
{
int i,j;
int nhosts=0;
char **hostlist;
char **newlist;
int count1 = 0;
char	*resource_name = "DXsession.applications";

/* Read the current settings from the resource file */
hostlist = ConvertDefResourceToList(display_id,xrm,&nhosts,0);

/* No defined applications*/
if (nhosts == 0)
    {
    return(resourcelist);
    }
newlist = (char **)malloc((nhosts + *count + 1) * sizeof(char *));
for (i=0; i<*count; i++)
    newlist[i] = resourcelist[i];
newlist[*count] = (char *)malloc(strlen(resource_name)+ 1);
strcpy(newlist[*count],resource_name);
for (i=0,j=(*count) + 1; i<nhosts; i++)
    {
    char    *command;
    int	dummy;
    get_app_def_command(hostlist[i], &command);
    if (command != NULL)
	{
	newlist[j] = (char *)malloc(strlen(command)+1);
	strcpy(newlist[j],command);
	XtFree(hostlist[i]);
	XtFree(command);
	count1++;
	j++;
	}
    }
XtFree((char *)hostlist);
free(resourcelist);
*count = *count + count1 + 1;
return(newlist);
}

char **ConvertDefResourceToList(display_id, xrm, nhosts_return, merge_system)
    Display *display_id;
    XrmDatabase xrm;
    int *nhosts_return;
    int	merge_system;
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
    char *resource1;    
    char tmp[80];
    int size = 0;
    int size1 = 0;
    int nbytes;
    int nhosts = 0;
    int nhosts1 = 0;
    int i;
    char **hostlist;
    char *the_rep1;
    XrmRepresentation	the_rep;
    XrmValue	the_value;
    XrmValue	the_value1;
    XrmDatabase app_database;
    
    app_database=XtDatabase(display_id);

    /* Get the string out of the user resource file*/
    XrmGetResource(xrm, "DXsession.applications", NULL, 
			&the_rep1, &the_value1);
    size1 = the_value1.size;
    if ((size <= 0) && (size1 <= 0))return (NULL);
    resource = the_value.addr;
    resource1 = the_value1.addr;

    /* walk through the list first time to find out how many
       element we are going to have */
    for (i=0; i<size; i++)
	if (resource[i] == ',')
	    nhosts++;
    for (i=0; i<size1; i++)
	if (resource1[i] == ',')
	    nhosts1++;
    /* we have one more element than we have commas */
    if(size != 0)
	nhosts++;
    if(size1 != 0)
	nhosts1++;

    /* Allocate memory for the list of names we are going to return */
    hostlist = (char **) XtMalloc ((nhosts + nhosts1) * sizeof(char *));

    i = 0;
    walkresource(size, resource, nhosts + nhosts1, hostlist, &i);
    walkresource(size1, resource1, nhosts + nhosts1, hostlist, &i);

    /* return the number of strings and the list */
    *nhosts_return = nhosts + nhosts1;
    return (hostlist);
}

walkresource(size, resource, nhosts, hostlist, index)
int size;
char	*resource;
int nhosts;
char **hostlist;
int *index;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Walk a resource string looking for the commas.
**	Return an array of strings, one for each
**	element in the comma separated resource string.
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
char *startpos;
char *endpos;
char *final_byte;
int i = *index;

if (size > 0)
    {
    final_byte = resource + size;
    startpos = resource;

    /* Search the string resource from the end of the string, looking
       for the first graphic char */
    while ( (!isgraph(*(final_byte-1))) || (*(final_byte-1) < 0) 
		    || (*(final_byte-1) > 127) ) {
	final_byte--;
	if (final_byte == startpos) {
	    break;
	}
    }
    if (final_byte != startpos)
	{
	/* pick off each menu name string */
	while (startpos < final_byte && i<nhosts) {
	    endpos = startpos+1;
	    while (*endpos != ',' && endpos != final_byte) endpos++;
	    hostlist[i] = (char *)XtMalloc (endpos-startpos+1);
	    strncpy (hostlist[i], startpos, endpos-startpos);
	    hostlist[i][endpos-startpos] = '\0';
	    i += 1;
	    startpos = endpos+1;
	    }
	}
    }
*index = i;
}

get_app_def_command(menu, retptr)
char	*menu;
char	**retptr;
{
    char	*command_string = ".command";
    char	*sm_string = "DXsession.";
    char	*command_request;

    /* Get the command of the menu which was selected */
    *retptr = (char *)XtMalloc(strlen(sm_string) + strlen(command_string) 
		        + strlen(menu) + 1);
    strcpy(*retptr, sm_string);
    strcat(*retptr, menu);
    strcat(*retptr, command_string);
}
