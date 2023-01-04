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
static char *sccsid  =  "@(#)$RCSfile: memx.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/11/23 20:12:19 $";
#endif

/*
 */

/************************************************************************
 *			Modification History
 * 6/25/91 -- Mike Ardehali
 *	Ported to OSF tin
 *
 * 7/15/88 -- prs
 *      Added char string DR_MEMX to allow proper use with -o option.
 *
 ************************************************************************/
/*
 *
 *	MEMX.C	--  This routine will set up the memory test, and spawn the
 *		    processes that actually perform the memory exercising
 *
 */
#include <stdio.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/syslog.h>
#include <nlist.h>
#include <string.h>
#include "diag.h"

#ifdef ALPHA_VARIABLE_PAGE_SIZE
#undef PGSHIFT
#define PGSHIFT 13
#endif

#define MODULE		"memx"			/* name of module */
#define MINUTE		60			/* minute in 1 sec counts */
#define HOUR		60*MINUTE
#define SECOND		1 			/* one second */
#define ZZZZZZ		HOUR
#define EXR_BG_NAME	"memxr"
#define EXR_SH_NAME	"shmx"

struct nlist nl[] =
{
	{"_maxmem"},
	{"_physmem"},
	{""},
};

char *help[] =
{
	"\n\n(memx) - Generic memory exerciser\n",
	"\n",
	"usage:\n",
	"\tmemx [-h] [-s] [-ofile] [-m#] [-p#] [-t#]\n",
	"\n",
	"-h\t(optional) Print this help message\n",
	"-s\t(optional) Disable shared memory testing\n",
	"-ofile\t(optional) Save output diagnostics in file\n",
	"-m#\t(optional) Memory size specified (# > 4095 bytes)\n",
	"-p#\t(optional) Number of processes specified (1-20 default 10)\n",
	"-t#\t(optional) Run time in minutes (default: run forever until cntl-c)\n",
	"\n",
	"examples:\n",
	"\tmemx -p10\n",
	"\t{ Run 10 memx processes forever }\n",
	"\tmemx -t180 &\n",
	"\t{ Run 20(default) memx processes for 180 min. in background }\n",
	"\n",
	"",
};

int pid[DM_MAXPROC];			/* array of process id for memxr
					   routines spawned */
int pid2[DM_MAXPROC];
int status[DM_MAXPROC];

int nmemxr;				/* number of nmemxr routines spawned */
char errbuf[512];			/* buffer for error message */

int timedelta;

char DR_MEMX[] = "#LOG_MEMX_1";		/* Logfile name */

