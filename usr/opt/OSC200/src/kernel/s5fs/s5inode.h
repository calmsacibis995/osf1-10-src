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
 *	@(#)$RCSfile: s5inode.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:52:06 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * 20-aug-1991:	vipul patel
 * 	OSF/1 Release 1.0.1, macro for fake_vnode 
 *	s5inode size change, redefination of locking macros.
 */


#ifndef	_S5INODE_H_
#define _S5INODE_H_
/*
 *	The I node is the focus of all
 *	file activity in unix. There is a unique
 *	inode allocated for each active file,
 *	each current directory, each mounted-on
 *	file, text file, and the root. An inode is 'named'
 *	by its dev/inumber pair. (iget/iget.c)
 *	Data, from mode on, is read in
 *	from permanent inode on volume.
 */

#define	NADDR	13
#define	NSADDR	(NADDR*sizeof(daddr_t)/sizeof(short))


struct	s5inode
{
	struct	s5inode	*i_forw;	/* inode hash chain */
	struct	s5inode	*i_back;	/* '' */
	struct  vnode   *i_vnode;  	/* the associated vnode */
	struct	vnode   *i_devvp;	/* vnode for block I/O */
	u_long	i_flag;
	dev_t	i_dev;		/* device where inode resides */
	s5ino_t	i_number;	/* i number, 1-to-1 with device address */
	struct  filsys *i_s5fs;  /* file system associated with this file */
	short	i_dirstamp;	/* optimize directory insertions */
#ifdef _KERNEL
	lock_data_t i_lock;	/* inode lock */
#endif /* _KERNEL */
	ushort	i_mode;
	short	i_nlink;	/* directory entries */
	ushort	i_uid;		/* owner */
	ushort	i_gid;		/* group of owner */
	off_t	i_size;		/* size of file */
	long    i_gen;          /* generation number */
	struct {
		union {
			daddr_t i_a[NADDR];	/* if normal file/directory */
			short	i_f[NSADDR];	/* if fifio's */
		} i_p;
		daddr_t	i_l;		/* last logical block read (for read-ahead) */
	}i_blks;
	long	*i_map;		/* Ptr to the block number map for the file */
};


/* modes */
#define	S5IFMT	0170000			/* type of file */
#define		S5IFDIR	0040000	/* directory */
#define		S5IFCHR	0020000		/* character special */
#define		S5IFBLK	0060000		/* block special */
#define		S5IFREG	0100000		/* regular */
#define		S5IFLNK	0120000 	/* symbolic link */	
#define		S5IFSOCK	0140000 /* socket */	
#define		S5IFMPC	0030000	/* multiplexed char special */
#define		S5IFMPB	0070000	/* multiplexed block special */
#define		S5IFIFO	0010000	/* fifo special */

#define	i_addr	i_blks.i_p.i_a
#define	i_lastr	i_blks.i_l
#define	i_rdev	i_blks.i_p.i_a[0]

#define	i_faddr	i_blks.i_p.i_a
#define	NFADDR	10
#define	PIPSIZ	NFADDR*BSIZE
#define	i_frptr	i_blks.i_p.i_f[NSADDR-5]
#define	i_fwptr	i_blks.i_p.i_f[NSADDR-4]
#define	i_frcnt	i_blks.i_p.i_f[NSADDR-3]
#define	i_fwcnt	i_blks.i_p.i_f[NSADDR-2]
#define	i_fflag	i_blks.i_p.i_f[NSADDR-1]
#define	S5IFIR	01
#define	S5IFIW	02

#define	S5ISUID		04000		/* set user id on execution */
#define	S5ISGID		02000		/* set group id on execution */
#define S5ISVTX		01000		/* save swapped text even after use */
#define	S5IREAD		0400		/* read, write, execute permissions */
#define	S5IWRITE	0200
#define	S5IEXEC		0100

#ifdef	_KERNEL
/* flags */
#define	S5IUPD		02		/* file has been modified */
#define	S5IACC		04		/* inode access time to be updated */
#define	S5IMOUNT	010		/* inode is mounted on */
#define	S5IWANT		020		/* some process waiting on lock */
#define	S5ITEXT		040		/* inode is pure text prototype */
#define	S5ICHG		0100		/* inode has been changed */
#define S5ISYN		0200		/* do synchronous write for iupdate */
#define S5IRENAME	0400		/* inode is being renamed */

/*
 Convert between inode pointers and vnode pointers
 */
#define S5VTOI(vp)	((struct s5inode *)(vp)->v_data)
#define S5ITOV(ip)	((ip)->i_vnode)

/*
 * Convert between vnode types and inode formats
 */
extern enum vtype	s5iftovt_tab[];
extern int		s5vttoif_tab[];
#define S5IFTOVT(mode)	(s5iftovt_tab[((mode) & S5IFMT) >> 12])
#define S5VTTOIF(indx)	(s5vttoif_tab[(int)(indx)])

/*
 * Occasionally a fake inode must be built ``by-hand''.
 */
#define FAKE_S5INODE_SIZE (sizeof(struct vnode)+sizeof(struct s5inode)-sizeof(long))

u_long	nextgennumber;		/* next generation number to assign */

#define	s5ILOCK(ip)					\
MACRO_BEGIN						\
	lock_write(&(ip)->i_lock);			\
MACRO_END

#define	s5IUNLOCK(ip)					\
MACRO_BEGIN						\
	lock_done(&(ip)->i_lock);			\
MACRO_END

#define s5_SET_RECURSIVE(ip)				\
	lock_set_recursive(&(ip)->i_lock);

#define s5_CLEAR_RECURSIVE(ip)				\
	lock_clear_recursive(&(ip)->i_lock);

#if MACH_LDEBUG
#define s5ILOCK_HOLDER(ip)				\
	LOCK_HOLDER(&(ip)->i_lock)
#else
#define s5ILOCK_HOLDER(ip) (TRUE)
#endif

#define s5LOCK_LOCKED(ip)				\
	lock_islocked(&(ip)->i_lock)

#define s5IN_LOCK_INIT(ip)	lock_init(&(ip)->i_lock, TRUE);

/*
 * This overlays the fid structure (see mount.h)
 */
struct ufid {
	u_short	ufid_len;	/* length of structure */
	u_short	ufid_pad;	/* force long alignment */
	ino_t	ufid_ino;	/* file number (ino) */
	long	ufid_gen;	/* generation number */
};

#endif	/* _KERNEL */
#endif	/*_S5INODE_H*/
