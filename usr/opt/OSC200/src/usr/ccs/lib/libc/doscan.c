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
static char	*sccsid = "@(#)$RCSfile: doscan.c,v $ $Revision: 4.3.10.6 $ (DEC) $Date: 1993/10/18 20:07:31 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * Modification History
 *  Fred Iannelli II     23-Jul-1991
 *  001 - Tin Qar 282 - %*n should not attempt to store the count. 
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif

/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: _doscan, nf, NLnan_doscan, getcc, ungetcc 
 *
 * ORIGINS: 3, 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * sccsid[] = "doscan.c	1.49  com/lib/c/io,3.1,9021 5/4/90 19:06:00";
 */

/*LINTLIBRARY*/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdio.h>
#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include <ctype.h>
#include <sys/localedef.h>
#include <varargs.h>
#include <values.h>
#include <fp.h>
#include <nl_types.h>
#include <langinfo.h>
#include <errno.h>
#include "print.h"

#define	PATTERN		"%1$"			/* arg ordering pattern */

#define NCHARS		(1 << BITSPERBYTE)

#ifdef _THREAD_SAFE
#define GETC(iop, count, cret)  { \
	if ((iop->_flag & _IONOFD) && iop->_cnt <= 0) \
		cret = WEOF; \
	else if ((cret = getwc_unlocked(iop)) != WEOF) \
		count += wctomb(mb_buf, cret); \
}
#else
#define GETC(iop, count, cret)	{ \
	if (fast_getc_flag) { \
	    cret = ((getc_ch = getc(iop)) != EOF) ? (wint_t) getc_ch : WEOF; \
	    if (cret != WEOF) \
		((cret <= 255) ? (count)++ : (count)--); \
	} \
	else \
	    cret = ((iop->_flag & _IONOFD) && iop->_cnt <= 0) \
	         ? WEOF : getWC(iop, &count); \
}
#endif  /* _THREAD_SAFE */


#define GETB(iop, count)	(((iop->_flag & _IONOFD) \
				  && iop->_cnt <= 0) ? EOF : getcc(iop, count))


static unsigned char	*setup(unsigned char *, char *);
extern int		_argnum(char **, int *, arglist *, int *);
static int		reorder(char *, va_list *, char **, arglist *);
static wint_t		ungetWC(wint_t, FILE*, int*);
static wint_t		getWC(FILE *, int *);
static int		getcc(FILE *, int *);
static int		ungetcc(int, FILE*, int*);

/*                                                                    
 * FUNCTION: _doscan: common code for scanf, sscanf, fscanf
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	     - returns number of matches
 *	     - EOF on failure
 */
