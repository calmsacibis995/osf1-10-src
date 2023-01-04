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
 * @(#)$RCSfile: savefile.h,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/21 14:35:20 $
 */
/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Based on:
 * savefile.h,v 1.10 90/12/17 13:48:58 mccanne Exp $
 *
 * Header for offline storage info.
 * Extraction/creation by Jeffrey Mogul, DECWRL.
 *
 * Used to save the received packet headers, after filtering, to
 * a file, and then read them later.
 */

/*
 * Each packet in the dump file is prepended with this generic header.
 * This gets around the problem of different headers for different
 * packet interfaces.
 */
struct packet_header {
	struct timeval ts;	/* time stamp */
	u_int32 len;		/* length this packet (off wire) */
	u_int32 caplen;		/* length of portion present */
};

/* true if the contents of the savefile being read are byte-swapped */
extern int sf_swapped;

/* macros for when sf_swapped is true: */
/*
 * We use the "receiver-makes-right" approach to byte order,
 * because time is at a premium when we are writing the file.
 * In other words, the file_header and packet_header records
 * are written in host byte order.
 * Note that the packets are always written in network byte order.
 *
 * ntoh[ls] aren't sufficient because we might need to swap on a big-endian
 * machine (if the file was written in little-end order).
 */
#define	SWAPLONG(y) \
((((y)&0xff)<<24) | (((y)&0xff00)<<8) | (((y)&0xff0000)>>8) | (((y)>>24)&0xff))
#define	SWAPSHORT(y) \
	( (((y)&0xff)<<8) | (((y)&0xff00)>>8) )


extern FILE *sf_readfile;	/* dump file being read from */
extern FILE *sf_writefile;	/* dump file being written to */

int sf_read_init();
int sf_read();
int sf_next_packet();
void sf_write_init();
void sf_write();
void sf_err();

#define SFERR_TRUNC		1
#define SFERR_BADVERSION	2
#define SFERR_BADF		3
#define SFERR_EOF		4 /* not really an error, just a status */

