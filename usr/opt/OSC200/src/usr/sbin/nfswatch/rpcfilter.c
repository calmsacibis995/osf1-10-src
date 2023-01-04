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
static char *rcsid = "@(#)$RCSfile: rpcfilter.c,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1993/12/21 23:36:10 $";
#endif
/* Based on:
 * /home/harbor/davy/system/nfswatch/RCS/rpcfilter.c,v 4.0 1993/03/01 19:59:00 davy Exp $";
 */

#include "os.h"

/*
 * rpcfilter.c - filter RPC packets.
 *
 * David A. Curry				Jeffrey C. Mogul
 * Purdue University				Digital Equipment Corporation
 * Engineering Computer Network			Western Research Laboratory
 * 1285 Electrical Engineering Building		250 University Avenue
 * West Lafayette, IN 47907-1285		Palo Alto, CA 94301
 * davy@ecn.purdue.edu				mogul@decwrl.dec.com
 *
 * log: rpcfilter.c,v
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 3.7  1993/03/01  19:57:23  davy
 * Fixed some byte-order bugs.
 *
 * Revision 3.6  1993/02/24  17:44:45  davy
 * Added -auth mode, changes to -proc mode, -map option, -server option.
 *
 * Revision 3.5  1993/01/16  19:08:59  davy
 * Corrected Jeff's address.
 *
 * Revision 3.4  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 3.3  1993/01/15  15:43:36  davy
 * Assorted changes for porting to Solaris 2.x/SVR4.
 *
 * Revision 3.2  1993/01/13  20:18:17  davy
 * Put in OS-specific define scheme, and merged in Tim Hudson's code for
 * SGI systems (as yet untested).
 *
 * Revision 3.1  1993/01/13  13:50:59  davy
 * Made the file matching code work on Sun-3's, and also put a generic one in
 * there in hopes it'll work.
 *
 * Revision 3.0  1991/01/23  08:23:20  davy
 * NFSWATCH Version 3.0.
 *
 * Revision 1.5  91/01/17  10:13:02  davy
 * Bug fix from Jeff Mogul.
 * 
 * Revision 1.7  91/01/16  15:49:12  mogul
 * Print server or client address in a.b.c.d notation if name not known
 * 
 * Revision 1.6  91/01/07  15:35:51  mogul
 * Uses hash table instead of linear search on clients
 * One-element "hint" cache to avoid client hash lookup
 * 
 * Revision 1.5  91/01/04  14:12:35  mogul
 * Support for client counters
 * Disable screen update during database upheaval
 * 
 * Revision 1.4  91/01/03  17:35:00  mogul
 * Count per-procedure info
 * 
 * Revision 1.3  90/12/04  08:22:06  davy
 * Fix from Dan Trinkle (trinkle@cs.purdue.edu) to determine byte order in
 * file handle.
 * 
 * Revision 1.2  90/08/17  15:47:44  davy
 * NFSWATCH Version 2.0.
 * 
 * Revision 1.1  88/11/29  11:20:51  davy
 * NFSWATCH Release 1.0
 * 
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#ifdef SVR4
#include <sys/tiuser.h>
#include <sys/sysmacros.h>
#endif
#include <rpc/types.h>
#include <rpc/xdr.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>
#include <rpc/rpc_msg.h>
#include <rpc/pmap_clnt.h>
#include <rpc/svc.h>
#include <netdb.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <math.h>
#include <pwd.h>

#define NFSSERVER	1

#ifdef sun
#include <sys/vfs.h>
#include <nfs/nfs.h>
#endif
#ifdef ultrix
#include <sys/types.h>
#include <sys/time.h>
#include <nfs/nfs.h>
#endif
#ifdef sgi
#include <sys/time.h>
#include "sgi.map.h"
#include <sys/sysmacros.h>
#endif
#ifdef __alpha
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mount.h>
#include <nfs/nfs.h>
#endif

#include "nfswatch.h"
#include "externs.h"
#include "rpcdefs.h"

/*
 * NFS procedure types and XDR argument decoding routines.
 */
