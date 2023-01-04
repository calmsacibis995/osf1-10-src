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
 *	@(#)$RCSfile: cdfs.h,v $ $Revision: 4.3.9.3 $ (DEC) $Date: 1993/09/07 16:42:30 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *

 */

#ifndef	_CDFS_FS_H_
#define _CDFS_FS_H_

#ifdef	_KERNEL
#include <sys/unix_defs.h>
#endif

#include <sys/secdefines.h>

#include <sys/types.h>
#include <sys/param.h>

#include <sys/cdrom.h>
#ifdef	_KERNEL
#include <kern/zalloc.h>
#endif

/*
 * The first boot and primary volume descriptor blocks are given in 
 * absolute disk addresses.
 */

#define BBSIZE		32768
#define PVD_SIZE	2048
#define ISO_SECSIZE	2048
#define	BBLOCK		((daddr_t)(0))
#define	PVD_BLOCK	((daddr_t)(BBLOCK + BBSIZE / DEV_BSIZE))
#define ISO_MAXNAMLEN	34
#define ISO_DTLEN 17
#define	ISO_MAXDIRENTLEN	256	/* limited by 8-bit size */
#define CDFS_IOBLOCKSIZE	8192

/*
 * ISO 9660 and HSG on-disk structures.
 */

struct iso_dir
{
	unsigned char dir_len;
	unsigned char dir_xar;
	unsigned char dir_extent_lsb[4];
	unsigned char dir_extent_msb[4];
	unsigned char dir_dat_len_lsb[4];
	unsigned char dir_dat_len_msb[4];
	unsigned char dir_dt[7];
	unsigned char dir_file_flags;
#define ISO_FLG_EXIST	0x01
#define ISO_FLG_DIR	0x02
#define ISO_FLG_ASSOC	0x04
#define ISO_FLG_RECFMT	0x08
#define ISO_FLG_PROTECT	0x10
#define ISO_FLG_RESRV1	0x20
#define ISO_FLG_RESRV2	0x40
#define ISO_FLG_NOTLAST	0x80
	unsigned char dir_file_unit_size;
	unsigned char dir_inger_gap_size;
	unsigned short dir_vol_seq_no_lsb;
	unsigned short dir_vol_seq_no_msb;
	unsigned char dir_namelen;
	unsigned char dir_name[1];	/* Up to ISO_MAXNAMLEN */
};

struct hsg_dir
{
	unsigned char dir_len;
	unsigned char dir_xar;
	unsigned char dir_extent_lsb[4];
	unsigned char dir_extent_msb[4];
	unsigned char dir_dat_len_lsb[4];
	unsigned char dir_dat_len_msb[4];
	unsigned char dir_dt[6];
	unsigned char dir_file_flags;
	unsigned char filler;
	unsigned char dir_file_unit_size;
	unsigned char dir_inger_gap_size;
	unsigned short dir_vol_seq_no_lsb;
	unsigned short dir_vol_seq_no_msb;
	unsigned char dir_namelen;
	unsigned char dir_name[1];	/* Up to ISO_MAXNAMLEN */
};


struct iso_fs
{
        unsigned char   iso_vol_desc_type;
#define PRIMARY_VOL_DESC 1
#define SUPPLEMENTARY_VOL_DESC 2
#define TERMINATING_VOL_DESC 255
        char            iso_std_id[5];
        unsigned char   iso_vol_desc_vers;
        char            not_used_1;
        char            iso_system_id[32];
        char            iso_vol_id[32];
        char            not_used_2[8];
        unsigned int    iso_vol_space_size_lsb;
        unsigned int    iso_vol_space_size_msb;
        char            not_used_3[32];
        unsigned short  iso_vol_set_size_lsb;
        unsigned short  iso_vol_set_size_msb;
        unsigned short  iso_vol_seq_num_lsb;
        unsigned short  iso_vol_seq_num_msb;
        unsigned short  iso_logical_block_size_lsb;
	unsigned short  iso_logical_block_size_msb;
        unsigned int    iso_path_tbl_size_lsb;
        unsigned int    iso_path_tbl_size_msb;
        unsigned int    iso_L_path_tbl;
        unsigned int    iso_opt_L_path_tbl;
        unsigned int    iso_M_path_tbl;
        unsigned int    iso_opt_M_path_tbl;
        struct iso_dir  iso_root_dir;
        char            iso_vol_set_id[128];
        char            iso_pub_id[128];
	char		iso_preparer_id[128];
        char            iso_application_id[128];
	char		iso_copyright_id[37];
	char		iso_abstract_id[37];
	char		iso_bibliographic_id[37];
	char		iso_vol_dtcre[ISO_DTLEN];
	char		iso_vol_dtmod[ISO_DTLEN];
	char		iso_vol_dtexp[ISO_DTLEN];
	char		iso_vol_dteff[ISO_DTLEN];
	char		iso_file_struct_version;
	char		not_used_4;
	char		iso_application_use[512];
	char		not_used_5[653];
}; /* 2048 bytes long */

struct hsg_fs
{
	unsigned int	iso_vol_desc_lbn_lsb;
	unsigned int	iso_vol_desc_lbn_msb;
        unsigned char   iso_vol_desc_type;
        char            iso_std_id[5];
        unsigned char   iso_vol_desc_vers;
        char            not_used_1;
        char            iso_system_id[32];
        char            iso_vol_id[32];
        char            not_used_2[8];
        unsigned int    iso_vol_space_size_lsb;
        unsigned int    iso_vol_space_size_msb;
        char            not_used_3[32];
        unsigned short  iso_vol_set_size_lsb;
        unsigned short  iso_vol_set_size_msb;
        unsigned short  iso_vol_seq_num_lsb;
        unsigned short  iso_vol_seq_num_msb;
        unsigned short  iso_logical_block_size_lsb;
	unsigned short  iso_logical_block_size_msb;
        unsigned int    iso_path_tbl_size_lsb;
        unsigned int    iso_path_tbl_size_msb;
        unsigned int    iso_L_path_tbl[2];
        unsigned int    iso_opt_L_path_tbl[2];
        unsigned int    iso_M_path_tbl[2];
        unsigned int    iso_opt_M_path_tbl[2];
        struct hsg_dir  iso_root_dir;
        char            iso_vol_set_id[128];
        char            iso_pub_id[128];
	char		iso_preparer_id[128];
        char            iso_application_id[128];
	char		iso_copyright_id[32];
	char		iso_abstract_id[32];
	char		iso_vol_dtcre[ISO_DTLEN-1];
	char		iso_vol_dtmod[ISO_DTLEN-1];
	char		iso_vol_dtexp[ISO_DTLEN-1];
	char		iso_vol_dteff[ISO_DTLEN-1];
	char		iso_file_struct_version;
	char		not_used_4;
	char		iso_application_use[512];
	char		not_used_5[680];
}; /* 2048 bytes long */

struct  iso_xar
{
	union {
		unsigned int xar_filler;
		struct iso_xar_oid_long {
			unsigned short iso_xar_oid_lsb;
			unsigned short iso_xar_oid_msb;
		} oid_un;
	} xar_oid;
#define iso_xar_oid xar_oid.oid_un.iso_xar_oid_lsb
	unsigned short iso_xar_gid_lsb;
	unsigned short iso_xar_gid_msb;
	unsigned short iso_xar_perm;	/* MSbyte first */
#define	ISO_NOT_OWN_READ 0x0010
#define ISO_NOT_OWN_EXEC 0x0040
#define ISO_NOT_GRP_READ 0x0100
#define ISO_NOT_GRP_EXEC 0x0400
#define ISO_NOT_OTH_READ 0x1000
#define ISO_NOT_OTH_EXEC 0x4000
	unsigned char iso_xar_dtcre[ISO_DTLEN];
	unsigned char iso_xar_dtmod[ISO_DTLEN];
	unsigned char iso_xar_dtexp[ISO_DTLEN];
	unsigned char iso_xar_dteff[ISO_DTLEN];
	unsigned char iso_xar_recfmt;
#define ISO_RFNULL 0
#define ISO_RFFIX 1
#define ISO_RFLVAR 2
#define ISO_RFMVAR 3
	unsigned char iso_xar_recatt;
#define ISO_RALFCR 0
#define ISO_RAISO 1
#define ISO_RAINREC 2
	unsigned short iso_xar_reclen_lsb;
	unsigned short iso_xar_reclen_msb;
	char iso_xar_sysid[32];
	char iso_xar_sysuse[64];
	unsigned char iso_xar_version;
	unsigned char iso_xar_esclen;
	char not_used_1[64];
	unsigned short iso_xar_aulen_lsb;
	unsigned short iso_xar_aulen_msb;
	char iso_xar_application_use[1]; /* variable length */
	/* real total length is 250 + aulen + esclen */
};
#define ISO_XARSIZE(xarp) (sizeof(*(xarp))-1 + \
			   (xarp)->iso_xar_aulen_lsb + (xarp)->iso_xar_esclen)

