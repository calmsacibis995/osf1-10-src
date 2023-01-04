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
#ifndef lint
static char *rcsid = "@(#)$RCSfile: getsubopt.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/08/23 20:37:13 $";
#endif
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak getsubopt = __getsubopt
#endif
#include <stdio.h>
#include <string.h>

/*
 *  Routine: getsubopt
 *
 *  Function: parses suboptions within an option string
 *
 *  Arguments:
 *	**optionp	a pointer to a pointer to the option string
 *	*tokens[]	a pointer to an array of pointers to valid token
 *			strings.
 *	**valuep	a pointer to the location to put the pointer to
 *			the string indicating some value for a suboption.
 *
 *  Returns:
 *	The index into the tokens[] array which matches the suboption, or -1
 *	if there was no match.
 */
int
getsubopt(char **optionp, char *tokens[], char **valuep) 
{
	int i=0;

	char *equal_sgn;		/* Remembers where '=' in string is */
	char *option;			/* Keeps matching option */
	char *comma;			/* Find end of option */

	if ( !optionp || !*optionp || !tokens ) {
	    				/* Null pointer or pointer to NULL */
	    if (valuep)			/* Be careful */
	      *valuep = NULL;		/* Indicate no value assignment */
	    return (-1);		/* ..and no matches either */
	}

	option = *optionp;		/* Remember where we were */
	comma = strchr(option, ',');	/* Look for end of option */
	if ( !comma )			/* Is this the _last_ option? */
	    *optionp = option +
	      	strlen(option);		/* YES - point at NUL */
	else {				/* NO */
	    *comma++ = '\0';		/* Break apart string */
	    *optionp = comma;		/* Update to start of next option */
	}

	equal_sgn = strchr(option,'=');	/* Check for a key=value */
	if ( equal_sgn ) {		/* Got one */

	    *equal_sgn++ = '\0';	/* Make two separate strings */
	}
	if (valuep) *valuep = equal_sgn; /* Either NULL or value string */

	for(i=0; tokens[i]; i++) { 	/* Search supplied table for a match */
	    if ( !strcmp(option, tokens[i]) )
	      break;			/* Got it! */
	}

	if ( !tokens[i] ) {		/* Ran off the end of the table? */
	    *valuep = &*option;		/* set value to search string 
					   for SVR4 compat */
	    return (-1);		/*  return failure */
	}

	return (i);
}

