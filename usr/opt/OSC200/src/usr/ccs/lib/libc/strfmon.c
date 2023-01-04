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
static char *rcsid = "@(#)$RCSfile: strfmon.c,v $ $Revision: 1.1.6.5 $ (DEC) $Date: 1993/10/18 20:55:32 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */

/*
 * COMPONENT_NAME: LIBCFMT
 *
 * FUNCTIONS:  strfmon
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.3  com/lib/c/fmt/strfmon.c, libcfmt, 9130320 7/17/91 15:21:47
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak strfmon = __strfmon
#endif
#include <sys/localedef.h>
#include <monetary.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <values.h>
#include <fp.h>
#include <math.h>
#include <errno.h>

#include "print.h"


#define PUT(c)	(strp < strend ? *strp++ = (c) : toolong++)
#define SIZE	1000	/* Size of a working buffer */

typedef	signed char schar;


/*
 * Global data for internal routines.
 * Use structure and pass by reference to make code thread safe.
 */
typedef struct strfmon_data {
	int	w_width;	/* minimum width for the current format */
	int	prec;		/* precision of current format */
	int	n_width;	/* width for the #n specifier */
	int	leftflag;	/* logical flag for left justification */
	int	wflag;		/* logical flag for width */
	int	pflag;		/* logical flag for precision */
	int	nflag;		/* logical flag for #n specifier */
	int	no_groupflag;	/* logical flag for grouping */
	int	signflag;	/* logical flag for the + specifier */
	int	cdflag;		/* logical flag for credit/debit specifier */
	int	parenflag;	/* logical flag for C parenthesis specifier */
	int	no_curflag;	/* logical flag for currency symbol supression*/
	int	byte_left;	/* number of byte left in the output buffer */
	char	*cur_symbol;	/* local or interantional currency symbol */
	char	fill_buf[MB_LEN_MAX+1];	/* filling character for =f specifier */
	char	*fill_char;	/* pointer to the fill char buffer */
	int	fillc_length;	/* len of multi-byte char of current locale */
} strfmon_data_t;

/*
 * Retain simple names.
 */
#define	w_width		(strfmon_data->w_width)
#define	prec		(strfmon_data->prec)
#define	n_width		(strfmon_data->n_width)
#define	leftflag	(strfmon_data->leftflag)
#define	wflag		(strfmon_data->wflag)
#define	pflag		(strfmon_data->pflag)
#define	nflag		(strfmon_data->nflag)
#define	no_groupflag	(strfmon_data->no_groupflag)
#define	signflag	(strfmon_data->signflag)
#define	cdflag		(strfmon_data->cdflag)
#define	parenflag	(strfmon_data->parenflag)
#define	no_curflag	(strfmon_data->no_curflag)
#define	byte_left	(strfmon_data->byte_left)
#define	cur_symbol	(strfmon_data->cur_symbol)
#define	fill_buf	(strfmon_data->fill_buf)
#define	fill_char	(strfmon_data->fill_char)
#define	fillc_length	(strfmon_data->fillc_length)


/*
 * libc routines for floating conversion
 */
#ifdef	_THREAD_SAFE

extern int fcvt_r();

#else

extern char *fcvt();

#endif	/* _THREAD_SAFE */


/*
 * FUNCTION: bidi_output()
 *	     This function copies the infield to output buffer, outfield, 
 *	     either by appending data to the end of the buffer when the 
 *	     direction (dir) is 1 or by inserting data to the beginning 
 *	     of the buffer when the direction (dir) is 0.
 *
 * PARAMETERS:
 *	    char *infield - The character string to be copied into the 
 *			    output buffer outfield.
 *	    char *infield - The output buffer.
 *	    int dir - When dir is 1, infield is appended to the end of 
 *		      the output buffer. 
 *	       	      When dir is 0, infield is inserted in front of the
 *		      output buffer.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	    void.
 */
static void
bidi_output(char *infield, char **outfield, int dir)
{
	int field_width;
	int i;
	char *out = *outfield;
	
	if ( !(*infield))
		return;
	field_width = strlen(infield);
	if (dir)			/* output from left to right */
		for (i = field_width ; i > 0; i--)
			*out++ = *infield++;
	else {				/* output from right to left */
		infield += (field_width - 1);
		for (i = field_width; i > 0; i--)
			*out-- = *infield--;
	}
	*outfield = out;
}


