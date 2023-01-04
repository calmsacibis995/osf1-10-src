/* #module xwdio.c "v1.0"
 *
 *  Copyright (c) Digital Equipment Corporation, 1990
 *  All Rights Reserved.  Unpublished rights reserved
 *  under the copyright laws of the United States.
 *  
 *  The software contained on this media is proprietary
 *  to and embodies the confidential technology of 
 *  Digital Equipment Corporation.  Possession, use,
 *  duplication or dissemination of the software and
 *  media is authorized only pursuant to a valid written
 *  license from Digital Equipment Corporation.
 *
 *  RESTRICTED RIGHTS LEGEND   Use, duplication, or 
 *  disclosure by the U.S. Government is subject to
 *  restrictions as set forth in Subparagraph (c)(1)(ii)
 *  of DFARS 252.227-7013, or in FAR 52.227-19, as
 *  applicable.
 *
 *
 * FACILITY:
 *	DECwindows Print Screen
 *
 * ABSTRACT:
 *	Various routines to handle getting and storing resources
 *
 * NOTES:
 *	Inherited from Session Manager code (whe Print Screen was
 * 	built in). 
 *
 * REVISION HISTORY:
 */

/*
** Include files
*/
#ifdef vms
#include <stdio.h>
#include <iodef.h>
#include <lnmdef.h>
#include <file.h>
#include <psldef.h>
#include <jpidef.h>
#include <ssdef.h>
#endif

#include "iprdw.h"
#include "prdw.h"
#include "smdata.h"
#include "smconstants.h"
#include "smresource.h"
#if defined(VMS) && !defined(__DECC)
#pragma nostandard
#endif
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/Vendor.h>
#include <DXm/DECspecific.h>
#ifdef vms
#include <decw$cursor.h>
#else
#include <X11/decwcursor.h>
#endif
#if defined(VMS) && !defined(__DECC)
#pragma standard
#endif
#include "prdw_entry.h"
#define	username_size	40

int sm_get_resource
#if _PRDW_PROTO_
(
    char		*name,
    char		*value
)
#else
(name, value)
    char		*name;
    char		*value;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a resource value from the database and return its value
**
**  FORMAL PARAMETERS:
**
**	name - A pointer to the resource we are looking for.  
**	value - A pointer to a string to return the value of the resource
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**	Returns length of the string or 0 if there was a problem
**
**  SIDE EFFECTS:
**
**	We don't make any checks to see that we are overwriting memory
**	when we copy this string into the return space.
**
**--
**/
{
    XrmRepresentation	the_rep;
    XrmValue	the_value;
    char	*get_ptr;

    /*
    ** We don't want to look for * when looking for the resource.  Strip that
    ** off
    */
    get_ptr = name;
    if ((*get_ptr) == '*')
    {
	get_ptr++;
    }

    /* Get the resource out of the xdefaults database */
    XrmGetResource(xrmdb, get_ptr, NULL, (char **)&the_rep, &the_value);
    if (the_value.size != 0)
    {
	/* It was there, copy the result to return storage */
	strncpy(value, the_value.addr, the_value.size);
	value[the_value.size] = '\0';
	return(the_value.size);
    }
    return(0);
}

int sm_get_int_resource
#if _PRDW_PROTO_
(
    unsigned int	index,
    int			intptr[4]
)
#else
(index, intptr)
    unsigned int	index;
    int			intptr[4];
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a resource value from the database and return its integer value
**
**  FORMAL PARAMETERS:
**
**	index - Index into the table which lists resource names.  
**	intptr - Place to return the integer(s) that are the resource value
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
char	svalue[256];
unsigned    int	size;

/* Get the resource from the database */
size = sm_get_resource(def_table[index].name,svalue);
/* If we couldn't find the resource, then use the default value.  The
   default value will be stored in the table */
if (size == 0)
    {
    size = sm_convert_int(index, NULL, intptr);
    }
else
    {
    /* We found the resource.  Convert the string to an int */
    size = sm_convert_int(index, svalue, intptr);
    if (size == 0)
	{
	/* If we have problems converting, use the default table value */
	size = sm_convert_int(index, NULL, intptr);
	}
    }
return(1);
}

int sm_get_string_resource
#if _PRDW_PROTO_
(
    unsigned int	index,
    char		*charptr
)
#else
(index, charptr)
    unsigned int	index;
    char		*charptr;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a resource value from the database and return its value
**
**  FORMAL PARAMETERS:
**
**	index - Index into the table which lists resource names.  
**	charptr - Place to return the string that is the resource value
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**	size - The size of the string returned
**
**  SIDE EFFECTS:
**
**	We don't make any checks to see that we are overwriting memory
**	when we copy this string into the return space.
**
**--
**/
{
unsigned int	size;
char		*temp;

/* Get the resource */
size = sm_get_resource(def_table[index].name,charptr);
if (size == 0)
    {
    /* If couldn't find it, just use the default table value */
    temp = strcpy(charptr, def_table[index].def_value);
    size = strlen(temp);
    }
return(size);
}

