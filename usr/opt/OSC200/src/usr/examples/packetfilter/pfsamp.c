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
static char *rcsid = "@(#)$RCSfile: pfsamp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/11 22:52:26 $";
#endif

/*
 * pfsamp.c - sample program which uses the packetfilter to
 * 	      display IP packets to stdout
 *
 * Description: This program displays IP packets read from the first
 *              up and running network interface.  This is a simple
 *              example of how to program to the packetfilter interface.
 *
 * Input:      none
 *
 * Output:     sent to standard output
 *
 * To compile:  cc pfsamp.c pfopen.c -o pfsamp
 *
 * Example:     pfsamp
 *
 * Comments:    This example includes a call to pfopen(), a function
 *              which finds the next available up and running network interface.
 *              The pfopen function is available in libc.a on ULTRIX V4.2
 *              and later systems, however, it does not currently appear
 *              in libc.a on DEC OSF/1 systems.  It is provided here for
 *              backward compatibility with programs which make existing calls
 *              to pfopen(), and it is expected that it will be included in
 *              libc.a in a future release of DEC OSF/1.
 */

/*
 * Digital Equipment Corporation supplies this software example on 
 * an "as-is" basis for general customer use.  Note that Digital 
 * does not offer any support for it, nor is it covered under any 
 * of Digital's support contracts. 
 */
 
#include <stdio.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/time.h>
#include <net/pfilt.h>
 
struct enfilter filter;

buildfilter()
{
        filter.enf_Priority = 37;       /* anything > 2 should work */
        filter.enf_FilterLen = 0;

	/* return(0); */ 	/* NULL filter code (accept everything) */

        /* packet type is last short in header */
        filter.enf_Filter[filter.enf_FilterLen++] =
                ENF_PUSHWORD + 6;
        filter.enf_Filter[filter.enf_FilterLen++] =
                ENF_PUSHLIT | ENF_CAND;
        filter.enf_Filter[filter.enf_FilterLen++] =
                htons(0x0800);	/* match all 0x0800 (IP) packets */
	return;
}

main()
{
	char ifname[256],buf[4096];
	int fd;
	u_short bits;
 
	ifname[0] = NULL;
	if ((fd = pfopen(ifname, O_RDONLY) ) < 0)
		err("pfopen");

	bits = (ENTSTAMP | ENBATCH | ENHOLDSIG);
	if (ioctl(fd, EIOCMBIC, &bits))
		err("ioctl: EIOCMBIC");

	bits = (ENPROMISC | ENNONEXCL | ENCOPYALL);
	if ( ioctl(fd, EIOCMBIS, &bits))
		err("ioctl: EIOCMBIS");

	buildfilter();

	if (ioctl(fd, EIOCSETF, &filter))
		err("ioctl: EIOCSETF");

	while (1) {
		int i;
		unsigned char *p;
	
		if ((i=read(fd,buf,4096)) < 0)
			err("read");
		if (i == 0)
			continue;	/* nothing to read */

		p = (unsigned char *)buf;

		/* decode IP packets */
		printf("\n%d bytes %x-%x-%x-%x-%x-%x -> %x-%x-%x-%x-%x-%x 0x%02x%02x\n",
			i, p[6], p[7], p[8], p[9], p[10], p[11],
			p[0], p[1], p[2], p[3], p[4], p[5],
			p[12], p[13]);

		printf("\t%d.%d.%d.%d -> %d.%d.%d.%d\n",
			(u_char) p[26],(u_char) p[27],
			(u_char) p[28],(u_char) p[29],
			(u_char) p[30],(u_char) p[31],
			(u_char) p[32],(u_char) p[33]);
	}
}
 
err(s)
	char *s;
{
	perror(s);
	exit(1);
}
