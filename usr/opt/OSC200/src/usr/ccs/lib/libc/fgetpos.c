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
static char	*sccsid = "@(#)$RCSfile: fgetpos.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:50:35 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * FUNCTIONS: fgetpos 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * fgetpos.c	1.8  com/lib/c/io,3.1,8943 10/10/89 15:18:53
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <stdio.h>
#include <errno.h>

/*                                                                    
 * FUNCTION: Stores the current value of the file position indicator for the
 *           stream pointed to by stream in the object pointed to by pos.
 *
 * PARAMETERS: FILE *stream - stream to be searched
 *
 *	       fpos_t  *pos      - current value of file position
 *
 * RETURN VALUE DESCRIPTIONS:
 * 	      zero if successful
 *	      non-zero if not successful
 */

int	
fgetpos(FILE *stream, fpos_t *pos)
{
	if (stream == NULL) {
	    seterrno(EINVAL);
	    return (-1);
	} else if ((*pos = ftell(stream)) >= 0) 
		return(0);
	/*
	 * errno was set by ftell()
	 */
	return(-1);
}