static struct nfs_proc nfs_procs[] = {
/* RFS_NULL (0)		*/
	NFS_READ,	xdr_void,	0,
/* RFS_GETATTR (1)	*/
	NFS_READ,	xdr_fhandle,	sizeof(fhandle_t),
/* RFS_SETATTR (2)	*/
	NFS_WRITE,	xdr_saargs,	sizeof(struct nfssaargs),
/* RFS_ROOT (3)		*/
	NFS_READ,	xdr_void,	0,
/* RFS_LOOKUP (4)	*/
	NFS_READ,	xdr_diropargs,	sizeof(struct nfsdiropargs),
/* RFS_READLINK (5)	*/
	NFS_READ,	xdr_fhandle,	sizeof(fhandle_t),
/* RFS_READ (6)		*/
	NFS_READ,	xdr_readargs,	sizeof(struct nfsreadargs),
/* RFS_WRITECACHE (7)	*/
	NFS_WRITE,	xdr_void,	0,
/* RFS_WRITE (8)	*/
	NFS_WRITE,	xdr_writeargs,	sizeof(struct nfswriteargs),
/* RFS_CREATE (9)	*/
	NFS_WRITE,	xdr_creatargs,	sizeof(struct nfscreatargs),
/* RFS_REMOVE (10)	*/
	NFS_WRITE,	xdr_diropargs,	sizeof(struct nfsdiropargs),
/* RFS_RENAME (11)	*/
	NFS_WRITE,	xdr_rnmargs,	sizeof(struct nfsrnmargs),
/* RFS_LINK (12)	*/
	NFS_WRITE,	xdr_linkargs,	sizeof(struct nfslinkargs),
/* RFS_SYMLINK (13)	*/
	NFS_WRITE,	xdr_slargs,	sizeof(struct nfsslargs),
/* RFS_MKDIR (14)	*/
	NFS_WRITE,	xdr_creatargs,	sizeof(struct nfscreatargs),
/* RFS_RMDIR (15)	*/
	NFS_WRITE,	xdr_diropargs,	sizeof(struct nfsdiropargs),
/* RFS_READDIR (16)	*/
	NFS_READ,	xdr_rddirargs,	sizeof(struct nfsrddirargs),
/* RFS_STATFS (17)	*/
	NFS_READ,	xdr_fhandle,	sizeof(fhandle_t)
};

NFSCall nfs_calls[NFSCALLHASHSIZE];

/*
 * rpc_filter - pass off RPC packets to other filters.
 */
void
rpc_filter(data, length, src, dst, tstamp)
register ipaddrt src, dst;
struct timeval *tstamp;
register u_int length;
register char *data;
{
	register struct rpc_msg *msg;

	msg = (struct rpc_msg *) data;

	/*
	 * See which "direction" the packet is going.  We
	 * can classify RPC CALLs, but we cannot classify
	 * REPLYs, since they no longer have the RPC
	 * program number in them (sigh).
	 */
	switch (ntohl(msg->rm_direction)) {
	case CALL:			/* RPC call			*/
		rpc_callfilter(data, length, src, dst, tstamp);
		break;
	case REPLY:			/* RPC reply			*/
		rpc_replyfilter(data, length, src, dst, tstamp);
		break;
	default:			/* probably not an RPC packet	*/
		break;
	}
}

/*
 * rpc_callfilter - filter RPC call packets.
 */
