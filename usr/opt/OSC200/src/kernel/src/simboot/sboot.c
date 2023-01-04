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
 * ISP Simulator boot program.
 *
 * 18-Oct-90	rjl
 *
 *	The purpose of this program is to setup the execution environment
 *	for a secondary program (vmunix) in the simulator environment. At
 *	this stage the assumption is made that the secondary program has
 *	been concatonated onto this program using `pmerge' and that the
 *	`prog_header' has been initialized to describe this secondary
 *	program.
 */
#include <sys/types.h>

#include "pmerge.h"

/*
 * The following structure is initialized with info from the secondary
 * program by pmerge.
 */
struct pheader prog_header;

extern char end;			/* end of us			*/

#define PGSIZE 8192
#define KSEGBASE ((unsigned long)(0xfffffc0000000000))
#define KERNBASE ((unsigned long)(0xffffffff00000000))
#define MAP 0
#define MOVE 1

main(stack)
long stack;				/* address of the stack		*/
{
	long spfn;			/* first available pfn		*/
	long ipfn;			/* first pfn of image		*/
	long npfns;			/* number to map		*/
	long isize;			/* total size of the image	*/
	long (*fun)();			/* function pointer		*/
	long ptbr=mfpr_ptbr();		/* contents of the page tbl br	*/

	printf("SAS Boot program\n");
	printf("Secondary program text=%lx data=%lx bss=%lx\n",
			prog_header.tsize,
			prog_header.dsize,
			prog_header.bsize);
	printf("                    va=%lx entry=%lx\n",
			prog_header.text_start, prog_header.entry);
	spfn = firstpfn();
	/*
	 * This image is loaded into the simulator using a physical
	 * load. The page tables may or may not map the entire image.
	 * We make the assumption that the image is in consecutive
	 * physical pages.  On a real system they would be mapped if
	 * the console code were running and we'd simply copy the
	 * pfn's to the new map. On the simulator a fixed number of
	 * pages are mapped, something less than what would hold a SAS
	 * kernel so we have to figure it out just like we would if
	 * we were doing the I/O ourself.
	 */

	isize = prog_header.tsize + prog_header.dsize + prog_header.bsize;
	npfns = (isize + PGSIZE)/PGSIZE;
	/*
	 * Map or move and execute the image
	 */
	if ( (unsigned long)prog_header.text_start < KSEGBASE ||
			(unsigned long)prog_header.text_start >= KERNBASE ){
		printf("Remapping image\n");
		spfn = mapimage( prog_header.text_start, spfn, npfns, prog_header.entry );
	} else {
		printf("Moving image\n");
		spfn = moveimage( prog_header.text_start, spfn, npfns );
	}
	printf("Starting image at %lx\n", prog_header.entry);
	fun = (void(*))prog_header.entry;
	(*fun)(spfn, ptbr, 0, 0, 0);
}