struct hsg_xar
{
	union {
		unsigned int xar_filler;
		struct iso_xar_oid_long oid_un;
	} xar_oid;
	unsigned short iso_xar_gid_lsb;
	unsigned short iso_xar_gid_msb;
	unsigned short iso_xar_perm;
	unsigned char iso_xar_dtcre[ISO_DTLEN-1];
	unsigned char iso_xar_dtmod[ISO_DTLEN-1];
	unsigned char iso_xar_dtexp[ISO_DTLEN-1];
	unsigned char iso_xar_dteff[ISO_DTLEN-1];
	unsigned char iso_xar_recfmt;
	unsigned char iso_xar_recatt;
	unsigned short iso_xar_reclen_lsb;
	unsigned short iso_xar_reclen_msb;
	char iso_xar_sysid[32];
	char iso_xar_sysuse[64];
	unsigned char iso_xar_version;
	char iso_xar_not_used_1[65];
	unsigned short iso_xar_pdir_num_lsb;
	unsigned short iso_xar_pdir_num_msb;
	unsigned short iso_xar_aulen_lsb;
	unsigned short iso_xar_aulen_msb;
	struct hsg_dir iso_xar_dirrec;	 /* variable length */
	char iso_xar_application_use[1]; /* variable length */
};

struct rrip_devmap {
    dev_t newdev;
    ino_t ino;
    char *path;
    int pathlen;
};

