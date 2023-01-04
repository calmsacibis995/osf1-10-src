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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: doprnt.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/06/10 16:32:38 $";
#endif
/*
 * HISTORY
 */
/*
 * OSF/1 1.1
 */
/*
 * COMPONENT_NAME: CMDCSH  c shell(csh)
 *
 * FUNCTIONS: csh_doprnt
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *	1.6  com/cmd/csh/doprnt.c, bos320 5/10/91 15:37:37";
 */

/*
 *	csh_doprnt: common code for csh_printf, fprintf, sprintf
 */


#include <ctype.h>
#include <stdarg.h>
#include <sys/types.h>

#	define	BITSPERBYTE	8
#	define HIBITL	(1L << (BITSPERBYTE * sizeof(long) - 1))
#	define MAXLONG	(~HIBITL)
#	define EOF	(-1)


/* Maximum number of digits in any integer representation */
#if defined(__alpha)
#  define MAXDIGS 24
#else /* !defined(alpha) */
#  define MAXDIGS 11
#endif /* defined(alpha) */

/* Maximum total number of digits in E format */
#define MAXECVT 17

/* Maximum number of digits after decimal point in F format */
#define MAXFCVT 60

/* Maximum significant figures in a floating-point number */
#define MAXFSIG 17

/* Maximum number of characters in an exponent */
#define MAXESIZ 5

/* Maximum (positive) exponent */
#define MAXEXP 310


/* Convert a digit character to the corresponding number */
#define tonumber(x) ((x)-'0')

/* Convert a number between 0 and 9 to the corresponding digit */
#define todigit(x) ((x)+'0')

/* Max and Min macros */
#define max(a,b) ((a) > (b)? (a): (b))
#define min(a,b) ((a) < (b)? (a): (b))

#define	putc(character, output)	display_char(character)
#define	ferror(stream)	(0)

#define PUT(p, n)	if (n == 1) \
				(void) putc((unsigned char *)*p, iop); \
			else { \
				int i = n;\
				unsigned char *q=(unsigned char *)p;\
				while(i--)\
					putc(*q++, iop);\
			}

/* 
 * C-Library routines for floating conversion  From DEC OSF/1.2
 */

extern uchar_t *fcvt(), *ecvt();
extern int strlen();


