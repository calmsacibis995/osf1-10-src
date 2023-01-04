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
#ifndef _SYS_VFS_PROTO_H_
#define _SYS_VFS_PROTO_H_
/* Forward declarations of enum/structure types so
   function prototypes are happy */
/* from files:
   vfs_bio.c
   vfs_cache.c
   vfs_conf.c
   vfs_flock.c
   vfs_subr.c
   vfs_vnops.c
 */
struct  buf;
struct  eflock;
struct  file;
struct  filock;
struct  flino;
struct  mount;
struct  nameidata;
struct  proc;
struct  stat;
struct  ucred;
struct  uio;
struct  utask_nd;
struct  vattr;
struct  vfsops;
struct  vnode;
struct  vnodeops;
enum  uio_seg;
enum  vnuio_rw;
enum  vtagtype;
extern int bawrite(struct buf * );
extern int bdwrite(struct buf *  , struct vnode * );
extern int bfree(struct buf * );
extern int bgetvp(struct vnode *  , struct buf * );
extern int biowait(struct buf * );
extern int bread(struct vnode *  , daddr_t  , int  , struct ucred *  , struct buf * * );
extern int breada(struct vnode *  , daddr_t  , int  , daddr_t  , int  , struct ucred *  , struct buf * * );
extern int brelse(struct buf * );
extern int brelvp(struct buf * );
extern int bwrite(struct buf * );
extern int cache_lookup(struct nameidata * );
extern int cache_purgevfs(struct mount * );
extern int chk_granted(struct flino *  , struct vnode *  , int );
extern int cleanlocks(struct vnode * );
extern int clear_vlocks(struct vnode * );
extern int deadflck(struct filock *  , pid_t  , pid_t  , u_int );
extern int delflck(struct flino *  , struct filock *  , struct vnode * );
extern int do_grant_locks(struct eflock * );
extern int flckadj(struct flino *  , struct filock *  , struct eflock *  , struct vnode *  , int  , pid_t );
extern int flckinit(void);
extern void freefid(struct flino * );
extern int getflck(struct vnode *  , struct eflock *  , off_t  , pid_t  , int );
extern int getnewvnode(enum vtagtype  , struct vnodeops *  , struct vnode * * );
extern int incore(struct vnode *  , daddr_t );
extern int klm_callbacks(void);
extern int locked(struct vnode *  , struct eflock *  , int );
extern int mntbusybuf(struct mount * );
extern int mntflushbuf(struct mount *  , int );
extern int mntinvalbuf(struct mount * );
extern int nchinit(void);
extern int nddup(struct nameidata *  , struct nameidata * );
extern int ndinit(struct nameidata *  , struct utask_nd * );
extern int ndrele(struct nameidata * );
extern int print_flino(struct flino * );
extern int print_flock(struct eflock * );
extern int quotactl(struct proc *  , void *  , long * );
extern int reassignbuf(struct buf *  , struct vnode * );
extern int regflck(struct eflock *  , struct filock * );
extern int setflck(struct vnode *  , struct eflock *  , int  , off_t  , pid_t  , int );
extern int vflush(struct mount *  , struct vnode *  , int );
extern int vfs_noop(void);
extern int vfs_nullop(void);
extern int vfsinit(void);
extern int vgetm(struct vnode *  , int );
extern int vgone(struct vnode *  , int  , struct vnodeops * );
extern int vinvalbuf(struct vnode *  , int );
extern int vn_close(struct file * );
extern int vn_fhtovp(fhandle_t *  , int  , struct vnode * * );
extern int vn_flock(struct file *  , int );
extern int vn_ioctl(struct file *  , unsigned int  , caddr_t , long *);
extern int vn_kopen(struct vnode *  , int  , int * );
extern int vn_open(struct nameidata *  , int  , int );
extern int vn_rdwr(enum vnuio_rw  , struct vnode *  , caddr_t  , int  , off_t  , enum uio_seg  , int  , struct ucred *  , int * );
extern int vn_read(struct file *  , struct uio *  , struct ucred * );
extern int vn_select(struct file *  , short *  , short *  , int );
extern int vn_stat(struct vnode *  , struct stat * );
extern int vn_write(struct file *  , struct uio *  , struct ucred * );
extern int vn_writechk(struct vnode * );
extern int vprint(char *  , struct vnode * );
extern int wait_for_vxlock(struct vnode *  , int );
extern struct buf * getblk(struct vnode *  , daddr_t  , int );
extern struct buf * geteblk(int );
extern struct buf * getnewbuf(void);
extern struct filock * blocked(struct filock *  , struct eflock *  , struct filock * *  , int  , pid_t );
extern struct filock * insflck(struct flino *  , struct eflock *  , struct filock *  , struct vnode *  , int  , pid_t );
extern struct flino * allocfid(struct vnode * );
extern struct flino * findfid(struct vnode * );
extern struct mount * getvfs(fsid_t * );
extern void allocbuf(struct buf *  , int );
extern void bio_init(void);
extern void biodone(struct buf * );
extern void bufstats(void);
extern void cache_enter(struct nameidata * );
extern void cache_purge(struct vnode * );
extern void clear_vxlock(struct vnode * );
extern void delsleeplck(pid_t );
extern void insmntque(struct vnode *  , struct mount * );
extern void kill_proc_locks(struct eflock * );
extern void vclean(struct vnode *  , long  , struct vnodeops * );
extern void vflushbuf(struct vnode *  , int );
extern void vfree(struct vnode * );
extern void vfs_mountroot(void);
extern void vn_funlock(struct file *  , int );
extern void vrele(struct vnode * );
/* Not included for the moment (type promotion issues to resolve):
   vfssw_add
   vfssw_del
   vattr_null (declared, without arguments, in <sys/vnode.h>)
*/
#endif /* _SYS_VFS_PROTO_H_ */
