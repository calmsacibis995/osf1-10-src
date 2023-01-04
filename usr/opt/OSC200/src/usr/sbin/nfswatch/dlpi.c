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
static char *rcsid = "@(#)$RCSfile: dlpi.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/07/21 14:47:44 $";
#endif

/* Based on:
 * "/home/harbor/davy/system/nfswatch/RCS/dlpi.c,v 4.0 1993/03/01 19:59:00 davy Exp $";
 */

#include "os.h"

#ifdef USE_DLPI
/*
 * dpli.c - routines for messing with the Data Link Provider Interface.
 *
 * The code in this module is based in large part (especially the dl*
 * routines) on the example code provided with the document "How to Use
 * DLPI", by Neal Nuckolls of Sun Internet Engineering.  Gotta give credit
 * where credit is due.  If it weren't for Neal's excellent document,
 * this module just plain wouldn't exist.
 *
 * David A. Curry
 * Purdue University
 * Engineering Computer Network
 * 1285 Electrical Engineering Building
 * West Lafayette, IN 47907-1285
 * davy@ecn.purdue.edu
 *
 * Log: dlpi.c,v
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 1.5  1993/02/19  19:54:36  davy
 * Another change in hopes of making things work on SVR4.
 *
 * Revision 1.4  1993/01/26  13:19:05  davy
 * Fixed a goof in passing buffer size.
 *
 * Revision 1.3  1993/01/26  13:18:39  davy
 * Added ifdef's to make it work on DLPI 1.3.
 *
 * Revision 1.2  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 1.1  1993/01/15  15:42:32  davy
 * Initial revision
 *
 */
#include <sys/param.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/dlpi.h>
#ifdef SUNOS5
#include <sys/bufmod.h>
#endif
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <net/if.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>

#include "nfswatch.h"
#include "externs.h"

static void	dlbindreq();
static void	dlinforeq();
static void	dlattachreq();
static void	dlpromisconreq();

/*
 * setup_dlpi_dev - set up the data link provider interface.
 */
int
setup_dlpi_dev(device)
char **device;
{
	char *p;
	u_int chunksz;
	char cbuf[BUFSIZ];
	struct ifconf ifc;
	struct ifreq *ifrp;
	struct strioctl si;
	char devname[BUFSIZ];
	int n, s, fd, devppa;
	struct timeval timeout;
	long buf[DLPI_MAXDLBUF];

	/*
	 * If the interface device was not specified,
	 * get the default one.
	 */
	if (*device == NULL) {
		/*
		 * Grab a socket.
		 */
		if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			error("socket");
			finish(-1);
		}

		ifc.ifc_buf = cbuf;
		ifc.ifc_len = sizeof(cbuf);

		/*
		 * See what devices we've got.
		 */
		if (ioctl(s, SIOCGIFCONF, (char *) &ifc) < 0) {
			error("ioctl: SIOCGIFCONF");
			finish(-1);
		}

		/*
		 * Take the first device we encounter.
		 */
		ifrp = ifc.ifc_req;
		for (n = ifc.ifc_len/sizeof(struct ifreq); n > 0; n--,ifrp++) {
			/*
			 * Skip the loopback interface.
			 */
			if (strcmp(ifrp->ifr_name, "lo0") == 0)
				continue;

			*device = savestr(ifrp->ifr_name);
			break;
		}

		(void) close(s);
	}

	/*
	 * Split the device name into type and unit number.
	 */
	if ((p = strpbrk(*device, "0123456789")) == NULL)
		return(-1);

	strcpy(devname, DLPI_DEVDIR);
	strncat(devname, *device, p - *device);
	devppa = atoi(p);

	/*
	 * Open the device.
	 */
	if ((fd = open(devname, O_RDWR)) < 0) {
		if (errno == ENOENT || errno == ENXIO)
			return(-1);

		error(devname);
		finish(-1);
	}

	/*
	 * Attach to the device.  If this fails, the device
	 * does not exist.
	 */
	dlattachreq(fd, devppa);

	if (dlokack(fd, buf) < 0) {
		close(fd);
		return(-1);
	}

	/*
	 * We want the ethernet in promiscuous mode if we're looking
	 * at nodes other than ourselves.
	 */
	if (allflag || dstflag) {
#ifdef DL_PROMISC_PHYS
		dlpromisconreq(fd, DL_PROMISC_PHYS);

		if (dlokack(fd, buf) < 0) {
			fprintf(stderr, "%s: DL_PROMISC_PHYS failed.\n", pname);
			finish(-1);
		}
#else
		fprintf(stderr, "%s: DLPI 1.3 does not support promiscuous ",
			pname);
		fprintf(stderr, "mode operation.\n");
		fprintf(stderr, "%s: cannot implement -all or -dst options.\n",
			pname);
		finish(-1);
#endif
	}

	/*
	 * Bind to the specific unit.
	 */
	dlbindreq(fd, DLPI_DEFAULTSAP, 0, DL_CLDLS, 0, 0);

	if (dlbindack(fd, buf) < 0) {
		fprintf(stderr, "%s: dlbindack failed.\n", pname);
		finish(-1);
	}

