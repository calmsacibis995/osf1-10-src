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
static char	*sccsid = "@(#)$RCSfile: vmstat.c,v $ $Revision: 4.2.8.6 $ (DEC) $Date: 1993/11/12 20:55:36 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif

#include <stdio.h>
#include <ctype.h>
#include <mach.h>
#include <sys/param.h>
#include <sys/table.h>
#include <sys/types.h>
#include <machine/machparam.h>
#include <mach/message.h>
#include <mach/notify.h>
#include <sys/malloc.h>

extern	char	*malloc();

#include <locale.h>
#include "vmstat_msg.h"
nl_catd catd;
#define MSGSTR(Num,Str) catgets(catd, MS_VMSTAT, Num, Str)

#define	fatal(x,str)	{ fprintf(stderr, "%s: %s\n", progname, str); exit(x); }
#define	printsum(str,x)		printf("%10d   %-25s\n", x, str);
#define	printlsum(str,x)	printf("%10ld   %-25s\n", x, str);
#define pgtok(x) 	((x) * NBPG / 1024)
#ifdef	__STDC__
#define	delta(x, y)	(x.y - x##_last.y)
#else
#define	delta(x, y)	(x.y - x/**/_last.y)
#endif


struct vm_statistics	vmstat, 
			vmstat_last;

struct tbl_sysinfo 	sysinfo, 
			sysinfo_last;

struct tbl_intr 	instat, 
			instat_last;

long			boottime,
			deltaseconds;

double			cpu_user,
			cpu_sys,
			cpu_idle,
			cpu_total; 

int			proc_runqueue,
			proc_waiting,
			proc_unintr,
			proc_stopped;

#define	MAXLINES	20
int	lines;
int	vmpercent;
char *	progname;

void	usage();

void	do_forks();
void	do_tabular();
void	do_sum();

void	fetch_proc();
void	fetch_sysinfo();
void	fetch_vmstats();
void	fetch_instat();

main(argc, argv)
	int	argc;
	char	*argv[];
{
        extern char 	       *optarg;
        extern int      	optind;
        register int    	c;
	int			count;
	int			nocount;
	int			interval;
	long			nproc;
	time_t			now;
	struct tbl_procinfo    *procTable;

	progname = argv[0];
	count = 0;
	nocount = 1;
	interval = 0;

	/*
	** Set buffering to line mode.  This will ensure that each output line
	** makes it to the file (buffer cache) even when the task is interrupted
	** (control/c).
	*/
	setlinebuf(stdout);

        if (getuid() && geteuid())
		fatal(1, MSGSTR(EPERM, "missing privilege to run"));

	/* Allocate all proc table memory once, now */
	nproc = table(TBL_PROCINFO, 0, (char *)0, 32767, 0);
	procTable = (struct tbl_procinfo *)malloc(nproc *
						  sizeof(struct tbl_procinfo));

	if (procTable == 0)
		fatal(1, MSGSTR(ENOMEM,
				"insufficient memory for process table"));

        while ((c = getopt(argc, argv, "scifM")) != EOF) {
                switch (c) {
                case 'c':
                        break;
                case 'f':
			fetch_instat();
			do_forks();
			exit(0);
                case 'i':
                        break;
                case 's':
			fetch_vmstats();
			fetch_sysinfo();
			fetch_proc(nproc, procTable);
			fetch_instat();
			do_sum();
			exit(0);
                case 'M':
			mallocstats();
			exit(0);
                default:
                        usage();
                        exit(1);
                }
        }
        if (optind < argc)
                interval = atoi(argv[optind++]);
        if (optind < argc) {
                count = atoi(argv[optind++]);
		nocount = 0;
	}

	/* We must disable the notification port.  Otherwise, if vmstat
	 * is indefinitely looping, NOTIFY_PORT_DELETED messages will
	 * pile up until all system memory is consumed!
	 */
	if (task_set_notify_port(task_self(), (port_t)0) != KERN_SUCCESS) {
		fatal(1, MSGSTR(NO_NOTIFY, "could not disable notify port"));
	}

        if (table(TBL_SYSINFO, 0, &sysinfo, 1, sizeof(sysinfo)) != 1) 
		fatal(1, MSGSTR(NO_SYSINFO, "cannot get system info"));
        time(&now);
        boottime 	= now - sysinfo.si_boottime;
        deltaseconds 	= boottime;

	if (interval == 0) {
		fetch_vmstats();
		fetch_sysinfo();
		fetch_proc(nproc, procTable);
		fetch_instat();
		do_tabular();
		exit(0);
	} 

	lines = 0;
	while (nocount || count--) {
		int i;

		fetch_vmstats();
		fetch_sysinfo();
		fetch_proc(nproc, procTable);
		fetch_instat();
		do_tabular();

		sleep(interval);
		deltaseconds = interval;

		if (++lines > MAXLINES)
			lines = 0;
	}
}


