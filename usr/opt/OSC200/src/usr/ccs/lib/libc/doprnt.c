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
static char	*sccsid = "@(#)$RCSfile: doprnt.c,v $ $Revision: 4.2.12.4 $ (DEC) $Date: 1993/08/12 21:16:55 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1.1
 */
/*
 * COMPONENT_NAME: (LIBCPRNT) Standard C Library Print Functions 
 *
 * FUNCTIONS: _doprnt 
 *
 * ORIGINS: 3, 27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef index
#undef index
#endif

#include <stdio.h>
#ifdef _THREAD_SAFE
#include "stdio_lock.h"
#endif	/* _THREAD_SAFE */
#include <ctype.h>
#include <sys/localedef.h>
#include <limits.h>
#include <langinfo.h>
#include <locale.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include <values.h>
#include <string.h>
#include <fp.h>
#include <stdlib.h>
#include <wchar.h>
#include "print.h"	/* parameters & macros for doprnt */

static int	reorder(char *, va_list *, char **, arglist *);

/* all putc's in doprnt call the macro 'PUTC', so that we can break */
/* from doprnt if it interrupted. */

#ifdef _THREAD_SAFE
#undef	putc
#define	putc	putc_unlocked
#undef	fwrite
#define	fwrite	fwrite_unlocked
#endif /* _THREAD_SAFE */


#define PUTC(A1,A2)	if (putc(A1, A2) < 0) { \
				RFREE(nformat); \
				return (-1); }

#define FWRITE(A1,A2,A3,A4) {if (fwrite(A1,A2,A3,A4) != A3) { \
			RFREE(nformat); \
			return (-1); } }

#define PUT(p, n)	{if (n == 1) { PUTC((int)(*p & 0xff), iop); } \
			else if (iop->_flag & _IONOFD) \
				iop->_ptr = n + (unsigned char *) \
						memcpy((void *)iop->_ptr, \
						       (void *)p, (size_t)n); \
			else FWRITE((void *)p, (size_t)1, (size_t)n, iop);}


#define F_BUFSIZ 512

#define F_PUTC(A1,A2)	if (fbuf_count < F_BUFSIZ) \
				fbuf[fbuf_count++] = A1; \
			else { \
				PUT(fbuf,fbuf_count); \
				fbuf_count = 0; \
				fbuf[fbuf_count++] = A1; }

#define F_PUT(p, n)	if ((fbuf_count + n) < F_BUFSIZ) { \
				memcpy((void *)&fbuf[fbuf_count], \
						(void *)p, (size_t)n); \
				fbuf_count += n; } \
			else if (n < F_BUFSIZ) { \
				PUT(fbuf,fbuf_count); \
				memcpy((void *)&fbuf[0], \
						(void *)p, (size_t)n); \
				fbuf_count = n; } \
			else { \
				PUT(fbuf,fbuf_count); \
				PUT(p,n); \
				fbuf_count = 0; }

#define F_FLUSH()	if (fbuf_count != 0) \
				PUT(fbuf,fbuf_count);


#define WCTOMB(c,x)     ((rc = wctomb(c, x)) > 1 ? rc : 1)
#define MBTOWC(c,x)     ((rc = mbtowc(x, c, mb_cur_max)) > 1 ? rc : 1)

/*
 *	C-Library routines for floating conversion
 */
#ifdef  _THREAD_SAFE
extern int	fcvt_r(), ecvt_r();
#else
extern char	*fcvt(), *ecvt();
#endif	/* _THREAD_SAFE */

/*
 * FUNCTION: regroup
 *
 * DESCRIPTION:
 * Takes a buffer of digits and inserts the grouping separator appropriately.
 *
 * INPUTS:
 * 	locdata		localeconf() results
 *	digstring	The digits to be grouped (NUL-terminated)
 *	bufend		Address of 'end' of a buffer large enough to hold
 * 			all the converted digits.
 *	size		size of buffer
 */
