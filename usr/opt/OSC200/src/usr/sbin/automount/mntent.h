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
 *      @(#)$RCSfile: mntent.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/10/11 17:32:32 $
 *      %W%  ULTRIX  %G%
 */

#define max(a,b)        ((a) > (b) ? (a) : (b))

#define MNTTAB          "/etc/fstab"
#define MOUNTED         "/etc/mtab"

#define MNTTYPE_42      "4.2"   /* 4.2 file system */
#define MNTTYPE_NFS     "nfs"   /* network file system */
#define MNTTYPE_PC      "pc"    /* IBM PC (MSDOS) file system */
#define MNTTYPE_SWAP    "swap"  /* swap file system */
#define MNTTYPE_IGNORE  "ignore"/* No type specified, ignore this entry */
#define MNTTYPE_LO      "lo"    /* Loop back File system */

#define MNTOPT_RO       "ro"            /* read only */
#define MNTOPT_RW       "rw"            /* read/write */
#define MNTOPT_GRPID    "grpid" 	/* SysV-compatible group-id on create */
#define MNTOPT_REMOUNT  "remount"	/* change options on previous mount */
#define MNTOPT_NOAUTO   "noauto"	/* hide entry from mount -a */
#define MNTOPT_NOSUB    "nosub"  	/* disallow mounts beneath this one */
#define MNTOPT_QUOTA    "quota" 	/* quotas */
#define MNTOPT_NOQUOTA  "noquota"	/* no quotas */
#define MNTOPT_SOFT     "soft"          /* soft mount */
#define MNTOPT_HARD     "hard"          /* hard mount */
#define MNTOPT_BG       "bg"            /* background mount if no answer */
#define MNTOPT_NOSUID   "nosuid"        /* no set uid allowed */
#define MNTOPT_NOEXEC   "noexec"        /* no execution allowed */
#define MNTOPT_NODEV    "nodev"         /* no devices access allowed */
#define MNTOPT_SECURE   "secure"        /* use secure RPC for NFS */
#define MNTOPT_FORCE    "force"         /* force the mount */
#define MNTOPT_SYNC     "sync"          /* synchronous writes */
#define MNTOPT_NOCACHE  "nocache"       /* don't keep in cache -- write thru */
#define MNTOPT_INT      "intr"          /* allow hard mount keybrd interrupts */
#define MNTOPT_RSIZE    "rsize"        /* read size */
#define MNTOPT_WSIZE    "wsize"        /* write size */
#define MNTOPT_RETRANS  "retrans"      /* # of NFS retries */
#define MNTOPT_RETRY    "retry"        /* # of mount retries */
#define MNTOPT_TIMEO    "timeo"        /* timeout interval */
#define MNTOPT_PORT     "port"         /* NFS port # */
#define MNTOPT_PGTHRESH "pgthresh"     /* paging threshold */
#define MNTOPT_NOAC	"noac"		/* no attribute caching */
#define MNTOPT_NOCTO	"nocto"		/* no "close to open" attr consistency*/
#define MNTOPT_ACREGMIN "acregmin"     /* min seconds for caching file attrs*/
#define MNTOPT_ACREGMAX "acregmax"     /* max seconds for caching file attrs*/
#define MNTOPT_ACDIRMIN "acdirmin"     /* min seconds for caching dir attrs*/
#define MNTOPT_ACDIRMAX "acdirmax"     /* max seconds for caching dir attrs*/
#define MNTOPT_ACTIMEO 	"actimeo"     	/* min and max times for both files
					   and directories 		*/

#define NFSAC_REGMIN    3
#define NFSAC_REGMAX    60
#define NFSAC_DIRMIN    30
#define NFSAC_DIRMAX    60
#define MAXACTIME       3600
#define SIXTYFOUR       64      /* default page threshhold */



struct  mntent{
        char    *mnt_fsname;            /* name of mounted file system */
        char    *mnt_dir;               /* file system path prefix */
	char    *mnt_type;              /* MNTTYPE_* */
        char    *mnt_opts;              /* MNTOPT* */
	int     mnt_freq;               /* dump frequency, in days */
	int     mnt_passno;             /* pass number on parallel fsck */
};

