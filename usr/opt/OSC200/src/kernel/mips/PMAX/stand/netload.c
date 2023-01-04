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
static char	*sccsid = "@(#)$RCSfile: netload.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:38:42 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * netload.c
 */
#ifndef lint

#endif lint
/************************************************************************
 *									*
 *			Copyright (c) 1986 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   This software is  derived  from  software  received  from  the	*
 *   University    of   California,   Berkeley,   and   from   Bell	*
 *   Laboratories.  Use, duplication, or disclosure is  subject  to	*
 *   restrictions  under  license  agreements  with  University  of	*
 *   California and with AT&T.						*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/
#include "../../../sys/param.h"
#include "mop.h"

#ifdef vax
#include "vax/vmb.h"
#endif vax

/*
 * Maintenance History
 *
 * 8-Feb-88 tresvik
 *	swap the definition of TERTIARY and SECONDARY so that Mop can
 *	be brought into spec.  Now TERTIARY = vmunix and SECONDARY =
 *	client specific paramater file.  Associated registration changes
 *	made to /etc/dms and /etc/ris in conjuction with this change.
 *	This turned out to more than anticipated.  A relatively minor
 * 	change to the mop_dlload code causes a significant restructuring 
 *	of this code.  It, in fact, represents the removal a fair amount
 *	of code which is no longer required.  Now that mop_dlload does the
 *	right things, it is easier to communicate with.
 */

struct netblk *netblk_ptr;

#ifdef vax
extern	struct	vmb_info *vmbinfo;
#endif vax

#ifdef mips
#define stop _prom_restart
#define printf _prom_printf
#define getenv _prom_getenv
#define open _prom_open
#define read _prom_read
#define write _prom_write
#define close _prom_close

int	io;	/* I/O channel for mips */
#define LEN 2

#endif mips

#ifdef vax
/*
 * Allocate and Initialize param_buf as follows
 *
 *	param_buf.pad = 1;
 *	param_buf.prot = 0x160;
 *
 * Start with a network broadcast address 
 *	0x0000010000ab 
 * 
 *	param_buf.dest[0] = 0xab; 
 *	param_buf.dest[1] = 0x00; 
 *	param_buf.dest[2] = 0x00; 
 *	param_buf.dest[3] = 0x01; 
 *	param_buf.dest[4] = 0x00; 
 *	param_buf.dest[5] = 0x00;
 */
struct {
	u_short	pad;
	u_short	prot;
	u_char	dest[6];
} param_buf = {1, 0x160, 0xab, 0, 0, 1, 0, 0};

#endif vax

#define BAD_LOAD 1

int	DEBUG=0;

#ifdef mips
main (argc,argv,envp)
int argc;
char **argv,**envp;
#endif mips
#ifdef vax
main ()
#endif vax
{
	int (*start)();
	int i;
	char *j;
	extern char *version;

	printf("\nUltrixload - %s\n\n", version);

#ifdef vax
	netblk_ptr = (struct netblk *)&vmbinfo->netblk;
#endif vax
#ifdef mips
	netblk_ptr = (struct netblk *)NETBLK_LDADDR;
#endif mips

	/*
	 * Clear out the netblk in case there is something there
	 * and we don't get a successful load.
	 */
	j = (char *)netblk_ptr;
	for (i = 0; i < sizeof (struct netblk); i++) 
		j[i] = (char) 0;

	if (DEBUG) {
		printf("DEBUG is enabled.\n");
		printf("`*' means that 32K bytes have been loaded\n");
		printf("`R' means that a read error occurred\n");
		printf("`W' means that a write error occurred\n");
		printf("`S' means that the packet rcvd was not the one asked for\n\n");
		printf("All errors are retried\n\n");
	}
#ifdef mips
	io = open("mop()", 2);
#endif mips
	for (i = 0; i < 2000000; i++);	/* Give the host a breather */
	i = upload(PGMTYP_SECONDARY, netblk_ptr, sizeof (struct netblk));
	if (i == BAD_LOAD) {
		printf("Network parameter file load failed.\n");
		printf("Continuing without network information.\n");
	} else
		printf ("Host server is '%s'\n", netblk_ptr->srvname);
	printf("Loading operating system image ...\n");
	for (i = 0; i < 2000000; i++); 	/* Give the host a breather */
	start = (int(*)()) upload(PGMTYP_TERTIARY, 0, RCV_BUF_SZ);
	printf("\n");
	if ((int)start == BAD_LOAD) {
		printf("Unrecoverable network failure\n");
		stop();
	}
#ifdef vax
	(*start)(vmbinfo);
#endif vax
#ifdef mips
	close(io);
	(*start)(argc,argv,envp);
#endif mips
	stop();
}


