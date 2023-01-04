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

#include "pmerge.h"

/* Need Ken's new version of the this header file with alpha coff format */
#include "new_aouthdr.h"

/*
 * Load an image written in mips coff format
 */
struct pheader *
load_ultrix_coff(aout_file)
char   *aout_file;
{
	int     fd;
	struct new_aouthdr exec;		/* aux header		*/
	struct new_filehdr fhdr;		/* coff file header	*/
	struct new_scnhdr  shdr;		/* section header	*/
	struct pheader *phdr;		/* program header	*/
	char  *bptr, *cptr;
	int i,len;
	int lastaddr;
	long mem_req;
	int logging = 1;

	if ((fd = open (aout_file, 0)) == -1) {
		printf ("Cannot open %s\n", aout_file);
		exit (1);
	}

	/* Read the file header information */

	i = read (fd, &fhdr, sizeof (struct new_filehdr));

	if (i != sizeof (struct new_filehdr) || fhdr.f_magic != ALPHAMAGIC ){
		printf ("%s is not an executable file\n", aout_file);
		close (fd);
		exit (1);
	}
	/*
	printf("No of sections = %d\n", fhdr.f_nscns);
	*/

	/*
	 * read the aouthdr and check for the right type of
	 * file.
	 */
	i = read (fd, &exec, sizeof (struct new_aouthdr));

	if( exec.magic != OMAGIC ){
		printf("%s is not an OMAGIC file, relink using -N\n", 
			aout_file);
		close (fd);
		exit (1);
	}
	/*
	printf("text	data	bss\n%lx\t%lx\t%lx\n%ld\t%ld\t%ld\ngp = %lx\n\n",
		exec.text_start, exec.data_start, exec.bss_start,
		exec.tsize, exec.dsize, exec.bsize, exec.gp_value);
	*/
	/*
	 * Is there enough memory to load ?
	 */
	mem_req = exec.tsize+exec.dsize+exec.bsize;
	if( mem_req > MAX_MEMORY ){
		printf( "Image file is too large for isp memory\n");
		exit (1);
	}

	bptr = (char *)malloc((int)(mem_req+PAGSIZ));
	printf("Image requires %ld bytes\n", mem_req);
	if( !bptr ){
		printf("can't allocate memory\n");
		exit(1);
	}

	/*
	 * Read the section headers
	 */
	/*
	for( i=0 ; i<fhdr.f_nscns ; i++ ){
		len = read (fd, &shdr, sizeof (struct new_scnhdr));
		printf("section name = %s\n", shdr.s_name);
		printf("\tvaddr=%lx size=%ld(%lx)\n\n",
			shdr.s_vaddr, shdr.s_size, shdr.s_size);
	}
	*/

	/* Setup the start PC */

	lseek (fd, (int)NEW_C_TXTOFF(fhdr,exec), 0);

	/*
	 * Read the text
	 */
	/*
	printf("Reading text at %p\n",bptr);
	*/
	i = read (fd, bptr, (int)exec.tsize);
	if( logging )
		printf ("Loading %s\n\ttext at %16lx\tlen=%lx\n",
			aout_file, exec.text_start, exec.tsize);

	/*
	 * Read the data
	 */
	cptr = bptr+exec.tsize;

	/*
	printf("reading data at %p\n",cptr);
	*/
	i = read (fd, cptr, (int)exec.dsize);
	if( logging )
		printf ("\tdata at %16lx\tlen=%lx\n", exec.data_start, exec.dsize);

	close (fd);

	/*
	 * Zero out bss
	 */
	if( logging )
		printf ("\tbss  at %16lx\tlen=%x\n", exec.bss_start, exec.bsize);
	cptr += exec.dsize;
	for (i = 0; i < exec.bsize; i++)
		*cptr++ = 0;

	if( logging )
		printf("\tentry at %16lx\n", exec.entry);

	/*
	 * Fill in the program header info and pass back to caller
	 */
	phdr = (struct pheader *)malloc(sizeof(struct pheader));
	if( !phdr ){
		printf("can't allocate memory\n");
		exit(1);
	}
	phdr->tsize = exec.tsize;
	phdr->dsize = exec.dsize;
	phdr->bsize = exec.bsize;
	phdr->text_start = exec.text_start;
	phdr->data_start = exec.data_start;
	phdr->bss_start = exec.bss_start;
	phdr->entry = exec.entry;
	phdr->image = (long)bptr;
	return phdr;
}
