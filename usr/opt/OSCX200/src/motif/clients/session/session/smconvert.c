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
#define SS_NORMAL 1
#include "smdata.h"
#include "smresource.h"

int	str_to_int(strptr, value)
char	*strptr;
int	*value;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	converts a character string to an integer value
**
**  FORMAL PARAMETERS:
**
**	strptr - Pointer to a string to convert
**	value - Pointer to storage for result.
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
	*value = atoi(strptr);
	return(TRUE);
}

static	int	convertchar(strptr, value)
char	*strptr;
char	*value;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Convert the ascii representation of 1 character to a byte number
**
**  FORMAL PARAMETERS:
**
**	strptr - Pointer to the 1 character
**	value - Pointer to a place to return the number
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
	*value = (char) (atoi(strptr));
	return(1);
}


extern	int	int_to_str(value, result, maxsize)
unsigned	int	value;
char	*result;
unsigned	int	maxsize;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Convert an integer to a null terminated string
**
**  FORMAL PARAMETERS:
**
**	value - The integer to convert
**	result - Pointer to a string array to store the result
**	maxsize - The maximum size of the string to be returned
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
	(void) sprintf(result,"%d",value);
	return(TRUE);
}

extern	int	sm_convert_int(index, svalue, value)
int	index;
char	*svalue;
int	value[4];
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
		{
		/* one integer value */
		status = str_to_int(sptr, &value[0]);
		if (status != SS_NORMAL)
			return(0);
		break;
		}
	case(t2int):
		{
		/* two integers seperated by commas */
		tempptr = sptr;
		for (i=0; i<2; i++)
			{
			strindex = strcspn(tempptr, ",");
			firststr = (char *)malloc(strindex + 1);
			strncpy(firststr, tempptr, strindex);
			firststr[strindex] = NULL;
			status = str_to_int(firststr, &value[i]);
			if (status != SS_NORMAL)
				return(0);
			remaining = strlen(tempptr) - (strindex + 1);
			if ((remaining <= 0) && (i != 1))
				return(0);
			free(firststr);
			tempptr = tempptr + strindex + 1;
			}
		break;
		}
	case(t3int):
		{
		tempptr = sptr;
		for (i=0; i<3; i++)
			{
			strindex = strcspn(tempptr, ",");
			firststr = (char *)malloc(strindex + 1);
			strncpy(firststr, tempptr, strindex);
			firststr[strindex] = NULL;
			status = str_to_int(firststr, &value[i]);
			if (status != SS_NORMAL)
				return(0);
			remaining = strlen(tempptr) - (strindex + 1);
			if ((remaining <= 0) && (i != 2))
				return(0);
			free(firststr);
			tempptr = tempptr + strindex + 1;
			}
		break;
		}
	default:	return(0);
	}
return(1);
}	

extern	int	int_to_hexstr(value, result, maxsize)
unsigned	short	value;
char	*result;
unsigned	int	maxsize;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Convert an integer into a  string representing a hex number
**
**  FORMAL PARAMETERS:
**
**	value - The short value which is to be changes into string
**	result - The array to return the string
**	maxsize - The largest we want the string to be
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
sprintf(result, "%x", value);
return(SS_NORMAL);
}

extern	int	rgb_to_str(value, result)
XColor	value;
char	*result;
/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	Convert a color structure into a string which represents
**	the hex values for RGB.  For resources this represents
**	the first character as a # and four hex digits for red, four
**	for green and four for blue.
**
**  FORMAL PARAMETERS:
**
**	value - The color structure which has the RGB values
**	result - The array to return the string
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
unsigned	int	status, i,j;
unsigned    short   color;
char	c_array[20];
char	*pound = "#";

strcpy(result, pound);

/* Get the hex number for red */
color = value.red;
status = int_to_hexstr(color, c_array, sizeof(c_array));
if (status != SS_NORMAL)
    return(0);

for (i=strlen(c_array); i<4; i++) {
  strcat(result, "0");
}

strcat(result, c_array);

/* Get the hex number for green */
color = value.green;
status = int_to_hexstr(color, c_array, sizeof(c_array));
if (status != SS_NORMAL)
	return(0);

for (i=strlen(c_array); i<4; i++) {
  strcat(result, "0");
}

strcat(result, c_array);

/* Get the hex number for blue */
color = value.blue;
status = int_to_hexstr(color,c_array, sizeof(c_array));
if (status != SS_NORMAL)
	return(0);

for (i=strlen(c_array); i<4; i++) {
  strcat(result, "0");
}

strcat(result, c_array);

return(1);
}