/*
 * Contents of mount point buf.
 */

struct	fs
{
	struct	fs *fs_link;		/* linked list of file systems */
	struct	fs *fs_rlink;		/* used for incore super blocks */
	char	fs_ronly;		/* Read only ? */
	char	fs_format;		/* ISO9660 or HSG */
#define ISO_9660 0
#define ISO_HSG  1
#define	ISO_RRIP 2
	union {
		struct iso_fs isofs;	/* pointer to primary volume desc */
		struct hsg_fs hsgfs;
        } fs_block;
	int rrip_susp_offset;		/* susp offset for each SUA */
	int rrip_numdevs;		/* how many in the table */
	struct rrip_devmap *rrip_devmap;	/* map table */
	int iso_rootino;		/* unique root number */
	int fs_ibsize;			/* File system block size */
	udecl_simple_lock_data(,fsu_lock) /* see notes below */
};

/*
 * CDFS macros
 */

#define ISOFS_LBS(fs) \
	(((fs->fs_format == ISO_9660 || fs->fs_format == ISO_RRIP) ?  \
	  fs->fs_block.isofs.iso_logical_block_size_lsb : \
	  fs->fs_block.hsgfs.iso_logical_block_size_lsb))

#define ISOFS_SETSIZE(fs) \
	(((fs->fs_format == ISO_9660 || fs->fs_format == ISO_RRIP) ?  \
	  fs->fs_block.isofs.iso_vol_set_size_lsb : \
	  fs->fs_block.hsgfs.iso_vol_set_size_lsb))

#define ISOFS_VOLSEQNUM(fs) \
	(((fs->fs_format == ISO_9660 || fs->fs_format == ISO_RRIP) ?  \
	  fs->fs_block.isofs.iso_vol_seq_num_lsb : \
	  fs->fs_block.hsgfs.iso_vol_seq_num_lsb))

#define MAXBUFHEADERS (MAXBSIZE / ISO_SECSIZE)
struct iso_strat
{
	struct iso_strat *strat_forw, *strat_back;
	int strat_numbufhdr;
	int strat_outstanding;
	struct buf *strat_bufhdr[MAXBUFHEADERS];
	caddr_t strat_save_baddr[MAXBUFHEADERS];
	struct buf *strat_bp;
};

