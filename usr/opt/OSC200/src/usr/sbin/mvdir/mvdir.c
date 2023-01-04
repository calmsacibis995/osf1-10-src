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
static char	*sccsid = "@(#)$RCSfile: mvdir.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/10/08 15:46:43 $";
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
 * COMPONENT_NAME: (CMDFS) commands that deal with the file system
 *
 * FUNCTIONS: mvdir
 *
 * ORIGINS: 3, 27
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
 *
 * mvdir.c	1.4  com/cmd/fs/progs,3.1,9013 11/8/89 17:05:01
 */

/*
 * Move directory or file
 */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <NLctype.h>
#include <NLchar.h>
#include <locale.h>
#include <nl_types.h>
#include "mvdir_msg.h"

nl_catd catd;

#define MSGSTR(Num,Str) catgets(catd,MS_MVDIR,Num,Str)

int     Errors = 0;
char   *path = NULL;
char   *pointer;
struct  stat statb;

main(argc,argv)
int argc;
char **argv;
{
     (void) setlocale (LC_ALL,"");
     catd = catopen((char *)MF_MVDIR,NL_CAT_LOCALE);
     if(argc != 3) {
	  fprintf(stderr, MSGSTR(USAGE,"Usage: mvdir fromdir newname\n"));
	  exit(1);
     }
     if ((path = (char *) malloc (strlen(argv[2]) + strlen(argv[1])
       + 2)) == NULL) {
	  perror("malloc");
	  ++Errors;
	  exit(1);
     }
     if ((stat(argv[2], &statb) == 0) &&
	((statb.st_mode & S_IFMT) == S_IFDIR)) {
	  strcpy (path, argv[2]);
	  strcat (path, "/");
	  if (pointer = strrchr (argv[1], '/')) {
	       strcat (path, (pointer + 1));
	  } else {
	       strcat (path, argv[1]);
	  }
     } else {
	  strcpy (path, argv[2]);
     }
     if ((rename(argv[1], path)) != 0) {
	  perror("rename");
	  ++Errors;
     }
     exit(Errors? 2: 0);
}
