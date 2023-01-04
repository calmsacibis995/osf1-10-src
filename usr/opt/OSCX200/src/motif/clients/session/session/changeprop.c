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
#include <sys/file.h>
#include <strings.h>
#define globalref extern
#define globaldef
#define noshare
#define SS_NORMAL 1
#include	<X11/Xatom.h>
#include "smshare.h"

extern int	count_items();

static noshare XrmQuark XrmQString;

char *user_resource = ".Xdefaults";
char *system_resource = "/usr/lib/X11/app-defaults/Xdefaults";

static noshare struct   buf_data
{
    char    *buffer;
    char        **removelist;
    unsigned    int removecount;
} bufdata;

put_property(pDisplay)
Display *pDisplay;
{
        if(reset_property(pDisplay, user_resource) == 0)
          if(reset_property(pDisplay,system_resource)== 0)
            return(0);
        return(1);
}

extern  int     sm_put_property(pDisplay,userResourcesDB, rm)
Display *pDisplay;
XrmDatabase userResourcesDB;
struct  resourcedata    *rm;

{

        do_merge_to_server(pDisplay);

        XSync(pDisplay, 0);
        return(SS_NORMAL);
}

/**************************************/
int store_items(bindings, quarks, type, value, data)
    XrmBindingList      bindings;
    XrmQuarkList        quarks;
    XrmRepresentation   type;
    XrmValuePtr         value;
    struct  buf_data	*data;
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
unsigned int i,j,same;
name[0] = 0;

/* get the ascii representation of this resource and value */
StoreEntry(bindings, quarks, type, value, name);

/* weed out the session manager specific resources. - They
   begin with "DXsession." */
for (i =0; i<data->removecount; i++)
    if (data->removelist[i] != 0)
        {
        int len;
        same = 1;
        len = strlen(data->removelist[i]);
        for (j=0; j<len; j++)
            if (name[j] != data->removelist[i][j])
                {
                same = 0;
                break;
                }
        if (same == 1) return (False);
        }
/* add this line to the buffer */
strcat(data->buffer, name);
return (False);
}

int StoreBindingQuarkList(bindings, quarks, stream)
    XrmBindingList      bindings;
    XrmQuarkList        quarks;
    char		*stream;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Every resource is broken down in to the specification of
**	the resource and the value of the resource.  The specification
**	of the resource is stored in something refered to as the
**	quarklist.   For each element of the specification, there will
**	be one quark (and one binding??).  For example:  the
**	specification of "wm.foreground", has two quarks - 1. wm and
**	2. foreground.  It also has one binding (".").   This routine
**	walks the quarklist and the binding list to create an ascii
**	string representing this resource.  For example:  from the
**	above example it would create "wm.foreground:".
**
**  FORMAL PARAMETERS:
**
**	bindings - The list of bindings - the binding will either be a * or a .
**	quarks - the list of quarks.  In the resource wm.foreground, there are
**	    two quarks - 1. wm and 2. foreground
**	stream - a pointer to the buffer to store the final ascii string.
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
Bool        firstNameSeen;

/* look at each quark and build the string */
for (firstNameSeen = False; (*quarks) != NULLQUARK; bindings++, quarks++) 
    {
    if (*bindings == XrmBindLoosely) 
	{
	strcat(stream, "*");
	} 
    else 
	if (firstNameSeen) 
	    /* if we have seen the first name add a . to the
	       name specification */
	    {
	    strcat(stream, ".");
	    }
    /* mark that we have seen the first part of the name*/
    firstNameSeen = True;
    /* add the first part of the name to the buffer */
    strcat(stream,XrmQuarkToString(*quarks));
    }
} 


int StoreEntry(bindings, quarks, type, value, stream)
    XrmBindingList      bindings;
    XrmQuarkList        quarks;
    XrmRepresentation   type;
    XrmValuePtr         value;
    char		*stream;
{
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Called once for every line in the database.
**	Calls storebindinglist to get a buffer with the
**      binding list ascii string.  Then formats
**	the value part of the resource into an
**	ascii string.  Special case for multi-line
**	resources
**
**  FORMAL PARAMETERS:
**
**	bindings - The list of bindings - the binding will either be a * or a .
**	quarks - the list of quarks.  In the resource wm.foreground, there are
**	    two quarks - 1. wm and 2. foreground
**	type - could be any of the xlib defined types for resources - string,
**		int, etc...
**	value - the value of the resource.  Contains an ascii string and a size
**	stream - the pointer to the buffer to hold the final resource string
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

register unsigned int       i;

/* walk the binding structure to get the resource name description*/
StoreBindingQuarkList(bindings, quarks, stream);
/* if it is a string resource */
if (type == XrmQString) 
    {
    /* simple string type.  Put in the value and the newline char */
    if (index(value->addr, '\n') == NULL)
	{
	strcat(stream,":\t");
	strcat(stream,value->addr);
	strcat(stream,"\n");
	}
    else {
	/* multiline string type.  Put in the value and the newline chars
	   and the continuation char */
	register char *s1, *s2;
	strcat(stream,":\t\\\n");
	s1 = value->addr;
	while ((s2 = index(s1, '\n')) != NULL) 
	    {
	    *s2 = '\0';
	    if (s2[1] == '\0') 
		{
		strcat(stream,s1);
		strcat(stream,"\\n\n"); 
		} 
	    else 
		{
		strcat(stream,s1);
		strcat(stream,"\\n\\\n"); 
		}
	    *s2 = '\n';
	    s1 = s2 + 1;
	    }
	    strcat(stream,s1);
	    strcat(stream,"\\n\n"); 
	}
    } 
else 
    {
    /* handle non string type. */
    strcat(stream, "!");
    strcat(stream,  XrmRepresentationToString(type));
    strcat(stream, "\t");
    for (i = 0; i < value->size; i++)
	{
	char    temp[3];
	(void) sprintf(temp, "%02x", (int) value->addr[i]);
	temp[2] = 0;
	strcat(stream,temp);
	}
    /* handle multiline non-string type */
    if (index(value->addr, '\n')) 
	{
	strcat(stream, ":\t\\\n");
	for (i = 0; value->addr[i]; i++) 
	    {
	    if (value->addr[i] == '\n') 
		{
		strcat(stream, "\\n");
		if (value->addr[i+1]) strcat(stream, "\\");
		strcat(stream, "\n");
		(void) strcat(stream, "\n");
		} 
	     else 
		{
		(void) strncat(stream,value->addr[i], 1);
		}
	    }
	} 
     else 
	{
	strncat(stream, ":\t");
	strncat(stream, value->addr);
	strncat(stream, "\n");
	}
   }
}



int	init_static_quark()
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**  Initialize the static quark data
**
**  FORMAL PARAMETERS:
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
**--
**/

{
XrmQString = XrmStringToQuark("String");
}


