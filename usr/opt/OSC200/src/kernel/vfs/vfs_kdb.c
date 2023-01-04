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
static char	*sccsid = "@(#)$RCSfile: vfs_kdb.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/05/12 18:19:03 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#include <mach_kdb.h>
#include <mach_assert.h>
#include <quota.h>
#if	MACH_KDB

#include <sys/secdefines.h>
#if SEC_ARCH
#include <sys/security.h>
#endif

#include <sys/param.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/errno.h>
#include <sys/ucred.h>
#include <sys/namei.h>
#include <sys/buf.h>
#include <sys/tty.h>
#include <sys/file.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <ufs/quota.h>
#include <ufs/inode.h>
#include <ufs/fs.h>
#include <ufs/ufsmount.h>
#include <sys/socket.h>
#include <nfs/nfs.h>
#include <kern/event.h>

#define	printf	kdbprintf

#if	MACH_SLOCKS
void print_simple_lock();
#endif
void print_rw_lock();

extern int indent;

extern void proc_print();

/*
 * VFS structures
 */

void
bflags(flag)
	int flag;
{
	if (flag & B_READ)
		printf("B_READ");
	else
		printf("B_WRITE");
	if (flag & B_ERROR)
		printf("|B_ERROR");
	if (flag & B_BUSY)
		printf("|B_BUSY");
	if (flag & B_PHYS)
		printf("|B_PHYS");
	if (flag & B_WANTED)
		printf("|B_WANTED");
	if (flag & B_AGE)
		printf("|B_AGE");
	if (flag & B_ASYNC)
		printf("|B_ASYNC");
	if (flag & B_DELWRI)
		printf("|B_DELWRI");
	if (flag & B_CACHE)
		printf("|B_CACHE");
	if (flag & B_INVAL)
		printf("|B_INVAL");
	if (flag & B_LOCKED)
		printf("|B_LOCKED");
	if (flag & B_HEAD)
		printf("|B_HEAD");
	if (flag & B_USELESS)
		printf("|B_USELESS");
	if (flag & B_BAD)
		printf("|B_BAD");
	if (flag & B_RAW)
		printf("|B_RAW");
	if (flag & B_NOCACHE)
		printf("|B_NOCACHE");
}

void
mntflags(flag)
	int flag;
{
	if (flag & M_RDONLY)
		printf("M_RDONLY");
	else
		printf("READ-WRITE");
	if (flag & M_SYNCHRONOUS)
		printf(" ,M_SYNCHRONOUS");
	if (flag & M_NOEXEC)
		printf(" ,M_NOEXEC");
	if (flag & M_NOSUID)
		printf(" ,M_NOSUID");
	if (flag & M_NODEV)
		printf(" ,M_NODEV");
	if (flag & M_EXPORTED)
		printf(" ,M_EXPORTED");
	if (flag & M_EXRDONLY)
		printf(" ,M_EXRDONLY");
	if (flag & M_LOCAL)
		printf(" ,M_LOCAL");
	if (flag & M_QUOTA)
		printf(" ,M_QUOTA");
	if (flag & M_SYNCING)
		printf(" ,M_SYNCING");
	if (flag & M_SWAP_PREFER)
		printf(" ,M_SWAP_PREFER");
	if (flag & M_SWAP_NEVER)
		printf(" ,M_SWAP_NEVER");
#if	SEC_ARCH
	if (flag & M_SECURE)
		printf(" ,M_SECURE");
#endif
}

void
vnflags(flag)
	int flag;
{
	if (flag & VROOT)
		printf("VROOT");
	if (flag & VXLOCK)
		printf("|VXLOCK");
	if (flag & VXWANT)
		printf("|VXWANT");
	if (flag & VEXLOCK)
		printf("|VEXLOCK");
	if (flag & VSHLOCK)
		printf("|VSHLOCK");
	if (flag & VLWAIT)
		printf("|VLWAIT");
	if (flag & VMOUNTING)
		printf("|VMOUNTING");
	if (flag & VMOUNTWAIT)
		printf("|VMOUNTWAIT");
	if (flag & VFLOCK)
		printf("|VFLOCK");
	if (flag & VFWAIT)
		printf("|VFWAIT");
}

void
iflags(flag)
	int flag;
{
	if (flag & IRENAME)
		printf("IRENAME");
	if (flag & IACC)
		printf(", IACC");
	if (flag & ICHG)
		printf(", ICHG");
	if (flag & IMOD)
		printf(", IMOD");
	if (flag & ISHLOCK)
		printf(", ISHLOCK");
	if (flag & IEXLOCK)
		printf(", IEXLOCK");
	if (flag & ILWAIT)
		printf(", ILWAIT");
	if (flag & INACTIVATING)
		printf(", INACTIVATING");
	if (flag & INACTWAIT)
		printf(", INACTWAIT");
	if (flag & IREADERROR)
		printf(", IREADERROR");
	if (flag & IQUOTA)
		printf(", IQUOTA");
	if (flag & IQUOTING)
		printf(", IQUOTING");
	if (flag & IQUOTWAIT)
		printf(", IQUOTWAIT");
}

void
alias_flags(flag)
{
	if (flag & SA_MOUNTED)
		printf("SA_MOUNTED");
	else
		printf("Not mounted");
	if (flag & SA_CLOSING)
		printf(", SA_CLOSING");
	if (flag & SA_GOING)
		printf(", SA_GOING");
	if (flag & SA_WAIT)
		printf(", SA_WAIT");
}

void
file_print(fp)
register struct file *fp;
{
	iprintf("f_flag = 0x%X, f_type = 0x%X, f_count = 0x%X\n",
		   fp->f_flag, fp->f_type, fp->f_count);
	printf("f_msgcount = 0x%X, f_cred = 0x%X, f_fops = 0x%X\n",
		   fp->f_msgcount, fp->f_cred, fp->f_ops);
	printf("f_data = 0x%X, f_offset = 0x%X\n",
		   fp->f_data, fp->f_offset, 0);
}

void
alias_print(sa)
	struct specalias *sa;
{
	iprintf("sa_type = %s, ",(sa->sa_type==VBLK? "VBLK": "VCHR"));
	printf("sa_flag = ");
	alias_flags(sa->sa_flag);
	printf("\n");
	iprintf("sa_rdev = 0x%X, sa_usecount = 0x%X\n",
		sa->sa_rdev, sa->sa_usecount);
	iprintf("sa_vlist = 0x%X, sa_vnode = 0x%X, sa_next = 0x%X\n",
		sa->sa_vlist, sa->sa_vnode, sa->sa_next);
}

void
print_specinfo(si)
	struct specinfo *si;
{
	iprintf("si_rdev = 0x%X, si_nextalias = 0x%X\n", 
		si->si_rdev, si->si_nextalias);	
	iprintf("si_shadowvp = 0x%X, si_alias = 0x%X\n", 
		si->si_shadowvp, si->si_alias);	
	if (si->si_alias) {
		struct specalias *sa = si->si_alias;
		indent += 2;
		alias_print(si->si_alias);
		indent -= 2;
	}
}