/*
 * FUNCTION: do_out_cur_sign()
 *	     This is a common function to process positive and negative 
 *	     monetary values. It outputs the sign related symbol (if needed)
 * 	     and the currency symbol (if needed) to the output buffer.
 *
 * PARAMETERS:
 *	     pn_cs_precedes - The p_cs_precedes or n_cs_precedes value.
 *	     pn_sign_posn - The p_sign_posn or n_sign_posn value.
 *	     pn_sep_by_space - The p_sep_by_space or n_sep_by_space value.
 *	     pn_sign - The positive_sign or negative_sign value.
 *	     char **begin - The address of the pointer which points to the 
 *			    begining of the output buffer.
 *	     char **end - The address of the pointer which points to the end
 *			  of the output buffer.
 *	     int sign - The sign of the current formatting monetary value.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     void.
 */
static void
do_out_cur_sign(schar pn_cs_precedes, schar pn_sign_posn, 
		schar pn_sep_by_space, char *pn_sign, char **begin, 
		char **end, int sign,
		strfmon_data_t *strfmon_data)
{
	char *b = *begin;
	char *e = *end;

	if (pn_cs_precedes == 1) {	/* cur_sym preceds quantity */
		switch (pn_sign_posn) {
		case 1:	
		case 3:
			if (!no_curflag) {	
				if (pn_sep_by_space == 1)
					*b-- = ' ';
				bidi_output(cur_symbol, &b, 0);
			}
			if (!parenflag && !cdflag)
				bidi_output(pn_sign, &b, 0);
			break;
		case 2:
			if (!no_curflag) {
				if (pn_sep_by_space == 1)
					*b-- = ' ';
				bidi_output(cur_symbol, &b, 0);
			}
			if (!parenflag && !cdflag)
				bidi_output(pn_sign, &e, 1);
			break;
		case 4:
			if (!parenflag && !cdflag)
				bidi_output(pn_sign, &b, 0);
			if (!no_curflag) {
				if (pn_sep_by_space == 1)
					*b-- = ' ';
				bidi_output(cur_symbol, &b, 0);
			}
			break;
		}
	} else if (pn_cs_precedes == 0) { /* cur_sym after quantity */
		switch (pn_sign_posn) {
		case 1:
			if (!parenflag && !cdflag)
				bidi_output(pn_sign, &b, 0);
			if (!no_curflag) {
				if (pn_sep_by_space == 1)
					*e++ = ' ';
				bidi_output(cur_symbol, &e, 1);
			}
			break;
		case 2:
		case 4:
			if (!no_curflag) {
				if (pn_sep_by_space == 1)
					*e++ = ' ';
				bidi_output(cur_symbol, &e, 1);
			}
			if (!parenflag && !cdflag)
				bidi_output(pn_sign, &e, 1);  
			break;
		case 3:
			if (!parenflag && !cdflag)
				bidi_output(pn_sign, &e, 1);
			if (!no_curflag) {
				if (pn_sep_by_space == 1)
					*e++ = ' ';
				bidi_output(cur_symbol, &e, 1);
			}
			break;
		}
	} else {		/* currency position not defined */
		switch (pn_sign_posn) {
		case 2:
			if (!parenflag && !cdflag)
				bidi_output(pn_sign, &e, 1);
			break;
		case 1:
		default:
			if (!parenflag && !cdflag)
				bidi_output(pn_sign, &b, 0);
			break;
		}
	}
	*begin = b;
	*end = e;
}


/*
 * FUNCTION: out_cur_sign()
 *	     This function ouputs the sign related symbol (if needed) and 
 *	     the currency symbol (if needed) to the ouput buffer. It also
 *	     updates the beginning and ending pointers of the formatted 
 *	     string. This function indeed extract the sign related information
 *	     of the current formatting value and pass them to the sign  
 *	     independent formatting function do_out_cur_sign().
 *
 * PARAMETERS:
 *           _LC_monetary_t *hdl - the handle of the pointer to the LC_MONETARY
 *                                 catagory of the specific locale.
 *	     char **begin - The address of the pointer which points to the 
 *			    begining of the output buffer.
 *	     char **end - The address of the pointer which points to the end
 *			  of the output buffer.
 *	     int sign - The sign of the current formatting monetary value.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     void.
 */
