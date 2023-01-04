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
static char *rcsid = "@(#)$RCSfile: print-nfs.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/21 14:34:32 $";
#endif
/*
 * Copyright (c) 1990, 1991, 1992 The Regents of the University of California.
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
 */

/*
 * Based on:
 * static char rcsid[] = "print-nfs.c,v 1.2 92/07/10 10:59:04 mogul Exp $ (LBL)";
 */

#include <stdio.h>
#include <setjmp.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>

#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <rpc/types.h>
#include "rpc.h"		/* Some local definitions to avoid problems */
#include <rpc/svc.h>
#include <rpc/xdr.h>
#include <rpc/rpc_msg.h>

#include <ctype.h>

#include "interface.h"
/* These must come after interface.h for BSD. */
#if BSD >= 199006
#include <sys/ucred.h>
#include <nfs/nfsv2.h>
#endif
#include <nfs/nfs.h>

#define classic_major(x) ((int)(((unsigned)(x)>>8)&0377))
#define classic_minor(x) ((int)((x)&0377))
#define osf_major(x)     ((int)(((dev_t)(x)>>20)&07777))
#define osf_minor(x)     ((int)((dev_t)(x)&03777777))

#include "addrtoname.h"
#include "extract.h"

static jmp_buf nfs_short;

static interp_reply();
static void nfs_printfh();
static void nfs_printfn();

static void xid_map_enter();
static int32 xid_map_find();

#if BYTE_ORDER == LITTLE_ENDIAN
/*
 * Byte swap an array of n words.
 * Assume input is word-aligned.
 * Check that buffer is bounded by "snapend".
 */
static void
bswap(bp, n)
	register u_int32 *bp;
	register u_int n;
{
	register int nwords = ((char *)snapend - (char *)bp) / sizeof(*bp);

	if (nwords > n)
		nwords = n;
	for (; --nwords >= 0; ++bp)
		*bp = ntohl(*bp);
}
#endif

void
nfsreply_print(rp, length, ip)
	register struct rpc_msg *rp;
	int length;
	register struct ip *ip;
{
	int32 proc;

#if BYTE_ORDER == LITTLE_ENDIAN
/*	bswap((u_int32 *)rp, sizeof(*rp) / sizeof(u_int32)); */
/*
 * We want to be careful about what we swap since we don't know yet
 * how big the headers are.
 * SWAPSIZE covers rm_xid, rm_direction, and rp_stat.
 */
#define	SWAPSIZE 	\
	(sizeof(u_int32) + sizeof(enum msg_type) + sizeof(enum reply_stat))
	bswap((u_int32 *)rp, SWAPSIZE / sizeof(u_int32));
#undef	SWAPSIZE
#endif
	if (!nflag)
		(void)printf("%s.nfs > %s.%x: %d reply %s",
			     ipaddr_string(&ip->ip_src),
			     ipaddr_string(&ip->ip_dst),
			     rp->rm_xid,
			     length,
			     rp->rm_reply.rp_stat == MSG_ACCEPTED? "ok":"ERR");
	else
		(void)printf("%s.%x > %s.%x: %d reply %s",
			     ipaddr_string(&ip->ip_src),
			     NFS_PORT,
			     ipaddr_string(&ip->ip_dst),
			     rp->rm_xid,
			     length,
			     rp->rm_reply.rp_stat == MSG_ACCEPTED? "ok":"ERR");

	proc = xid_map_find(rp, ip);
	if (proc >= 0) {
	    interp_reply(rp, (u_int32)proc, length);
	}	
}

/*
 * Return a pointer to the first file handle in the packet.
 * If the packet was truncated, return 0.
 */
static u_int32 *
parsereq(rp, length)
	register struct rpc_msg *rp;
	register int length;
{
	register u_int32 *dp = (u_int32 *)&rp->rm_call.cb_cred;
	register u_int32 *ep = (u_int32 *)snapend;

	/* 
	 * find the start of the req data (if we captured it) 
	 * note that dp[1] was already byte swapped by bswap()
	 */
	if (dp < ep && dp[1] < length) {
		dp += (dp[1] + (2*sizeof(u_int32) + 3)) / sizeof(u_int32);
		if ((dp < ep) && (dp[1] < length)) {
			dp += (dp[1] + (2*sizeof(u_int32) + 3)) /
				sizeof(u_int32);
			if (dp < ep)
				return (dp);
		}
	}
	return (0);
}

/*
 * Print out an NFS file handle and return a pointer to following word.
 * If packet was truncated, return 0.
 */
