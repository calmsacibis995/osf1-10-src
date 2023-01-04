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
static char *rcsid = "@(#)$RCSfile: iprsetup.c,v $ $Revision: 1.1.4.3 $ (DEC) $Date: 1992/12/15 01:35:52 $";
#endif
/*
 *  Name:  iprsetup  (IP router setup)
 *
 *  Description:  This program sets, resets and/or displays the global
 *		  variables ipforwarding and ipgateway in the kernel.
 *		  It is called by /usr/sbin/netsetup and /sbin/init.d/inet.
 *
 *  Commands:
 *			iprsetup -s
 *			iprsetup -sd
 *
 *		Sets or sets and displays the value of ipforwarding and
 *		ipgateway to 1.  This enables your system to function as
 *		be a router.
 *
 *			iprsetup -r
 *			iprsetup -rd
 *
 *		Resets or resets and displays the value of ipforwarding and
 *		ipgateway to 0.  This disables the routing functions in
 *		your system.
 *
 *			iprsetup -d
 *
 *		Displays the value of ipforwarding and ipgateway on stdout.
 *		This option can be combined with set and reset.
 *
 *  Modifies:	kernel global variables ipforwarding and ipgateway via the
 *		setsysinfo() system call
 */
#include <stdio.h>
#include <errno.h>
#include <paths.h>
#include <sys/file.h>
#include <sys/sysinfo.h>

/* valid command options */
#define IPR_SET		0x1	/* set ipforwarding and ipgateway */
#define IPR_RESET	0x2	/* reset ipforwarding and ipgateway */
#define IPR_DISPLAY	0x4	/* display ipforwarding and ipgateway */

/* set and display ipforwarding and ipgateway */
#define IPR_SETDISPLAY		(IPR_SET | IPR_DISPLAY)

/* reset and display ipforwarding and ipgateway */
#define IPR_RESETDISPLAY	(IPR_RESET | IPR_DISPLAY)

#define IPR_BUF_SIZE	256	/* buffer size */

char ebuf[IPR_BUF_SIZE];	/* error message buffer */
char myName[IPR_BUF_SIZE];	/* proccess name buffer */


main( argc, argv )
int argc;
char *argv[];
{
	char ch, *p;
	int flags, status;

	/* save a global copy of proc name */
	p = (char *)strrchr(argv[0], '/');
	if (p == (char *)0) {
		p = argv[0];
	} else {
		p++;
	}
	strcpy(myName, p);

	/* parse command line */
	flags = 0;
	while ((ch = getopt(argc, argv, "srd")) != EOF) {
		switch ((char)ch) {
			case 's':
				flags |= IPR_SET;
				break;
	        	case 'r':
				flags |= IPR_RESET;
				break;
	        	case 'd':
				flags |= IPR_DISPLAY;
				break;
			default:
				flags = -1;	/* invalid options */
				break;
		}
	}

	switch (flags) {
		case IPR_SET:
		case IPR_RESET:
			status = DoIprSet(flags);
			break;
		case IPR_SETDISPLAY:
		case IPR_RESETDISPLAY:
			status = DoIprSet(flags & ~IPR_DISPLAY);
			DoDisplay();
			break;
		case IPR_DISPLAY:
			status = DoDisplay();
			break;
		default:
			status = DoUsage();
			break;
	}

	exit(status);
}


/*
 * Give some help information.
 */
DoUsage()
{
	printf("usage: %s -s\n\
       %s -sd\n\
       %s -r\n\
       %s -rd\n\
       %s -d\n", myName, myName, myName, myName, myName);
	return(1);
}


/*
 * Set or Reset ipforwarding and ipgateway in the kernel.
 *
 * If value = IPR_SET, set ipforwarding and ipgateway to one.
 * If value = IPR_RESET, reset ipforwarding and ipgateway to zero.
 */
DoIprSet(value)
int value;
{
	int temp;

	/* bit 1 controls ipforwarding, bit 0 controls ipgateway */
	temp = (value == IPR_SET) ? 3 : 0;
	if (setsysinfo(SSI_IPRSETUP, (char *)&temp, sizeof(int), 0, 0) < 0) {
		sprintf(ebuf, "%s: setsysinfo(SSI_IPRSETUP)", myName);
		perror(ebuf);
		return(1);
	}
	return(0);
}


/*
 * Display ipforwarding and ipgateway.
 */
DoDisplay()
{
	int temp;

	/* bit 1 controls ipforwarding, bit 0 controls ipgateway */
	if (getsysinfo(GSI_IPRSETUP, (char *)&temp, sizeof(int), 0, 0) < 0) {
		sprintf(ebuf, "%s: getsysinfo(GSI_IPRSETUP)", myName);
		perror(ebuf);
		return(1);
	}
	printf("ipforwarding = %d\nipgateway = %d\n",
		(temp & 2) != 0, (temp & 1) != 0);
	return(0);
}
