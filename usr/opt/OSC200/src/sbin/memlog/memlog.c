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
static char *rcsid = "@(#)$RCSfile: memlog.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/10/29 20:36:07 $";
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/memlog.h>

#define  USAGE	fprintf(stderr,"Usage: memlog [[-s size][-l #]|[-t]]\n")

extern void translate();

int    	mlog=0;
int	logfile=0;

void
main(int argc, char *argv[])
{
        char   	buf[MEM_RECORD_SIZE], *bufp;
	int 	opt, i=0;
	long 	mem_count;
	int 	type;
	void 	*caller;
	long	logsize=0;
	int	xlate=0, sopt=0;
	int	maxentries=10000;
	int	entries=0;

        while ( (opt = getopt(argc, argv, "s:l:t")) != EOF ) {
                switch ( opt ) {
                  case 's':
                        logsize = atoi(optarg);
			sopt = 1;
                        break;
                  case 'l':
                        maxentries = atoi(optarg);
                        break;
                  case 't':
                        xlate = 1;
                        break;
		  default:
			USAGE;
                        exit(1);
                }
        }


	if(xlate) {
		if(sopt) {
			USAGE;
			exit(1);
		}
		translate();
		exit(0);
	}

        mlog = open("/dev/mlog", O_RDONLY);
	if(mlog == -1) {
		printf("memlog: /dev/mlog open failed, errno=%d\n", errno);
		exit(1);
	}
        logfile = open("/memloglog", O_CREAT|O_TRUNC|O_RDWR, 0600);
	if(logfile == -1) {
		printf("memlog: /memloglog open failed, errno=%d\n", errno);
		exit(1);
	}

	if (logsize) {
		i = ioctl(mlog, MEM_LOG_ENABLE, &logsize);
		if(i == -1) {
			printf("memlog: ioctl to mlog failed, errno=%d\n", errno);
			exit(1);
		}	
	}

	daemon(0,0);

	while(entries < maxentries) {
        	i = read (mlog, buf, MEM_RECORD_SIZE);
		if(i == -1) {
			printf("memlog: read from /dev/mlog failed, errno=%d\n", errno);
			exit(1);
		}
#define DEBUG 0
#if DEBUG 
	bufp = buf;
	while(*bufp != '\n')
		putchar(*bufp++);
#endif
        	i = write (logfile, buf, MEM_RECORD_SIZE);
		if(i == -1) {
			printf("memlog: write to /logfile failed, errno=%d\n", errno);
			exit(1);
		}
		entries++;

	}


	return;
}
