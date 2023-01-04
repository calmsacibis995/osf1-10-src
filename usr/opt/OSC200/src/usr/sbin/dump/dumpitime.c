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
static char	*sccsid = "@(#)$RCSfile: dumpitime.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/08/24 20:41:27 $";
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
#if !defined( lint) && !defined(_NOIDENT)

#endif

/*
 * This module contains IBM CONFIDENTIAL code. -- (IBM Confidential Restricted
 * when combined with the aggregated modules for this product) OBJECT CODE ONLY
 * SOURCE MATERIALS (C) COPYRIGHT International Business Machines Corp. 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or disclosure
 * restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * Copyright (c) 1980 Regents of the University of California. All
 * rights reserved.  The Berkeley software License Agreement specifies the
 * terms and conditions for redistribution.
 */

#include	"dump.h"

static void	readitimes();
static void	recout();
static int	getrecord();
static int	makeidate();
static int	idatesort();

static struct itime   *it_list = NULL;	/* head of the list version */

void
getitime()
{
	register struct idates *ip;
	register int	i;

#if	FDEBUG

	msg("getitime(): Looking for name %s in dump history file %s for increment %c\n",
	    disk_file_name, dump_hist_file_name, incr_num);

#endif	FDEBUG

	spcl.c_ddate = (time_t) 0;
	last_incr_num = '0';

	inititimes();

	/*
	 * find latest entry with the same name and a lower increment
	 */

	for (i = 0; i < num_idate_records; ++i)
	{
		ip = idate_array[i];

		/* if not the same name, skip it */

		if (strncmp(disk_file_name, ip->id_name, sizeof(ip->id_name)) != 0)
		{
			continue;
		}

		/* if increment number is not less, skip it */

		if (ip->id_incno >= incr_num)
		{
			continue;
		}

		/* if it is not the newest, skip it */

		if (ip->id_ddate <= spcl.c_ddate)
		{
			continue;
		}

		spcl.c_ddate = ip->id_ddate;
		last_incr_num = ip->id_incno;
	}
}

void
putitime()
{
	FILE	       *hist_file_fp;
	register struct idates *it_walk;
	register int	i;

	/* throw away old idate structures and array */
	/* (don't bother free()ing them) */

	it_list = NULL;
	idate_array = NULL;
	num_idate_records = 0;

	if ((hist_file_fp = fopen(dump_hist_file_name, "r+")) == NULL)
	{
		msg(MSGSTR(CODHFRR, "Cannot open dump history file %s for reading and rewriting\n"), dump_hist_file_name);
		dump_perror("putitime(): fopen()");
		abort_dump();

		/* NOTREACHED */
	}

	/* wait until no other process has either a shared or exclusive */
	/* lock on the dump history file then get an exclusive lock on */
	/* it so we can write it */

	(void) flock((int) fileno(hist_file_fp), LOCK_EX);

	readitimes(hist_file_fp);

	/* go back to beginning of history file so we can overwrite it */

	if (fseek(hist_file_fp, 0L, 0) < 0)
	{
		msg(MSGSTR(CSDHF, "Cannot seek back to beginning of dump history file %s\n"), dump_hist_file_name);
		dump_perror("putitime(): fseek()");
		abort_dump();

		/* NOTREACHED */
	}

	spcl.c_ddate = 0;
	for (i = 0; i < num_idate_records; ++i)
	{
		it_walk = idate_array[i];

		if (strncmp(disk_file_name, it_walk->id_name, sizeof(it_walk->id_name)) != 0)
		{
			continue;
		}
		if (it_walk->id_incno != incr_num)
		{
			continue;
		}

		break;
	}

	/*
	 * construct the new upper bound; Enough room has been allocated.
	 */

	if (i == num_idate_records)
	{
		idate_array[num_idate_records] = (struct idates *) calloc(1, sizeof(struct idates));
		it_walk = idate_array[num_idate_records];
		++num_idate_records;
	}

	(void) strncpy(it_walk->id_name, disk_file_name, sizeof(it_walk->id_name));
	it_walk->id_incno = incr_num;
	it_walk->id_ddate = spcl.c_date;

	qsort(idate_array, num_idate_records, sizeof(struct idates *), idatesort);

	for (i = 0; i < num_idate_records; ++i)
	{
		it_walk = idate_array[i];

		/* throw out higher level dump information for same disk */

		if (strncmp(disk_file_name, it_walk->id_name, sizeof(it_walk->id_name)) == 0 && 
				it_walk->id_incno > incr_num)
		{
			continue;
		}

		recout(hist_file_fp, it_walk);
	}

	/* cut off the rest of the old dump history data */

	if (ftruncate(fileno(hist_file_fp), (int) ftell(hist_file_fp)) != 0)
	{
		msg(MSGSTR(CTDHF, "Cannot truncate dump history file %s\n"), dump_hist_file_name);
		dump_perror("putitime(): ftruncate()");
		abort_dump();

		/* NOTREACHED */
	}

	/* this releases the exclusive lock, too */

	(void) fclose(hist_file_fp);

	msg(MSGSTR(DUMPL, "Level %c dump on %s\n"), incr_num, prdate(spcl.c_date));
}