static char *
regroup(struct lconv *locdata, const char *digstring, char *bufend, int size)
{
    char *group = locdata->grouping;
    const char *dp;
    char *gp;
    int	i,gc,gl;

    if ((unsigned char)group[0] == CHAR_MAX || 	/* No grouping supported */
	group[0] == 0)				/*  or starts with "repeat" */
	return strcpy(bufend-size, digstring);  /* Give back string as-is */

    dp = digstring + strlen(digstring);
    gl = strlen(locdata->thousands_sep);

    i = 0;					/* Start at first group */

    *--bufend = '\0';				/* Make NUL-terminated */
    --size;					/* Count in outbuf space */

    while( size > 0 && dp != digstring) {	/* Still uncopied digits... */
	
	for(gc = group[i]; gc !=0; gc--)	/* Write one group */
	    if (dp != digstring)
		--size,*--bufend = *--dp;
	    else
		break;

	if (dp != digstring) {
	    /*
	     * There are still more digits coming, so write a separator
	     */
	    bufend -= gl;
	    size -= gl;
	    if (size < 0)
		break;		/* No more space in buffer */

	    memcpy(bufend,locdata->thousands_sep, gl);
	}

	if (group[i+1] != 0 && (unsigned char)group[i] != CHAR_MAX)
	    /*
	     * If it were zero, that says repeat current grouping.
	     * If it were CHAR_MAX, that says 'no more grouping'
	     * Otherwise, take next grouping factor
	     */
	    i++;
    }

    /* Last check for additional digits to dump */

    while( --size > 0 && dp != digstring)
	*--bufend = *--dp;

    return bufend;
}



/*
 * FUNCTION: _doprnt: common code for printf, fprintf, sprintf
 *	modifications to support ieee NaNs and +-infinity +-zero
 *	no longer calls KillNaN
 *
 *	Originaly derived from merge of NLdoprnt 8.9 and doprnt 1.11 then
 *	changes from 1.12 com/lib/c/prnt,3.1,8910" added
 */
