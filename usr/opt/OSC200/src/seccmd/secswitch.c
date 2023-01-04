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
static char *rcsid = "@(#)$RCSfile: secswitch.c,v $ $Revision: 4.1.10.2 $ (DEC) $Date: 1993/04/01 20:18:23 $";
#endif
/* secswitch.c
 *	- uday gupta Aug 1991
 */
/* This program is used to
 *	(a) Get the status of the runtime security switch flag
 *	    Exit status 0 indicates OFF, 1 indicates ON
 *	(b) Change the value of the runtime security switch flag
 *		(to either ON or OFF)
 *	(c) Set the kernel privileged groupid (SPG)
 *	(d) Check if SPG exists in /etc/group
 *		otherwise prohibit all user logins
 */

#include <stdio.h>
#include <fcntl.h>
#include <paths.h>
#include <grp.h>
#include <errno.h>

#define GRP_SEC	"sec"
#define SPG "Security-Privileged-Group-id"

char *grpsec = GRP_SEC;
gid_t egid;
FILE *nlfp = NULL;

usage(s)
char *s;
{
	printf("Usage: %s [-disable | -enable | -quiet | -test]\n", s);
	exit(-1);
}

/* open the nologin file, and write out the opening banner
 */
wr_nologin(err)
int err;
{

  if (nlfp == NULL) {
    if ((nlfp = fopen(_PATH_NOLOGIN, "a+")) == NULL) {
	printf("Cannot create %s\n", _PATH_NOLOGIN);
	nlfp = stderr;
    }

    fprintf(nlfp,"\n***************************************************\n");
    fprintf(nlfp,"\tAll logins must be prohibited\n\tSystem Security needs to be repaired\n");
    fprintf(nlfp,"***************************************************\n\n");
  }

  switch (err) {

   case 1:
	fprintf(nlfp,"\nThe '%s' entry in /etc/group is not valid\n", grpsec);
	fprintf(nlfp,"Please set group '%s' to a positive number greater than zero\n",
		grpsec);
	fprintf(nlfp,"Reset all commands set-group-id to '%s' to this number\n",
		grpsec);
	break;

   case 2:
	 fprintf(nlfp,"\nThe group '%s' entry in /etc/group is missing\n",
		grpsec);
	 fprintf(nlfp,"Please create group '%s' with group-id = '%d'\n",
		grpsec, egid);
	break;

   case 3:
	fprintf(nlfp,"\nThe '%s' entry in /etc/group is not the '%s'\n", grpsec, SPG);
	fprintf(nlfp,"Please set group '%s' to group-id = '%d'\n",
		grpsec, egid);
	break;

   case 4:
	fprintf(nlfp,"\nThe '%s' is not set to '%d'\n", SPG, egid);
	fprintf(nlfp,"Suggest reboot of system\n");
	break;

  }

  fflush(nlfp);
}

/* see if the egid matches that of grpsec in /etc/group
 */
chk_gid(sgid)
register int sgid;
{
	register struct group *sg;

	if (sgid < 0)
		wr_nologin(1);
	else if ((sg = (struct group *) getgrnam(grpsec)) == NULL)
	 	wr_nologin(2);
	else if (egid != sg->gr_gid)
		wr_nologin(3);
	else if ((sgid > 0) && (sgid != egid))
		wr_nologin(4);

	if (nlfp != stderr) {
		fflush(nlfp);
		fclose(nlfp);
	}

	return (nlfp ? 1 : 0);
}

main(ac,av)
int ac; char **av;
{
	register int ret=0;

	if (ac == 1) {
		ret = security_is_on();
		if (ret == 0) {
		  printf("security is currently switched OFF\n");
		} else {
		  printf("security is currently switched ON\n");
		  ret = 1;
		}
		exit(ret);
	}

	if (av[1][0] != '-')
		usage(av[0]);

	if(av[1][1] == 'q') {
		ret = security_is_on();
		if (ret != 0)
			ret = 1; /* is ON */
		exit(ret);
	}

	if (getuid() != 0) {
		fprintf(stderr,"You must be superuser to use options\n");
		exit(-1);
	}

	egid = getegid();

	switch(av[1][1]) {
	  case 'd':
		ret = security_turn_off();
		if (ret < 0) {
			psecerror(av[0]);
			ret = 1;
		}
		break;

	  case 'e':
		ret = security_turn_on(egid);
		if (ret < 0) {
		  if (errno == EINVAL)
		    printf("Invalid gid '%d' specified for %s\n",
			egid, SPG);
		  psecerror(av[0]);
		  ret = 1;
		} else {
		  nlfp = stderr; /* don't create nologin file */
		  ret = chk_gid(egid);
		}
		break;

	  case 't': /* if security is ON, test the group-id */
		ret = security_is_on();
		if (ret != 0) {
			ret = chk_gid(ret);
			if (ret)
			  printf("%s: Logins Disabled\n", av[0]);
		}
		break;

	  default:
		usage(av[0]);
	}

	exit(ret);
}
