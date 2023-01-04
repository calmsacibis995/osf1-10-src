/*
*****************************************************************************
**									    *
**  COPYRIGHT (c) 1988, 1989, 1992 BY					    *
**  DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASSACHUSETTS.		    *
**  ALL RIGHTS RESERVED.						    *
** 									    *
**  THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND COPIED   *
**  ONLY IN  ACCORDANCE WITH  THE  TERMS  OF  SUCH  LICENSE  AND WITH THE   *
**  INCLUSION OF THE ABOVE COPYRIGHT NOTICE. THIS SOFTWARE OR  ANY  OTHER   *
**  COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE AVAILABLE TO ANY   *
**  OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF  THE  SOFTWARE IS  HEREBY   *
**  TRANSFERRED.							    *
** 									    *
**  THE INFORMATION IN THIS SOFTWARE IS  SUBJECT TO CHANGE WITHOUT NOTICE   *
**  AND  SHOULD  NOT  BE  CONSTRUED AS  A COMMITMENT BY DIGITAL EQUIPMENT   *
**  CORPORATION.							    *
** 									    *
**  DIGITAL ASSUMES NO RESPONSIBILITY FOR THE USE  OR  RELIABILITY OF ITS   *
**  SOFTWARE ON EQUIPMENT WHICH IS NOT SUPPLIED BY DIGITAL.		    *
** 									    *
**									    *
*****************************************************************************
**++
** FACILITY:  Session
**
** ABSTRACT:
**
**	This module converts from strings to different data structures.
**	It is used for resource management functions
**
** ENVIRONMENT:
**
**	VAX/VMS operating system.
**
** AUTHOR:  Karen Brouillette December 1987
**
** Modified by:
**
**	25-APR-1989	Jackie Ferguson
**	    Change VMS specific items for Ultrix.
**
**	01-FEB-1988	Karen Brouillette
**	    Update copyright, add module headers
*/

/*
** Include files
*/

#include "iprdw.h"

#ifdef VMS
#include <ssdef.h>
#endif /* VMS */

#ifdef OSF1
#include <string.h>
#endif

#include "prdw.h"
#include "smdata.h"
#include "smresource.h"
#include "prdw_entry.h"

static int convertchar PROTOTYPE((
    char	*strptr,
    char	*value
));

int str_to_int
#if _PRDW_PROTO_
(
    char	*strptr,
    int		*value
)
#else
(strptr, value)
    char	*strptr;
    int		*value;
#endif
{
    *value = atoi(strptr);
    return(Normal);
}

static int convertchar
#if _PRDW_PROTO_
(
    char	*strptr,
    char	*value
)
#else
(strptr, value)
    char	*strptr;
    char	*value;
#endif
{
    *value = (char) (atoi(strptr));
    return(Normal);
}

int int_to_str
#if _PRDW_PROTO_
(
    unsigned int	value,
    char		*result,
    unsigned int	maxsize
)
#else
(value, result, maxsize)
    unsigned int	value;
    char		*result;
    unsigned int	maxsize;
#endif
{
    (void) sprintf(result,"%d",value);
    return(Normal);
}