static u_int32 *
parsefh(dp)
	register u_int32 *dp;
{
	if (dp + 8 <= (u_int32 *)snapend) {
		nfs_printfh(dp);
		return (dp + 8);
	}
	return (0);
}

/*
 * Print out a file name and return pointer to longword past it.
 * If packet was truncated, return 0.
 */
static u_int32 *
parsefn(dp)
	register u_int32 *dp;
{
	register int32 len;
	register u_char *cp;

	/* Bail if we don't have the string length */
	if ((u_char *)dp > snapend - sizeof(*dp))
		return(0);

	/* Fetch string length; convert to host order */
	len = *dp++;
	NTOHL(len);

	cp = (u_char *)dp;
	/* Update long pointer (NFS filenames are padded to int32) */
	dp += ((len + 3) & ~3) / sizeof(*dp);
	if ((u_char *)dp > snapend)
		return (0);
	nfs_printfn(cp, len);

	return (dp);
}

/*
 * Print out file handle and file name.
 * Return pointer to longword past file name.
 * If packet was truncated (or there was some other error), return 0.
 */
static u_int32 *
parsefhn(dp)
	register u_int32 *dp;
{
	dp = parsefh(dp);
	if (dp == 0)
		return (0);
	putchar(' ');
	return (parsefn(dp));
}

void
nfsreq_print(rp, length, ip)
	register struct rpc_msg *rp;
	int length;
	register struct ip *ip;
{
	register u_int32 *dp;
	register u_char *ep = snapend;
#define TCHECK(p, l) if ((u_char *)(p) > ep - l) break

#if BYTE_ORDER == LITTLE_ENDIAN
	bswap((u_int32 *)rp, sizeof(*rp) / sizeof(u_int32));
#endif

	if (!nflag)
		(void)printf("%s.%x > %s.nfs: %d",
			     ipaddr_string(&ip->ip_src),
			     rp->rm_xid,
			     ipaddr_string(&ip->ip_dst),
			     length);
	else
		(void)printf("%s.%x > %s.%x: %d",
			     ipaddr_string(&ip->ip_src),
			     rp->rm_xid,
			     ipaddr_string(&ip->ip_dst),
			     NFS_PORT,
			     length);

	xid_map_enter(rp, ip);	/* record proc number for later on */

	switch (rp->rm_call.cb_proc) {
#ifdef NFSPROC_NOOP
	case NFSPROC_NOOP:
		printf(" nop");
		return;
#else
#define NFSPROC_NOOP -1
#endif
	case RFS_NULL:
		printf(" null");
		return;

	case RFS_GETATTR:
		printf(" getattr");
		if ((dp = parsereq(rp, length)) != 0 && parsefh(dp) != 0)
			return;
		break;

	case RFS_SETATTR:
		printf(" setattr");
		if ((dp = parsereq(rp, length)) != 0 && parsefh(dp) != 0)
			return;
		break;

#if RFS_ROOT != NFSPROC_NOOP
	case RFS_ROOT:
		printf(" root");
		break;
#endif
	case RFS_LOOKUP:
		printf(" lookup");
		if ((dp = parsereq(rp, length)) != 0 && parsefhn(dp) != 0)
			return;
		break;

	case RFS_READLINK:
		printf(" readlink");
		if ((dp = parsereq(rp, length)) != 0 && parsefh(dp) != 0)
			return;
		break;

	case RFS_READ:
		printf(" read");
		if ((dp = parsereq(rp, length)) != 0 &&
		    (dp = parsefh(dp)) != 0) {
			TCHECK(dp, 3 * sizeof(*dp));
			printf(" %u bytes @ %u",
			       ntohl(dp[1]), ntohl(dp[0]));
			return;
		}
		break;

#if RFS_WRITECACHE != NFSPROC_NOOP
	case RFS_WRITECACHE:
		printf(" writecache");
		if ((dp = parsereq(rp, length)) != 0 &&
		    (dp = parsefh(dp)) != 0) {
			TCHECK(dp, 4 * sizeof(*dp));
			printf(" %u (%u) bytes @ %u (%u)",
			       ntohl(dp[3]), ntohl(dp[2]),
			       ntohl(dp[1]), ntohl(dp[0]));
			return;
		}
		break;
#endif
	case RFS_WRITE:
		printf(" write");
		if ((dp = parsereq(rp, length)) != 0 &&
		    (dp = parsefh(dp)) != 0) {
			TCHECK(dp, 4 * sizeof(*dp));
			printf(" %u (%u) bytes @ %u (%u)",
			       ntohl(dp[3]), ntohl(dp[2]),
			       ntohl(dp[1]), ntohl(dp[0]));
			return;
		}
		break;

	case RFS_CREATE:
		printf(" create");
		if ((dp = parsereq(rp, length)) != 0 && parsefhn(dp) != 0)
			return;
		break;

	case RFS_REMOVE:
		printf(" remove");
		if ((dp = parsereq(rp, length)) != 0 && parsefhn(dp) != 0)
			return;
		break;

	case RFS_RENAME:
		printf(" rename");
		if ((dp = parsereq(rp, length)) != 0 && 
		    (dp = parsefhn(dp)) != 0) {
			fputs(" ->", stdout);
			if (parsefhn(dp) != 0)
				return;
		}
		break;

	case RFS_LINK:
		printf(" link");
		if ((dp = parsereq(rp, length)) != 0 &&
		    (dp = parsefh(dp)) != 0) {
			fputs(" ->", stdout);
			if (parsefhn(dp) != 0)
				return;
		}
		break;

	case RFS_SYMLINK:
		printf(" symlink");
		if ((dp = parsereq(rp, length)) != 0 &&
		    (dp = parsefhn(dp)) != 0) {
			fputs(" -> ", stdout);
			if (parsefn(dp) != 0)
				return;
		}
		break;

	case RFS_MKDIR:
		printf(" mkdir");
		if ((dp = parsereq(rp, length)) != 0 && parsefhn(dp) != 0)
			return;
		break;

	case RFS_RMDIR:
		printf(" rmdir");
		if ((dp = parsereq(rp, length)) != 0 && parsefhn(dp) != 0)
			return;
		break;

	case RFS_READDIR:
		printf(" readdir");
		if ((dp = parsereq(rp, length)) != 0 &&
		    (dp = parsefh(dp)) != 0) {
			TCHECK(dp, 2 * sizeof(*dp));
			printf(" %u bytes @ %u", ntohl(dp[1]), ntohl(dp[0]));
			return;
		}
		break;

	case RFS_STATFS:
		printf(" statfs");
		if ((dp = parsereq(rp, length)) != 0 && parsefh(dp) != 0)
			return;
		break;

	default:
		printf(" proc-%u", rp->rm_call.cb_proc);
		return;
	}
	fputs(" [|nfs]", stdout);
#undef TCHECK
}