int
_doscan(register FILE		*iop,
	register unsigned char	*format,
	va_list			args)
{
	char		tab[NCHARS];
	register int	ch;
	int		nmatch = 0;	/* matches found (return value) */
	int		len;
	int		num_str_len;
	wint_t		inchar;
	int		stow;
	int		size;
	char		wlflag = 0;
	int		incount = 0;	/* chars read from input stream */
	int		inpeof = 0;	/* EOF on input reached */
	register int	getc_ch;
	register int	fast_getc_flag;
	char		mb_buf[MB_LEN_MAX+1];

	/* Data for arg reordering.
	 */
	unsigned char	*nformat = 0;		/* reordered format string */
	arglist		nargs[NL_ARGMAX];	/* reordered arguments */
	arglist		*nargp = nargs;
	int		reorderflag = UNSET;

	fast_getc_flag = ((iop->_flag & _IONOFD) == 0 &&
			  (__lc_charmap->cm_mb_cur_max == 1) &&
			  (*__lc_charmap->core.wctomb == NULL));

	if (strstr((char *)format, PATTERN) != (char *)NULL) {
		reorderflag = TRUE;
		if (reorder((char *)format, &args, (void *)&nformat, nargs) < 0)
			return (EOF);
		format = nformat;
	} else
		reorderflag = FALSE;

	/* Main Loop:  Read the format specifications for values to
	 * to be scanned one at a time.  For each specification read
	 * from the input stream the expected type of value.
	 */

	for ( ; ; ) {
		if ((ch = *format++) == '\0') {
			RFREE(nformat);
			return (nmatch); /* end of format */
		}

		if (isspace(ch)) {
			GETC(iop, incount, inchar);
			while (iswspace(inchar))
				GETC(iop, incount, inchar);
			if (ungetWC(inchar, iop, &incount) != WEOF)
				continue;
			do
				ch = *format++;
			while (isspace(ch));
			if (ch != '%' || *format != 'n')
				break;
		}

		if (!wlflag)
			if (ch != '%' || (ch = *format++) == '%') {
				GETC(iop, incount, inchar);
				if (inchar == (wint_t)ch)
					continue;
				if (ungetWC(inchar, iop, &incount) != WEOF) {
					RFREE(nformat);
					/* failed to match input */
					return (nmatch);
				}
				break;
			}

		/* When stow is 0, the input is read but not written. */
		if (ch == '*') {
			stow = 0;
			ch = *format++;
		} else
			stow = 1;

		/* N flag is no-op starting V3.2 */
		if (ch == 'N')
			ch = *format++;

		/* B flag is no-op starting V3.2 */
		if (ch == 'B')
			ch = *format++;

		/*  If wlflag is not already set, check to see if current
		 *  character is a l or w.  Set wlflag accordingly and continue
		 *  processing.
		 */

		if (!wlflag)
			if (ch == 'l')
				wlflag = 'l';
			else if (ch == 'w') {
				wlflag = 'w';
				continue;
			}

		for (len = 0; isdigit(ch); ch = *format++)
			len = len * 10 + ch - '0';
		if (len == 0)
			len = MAXINT;

		/* Check for syntax "%[*][w][B][N][<length>]<fmt_desc>"
		 */
		if (ch == 'N' || ch == 'B' || ch == 'w')
			break;

		/* long double == double in this implementation */
		if (ch == 'L')
			ch = 'l';
		if ((size = ch) == 'l' || size == 'h')
			ch = *format++;

		if (ch == 'n') {
			if (!stow) 
				continue;
			switch (size) {
			case 'h':
				*get_arg(args, nargp, short *) = incount;
				break;
			case 'l':
				*get_arg(args, nargp, long *) = incount;
				break;
			default:
				*get_arg(args, nargp, int *) = incount;
			}
			continue;
		}

		if (inpeof)
			break;

		/* A call to setup defines scansets. */
		if (ch == '\0' || 
		    ch == '[' && (format = setup(format, tab)) == NULL) {
			RFREE(nformat);
			return (EOF); /* unexpected end of format */
		}
		if (ch != 'c' && ch != 'C' && ch != '[') {
			GETC(iop, incount, inchar);
			while (iswspace(inchar))
				GETC(iop, incount, inchar);
			if (ungetWC(inchar, iop, &incount) == WEOF)
				break;
		}


		/* if wlflag is true, current character must be set listed
		 * below.  If it is not, a format error is reached and
		 * we should stop processing.
		 */
		if (wlflag) {
			switch (ch) {
			case 's':
			case 'c':
			case 'd':
			case 'i':
			case 'o':
			case 'u':
			case 'X':
			case 'x':
			case 'p':
			case 'E':
			case 'e':
			case 'f':
			case 'G':
			case 'g':
				break;
			default:
				RFREE(nformat);
				return (nmatch != 0 ?nmatch : inpeof ? EOF : 0);
			}
		}

		inpeof = 0;

		/* No more parameters left in which to place the output and
		 * we are processing an output specification.
		 * Instead of crashing, let's just return.
		 */
		if (stow) {
#if __alpha
		    /* Must make sure we are checking correct arglist */
		    if (reorderflag) {
		        if ((nargp) && (nargp->da_type == DA_T_UNDEF))
			    break;
		    } else
			if(*(char **)(args._a0+args._offset) == NULL)
			    break;
#else
		  if(args == NULL) /* end of input */
			break;
#endif
		}

		/* If a character string is specified, then . . . else
		 * if a number . . .
		 * Note that "number" is not restricted to handling
		 * digit strings, it must also recognize special IEEE
		 * values (INF, NaNQ, NaNS).
		 */

		num_str_len = len;
		if (ch == 'S' || ch == 'C' || 
		    ch == 'c' || ch == 's' || ch == '[') {
			char	*ptr;
			wint_t	Nch;
			char	*start;
			char	cpsize, cpbuf[MB_LEN_MAX];
			char	i;
			int	lowlim = 0;
			wint_t	*NLstart;
			wint_t	*NLptr;

			if ((wlflag == 'l') && ((ch == 'c') || (ch == 's')))
				wlflag = '\0';

			if (ch == 'S' || ch == 'C'
			    || (wlflag && (ch == 's' || ch == 'c')))
				NLstart = NLptr = stow 
					? get_arg(args, nargp, wint_t *) : NULL;
			else
				start = ptr = stow 
					? get_arg(args, nargp, char *) : NULL;
			if ((ch == 'c' || ch == 'C') && num_str_len == MAXINT)
				num_str_len = 1;

			if (ch == '[') {
			    /*
			     * Read a byte-at-a-time, not a character.
			     */
			    while ( (Nch = GETB(iop,&incount)) != EOF) {

				if ( tab[Nch] ) /* Not in the scan-set? */
				    break;

				if (stow)
				    *ptr++ = Nch;
				else
				    ptr++;
				if (--num_str_len <= 0)
				    break;
			    }
			} else if (!((wlflag && (ch == 's' || ch == 'c' ))
			      || ch == 'S' || ch == 'C')) {
			    /*
			     * Reading a normal (possibly multibyte) string 
			     * into a %s or %c format item
			     */
				GETC(iop, incount, Nch);
				while (Nch != WEOF
				      && !(ch == 's' && iswspace(Nch))) {

					cpsize = wctomb(cpbuf, Nch);

					lowlim += cpsize - 1;

					if (stow)
						for (i = 0; i < cpsize; i++)
							*ptr++ = cpbuf[i];
					else
						ptr += cpsize;

					if (--num_str_len <= lowlim)
						break;

					GETC(iop, incount, Nch);
				}
			}
			else {
				GETC(iop, incount, Nch);
				while (Nch != WEOF) {
				    /*
				     * Stop on white space unless C or lc format
				     */
				    if ((wlflag && ch=='c') || ch == 'C')
					;
				    else if (iswspace(Nch))
					break;

				    if (stow)
					*NLptr = Nch;
				    NLptr++;

				    if (--num_str_len <= 0)
					break;
				    GETC(iop, incount, Nch);
				}
			}

			if (Nch == WEOF || (num_str_len > lowlim && 
					    ungetWC(Nch,iop,&incount) == WEOF))
				(inpeof)++;	/* end of input */

			if (!((wlflag && (ch == 's' || ch == 'c'))
			      || ch == 'S' || ch == 'C')) {

				if (ptr == start) {
					size = 0; /* no match */
					goto num_str_ret;
				}

				/* note wc excluded above */
				if (stow && ch != 'c')
					*ptr = '\0';
			} else {
				if (NLptr == NLstart) {
					size = 0;		/* no match */
					goto num_str_ret;
				}
				if (stow && !(ch == 'C' || 
				   (wlflag && ch == 'c')))
					*NLptr = '\0';
			}
			size = 1;	/* successful match */
			goto num_str_ret;
		}
		else {
			char	numbuf[256]; /* Holds digits of int or float */
			char	*np = numbuf;
			wint_t	c;
			int	base;
			int	digitseen = 0;
			long	lcval = 0;

			int	pointer = 0;
			wchar_t	radix; 	/* locale's radix as wide-char */
			char	*radixv; /* locale's radix as multibyte */

			const char	*rest;	/* Identify unconverted tail */
			int	SaveErrno;
			double	dval;

			switch (ch) {
			  case 'E':
			  case 'e':
			  case 'f':
			  case 'G':
			  case 'g':

			    radixv = __lc_locale->nl_info[RADIXCHAR];
			    /* Bad radix char */
			    if (mbtowc(&radix, radixv, MB_CUR_MAX) <= 0)
				radix = '.';	/* Use C locale default */

			    GETC(iop, incount, c);
			    if (c==WEOF) {	/* Empty input IS an error */
				size = 0;
				goto num_str_ret;
			    }

			    /* Optional +|- */
			    if (num_str_len>0 && (c == '+' || c == '-')) {
				--num_str_len;
				*np++ = c;
				GETC(iop, incount, c);
			    }

			    /* Optional Integer part */
			    while(num_str_len>0 && iswdigit(c)) {
				digitseen++;
				--num_str_len;
				*np++ = c;
				GETC(iop, incount, c);
				if (c==WEOF) break;
			    }

			    /* Radix-point */
			    if (num_str_len>0 && c == radix ) {
				digitseen++;
				--num_str_len;
				*np++ = c;
				GETC(iop, incount, c);
			    }

			    /* Optional fraction */
			    while (num_str_len>0 && iswdigit(c)) {
				digitseen++;
				--num_str_len;
				*np++ = c;
				GETC(iop, incount, c);
				if (c == WEOF) break;
			    }

			    /* Optional exponent */
			    if (num_str_len>0 && (c == 'E' || c == 'e')) {
				digitseen++;
				--num_str_len;
				*np++ = c;
				GETC(iop, incount, c);

				if (num_str_len>0 && (c == '-' || c == '+') ){
				    --num_str_len;
				    *np++ = c;
				    GETC(iop, incount, c);
				}
		
				while (num_str_len>0 && iswdigit(c) ) {
				    --num_str_len;
				    *np++ = c;
				    GETC(iop, incount, c);
				    if (c == WEOF) break;
				}
			    }

			    /*
			     * Still a few special cases left.  It might 
			     * be INF or NaN
			     */
			    if (!digitseen) {
				static const char INF[] = {"INF"};
				static const char NAN[] = {"NAN"};

				for (rest=INF; 
				     (num_str_len>0) && (towupper(c) == *rest); 
				     rest++) {
				    --num_str_len;
				    *np++ = c;
				    GETC(iop, incount, c);
				}

				if (rest == INF) { /* Not INF */
				    for(rest=NAN; 
				      (num_str_len>0) && (towupper(c) == *rest);
					rest++) {
					--num_str_len;
					*np++ = c;
					GETC(iop, incount, c);
				    }

				    if (!*rest)	/* Looks good- matched NAN */
					switch(towupper(c)) {
					  case 'S':
					  case 'Q':
					    *np++ = c;
					    GETC(iop, incount, c);
					    break;
					}
				}
			    }
		    
			    /* At this point, we've got a character that is 
			     * either WEOF or something that cannot be part 
			     * of a FP sequence.
			     */
			    if (ungetWC(c, iop, &incount) == WEOF)
				(inpeof)++;

			    *np = '\0';

			    /* Handle conversion errors */
			    SaveErrno = geterrno();
			    seterrno(0);

			    dval = strtod(numbuf, (char **)&rest);
	    
			    seterrno(SaveErrno);

			    if (rest == numbuf)	{ /* Bad format for fp number */
				size = 0;		/* FAILURE! */
				goto num_str_ret;
			    }
	    
			    /* Put back any extra characters we read */
			    while (rest != np) {
				ungetWC(*(unsigned char *)--np, iop, &incount);
			    }
	    
			    if (stow) {
				if (size == 'l') {
				    *get_arg(args,nargp,double *) = dval;
				} else {
				    *get_arg(args,nargp,float *) = (float) dval;
				}
			    }
			    size = 1;
			    goto num_str_ret;

			case 'd':
			case 'u':
				base = 10;
				break;
			case 'o':
				base = 8;
				break;
			case 'p':
				pointer = 1;
				/*
					fall through
				*/
			case 'X':
			case 'x':
				base = 16;
				break;
			case 'i':
				base = -1;
				break;
			default:
				/* unrecognized conversion character */
				size = 0;
				goto num_str_ret;

			} /* switch on format */


			/*
			 * Put characters in numbuf for remaining integral cases
			 */

/*
 * GETDIGIT reads no more than "num_str_len" characters.  Once it has read
 *	'num_str_len' characters, it returns 0.  On EOF, it returns WEOF.
 */
#define GETDIGIT(c)	if (--num_str_len >= 0) GETC(iop,incount,c) else c = 0;

			GETDIGIT(c); /* Get first character */

			if (c=='-' || c == '+') {
			    *np++ = c;
			    GETDIGIT(c);
			}

			/*
			 * Accept hex prefix if reading %x or %i
			 * n.b. that we have to be careful (later) about "0XQ"
			 */

			if ((base == -1 || base == 16) && c == '0' ) {
			    digitseen++;
			    GETDIGIT(c);

			    if (c == 'x' || c == 'X') {
				*np++ = '0';
				*np++ = c;
				base = 16;
				GETDIGIT(c);
			    } else {
				num_str_len++;
				if (c)
				    ungetWC(c, iop, &incount);
				c = '0'; /*restore original value */
			    }
			}

			/* set base if %i format specified */
			if (base == -1 )
			    if (c == '0') {
				digitseen++;
				base = 8;
				*np++ = c;
				GETDIGIT(c);
			    } else
				base = 10;

			/*
			 * Store the number in numbuf as it's read in.
			 * A blank will cause a break in the loop.
			 */

			while (c != WEOF) {
			    if (base == 16) {
				if (!iswxdigit(c)) {
				    break;
				}
			    } else if (!iswdigit(c)) {
				break; /* Ends DIGIT STRING */
			    }
			    digitseen++;
			    *np++ = c;
			    GETDIGIT(c);
			}
	
			*np = '\0';

			/*
			 * If we read an "extra characters", give back the 
			 * last one
			 */
			if (c != 0 && ungetWC(c, iop, &incount) == WEOF)
			    (inpeof)++;

			SaveErrno = geterrno();
			seterrno(0);

			switch (ch) {
			  case 'u':
			  case 'x':
			  case 'o':
			  case 'X':
			  case 'p':
			    lcval = strtoul(numbuf, (char **)&rest, base);
			    break;

			  default:
			    lcval = strtol(numbuf, (char **)&rest, base);
			}

			if (geterrno() || numbuf == rest) {
			    seterrno(SaveErrno);
			    size = 0;
			    goto num_str_ret;
			}

			seterrno(SaveErrno);

			while (np != rest) {
			    ungetWC(*(unsigned char *)--np, iop, &incount);
			}

			if (stow) {
			    if (!pointer)
				if (size == 'l')
				    *get_arg(args,nargp,long *) = lcval;
				else if (size == 'h')
				    *get_arg(args,nargp,short *) = (short)lcval;
				else
				    *get_arg(args,nargp,int *) = (int)lcval;
			    else {
				*get_arg(args,nargp,void **) =(void *)lcval;
			    }
			}

			size = digitseen; /* successful match if non-zero */
			goto num_str_ret;
		}

num_str_ret:

		if (size != 0)
			nmatch += stow;

		/* 'w' qualifiers should affect only the current
		 * descriptor, so we turn them off here.
		 */

		wlflag = 0;

		if (size == 0) { /* failed to match input */
			RFREE(nformat);
			return (nmatch != 0 ? nmatch : inpeof ? EOF : 0);
		}
	}
	RFREE(nformat);
	return (nmatch != 0 ? nmatch : EOF); /* end of input */
}



