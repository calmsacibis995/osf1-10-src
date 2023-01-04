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
 * Merge two images for execution on the ISP simulator
 *
 * The ISP simulator by default loads programs at 0x20000 with unity
 * mapping of virtual and physical addresses.
 *
 * The purpose of this program is to take a bootstrap loader image
 * and combine it with another program (vmunix) in such a way that
 * the first program has the info necessary to setup mapping for the
 * second program.
 *
 * 18-Oct-90	rjl
 *
 */

#if 0
/* Need the stdio.h that goes with the mcc (mips64) compiler */
#include "/usr/projects/alpha/Ultrix.mips64/usr/include/stdio.h"

/* Need the nlist.h that goes with the acc (alpha) compiler */
#include "/usr/projects/alpha/wp/include/nlist.h"
#else

#include "stdio.h"
#include "nlist.h"

#endif

#include "pmerge.h"

/*
 * This definition of exec is for the GNU a.out module and comes
 * from a_out.h.  The file could not be included as there is it also
 * a definition of the GNU nlist structure which conflicts with the
 * MIPS COFF format.
 */
#define OMAGIC 0407

struct exec {
	int	a_magic;		/* number identifies as .o  */
	unsigned a_text;		/* length of text, in bytes */
	unsigned a_data;		/* length of data, in bytes */
	unsigned a_bss;			/* length of uninitialized data area */
	unsigned a_syms;		/* length of symbol table data  */
	unsigned a_entry;		/* start address */
	unsigned a_trsize;		/* length of relocation info for text*/
	unsigned a_drsize;		/* length of relocation info for data*/
};

extern struct pheader *load_ultrix_coff();	/* image loader	*/
struct nlist nl[2];

struct pheader prog_header;

main(argc, argv)
int argc;
char **argv;
{
	struct pheader *boot,*prog;	/* headers for each image	*/
	struct pheader *bhdr;		/* header in the boot image	*/
	int len, len2, tlen;
	FILE *fd;			/* output file descriptor	*/
	struct exec exec;		/* gnu type exec		*/

	if( argc != 4 ){
		printf("usage: %s bootstrap vmunix output\n", argv[0]);
		exit();
	}

	/*
	 * Read the two images into memory
	 */
	boot = load_ultrix_coff( argv[1] );
	prog = load_ultrix_coff( argv[2] );

	/*
	 * If either of the pointers is NULL we can't go on
	 */
	if( !(boot && prog) )
		exit(1);
	/*
	 * Locate the prog_header structure in the boot image
	 */
	nl[0].n_name = "prog_header";
	nlist(argv[1], nl);
	if( nl[0].n_type == 0 ){
		printf("Can't find program header in boot program\n");
		exit(1);
	}
	/*
	printf("prog_header located at %lx type=%x\n", nl[0].n_value,
		nl[0].n_type);
	*/
	/*
	 * Save the vmunix header inside of the boot image
	 *
	 * The nl[0].n_value is the absolute address in the original image.
	 * It's relative location in our memory is the address - the start
	 * of the text space.
	 */
	bhdr = (struct pheader *) (boot->image+nl[0].n_value-boot->text_start);
	*bhdr = *prog;

	/*
	 * compute len of first prog and round to next page if it's
	 * not already there
	 */
	len = ((boot->tsize+boot->dsize+boot->bsize+PAGSIZ)/PAGSIZ) * PAGSIZ;
	/*
	 * compute length of second program
	 */
	len2 = prog->tsize + prog->dsize + prog->bsize;

	/*
	 * Now fixup the image pointer inside the header to be relative to
	 * the boot program in memory so that the boot program will know
	 * where the start of the second program is.
	 */
	bhdr->image = boot->text_start+len;

	/*
	 * Simulator loads VMS EXEC, Ultrix COFF and GNU a.out. Of the
	 * three choices GNU a.out is the simplest. Fill out a header and
	 * write to the output file.
	 */
	exec.a_magic = OMAGIC;
	exec.a_text = len;
	exec.a_data = len2;
	exec.a_bss = exec.a_syms = exec.a_trsize = exec.a_drsize = 0;
	exec.a_entry = boot->entry;

	/*
	 * now write out the results of both files
	 */
	if( (fd=fopen(argv[3], "w")) == NULL ){
		printf("Couldn't create %s\n", argv[3]);
		exit(1);
	}

	/*
	 * Write the header and both images of the programs
	 */
#ifdef notyet
	if( fwrite(&exec, 1, sizeof(struct exec), fd) != sizeof(struct exec) ) {
		printf("Couldn't write header into %s\n", argv[3]);
		exit(1);
	}
#endif
	if( (tlen=fwrite((char *)boot->image, 1, len, fd)) != len ){
		printf("Couldn't write %s into %s\n", argv[1], argv[3]);
		printf("Requested %d wrote %d\n", len, tlen);
		exit(1);
	}
	if( (tlen=fwrite((char *)prog->image, 1, len2, fd)) != len2 ){
		printf("Couldn't write %s into  %s\n", argv[2], argv[3]);
		printf("Requested %d wrote %d\n", len, tlen);
		exit(1);
	}
	fclose(fd);
	exit(0);
}
