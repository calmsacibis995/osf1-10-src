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
static char	*sccsid = "@(#)$RCSfile: strtod.c,v $ $Revision: 4.2.11.4 $ (DEC) $Date: 1993/11/09 20:53:41 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Modification History
 * 001  DECwest ANSI dls 019
 *      Ensure that endptr in strtod does not change unless we can
 *      return a valid number. The endptr was being modified if we scanned
 *      white space but did not find a number.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <sys/localedef.h>
#include <ctype.h>
#include <stdio.h>
#include <nl_types.h>
#include <langinfo.h>

extern double atof();

/* strtod
 *
 *	Convert string to double.  This is the ULTRIX implementation of
 *	the System V strtod() routine.  This routine simply parses the
 *	string and then calls atof() to actually perform the conversion.
 *	This allows all the details of GFLOAT/DFLOAT conversion to be
 *	hidden in atof().
 */

double strtod(const char *str, char **ptr)
	/* char	*str,  string to convert */
	/*	**ptr;	 return pointer for terminating character */
{
	register char	*scanptr = str ;
	char   *radix;        /* radix character from locale info  */
	double __actual_atof();
	/* Skip whitespace then parse fraction.
	 */
	radix = __lc_locale->nl_info[RADIXCHAR];
	if ((radix == 0) || !(*radix))
	     radix = ".";
	while (isspace(*scanptr)) scanptr++;

	if (*scanptr == '+' || *scanptr == '-') scanptr++ ;

	/*
	 * Ensure string can be converted
	 */
	if(isdigit(*scanptr) ||
	                   (*scanptr == *radix && isdigit(scanptr[1]))){
	    while (isdigit(*scanptr)) scanptr++;

	    if (*scanptr == *radix) {
		scanptr++;
		while (isdigit(*scanptr)) scanptr++;
	    }

	    /* Parse exponent.
	     */
	    if (*scanptr == 'E' || *scanptr == 'e') {

		char	*e_ptr = scanptr++; /* save ptr to 'E'|'e' */

		if (*scanptr == '+' || *scanptr == '-') scanptr++ ;

		if (isdigit(*scanptr++)) 
			while (isdigit(*scanptr)) scanptr++;
		else
			/* Return pointer to beginning of 'E'|'e' if
			 * this wasn't a well formed exponent.
			 */
			scanptr = e_ptr ;
	    }
	} else
	    scanptr = str;
	
	/* Return pointer to terminating character
	 */
	if (ptr != NULL) *ptr = scanptr;

	/* Return converted string
	 */
	return(__actual_atof(str));
}