char *vtypes[] = {"VNON", "VREG", "VDIR", "VBLK", "VCHR", "VLNK", "VSOCK", "VFIFO", "VBAD"};

void
vnode_print(vp)
register struct vnode *vp;
{
	int i;

	iprintf("v_usecount = 0x%X, v_holdcnt = 0x%X, v_flag = 0x%X\n",
		   vp->v_usecount, vp->v_holdcnt, vp->v_flag);
	if (vp->v_flag) {
		printf("	");
		vnflags(vp->v_flag);
		printf("\n");
	}
	printf("v_shlockc = 0x%X, v_exlockc = 0x%X, v_lastr = 0x%X\n",
		   vp->v_shlockc, vp->v_exlockc, vp->v_lastr);
	printf("v_id = 0x%X, v_mount = 0x%X, v_op = 0x%X\n",
		   vp->v_id, vp->v_mount, vp->v_op);
	printf("v_freef = 0x%X, v_freeb = 0x%X, v_mountf = 0x%X\n",
		   vp->v_freef, vp->v_freeb, vp->v_mountf);
	printf("v_mountb = 0x%X, v_cleanblkhd = 0x%X, v_dirtyblkhd = 0x%X\n",
		   vp->v_mountb, vp->v_cleanblkhd, vp->v_dirtyblkhd);
	i = (int) vp->v_type;
	printf("v_numoutput = 0x%X, v_type = %s, v_un = 0x%X\n",
		   vp->v_numoutput, vtypes[i], vp->v_mountedhere);
	switch(vp->v_type) {
	case VDIR:
		printf("v_mountedhere = 0x%X\n",
			   vp->v_mountedhere);
		break;
	case VSOCK:
		printf("v_socket = 0x%X\n",
			   vp->v_socket);
		break;
	case VCHR:
	case VBLK:
		indent += 2;
		print_specinfo(vp->v_specinfo);
		indent -= 2;
		break;
	default:
		break;
	}
#if	MACH_SLOCKS
	iprintf("v_lock:  ");
	indent += 2;
	print_simple_lock(&vp->v_lock);
	indent -= 2;
	iprintf("v_buflists_lock:  ");
	indent += 2;
	print_simple_lock(&vp->v_buflists_lock);
	indent -= 2;
#endif
	iprintf("v_tag = 0x%X, v_vm_info = 0x%X, v_data = 0x%X\n",
		   vp->v_tag, vp->v_vm_info, vp->v_data);
}

void
vnode_check()
{
	register struct vnode *vp;
	int flag;
	extern struct vnode *vfreeh;

	for (vp = vfreeh; vp; vp = vp->v_freef) {
		/* see if we tlb miss on the v_mount */
		if (vp->v_mount != DEADMOUNT)
			flag = vp->v_mount->m_flag;	
	}
}

void
statfs_print(sp)
register struct statfs *sp;
{
	
	iprintf("f_type = %d, f_flags = 0x%x, f_fsize = %D\n",
		   sp->f_type, sp->f_flags, sp->f_fsize);
	iprintf("f_bsize = %D, f_blocks = %D, f_bfree = %D\n",
		   sp->f_bsize, sp->f_blocks, sp->f_bfree);
	iprintf("f_bavail = %D, f_files = %D, f_ffree = %D, f_fsid = 0x%X\n",
		   sp->f_bavail, sp->f_files, sp->f_ffree, &sp->f_fsid);
	iprintf("f_mntonname = %s\n", sp->f_mntonname);
	iprintf("f_mntfromname = %s\n", sp->f_mntfromname);
}


void
mount_print(mp)
register struct mount *mp;
{
	iprintf("m_next = 0x%X, m_prev = 0x%X, m_op = 0x%X, m_flag = ",
		   mp->m_next, mp->m_prev, mp->m_op);
	mntflags(mp->m_flag);
	printf("\n");
	iprintf("m_vnodecovered = 0x%X, m_mounth = 0x%X, m_exroot = 0x%X, m_data = 0x%X\n",
		   mp->m_vnodecovered, mp->m_mounth,
		   mp->m_exroot, (char *)(mp->m_data));
	iprintf("m_stat:\n");
	indent += 2;
	statfs_print(&mp->m_stat);
	indent -= 2;
#if	SEC_ARCH
	if ((mp->m_flag & M_SECURE) == 0) {
		register int i;

		iprintf("Security Policy Tags:");
		for (i = 0; i < SEC_TAG_COUNT; i++) {
			if ((i % 4) == 0)
				printf("\n");
			printf("\t%x", mp->m_tag[i]);
		}
		printf("\n");
	}
#endif	/* SEC_ARCH */
}


void
mnt_vnode_print(mp)
register struct mount *mp;
{

	register struct vnode *vp = mp->m_mounth;

	if (vp == (struct vnode *)0) {
		iprintf("vnode list is empty\n");
		return;
	}
	do {
		iprintf("vp = 0x%X: ", vp);
		printf("v_flag = ");
		vnflags(vp->v_flag);
		printf(", v_usecount = 0x%X, v_mountf = 0x%X, v_mountb = 0x%X\n",
			vp->v_usecount, vp->v_mountf, vp->v_mountb);
		vp = vp->v_mountf;
	} while( vp != (struct vnode *)0);
}



#define v_alias v_specinfo->si_alias
#define v_nextalias v_specinfo->si_nextalias

void
vnode_alias_print(vp)
register struct vnode *vp;
{
	register struct specalias *sa = vp->v_alias;

	if (sa == (struct specalias *)0) {
		iprintf("no alias structure\n");
		return;
	}
	iprintf("Alias structure:\n");
	indent += 2;
	alias_print(sa);
	indent -= 2;
	iprintf("Alias list:\n");
	indent += 2;
	vp = sa->sa_vlist;
	while (vp) {
		iprintf("vnode = 0x%X\n", vp);
		vp = vp->v_nextalias;
	}
	indent -= 2;
}

void
event_print(event)
	event_t *event;
{
	iprintf("ev_event = %d\n", event->ev_event);
#if	MACH_SLOCKS
	print_simple_lock(&event->ev_slock);
#endif
}

