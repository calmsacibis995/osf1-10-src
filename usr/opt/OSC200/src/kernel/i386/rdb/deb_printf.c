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
static char	*sccsid = "@(#)$RCSfile: deb_printf.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:12:43 $";
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
/*
 * 5799-CGZ (C) COPYRIGHT IBM CORPORATION 1986,1990
 * LICENSED MATERIALS - PROPERTY OF IBM
 * REFER TO COPYRIGHT INSTRUCTIONS FORM NUMBER G120-2083
 */

#ifndef lint
#endif

/*
 * following conditionals generate separate versions of printf
 * STANDALONE	generates code for standalone environment, it creates
 *		a printf and an sprintf.
 * BFORMAT	generates code for the 'b' format item
 * DEBUG	generates an xprintf for use in testing printf
 * NOFLOAT	suppresses code for floating point formats
 */

#if defined(MACH) && !defined(STANDALONE)
#define printf db_printf		       /* we will generate a real printf in standalone */
#define sprintf db_sprintf
#define putchar db_putchar
#define STANDALONE 1		/* a necessary lie */
#endif

#ifndef STANDALONE

#include <stdio.h>
#define PUTCHAR(c) putc(c,file)

#else	/* STANDALONE */
#define NOFLOAT 1
#define stdout 0
#define FILE char
#define PUTCHAR(c)	if (file == stdout) putchar(c); else *file++ = c
#define xprintf printf		       /* we will generate a real printf in standalone */
#endif /* STANDALONE */

/* varargs are not properly used, but do provide some portability */
#include <varargs.h>

/*#include <ctype.h>
*/

#define MAXBUFF 64		       /* maximum number of digits allowed */
#define EXPWIDTH 4		       /* number of digits in e format exponent */

#if defined(DEBUG) || defined(STANDALONE)
 /*
  * in DEBUG mode, arrange for this routine to be called as "xprintf" so
  * that it may be compared with the standard unix printf routine.
  */
xprintf(format, va_alist)
	char *format;
	va_dcl
    {
	va_list list;
	va_start(list);
	return (doprnt(format, list, stdout));
	va_end(list)
}


#else				       /* not debugging - define real _doprnt */
#define doprnt _doprnt
#endif

#if defined(STANDALONE) || defined(SPRINTF)
 /*
  * in STANDALONE mode generate a sprintf here in case it is needed
  */
sprintf(string, format, va_alist)
	char *string, *format;
	va_dcl
    {
	va_list list;
	va_start(list);
	return (doprnt(format, list, string));
	va_end(list)
}


#endif

/*
 * following eliminates extra code for known 32 bit machines
 * where sizeof (long) == sizeof (int).
 */

#if defined(vax) || defined(sun) || defined(ibm370) || defined(ibm032)
#define GETINT(argp) va_arg(argp,int)
#else				       /* assume long != int */
#define GETINT(argp) lflag ? va_arg(argp,long) : va_arg(argp,int)
#endif

char *_atoi();			       /* declare it before use */