static void
out_cur_sign(_LC_monetary_t *hdl, char **begin, char **end, int sign,
	     strfmon_data_t *strfmon_data)
{
	schar pn_cs_precedes;
	schar pn_sign_posn;
	schar pn_sep_by_space;
	char *pn_sign;
	size_t sign_len;
	register int i;

	if (sign) {	/* negative number with sign and currency symbol */ 
		pn_cs_precedes = hdl->n_cs_precedes;
		pn_sign_posn = hdl->n_sign_posn;
		pn_sep_by_space = hdl->n_sep_by_space;
		if (!*(pn_sign = hdl->negative_sign))
		  pn_sign = "-\0";
	} else {	/* positive number with sign and currency symbol */
		pn_cs_precedes = hdl->p_cs_precedes;
		pn_sign_posn = hdl->p_sign_posn;
		pn_sep_by_space = hdl->p_sep_by_space;

                /* bogus code to pad for opposite sign */
                sign_len = strlen(hdl->negative_sign);
                if (nflag && !(strlen(hdl->positive_sign)) && (sign_len)) {
		    pn_sign = (char *)malloc(sign_len+1);
		    for (i = 0; i < sign_len; i++)
		        pn_sign[i] = ' ';
		    pn_sign[sign_len] = '\0';
		} else
		    pn_sign = hdl->positive_sign;
	}
	do_out_cur_sign(pn_cs_precedes, pn_sign_posn,
			pn_sep_by_space, pn_sign, begin, end,
			sign, strfmon_data);
}


/*
 * FUNCTION: digits_to_left()
 *	Compute number of digits to left of decimal point.
 *	Scale number to range 10.0..1.0 counting divides or
 *	deducting multiplies.
 *	Positive value mean digits to left, negative is digits to right.
 *	If the number is very large scale using large divisors.
 *	If its intermediate do it the slow way.
 *	If its very small scale using large multipliers
 *	This replaces the totally IEEE dependent __ld() with a
 *	(mostly) independent version stolen from ecvt.c.
 *	Slight speed mods since ecvt does more work than we need.
 *
 * PARAMETERS:
 *	double dvalue	- value to work with
 *
 * RETURN VALUE DESCRIPTIONS:
 *	int		- number of digits to left of decimal point
 */
static int
digits_to_left(double dvalue)
{
	struct log {		/* scale factors */
		double	pow10;
		int	pow;
	} log[] = {	1e32,	32,
			1e16,	16,
			1e8,	8,
			1e4,	4,
			1e2,	2,
			1e1,	1 };
	register struct log	*scale = log;
	register int		digits = 1;	/* default (no scale) */

	/* check for fluff */
	if (IS_NAN(dvalue) || IS_INF(dvalue) || IS_ZERO(dvalue))
		return (0);

	/* make it positive */
	dvalue = fabs(dvalue);

	/* now scale it into 10.0..1.0 range */
	if (dvalue >= 2.0 * DMAXPOWTWO) {	/* big */
		do {	/* divide down */
			for (; dvalue >= scale->pow10; digits += scale->pow)
				dvalue /= scale->pow10;
		} while ((scale++)->pow > 1);

	} else if (dvalue >= 10.0) {		/* medium */
		do {	/* divide down */
			digits++;
		} while ((dvalue /= 10.0) >= 10.0);

	} else if (dvalue < 1.0) {		/* small */
		do {	/* multiply up */
			for (; dvalue * scale->pow10 < 10.0;
			     digits -= scale->pow)
				dvalue *= scale->pow10;
		} while ((scale++)->pow > 1);
	}
	return (digits);
}


/*
 * FUNCTION: do_format()
 *           This function performs all the necessary formating for directives
 *	     %a and %c. The output will be written to the output buffer str.
 *	     and the number of formatted bytes are returned.
 *
 * PARAMETERS:
 *           _LC_monetary_t *hdl - the handle of the pointer to the LC_MONETARY
 *                                 catagory of the specific locale.
 *           char *str - location of returned string
 *           size_t maxsize - maximum length of output including the null
 *                            termination character.
 *           char *dval - the double value to be formatted.
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns the number of bytes that comprise the return string
 *             excluding the terminating null character.
 *           - returns 0 if s is longer than maxsize or any error.
 */