static int
csh_doprnt(register uchar_t *format, va_list args, register int iop)
{
	/* This variable counts output characters. */
	int	count = 0;

	/* Starting and ending points for value to be printed */
	register uchar_t	*bp, *p;

	/* Field width and precision */
	int	width, prec;

	/* Format code */
	register int	fcode;

	/* Number of padding zeroes required on the left and right */
	int	lzero, rzero;

	/* Flags - nonzero if corresponding character is in format */
	int	length;		/* l */
	int	fplus;		/* + */
	int	fminus;		/* - */
	int	fblank;		/* blank */
	int	fsharp;		/* # */

	/* Values are developed in this buffer */
	uchar_t	buf[max(MAXDIGS, 1+max(MAXFCVT+MAXEXP, MAXECVT))];

	/* Pointer to sign, "0x", "0X", or empty */
	uchar_t	*prefix;

	/* Exponent or empty */
	uchar_t	*suffix;

	/* Buffer to create exponent */
	uchar_t	expbuf[MAXESIZ + 1];

	/* The value being converted, if integer */
	long	val;

	/* The value being converted, if real */
	double	dval;

	/* Output values from fcvt and ecvt */
	int	decpt, sign;

	/* Pointer to a translate table for digits of whatever radix */
	uchar_t	*tab;

	/* Work variables */
	int	k, n, hradix, lowbit;

	expbuf[MAXESIZ] = '\0';

	/*
	 *	The main loop -- this loop goes through one iteration
	 *	for each string of ordinary characters or format specification.
	 */
	for ( ; ; ) {
		for (bp = format;
		    (fcode = *format) != '\0' && fcode != '%'; format++)
			;
		if (n = format - bp) { /* ordinary non-% characters */
			count += n;
			PUT(bp, n);
		}
		if (fcode == '\0') /* end of format; normal return */
			return (ferror(iop) ? EOF : count);
		/*
		 *	% has been found.
		 *	First, parse the format specification.
		 */
		fplus = fminus = fblank = fsharp = lzero = 0;
		for ( ; ; ) { /* Scan the <flags> */
			switch (fcode = *++format) {
			case '+':
				fplus++;
				continue;
			case '-':
				fminus++;
				continue;
			case ' ':
				fblank++;
				continue;
			case '#':
				fsharp++;
				continue;
			}
			break;
		}

		/* Scan the field width */
		if (fcode == '*') {
			width = va_arg(args, int);
			if (width < 0) {
				width = -width;
				fminus++;
			}
			format++;
		} else {
			if (fcode == '0') /* obsolescent spec. */
				lzero++; /* pad with leading zeros */
			for (width = 0; isdigit(fcode = *format); format++)
				width = width * 10 + tonumber(fcode);
		}

		/* Scan the precision */
		if (*format != '.')
			prec = lzero ? width : -1;
		else if (*++format == '*') { /* '*' instead of digits? */
			prec = va_arg(args, int);
			format++;
		} else
			for (prec = 0; isdigit(fcode = *format); format++)
				prec = prec * 10 + tonumber(fcode);

		/* Scan the length modifier */
		length = 0;
		switch (*format) {
		case 'l':
			length++;
			/* No break */
		case 'h':
			format++;
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
		 *	The following switch sets things up
		 *	for printing.  What ultimately gets
		 *	printed will be padding blanks, a
		 *	prefix, left padding zeroes, a value,
		 *	right padding zeroes, a suffix, and
		 *	more padding blanks.  Padding blanks
		 *	will not appear simultaneously on both
		 *	the left and the right.  Each case in
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
		
		prefix = (uchar_t *)"";
		suffix = &expbuf[MAXESIZ];
		lzero = rzero = 0;

		switch (fcode = *format++) {

		/*
		 *	fixed point representations
		 *
		 *	"hradix" is half the radix for the
		 *	conversion. Conversion is unsigned
		 *	unless fcode is 'd'. HIBITL is 100...000
		 *	binary, and is equal to	the maximum
		 *	negative number.
		 *	We assume a 2's complement machine
		 */

		case 'd':
		case 'u':
			hradix = 10/2;
			goto fixed;

		case 'o':
			hradix = 8/2;
			goto fixed;

		case 'X':
		case 'x':
			hradix = 16/2;

		fixed:
			/* Establish default precision */
			if (prec < 0)
				prec = 1;

			/* Fetch the argument to be printed */
			if(length)
				val = va_arg(args, long);
			else if(fcode == 'd')
				val = va_arg(args, int);
			else
				val = va_arg(args, unsigned);

			/* If signed conversion, make sign */
			if(fcode == 'd') {
				if(val < 0) {
					prefix = (uchar_t *)"-";
					/*
					 * Negate, checking in
					 * advance for possible
					 * overflow.
					 */
					if(val != HIBITL)
						val = -val;
				} else if (fplus)
					prefix = (uchar_t *)"+";
				else if (fblank)
					prefix = (uchar_t *)" ";
			}

			/* Set translate table for digits */
			tab = (fcode == 'X') ?
			    (uchar_t *)"0123456789ABCDEF" : (uchar_t *)"0123456789abcdef";

			/* Develop the digits of the value */
			p = bp = buf + MAXDIGS;
			while (val != 0) {
				lowbit = val & 1;
				val = (val >> 1) & ~HIBITL;
				*--bp = tab[val % hradix * 2 + lowbit];
				val /= hradix;
			}

			/* Calculate padding zero requirement */
			lzero = bp - p + prec;

			/* Handle the # flag */
			if (fsharp && bp != p)
				switch (fcode) {
				case 'o':
					if (lzero < 1)
						lzero = 1;
					break;
				case 'x':
					prefix = (uchar_t *)"0x";
					break;
				case 'X':
					prefix = (uchar_t *)"0X";
					break;
				}

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
			dval = va_arg(args, double);

			/* Develop the mantissa */
			bp = (uchar_t *)ecvt(dval, min(prec + 1, MAXECVT), &decpt, &sign);

			/* Determine the prefix */
		e_merge:
			if (sign)
				prefix = (uchar_t *)"-";
			else if (fplus)
				prefix = (uchar_t *)"+";
			else if (fblank)
				prefix = (uchar_t *)" ";

			/* Place the first digit in the buffer*/
			p = &buf[0];
			*p++ = (*bp != '\0') ? *bp++ : '0';

			/* Put in a decimal point if needed */
			if (prec != 0 || fsharp)
				*p++ = '.';

			/* Create the rest of the mantissa */
			for (rzero = prec; rzero > 0 && *bp != '\0'; --rzero)
				*p++ = *bp++;

			bp = &buf[0];

			/* Create the exponent */
			if (dval != 0) {
				n = decpt - 1;
				if (n < 0)
				    n = -n;
				for ( ; n != 0; n /= 10)
					*--suffix = todigit(n % 10);
			}

			/* Prepend leading zeroes to the exponent */
			while (suffix > &expbuf[MAXESIZ - 2])
				*--suffix = '0';

			/* Put in the exponent sign */
			*--suffix = (decpt > 0 || dval == 0) ? '+' : '-';

			/* Put in the e */
			*--suffix = isupper(fcode) ? 'E'  : 'e';

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
			 */

			/* Establish default precision */
			if (prec < 0)
				prec = 6;

			/* Fetch the value */
			dval = va_arg(args, double);

			/* Do the conversion */
			bp = (uchar_t *)fcvt(dval, min(prec, MAXFCVT), &decpt, &sign);

			/* Determine the prefix */
		f_merge:
			if (sign && decpt > -prec && *bp != '0')
				prefix = (uchar_t *)"-";
			else if (fplus)
				prefix = (uchar_t *)"+";
			else if (fblank)
				prefix = (uchar_t *)" ";

			/* Initialize buffer pointer */
			p = &buf[0];

			/* Emit the digits before the decimal point */
			n = decpt;
			k = 0;
			do {
				*p++ = (n <= 0 || *bp == '\0' || k >= MAXFSIG) ?
				    '0' : (k++, *bp++);
			} while (--n > 0);

			/* Decide whether we need a decimal point */
			if (fsharp || prec > 0)
				*p++ = '.';

			/* Digits (if any) after the decimal point */
			n = min(prec, MAXFCVT);
			rzero = prec - n;
			while (--n >= 0)
				*p++ = (++decpt <= 0 || *bp == '\0' ||
				    k >= MAXFSIG) ? '0' : (k++, *bp++);

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

			/* Fetch the value */
			dval = va_arg(args, double);

			/* Do the conversion */
			bp = (uchar_t *)ecvt(dval, min(prec, MAXECVT), &decpt, &sign);
			if (dval == 0)
				decpt = 1;

			k = prec;
			if (!fsharp) {
				n = strlen((char *)bp);
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

	/*	case '%':	*/
		default:
			buf[0] = fcode;
			goto c_merge;

		case 'c':
			buf[0] = va_arg(args, int);
		c_merge:
			p = (bp = &buf[0]) + 1;
			break;

		case 's':
			p = bp = va_arg(args, uchar_t *);
			if (prec < 0)
				p += strlen((char *)bp);
			else { /* a strnlen function would  be useful here! */
				while (*p++ != '\0' && --prec >= 0)
					;
				--p;
			}
			break;

		case '\0': /* unexpected end of format; return error */
			return (EOF);

		}

		/* Calculate number of padding blanks */
		if (lzero < 0)
			lzero = 0;
		if (rzero < 0)
			rzero = 0;
		k = (n = p - bp) + lzero + rzero +
		    (prefix[0] == '\0' ? 0 : prefix[1] == '\0' ? 1 : 2) +
		    (&expbuf[MAXESIZ] - suffix);
		count += (width > k) ? width : k;

		/* Blanks on left if required */
		if (!fminus)
			while (--width >= k)
				(void) putc(' ', iop);

		/* Prefix, if any */
		while (*prefix != '\0')
			(void) putc(*prefix++, iop);

		/* Zeroes on the left */
		while (--lzero >= 0)
			(void) putc('0', iop);
		
		/* The value itself */
		if (n > 0)
			PUT(bp, n);

		/* Zeroes on the right */
		while (--rzero >= 0)
			(void) putc('0', iop);

		/* The suffix */
		while (*suffix != '\0')
			(void) putc(*suffix++, iop);

		/* Blanks on the right if required */
		while (--width >= k)
			(void) putc(' ', iop);
	}
}

int
csh_printf(char *format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	/* NB last argument is not used */
	count = csh_doprnt((uchar_t *)format, ap, 1);
	va_end(ap);
}
