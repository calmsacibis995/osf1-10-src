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
#ifdef lint
static	char	*sccsid = "@(#)bootblock.c	9.3	(ULTRIX)	3/11/91";
#endif lint

/*
 * This program creates a boot block which resides on block 0 of the 'a'
 * partition of a disk, of the first block of a tape.  The program defines
 * a structure for the boot block, then fills in the values and finally
 * writes that out to the bootblk file.
 */

#include <stdio.h>
#include <sys/file.h>

#define DISK_BLOCK_SIZE 512     /* Assume 512 bytes per disk block      */
#define BB_FILENAME	"xxboot"	/* Boot block file name		*/

#ifdef do_not_include
***JAC 27-sep don't know why 18 was used, it must be 16 because of the
magic block
#define BB_LBNS         18      /* Length of primary bootstrap program  */
#endif do_not_include

#define BB_LBNS         16      /* Length of primary bootstrap program  */

#define BB_START_LBN    1       /* Starting lbn of primary bootstrap    */
#define BB_VAX_BYTES    0x88    /* Used by VAX booting mechinism        */
#define BB_ALPHA_START  0x1e0   /* First offset used by alpha           */
#define BB_EXPANSION    (BB_ALPHA_START - BB_VAX_BYTES) /* Alpha will add */
                                /* any entensions fromhigh to low address */
#ifdef __alpha
#define BB_QUAD_LENGTH  64 	/* Number of quadwords in boot block    */
#define BB_SUM_LENGTH   63 	/* Checksum done over first 63 quads    */
#else
#define BB_QUAD_LENGTH (64 * 2) /* Number of quadwords in boot block    */
#define BB_SUM_LENGTH  (63 * 2) /* Checksum done over first 63 quads    */
#endif

/*
 * Description of the alpha boot block.  The definition comes from the alpha
 * srm V2.1.
 *
 * For non-alpha systems declare twice as many longs to get the propper
 * structure size.
 */
typedef struct bootblock {
        union {
                struct {
                        char vax_boot_block[BB_VAX_BYTES];
                        char expansion_reserve[BB_EXPANSION];
#ifdef __alpha
                        long count;
                        long starting_lbn;
                        long flags;
                        long checksum;
#else
                        long count;
                        long count_high;
                        long starting_lbn;
                        long starting_lbn_high;
                        long flags;
                        long flags_high;
                        long checksum;
                        long checksum_high;     /* Falls appart on overflow */
#endif
                } bb_fields;
                struct {
#ifdef __alpha
                        long bb_quadwords[BB_QUAD_LENGTH];
#else
                        long bb_quadwords[BB_QUAD_LENGTH];
#endif
                } bb_quads;
        } bb_def;
} BOOTBLOCK;



main()
{
        BOOTBLOCK bb;
	int i;
	int fd;

	/*
	 * Sanity check to insure that the size of the boot block will be
	 * 512 bytes.
	 */
	if ((sizeof(bb)) != DISK_BLOCK_SIZE) {
		fprintf(stderr,"ERROR: the size of the boot block is %d, not %d\n", sizeof(bb), DISK_BLOCK_SIZE);
		exit(1);
	}
        bzero(bb, sizeof(bb));

        /*
         * The count field specifies the number of physically contiguous
         * logical blocks of the primary bootstrap program.
         */
        bb.bb_def.bb_fields.count = BB_LBNS;
        /*
         * The starting_lbn field is set to the starting lbn of the primary
         * bootstrap program.  Since the bootblock itself occupies lbn 0
         * the primary bootstrap program starts at lbn 1.
         */
        bb.bb_def.bb_fields.starting_lbn = BB_START_LBN;
        /*
         * The flags field is set to 0.  A flags field of 1 is reserved to
         */
	bb.bb_def.bb_fields.flags = 0;
	/*
	 * The checksum field is a 2's complement addition ignoring overflow.
	 */
	for (i=0; i < BB_SUM_LENGTH; i++) {
		bb.bb_def.bb_fields.checksum += 
			bb.bb_def.bb_quads.bb_quadwords[i];
	}

	/*
	 * Now that the boot block has been setup, blast it as output to
	 * be used as the 512 byte bootblock.
	 */
	fd = open(BB_FILENAME, (O_CREAT | O_TRUNC | O_WRONLY), 0644);
	if (fd < 0) {
		fprintf(stderr, "Unable to open file %s\n",BB_FILENAME);
		perror("Open failed");
		exit(1);
	}
	i = write(fd, &bb, sizeof(bb));
	if (i < 0) {
		perror("Write failed");
	}
	if (i != DISK_BLOCK_SIZE) {
		fprintf(stderr, "ERROR: only wrote %d of %d bytes.\n",i,
			DISK_BLOCK_SIZE);
		exit(1);
	}
	close(fd);
	exit(0);
}