void
buffer_print(bp)
register struct buf *bp;
{
	
	iprintf("b_flags = ");
	bflags(bp->b_flags);
	printf(", b_forw = 0x%X, b_back = 0x%X\n",
		bp->b_forw, bp->b_back);
	iprintf("av_forw = 0x%X, av_back = 0x%X, b_blockf = 0x%X\n",
		bp->av_forw, bp->av_back, bp->b_blockf);
	iprintf("b_blockb = 0x%X, b_bcount = 0x%X, b_bufsize = 0x%X\n",
		bp->b_blockb, bp->b_bcount, bp->b_bufsize);
	iprintf("b_error = 0x%X, b_dev = 0x%X, b_addr = 0x%X\n",
		bp->b_error, bp->b_dev, bp->b_un.b_addr);
	iprintf("b_lblkno = 0x%X, b_blkno = 0x%X, b_resid = 0x%X\n",
		bp->b_lblkno, bp->b_blkno, bp->b_resid);
	iprintf("b_iodone = 0x%X, h_chain = 0x%X, b_vp = 0x%X\n", 
		bp->b_iodone, bp->b_hash_chain, bp->b_vp);
	iprintf("b_rvp = 0x%X, b_rcred = 0x%X, b_wcred = 0x%X\n",
		bp->b_rvp, bp->b_rcred, bp->b_wcred);
	iprintf("b_dirtyoff = 0x%X, b_dirtyend = 0x%X\n",
		   bp->b_dirtyoff, bp->b_dirtyend);
	iprintf("b_lock:\n");
	indent += 2;
	print_rw_lock(&bp->b_lock);
	indent -= 2;
	iprintf("b_iocomplete (event):\n");
	indent += 2;
	event_print(&bp->b_iocomplete);
	indent -= 2;
}

void
bfreelist_print(sbp)
register struct buf *sbp;
{
	register struct buf *bp = sbp;

	do {
		iprintf("b_flags = ");
		bflags(bp->b_flags);
		printf(", av_forw = 0x%X, av_back = 0x%X\n",
			bp->av_forw, bp->av_back);
		bp = bp->av_forw;
	} while( bp && bp != sbp);
}


void
bhash_chain_print(sbp)
struct buf *sbp;
{
	struct buf *bp = sbp;

	do {
		iprintf("b_flags = ");
		bflags(bp->b_flags);
		printf(", b_forw = 0x%X, b_back = 0x%X\n",
			bp->b_forw, bp->b_back);
		bp = bp->b_forw;
	} while( bp && bp != sbp);
}

void
bvnode_list_print(bp)
struct buf *bp;
{
	do {
		iprintf("b_flags = ");
		bflags(bp->b_flags);
		printf(", b_blockf = 0x%X, b_blockb = 0x%X\n",
			bp->b_blockf, bp->b_blockb);
		bp = bp->b_blockf;
	} while( bp != (struct buf *)0);
}

void
vnode_clnlist_print(vp)
struct vnode *vp;
{
	struct buf *bp = vp->v_cleanblkhd;

	if (bp == (struct buf *)0) {
		return;
	}
	iprintf("vnode = 0x%X\n", vp);
	do {
		iprintf("b_flags = ");
		bflags(bp->b_flags);
		printf(", b_blockf = 0x%X, b_blockb = 0x%X\n",
			bp->b_blockf, bp->b_blockb);
		bp = bp->b_blockf;
	} while( bp != (struct buf *)0);
}

void
vnode_drtylist_print(vp)
struct vnode *vp;
{
	register struct buf *bp = vp->v_dirtyblkhd;

	if (bp == (struct buf *)0) {
		return;
	}
	iprintf("vnode = 0x%X\n", vp);
	indent += 2;
	do {
		iprintf("b_flags = ");
		bflags(bp->b_flags);
		printf(", b_blockf = 0x%X, b_blockb = 0x%X\n",
			bp->b_blockf, bp->b_blockb);
		bp = bp->b_blockf;
	} while( bp != (struct buf *)0);
	indent -= 2;
}

dirtyino_print()
{
	register struct mount *mp;
	register struct vnode *vp;
	register struct inode *ip;
	/*
	 * do a filesystem at a time, starting with root
	 */
	mp = rootfs;
	do {
		/*
		 * Only examine UFS filesystems.
		 */
		if (mp->m_stat.f_type == MOUNT_UFS) {
			iprintf("File system: %s (flags 0x%X) (mnt ptr 0x%X)\n", 
				mp->m_stat.f_mntonname, mp->m_flag, mp);
			indent += 2;
			for (vp = mp->m_mounth; vp; vp = vp->v_mountf) {
				ip = VTOI(vp);
				if (ip->i_flag & (IMOD|IACC|IUPD|ICHG))
					iprintf("inode dirty: vp = 0x%X\n",vp);
			}
			indent -= 2;
		}
		mp = mp->m_next;
	} while (mp && (mp != rootfs));
}

dirtybuf_print()
{
	register struct mount *mp;
	register struct vnode *vp;
	/*
	 * do a filesystem at a time, starting with root
	 */
	mp = rootfs;
	do {
		/*
		 * Only examine UFS filesystems.
		 */
		if (mp->m_stat.f_type == MOUNT_UFS) {
			iprintf("File system: %s (flags 0x%X) (mnt ptr 0x%X)\n", 
				mp->m_stat.f_mntonname, mp->m_flag, mp);
			indent += 2;
			for (vp = mp->m_mounth; vp; vp = vp->v_mountf) {
				vnode_drtylist_print(vp);
			}
			indent -= 2;
		}
		mp = mp->m_next;
	} while (mp && (mp != rootfs));
}

/*
 * UFS structures
 */

void
inode_print(ip)
register struct inode *ip;
{
	register i;

	iprintf("i_chain[0]  = 0x%X, i_chain[1] = 0x%X, i_vnode = 0x%X\n",
		   ip->i_chain[0], ip->i_chain[1], ip->i_vnode);
	iprintf("i_devvp = 0x%X, i_flag = 0x%X, i_dev = 0x%X\n",
		   ip->i_devvp, ip->i_flag, ip->i_dev);
	iprintf("i_number = 0x%X, i_fs = 0x%X\n", ip->i_number, ip->i_fs);
	iprintf("i_diroff = 0x%X, i_endoff = 0x%X i_dirstamp = 0x%x\n",
		   ip->i_diroff, ip->i_endoff, ip->i_dirstamp);
	iprintf("di_mode = 0%o, di_nlink = 0x%X, di_uid =%d\n",
		   ip->i_mode, ip->i_nlink, ip->i_uid);
	iprintf("di_gid = %d, di_size = 0x%lX, di_atime = 0x%X\n",
		   ip->i_gid, ip->i_size, ip->i_atime);
	iprintf("di_mtime = 0x%X, di_ctime = 0x%X\n",
		   ip->i_mtime, ip->i_ctime);
	iprintf("flags = ");
	iflags(ip->i_flags);
	printf(", blocks = 0x%X, gen = 0x%X\n",
		ip->i_blocks, ip->i_gen);
	iprintf("i_io_lock:  ");
	indent += 2;
	print_rw_lock(&ip->i_io_lock);
	indent -= 2;
#if	MACH_SLOCKS
	iprintf("i_incore_lock:  ");
	indent += 2;
	print_simple_lock(&ip->i_incore_lock);
	indent -= 2;
#endif
	if (ip->i_flags & IC_FASTLINK)
		iprintf("di_Msymlink: %s\n", ip->i_symlink);
	else if (((ip->i_mode & IFMT) == IFCHR) || 
		 ((ip->i_mode & IFMT) == IFBLK)) {
		iprintf("i_rdev = 0x%X\n", ip->i_rdev);
	} else {
		iprintf("Disk block addresses:");
		indent += 2;
		for (i = 0; i < NDADDR; i++) {
			if ((i % 6) == 0)
				printf("\n");
			iprintf("\t%x", ip->i_db[i]);
		}
		indent -= 2;
		printf("\nIndirect Block Addresses");
		indent += 2;
		for (i = 0; i < NIADDR; i++) {
			if ((i % 6) == 0)
				printf("\n");
			iprintf("\t%x", ip->i_ib[i]);
		}
		printf("\n");
		indent -= 2;
	}
#if	SEC_FSCHANGE
	iprintf("di_gpriv = %X %X, di_ppriv = %X %X\n",
		ip->i_gpriv[0], ip->i_gpriv[1], ip->i_ppriv[0], ip->i_ppriv[1]);
	iprintf("di_type_flags = %x, di_parent = %d\n",
		ip->i_type_flags, ip->i_parent);
	iprintf("Security Policy Tags:");
	for (i = 0; i < SEC_TAG_COUNT; i++) {
		if ((i % 4) == 0)
			printf("\n");
		iprintf("\t%x", ip->i_tag[i]);
	}
	printf("\n");
#endif	/* SEC_FSCHANGE */
}