int sm_get_any_resource
#if _PRDW_PROTO_
(
    unsigned int	index,
    char		*charptr,
    int			intptr[4]
)
#else
(index, charptr, intptr)
    unsigned int	index;
    char		*charptr;
    int			intptr[4];
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Look in the resource table to find the format of a particular
**	resource.  If it is integer or string, call the appropriate
**	routine and return the value as either an integer or string.
**
**  FORMAL PARAMETERS:
**
**	index - Index into the resource table of the resource we want
**	charptr - Place to return string resources
**	intptr - Place to return integer resources
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**	We don't make any checks to see that we are overwriting memory
**	when we copy this string into the return space.
**
**--
**/
{
unsigned    int	size;

/* String resources */
if (def_table[index].format == tstring)
    {
    size = sm_get_string_resource(index, charptr);
    }
/* Integer resources */
if (def_table[index].format == tint)
    {
    size = sm_get_int_resource(index, intptr);
    }
/* Two integers */
if (def_table[index].format == t2int)
    {
    size = sm_get_int_resource(index, intptr);
    }
return(1);
}

void sm_put_resource
#if _PRDW_PROTO_
(
    unsigned int	index,
    char		*value
)
#else
(index, value)
    unsigned int	index;
    char		*value;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Store a resource value.  We maintain two databases.  One with
**	all of the resources merged together, and one with the current
**	values seperated by file they are stored in (general or color
**	specific).   Update both databases.
**
**  FORMAL PARAMETERS:
**
**	index - index into the resource table
**	value - The string value to store for the resource
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
    char	*name;
    unsigned int	i,numscreens,j;

    /* Get the name of the resource out of the table */
    name = def_table[index].name;

    if (xrmdb != 0)
	XrmPutStringResource(&xrmdb, name, value);
    return;
}

void sm_put_int_resource
#if _PRDW_PROTO_
(
    unsigned int	index,
    int			value
)
#else
(index, value)
    unsigned int	index;
    int			value;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Store a resource value.  We maintain two databases.  One with
**	all of the resources merged together, and one with the current
**	values seperated by file they are stored in (general or color
**	specific).   Update both databases.
**
**  FORMAL PARAMETERS:
**
**	index - index into the resource table
**	value - The integer value to store for the resource
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
    char	astring[256];
    unsigned    int	status;

    /* change the integer value into a string */
    status = int_to_str(value, astring, sizeof(astring));

    /* Now store the string */
    if (status == Normal)
    {
	sm_put_resource(index, astring);
    }
    return;
}

int sm_get_screen_resource
#if _PRDW_PROTO_
(
    char		*name,
    unsigned int	screen_type,
    char		*value
)
#else
(name, screen_type, value)
    char		*name;
    unsigned int	screen_type;
    char		*value;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a resource value from the database and return its value
**
**  FORMAL PARAMETERS:
**
**	name - A pointer to the resource we are looking for.  
**	value - A pointer to a string to return the value of the resource
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**  COMPLETION CODES:
**
**	Returns length of the string or 0 if there was a problem
**
**  SIDE EFFECTS:
**
**	We don't make any checks to see that we are overwriting memory
**	when we copy this string into the return space.
**
**--
**/
{
    XrmRepresentation	the_rep;
    XrmValue	the_value;
    char	*get_ptr;
    int type;

    /*
    ** We don't want to look for * when looking for the resource.  Strip that
    ** off
    */
    get_ptr = name;
    if ((*get_ptr) == '*')
    {
	get_ptr++;
    }


    XrmGetResource(xrmdb, get_ptr, NULL, (char **)&the_rep, &the_value);
    if (the_value.size != 0)
    {
	/* It was there, copy the result to return storage */
	strncpy(value, the_value.addr, the_value.size);
	value[the_value.size] = '\0';
	return(the_value.size);
    }
    return(0);
}

int sm_get_int_screen_resource
#if _PRDW_PROTO_
(
    unsigned int	index,
    unsigned int	screen_type,
    int			intptr[4]
)
#else
(index, screen_type, intptr)
    unsigned int	index;
    unsigned int	screen_type;
    int			intptr[4];
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Get a resource value from the database and return its integer value
**
**  FORMAL PARAMETERS:
**
**	index - Index into the table which lists resource names.  
**	intptr - Place to return the integer(s) that are the resource value
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
char	svalue[256];
unsigned    int	size;

/* Get the resource from the database */
size = sm_get_screen_resource(def_table[index].name,screen_type, svalue);
/* If we couldn't find the resource, then use the default value.  The
   default value will be stored in the table */
if (size == 0)
    {
    size = sm_convert_int(index, NULL, intptr);
    }
else
    {
    /* We found the resource.  Convert the string to an int */
    size = sm_convert_int(index, svalue, intptr);
    if (size == 0)
	{
	/* If we have problems converting, use the default table value */
	size = sm_convert_int(index, NULL, intptr);
	}
    }
return(1);
}

char *CSToLatin1
#if _PRDW_PROTO_
(
    XmString cs
)
#else
(cs)
    XmString cs;
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      Convert a compound string to a null terminated string
**
**  FORMAL PARAMETERS:
**
**      cs - Pointer to a compound string
**
**  IMPLICIT INPUTS:
**
**  IMPLICIT OUTPUTS:
**
**      Returns a pointer to a compound string
**
**  COMPLETION CODES:
**
**  SIDE EFFECTS:
**
**--
**/
{
    long byte_count, cvt_status;
    return ((char *)DXmCvtCStoFC(cs, &byte_count, &cvt_status));
}