/*
 * Print out an NFS file handle.
 * We assume packet was not truncated before the end of the
 * file handle pointed to by dp.
 * Note - this routine is completely blind to endianness of the machine.
 */
static void
nfs_printfh(dp)
	register u_int32 *dp;
{
	/*
	 * take a wild guess at the structure of file handles.
	 * On sun 3s, there are 2 int32s of fsid, a short
	 * len == 8, a int32 of inode & a int32 of generation number.
	 * On sun 4s, the len == 10 & there are 2 bytes of
	 * padding immediately following it.
	 */
	if (dp[2] == 0xa0000) {
		if (dp[1])
			(void) printf(" fh %d.%d.%u", dp[0], dp[1], dp[3]);
		else
			(void) printf(" fh %d.%d", dp[0], dp[3]);
	} else if ((dp[2] >> 16) == 8)
		/*
		 * 'dp' is longword aligned, so we must use the extract
		 * macros below for dp+10 which cannot possibly be aligned.
		 */
		if (dp[1])
			(void) printf(" fh %d.%d.%u", dp[0], dp[1],
				      EXTRACT_LONG((u_char *)dp + 10));
		else
			(void) printf(" fh %d.%d", dp[0],
				      EXTRACT_LONG((u_char *)dp + 10));
	/* On Ultrix pre-4.0, three longs: fsid, fno, fgen and then zeros */
	else if (dp[3] == 0) {
		(void)printf(" fh %d,%d/%d.%d", 
                             classic_major(dp[0]), 
                             classic_minor(dp[0]), 
                             dp[1], dp[2]);
	}
	/*
	 * On Ultrix 4.0,
	 * five int32s: fsid, fno, fgen, eno, egen and then zeros
	 */
	else if (dp[5] == 0) {
		(void)printf(" fh %d,%d/%d.%d", 
                             classic_major(dp[0]), 
                             classic_minor(dp[0]),
			     dp[1], dp[2]);
		if (vflag) {
			/* print additional info */
			(void)printf("[%d.%d]", dp[3], dp[4]);
		}
	}
	/*
	 * On DEC OSF/1, fsid, file, exported dir
	 * int32s: fsid[2], 12, inode, gen, 12, inode, gen
	 */
	else if (dp[2] == 12 && dp[5] == 12) {
		(void) printf(" OSF/1 fh %d,%d/%d.%d", 
                             osf_major(dp[0]), 
                             osf_minor(dp[0]),
                             dp[1], dp[3]);
		if (vflag) {
			/* print additional info */
			(void) printf(".%d/%d.%d", dp[4], dp[6], dp[7]);
		}
	}
	else
		(void) printf(" fh %u.%u.%u.%u",
		    dp[0], dp[1], dp[2], dp[3]);
}

