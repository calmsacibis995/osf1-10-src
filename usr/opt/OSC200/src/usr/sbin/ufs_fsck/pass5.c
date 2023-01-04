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
static char	*sccsid = "@(#)$RCSfile: pass5.c,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1992/10/30 15:48:29 $";
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
 * Copyright (c) 1980, 1986 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint

#endif /* not lint */

#include <sys/param.h>
#include <ufs/dinode.h>
#include <ufs/fs.h>
#include <string.h>
#include "fsck.h"

pass5()
{
	int c, blk, frags, basesize, sumsize, mapsize, savednrpos;
	register struct fs *fs = &sblock;
	register struct cg *cg = &cgrp;
	daddr_t dbase, dmax;
	register daddr_t d;
	register int i, j;
	struct csum *cs;
	time_t now;
	struct csum cstotal;
	struct inodesc idesc[3];
	char buf[MAXBSIZE];
	register struct cg *newcg = (struct cg *)buf;
	struct ocg *ocg = (struct ocg *)buf;
	int bad_magic;

#ifdef DEBUG
	printf ("Entering Pass5\n");
#endif
	bzero((char *)newcg, (size_t)fs->fs_cgsize);
	newcg->cg_niblk = fs->fs_ipg;
	switch ((int)fs->fs_postblformat) {

	case FS_42POSTBLFMT:
		basesize = (char *)&ocg->cg_btot[0] - (char *)ocg;
		sumsize = &ocg->cg_iused[0] - (char *)(&ocg->cg_btot[0]);
		mapsize = &ocg->cg_free[howmany(fs->fs_fpg, NBBY)] -
			(u_char *)&ocg->cg_iused[0];
		ocg->cg_magic = CG_MAGIC;
		savednrpos = fs->fs_nrpos;
		fs->fs_nrpos = 8;
		break;

	case FS_DYNAMICPOSTBLFMT:
		newcg->cg_btotoff = &newcg->cg_space[0] - (u_char *)newcg;
		newcg->cg_boff =
			newcg->cg_btotoff + fs->fs_cpg * sizeof(int);
		newcg->cg_iusedoff = newcg->cg_boff + 
			fs->fs_cpg * fs->fs_nrpos * sizeof(short);
		newcg->cg_freeoff =
			newcg->cg_iusedoff + howmany(fs->fs_ipg, NBBY);
		newcg->cg_nextfreeoff = newcg->cg_freeoff +
			howmany(fs->fs_cpg * fs->fs_spc / NSPF(fs),
				NBBY);
		newcg->cg_magic = CG_MAGIC;
		basesize = &newcg->cg_space[0] - (u_char *)newcg;
		sumsize = newcg->cg_iusedoff - newcg->cg_btotoff;
		mapsize = newcg->cg_nextfreeoff - newcg->cg_iusedoff;
		break;

	default:
		errexit("UNKNOWN ROTATIONAL TABLE FORMAT %d\n",
			fs->fs_postblformat);
	}
	bzero((char *)&idesc[0], sizeof idesc);
	for (i = 0; i < 3; i++)
		idesc[i].id_type = ADDR;
	bzero((char *)&cstotal, sizeof(struct csum));
	(void)time(&now);
	j = blknum(fs, fs->fs_size + fs->fs_frag - 1);
	for (i = fs->fs_size; i < j; i++)
		setbmap(i);
	for (c = 0; c < fs->fs_ncg; c++) {
		bad_magic = 0;
		getblk(&cgblk, cgtod(fs, c), fs->fs_cgsize);
		/*
		 * If the magic number is wrong, then
		 * we can't believe anything, including
		 * cg_chkmagic(). Set bad_magic flag which
		 * rewrites the cylinder group with newcg.
		 */
		if (!cg_chkmagic(cg)) {
			pfatal("CG %d: BAD MAGIC NUMBER\n", c);
			bad_magic = 1;
		}
		dbase = cgbase(fs, c);
		dmax = dbase + fs->fs_fpg;
		if (dmax > fs->fs_size)
			dmax = fs->fs_size;
		if (now > cg->cg_time)
			newcg->cg_time = cg->cg_time;
		else
			newcg->cg_time = now;
		newcg->cg_cgx = c;
		if (c == fs->fs_ncg - 1)
			newcg->cg_ncyl = fs->fs_ncyl % fs->fs_cpg;
		else
			newcg->cg_ncyl = fs->fs_cpg;
		newcg->cg_ndblk = dmax - dbase;
		newcg->cg_cs.cs_ndir = 0;
		newcg->cg_cs.cs_nffree = 0;
		newcg->cg_cs.cs_nbfree = 0;
		newcg->cg_cs.cs_nifree = fs->fs_ipg;
		if (cg->cg_rotor < newcg->cg_ndblk)
			newcg->cg_rotor = cg->cg_rotor;
		else
			newcg->cg_rotor = 0;
		if (cg->cg_frotor < newcg->cg_ndblk)
			newcg->cg_frotor = cg->cg_frotor;
		else
			newcg->cg_frotor = 0;
		if (cg->cg_irotor < newcg->cg_niblk)
			newcg->cg_irotor = cg->cg_irotor;
		else
			newcg->cg_irotor = 0;
		bzero((char *)&newcg->cg_frsum[0], sizeof newcg->cg_frsum);
		bzero((char *)&cg_blktot(newcg)[0], 
		      (size_t)(sumsize + mapsize));
		if (fs->fs_postblformat == FS_42POSTBLFMT)
			ocg->cg_magic = CG_MAGIC;
		j = fs->fs_ipg * c;
		for (i = 0; i < fs->fs_ipg; j++, i++) {
			switch (statemap[j]) {

			case USTATE:
				break;

			case DSTATE:
			case DCLEAR:
			case DFOUND:
				newcg->cg_cs.cs_ndir++;
				/* fall through */

			case FSTATE:
			case FCLEAR:
				newcg->cg_cs.cs_nifree--;
				setbit(cg_inosused(newcg), i);
				break;

			default:
				if (j < ROOTINO)
					break;
				errexit("BAD STATE %d FOR INODE I=%d",
				    statemap[j], j);
			}
		}
		if (c == 0)
			for (i = 0; i < ROOTINO; i++) {
				setbit(cg_inosused(newcg), i);
				newcg->cg_cs.cs_nifree--;
			}
		for (i = 0, d = dbase;
		     d < dmax;
		     d += fs->fs_frag, i += fs->fs_frag) {
			frags = 0;
			for (j = 0; j < fs->fs_frag; j++) {
				if (testbmap(d + j))
					continue;
				setbit(cg_blksfree(newcg), i + j);
				frags++;
			}
			if (frags == fs->fs_frag) {
				newcg->cg_cs.cs_nbfree++;
				j = cbtocylno(fs, i);
				cg_blktot(newcg)[j]++;
				cg_blks(fs, newcg, j)[cbtorpos(fs, i)]++;
			} else if (frags > 0) {
				newcg->cg_cs.cs_nffree += frags;
				blk = blkmap(fs, cg_blksfree(newcg), i);
				fragacct(fs, blk, newcg->cg_frsum, 1);
			}
		}
		cstotal.cs_nffree += newcg->cg_cs.cs_nffree;
		cstotal.cs_nbfree += newcg->cg_cs.cs_nbfree;
		cstotal.cs_nifree += newcg->cg_cs.cs_nifree;
		cstotal.cs_ndir += newcg->cg_cs.cs_ndir;
		cs = &fs->fs_cs(fs, c);
		if (bcmp((char *)&newcg->cg_cs, (char *)cs, sizeof *cs) != 0 &&
		    dofix(&idesc[0], "FREE BLK COUNT(S) WRONG IN SUPERBLK")) {
			bcopy((char *)&newcg->cg_cs, (char *)cs, sizeof *cs);
			sbdirty();
		}
		if (cvtflag || bad_magic) {
			bcopy((char *)newcg, (char *)cg, (size_t)fs->fs_cgsize);
			cgdirty();
			continue;
		}
		if (bcmp(cg_inosused(newcg),
			 cg_inosused(cg), mapsize) != 0 &&
		    dofix(&idesc[1], "BLK(S) MISSING IN BIT MAPS")) {
			bcopy(cg_inosused(newcg), cg_inosused(cg), 
			      (size_t)mapsize);
			cgdirty();
		}
		if ((bcmp((char *)newcg, (char *)cg, basesize) != 0 ||
		     bcmp((char *)&cg_blktot(newcg)[0],
			  (char *)&cg_blktot(cg)[0], sumsize) != 0) &&
		    dofix(&idesc[2], "SUMMARY INFORMATION BAD")) {
			bcopy((char *)newcg, (char *)cg, (size_t)basesize);
			bcopy((char *)&cg_blktot(newcg)[0],
			      (char *)&cg_blktot(cg)[0], (size_t)sumsize);
			cgdirty();
		}
	}
	if (fs->fs_postblformat == FS_42POSTBLFMT)
		fs->fs_nrpos = savednrpos;
	if (bcmp((char *)&cstotal, (char *)&fs->fs_cstotal, sizeof *cs) != 0
	    && dofix(&idesc[0], "FREE BLK COUNT(S) WRONG IN SUPERBLK")) {
		bcopy((char *)&cstotal, (char *)&fs->fs_cstotal, sizeof *cs);
		fs->fs_ronly = 0;
		fs->fs_fmod = 0;
		sbdirty();
	}
#ifdef DEBUG
	printf ("Pass5 complete\n");
#endif
}