int
_doprnt(register char	*format,
	va_list		args,
	register FILE	*iop)
{
	int		count = 0;	/* counts the number of output bytes */

	wchar_t		*wcp, wc;	/* char pointers used in 'S' format */

	register char	*bp, *p; /* start/end points for value to be printed */

	char	*radchr;	/* character pointer to the radix character */

	struct lconv	*locdata = NULL; /* grouping and separators */
	
	int	width, prec;	/* Field width and precision */

	register int	fcode;	/* Format code */

	int	leadz;		/* int indicating leading zeros format */

	int	lzero, rzero;	/* no. of padding zeroes required left/right */

	/* Flags - nonzero if corresponding character is in format */
	int	length;		/* l */
	int	fplus;		/* + */
	int	fminus;		/* - */
	int	fblank;		/* blank */
	int	fsharp;		/* # */
	int	hflag;
	int	wlflag;		/* e.g. %ws or %wc */

	char	buf[NDIG];	/* Values are developed in this buffer */
	char	*mbbuf = NULL;

	/* Pointer to sign, "0x", "0X", or empty */
	char	*prefix;

	char	*suffix;	/* Exponent or empty or blank */

	char	expbuf[MAXESIZ + 1];	/* Buffer to create exponent */

	long	val;		/* The value being converted, if integer */
	double	dval;		/* The value being converted, if real */
	int	mb_cur_max;	/* The value of MB_CUR_MAX */

#ifdef  _THREAD_SAFE
	char	cvtbuf[NDIG];	/* cvt buffer plus extra space */
#endif	/* _THREAD_SAFE */

	int	decpt, sign;	/* Output values from fcvt and ecvt */

	char	*tab;		/* translate table for digits of whatever radix */

	/* Work variables */
	int	k, hradix, lowbit;
	int	n = 0;		/* Actual width of component */
	int	i, j, nb, rc;

	/* Data for arg reordering.
	 */
	char	*nformat = 0;		/* reordered format string */
	arglist	nargs[NL_ARGMAX];	/* reordered arguments */
	arglist	*nargp = nargs;
	int	reorderflag = UNSET;

	/* Buffer for doing fast I/O */
	char fbuf[F_BUFSIZ];
	int fbuf_count;

	fbuf_count = 0;
	expbuf[MAXESIZ] = '\0';
	mb_cur_max = MB_CUR_MAX;
	tab = "0123456789abcdef";

	/*
	 *	The main loop -- this loop goes through one iteration
	 *	for each string of ordinary characters or format specification.
	 */
	for ( ; ; ) {
		bp = format;
		while ((fcode = *format) != '\0' && fcode != '%') {
			if ((p = strchr(format, '%')) == NULL)
				i = strlen(format);
			else
				i = p - format;
			F_PUT(format, i);
			format += i;
			count += i;
		}

		if (fcode == '\0') { /* end of format; normal return */
			F_FLUSH();
			RFREE(nformat);
			return (ferror(iop) ? EOF : count);
		}
		/*
		 *	% has been found.
		 *	First, parse the format specification.
		 */
		fplus = fminus = fblank = fsharp = leadz = 0;
		hflag = wlflag = length = 0;

		if (reorderflag == UNSET && format[1] != '%') {
			if (isdigit(format[1])) {
				char	*p = format + 1;

				while (isdigit(*p))
					p++;
				if (*p == '$') {
					reorderflag = TRUE;
					if (reorder(format, &args,
					    &nformat, nargs) < 0)
						return (EOF);
					format = nformat;
				}
			}
			if (reorderflag == UNSET) {
				reorderflag = FALSE;
			}
		}

		for ( ; ; ) { /* Scan the <flags> */
			switch (fcode = *++format) {
			case '+':
				fplus++;
				continue;
			case '-':
				fminus++;
				leadz = 0;
				continue;
			case ' ':
				fblank++;
				continue;
			case '#':
				fsharp++;
				continue;
			case 'B':		/* no-op flag, obsolescent */
				continue;
			case 'N':		/* no-op flag, obsolescent */
				continue;
			case 'J':		/* no-op flag, obsolescent */
				continue;
			case 'w':
				wlflag++;
				continue;
			case '0':
				if (fminus)
					leadz = 0;
				else
					leadz = 1;
				continue;
			case '\'':
				locdata = localeconv();
				continue;
			}
			break;
		}

		/* Scan the field width */
		if (fcode == '*') {
			width = get_arg(args, nargp, int);
			if (width < 0) {
				width = -width;
				fminus++;
			}
			format++;
		} else {
		    for (width = 0; isdigit(fcode = *format);format++)
		      width = width * 10 + fcode - '0';
		}

		/* Scan the precision */
		if (*format != '.')
			prec = -1;
		else if (*++format == '*') { /* '*' instead of digits? */
			prec = get_arg(args, nargp, int);
			format++;
		} else
			for (prec = 0; isdigit(fcode = *format); format++)
				prec = prec * 10 + fcode - '0';

		/* Scan the length modifier */
		length = 0;
		switch (*format) {
		case 'w':
			wlflag++;
			format++;
			break;
		case 'l':
			length++;
			format++;
			break;

		case 'L': /* long double == double in this implementation */
			format++; 	/* Just ignore it */
			break;
		case 'h':
			hflag++;
			format++;
			break;
		}

		/*
		 *	The character addressed by format must be
		 *	the format letter -- there is nothing
		 *	left for it to be.
		 *
		 *	The status of the +, -, #, and blank
		 *	flags are reflected in the variables
		 *	"fplus", "fminus", "fsharp", and
		 *	"fblank".  "width" and "prec" contain
		 *	numbers corresponding to the digit
		 *	strings before and after the decimal
		 *	point, respectively. If there was no
		 *	decimal point, "prec" is -1.
		 *
		 *	The IBM NLS flag B is a no-op. Since field width and
		 *	precision are only considered in bytes (or character
		 *	where no special consideration is given to multi-
		 * 	byte character as specified by ANSI) by default. This
	 	 * 	change is effective starting V3.2
		 *
		 *	The IBM NLS flag J is a no-op starting V3.2
		 *
		 *	The IBM NLS flag N is a no-op starting V3.2
		 *
		 *	The following switch sets things up
		 *	for printing.  What ultimately gets
		 *	printed will be padding blanks, a
		 *	prefix, left padding zeroes, a value,
		 *	right padding zeroes, a suffix, and
		 *	more padding blanks.  Padding blanks
		 *	will not appear simultaneously on both
		 *	the left and the right.	 Each case in
		 *	this switch will compute the value, and
		 *	leave in several variables the informa-
		 *	tion necessary to construct what is to
		 *	be printed.
		 *
		 *	The prefix is a sign, a blank, "0x",
		 *	"0X", or null, and is addressed by
		 *	"prefix".
		 *
		 *	The suffix is either null or an
		 *	exponent, and is addressed by "suffix".
		 *
		 *	The value to be printed starts at "bp"
		 *	and continues up to and not including
		 *	"p".
		 *
		 *	"lzero" and "rzero" will contain the
		 *	number of padding zeroes required on
		 *	the left and right, respectively.  If
		 *	either of these variables is negative,
		 *	it will be treated as if it were zero.
		 *
		 *	The number of padding blanks, and
		 *	whether they go on the left or the
		 *	right, will be computed on exit from
		 *	the switch.
		 */

		prefix = "";
		suffix = &expbuf[MAXESIZ];
		lzero = rzero = 0;

		switch (fcode = *format++) {

		/*
		 *	fixed point representations
		 *
		 *	"hradix" is half the radix for the
		 *	conversion. Conversion is unsigned
		 *	unless fcode is 'd'. HIBITL is 100...000
		 *	binary, and is equal to the maximum
		 *	negative number.
		 *	We assume a 2's complement machine
		 */
		case 'n':
			if (!hflag)
				*get_arg(args, nargp, long *) = count;
			else
				*get_arg(args, nargp, short *) = count;
			continue;

		case 'i':
		case 'd':
			hradix = 5;	/* 10 divided by 2 */
			val = get_arg(args, nargp, long);
			if (hflag)
				val = (short)val;
			else if (!length)
				val = (int)val;
			goto fixed;

		case 'u':
			hradix = 5;	/* 10 divided by 2 */
			val = get_arg(args, nargp, long);
			if (hflag)
				val = (unsigned short)val;
			else if (length)
				val = (unsigned long)val;
			else
				val = (unsigned)val;
			goto fixed;

		case 'o':
			hradix = 4;	/* 8 divided by 2 */
			val = get_arg(args, nargp, long);
			if (hflag)
				val = (unsigned short)val;
			else if (length)
				val = (unsigned long)val;
			else
				val = (unsigned)val;
			goto fixed;

		case 'p':
			hradix = 8;	/* 16 divided by 2 */
			val = (long)get_arg(args, nargp, void *);
			goto fixed;

		case 'X':
		case 'x':
			hradix = 8;	/* 16 divided by 2 */
			val = get_arg(args, nargp, long);
			if (hflag)
				val = (unsigned short)val;
			else if (length)
				val = (unsigned long)val;
			else
				val = (unsigned)val;

fixed:
			/* Establish default precision */
			if (prec < 0)
				prec = 1;
			else if (leadz)		/* ignore 0 flag with prec */
				leadz = 0;

			/* If signed conversion, make sign */
			if ((fcode == 'd') || (fcode == 'i')) {
				if (val < 0) {
					prefix = "-";
					/*
					 * Negate, checking in
					 * advance for possible
					 * overflow.
					 */
					if (val != HIBITL)
						val = -val;
				} else if (fplus)
					prefix = "+";
				else if (fblank)
					prefix = " ";
			}

			/* Handle the # flag for hex and octal specifiers */
			if (fsharp && bp != p && val)
				switch (fcode) {
				case 'x':
					prefix = "0x";
					break;
				case 'X':
					prefix = "0X";
					break;
				}

			/* Develop the digits of the value.  Special-case
			   0 value with 0 precision to print 0 characters. */
			p = bp = buf + MAXDIGS;
			bp[0] = '\0'; /* Null terminate the string space */

			if (val || prec) {
			    if (hradix == 5) { /* Decimal */
				do {
				    unsigned long nextval = 
					(unsigned long) val / 10UL;
				    unsigned long rem = 
					(unsigned long) val - (10UL * nextval);
				    val = nextval;
				    *--bp = tab[rem];
				} while (val != 0);
			    }
			    else if (hradix == 8) { /* Hexadecimal */
				/* Set translate table for digits */
				tab = (fcode == 'X') ? 
				    "0123456789ABCDEF" : "0123456789abcdef";
				do {
				    *--bp = tab[(unsigned long) val % 16UL];
				    val /= 16UL;
				} while (val != 0);
			    }
			    else if (hradix == 4) { /* Octal */
				do {
				    *--bp = tab[(unsigned long) val % 8UL];
				    val /= 8UL;
				} while (val != 0);
			    }
			}

			if (locdata) {
			    char	tmp[128];
			    bp = regroup(locdata, bp, &tmp[128], 128 );
			    strcpy(buf,bp);
			    p = buf + strlen(buf);
			    bp = buf;
			}

			/* Calculate padding zero requirement */
			if (leadz)
				lzero = bp - p +
				    max(prec, (width - (prefix[0] == '\0' ? 0
						: prefix[1] == '\0' ? 1
						: 2)));
			else if ((lzero = bp - p + prec) <= 0)  /* precision */
				lzero = 0;

			/* Handle the # flag for the octal specifier */
			if (fsharp && (fcode == 'o') && (lzero <= 0) )
				lzero = 1;


			break;

		case 'E':
		case 'e':
			/*
			 * E-format.  The general strategy
			 * here is fairly easy: we take
			 * what ecvt gives us and re-format it.
			 */

			/* Establish default precision */
			if (prec < 0)
				prec = 6;

			/* Fetch the value */
			dval = get_arg(args, nargp, double);

			/* Develop the mantissa */
#ifdef _THREAD_SAFE
			ecvt_r(dval, min(prec + 1, MAXECVT), &decpt, &sign,
			       cvtbuf, NDIG);
			bp = cvtbuf;
#else
			bp = ecvt(dval, min(prec + 1, MAXECVT), &decpt, &sign);
#endif	/* _THREAD_SAFE */

			/* Determine the prefix */
e_merge:
			if (sign)
				prefix = "-";
			else if (fplus)
				prefix = "+";
			else if (fblank)
				prefix = " ";

			/* Place the first digit in the buffer*/
			p = &buf[0];

			if (!FINITE(dval)) {
				/* INF, NAN() */
				while (*p = *bp++) {
					p++;
				}
				bp = &buf[0];
				break; 		/* Escape switch */
			}

			k = 0;
			*p++ = (*bp != '\0') ? (k++, *bp++) : (k++, '0');

			/* Put in a decimal point if needed */
			if (prec != 0 || fsharp) {
				radchr = __lc_locale->nl_info[RADIXCHAR];
				if (radchr[0] != '\0')
				    do
					{*p++ = *radchr++;}
				    while (*radchr);
				else
					*p++ = '.';
			}

			/* Create the rest of the mantissa */
			for (rzero = prec; rzero > 0 && *bp != '\0'; --rzero) {
				*p++ = *bp++;
				k++;
			}
			bp = &buf[0];

			/* Create the exponent */
			if (dval != 0) {
				n = decpt - 1;
				if (n < 0)
					n = -n;
				for ( ; n != 0; n /= 10) {
					*--suffix = (n % 10) + '0';
					k++;
				}
			}

			/* Prepend leading zeroes to the exponent */
			while (suffix > &expbuf[MAXESIZ - 2]) {
				*--suffix = '0';
				k++;
			}

			/* Put in the exponent sign */
			*--suffix = (decpt > 0 || dval == 0) ? (k++, '+')
							     : (k++, '-');

			/* Put in the e */
			*--suffix = isupper(fcode) ? (k++, 'E')  : (k++, 'e');

			if (leadz == 1)
				if (fsharp || prec > 0)
					lzero = width - (k + 1);
				else
					lzero = width - k;

			/* embed prefix within the field width */
			lzero -= (prefix[0] == '\0' ? 0 : 1);

			break;

		case 'f':
			/*
			 * F-format floating point.  This is a
			 * good deal less simple than E-format.
			 * The overall strategy will be to call
			 * fcvt, reformat its result into buf,
			 * and calculate how many trailing
			 * zeroes will be required.  There will
			 * never be any leading zeroes needed.
			 *
			 * 8/24/88 apar 1890:
			 * leading zeros are a valid option for
			 * floating point output.
			 */

			/* Establish default precision */
			if (prec < 0)
				prec = 6;

			/* Fetch the value */
			dval = get_arg(args, nargp, double);

			/* Do the conversion */
#ifdef _THREAD_SAFE
			fcvt_r(dval, min(prec, MAXFCVT), &decpt, &sign,
			       cvtbuf, NDIG);
			bp = cvtbuf;
#else
			bp = fcvt(dval, min(prec, MAXFCVT), &decpt, &sign);
#endif	/* _THREAD_SAFE */

			/* Determine the prefix */
f_merge:
			if (sign)
				prefix = "-";
			else if (fplus)
				prefix = "+";
			else if (fblank)
				prefix = " ";

			/* Initialize buffer pointer */
			p = &buf[0];

			if (!FINITE(dval)) {
				/* INF, NAN() */
				if (sign)
					prefix = "-";
				else if (fplus)
					prefix = "+";
				else if (fblank)
					prefix = " ";
				while (*p = *bp++) {
					p++;
				}
				bp = &buf[0];
				break;
			}

			/* Emit the digits before the decimal point */
			n = decpt;
			k = 0;
			do {
				*p++ = (n <= 0 || *bp == '\0'
					|| k >= MAXFSIG) ? (k++, '0')
							 : (k++, *bp++);
			} while (--n > 0);


			/* Re-group if necessary */
			if (locdata) {
			    char	tmp[128];
			    *p = '\0';
			    p = regroup(locdata, buf, &tmp[128], 128);
			    strcpy(buf,p);
			    p = buf + strlen(buf);
			}

			/* Decide whether we need a decimal point */
			if (fsharp || prec > 0) {
				radchr = __lc_locale->nl_info[RADIXCHAR];
				if (radchr[0] != '\0')
				    do
					{*p++ = *radchr++;}
				    while (*radchr);
				else
					*p++ = '.';
			}

			/* Digits (if any) after the decimal point */
			n = min(prec, MAXFCVT);
			rzero = prec - n;
			while (--n >= 0)
				*p++ = (++decpt <= 0 || *bp == '\0'
					|| k > MAXFSIG) ? (k++, '0')
							: (k++, *bp++);

			/* determine if leading zeros are requested and if
			   so, how many. reference apar 1890 */
			if ( leadz == 1 )
				if (fsharp || prec > 0)
					lzero = width - (k + 1);
				else
					lzero = width - k;

			/* embed prefix within the field width */
			lzero -= (prefix[0] == '\0' ? 0 : 1);

			bp = &buf[0];

			break;

		case 'G':
		case 'g':
			/*
			 * g-format.  We play around a bit
			 * and then jump into e or f, as needed.
			 */

			/* Establish default precision */
			if (prec < 0)
				prec = 6;

			/* If prec is 0, 1 is assumed - ANSI */
			if (prec == 0)
				prec = 1;

			/* Fetch the value */
			dval = get_arg(args, nargp, double);

			/* Do the conversion */
#ifdef _THREAD_SAFE
			ecvt_r(dval, min(prec, MAXECVT), &decpt, &sign,
			       cvtbuf, NDIG);
			bp = cvtbuf;
#else
			bp = ecvt(dval, min(prec, MAXECVT), &decpt, &sign);
#endif	/* _THREAD_SAFE */

			if ( !FINITE(dval) ) {
				/* INF, NAN() */
				goto e_merge;
			}

			if (dval == 0)
				decpt = 1;

			k = prec;
			if (!fsharp) {
				n = strlen(bp);
				if (n < k)
					k = n;
				while (k >= 1 && bp[k-1] == '0')
					--k;
			}

			if (decpt < -3 || decpt > prec) {
				prec = k - 1;
				goto e_merge;
			}
			prec = k - decpt;
			goto f_merge;

		/* case '%':	*/
		default:
			buf[0] = fcode;
			goto c_merge;

		case 'c':
			/* note: if wlflag, treate c as C */
			/* by falling through to next case */
			if (!wlflag) {
				buf[0] = get_arg(args, nargp, int);
c_merge:
				p = (bp = &buf[0]) + 1;
				break;
			}

		/* Specification to print out a
		 * single wchar_t value.
		 */
		case 'C':
			wc = get_arg(args, nargp, int);
			buf[0] = fcode;
			bp = buf;
			p = bp + WCTOMB(buf, wc);
			break;

		case 's':
			/* note: if wlflag, treate s as S */
			/* by falling through to next case */
			if (!wlflag) {
				p = bp = get_arg(args, nargp, char *);
				if (!p) 	/* tolerate NULL pointers */
					p = bp = "(null)";
nullstring:
				n = strlen(bp);
				if (prec >= 0 && prec < n)
					n = prec;
				p += n;
				break;
			}

		case 'S':
			wcp = get_arg(args, nargp, wchar_t *);
			if (!wcp) {
			    p = bp = "(null)";
			    goto nullstring;
			}

			i = wcslen(wcp);
			j = mb_cur_max * (i + 1);
			if (!(p = bp = mbbuf = (char *)malloc(j))) {
			    RFREE(nformat);
			    return (EOF);
			}

			if (prec >= 0) {
			    n = wcstombs(mbbuf, wcp, prec);
			    if (n > prec) n = prec;
			} else
			    n = wcstombs(mbbuf, wcp, j);

			if (n >= 0) {
			    mbbuf[n] = '\0';
			    p += n;
			} else {
			    free(mbbuf);
			    mbbuf = NULL;
			    RFREE(nformat);
			    return (EOF);
			}
			break;

		case '\0': /* unexpected end of format; return error */
			RFREE(nformat);
			return (EOF);

		}

		/* Calculate number of padding blanks */
		if (lzero < 0)
			lzero = 0;
		if (rzero < 0)
			rzero = 0;
		/* nb = number of bytes in buffer. */
		nb = p - bp;
		k = nb + lzero + rzero + (prefix[0] == '\0' ? 0
					: prefix[1] == '\0' ? 1
					: 2) + (&expbuf[MAXESIZ] - suffix);
		count += (width > k) ? width : k;

		/* Blanks on left if required */
		if (!fminus)
			while (--width >= k)
				F_PUTC(' ', iop)

		/* Prefix, if any */
		while (*prefix != '\0')
			F_PUTC(*prefix++, iop);

		/* Zeroes on the left */
		while (--lzero >= 0)
			F_PUTC('0', iop);

		/* The value itself */
		if ((fcode == 'S' || (wlflag && fcode == 's')) && mbbuf) {
			F_PUT(mbbuf, n);
			free (mbbuf);
			mbbuf = NULL;
		} else if (nb > 0)
			F_PUT(bp, nb);

		/* Zeroes on the right */
		while (--rzero >= 0)
			F_PUTC('0', iop);

		/* The suffix */
		while (*suffix != '\0')
			F_PUTC(*suffix++, iop);

		/* Blanks on the right if required */
		while (--width >= k)
			F_PUTC(' ', iop);
	}
}