void
rpc_callfilter(data, length, src, dst, tstamp)
register ipaddrt src, dst;
struct timeval *tstamp;
register u_int length;
register char *data;
{
	register struct rpc_msg *msg;

	msg = (struct rpc_msg *) data;

	/*
	 * Decide what to do based on the program.
	 */
	switch (ntohl(msg->rm_call.cb_prog)) {
	case RPC_NFSPROG:
		nfs_filter(data, length, src, dst, tstamp);
		break;
	case RPC_YPPROG:
	case RPC_YPBINDPROG:
	case RPC_YPPASSWDPROG:
	case RPC_YPUPDATEPROG:
	case RPC_CACHEPROG:
	case RPC_CB_PROG:
		pkt_counters[PKT_YELLOWPAGES].pc_interval++;
		pkt_counters[PKT_YELLOWPAGES].pc_total++;
		break;
	case RPC_MOUNTPROG:
		pkt_counters[PKT_NFSMOUNT].pc_interval++;
		pkt_counters[PKT_NFSMOUNT].pc_total++;
		break;
#ifdef notdef
	case RPC_PMAPPROG:
	case RPC_RSTATPROG:
	case RPC_RUSERSPROG:
	case RPC_DBXPROG:
	case RPC_WALLPROG:
	case RPC_ETHERSTATPROG:
	case RPC_RQUOTAPROG:
	case RPC_SPRAYPROG:
	case RPC_IBM3270PROG:
	case RPC_IBMRJEPROG:
	case RPC_SELNSVCPROG:
	case RPC_RDATABASEPROG:
	case RPC_REXECPROG:
	case RPC_ALICEPROG:
	case RPC_SCHEDPROG:
	case RPC_LOCKPROG:
	case RPC_NETLOCKPROG:
	case RPC_X25PROG:
	case RPC_STATMON1PROG:
	case RPC_STATMON2PROG:
	case RPC_SELNLIBPROG:
	case RPC_BOOTPARAMPROG:
	case RPC_MAZEPROG:
	case RPC_KEYSERVEPROG:
	case RPC_SECURECMDPROG:
	case RPC_NETFWDIPROG:
	case RPC_NETFWDTPROG:
	case RPC_SUNLINKMAP_PROG:
	case RPC_NETMONPROG:
	case RPC_DBASEPROG:
	case RPC_PWDAUTHPROG:
	case RPC_TFSPROG:
	case RPC_NSEPROG:
	case RPC_NSE_ACTIVATE_PROG:
	case RPC_PCNFSDPROG:
	case RPC_PYRAMIDLOCKINGPROG:
	case RPC_PYRAMIDSYS5:
	case RPC_CADDS_IMAGE:
	case RPC_ADT_RFLOCKPROG:
#endif
	default:
		pkt_counters[PKT_OTHERRPC].pc_interval++;
		pkt_counters[PKT_OTHERRPC].pc_total++;
		break;
	}
}

/*
 * rpc_replyfilter - count RPC reply packets.
 */
void
rpc_replyfilter(data, length, src, dst, tstamp)
register ipaddrt src, dst;
struct timeval *tstamp;
register u_int length;
register char *data;
{
	register struct rpc_msg *msg;

	msg = (struct rpc_msg *) data;

	pkt_counters[PKT_RPCAUTH].pc_interval++;
	pkt_counters[PKT_RPCAUTH].pc_total++;
	nfs_hash_reply(msg, dst, tstamp);
}

/*
 * nfs_filter - filter NFS packets.
 */
