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
static char	*sccsid = "@(#)$RCSfile: calloc.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/06/08 01:26:32 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: calloc, cfree
 *
 * ORIGINS: 3 26 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 * calloc.c	1.11  com/lib/c/gen,3.1,8943 10/9/89 17:54:38
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak cfree = __cfree
#endif
#include <stdlib.h>
#include <string.h>

/*
 * FUNCTION: -  Allocate and clear memory block.
 *      	Note that the multiplication of num by size,
 *      	necessary to determine the number of bytes
 *      	to malloc, may result in truncation if the
 *      	request is VERY large.  If the result of this
 *      	multiplication is less than the requested number
 *      	of elements, and no zeros were passed in as
 *      	parameters, then overflow is assumed and NULL
 *      	is returned.
 *
 *
 * PARAMETERS: nmemb  - number of members to allocate for
 *
 *		size   - size of objects
 *
 * RETURN VALUE DESCRIPTIONS:
 *		either a pointer to the allocated space or a NULL pointer
 */



void *
calloc(size_t nmemb, size_t size)
{
	size_t total;
	void *mp;

	total = nmemb * size;
	if (total < nmemb && nmemb != 0 && size != 0)    /* overflow */
		return(NULL);

	mp = malloc(total);
	if(mp != NULL)
		(void) memset(mp, 0, total);

	return(mp);
}

/*
 * NAME:	cfree
 *
 * FUNCTION:	cfree - free memory allocated by calloc
 *
 * NOTES:	Cfree frees memory previously allocated by
 *		calloc.  It's retained for backward compatibility.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

/*ARGSUSED*/

void
cfree(void *p, unsigned nmemb, unsigned size)
{
	free(p);
}
