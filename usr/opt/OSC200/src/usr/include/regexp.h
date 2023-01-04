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
 *	@(#)$RCSfile: regexp.h,v $ $Revision: 4.2.5.7 $ (DEC) $Date: 1993/12/15 22:13:58 $
 */ 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/* regexp.h 1.6  com/inc,3.1,9013 1/15/90 16:39:26 */

#define __regcomp __Regcomp
#define __regexec __Regexec
#define __regfree __Regfree

#include <standards.h>


#ifdef ESIZE
#define _ESIZE ESIZE
#else
#define _ESIZE 512
#endif

#ifdef NBRA
#define _NBRA NBRA
#else
#define _NBRA 9
#endif

#ifdef _OSF_SOURCE
#if !defined(ESIZE)
#define ESIZE   _ESIZE
#endif

#if !defined(NBRA)
#define NBRA    _NBRA
#endif

#endif /* _OSF_SOURCE */


#define	_BIGRANGE	11	/* range endpoint too large */
#define	_BADNUM		16	/* bad number */
#define	_BIGDIGIT	25	/* \digit out of range */
#define	_NODELIM	36	/* illegal or missing delimiter */
#define	_NOMATCHSTR	41	/* no remembered match string */
#define	_BADPAREN	42	/* \( \) imbalance */
#define	_BIGPAREN	43	/* too many \( */
#define	_SYNBRACEFMT	44	/* more than 2 numbers in \{ \} */
#define	_NOCLOSEBRACE	45	/* } expected after \ */
#define	_SEMBRACEFMT	46	/* first number exceeds second in \{ \} */
#define	_BADBRACKET	49	/* [ ] imbalance */
#define	_BIGREGEXP	50	/* regular expression overflow */
#define	_ABNORMAL	99	/* not a normal error */

#ifndef _REGEXP_DEFS_ONLY
#if defined(_REENTRANT) || defined(_THREAD_SAFE)

struct regexp_data {
    char	*loc1, *loc2, *locs;
    int		circf;
};

#define _LOC1	(__regexp_data->loc1)
#define _LOC2	(__regexp_data->loc2)
#define _LOCS	(__regexp_data->locs)
#define _CIRCF	(__regexp_data->circf)

#define _ADVANCE(_LP, _EXPBUF) advance_r(_LP, _EXPBUF, __regexp_data)

#else

int		circf;

char		*loc1, *loc2, *locs;

#define	_LOC1	loc1
#define	_LOC2	loc2
#define	_LOCS	locs
#define _CIRCF	circf

#define	_ADVANCE(_LP, _EXPBUF)	advance(_LP, _EXPBUF)

#endif

int		sed, nbra;

#endif /* _REGEXP_DEFS_ONLY */

/* jacket-ed versions of regex.h routines */

_BEGIN_CPLUSPLUS
extern int	__regcomp __((void **, const char *, int, int *));
extern int	__regexec __((void *, const char *, int*, int*, int));
extern void	__regfree __((void *));
extern char	*strcpy __((char *, const char *));
extern char	*mbsadvance __((const char *));
extern unsigned long int strtoul __((const char *, char **, int));
_END_CPLUSPLUS

#ifndef _REGEXP_DEFS_ONLY
#if defined(_REENTRANT) || defined(_THREAD_SAFE)
char *
#ifndef __cplusplus
compile_r(__instring, __expbuf, __endbuf, __seof, __regexp_data)
char *__instring, *__expbuf;
const char *__endbuf;
int __seof;
struct regexp_data *__regexp_data;
#else
compile_r(char *__instring, char *__expbuf, const char *__endbuf, int __seof, struct regexp_data *__regexp_data)
#endif
#else
char *
#ifndef __cplusplus
compile(__instring, __expbuf, __endbuf, __seof)
char *__instring, *__expbuf;
const char *__endbuf;
int __seof;
#else
compile(char *__instring, char *__expbuf, const char *__endbuf, int __seof)
#endif
#endif	/* _REENTRANT || _THREAD_SAFE */
{
	/*
	 * User dependent declarations and initializations
	 */
#ifdef INIT
    	char *instring = __instring;
	INIT
#endif
	register int	__c;
	char		__pattern[_ESIZE];
	int		i = 0;
	void		*__re_temp = (void *) 0;
	int		__status;
	char		__cx;		/* XPG3 does not allow embedded '\n' */
	int		__nsub;
	unsigned long	n, k;		/* variables		*/
	int		m, b;		/* for			*/
	char		*__save;		/* error number mapping */

	/* re-init these at each compile */
	_CIRCF = sed = nbra = 0;
	_LOC1 = _LOC2 = _LOCS = (char *)0;

	__c = GETC();

	if (__c == __seof || __c == '\0') { /* check for immediate EOP */
		ERROR(_NOMATCHSTR);
		RETURN(0);
	}

	if (__c == '^') {
	    _CIRCF = 1;
	    __c = GETC();
	}

	__pattern[i++] = '^';		/* For advance() */

	while (__c != __seof && __c != '\0') {
		if (i >= _ESIZE) {
			ERROR(_BIGREGEXP);
			RETURN(0);
		}
		if (__c == '\\') {	/* embedded '\n' check */
			__cx = GETC();
			if (__cx == '\n') {
				ERROR(_NODELIM);
				RETURN(0);
			}
			__pattern[i++] = (char)__c;
			__pattern[i++] = (char)__cx;
		} else
			__pattern[i++] = (char)__c;

		__c = GETC();
	}

	if (__c != __seof) {
		ERROR(_NODELIM);
		RETURN(0);
	}

	__pattern[i++] = '\0';	/* BRE should be terminated by NULL */

	__status = __regcomp( &__re_temp, __pattern, 0, &__nsub);

	if (__status == 0) {
		if (__nsub > _NBRA) {
		        __regfree(__re_temp);
			ERROR(_BIGPAREN);
			return(0);
		}
		__regfree(__re_temp);
		if ( (__endbuf-__expbuf) >= i ) {
		    strcpy(__expbuf, __pattern);
		    __endbuf = __expbuf + i;
		    RETURN((char *)__endbuf);
		} else {
		    __regfree(__re_temp);
		    ERROR(_BIGREGEXP);
		    return(0);
		}
	}
	__regfree(__re_temp);

	/*
	 * start mapping error numbers
	 */

	switch (__status) {
	  default:
	  case _BIGRANGE:
	  case _BIGREGEXP:
	  case _BIGDIGIT:
	  case _BADBRACKET:
	    ERROR(__status);
	    break;

	  case _BIGPAREN:
	  case _BADPAREN:
	    __save = __pattern;
	    m = 0;
	    while (*__save++) {
		if (*__save == '(')
		    m++;
		else if (*__save == ')')
		    m--;
	    }
	    if (m == 0) {
		ERROR(_BIGPAREN);
	    } else {
		ERROR(_BADPAREN);
	    }
	    break;

	  case _NODELIM:
	    __save = __pattern;
	    m = b = 0;
	    while (*__save++) {
		if (*__save == '(')
		    m++;
		else if (*__save == ')')
		    m--;
		else if (*__save == '{')
		    b++;
		else if (*__save == '}')
		    b--;
	    }
	    if(m > 0) {
	      ERROR(_BADPAREN);
	    } else if (b > 0) {
	      ERROR(_NOCLOSEBRACE);
	    } else {
	      ERROR(__status);
	    }
	    break;

	  case _ABNORMAL:
	    __save = __pattern;
	    while (*__save != '{') {
		__save = (char *)mbsadvance(__save);
		if (!__save) {
		    ERROR(_ABNORMAL);
		}
	    }
	    __save++;
	    n = strtoul(__save, &__save, 10);
	    if (!n) {
		ERROR(_BADNUM);
		break;
	    }
	    if (n > 255) {
		ERROR(_BIGRANGE);
		break;
	    }

	    if (*__save == ',') {		/* get 2nd number */
		__save++;
		if (*__save == '\\' && *++__save != '}') {
		    ERROR (_NOCLOSEBRACE);
		    break;
		}
		k = strtoul(__save, &__save, 10);
		if (!k) {
		    ERROR(_BADNUM);
		    break;
		} else if (k > 255) {
		    ERROR(_BIGRANGE);
		    break;
		} else if (n > k) {
		    ERROR(_SEMBRACEFMT);
		    break;
		}
	    }
	    if (*__save != '\\') {
		ERROR(_SYNBRACEFMT);
		break;
	    } else if (*__save != '}') {
		ERROR(_NOCLOSEBRACE);
		break;
	    }
	    ERROR(_BADNUM);
	    break;

	  case 0:
	    break;

	  case -1:
	    ERROR(_ABNORMAL);
	}

	return(0);
}

#if defined(_REENTRANT) || defined(_THREAD_SAFE)
int
#ifndef __cplusplus
step_r(__p1, __p2, __regexp_data)
const char *__p1, *__p2;
struct regexp_data *__regexp_data;
#else
step_r(const char *__p1, const char *__p2, struct regexp_data *__regexp_data)
#endif
#else
int
#ifndef __cplusplus
step(__p1, __p2)
const char *__p1, *__p2;
#else
step(const char *__p1, const char *__p2)
#endif
#endif	/* _REENTRANT || _THREAD_SAFE */
{
    void 	*__re_temp;
    int		__eflags = 0;
    int		__status;
    int		__eo, __so;

    if ( !__p1 )
	return (0);

    if (_CIRCF)
	__status = __regcomp( &__re_temp, __p2, 0, (void *)0); /* Compile left-anchored flavor */
    else
	__status = __regcomp( &__re_temp, __p2+1, 0, (void *)0); /* Unanchored, can drift left */
    if (__status) {
	__regfree( __re_temp );
	return (0);
    }

    do {
	if (__regexec( __re_temp, __p1, &__so, &__eo, __eflags) == 0) {
	    _LOC2 = (char *)__p1 + __eo;
	    _LOC1 = (char *)__p1 + __so;
	    __regfree(__re_temp);
	    return (1);
	}

	if (_CIRCF)
	    break;

	__eflags = 1;			/* Turn on REG_NOTBOL */
    } while ((__p1 = (char *)mbsadvance(__p1)));

    __regfree(__re_temp);
    return(0);
}


#if defined(_REENTRANT) || defined(_THREAD_SAFE)
int
#ifndef __cplusplus
advance_r(__lp, __expbuf, __regexp_data)
const char *__lp, *__expbuf;
struct regexp_data *__regexp_data;
#else
advance_r(const char *__lp, const char *__expbuf, struct regexp_data *__regexp_data)
#endif
#else
int
#ifndef __cplusplus
advance(__lp, __expbuf)
const char *__lp, *__expbuf;
#else
advance(const char *__lp, const char *__expbuf)
#endif
#endif
{
    int		__status;
    void	*__re_temp;
    int		__eo, __so;

    if (__status = __regcomp(&__re_temp, __expbuf, 0, (void *)0)) {
	/*
	 * Should NEVER happen unless malloc fails
	 */
	__regfree(__re_temp);
	return (0);
    }

    if (__regexec(__re_temp, __lp, &__so, &__eo, 0) == 0) {
	_LOC1 = (char *)__lp;
	_LOC2 = (char *)__lp + __eo;
	__regfree(__re_temp);
	return (1);
    }
    __regfree(__re_temp);
    return (0);
}

#endif /* _REGEXP_DEFS_ONLY */
#ifdef _NAME_SPACE_WEAK_STRONG
#undef __regcomp
#undef __regexec
#undef __regfree
#endif