csum_print(csp)
struct csum *csp;
{
	iprintf(" ndir: %x nbfree: %x ",
	       csp->cs_ndir, csp->cs_nbfree);
	printf("nifree: %x nffree: %x\n",
	       csp->cs_nifree, csp->cs_nffree);
}

void
cg_print(cgp)
register struct cg *cgp;
{
	register struct ocg *ocgp;
	int i;
	iprintf("format: %s\n", 
		(cgp->cg_magic == CG_MAGIC ? "dynamic" : "static"));
	iprintf("cg_link = 0x%X, cg_time = 0x%X, cg_cgx = 0x%X\n",
		cgp->cg_link, cgp->cg_time, cgp->cg_cgx);
	iprintf("cg_ncyl = 0x%X, cg_niblk = 0x%X, cg_ndblk = 0x%X\n",
		cgp->cg_ncyl, cgp->cg_niblk, cgp->cg_ndblk);
	iprintf("summary info: ");
	indent += 4;
	csum_print(&cgp->cg_cs);
	indent -= 4;
	iprintf("cg_rotor = 0x%X, cg_frotor = 0x%X, cg_irotor = 0x%X\n",
		cgp->cg_rotor, cgp->cg_frotor, cgp->cg_irotor);
	iprintf("cg_frsum[]: ");
	if (cgp->cg_magic != CG_MAGIC) {
		/*
		 * Old style file system.
		 */
		ocgp = (struct ocg *)cgp;
		for (i=0; i < 8; i++)
			printf("%d=%d; ", i, ocgp->cg_frsum[i]);
		printf("\n");
		iprintf("cg_btot[]: ");
		for (i=0; i < 32; i++)
			printf("%d=%d; ", i, ocgp->cg_btot[i]);
		printf("\n");
		iprintf("ocg->cg_b = 0x%X, ocg->cg_iused = 0x%X\n",
			 (caddr_t)ocgp->cg_b, (caddr_t)ocgp->cg_iused);

	} else {
		/*
		 * New file system (tahoe).
		 */
		for (i=0; i<MAXFRAG; i++)
			printf("%d=%d; ", i, cgp->cg_frsum[i]);
		printf("\n");
		iprintf("cg_btotoff = 0x%X, cg_boff = 0x%X cg_iusedoff= 0x%X\n",
			 cgp->cg_btotoff, cgp->cg_boff, cgp->cg_iusedoff);
		iprintf("cg_freeoff = 0x%X, cg_nextfreeoff= 0x%X\n",
		 	cgp->cg_freeoff, cgp->cg_nextfreeoff);
	}
}

void
fs_print(fs)
register struct fs *fs;
{

	register i, cspflag;

	iprintf("fs_link = 0x%X, fs_rlink = 0x%X, fs_sblkno = 0x%X\n",
		   fs->fs_link, fs->fs_rlink, fs->fs_sblkno);
	iprintf("fs_cblkno = 0x%X, fs_iblkno = 0x%X, fs_dblkno = 0x%X\n",
		   fs->fs_cblkno, fs->fs_iblkno, fs->fs_dblkno);
	iprintf("fs_cgoffset = 0x%X, fs_cgmask = 0x%X, fs_time = 0x%X\n",
		   fs->fs_cgoffset, fs->fs_cgmask, fs->fs_time);
	iprintf("fs_size = 0x%X, fs_dsize = 0x%X, fs_ncg = 0x%X\n",
		   fs->fs_size, fs->fs_dsize, fs->fs_ncg);
	iprintf("fs_bsize = 0x%X, fs_fsize = 0x%X, fs_frag = 0x%X\n",
		   fs->fs_bsize, fs->fs_fsize, fs->fs_frag);
	iprintf("fs_minfree = 0x%X, fs_rotdelay = 0x%X, fs_rps = 0x%X\n",
		   fs->fs_minfree, fs->fs_rotdelay, fs->fs_rps);
	iprintf("fs_bmask = 0x%X, fs_fmask = 0x%X, fs_bshift = 0x%X\n",
		   fs->fs_bmask, fs->fs_fmask, fs->fs_bshift);
	iprintf("fs_maxcontig = 0x%X, fs_maxbpg = 0x%X, fs_fragshift = 0x%X\n",
		   fs->fs_maxcontig, fs->fs_maxbpg, fs->fs_fragshift);
	iprintf("fs_fsbtodb = 0x%X, fs_sbsize = 0x%X, fs_csmask = 0x%X\n",
		   fs->fs_fsbtodb, fs->fs_sbsize, fs->fs_csmask);
	iprintf("fs_csshift = 0x%X, fs_nindir = 0x%X, fs_inopb = 0x%X\n",
		   fs->fs_csshift, fs->fs_nindir, fs->fs_inopb);
	iprintf("fs_nspf:%x       fs_optim:%x\n = 0x%X\n",
	           fs->fs_nspf, fs->fs_optim);
	iprintf("fs_npsect = 0x%X, fs_interleave = 0x%X, fs_trackskew = 0x%X\n",
		   fs->fs_npsect, fs->fs_interleave, fs->fs_trackskew);
	iprintf("fs_headswitch = 0x%X, fs_trkseek = 0x%X\n",
		   fs->fs_headswitch, fs->fs_trkseek);
	iprintf("fs_csaddr = 0x%X, fs_cssize = 0x%X, fs_cgsize = 0x%X\n",
		   fs->fs_csaddr, fs->fs_cssize, fs->fs_cgsize);
	iprintf("fs_ntrak = 0x%X, fs_nsect = 0x%X, fs_spc = 0x%X\n",
		   fs->fs_ntrak, fs->fs_nsect, fs->fs_spc);
	iprintf("fs_ncyl = 0x%X, fs_cpg = 0x%X, fs_ipg = 0x%X\n",
		   fs->fs_ncyl, fs->fs_cpg, fs->fs_ipg);
	iprintf("fs_fpg = 0x%X, fs_fmod = 0x%X, fs_clean = 0x%X\n",
		   fs->fs_fpg, fs->fs_fmod, fs->fs_clean);
	iprintf("fs_ronly = 0x%X, fs_flags = 0x%X, fs_cgrotor = 0x%X\n",
		   fs->fs_ronly, fs->fs_flags, fs->fs_cgrotor);
#if	SEC_FSCHANGE
	iprintf("fs_cpc:%x       fs_magic:%x%s\n",
	       fs->fs_cpc, fs->fs_magic, FsSEC(fs) ? " (labeled)" : "");
#else
	iprintf("fs_cpc:%x       fs_magic:%x\n",
	       fs->fs_cpc, fs->fs_magic);
#endif
	iprintf("format = ");
	if (fs->fs_postblformat == FS_42POSTBLFMT)
		printf("4.2 style; ");
	else if (fs->fs_postblformat == FS_DYNAMICPOSTBLFMT)
		printf("dynamic; ");
	else
		printf("UNKNOWN; ");
	printf(" fs_nrpos = %d, fs_postbloff = 0x%X, fs_rotbloff = 0x%X\n",
		fs->fs_nrpos, fs->fs_postbloff, fs->fs_rotbloff);
	iprintf("fs_fsmnt: %s\n", fs->fs_fsmnt);
#if	MACH_SLOCKS
	iprintf("fs_lock:");
	indent += 2;
	print_simple_lock(&fs->fs_lock);
	indent -= 2;
#endif
	iprintf("fs_cstotal:\n");
	indent += 2;
	csum_print(&fs->fs_cstotal);
	indent -= 2;
	cspflag = FALSE;
	for (i = 0; i < MAXCSBUFS; i++) {
		if (fs->fs_csp[i]) {
			if (cspflag == FALSE) {
				cspflag = TRUE;
				indent += 2;
				iprintf("cs_csp:\n");
				indent -= 2;
			}
			indent +=2;
			iprintf("%d:", i);
			csum_print(fs->fs_csp[i]);
			indent -= 2;
		}
	}
}


