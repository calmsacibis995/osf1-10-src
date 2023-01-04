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
static char *rcsid = "@(#)$RCSfile: mopify.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1992/10/30 15:20:14 $";
#endif
/* file:	mopify.c
 *
 *  MODULE DESCRIPTION:
 *
 *  Add a header to a file to make it MOP loadable
 *
 * Author:Philippe KLEIN
 * creation: 06-Nov 1991
 *
 */

#ifdef __mips64
#include "/usr/projects/alpha/Ultrix.mips64/usr/include/stdio.h"
#else /* native - not cross development */
#include <stdio.h>
#endif /* mips64 */

unsigned int header[128] = {
 0x003000A8 , 0x00580044 , 0x00000000 , 0x35303230 , 0x00000101 ,
0xFFFFFFFF , 0xFFFFFFFF , 0x00000000 ,
 0x01000020 , 0x00000000 , 0x00000000 , 0x00000000 , 0x00000000 ,
0x00000000 , 0x00000000 , 0x00000000 ,
 0x00000000 , 0x00000000 , 0x00000000 , 0x00000000 , 0x00000000 ,
0x00000000 , 0x504F4D03 , 0x00000000 ,
 0x00000000 , 0x00000000 , 0x00000000 , 0x00000000 , 0x00000000 ,
0x00000000 , 0x00000000 , 0x00000000 ,
 0x2E315604 , 0x00000030 , 0x00000000 , 0x00000000 , 0x00000000 ,
0x00000000 , 0x2D353005 , 0x00003530 ,
 0x00000000 , 0x00000000 , 0x00000010 , 0x00000000 , 0x00000080 ,
0x00000002 , 0xFFFF0000 , 0xFFFFFFFF ,
 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF ,
0xFFFFFFFF , 0xFFFFFFFF , 0xFFFFFFFF 
};

#define BLOCK_SIZE   512
#define NB_BLOCK_IX  0xAA  /* offset of 'number_of_block' in the header */

main (int argc, char *argv[]) {

        FILE   *fin,*fout;
        char buf[BLOCK_SIZE];
        int len;
        int bk;

	/* verify number of arguments */
	if (argc != 3) {
           printf ("Usage: %s input_file output_file\n", argv [0]);
           exit (1);
        }

        if (!(fin = fopen(argv[1],"r"))) {
            printf ("%s: %s cannot be open for read\n", argv [0], argv[1]);
            exit (1);
        }
        if (!(fout = fopen(argv[2],"w"))) {  
            printf ("%s: %s cannot be open for write\n",argv [0], argv [2]);
            exit (1);
        }

	/*get the number of blocks of the input file*/
        bk = 0;
	while (fread (buf, 1, BLOCK_SIZE , fin)) {
           bk ++;
        }

	/*write the block size into the 'number_of_block' header field*/
        *(short *)((char *)header + NB_BLOCK_IX) = bk;

	/*write the header into the first block of the file*/
        if (fwrite (header, sizeof (header), 1, fout) == 0)
               printf ("fwrite error (header)\n");  

	/*copy the input file into the output file, and pad out
	 * to a block size
	 */
        fseek (fin,0,0);
        while ( (len = fread (buf, 1, BLOCK_SIZE , fin)) != 0 ) {
            memset (&buf[len], 0, BLOCK_SIZE-len);
            if (fwrite (buf, BLOCK_SIZE , 1, fout) == 0)
                   printf ("fwrite error (buf)\n");  
        }

        fclose(fin);
        fclose(fout);
}
