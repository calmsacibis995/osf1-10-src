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
static char *sccsid = "@(#)$RCSfile: fsx.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/11/23 20:12:14 $";
#endif lint

/************************************************************************
 *									*
 *			Copyright (c) 1984 by				*
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
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
/************************************************************************
 *			Modification History
 *
 * 7/15/88 -- prs
 *      Added char string DR_FSX to allow proper use with -o option.
 *
 ************************************************************************/
/*
 *
 *	FSX.C	--  This routine will set up the file system test, and spawn the
 *		    processes that actually perform the exercising
 *
 */
#include <stdio.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <nlist.h>
#include <string.h>
#include "diag.h"

#define MODULE		"fsx"			/* name of module */
#define MINUTE		60			/* minute in 1 sec counts */
#define SECOND		1 			/* one second */
#define EXR_BG_NAME	"fsxr"

char *help[] =
{
#ifdef	OSF
	"\n\n(fsx) - DEC OSF/1 file system exerciser\n",
#else	/* !OSF */
	"\n\n(fsx) - ULTRIX-32 file system exerciser\n",
#endif	/* OSF */
	"\n",
	"usage:\n",
	"\tfsx [-h] [-p#] [-fpath] [-t#] [-ofile]\n",
	"\n",
	"-h\t(optional) Print this help message\n",
	"-ofile\t(optional) Save output in file\n",
	"-p#\t(optional) Number of processes to be spawned (1-250 default: 20)\n",
	"-t#\t(optional) Run time in minutes (default: run forever until cntl-c\n",
	"-fpath\t(optional) Path name of directory on mounted f.s. you wish\n",
	"      \tto test (ie. /usr or /mnt etc, default /usr/field)\n",
	"\n",
	"examples:\n",
	"\tfsx -p10\n",
	"\t{ Run 10 fsx processes (default path=/usr/field) forever }\n",
	"\tfsx -t180 -f/mnt &\n",
	"\t{ Run 20(default) fsx processes for 180 min. path=/mnt in background }\n",
	"\n",
	"",
};

int pid[DF_MAXPROC];			/* array of process id for fsxr
					   routines spawned */
int pid2[DF_MAXPROC];
int status[DF_MAXPROC];

char errbuf[512];			/* buffer for error message */

char path[128];
char *pathptr;
/* run time variables */
int timedelta;

char DR_FSX[] = "#LOG_FSX_01";		/* Logfile name */

main (argc,argv)
int argc;
char *argv[];
{
register i,j;
int maxproc;
int numproc;
int sig;
void fsx_clean();
char procid[10];
char clogfd[10];
char time[10];
int  c; /* paw */
char exr_filename[PATH_MAX], *exr_pathname;


	/* set up kill signal */
	signal(SIGINT,fsx_clean);
	signal(SIGTERM,fsx_clean);

	/* handle input args */
	numproc = 0;
	maxproc = DF_DEFMAXPROC;
	while ((c = getopt(argc, argv, "ho:p:t:f:")) != EOF) /* paw-9148 */
	{
		switch (c)
		{

		case 'h':	/* print help message */
			for (i = 0; *help[i]; i++)
				printf("%s",help[i]);
			exit(0);

		case 'o':	/* save output into file */
			strcpy(filename, optarg);
			break;

       		case 'p':	/* restrict number of processes spawned */
			maxproc = atoi(optarg);
			if (maxproc < 1)
				maxproc = 1;
			if (maxproc > DF_MAXPROC)
				maxproc = DF_MAXPROC;
			break;

		case 't':
			timedelta = atoi(optarg);
			break;

		case 'f':
			strcpy(path, optarg);
			if (path[0] != '\0' && path[1] != '\0') {
			    pathptr = &path[strlen(path) - 1];
			    if (*pathptr == '/')
			        *pathptr = '\0';
			}
			break;
		default:
			printf("fsx: Invalid arg %s, type \"fsx -h\" for help\n",*argv);
			exit(1);
		}
	}

	/* open logger */
	if (report(DR_OPEN,MODULE,DR_FSX))
	{
		fprintf(stderr,"%s: Can not start report generator, test aborted\n", MODULE);
		exit(1);
	}


	if (path[0] == '\0')
	    strcpy(path,"/usr/field");

	/* This allows using exerciser from any directory, even if user */
	/* changes its name */
	if (!strcpy (exr_filename, argv[0]))
		*exr_filename = '\0';
	if (exr_pathname = rindex(exr_filename, '/'))
		exr_pathname++;
	else
		exr_pathname = exr_filename;
	strcpy (exr_pathname, EXR_BG_NAME);


	/* fork and exec fsxr processes */
	for (i = 0; i < maxproc; i++)
	{
		if ((pid[i] = fork()) == 0)
		{
			sprintf(procid,"%d",i + 1);
			sprintf(clogfd,"%d",logfd);
			sprintf(time,"%d",timedelta);
			if (execl(exr_filename,exr_filename,path,procid,clogfd,time,0) < 0)
			{
				sprintf(errbuf,"Could not execl fsxr%d %s",
				  i+1,sys_errlist[errno]);
				report(DR_WRITE,MODULE,errbuf);
			}
			exit(0);
		}

		if (pid[i] == -1)
		{
			sprintf(errbuf,"Could not fork fsxr%d %s",
				i+1,sys_errlist[errno]);
			report(DR_WRITE,MODULE,errbuf);
			continue;
		}

		sprintf(errbuf,"Exerciser Process fsxr%d pid %d Started",i+1,pid[i]);
		report(DR_WRITE,MODULE,errbuf);
		numproc++;
	}
	sprintf(errbuf,"Started fsx exerciser - on %s", path);
#ifdef PRS
	(void)logerr(ELMSGT_DIAG,errbuf);
#endif

	for(i = 0; i < numproc; i++)
		pid2[i] = wait(&status[i]);

	for ( i = 0; i < numproc; i++) {
		for ( j = 0; pid2[i] != pid[j] && j < numproc; j++) ;
		if (WIFSIGNALED(status[i]))
			sig = WTERMSIG(status[i]);
		else
			sig = 0;
		sprintf(errbuf,"Process Termination fsxr%d pid %d Status %d %s",j+1,pid2[i],sig,sys_siglist[sig]);
		report(DR_WRITE,MODULE,errbuf);
	}
	sprintf(errbuf,"Stopped fsx exerciser - on %s", path);
#ifdef PRS
	(void)logerr(ELMSGT_DIAG,errbuf);
#endif
	report(DR_CLOSE,0,DR_FSX);
	exit(0);

}


void fsx_clean()
{
register i;

	for(i = 0; i < DF_MAXPROC; i++)
		if (pid[i] > 0)
			kill (pid[i],SIGINT);

}
