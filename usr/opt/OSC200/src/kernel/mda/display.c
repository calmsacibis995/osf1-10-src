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
static char	*sccsid = "@(#)$RCSfile: display.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:36:41 $";
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
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#include <sys/unix_defs.h>
#include <mmax_mp.h>
#include <mmax_mpdebug.h>
#include <kern/lock.h>

#include <sys/param.h>
#include <sys/file.h>
#include <kern/task.h>
#include <kern/mfs.h>
#if	BSD44
#include <sys/kernel.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <nfs/nfsnode.h>
#else	BSD44
#include <sys/inode.h>
#endif	BSD44
#include <sys/user.h>
#include <sys/proc.h>
#include <mmax/pcb.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <mmax/boot.h>
#if	BSD44
#include <ufs/fs.h>
#include <ufs/ufsmount.h>
#include <nfs/nfsmount.h>
#include <nfs/nfsiom.h>
#include <nfs/nfsv2.h>
#include <nfs/nfs.h>
#else	BSD44
#include <sys/fs.h>
#endif	BSD44
#include "mda.h"
#include <mmax_apc.h>
#include <pty.h>
#include <cpus.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <vm/vm_map.h>
#include <vm/vm_object.h>
#include <sys/callout.h>
#include <sys/msgbuf.h>
#include <kern/zalloc.h>
#include <mmax/panic.h>
#include <mmax/psl.h>
#include <mmaxio/io.h>
#include <kern/processor.h>
#include <stdio.h>
#include <kern/thread.h>


extern vm_map_entry_t ve0;
extern struct thread *th0;
extern struct callout *callout0;
struct processor_set *pset0 = (struct processor_set *) 0;
struct crq *crq0 = (struct crq *) 0;

thread_t active_threads[1];	/* To keep ld happy ! 	 */

struct proc *getproc();
extern int display_links();
extern int display_thread_brief();
extern int display_vm_map_entry_brief();
extern int display_callout_brief();
extern int display_crq_msg_brief();
extern int display_pset_threads();

/*     extern int	display_buffer();     */

extern char Canttranslate[];
boolean_t thread_on_master;
char   *truefalse();

extern char *pt_ttyadr;

 /* adr of active thread table in kernel */
struct thread *active_th_table = (struct thread *) (-1);

display_active_threads()
{
	int     cpu, result;
	struct thread *vth, *pth;

	if (active_th_table == (struct thread *) - 1) {
		result = get_address("_active_threads", &active_th_table);
		if (result != SUCCESS) {
			printf("mda: Could not get address of 'active_threads'\n");
			return(FAILED);
		}
	}
	printf("cpu      thread       task   state             pcb   command  wait_event\n");
	thread_on_master = FALSE;
	for (cpu = 0; cpu < NCPUS; cpu++) {
		vth = (struct thread *) (MAP((int) (active_th_table) + 4 * cpu));
		if (vth != (struct thread *) 0) {
			result = phys(vth, &pth, ptb0);
			if (result != SUCCESS) {
				printf(Canttranslate, vth);
				return(FAILED);
			}
			printf("%3d: ", cpu);
			display_thread_brief(vth, pth);
		}
	}

	if (thread_on_master)
		printf("* ==> on master runq\n");

}



display_address_space(mapv)
vm_map_t mapv;
{

	int     result, *offset;
	struct vm_map *mapp;
	struct vm_map_entry *hdr;

	offset = &(ve0->vme_next);
	printf("                                                       cur  max\n");
	printf("                start        end     object     offset prot prot\n");
	display_links(mapv->links.next, offset, offset, display_vm_map_entry_brief);
}



display_all_threads()
{
	int     result, *offset;
	struct processor_set *pset;

	if (pt_ttyadr == (char *) -1) {
		result = get_address("_pt_tty", &pt_ttyadr);
		if (result != SUCCESS) {
			printf("mda: Could not get address of 'pt_tty'\n");
			return(FAILED);
		}
	}
	offset = &(pset0->all_psets);
	result = get_address("_all_psets", &pset);
	if (result != SUCCESS) {
		printf("mda: Could not get address of 'all_psets'\n");
		return(FAILED);
	}
	printf("    thread       task   state             pcb   command  wait_event\n");
	thread_on_master = FALSE;
	display_links(pset, 0, offset, display_pset_threads);
	if (thread_on_master)
		printf("* ==> on master runq\n");

}



display_callouts()
{
	struct callout *calltodo;
	int     result, offset, hdroffset;

	result = get_address("_calltodo", &calltodo);
	if (result != SUCCESS) {
		printf("mda: Could not get address of 'calltodo'\n");
		return(FAILED);
	}
	offset = hdroffset = (int) (&callout0->c_next);
	printf("                time   parameter   function\n");
	display_list(calltodo, hdroffset, offset, display_callout_brief);
}