void
nfs_filter(data, length, src, dst, tstamp)
register ipaddrt src, dst;
struct timeval *tstamp;
register u_int length;
register char *data;
{
	u_int proc;
	caddr_t args;
	SVCXPRT *xprt;
	struct rpc_msg msg;
	union nfs_rfsargs nfs_rfsargs;
	char cred_area[2*MAX_AUTH_BYTES];

	msg.rm_call.cb_cred.oa_base = cred_area;
	msg.rm_call.cb_verf.oa_base = &(cred_area[MAX_AUTH_BYTES]);

	/*
	 * Act as if we received this packet through RPC.
	 */
	if (!udprpc_recv(data, length, &msg, &xprt))
		return;

	/*
	 * Get the NFS procedure number.
	 */
	proc = msg.rm_call.cb_proc;

	if (proc >= RFS_NPROC)
		return;

	CountCallAuth(&msg);
	nfs_hash_call(&msg, src, tstamp);

	/*
	 * Now decode the arguments to the procedure from
	 * XDR format.
	 */
	args = (caddr_t) &nfs_rfsargs;
	(void) bzero(args, nfs_procs[proc].nfs_argsz);

	if (!SVC_GETARGS(xprt, nfs_procs[proc].nfs_xdrargs, args))
		return;

	prc_counters[prc_countmap[proc]].pr_total++;
	prc_counters[prc_countmap[proc]].pr_interval++;

	CountSrc(src);

	/*
	 * Now count the packet in the appropriate file system's
	 * counters.
	 */
	switch (proc) {
	case RFS_NULL:
		break;
	case RFS_GETATTR:
		nfs_count(&nfs_rfsargs.fhandle, proc);
		break;
	case RFS_SETATTR:
		nfs_count(&nfs_rfsargs.nfssaargs.saa_fh, proc);
		break;
	case RFS_ROOT:
		break;
	case RFS_LOOKUP:
		nfs_count(&nfs_rfsargs.nfsdiropargs.da_fhandle, proc);
		break;
	case RFS_READLINK:
		nfs_count(&nfs_rfsargs.fhandle, proc);
		break;
	case RFS_READ:
		nfs_count(&nfs_rfsargs.nfsreadargs.ra_fhandle, proc);
		break;
	case RFS_WRITECACHE:
		break;
	case RFS_WRITE:
		nfs_count(&nfs_rfsargs.nfswriteargs.wa_fhandle, proc);
		break;
	case RFS_CREATE:
		nfs_count(&nfs_rfsargs.nfscreatargs.ca_da.da_fhandle, proc);
		break;
	case RFS_REMOVE:
		nfs_count(&nfs_rfsargs.nfsdiropargs.da_fhandle, proc);
		break;
	case RFS_RENAME:
		nfs_count(&nfs_rfsargs.nfsrnmargs.rna_from.da_fhandle, proc);
		break;
	case RFS_LINK:
		nfs_count(&nfs_rfsargs.nfslinkargs.la_from, proc);
		break;
	case RFS_SYMLINK:
		nfs_count(&nfs_rfsargs.nfsslargs.sla_from.da_fhandle, proc);
		break;
	case RFS_MKDIR:
		nfs_count(&nfs_rfsargs.nfscreatargs.ca_da.da_fhandle, proc);
		break;
	case RFS_RMDIR:
		nfs_count(&nfs_rfsargs.nfsdiropargs.da_fhandle, proc);
		break;
	case RFS_READDIR:
		nfs_count(&nfs_rfsargs.nfsrddirargs.rda_fh, proc);
		break;
	case RFS_STATFS:
		nfs_count(&nfs_rfsargs.fhandle, proc);
		break;
	}

	/*
	 * Decide whether it's a read or write process.
	 */
	switch (nfs_procs[proc].nfs_proctype) {
	case NFS_READ:
		pkt_counters[PKT_NFSREAD].pc_interval++;
		pkt_counters[PKT_NFSREAD].pc_total++;
		break;
	case NFS_WRITE:
		pkt_counters[PKT_NFSWRITE].pc_interval++;
		pkt_counters[PKT_NFSWRITE].pc_total++;
		break;
	}
}

#define nfsmajor(x)	((int)(((unsigned)(x)>>8)&0377))
#define nfsminor(x)	((int)((x)&0377))
#define nfsmakedev(x,y)	((dev_t)(((x)<<8) | (y)))
#define osfmajor(x)	((int)  (((dev_t)(x)>>20)&07777))
#define osfminor(x)	((int)  ((dev_t)(x)&03777777))
#define osfmakedev(x,y)	((dev_t)(((int)(x)<<20) | (int)(y)))

#ifdef DEBUG
static int call;
#endif
/*
 * nfs_count - count an NFS reference to a specific file system.
 */