/*	_argnum: common code for reorder, width.
 *		Originally design for PTM p36289
 *		to support XPG3 standards.
 */
int
_argnum(register char **prt, int *arg_index, arglist *alist, int *max_arg)
{
	int	i;
	char	fcode;

	for (i = 0; isdigit(fcode = *((*prt)++)); )
		i = i * 10 + fcode - '0';
	if (i > NL_ARGMAX || i == 0)
		return (-1);
	if (i > *max_arg)
		*max_arg = i;
	*arg_index = i - 1;	/* the arrays are zero based */
	return (0);
}


/*	width: Used for reorder to handle width and precision.
 *		Originally design for PTM p36289 to support
 *		XPG3 standards.
 */
static int
width(register char **prt, register char **new_prt,
      arglist *vlist, arglist *alist, int *arg_count, int *max_arg)
{
	char	*sav_prt;
	int	index, rc = 0;
	arglist *ap;

	if (**prt == '*' && isdigit( *(*prt + 1))) {
		*(++(*new_prt)) = *((*prt)++);
		sav_prt = *prt;
		while (isdigit(*((*prt)++)))
			;
		(*prt)--;
		if (**prt != '$')
			return (-1);
		*prt = sav_prt;
		if (_argnum(prt, &index, alist, max_arg) != 0)
			return (-1);
		if (*arg_count >= NL_ARGMAX)
			return (-1);
		ap = &alist[index];
		vlist[*arg_count].da_arg.au_arg = ap;
		ap->da_type = DA_T_INT;
		(*arg_count)++;
	} else {
		if (**prt == '*')
			rc = -1;
		while (isdigit( **prt))
			*(++(*new_prt)) = *((*prt)++);
	}
	return (rc);
}


