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
static char *rcsid = "@(#)$RCSfile: regexec.c,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1993/09/03 18:47:30 $";
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
 * FUNCTIONS: regexec
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
 * 1.3  com/lib/c/pat/regexec.c, , bos320, 9134320 8/13/91 14:50:14
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak regexec = __regexec
#endif
#endif
#ifdef _NAME_SPACE_WEAK_STRONG
#ifdef stat
#undef stat
#endif
#endif

#include <sys/localedef.h>
#include <regex.h>

/************************************************************************/
/* Global data shared by all regcomp() and regexec() methods            */
/************************************************************************/

int __reg_bits[8] = {      		/* bitmask for [] bitmap        */
                0x00000001, 0x00000002, 0x00000004, 0x00000008,
                0x00000010, 0x00000020, 0x00000040, 0x00000080};


/*
 * FUNCTION: regexec()
 *
 * DESCRIPTION: determine if Regular Expression pattern matches string
 *	        invoke appropriate method for this locale
*/

int 
regexec(const regex_t *preg, const char *string, 
	    size_t nmatch, regmatch_t pmatch[], int eflags)
{
	if (METHOD(__lc_collate, regexec) == NULL)
		return __regexec_C(preg, string, nmatch, 
					pmatch, eflags, __lc_collate);
	else
		return METHOD(__lc_collate, regexec)( preg, string, nmatch, 
					pmatch, eflags, __lc_collate);
}


#include <sys/types.h>
#include <ctype.h>
#include <limits.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>
#include "reglocal_C.h"
#include "libc_msg.h"


/************************************************************************/
/* External data defined in regexec()					*/
/************************************************************************/

extern	int	__reg_bits[];	/* bitmask for [] bitmap		*/


/************************************************************************/
/* Internal definitions							*/
/************************************************************************/

typedef struct {                /* subexpression addresses		*/
        uchar_t	*sa;		/* start subexpression address		*/
        uchar_t	*ea;		/* end subexpression address + 1	*/
} ea_t;

typedef	struct 			/* structure for data during recursion	*/
	{
	int	flags;		/* combined cflags and eflags		*/
	unsigned wander;	/* flag: OK to wander			*/
	uchar_t	*string;	/* ptr to start of string		*/
	uchar_t	*pend_string;	/* ptr to byte beyond match in string	*/
	uchar_t *start_match;	/* ^ to beginning of match		*/
	int	submatch[_REG_SUBEXP_MAX+1]; /* # subexp matches	*/
	ea_t	tmatch[_REG_SUBEXP_MAX+1]; /* subexp addresses		*/
} EXEC;

			/* get the differences between two pointers */
#define VDIFF(p1,p2)	(((char *)(p1)) - ((char *)(p2)))

/************************************************************************/
/* Internal function prototypes						*/
/************************************************************************/

static	int	match_re(	/* match entire pattern against string	*/
			uchar_t *,
			uchar_t *,
			regex_t	*,
			EXEC	*);


/************************************************************************/
/* __regexec_C()	- Determine if RE pattern matches string		*/
/*		- valid for the C locale and any codeset without	*/
/*		  equivalence classes and multibyte collating elements	*/
/*		- does not support multibyte collating element		*/
/*		- assumes [= =] and [. .] do not have multiple elements	*/
/*									*/
/*		- phdl		ptr to __lc_collate table (unused)	*/
/*		- preg		ptr to structure with compiled pattern	*/
/*		- string	string to be matched			*/
/*		- nmatch	# of matches to report in pmatch	*/
/*		- pmatch	reported subexpression offsets		*/
/*		- eflags	regexec() flags				*/
/************************************************************************/