void
ufsmount_print(ump)
register struct ufsmount *ump;
{
#if	QUOTA
	int i;
#endif

	iprintf("um_mountp = 0x%X, um_dev = 0x%X, um_devvp = 0x%X\n",
		   ump->um_mountp, ump->um_dev, ump->um_devvp);
#if	QUOTA
	iprintf("um_fs = 0x%X, um_qsync = 0x%X\n", ump->um_fs, ump->um_qsync);
	for (i = 0; i < MAXQUOTAS; ++i) {
		iprintf("um_quotas[%d] = 0x%X, um_cred[%d] = 0x%X",
		       ump->um_quotas[i], ump->um_cred[i]);
		printf(",um_btime[%d] = 0x%X, um_itime[%d] = 0x%x\n",
		       ump->um_btime[i], ump->um_itime[i]);
	}
#else
	iprintf("um_fs = 0x%X\n", ump->um_fs);
#endif
	iprintf("device name: %s\n", ump->um_mountp->m_stat.f_mntfromname);
	iprintf("mounted on: %s\n", ump->um_mountp->m_stat.f_mntonname);
}


void
ihash_chain_print(vip)
register struct inode *vip;
{
	register struct inode *ip = vip;

	do {
		iprintf("i_flag = 0x%X, i_forw = 0x%X, i_back = 0x%X\n",
			ip->i_flag, ip->i_forw, ip->i_back);
		ip = ip->i_forw;
	} while(ip != vip);
}



/*
 * NFS structures
 */

void
vattr_print(vp)
register struct vattr *vp;
{
	iprintf("va_type = 0x%X, va_mode = 0%o,	va_nlink = 0x%X\n",
		   vp->va_type, vp->va_mode, vp->va_nlink);
	iprintf("va_uid = %d, va_gid = %d, va_fsid = 0x%X\n",
		   vp->va_uid, vp->va_gid, vp->va_fsid);
	iprintf("va_fileid = 0x%X, va_size = 0x%lX 
#if !__alpha
			, va_size_rsv = 0x%X
#endif
				\n",
		   vp->va_fileid, vp->va_size
#if !__alpha
			, vp->va_size_rsv
#endif
		);
	iprintf("va_blocksize = 0x%X, va_gen = 0x%X, va_flags = 0x%X\n",
		   vp->va_blocksize, vp->va_gen, vp->va_flags);
	iprintf("va_rdev = 0x%X,	va_bytes = 0x%lX
#if !__alpha
			, va_bytes_rsv = 0x%X
#endif
				\n",
		   vp->va_rdev, vp->va_bytes
#if !__alpha
			, vp->va_bytes_rsv
#endif
		);
}

void
nfsnode_print(np)
register struct nfsnode *np;
{
	iprintf("n_chain[0] = 0x%X,	n_chain[1] = 0x%X,	n_flag = 0x%X\n",
		   np->n_chain[0], np->n_chain[1], np->n_flag);
	iprintf("n_vnode = 0x%X,	n_attrstamp = 0x%X,  = 0x%X\n",
		   np->n_vnode, np->n_attrstamp);
	iprintf("\tn_vattr:\n");
	indent += 2;
	vattr_print(&np->n_vattr);
	indent -= 2;
	iprintf("n_sillyrename = 0x%X,	n_lastr = 0x%X,	n_size = 0x%X\n",
		   np->n_sillyrename, np->n_lastr, np->n_size);
	iprintf("n_mtime = 0x%X,	n_error = 0x%X,  = 0x%X\n",
		   np->n_mtime, np->n_error);
}


void
nfsmount_print(nmp)
register struct nfsmount *nmp;
{
	iprintf("nm_flag = 0x%X, nm_mountp = 0x%X, nm_so = 0x%X\n",
		   nmp->nm_flag, nmp->nm_mountp, nmp->nm_so);
	iprintf("nm_hostinfo = 0x%X, nm_retry = 0x%X, nm_rexmit = 0x%X\n",
		   nmp->nm_hostinfo, nmp->nm_retry, nmp->nm_rexmit);
	iprintf("nm_rtt = 0x%X, nm_rto = 0x%X, nm_srtt = 0x%X\n",
		   nmp->nm_rtt, nmp->nm_rto, nmp->nm_srtt);
	iprintf("nm_rttvar = 0x%X, nm_rsize = 0x%X, nm_wsize = 0x%X\n",
		   nmp->nm_rttvar, nmp->nm_rsize, nmp->nm_wsize);
	/*
	 * printf("nm_host: %s\n", nmp->nm_host);
	 * printf("nm_path: %s\n", nmp->nm_path);
	 */
}


void
nfsreq_print(rep)
struct nfsreq *rep;
{
	iprintf("r_next = 0x%X, r_prev = 0x%X, r_mreq = 0x%X\n",
		   rep->r_next, rep->r_prev, rep->r_mreq);
	iprintf("r_mrep = 0x%X, r_mntp = 0x%X, r_vp = 0x%X, r_procp = 0x%X\n",
		   rep->r_mrep, rep->r_mntp, rep->r_vp, rep->r_procp);
	iprintf("r_msiz = 0x%X, r_xid = 0x%X, r_flags = 0x%X\n",
		   rep->r_msiz, rep->r_xid, rep->r_flags);
	iprintf("r_retry = 0x%X, r_rexmit = 0x%X, r_timer = 0x%X\n",
		   rep->r_retry, rep->r_rexmit, rep->r_timer);
}

