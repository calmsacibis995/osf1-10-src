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
static char	*sccsid = "@(#)$RCSfile: mt.c,v $ $Revision: 4.2.11.3 $ (DEC) $Date: 1993/10/11 17:26:26 $";
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
 * Copyright (c) 1980 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that: (1) source distributions retain this entire copyright
 * notice and comment, and (2) distributions including binaries display
 * the following acknowledgement:  ``This product includes software
 * developed by the University of California, Berkeley and its contributors''
 * in the documentation or other materials provided with the distribution
 * and in all advertising materials mentioning features or use of this
 * software. Neither the name of the University nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	mt.c	5.4 (Berkeley) 6/1/90
 *	mt.c	5.1 17:50:45 8/15/90 SecureWare
 */
 
#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1980 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/*
 * mt --
 *   magnetic tape manipulation program
 */

/* ---------------------------------------------------------------------
 * Modification History
 *
 * June 10, 1991	Tom Tierney
 *	Ported this baby to OSF/1 (actually, to be more correct, merged
 *	additional ULTRIX features and functions with OSF/1's existing
 *	mt).
 *
 * January 8, 1991 by Robin Miller.
 *	Updated the print_category() function to check for density codes
 *	added for the TZK08 (DEV_54000_BPI) and the TLZ04 (DEV_61000_BPI).
 *
 * September 12, 1990 by Robin Miller.
 *	Updated the print_category() function to check for and display
 *	the density codes for QIC tapes.  Also rearranged code to allow
 *	checking for unspecified density.  This is now consistant with
 *	what the 'file' utility displays.
 *
 * July 5 1990	Bill Dallas
 *	Added retension for tzk10
 *
 * Nov  8 1989	Tim Burke
 *	Set the exit value to 0 on success.
 *	Changed the format of the `status` command to display the contents
 *	of the devget and mtiocget data structures.  Removed device dependant
 *	attribute code which was part of the old status mechanism.
 *
 * Jun 15 1989  tim 	Tim Burke
 *	Made massbus tapes the only vax-specific tapes.  Corrected a
 *	seg fault resulting from a null pointer reference if `mt status`
 *	was issued to a TZK50 in a mips box.
 *
 * Nov 23 1988  jag	John A. Gallant
 *	Moved the TZK50 entry into the tapes[], to allow the tape type from
 *	the driver to be recognized.
 *
 * Feb 11 1986  rsp     (Ricky Palmer)
 *	Removed "don't grok" error message.
 *
 * Sep 11 1986  fred	(Fred Canter)
 *	Bug fix to allow "mt status" to work with VAXstar TZK50.
 *
 * Sep  9 1986  fries	Corrected bugs whereas device was opened twice
 *			could not perform non-write functions to a write
 *			protected device.
 *
 * Aug 27 1986  fries	Made commands: clserex, clhrdsf, clsub, eoten
 *			and eotdis read only commands.
 *
 * Apr 28 1986  fries	Added commands: clserex, clhrdsf, clsub, eoten
 *			and eotdis. Added code to insure only superuser
 *			can change eot detect flag.
 *
 * Feb 10 1986  fries	Added commands: clserex, clhrdsf, clsub, eoten
 *			and eotdis. Added code to insure only superuser
 *			can change eot detect flag.
 *
 * Jan 29 1986  fries	Changed default tape definition DEFTAPE to
 *			DEFTAPE_NH to coincide with new mtio.h naming
 *			convention.
 *
 * Jul 17 1985	afd	on status command, check for tmscp device types
 *			and interpret the returned data accordingly.
 *
 * Jul 17 1985	afd	Added mt ops: "cache" and "nocache" to enable &
 *			disable caching on tmscp units.
 *
 * Dec 6 1984	afd	took out references to Sun tape devices and
 *			added tmscp device to the "tapes" table.
 *
 * Dec 6 1984	afd	derived from Berkeley 4.2BSD labeled
 * 			"@(#)mt.c	4.8 (Berkeley) 83/05/08"
 *
 * ---------------------------------------------------------------------
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/mtio.h>
#include <sys/ioctl.h>
#include <io/common/devio.h>
#include <sys/mtio.h>
#include <sys/errno.h>
#include <sys/file.h>

#include <locale.h>
#include <nl_types.h>
#include "mt_msg.h"
#define MSGSTR(N,S) catgets(catd,MS_MT,N,S)
nl_catd catd;

#ifndef MTSTATUS
#define MTSTATUS MTNOP
#endif  MTSTATUS

#define	equal(s1,s2)	(strcmp(s1, s2) == 0)

struct commands {
	char *c_name;
	int c_code;
	int c_ronly;
	int c_onlnck;
} com[] = {
	{ "weof",	MTWEOF,	0 , 1 },
	{ "eof",	MTWEOF,	0 , 1 },
	{ "fsf",	MTFSF,	1 , 1 },
	{ "bsf",	MTBSF,	1 , 1 },
	{ "fsr",	MTFSR,	1 , 1 },
	{ "bsr",	MTBSR,	1 , 1 },
	{ "rewind",	MTREW,	1 , 1 },
	{ "offline",	MTOFFL,	1 , 1 },
	{ "rewoffl",	MTOFFL,	1 , 1 },
	{ "status",	MTSTATUS, 1 , 0 },
	{ "cache",	MTCACHE, 1 , 0 },
	{ "nocache",	MTNOCACHE, 1 , 0 },
	{ "clserex",	MTCSE, 1 , 0 },
	{ "clhrdsf",	MTCLX, 1 , 0 },
	{ "clsub",	MTCLS, 1 , 0 },
	{ "eoten",	MTENAEOT, 1 , 0 },
	{ "eotdis",	MTDISEOT, 1 , 0 },
	{ "flush",	MTFLUSH, 0 , 1 },
	{ "retension",	MTRETEN, 1 , 1 },
	{ "seod",	MTSEOD, 1 , 1 },
	{ "erase",	MTERASE, 1 , 1 },
	{ "online",	MTONLINE, 1 , 0 },
	{ "load",	MTLOAD, 1 , 0 },
	{ "unload",	MTUNLOAD, 1 , 1 },
	{ 0 }
};

int mtfd;
int generic_ioctl_suceed;

struct mtop mt_com;
struct mtget mt_status;
char *tape;

main(argc, argv)
	char **argv;
{
	char line[80], *getenv();
	register char *cp;
	register struct commands *comp;

	(void) setlocale(LC_ALL, "");
	catd = catopen(MF_MT, NL_CAT_LOCALE);

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();

	if (!authorized_user("tape")) {
		fprintf(stderr, MSGSTR(AUTH, "mt: need tape authorization\n"));
	exit(1);
	}
#endif
 
	if (argc > 2 && (equal(argv[1], "-t") || equal(argv[1], "-f"))) {
		argc -= 2;
		tape = argv[2];
		argv += 2;
	} else
		if ((tape = getenv("TAPE")) == NULL)
			tape = DEFTAPE_NH;
	if (argc < 2) {
		fprintf(stderr, 
		  MSGSTR(USAGE, "usage: mt [ -f device ] command [ count ]\n"));
		exit(1);
	}
	cp = argv[1];
	for (comp = com; comp->c_name != NULL; comp++)
		if (strncmp(cp, comp->c_name, strlen(cp)) == 0)
			break;
	if (comp->c_name == NULL) {
		fprintf(stderr, 
		  MSGSTR(INVALID, "mt: invalid command: \"%s\"\n"), cp);
		exit(1);
	}
#if SEC_BASE
	if (forceprivs(privvec(SEC_ALLOWDACACCESS,
#if SEC_MAC
				SEC_ALLOWMACACCESS,
#endif
#if SEC_ILB
				SEC_ILNOFLOAT,
#endif
#if SEC_NCAV
				SEC_ALLOWNCAVACCESS,
#endif
				-1), (priv_t *) 0)) {
		fprintf(stderr, MSGSTR(NOPRIV,"mt: insufficient privileges\n"));
		exit(1);
	}
#endif
	if ((mtfd = statchk(tape, comp->c_ronly ? 0 : 2, comp->c_onlnck)) < 0) {
		if (errno)perror(tape);
		exit(1);
	}
	/* check for enable or disable eot(must be superuser) */
	if ((comp->c_code == MTENAEOT || comp->c_code == MTDISEOT) && geteuid())
           {
            fprintf(stderr, 
	      MSGSTR(NOPRIV, "mt: must be Superuser to perform %s\n"),comp->c_name);
	    exit(1);
	    }

	if (comp->c_code != MTSTATUS) {
		mt_com.mt_op = comp->c_code;
		mt_com.mt_count = (argc > 2 ? atoi(argv[2]) : 1);
		if (mt_com.mt_count < 0) {
			fprintf(stderr, 
			  MSGSTR(NEGRCNT, "mt: negative repeat count\n"));
			exit(1);
		}
		if (ioctl(mtfd, MTIOCTOP, &mt_com) < 0) {
			fprintf(stderr, "%s %s %d ", tape, comp->c_name,
				mt_com.mt_count);

			/* The following test is in case you are past */
			/* the EOT and rewind, the rewind completes   */
			/* but an errno of ENOSPC is returned...      */
			/* ALL OTHER ERRORS are REPORTED              */
			if ((mt_com.mt_op == MTREW) && (errno == ENOSPC))
						;/* do nothing */
			else
				/* else perror */
				perror(MSGSTR(FAILED, "failed"));
			exit(2);
		}
	} 
	/* The command is MTSTATUS meaning that `mt status` was issued */
	else {
		status();
	}
