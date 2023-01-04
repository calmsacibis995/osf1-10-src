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
static char     *sccsid = "@(#)$RCSfile: misc.c,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/09/03 14:30:50 $";
#endif lint
/************************************************************************
 *									*
 *			Copyright(c) 1987 by				*
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
 *
 * Name: misc.c
 *
 * Modification History
 * 
 * Jun 5, 1991 - jaa
 *	ported to OSF/1.  modified to use new bus/controller/device
 *	kernel structures.  in porting, we don't use /dev/kmem anymore.
 *	getsysinfo(3) was modified to return the needed information.
 *	NOTE: ifdef TODO's in this code will have to be done as the 
 *	pieces are added (i.e. floating csr space, workstation specifics)
 *
 * May 21, 1990 - Robin
 *	added code to generate the pseudo device entry to support
 *	presto nvram driver.
 *
 * Dec 15, 1989 - Alan Frechette
 *	Added subroutine getwsinfo().
 *
 * Oct 11, 1989 - Alan Frechette
 *	Fixed a bug in the search_config() subroutine.
 *
 * May 10, 1989 - Alan Frechette
 *	Set pagesize to NBPG in routine getphysmem() to correctly
 *	determine the size of physical memory.
 *
 * Feb 12, 1989 - Alan Frechette
 *	New sizer code which supports multiple architectures.
 *	Restructured this code and cleaned it up considerably.
 *	Based on the original V3.0 sizer by Tungning Cherng.
 *
 ***********************************************************************/

#include <stdio.h>
#include <ctype.h>
#include "sizer.h"

/****************************************************************
*    checksysname						*
*								*
*    Check the system name and if it is not a valid system	*
*    name then prompt the user for a correct system name.	*
****************************************************************/
checksysname()
{
	char *ptr;
	char newsysname[100];
	int errorflag;
	int rcode;
	extern char *Sysname;

	while(1) {
		ptr = Sysname;
		errorflag = 0;
		while(*ptr != '\0') {
			if(isdigit(*ptr) || isalpha(*ptr) ||
				*ptr == '_' || *ptr == '.' || *ptr == '-' )
				ptr++;
			else {
				fprintf(stderr,
				"Invalid character %c in system name. ",*ptr);
				fprintf(stderr,"Only use alphanumeric.");
				errorflag = 1;
				break;
			}
		}
		if(errorflag == 0)
			break;
		else {
			fprintf(stdout, "Enter a valid system name: ");
			rcode = scanf("%s", newsysname);
			Sysname = &newsysname[0];
			if(rcode < 1 || rcode  == EOF)
				quitonerror(-3);
		}
	}
}

/****************************************************************
*    getconfig_string						*
*								*
*    Search the /tmp/.config file for any options,	*
*    pseudo-devices, controllers, etc, to add to our config 	*
*    file.  It's only argument is the type of config file	*
*    entry it is to look for in /tmp/.config. When	*
*    it finds a match it adds that line to our config file.	*
****************************************************************/
getconfig_string(type)
char *type;
{
	FILE *fpconfig;
	char line[PATHSIZE],conftype[20];
 	int  i,len,found=0;

	if((fpconfig = fopen("/tmp/.config","r")) == NULL) {
 		return(found);
	}
	while(fgets(line,PATHSIZE,fpconfig) != NULL) {
		sscanf(line,"%s",conftype);
		if(strcmp(type,"hardware") == 0) {
			for(i=0; i<Hardtblsize; i++) {
				if(strcmp(conftype,hardtbl[i].typename) == 0) {
					found = 1;
					fprintf(Fp,"%s",line);
				}
			}
		}
		else if(strcmp(conftype,type) == 0) {
			found = 1;
			if(strcmp(type,"swap") == 0 
				|| strcmp(type,"dumps") == 0) {
				len = strlen(line);
				line[len-1] = ' ';
				strcat(line," \0");
			}
 			fprintf(Fp,"%s",line);
 		}
	}
 	fclose(fpconfig);
	return(found);
}

/****************************************************************
*    search_config						*
*                 						*
*    	Search the /tmp/.config file for the given	*
*	string fields.						*
****************************************************************/
search_config(key1,word1)
char key1[];
char word1[];
{
	FILE *fpconfig;
	char line[PATHSIZE];
	char key2[20];
	char word2[30];
	int found=0;

	if((fpconfig = fopen("/tmp/.config","r")) == NULL) {
 		return(found);
	}
	while(fgets(line,PATHSIZE,fpconfig) != NULL) {
		sscanf(line,"%s%s",key2,word2);
		if(strcmp(key1,key2) == 0 && strcmp(word1,word2)==0) {
 			found=1;
			break;
 		}
	}
 	fclose(fpconfig);
	return(found);
}

