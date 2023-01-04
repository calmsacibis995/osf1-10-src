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
static char     *sccsid = "@(#)$RCSfile: sizer.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/09/03 14:30:57 $";
#endif lint
/************************************************************************
 *									*
 *			Copyright (c) 1987 by				*
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
 * Name: sizer.c
 *
 *	Usage:
 *	sizer [-c][-r][-k kfile] [-n sysnam] [-t timezone]
#if ULT_BIN_COMPAT
 *	      [-wt][-wu][-wc][-gt]
#endif
 *
 *	-c	Write the cpu type (only) to standard output
 *	-r	Write the root device (only) to standard output
 *	-k	The next argument is an alternate kernel file name
 *	-n	The next arguement is the system name
 *	-t	The next argument is the "timezone" string for config
 *	-b	Write the booted kernel's filename to standard output
#if ULT_BIN_COMPAT
 *	-wt	Write workstation display type field to standard output
 *	-wu	Write workstation display units field to standard output
 *	-wc	Write console device (0=gfx, 1=alt) to standard output
 *	-wk	Write workstation keyboard name field to standard output
 *	-wp	Write workstation pointer name field to standard output
 * 	-gt	Write graphics display types to standard output 
 *		(NOTE : This obsoletes -wt on mips systems )
 *	-gr	Write graphics display resolution to standard output 
#endif
 *
 * Modification History
 *
 * Feb 14, 1992 - Joel Gringorten
 *	Added the -gt option for getting the names of the
 *	graphic controllers. 
 *
 * Jun 5, 1991 - jaa
 *	ported to OSF/1.  modified to use new bus/controller/device
 *	kernel structures.  in porting, we don't use /dev/kmem anymore.
 *	getsysinfo(3) was modified to return the needed information.
 *	NOTE: ifdef TODO's in this code will have to be done as the 
 *	pieces are added (i.e. floating csr space, workstation specifics)
 *
 * Oct 15, 1990 - Paul Grist
 *      Bugfix for error checking on sizer commands with arguments,
 *      the old check resulted in a seg fault on mips because of
 *      a null pointer situation.
 * 
 * Dec 15, 1989 - Alan Frechette
 *	Added 2 more options "-wt" and "-wu" to display the workstation
 *	display type field and the workstation display units field.
 *
 * May 10, 1989 - Alan Frechette
 *	Added an error message for the name list type "NL_roottype"
 *	which was added to determine if a network boot was performed.
 *
 * Feb 12, 1989 - Alan Frechette
 *	New sizer code which supports multiple architectures.
 *      This file is a complete redesign and rewrite of the 
 *	original V3.0 sizer code by Tungning Cherng.
 *
 ***********************************************************************/

#include "sizer.h"

char *Sysname;
char *Tzone;
char *Altfile;

main(argc, argv)
char **argv;
int argc;
{

	int kflag, tflag, gflag;
	char option;

	Tzone = Altfile = Sysname = NULL;

    	getargs(argv, argc, &option, &kflag, &tflag, &gflag);

#ifdef notdef
	nlist((kflag) ? Altfile : "/vmunix", nl);

	if((Kmem = open("/dev/kmem", 0)) < 0)
		quitonerror (-2);
#endif /* notdef */

	if(signal(SIGINT,SIG_IGN) != SIG_IGN)
		signal(SIGINT,SIG_IGN);

	switch(option) {
	      case 'n': 
		getconfig();
		break;
		
	      case 'b': 
		getbootedfile();
		break;
		
	      case 'c': 
		getcpu(DISPLAY);
		break;
		
	      case 'r': 
		getroot(DISPLAY);
		break;
		
#if	ULT_BIN_COMPAT
	      case 'w':
		getwsinfo(tflag);
		break;

              case 'g':
		getgraphicinfo(gflag);
		break;
#endif /* ULT_BIN_COMPAT */
		
	      default: 
		break;
	}  
	exit(0);
}