static int
do_format(_LC_monetary_t *hdl, char *str, size_t maxsize, double dval,
	  strfmon_data_t *strfmon_data)
{
	char *s;		/* work buffer for string handling */
	char *number;		/* work buffer for converted string of fcvt() */
	int dig_to_left;	/* number of digits to the left of decimal in
				   the actual value of dval */
	int fcvt_prec;		/* number of digits to the right of decimal for 
				   conversion of fcvt */
	int decpt;		/* a decimal number to show where the radix 
				   character is when counting from beginning */
	int sign;		/* 1 for negative, 0 for positive */
	char *b;		/* points to the beginning of the output */
	char *e;		/* points to the end of the output */
	char lead_zero[MAXFSIG];/* leading zeros if necessary */
	char *grouping;		/* the grouping string of the locale */
	char *separator;	/* thousand separator string of the locale */
	char *radix;		/* locale's radix character */
	char *left_par;		/* locale's left parenthesis */
	char *right_par;	/* locale's right parenthesis */
	int i, j, k = 0;	/* working variable as index */
	int gap;		/* number of filling character needed */
	int group = 0;		/* current group */
	int repeat_group = 0;	/* flag for repeating the grouping number */
	int last_group = 0;	/* Previous group */
	int sep_width;		/* the width of the thousand separator */
	int radix_width;	/* the width of the radix character */
	char temp_buff[SIZE];	/* a work buffer for the whole formatted str */
	char *temp;		/* pointer to temp_buff */
	char *temp_m = NULL;	/* pointer to temp buffer created by malloc */
	int digits_todo;	/* remaining digits to left to process */
	int fill_todo;		/* remaining number of fill to process */
#ifdef	_THREAD_SAFE
	char fcvt_buf[NDIG];	/* buffer holds the string ret. from fcvt() */
#endif	/* _THREAD_SAFE */

	/*
	 * First, convert the double value into a character string buy
	 * using fcvt(). To determin the precision used by fcvt():
	 *
	 * if no digits to left of decimal,
	 * 	then all requested digits will be emitted to right of decimal.
	 *	hence, use max of (max-sig-digits,user's-requested-precision).
	 *
	 * else if max-sig-digits <= digits-to-left
	 * 	then all digits will be emitted to left of decimal point.
	 *  	Want to use zero or negative prec value to insure that rounding
	 * 	will occur rather than truncation.
	 *
	 * else
	 *	digits can be emitted both to left and right of decimal, but
	 *	only potential for rounding/truncation is to right of decimal.
	 *	Hence, want to use user's request precision if it will not 
	 *	cause truncation, else use largest prec that will round 
	 *	correctly when max. number of digits is emitted.
	 */
  
	if ( 2*maxsize < SIZE)
		temp = temp_buff;
	else if (temp_m = (char *) malloc(2*maxsize))
		temp = temp_m;
	else
		return (0);	/* malloc failed */

	dig_to_left = digits_to_left(dval);
	if (dig_to_left <= 0)		/* determine the precision to be used */
		fcvt_prec = min(prec, MAXFSIG);
	else if (dig_to_left >= MAXFSIG) 
		fcvt_prec = MAXFSIG - dig_to_left;
	else
		fcvt_prec = min(prec, MAXFSIG - dig_to_left);

#ifdef	_THREAD_SAFE
	fcvt_r(dval, fcvt_prec, &decpt, &sign, fcvt_buf, NDIG);
	number = fcvt_buf;
#else
	number = fcvt(dval, fcvt_prec, &decpt, &sign);
#endif	/* _THREAD_SAFE */
	/*
	 * Fixing the position of the radix character (if any). Output the
	 * number using the radix as the reference point. When output grows to
	 * the right, decimal digits are processed and appropriate precision (if
	 * any) is used. When output grows to the left, grouping of digits
	 * (if needed), thousands separator (if any), and filling character
 	 * (if any) will be used.
	 * 
	 * Begin by processing the decimal digits.
	 */

	b = temp + maxsize;
	if (prec) {
		if (*(radix = hdl->mon_decimal_point))
		{
					 /* set radix character position */
			radix_width = strlen(radix);
			for (i=1; i <= radix_width; i++)
				*(b+i) = *radix++;
			e = b + radix_width + 1;
		}
		else {			/* Radix character is null string */
			*(b+1)='.';	/* in locale.  Use a period */
			e = b + 2;
		}
		s = number + decpt; 	/* copy the digits after the decimal */

		for (i=0; i < prec; i++)
			if (*s)
			    *e++ = *s++;
			else
			    *e++ = '0';  /* trailing zeroes needed */
	} else 		/* zero precision, no radix character needed */
	  e = b + 1;

	/*
	 * Output the digits preceeding the radix character.
	 */

	s = number + decpt -1;
	grouping = hdl->mon_grouping;
	separator = hdl->mon_thousands_sep;
	sep_width = strlen(separator);
	if (no_groupflag || !*grouping || !*separator){ /* no grouping needed*/
		for (i=0; i < dig_to_left; i++) 
		  *b-- = *s--;
		if ((gap = n_width - dig_to_left) > 0)
		  for (i=0; i < gap; i++)
		    for (j = fillc_length-1; j >= 0; j--)
		      *b-- = fill_buf[j];
	} else {
		digits_todo = dig_to_left;
		fill_todo = ( (n_width > dig_to_left) ? 
			     n_width - dig_to_left : 0);
		repeat_group = 0;
		last_group=0;
		while (digits_todo || fill_todo) {
			if (!repeat_group) {
				group = (int)*grouping;
				if (group) {	/* group != 0 */
						 /* save current group */
					last_group = group;
					grouping++;
				} else {
						/* check to make sure */
						/* first group not 0 */
					if (!last_group)
					  last_group = CHAR_MAX;
						/* restore prev group */
					group = last_group;
					repeat_group++;
				}
			} else
			    group = last_group;
			for ( ; group > 0 ; group--) {
				if (digits_todo) {
					*b-- = *s--;
					digits_todo--;
				} else if (fill_todo) {
					for (i = fillc_length-1; i>=0; i--) 
					  *b-- = fill_buf[i];
					fill_todo--;
				}
			}
			if (digits_todo)
			  for (i = sep_width-1; i >= 0; i--)
			    *b-- = *(separator+i);
			else if (fill_todo)
			  for (i = fillc_length-1; i >= 0; i--)
			    *b-- = fill_buf[i];
			/* Don't decrement fill_todo.  This fill character */
			/* is taking the place of a separator */
		} /* while digits_todo or fill_todo */
	} /* else (grouping is needed) */ 

	/*
	 * Determine if padding is needed. 
	 * If the number of digit prceeding the radix character is greater
	 * than #n (if any), #n is ignored. Otherwise, the output is padded
	 * with filling character (=f, if any) or blank is used by default.
	 */
	
	if (nflag && (gap = n_width - (temp + maxsize - b)) > 0) {
		for (i=0; i < gap; i++)
			for (j=fillc_length-1; j>=0; j--)
				*b-- = fill_buf[j];
	}

	/* 
	 * At here, the quantity value has already been decided. What comes
	 * next are the positive/negative sign, monetary symbol, parenthesis,
	 * and CR/DB symbols. The signflag, cdflag, parenflag, and no_curflag
	 * will be considered first to determine the sign and currency format.
	 * If none of them are defined, the locale's defaults are used.
	 */

	if (signflag) {		/* use locale's +/- sign repesentation */
		out_cur_sign(hdl, &b, &e, sign, strfmon_data);
	} else if (cdflag) {	/* CR/DB sign is used */
		if (sign) {	/* negative number */
			if (*hdl->debit_sign)
				bidi_output(hdl->debit_sign, &e, 1);
		}
		else {
			if (*hdl->credit_sign)
				bidi_output(hdl->credit_sign, &e, 1);
		}
		out_cur_sign(hdl, &b, &e, sign, strfmon_data);
	}
	else if (parenflag) {
		if (!*(left_par = hdl->left_parenthesis))
		  left_par = "(\0";
		if (!*(right_par = hdl->right_parenthesis))
		  right_par = ")\0";

		if (sign)
			if (hdl->n_sign_posn == 0) {
				out_cur_sign(hdl, &b, &e, sign, strfmon_data);
				bidi_output(left_par, &b, 0);
				bidi_output(right_par, &e, 1);
			}
			else {
				out_cur_sign(hdl, &b, &e, sign, strfmon_data);
				bidi_output(left_par, &b, 0);
				bidi_output(right_par, &e, 1);
			}
		else
			out_cur_sign(hdl, &b, &e, sign, strfmon_data);

	} else {	/* use all the attributes of the locale's default */
		if (sign)
			i = hdl->n_sign_posn;
		else
			i = hdl->p_sign_posn;
		switch (i) {
		case 0:		/* Paren. around currency and quantity*/
			out_cur_sign(hdl, &b, &e, sign, strfmon_data);
			if (!*(left_par = hdl->left_parenthesis))
			  left_par = "(\0";
			if (!*(right_par = hdl->right_parenthesis))
			  right_par = ")\0";
			bidi_output(left_par, &b, 0);
			bidi_output(right_par, &e, 1);
			break;
				
		case 5:
			if (sign && *hdl->credit_sign)
				bidi_output(hdl->credit_sign, &e, 1);
			else if (!sign && *hdl->debit_sign)
				bidi_output(hdl->debit_sign, &e, 1);
			out_cur_sign(hdl, &b, &e, sign, strfmon_data);
			break;

		case 1: 	/* sign preceed currency and quantity */
		case 2:		/* sign succeed currency and quantity */
		case 3:		/* sign preceed currency symbol */
		case 4:		/* sign succeed currency symbol */
		default:
			out_cur_sign(hdl, &b, &e, sign, strfmon_data);
			break;

		}
	} /* else */

	/* 
	 * By setting e (the last byte of the buffer) to \0 and increment 
	 * b (the first byte of the buffer), now the temp buffer should 
 	 * have a completely formatted null terminated string starting and
	 * ending at b and e. Before the result is copied into the s buffer,
	 * check if the formatted string is less than the w-field width and
	 * determine its left or right justification.
	 */
				
	b++;
	*e = '\0';
	i = strlen(b);
	if (max(i, w_width) > byte_left) {
		_Seterrno(E2BIG);
		return (0);
	}
	if (wflag && i < w_width) {	/* justification is needed */
		if (leftflag) {
			while (*b)
				*str++ = *b++;
			for (i = w_width-i; i > 0; i--)
				*str++ = ' ';
		} else {
			for (i = w_width-i; i > 0; i--)
				*str++ = ' ';
			while (*b)
				*str++ = *b++;
		}
		*str = '\0';
		if (temp_m)
			free(temp_m);
		return (w_width);
	} else {
		strcpy(str, b);
		if (temp_m)
			free(temp_m);
		return (i);
	}
}