main (argc,argv)
int argc;
char **argv;
{
register i,j;
long maxmem,physmem,rb,mem;
long memsize;				/* memory size for each test */
long exmemsize;				/* extra size for one test */
long smemsize;				/* memory size for shmx test */
char cmemsize[10],ctestnum[5];
char time[10],clogfd[5];
char csmemsize[10];
char smemtime[10];
int maxproc;
int numproc = 0;
int sig;
int noshmxflg = 0;
void mem_clean();
int c;
char exr_bg_filename[PATH_MAX];
char exr_sh_filename[PATH_MAX];
char *exr_pathname;

	/* set up kill signal */
	signal(SIGINT,mem_clean);
	signal(SIGTERM,mem_clean);

	/* handle input args */
	maxproc = DM_MAXPROC;
	memsize = 0;
	exmemsize = 0;
	while ((c = getopt(argc, argv, "m:p:t:o:hs")) != EOF) /* paw-9148 */
	{
		switch (c)
		{
		case 'm':
			memsize = atoi(optarg);
			if (memsize < DM_MINMEM)
			{
				printf("memx: Memory size must be at least %d bytes\n",DM_MINMEM);
				exit(0);
			}
			break;
		case 'p':
			maxproc = atoi(optarg);
			if (maxproc < 1)
				maxproc = 1;
			if (maxproc > DM_MAXPROC)
				maxproc = DM_MAXPROC;
			break;
		case 't':
			timedelta = atoi(optarg);
			break;
		case 'o':	/* save output into file */
			strcpy(filename, optarg);
			break;
		case 'h':
			for (i = 0; *help[i]; i++)
				printf("%s",help[i]);
			exit(0);
		case 's':
			noshmxflg = 1;
			break;
		default:
			for (i = 0; *help[i]; i++)
				printf("%s",help[i]);
			exit(0);
		}

	}

	/* open log file */
	if (report(DR_OPEN,MODULE,DR_MEMX))
	{
		fprintf(stderr,"%s: Can not start report generator, test aborted\n", MODULE);
		exit(0);
	}

	/* 
	 *  The following section looks into kernel memory and retrieves
	 *  the usable memory sizes
	 */
	nlist("/vmunix",nl);
	for (i = 0; i < 2; i++)
		if (nl[i].n_type == N_UNDF)
		{
			printf("nl[%d] not accessed\n",i);
			mem = -1;
		}
	if (mem == -1)
		exit(0);
	if ((mem = open("/dev/kmem",0)) < 0)
	{
		sprintf(errbuf,"Could not open memory; %s\n",
			sys_errlist[errno]);
		report(DR_WRITE,MODULE,errbuf);
		exit(0);
	}
	if (lseek(mem,nl[0].n_value,L_SET) == -1)
	{
		sprintf(errbuf,"seek on nl[0] failed; %s\n",
			sys_errlist[errno]);
		report(DR_WRITE,MODULE,errbuf);
		exit(0);
	}
	if ((rb = read(mem,(char *) &maxmem,sizeof(maxmem))) != sizeof(maxmem))
	{
		sprintf(errbuf,"read on nl[0] failed; %s; rb = %d; request = %d",sys_errlist[errno],rb,sizeof(maxmem));
		report(DR_WRITE,MODULE,errbuf);
		exit(0);
	}
	if (lseek(mem,nl[1].n_value,L_SET) == -1)
	{
		sprintf(errbuf,"seek on nl[1] failed; %s\n",
			sys_errlist[errno]);
		report(DR_WRITE,MODULE,errbuf);
		exit(0);
	}
	if ((rb = read(mem,(char *)&physmem,sizeof(physmem))) != sizeof(physmem))
	{
		sprintf(errbuf,"read on nl[1] failed; %s; rb = %d; request = %d",sys_errlist[errno], rb,sizeof(physmem));
		report(DR_WRITE,MODULE,errbuf);
		exit(0);
	}

	/*
	 * calculate the number of processes to be spawned and the memory
	 * size for each to test
	 */

	smemsize = memsize;
	if (memsize == 0) {
		memsize = ctob(maxmem/DM_MAXPROC);
		exmemsize = ctob(maxmem%DM_MAXPROC);
		smemsize = memsize/3;
	}

	/* fork and exec memxr processes */
	sprintf(cmemsize,"%d",memsize);
	sprintf(clogfd,"%d",logfd);
	sprintf(time,"%d",timedelta);

	/* These allow exerciser to be called from any directory even */
	/* if user changes its name */ 
	if (!strcpy(exr_bg_filename, argv[0]))
		*exr_bg_filename = '\0';
	if (exr_pathname = rindex (exr_bg_filename, '/'))
		exr_pathname++;
	else
		exr_pathname = exr_bg_filename;
	strcpy (exr_pathname, EXR_BG_NAME);	

	if (!strcpy(exr_sh_filename, argv[0]))
		*exr_sh_filename = '\0';
	exr_pathname = rindex (exr_sh_filename, '/');
	if (exr_pathname)
		exr_pathname++;
	else
		exr_pathname = exr_sh_filename;
	strcpy (exr_pathname, EXR_SH_NAME);	

	for (i = 0; i < maxproc; i++) {
		if ((pid[i] = fork()) == 0) {
			sprintf(ctestnum,"%d",i + 1);
			if ((i == 0) && (noshmxflg == 0)) {
			    sprintf(csmemsize,"-m%d",smemsize);
			    sprintf(smemtime,"-t%d",timedelta);
			    if (execl(exr_sh_filename, exr_sh_filename,csmemsize,smemtime,0) < 0) {
				sprintf(errbuf,"Could not execl shmx %s",
				        sys_errlist[errno]);
				report(DR_WRITE,MODULE,errbuf);
			    }
			    exit(0);
			}
			if (i == DM_MAXPROC - 1)
				sprintf(cmemsize,"%d",memsize + exmemsize);
			if (execl(exr_bg_filename,exr_bg_filename,cmemsize,ctestnum,clogfd,time,0) < 0) {
				sprintf(errbuf,"Could not execl memxr%d %s",
				        i+1,sys_errlist[errno]);
				report(DR_WRITE,MODULE,errbuf);
			}
			exit(0);
		}
		if ((i == 0) && (noshmxflg == 0) && (pid[i] == -1)) {
			sprintf(errbuf,"Could not fork shmx %s",
				sys_errlist[errno]);
			report(DR_WRITE,MODULE,errbuf);
			continue;
		}
		if (pid[i] == -1) {
			sprintf(errbuf,"Could not fork memxr%d %s",
				i+1,sys_errlist[errno]);
			report(DR_WRITE,MODULE,errbuf);
			continue;
		}
		if ((i == 0) && (noshmxflg == 0)) {
			sprintf(csmemsize,"%d",smemsize);
			sprintf(errbuf,"Shared Memory Exerciser Process pid %d Started; memory size %s bytes\n",pid[i],csmemsize);
			report(DR_WRITE,MODULE,errbuf);
			numproc++;
			continue;
		}
		if (i == DM_MAXPROC - 1)
			sprintf(cmemsize,"%d",memsize + exmemsize);
		sprintf(errbuf,"Exerciser Process memxr%d pid %d Started; memory size %s bytes\n",i+1,pid[i],cmemsize);
		report(DR_WRITE,MODULE,errbuf);
		numproc++;
	}
	syslog(LOG_INFO, "Started Memory Exerciser"); /* instead of UERF */

	for(i = 0; i < numproc; i++)
		pid2[i] = wait(&status[i]);


	for (i = 0; i < numproc; i++) {
		if (pid2[i] != -1) {
			
			for (j = 0; pid2[i] != pid[j] && j < numproc; j++) ;
                	if (WIFSIGNALED(status[i]))
                        	sig = WTERMSIG(status[i]);
                	else
                        	sig =0;

			if ((pid2[i] == pid[0]) && (noshmxflg == 0))
		    		sprintf(errbuf,"Process Termination shmx pid %d Status %d %s",pid2[i],sig,sys_siglist[sig]);
			else
		    		sprintf(errbuf,"Process Termination memxr%d pid %d Status %d %s",j+1,pid2[i],sig,sys_siglist[sig]);
			report(DR_WRITE,MODULE,errbuf);
		}
	}
	syslog(LOG_INFO, "Started Memory Exerciser"); /* instead of UERF */
		
		report(DR_CLOSE,0,DR_MEMX);
	exit(0);
}

void mem_clean()
{
register i;

	for(i = 0; i < DM_MAXPROC; i++)
		if (pid[i] > 0)
			kill (pid[i],SIGINT);

}