void
usage()
{
	fprintf(stderr, 
		MSGSTR(USAGE, "usage: vmstat [-fisM] [interval [count]]\n"));
	exit(1);
}


#define	TABHDR \
"  procs    memory         pages                          intr        cpu      \n\
  r  w  u  act  free wire fault cow zero react pin pout  in  sy  cs  us  sy  id\n"

void
do_tabular()
{
	if (lines == 0) {
		printf(MSGSTR(RPT_VMSTATS, 
			"Virtual Memory Statistics: (pagesize = %d)\n"),
			vmstat.pagesize);
		printf(TABHDR);
		lines = 0;
	}

        printf("%3d%3d%3d", 
		proc_runqueue,
		proc_waiting,
		proc_unintr);

        /*
        ** Display active_count, free_count, and wire_count.
        ** (6 columns, 5 columns, and 5 columns each.)
        */
        if (((long)vmstat.active_count + (long)vmstat.inactive_count) < 10000)
          printf(" %5ld", (long)vmstat.active_count + (long)vmstat.inactive_count);
        else
          if (((long)vmstat.active_count + (long)vmstat.inactive_count) < 1000000)
            printf(" %4ldK", ((long)vmstat.active_count + (long)vmstat.inactive_count)/1000);
          else
            printf(" %4ldM", ((long)vmstat.active_count + (long)vmstat.inactive_count)/1000000);

        if ((long) vmstat.free_count < 10000)
          printf(" %4ld",  (long) vmstat.free_count);
        else
          if ((long) vmstat.free_count < 1000000)
            printf(" %3ldK", ((long) vmstat.free_count)/1000);
          else
            printf(" %3ldM", ((long) vmstat.free_count)/1000000);

        if ((long) vmstat.wire_count < 10000)
          printf(" %4ld",  (long) vmstat.wire_count);
        else
          if ((long) vmstat.wire_count < 1000000)
            printf(" %3ldK", ((long) vmstat.wire_count)/1000);
          else
            printf(" %3ldM", ((long) vmstat.wire_count)/1000000);

        /*
        ** Display DELTA values of faults, cow_faults, zero_fill_count,
        ** reactivations, pageins and pageouts, 5 columns each.
        ** NOTE: inactive count is not reported.
        */
        if ((long) delta(vmstat, faults) < 10000)
          printf(" %4ld",  (long) delta(vmstat, faults));
        else
          if (delta(vmstat, faults) < 1000000)
            printf(" %3ldK", ((long) delta(vmstat, faults))/1000);
          else
            printf(" %3ldM", ((long) delta(vmstat, faults))/1000000);

        if ((long) delta(vmstat, cow_faults) < 10000)
          printf(" %4ld",  (long) delta(vmstat, cow_faults));
        else
          if ((long) delta(vmstat, cow_faults) < 1000000)
            printf(" %3ldK", ((long) delta(vmstat, cow_faults))/1000);
          else
            printf(" %3ldM", ((long) delta(vmstat, cow_faults))/1000000);

        if ((long) delta(vmstat, zero_fill_count) < 10000)
          printf(" %4ld",  (long) delta(vmstat, zero_fill_count));
        else
          if ((long) delta(vmstat, zero_fill_count) < 1000000)
            printf(" %3ldK", ((long) delta(vmstat, zero_fill_count))/1000);
          else
            printf(" %3ldM", ((long) delta(vmstat, zero_fill_count))/1000000);

        if ((long) delta(vmstat, reactivations) < 10000)
          printf(" %4ld",  (long) delta(vmstat, reactivations));
        else
          if ((long) delta(vmstat, reactivations) < 1000000)
            printf(" %3ldK", ((long) delta(vmstat, reactivations))/1000);
          else
            printf(" %3ldM", ((long) delta(vmstat, reactivations))/1000000);

        if ((long) delta(vmstat, pageins) < 10000)
          printf(" %4ld",  (long) delta(vmstat, pageins));
        else
          if ((long) delta(vmstat, pageins) < 1000000)
            printf(" %3ldK", ((long) delta(vmstat, pageins))/1000);
          else
            printf(" %3ldM", ((long) delta(vmstat, pageins))/1000000);

        if ((long) delta(vmstat, pageouts) < 10000)
          printf(" %4ld",  (long) delta(vmstat, pageouts));
        else
          if ((long) delta(vmstat, pageouts) < 1000000)
            printf(" %3ldK", ((long) delta(vmstat, pageouts))/1000);
          else
            printf(" %3ldM", ((long) delta(vmstat, pageouts))/1000000);

        /*
        ** Display devintr, syscalls and context-switches.
        ** (4 columns each.)
        */
        if ((long) delta(instat, in_devintr)/deltaseconds < 1000)
          printf(" %3ld",   (long) delta(instat, in_devintr)/deltaseconds);
        else
          if ((long) delta(instat, in_devintr)/deltaseconds < 1000000)
            printf("%3ldK", ((long) delta(instat, in_devintr))/deltaseconds/1000);
          else
            printf("%3ldM", ((long) delta(instat, in_devintr))/deltaseconds/1000000);

        if ((long) delta(instat, in_syscalls)/deltaseconds < 1000)
          printf(" %3ld",   (long) delta(instat, in_syscalls)/deltaseconds);
        else
          if ((long) delta(instat, in_syscalls)/deltaseconds < 1000000)
            printf("%3ldK", ((long) delta(instat, in_syscalls))/deltaseconds/1000);
          else
            printf("%3ldM", ((long) delta(instat, in_syscalls))/deltaseconds/1000000);

        if ((long) delta(instat, in_context)/deltaseconds < 1000)
          printf(" %3ld",   (long) delta(instat, in_context)/deltaseconds);
        else
          if ((long) delta(instat, in_context)/deltaseconds < 1000000)
            printf("%3ldK", ((long) delta(instat, in_context))/deltaseconds/1000);
          else
            printf("%3ldM", ((long) delta(instat, in_context))/deltaseconds/1000000);

        /*
        ** Display CPU percentages.
        */
	cpu_user = delta(sysinfo,si_user) + delta(sysinfo, si_nice);
	cpu_sys = delta(sysinfo,si_sys);
	cpu_idle = delta(sysinfo,si_idle) + delta(sysinfo,wait);
	if ((cpu_total=cpu_user +cpu_sys +cpu_idle) == 0.)
		cpu_total = 1;

	printf("%4.0f%4.0f%4.0f\n",
		(float) cpu_user * 100. / cpu_total,
		(float) cpu_sys * 100. / cpu_total,
		(float) cpu_idle * 100. / cpu_total);
}


