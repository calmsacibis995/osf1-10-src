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
static char	*sccsid = "@(#)$RCSfile: iostat.c,v $ $Revision: 4.2.13.3 $ (DEC) $Date: 1993/10/14 20:24:48 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <sys/types.h>
#include <sys/file.h>
#include <sys/buf.h>
#include <sys/table.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <ctype.h>

#if defined(NLS) || defined(KJI)
#define	NLSKJI 1
#include <NLctype.h>
#include <NLchar.h>
#endif
#include <locale.h>

#ifdef MSG
#include "iostat_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) NLcatgets(catd,MS_IOSTAT,Num,Str)
#else
#define MSGSTR(Num,Str) Str
#endif

char	**dr_name;
int	*dr_select;
int	*dr_alive;
long	*dk_wpms;
int	dk_ndrive;
int	ndrives = 0;
#ifdef vax
char	*defdrives[] = { "hp0", "hp1", "hp2",  0 };
#else
char	*defdrives[] = { 0 };
#endif

#define CPUSTATES 4

struct {
	long	cp_time[CPUSTATES];
	int	dk_busy;
	long	*dk_time;
	long	*dk_wds;
	long	*dk_seek;
	long	*dk_xfer;
	long	tk_nin;
	long	tk_nout;
} s, s1;

struct tbl_sysinfo sysinfo;
struct tbl_dkinfo dkinfo;
struct tbl_ttyinfo ttyinfo;

int	mf;
int	hz;
int	phz;
double	etime;
int	tohdr = 1;
void	printhdr();

main(argc, argv)
	char *argv[];
{
	extern char *optarg;
        extern int optind;
	register  int i;
	int iter, ndrives, ch;
	double f1, f2;
	long t;
	char *arg, **cp, name[6], buf[BUFSIZ];

        (void) setlocale (LC_ALL,"");
#ifdef MSG
        catd = NLcatopen((char *)MF_IOSTAT, NL_CAT_LOCALE);
#endif

        while ((ch = getopt(argc, argv, "?")) != EOF)
                switch ((char)ch) {
                case '?':
                default:
                        fprintf(stderr, MSGSTR(USAGE, "usage: iostat [drives] [interval [count]]\n"));
                        exit(1);
                        break;
                }

	iter = 0;
	for (argc--, argv++; argc > 0 && argv[0][0] == '-'; argc--, argv++)
		;

        if (table(TBL_DKINFO, 0, &dkinfo, 1, sizeof(dkinfo)) != 1) {
		fprintf(stderr, MSGSTR(NO_DISKS, "iostat: no disks\n"));
                exit(1);
        }
        
        dk_ndrive = dkinfo.di_ndrive;
	if (dk_ndrive == 0) {
		fprintf(stderr, MSGSTR(NO_DISKS, "iostat: no disks\n"));
		exit(1);
	}
	dr_select = (int *)calloc(dk_ndrive, sizeof (int));
	dr_alive = (int *)calloc(dk_ndrive, sizeof (int));
	dr_name = (char **)calloc(dk_ndrive, sizeof (char *));
	dk_wpms = (long *)calloc(dk_ndrive, sizeof (long));
#define	allocate(e, t) \
    s./**/e = (t *)calloc(dk_ndrive, sizeof (t)); \
    s1./**/e = (t *)calloc(dk_ndrive, sizeof (t));
	allocate(dk_time, long);
	allocate(dk_wds, long);
	allocate(dk_seek, long);
	allocate(dk_xfer, long);
	for (arg = buf, i = 0; i < dk_ndrive; i++) {
		dr_name[i] = arg;
		sprintf(dr_name[i], "dk%03d", i);
		arg += strlen(dr_name[i]) + 1;
	}

        if (table(TBL_SYSINFO, 0, &sysinfo, 1, sizeof(sysinfo)) != 1) {
                fprintf(stderr, MSGSTR(NO_SYSINFO, "iostat: cannot get system info.\n"));
                exit(1);
        }
        hz = sysinfo.si_hz;
        phz = sysinfo.si_phz;
	if (phz)
		hz = phz;

        read_names();

	/*
	 * Choose drives to be displayed.  Priority
	 * goes to (in order) drives supplied as arguments,
	 * default drives.  If everything isn't filled
	 * in and there are drives not taken care of,
	 * display the first few that fit.
	 */
	ndrives = 0;
	while (argc > 0 && !isdigit(argv[0][0])) {
		for (i = 0; i < dk_ndrive; i++) {
			if (strncmp(dr_name[i], argv[0], 5))
				continue;
			dr_select[i] = 1;
			ndrives++;
		}
		argc--, argv++;
	}
	for (i = 0; i < dk_ndrive && ndrives < 4; i++) {
		if (dr_select[i] || dk_wpms[i] == 0)
			continue;
		for (cp = defdrives; *cp; cp++)
			if (strcmp(dr_name[i], *cp) == 0) {
				dr_select[i] = 1;
				ndrives++;
				break;
			}
	}
	for (i = 0; i < dk_ndrive && ndrives < 4; i++) {
		if (dr_select[i])
			continue;
		dr_select[i] = 1;
		ndrives++;
	}
	if (argc > 1)
		iter = atoi(argv[1]);
	(void) signal(SIGCONT, printhdr);
loop:
	if (--tohdr == 0)
		printhdr();

        read_stats();
        
	for (i = 0; i < dk_ndrive; i++) {
		if (!dr_select[i])
			continue;
#define X(fld)	t = s.fld[i]; s.fld[i] -= s1.fld[i]; s1.fld[i] = t
		X(dk_xfer); X(dk_seek); X(dk_wds); X(dk_time);
	}
	t = s.tk_nin; s.tk_nin -= s1.tk_nin; s1.tk_nin = t;
	t = s.tk_nout; s.tk_nout -= s1.tk_nout; s1.tk_nout = t;
	etime = 0;
	for(i=0; i<CPUSTATES; i++) {
		X(cp_time);
		etime += s.cp_time[i];
	}
	if (etime == 0.0)
		etime = 1.0;
	etime /= (float) hz;
	printf("%4.0f%5.0f", s.tk_nin/etime, s.tk_nout/etime);
	for (i=0; i<dk_ndrive; i++)
		if (dr_select[i] && dr_alive[i])
			stats(i);
	for (i=0; i<CPUSTATES; i++)
		stat1(i);
	printf("\n");
	fflush(stdout);
contin:
	if (--iter && argc > 0) {
		sleep(atoi(argv[0]));
		goto loop;
	}
}