void
nfs_count(fh, proc)
register fhandle_t *fh;
int proc;
{
	uint32 fsid;
	register int i, match1, match2;
#ifdef DEBUG
call++;
#endif

	/*
	 * Run through the NFS counters looking for the matching
	 * file system.
	 */
	match1 = 0;

	for (i = 0; i < nnfscounters; i++) {
		if (learnfs)
			fsid = nfs_counters[i].nc_fsid;
		else
			fsid = (uint32) nfs_counters[i].nc_dev;  

		/*
		 * Compare the device numbers.  Sun uses an
		 * fsid_t for the device number, which is an
		 * array of 2 longs.  The first long contains
		 * the device number.
		 */
		match1 = !bcmp((char *) &(fh->fh_fsid), (char *) &fsid,
			sizeof(uint32));

#ifdef DEBUG
fprintf(stderr,"fh: input: 0x%x table: 0x%x match:%d call:%d\n",
  fh->fh_fsid,fsid,match1,call);
#endif

		/*
		 * Check server address.
		 */
		if (allflag && match1)
			match1 = (thisdst == nfs_counters[i].nc_ipaddr);

#ifdef DEBUG
if (allflag && match1)
  fprintf(stderr,"ip: input: 0x%x table: 0x%x match:%d call:%d\n",thisdst, 
    nfs_counters[i].nc_ipaddr, match1,call);
#endif


		if (match1) {
			nfs_counters[i].nc_proc[proc]++;
			nfs_counters[i].nc_interval++;
			nfs_counters[i].nc_total++;
			break;
		}
	}

	/*
	 * We don't know about this file system, but we can
	 * learn.
	 */
	if (!match1 && learnfs && (nnfscounters < MAXEXPORT)) {
		static char fsname[64], prefix[64];
		uint32 fsid;
		int oldm;
		int osf_fh;

#ifdef SVR4
		sighold(SIGALRM);
#else
		oldm = sigblock(sigmask(SIGALRM));
#endif
	    				/* no redisplay while unstable */

		i = nnfscounters++;

		bcopy((char *) &(fh->fh_fsid), (char *) &fsid, sizeof(uint32));

		nfs_counters[i].nc_fsid = fsid;
		nfs_counters[i].nc_proc[proc]++;
		nfs_counters[i].nc_interval++;
		nfs_counters[i].nc_total++;

#ifdef DEBUG
fprintf(stderr,"\noriginal fsid: 0x%x",fsid);
#endif

		/*
		 * See if server uses opposite byte order.
		 */
		if ((fsid & 0xffff0000) && ((fsid & 0xffff) == 0))
			fsid = ntohl(fsid);

#ifdef DEBUG
fprintf(stderr,", swapped: 0x%x\n",fsid);
fprintf(stderr,"fh_fid.fid_len: 0x%x, fh_fid.fid_reserved: 0x%x\n",
    fh->fh_fid.fid_len&0xffff,fh->fh_fid.fid_reserved&0xffff);
fprintf(stderr,"fh_efid.fid_len: 0x%x, fh_efid.fid_reserved: 0x%x\n",
    fh->fh_efid.fid_len&0xffff,fh->fh_efid.fid_reserved&0xffff);
fprintf(stderr,"fh[2]: 0x%x, fh[5]: 0x%x\n",
    (((uint32 *)fh)[2]), (((uint32 *)fh)[5]));
#endif

		/*
		 * Some hosts use 32-bit values.
		 */
		/*
		 * Special case the OSF fh's first...
		 * 
		 * OSF uses a 12 bit major, 20 bit minor.
		 * There is no nice zero bytes that may be used
		 * to detect endianness, etc.  We need to dig
		 * deeper into the fh... 
		 * 
		 */
		osf_fh = 0;
		if ((((uint32 *)fh)[2] == 0xc) &&
		    (((uint32 *)fh)[5] == 0xc)) {
			 osf_fh = 1;
			 nfs_counters[i].nc_dev = osfmakedev(osfmajor(fsid),
			     osfminor(fsid));
		} else if ((((uint32 *)fh)[2] == 0xc0000) &&
			   (((uint32 *)fh)[5] == 0xc0000)) {
			 osf_fh = -1;
			 /* test me! (on a sun...) */
			 nfs_counters[i].nc_dev = osfmakedev((fsid >> 4)&0xfff,
			   ((fsid & 0xf)<<16)|(fsid >> 16)&0xffff);
		} else if (fsid & 0xffff0000) {
			/*
			 * Try to intuit the byte order.
			 */
			if (fsid & 0xff00) {
			  nfs_counters[i].nc_dev = nfsmakedev((fsid >> 8)&0xff,
							   (fsid >> 24)&0xff);
			}
			else {
			  nfs_counters[i].nc_dev = nfsmakedev((fsid >> 16)&0xff,
							   fsid&0xff);
			}
		}
		else {
			nfs_counters[i].nc_dev = nfsmakedev(nfsmajor(fsid),
							 nfsminor(fsid));
		}

		*prefix = 0;

		if (allflag) {
			struct hostent *hp;

			nfs_counters[i].nc_ipaddr = thisdst;
			hp = gethostbyaddr((char *) &thisdst, sizeof(thisdst),
					   AF_INET);

			if (hp) {
				char *index();
				char *dotp;

				sprintf(prefix, "%s", hp->h_name);

				if ((dotp = index(prefix, '.')) != NULL)
					*dotp = 0;
			}
			else {
				struct in_addr ia;
				ia.s_addr = thisdst;
				sprintf(prefix, "%s", inet_ntoa(ia));
			}
		}

		if (osf_fh) 
		    sprintf(fsname, "%.12s(%d,%d)", prefix,
			    osfmajor(nfs_counters[i].nc_dev),
			    osfminor(nfs_counters[i].nc_dev));
		else
		    sprintf(fsname, "%.12s(%d,%d)", prefix,
			    nfsmajor(nfs_counters[i].nc_dev),
			    nfsminor(nfs_counters[i].nc_dev));

#ifdef DEBUG
if (osf_fh) 
  fprintf(stderr, "osf fsid is %.12s(%d,%d) (%d)\n", prefix,
    osfmajor(nfs_counters[i].nc_dev),
    osfminor(nfs_counters[i].nc_dev), osf_fh);
else
  fprintf(stderr, "nfs fsid is %.12s(%d,%d)\n", prefix,
    nfsmajor(nfs_counters[i].nc_dev),
    nfsminor(nfs_counters[i].nc_dev));
fflush(stderr);
#endif

		if (mapfile)
			nfs_counters[i].nc_name = savestr(map_fs_name(fsname));
		else
			nfs_counters[i].nc_name = savestr(fsname);

		sort_nfs_counters();
#ifdef SVR4
		sigrelse(SIGALRM);
#else
		(void) sigsetmask(oldm);	/* permit redisplay */
#endif
	}

	if (filelist == NULL)
		return;

	/*
	 * Run through the file counters looking for the matching
	 * file.
	 */
	for (i = 0; i < nfilecounters; i++) {
		fsid = (uint32) fil_counters[i].fc_dev;

		/*
		 * Compare device numbers and file numbers.  Sun
		 * uses an fsid_t for the device, which is an
		 * array of two longs.  They use an fid for the
		 * inode.  The inode number is the first part
		 * of this.
		 */
		match1 = !bcmp((char *) &(fh->fh_fsid), (char *) &fsid,
			 sizeof(uint32));

#ifdef DEBUG
fprintf(stderr,"xx: input: 0x%x table: 0x%x match:%d call:%d\n",
  fh->fh_fsid,fsid,match1,call);
#endif

		if (!match1)
			continue;

#if defined(sun) && defined(sparc)
		/*
		 * NOTE: this is dependent on the contents of the fh_data
		 *       part of the file handle.  This is correct for
		 *       SunOS 4.1 on SPARCs.
		 */
		match2 = !bcmp((char *) &(fh->fh_data[2]),
			 (char *) &(fil_counters[i].fc_ino), sizeof(ino_t));
#endif

#if defined(sun) && defined(mc68000)
		/*
		 * Correct for SunOS 4.1 on Sun-3.
		 */
		match2 = !bcmp((char *) &(fh->fh_data[0]),
			 (char *) &(fil_counters[i].fc_ino), sizeof(ino_t));
#endif

#ifdef ultrix
		/*
		 * Correct for Ultrix 4.x systems.
		 */
		match2 = !bcmp((char *) fh->fh_fno,
			 (char *) &(fil_counters[i].fc_ino), sizeof(ino_t));
#endif

#ifdef __alpha
		/*
		 * Correct for Alpha OSF/1 systems (using 32 bit nfs)
		 */
		match2 = !bcmp(fh->fh_fid.fid_data, 
			(char *) &(fil_counters[i].fc_ino), sizeof(ino_t));
#endif

#ifdef sgi
		/*
		 * Correct for IRIX 3.3.
		 */
		match2 = !bcmp((char *) &(fh->fh_data[4]),
			 (char *) &(fil_counters[i].fc_ino), sizeof(ino_t));
#endif

#if !defined(sgi) && !defined(sun) && !defined(ultrix) && !defined(__alpha)
		/*
		 * This is a guess... it's probably correct for most
		 * SVR4 PC systems, anyway.
		 */
		match2 = !bcmp((char *) &(fh->fh_data[0]),
			 (char *) &(fil_counters[i].fc_ino), sizeof(ino_t));
#endif
		
		if (match2) {
			fil_counters[i].fc_proc[proc]++;
			fil_counters[i].fc_interval++;
			fil_counters[i].fc_total++;
			break;
		}
	}
}