void
do_forks()
{
	printsum(MSGSTR(RPT_FORKS, "forks"),
			instat.in_forks);
	printsum(MSGSTR(RPT_VFORKS, "vforks"),
			instat.in_vforks);
}


void
do_sum()
{
	printf(MSGSTR(RPT_VMSTATS, 
		"Virtual Memory Statistics: (pagesize = %d)\n"),
		vmstat.pagesize);

	printlsum(MSGSTR(RPT_ACTIVE, "active pages"),
		vmstat.active_count);

	printlsum(MSGSTR(RPT_INACTIVE, "inactive pages"),
		vmstat.inactive_count);

	printlsum(MSGSTR(RPT_FREEPAGES, "free pages"), 
		vmstat.free_count);

	printlsum(MSGSTR(RPT_WIRED, "wired pages"),
		vmstat.wire_count);
		
	printlsum(MSGSTR(RPT_FAULTS, "virtual memory page faults"),
		vmstat.faults);

	printlsum(MSGSTR(RPT_COW, "copy-on-write page faults"),
		vmstat.cow_faults);
		
	printlsum(MSGSTR(RPT_ZEROFILL, "zero fill page faults"),
		vmstat.zero_fill_count);
		
	printlsum(MSGSTR(RPT_REACTIVE, "reattaches from reclaim list"),
		vmstat.reactivations);
		
	printlsum(MSGSTR(RPT_PAGEINS, "pages paged in"),
		vmstat.pageins);
		
	printlsum(MSGSTR(RPT_PAGEOUTS, "pages paged out"),
		vmstat.pageouts);
		
	printlsum(MSGSTR(RPT_CONTEXT, "task and thread context switches"),
		instat.in_context);

	printlsum(MSGSTR(RPT_DEVINTR, "device interrupts"),
		instat.in_devintr);

	printlsum(MSGSTR(RPT_SYSCALL, "system calls"),
		instat.in_syscalls);
}

