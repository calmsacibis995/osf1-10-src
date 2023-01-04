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
static char	*sccsid = "@(#)$RCSfile: loadmappedimage.c,v $ $Revision: 1.1.2.15 $ (DEC) $Date: 1992/12/22 14:54:46 $";
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

#if !defined(COFF) && !defined(MACHO)
#error Must define at least one object file format (COFF or MACHO)
#endif

#include <sys/param.h>
#include <machine/cpu.h>
#include <machine/entrypt.h>
#include <machine/rpb.h>
#include <alpha/load_image.h>

/* globals */
extern long palmode;

#ifdef NETLOAD
#define read alpha_tftp_read
extern int tot, sofar, spin, check; 	/* For %loaded */
#endif /* NETLOAD */

extern int ub_argc;
extern char **ub_argv;
extern int gub_argc;
extern char **gub_argv;
extern char **ub_envp;

#define	PGSIZE 8192

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
long
load_coff(io,startimage)
int io;
int startimage;
{
	struct execinfo  ei;
	HDRR		*si;
	unsigned int	 i, j;
	long spfn;                      /* first available pfn          */
	long ipfn;                      /* first pfn of image           */
	long npfns;                     /* number to map                */
	long isize;              
	struct rpb *rpb = (struct rpb *)HWRPB_ADDR; /* the rpb		*/
	static long ptbr;		/* pagetable base register	*/

	if (palmode != 1) {
		ptbr = mfpr_ptbr();	/* pagetable base register	*/
	}

#ifdef NETLOAD
	sofar = 0;	/* Need to initialize before calling alpha_tftp_read */
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

	spfn = firstpfn();
	isize = ei.ah.tsize + ei.ah.dsize + ei.ah.bsize;
	npfns = (isize + PGSIZE)/PGSIZE;
	/*
	 * Map the image
	 */
	spfn = mapimage( ei.ah.text_start, spfn, npfns, ei.ah.entry );

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

	prom_putc("\nPercentage loaded:   0  ");
#endif /* NETLOAD */

	if (read(io, ei.ah.text_start, ei.ah.tsize) != ei.ah.tsize) {
		printf("short read (text)\n");
		goto bad;
	}

#ifdef SECONDARY
	printf ("data = %d\n", ei.ah.dsize);
#endif /* SECONDARY */
	if (read(io, ei.ah.data_start, ei.ah.dsize) != ei.ah.dsize) {
		printf("short read (data)\n");
		goto bad;
	}

#ifdef SECONDARY
	printf ("bss  = %d\n", ei.ah.bsize);
#endif /* SECONDARY */
	bzero(ei.ah.bss_start, ei.ah.bsize);

#ifdef NETLOAD
	prom_putc("\b\b\b\b100  ");
	prom_putc("\n");
#endif /* NETLOAD */

#ifdef notdef
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
			printf("short read (symtab)\n");
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
#endif

#ifdef NETLOAD
	return(ei.ah.entry);
#endif /* NETLOAD */
#ifdef SECONDARY

	/* 
	 * This field is set by kdebug.  It informs the kernel that
	 * kdebug is loaded if it is non zero.
	 */
	rpb->rpb_software = 0;	

	/*
	 * if startimage not true, just return information to the caller so
	 * that the image may be later started
	 */
	if (!startimage) {
	    static struct image_return image_return;

	    image_return.entry = ei.ah.entry;
	    image_return.spfn = ++spfn;
	    image_return.ptbr = ptbr;
	    return((long)&image_return);
	}

	printf ("Starting at 0x%lx\n\n", ei.ah.entry);
#endif /* SECONDARY */
	close(io);
	{
		extern prom_io;
		if(prom_io >= 0)
			prom_close(prom_io);
	}
	imb();
	(*((int (*) ()) ei.ah.entry)) 
			(++spfn,ptbr,ub_argc,ub_argv,ub_envp, ei.ah.entry);
bad:
	return(-1);
}

