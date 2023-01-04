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
static char	*sccsid = "@(#)$RCSfile: ns_stats.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:15:02 $";
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
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint

#endif  not lint */

/**************************************************************************/
/*                simple monitoring of named behavior                     */
/*            dumps a bunch of values into a well-know file               */
/*                                                                        */
/**************************************************************************/

#ifdef STATS

#include <sys/param.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/syslog.h>
#include <arpa/nameser.h>
#include "ns.h"
#include "pathnames.h"

#ifdef STATSFILE
char *statsfile = STATSFILE;
#else
char *statsfile = _PATH_STATS;
#endif /* STATSFILE */

extern	time_t	boottime, resettime;
extern	int	needStatsDump;

/*
 * General statistics gathered
 */
/* The position in this table must agree with the defines in ns.h */
struct stats stats[S_NSTATS] = {
	{ 0, "input packets" },
	{ 0, "output packets" },
	{ 0, "queries" },
	{ 0, "iqueries" },
	{ 0, "duplicate queries" },
	{ 0, "responses" },
	{ 0, "duplicate responses" },
	{ 0, "OK answers" },
	{ 0, "FAIL answers" },
	{ 0, "FORMERR answers" },
	{ 0, "system queries" },
	{ 0, "prime cache calls" },
	{ 0, "check_ns calls" },
	{ 0, "bad responses dropped" },
	{ 0, "martian responses" },
};

/*
 *  Statistics for queries (by type)
 */
unsigned long typestats[T_ANY+1];
char *typenames[T_ANY+1] = {
	/* 5 types per line */
	"Unknown", "A", "NS", "invalid(MD)", "invalid(MF)",
	"CNAME", "SOA", "MB", "MG", "MR",
	"NULL", "WKS", "PTR", "HINFO", "MINFO",
	"MX", "TXT", 0, 0, 0,
	/* 20 per line */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 100 */
	"UINFO", "UID", "GID", "UNSPEC", 0, 0, 0, 0, 0, 0,
	/* 110 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 120 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 200 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 240 */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 250 */
	0, 0, "AXFR", "MAILB", "MAILA", "ANY" 
};

ns_stats()
{
	time_t timenow;
	register FILE *f;
	register int i;

	if ((f = fopen(statsfile,"a")) == 0)
	{
#ifdef DEBUG
		if (debug)
			fprintf(ddt,"can't open stat file, \"%s\"\n",statsfile);
#endif
		syslog(LOG_ERR, MSGSTR(STATOPEN, "cannot open stat file, \"%s\"\n"),statsfile); /*MSG*/
		return;
	}

	time(&timenow);
	fprintf(f, "###  %s", ctime(&timenow));
	fprintf(f, "%d\ttime since boot (secs)\n", timenow - boottime);
	fprintf(f, "%d\ttime since reset (secs)\n", timenow - resettime);

	/* general statistics */
	for (i = 0; i < S_NSTATS; i++)
		fprintf(f,"%lu\t%s\n", stats[i].cnt, stats[i].description);
	/* query type statistics */
	fprintf(f, "%d\tUnknown query types\n", typestats[0]);
	for(i=1; i < T_ANY+1; i++)
		if (typestats[i])
			if (typenames[i])
                                fprintf(f, "%lu\t%s queries\n", typestats[i],
                                        typenames[i]);
                        else
                                fprintf(f, "%lu\ttype %d queries\n",
                                        typestats[i], i);
	(void) fclose(f);
}
#endif STATS