void
inititimes()
{
	FILE	       *hist_file_fp;

	if ( access(dump_hist_file_name, F_OK) )
		hist_file_fp = fopen(dump_hist_file_name, "a+");
	else
		hist_file_fp = fopen(dump_hist_file_name, "r");
	if (hist_file_fp == NULL)
	{
		msg(MSGSTR(CODHFR, "Cannot open dump history file %s for reading\n"), dump_hist_file_name);
		dump_perror("inititimes(): fopen()");
		abort_dump();

		/* NOTREACHED */
	}

	/* wait until no other processes have an exclusive lock on the */
	/* dump history file and then put a shared lock on it */

	(void) flock((int) fileno(hist_file_fp), LOCK_SH);

	readitimes(hist_file_fp);

	/* this releases the shared lock, too */

	(void) fclose(hist_file_fp);

	qsort(idate_array, num_idate_records, sizeof(struct idates *), idatesort);
}

static void
readitimes(hist_file_fp)
	FILE	       *hist_file_fp;
{
	register int	i;
	register struct itime *it_walk;

	for (;;)
	{
		it_walk = (struct itime *) calloc(1, sizeof(struct itime));
		if (getrecord(hist_file_fp, &(it_walk->it_value)) < 0)
		{
			break;
		}
		++num_idate_records;
		it_walk->it_next = it_list;
		it_list = it_walk;
	}

	/*
	 * arrayify the list, leaving enough room for the additional record
	 * that we may have to add to the idate structure
	 */

	idate_array = (struct idates **) calloc((unsigned) num_idate_records + 1, sizeof(struct idates *));

	for (i = 0, it_walk = it_list; i < num_idate_records; ++i, it_walk = it_walk->it_next)
	{
		idate_array[i] = &it_walk->it_value;
	}
}

static void
recout(hist_file_fp, what)
	FILE	       *hist_file_fp;
	struct idates  *what;
{
	(void) fprintf(hist_file_fp, DUMPOUTFMT, what->id_name, what->id_incno, ctime(&(what->id_ddate)));
}

static int
getrecord(hist_file_fp, idatep)
	FILE	       *hist_file_fp;
	struct idates  *idatep;
{
	char		buf[BUFSIZ];
	static int	recno = 0;

	if (fgets(buf, BUFSIZ, hist_file_fp) != buf)
	{
		return(-1);
	}

	++recno;
	if (makeidate(idatep, buf) < 0)
	{
		msg(MSGSTR(UNKIF, "Bad dump history format in %s: line %d\n"), dump_hist_file_name, recno);
	}

#if	FDEBUG

	msg("getrecord(): %s %c %s\n", idatep->id_name, idatep->id_incno, prdate(idatep->id_ddate));

#endif	FDEBUG

	return(0);
}


static int
makeidate(ip, buf)
	struct idates  *ip;
	char	       *buf;
{
	char		un_buf[256];
	char		tmp_buf[256];
	char		*cp;
	extern char	*strchr();

	strcpy(tmp_buf,buf);  /* Just in case some other C fcn uses buf */
	/* (void) sscanf(buf, DUMPINFMT, ip->id_name, &ip->id_incno, un_buf); */
	(void) sscanf(buf, "%s %c %[^\n]\n", ip->id_name, &ip->id_incno, un_buf); 
	if ((cp = strchr(tmp_buf, ' ')) != NULL )
		*cp = '\0';
	strcpy(ip->id_name,tmp_buf); /* copy true isolated fs driver name */
	ip->id_ddate = unctime(un_buf);
	if (ip->id_ddate < 0)
	{
		return(-1);
	}
	return(0);
}

static int
idatesort(p1, p2)
	register struct idates **p1, **p2;
{
	register int		diff;

	diff = strncmp((*p1)->id_name, (*p2)->id_name, sizeof((*p1)->id_name));
	if (diff != 0)
	{
		return(diff);
	}
	return((*p2)->id_ddate - (*p1)->id_ddate);
}

/* Prdate returns a pointer to a string which contains the representation */
/* of the time_t argument */

char	       *
prdate(time_to_convert)
	time_t		time_to_convert;
{
	char 		timbuf[BUFSIZ];
	extern struct tm *localtime();
	/* if phony time (zero) was input, return an appropriate string */

	if (time_to_convert == (time_t) 0)
	{
		return(MSGSTR(EPOCH, "the start of the epoch "));
	}

	/* convert time to printable form */

	(void)strftime(timbuf, BUFSIZ, "%c %Z\0", localtime(&time_to_convert));

	return(timbuf);
}