extern enum vtype	cdftovt_tab[];
#define CDFTOVT(mode)	(cdftovt_tab[((mode) & CDFMT) >> 12])

#define CDFS_COPYINT(src,dest) {dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2]; dest[3] = src[3];}
#define CDFS_COPYSHORT(src,dest) {dest[0] = src[0]; dest[1] = src[1];}


#ifdef _KERNEL

#ifdef CDFSDEBUG
#define CDFS_DEB_GENERAL	0x01
#define CDFS_DEB_RRIP		0x02
#define CDFS_DEB_GETNUM		0x04
#define CDFS_DEB_RRIP_DIR	0x08
#define CDFS_DEB_SUSP		0x10
#define CDFS_DEB_MOUNT		0x20
#define CDFS_DEB_READDIR	0x40
#define CDFS_DEB_NAMEMATCH	0x80
#define CDFS_DEB_RRIP_SL	0x100
#define CDFS_DEB_RRIP_WEIRD	0x10000
#define ISODEBUG (cdfs_isodebug & CDFS_DEB_GENERAL)
#define RRIPDEBUG (cdfs_isodebug & CDFS_DEB_RRIP)
#define GETNUMDEBUG (cdfs_isodebug & CDFS_DEB_GETNUM)
#define RRIPDIRDEBUG (cdfs_isodebug & CDFS_DEB_RRIP_DIR)
#define SUSPDEBUG (cdfs_isodebug & CDFS_DEB_SUSP)
#define MNTDEBUG (cdfs_isodebug  & CDFS_DEB_MOUNT)
#define READDIRDEBUG (cdfs_isodebug  & CDFS_DEB_READDIR)
#define MATCHDEBUG (cdfs_isodebug  & CDFS_DEB_NAMEMATCH)
#define RRIPSLDEBUG (cdfs_isodebug & CDFS_DEB_RRIP_SL)
#define WEIRDDEBUG (cdfs_isodebug & CDFS_DEB_RRIP_WEIRD)

extern int cdfs_isodebug;
#define CDDEBUG1(test, action) if (test) action
#else
#define CDDEBUG1(test, action) /**/
#endif /* CDFSDEBUG */
#define FS(gp) ((gp)->g_mp->m_bufp->b_un.b_fs)

#define	fs_lock		fsu_lock

#define FS_LOCK(fs)		usimple_lock(&(fs)->fs_lock)
#define	FS_UNLOCK(fs)		usimple_unlock(&(fs)->fs_lock)
#define	FS_LOCK_INIT(fs)	usimple_lock_init(&(fs)->fs_lock)
#define	FS_LOCK_HOLDER(fs)	SLOCK_HOLDER(&(fs)->fs_lock)

#ifdef MACH
extern zone_t	cdfspvd_zone;
extern zone_t	cdfsreaddir_zone;
extern zone_t	cdfsmount_zone;
/* slightly cheat and use "unsigned char *" instead of caddr_t */
#define PN_ALLOCATE(buf)	ZALLOC(pathname_zone, (buf), unsigned char *)
#define	PN_DEALLOCATE(buf)	ZFREE(pathname_zone, (buf))
extern zone_t pathname_zone;
#else
#define PN_ALLOCATE(buf)	MALLOC((buf), unsigned char *, MAXPATHLEN, M_NAMEI, M_WAITOK)
#define	PN_DEALLOCATE(buf) 	FREE((buf), M_NAMEI);
#endif

struct	fs *getfs();


struct  cdnode;
extern int cdfs_setuptransfer(register struct cdnode *  , int  , int *  , int *  , int *  , int );
struct  vnode;
struct  nameidata;
struct  cdfsmount;
extern int cdfs_lookup(struct vnode *  , register struct nameidata * );
extern int cdfs_getnumber(struct vnode *  , struct nameidata *  , ino_t * );
		/* Prototype include a typedef name.
		   It should be moved after the typedef declaration */
