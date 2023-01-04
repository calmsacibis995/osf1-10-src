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
 * @(#)$RCSfile: fs_proto.h,v $ $Revision: 1.1.9.2 $ (DEC) $Date: 1993/09/20 16:56:38 $
 */
#ifndef _UFS_FS_PROTO_H_
#define _UFS_FS_PROTO_H_
/* forward declarations of structures to avoid warnings about
   structure definitions only within prototype scope */
struct  buf;
struct  cg;
struct  dirent;
struct  disklabel;
struct  dquot;
struct  eflock;
struct  fid;
struct  fs;
struct  inode;
struct  mount;
struct  nameidata;
struct  timeval;
struct  ucred;
struct  ufs_realloc;
struct  ufsmount;
struct  uio;
struct  vattr;
struct  vnode;

extern void blkfree(struct inode * , daddr_t , off_t);
extern void ifree(struct inode *  , ino_t , int);
extern void clrblock(struct fs *  , unsigned char *  , daddr_t);
extern void setblock(struct fs *  , unsigned char *  , daddr_t);
extern void disksort(struct buf *  , struct buf * );
extern void itimes(struct inode * , struct timeval * , struct timeval * );
extern void dqrele(struct vnode *  , struct dquot * );
extern void dqput(struct vnode *  , struct dquot * );

extern char * readdisklabel(dev_t , int (* )(void) , struct disklabel * );
extern daddr_t alloccg(struct inode * , int  , daddr_t , int);
extern daddr_t alloccgblk(struct fs * , struct cg * , daddr_t);
extern daddr_t blkpref(struct inode *  , daddr_t , int , daddr_t * );
extern daddr_t fragextend(struct inode *  , int  , int  , int  , int);
extern daddr_t mapsearch(struct fs *  , struct cg *  , daddr_t , int);
extern ino_t dirpref(struct fs *);
extern ino_t ialloccg(struct inode *  , int , daddr_t , int);
extern int alloc(struct inode * , daddr_t , daddr_t , int  , daddr_t *);
extern int badblock(struct fs * , daddr_t );
extern int balloc(struct inode * , daddr_t  , u_int , struct buf * * , int);
extern int blkatoff(struct inode *  , off_t  , char * *  , struct buf * * );
extern int bmap(struct inode *  , daddr_t , daddr_t *  , int * );
extern int bootcksum(u_long *);
extern int checkdir(struct nameidata * , int );
extern int checkpath(struct inode * , struct inode * , struct ucred *);
extern int chkdq(struct inode *  , long  , struct ucred *  , int );
extern int chkdqchg(struct inode *  , long  , struct ucred *  , int );
extern int chkiq(struct inode *  , long  , struct ucred *  , int );
extern int chkiqchg(struct inode *  , long  , struct ucred *  , int );
extern int chmod1(struct inode *  , int  , struct ucred * );
extern int chown1(struct inode *  , uid_t  , gid_t  , struct ucred * );
extern int dirbad(struct inode *  , off_t  , char * );
extern int dirbadblock(struct buf * );
extern int dirbadentry(struct dirent *  , int );
extern int dirempty(struct inode *  , ino_t  , struct ucred * );
extern int direnter(struct inode *  , struct nameidata * );
extern int dirremove(struct nameidata * );
extern int dirrewrite(struct inode *  , struct inode *  , struct nameidata * );
extern int diskerr(struct buf *, char *, char *, int, int, struct disklabel *);
extern int dkcksum(struct disklabel * );
extern int dqflush(struct ufsmount *  , struct vnode * );
extern int dqget(struct vnode *  , u_int  , struct ufsmount *  , int  , struct dquot * * );
extern int dqinit(void);
extern int dqsync(struct vnode *  , struct dquot * );
extern int fake_inode_init(struct vnode *  , struct mount * );
extern int fragacct(struct fs *  , int  , int [] , int );
extern int fserr(struct fs *  , char * );
extern int getinoquota(struct inode * );
extern int getmdev(struct vnode * *  , caddr_t  , struct nameidata * );
extern int getquota(struct mount *  , u_int  , int  , caddr_t );
extern int iaccess(struct inode *  , int  , struct ucred * );
extern int ialloc(struct inode *  , ino_t  , int  , struct inode * * );
extern int idrop(struct inode * );
extern int iget(struct inode *  , ino_t  , struct inode * *  , int );
extern int indirtrunc(struct inode *  , daddr_t  , daddr_t  , int  , int * );
extern int iput(struct inode * );
extern int isblock(struct fs *  , unsigned char *  , daddr_t );
extern int itrunc(struct inode *  , u_long  , int );
extern int iupdat(struct inode *, struct timeval *, struct timeval *, int );
extern int maknode(struct vattr *  , struct nameidata *  , struct inode * * );
extern int mfs_badop(void);
extern int mfs_badoper(void);
extern int mfs_bmap(struct vnode *  , daddr_t  , struct vnode * *  , daddr_t * );
extern int mfs_bread(struct vnode *  , off_t  , struct buf * *  , struct ucred * );
extern int mfs_brelse(struct vnode *  , struct buf * );
extern int mfs_close(struct vnode *  , int  , struct ucred * );
extern int mfs_doio(struct buf *  , caddr_t );
extern int mfs_error_state(struct vnode * );
extern int mfs_inactive(struct vnode * );
extern int mfs_ioctl(struct vnode *  , int  , caddr_t  , int  , struct ucred * );
extern int mfs_lbolt(struct mount * * );
extern int mfs_mount(struct mount *  , char *  , caddr_t, struct nameidata *);
extern int mfs_nullop(void);
extern int mfs_open(struct vnode * *  , int  , struct ucred * );
extern int mfs_print(struct vnode * );
extern int mfs_start(struct mount *  , int );
extern int mfs_statfs(struct mount * );
extern int mfs_strategy(struct buf * );
extern int mfs_sync(struct mount *  , int );
extern int mfs_unmount(struct mount *  , int );
extern int mountfs(struct vnode *  , struct mount *  , struct ufsmount * );
extern int qsync(struct mount * );
extern int quota_chown(struct inode *, uid_t, gid_t, int, struct ucred * );
extern int quotaoff(struct mount *  , int );
extern int quotaon(struct nameidata *  , struct mount *  , int  , caddr_t );
extern int realloccg(struct inode *, off_t, daddr_t, int, int,
		     struct ufs_realloc * );
extern int sbupdate(struct ufsmount *  , int );
extern int scandir(struct vnode *  , struct nameidata * );
extern int setdisklabel(struct disklabel *  , struct disklabel *  , u_int );
extern int setquota(struct mount *  , u_int  , int  , caddr_t );
extern int setuse(struct mount *  , u_int  , int  , caddr_t );
extern int ufs_abortop(struct nameidata * );
extern int ufs_access(struct vnode *  , int  , struct ucred * );
extern int ufs_bmap(struct vnode *  , daddr_t  , struct vnode * *  , daddr_t * );
extern int ufs_bread(struct vnode *, off_t, struct buf * *, struct ucred * );
extern int ufs_brelse(struct vnode *  , struct buf * );
extern int ufs_close(struct vnode *  , int  , struct ucred * );
extern int ufs_create(struct nameidata *  , struct vattr * );
extern int ufs_delay_cluster(struct inode *  , off_t, int );
extern int ufs_fhtovp(struct mount *  , struct fid *  , struct vnode * * );
extern int ufs_flushdata(struct inode *  , off_t  , off_t  , int  , int );
extern int ufs_fsync(struct vnode *  , int  , struct ucred *  , int );
extern int ufs_getattr(struct vnode * , struct vattr *  , struct ucred * );
extern int ufs_inactive(struct vnode * );
extern int ufs_init(void);
extern int ufs_ioctl(struct vnode *, int, caddr_t, int, struct ucred * );
extern int ufs_link(struct vnode *, struct nameidata *);
extern int ufs_lockctl(struct vnode *, struct eflock *,
		       int, struct ucred *, pid_t, off_t );
extern int ufs_lookup(struct vnode *  , struct nameidata * );
extern int ufs_mkdir(struct nameidata *  , struct vattr * );
extern int ufs_mknod(struct nameidata *  , struct vattr *  , struct ucred * );
extern int ufs_mount(struct mount *, char *, caddr_t, struct nameidata * );
extern int ufs_mountroot(struct mount *  , struct vnode * * );
extern int ufs_noop(void);
extern int ufs_open(struct vnode * *  , int  , struct ucred * );
extern int ufs_page_read(struct vnode *  , struct uio *  , struct ucred * );
extern int ufs_print(struct vnode * );
extern int ufs_quotactl(struct mount *  , int  , uid_t  , caddr_t );
extern int ufs_rablk(struct vnode *, struct inode *, struct fs *, daddr_t );
extern int ufs_read(struct vnode *  , struct uio *  , int  , struct ucred * );
extern int ufs_readdir(struct vnode *, struct uio *, struct ucred *, int *);
extern int ufs_readlink(struct vnode *  , struct uio *  , struct ucred * );
extern int ufs_reclaim(struct vnode * );
extern int ufs_remove(struct nameidata * );
extern int ufs_rename(struct nameidata *  , struct nameidata * );
extern int ufs_rmdir(struct nameidata * );
extern int ufs_root(struct mount *  , struct vnode * * );
extern int ufs_rwip(struct vnode *  , struct uio *  , int  , struct ucred * );
extern int ufs_seek(struct vnode *  , off_t  , off_t  , struct ucred * );
extern int ufs_setattr(struct vnode *  , struct vattr *  , struct ucred * );
extern int ufs_start(struct mount *  , int );
extern int ufs_statfs(struct mount * );
extern int ufs_strategy(struct buf * );
extern int ufs_symlink(struct nameidata *  , struct vattr *  , char * );
extern int ufs_sync(struct mount *  , int );
extern int ufs_ubcbmap(struct inode *  , daddr_t  , u_int  , daddr_t *,
		       int  , boolean_t * );
extern int ufs_unmount(struct mount *  , int );
extern int ufs_vptofh(struct vnode *  , struct fid * );
extern int ufs_write(struct vnode *  , struct uio *  , int  , struct ucred * );
extern int ufsfifo_close(struct vnode *  , int  , struct ucred * );
extern int ufsfifo_getattr(struct vnode *  , struct vattr *  , struct ucred * );
extern int ufsfifo_read(struct vnode *, struct uio *, int, struct ucred *);
extern int ufsfifo_write(struct vnode *  , struct uio *  , int  , struct ucred * );
extern int ufsspec_close(struct vnode *  , int  , struct ucred * );
extern int ufsspec_read(struct vnode *, struct uio *, int, struct ucred *);
extern int ufsspec_reclaim(struct vnode * );
extern int ufsspec_write(struct vnode *, struct uio *, int, struct ucred * );
extern int xlate_dev(dev_t  , int );
extern u_int get_nextgen(void);
extern u_int minphys(struct buf * );
extern void irefresh(struct mount * );
extern void mfs_thread_start(void);

/* vm related functions not prototyped here:
   ufs_page_write
   ufs_putpage
   ufs_rwblk
   ufs_swap
   ufs_syncdata
   ufs_fsync_int
   ufs_getapage
   ufs_writepages
   ufs_ubcrealloc
   ufs_ubcrealloc_done
   ufs_getpage
   ufs_mmap
 */
/* other UFS functions not prototyped (function arguments cause minor
   problems);
   hashalloc
   writedisklabel
   physio
 */
#endif /* _UFS_FS_PROTO_H_ */
