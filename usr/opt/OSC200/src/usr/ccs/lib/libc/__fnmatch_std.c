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
static char *rcsid = "@(#)$RCSfile: __fnmatch_std.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/10/29 18:58:46 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCPAT) Standard C Library Pattern Functions
 *
 * FUNCTIONS: __fnmatch_std
 *
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.6  com/lib/c/pat/__fnmatch_std.c, , bos320, 9134320 8/13/91 14:48:01
 *
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef stat
#undef stat
#endif

#include <sys/types.h>
#include <ctype.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <sys/localedef.h>
#include <fnmatch.h>
#include "patlocal.h"
#include "collation.h"

static int bracket(		/* function declaration			*/
		_LC_collate_t *,
		const char *,
		const char **,
		wchar_t,
		wchar_t,
		int);

/************************************************************************/
/* __fnmatch_std() - Standard method for Filename Matching		*/
/*               - works for all locale codesets			*/
/*									*/
/*	phdl     - ptr to __lc_collate structure			*/
/*	ppat	 - ptr to filename pattern				*/
/*	string	 - ptr to beginning of filename				*/
/*	pstr	 - ptr to current position within filename		*/
/*	flags	 - fnmatch() flags, see <fnmatch.h>			*/
/************************************************************************/

int
__fnmatch_std(const char *ppat, const char *string, const char *pstr, int flags, _LC_collate_t *phdl)
{
	int	mblen_p;	/* # bytes in next pattern character	*/
	int	mblen_s;	/* # bytes in next string character	*/
	char	*pse;		/* ptr to next string character		*/
	int	stat;		/* recursive rfnmatch() return status	*/
	wchar_t	ucoll_s;	/* string unique coll value		*/
	wchar_t	wc_p;		/* next patatern character		*/
	wchar_t	wc_s;		/* next string character		*/

/*
 * Loop through pattern, matching string characters with pattern
 * Return success when end-of-pattern and end-of-string reached simultaneously
 * Return no match if pattern/string mismatch
 */
	while (*ppat != '\0')
		{
		mblen_p = mbtowc(&wc_p, ppat, MB_CUR_MAX);
		if (mblen_p < 0)
			{
			mblen_p = 1;
			wc_p = *ppat & 0xff;
			}
		mblen_s = mbtowc(&wc_s, pstr, MB_CUR_MAX);
		if (mblen_s < 0)
			{
			mblen_s = 1;
			wc_s = *pstr & 0xff;
			}
		switch (wc_p)
			{
/*
 * <backslash> quotes the next character unless FNM_NOESCAPE flag is set
 * Otherwise treat <backslash> as itself
 * Return no match if pattern ends with quoting <backslash>
 */
		case '\\':
			if ((flags & FNM_NOESCAPE) == 0)
				{
				ppat++;
				mblen_p = mbtowc(&wc_p, ppat, MB_CUR_MAX);
				if (mblen_p == 0)
					return (FNM_NOMATCH);
				if (mblen_p < 0)
					{
					mblen_p = 1;
					wc_p = *ppat & 0xff;
					}
				}
/*
 * Ordinary character in pattern matches itself in string
 * Need to compare process codes for multibyte languages
 * Continue if pattern character matches string character
 * Return no match if pattern character does not match string character
 */
		default:
		ordinary:
			if (wc_p == wc_s)
				{
				ppat += mblen_p;
				pstr += mblen_s;
				break;
				}
			else
				return (FNM_NOMATCH);
/*
 * <asterisk> matches zero or more string characters
 * Cannot match <slash> if FNM_PATHNAME is set
 * Cannot match leading <period> if FNM_PERIOD is set
 * Consecutive <asterisk> are redundant
 *
 * Return success if remaining pattern matches remaining string
 * Otherwise advance to the next string character and try again
 * Return no match if string exhausted and more pattern remains
 */
		case '*':
			while (*++ppat == '*')
				;
			if (*ppat == '\0')
				{
				if ((flags & FNM_PATHNAME) != 0 && strchr(pstr, '/') != NULL)
					return (FNM_ESLASH);
				if (*pstr == '.' && (flags & FNM_PERIOD) != 0)
					if (pstr == string || (pstr[-1] == '/' && (flags & FNM_PATHNAME) != 0))
						return (FNM_EPERIOD);
				return (0);
				}
			while (*pstr != '\0')
				{
				stat = __fnmatch_std(ppat, string, pstr, flags, phdl);
				if (stat != FNM_NOMATCH)
					return (stat);
				if (*pstr == '/')
					{
					if ((flags & FNM_PATHNAME) != 0)
						return (FNM_ESLASH);
					}
				else if (*pstr == '.' && (flags & FNM_PERIOD) != 0)
					if (pstr == string || (pstr[-1] =='/' && (flags & FNM_PATHNAME) != 0))
						return (FNM_EPERIOD);
				mblen_s = mblen(pstr, MB_CUR_MAX);
				if (mblen_s > 0)
					pstr += mblen_s;
				else
					pstr++;
				}
			return (FNM_NOMATCH);
/*
 * <question-mark> matches any single character
 * Cannot match <slash> if FNM_PATHNAME is set
 * Cannot match leading <period> if FNM_PERIOD is set
 *
 * Return no match if string is exhausted
 * Otherwise continue with next pattern and string character
 */
		case '?':
			if (wc_s == '/')
				{
				if ((flags & FNM_PATHNAME) != 0)
					return (FNM_ESLASH);
				}
			else if (wc_s == '.' && (flags & FNM_PERIOD) != 0)
				if (pstr == string || (pstr[-1] == '/' && (flags & FNM_PATHNAME) != 0))
					return (FNM_EPERIOD);
			if (mblen_s >= 0)
				pstr += mblen_s;
			else
				pstr++;
			ppat++;
			break;
/*
 * <left-bracket> begins a [bracket expression] which matches single collating element
 * [bracket expression] cannot match <slash> if FNM_PATHNAME is set
 * [bracket expression] cannot match leading <period> if FNM_PERIOD is set
 */
		case '[':
			if (wc_s == '/')
				{
				if ((flags & FNM_PATHNAME) != 0)
					return (FNM_ESLASH);
				}
			else if (wc_s == '.' && (flags & FNM_PERIOD) != 0)
				if (pstr == string || (pstr[-1] == '/' && (flags & FNM_PATHNAME) != 0))
					return (FNM_EPERIOD);
/*
 * Determine unique collating value of next collating element
 */
			ucoll_s = _mbucoll(phdl, (char *)pstr, (char **)&pse);
			ucoll_s = byte_string_to_collation(ucoll_s);
			if ((ucoll_s < MIN_UCOLL) || (ucoll_s > MAX_UCOLL))
				return (FNM_NOMATCH);
/*
 * Compare unique collating value to [bracket expression]
 *   > 0  no match
 *   = 0  match found
 *   < 0  error, treat [] as individual characters
 */
			stat = bracket(phdl, ppat+1, &ppat, wc_s, ucoll_s, flags);
			if (stat == 0)
				pstr = pse;
			else if (stat > 0)
				return (FNM_NOMATCH);
			else
				goto ordinary;
			break;
			}
		}
/*
 * <NUL> following end-of-pattern
 * Return success if string is also at <NUL>
 * Return no match if string not at <NUL>
 */
		if (*pstr == '\0')
			return (0);
		else
			return (FNM_NOMATCH);
	}


