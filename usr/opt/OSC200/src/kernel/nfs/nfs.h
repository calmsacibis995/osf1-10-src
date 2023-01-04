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
 *	@(#)$RCSfile: nfs.h,v $ $Revision: 4.3.10.4 $ (DEC) $Date: 1993/06/29 18:53:14 $
 */ 
/*
 */
#ifndef __NFS_HEADER__
#define __NFS_HEADER__

#include <sys/mount.h>		/* Need for nfsv2fh define */
#include <sys/socket.h>		/* Needed for exaddrlist defines */

/* Maximum size of data portion of a remote request */
#define	NFS_MAXDATA	8192
#define	NFS_MAXREADDIR	NFS_MAXDATA	/* Max. size of directory read */
#define	NFS_MAXNAMLEN	255
#define	NFS_MAXPATHLEN	1024
#define NFS_FABLKSIZE	512	/* CJXXX: for na_blocks */

/*
 * Rpc retransmission parameters
 */
#define	NFS_TIMEO	11	/* initial timeout in tenths of a sec. */
#define	NFS_RETRIES	4	/* times to retry request */

/* Maximum number of concurrent nfs daemons */
#define MAXNFSDS	128

/*
 * Error status
 * Should include all possible net errors.
 * For now we just cast errno into an enum nfsstat.
 */
enum nfsstat {
	NFS_OK = 0,			/* no error */
	NFSERR_PERM=EPERM,		/* Not owner */
	NFSERR_NOENT=ENOENT,		/* No such file or directory */
	NFSERR_IO=EIO,			/* I/O error */
	NFSERR_NXIO=ENXIO,		/* No such device or address */
	NFSERR_ACCES=EACCES,		/* Permission denied */
	NFSERR_EXIST=EEXIST,		/* File exists */
	NFSERR_NODEV=ENODEV,		/* No such device */
	NFSERR_NOTDIR=ENOTDIR,		/* Not a directory */
	NFSERR_ISDIR=EISDIR,		/* Is a directory */
	NFSERR_FBIG=EFBIG,		/* File too large */
	NFSERR_NOSPC=ENOSPC,		/* No space left on device */
	NFSERR_ROFS=EROFS,		/* Read-only file system */
	NFSERR_NAMETOOLONG=ENAMETOOLONG,/* File name too long */
	NFSERR_NOTEMPTY=ENOTEMPTY,	/* Directory not empty */
	NFSERR_DQUOT=EDQUOT,		/* Disc quota exceeded */
	NFSERR_STALE=ESTALE,		/* Stale NFS file handle */
	NFSERR_WFLUSH			/* write cache flushed */
};
#define	puterrno(error)		((enum nfsstat)error)
#define	geterrno(status)	((int)status)

/*
 * File types
 */
enum nfsftype {
	NFNON,
	NFREG,		/* regular file */
	NFDIR,		/* directory */
	NFBLK,		/* block special */
	NFCHR,		/* character special */
	NFLNK		/* symbolic link */
};
/*
 * Special kludge for fifos (named pipes)  [to adhere to NFS Protocol Spec]
 *
 * VFIFO is not in the protocol spec (VNON will be replaced by VFIFO)
 * so the over-the-wire representation is VCHR with a '-1' device number.
 *
 * NOTE: This kludge becomes unnecessary with the Protocol Revision,
 *       but it may be necessary to support it (backwards compatibility).
 */

/* CJXXX */
#define	S_IFMT		0170000		/* type of file */
#define S_IFCHR		0020000		/* character special */

#define NFS_FIFO_TYPE   NFCHR
#define NFS_FIFO_MODE   S_IFCHR
#define NFS_FIFO_DEV    ((unsigned) ~0)

/* identify fifo in nfs attributes */
#define NA_ISFIFO(NA)	(((NA)->na_type == NFS_FIFO_TYPE) && \
			    ((NA)->na_rdev == NFS_FIFO_DEV))

/* set fifo in nfs attributes */
#define NA_SETFIFO(NA)	{ \
			(NA)->na_type = NFS_FIFO_TYPE; \
			(NA)->na_rdev = NFS_FIFO_DEV; \
			(NA)->na_mode = ((NA)->na_mode&~S_IFMT)|NFS_FIFO_MODE; \
			}