/*
 * Print out an NFS filename.
 * Assumes that len bytes from cp are present in packet.
 */
static void
nfs_printfn(cp, len)
	register u_char *cp;
	register int len;
{
	register char c;

	/* Sanity */
	if (len >= 64) {
		fputs("[\">]", stdout);
		return;
	}
	/* Print out the filename */
	putchar('"');
	while (--len >= 0) {
		c = toascii(*cp++);
		if (!isascii(c)) {
			c = toascii(c);
			putchar('M');
			putchar('-');
		}
		if (!isprint(c)) {
			c ^= 0x40;	/* DEL to ?, others to alpha */
			putchar('^');
		}
		putchar(c);
	}
	putchar('"');
}

/*
 * Maintain a small cache of recent client.XID.server/proc pairs, to allow
 * us to match up replies with requests and thus to know how to parse
 * the reply.
 */

struct xid_map_entry {
	u_int32		xid;		/* transaction ID */
	struct in_addr	client;		/* client IP address */
	struct in_addr	server;		/* server IP address */
	u_int32		proc;		/* call procedure number */
};

/*
 * Map entries are kept in an array that we manage as a ring;
 * new entries are always added at the tail of the ring.  Initially,
 * all the entries are zero and hence don't match anything.
 */

#define	XIDMAPSIZE	64

struct xid_map_entry xid_map[XIDMAPSIZE];

u_long	xid_map_next = 0;
u_long	xid_map_hint = 0;

static void
xid_map_enter(rp, ip)
struct rpc_msg *rp;
struct ip *ip;
{
	struct xid_map_entry *xmep;
	
	xmep = &(xid_map[xid_map_next]);

	if (++xid_map_next >= XIDMAPSIZE)
	    xid_map_next = 0;

	xmep->xid = rp->rm_xid;
	xmep->client = ip->ip_src;
	xmep->server = ip->ip_dst;
	xmep->proc = rp->rm_call.cb_proc;
}

/* Returns RFS_xxx or -1 on failure */
static int32
xid_map_find(rp, ip)
struct rpc_msg *rp;
struct ip *ip;
{
	int i;
	struct xid_map_entry *xmep;
	u_int32 xid = rp->rm_xid;
	u_int32 clip = ip->ip_dst.s_addr;
	u_int32 sip = ip->ip_src.s_addr;
	
	/* Start searching from where we last left off */
	i = xid_map_hint;
	do {
	    xmep = &(xid_map[i]);
	    if ((xmep->xid == xid) && (xmep->client.s_addr == clip)
		 && (xmep->server.s_addr == sip)) {
		/* match */
		xid_map_hint = i;
		return((int32)(xmep->proc));
	    }
	    i++;
	    if (i >= XIDMAPSIZE)
		i = 0;
	} while (i != xid_map_hint);
	
	/* search failed */
	return(-1);
}

/*
 * Routines for parsing reply packets
 */

/*
 * Return a pointer to the beginning of the actual results.
 * If the packet was truncated, return 0.  Porting note (kludge alert):
 * A caddr_t in struct opaque_auth forces ar_verf to be offset by a pad
 * on 64 bit machines.  Therefore we set dp to rp_stat+1 instead of
 * rp_acpt.ar_verf.  Hey - be glad we didn't pull in all the XDR code!
 */
