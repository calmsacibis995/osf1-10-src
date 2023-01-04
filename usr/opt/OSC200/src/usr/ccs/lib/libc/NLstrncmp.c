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
static char	*sccsid = "@(#)$RCSfile: NLstrncmp.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:18:42 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * IBM CONFIDENTIAL
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * NLstrncmp.c	1.9  com/lib/c/str,3.1,9013 2/11/90 17:35:30
 */
/*
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */ 
/*
 * FUNCTION: Compares at most n bytes of the strings pointed to by s1 and s2, returning an
 *	     integer as follows:
 *
 *		Less than 0	If s1 is less than s2
 *		Equal to 0	If s1 is equal to s2
 *		Greater than 0	If s1 is greater than s2.
 *
 *	     The comparison is based on the collating sequence specified
 *	     by the locale category LC_COLLATE affected by the setlocale
 *	     function.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS: (Uses file codes )
 *	     char *s1 - first string
 *	     char *s2 - second string
 *           in n - length
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */
/*LINTLIBRARY*/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NCstrncmp = __NCstrncmp
#pragma weak NLstrncmp = __NLstrncmp
#endif
#endif
#include <sys/types.h>
#include  <NLchar.h>


#define	chnext(s,e)	    ( s > e ? '\0' \
				: (NCisshift(s[0]) ? (s+1 > e ? '\0' \
					: (s+=2, _NCd2(s[-2], s[-1]))) : *s++))
#define	extcol(s, e, ch, cu)   ((cu = NCcoluniq(ch = chnext(s,e))), NCcollate(ch))


/* The code consists of two loops: the first compares the two strings based
 * on primary weight, the second compares based on secondary weights (if there
 * were any).
 * In each loop we fetch the desired value for a char ( or collation element)
 * from each string, discarding "ignore" characters, and compare them.
 * We exit from the loops if either string terminates (i.e., the other is
 * longer or we hit an inequality.
 * If the first loop is exited with both strings equal and we found a char
 * with secondary weights (NCcollate value different from NCcoluniq value),
 * then we enter the second loop. In this we only compare NCcoluniq values
 * from the two strings if the char has secondary weight.
 * The NLxcolu routine is entered if the primary collation value is -1
 * (extended collation), and returns either a collation value  (n-to-1),
 * or a -1 for replacement string. In the last case, the 'p' parameter
 * now points to the first character of the replacement string.
 */