#ifdef SUNOS5
	/*
	 * We really want all types of packets.  However, the SVR4 DLPI does
	 * not let you have the packet frame header, so we won't be able to
	 * distinguish protocol types.  But SunOS5 gives you the DLIOCRAW
	 * ioctl to get the frame headers, so we can do this on SunOS5.
	 */
	dlpromisconreq(fd, DL_PROMISC_SAP);

	if (dlokack(fd, buf) < 0) {
		fprintf(stderr, "%s: DL_PROMISC_SAP failed.\n", pname);
		finish(-1);
	}

	/*
	 * We want raw packets with the packet frame header.  But we can
	 * only get this in SunOS5 with the DLIOCRAW ioctl; it's not in
	 * standard SVR4.
	 */
	si.ic_cmd = DLIOCRAW;
	si.ic_timout = -1;
	si.ic_len = 0;
	si.ic_dp = 0;

	if (ioctl(fd, I_STR, &si) < 0) {
		error("ioctl: I_STR DLIOCRAW");
		finish(-1);
	}
#endif /* SUNOS5 */

	/*
	 * Arrange to get discrete messages.
	 */
	if (ioctl(fd, I_SRDOPT, (char *) RMSGD) < 0) {
		error("ioctl: I_SRDOPT RMSGD");
		finish(-1);
	}

#ifdef SUNOS5
	/*
	 * Push and configure the streams buffering module.  This is once
	 * again SunOS-specific.
	 */
	if (ioctl(fd, I_PUSH, DLPI_BUFMOD) < 0) {
		error("ioctl: I_PUSH BUFMOD");
		finish(-1);
	}

	/*
	 * Set the read timeout.
	 */
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;

	si.ic_cmd = SBIOCSTIME;
	si.ic_timout = INFTIM;
	si.ic_len = sizeof(timeout);
	si.ic_dp = (char *) &timeout;

	if (ioctl(fd, I_STR, (char *) &si) < 0) {
		error("ioctl: I_STR SBIOCSTIME");
		finish(-1);
	}

	/*
	 * Set the chunk size.
	 */
	chunksz = DLPI_CHUNKSIZE;

	si.ic_cmd = SBIOCSCHUNK;
	si.ic_len = sizeof(chunksz);
	si.ic_dp = (char *) &chunksz;

	if (ioctl(fd, I_STR, (char *) &si) < 0) {
		error("ioctl: I_STR SBIOCSCHUNK");
		finish(-1);
	}

	/*
	 * Set snapshot mode.
	 */
	si.ic_cmd = SBIOCSSNAP;
	si.ic_len = sizeof(truncation);
	si.ic_dp = (char *) &truncation;

	if (ioctl(fd, I_STR, (char *) &si) < 0) {
		error("ioctl: I_STR SBIOCSSNAP");
		finish(-1);
	}
#endif /* SUNOS5 */

	return(fd);
}

/*
 * flush_dlpi - flush data from the dlpi.
 */
void
flush_dlpi(fd)
int fd;
{
	if (ioctl(fd, I_FLUSH, (char *) FLUSHR) < 0) {
		error("ioctl: I_FLUSH");
		finish(-1);
	}
}

