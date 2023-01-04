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

#if !defined(COFF) && !defined(MACHO)
#error Must define at least one object file format (COFF or MACHO)
#endif

#include <sys/param.h>
#include <machine/cpu.h>
#include <machine/entrypt.h>

#include "load_image.h"

extern long prom_io;
extern int ub_argc;
extern char **ub_argv;
extern char **ub_envp;

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
#ifdef SECONDARY
#define	load_coff	load_bootstrap_image
#else
#if defined (COFF) && !defined(MACHO)
#define load_coff load_image
#endif
#endif /* SECONDARY */

int
load_coff(io,startimage)
int io;
int startimage;
{
	struct execinfo  ei;
	HDRR		*si;
	unsigned int	 i, j;
	
	if (read(io, &ei, sizeof(ei)) != sizeof(ei)) {
#ifdef DEBUG
		prom_putc("bad a.out format\n");
#endif
		goto bad;
	}
	if (N_BADMAG(ei.ah)) {
#ifdef DEBUG
		prom_putc("bad magic number\n");
#endif
		goto bad;
	}
	
	lseek(io, N_TXTOFF(ei.fh, ei.ah), 0);
	if (read(io, ei.ah.text_start, ei.ah.tsize) != ei.ah.tsize) {
#ifdef DEBUG
		prom_putc("short read\n");
#endif
		goto bad;
	}
	
	if (read(io, ei.ah.data_start, ei.ah.dsize) != ei.ah.dsize) {
#ifdef DEBUG
		prom_putc("short read\n");
#endif
		goto bad;
	}
	
	bzero(ei.ah.bss_start, ei.ah.bsize);
	
#ifdef notdef
	/*
	 * Read in symbols, if there are any and if they fit 
	 */
	bzero( ei.ah.bss_start + ei.ah.bsize, sizeof ei);
	
	if (ei.fh.f_symptr > 0) {
		
		/*
		 * Pass along all the information we have and are about
		 * to lose, e.g. the exec struct and symtab header
		 */
		si = (HDRR*) (ei.ah.bss_start + ei.ah.bsize + sizeof ei);
		
		lseek(io, ei.fh.f_symptr, 0);
		if ((i = read(io, si, ei.fh.f_nsyms)) != ei.fh.f_nsyms) {
#ifdef DEBUG
			prom_putc("short read\n");
#endif
			goto nosyms;
		}

		/* how much stuff is there */
		i = si->cbExtOffset + si->iextMax * cbEXTR;
		j = i - (ei.fh.f_symptr +  ei.fh.f_nsyms);

		if (read(io, (char*)si + ei.fh.f_nsyms, j) != j) {
#ifdef DEBUG
			prom_putc("Inconsistent symtable\n");
#endif
			goto nosyms;
		}

		bcopy( &ei, ei.ah.bss_start + ei.ah.bsize, sizeof ei);

	}
nosyms:
#endif

	/*
	 * if startimage not true, just return information to the caller so
	 * that the image may be later started
	 */
	if (!startimage) {
	    static struct image_return image_return;

	    image_return.entry = ei.ah.entry;
	    imb();
	    return((long)&image_return);
	}

	/* All done */
 	close(io);
	prom_close(prom_io);
	imb();
	(*((int (*) ()) ei.ah.entry)) (ub_argc,ub_argv,ub_envp);
bad:
	return(-1);
}

#endif /* COFF */