/*
 * Arguments to remote write and writecache
 */
struct nfswriteargs {
	nfsv2fh_t	wa_fhandle;	/* handle for file */
	u_int		wa_begoff;	/* beginning byte offset in file */
	u_int		wa_offset;	/* current byte offset in file */
	u_int		wa_totcount;	/* total write cnt (to this offset) */
	u_int		wa_count;	/* size of this write */
	int		wa_dupbusy;	/* wg processing a busy write */
	char		*wa_data;	/* data to write (up to NFS_MAXDATA) */
	struct mbuf	*wa_mbuf;	/* mbuf containing data */
};

/*
 * File attributes
 */
struct nfsfattr {
	enum nfsftype	na_type;	/* file type */
	u_int		na_mode;	/* protection mode bits */
	u_int		na_nlink;	/* # hard links */
	u_int		na_uid;		/* owner user id */
	u_int		na_gid;		/* owner group id */
	u_int		na_size;	/* file size in bytes */
	u_int		na_blocksize;	/* preferred block size */
	u_int		na_rdev;	/* special device # */
	u_int		na_blocks;	/* Kb of disk used by file */
	u_int		na_fsid;	/* device # */
	u_int		na_nodeid;	/* inode # */
	struct timeval	na_atime;	/* time of last access */
	struct timeval	na_mtime;	/* time of last modification */
	struct timeval	na_ctime;	/* time of last change */
};

#define n2v_type(x)	(NA_ISFIFO(x) ? VFIFO : (enum vtype)((x)->na_type))
#define n2v_rdev(x)	(NA_ISFIFO(x) ? 0 : (x)->na_rdev)

/*
 * Arguments to remote read
 */
struct nfsreadargs {
	nfsv2fh_t	ra_fhandle;	/* handle for file */
	u_int		ra_offset;	/* byte offset in file */
	u_int		ra_count;	/* immediate read count */
	u_int		ra_totcount;	/* total read cnt (from this offset) */
};

/*
 * Status OK portion of remote read reply
 */
struct nfsrrok {
	struct nfsfattr	rrok_attr;	/* attributes, need for pagin */
	u_int		rrok_count;	/* bytes of data */
	char		*rrok_data;	/* data (up to NFS_MAXDATA bytes) */
	u_int		rrok_allocsize;
	struct buf	*rrok_bp;	/* buffer pointer for bread */
	struct vnode	*rrok_vp;	/* vnode assoc. with buffer */
};

/*
 * Reply from remote read
 */
struct nfsrdresult {
	enum nfsstat	rr_status;	    	/* status of read */
	union {
		struct nfsrrok	rr_ok_u;	/* attributes */
		                                /*  (needed for pagin) */
	} rr_u;
};
#define	rr_ok		rr_u.rr_ok_u
#define	rr_attr		rr_u.rr_ok_u.rrok_attr
#define	rr_count	rr_u.rr_ok_u.rrok_count
#define	rr_data		rr_u.rr_ok_u.rrok_data
#define	rr_allocsize	rr_u.rr_ok_u.rrok_allocsize
#define rr_bp		rr_u.rr_ok_u.rrok_bp
#define rr_vp		rr_u.rr_ok_u.rrok_vp


/*
 * File attributes which can be set
 */
struct nfssattr {
	u_int		sa_mode;	/* protection mode bits */
	u_int		sa_uid;		/* owner user id */
	u_int		sa_gid;		/* owner group id */
	u_int		sa_size;	/* file size in bytes */
	struct timeval	sa_atime;	/* time of last access */
	struct timeval	sa_mtime;	/* time of last modification */
};


/*
 * Reply status with file attributes
 */
struct nfsattrstat {
	enum nfsstat	ns_status;		/* reply status */
	union {
		struct nfsfattr ns_attr_u;	/* NFS_OK: file attributes */
	} ns_u;
};
#define	ns_attr	ns_u.ns_attr_u


/*
 * NFS_OK part of read sym link reply union
 */
struct nfssrok {
	u_int	srok_count;	/* size of string */
	char	*srok_data;	/* string (up to NFS_MAXPATHLEN bytes) */
};

/*
 * Result of reading symbolic link
 */