#if SEC_BASE
	/* Success of the mt command; return 0 */
	exit(0);
#endif
}

/*
 * Print out tape status based on deviocget and mtiocget.
 */
status()
{
	print_devio(mtfd);
	print_mtio(mtfd);
}

/* Routine to obtain generic device status */
statchk(tape,mode, c_onlnck)
	char	*tape;
	int	mode, c_onlnck;
{
	int to;
	int error = 0;
	struct devget mt_info;
	
	generic_ioctl_suceed = 0;

	/* Force device open to obtain status */
	to = open(tape,mode|O_NDELAY);

	/* If open error, then error must be no such device and address */
	if (to < 0)return(-1);
	
	/* Get generic device status */
	if (ioctl(to,DEVIOCGET,(char *)&mt_info) < 0)return(to);

	/* Set flag indicating successful generic ioctl */
	generic_ioctl_suceed = 1;

	/* Check for device on line */
	if((c_onlnck) && (mt_info.stat & DEV_OFFLINE)){
	  fprintf(stderr,MSGSTR(OFFLINE, "\7\nError on device named %s - Place %s tape drive unit #%u ONLINE\n"),tape,mt_info.device,mt_info.unit_num);
	  exit(1);
	}

	/* Check for device write locked when in write mode */
	if((c_onlnck) && (mt_info.stat & DEV_WRTLCK) && (mode != O_RDONLY)){
           fprintf(stderr, MSGSTR(WRTLCKD, "\7\nError on device named %s - WRITE ENABLE %s tape drive unit #%u\n"),tape,mt_info.device,mt_info.unit_num);
	   exit(1);
	 }
	   
	 /* All checked out ok, return file descriptor */
	 return(to);
}
/*
 * Display the contents of the mtget struct.
 * Args: fd a file descriptor of the already opened tape device.
 */