void printhdr()
{
	register int i;

	printf("      tty");
	for (i = 0; i < dk_ndrive; i++)
		if (dr_select[i] && dr_alive[i])
			printf("   %s%5.5s ",
                               (dk_wpms[i] != 0)?"      ":"", dr_name[i]);
	printf("    %scpu\n", (dk_wpms[i] != 0)?"      ":"");
	printf(" tin tout");
	for (i = 0; i < dk_ndrive; i++)
                if (dr_select[i] && dr_alive[i])
			printf(" bps tps%s ", (dk_wpms[i] != 0)?" msps":"");
        printf(" us ni sy id\n");
	tohdr = 19;
}

stats(dn)
{
	register i;
	double atime, words, xtime, itime;

	atime = s.dk_time[dn];
	atime /= (float) hz;
	words = s.dk_wds[dn]*32.0;	/* number of words transferred */
	printf("%4.0f", words/512/etime);
	printf("%4.0f", s.dk_xfer[dn]/etime);
	if (dk_wpms[dn] != 0) {
                xtime = words/dk_wpms[dn];	/* transfer time */
                itime = atime - xtime;		/* time not transferring */
                if (xtime < 0)
                        itime += xtime, xtime = 0;
                if (itime < 0)
                        xtime += itime, itime = 0;
                printf("%5.1f ",
                       s.dk_seek[dn] ? itime*1000./s.dk_seek[dn] : 0.0);
        }
        else
                putchar(' ');
}

stat1(o)
{
	register i;
	double time;

	time = 0;
	for(i=0; i<CPUSTATES; i++)
		time += s.cp_time[i];
	if (time == 0.0)
		time = 1.0;
	printf("%3.0f", 100.*s.cp_time[o]/time);
}

read_stats()
{
        int i;

        for (i = 0; i < dk_ndrive; i++)
                if (dr_alive[i] && (table(TBL_DKINFO, i, &dkinfo, 1, sizeof(dkinfo)) == 1)) {
                        s.dk_busy = dkinfo.di_busy;
                        s.dk_time[i] = dkinfo.di_time;
                        s.dk_xfer[i] = dkinfo.di_xfer;
                        s.dk_wds[i] = dkinfo.di_wds;
                        s.dk_seek[i] = dkinfo.di_seek;
                }

        if (table(TBL_SYSINFO, 0, &sysinfo, 1, sizeof(sysinfo)) != 1) {
                fprintf(stderr, MSGSTR(NO_SYSINFO, "iostat: cannot get system info.\n"));
                exit(1);
        }
        s.cp_time[0] = sysinfo.si_user;
        s.cp_time[1] = sysinfo.si_nice;
        s.cp_time[2] = sysinfo.si_sys;
        s.cp_time[3] = sysinfo.si_idle + sysinfo.wait;
        
        if (table(TBL_TTYINFO, 0, &ttyinfo, 1, sizeof(ttyinfo)) != 1) {
                fprintf(stderr, MSGSTR(NO_TTYINFO, "iostat: cannot get tty info.\n"));
                exit(1);
        }
        s.tk_nin = ttyinfo.ti_nin;
        s.tk_nout = ttyinfo.ti_nout;
}

read_names()
{
        int i;

        for (i = 0; i < dk_ndrive; i++)
                if (table(TBL_DKINFO, i, &dkinfo, 1, sizeof(dkinfo)) == 1) {
                        dr_alive[i] = 1;
                        dk_wpms[i] = dkinfo.di_wpms;
                        sprintf(dr_name[i], "%c%c%d",
                                tolower(dkinfo.di_name[0]),
                                tolower(dkinfo.di_name[1]),
                                dkinfo.di_unit);
#if defined(alpha) || defined(mips)
			/* the RRD drives are actual rz types not rr types */
			if(dr_name[i][1] == 'r') dr_name[i][1]='z';
#endif
                }
}
