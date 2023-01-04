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
/*
*/
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 *  ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: uumon.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/09/07 16:10:56 $";
#endif
/*
 * COMPONENT_NAME: UUCP uumon.c
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 10  27  3
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 * uumon.c
 *
 *	Program to monitor the uucp usage on a machine.
 *
 *					martin levy. (houxg!lime!martin).
 *	Ported from Ultrix to OSF; Mangala Kadaba (mik)
 */
#include	"uucp.h"
#include	<sys/types.h>
#include	"dirent.h"
#ifdef NDIR
#include "ndir.h"
#else
#include <sys/dir.h>
#endif
#include	<sys/time.h>
#include	<sys/stat.h>

#define	NMACH	200
#define	SIZE	132

struct m
{
	char mach[15];
	char locked;
	int ccount,xcount;
	int count,type;
	int retrytime;
	time_t lasttime;
	char timbuf[TIMBUF_SIZE];
	char timbuf_locked[TIMBUF_SIZE];
	char stst[SIZE];
} M[NMACH];

struct m *machine();
time_t time();

main()
{
	struct dirent *dirp, *Cdirp;
	struct m *m;
	DIR *ufd, *Cfd;
	FILE *sfd;
	char line[132],*c;
	int Ctotal=0;
	int Xtotal=0;
	struct stat statbuff;
	char tempname[16];
	char directory[250];
	FILE *dirlist;
	time_t successtime;
	int lenname, totstst, final;
	int ret1, ret2, i, ret3;
	char LOCKFILE[TIMBUF_SIZE];

	if ((Cfd = opendir(SPOOL))==NULL) {
		fprintf(stderr,"\nCan not open /usr/spool/uucp directory\n");
		exit(20);
	}
	/*  scan through the uucp directory.  For each system process
	 *   count the number of C. and X. files 
	 */
	while ((Cdirp=readdir(Cfd)) != NULL) {
		if(Cdirp->d_ino==(ino_t)0 || strncmp(".",Cdirp->d_name,1)==0)
			continue;

		sprintf(directory,"/usr/spool/uucp/%s",Cdirp->d_name);
		if( (ufd=opendir(directory)) == NULL ) 
			continue;	

		while( (dirp=readdir(ufd))!=NULL)
		{
			if( dirp->d_ino == (ino_t)0 )
				continue;
			if( strncmp("C.",dirp->d_name,2) == 0 )
			{
				*(dirp->d_name+strlen(dirp->d_name)-5) = '\0';
				m = machine(dirp->d_name+2);
				m->ccount++;
				Ctotal++;
				continue;
			}
		}
		closedir(ufd);

		sprintf(directory,"/usr/spool/uucp/%s",Cdirp->d_name);
		if( (ufd=opendir(directory)) == NULL ) 
			continue;
		
		while( (dirp=readdir(ufd))!=NULL)
		{
			if( strncmp("X.",dirp->d_name,2) == 0 )
			{
				*(dirp->d_name+strlen(dirp->d_name)-5) = '\0';
				m = machine(dirp->d_name+2);
				m->xcount++;
				Xtotal++;
				continue;
			}
		}
		closedir(ufd);

	} /* big while loop */
	closedir(Cfd);
	/* get the latest status for each system */

	if( (ufd=opendir(STATDIR)) == NULL ) {
		fprintf(stderr, "\nCan not open /usr/spool/uucp/.Status directory\n");
		exit(299);
	}

	ret1=ret2=1;

	for(i=0; i<=132; i++) line[i]='';
	chdir(STATDIR);
	while( (dirp=readdir(ufd))!=NULL) { 
			if( (sfd=fopen(dirp->d_name,"r")) != NULL ) {
			if (strcmp(dirp->d_name,"core") == 0) continue;
			m = machine(dirp->d_name);
			ret1 = strncmp(".",dirp->d_name,1);
			ret2 = strncmp("..",dirp->d_name,2);
			if(ret1 == 0) 
			{
				strncpy(m->mach,"",7);
				m->locked = 0;
				m->ccount = 0;
				m->xcount = 0;
				m->lasttime = 0;
				for(i=0; i<=SIZE; i++) m->stst[i]=''; 	
				for(i=0; i<=TIMBUF_SIZE; i++) m->timbuf[i]=''; 	
				for(i=0; i<=TIMBUF_SIZE; i++) m->timbuf_locked[i]=''; 	
				m->retrytime = 0;
			}
			if(ret2 == 0) 
			{
				strncpy(m->mach,"",7);
				m->locked = 0;
				m->ccount = 0;
				m->xcount = 0;
				m->lasttime = 0;
				for(i=0; i<=SIZE; i++) m->stst[i]=''; 	
				for(i=0; i<=TIMBUF_SIZE; i++) m->timbuf[i]=''; 	
				for(i=0; i<=TIMBUF_SIZE; i++) m->timbuf_locked[i]=''; 	
				m->retrytime = 0;
			}
			if((ret1 != 0) && (ret2 != 0)) 
			{
				fscanf(sfd, "%d %d %d %d",
					&m->type, &m->count, &m->lasttime,
					&m->retrytime);
strftime(m->timbuf,TIMBUF_SIZE,nl_langinfo(D_T_FMT),localtime(&m->lasttime));
				if( fgets(m->stst,SIZE,sfd) != NULL ) 
				{
				/* remove remote name from STST file */
					lenname=strlen(dirp->d_name);
					totstst=strlen(m->stst);
					final=totstst-lenname-2;
					strncpy(line,m->stst,final);
					strcpy(m->stst,line);
				} /* stst fgets */
			} /* ret1 and ret2 */
				fclose(sfd);
				for(i=0; i<=132; i++) line[i]='';
				ret1=ret2=1;
			} /* sfd */

	} /* end of while for STATDIR loop */
	closedir(ufd);

	/* record existing lock files */
	if( (ufd=opendir(DEVICE_LOCKPRE)) == NULL ) {
		fprintf(stderr, "\nCan not open %s directory\n", DEVICE_LOCKPRE);
		exit(99);
	}

	chdir(DEVICE_LOCKPRE);
	while( (dirp=readdir(ufd))!=NULL) {
		if( strncmp("LCK..",dirp->d_name,5) == 0 ) {
			strcpy(tempname,dirp->d_name+5);
			m = machine(tempname);
			m->locked++;
			strcpy(LOCKFILE,"");
			strcpy(LOCKFILE,DEVICE_LOCKPRE);
			strcat(LOCKFILE,"/");
			strcat(LOCKFILE,dirp->d_name);
			stat(LOCKFILE, &statbuff);
			m->lasttime = statbuff.st_ctime;
strftime(m->timbuf_locked,TIMBUF_SIZE,nl_langinfo(D_T_FMT),localtime(&m->lasttime));
			continue;
		}
	}
	closedir(ufd);

	printf("C.total=%d      X.total=%d\n\n", Ctotal, Xtotal);

	for(m = &M[0];*(m->mach) != NULL;m++)
	{
		printit(m);
	}
}