print_mtio(fd)
	int fd;
{
	struct mtget mt;
	if (ioctl(fd, MTIOCGET, (char *)&mt) < 0) {
		printf("\nmtiocget ioctl failed!\n");
		exit(1);
	}
	printf("\nMTIOCGET ELEMENT	CONTENTS");
	printf("\n----------------	--------\n");
	printf("mt_type			");
	switch(mt.mt_type) {
	case MT_ISTS:
		printf("MT_ISTS\n");
		break;
	case MT_ISHT:
		printf("MT_ISHT\n");
		break;
	case MT_ISTM:
		printf("MT_ISTM\n");
		break;
	case MT_ISMT:
		printf("MT_ISMT\n");
		break;
	case MT_ISUT:
		printf("MT_ISUT\n");
		break;
	case MT_ISTMSCP:
		printf("MT_ISTMSCP\n");
		break;
	case MT_ISST:
		printf("MT_ISST\n");
		break;
	case MT_ISSCSI:
		printf("MT_ISSCSI\n");
		break;
	default:
		printf("Unknown mt_type = 0x%x\n",mt.mt_type);
	}
	printf("mt_dsreg		%X\n", mt.mt_dsreg);
	printf("mt_erreg		%X\n", mt.mt_erreg);
	printf("mt_resid		%X\n", mt.mt_resid);
	printf("\n");
}
/*
 * Display the contents of the deviocget struct.
 * Args: fd a file descriptor of the already opened tape device.
 */
print_devio(fd)
	int fd;
{
	struct devget devinf;
	if (ioctl(fd, DEVIOCGET, (char *)&devinf) < 0) {
		printf("\ndevget ioctl failed!\n");
		exit(1);
	}
	printf("\nDEVIOGET ELEMENT	CONTENTS");
	printf("\n----------------	--------\n");
	printf("category		");
	switch(devinf.category) {
	case DEV_TAPE:
		printf("DEV_TAPE\n");
		break;
	case DEV_DISK:
		printf("DEV_DISK\n");
		break;
	case DEV_TERMINAL:
		printf("DEV_TERMINAL\n");
		break;
	case DEV_PRINTER:
		printf("DEV_PRINTER\n");
		break;
	case DEV_SPECIAL:
		printf("DEV_SPECIAL\n");
		break;
	default:
		printf("UNDEFINED VALUE (%x)\n", devinf.category);
		break;
	}
	printf("bus			");
	switch(devinf.bus) {
	case DEV_UB:
		printf("DEV_UB\n");
		break;
	case DEV_QB:
		printf("DEV_QB\n");
		break;
	case DEV_MB:
		printf("DEV_MB\n");
		break;
	case DEV_BI:
		printf("DEV_BI\n");
		break;
	case DEV_CI:
		printf("DEV_CI\n");
		break;
	case DEV_NB:	/* should be DEV_NB */
		printf("DEV_NB\n");
		break;
	case DEV_MSI:
		printf("DEV_MSI\n");
		break;
	case DEV_SCSI:
		printf("DEV_SCSI\n");
		break;
	case DEV_UNKBUS:
		printf("DEV_UNKBUS\n");
		break;
	default:
		printf("UNDEFINED VALUE (%x)\n", devinf.bus);
		break;
	}
	printf("interface		%s\n", devinf.interface);
	printf("device			%s\n", devinf.device);
	printf("adpt_num		%d\n", devinf.adpt_num);
	printf("nexus_num		%d\n", devinf.nexus_num);
	printf("bus_num			%d\n", devinf.bus_num);
	printf("ctlr_num		%d\n", devinf.ctlr_num);
	printf("slave_num		%d\n", devinf.slave_num);
	printf("dev_name		%s\n", devinf.dev_name);
	printf("unit_num		%d\n", devinf.unit_num);
	printf("soft_count		%u\n", devinf.soft_count);
	printf("hard_count		%u\n", devinf.hard_count);
	printf("stat			%X\n", devinf.stat);
	print_stat(devinf.stat);
	printf("category_stat		%X\n", devinf.category_stat);
	print_category(devinf.category_stat);
	
}