/*
 * CountSrc uses a hash table to speed lookups.  Hash function
 *	uses high and low octect of IP address, so as to be
 *	fast and byte-order independent.  Table is organized
 *	as a array of linked lists.
 */
#define	HASHSIZE	0x100
#define	HASH(addr)	(((addr) & 0xFF) ^ (((addr) >> 24) & 0xFF))

ClientCounter *Addr_hashtable[HASHSIZE];	/* initially all NULL ptrs */

ClientCounter *cc_hint = clnt_counters;		/* one-element cache */

CountSrc(src)
register ipaddrt src;
{
	register ClientCounter *ccp;
	int hcode = HASH(src);
	
	/* See if this is the same client as last time */
	if (cc_hint->cl_ipaddr == src) {
	    cc_hint->cl_total++;
	    cc_hint->cl_interval++;
	    return;
	}

	/* Search hash table */
	ccp = Addr_hashtable[hcode];
	while (ccp) {
	    if (ccp->cl_ipaddr == src) {
		ccp->cl_total++;
		ccp->cl_interval++;
		cc_hint = ccp;
		return;
	    }
	    ccp = ccp->cl_next;
	}
	
	/* new client */
	if (nclientcounters < MAXCLIENTS) {
	    struct hostent *hp;
	    static char clnt_name[64];
	    int oldm;
	    
#ifdef SVR4
	    sighold(SIGALRM);
#else
	    oldm = sigblock(sigmask(SIGALRM));
#endif
	    				/* no redisplay while unstable */
	    
	    ccp = &(clnt_counters[nclientcounters]);
	    nclientcounters++;
	    
	    /* Add to hash table */
	    ccp->cl_next = Addr_hashtable[hcode];
	    Addr_hashtable[hcode] = ccp;

	    /* Fill in new ClientCounter */
	    ccp->cl_ipaddr = src;
	    hp = gethostbyaddr((char *) &ccp->cl_ipaddr,
			       sizeof(ccp->cl_ipaddr), AF_INET);
	    if (hp) {
		char *index();
		char *dotp;

		sprintf(clnt_name, "%s", hp->h_name);

		if ((dotp = index(clnt_name, '.')) != NULL)
			*dotp = 0;
	    }
	    else {
		struct in_addr ia;
		ia.s_addr = ccp->cl_ipaddr;
		sprintf(clnt_name, "%s", inet_ntoa(ia));
	    }

	    ccp->cl_name = savestr(clnt_name);
	    ccp->cl_total = 1;
	    ccp->cl_interval = 1;
	    sort_clnt_counters();
#ifdef SVR4
	     sigrelse(SIGALRM);
#else
	    (void) sigsetmask(oldm);	/* permit redisplay */
#endif
	}
}

