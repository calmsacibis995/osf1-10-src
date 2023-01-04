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
 *	@(#)$RCSfile: print.h,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 23:35:42 $
 */
/*
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCPRNT) Standard C Library Print Functions 
 *
 * FUNCTIONS: 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/types.h>

/* Maximum number of digits in any integer representation */
#ifdef __alpha
#define MAXDIGS 24
#else
#define MAXDIGS 11
#endif

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

/* Data type for flags */
typedef char bool;

/* Convert a digit character to the corresponding number */
#define tonumber(x) ((x)-'0')

/* Convert a number between 0 and 9 to the corresponding digit */
#define todigit(x) ((x)+'0')

/* Max and Min macros */
#define max(a,b) ((a) > (b)? (a): (b))
#define min(a,b) ((a) < (b)? (a): (b))

/* duplicate of code in stdiom.h */
#define _WRTCHK(iop)	((((iop->_flag & (_IOWRT | _IOEOF)) != _IOWRT) \
				|| (iop->_base == NULL)  \
				|| (iop->_ptr == iop->_base && iop->_cnt == 0 \
					&& !(iop->_flag & (_IONBF | _IOLBF)))) \
			? _wrtchk(iop) : 0 )

/* The infamous NDIG for cvt buffers */
#define	NDIG	(max(MAXDIGS, 1+max(MAXFCVT+MAXEXP, MAXECVT)))

/* Argument manipulation (for reordered arguments)
 */
#define RFREE(s)	if (s) free((void *)s)
#define UNSET	(-1)
#define get_arg(va,da,type) (reorderflag == FALSE ? \
			va_arg(va,type) : (((type *)&((da)++->da_arg))[0]))

enum arg_type {
	DA_T_UNDEF,
	DA_T_INT,	/* short promotes to int */
	DA_T_LONG,
	DA_T_DOUBLE,	/* float promotes to double */
	DA_T_POINTER,	/* void * is equivalent to char * (ANSI) */
	DA_T_SHORTP,
	DA_T_INTP,
	DA_T_WINTP,
	DA_T_LONGP,
	DA_T_FLOATP,
	DA_T_DOUBLEP
	/* unsigned types have same alignment/ size as signed (ANSI) */
};

typedef struct arglist {
        union {
		struct arglist	*au_arg;

		int		au_int;
		long		au_long;
		unsigned	au_uint;
		double		au_double;
		void		*au_pointer;
		short		*au_shortp;
		int		*au_intp;
		wint_t		*au_wintp;
		long		*au_longp;
		float		*au_floatp;
		double		*au_doublep;
	} da_arg;
        enum arg_type	da_type;
} arglist;