/*
 * Disect the stat field of a devio structure
 */
print_stat(stat)
	long stat;
{
	printf("\t\t\t");
	if (stat & DEV_BOM)
		printf("DEV_BOM ");
	if (stat & DEV_EOM)
		printf("DEV_EOM ");
	if (stat & DEV_OFFLINE)
		printf("DEV_OFFLINE ");
	if (stat & DEV_WRTLCK)
		printf("DEV_WRTLCK ");
	if (stat & DEV_BLANK)
		printf("DEV_BLANK ");
	if (stat & DEV_WRITTEN)
		printf("DEV_WRITTEN ");
	if (stat & DEV_CSE)
		printf("DEV_CSE ");
	if (stat & DEV_SOFTERR)
		printf("DEV_SOFTERR ");
	if (stat & DEV_HARDERR)
		printf("DEV_HARDERR ");
	if (stat & DEV_DONE)
		printf("DEV_DONE ");
	if (stat & DEV_RETRY)
		printf("DEV_RETRY ");
	if (stat & DEV_ERASED)
		printf("DEV_ERASED ");
	printf("\n");
}
/*
 * Disect the category_stat field of a devio structure
 */
print_category(stat)
	long stat;
{
	printf("\t\t\t");
	if (stat & DEV_TPMARK)
		printf("DEV_TPMARK ");
	if (stat & DEV_SHRTREC)
		printf("DEV_SHRTREC ");
	if (stat & DEV_RDOPP)
		printf("DEV_RDOPP ");
	if (stat & DEV_RWDING)
		printf("DEV_RWDING ");
	if (stat & DEV_LOADER)
		printf("DEV_LOADER ");

	if (stat & DEV_800BPI) {
		printf("DEV_800BPI");
	} else if (stat & DEV_1600BPI) {
		printf("DEV_1600BPI");
	} else if (stat & DEV_6250BPI) {
		printf("DEV_6250BPI");
	} else if (stat & DEV_6666BPI) {
		printf("DEV_6666BPI");
	} else if (stat & DEV_10240BPI) {
		printf("DEV_10240BPI");
	} else if (stat & DEV_38000BPI) {
		printf("DEV_38000BPI");
#ifdef DEV_38000_CP
	} else if (stat & DEV_38000_CP) {
		printf("DEV_38000_CP");
	} else if (stat & DEV_76000BPI) {
		printf("DEV_76000BPI");
	} else if (stat & DEV_76000_CP) {
		printf("DEV_76000_CP");
#endif DEV_38000_CP
	} else if (stat & DEV_8000_BPI) {
		printf("DEV_8000_BPI");
	} else if (stat & DEV_10000_BPI) {
		printf("DEV_10000_BPI");
	} else if (stat & DEV_16000_BPI) {
		printf("DEV_16000_BPI");
	} else if (stat & DEV_54000_BPI) {
		printf("DEV_54000_BPI");
	} else if (stat & DEV_61000_BPI) {
		printf ("DEV_61000_BPI");
	} else if (stat & DEV_45434_BPI) {
		printf ("DEV_45434_BPI");
	} else if (stat & DEV_42500_BPI) {
		printf ("DEV_42500_BPI");
	} else if (stat & DEV_62500_BPI) {
		printf ("DEV_62500_BPI");
	} else if (stat & DEV_40640_BPI) {
		printf ("DEV_40640_BPI");
	} else if (stat & DEV_36000_BPI) {
		printf ("DEV_36000_BPI");
	} else {
		printf("<unspecified density>");
	}
	printf("\n");
}