extern int cdfs_namematch(struct cdfsmount *  , char *  , int  , struct nameidata *  , int );
extern char cdfs_toupper(int );
struct  mount;
struct  vnode;
struct  timeval;
struct  ucred;
struct  cdnode;
extern int cdfs_zone_init(void);
extern int cdfs_init(void);
extern int cdnodeget(struct mount *  , ino_t  , struct cdnode * * );
		/* Prototype include a typedef name.
		   It should be moved after the typedef declaration */
extern int cdnodeput(register struct cdnode * );
extern int cdnodedrop(register struct cdnode * );
extern int cdfs_inactive(struct vnode * );
extern int cdfs_reclaim(register struct vnode * );
extern int cdnodeupdat(register struct cdnode *  , struct timeval *  , struct timeval *  , int );
extern int cdnodetimes(struct cdnode *  , struct timeval *  , struct timeval * );
extern int cdnodeaccess(register struct cdnode *  , register int  , struct ucred * );
struct  hsg_dir;
struct  iso_dir;
struct  iso_idir;
struct  iso_xar;
struct  cdfsmount;
struct  iso_pt;
struct  cdnode;
struct  buf;
struct  vnode;
struct  fs;
extern int cdfs_isodir_to_idir(struct fs *  , struct iso_dir *  , struct hsg_dir *  , struct iso_idir * );
extern int cdfs_readisodir(register struct cdnode *  , struct iso_dir * );
extern int cdfs_readisodir_int(struct fs *  , ino_t  , struct vnode *  , dev_t  , struct iso_idir *  , struct iso_dir * );
		/* Prototype include a typedef name.
		   It should be moved after the typedef declaration */
extern int cdfs_readxar(struct cdnode *  , struct cdfsmount *  , struct uio *);
#if 0
extern int cdfs_tounixdate(time_t *  , char  , char  , char  , char  , char  , char  , char );
		/* Prototype include a typedef name.
		   It should be moved after the typedef declaration */
time_t
rrip_convert_tf_ts(struct rrip_tf *, signed char *);
time_t
cdfs_longdate(signed char *);
#endif
extern int cdfs_readpt(struct cdnode *  , struct iso_pt * );
extern struct cd_suf_header * susp_search_dir(struct fs *  , struct vnode *  , struct iso_dir *  , int  , int, const char *  , struct buf * *, struct cdnode * );
extern struct cd_suf_header * susp_search(struct fs *  , struct vnode *  , unsigned char ** , int * , int, const char *  , struct buf * *, struct cdnode *, struct cd_suf_ce *);
extern struct cd_suf_header * susp_search_cached(struct fs *  , struct vnode *  , unsigned char ** , int * , int, const char *  , struct buf * *, struct cdnode *, struct cd_suf_ce *);
extern void susp_compute_diroffs(struct iso_dir *, int, int *, unsigned char **);
unsigned char * rrip_compose_altname(struct fs *, struct iso_dir *, struct buf **, struct vnode *, int *, int *);
unsigned char *rrip_pullup_symlink(struct fs *, struct mount *, struct cdnode *, struct iso_dir *);
struct  nameidata;
struct  fid;
struct  cdfsmount;
struct  cdfs_args;
struct  mount;
struct  vnode;
extern int cdfs_mount(register struct mount *  , char *  , caddr_t  , struct nameidata * );
		/* Prototype include a typedef name.
		   It should be moved after the typedef declaration */
extern int cdfs_start(struct mount *  , int );
extern int cdfs_unmount(struct mount *  , int );
extern int cdfs_root(struct mount *  , struct vnode * * );
extern int cdfs_quotactl(struct mount *  , int  , uid_t  , caddr_t );
		/* Prototype include a typedef name.
		   It should be moved after the typedef declaration */
