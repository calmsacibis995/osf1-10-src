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
 * $XConsortium: setvalues.c,v 1.4 91/03/20 17:08:49 gildea Exp $
 *
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Chris D. Peterson, MIT X Consortium
 */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xresource.h>

#include <stdio.h>

#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Cardinals.h>	
#include <X11/Xfuncs.h>
#include <X11/Xos.h>
#include "editresP.h"

extern ScreenData global_screen_data;

extern WNode * FindNode();
extern void AddString();

#define RESOURCE_NAME ("name")
#define RESOURCE_CLASS ("Class")

/*	Function Name: PrintSetValuesError
 *	Description: Allow the SetValues error to be printed.
 *	Arguments: event - the set values call that caused this event.
 *	Returns: str - a string contining the errors.
 */

char *
PrintSetValuesError(event)
Event * event;
{
    char * errors = NULL;
    WNode * node;
    int i;
    SetValuesEvent * sv_event = (SetValuesEvent *) event;
    char buf[BUFSIZ];
    Arg args[1];
    char **value_ptr;

    if (sv_event->num_entries == 0)
	{
	/*
	 * The SetValues was successful.  Update the stored resource values.
	 * First, make sure we have stored values by checking the protocol version.
	 */
	if (global_client.protocol_version == GETVALUES_PROTOCOL_VERSION)
	  for (i = 0; i < global_tree_info->num_nodes; i++)
	    {
	    node = global_tree_info->active_nodes[i];
	    if (node->resources != NULL)
		{
		value_ptr = NULL;
		for (i = 0; (i < node->resources->num_normal) && (value_ptr == NULL); i++)
		    if (0 == strcmp(global_tree_info->res_name, node->resources->normal[i].name))
		   	value_ptr = &(node->resources->normal[i].value);
		for (i = 0; (i < node->resources->num_constraint) && (value_ptr == NULL); i++)
		    if (0 == strcmp(global_tree_info->res_name, node->resources->constraint[i].name))
	 		value_ptr = &(node->resources->constraint[i].value);
	  	if (value_ptr != NULL)
		    {
		    XtFree(*value_ptr);
		    *value_ptr = XtMalloc(strlen(global_tree_info->res_value) + 1);
		    strcpy(*value_ptr, global_tree_info->res_value);
		    }
		}
	    }
	return(XtNewString("SetValues was Successful."));
	}

    for (i = 0 ; i < (int)sv_event->num_entries ; i++) {
	node = FindNode(global_tree_info->top_node,
			sv_event->info[i].widgets.ids, 
			sv_event->info[i].widgets.num_widgets);

	if (node == NULL) {
	    sprintf(buf, "Editres Internal Error: Unable to FindNode.\n");
	    AddString(&errors, buf); 
	    continue;
	}

	sprintf(buf, "%s(0x%lx) - %s\n", node->name, node->id,
		sv_event->info[i].message);
	AddString(&errors, buf);
    }
    return(errors);
}

/*	Function Name: GetResourceValueForSetValues(node);
 *	Description: Returns the value that should be sent to SetValues.
 *	Arguments: node - the node which contains the resource box.
 *	Returns: value - allocated value.
 */

char *
GetResourceValueForSetValues(node, size)
WNode * node;
unsigned short * size;
{
    Arg args[1];
    char *ptr, *temp;
    XrmDatabase db = NULL;
    XrmValue value;

    XtSetArg(args[0], XtNstring, &ptr);
    XtGetValues(node->resources->res_box->value_wid, args, ONE);

    /*
     * This makes sure that exactly the same thing happens during a set
     * values, that would happend of we were to insert this value into
     * the resource database.
     */

    temp = XtMalloc(sizeof(char) * (strlen(ptr) + strlen(RESOURCE_NAME) + 2));
    sprintf(temp, "%s:%s", RESOURCE_NAME, ptr);
    XrmPutLineResource(&db, temp);
    XtFree(temp);

    XrmGetResource(db, RESOURCE_NAME, RESOURCE_CLASS, &temp, &value);

    ptr = XtMalloc(sizeof(char) * value.size);
    bcopy(value.addr, ptr, value.size);
    XrmDestroyDatabase(db);
    
    *size = (unsigned short) value.size;
    return(ptr);
}