upload (prog, addr, bufsz)
int	prog, addr, bufsz;
{
#ifdef vax
	/*
	 * with VMB boot drivers the buffers need to be page aligned
	 */
	union mop_packets *mop_i = (union mop_packets *)(char *)vmbinfo-2048;
	union mop_packets *mop_o = (union mop_packets *)(char *)vmbinfo-4096;
#endif vax
#ifdef mips
	union mop_packets mop_output;
	union mop_packets mop_input;
	union mop_packets *mop_i = &mop_input;
	union mop_packets *mop_o = &mop_output;
#endif mips

	int j=0;
	int wrt_cnt, wrt_retry=5, rd_retry=5, seq_errs=0;
	int ldnum=1;
	int status;
	
#ifdef vax
	drvinit();			/* re-init the VMB boot driver */
#endif vax
	/*
	 * The following setup of ...code is needed to get things 
	 * looping properly below.
	 */
	mop_i->memload.code = NETLOAD_REQUEST; 
	for (;;) {
		int tmp;

		switch (mop_i->memload.code) {
		/*
		 * This is local variable use to kick start the network
		 * boot sequence.  It initiates program requests by
		 * falling out to write after creating the desired
		 * program request packet.
		 */
		case NETLOAD_REQUEST:
			mop_o->req_pgm.code = REQ_PROG_CODE;
			mop_o->req_pgm.devtype = NET_QNA;
			mop_o->req_pgm.mopver = MOP_VERSION;
			mop_o->req_pgm.pgmtyp = prog;
			mop_o->req_pgm.swid_form = -1; /* force sys load */
			mop_o->req_pgm.proc = SYSTEMPROC;
			mop_o->req_pgm.rbufsz_param = XTRA_BUFSZ;
			mop_o->req_pgm.sz_field = 2;
			tmp = sizeof mop_i->memload;
			bcopy((char *)&tmp, mop_o->req_pgm.rcvbufsz,
				sizeof mop_o->req_pgm.rcvbufsz);
			wrt_cnt = sizeof mop_o->req_pgm;
			break;
		/*
		 * In response to a request for a multisegment tertiary
		 * load from the network  (except for the last segment)
		 */
		case VOLASS_CODE:
		/*
		 * Send the same packet out again, which is the original
		 * request
		 */
			mop_i->memload.code = NETLOAD_REQUEST;
			continue;
		case MEMLD_CODE:
			/*
			 * The load number of the packet received must
			 * equal the number requested.  If it doesn't, the
			 * host is again asked for the same packet by
			 * load sequence number
			 */
			if (mop_i->memload.loadnum != 
				(u_char)((ldnum - 1) & 0xff)) {
				if (DEBUG) printf("S");
				if (++seq_errs == 5) {
					printf("\n\
Wrong packet number received from server - retries exceeded\n");
					goto error;
				}
				break;		/* fall out to ask again */
			}
			seq_errs=0;
			bcopy(mop_i->memload.loadaddr, (char *)&tmp,
				sizeof mop_i->memload.loadaddr);
			bcopy(mop_i->memload.data, addr + tmp,
				bufsz);
			/* 
			 * Display a progress indicator about every 
			 * 32k bytes
			 */
			if (j++ == 23){
				printf("*");
				j=0;
			}
			/*
			 * Now, prepare the next request packet before 
			 * falling out to send it.
			 */
			mop_o->req_memload.code = REQ_LOAD_CODE;
			mop_o->req_memload.loadnum = ldnum++;
			if (ldnum > 255) ldnum =0;
			mop_o->req_memload.error = 0;
			wrt_cnt = sizeof mop_o->req_memload;
			break;
		/* 
	 	 * In response to SECONDARY load request and
	 	 * the last packet on a multisegment tertiary load
	 	 */
		case MEMLD_XFR_CODE:
			/*
			 * For SECONDARY requests the rcvmsg contains
			 * real data and we don't care about any other
			 * part of the packet.  There is only one program
			 * segment allowed.  This is the netblk piece of
			 * vmbinfo.
			 *
			 * If this code is received to indicate the end of
			 * a multisegment load (our request for TERTIARY),
			 * then there is no data and all we care about
			 * is returning the transfer address of (presumably) 
			 * the vmunix that was just loaded.
			 */
			if (prog == PGMTYP_SECONDARY) {
				bcopy(mop_i->memload_xfr.loadaddr,
					(char *)&tmp, 
					sizeof mop_i->memload_xfr.loadaddr);
				bcopy(mop_i->memload_xfr.type.data,
				    addr + tmp, bufsz);
				return(0);
			} else {
#ifdef vax
				disconnect();		/* shutdown the link */
#endif vax
				bcopy(mop_i->memload_xfr.type.xfr_addr,
					(char *)&tmp,
					sizeof mop_i->memload_xfr.type.xfr_addr);
				return (tmp);
			}
			break;
		/*
		 * Other codes are unexpected and considered fatal.
		 */
		default:
			printf("Unexpected MOP response code of %d\n",
				mop_i->memload.code);
			goto error;
		}
		/*
		 * At least one write and one read occurs now before
		 * looping back up to evaluate the packet received.  Of
		 * course, too many read or write errors will cause
		 * failures to occur.
		 */
		while (wrt_retry--) {
#ifdef vax
			if (write_net(wrt_cnt,&mop_o->req_pgm.code))
#endif vax
#ifdef mips
			if (write_net(wrt_cnt,&mop_o->req_pgm))
#endif mips
				break;
			else {
				if (DEBUG) printf("W");
				continue;
			}
		}
		if (wrt_retry <= 0 ) { 	/* if we ran out of retries */
			printf("write I/O error: retries exceeded\n");
			goto error;
		}
		wrt_retry=5;

		if ((read_net((sizeof mop_i->memload),&mop_i->memload)) == 0) {
			if (DEBUG) printf("R");
			if (rd_retry--)
				continue;	/* retry */
			printf("read I/O error: retries exceeded\n");
			goto error;
		}
		rd_retry=5;
	}
error:
#ifdef vax
	disconnect();			/* shutdown the link */
#endif vax
	return (-1);
}

write_net(size, addr)
int	size, addr;
{
	int status;
#ifdef vax

	status = qio(PHYSMODE,IO$_WRITELBLK,&param_buf,size,addr);
	return(status & 1);
#endif vax
#ifdef mips
	status = write(io, addr, size);
	return((status < 0) ? 0 : 1);
#endif mips
}

read_net(size, addr)
int	size, addr;
{
	int status;

#ifdef vax
	status = qio(PHYSMODE,IO$_READLBLK,&param_buf,size,addr);
#endif vax

#ifdef mips
	status = read(io, addr, size);
	return((status < 0) ? 0 : 1);
#endif mips

}