/************************************************************************/
/* bracket()    - Determine if [bracket] matches filename character	*/
/*									*/
/*	pdhl	 - ptr to __lc_collate structure			*/
/*	ppat	 - ptr to position of '[' in pattern			*/
/*	wc_s	 - process code of next filename character		*/
/*	ucoll_s	 - unique collating weight of next filename character	*/
/*	flags	 - fnmatch() flags, see <fnmatch.h>			*/
/************************************************************************/

static int
bracket(_LC_collate_t *phdl, const char *ppat, const char **pend, wchar_t wc_s, wchar_t ucoll_s, int flags)
{
	int	neg;		/* nonmatching [] expression	*/
	int	dash;		/* <hyphen> found for a-z range expr	*/
	wchar_t	prev_min_ucoll;	/* low end of range expr		*/
	wchar_t	min_ucoll;	/* minimum unique collating value	*/
	wchar_t	max_ucoll;	/* maximum unique collating value	*/
	const char *pb;		/* ptr to [bracket] pattern		*/
	const char *pi;		/* ptr to international [] expression	*/
	char	*piend;		/* ptr to character after intl [] expr	*/
	int	found;		/* match found flag			*/
	wchar_t	wc_b;		/* bracket process code			*/
	int	mblen_b;	/* next bracket character length	*/
	char	type;		/* international [] type =:.		*/
	wchar_t	pcoll;		/* primary collation weight		*/
	wint_t	i;		/* process code loop index		*/
	char	class[CLASS_SIZE+1]; /* character class with <NUL>	*/
	char	*pclass;	/* class[] ptr				*/
	wchar_t	t_pcoll;	/* temporary collation value		*/
	wchar_t	temp_coll;	/* temporary collation value		*/
/*
 * Leading <exclamation-mark> designates nonmatching [bracket expression]
 */
	pb = ppat;
	neg = 0;
	if (*pb == '!')
		{
		pb++;
		neg++;
		}
/*
 * Loop through each [] collating element comparing unique collating values
 */
	dash = 0;
	found = 0;
	prev_min_ucoll = 0;
	min_ucoll = 0;
	max_ucoll = 0;
	while ((mblen_b = mbtowc(&wc_b, pb, MB_CUR_MAX)) != 0)
		{
/*
 * Use next byte if invalid multibyte character
 */
		if (mblen_b < 0)
			{
			mblen_b = 1;
			wc_b = *pb & 0xff;
			}
/*
 * Final <right-bracket> so return status based upon whether match was found
 * Return character after final ] if match is found
 * Ordinary character if first character of [barcket expression]
 */
		if (wc_b == ']')
			if ((neg == 0 && pb > ppat) || (neg != 0 && pb > ppat + 1))
				{
				if ((found ^ neg) == 0)
					return (FNM_NOMATCH);
				*pend = ++pb;
				return (0);
				}
/*
 * Return error if embedded <slash> found
 */
		else if (wc_b == '/' && (flags&FNM_PATHNAME))
			return (-1);
/*
 * Decode next [] element
 */
		if (dash == 0)
			prev_min_ucoll = min_ucoll;
		switch (wc_b)
			{
/*
 * <hyphen> deliniates a range expression unless it is first character of []
 * or it immediately follows another <hyphen> and is therefore an end point
 */
		case '-':
			if (dash == 0 && !((neg == 0 && pb == ppat) || (neg != 0 && pb == ppat + 1) || (pb[1] == ']')))
				{
				dash++;
				pb++;
				continue;
				}
/*
 * ordinary character - compare unique collating weight of collating symbol
 */
		default:
		ordinary:
			_getcolval(phdl, &min_ucoll, wc_b, "", MAX_NORDS);
			min_ucoll = byte_string_to_collation(min_ucoll);
			if ((min_ucoll < MIN_UCOLL) || (min_ucoll > MAX_UCOLL))
				return (-1);
			if (min_ucoll == ucoll_s)
				found = 1;
			max_ucoll = min_ucoll;
			pb += mblen_b;
			break;
/*
 * <left-bracket> initiates one of the following internationalization
 *   character expressions
 *   [: :] character class
 *   [= =] equivalence character class
 *   [. .] collation symbol
 *
 * it is treated as itself if not followed by appropriate special character
 * it is treated as itself if any error is encountered
 */
		case '[':
			pi = pb + 2;
			if ((type = pb[1]) == ':')
				{
				pclass = class;
				while (1)
					{
					if (*pi == '\0')
						return (-1);
					if (*pi == ':' && pi[1] == ']')
						break;
					if (pclass >= &class[CLASS_SIZE])
						return (-1);
					*pclass++ = *pi++;
					}
				if (pclass == class)
					return (-1);
				*pclass = '\0';
				if (iswctype(wc_s, wctype(class)) != 0)
					found = 1;
				min_ucoll = 0;
				pb = pi + 2;
				break;
				}
/*
 * equivalence character class
 *   get process code of character, error if invalid or <NUL>
 *   treat as collation symbol if not entire contents of [= =]
 *   locate address of collation weight table
 *   get unique collation weight, error if none
 *   set found flag if unique collation weight matches that of string collating element
 *   if no match, compare unique collation weight of all equivalent characters
 */
			else if (type == '=')
				{
				mblen_b = mbtowc(&wc_b, pi, MB_CUR_MAX);
				if (mblen_b <= 0)
					return (-1);
				if (pi[mblen_b] != type)
					goto coll_sym;
				if (pi[mblen_b + 1] != ']')
					return (-1);
				_getcolval(phdl, &min_ucoll, wc_b, "", MAX_NORDS);
				min_ucoll = byte_string_to_collation(min_ucoll);
				max_ucoll = min_ucoll;
				if ((min_ucoll < MIN_UCOLL) || (min_ucoll > MAX_UCOLL))
					return (-1);
				_getcolval(phdl, &pcoll, wc_b, "", 0);
				pcoll = byte_string_to_collation(pcoll);
				for (i = MIN_PC; i <= MAX_PC; i++)
				{
					_getcolval(phdl, &t_pcoll, i, "", 0);
					t_pcoll = byte_string_to_collation(t_pcoll);
					if (t_pcoll == pcoll)
						{
						_getcolval(phdl, &temp_coll, i, "", MAX_NORDS);
						temp_coll = byte_string_to_collation(temp_coll);
						if (temp_coll == ucoll_s)
							found = 1;
						if (temp_coll < min_ucoll)
							min_ucoll = temp_coll;
						if (temp_coll > max_ucoll)
							max_ucoll = temp_coll;
						}
				}
				pb = pi + mblen_b + 2;
				break;
				}
/*
 * collation symbol
 *   locate address of collation weight table, error if none
 *   verify collation symbol is entire contents of [. .] expression, error if not
 *   get unique collation weight, error if none
 *   set found flag if collation weight matches that of string collating element
 */
			else if (type == '.')
				{
		coll_sym:
				min_ucoll = _mbucoll(phdl, (char *)pi, (char **)&piend);
				min_ucoll = byte_string_to_collation(min_ucoll);

				if ((min_ucoll < MIN_UCOLL) ||
				    (min_ucoll > MAX_UCOLL) ||
				    (*piend != type) || (piend[1] != ']'))
					return (-1);
				if (min_ucoll == ucoll_s)
					found = 1;
				max_ucoll = min_ucoll;
				pb = piend + 2;
				break;
				}
			else
				goto ordinary;
			} /* end of switch */
/*
 * Check for the completion of a range expression and determine
 * whether string collating element falls between end points
 */
		if (dash != 0)
			{
			dash = 0;
			if (prev_min_ucoll == 0 || prev_min_ucoll > max_ucoll)
				return (-1);
			if (ucoll_s >= prev_min_ucoll && ucoll_s <= max_ucoll)
				found = 1;
			min_ucoll = 0;
			}
		} /* end of while */
/*
 * Return < 0 since <NUL> was found
 */
	return (-1);
}