/*map_mop_image (buffer, addr, aoutadj, bssaddr, bssize)
 *
 *function:
 *    Map an image about to be downloaded
 *
 *inputs:
 *    buffer - contains coff header
 *
 *outputs:
 *    addr - text start address 
 *    aoutadj - text offset, from start of buffer
 *    bssaddr - bss start address 
 *    bssize - size of bss section (to be zeroed)
 *
 *returns:
 *    image pointer 
 *
 */
#ifdef	NETLOAD
long
map_mop_image(buffer, addr, aoutadj, bssaddr, bssize)
unsigned buffer[];
long *addr;
int *aoutadj;
long *bssaddr;
int *bssize;
{
	static struct image_return image_return;
	struct execinfo  ei;
	long spfn;                      /* first available pfn          */
	long ipfn;                      /* first pfn of image           */
	long npfns;                     /* number to map                */
	long isize;
	long entryaddr;
	long ptbr;

	bcopy(buffer, &ei, sizeof(ei));
	if (N_BADMAG(ei.ah)) {
		printf("load_mop_image: image contains bad magic number\n");
		return(-1); 
	}
	if (ei.fh.f_symptr > 0) {
		printf("load_mop_image: image not stripped\n");
		return(-1); 
	}
	entryaddr = ei.ah.entry;
	isize = ei.ah.tsize + ei.ah.dsize + ei.ah.bsize;
	npfns = (isize + PGSIZE)/PGSIZE;
	if (palmode != 1) {
		/* pagetable base register */
		ptbr = mfpr_ptbr(); 
	}

	spfn = firstpfn();
	/*
	 * Map the image
	 */
	spfn = mapimage( ei.ah.text_start, spfn, npfns, ei.ah.entry );
	/* Some safety checks */
	if (ei.ah.text_start + ei.ah.tsize != ei.ah.data_start) {
		printf("load_mop_image: bad image - data does not follow text\n");
	}

	image_return.entry = ei.ah.entry;
	image_return.spfn = spfn;	/* bump before calling entry */
	image_return.ptbr = ptbr;
	*addr = ei.ah.text_start;
	*aoutadj = N_TXTOFF(ei.fh, ei.ah);
	*bssaddr = ei.ah.bss_start;
	*bssize = ei.ah.bsize;
	return((long)&image_return);
}

/*align_netblk (bufaddrstart, bufaddrend, netblksz)
 *
 *function:
 *    Calculate address within bufaddr that a structure of netblksz can be
 *    stored within a single physical page.
 *
 *inputs:
 *    bufaddrstart - starting address of a buffer
 *    bufaddrend - ending address of a buffer
 *    netblksz - the size of the structure to locate within buffer
 *
 *outputs:
 *    NONE
 *
 *returns:
 *    address to store a netblksz sized structure within a single page
 */
char *
align_netblk(bufaddrstart, bufaddrend, netblksz)
long bufaddrstart;
long bufaddrend;
int netblksz;
{

	long pgshift;
	long bufstartpg;	/* address of the page in which the buffer 
				 * begins */
	long netblkaddrend;	/* address of the end of the netblk, if the
				 * netblk were put at the start of the buf */
	long netblkendpg;	/* address of the page in which the end of 
				 * netblk, if it were to be put at the start
				 * of the buffer 
				 */
	char *sptr;		

	if (netblksz > PGSIZE) {
		printf ("align_netblk: netblksz larger than 1 page\n");
		stop();
	}
	pgshift = msb((unsigned long)PGSIZE);
	bufstartpg = ((long)bufaddrstart >> pgshift) << pgshift;

	netblkaddrend = (long)bufaddrstart + netblksz;
	netblkendpg = (netblkaddrend >> pgshift) << pgshift;
	if (bufstartpg != netblkendpg) {
		/* struct would span a page boundary, so start it at the
		 * page break */
		sptr = (char *)netblkendpg;
	} else {
		/* struct won't span a page boundary if we start it at the
		 * beginning of the bufaddr.
		 */
		sptr = (char *)bufaddrstart;
	}
	/* safety check */
	if ( ((long)sptr + netblksz) > bufaddrend) {
		printf ("align_netblk: buffer too small\n");
		stop();
	}
	return (sptr);
}
#endif	/* NETLOAD */
#endif /* COFF */