/*
 * dlpi_devtype - determine the type of device we're looking at.
 */
int
dlpi_devtype(fd)
int fd;
{
	long buf[DLPI_MAXDLBUF];
	union DL_primitives *dlp;

	dlp = (union DL_primitives *) buf;

	dlinforeq(fd);

	if (dlinfoack(fd, buf) < 0)
		return(DLT_EN10MB);

	switch (dlp->info_ack.dl_mac_type) {
	case DL_CSMACD:
	case DL_ETHER:
		return(DLT_EN10MB);
#ifdef DL_FDDI
	case DL_FDDI:
		return(DLT_FDDI);
#endif
	default:
		fprintf(stderr, "%s: DLPI MACtype %d unknown, ", pname,
			dlp->info_ack.dl_mac_type);
		fprintf(stderr, "assuming ethernet.\n");
		return(DLT_EN10MB);
	}
}

/*
 * dlinforeq - request information about the data link provider.
 */
static void
dlinforeq(fd)
int fd;
{
	dl_info_req_t info_req;
	struct strbuf ctl;
	int flags;

	info_req.dl_primitive = DL_INFO_REQ;

	ctl.maxlen = 0;
	ctl.len = sizeof (info_req);
	ctl.buf = (char *) &info_req;

	flags = RS_HIPRI;

	if (putmsg(fd, &ctl, (struct strbuf *) NULL, flags) < 0) {
		error("putmsg");
		finish(-1);
	}
}

/*
 * dlattachreq - send a request to attach.
 */
static void
dlattachreq(fd, ppa)
u_long ppa;
int fd;
{
	dl_attach_req_t	attach_req;
	struct strbuf ctl;
	int flags;

	attach_req.dl_primitive = DL_ATTACH_REQ;
	attach_req.dl_ppa = ppa;

	ctl.maxlen = 0;
	ctl.len = sizeof (attach_req);
	ctl.buf = (char *) &attach_req;

	flags = 0;

	if (putmsg(fd, &ctl, (struct strbuf*) NULL, flags) < 0) {
		error("putmsg");
		finish(-1);
	}
}

#ifdef DL_PROMISCON_REQ
/*
 * dlpromisconreq - send a request to turn promiscuous mode on.
 */
static void
dlpromisconreq(fd, level)
u_long level;
int fd;
{
	dl_promiscon_req_t promiscon_req;
	struct strbuf ctl;
	int flags;

	promiscon_req.dl_primitive = DL_PROMISCON_REQ;
	promiscon_req.dl_level = level;

	ctl.maxlen = 0;
	ctl.len = sizeof (promiscon_req);
	ctl.buf = (char *) &promiscon_req;

	flags = 0;

	if (putmsg(fd, &ctl, (struct strbuf*) NULL, flags) < 0) {
		error("putmsg");
		finish(-1);
	}
}
#endif /* DL_PROMISCON_REQ */

/*
 * dlbindreq - send a request to bind.
 */
static void
dlbindreq(fd, sap, max_conind, service_mode, conn_mgmt, xidtest)
u_long sap, max_conind, service_mode, conn_mgmt, xidtest;
int fd;
{
	dl_bind_req_t bind_req;
	struct strbuf ctl;
	int flags;

	bind_req.dl_primitive = DL_BIND_REQ;
	bind_req.dl_sap = sap;
	bind_req.dl_max_conind = max_conind;
	bind_req.dl_service_mode = service_mode;
	bind_req.dl_conn_mgmt = conn_mgmt;
#ifdef DL_PROMISC_PHYS
	/*
	 * DLPI 2.0 only?
	 */
	bind_req.dl_xidtest_flg = xidtest;
#endif

	ctl.maxlen = 0;
	ctl.len = sizeof (bind_req);
	ctl.buf = (char *) &bind_req;

	flags = 0;

	if (putmsg(fd, &ctl, (struct strbuf*) NULL, flags) < 0) {
		error("putmsg");
		finish(-1);
	}
}

/*
 * dlokack - general acknowledgement reception.
 */