struct nfsrdlnres {
	enum nfsstat	rl_status;		/* status of symlink read */
	union {
		struct nfssrok	rl_srok_u;	/* name of linked to */
	} rl_u;
};
#define	rl_srok		rl_u.rl_srok_u
#define	rl_count	rl_u.rl_srok_u.srok_count
#define	rl_data		rl_u.rl_srok_u.srok_data


/*
 * Arguments to readdir
 */
struct nfsrddirargs {
	nfsv2fh_t rda_fh;	/* directory handle */
	u_int rda_offset;	/* offset in directory (opaque) */
	u_int rda_count;	/* number of directory bytes to read */
};

/*
 * NFS_OK part of readdir result
 */
struct nfsrdok {
	u_int	rdok_offset;		/* next offset (opaque) */
	u_int	rdok_size;		/* size in bytes of entries */
	bool_t	rdok_eof;		/* true if last entry is in result */
	struct dirent *rdok_entries;	/* variable number of entries */
};

/*
 * Readdir result
 */
struct nfsrddirres {
	u_int		rd_bufsize;	/* client request size (not xdr'ed) */
 	u_int		rd_origreqsize;	/* client request size */
	enum nfsstat	rd_status;
	union {
		struct nfsrdok rd_rdok_u;
	} rd_u;
};
#define	rd_rdok		rd_u.rd_rdok_u
#define	rd_offset	rd_u.rd_rdok_u.rdok_offset
#define	rd_size		rd_u.rd_rdok_u.rdok_size
#define	rd_eof		rd_u.rd_rdok_u.rdok_eof
#define	rd_entries	rd_u.rd_rdok_u.rdok_entries


/*
 * Arguments for directory operations
 */
struct nfsdiropargs {
	nfsv2fh_t	da_fhandle;	/* directory file handle */
	char		*da_name;	/* name (up to MAXNAMLEN bytes) */
	int		da_len;		/* length of name */
};

/*
 * NFS_OK part of directory operation result
 */
struct  nfsdrok {
	nfsv2fh_t	drok_fhandle;	/* result file handle */
	struct nfsfattr	drok_attr;	/* result file attributes */
};

/*
 * Results from directory operation
 */
struct  nfsdiropres {
	enum nfsstat	dr_status;	/* result status */
	union {
		struct  nfsdrok	dr_drok_u;	/* NFS_OK result */
	} dr_u;
};
#define	dr_drok		dr_u.dr_drok_u
#define	dr_fhandle	dr_u.dr_drok_u.drok_fhandle
#define	dr_attr		dr_u.dr_drok_u.drok_attr

/*
 * arguments to setattr
 */
struct nfssaargs {
	nfsv2fh_t	saa_fh;		/* fhandle of file to be set */
	struct nfssattr	saa_sa;		/* new attributes */
};

/*
 * arguments to create and mkdir
 */
struct nfscreatargs {
	struct nfsdiropargs	ca_da;	/* file name to create and parent dir */
	struct nfssattr		ca_sa;	/* initial attributes */
};

/*
 * arguments to link
 */
struct nfslinkargs {
	nfsv2fh_t		la_from;	/* old file */
	struct nfsdiropargs	la_to;		/* new file and parent dir */
};

/*
 * arguments to rename
 */
struct nfsrnmargs {
	struct nfsdiropargs rna_from;	/* old file and parent dir */
	struct nfsdiropargs rna_to;	/* new file and parent dir */
};

/*
 * arguments to symlink
 */
struct nfsslargs {
	struct nfsdiropargs	sla_from;	/* old file and parent dir */
	char			*sla_tnm;	/* new name */
	struct nfssattr		sla_sa;		/* attributes */
};

/*
 * NFS_OK part of statfs operation
 */
struct nfsstatfsok {
	u_int fsok_tsize;	/* preferred transfer size in bytes */
	u_int fsok_bsize;	/* fundamental file system block size */
	u_int fsok_blocks;	/* total blocks in file system */
	u_int fsok_bfree;	/* free blocks in fs */
	u_int fsok_bavail;	/* free blocks avail to non-superuser */
};

/*
 * Results of statfs operation
 */
