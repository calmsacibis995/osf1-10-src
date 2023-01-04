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
static char	*sccsid = "@(#)$RCSfile: groups.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/10/11 17:08:33 $";
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

/*
 * COMPONENT_NAME: (CMDOPER) commands needed for basic system needs
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <sys/param.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef NLS
#include <locale.h>
#endif

#ifdef  MSG
#include <nl_types.h>
#include "groups_msg.h"
nl_catd catd;
#define MSGSTR(num,str) NLcatgets(catd,MS_groups,num,str)  /*MSG*/
#else
#define MSGSTR(num,str) str
#endif

#define GROUP_FILE	"/etc/group"

extern struct passwd *getpwnam();   /* get passwd struct for user */
extern struct group *getgrent(); /* get the next group struct in the file */

int	groups[NGROUPS];

/*
 * NAME: groups [user]
 *                                                                    
 * FUNCTION: Write to standard out user group membership.
 *           Default is current user.
 */  
main(argc, argv)
	int argc;
	char *argv[];
{
	int ngroups, i;
	char *sep = "";
	struct group *gr;
	struct stat sbuf;

#ifdef NLS
	(void ) setlocale (LC_ALL,"");   /* set up table for current lang */
#endif

#ifdef  MSG
	catd = NLcatopen(MF_GROUPS, NL_CAT_LOCALE);
#endif

	if(stat(GROUP_FILE, &sbuf) < 0) {
		fprintf(stderr,MSGSTR(M_NO_GRP_FILE, "groups: %s file must be created\n"), GROUP_FILE);
		exit(1);
		}

	if(sbuf.st_size == 0)
		fprintf(stderr,MSGSTR(M_BAD_SIZE, "groups: %s bad size\n"), GROUP_FILE);

	if (argc > 1)
		showgroups(argv[1]);
	else {
	ngroups = getgroups(NGROUPS, groups); /* get list of current */
                                                /* user's group membership */
	for (i = 0; i < ngroups; i++) {
		gr = getgrgid(groups[i]);             /* get data on group */
		if (gr == NULL)
			printf("%s%d", sep, groups[i]);
		else
			printf("%s%s", sep, gr->gr_name);
		sep = " ";
		}
	printf("\n");
	exit(0);
	}
}

/*
 * NAME: showgroup
 *                                                                    
 * FUNCTION: Display group membership for user.
 */  
showgroups(user)
	char *user;
{
	struct group *gr;
	struct passwd *pw;
	char **cp;
	char *sep = "";
		
	if ((pw = getpwnam(user)) == NULL) {       /* get data on user */
		fprintf(stderr,MSGSTR(M_MSG_3, "No such user\n"));
		exit(1);
	}
	 while (gr = getgrent()) {         /* search group file for user */
		if (pw->pw_gid == gr->gr_gid) {
			printf("%s%s", sep, gr->gr_name);
			sep = " ";
			continue;
		}	
		for (cp = gr->gr_mem; cp && *cp; cp++)
			if (strcmp(*cp, user) == 0) {
				printf("%s%s", sep, gr->gr_name);
				sep = " ";
				break;
			}
	}
	printf("\n");
	return;
}