void
fetch_vmstats()
{
	static int	initialized = 0;

	if (!initialized++)
 		bzero(&vmstat, sizeof(vmstat));

	vmstat_last = vmstat;

	if (vm_statistics(current_task(), &vmstat) != KERN_SUCCESS)
                fatal(2, MSGSTR(NO_VMSTATS, "cannot get vm statistics info"));

}

void
fetch_sysinfo()
{
	static int	initialized = 0;

	if (!initialized++)
		bzero(&sysinfo, sizeof(sysinfo));

	sysinfo_last = sysinfo;

        if (table(TBL_SYSINFO, 0, &sysinfo, 1, sizeof(sysinfo)) != 1) 
		fatal(1, MSGSTR(NO_SYSINFO, "cannot get system info"));
}



void
fetch_instat()
{
	static int	initialized = 0;

	if (!initialized++)
		bzero(&instat, sizeof(instat));

	instat_last = instat;

        if (table(TBL_INTR, 0, &instat, 1, sizeof(instat)) != 1) 
		fatal(1, MSGSTR(NO_INTR, "cannot get interrupt info"));
}



void
fetch_proc(nproc, procTable)
long nproc;
struct tbl_procinfo *procTable;
{
	int 	i;
	int	j;
	int	rc;
	task_t			task;
	thread_array_t		thread_table;
	unsigned int		table_size;
	unsigned int		cnt;
	thread_basic_info_t     thi;
	thread_basic_info_data_t thi_data;

	proc_runqueue = 0;
	proc_waiting = 0;
	proc_unintr = 0;
	proc_stopped = 0;

