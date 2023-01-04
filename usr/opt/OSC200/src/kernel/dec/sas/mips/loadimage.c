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
static char	*sccsid = "@(#)$RCSfile: loadimage.c,v $ $Revision: 4.3.4.3 $ (DEC) $Date: 1992/05/05 13:38:42 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * loadimage.c
 */

/* 
 * derived from loadimage.c	3.1	(ULTRIX/OSF)	2/28/91";
 */

/*
 * Revision History:
 *
 * 05-Mar-1991 -- Don Dutile
 *	Removed MACHO routines.
 *	Merged v4.2 rex console related changes into osf's loadimage.c.
 */

#if !defined(COFF) && !defined(MACHO)
#error Must define at least one object file format (COFF or MACHO)
#endif

#include "../../../sys/param.h"
#include <machine/cpu.h>
#include <machine/entrypt.h>

#define printf _prom_printf
#ifdef NETLOAD
#define read pmax_tftp_read
#define lseek _prom_lseek
extern int tot, sofar, spin, check; 	/* For %loaded */
#else
extern int ub_argc;
extern char **ub_argv;
extern int gub_argc;
extern char **gub_argv;
#endif
extern int prom_io;
extern char **ub_envp;
extern char **ub_vector;
extern int rex_base;

/**************************************************************************
 *STARTCOFF- start of COFF specific information                           *
 *                                                                        *
 **************************************************************************/
#if defined(COFF)
#include <a.out.h>

/*
 * format of bootable a.out file headers
 */
struct execinfo {
  struct filehdr fh;
  AOUTHDR ah;
};


/*
 * getxfile -- load binary image
 */

/*load_coff (load_image)
 *
 *function:
 *    Load a coff format image
 *
 *inputs:
 *    io - input file descriptor
 *
 *outputs:
 *    NONE
 *
 *returns:
 *    NONE
 *
 */
#if defined (COFF) && !defined(MACHO)
#define load_coff load_image
#endif
int
load_coff(io,startimage)
int io;
int startimage;
{
	struct execinfo  ei;
	HDRR		*si;
	unsigned int	 i, j;
#ifdef NETLOAD
	sofar = 0;	/* Need to initialize before calling pmax_tftp_read */
	spin = 1;
	check = 400;	/* Set to high value...needed in tftp_read */
#endif /* NETLOAD */
	if (read(io, &ei, sizeof(ei)) != sizeof(ei)) {
		printf("bad a.out format\n");
		goto bad;
	}
	if (N_BADMAG(ei.ah)) {
		printf("bad magic number\n");
		goto bad;
	}

	lseek(io, N_TXTOFF(ei.fh, ei.ah), 0);

#ifdef SECONDARY
	printf ("\nSizes:\ntext = %d\n", ei.ah.tsize);
#endif /* SECONDARY */
#ifdef NETLOAD
	tot = (ei.ah.tsize + ei.ah.dsize);
	sofar = 0;	/* Reinit the variables */
	spin = 1;
        /*
         * number of bytes per 1% of image loaded
         * transfer 1024 bytes per packet for pmax
         */
        check = (tot/(100*1024));

	printf("\nPercentage loaded:   0  ");
#endif /* NETLOAD */

	if (read(io, ei.ah.text_start, ei.ah.tsize)
	    != ei.ah.tsize) {
		printf("short read1\n");
		goto bad;
	}

#ifdef SECONDARY
	printf ("data = %d\n", ei.ah.dsize);
#endif /* SECONDARY */
	if (read(io, ei.ah.data_start, ei.ah.dsize)
	    != ei.ah.dsize) {
		printf("short read2\n");
		goto bad;
	}

#ifdef SECONDARY
	printf ("bss  = %d\n", ei.ah.bsize);
#endif /* SECONDARY */
	bzero(ei.ah.bss_start, ei.ah.bsize);

#ifdef NETLOAD
	printf("\b\b\b\b100  ");
	printf("\n");
#endif /* NETLOAD */

#if !MACH_KDB
	/*
	 * Only KDB needs the kernel's symbols loaded.
	 */
	goto nosyms;
#endif MACH_KDB

	/*
	 * Read in symbols, if there are any and if they fit 
	 */
	bzero( ei.ah.bss_start + ei.ah.bsize, sizeof ei);

	if (ei.fh.f_symptr > 0) {
#ifdef	SECONDARY
		printf("symtab = ");
#endif	/* SECONDARY */

		/*
		 * Pass along all the information we have and are about
		 * to lose, e.g. the exec struct and symtab header
		 */
		si = (HDRR*) (ei.ah.bss_start + ei.ah.bsize + sizeof ei);

		lseek(io, ei.fh.f_symptr, 0);
		if ((i = read(io, si, ei.fh.f_nsyms)) != ei.fh.f_nsyms) {
			printf("short read\n");
			goto nosyms;
		}

		/* how much stuff is there */
		i = si->cbExtOffset + si->iextMax * cbEXTR;
		j = i - (ei.fh.f_symptr +  ei.fh.f_nsyms);

		if (read(io, (char*)si + ei.fh.f_nsyms, j) != j) {
			printf("Inconsistent symtable\n");
			goto nosyms;
		}

		bcopy( &ei, ei.ah.bss_start + ei.ah.bsize, sizeof ei);

#ifdef	SECONDARY
		printf("%d\n", i - ei.fh.f_symptr);
#endif	/* SECONDARY */
	}
nosyms:
#ifdef NETLOAD
	return(ei.ah.entry);
#else

	/* All done */
	if(!rex_base)
		_prom_close(prom_io);

#ifdef SECONDARY
	/* if loading kdebug then just return entry point to kernel */
	if (!startimage)
	    return(ei.ah.entry);

	printf ("Starting at 0x%x\n\n", ei.ah.entry);
	if(rex_base) {
	        ub_argc = gub_argc;	/* gub vars assigned in machboot */
	        ub_argv = gub_argv;
	}
#endif /* SECONDARY */
	(*((int (*) ()) ei.ah.entry)) (ub_argc,ub_argv,ub_envp,ub_vector);
#endif /* NETLOAD */
bad:
	return(-1);
}

#endif /* COFF */


/*************************************************************************
 *ENDCOFF - End of COFF specific code                                    *
 *                                                                       *
 *************************************************************************/

