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
static char *rcsid = "@(#)$RCSfile: prtmem.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/11/20 17:25:34 $";
#endif

/* static char	*sccsid = "@(#)prtmem.c	4.1	(ULTRIX)	8/17/88"; */

/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/*
 *
 *   Modification History:
 *
 *
 */

#ifdef GETU
#define udsize uu[0]
#define ussize uu[1]
#endif


prtmem()
{
#ifdef GETU
#include "stdio.h"
#include "sys/param.h"
#include "sys/dir.h"
#include "sys/user.h"
	unsigned uu[2];
	register int i;

	if(getu( &((struct user *)0)->u_dsize, &uu, sizeof uu) > 0)
	{
		udsize *= 64;
		ussize *= 64;
		printf("mem: data = %u(0%o) stack = %u(0%o)\n",
			udsize, udsize, ussize, ussize);
/*
 *	The following works only when `make' is compiled
 *	with I&D space separated (i.e. cc -i ...).
 *	(Notice the hard coded `65' below!)
 */
		udsize /= 1000;
		ussize /= 1000;
		printf("mem:");
		for(i=1; i<=udsize;i++)
		{
			if((i%10) == 0)
				printf("___");
			printf("d");
		}
		for(;i<=65-ussize;i++)
		{
			if((i%10) == 0)
				printf("___");
			printf(".");
		}
		for(;i<=65;i++)
		{
			if((i%10) == 0)
				printf("___");
			printf("s");
		}
		printf("\n");
		fflush(stdout);
	}
#endif
}
