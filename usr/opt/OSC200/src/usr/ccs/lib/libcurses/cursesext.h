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
/*
static char rcsid[] = "@(#)$RCSfile: cursesext.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/12 21:12:30 $";
 */
/*
 * HISTORY
/* cursesext.h       1.8  com/lib/curses,3.1,8943 10/18/89 16:49:55 */
#ifndef _H_CURSESEXT
#define _H_CURSESEXT

/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   cursesext.h
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * NAME:        cursesext.h
 *
 * FUNCTION:
 *
 *      External variables for the library.
 */


/* LINTLIBRARY */

# define CURSES	/* We are internal to curses */

# ifdef NONSTANDARD
#  include "RecStruct.h"
#  include "VTio.h"
# endif

# include "curses.h"
# include "term.h"
# include "curshdr.h"
# include "unctrl.h"

#endif /* _H_CURSESEXT */