/****************************************************************
*    asksysid							*
*								*
*    Ask for the scs_sysid. 					*
****************************************************************/
asksysid()
{
	char buf[60];
	int i,n;
RETRY:
	printf("\nEnter the scs_sysid number: ");
	fflush(stdout);
	gets(buf);
	for (i=0; buf[i]==' ' || buf[i]=='\t'; i++)
		;
	for (n=i; buf[i]!='\0' ;i++)
		if (!(isdigit(buf[i]))) {
			fprintf(stderr,"Invalid scs_sysid %s. ", &buf[i]);
			fprintf(stderr,"Only use integer number.\n"); 
			goto RETRY;
		}
	n=atoi(&buf[n]);
	return(n);
}

/****************************************************************
*    getsysid							*
*								*
*    Get the scs_sysid. 					*
****************************************************************/
getsysid()
{
	u_short sysid = 0;

	getsysinfo( GSI_SCS_SYSID , &sysid, sizeof(sysid), 0, 0);
	if( sysid == 0 )	
		return(1);
	else
		return(sysid);
}

#ifdef TODO 
/****************************************************************
*    reset_anythg						*
*								*
*    Go back to the begining of a namelist location.		*
****************************************************************/
long reset_anythg(nlindex)
int nlindex;
{
	long offset;

	offset = lseek(Kmem,(long)nl[nlindex].n_value, 0);
	return(offset);
}
#endif /* TODO */

/************************************************************************
*    getwsinfo               						*
*                        						*
*    Get and display the workstation display info. 			*
*                        						*
************************************************************************/
getwsinfo(tflag)
int tflag;
{
 	int ws = 0, cmd;
#define BUFSZ 20
	char buf[BUFSZ];

	if (tflag < 4) {
		cmd = (tflag == 1) ? GSI_WSD_TYPE :
			((tflag == 2) ? GSI_WSD_UNITS : GSI_WSD_CONS);
	
		getsysinfo(cmd, &ws, sizeof(ws), 0, 0);
		fprintf(stdout, "%d\n", ws);
	} else {
		cmd = (tflag == 4) ? GSI_KEYBOARD : GSI_POINTER;
		getsysinfo(cmd, buf, BUFSZ, 0, 0);
		fprintf(stdout, "%s\n", buf);
	}
}


/************************************************************************
*    getbootedfile             						*
*                        						*
*    Get and display the booted kernel's filename. 			*
*                        						*
************************************************************************/
getbootedfile()
{
#define BOOTEDFILELEN 80
	char bootedfile[BOOTEDFILELEN];
	
	getsysinfo(GSI_BOOTEDFILE, bootedfile, BOOTEDFILELEN, 0, 0);
	fprintf(stdout, "%s\n", bootedfile);
}


/****************************************************************
 *  getgraphicinfo()						*
 *								*
 *  gflag = 1	Gets the graphic modules names from the kernel. *
 *  gflag = 2	Gets the resolution of modules from the kernel. *
 *								*
 *  Passes a cookie to the kernel to be filled in.  When the    *
 *  getsysinfo() system call returns a zero for the cookie,  	*
 *  there are no more modules					*
 ***************************************************************/
getgraphicinfo(gflag)
int gflag;
{
        int cookie = 0;	/* Zero signifies beginning of list */
	int first;
	char buf[9];
	struct {
	    int	width;	
	    int height;
	} res_buf = {0, 0};

	first = 1;
	while ((first) || (cookie != 0)) {
	    first = 0;
	    if (gflag == 1) {
		if (getsysinfo(GSI_GRAPHICTYPE, buf, sizeof(buf) - 1, &cookie, NULL) < 0) {
		    perror("getgraphicinfo");
		    exit(-1);
		}
		/* kernel will fill in null pointer if there is no graphics device */
		if (buf[0]) {
		    buf[8] = 0; /* terminate string */
		    fprintf(stdout, "%s\n", buf);
		}
	    }
	    if (gflag == 2) {
		if (getsysinfo(GSI_GRAPHIC_RES, &res_buf, sizeof(res_buf), &cookie, NULL) < 0) {
		    perror("getgraphicinfo");
		    exit(-1);
		}
		fprintf(stdout, "%dx%d ", res_buf.width, res_buf.height);
	    }
	}
	if (gflag == 2)
	    fprintf(stdout, "\n");
}


