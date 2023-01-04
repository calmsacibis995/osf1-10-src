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
static char *rcsid = "@(#)$RCSfile: rpc.rquotad.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/08/30 21:25:46 $";
#endif

/*
 * Copyright (c) 1985 by Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <fstab.h>
#include <syslog.h>
#include "mntent.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/file.h>
#include <ufs/quota.h>
#include <rpc/rpc.h>
#include <rpcsvc/rquota.h>

#define QFNAME	"quotas"

int rquota_service();
void log_cant_reply();

struct fsquot {
	struct fsquot *fsq_next;
	char *fsq_dir;
	char *fsq_devname;
	dev_t fsq_dev;
};

struct fsquot *fsqlist = NULL;

typedef struct authunix_parms *authp;

extern int errno;

/*ARGSUSED*/
main(argc, argv)
	int argc;
	char **argv;
{
	register SVCXPRT *transp;
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);

	openlog("rquotad", LOG_PID, LOG_DAEMON);

#ifdef DEBUG
	{
		int s;
		struct sockaddr_in addr;
		int len = sizeof(struct sockaddr_in);

		if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			syslog(LOG_ERR, "socket: %m");
			return -1;
		}
		if (bind(s, &addr, sizeof(addr)) < 0) {
			syslog(LOG_ERR, "bind: %m");
			return -1;
		}
		if (getsockname(s, &addr, &len) != 0) {
			syslog(LOG_ERR, "getsockname: %m");
			(void)close(s);
			return -1;
		}
		pmap_unset(RQUOTAPROG, RQUOTAVERS);
		pmap_set(RQUOTAPROG, RQUOTAVERS, IPPROTO_UDP,
		    ntohs(addr.sin_port));
		if (dup2(s, 0) < 0) {
			syslog(LOG_ERR, "dup2: %m");
			exit(1);
		}
	}
#endif	

#define USE_INETD