doprnt(fmtp, argp, file)
	register char *fmtp;
	register va_list argp;
	register FILE * file;
{
	register int c;
	long num;		       /* for numeric conversions */
	int ok,			       /* for conversion testing */
	width,			       /* field width */
	decimals,		       /* number of decimals */
	dec_found,		       /* have found a decimal number */
	left_just,		       /* if left justifying */
	padchar,		       /* padding character ' ' or '0' */
	lflag,			       /* if %l specified */
	flag,			       /* if %# specified */
	sign,			       /* if %+ specified */
	space,			       /* if %space specified */
	len;
	register char *str;
	char buffer[MAXBUFF + 2];      /* don't trust putfloat */
	double f;
	static char DIGITS[] = "0123456789ABCDEF";
	static char digits[] = "0123456789abcdef";
	char prefixbuff[4];
	register char *prefix;
	char *s;		       /* used for b formats */

	while (c = *fmtp++) {
		if (c != '%') {
			PUTCHAR(c);
			continue;
		}
/*
 * found the start of a format item
 */
		left_just = 0;
		flag = 0;
		padchar = ' ';
		sign = 0;
		space = 0;
		for (;; ++fmtp) {
/*
 * look for a flag character first
 */
			switch (*fmtp) {
			case '-':
				left_just++;
				continue;
			case '#':
				flag++;
				continue;
			case '0':
				padchar = '0';
				continue;
			case '+':
				sign = '+';
				continue;
			case ' ':
				space++;
				continue;
			}
			break;
		}
/*
 * look for field width
 * take * as meaning "take value from argument list"
 */
		if (*fmtp == '*') {
			width = va_arg(argp, int);
			++fmtp;
		} else
			fmtp = _atoi(fmtp, &width, &ok);
/*
 * if width is followed by "." look for "decimal" specification.
 * also take * here.
 */
		decimals = 0;
		dec_found = 0;
		if ((c = *fmtp++) == '.') {
			if (*fmtp == '*') {
				decimals = va_arg(argp, int);
				++dec_found;
				++fmtp;
			} else
				fmtp = _atoi(fmtp, &decimals, &dec_found);
			c = *fmtp++;
		}
/*
 * look for "l" (long number specification)
 * this is required for outputing longs on machines where
 * long != int
 */
		if (c == 'l') {
			c = *fmtp++;
			lflag = 1;
		} else
			lflag = 0;
		str = buffer;
		prefix = prefixbuff;
		if (!sign && space)
			sign = ' ';  /* get sign set up */
/*
 * now look for a format type code
 */
		switch (c) {
		case 'd':
			num = GETINT(argp);
			len = 0;
			if (num < 0) {
				num = -num;
				sign = '-';
			}
			if (sign)
				*prefix++ = sign;
			len = _itoa(num, str, 10, digits);
			break;
		case 'o':
			num = GETINT(argp);
			if (flag && num)
				*prefix++ = '0';
			len = _itoa(num, str, 010, digits);
			break;
		case 'x':
		case 'X':
			num = GETINT(argp);
			if (flag && num) {
				*prefix++ = '0';
				*prefix++ = c;
			}
			len = _itoa(num, str, 0x10,
			    c == 'X' ? DIGITS : digits);
			break;
		case 'u':
			num = GETINT(argp);
			len = _itoa(num, str, 10, digits);
			break;
		case 'c':
			*str = va_arg(argp, int);
			len = 1;
			break;
		case 's':
			str = va_arg(argp, char *);
			if (str == (char *)0)
				str = "(null)";
			len = strlen(str);
			if (dec_found && len > decimals)
				len = decimals;
			break;
#ifndef NOFLOAT
		case 'f':
		case 'e':
		case 'E':
			f = va_arg(argp, double);
			if (f < 0.0) {
				f = -f;
				sign = '-';
			}
			if (sign)
				*prefix++ = sign;
			if (!dec_found)
				decimals = 6;

			if (c == 'f') {
				len = _putfloat(f, decimals, 0, MAXBUFF, str);
				if (decimals == 0)
					--len; /* %.0f means no decimal place */
			} else
				len = _putscientific(f, decimals, 0, MAXBUFF, str, c);
			break;
		case 'g':
		case 'G':
			{
				int n;
				char *epos;

				f = va_arg(argp, double);
				if (f < 0.0) {
					f = -f;
					sign = '-';
				}
				if (sign)
					*prefix++ = sign;
				if (!dec_found)
					decimals = 6;
/*
 * convert using 'e' format and then pick up exponent
 * if the exponent is between decimals and -3 then
 * convert according to the appropriate 'f' format.
 */
				c = (c == 'G') ? 'E' : 'e';
				len = _putscientific(f, decimals - 1, 0, MAXBUFF, str, c);
				epos = str + len - EXPWIDTH;
				if (*epos == c) {
					n = atoi(epos + 1) + 1;
					if (n <= decimals && n >= -3) {
						len = _putfloat(f, decimals - n, 0, MAXBUFF, str);
						if (!flag)
							len -= gtrim(str, len);
					} else if (!flag) { /* use e format */
						int k = gtrim(str, len - EXPWIDTH);
						if (k) {
							for (n = 0; n < EXPWIDTH; ++n)
								epos[n - k] = epos[n];
							len -= k;
						}
					}
				}
			}
			break;
#endif				       /* NOFLOAT */
		case '%':
			str = "%";
			len = 1;
			break;
#if defined(STANDALONE) || defined(BFORMAT)
#define NBITS (8 * sizeof (int))
			/*
			 * b format item is for standalone use only - it is included here
			 * for when this code is used in kernel or standalone use.
			 */
		case 'b':
		case 'B':
			num = va_arg(argp, int);
			s = va_arg(argp, char *);
			if (flag && num)
				*prefix++ = '0';
			len = _itoa(num, str, *s++,
			    c == 'B' ? DIGITS : digits);
			break;
#endif				       /* STANDALONE */
		default:
			PUTCHAR(c);
			continue;
		}
/*
 * now ready to output the given string.
 * "str" points to the string of length "len".
 * output the appropriate number of pad characters
 * before the prefix and the string.
 * we adjust the width to take into account the prefix length.
 */
		width -= prefix - prefixbuff;
		*prefix = 0;
		if (!left_just && (padchar == ' ' || !prefixbuff[0]))
			while (width > len) {
				PUTCHAR(padchar);
				width--;
			}
/* output the prefix ( "", "0", "+", "-" or "0x" */
		for (prefix = prefixbuff; *prefix;)
			PUTCHAR(*prefix++);
/* if we have a prefix and a 0 pad then output pad after the prefix */
		if (!left_just && !(padchar == ' ' || !prefixbuff[0]))
			while (width > len) {
				PUTCHAR(padchar);
				width--;
			}
/* now output the generated string pointed to by str of length len */
		while (len > 0) {
			PUTCHAR(*str++);
			width--;
			len--;
		}
#if defined(STANDALONE) || defined(BFORMAT)
		if (c == 'b' || c == 'B')
			bformat(num, file, s);
#endif
		while (width > 0) {
			PUTCHAR(' ');  /* pad on right with blanks */
			width--;
		}
	}
#ifdef STANDALONE
	if (file != 0)
		*file = 0;
#endif
}