static unsigned char	*
setup(unsigned char *fmt, char *tab)
{
	int	b, c, d, t = 0;

	if (*fmt == '^') {
		t++;
		fmt++;
	}
	(void)memset((void *)tab, !t, (size_t)NCHARS);
	if ((c = *fmt) == ']' || c == '-') { /* first char is special */
		tab[c] = t;
		fmt++;
	}
	while ((c = *fmt++) != ']') {
		if (c == '\0')
			return (NULL); /* unexpected end of format */

		/* This is where the scanset range is specified,
		 * b represents the first and d the last character
		 * included in the scanset.
		 */
		if (c == '-' && (d = *fmt) != ']' && (b = fmt[-2]) < d) {
			(void)memset((void *) & tab[b], t, (size_t)(d - b + 1));
			fmt++;
		} else
			tab[c] = t;
	}
	return (fmt);
}


/*
 *	Reorder goes through a format statement containing variably
 *	ordered arguments and reorders the arguments accordingly,
 *	stripping out the x$'s.  Upon success, reorder() returns 0;
 *	the new format string is returned via the argument "bp_new_fmt",
 *	and the new arg pointer is returned via the argument "vlist".
 *	Upon failure, reorder() returns -1;
 */

/* Convert a digit character to the corresponding number */
#define tonumber(x)	((x)-'0')
#define MBLEN(x)	((mbleng = mblen(x, mb_cur_max)) > 1 ? mbleng : 1)