/****************************************************************
*								*
* getargs 							*
*								*
* Parse the command line arguments.				*
*								*
****************************************************************/
getargs(argv, argc, option, kflag, tflag, gflag)
char **argv;
int argc;
char *option;
int *kflag;
int *tflag;
int *gflag;
{

	int k;
	int error;

	*kflag = *option = error = 0;
	for(k=1; (k<argc && !error); k++) {
	    	switch(argv[k][1]) {
		case 'k': 	/* -k: use alternate file */
			k++;
			*kflag = 1;
			if(argc == 2)
				error++;
	    		else
		    		Altfile = argv[k];
	    		break;

		case 'n': 	/* -n: system name */
	    		k++;
			*option = 'n';
	    		if(argc == 2)
				error++;
	    		else
		    		Sysname = argv[k];
	   		break;

		case 'r': 	/* -r: display root device */
			*option = 'r';
			break;

		case 'b': 	/* -b: display booted kernel filename */
			*option = 'b';
			break;

		case 'c': 	/* -c: display cpu type */
			*option = 'c';
			break;

		case 'g':	/* -g: return graphics controller info */
			*option = 'g';
			*gflag = 0;
			if(argv[k][2] == 't')
				*gflag = 1;
			else if (argv[k][2] == 'r')
			        *gflag = 2;
			else
				error++;
			break;

		case 't':	/* -t: get time zone for -n */
			k++;
			if(argc == 2)
				error++;
			else
				Tzone = argv[k];
			break;

		case 'w':	/* -w: get workstation display info */
			*option = 'w';
			if(argv[k][2] == 't')
				*tflag = 1;
			else if(argv[k][2] == 'u')
				*tflag = 2;
			else if(argv[k][2] == 'c')
				*tflag = 3;
			else if(argv[k][2] == 'k')
				*tflag = 4;
			else if(argv[k][2] == 'p')
				*tflag = 5;
			else
				error++;
			break;

		default: 
			error++;
			break;
		}
	}
	if(error || argc == 1) {
		usage();
		exit(1);
	}
}

/****************************************************************
* quitonerror							*
*								*
* If an error occurs control is passed to this routine to 	*
* allow for a graceful exit.					*
*								*
****************************************************************/
quitonerror(code)
int code;
{

	switch(code) {
	case -1: 
		fprintf(stderr, "No namelist found.\n");
		break;

	case -2: 
		fprintf(stderr, "Cannot read kernel memory.\n");
		break;

	case -3: 
		fprintf(stderr, "Too many errors (ABORTING).\n");
		break;

	case -4: 
		fprintf(stderr, "Cannot get the cpu information.\n");
		break;

	case -5:
		fprintf(stderr, 
			"Cannot get the root, swap, or dump information.\n");
		break;

	case -6: 
		fprintf(stderr, "Cannot get the timezone information.\n");
		break;

	case -7: 
		fprintf(stderr, "Cannot get the nvram size information.\n");
		break;

	case -9:
		fprintf(stderr, "Cannot read UNIBUS/QBUS memory.\n");
		break;

	case -10:
		fprintf(stderr, "Cannot create the MAKEDEVICES File.\n");
		break;

	case -11:
		fprintf(stderr, "Cannot create the CONFIG File.\n");
		break;

	case -13:
		fprintf(stderr, "Cannot get the workstation display type.\n");
		break;

	case -14:
		fprintf(stderr, "Cannot get the roottype information.\n");
		break;

	default: 
		break;
	}
	exit(-1);
}

/****************************************************************
*								*
* usage 							*
*								*
* Display usage message.					*
*								*
****************************************************************/
usage()
{

	fprintf(stderr,"\nUsage: sizer\n");
	fprintf(stderr,"-c\t\t Returns cpu type.\n");
	fprintf(stderr,"-r\t\t Returns root device.\n");
	fprintf(stderr,"-k image\t Use image instead of /vmunix.\n");
	fprintf(stderr,"-n name \t Create a config file using this name.\n");
	fprintf(stderr,"-t timezone\t Use timezone in the config file.\n");
	fprintf(stderr,"-b\t\t Returns the booted kernel's filename.\n");

#if	ULT_BIN_COMPAT
	fprintf(stderr,"-wt\t\t Returns workstation display type.\n");
	fprintf(stderr,"-wu\t\t Returns workstation display units.\n");
	fprintf(stderr,"-wc\t\t Returns workstation console selected.\n");
	fprintf(stderr,"-wk\t\t Returns workstation keyboard name.\n");
	fprintf(stderr,"-wp\t\t Returns workstation pointer name.\n");
	fprintf(stderr,"-gt\t\t Returns graphics controller information.\n");
	fprintf(stderr,"-gr\t\t Returns graphics controller resolution.\n\n");
#endif /* ULT_BIN_COMPAT */
}
