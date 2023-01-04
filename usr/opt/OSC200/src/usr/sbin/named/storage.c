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
static char	*sccsid = "@(#)$RCSfile: storage.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:15:17 $";
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

*/
/* 
 * COMPONENT_NAME: TCPIP storage.c
 * 
 * FUNCTIONS: calloc, cfree, openlog, rt_free, rt_malloc, rt_prmem, 
 *            syslog 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *			S T O R A G E . C
 *
 * Ray Tracing program, storage manager.
 *
 *  Functions -
 *	rt_malloc	Allocate storage, with visibility & checking
 *	rt_free		Similarly, free storage
 *	rt_prmem	When debugging, print memory map
 *	calloc, cfree	Which call rt_malloc, rt_free
 *
 *  Author -
 *	Michael John Muuss
 *  
 *  Source -
 *	SECAD/VLD Computing Consortium, Bldg 394
 *	The U. S. Army Ballistic Research Laboratory
 *	Aberdeen Proving Ground, Maryland  21005-5066
 *  
 *  Copyright Notice -
 *	This software is Copyright (C) 1987 by the United States Army.
 *	All rights reserved.
 */
/*
#ifndef lint

#endif
*/

#include <sys/param.h>
#if BSD >= 43
#include <sys/syslog.h>
#else
#include <stdio.h>
#define	LOG_ERR	0
#endif BSD

#undef malloc
#undef free

#define MDB_SIZE	20000
#define MDB_MAGIC	0x12348969
struct memdebug {
	char	*mdb_addr;
	char	*mdb_str;
	int	mdb_len;
} rt_mdb[MDB_SIZE];

/*
 *			R T _ M A L L O C
 */
char *
rt_malloc(cnt)
unsigned int cnt;
{
	register char *ptr;
	extern char *malloc();

	cnt = (cnt+2*sizeof(int)-1)&(~(sizeof(int)-1));
	ptr = malloc(cnt);

	if( ptr==(char *)0 ) {
		syslog(LOG_ERR, "rt_malloc: malloc failure");
		abort();
	} else 	{
		register struct memdebug *mp = rt_mdb;
		for( ; mp < &rt_mdb[MDB_SIZE]; mp++ )  {
			if( mp->mdb_len > 0 )  continue;
			mp->mdb_addr = ptr;
			mp->mdb_len = cnt;
			mp->mdb_str = "???";
			goto ok;
		}
		syslog(LOG_ERR, "rt_malloc:  memdebug overflow\n");
	}
ok:	;
	{
		register int *ip = (int *)(ptr+cnt-sizeof(int));
		*ip = MDB_MAGIC;
	}
	return(ptr);
}

/*
 *			R T _ F R E E
 */
void
rt_free(ptr)
char *ptr;
{
	register struct memdebug *mp = rt_mdb;
	for( ; mp < &rt_mdb[MDB_SIZE]; mp++ )  {
			if( mp->mdb_len <= 0 )  continue;
		if( mp->mdb_addr != ptr )  continue;
		{
			register int *ip = (int *)(ptr+mp->mdb_len-sizeof(int));
			if( *ip != MDB_MAGIC )  {
				syslog(LOG_ERR, "ERROR rt_free(x%x, %s) corrupted! x%x!=x%x\n", ptr, "???", *ip, MDB_MAGIC);
				abort();
			}
		}
		mp->mdb_len = 0;	/* successful free */
		goto ok;
	}
	syslog(LOG_ERR, "ERROR rt_free(x%x, %s) bad pointer!\n", ptr, "???");
	abort();
ok:	;

	*((int *)ptr) = -1;	/* zappo! */
	free(ptr);
}

/*
 *			R T _ P R M E M
 * 
 *  Print map of memory currently in use.
 */
void
rt_prmem(str)
char *str;
{
	register struct memdebug *mp = rt_mdb;
	register int *ip;

	printf("\nRT memory use\t\t%s\n", str);
	for( ; mp < &rt_mdb[MDB_SIZE]; mp++ )  {
		if( mp->mdb_len <= 0 )  continue;
		ip = (int *)(mp->mdb_addr+mp->mdb_len-sizeof(int));
		printf("%7x %5x %s %s\n",
			mp->mdb_addr, mp->mdb_len, mp->mdb_str,
			*ip!=MDB_MAGIC ? "-BAD-" : "" );
		if( *ip != MDB_MAGIC )
			printf("\t%x\t%x\n", *ip, MDB_MAGIC);
	}
}

char *
calloc(num, size)
	register unsigned num, size;
{
	extern char *malloc();
	register char *p;

	size *= num;
	if (p = rt_malloc(size))
		bzero(p, size);
	return (p);
}

cfree(p, num, size)
	char *p;
	unsigned num;
	unsigned size;
{
	rt_free(p);
}

#if BSD < 43
openlog() {}

syslog(x, str, a, b, c, d, e, f)
int	x;
char	*str;
int	a, b, c, d, e, f;
{
	fprintf(stderr, str, a, b, c, d, e, f);
}
#endif BSD