struct m *machine(name)
char *name;
{
	static int first = -1;
	struct m *m;
	int i;

	if( first )
	{
		first = 0;
		for(i=0;i<NMACH;*(M[i++].mach)=NULL);
	}

	for(m = &M[0];*(m->mach)!=NULL;m++)
		if( strncmp(name,m->mach,7) == 0 )
			return(m);

	strncpy(m->mach,name,7);
	m->locked = 0;
	m->ccount = 0;
	m->xcount = 0;
	m->lasttime = 0;
	for(i=0; i<=SIZE; i++) m->stst[i]=''; 		
	for(i=0; i<=TIMBUF_SIZE; i++) m->timbuf[i]=''; 		
	for(i=0; i<=TIMBUF_SIZE; i++) m->timbuf_locked[i]=''; 		
	m->retrytime = 0;
	return(m);
}

printit(m)
struct m *m;
{

	printf("%s\t",m->mach);
	if(m->locked) {
		printf("\tlocked\t");
		printf("%s", m->timbuf_locked);
	}	
	else {
		printf("%3dC\t",m->ccount);
		printf(" %3dX\t",m->xcount);
	}

	if( *(m->stst) != NULL )
	{
		printf("%s\t",m->stst);
		if(m->type == SS_LOGIN_FAILED)
			printf("CNT: %2d     ",m->count);
		printf("%s", m->timbuf);
	}
	putchar('\n');
}


