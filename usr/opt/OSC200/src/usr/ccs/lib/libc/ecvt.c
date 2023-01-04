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
static char	*sccsid = "@(#)$RCSfile: ecvt.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/06/25 21:47:46 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 */
/*
 * FUNCTIONS: ecvt, fcvt
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * ecvt.c	1.9  com/lib/c/cnv,3.1,8943 10/27/89 16:12:54
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak ecvt_r = __ecvt_r
#pragma weak fcvt_r = __fcvt_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak ecvt = __ecvt
#pragma weak fcvt = __fcvt
#endif
#endif
#include <math.h>
#include <fp.h>
#include <values.h>
#include <errno.h>
#include "print.h"

#include "ts_supp.h"

#define	NMAX	((DSIGNIF * 3 + 19)/10) /* restrict max precision */

#ifdef	_THREAD_SAFE
#define	RETURN(s)	{ strcpy(buf, s); return TS_SUCCESS;}
#else
#define	RETURN(s)	return s
static char	buf[NDIG];
#endif	/* _THREAD_SAFE */

static char *
cvt(double value, int mode, int ndigits, int *decpt, int *sign, char *buf)
{
	register char *p;
	register char *p_last;
	double valuem = value;		/* in memory for VALH to work */
	int f_flag;

	f_flag = (mode == 3) ? 1 : 0;
	p = &buf[0];
        p_last = &buf[ndigits];
        buf[0] = '\0';
        *decpt = 0;
        *sign = VALH(valuem) >> 31;

        if( IS_QNAN(valuem) )
                return("NaNQ");
        if( IS_NAN(valuem) )
                return("NaNS");
        if( IS_INF(valuem) )
                return("INF");
        if( IS_ZERO(valuem) ) {
                for ( ; p < p_last ; p++ )
                        *p = '0';
                *p = '\0';
                return(buf);
        }
        if (ndigits > 17)  {
                *decpt = _dtoa (buf, 17, value, f_flag) + 1;
        }
        else if (ndigits < 0) {
                *decpt = _dtoa (buf, 0, value, f_flag) + 1;
        }
        else {
                *decpt = _dtoa (buf, ndigits, value, f_flag) + 1;
        };
        *sign = buf[0] == '-';
        return(buf+1);
}


/*
 * NAME: ecvt
 *
 * FUNCTION: convert double to character string
 *
 * NOTES:
 *
 *	ecvt converts to decimal
 *	the number of digits is specified by ndigit
 *	decpt is set to the position of the decimal point
 *	sign is set to 0 for positive, 1 for negative
 *
 * RETURNS: a character string
 */
#ifdef _THREAD_SAFE
int
ecvt_r(double value, int ndigit, int *decpt, int *sign, char *buf, int len)
#else
char *
ecvt(double value, int ndigit, int *decpt, int *sign)
#endif	/* _THREAD_SAFE */
{
	TS_EINVAL((buf == 0) || (len < NDIG));
	RETURN(cvt(value, 2, ndigit, decpt, sign, buf));
}


/*
 * NAME: fcvt
 *
 * FUNCTION: convert double to character string if FORTRAN format
 *
 * NOTES:
 *
 * RETURNS: a character string
 *
 */
#ifdef _THREAD_SAFE
int
fcvt_r(double value, int ndigit, int *decpt, int *sign, char *buf, int len)

#else
char *
fcvt(double value, int ndigit, int *decpt, int *sign)
#endif	/* _THREAD_SAFE */
{
	TS_EINVAL((buf == 0) || (len < NDIG));
	RETURN( cvt(value, 3, ndigit, decpt, sign, buf));
}

