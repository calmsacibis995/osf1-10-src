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
 *	@(#)$RCSfile: stdiom.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:52:13 $
 */ 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: 
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* The following macros improve performance of the stdio by reducing the
	number of calls to _bufsync and _wrtchk.  _BUFSYNC has the same
	effect as _bufsync, and _WRTCHK has the same effect as _wrtchk,
	but often these functions have no effect, and in those cases the
	macros avoid the expense of calling the functions.  */

extern void	_bufsync(FILE *);
extern int	_wrtchk(FILE *);

#define _BUFSYNC(iop)	if (_bufend(iop) - iop->_ptr <   \
				( iop->_cnt < 0 ? 0 : iop->_cnt ) )  \
					_bufsync(iop)

#define _WRTCHK(iop)    ((((iop->_flag & (_IOWRT | _IOEOF)) != _IOWRT) \
                                || (iop->_base == NULL)  \
                                || (iop->_ptr == iop->_base && iop->_cnt == 0 \
                                        && !(iop->_flag & (_IONBF | _IOLBF))))\
			? _wrtchk(iop) : 0 )
