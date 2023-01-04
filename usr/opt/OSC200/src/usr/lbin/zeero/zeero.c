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
static char *rcsid = "@(#)$RCSfile: zeero.c,v $ $Revision: 4.1.3.4 $ (DEC) $Date: 1992/10/13 14:20:39 $";
#endif
/*
 *	zeero.c
 *		zero out disks prior to rewrite.
 *
 *  History:							
 *								
 *	000	01-Feb-1988						
 *		Created by Jon Wallace					
 *
 *	001	19-Sep-1991	ech
 *		ported to OSF/1
 *
 *	002	30-Sep-1991	Tom Tierney
 *		Fix for OSF/Tin: updated to check return byte count
 *		rather than -1 for write routine.  NOTE: status returned
 *		when end of partition is reached will in some cases be
 *		EIO and others ENOSPC.  This Tin peculiarty will be fixed
 *		to behave like ULTRIX and return ENOSPC for all cases.
 *
 *      003     17-Oct-1991     ech
 *              call ioctl  DIOCWLABEL to make disklabel sector writable. 
 *		calculate # of full transfers and size of last transfer
 *              add more error checking.
 *								
 */

#include	<errno.h>
#include	<stdio.h>
#include	<string.h>
#include	<sys/file.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/disklabel.h>
#include	<io/common/devio.h> 
#include	<sys/ioctl.h>

#define		ON		1
#define		OFF		0
#define		XFERSIZE	65536	/* 64K */	
#define		YES		'Y'
#define		NO		'N' 

extern		int		errno;
extern 		int      	optind;                 /* getopt(3) */



/* **********************
Program ZEERO starts here
************************* */

main(argc,argv)

	int 	argc;
	char	*argv[];
{




/* ************** 
Declare variables 
***************** */


	char		buffer[XFERSIZE];
	char		*dev;
	char		*prog;
	int 		fp;
	int		xstat;
	int		fflag = OFF;
	int		stat;
	int		write_flag;
	struct		stat	status;
	int             c;              /* for use w/getopt(3) */
	char		str[BUFSIZ], answer[BUFSIZ]; /* User input */
	char		ans = ' '; 
	short		debug = 0;
	int		i = 0;

/* ******* 
Initialize 
********** */

	prog = *argv;
	bzero (buffer, XFERSIZE);

        while( (c = getopt( argc, argv, "df" )) != EOF )
        {
                switch( c )
                {
			case 'd':
				debug ++;
				break; 
                	case 'f':
				fflag = ON;
                        	break;
                	default:
			{
				fprintf (stderr,"usage: zeero [ -d ] [ -f ] device \n");
                        	exit(1);
			}
                }
                --argc;
        }

	if (argv[optind] == '\0')
	{
		fprintf (stderr, "usage: zeero [-f] device \n");
		exit (1);
	}
	else
		dev = argv[optind];	
	
	if ((fp = open (dev, O_RDWR, 0)) < 0)
	{
		fprintf (stderr, 
			  "%s: failed to open device %s. \n", prog, dev);
		exit (1);
	} 		




/* ***************************************************************** 
Check status of file input by user to make sure it is a SPECIAL FILE
******************************************************************** */

	fstat(fp, &status);

	if (errno != 0)
	{
		fprintf (stderr, 
			  "%s: failed to stat device %s. \n", prog, dev);
		exit(1);
	}
	else
	{ 
		xstat = (status.st_mode & S_IFMT); 
		if (xstat != S_IFCHR) 
		{ 
			fprintf (stderr, 
			  "%s: %s is not a valid device. Please specify a character special file.\n", prog, dev);
			exit(1);
		}
	}




/* *****************************************************
Check to see if the Force (-f) flag is on or off.  If it
is off, this program is being used interactively, so ask
the user to confirm that they want to zero out the disk.
******************************************************** */

	if (fflag == OFF)
	{
		while (ans != YES) 	
		{
			printf ("\n\n******** W A R N I N G ******** \n\nThis program will completely erase all data on %s.\n\nAre you sure you want to do this (y/n): ", dev); 
			fflush (stdout);
			gets (str);
			sscanf (str, "%s", answer);
			if ((ans = toupper (*answer)) == NO) 
				exit (0);
		}
	}

/****************************************************** 
Before zeeroing out the disk, first make the disklabel
sector writable so it can be cleaned too. Then calculate
how m
 
******************************************************* */

	printf ("\nCleaning %s.....", dev);
	fflush (stdout);

	/* make the disklabel sector writable */
	write_flag = 1;
	if (ioctl(fp, DIOCWLABEL, &write_flag) < 0)
	{
		fprintf (stderr, "%s: can not write over label sector.\n",
			 prog);
		exit (1);
	}

	/* start writing zeros till we get an error then check the
	   error to make sure it is ENOSPC (28) from errno.h, which
	   means the device ran out of space */

	/* i = space in /etc/disktab entry * 512 / 65536 (+1) */ 
	if (debug)
	{ 
		while ( write (fp, buffer, XFERSIZE) > 0 )
        	{
			i++;
        	}
		printf ("number of writes = %d, errno = %d \n", i, errno);
	}
	else
	{
		while ( write (fp, buffer, XFERSIZE) > 0 )
			;
	}

        if (errno == ENOSPC)
        {
                printf ("done. \n");
                exit(0);
        }
        else
                perror("write");

	close (fp);
}