/*
 * Must be called after sorting the clnt_counters[] table
 *	Should put busiest ones at front of list, but doesn't
 */
ClientHashRebuild()
{
	register int i;
	register ClientCounter *ccp;
	int hcode;

	bzero(Addr_hashtable, sizeof(Addr_hashtable));
	
	for (i = 0, ccp = clnt_counters; i < nclientcounters; i++, ccp++) {
	    hcode = HASH(ccp->cl_ipaddr);
	    ccp->cl_next = Addr_hashtable[hcode];
	    Addr_hashtable[hcode] = ccp;
	}
}

/*
 * Code to count authentication instances.
 */
#define UIDOFFSET	100000		/* add to a non-uid so it won't
					   conflict with uids		*/

CountCallAuth(msg)
struct rpc_msg *msg;
{
	int i, len, auth;
	struct passwd *pw;
	unsigned int *oauth;
	static char user_name[16];

	auth = msg->ru.RM_cmb.cb_cred.oa_flavor;

	if (auth != AUTH_UNIX) {
		auth += UIDOFFSET;
	}
	else {
		/*
		 * Convert the opaque authorization into a uid.
		 * Should use the XDR decoders.
		 */
		oauth = (unsigned int *) msg->ru.RM_cmb.cb_cred.oa_base;
		len = ntohl(oauth[1]);

		if ((len % 4) != 0)
			len = len + 4 - len % 4;

		auth = ntohl(oauth[2 + len / 4]);
	}