static int
dlokack(fd, bufp)
char *bufp;
int fd;
{
	union DL_primitives *dlp;
	struct strbuf ctl;
	int flags;

	ctl.maxlen = DLPI_MAXDLBUF;
	ctl.len = 0;
	ctl.buf = bufp;

	if (strgetmsg(fd, &ctl, (struct strbuf*)NULL, &flags, "dlokack") < 0)
		return(-1);

	dlp = (union DL_primitives *) ctl.buf;

	if (expecting(DL_OK_ACK, dlp) < 0)
		return(-1);

	if (ctl.len < sizeof (dl_ok_ack_t))
		return(-1);

	if (flags != RS_HIPRI)
		return(-1);

	if (ctl.len < sizeof (dl_ok_ack_t))
		return(-1);

	return(0);
}

/*
 * dlinfoack - receive an ack to a dlinforeq.
 */
static int
dlinfoack(fd, bufp)
char *bufp;
int fd;
{
	union DL_primitives *dlp;
	struct strbuf ctl;
	int flags;

	ctl.maxlen = DLPI_MAXDLBUF;
	ctl.len = 0;
	ctl.buf = bufp;

	if (strgetmsg(fd, &ctl, (struct strbuf *)NULL, &flags, "dlinfoack") < 0)
		return(-1);

	dlp = (union DL_primitives *) ctl.buf;

	if (expecting(DL_INFO_ACK, dlp) < 0)
		return(-1);

	if (ctl.len < sizeof (dl_info_ack_t))
		return(-1);

	if (flags != RS_HIPRI)
		return(-1);

	if (ctl.len < sizeof (dl_info_ack_t))
		return(-1);

	return(0);
}

/*
 * dlbindack - receive an ack to a dlbindreq.
 */
static int
dlbindack(fd, bufp)
char *bufp;
int fd;
{
	union DL_primitives *dlp;
	struct strbuf ctl;
	int flags;

	ctl.maxlen = DLPI_MAXDLBUF;
	ctl.len = 0;
	ctl.buf = bufp;

	if (strgetmsg(fd, &ctl, (struct strbuf*)NULL, &flags, "dlbindack") < 0)
		return(-1);

	dlp = (union DL_primitives *) ctl.buf;

	if (expecting(DL_BIND_ACK, dlp) < 0)
		return(-1);

	if (flags != RS_HIPRI)
		return(-1);

	if (ctl.len < sizeof (dl_bind_ack_t))
		return(-1);

	return(0);
}

/*
 * expecting - see if we got what we wanted.
 */
static int
expecting(prim, dlp)
union DL_primitives *dlp;
int prim;
{
	if (dlp->dl_primitive != (u_long)prim)
		return(-1);

	return(0);
}

/*
 * strgetmsg - get a message from a stream, with timeout.
 */
static int
strgetmsg(fd, ctlp, datap, flagsp, caller)
struct strbuf *ctlp, *datap;
char *caller;
int *flagsp;
int fd;
{
	int rc;
	void sigalrm();

	/*
	 * Start timer.
	 */
	(void) sigset(SIGALRM, sigalrm);

	if (alarm(DLPI_MAXWAIT) < 0) {
		error("alarm");
		finish(-1);
	}

	/*
	 * Set flags argument and issue getmsg().
	 */
	*flagsp = 0;
	if ((rc = getmsg(fd, ctlp, datap, flagsp)) < 0) {
		error("getmsg");
		finish(-1);
	}

	/*
	 * Stop timer.
	 */
	if (alarm(0) < 0) {
		error("alarm");
		finish(-1);
	}

	/*
	 * Check for MOREDATA and/or MORECTL.
	 */
	if ((rc & (MORECTL | MOREDATA)) == (MORECTL | MOREDATA))
		return(-1);
	if (rc & MORECTL)
		return(-1);
	if (rc & MOREDATA)
		return(-1);

	/*
	 * Check for at least sizeof (long) control data portion.
	 */
	if (ctlp->len < sizeof (long))
		return(-1);

	return(0);
}

/*
 * sigalrm - handle alarms.
 */
static void
sigalrm()
{
	(void) fprintf(stderr, "dlpi: timeout\n");
}
#endif /* USE_DLPI */