int
__regexec_C( const regex_t *preg, const char *string,
	size_t nmatch, regmatch_t pmatch[], int eflags, void *phdl)
{
	EXEC	e;		/* initial data passing structure	*/
	register int	i;	/* loop index				*/
	uchar_t	*pmap;		/* ptr to character map table		*/
	uchar_t	*pstr;		/* ptr to next string byte		*/
	int	stat;		/* match_re() return status		*/
	static EXEC zero;	/* faster than bzero() call		*/

/*
 * Return error if RE pattern is undefined
 */
	if (preg->re_comp == NULL)
		return (REG_BADPAT);

	pmap = (uchar_t *)preg->re_map;
	pstr = (uchar_t *)string;

/*
 * optimisation:
 *	if the pattern doesn't start with "^",
 *	trim off prefix of pstr that has no hope of matching.  If we
 *	exhaust pstr via this method, we may already be a wiener!
 */
	if (*(uchar_t *) preg->re_comp != CC_BOL) {
		while (*pstr && pmap[*pstr] == 0)
			++pstr;

		if (!*pstr && !pmap[0])
			return REG_NOMATCH;
	}

/*
 * Initialize data recursion buffer
 */
	e         = zero;
	e.flags   = preg->re_cflags | eflags;
	e.string = (uchar_t *)string;
	e.wander  = 1;
	e.start_match = pstr;

/*
 * This ("NEW_IBM_WAY") is turned off because it breaks some VXS4 tests
 * and the reason it was added by IBM is not known.
 */
#ifdef NEW_IBM_WAY
/*
 * Attempt to match entire compiled RE pattern starting at current
 *     position in string
 */
	stat = match_re((uchar_t *)preg->re_comp, pstr, (regex_t *)preg, &e);
#else
/*
 * Attempt to match entire compiled RE pattern starting at current
 *     position in string
 * If matching of entire pattern fails, advance to next string
 *     character and try again until string is exhausted
 * Use character map to skip over nonmatching characters
 */
	stat = REG_NOMATCH;
	do
		{
		if (pmap[*pstr] != 0)
			stat = match_re((uchar_t *)preg->re_comp, pstr, 
					(regex_t *)preg, &e);
		}
	while (stat == REG_NOMATCH && *pstr++ != '\0');
#endif

/*
 * Return offsets of entire pattern match
 * Return subexpression offsets, zero-length changed to -1
 */
	if (stat == 0)
		{
		if (nmatch > 0 && (preg->re_cflags & REG_NOSUB) == 0)
			{
			pmatch[0].rm_so = VDIFF(pstr, string);
			pmatch[0].rm_eo = VDIFF(e.pend_string, string);
			for (i=1; i<nmatch && i<=_REG_SUBEXP_MAX; i++)
				{
				if (e.tmatch[i].sa != NULL)
					{
					pmatch[i].rm_so = 
						VDIFF(e.tmatch[i].sa, string);
					pmatch[i].rm_eo = 
						VDIFF(e.tmatch[i].ea, string);
					}
				else
					{
					pmatch[i].rm_so = (off_t)-1;
					pmatch[i].rm_eo = (off_t)-1;
					}
				}
			}
		}
	return (stat);
}


/************************************************************************/
/* match_re()	- Match entire RE pattern to string			*/
/*									*/
/*		- ppat		ptr to pattern				*/
/*		- pstr		ptr to string				*/
/*		- preg		ptr to caller's regex_t structure	*/
/*		- pe		ptr to recursion data structure		*/
/************************************************************************/