	for (i=0; i < nauthcounters; i++) {
		if (auth_counters[i].ac_uid == auth) {
			auth_counters[i].ac_interval++;
			auth_counters[i].ac_total++;
			return;
		}
	}
	
	if (nauthcounters < MAXAUTHS) {
		nauthcounters++;

		auth_counters[i].ac_uid = auth;
		auth_counters[i].ac_total = 1;
		auth_counters[i].ac_interval = 1;

		switch (auth) {
		case AUTH_NULL + UIDOFFSET:
			auth_counters[i].ac_name = "AUTH_NULL";
			break;
		case AUTH_UNIX + UIDOFFSET:
			auth_counters[i].ac_name = "AUTH_UNIX";
			break;
		case AUTH_SHORT + UIDOFFSET:
			auth_counters[i].ac_name = "AUTH_SHORT";
			break;
		case AUTH_DES + UIDOFFSET:
			auth_counters[i].ac_name = "AUTH_DES";
			break;
		default:
			if ((pw = getpwuid(auth)) != NULL)
				strcpy(user_name, pw->pw_name);
			else
				sprintf(user_name, "#%d", auth);

			auth_counters[i].ac_name = savestr(user_name);
			break;
		}

		sort_auth_counters();
	}
}

/*
 * Some code to look at response times.
 */
void
nfs_hash_call(msg, client, tstamp)
struct timeval *tstamp;
struct rpc_msg *msg;
ipaddrt client;
{
	int i;

	if (tstamp == NULL)
		return;

	for (i=0; i < NFSCALLHASHSIZE; i++) {
		if (nfs_calls[i].used == 0) {
			nfs_calls[i].used = 1;
			nfs_calls[i].proc = msg->rm_call.cb_proc;
			nfs_calls[i].client = client;
			nfs_calls[i].xid = msg->rm_xid;
			nfs_calls[i].time_sec = tstamp->tv_sec;
			nfs_calls[i].time_usec = tstamp->tv_usec;
			return;
		}
	}
}

void
nfs_hash_reply(msg, client, tstamp)
struct timeval *tstamp;
struct rpc_msg *msg;
ipaddrt client;
{
	int i;
	double diff;
	u_long proc;
	u_int32 xid = ntohl(msg->rm_xid);

	if (tstamp == NULL)
		return;

	for (i=0; i < NFSCALLHASHSIZE; i++) {
		if (nfs_calls[i].used == 0)
			continue;

		if (nfs_calls[i].client == client &&
		    nfs_calls[i].xid == xid) {
			proc = prc_countmap[nfs_calls[i].proc];

			diff = ((tstamp->tv_sec - nfs_calls[i].time_sec) *
				1000000 +
				tstamp->tv_usec - nfs_calls[i].time_usec) /
				1000.0;

			prc_counters[proc].pr_complete++;
			prc_counters[proc].pr_response += diff;
			prc_counters[proc].pr_respsqr += diff * diff;

			if (diff > prc_counters[proc].pr_maxresp)
				prc_counters[proc].pr_maxresp = diff;

			nfs_calls[i].used = 0;
			return;
		}
	}
}