struct nfsstatfs {
	enum nfsstat	fs_status;	/* result status */
	union {
		struct	nfsstatfsok fs_fsok_u;	/* NFS_OK result */
	} fs_u;
};
#define	fs_fsok		fs_u.fs_fsok_u
#define	fs_tsize	fs_u.fs_fsok_u.fsok_tsize
#define fs_bsize	fs_u.fs_fsok_u.fsok_bsize
#define	fs_blocks	fs_u.fs_fsok_u.fsok_blocks
#define	fs_bfree	fs_u.fs_fsok_u.fsok_bfree
#define	fs_bavail	fs_u.fs_fsok_u.fsok_bavail

/*
 * XDR routines for handling structures defined above
 */
bool_t xdr_attrstat();
bool_t xdr_creatargs();
bool_t xdr_diropargs();
bool_t xdr_diropres();
bool_t xdr_drok();
bool_t xdr_fattr();
bool_t xdr_fhandle();
bool_t xdr_linkargs();
bool_t xdr_rddirargs();
bool_t xdr_putrddirres();
bool_t xdr_getrddirres();
bool_t xdr_rdlnres();
bool_t xdr_rdresult();
bool_t xdr_readargs();
bool_t xdr_rnmargs();
bool_t xdr_rrok();
bool_t xdr_saargs();
bool_t xdr_sattr();
bool_t xdr_slargs();
bool_t xdr_srok();
bool_t xdr_timeval();
bool_t xdr_writeargs();
bool_t xdr_statfs();

/*
 * Remote file service routines
 */
#define	RFS_NULL	0
#define	RFS_GETATTR	1
#define	RFS_SETATTR	2
#define	RFS_ROOT	3
#define	RFS_LOOKUP	4
#define	RFS_READLINK	5
#define	RFS_READ	6
#define	RFS_WRITECACHE	7
#define	RFS_WRITE	8
#define	RFS_CREATE	9
#define	RFS_REMOVE	10
#define	RFS_RENAME	11
#define	RFS_LINK	12
#define	RFS_SYMLINK	13
#define	RFS_MKDIR	14
#define	RFS_RMDIR	15
#define	RFS_READDIR	16
#define	RFS_STATFS	17
#define	RFS_NPROC	18

/*
 * remote file service numbers
 */
#define	NFS_PROGRAM	((u_int)100003)
#define	NFS_VERSION	((u_int)2)
#define	NFS_PORT	2049

/*
 * exported vfs flags.
 */
#define EXPORTFS_CREATE	0x01		/* create a new export record */
#define EXPORTFS_REMOVE	0x02		/* remove an old export record */
#define EXPORTFS_READ	0x03		/* read an old export record */

#define EXMAXADDRS 256
struct exaddrlist {
        unsigned naddrs;                /* number of addresses */
        struct sockaddr *addrvec;       /* pointer to array of addresses */
};

struct exportfsaddrlist {
        unsigned naddrs;
        struct sockaddr addrvec[EXMAXADDRS];
};

struct exportfsdata {
	/* these match the values that can be accessed via stat() in stat.h */
	dev_t	e_dev;		/* ID of device containing a directory*/
	ino_t	e_ino;		/* File serial number */
        uint_t  e_gen;         /* file generation number */
	char    e_path[MAXPATHLEN];
	int     e_flags;
	uid_t   e_rootmap;
	uid_t   e_anon;
	struct exportfsaddrlist e_rootaddrs;
	struct exportfsaddrlist e_writeaddrs;
	int	e_more;
};

#ifdef _KERNEL

struct kexport {
	int	e_flags;
	fsid_t	e_fsid;
	fh_fid_t e_fid;
	uid_t   e_rootmap;
	uid_t	e_anon;
	int	e_pathlen;
	char	*e_path;
        struct exaddrlist e_rootaddrs;
        struct exaddrlist e_writeaddrs;
	struct kexport	*e_next;
	/* These match the values that can be accessed via stat() in stat.h.
	 * We only hang on to these so that we can return these values to 
	 * the user level programs (mountd) that need them.
	 */
	dev_t	e_dev;		/* ID of device containing a directory*/
	ino_t	e_ino;		/* File serial number */
        uint_t  e_gen;          /* file generation number */
};

#define EXPORT_SIZE	(sizeof(struct kexport))

#endif	/* _KERNEL */

#endif /* !__NFS_HEADER__ */