static int
match_re(uchar_t *ppat, uchar_t *pstr, regex_t *preg, EXEC *pe)
{
	uchar_t	*best_alt;	/* best alternative pend_string		*/
	size_t	count;		/* # bytes to backtrack each time	*/
	size_t	count2;		/* ignore case backreference counter	*/
	int	cp;		/* pattern character			*/
	int	cp2;		/* opposite case pattern character	*/
	int	cs;		/* string character			*/
	int	idx;		/* subexpression index			*/
	int	max;		/* maximum repetition count - min	*/
	int	min;		/* minimum repetition count		*/
	uchar_t	*pback;		/* ptr to subexpression backreference	*/
	uchar_t	*pbitmap;	/* ptr to bracket expression bitmap	*/
	uchar_t	*pea;		/* ptr to subexpression end address	*/
	uchar_t	*psa;		/* ptr to subexpression start address	*/
	uchar_t	*pstop;		/* ptr to backtracking string point	*/
	uchar_t	*ptemp;		/* ptr to string during backreference	*/
	uchar_t	*sav_pat;	/* saved pattern			*/
	uchar_t	*sav_str;	/* saved string				*/
	uchar_t	*pmap;		/* ptr to character map table		*/
	int	wander;		/* copy of EXEC.wander			*/
	int	stat;		/* match_re() recursive status		*/
	EXEC	r;		/* another copy of *pe for recursion	*/
	EXEC	rbest;		/* best alternative recursion data	*/

	pmap       = preg->re_map;
	wander     = pe->wander;
	pe->wander = 0;
	sav_pat    = ppat;
	sav_str    = pstr;

	if (0) {
	    no_match:
#ifndef NEW_IBM_WAY
		return REG_NOMATCH;
#endif
		/*
		 * NOTE: the only way to come here is via a goto.
		 */
		if (wander) {
			/*
			 * we come here if we fail to match, and ok to wander
			 * down the string looking for a match.
			 *	- restore the pattern to the start
			 *	- restore string to one past where we left off
			 *	  and trim unmatchables
			 */
			ppat = sav_pat;		/* restore patterm	*/
			pstr = sav_str + 1;

			while (*pstr && pmap[*pstr] == 0)
				++pstr;

			if (*pstr == 0)
				return REG_NOMATCH;

			pe->start_match = sav_str = pstr;
		} else
			return REG_NOMATCH;
	}

/*
 * Perform each compiled RE pattern code until end-of-pattern or non-match
 * Break to bottom of loop to match remaining pattern/string when extra
 *   expressions have been matched
 */
    while (1)
	{
	count = 1;
	switch (*ppat++)
		{
/*
 * single character, no repetition
 *   continue if pattern character matches next string character
 *   otherwise return no match
 */

	case CC_CHAR:
		if (*ppat != *pstr)
			goto no_match;
		ppat++;
		pstr++;
		continue;
/*
 * character string, no repetition
 *   continue if next n pattern characters matches next n string characters
 *   otherwise return no match
 */

	case CC_STRING:
		min = *ppat++;
		do
			if (*ppat++ != *pstr++)
				goto no_match;
		while (--min > 0);
		continue;
/*
 * single character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_CHAR processing
 */

	case CC_CHAR | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_char;
/*
 * single character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_CHAR processing
 */

	case CC_CHAR | CR_INTERVAL_ALL:
		min = *ppat++;
		*ppat++;
		max = INT_MAX - 1;
		goto cc_char;
/*
 * single character, one or more occurances "+"
 *   define min/max and jump to common CC_CHAR processing
 */

	case CC_CHAR | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_char;
/*
 * single character, zero or one occurances "?"
 *   define min/max and jump to common CC_CHAR processing
 */

	case CC_CHAR | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_char;
/*
 * single character, zero or more occurances "*"
 *   define min/max and jump to common CC_CHAR processing
 */

	case CC_CHAR | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_char;
/*
 * single character - variable number of matches
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_char:
		cp = *ppat++;
		while (min-- > 0)
			if (cp != *pstr++)
				goto no_match;
		pstop = pstr;
		while (max-- > 0 && cp == *pstr)
			pstr++;
		break;
/*
 * bracket expression, no repetition
 *   continue if next string character has bit set in bitmap
 *   otherwise return no match
 */

	case CC_BITMAP:
		cs = *pstr++;
		if ((*(ppat+((cs & 0xf8)>>3)) & __reg_bits[cs&0x07]) != 0)
			{
			ppat += BITMAP_LEN;
			continue;
			}
		goto no_match;
/*
 * bracket expression, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_BITMAP processing
 */

	case CC_BITMAP | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_bitmap;
/*
 * bracket expression, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_BITMAP processing
 */

	case CC_BITMAP | CR_INTERVAL_ALL:
		min = *ppat++;
		*ppat++;
		max = INT_MAX - 1;
		goto cc_bitmap;
/*
 * bracket expression, one or more occurances "+"
 *   define min/max and jump to common CC_BITMAP processing
 */

	case CC_BITMAP | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_bitmap;
/*
 * bracket expression, zero or one occurances "?"
 *   define min/max and jump to common CC_BITMAP processing
 */

	case CC_BITMAP | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_bitmap;
/*
 * bracket expression, zero or more occurances "*"
 *   define min/max and jump to common CC_BITMAP processing
 */

	case CC_BITMAP | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_bitmap;
/*
 * bracket expression - variable number of matches
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_bitmap:
		pbitmap = ppat;
		ppat += BITMAP_LEN;
		while (min-- > 0)
			{
			cs = *pstr++;
			if ((*(pbitmap+((cs & 0xf8)>>3)) & __reg_bits[cs&0x07]) == 0)
				goto no_match;
			}
		pstop = pstr;
		while (max-- > 0)
			{
			cs = *pstr;
			if ((*(pbitmap+((cs & 0xf8)>>3)) & __reg_bits[cs&0x07]) != 0)
				pstr++;
			else
				break;
			}
		break;
/*
 * any single character, no repetition
 *   continue if next string character is anything but NUL
 *     or newline and REG_NEWLINE is set
 *   otherwise return no match
 */

	case CC_DOT:
		if (*pstr == 0 || (*pstr++ == '\n' && (pe->flags & REG_NEWLINE) != 0))
			return (REG_NOMATCH);
		continue;
/*
 * any single character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_DOT processing
 */

	case CC_DOT | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_dot;
/*
 * any single character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_DOT processing
 */

	case CC_DOT | CR_INTERVAL_ALL:
		min = *ppat++;
		*ppat++;
		max = INT_MAX - 1;
		goto cc_dot;
/*
 * any single character, one or more occurances "+"
 *   define min/max and jump to common CC_DOT processing
 */

	case CC_DOT | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_dot;
/*
 * any single character, zero or one occurances "?"
 *   define min/max and jump to common CC_DOT processing
 */

	case CC_DOT | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_dot;
/*
 * any single character, zero or more occurances "*"
 *   define min/max and jump to common CC_DOT processing
 */

	case CC_DOT | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_dot;
/*
 * any single character - variable number of matches
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_dot:
		while (min-- > 0)
			if (*pstr == '\0' || (*pstr++ == '\n' && (pe->flags & REG_NEWLINE) != 0))
				return (REG_NOMATCH);
		pstop = pstr;
		while (max-- > 0 && *pstr != '\0' && (*pstr != '\n' || (pe->flags & REG_NEWLINE) == 0))
			pstr++;
		break;
/*
 * end-of-pattern
 *   update forward progress of matched location in string
 *   return success
 */

	case CC_EOP:
		pe->pend_string = pstr;
		return (0);
/*
 * beginning-of-line anchor
 *   REG_NEWLINE allows ^ to match null string following a newline
 *   REG_NOTBOL means first character is not beginning of line
 *
 *   REG_NOTBOL   REG_NEWLINE   at BOL   ACTION
 *   ----------   -----------   ------   -------------------------
 *        N            N           Y     continue
 *        N            N           N     return REG_NOMATCH
 *        N            Y           Y     continue
 *        N            Y           N     continue if \n, else return REG_NOMATCH
 *        Y            N           Y     return REG_NOMATCH
 *        Y            N           N     return REG_NOMATCH
 *        Y            Y           Y     continue if \n, else return REG_NOMATCH
 *        Y            Y           N     continue if \n, else return REG_NOMATCH
 */

	case CC_BOL:
		if ((pe->flags & REG_NOTBOL) == 0)
			{
			if (pstr == pe->string)
				continue;
			else if ((pe->flags & REG_NEWLINE) == 0)
				goto no_match;
			}
		else if ((pe->flags & REG_NEWLINE) == 0)
			goto no_match;
		if (pstr > pe->string && *(pstr-1) == '\n')
			continue;
		goto no_match;
/*
 * end-of-line anchor
 *   REG_NEWLINE allows $ to match null string preceeding a newline
 *   REG_NOTEOL means last character is not end of line
 *
 *   REG_NOTEOL   REG_NEWLINE   at EOL   ACTION
 *   ----------   -----------   ------   --------------------------
 *        N            N           Y     continue
 *        N            N           N     return REG_NOMATCH
 *        N            Y           Y     continue
 *        N            Y           N     continue if \n, else return REG_NOMATCH
 *        Y            N           Y     return REG_NOMATCH
 *        Y            N           N     return REG_NOMATCH
 *        Y            Y           Y     continue if \n, else return REG_NOMATCH
 *        Y            Y           N     continue if \n, else return REG_NOMATCH
 */

	case CC_EOL:
		if ((pe->flags & REG_NOTEOL) == 0)
			{
			if (*pstr == '\0')
				continue;
			else if ((pe->flags & REG_NEWLINE) == 0)
				goto no_match;
			}
		else if ((pe->flags & REG_NEWLINE) == 0)
			goto no_match;
		if (*pstr == '\n')
			continue;
		goto no_match;
/*
 * ignore case single character, no repetition
 *   continue if next string character matches pattern character or
 *     opposite case of pattern character
 *   otherwise return no match
 */

	case CC_I_CHAR:
		if (*ppat++ == *pstr)
			{
			ppat++;
			pstr++;
			continue;
			}
		if (*ppat++ == *pstr++)
			continue;
		goto no_match;
/*
 * ignore case character string, no repetition
 *   continue if next n string characters match next n pattern characters or
 *     opposite case of next n pattern characters
 *   otherwise return no match
 */

	case CC_I_STRING:
		min = *ppat++;
		do
			{
			if (*ppat++ == *pstr)
				{
				ppat++;
				pstr++;
				}
			else if (*ppat++ != *pstr++)
				goto no_match;
			}
		while (--min > 0);
		continue;

/*
 * ignore case single character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_I_CHAR processing
 */

	case CC_I_CHAR | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_ichar;
/*
 * ignore case single character, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_I_CHAR processing
 */

	case CC_I_CHAR | CR_INTERVAL_ALL:
		min = *ppat++;
		*ppat++;
		max = INT_MAX - 1;
		goto cc_ichar;
/*
 * ignore case single character, one or more occurances "+"
 *   define min/max and jump to common CC_I_CHAR processing
 */

	case CC_I_CHAR | CR_PLUS:
		min = 1;
		max = INT_MAX - 1;
		goto cc_ichar;
/*
 * ignore case single character, zero or one occurances "?"
 *   define min/max and jump to common CC_I_CHAR processing
 */

	case CC_I_CHAR | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_ichar;
/*
 * ignore case single character, zero or more occurances "*"
 *   define min/max and jump to common CC_I_CHAR processing
 */

	case CC_I_CHAR | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_ichar;
/*
 * ignore case single character - variable number of matches
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_ichar:
		cp = *ppat++;
		cp2 = *ppat++;
		while (min-- > 0)
			{
			if (cp != *pstr && cp2 != *pstr)
				goto no_match;
			pstr++;
			}
		pstop = pstr;
		while (max-- > 0 && (cp == *pstr || cp2 == *pstr))
			pstr++;
		break;
/*
 * ignore case subexpression backreference, no repetition
 *   continue if next n string characters matches what was previously
 *     matched by the referenced subexpression
 *   otherwise return no match
 *   
 */
	case CC_I_BACKREF:
		idx = *ppat++;
		pback = pe->tmatch[idx].sa;
		pea = pe->tmatch[idx].ea;
		while (pback < pea)
			{
			if (((cp= *pback) != (cs= *pstr)) &&
			    (tolower(cp) != tolower(cs)))
				goto no_match;
			pback++;
			pstr++;
			}
		continue;
/*
 * ignore case subexpression backreference, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_I_BACKREF processing
 */

	case CC_I_BACKREF | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_ibackref;
/*
 * ignore case subexpression backreference, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_I_BACKREF processing
 */

	case CC_I_BACKREF | CR_INTERVAL_ALL:
		min = *ppat++;
		ppat++;
		max = INT_MAX - 1;
		goto cc_ibackref;
/*
 * ignore case subexpression backreference, zero or more occurances "*"
 *   define min/max and jump to common CC_I_BACKREF processing
 */

	case CC_I_BACKREF | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_ibackref;
/*
 * ignore case subexpression backreference - variable number of matches
 *   continue if subexpression match was zero-length
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_ibackref:
		idx = *ppat++;
		psa = pe->tmatch[idx].sa;
		pea = pe->tmatch[idx].ea;
		count = pea - psa;
		if (count == 0)
			continue;
		while (min-- > 0)
			{
			pback = psa;
			while (pback < pea)
				{
				if (((cp= *pback) != (cs= *pstr)) &&
				    (tolower(cp) != tolower(cs)))
					goto no_match;
				pback++;
				pstr++;
				}
			}
		pstop = pstr;
		while (max-- > 0)
			{
			pback = psa;
			ptemp = pstr;
			count2 = count;
			do
				{
				if (((cp = *pback) !=  (cs = *ptemp)) &&
				    (tolower(cp) != tolower(cs)))
					break;
				pback++;
				ptemp++;
				}
			while (--count2 > 0);
			if (count2 == 0)
				pstr = ptemp;
			else
				break;
			}
		break;
/*
 * begin subexpression
 *   generate new copy of recursion data
 *   preserve subexpression starting address
 *   match remaining pattern against remaining string
 *   if remaining pattern match succeeds, update recursion data with
 *     new copy and return success
 *   if remaining pattern match fails and zero length subexpression is ok, continue
 *     with pattern immediately following CC_SUBEXP_E
 *   otherwise return fatal error
 */

	case CC_SUBEXP:
		idx = *ppat++;
		r = *pe;
		r.tmatch[idx].sa = pstr;
		stat = match_re(ppat, pstr, preg, &r);
		if (stat == 0)
			{
			*pe = r;
			return (0);
			}
		if (((cp2 = (*(uchar_t *)preg->re_esub[idx] & CR_MASK)) == CR_QUESTION || cp2 == CR_STAR) ||
			((cp2 == CR_INTERVAL || cp2 == CR_INTERVAL_ALL) && *(((uchar_t *)preg->re_esub[idx])+1) == 0))
			{
			ppat = preg->re_esub[idx];
			if ((*ppat != (CC_SUBEXP_E | CR_INTERVAL)) && (*ppat != (CC_SUBEXP_E | CR_INTERVAL_ALL)))
				ppat += 2;
			else
				ppat += 4;
			continue;
			}
		return (stat);

/*
 * end subexpression, no repetition
 *   save subexpression ending address
 *   continue in all cases
 */

	case CC_SUBEXP_E:
		idx = *ppat++;
		pe->tmatch[idx].ea = pstr;
		continue;
/*
 * end subexpression, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_SUBEXP_E processing
 */

	case CC_SUBEXP_E | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_subexpe;
/*
 * end subexpression, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_SUBEXP_E processing
 */

	case CC_SUBEXP_E | CR_INTERVAL_ALL:
		min = *ppat++;
		*ppat++;
		max = INT_MAX - min - 1;
		goto cc_subexpe;
/*
 * end subexpression, one or more occurances "+"
 *   define min/max and jump to common CC_SUBEXP_E processing
 */

	case CC_SUBEXP_E | CR_PLUS:
		min = 1;
		max = INT_MAX - min - 1;
		goto cc_subexpe;
/*
 * end subexpression, zero or one occurances "?"
 *   define min/max and jump to common CC_SUBEXP_E processing
 */

	case CC_SUBEXP_E | CR_QUESTION:
		min = 0;
		max = 1;
		goto cc_subexpe;
/*
 * end subexpression, zero or more occurances "*"
 *   define min/max and jump to common CC_SUBEXP_E processing
 */

	case CC_SUBEXP_E | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_subexpe;
/*
 * end subexpression - variable number of matches
 *   save subexpression ending address
 *   if zero-length match, continue with remaining pattern if
 *     at or below minimum # of required matches
 *     otherwise return an error so that the last previous string
 *     matching locations can be used
 *   increment # of subexpression matches
 *   if the maximum # of required matches have not been found,
 *     reexecute the subexpression
 *     if it succeeds or fails without reaching the minimum # of matches
 *       return with the appropriate status
 *   if maximum number of matches found or the last match_re() failed and
 *     the minimum # of matches have been found, continue matching the
 *     remaining pattern against the remaining string
 */

	cc_subexpe:
		idx = *ppat++;
		pe->tmatch[idx].ea = pstr;
		if (pe->tmatch[idx].ea == pe->tmatch[idx].sa)
			if (pe->submatch[idx] < min)
				continue;
			else
				goto no_match;
		pe->submatch[idx]++;
		if (pe->submatch[idx] < min + max)
			{
			r = *pe;
			stat = match_re((uchar_t *)preg->re_lsub[idx], pstr, preg, &r);
			if (stat != REG_NOMATCH || pe->submatch[idx] < min)
				{
				if (stat == 0)
					*pe = r;
				return (stat);
				}
			}
		continue;
/*
 * subexpression backreference, no repetition
 *   continue if next n string characters matches what was previously
 *     matched by the referenced subexpression
 *   otherwise return no match
 *   
 */
	case CC_BACKREF:
		idx = *ppat++;
		pback = pe->tmatch[idx].sa;
		pea = pe->tmatch[idx].ea;
		while (pback < pea)
			if (*pback++ != *pstr++)
				goto no_match;
		continue;
/*
 * subexpression backreference, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_BACKREF processing
 */

	case CC_BACKREF | CR_INTERVAL:
		min = *ppat++;
		max = *ppat++;
		goto cc_backref;
/*
 * subexpression backreference, min/max occurances "{m,n}"
 *   define min/max and jump to common CC_BACKREF processing
 */

	case CC_BACKREF | CR_INTERVAL_ALL:
		min = *ppat++;
		*ppat++;
		max = INT_MAX - 1;
		goto cc_backref;
/*
 * subexpression backreference, zero or more occurances "*"
 *   define min/max and jump to common CC_BACKREF processing
 */

	case CC_BACKREF | CR_STAR:
		min = 0;
		max = INT_MAX - 1;
		goto cc_backref;
/*
 * subexpression backreference - variable number of matches
 *   continue if subexpression match was zero-length
 *   match minimum number of required times
 *     return no match if cannot meet minimum count
 *   save new string position for backtracking
 *   match maximum number of required times, where max is
 *     number of remaining matches
 *   break-out to match remaining pattern/string
 */

	cc_backref:
		idx = *ppat++;
		psa = pe->tmatch[idx].sa;
		pea = pe->tmatch[idx].ea;
		count = pea - psa;
		if (count == 0)
			continue;
		while (min-- > 0)
			{
			pback = psa;
			while (pback < pea)
				if (*pback++ != *pstr++)
					goto no_match;
			}
		pstop = pstr;
		while (max-- > 0)
			{
			if (strncmp((const char *)psa, (const char *)pstr, count) != 0)
				break;
			pstr += count;
			}
		break;
/*
 * start alternative
 */

	case CC_ALTERNATE:
		best_alt = NULL;
		do
			{
			idx = *ppat++ << 8;
			idx += *ppat++;
			r = *pe;
			stat = match_re(ppat, pstr, preg, &r);
			if (stat == 0 && best_alt < r.pend_string)
				{
				if (*r.pend_string == '\0')
					{
					*pe = r;
					return (0);
					}
				best_alt = r.pend_string;
				rbest = r;
				}
			if (idx == 0)
				break;
			ppat += idx + 1;
			}
		while (1);
		if (best_alt != NULL)
			{
			*pe = rbest;
			return (0);
			}
		goto no_match;
/*
 * end alternative
 *   skip over any other alternative patterns and continue matching
 *     pattern to string
 */

	case CC_ALTERNATE_E:
		idx = *ppat++;
		ppat = preg->re_esub[idx];
		continue;
/*
 * invalid compiled RE code
 *   return fatal error
 */

	default:
		return (REG_BADPAT);
		} /* switch */
	break;
	} /* while */
/*
 * surplus matched expressions end up here
 * generate new copy of recursion data
 * match remaining pattern against remaining string
 * if remaining pattern match fails, forfeit one extra matched
 *   character and try again until no spare matches are left
 * return success and new recursion data if entire remaining pattern matches
 * otherwise return no match
 */
	while (1)
		{
		r = *pe;
		stat = match_re(ppat, pstr, preg, &r);
		if (stat != REG_NOMATCH)
			{
			if (stat == 0)
				*pe = r;
			return (stat);
			}
		if (pstr <= pstop)
			break;
		pstr -= count;;
		}
	goto no_match;
}