static int
reorder(char	*format,
	va_list *args,
	char	**bp_new_fmt,	/* new format */
	arglist	*vlist)		/* new argument list */
{
	char	*new_fmt;	/* working ptr in the new format string */
	int	fcode;		/* Format code */

	int	length;		/* l */

	arglist	alist[NL_ARGMAX];	/* original order list */

	int	arg_index;	/* number of args (i.e. % occurrences) */
	int	arg_count;	/* number of args in format string */
	int	max_arg;	/* highest numbered arg */

	/* work variable(s) */
	register arglist	*ap, *vp;
	int	mb_cur_max;
	int	mbleng;
	int	i;

	mb_cur_max = MB_CUR_MAX;

	/* The format loop interrogates all conversion characters to
	 * determine the address and type of each argument, building the
	 * alist and vlist arrays and filling in new_fmt.
	 */
	arg_count = max_arg = 0;

	*bp_new_fmt = new_fmt = malloc(strlen(format) + 1);

	if (new_fmt == 0)
		return (-1);

	for (i = strlen(format); i; i--)
		*(new_fmt + i) = '\0';

	/* Make sure no holes are in format string (checked later) */
	for (i = 0; i < NL_ARGMAX; i++)
		alist[i].da_type = DA_T_UNDEF;

	new_fmt--;
	for ( ; ; ) {
		while ((*++new_fmt = *format) != '\0') {
			if (*format == '%') {
				if (*(format + 1) == '%'
				    || *(format + 1) == '*') {
					*++new_fmt = *++format;
				} else
					break;
			}
			format++;
		}

		fcode = *format;
		if (fcode == '\0') {
			break;	/* Now do the get_args loop */
		}
		/*
		 *	% has been found.
		 *	First extract digit$ from format and compute arg_index.
		 *	Next parse the format specification.
		 */
		format++;
		if (_argnum(&format, &arg_index, alist, &max_arg) != 0)
			goto badret;

		/* Now store pointer to this arg into arg-count-based
		 * list.
		 */
		if (arg_count >= NL_ARGMAX)
			goto badret;
		ap = &alist[arg_index];
		vlist[arg_count].da_arg.au_arg = ap;
		arg_count++;

		for ( ; ; ) { /* Scan the <flags> */
			switch (fcode = *++new_fmt = *format++) {
			case ' ':
			case 'B':
			case 'N':
				continue;
			}
			*new_fmt-- = '\0';
			format--;
			break;
		}

		/* Scan the field width */
		while (isdigit(fcode = *format))
			*++new_fmt = *format++;

		/* Scan the length modifier */
		length = 0;
		switch (*format) {
		case 'L': /* long double == double in this implementation */
		case 'l':
			length = 1;
			*++new_fmt = *format++;
			break;
		case 'h':
			length = -1;
			*++new_fmt = *format++;
			break;
		}

		switch (fcode = *++new_fmt = *format++) {

		case 'd':
		case 'i':
		case 'o':
		case 'u':
		case 'x':
		case 'X':
		case 'n':
			switch (length) {
			case -1	:
				ap->da_type = DA_T_SHORTP;
				break;
			case 0	:
				ap->da_type = DA_T_INTP;
				break;
			case 1	:
				ap->da_type = DA_T_LONGP;
				break;
			}
			break;

		case 'p':
			ap->da_type = DA_T_POINTER;
			break;

		case 'E':
		case 'e':
		case 'f':
		case 'G':
		case 'g':
			if (length == 1)
				ap->da_type = DA_T_DOUBLEP;
			else
				ap->da_type = DA_T_FLOATP;
			break;

		case 'c':
		case 's':
			if (length == 0) {
				ap->da_type = DA_T_POINTER;
				break;
			}
			/* FALLTHROUGH */
		case 'C':
		case 'S':
			ap->da_type = DA_T_WINTP;
			break;

		case '[':	/* string of chars follows ending with a ']' */ 
			{
				int	loop_cnt;

				while (*format != '\0' && *format != ']') {
					loop_cnt = MBLEN(format);
					for (i = 0; i < loop_cnt; i++)
						*++new_fmt = *format++;
				}
			}

			fcode = *++new_fmt = *format;
			if (fcode == ']')
				format++;
			ap->da_type = DA_T_POINTER;
			break;

		case '\0':	/* unexpected end of format; return error */
			goto badret;
		}
	}

	/* Get ARGS Loop:  fill in the alist array...pointers to args. */
	for (i = 0, ap = alist; i < max_arg; i++, ap++) {
		switch (ap->da_type) {
		case DA_T_UNDEF	:
			/* Make sure no holes are in format string */
			goto badret;
		case DA_T_POINTER:
			ap->da_arg.au_pointer = va_arg(*args, void *);
			break;
		case DA_T_SHORTP:
			ap->da_arg.au_shortp = va_arg(*args, short *);
			break;
		case DA_T_INTP	:
			ap->da_arg.au_intp = va_arg(*args, int *);
			break;
		case DA_T_WINTP	:
			ap->da_arg.au_wintp = va_arg(*args, wint_t *);
			break;
		case DA_T_LONGP	:
			ap->da_arg.au_longp = va_arg(*args, long *);
			break;
		case DA_T_FLOATP:
			ap->da_arg.au_floatp = va_arg(*args, float *);
			break;
		case DA_T_DOUBLEP:
			ap->da_arg.au_doublep = va_arg(*args, double *);
			break;
		}
	}

	/* Reorder ARGS Loop:  loop through vlist array to gather
			       argument addresses and types */
	for (i = 0, vp = vlist; i < arg_count; i++, vp++) {
		ap = vp->da_arg.au_arg;
		vp->da_arg = ap->da_arg;
		vp->da_type = ap->da_type;
	}

	return (0);

badret:
	free(*bp_new_fmt);
	*bp_new_fmt = 0;
	return (-1);
}


