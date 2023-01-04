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
static char *rcsid = "@(#)$RCSfile: kdebug_proto.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/07 21:02:19 $";
#endif

#include <sys/kdebug.h>

static unsigned long get_csum;		/* receive csum accumulator */
static unsigned long put_csum;		/* xmit csum accumulator */
static long resync;

/*
 * csum_getc -- get next character, handling checksum calculation
 * and DLE escapes
 */
static
csum_getc()
{
    char c;

    c = kdebug_getc();
    get_csum += (unsigned long) c;

    if (c == SYN)
	resync = 1;

    if (c == DLE) {
	c = csum_getc();

	switch (c) {
	case 'S':
	    c = SYN;
	    break;

	case 'D':
	    c = DLE;
	    break;

	default:
	    kprintf(DBG_WARNING, "unknown DLE escape, 0x%x\n", c);
	    break;
	}
    }
    return(c);
}

/*
 * csum_putc -- put character handling checksum calculation and doing
 * DLE stuffing for characters that must be escaped
 */
static void
csum_putc(
    char c)
{
    switch (c) {
    case SYN:
	put_csum += DLE;
	kdebug_putc(DLE);
	c = 'S';
	break;

    case DLE:
	put_csum += DLE;
	kdebug_putc(DLE);
	c = 'D';
	break;
    }

    put_csum += (unsigned long) c;
    kdebug_putc(c);
}

/*
 * getpkt -- unwrap incoming packet, check checksum and send appropriate
 * acknowledgment.  Returns pack type.
 */
char
getpkt(
    unsigned long *buf,
    unsigned long maxlen)
{
    unsigned long csum, pkt_csum;
    char type, ch;
    long size;
    char *cp;
    long i;
    long good_get = 0;

    while (!good_get) {
        /*
         * sync to start of packet
         */
        while (kdebug_getc() != SYN);
	resync = 0;
        get_csum = 0;

        type = csum_getc();
	if (resync)
	    continue;
        csum_getc();
	if (resync)
	    continue;
        ch = csum_getc();
	if (resync)
	    continue;
        size = (ch & 0x3f) << 12;
        ch = csum_getc();
	if (resync)
	    continue;
        size |= (ch & 0x3f) << 6;
        ch = csum_getc();
	if (resync)
	    continue;
        size |= ch & 0x3f;
        if (size > maxlen) {
	    kprintf(DBG_PROTO, "getpkt: bad packet length\n");
	    continue;
        }
    
        cp = (char *) buf;
    
        while (!resync && size--) {
    	    csum_getc();
    
	    for (i = sizeof(long) - 1; !resync && i >= 0; i--) {
    	        ch = csum_getc();
	        if (ch >= 'a' && ch <= 'f')
		    cp[i] = (ch - 'a' + 10) << 4;
	        else
		    cp[i] = (ch - '0') << 4;
	        ch = csum_getc();
	        if (ch >= 'a' && ch <= 'f')
		    cp[i] |= (ch - 'a' + 10) & 0xf;
	        else
		    cp[i] |= (ch - '0') & 0xf;
	    }
	    cp += sizeof(long);
        }
	if (resync)
	    continue;
    	
        csum_getc();
	if (resync)
	    continue;
        pkt_csum = get_csum;
        ch = csum_getc();
	if (resync)
	    continue;
        csum = (ch & 0x3f) << 6;
        ch = csum_getc();
	if (resync)
	    continue;
        csum |= ch & 0x3f;
    
        if ((pkt_csum & 0xfff) != csum) {
	    kprintf(DBG_PROTO, "getpkt: bad csum\n");
	    continue;
        }
	good_get = 1;
    }
    
    /*
     * Send the ACK
     */
    kdebug_putc(P_ACK);
    
    return(type);
}

/*
 * putpkt -- wrap data in packet and transmit.
 * Waits for acknowledgment and retransmits as necessary
 */
void
putpkt(
    char type,
    unsigned long *buf,
    unsigned long size)
{
    unsigned long i;
    long j;
    char *cp;
    long good_put = 0;
    long success;

    while (!good_put) {
    
        kdebug_putc(SYN);
    
        put_csum = 0;
        csum_putc(type);
        csum_putc(' ');
        csum_putc(((size >> 12) & 0x3f) | 0x40);
        csum_putc(((size >> 6) & 0x3f) | 0x40);
        csum_putc((size & 0x3f) | 0x40);

        cp = (char *) buf;
        for (i = 0; i < size; i++) {
	    csum_putc(' ');
    
	    for (j = sizeof(long) - 1; j >= 0; j--) {
	        csum_putc("0123456789abcdef"[(cp[j] >> 4) & 0xf]);
	        csum_putc("0123456789abcdef"[cp[j] & 0xf]);
	    }
	    cp += sizeof(long);
        }

        csum_putc(' ');
        kdebug_putc(((put_csum >> 6) & 0x3f) | 0x40);
        kdebug_putc((put_csum & 0x3f) | 0x40);

        kdebug_nblock_getc(&success);
	if (!success) {
	    kprintf(DBG_PROTO, "timeout\n");
	    continue;
	}
	good_put = 1;
    }
}