	table(TBL_PROCINFO, 0, (char *)procTable, nproc,
	      sizeof(struct tbl_procinfo));
	for (i=0; i < nproc; i++) {

		if (procTable[i].pi_status != PI_ACTIVE)  
			continue;

		rc = task_by_unix_pid(task_self(), procTable[i].pi_pid, &task);
		if (rc != KERN_SUCCESS)
			continue;

		thi = &thi_data;
		if (task_threads(task, &thread_table, &table_size) !=
			KERN_SUCCESS) continue;
		for (j=0; j < table_size; j++) {
			cnt = THREAD_BASIC_INFO_COUNT;
			rc = thread_info(thread_table[j], THREAD_BASIC_INFO,
				(thread_info_t)thi, &cnt);
			if (rc != KERN_SUCCESS) 
				continue;
		
			switch (thi->run_state) {
						/* Runnable / running */
			case TH_STATE_RUNNING:	
				proc_runqueue++;
				break;
						/* Sleeping intr-able */
			case TH_STATE_WAITING:      
				proc_waiting++;
				break;
						/* Sleeping unintr-able */
			case TH_STATE_UNINTERRUPTIBLE:
				proc_unintr++;
				break;
				 		/* Stopped */
			case TH_STATE_STOPPED:
				proc_stopped++;
				break;
						/* Halted */
			case TH_STATE_HALTED:	
			default:			/* ??? */
				break;
			}
		}

		vm_deallocate(task_self(), (vm_address_t)thread_table,
			      table_size * sizeof(thread_t));
	}
}

mallocstats()
 {
	struct kmembuckets bucket[MINBUCKET + 16];
	struct kmemtypes kmemtypes[M_LAST];
	char kmemnames[M_LAST][KMEMNAMSZ];
	long totalloc=0;
	long totfree=0;
	int i,nout;

        if(table(TBL_MALLOCBUCKETS, 0, bucket, MINBUCKET + 16,
				 sizeof(struct kmembuckets)) != MINBUCKET + 16)
		fatal(1, MSGSTR(NO_MALLOCBUCKTES, "cannot get malloc buckets info"));

        printf(MSGSTR(RPT_MEMUSE1,"\nMemory usage by bucket:\n\n"));
	printf(MSGSTR(RPT_MEMUSE2, "bucket#  element_size  elements_in_use  elements_free  bytes_in_use\n")); 
	for(i=MINBUCKET; i < MINBUCKET + 16; i++) {
		printf("   %2d        %6d        %6d          %6d      %10d\n",  
				i, bucket[i].kb_size,
			 	bucket[i].kb_total - bucket[i].kb_totalfree,
			 	bucket[i].kb_totalfree,
			 	(bucket[i].kb_total - bucket[i].kb_totalfree)*bucket[i].kb_size);
		totalloc += bucket[i].kb_size * bucket[i].kb_total;
		totfree += bucket[i].kb_size * bucket[i].kb_totalfree;
	}
        printf(MSGSTR(RPT_MEMUSE3,
		"\nTotal memory being used from buckets =%d bytes\n"),
		   totalloc-totfree);
        printf(MSGSTR(RPT_MEMUSE4,
		"Total free memory in buckets =%d bytes\n\n\n"), totfree );

        if(table(TBL_MALLOCTYPES, 0, kmemtypes, M_LAST,
				 sizeof(struct kmemtypes)) != M_LAST)
		fatal(1, MSGSTR(NO_MALLOCTYPES, "cannot get malloc kmemtypes info"));

        if(table(TBL_MALLOCNAMES, 0, kmemnames, M_LAST, KMEMNAMSZ) != M_LAST)
		fatal(1, MSGSTR(NO_MALLOCNAMES, "cannot get memory type names"));

        printf(MSGSTR(RPT_MEMUSE5,
	      "Memory usage by type: Type and Number of bytes being used\n\n"));
	i=0;
	nout=0;
	while(1) {
	    if(kmemtypes[i].kt_memuse == 0) {
		if(++i == M_LAST) break; 
	    } 
	    else  {
		printf("%-12s=%-10d", kmemnames[i], kmemtypes[i].kt_memuse);
		nout++;
                /* if 3th on line, put out newline */
                if(nout%3 == 0) 
		    printf("\n");
		if(++i == M_LAST) break; 
	    }
	}
	if(nout%3 != 0)
	   printf("\n");
}