#ifdef _NO_PROTO
int NLstrncmp(s1, s2, n)
char *s1;
char *s2;
int n;
#else
int	
NLstrncmp(const char *s1, const char *s2, int n)
#endif
{
	int co1, co2;			/* primary collation value */
	int cu1, cu2;			/* secondary collation value */
	int secflag = 0;
	char *e1, *e2;
	char *sav1, *sav2;
	wchar_t ch1, ch2;
	wchar_t *p1, *p2;		/* pointers for NLxcolu's use */
	wchar_t *x1, *x2;		/* pointers for NLxcolu's use */

	if (s1 == s2)
		return (0);

	p1 = p2 = NULL;
	sav1 = (char *)s1;			/* save string pointers for possible */
	sav2 = (char *)s2;			/* second pass */
	e1 = (char *)s1 + n - 1;
	e2 = (char *)s2 + n - 1;
	

	do {
		co1  = 0;			/* so first pass will work */
		while  (co1 == 0) 
					/* loop until non-zero */
		{
					/* The if extracts co and cu and */
					/* evaluates to TRUE only if it is */
					/* 1-to-n mapping */
		    if (p1 != NULL || (co1 = extcol(s1, e1, ch1, cu1)) < 0 &&
		       (co1 = _NLxcolu(co1, &s1, &p1, &cu1)) == -1) {
					/* This if is for the case where the replacement */
					/* char is part of an extended collation (except */
					/* replacement, because we check that in ctab) */
			    if ((co1 = NCcollate(*p1)) < 0) {
				 p1++;
				 co1 = _NLxcolu(co1, &p1, &x1, &cu1);
			    } else 
				 cu1 = NCcoluniq(*p1++);

			    if (*p1 == '\0')	/* at end of 1-to-n str */ 
				p1 = NULL;  
			}
		}

		co2 = 0;
		while (co2 == 0)
		{
		    if (p2 != NULL || (co2 = extcol(s2, e2, ch2, cu2)) < 0 &&
		       (co2 = _NLxcolu(co2, &s2, &p2, &cu2)) == -1) {
			    if ((co2 = NCcollate(*p2)) < 0) {
				 p2++;
				 co2 = _NLxcolu(co2, &p2, &x2, &cu2);
			    } else 
				 cu2 = NCcoluniq(*p2++);
			    if (*p2 == '\0')
				p2 = NULL;
		    }
		}
		if ((co1 != cu1) || (co2 != cu2))
			secflag = 1;		/* we found a sec ordering */

	} while (ch1 != '\0' && ch2 != '\0' && co1 == co2);

	if (((ch1  != '\0') || (ch2 != '\0')) || (secflag == 0))
		return (co1 - co2);

 /* At this point, we have two strings that collate equal on their primary
  * collation values. We now go back to the beginning of the strings and
  * recompare them based on secondary collation values (if any)...
  */


	p1 = p2 = NULL;
	s1 = sav1;
	s2 = sav2;

	do {
		co1 = co2 =0;
		while ((co1 == 0) || (co1 == cu1)) 
		{
		    if (p1 != NULL || (co1 = extcol(s1, e1, ch1, cu1)) < 0 &&
		       (co1 = _NLxcolu(co1, &s1, &p1, &cu1)) == -1) {
			    co1 = NCcollate(*p1);
			    cu1 = NCcoluniq(*p1++);
			    if (*p1 == '\0')
				p1 = NULL;
		    }
		}

		while ((co2 == 0) || (co2 == cu2))
		{
		    if (p2 != NULL || (co2 = extcol(s2, e2, ch2, cu2)) < 0 &&
		       (co2 = _NLxcolu(co2, &s2, &p2, &cu2)) == -1) {
			    co2 = NCcollate(*p2);
			    cu2 = NCcoluniq(*p2++);
			    if (*p2 == '\0')
				p2 = NULL;
		    }
		}

		
	} while (ch1 != '\0' && ch2 != '\0' && cu1 == cu2);

	return (cu1 - cu2);
}

/*
 * FUNCTION: Compares at most n wchar_t's of the strings pointed to by s1 and s2, returning an
 *	     integer as follows:
 *
 *		Less than 0	If s1 is less than s2
 *		Equal to 0	If s1 is equal to s2
 *		Greater than 0	If s1 is greater than s2.
 *
 *	     The comparison is based on the collating sequence specified
 *	     by the locale category LC_COLLATE affected by the setlocale
 *	     function.
 *
 * NOTES:    The ANSI Programming Language C standard requires this routine.
 *
 * PARAMETERS: (Uses wchar_t's)
 *	     wchar_t *s1 - first string
 *	     wchar_t *s2 - second string
 *           int n - length
 *
 * RETURN VALUE DESCRIPTIONS: Returns a negative, zero, or positive value
 *	     as described above.
 */
/*LINTLIBRARY*/

#define	chnext1(s,e)	    ( s > e ? '\0' : *s++)
#define	extcol1(s, e, ch, cu)   ((cu = NCcoluniq(ch = chnext1(s,e))), NCcollate(ch))


/* The code consists of two loops: the first compares the two strings based
 * on primary weight, the second compares based on secondary weights (if there
 * were any).
 * In each loop we fetch the desired value for a char ( or collation element)
 * from each string, discarding "ignore" characters, and compare them.
 * We exit from the loops if either string terminates (i.e., the other is
 * longer or we hit an inequality.
 * If the first loop is exited with both strings equal and we found a char
 * with secondary weights (NCcollate value different from NCcoluniq value),
 * then we enter the second loop. In this we only compare NCcoluniq values
 * from the two strings if the char has secondary weight.
 * The NLxcolu routine is entered if the primary collation value is -1
 * (extended collation), and returns either a collation value  (n-to-1),
 * or a -1 for replacement string. In the last case, the 'p' parameter
 * now points to the first character of the replacement string.
 */

