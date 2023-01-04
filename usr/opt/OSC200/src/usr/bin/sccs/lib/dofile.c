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
static char	*sccsid = "@(#)$RCSfile: dofile.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 00:48:49 $";
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
 * COMPONENT_NAME: CMDSCCS      Source Code Control System (sccs)
 *
 * FUNCTIONS: do_file
 *
 * ORIGINS: 3, 10, 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*   dofile.c 1.6 com/cmd/sccs/lib/comobj,3.1,9013 11/8/89 14:24:39"; */


# include	"defines.h"
#ifdef PDA
#include	<sys/types.h>
#include 	<sys/param.h>
#include	<dirent.h>
#else
# include	<dirent.h>
#endif

int	nfiles;
char	had_dir;
char	had_standinp;


do_file(p,func)
register char *p;
int (*func)();
{
	extern char *Ffile;
	char str[FILESIZE];
	char ibuf[FILESIZE];
#ifdef PDA
	DIR *dirdat;
	struct dirent *dirent;
#else
	char	dbuf[BUFSIZ];
	FILE *iop;
	struct dirent dir[2];
#endif

	if (p[0] == '-') {
		had_standinp = 1;
		while (gets(ibuf) != NULL) {
			if (sccsfile(ibuf)) {
				Ffile = ibuf;
				(*func)(ibuf);
				nfiles++;
			}
		}
	}
	else if (exists(p) && (Statbuf.st_mode & S_IFMT) == S_IFDIR) {
		had_dir = 1;
		Ffile = p;
#ifdef PDA
		if ((dirdat = opendir(p)) == NULL)
			return;
		readdir(dirdat);			/* skip "." */
		readdir(dirdat);			/* skip ".." */
		do {
			dirent = readdir(dirdat) ;
			if (dirent != NULL) {
				sprintf(str,"%s/%s",p,dirent->d_name);
				if(sccsfile(str)) {
					Ffile = str;
					(*func)(str);
					nfiles++;
				}
			}
		} while (dirent != NULL);
		closedir(dirdat);
#else
		if((iop = fopen(p,"r")) == NULL)
			return;
		setbuf(iop,dbuf);
		dir[1].d_ino = 0;
		fread((char *)dir,sizeof(dir[0]),1,iop);   /* skip "."  */
		fread((char *)dir,sizeof(dir[0]),1,iop);   /* skip ".."  */
		while(fread((char *)dir,sizeof(dir[0]),1,iop) == 1) {
			if(dir[0].d_ino == 0) continue;
			sprintf(str,"%s/%s",p,dir[0].d_name);
			if(sccsfile(str)) {
				Ffile = str;
				(*func)(str);
				nfiles++;
			}
		}
		fclose(iop);
#endif
	}
	else {
		Ffile = p;
		(*func)(p);
		nfiles++;
	}
}
