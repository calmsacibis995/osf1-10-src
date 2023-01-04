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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: cksum.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/06/10 15:36:04 $";
#endif
/*
 * HISTORY
 */

/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (CMDPOSIX) Commands required by Posix 1003.2
 *
 * FUNCTIONS: cksum
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1990
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.1  com/cmd/posix/cksum.c, 9121320k, bos320 11/12/90 13:31:48
 */

/* 
 * One can verify the crc of a file by recompiling with DEBUG
 * turned on, and then recompute the crc.  You'll need to run cksum 
 * twice, once to add the original crc and second to recalculate the
 * resulting crc.  The result should be zero.  This verifys the algorithm.
 *
 */

#include <stdio.h>
#include <locale.h>

int	errflg = 0;

FILE	*fp;

unsigned long crc_32(unsigned char *, int, unsigned long);

/*
 * NAME: cksum file
 *                                                                    
 * FUNCTION: Displays the checksum and byte count of a file.
 * 
 */  
main(argc, argv)
int   argc;
char *argv[];
{

	ulong crc32;
	extern ulong crc_sum();
	int j = 1;

	/* XPG4: ignore leading -- */
	if ((argc > 1) && (strcmp(argv[1], "--") == 0))
		j++;

	(void) setlocale(LC_ALL,"");

	do {
		if (j < argc) {
#if DEBUG
			if ((fp = fopen(argv[j], "a+")) == NULL) {
#else
			if ((fp = fopen(argv[j], "r")) == NULL) {
#endif
				fprintf(stderr, "cksum: ");
				perror(argv[j]);
				errflg++;
				continue;
				}
			}
		else {
			fp = stdin;
			}

#if DEBUG
		rewind(fp);
		crc32 = crc_sum(j, argv);
		add_word(crc32);
#else
		crc32 = crc_sum(j, argv);
#endif
		fclose(fp);

	} while (++j < argc);

	exit(errflg);
}

ulong
crc_sum(fcnt, argv)
int fcnt;
char *argv[];
{

	int bytes = 0;
	int nbytes = 0;
	unsigned char buf[BUFSIZ];
	ulong crc = 0;

	/*
	 * complete read
	 */

	while ( (bytes = fread(buf, sizeof(char), BUFSIZ, fp) ) > 0) {
		crc = crc_32(buf, bytes, crc);
		nbytes+=bytes;
		if (feof(fp)) break ;
	}

	if (fp == stdin)
		fprintf(stdout, "%u %d\n",  crc, nbytes); 
	else
		fprintf(stdout, "%u %d %s\n",  crc, nbytes, argv[fcnt]); 
	return(crc);

}

/*
 * NAME: crc_32
 *
 * FUNCTION:
 *     crc32 generates a 32 bit "classic" CRC using the following
 *     CRC polynomial:
 *
 *   g(x) = x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8
 *               + x^7  + x^5  + x^4  + x^2  + x^1  + x^0
 *
 *   e.g. g(x) = 1 04c1 1db7
 *
*/

unsigned long crctab[] = {
0x7fffffff,
0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e,
0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d,
0x6ddde4eb, 0xf4d4b551, 0x83d385c7, 0x136c9856, 0x646ba8c0,
0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63,
0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa,
0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75,
0xdcd60dcf, 0xabd13d59, 0x26d930ac, 0x51de003a, 0xc8d75180, 
0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87,
0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5,
0xe8b8e433, 0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01, 0x6b6b51f4,
0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b,
0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea,
0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541,
0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc,
0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f,
0xdd0d7cc9, 0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f, 0x5edef90e,
0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c,
0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b,
0x9309ff9d, 0x0a00ae27, 0x7d079eb1, 0xf00f9344, 0x8708a3d2, 
0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671,
0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 
0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767,
0x3fb506dd, 0x48b2364b, 0xd80d2bda, 0xaf0a1b4c, 0x36034af6,
0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795,
0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b,
0x5bdeae1d, 0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713, 0x95bf4a82,
0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 
0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8,
0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff,
0xf862ae69, 0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 
0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d,
0x3e6e77db, 0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9, 0xbdbdf21c,
0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02,
0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d 
};

/*
 * code straight out of POSIX.2 Draft 11
 */

unsigned long
crc_32(b, n, s)
register unsigned char *b;	/* byte sequence to checksum */	
register int 		n;	/* length of sequence */
register ulong 		s;	/* initial checksum value */
{

	register int i, aux;

	aux = 0;
	while (n-- > 0) {
		/* compute the index to the crc table */
		i = (s >> 24) ^ ((unsigned int)(*b++));

		if (i == 0) {
			/* replace an intermediate zero with the
			   next value from the sequence */
			i = aux++;
			if (aux >= sizeof(crctab)/sizeof(crctab[0]))
				aux = 0;
		}

		/* new checksum value */
		s = ((s << 8) & 0xffffffffUL) ^ crctab[i];
	}
	return s;

}

#ifdef DEBUG
add_word(word)
ulong word;
{
	unsigned char buf[BUFSIZ];
	long bytes;

	fputc((word >> 24) & 0xff, fp);
	fputc((word >> 16) & 0xff, fp);
	fputc((word >> 8) & 0xff, fp);
	fputc(word & 0xff, fp);

}
#endif