/* Routines to keep track of chars read from input stream  */
static wint_t
getWC(FILE *iop, int *incount)
{
	wint_t	wc;
	char	mb_buf[MB_LEN_MAX+1];	/* junk */

	if ((wc = getwc(iop)) != WEOF)
		if (*__lc_charmap->core.wctomb == NULL)
			*incount += ((wc <= 255) ? 1 : -1);
		else
			*incount += (*__lc_charmap->core.wctomb)
					(mb_buf,wc,__lc_charmap);

	return (wc);
}

static wint_t
ungetWC(register wint_t wc, register FILE *iop, register int *incount)
{
	char	mb_buf[MB_LEN_MAX+1];	/* junk */

	if (wc != WEOF)
		if (*__lc_charmap->core.wctomb == NULL)
			*incount -= ((wc <= 255) ? 1 : -1);
		else
			*incount -= (*__lc_charmap->core.wctomb)
					(mb_buf,wc,__lc_charmap);

	return (ungetwc(wc,iop));
}


static int
getcc(FILE *iop, int *incount)
{
    int c;

#ifdef  _THREAD_SAFE
	if ((c = getc_unlocked(iop)) != EOF)
#else
	if ((c = getc(iop)) != EOF)
#endif	/* _THREAD_SAFE */
		*incount += 1;

	return (c);
}


static int
ungetcc(int c, FILE *iop, int *incount)
{
	if (c != EOF)
		*incount -= 1;

	return (ungetc(c,iop));
}