static u_int32 *
parserep(rp, length)
	register struct rpc_msg *rp;
	register int length;
{
	register u_int *dp = (u_int *)&rp->rm_reply.rp_stat + 1;
	register u_int32 *ep = (u_int32 *)snapend;
	enum accept_stat astat;

	NTOHL(dp[1]);
	/*
	 * skip past the ar_verf credentials.
	 */
	if (dp >= ep || dp[1] >= length)
	    return(0);

	dp += 2 + (dp[1] + 3) / sizeof(u_int32);
	if (dp >= ep)
	    return(0);

	/*
	 * now we can check the ar_stat field
	 */
	astat = ntohl(*(enum accept_stat *)dp);
	switch (astat) {
	case SUCCESS:
	    break;
	case PROG_UNAVAIL:
	    printf(" PROG_UNAVAIL");
	    return(0);
	case PROG_MISMATCH:
	    printf(" PROG_MISMATCH");
	    return(0);
	case PROC_UNAVAIL:
	    printf(" PROC_UNAVAIL");
	    return(0);
	case GARBAGE_ARGS:
	    printf(" GARBAGE_ARGS");
	    return(0);
	case SYSTEM_ERR:
	    printf(" SYSTEM_ERR");
	    return(0);
	default:
	    printf(" ar_stat %d", astat);
	    return(0);
	}
	
	/* successful return */
	if ((sizeof(astat) + ((char *)dp)) < (char *)ep)
	    return((u_int32 *) (sizeof(astat) + ((char *)dp)));
	else
	    return(0);
}

#define T2CHECK(p, l) if ((u_char *)(p) > ((u_char *)snapend) - l) _longjmp(nfs_short, 1)

static u_int32 *
parsestatus(dp)
u_int32 *dp;
{
	int errno;
	T2CHECK(dp, sizeof(enum nfsstat));

	NTOHL(dp[0]);
	errno = geterrno((*(enum nfsstat *)dp));

	if (errno) {
	    char *errmsg;
	    
	    if (qflag)
		return(0);

	    errmsg = strerror(errno);
	    if (errmsg)
		printf(" ERROR: %s", errmsg);
	    else
		printf(" ERROR: %d", errno);
	    return(0);
	}

	return(dp + (sizeof(enum nfsstat)/sizeof(u_int32)));
}

static u_int32 *
parsefattr(dp, verbose)
u_int32 *dp;
int verbose;
{
	struct nfsfattr *fap;

	T2CHECK(dp, sizeof(struct nfsfattr));
	
	fap = (struct nfsfattr *)dp;
#if BYTE_ORDER == LITTLE_ENDIAN
	bswap(fap, sizeof(*fap)/sizeof(int32));
#endif
	
	if (verbose) {
	    putchar(' ');
	    switch (fap->na_type) {
	    case NFNON:
		printf("NON ");
		break;
	    case NFREG:
		printf("REG ");
		break;
	    case NFDIR:
		printf("DIR ");
		break;
	    case NFBLK:
		printf("BLK ");
		break;
	    case NFCHR:
		printf("CHR ");
		break;
	    case NFLNK:
		printf("LNK ");
		break;
	    default:
		printf("unk-ft %d ", fap->na_type);
		break;
	    }
	    printf("%o ids %d/%d sz %d ",
			fap->na_mode, fap->na_uid, fap->na_gid,
			fap->na_size);
	}

	/* print lots more stuff */
	if (verbose > 1) {
	    int i;
	    printf("nlink %d rdev %x fsid %x nodeid %x a/m/ctime ",
			fap->na_nlink, fap->na_rdev, fap->na_fsid,
			fap->na_nodeid);
	    printf("%d.%06d ", fap->na_atime.tv_sec, fap->na_atime.tv_usec);
	    printf("%d.%06d ", fap->na_mtime.tv_sec, fap->na_mtime.tv_usec);
	    printf("%d.%06d ", fap->na_ctime.tv_sec, fap->na_ctime.tv_usec);
	}

	return((u_int32 *)&(fap[1]));
}

static int32
parseattrstat(dp, verbose)
u_int32 *dp;
int verbose;
{
	dp = parsestatus(dp);
	if (dp == NULL)
	    return(0);

	return((int32)parsefattr(dp, verbose));
}

static int32
parsediropres(dp)
u_int32 *dp;
{
	dp = parsestatus(dp);
	if (dp == NULL)
	    return(0);

	dp = parsefh(dp);
	if (dp == NULL)
	    return(0);
	
	return(parsefattr(dp, vflag) != NULL);
}

static int32
parselinkres(dp)
u_int32 *dp;
{
	dp = parsestatus(dp);
	if (dp == NULL)
	    return(0);

	putchar(' ');
	return(parsefn(dp) != NULL);
}