void
nfshost_print(nfshp)
struct nfshost *nfshp;
{
	iprintf("nh_next = 0x%X, nh_prev = 0x%X, nh_refcnt = 0x%X\n",
		   nfshp->nh_next, nfshp->nh_prev, nfshp->nh_refcnt);
	iprintf("nh_currto = 0x%X, nh_currexmit = 0x%X, nh_sent = 0x%X\n",
		   nfshp->nh_currto, nfshp->nh_currexmit, nfshp->nh_sent);
	iprintf("nh_window = 0x%X, nh_winext = 0x%X, nh_ssthresh = 0x%X\n",
		   nfshp->nh_window, nfshp->nh_winext, nfshp->nh_ssthresh);
	iprintf("nh_salen = 0x%X, nh_sockaddr = 0x%X\n",
		   nfshp->nh_salen, nfshp->nh_sockaddr);
}

void
nfshash_chain_print(snp)
register struct nfsnode *snp;
{
	register struct nfsnode *np = snp;

	do {
		iprintf("n_flag = 0x%X, n_forw = 0x%X, n_back = 0x%X\n",
			np->n_flag, np->n_forw, np->n_back);
		np = np->n_forw;
	} while (np != snp);
}

void
nfsreq_list_print(vrep)
register struct nfsreq *vrep;
{
	register struct nfsreq *rep = vrep;

	do {
		iprintf("r_next = 0x%X, n_prev = 0x%X\n",
			rep->r_next, rep->r_prev);
		rep = rep->r_next;
	} while (rep != vrep && rep != (struct nfsreq *)0);
}

void
nfshost_list_print(vnfshp)
struct nfshost *vnfshp;
{
	register struct nfshost *nfshp = vnfshp;

	do {
		iprintf("nh_next = 0x%X, nh_prev = 0x%X\n",
			nfshp->nh_next, nfshp->nh_prev, 0);
		nfshp = nfshp->nh_next;
	} while (nfshp != vnfshp && nfshp != (struct nfshost *)0);
}


/*
 * Networking structures
 */

#include <sys/mbuf.h>

void
mbuf_print(m)
struct mbuf *m;
{
	
	iprintf("m_next = 0x%X, m_nextpkt = 0x%X, m_len = %D\n",
		   m->m_next, m->m_nextpkt, m->m_len);
	iprintf("m_data = 0x%X, m_type = %D, m_flags = 0x%X\n",
		   m->m_data, m->m_type, m->m_flags);
	if (m->m_flags & M_PKTHDR) {
		iprintf("packet header:\n");
		iprintf("len = %D, rcvif = 0x%X\n",
			   m->m_pkthdr.len, m->m_pkthdr.rcvif);
	}
	if (m->m_flags & M_EXT) {
		iprintf("external storage:\n");
		iprintf("buf = 0x%X, size = %D, free = 0x%X(0x%X)\n",
			   m->m_ext.ext_buf, m->m_ext.ext_size,
			   m->m_ext.ext_free, m->m_ext.ext_arg);
		if (MCLREFERENCED(m))
			iprintf("<referenced elsewhere>\n");

	}
}

void
mbuf_print_list(m)
struct mbuf *m;
{
	while (m) {
		iprintf("mbuf 0x%X:\n", m);
		mbuf_print(m);
		m = m->m_next;
	}
}

/*
 * print out lock data structures
 */
void
print_rw_lock(lock)
	lock_t lock;
{
	iprintf("Mutex lock: 0x%X\n", lock);
	iprintf("	want_write = 0x%X\n", lock->want_write);
	iprintf("	want_upgrade = 0x%X\n", lock->want_upgrade);
	iprintf("	waiting = 0x%X\n", lock->waiting);
	iprintf("	can_sleep = 0x%X\n", lock->can_sleep);
	iprintf("	read_count = 0x%X\n", lock->read_count);
	iprintf("	thread = 0x%X\n", lock->thread);
	iprintf("	recursion_depth = 0x%X\n", lock->recursion_depth);
#if	MACH_SLOCKS
	indent += 2;
	print_simple_lock(&lock->interlock);
	indent -= 2;
#endif
#if	MACH_LDEBUG || MACH_LTRACKS
	iprintf("	lthread = 0x%X\n", lock->lthread);
	iprintf("	lck_addr = 0x%X\n", lock->lck_addr);
	iprintf("	unlck_addr = 0x%X\n", lock->unlck_addr);
#endif
#if	LOCK_STATS && notyet
	iprintf("	lock_tries = 0x%X\n", lock->lock_tries);
	iprintf("	lock_fails = 0x%X\n", lock->lock_fails);
	iprintf("	lock_sleeps = 0x%X\n", lock->lock_sleeps);
	iprintf("	lock_wait_min = 0x%X\n", lock->lock_wait_min);
	iprintf("	lock_wait_max = 0x%X\n", lock->lock_wait_max);
	iprintf("	lock_wait_sum = 0x%X\n", lock->lock_wait_sum);
	iprintf("	lock_max_read = 0x%X\n", lock->lock_max_read);
	iprintf("	lock_nreads = 0x%X\n", lock->lock_nreads);
#endif
}

#if	MACH_SLOCKS
void
print_simple_lock(slock)
	struct slock *slock;
{
	iprintf("Simple lock: 0x%X\n", slock);
	iprintf("	lock_data = 0x%X\n", slock->lock_data);
#if	MACH_LDEBUG
	iprintf("	slthread = 0x%X\n", slock->slthread);
	iprintf("	slck_addr = 0x%X\n", slock->slck_addr);
	iprintf("	sunlck_addr = 0x%X\n", slock->sunlck_addr);
#endif
#if	SLOCK_STATS && notyet
	iprintf("	slock_tries = 0x%X\n", slock->slock_tries);
	iprintf("	slock_fails = 0x%X\n", slock->slock_fails);
	iprintf("	slock_min_time = 0x%X\n", slock->slock_min_time);
	iprintf("	slock_max_time = 0x%X\n", slock->slock_max_time);
	iprintf("	slock_avg_time = 0x%X\n", slock->slock_avg_time);
#endif
}
#endif	/* MACH_SLOCKS */

void
cred_print(cr)
	register struct ucred *cr;
{
	int i;
	if (!cr) {
		printf("NULL CREDS\n");
		return;
	}
	iprintf("cr_ref = %d, cr_uid = %d, cr_gid = %d\n", 
		cr->cr_ref, cr->cr_uid, cr->cr_gid);
	iprintf("groups:\n");
	indent += 2;
	iprintf("");
	for (i=0; i < NGROUPS; i++)
		printf("%d ", cr->cr_groups[i]);
	printf("\n");
	indent -= 2;
#if	MACH_SLOCKS
	iprintf("cr_lock:\n");
	indent += 2;
	print_simple_lock(&cr->cr_lock);
	indent -= 2;
#endif
}