#ifdef _NO_PROTO
int NCstrncmp(s1, s2, n)
wchar_t *s1;
wchar_t *s2;
int n;
#else
int	
NCstrncmp(wchar_t *s1, wchar_t *s2, int n)
#endif
{
	int co1, co2;			/* primary collation value */
	int cu1, cu2;			/* secondary collation value */
	int secflag = 0;
	wchar_t *e1, *e2;
	wchar_t *sav1, *sav2;
	wchar_t ch1, ch2;
	wchar_t *p1, *p2;		/* pointers for NLxcolu's use */
	wchar_t *x1, *x2;		/* pointers for NLxcolu's use */

	if (s1 == s2)
		return (0);

	p1 = p2 = NULL;
	sav1 = s1;			/* save string pointers for possible */
	sav2 = s2;			/* second pass */
	e1 = s1 + n - 1;
	e2 = s2 + n - 1;
	

	do {
		co1  = 0;			/* so first pass will work */
		while (co1 == 0)	/* loop until non-zero */
		{
					/* The if extracts co and cu and */
					/* evaluates to TRUE only if it is */
					/* 1-to-n mapping */
		    if (p1 != NULL || (co1 = extcol1(s1, e1, ch1, cu1)) < 0 &&
		       (co1 = _NCxcolu(co1, &s1, &p1, &cu1)) == -1) {
					/* This if is for the case where the replacement */
					/* char is part of an extended collation (except */
					/* replacement, because we check that in ctab) */
			    if ((co1 = NCcollate(*p1)) < 0) {
				 p1++;
				 co1 = _NCxcolu(co1, &p1, &x1, &cu1);
			    } else 
				cu1 = NCcoluniq(*p1++);

			    if (*p1 == '\0')	/* at end of 1-to-n str */
				p1 = NULL;
		    }
		}

		co2 = 0;
		while (co2 == 0)
		{
		    if (p2 != NULL || (co2 = extcol1(s2, e2, ch2, cu2)) < 0 &&
		       (co2 = _NCxcolu(co2, &s2, &p2, &cu2)) == -1) {
			    if ((co2 = NCcollate(*p2)) < 0) {
				 p2++;
				 co2 = _NCxcolu(co2, &p2, &x2, &cu2);
			    } else 
				cu2 = NCcoluniq(*p2++);
			    if (*p2 == '\0')
				p2 = NULL;
		    }
		}
		if ((co1 != cu1) || (co2 != cu2))
			secflag = 1;		/* we found a sec ordering */


	} while (ch1 != '\0' && ch2 != '\0' && co1 == co2);

	if (((ch1  != '\0') || (ch2 != '\0')) || (secflag == 0))
		return (co1 - co2);

 /* At this point, we have two strings that collate equal on their primary
  * collation values. We now go back to the beginning of the strings and
  * recompare them based on secondary collation values (if any)...
  */


	p1 = p2 = NULL;
	s1 = sav1;
	s2 = sav2;
	co1 = co2 = cu1 = cu2 = 0;

	do {
		while ((co1 == 0) || (co1 == cu1)) 
		{
		    if (p1 != NULL || (co1 = extcol1(s1, e1, ch1, cu1)) < 0 &&
		       (co1 = _NCxcolu(co1, &s1, &p1, &cu1)) == -1) {
			    co1 = NCcollate(*p1);
			    cu1 = NCcoluniq(*p1++);
			    if (*p1 == '\0')
				p1 = NULL;
		    }
		}

		while ((co2 == 0) || (co2 == cu2))
		{
		    if (p2 != NULL || (co2 = extcol1(s2, e2, ch2, cu2)) < 0 &&
		       (co2 = _NCxcolu(co2, &s2, &p2, &cu2)) == -1) {
			    co2 = NCcollate(*p2);
			    cu2 = NCcoluniq(*p2++);
			    if (*p2 == '\0')
				p2 = NULL;
		    }
		}

		
	} while (ch1 != '\0' && ch2 != '\0' && cu1 == cu2);

	return (cu1 - cu2);
}

#ifdef KJI
#include <wchar.h>
#undef wstrncmp
#ifdef _NAME_SPACE_WEAK_STRONG
#define wstrncmp __wstrncmp
#endif

int
wstrncmp(s1, s2, n)
wchar_t *s1, *s2;
int n;
{
	return(NCstrncmp(s1, s2, n));
}
#endif