#ifdef USE_INETD
	if (getsockname(0, &addr, &len) != 0) {
		syslog(LOG_ERR, "getsockname: %m");
		exit(1);
	}
	if ((transp = svcudp_create(0)) == NULL) {
#else
        (void) pmap_unset(RQUOTAPROG, RQUOTAVERS);
	if ((transp = svcudp_create(RPC_ANYSOCK)) == NULL) {
#endif
		syslog(LOG_ERR, "couldn't create UDP transport");
		exit(1);
	}
#ifdef USE_INETD
	if (!svc_register(transp, RQUOTAPROG, RQUOTAVERS, rquota_service, 0)) {
#else
	if (!svc_register(transp, RQUOTAPROG, RQUOTAVERS, rquota_service, IPPROTO_UDP)) {
#endif 		syslog(LOG_ERR, "couldn't register RQUOTAPROG");
		exit(1);
	}
	svc_run();		/* never returns */
	syslog(LOG_ERR, "Error: svc_run shouldn't have returned");
	exit(1);
	/* NOTREACHED */
}

rquota_service(rqstp, transp)
	register struct svc_req *rqstp;
	register SVCXPRT *transp;
{

	switch (rqstp->rq_proc) {
	case NULLPROC:
		errno = 0;
		if (!svc_sendreply(transp, xdr_void, 0))
			log_cant_reply(transp);
		return;

	case RQUOTAPROC_GETQUOTA:
	case RQUOTAPROC_GETACTIVEQUOTA:
		getquota(rqstp, transp);
		return;

	default: 
		svcerr_noproc(transp);
		return;
	}
}

getquota(rqstp, transp)
	register struct svc_req *rqstp;
	register SVCXPRT *transp;
{
	struct getquota_args gqa;
	struct getquota_rslt gqr;
	struct dqblk dqblk;
	struct fsquot *fsqp;
	struct timeval tv;
	bool_t qactive;

	gqa.gqa_pathp = NULL;		/* let xdr allocate the storage */
	if (!svc_getargs(transp, xdr_getquota_args, &gqa)) {
		svcerr_decode(transp);
		return;
	}
	/*
	 * This authentication is really bogus with the current rpc
	 * authentication scheme. One day we will have something for real.
	 */
	
	if ( rqstp->rq_cred.oa_flavor != AUTH_UNIX ||
	    ( ((authp)rqstp->rq_clntcred)->aup_uid != 0 &&
	      ((authp)rqstp->rq_clntcred)->aup_uid != gqa.gqa_uid) ) {
		gqr.gqr_status = Q_EPERM;
		goto sendreply;
	} 
	/*
	 * authorization code ends here
         */


		if ( !getdiskquota(gqa.gqa_pathp, gqa.gqa_uid, &dqblk) ) { 
			gqr.gqr_status = Q_NOQUOTA; 
			goto sendreply;
		}

		qactive = TRUE;

	/*
	 * We send the remaining time instead of the absolute time
	 * because clock skew between machines should be much greater
	 * than rpc delay.
	 */

	gettimeofday(&tv, NULL);
	gqr.gqr_status = Q_OK;
	gqr.gqr_rquota.rq_active = qactive;
	gqr.gqr_rquota.rq_bsize = DEV_BSIZE;
	gqr.gqr_rquota.rq_bhardlimit = dqblk.dqb_bhardlimit;
	gqr.gqr_rquota.rq_bsoftlimit = dqblk.dqb_bsoftlimit;
	gqr.gqr_rquota.rq_curblocks = dqblk.dqb_curblocks;
	gqr.gqr_rquota.rq_fhardlimit = dqblk.dqb_ihardlimit;
	gqr.gqr_rquota.rq_fsoftlimit = dqblk.dqb_isoftlimit;
	gqr.gqr_rquota.rq_curfiles = dqblk.dqb_curinodes;
	gqr.gqr_rquota.rq_btimeleft = dqblk.dqb_btime - tv.tv_sec;
	gqr.gqr_rquota.rq_ftimeleft = dqblk.dqb_itime - tv.tv_sec; 

sendreply:
	errno = 0;
	if (!svc_sendreply(transp, xdr_getquota_rslt, &gqr))
		log_cant_reply(transp);
}


int
getdiskquota(path, uid, dqp)
        char *path;
	int uid;
	struct dqblk *dqp;
{


        register struct fstab *fs;
	int quotatype;
	char *qfpathname;
	int qcmd, fd, len;
	struct statfs buffer;

	/*
	 * to get the mount point of the file system
	 */
	if (statfs(path, &buffer))
	  return(0);
	
	setfsent();
	while (fs = getfsent()) {
	  if (!strcmp(fs->fs_file,  buffer.f_mntonname)){ 
	    /*  matches requested fs */
	    break;
	  }
	}
	endfsent();

	if (!fs)
	  return(0);
	 	  
	if (strncmp(fs->fs_vfstype, "ufs"))
	  return(0);
	  
	 /*
	  * rquota's xdr and SUN's quota does not support the 
          * type of quota that is requested. Logic first assumes
	  * it is user quota. If that fails, it tries group quota.
	  * This works for most part.
	  */
	quotatype = USRQUOTA;
	qcmd = QCMD(Q_GETQUOTA, quotatype);
	if (!hasquota(fs, quotatype, &qfpathname)){
	  quotatype = GRPQUOTA;
	  qcmd = QCMD(Q_GETQUOTA, quotatype);
	  if (!hasquota(fs, quotatype, &qfpathname))
	    return(0);
	}


	if (quotactl(fs->fs_file, qcmd, uid, dqp) != 0) {
	  if ((fd = open(qfpathname, O_RDONLY)) < 0) {
	    perror(qfpathname);
	    return(0);
	  }
	  lseek(fd, ((long)uid * sizeof(struct dqblk)), L_SET);
	  switch (read(fd, dqp, sizeof(struct dqblk))) {

	  case 0:			/* EOF */
	    /*
	     * Convert implicit 0 quota (EOF)
	     * into an explicit one (zero'ed dqblk)
	     */
	    bzero((caddr_t)dqp,
		  sizeof(struct dqblk));
	    break;
	    
	  case sizeof(struct dqblk):	/* OK */
	    break;
	    
	  default:		/* ERROR */
	    fprintf(stderr,  "quota: read error");
	    perror(qfpathname);
	    close(fd);
	    return(0);
	  }
	  close(fd);
	}
	  
	if (dqp->dqb_bhardlimit == 0 && dqp->dqb_bsoftlimit == 0 &&
	    dqp->dqb_ihardlimit == 0 && dqp->dqb_isoftlimit == 0) {
	  return (0);
	}
	return (1);
}


/*
 * Check to see if a particular quota is to be enabled.
 */
hasquota(fs, type, qfnamep)
	register struct fstab *fs;
	int type;
	char **qfnamep;
{
	register char *opt;
	char *cp, *index(), *strtok();
	static char initname, usrname[100], grpname[100];
	static char buf[BUFSIZ];

	if (!initname) {
		sprintf(usrname, "%s%s", qfextension[USRQUOTA], qfname);
		sprintf(grpname, "%s%s", qfextension[GRPQUOTA], qfname);
		initname = 1;
	}
	strcpy(buf, fs->fs_mntops);
	for (opt = strtok(buf, ","); opt; opt = strtok(NULL, ",")) {
		if (cp = index(opt, '='))
			*cp++ = '\0';
		if (type == USRQUOTA && strcmp(opt, usrname) == 0)
			break;
		if (type == GRPQUOTA && strcmp(opt, grpname) == 0)
			break;
	}
	if (!opt)
		return (0);
	if (cp) {
		*qfnamep = cp;
		return (1);
	}
	(void) sprintf(buf, "%s/%s.%s", fs->fs_file, qfname, qfextension[type]);
	*qfnamep = buf;
	return (1);
}

void
log_cant_reply(transp)
	SVCXPRT *transp;
{
	int saverrno;
	struct sockaddr_in actual;
	register struct hostent *hp;
	register char *name;

	saverrno = errno;	/* save error code */
	actual = *svc_getcaller(transp);
	/*
	 * Don't use the unix credentials to get the machine name, instead use
	 * the source IP address. 
	 */
	if ((hp = gethostbyaddr(&actual.sin_addr, sizeof(actual.sin_addr),
	   AF_INET)) != NULL)
		name = hp->h_name;
	else
		name = inet_ntoa(&actual.sin_addr);

	errno = saverrno;
	if (errno == 0)
		syslog(LOG_ERR, "couldn't send reply to %s", name);
	else
		syslog(LOG_ERR, "couldn't send reply to %s: %m", name);
}