void
proc_print_vfs(th)
	thread_t th;
{
	proc_print(th->u_address.utask->uu_procp);
}

void
utask_print(th)
	thread_t th;
{
	register struct utask *ut = th->u_address.utask;
	/*
	 * print only "interesting" fields
	 */
	iprintf("Utask 0x%X:\n", ut);
	indent += 5;
        iprintf("uu_procp = 0x%X, uu_logname = %s uu_tsize = 0x%X\n", 
		ut->uu_procp, ut->uu_logname, ut->uu_tsize);
        iprintf("uu_dsize = 0x%X, uu_ssize = 0x%X, uu_text_start = 0x%X\n", 
		ut->uu_dsize, ut->uu_ssize, ut->uu_text_start);
        iprintf("uu_stack_start = 0x%X, uu_stack_end = 0x%X, uu_sigonstack = 0x%X\n", 
		ut->uu_stack_start, ut->uu_stack_end, ut->uu_sigonstack);
        iprintf("uu_sigintr = 0x%X, uu_oldmask = 0x%X, uu_lastfile = 0x%X\n", 
		ut->uu_sigintr, ut->uu_oldmask, ut->uu_file_state.uf_lastfile);
        iprintf("uu_cdir = 0x%X, uu_rdir = 0x%X, uu_cmask = 0x%X\n", 
		ut->uu_cdir, ut->uu_rdir, ut->uu_cmask);
        iprintf("uu_maxuprc = 0x%X\n", ut->uu_maxuprc);
#if	MACH_SLOCKS
	iprintf("Handy lock:\n");
	indent += 2;
	print_simple_lock(&ut->uu_handy_lock);
	indent -= 2;
#endif
	indent -= 5;
}

void
nameidata_print(ndp)
	register struct nameidata *ndp;
{
    iprintf("ni_cdir = 0x%X, ni_rdir = 0x%X, ni_pnbuf = 0x%X, pathname = %s\n",
	ndp->ni_cdir, ndp->ni_rdir, ndp->ni_pnbuf,
	(ndp->ni_pnbuf ? ndp->ni_pnbuf : "XXX"));
    iprintf("ni_ptr = 0x%X, ni_next = 0x%X, ni_pathlen = 0x%X, ni_namelen = 0x%X\n",
	ndp->ni_ptr, ndp->ni_next, ndp->ni_pathlen, ndp->ni_namelen);
    iprintf("ni_vp = 0x%X, ni_dvp = 0x%X\n",
	ndp->ni_vp, ndp->ni_dvp); 
    /* print struct dirent */
    indent += 2;
    iprintf("ni_dent:\n");
    iprintf("d_fileno = %D, d_reclen = %x, d_namlen = %d, d_name = %s\n",
	    ndp->ni_dent.d_fileno, ndp->ni_dent.d_reclen, 
	    ndp->ni_dent.d_namlen, ndp->ni_dent.d_name);
    indent -= 2;
    iprintf("ni_base = 0x%X, ni_count = 0x%X, ni_offset = 0x%X, ni_resid = 0x%X\n",
	ndp->ni_base, ndp->ni_count, ndp->ni_offset, ndp->ni_resid);
    iprintf("Credentials (0x%X):\n", ndp->ni_cred);
    indent += 2;
    if ((ndp->ni_cred != NOCRED) && (ndp->ni_cred != 0))
    	cred_print(ndp->ni_cred);
    indent -= 2;
}

void
uthread_print(th)
	register thread_t th;
{
	register struct uthread *uth = th->u_address.uthread;
	iprintf("Thread 0x%X:\n", th);
	indent += 2;
	iprintf("task = 0x%X, ref_count = %d, kernel_stack = 0x%X\n",
		th->task, th->ref_count, th->kernel_stack);
	iprintf("wait_event = 0x%X, interruptible = %d, wait_result = %d\n",
		th->wait_event, th->interruptible, th->wait_result);
	iprintf("timer_set = %d, swap_state = %d, state = 0x%X\n",
		th->timer_set, th->swap_state, th->state);
	iprintf("wait_mesg = %s, priority = %d, event = %d\n",
		(th->wait_mesg?th->wait_mesg:"n/a"), th->priority, 
		th->select_event.ev_event);
	indent -= 2;
	iprintf("Uthread 0x%X:\n", uth);
	indent += 2;
	iprintf("uu_ar0 = 0x%X\n",uth->uu_ar0);
#if	SEC_BASE
	iprintf("uu_error = 0x%x\n", uth->uu_error);
#endif
#if	MACH_ASSERT 
#if	PMAX
	iprintf("Last system call: %s\n", (uth->uu_spare[2] ? 
				(char *)uth->uu_spare[2] : "n/a"));
	iprintf("Current system call: %s\n", (uth->uu_spare[1] ? 
				(char *)uth->uu_spare[1] : "n/a"));
#endif
#endif
	iprintf("Nameidata structure at 0x%X:\n", &uth->uu_nd);
	indent += 2;
	nameidata_print(&uth->uu_nd);
	indent -= 2;
	iprintf("uu_code = 0x%X, uu_cursig = 0x%X, uu_sig = 0x%X\n",
		uth->uu_code, uth->uu_cursig, uth->uu_sig);
	indent -= 2;
}

void
tty_print(tty)
	struct tty *tty;
{
	extern void pgrp_print();
	struct session *s = tty->t_session;

	printf("tty 0x%X:\n", tty);
        indent += 2;
        iprintf("t_rawq.c_cc = %D, t_canq.c_cc = %D, t_outq.c_cc = %D\n",
               tty->t_rawq.c_cc, tty->t_canq.c_cc, tty->t_outq.c_cc);
        iprintf("t_dev = %X t_flags = %X t_state = %X t_line = %D\n",
               tty->t_dev, tty->t_flags, tty->t_state, tty->t_line);
        iprintf("t_col = %D t_rocount = %D t_rocol = %D\n",
               tty->t_col, tty->t_rocount, tty->t_rocol);
        iprintf("t_hiwat = %D t_lowat = %D t_shad_time = %D\n",
               tty->t_hiwat, tty->t_lowat, tty->t_shad_time);
        iprintf("t_min = %D t_time = %D t_ispeed = %D t_ospeed = %D\n",
               tty->t_min, tty->t_time, tty->t_ispeed, tty->t_ospeed);
        iprintf("c_iflag = %X c_oflag = %X c_cflag = %X c_lflag = %X\n",
               tty->t_iflag, tty->t_oflag, tty->t_cflag, tty->t_lflag);
        iprintf("&rawq = %X &canq = %X &outq = %X\n",
               &tty->t_rawq, &tty->t_canq, &tty->t_outq);
        iprintf("&rawq.c_cf = %X &canq.c_cf = %X &outq.c_cf = %X\n",
               &tty->t_rawq.c_cf, &tty->t_canq.c_cf, &tty->t_outq.c_cf);
	indent -= 2;
	if (tty->t_pgrp) {
                iprintf("struct pgrp:\n");
		indent += 2;
		pgrp_print(tty->t_pgrp);
                indent -= 2;
        }
	else
		printf("NULL t_pgrp\n");
        