/*
 * FUNCTION: This is the standard method for function strfmon().
 *           It formats a list of double values and output them to the
 *	     output buffer s. The values returned are affected by the format
 *	     string and the setting of the locale category LC_MONETARY. 
 *
 * PARAMETERS:
 *           _LC_monetary_t *hdl - the handle of the pointer to the LC_MONETARY
 *                                 catagory of the specific locale.
 *           char *s - location of returned string
 *           size_t maxsize - maximum length of output including the null
 *			      termination character.
 *           char *format - format that montary value is to be printed out
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns the number of bytes that comprise the return string
 *             excluding the terminating null character.
 *           - returns 0 if s is longer than maxsize or error detected.
 */
ssize_t 
__strfmon_std(char *s, size_t maxsize, const char *format,
	      va_list ap, _LC_monetary_t *hdl)
{
	char *strp;		/* pointer to output buffer s */
	const char *fbad;	/* points to where format start to be invalid */
	char *strend;		/* last available byte in output buffer s */
	char ch;		/* the current character in the format string */
	int toolong = 0;	/* logical flag for valid output length */
	int i;			/* a working variable */

	strfmon_data_t real_strfmon_data;
	strfmon_data_t *strfmon_data = &real_strfmon_data;

	fill_char = fill_buf;

	byte_left = maxsize;
	strp = s;
	strend = s + maxsize - 1;
	while ((ch = *format++) && !toolong) {
		if (ch != '%') {
			strp < strend ? *strp++ = (ch) : toolong++;
			byte_left--;
		}
		else { 
			fbad = format;
			fill_buf[0] = ' ';
			fill_buf[1] = '\0';
			fillc_length = 1;
			prec = w_width = n_width = 0;
			leftflag = wflag = pflag = nflag = no_groupflag = 0;
			signflag = cdflag = parenflag = no_curflag = 0;

			/* scan the flags */
			for ( ; ; ) {
			  switch (*format) {
			  case '=':	/* =f fill character */
			     format++;
			     if ((fillc_length=mblen(format, MB_CUR_MAX))!=-1){
				for (i = fillc_length; i>0; i--)
					*fill_char++ = *format++;
				        *fill_char = '\0';
					fill_char = fill_buf;
				} else
					return (-1);	/* invalid char */
			    continue;
			  case '^':     /* no grouping for thousands */
			    format++;
			    no_groupflag++;
			    continue;
			  case '+':	/* locale's +/- sign */
			    format++;
			    signflag++;
			    continue;
			  case '(':    /* locale's parentheses */
			    format++;
			    parenflag++;
			    continue;
			  case '!':    /* suppress currency symbol */
			    format++;
			    no_curflag++;
			    continue;
			  case '-': /* Left Justify */
			    format++;
			    leftflag++;
			    continue;
			  }
			  break;
		        }

			/* get width alignment value, if exists */
			while (isdigit(*format)) {	/* w width field */
				w_width *= 10;
				w_width += *format++ - '0';
				wflag++;
			}

			if (*format == '#') {	/* #n digits preceeds decimal */
				nflag++;
				format++;
				while (isdigit(*format)) {
					n_width *= 10;
					n_width += *format++ - '0';
				}
			}
			if (*format == '.') {		/* .p precision */
				pflag++;
				format++;
				while (isdigit(*format)) {
					prec *= 10;
					prec += *format++ - '0';
				}
			}
			switch (*format++) {
			case '%':
				strp < strend ? *strp++ = '%' : toolong++;
				byte_left--;
				break;

			case 'i':	/* international currency format */
				cur_symbol = hdl->int_curr_symbol;
				if (!pflag 
				  && (prec = hdl->int_frac_digits) == CHAR_MAX)
					prec = 2;
				if ((i = do_format(hdl, strp, maxsize,
						   va_arg(ap, double),
						   strfmon_data)) == 0)
					return (-1);
				strp += i;
				byte_left -= i;
				break;

			case 'n':	/* local currency format */
				cur_symbol = hdl->currency_symbol;
				if (!pflag 
				  && (prec = hdl->frac_digits) == CHAR_MAX)
				  prec = 2;
				if ((i = do_format(hdl, strp, maxsize,
						   va_arg(ap, double),
						   strfmon_data)) == 0)
					return (-1);
				strp += i;
				byte_left -= i;
				break;
			default:
				format = fbad;
				strp < strend ? *strp++ = '%' : toolong++;
				byte_left--;
				break;
			}
		} /* else */
		
	} /* while */
	/* Trailing null if it fits */
	strp < strend ? *strp = '\0' : toolong++; 
	if (toolong) {
		_Seterrno(E2BIG);
		return (-1);
	}
	return (strp - s);
}

/*
 *
 * FUNCTION: strfmon() is a method driven fucntion where the actual monetary
 *	     formatting is done by a method pointed to by the current locale.
 *           The method formats a list of values and outputs them to the
 *           output buffer s. The values returned are affected by the format
 *           string and the setting of the locale category LC_MONETARY.
 *
 * PARAMETERS:
 *           char *s - location of returned string
 *           size_t maxsize - maximum length of output including the null
 *                            termination character.
 *           char *format - format that montary value is to be printed out
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns the number of bytes that comprise the return string
 *             excluding the terminating null character.
 *           - returns 0 if s is longer than maxsize
 */
ssize_t
strfmon(char *s, size_t maxsize, const char *format, ...)
{
	va_list ap;
	int	i;

	va_start(ap, format);
	if (METHOD(__lc_monetary,strfmon) == NULL)
		i = __strfmon_std( s, maxsize, format, ap, __lc_monetary);
	else
		i = METHOD(__lc_monetary,strfmon)(s, maxsize,
				          format, ap, __lc_monetary);
	va_end(ap);
	return ((ssize_t)i);
}