extern int cdfs_statfs(struct mount * );
extern int cdfs_sync(struct mount *  , int );
extern int cdfs_fhtovp(register struct mount *  , struct fid *  , struct vnode * * );
extern int cdfs_vptofh(struct vnode *  , struct fid * );
extern int mountcdfs(struct vnode *  , struct mount *  , struct cdfs_args *  , struct cdfsmount * );
struct  vattr;
struct  nameidata;
struct  uio;
struct  buf;
struct  ucred;
struct  iso9660_pvd;
struct  cdfsmount;
struct  iso9660_xar;
struct  iso9660_drec;
struct  vnode;
extern int cdfs_create(struct nameidata *  , struct vattr * );
extern int cdfs_mknod(struct nameidata *  , register struct vattr *  , struct ucred * );
extern int cdfs_open(struct vnode * *  , int  , struct ucred * );
extern int cdfs_close(struct vnode *  , int  , struct ucred * );
extern int cdfs_access(struct vnode *  , int  , struct ucred * );
extern int cdfs_getattr(struct vnode *  , register struct vattr *  , struct ucred * );
extern int cdfs_setattr(register struct vnode *  , register struct vattr *  , register struct ucred * );
extern int cdfs_read(struct vnode *  , register struct uio *  , int  , struct ucred * );
extern int cdfs_write(register struct vnode *  , struct uio *  , int  , struct ucred * );
extern int cdfs_ioctl(struct vnode *  , int  , caddr_t  , int  , struct ucred * );
		/* Prototype include a typedef name.
		   It should be moved after the typedef declaration */
extern int cdfs_fsync(struct vnode *  , int  , struct ucred *  , int );
extern int cdfs_seek(struct vnode *  , off_t  , off_t  , struct ucred * );
		/* Prototype include a typedef name.
		   It should be moved after the typedef declaration */
extern int cdfs_remove(struct nameidata * );
extern int cdfs_link(register struct vnode *  , register struct nameidata * );
extern int cdfs_rename(register struct nameidata *  , register struct nameidata * );
extern int cdfs_mkdir(struct nameidata *  , struct vattr * );
extern int cdfs_rmdir(register struct nameidata * );
extern int cdfs_symlink(struct nameidata *  , struct vattr *  , char * );
extern int cdfs_readdir(struct vnode *  , register struct uio *  , struct ucred *  , int * );
extern int cdfs_readlink(struct vnode *  , struct uio *  , struct ucred * );
extern int cdfs_abortop(register struct nameidata * );
extern int cdfs_bmap(struct vnode *  , daddr_t  , struct vnode * *  , daddr_t * );
		/* Prototype include a typedef name.
		   It should be moved after the typedef declaration */
extern int cdfs_strategy(register struct buf * );
extern int cdfs_print(struct vnode * );
extern int cdfs_page_read(struct vnode *  , struct uio *  , struct ucred * );
extern int cdfs_bread(register struct vnode *  , off_t  , struct buf * *  , struct ucred * );
		/* Prototype include a typedef name.
		   It should be moved after the typedef declaration */
extern int cdfs_brelse(register struct vnode *  , register struct buf * );
extern char cdfs_tolower(int );
extern int cdfs_pvd(struct cdfsmount *  , struct iso9660_pvd * );
extern int cdfs_cpvd(struct cdfsmount *  , char * * );
extern int cdfs_xar(struct vnode *  , int  , struct iso9660_xar *  , int  , int  , int * );
extern int cdfs_cxar(struct vnode *  , int  , char *  , int  , int * );
extern int cdfs_drec(struct vnode *  , int  , struct iso9660_drec * );
extern int cdfs_cdrec(struct vnode *  , int  , char * );
extern void xcdr_map_uid_gid(uid_t,gid_t, struct cdfsmount *, struct cdnode *);
extern void cdfs_adjust_dirent_name(struct cdfsmount *, char *,
				    unsigned short *);
extern ino_t rrip_parent_num(struct fs *, struct vnode *, struct iso_dir *, struct cdnode *);
extern int rrip_skipdir(struct fs *, struct vnode *, struct iso_dir *, struct cdnode *);
extern ino_t rrip_child_num(struct fs *, struct vnode *, struct iso_dir *, struct cdnode *);
extern dev_t rrip_getnodedevmap(struct fs *, struct cdnode *);
extern int rrip_mapdev(struct fs *, struct cdnode *, dev_t, char *, int);
extern int rrip_unmapdev(struct fs *, struct cdnode *);
extern void rrip_dealloc_devmap(struct fs *);
struct rrip_devmap *rrip_getnthdevmap(struct fs *, int n);
#endif /* _KERNEL */
#endif	/* _CDFS_FS_H_ */