	if (s) {
		struct session * ls;
		
		ls = s;
		iprintf("t_session (0x%X):\n", s);
		indent += 2;
		iprintf("s_count: %D, s_leader: 0x%X, s_ttyvp: 0x%X\n",
			s->s_count, s->s_leader, s->s_ttyvp); 
		indent -= 2;
	} else
		iprintf("NULL t_session\n");
}

/*
 * help message
 */
fhelp_print()
{
	printf("$G (file system) options:\n");
	/*
	 * upper layer structures
	 */
	printf("	j:	check free vnodes for bad ptrs\n");
	printf("	m:	print struct mount (struct mount *)\n");
	printf("	v:	print struct vnode (struct vnode *)\n");
	printf("	V:	print vnodes of mnt point (struct mount *)\n");
	printf("	F:	print struct file (struct file *)\n");

	/*
	 * Buffer cache-related
	 */
	printf("\n");
	printf("	b:	print struct buf (struct buf *)\n");
	printf("	B:	print bvnode list of struct buf (struct buf *)\n");
	printf("	c:	print vnode clean buflist (struct vnode *)\n");
	printf("	C:	print vnode hash chain (struct vnode *)\n");
	printf("	d:	print vnode dirty buflist (struct vnode *)\n");
	printf("	e:	print bfreelist (struct buf *)\n");
	printf("	E:	print bhash chain (struct buf *)\n");
	printf("	g:	print list of all dirty UFS inodes\n");
	printf("	G:	print list of all dirty UFS vnodes (data)\n");

	/*
	 * UFS-related
	 */
	printf("\n");
	printf("	f:	print struct fs (struct fs *)\n");
	printf("	u:	print struct ufsmount (struct ufsmount *)\n");
	printf("	i:	print struct inode (struct inode *)\n");
	printf("	o:	print ihash chain (struct inode *)\n");

	/*
	 * NFS-related
	 */
	printf("\n");
	printf("	S:	print struct nfsmount (struct nfsmount *)\n");
	printf("	n:	print struct nfsnode (struct nfsnode *)\n");
	printf("	h:	print struct nfshost (struct nfshost *)\n");
	printf("	H:	print struct nfshost list (struct nfshost *)\n");
	printf("	N:	print nfshash chain (struct nfsnode *)\n");
	printf("	r:	print struct nfsreq (struct nfsreq *)\n");
	printf("	R:	print nfsreq list (struct nfsreq *)\n");

	/*
	 * lock related and misc.
	 */
	printf("\n");
	printf("	w:	print struct mbuf (struct mbuf *)\n");
	printf("	W:	print mbuf chain (struct mbuf *)\n");
	printf("\n");
	printf("	l:	print struct lock (struct lock *)\n");
#if	MACH_SLOCKS
	printf("	s:	print simple lock (struct slock *)\n");
#endif
	printf("	p:	print proc structure (struct proc *)\n");
	printf("	P:	print current proc (struct proc *)\n");
	printf("	a:	print utask structure (struct utask *)\n");
	printf("	A:	print current utask (struct utask *)\n");
	printf("	I:	print credential struct (struct ucred *)\n");
	printf("	t:	print uthread struct (struct uthread *)\n");
	printf("	T:	print current uthread (struct uthread *)\n");
	printf("	x:	print nameidata (struct nameidata *)\n");
	printf("	z:	print struct tty (struct tty *)\n");
	printf("	?:	print this list\n");
}


/*
 * main file system print switch
 */

void
vfs_print(ch, addr)
	int ch;
	long addr;
{
	switch (ch) {
		case 'b':
			(void)buffer_print((struct buf *)addr);
			break;
		case 'B':
			(void)bvnode_list_print((struct buf *)addr);
			break;
		case 'c':
			(void)vnode_clnlist_print((struct vnode *)addr);
			break;
		case 'C':
			(void)vnode_alias_print((struct vnode *)addr);
			break;
		case 'd':
			(void)vnode_drtylist_print((struct vnode *)addr);
			break;
		case 'e':
			(void)bfreelist_print((struct buf *)addr);
			break;
		case 'E':
			(void)bhash_chain_print((struct buf *)addr);
			break;
		case 'f':
			(void)fs_print((struct fs *)addr);
			break;
		case 'F':
			(void)file_print((struct file *)addr);
			break;
		case 'g':
			(void)dirtyino_print();
			break;
		case 'G':
			(void)dirtybuf_print();
			break;
		case 'h':
			(void)nfshost_print((struct nfshost *)addr);
			break;
		case 'H':
			(void)nfshost_list_print((struct nfshost *)addr);
			break;
		case 'i':
			(void)inode_print((struct inode *)addr);
			break;
		case 'I':
			(void)cred_print((struct ucred *)addr);
			break;
		case 'j':
			(void)vnode_check();
			break;
		case 'm':
			(void)mount_print((struct mount *)addr);
			break;
		case 'n':
			(void)nfsnode_print((struct nfsnode *)addr);
			break;
		case 'N':
			(void)nfshash_chain_print((struct nfsnode *)addr);
			break;
		case 'o':
			(void)ihash_chain_print((struct inode *)addr);
			break;
		case 'r':
			(void)nfsreq_print((struct nfsreq *)addr);
			break;
		case 'R':
			(void)nfsreq_list_print((struct nfsreq *)addr);
			break;
		case 'S':
			(void)nfsmount_print((struct nfsmount *)addr);
			break;
		case 'u':
			(void)ufsmount_print((struct ufsmount *)addr);
			break;
		case 'v':
			(void)vnode_print((struct vnode *)addr);
			break;
		case 'V':
			(void)mnt_vnode_print((struct mount *)addr);
			break;
		case 'w':
			(void)mbuf_print((struct mbuf *)addr);
			break;
		case 'W':
			(void)mbuf_print_list((struct mbuf *)addr);
			break;
		case 'l':
			(void)print_rw_lock((struct lock *)addr);
			break;
#if	MACH_SLOCKS
		case 's':
			(void)print_simple_lock((struct slock *)addr);
			break;
#endif
		case 'p':
			(void)proc_print_vfs((struct thread *)addr);
			break;
		case 'P':
			(void)proc_print_vfs(current_thread());
			break;
		case 'a':
			(void)utask_print((struct thread *)addr);
			break;
		case 'A':
			(void)utask_print(current_thread());
			break;
		case 't':
			(void)uthread_print((struct thread *)addr);
			break;
		case 'T':
			(void)uthread_print(current_thread());
			break;
		case 'x':
			(void)nameidata_print((struct nameidata *)addr);
			break;
		case 'z':
			(void)tty_print((struct tty *)addr);
			break;
		case '?':
		default:
			fhelp_print();
			break;
	}
}
#endif	/* MACH_KDB */