/*
 *	Reorder goes through a format statement containing variably
 *	ordered arguments and reorders the arguments accordingly,
 *	stripping out the x$'s.	 Upon success, reorder() returns 0;
 *	the new format string is returned via the argument "bp_new_fmt",
 *	and the new arg pointer is returned via the argument "vlist".
 *	Upon failure, reorder() returns -1;
 */
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
	int	i;

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
				if (*(format + 1) == '%') {
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
		ap = &alist[arg_index];

		for ( ; ; ) { /* Scan the <flags> */
			switch (fcode = *++new_fmt = *(format++)) {
			case '+':
			case '-':
			case ' ':
			case '#':
			case 'B':
			case 'N':
			case 'J':
			case '\'':
				continue;
			}
			*new_fmt-- = '\0';
			format--;
			break;
		}

		/* Scan the field width */
		if (width(&format, &new_fmt, vlist, alist,
			  &arg_count, &max_arg) != 0)
			goto badret;

		/* Scan the precision */
		if (*format == '.') {
			*++new_fmt = *format++;
			if (width(&format, &new_fmt, vlist, alist,
				  &arg_count, &max_arg) != 0)
				goto badret;
		}

		/* Now store pointer to this arg into arg-count-based
		 * list.  Must defer this till after we have scanned
		 * width and precision above!
		 */
		if (arg_count >= NL_ARGMAX)
			goto badret;
		vp = &vlist[arg_count];
		vp->da_arg.au_arg = ap;
		arg_count++;

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
			if (length == 1)
				ap->da_type = DA_T_LONG;
			else
				ap->da_type = DA_T_INT;
			break;

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

		case 'E':
		case 'e':
		case 'f':
		case 'G':
		case 'g':
			ap->da_type = DA_T_DOUBLE;
			break;

		case 'c':
		case 'C':
			ap->da_type = DA_T_INT;
			break;

		case 'p':
		case 's':
		case 'S':
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
		case DA_T_INT:
			ap->da_arg.au_int = va_arg(*args, int);
			break;
		case DA_T_LONG:
			ap->da_arg.au_long = va_arg(*args, long);
			break;
		case DA_T_DOUBLE:
			ap->da_arg.au_double = va_arg(*args, double);
			break;
		case DA_T_POINTER:
			ap->da_arg.au_pointer = va_arg(*args, void *);
			break;
		case DA_T_SHORTP:
			ap->da_arg.au_shortp = va_arg(*args, short *);
			break;
		case DA_T_INTP	:
			ap->da_arg.au_intp = va_arg(*args, int *);
			break;
		case DA_T_LONGP	:
			ap->da_arg.au_longp = va_arg(*args, long *);
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