#ifndef NOFLOAT
 /*
  * routine to remove the trailing 0's and . from a G or E format
  * item.
  * returns the number of bytes REMOVED.
  */
static gtrim(str, len)
	register char *str;
	register int len;
{
	register int l = len;

	while (str[l - 1] == '0')
		--l;
	if (str[l - 1] == '.')
		--l;

	return (len - l);
}


#endif				       /* NOFLOAT */


/*
 * internal conversion routine to convert a number (unsigned long)
 * to character (ASCII) format.
 * for simplicity this version is recursive.
 * it could be make non-recursive by providing an internal buffer
 * and doing the conversion into that, or by changing the calling
 * sequence.
 */
int _itoa(num, str, base, digits)
	long num;		       /* number to convert */
	int base;		       /* base system 8, 10, 16 */
	char *str,		       /* where to put the result */
*digits;			       /* what to use for digits */
{
	register int digit, len;

	digit = ((unsigned long)num) % base; /* unsigned division!!! */
	num = ((unsigned long)num) / base; /* ditto */
	if (num)
		len = _itoa(num, str, base, digits);
	else
		len = 0;
	str[len] = digits[digit];
	return (len + 1);
}


/*
 * internal conversion routine to convert format width and number
 * of decimals.
 * input:
 *	fmtp	pointer to the number
 *	number	pointer to the resulting number
 *	ok	pointer to a flag to set if it was found
 * output: updated fmtp
 */
static char *_atoi(fmtp, number, ok)
	register char *fmtp;	       /* the string pointer address */
	int *number;		       /* the resulting value */
	int *ok;		       /* 1=found number, 0=didn't find number */
{
	register int c = *fmtp;
	register int n = 0;

	*ok = 0;
	while ('0' <= c && c <= '9') {
		*ok = 1;
		n = n * 10 + (c - '0');
		c = *(++fmtp);
	}
	*number = n;
	return (fmtp);
}


#if defined(STANDALONE) || defined(BFORMAT)
bformat(num, file, s)
	long num;
	register FILE * file;
	register char *s;
{
	register int i, c;
	register int any = 0;

	if (num) {
		PUTCHAR('<');
		while (i = *s++) {
			if (num & (1 << (i - 1))) {
				if (any)
					PUTCHAR(',');
				any = 1;
				for (; (c = *s) > ' '; s++)
					PUTCHAR(c);
			} else for (; *s > ' '; s++);
		}
		PUTCHAR('>');
	}
}


#endif
