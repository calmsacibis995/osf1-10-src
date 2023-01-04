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
 * Copyright (c) 1989-90 SecureWare, Inc.    All rights reserved.
 *
 * spconfig(1)-determine what security policies are configured on
 * the system
 *
 * -v	verbose-print security policy names
 *
 * exit status-a bit mask of configured policies according to the
 *		defined policy magic numbers-SEC_ACL_MAGIC for
 *		instance is policy magic 0xf001 and thus is marked
 *		by bit 0, MAC is 0xf002 and is thus bit 1, etc.
 */

/* #ident "@(#)spconfig.c	3.1 10:02:07 6/7/90 SecureWare" */

/*
 * Based on:
 *   "@(#)spconfig.c	2.2.1.1 11:10:50 12/27/89 SecureWare, Inc."
 */

#include <sys/secdefines.h>
#include <locale.h>
#include "policy_msg.h"

nl_catd catd;
#define MSGSTR(n,s) catgets(catd,MS_POLICY,n,s)


#if SEC_ARCH

#include "errno.h"
#include "stdio.h"

#include "sys/types.h"
#include "sys/security.h"
#include "sys/secpolicy.h"
#include "sys/secioctl.h"
#include "fcntl.h"

int policy_magic[] = {
	SEC_ACL_MAGIC,
	SEC_MAC_MAGIC,
	SEC_MACILB_MAGIC,
	SEC_NCAV_MAGIC,
	SEC_PACL_MAGIC,
};

char *policy_name[] = {
	"Discretionary access control lists",
	"Mandatory access control",
	"Mandatory access control with information labels",
	"Nationality caveats",
	"Posix discretionary access control lists",
};

#define POLICY_COUNT (sizeof(policy_magic)/sizeof(policy_magic[0]))

struct sp_init sp_init;

main(argc,argv)
int argc;
char *argv[];
{
	register int i, pcount = 0, c;
	register int found = 0;
	register int spdfd = -1;
	register char *optarg;
	int verbose = 0;
	ushort mask = 0;


        (void) setlocale( LC_ALL, "" );
        catd = catopen(MF_POLICY,NL_CAT_LOCALE);

	while((c = getopt(argc,argv,"v")) != -1) {

	   switch(c) {

		case 'v':

			verbose++;
			break;

		case '?':

			usage();
			break;

	   }
	}

	if((spdfd = open(SPD_CONTROL_DEVICE,O_RDWR)) == -1) {
		perror(MSGSTR(SPCONFIG_1, "Error on open of policy control device"));
		exit(1);
	}

	for(i=0; i < POLICY_COUNT; i++) {

		sp_init.magic = policy_magic[i];
		if(ioctl(spdfd,SPIOC_GETCONF,&sp_init) == -1) {
			if(errno != ENXIO) {
				perror(MSGSTR(SPCONFIG_2, "Error on policy info request"));
				exit(0);
			}
			else continue;
		}

		pcount++;
		mask |= (1L << ((uint) sp_init.magic - (uint) SEC_ACL_MAGIC));

		if(verbose && found == 0) {
		     printf(MSGSTR(SPCONFIG_3, "\n\t\t *** Security policies configured ***\n\n"));
		     found = 1;
		}

		if(verbose)
			printf("\t%d) %s\n",pcount,policy_name[i]);
	}

	if(!verbose)
		printf("%d\n",mask);
	exit(mask);
}

usage()
{
	fprintf(stderr,MSGSTR(SPCONFIG_4, "usage: spconfig [ -v ]\n"));
	exit(0);
}
#endif