static int32
parsestatfs(dp)
u_int32 *dp;
{
	struct nfsstatfsok *sfsp;

	dp = parsestatus(dp);
	if (dp == NULL)
	    return(0);

	if (qflag)
	    return(1);

	T2CHECK(dp, sizeof(*sfsp));

	sfsp = (struct nfsstatfsok *)dp;
#if BYTE_ORDER == LITTLE_ENDIAN
	bswap(sfsp, sizeof(*sfsp)/sizeof(int32));
#endif

	printf(" tsize %d bsize %d blocks %d bfree %d bavail %d",
		sfsp->fsok_tsize, sfsp->fsok_bsize, sfsp->fsok_blocks,
		sfsp->fsok_bfree, sfsp->fsok_bavail);

	return(1);
}

static int32
parserddires(dp)
u_int32 *dp;
{
	struct nfsrdok *rdp;

	dp = parsestatus(dp);
	if (dp == NULL)
	    return(0);

	if (qflag)
	    return(1);

	T2CHECK(dp, sizeof(*rdp));

	rdp = (struct nfsrdok *)dp;
#if BYTE_ORDER == LITTLE_ENDIAN
	bswap(rdp, sizeof(*rdp)/sizeof(int32));
#endif

	printf(" offset %x size %d ",
		rdp->rdok_offset, rdp->rdok_size);
	if (rdp->rdok_eof)
	    printf("eof");

	return(1);
}

static
interp_reply(rp, proc, length)
struct rpc_msg *rp;
u_int32 proc;		/* RPC procedure number */
int length;
{
	register u_int32 *dp;
	register u_char *ep = snapend;

	if (setjmp(nfs_short)) {
		fputs(" [|nfs]", stdout);
		return;
	}
	switch (proc) {

#ifdef NFSPROC_NOOP
	case NFSPROC_NOOP:
		printf(" nop");
		break;
#else
#define NFSPROC_NOOP -1
#endif
	case RFS_NULL:
		printf(" null");
		break;

	case RFS_GETATTR:
		printf(" getattr");
		if ((dp = parserep(rp, length)) != 0)
			parseattrstat(dp, !qflag);
		break;

	case RFS_SETATTR:
		printf(" setattr");
		if ((dp = parserep(rp, length)) != 0)
			parseattrstat(dp, !qflag);
		break;

#if RFS_ROOT != NFSPROC_NOOP
	case RFS_ROOT:
		printf(" root");
		break;
#endif
	case RFS_LOOKUP:
		printf(" lookup");
		if ((dp = parserep(rp, length)) != 0)
			parsediropres(dp);
		break;

	case RFS_READLINK:
		printf(" readlink");
		if ((dp = parserep(rp, length)) != 0)
			parselinkres(dp);
		break;

	case RFS_READ:
		printf(" read");
		if ((dp = parserep(rp, length)) != 0)
			parseattrstat(dp, vflag);
		break;

#if RFS_WRITECACHE != NFSPROC_NOOP
	case RFS_WRITECACHE:
		printf(" writecache");
		break;
#endif
	case RFS_WRITE:
		printf(" write");
		if ((dp = parserep(rp, length)) != 0)
			parseattrstat(dp, vflag);
		break;

	case RFS_CREATE:
		printf(" create");
		if ((dp = parserep(rp, length)) != 0)
			parsediropres(dp);
		break;

	case RFS_REMOVE:
		printf(" remove");
		if ((dp = parserep(rp, length)) != 0)
			parsestatus(dp);
		break;

	case RFS_RENAME:
		printf(" rename");
		if ((dp = parserep(rp, length)) != 0)
			parsestatus(dp);
		break;

	case RFS_LINK:
		printf(" link");
		if ((dp = parserep(rp, length)) != 0)
			parsestatus(dp);
		break;

	case RFS_SYMLINK:
		printf(" symlink");
		if ((dp = parserep(rp, length)) != 0)
			parsestatus(dp);
		break;

	case RFS_MKDIR:
		printf(" mkdir");
		if ((dp = parserep(rp, length)) != 0)
			parsediropres(dp);
		break;

	case RFS_RMDIR:
		printf(" rmdir");
		if ((dp = parserep(rp, length)) != 0)
			parsestatus(dp);
		break;

	case RFS_READDIR:
		printf(" readdir");
		if ((dp = parserep(rp, length)) != 0)
			parserddires(dp);
		break;

	case RFS_STATFS:
		printf(" statfs");
		if ((dp = parserep(rp, length)) != 0)
			parsestatfs(dp);
		break;

	default:
		printf(" proc-%u", proc);
	}
}