display_crq_msgs(vcrq, pcrq)
crq_t  *vcrq, *pcrq;
{
	int     result;
	unsigned int hoffset;
	char   *hdr;

	result = phys(vcrq, &pcrq, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	hoffset = (unsigned int) &crq0->crq_cmd;
	hdr = (char *) pcrq + hoffset;
	if ((char *) MAP(hdr) == hdr)
		printf("No Commands queued\n");
	else {
		printf("Commands:\n");
		printf("            Code  Unit   Refnum   Status\n");
		display_links(hdr, 0, 0, display_crq_msg_brief);
	}
	hoffset = (unsigned int) &crq0->crq_rsp;
	hdr = (char *) pcrq + hoffset;
	if ((char *) MAP(hdr) == hdr)
		printf("No Responses queued\n");
	else {
		printf("Responses:\n");
		printf("            Code  Unit   Refnum   Status\n");
		display_links(hdr, 0, 0, display_crq_msg_brief);
	}
	hoffset = (unsigned int) &crq0->crq_immedcmd;
	hdr = (char *) pcrq + hoffset;
	if ((char *) MAP(hdr) == hdr)
		printf("No Immediate Commands queued\n");
	else {
		printf("Immediate Commands:\n");
		printf("            Code  Unit   Refnum   Status\n");
		display_links(hdr, 0, 0, display_crq_msg_brief);
	}
	hoffset = (unsigned int) &crq0->crq_attn;
	hdr = (char *) pcrq + hoffset;
	if ((char *) MAP(hdr) == hdr)
		printf("No Attentions queued\n");
	else {
		printf("Attentions:\n");
		printf("            Code  Unit   Refnum   Status\n");
		display_links(hdr, 0, 0, display_crq_msg_brief);
	}
	hoffset = (unsigned int) &crq0->crq_free;
	hdr = (char *) pcrq + hoffset;
	if ((char *) MAP(hdr) == hdr)
		printf("No Free Command Blocks queued\n");
	else {
		printf("Free Command Blocks:\n");
		printf("            Code  Unit   Refnum   Status\n");
		display_links(hdr, 0, 0, display_crq_msg_brief);
	}
}



/* removed, no struct hash_entry
 */
#ifdef notdef
display_hash_entry(ve, pe)
struct hash_entry *ve;
struct hash_entry *pe;
{
	int     result;
	char   *comp_name;

	result = phys(pe->name, &comp_name, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	comp_name = (char *) (MAPPED(comp_name));
	printf("\tHash Entry \"%.*s\" @ %#x:\n", pe->name_length, comp_name, ve);
	printthree("xx ", "reclen", "prev_size", "",
		   pe->reclen, pe->prev_size);
	printthree("xx ", "ino", "dir_offset", "",
		   pe->ino, pe->dir_offset);
}
#endif /* notdef */


/* removed, no struct hash_entry
 */
#ifdef notdef
display_hash_chain(ve)
struct hash_entry *ve;
{
	struct hash_entry *pe;
	int     result, ntabs;

	result = phys(ve, &pe, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	pe = (struct hash_entry *) (MAPPED(pe));

	display_hash_entry(ve, pe);

	while (pe->next != NULL) {
		if (ntabs == 0) {
			ntabs++;
		}
		printf("\n\tHash Entry @ %#x\n", pe->next);
		ve = pe->next;
		result = phys(pe->next, &pe, ptb0);
		if (result != SUCCESS)
			return(FAILED);
		pe = (struct hash_entry *) (MAPPED(pe));
		display_hash_entry(ve, pe);
	}
}
#endif /* notdef */



/* removed, no struct hash_entry
 */
#ifdef notdef
display_dir_hash_entry(ve, pe, ntabs)
struct dir_hash_entry *ve;
struct dir_hash_entry *pe;
int     ntabs;
{
	int     result, dev, i;
	struct hash_entry **htable;

	printthree("xxx", "last_unused", "ref_count", "last_off",
		   pe->last_unused, pe->ref_count, pe->last_off);
	dev = pe->dev && 0xffff;
	printthree("xxx", "dev", "ino", "hash_table",
		   dev, pe->ino, pe->hash_table);

	ve = pe->hash_table;
	result = phys(ve, &pe, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	htable = (struct hash_entry **) (MAPPED(pe));
	for (i = 0; i < HASHSIZE; i++) {
		if (htable[i] != NULL) {
			printf("\n\tHash_table %#x[%d]:\n", ve, i);
			display_hash_chain(htable[i]);
		}
	}

	printf("\n");
}
#endif /* notdef */



/* removed, no struct hash_entry
 */
#ifdef notdef
display_dir_hash_chain(ve)
struct dir_hash_entry *ve;
{
	int     ntabs = 0;
	int     result, dev;
	struct dir_hash_entry *pe;

	result = phys(ve, &pe, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	pe = (struct dir_hash_entry *) (MAPPED(pe));

	display_dir_hash_entry(ve, pe, ntabs);

	while (pe->next != NULL) {
		if (ntabs == 0) {
			ntabs++;
		}
		printf("%.*s", ntabs * 8, "---------------------------------------------");
		printf("Dir Hash Entry @ %#x\n", pe->next);
		ve = pe->next;
		result = phys(pe->next, &pe, ptb0);
		if (result != SUCCESS)
			return(FAILED);
		pe = (struct dir_hash_entry *) (MAPPED(pe));
		display_dir_hash_entry(ve, pe, ntabs);
	}
}
#endif /* notdef */



/* removed, no struct hash_entry
 */
#ifdef notdef
display_dir_hash_table()
{
	int     result, i, next_dir_index;
	struct dir_hash_entry **dir_hash_tab = (struct dir_hash_entry **) - 1;
	struct dir_hash_entry *nextp;

	if (dir_hash_tab == (struct dir_hash_entry **) - 1) {
		result = get_address("_dir_hash_table", &dir_hash_tab);
		if (result != SUCCESS)
			return(FAILED);
		result = phys(dir_hash_tab, &dir_hash_tab, ptb0);
		if (result != SUCCESS)
			return(FAILED);
		dir_hash_tab = (struct dir_hash_entry **) (MAP(dir_hash_tab));
		result = phys(dir_hash_tab, &dir_hash_tab, ptb0);
		if (result != SUCCESS)
			return(FAILED);
		dir_hash_tab = (struct dir_hash_entry **) (MAPPED(dir_hash_tab));

		result = get_address("_next_directory_index", &next_dir_index);
		if (result != SUCCESS)
			return(FAILED);
		result = phys(next_dir_index, &next_dir_index, ptb0);
		if (result != SUCCESS)
			return(FAILED);
		next_dir_index = MAP(next_dir_index);
	}
	printf("\nNext Directory Index: %d\n", next_dir_index);
	for (i = 0; i < DIRHASHSIZE; i++) {
		if (dir_hash_tab[i] != NULL) {
			printf("\n******************************************");
			printf("Dir_hash_table[%d] at %#x:\n", i, dir_hash_tab[i]);
			display_dir_hash_chain(dir_hash_tab[i]);
		}
	}
}
#endif /* notdef */



display_file(fp)
struct file *fp;
{
	printf("file structure at 0x%.8x\n", fp);

	printthree("xxx", "f_flag", "f_type", "f_count",
		   fp->f_flag, fp->f_type, fp->f_count);
	printthree("xxx", "f_msgcount", "f_cred", "f_fops",
		   fp->f_msgcount, fp->f_cred, fp->f_ops);
	printthree("xx ", "f_data", "f_offset", "",
		   fp->f_data, fp->f_offset, 0);
#if	MMAX_MP
	display_simple_lock(&fp->f_data_lock);
	printf("I/O Lock:\n");
	display_lock(&fp->f_io_lock);
	putchar('\n');
#endif	MMAX_MP
}



display_csum(csp)
struct csum *csp;
{
	printf(" ndir: %#10x nbfree: %#10x ",
	       csp->cs_ndir, csp->cs_nbfree);
	printf("nifree: %#10x nffree: %#10x\n",
	       csp->cs_nifree, csp->cs_nffree);
	return(SUCCESS);
}



display_fs(fsp)
struct fs *fsp;
{
	int     i, cspflag;

	printthree("xxx", "fs_link", "fs_rlink", "fs_sblkno",
		   fsp->fs_link, fsp->fs_rlink, fsp->fs_sblkno);
	printthree("xxx", "fs_cblkno", "fs_iblkno", "fs_dblkno",
		   fsp->fs_cblkno, fsp->fs_iblkno, fsp->fs_dblkno);
	printthree("xxx", "fs_cgoffset", "fs_cgmask", "fs_time",
		   fsp->fs_cgoffset, fsp->fs_cgmask, fsp->fs_time);
	printthree("xxx", "fs_size", "fs_dsize", "fs_ncg",
		   fsp->fs_size, fsp->fs_dsize, fsp->fs_ncg);
	printthree("xxx", "fs_bsize", "fs_fsize", "fs_frag",
		   fsp->fs_bsize, fsp->fs_fsize, fsp->fs_frag);
	printthree("xxx", "fs_minfree", "fs_rotdelay", "fs_rps",
		   fsp->fs_minfree, fsp->fs_rotdelay, fsp->fs_rps);
	printthree("xxx", "fs_bmask", "fs_fmask", "fs_bshift",
		   fsp->fs_bmask, fsp->fs_fmask, fsp->fs_bshift);
	printthree("xxx", "fs_maxcontig", "fs_maxbpg", "fs_fragshift",
		   fsp->fs_maxcontig, fsp->fs_maxbpg, fsp->fs_fragshift);
	printthree("xxx", "fs_fsbtodb", "fs_sbsize", "fs_csmask",
		   fsp->fs_fsbtodb, fsp->fs_sbsize, fsp->fs_csmask);
	printthree("xxx", "fs_csshift", "fs_nindir", "fs_inopb",
		   fsp->fs_csshift, fsp->fs_nindir, fsp->fs_inopb);
	printf("        fs_nspf:%#10x       fs_optim:%#10x\n",
	       fsp->fs_nspf, fsp->fs_optim);
	printthree("xxx", "fs_npsect", "fs_interleave", "fs_trackskew",
		   fsp->fs_npsect, fsp->fs_interleave, fsp->fs_trackskew);
	printthree("xx ", "fs_headswitch", "fs_trkseek", "",
		   fsp->fs_headswitch, fsp->fs_trkseek, 0);
	printthree("xxx", "fs_csaddr", "fs_cssize", "fs_cgsize",
		   fsp->fs_csaddr, fsp->fs_cssize, fsp->fs_cgsize);
	printthree("xxx", "fs_ntrak", "fs_nsect", "fs_spc",
		   fsp->fs_ntrak, fsp->fs_nsect, fsp->fs_spc);
	printthree("xxx", "fs_ncyl", "fs_cpg", "fs_ipg",
		   fsp->fs_ncyl, fsp->fs_cpg, fsp->fs_ipg);
	printthree("xxx", "fs_fpg", "fs_fmod", "fs_clean",
		   fsp->fs_fpg, fsp->fs_fmod, fsp->fs_clean);
	printthree("xxx", "fs_ronly", "fs_flags", "fs_cgrotor",
		   fsp->fs_ronly, fsp->fs_flags, fsp->fs_cgrotor);
	printf("         fs_cpc:%#10x       fs_magic:%#10x\n",
	       fsp->fs_cpc, fsp->fs_magic);
	printf("       fs_fsmnt: %s\n", fsp->fs_fsmnt);
	printf("fs_cstotal:\n  ");
	display_csum(&fsp->fs_cstotal);
	cspflag = FALSE;
	for (i = 0; i < MAXCSBUFS; i++) {
		if (fsp->fs_csp[i]) {
			if (cspflag == FALSE) {
				cspflag = TRUE;
				printf("cs_csp:\n");
			}
			printf("%#2d:", i);
			display_csum(fsp->fs_csp[i]);
		}
	}
}




/*
**  Which = 0	- From start of inode
**          1	- From data-lock
**          2	- From io-lock
**          3	- From event
*/
display_inode(vip, ip, which)
unsigned int vip;
struct inode *ip;
int     which;
{
	int     i;
	struct inode *fip;

#if	MMAX_MP
	if (which) {
		fip = (struct inode *) 0;
		switch (which) {
			case 0:
				break;	/* oops */
			case 1:
				i = (int) &fip->i_data_lock;
				break;
			case 2:
				i = (int) &fip->i_io_lock;
				break;
			case 3:
				i = (int) &fip->i_iodone;
				break;
		}

		printf("Inode at %#x:\n", vip - i);
		ip = (struct inode *) ((char *) ip - (char *) i);
	}
#endif	MMAX_MP
	printthree("xxx", "i_chain[0]", "i_chain[1]", "i_vnode",
		   ip->i_chain[0], ip->i_chain[1], ip->i_vnode);
	printthree("xxx", "i_devvp", "i_flag", "i_dev",
		   ip->i_devvp, ip->i_flag, ip->i_dev);
	printthree("xx", "i_number", "i_fs", "",
		   ip->i_number, ip->i_fs, 0);
#if	!MMAX_MP
	printthree("x  ", "i_dquot", "", "",
		   ip->i_dquot, 0, 0);
#endif	!MMAX_MP

	printthree("xx ", "i_diroff", "i_endoff", "",
		   ip->i_diroff, ip->i_endoff);
	printthree("oxd", "di_mode", "di_nlink", "di_uid",
		   ip->i_mode, ip->i_nlink, ip->i_uid);
	printthree("dxx", "di_gid", "di_size", "di_atime",
		   ip->i_gid, ip->i_size, ip->i_atime);
	printthree("xx ", "di_mtime", "di_ctime", "",
		   ip->i_mtime, ip->i_ctime, 0);
	if (ip->i_flags & IC_FASTLINK)
		printf("di_Msymlink: %s\n", ip->i_symlink);
	else {
		printf("Disk block addresses:");
		for (i = 0; i < NDADDR; i++) {
			if ((i % 6) == 0)
				printf("\n");
			printf("\t%#10x", ip->i_db[i]);
		}

		printf("\nIndirect Block Addresses");
		for (i = 0; i < NIADDR; i++) {
			if ((i % 6) == 0)
				putchar('\n');
			printf("\t%#10x", ip->i_ib[i]);
		}
		putchar('\n');
		putchar('\n');
	}

	printthree("xxx", "flags", "blocks", "gen",
		   ip->i_flags, ip->i_blocks, ip->i_gen);

#if	MMAX_MP
	printf("I/O Lock:\n");
	display_lock(&ip->i_io_lock);
	putchar('\n');
	display_simple_lock(&ip->i_data_lock);
	printthree("xx ", "event/lock", "event/bool", "",
		   ip->i_iodone.ev_slock, ip->i_iodone.ev_event, 0);
#endif	MMAX_MP
}

display_ihash_chain(vip)
struct inode *vip;
{
	struct inode *ip = vip;

	do {
		if(phys(ip, &ip, ptb0) != SUCCESS)
			break;

		ip = (struct inode *)MAPPED(ip);
		printthree("xxx", "i_flag", "i_forw", "i_back",
			ip->i_flag, ip->i_forw, ip->i_back);
		ip = ip->i_forw;
	} while (ip != vip);
}


display_vnode(vp, vvp)
struct vnode *vp, *vvp;
{
	printthree("xxx", "v_flag", "v_usecount", "v_holdcnt",
		   vp->v_flag, vp->v_usecount, vp->v_holdcnt);
	printthree("xxx", "v_shlockc", "v_exlockc", "v_lastr",
		   vp->v_shlockc, vp->v_exlockc, vp->v_lastr);
	printthree("xxx", "v_id", "v_mount", "v_op",
		   vp->v_id, vp->v_mount, vp->v_op);
	printthree("xxx", "v_freef", "v_freeb", "v_mountf",
		   vp->v_freef, vp->v_freeb, vp->v_mountf);
	printthree("xxx", "v_mountb", "v_cleanblkhd", "v_dirtyblkhd",
		   vp->v_mountb, vp->v_cleanblkhd, vp->v_dirtyblkhd);
	printthree("xxx", "v_numoutput", "v_type", "v_un",
		   vp->v_numoutput, vp->v_type, vp->v_mountedhere);
	switch(vp->v_type) {
	case VDIR:
		printthree("x  ", "v_mountedhere", "", "",
			   vp->v_mountedhere, 0, 0);
		break;
	case VSOCK:
		printthree("x  ", "v_socket", "", "",
			   vp->v_socket, 0, 0);
		break;
	case VREG:
		printthree("x  ", "v_text", "", "",
			   vp->v_text, 0, 0);
		break;
	case VCHR:
	case VBLK:
		printthree("xxx", "v_specinfo", "v_hashchain", "v_specnext",
			   vp->v_specinfo, vp->v_hashchain, vp->v_specnext);
		printthree("x  ", "v_rdev", "", "",
			   vp->v_rdev, 0, 0);
		break;
	}
	printthree("xxx", "v_tag", "v_vm_info", "v_data",
		   vp->v_tag, vp->v_vm_info, vvp->v_data);
}

display_vnode_hashchain(vp)
struct vnode *vp;
{
	struct vnode **vpp = vp->v_hashchain;

	if (vpp == (struct vnode *)0) {
		printf("vnode hash chain is empty\n");
		return;
	}
	if(phys(vpp, &vpp, ptb0) != SUCCESS)
		return;
	vpp = (struct vnode **)MAPPED(vpp);

	vp = *vpp;
	if (vp == (struct vnode *)0) {
		printf("vnode hash chain is empty\n");
		return;
	}
	if(phys(vp, &vp, ptb0) != SUCCESS)
		return;
	vp = (struct vnode *)MAPPED(vp);

	printthree("x  ", "*v_hashchain", "", "",
		*vp->v_hashchain, 0, 0);
	do {
		if(phys(vp, &vp, ptb0) != SUCCESS)
			break;

		vp = (struct vnode *)MAPPED(vp);
		printthree("xxx", "*v_hashchain", "v_specnext", "v_rdev",
			*vp->v_hashchain, vp->v_specnext, vp->v_rdev);
		vp = vp->v_specnext;
	} while (vp != (struct vnode *)0);
}


display_nfsnode(np)
struct nfsnode *np;
{
	printthree("xxx", "n_chain[0]", "n_chain[1]", "n_flag",
		   np->n_chain[0], np->n_chain[1], np->n_flag);
	printthree("xx ", "n_vnode", "n_attrstamp", "",
		   np->n_vnode, np->n_attrstamp, 0);
	printf("\tn_vattr:\n");
	display_vattr(&np->n_vattr);
	printthree("xxx", "n_sillyrename", "n_lastr", "n_size",
		   np->n_sillyrename, np->n_lastr, np->n_size);
	printthree("xx ", "n_mtime", "n_error", "",
		   np->n_mtime, np->n_error, 0);
}

display_vattr(vp)
struct vattr *vp;
{
	printthree("xox", "va_type", "va_mode", "va_nlink",
		   vp->va_type, vp->va_mode, vp->va_nlink);
	printthree("ddx", "va_uid", "va_gid", "va_fsid",
		   vp->va_uid, vp->va_gid, vp->va_fsid);
	printthree("xxx", "va_fileid", "va_size", "va_size1",
		   vp->va_fileid, vp->va_size, vp->va_size1);
	printthree("xxx", "va_blocksize", "va_gen", "va_flags",
		   vp->va_blocksize, vp->va_gen, vp->va_flags);
	printthree("xxx", "va_rdev", "va_bytes", "va_bytes1",
		   vp->va_rdev, vp->va_bytes, vp->va_bytes);
}


display_nfshash_chain(vnp)
struct nfsnode *vnp;
{
	struct nfsnode *np = vnp;

	do {
		if(phys(np, &np, ptb0) != SUCCESS)
			break;

		np = (struct nfsnode *)MAPPED(np);
		printthree("xxx", "n_flag", "n_forw", "n_back",
			np->n_flag, np->n_forw, np->n_back);
		np = np->n_forw;
	} while (np != vnp);
}


display_nfsreq(rep)
struct nfsreq *rep;
{
	printthree("xxx", "r_next", "r_prev", "r_mreq",
		   rep->r_next, rep->r_prev, rep->r_mreq);
	printthree("xxx", "r_mrep", "r_mntp", "r_vp",
		   rep->r_mrep, rep->r_mntp, rep->r_vp);
	printthree("xxx", "r_msiz", "r_xid", "r_flags",
		   rep->r_msiz, rep->r_xid, rep->r_flags);
	printthree("xxx", "r_retry", "r_rexmit", "r_timer",
		   rep->r_retry, rep->r_rexmit, rep->r_timer);
}

display_nfsreq_list(vrep)
struct nfsreq *vrep;
{
	struct nfsreq *rep = vrep;

	do {
		if(phys(rep, &rep, ptb0) != SUCCESS)
			break;

		rep = (struct nfsreq *)MAPPED(rep);
		printthree("xx ", "r_next", "n_prev", "",
			rep->r_next, rep->r_prev, 0);
		rep = rep->r_next;
	} while(rep != vrep && rep != (struct nfsreq *)0);
}


display_nfshost(nfshp)
struct nfshost *nfshp;
{
	printthree("xxx", "nh_next", "nh_prev", "nh_refcnt",
		   nfshp->nh_next, nfshp->nh_prev, nfshp->nh_refcnt);
	printthree("xxx", "nh_currto", "nh_currexmit", "nh_sent",
		   nfshp->nh_currto, nfshp->nh_currexmit, nfshp->nh_sent);
	printthree("xxx", "nh_window", "nh_winext", "nh_ssthresh",
		   nfshp->nh_window, nfshp->nh_winext, nfshp->nh_ssthresh);
	printthree("xxx", "nh_salen", "nh_sockaddr", "",
		   nfshp->nh_salen, nfshp->nh_sockaddr, 0);
}

display_nfshost_list(vnfshp)
struct nfshost *vnfshp;
{
	struct nfshost *nfshp = vnfshp;

	do {
		if(phys(nfshp, &nfshp, ptb0) != SUCCESS)
			break;

		nfshp = (struct nfshost *)MAPPED(nfshp);
		printthree("xx ", "nh_next", "nh_prev", "",
			nfshp->nh_next, nfshp->nh_prev, 0);
		nfshp = nfshp->nh_next;
	} while (nfshp != vnfshp && nfshp != (struct nfshost *)0);
}


display_mount(mp)
struct mount *mp;
{
	printthree("xxx", "m_next", "m_prev", "m_op",
		   mp->m_next, mp->m_prev, mp->m_op);
	printthree("xxx", "m_vnodecovered", "m_mounth", "m_flag",
		   mp->m_vnodecovered, mp->m_mounth, mp->m_flag);
	printthree("xx ", "m_exroot","m_data", "",
		   mp->m_exroot, (char *)(mp->m_data), 0);
}

#include <sys/mbuf.h>

display_mbuf(m)
struct mbuf *m;
{
	
	printthree("xxx", "m_next", "m_nextpkt", "m_len",
		   m->m_next, m->m_nextpkt, m->m_len);
	printthree("xxx", "m_data", "m_type", "m_flags",
		   m->m_data, m->m_type, m->m_flags);
	if (m->m_flags & M_PKTHDR) {
		printf("packet header:\n");
		printthree("xxx", "len", "rcvif", "",
			   m->m_pkthdr.len, m->m_pkthdr.rcvif, 0);
	}
	else if (m->m_flags & M_EXT) {
		printf("external storage:\n");
		printthree("xxx", "ext_buf", "ext_free", "ext_size",
			   m->m_ext.ext_buf, m->m_ext.ext_free, m->m_ext.ext_size);

	}
}


display_ufsmount(ump)
struct ufsmount *ump;
{
	printthree("xxx", "um_mountp", "um_dev", "um_devvp",
		   ump->um_mountp, ump->um_dev, ump->um_devvp);
	printthree("xx ", "um_fs", "um_qinod", "",
		   ump->um_fs, ump->um_qinod, 0);
}

display_nfsmount(nmp)
struct nfsmount *nmp;
{
	printthree("xxx", "nm_flag", "nm_mountp", "nm_so",
		   nmp->nm_flag, nmp->nm_mountp, nmp->nm_so);
	printthree("xxx", "nm_hostinfo", "nm_retry", "nm_rexmit",
		   nmp->nm_hostinfo, nmp->nm_retry, nmp->nm_rexmit);
	printthree("xxx", "nm_rtt", "nm_rto", "nm_srtt",
		   nmp->nm_rtt, nmp->nm_rto, nmp->nm_srtt);
	printthree("xxx", "nm_rttvar", "nm_rsize", "nm_wsize",
		   nmp->nm_rttvar, nmp->nm_rsize, nmp->nm_wsize);
}

display_mnt_vnodelist(mp)
struct mount *mp;
{

	struct vnode *vp = mp->m_mounth;

	if (vp == (struct vnode *)0) {
		printf("vnode list is empty\n");
		return;
	}
	printthree("x  ", "m_mounth", "", "",
		mp->m_mounth, 0, 0);
	do {
		if(phys(vp, &vp, ptb0) != SUCCESS)
			break;

		vp = (struct vp *)MAPPED(vp);
		printthree("xxx", "v_flag", "v_mountf", "v_mountb",
			vp->v_flag, vp->v_mountf, vp->v_mountb);
		vp = vp->v_mountf;
	} while (vp != (struct vnode *)0);
}

display_messages(pending_only)
boolean_t pending_only;
{
	int     result, l;
	struct msgbuf *msgbuf;

	result = get_address("_msgbuf", &msgbuf);
	if (result != SUCCESS)
		return(FAILED);
	result = phys(msgbuf, &msgbuf, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	msgbuf = (struct msgbuf *) (MAPPED(msgbuf));

	if (pending_only) {
		l = msgbuf->msg_bufx - msgbuf->msg_bufr;
		if (l > 0)
			printf("%.*s\n", l, &msgbuf->msg_bufc[msgbuf->msg_bufr]);
		else {
			l = MSG_BSIZE - msgbuf->msg_bufr;
			printf("%.*s", l, &msgbuf->msg_bufc[msgbuf->msg_bufr]);
			printf("%.*s\n", msgbuf->msg_bufx, msgbuf->msg_bufc);
		}
	} else
		printf("%.*s\n", (MSG_BSIZE), msgbuf->msg_bufc);
}



display_registers()
{
	psa_t  *psa;

	printf("Registers for cpu %d:\n", current_cpu);
	if (get_address("_psa", &psa) == FAILED) {
		printf("mda:  can't find address of Panic Save Area (psa)\n");
		return(FAILED);
	}
	psa += current_cpu;

	printf("r0: 0x%.8x ", MAP(&psa->psa_r0));
	printf("r1: 0x%.8x ", MAP(&psa->psa_r1));
	printf("r2: 0x%.8x ", MAP(&psa->psa_r2));
	printf("r3: 0x%.8x ", MAP(&psa->psa_r3));
	putchar('\n');
	printf("r4: 0x%.8x ", MAP(&psa->psa_r4));
	printf("r5: 0x%.8x ", MAP(&psa->psa_r5));
	printf("r6: 0x%.8x ", MAP(&psa->psa_r6));
	printf("r7: 0x%.8x ", MAP(&psa->psa_r7));
	putchar('\n');

	printf("fp: %#x   pc: %#x   psr: %#x\nssp: %#x   usp: %#x\n",
	       MAP(&psa->psa_fp), MAP(&psa->psa_pc), MAP(&psa->psa_psr),
	       MAP(&psa->psa_ssp), MAP(&psa->psa_usp));
}



char   *psr_bit_names[] = {
			   "Carry",
			   "Trace",
			   "Low",
			   "oVerflow",
			   "Undefined",
			   "Flag",
			   "Zero",
			   "Negative",
			   "User",
			   "Sp1",
			   "PendingTrap",
			   "IntsOn"
};

char   *cfg_bit_names[] = {
			   "ICU",
			   "FPU",
			   "MMU",
			   "Cust",
			   "1",	/* forced to 1 */
			   "1",	/* forced to 1 */
			   "1",	/* forced to 1 */
			   "1",	/* forced to 1 */
			   "DirectExc",
			   "DC",
			   "LockDC",
			   "IC",
			   "LockIC",
			   "FPFIFO"
};

#if	MMAX_XPC
char   *icureg_bit_names[] = {
			      "Powerfail",
			      "Sysnmi",
			      "HardNBI/NONFATAL",
			      "Destsel",
			      "BtagBit",
			      "BtagCache",
			      "RcvAddr",
			      "CachePar",
			      "BtagTag",
			      "SoftNBI",
			      "VbErr",
			      "TimerL",
			      "Normal",
			      "TimerH",
			      "VecBus",
			      "Bogus"
};

#endif	MMAX_XPC
#if	MMAX_APC
char   *icureg_bit_names[] = {
			      "Powerfail",
			      "Sysnmi",
			      "HardNBI0",
			      "HardNBI1",
			      "HardNBI2",
			      "HardNBI3",
			      "Destsel",
			      "Nonfatal",
			      "Slave",
			      "SoftNBI0",
			      "Normal",
			      "TimerM",
			      "VecBus",
			      "BogusM"
};

#endif	MMAX_APC

#define	PSRBITS		(sizeof(psr_bit_names)/(sizeof(char *)))
#define	CFGBITS		(sizeof(cfg_bit_names)/(sizeof(char *)))
#define	ICUBITS		(sizeof(icureg_bit_names)/(sizeof(char *)))
#define	PSAV(foo)	MAP(&psa->psa_/**/foo)

display_cpu_state()
{
	psa_t  *psa;
	int     cputype, i, k, previous, printed, anyprinted;
	int     isrv, ipnd, imsk, psr, cfg, fsr, vector;
	int     sym_size, value, punt;
	char   *vpc, *sym;
	struct itbl *itbl;

	if (get_address("_psa", &psa) == FAILED) {
		printf("mda:  can't find address of Panic Save Area (psa)\n");
		return(FAILED);
	}
	psa += current_cpu;

	cputype = PSAV(cputype);
	printf("Cpu state for cpu %d (psa size %d, cputype %d):\n",
	       current_cpu, PSAV(size), cputype);

	putchar('\n');
	printf("\t  r0: %#10x    r1: %#10x    r2: %#10x    r3: %#10x\n",
	       PSAV(r0), PSAV(r1), PSAV(r2), PSAV(r2));
	printf("\t  r4: %#10x    r5: %#10x    r6: %#10x    r7: %#10x\n",
	       PSAV(r4), PSAV(r5), PSAV(r6), PSAV(r7));
	switch (cputype) {	/* floating point */
		case PSA_DPC_TYPE:
		case PSA_APC_TYPE:
			/* someday */
			break;
		case PSA_XPC_TYPE:
			/* someday */
			break;
	}


	putchar('\n');
	printf("\t  fp: %#10x    pc: %#10x   ssp: %#10x   usp: %#10x\n",
	       PSAV(fp), PSAV(pc), PSAV(ssp), PSAV(usp));
	printf("\t mod: %#10x    sb: %#10x  intb: %#10x\n",
	       PSAV(psr) & 0xffff, PSAV(sb), PSAV(intbase));
	putchar('\n');

	psr = (PSAV(psr) & 0xffff0000) >> 16;
	printf("\t psr: %#10x ", psr);
	diagram(psr, psr_bit_names, PSRBITS);
	putchar('\n');
	cfg = PSAV(cfg);
	printf("\t cfg: %#10x ", cfg);
	diagram(cfg, cfg_bit_names, CFGBITS);
	putchar('\n');
	fsr = PSAV(fsr);
	printf("\t fsr: %#10x ", fsr);
	/* diagram(fsr, fsr_bit_names, FSRBITS); */
	putchar('\n');

	putchar('\n');
	printf("\tptb0: %#10x  ptb1: %#10x\n",
	       PSAV(ptb0), PSAV(ptb1));

	switch (cputype) {
		case PSA_DPC_TYPE:
			printf("\t msr: %#10x   eia: %#10x\n", PSAV(msr), PSAV(eia));
			printf("\tbpr0: %#10x  bpr1: %#10x   bdr: %#10x\n",
			       PSAV(bpr0), PSAV(bpr1), PSAV(bdr));
			break;
		case PSA_APC_TYPE:
			printf("\t few: %#10x   asr: %#10x  tear: %#10x\n",
			       PSAV(few), PSAV(asr), PSAV(tear));
			printf("\tbear: %#10x   bar: %#10x   bmr: %#10x   bdr: %#10x\n",
			       PSAV(bear), PSAV(bar), PSAV(bmr), PSAV(bdr));
			break;
		case PSA_XPC_TYPE:
			printf("\t mcr: %#10x   msr: %#10x  tear: %#10x\n",
			       PSAV(mcr), PSAV(msr), PSAV(tear));
			printf("\t bpc: %#10x   car: %#10x   dcr: %#10x   dsr: %#10x\n",
			       PSAV(bpc), PSAV(car), PSAV(dcr), PSAV(dsr));
			break;
	}

	switch (cputype) {
		case PSA_DPC_TYPE:
			break;	/* no ICU state */
		case PSA_APC_TYPE:
		case PSA_XPC_TYPE:
#if	MMAX_XPC || MMAX_APC
			putchar('\n');
			isrv = PSAV(icu_isrv) & 0xffff;
			ipnd = PSAV(icu_ipnd) & 0xffff;
			imsk = PSAV(icu_imsk) & 0xffff;
			printf("\tisrv: %#10x ", isrv);
			diagram(isrv, icureg_bit_names, ICUBITS);
			printf("\n\tipnd: %#10x ", ipnd);
			diagram(ipnd, icureg_bit_names, ICUBITS);
			printf("\n\timsk: %#10x ", imsk);
			diagram(imsk, icureg_bit_names, ICUBITS);
			putchar('\n');
			if (cputype == PSA_XPC_TYPE)
				break;
			printf("\t  (slave):  isrv: %#10x  ipnd: %#10x  imsk: %#10x\n",
			  PSAV(sicu_isrv) & 0xffff, PSAV(sicu_ipnd) & 0xffff,
			       PSAV(sicu_imsk) & 0xffff);
#endif	MMAX_XPC || MMAX_APC
	}

	putchar('\n');
	printf("\t csr: %#10x   err: %#10x  time: %#10x\n",
	       PSAV(cpu_reg_csr), PSAV(cpu_reg_err), PSAV(timestamp));

	printf("\n\tPending vectors:\n");
	for (i = punt = 0; i < PSA_VBMAX; ++i) {
		vector = MAP(&psa->psa_vbfifo[i]) & 0xff;
		if (vector == 0xff)
			continue;
		if (!punt) {
			if (get_address("_itbl", &itbl) != SUCCESS) {
				punt++;
				printf("\t\t[%02d] %#6x\n", i, vector);
				continue;
			}
			itbl += vector;
			vpc = (char *) MAP(&itbl->i_func);
			if (get_symbol(&sym, &sym_size, &value, vpc) != SUCCESS) {
				punt++;
				printf("\t\t[%02d] %#6x\n", i, vector);
			}
			printf("\t\t[%02d] %#6x <%.*s>\n", i, vector,
			       sym_size, sym);
		}
	}
	putchar('\n');
}


display_rusage(vrp)
struct rusage *vrp;
{
	int     result;
	struct rusage *rp;

	result = phys(vrp, &rp, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	rp = (struct rusage *) (MAPPED(rp));
	printf
	    ("     utime: 0x%.8x 0x%.8x   stime: 0x%.8x 0x%.8x\n",
	     rp->ru_utime.tv_sec, rp->ru_utime.tv_usec, rp->ru_stime.tv_sec,
	     rp->ru_stime.tv_usec);
	return(SUCCESS);
}



display_task(task)
task_t  task;
{
	int     i;

	printthree("xxx", "lock", "ref_count", "active",
		   task->lock, task->ref_count, task->active);

	printthree("xxx", "map", "pset_tasks", "suspend_count",
		   task->map, task->pset_tasks.next, task->suspend_count);

	printthree("sxs", "may_assign", "", "assign_active",
		   truefalse(task->may_assign),
		   task->pset_tasks.prev,
		   truefalse(task->assign_active));

	printthree("xxx", "thread_list", "thread_count", "processor_set",
	    task->thread_list.next, task->thread_count, task->processor_set);

	printthree("xxx", "", "u_address", "proc_index",
		   task->thread_list.prev, task->u_address, task->proc_index);

	printthree("xss", "user_stop_count", "kern_ipc_space", "kern_vm_space",
		   task->user_stop_count,
		   truefalse(task->kernel_ipc_space),
		   truefalse(task->kernel_vm_space));

	printthree("sdd", "total_user_time", "seconds", "microsec",
		   "",
		   task->total_user_time.seconds,
		   task->total_user_time.microseconds);

	printthree("sdd", "total_sys_time", "seconds", "microsec",
		   "",
		   task->total_system_time.seconds,
		   task->total_system_time.microseconds);

	printthree("xxx", "task_self", "task_notify", "exception_port",
		   task->task_self, task->task_notify, task->exception_port);

	printthree("xxs", "bootstrap_port", "exception_port", "ipc_privilege",
		   task->bootstrap_port, task->exception_port,
		   truefalse(task->ipc_privilege));

	printthree("xx ", "ipc_xlations", "ipc_xlat_lock", "",
		   task->ipc_translations.next,
		   task->ipc_translation_lock,
		   0);

	printthree("xs ", "ipc_next_name", "ipc_active", "",
		   task->ipc_next_name, truefalse(task->ipc_active), 0);
#if	MACH_IPC_XXXHACK
	printthree("x  ", "ipc_enabled", "", "",
		   task->ipc_enabled, 0, 0);
#endif	MACH_IPC_XXXHACK

	printf("\n   Object Cache:\n");
	for (i = 0; i < OBJ_CACHE_MAX; i++) {
		printthree("xx ", "Name", "Object", "",
		      task->obj_cache[i].name, task->obj_cache[i].object, 0);
	}

	printf("\n");
	printthree("x  ", "ipc_intr_msg", "", "",
		   task->ipc_intr_msg, 0, 0);

	printf("\nRegistered IPC ports:\n");
	for (i = 0; i < TASK_PORT_REGISTER_MAX; i++) {
		printthree("x  ", "", "", "",
			   task->ipc_ports_registered[i], 0, 0);
	}

#if	MACH_EMULATION
	printthree("x  ", "eml_dispatch", "", "",
		   task->eml_dispatch, 0, 0);
#endif	MACH_EMULATION
}


display_timer_data(name, tmr)
char   *name;
timer_data_t *tmr;
{
	printf("%15.15s: lo: %#d\thi: %#d\tstamp: %#x\thi_bits_check: %#d\n",
	      name,
	      tmr->low_bits, tmr->high_bits, tmr->tstamp, tmr->high_bits_check);
	return(SUCCESS);
}

display_timer_save_data(name, tmr)
char   *name;
timer_save_t tmr;
{
	printf("%15.15s: lo: %#d\thi: %#d\n",
	       name,
	       tmr->low, tmr->high);
	return(SUCCESS);
}





char   *thread_state_names[] = {
				"waiting",
				"stopped",
				"runnable",
				"swapped",
				"idle"
};

#define STATEBITS	(sizeof(thread_state_names)/(sizeof(char *)))


display_thread(th)
thread_t th;
{
	int     i;
	char    processor_str[256];
	processor_t pr;

	printthree("xxx", "links", "runq", "task",
		   th->links.next, th->runq, th->task);
	printthree("xxx", "", "thread_list", "pset_threads",
		th->links.prev, th->thread_list.next, th->pset_threads.next);
	printthree("xxx", "lock", "", "",
		   th->lock, th->thread_list.prev, th->pset_threads.prev);
	printthree("xxx", "ref_count", "pcb", "kernel_stack",
		   th->ref_count, th->pcb, th->kernel_stack);
	printthree("xxs", "wait_event", "suspend_count", "interruptible",
	    th->wait_event, th->suspend_count, truefalse(th->interruptible));
	printthree("xxx", "wait_result", "timer_set", "wake_active",
		   th->wait_result,
		   truefalse(th->timer_set),
		   truefalse(th->wake_active));
	printthree("xxx", "swap_state", "priority", "sched_pri",
		   th->swap_state, th->priority, th->sched_pri);
	printf("         state : ");
	diagram((int) (th->state),
		thread_state_names,
		(int) STATEBITS);
	printf("\n");
	printthree("xxx", "cpu_usage", "sched_usage", "sched_stamp",
		   th->cpu_usage, th->sched_usage, th->sched_stamp);
	printthree("xsx", "recover", "vm_privilege", "unix_lock",
		   th->recover, truefalse(th->vm_privilege), th->unix_lock);
	printthree("xx ", "uthread", "utask", "",
		   th->u_address.uthread, th->u_address.utask, 0);
	printthree("xxx", "user_stop_count", "ipc_state_lock", "thread_self",
		   th->user_stop_count, th->ipc_state_lock, th->thread_self);
	printthree("xxx", "thread_reply", "ipc_wait_queue", "ipc_state",
		   th->thread_reply, th->ipc_wait_queue.next, th->ipc_state);
	printthree("xxx", "msize/kmsg", "", "ipc_kernel",
		   th->ipc_data.kmsg,
		   th->ipc_wait_queue.prev,
		   truefalse(th->ipc_kernel));
	display_timer_data("user-timer", &th->user_timer);
	display_timer_data("system-timer", &th->system_timer);
	display_timer_save_data("user-timer-save", &th->user_timer_save);
	display_timer_save_data("system-timer-save", &th->system_timer_save);
	printthree("xx ", "cpu_delta", "sched_delta", "",
		   th->cpu_delta, th->sched_delta, 0);

	printthree("xx ", "exception_port", "exc_clear_port", "",
		   th->exception_port, th->exception_clear_port);

	printthree("ssx", "active", "halted", "ast",
		   truefalse(th->active), truefalse(th->halted), th->ast);

	printthree("xss", "processor_set", "may_assign", "assign_active",
		   th->processor_set,
		   truefalse(th->may_assign),
		   truefalse(th->assign_active));

	pr = (processor_t) MAPPED(th->bound_processor);
	sprintf(&processor_str[0], " %#x (%d)",
		th->bound_processor,
		pr->slot_num);

	printthree("s  ", "bound_processor", "", "",
		   processor_str, 0, 0);
}




display_utask(vutp)
struct utask *vutp;
{
	struct utask *utp;
	int     i, result;
	int     filler = 0;
	int    *grpadr, grpval, lastfile, ofile, pofile;
	char    pofileval, *commadr;

	result = phys(vutp, &utp, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	utp = (struct utask *) (MAPPED(utp));

	printf("utask structure at 0x%.8x:\n", vutp);

	printthree("x  ", "procp", "", "",
		   utp->uu_procp, 0, 0);
	result = phys(&vutp->uu_comm[0], &commadr, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	commadr = (char *) (MAPPED(commadr));
	printf
	    ("           comm: %s\n", commadr);

	printthree("xxx", "uid", "ruid", "gid",
		   utp->uu_utnd.utnd_cred->cr_uid, utp->uu_procp->p_ruid, utp->uu_utnd.utnd_cred->cr_gid);
	printthree("x  ", "rgid", "", "",
		   utp->uu_procp->p_rgid, 0, 0);

#if	!BSD44
	printf("\nGroups: \n");
	for (i = 0; i < NGROUPS; i++) {
		result = phys(&vutp->uu_groups[i], &grpadr, ptb0);
		if (result != SUCCESS)
			return(FAILED);
		grpval = (gid_t) (MAP(grpadr));
		if (grpval == NOGROUP)
			break;
		printf(" 0x%.4x\t", grpval);
		if ((((i + 1) % 4) == 0) && (i < NGROUPS))
			printf("\n");
	}
#endif	!BSD44
	printf("\n");

	printthree("xxx", "tsize", "dsize", "ssize",
		   utp->uu_tsize, utp->uu_dsize, utp->uu_ssize);

	printthree("xxx", "text_start", "data_start", "outime",
		   utp->uu_text_start, utp->uu_data_start, utp->uu_outime);

	printthree("xxs", "stack_start", "stack_end", "stack_grows_up",
		   utp->uu_stack_start,
		   utp->uu_stack_end,
		   truefalse(utp->uu_stack_grows_up));

	printf("\nSignals: \n");
	for (i = 0; i < NSIG; i++) {
		int     sigval, *sigadr;

		result = phys(&vutp->uu_signal[i], &sigadr, ptb0);
		if (result != SUCCESS) {
			printf("mda: Could not translate signal address %#x\n",
			       vutp->uu_signal[i]);
			return(FAILED);
		}
		sigval = MAP(sigadr);
		printf(" 0x%.8x\t", sigval);
		if ((((i + 1) % 4) == 0) && (i < NSIG))
			printf("\n");
	}
	printf("\n");

	printf("Sigmask: \n");
	for (i = 0; i < NSIG; i++) {
		int     sigval, *sigadr;

		result = phys(&vutp->uu_sigmask[i], &sigadr, ptb0);
		if (result != SUCCESS) {
			printf("mda: Could not translate signal address %#x\n",
			       vutp->uu_sigmask[i]);
			return(FAILED);
		}
		sigval = MAP(sigadr);
		printf(" 0x%.8x\t", sigval);
		if ((((i + 1) % 4) == 0) && (i < NSIG))
			printf("\n");
	}
	printf("\n");

	vprt3l("sigcatch", "sigonstack", "sigintr",
	       &vutp->uu_sigcatch, &vutp->uu_sigonstack, &vutp->uu_sigintr);

	vprt3l("oldmask", "ss_sp", "ss_onstack",
	       &vutp->uu_oldmask,
	       &vutp->uu_sigstack.ss_sp,
	       &vutp->uu_sigstack.ss_onstack);

	vprt1l("lastfile", &vutp->uu_lastfile);

	result = phys(&vutp->uu_lastfile, &lastfile, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	lastfile = MAP(lastfile);
	printf("\n");

/*ttt*/

	printf("ofile: \n");
	for (i = 0; i < NOFILE; i++) {
		result = phys(&vutp->uu_ofile[i], &ofile, ptb0);
		if (result != SUCCESS)
			return(FAILED);
		ofile = MAP(ofile);
		printf(" 0x%.8x\t", ofile);
		if ((((i + 1) % 4) == 0) && (i < NOFILE))
			printf("\n");
	}
	printf("\n");

	printf("pofile: \n");
	for (i = 0; i < NOFILE; i++) {
		result = phys(&vutp->uu_pofile[i], &pofile, ptb0);
		if (result != SUCCESS)
			return(FAILED);
		pofileval = (char) (MAP(pofile));
		printf(" 0x%.8x\t", pofileval);
		if ((((i + 1) % 4) == 0) && (i < NOFILE))
			printf("\n");
	}
	printf("\n");

	printthree("xx  ", "cdir", "rdir", "",
	       utp->uu_utnd.utnd_cdir, utp->uu_utnd.utnd_cdir, 0);

	printf
	    ("   cmask: 0x%.4x\n",
	     utp->uu_cmask);
	printf("  uu_ru:   ");
	display_rusage(&utp->uu_ru);
	printf(" uu_cru:   ");
	display_rusage(&utp->uu_cru);
	for (i = 0; i < 3; i++)
		printf("timer[%d]: interval 0x%.8x 0x%.8x value 0x%.8x 0x%.8x\n",
		       i, utp->uu_timer[i].it_interval.tv_sec,
		       utp->uu_timer[i].it_interval.tv_usec,
		       utp->uu_timer[i].it_value.tv_sec,
		       utp->uu_timer[i].it_value.tv_usec);
	printf
	    ("start: 0x%.8x  0x%.8x\n",
	     utp->uu_start.tv_sec, utp->uu_start.tv_usec);
	printf(" acflag: 0x%.4x\n", utp->uu_acflag);
	vprt3l("pr_lock", "pr_base", "pr_size",
	       &vutp->uu_prof.pr_lock, &vutp->uu_prof.pr_base, &vutp->uu_prof.pr_size);
	printf
	    ("  pr_off: 0x%.8x   pr_scale: 0x%.8x\n",
	     utp->uu_prof.pr_off, utp->uu_prof.pr_scale);
#ifdef	notdef
	printf
	    ("  aid: 0x%.4x        maxuprc: 0x%.4x\n",
	     utp->uu_aid, utp->uu_maxuprc);
#else
	printf
	    ("  maxuprc: 0x%.4x\n",
	     utp->uu_maxuprc);
#endif
	for (i = 0; i < RLIM_NLIMITS; i++) {
		printf("rlimit[%d]: cur: 0x%.8x max: 0x%.8x\n", i,
		     utp->uu_rlimit[i].rlim_cur, utp->uu_rlimit[i].rlim_max);
	}
	printf
	    ("  quota: 0x%.8x   qflags: 0x%.8x", utp->uu_quota, utp->uu_qflags);
#ifdef	notdef
	printf("   modes: %c", utp->uu_modes);
#endif	notdef
	printf("\n");
#if	MMAX_MP
	printf("Locked or changing {p}ofile locks:\n");
	for (i = 0; i < NOFILE; i++) {
		if (utp->uu_lfile[i].want_write |
		    utp->uu_lfile[i].l_un.Interlock.lock_data)
			display_lock(&utp->uu_lfile[i]);
	}
	printthree("xxx", "timer_lock", "nhops", "ip_lock",
		   utp->uu_timer_lock, utp->uu_ip_nhops, utp->uu_ip_lock);
#endif	MMAX_MP
}


display_uthread(uthp)
struct uthread *uthp;
{
	int	i;

	printf("uthread structure at 0x%.8x:\n", uthp);

	printf("       ar0: 0x%.8x    arg[0]: 0x%.8x       arg[1]: 0x%.8x\n",
	       uthp->uu_ar0, uthp->uu_arg[0], uthp->uu_arg[1]);
	printf("    arg[2]: 0x%.8x    arg[3]: 0x%.8x       arg[4]: 0x%.8x\n",
	       uthp->uu_arg[2], uthp->uu_arg[3], uthp->uu_arg[4]);
	printf("    arg[5]: 0x%.8x    arg[6]: 0x%.8x       arg[7]: 0x%.8x\n",
	       uthp->uu_arg[5], uthp->uu_arg[6], uthp->uu_arg[7]);

	printf("        ap: 0x%.8x\n", uthp->uu_ap);
	printf("     qsave:\n");
	display_qsave(&uthp->uu_qsave);
	printf("    r_val1: 0x%.8x    r_val2: 0x%.8x\n",
#ifdef	notdef
	       uthp->r_val1, uthp->r_val2);
#else	notdef
	       0, 0);
#endif	notdef
	printf("     error: 0x%.8x     eosys: 0x%.8x\n",
	       uthp->uu_error, uthp->uu_eosys);
#if	CS_RPAUSE
	printf("     rpsfs: 0x%.8x  rpswhich: 0x%.8x      rpause: 0x%.8x\n",
	       uthp->uu_rpsfs, uthp->uu_rpswhich, uthp->uu_rpause);
#endif	CS_RPAUSE
#if	CS_RFS
	printf("       rfs: 0x%.8x   rfscode: 0x%.8x     rfsncnt: 0x%.8x\n",
	       uthp->uu_rfs, uthp->uu_rfscode, uthp->uu_rfsncnt);
#endif	CS_RFS
	printf("    unameicache:\n");
	printf("prevoffset: 0x%.8x   inumber: 0x%.8x         dev: 0x%.8x\n",
	       uthp->uu_ncache.nc_prevoffset, uthp->uu_ncache.nc_inumber,
	       uthp->uu_ncache.nc_dev);
	printf("      time: 0x%.8x\n", uthp->uu_ncache.nc_time);
	printf("    ---nameidata stored here at 0x%.8x\n", &uthp->uu_nd);
	printf("      code: 0x%.8x    cursig: 0x%.8x         sig: 0x%.8x\n",
	       uthp->uu_code, uthp->uu_cursig, uthp->uu_sig);

	printf("\nSignals: \n");
	for (i = 0; i < NSIG; i++) {
		int     sigval, *sigadr;

		result = phys(&uthp->uu_tsignal[i], &sigadr, ptb0);
		if (result != SUCCESS) {
			printf("mda: Could not translate signal address %#x\n",
			       uthp->uu_tsignal[i]);
			return(FAILED);
		}
		sigval = MAP(sigadr);
		printf(" 0x%.8x\t", sigval);
		if ((((i + 1) % 4) == 0) && (i < NSIG))
			printf("\n");
	}
	printf("\n");
}



display_qsave(qp)
label_t *qp;
{
#if	ns32000
#define	Q_R3	0
#define	Q_R4	1
#define	Q_R5	2
#define	Q_R6	3
#define	Q_R7	4
#define	Q_FP	5
#define	Q_SP	6
#define	Q_UPSR	7		/* only high 16 bits are valid */
#define	Q_PC	8
	printf("        r3: 0x%.8x        r4: 0x%.8x          r5: 0x%.8x\n",
	       qp->val[Q_R3], qp->val[Q_R4], qp->val[Q_R5]);
	printf("        r6: 0x%.8x        r7: 0x%.8x          fp: 0x%.8x\n",
	       qp->val[Q_R6], qp->val[Q_R7], qp->val[Q_FP]);
	printf("        sp: 0x%.8x      upsr: 0x%.8x          pc: 0x%.8x\n",
	       qp->val[Q_SP], (qp->val[Q_UPSR]) & 0xff00, qp->val[Q_PC]);
#else	ns32000
	display_qsave_must_be_fixed();
#endif	ns32000
}



static int nproc;
static struct proc *procp;

display_proc(vproc)
struct proc *vproc;
{
	struct proc *pp;

	if (phys(vproc, &pp, ptb0) != SUCCESS) {
		printf(Canttranslate, vproc);
		return(FAILED);
	}
	pp = (struct proc *) (MAPPED(pp));
	printthree("xxx", "p_link", "p_rlink", "p_nxt",
		   pp->p_link, pp->p_rlink, pp->p_nxt);
	printthree("xxx", "*p_prev", "p_usrpri", "p_pri",
		   pp->p_prev, pp->p_usrpri, pp->p_pri);
	printthree("xxx", "p_cpu", "p_stat", "p_time",
		   pp->p_cpu, pp->p_stat, pp->p_time);
	printthree("xxx", "p_nice", "p_slptime", "p_cursig",
		   pp->p_nice, pp->p_slptime, pp->p_cursig);
	printthree("xxx", "p_sig", "p_sigmask", "p_sigignore",
		   pp->p_sig, pp->p_sigmask, pp->p_sigignore);

	printthree("xxx", "p_sigcatch", "p_flag", "p_uid",
		   pp->p_sigcatch, pp->p_flag, pp->p_uid);
	printthree("xxx", "p_pgrp", "p_pid", "p_ppid",
		   pp->p_pgrp, pp->p_pid, pp->p_ppid);
	printthree("xxx", "p_xstat", "p_logdev", "p_ru",
		   pp->p_xstat,
#if	CS_GENERIC
		   pp->p_logdev,
#else	CS_GENERIC
		   0,
#endif	CS_GENERIC
		   pp->p_ru);

	printthree("xxx", "p_rssize", "p_maxrss", "p_swrss",
		   pp->p_rssize, pp->p_maxrss, pp->p_swrss);
	printthree("xxx", "p_swaddr", "p_stopsig", "p_cpticks",
		   pp->p_swaddr, pp->p_stopsig, pp->p_cpticks);
	printthree("xxx", "p_pctcpu", "p_ndx", "p_idhash",
		   pp->p_pctcpu, pp->p_ndx, pp->p_idhash);
	printthree("xxx", "p_pptr", "p_cptr", "p_osptr",
		   pp->p_pptr, pp->p_cptr, pp->p_osptr);
	printthree("xxx", "p_ysptr", "p_quota", "task",
		   pp->p_ysptr, pp->p_quota, pp->task);
	printthree("xx ", "thread", "siglock", "",
		   pp->thread,
		   pp->siglock, 0);

	printf("        sigwait: %10s    exit_thread: %#10x\n",
	       truefalse(pp->sigwait),
	       pp->exit_thread);
#if 0
	struct itimerval p_realtimer
	struct lock lock
	        printf("Lock structure @ %#x\n", &vproc->lock);

	display_lock(&pp->lock);
#endif 0
}



display_vm_map_entry(ve)
vm_map_entry_t ve;		/* Virtual adr of map entry */
{
	int     result;
	vm_map_entry_t pe;	/* Physical adr of map entry */

	result = phys(ve, &pe, ptb0);
	if (result != SUCCESS) {
		printf("mda: Could not translate map entry address 0x%x",
		       ve);
		return(FAILED);
	}
	pe = (vm_map_entry_t) (MAPPED(pe));
	printf("vm_map_entry at %#x:\n", ve);

	printthree("xxx", "prev", "start", "object",
		   pe->vme_prev, pe->vme_start, pe->object);
	printthree("xxx", "next", "end", "offset",
		   pe->vme_next, pe->vme_end, pe->offset);
	printthree("sss", "is_a_map", "is_sub_map", "copy_on_write",
		   truefalse(pe->is_a_map),
		   truefalse(pe->is_sub_map),
		   truefalse(pe->copy_on_write));
	printthree("sxx", "needs_copy", "protection", "max_protection",
		   truefalse(pe->needs_copy), pe->protection,
		   pe->max_protection);
	printthree("xd ", "inheritance", "wired_count", "",
		   pe->inheritance, pe->wired_count, 0);
}



display_vm_map(vmap)
vm_map_t vmap;			/* Virtual address of map */
{
	vm_map_t phmap;		/* Physical address of map */
	vm_map_entry_t mep, pmep;
	int     result, i;
	lock_t  vlp;

	result = phys(vmap, &phmap, ptb0);
	if (result != SUCCESS) {
		printf(Canttranslate, vmap);
		return(FAILED);
	}
	phmap = (vm_map_t) (MAPPED(phmap));
	printf("vm_map at %#x:\n", vmap);

	vlp = &vmap->lock;
	printf("\n... lock at %#x:\n", vlp);
	display_lock(&phmap->lock);

	printf("\n... header ");
#if	0
	display_vm_map_entry(&vmap->header);
#endif	0

	printf("\n...\n");
	printthree("dxx", "nentries", "pmap", "size",
		   phmap->nentries, phmap->pmap, phmap->size);
	printthree("sdx", "is_main_map", "ref_count", "ref_lock",
		   truefalse(phmap->is_main_map), phmap->ref_count,
		   phmap->ref_lock);
	printthree("xxx", "hint", "hint_lock", "first_free",
		   phmap->hint, phmap->hint_lock, phmap->first_free);
	printthree("sx ", "entries_pageabl", "timestamp", "",
		   truefalse(phmap->entries_pageable), phmap->timestamp, 0);

	for (i = 0, mep = phmap->links.next; i < phmap->nentries; i++) {
		printf("\n... ");
		display_vm_map_entry(mep);
		if (phys(mep, &pmep, ptb0) != SUCCESS) {
			printf(Canttranslate, mep);
			return(FAILED);
		}
		pmep = (vm_map_entry_t) (MAPPED(pmep));
		mep = pmep->vme_next;
	}
}



display_zone(vzone)
zone_t  vzone;
{
	zone_t  pzone;
	int     result;
	char   *zname;
	int    *zlock;

	result = phys(vzone, &pzone, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	pzone = (zone_t) (MAPPED(pzone));

	zname = pzone->zone_name;
	result = phys(zname, &zname, ptb0);
	if (result != SUCCESS)
		return(FAILED);
	zname = (char *) (MAPPED(zname));
	printf("Zone %s at %#x:\n", zname, vzone);
	printthree("xxx", "lock", "count", "free",
		   pzone->lock, pzone->count, pzone->free_elements);
	printthree("xxx", "cur_size", "max_size", "elem_size",
		   pzone->cur_size, pzone->max_size, pzone->elem_size);
	printthree("xss", "alloc_size", "doing_alloc", "pageable",
		   pzone->alloc_size, truefalse(pzone->doing_alloc),
		   truefalse(pzone->pageable));
	printthree("ss ", "sleepable", "exhaustible", "",
		 truefalse(pzone->sleepable), truefalse(pzone->exhaustible));
	display_lock(&pzone->complex_lock);
}



display_pcb(pcb)
struct pcb *pcb;
{
	printthree("xxx", "pcb_usp", "pcb_ssp", "pcb_r0",
		   pcb->pcb_usp, pcb->pcb_ssp, pcb->pcb_r0);

	printthree("xxx", "pcb_r1", "pcb_r2", "pcb_r3",
		   pcb->pcb_r1, pcb->pcb_r2, pcb->pcb_r3);

	printthree("xxx", "pcb_r4", "pcb_r5", "pcb_r6",
		   pcb->pcb_r4, pcb->pcb_r5, pcb->pcb_r6);

	printthree("xxx", "pcb_r7", "pcb_fp", "pcb_pc",
		   pcb->pcb_r7, pcb->pcb_fp, pcb->pcb_pc);

	printthree("xxx", "pcb_modpsr", "pcb_fsr", "pcb_ptbr",
		   pcb->pcb_modpsr, pcb->pcb_fsr, pcb->pcb_ptbr);

#if	MMAX_XPC || MMAX_APC
	printthree("x  ", "pcb_isrv", 0, 0,
		   pcb->pcb_isrv, 0, 0);
#endif	MMAX_XPC || MMAX_APC
}

#if	MMAX_XPC || MMAX_APC
diagram(reg, names, size)
int     reg;
char   *names[];
int     size;

{
	int     prev, i, k;

	if (reg) {
		prev = 0;
		for (i = 1, prev = k = 0; k < size; (i = (i << 1)), k++)
			if (reg & i)
				printf("%s%s", prev++ ? "," : "<", names[k]);
		putchar('>');
	}
}

#endif	MMAX_XPC || MMAX_APC



char *strategy[] = {
		"NONE ",
		"CALL ",
		"DELAY"
};

display_vm_object(vobj)
vm_object_t vobj;
{
	int	result;
	vm_object_t obj;

	result = phys(vobj, &obj, ptb0);
	if (result != SUCCESS) {
		printf(Canttranslate, vobj);
		return(FAILED);
	}
	obj = (vm_object_t) (MAPPED(obj));

	printthree("xxx", "memq.next", "", "Lock",
		   obj->memq.next, 0, obj->Lock);
#if	VM_OBJECT_DEBUG
	printthree("xxx", "memq.prev", "", "LockHolder",
		   obj->memq.prev, 0, obj->LockHolder);
#else	VM_OBJECT_DEBUG
	printthree("xxx", "memq.prev", "", "",
		   obj->memq.prev, 0, 0);
#endif	VM_OBJECT_DEBUG
	printthree("xxx", "ref_count", "size", "resident_page",
		   obj->ref_count, obj->size, obj->resident_page_count);
	printthree("xxx", "copy", "shadow", "shadow_offset",
		   obj->copy, obj->shadow, obj->shadow_offset);
	printthree("xxx", "pager", "paging_offset", "pager_request",
		   obj->pager, obj->paging_offset, obj->pager_request);
	printthree("xsx", "pager_name", "copy_strategy", "paging",
		   obj->pager_name, strategy[obj->copy_strategy], obj->paging_in_progress);
	printthree("xxx", "", "can_persist", "internal",
		   0, obj->can_persist, obj->internal);

	printthree("xxx", "temporary", "alive", "cached_list",
		    obj->temporary, obj->alive, obj->cached_list.next);
	printthree("x x", "single_use", "", "cached_list", "",
		   obj->single_use, 0, obj->cached_list.prev);
}
