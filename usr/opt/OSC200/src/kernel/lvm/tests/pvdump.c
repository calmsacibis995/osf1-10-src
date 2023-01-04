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
static char	*sccsid = "@(#)$RCSfile: pvdump.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/04/29 14:23:04 $";
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
#include <sys/file.h>
#include <lvm/pvres.h>

main(argc,argv)
int argc;
char **argv;
{
	while (*++argv) {
		dumplvrec(*argv);
	}
}

dumplvrec(path)
char *path;
{
int fd, rcount;
int differ = 0;
int secsize = 512;
char *s;
union {
	struct lv_lvmrec data;
	char space[512];
} lr[2];

	if ((fd = open(path,O_RDONLY)) < 0) {
		perror("open");
		return;
	}

	/* Read LVM Record 1 */
	if (lseek(fd, (off_t)(PVRA_LVM_REC_SN1*512), 0) < 0) {
		perror("lseek 1");
	}
	if ((rcount = read(fd, &lr[0], secsize)) != secsize) {
		perror("read 1");
	}
	if (lseek(fd, (off_t)(PVRA_LVM_REC_SN2*512), 0) < 0) {
		perror("lseek 2");
	}
	if ((rcount = read(fd, &lr[1], secsize)) != secsize) {
		perror("read 2");
	}
	if (bcmp(&lr[0].data, &lr[1].data, 512)) {
		printf("LVM Record #1 and #2 do not match.\n");	
		differ++;
	}
	checkreserved(lr);

	print_lvmrec(&lr[0].data);
	
	return;
}

checkreserved(p)
char *p;
{
char *s;
int nonnull = 0;
int i;

	for (i = 0; i < 2; i++) {
		for (s = p + sizeof(struct lv_lvmrec); s < (p + 512); s++) {
			if (*s) nonnull++;
		}
		if (nonnull) {
			printf("Reserved area not zeroed:\n");
			printf("%d non-null bytes [Record %d]\n", nonnull, i);
		}
		p += 512;
	}

	return;
}

#if __STDC__
#define print_member(S,M) printf("\t" #M ": %8d (0x%08x)\n", S->M, S->M)
#else
#define print_member(S,M) printf("%15s: %8d (0x%08x)\n", "M", S->M, S->M)
#endif

print_lvmrec(lrp)
struct lv_lvmrec *lrp;
{
	printf("LVM Record Tag:            %8s\n", lrp->lvm_id);
	printf("Physical Volume Unique Id: 0x%08x%08x\n",
		lrp->pv_id.id1, lrp->pv_id.id2);
	printf("Volume Group Unique Id:    0x%08x%08x\n",
		lrp->vg_id.id1, lrp->vg_id.id2);

	print_member(lrp, last_psn);
	print_member(lrp, pv_num);
	print_member(lrp, vgra_len);
	print_member(lrp, vgra_psn);
	print_member(lrp, vgda_len);
	print_member(lrp, vgsa_len);
	print_member(lrp, vgda_psn1);
	print_member(lrp, vgda_psn2);
	print_member(lrp, mcr_len);
	print_member(lrp, mcr_psn1);
	print_member(lrp, mcr_psn2);
	print_member(lrp, data_len);
	print_member(lrp, data_psn);
	print_member(lrp, pxsize);
	print_member(lrp, pxspace);
	print_member(lrp, altpool_len);
	print_member(lrp, altpool_psn);

	return;
}
