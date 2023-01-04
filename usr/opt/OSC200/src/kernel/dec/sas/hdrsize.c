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
static char *rcsid = "@(#)$RCSfile: hdrsize.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1992/11/24 15:52:47 $";
#endif
/* 
 *
 * NAME: hdrsize
 *
 * USAGE: hdrsize image
 *
 * DESCRIPTION: Determine the size of the input image's COFF header.
 *              Useful for calculating the number of bytes of header to
 *              strip off a standalone program. 
 *
 * Author: Gerri Harter
 * Creation: 11-09-92
 * 
 */

#include <a.out.h>

struct exec {
        struct filehdr  ex_f;
        struct aouthdr  ex_o;
};

main(argc, argv)
int argc;
char **argv;
{
        int fd, i;
        int hdrsize;
        struct exec ex;

        if ((fd = open(argv[1], 2)) < 0) {
                printf("hdrsize: can't open %s\n", argv[1]);
                exit(1);
        }

        if ((i = read(fd, (char *)&ex, sizeof(ex))) != sizeof(ex)) {
                printf("hdrsize: can't read %s, size read is %d\n", argv[1], i);
                exit(1);
        }

        hdrsize = N_TXTOFF(ex.ex_f, ex.ex_o);

#ifdef __alpha
	/* This program will be used to analyze an Alpha image */
	if (!IS_ALPHAMAGIC(ex.ex_f.f_magic)) {
		/* 
		 * The Alpha header template used by this program is different
		 * from the Alpha header on the image being analyzed.  
		 * If this happens, make sure that the "a.out.h" file included 
		 * by this program is the same as the version used to create 
		 * the input image.
		 */
                printf("hdrsize: Magic number mismatch (image ALPHAMAGIC = 0%o)\n", ex.ex_f.f_magic);
                exit(1);
	}
#endif /* __alpha */
#ifdef __mips
	/* NOTE: at the time this program was written, the Makefile was
	 * not set up to handle the case where the host and target machine
	 * are MIPS.  Once the Makefile works in a Multiarchitecture 
	 * environment, remove the following print() and exit statements so 
	 * the code to handle MIPs image gets executed.
	 */
         printf("hdrsize: Handling of Mips objects not supported\n"); 
         exit(1);

	/* This program will be used to analyze a MIPs image */
	if (!IS_MIPSELMAGIC(ex.ex_f.f_magic)) {
		/* 
		 * The Mips header template used by this program is different
		 * from the Mips header on the image being analyzed.  
		 * If this happens, make sure that the "a.out.h" file included 
		 * by this program is the same as the version used to create 
		 * the input image.
		 */
                printf("hdrsize: Magic number mismatch (image MIPSMAGIC = 0%o)\n", ex.ex_f.f_magic);
                exit(1);
	}
#endif /* __mips */

        printf("%d", hdrsize);
}