int sm_convert_int
#if _PRDW_PROTO_
(
    int		index,
    char	*svalue,
    int		value[4]
)
#else
(index, svalue, value)
    int		index;
    char	*svalue;
    int		value[4];
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Gets an integer value for a session manager resource.  The
**	names of the session manager resources are stored in a
**	table, along with default values for the resources.   If
**	svalue is passed in as NULL, we will get the default value
**	out of the table.  Otherwise, we will look at the format type
**	for the resource (in the table), and convert the given string
**	into however many integer values the format type specifies.
**
**  FORMAL PARAMETERS:
**
**	index - Index into the resource table
**	svalue - Points to the resource string, or can be NULL which
**		 signifys to use the default string from the table
**	value - Place to return the integers
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
char	*sptr;
unsigned	int	status, i;
char	*firststr,*tempptr;
int	strindex, remaining;

    /* check if we should use the default value.  This is flaged by svalue
	being NULL */
    if (svalue == NULL)
    {
	/* Get the default  resource string from the table */
	strcpy(astring, def_table[index].def_value);	
	sptr = astring;
    }
    else
    /* Use the value which was passed in */
	sptr = svalue;


    /* we could be looking for 1 integer. 2 seperated by commas, or 3
	seperated by commas.  */
    switch(def_table[index].format)
    {
    case(tint):
	/* one integer value */
	status = str_to_int (sptr, &value[0]);
	if (status != Normal) return(0);
	break;
    case(t2int):
	/* two integers seperated by commas */
	tempptr = sptr;
	for (i=0; i<2; i++)
	{
	    strindex = strcspn(tempptr, ",");
	    firststr = (char *)XtMalloc(strindex + 1);
	    strncpy(firststr, tempptr, strindex);
	    firststr[strindex] = 0;
	    status = str_to_int(firststr, &value[i]);
	    if (status != Normal) return(0);
	    remaining = strlen(tempptr) - (strindex + 1);
	    if ((remaining <= 0) && (i != 1)) return(0);
	    XtFree (firststr);
	    tempptr = tempptr + strindex + 1;
	}
	break;
    case(t3int):
	tempptr = sptr;
	for (i=0; i<3; i++)
	{
	    strindex = strcspn(tempptr, ",");
	    firststr = (char *)XtMalloc(strindex + 1);
	    strncpy(firststr, tempptr, strindex);
	    firststr[strindex] = 0;
	    status = str_to_int(firststr, &value[i]);
	    if (status != Normal) return(0);
	    remaining = strlen(tempptr) - (strindex + 1);
	    if ((remaining <= 0) && (i != 2)) return(0);
	    XtFree (firststr);
	    tempptr = tempptr + strindex + 1;
	}
	break;
    default:	return(0);
    }
    return(1);
}	

int sm_convert_str
#if _PRDW_PROTO_
(
    unsigned int	index,
    char		*source,
    char		schar[4]
)
#else
(index, source, schar)
    unsigned int	index;
    char		*source;
    char		schar[4];
#endif
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Gets a string value for a session manager resource.  The
**	names of the session manager resources are stored in a
**	table, along with default values for the resources.   If
**	source is passed in as NULL, we will get the default value
**	out of the table.  Otherwise, we will look at the format type
**	for the resource (in the table), and return the  string
**
**  FORMAL PARAMETERS:
**
**	index - Index into the resource table
**	source - Points to the resource string, or can be NULL which
**		 signifys to use the default string from the table
**	schar - Place to return the characters
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
    char	*sptr;
    unsigned	int	status, i, max;
    char	*tempptr;
    int	strindex, remaining;

    /* check if we should use the default value.  This is flaged by svalue
	being NULL */

    if (source == NULL)
    {
	strcpy(astring, def_table[index].def_value);	
	sptr = astring;
    }
    else
	sptr = source;


    /* we could be looking for 1 char, 2 seperated by commas, or 3
	seperated by commas.  */

    switch(def_table[index].format)
    {
    case(t2char):
	max = 2;
	break;
    case(t3char):
	max = 3;
	break;
    case(t4char):
	max = 4;
	break;
    default:
	return(0);
    }

    tempptr = sptr;
    for (i=0; i<max; i++)
    {
	strindex = strcspn(tempptr, ",");
	if (strindex > 1) return(0);
	convertchar(tempptr,&schar[i]);
	remaining = strlen(tempptr) - (strindex + 1);
	if ((remaining <= 0) && (i != (max - 1))) return(0);
	tempptr = tempptr + strindex + 1;
    }
    return (Normal);
}

int int_to_hexstr
#if _PRDW_PROTO_
(
    unsigned short	value,
    char		*result,
    unsigned int	maxsize
)
#else
(value, result, maxsize)
    unsigned short	value;
    char		*result;
    unsigned int	maxsize;
#endif
{
    sprintf(result,"%x",value);
    return (Normal);
}
