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
 * Modification History: /sys/dist/mfg.d/fsmrg.c
 *
 * This utility opens the kernel image specified, locates two
 * variables (mdsize and memdev).  If present, `mdsize' blocks of the
 * specified file system image are copied into the kernel image at
 * `memdev'. It is used to install a memory file system for use in the
 * Ultrix-32 standalone environment.
 *
 * 10-Oct-90 -- afd
 *	Added Alpha support.  For mips-alpha cross development environment,
 *	this program must be built with the 64-bit mips compiler and linked
 *	with the C library that contains 64bit nlist (for 64bit object files).
 */

#ifdef __mips64
#include "/usr/projects/alpha/Ultrix.mips64/usr/include/stdio.h"
#include "/usr/projects/alpha/Ultrix.mips64/usr/include/sys/types.h"
#include "/usr/projects/alpha/Ultrix.mips64/usr/include/sys/buf.h"
#include <sys/param.h>

#include "aouthdr.h"
#include "filehdr.h"
#include "a.out.h"

struct exec {
	struct filehdr	ex_f;
	struct aouthdr	ex_o;
};


#include <nlist.h>

#else /* native - not cross development */

#include <stdio.h>
#include <sys/param.h>
#ifdef vax
#include <a.out.h>
#else /* mips or alpha */
#include <sys/exec.h>
#include <nlist.h>
#include <scnhdr.h>
#endif
#include <sys/buf.h>
#endif /* mips64 */

struct nlist nmlst[3];
#define BUFSZ 512	/* file system blocks */

main(argc, argv)
	int argc;
	char **argv;
{
	struct exec x;
	int i, fs, vm;
	int mdsize;
	int memdev;
	int ts;
	char *buf[BUFSZ];

	argc--, argv++;
	if (argc < 2) {
		fprintf(stderr, "Usage: fsmrg fs_image kernel_image\n");
		exit(1);
	}
	/*
	 * Initialize the name list
	 */
#if defined mips || defined __alpha || defined __mips64
	nmlst[0].n_name = "mdsize";
	nmlst[1].n_name = "memdev";
	nmlst[2].n_name = "";
#else   /* vax */
	nmlst[0].n_un.n_name = "_mdsize";
	nmlst[1].n_un.n_name = "_memdev";
	nmlst[2].n_un.n_name = "";
#endif
	nlist(argv[1], nmlst);
	if(nmlst[0].n_type == 0) {
		fprintf(stderr, "no %s namelist\n", argv[1]);
		exit(1);
	}
	if ((vm = open(argv[1], 2)) < 0) {
		fprintf(stderr, "Can't open %s\n", argv[1]);
		exit(1);
	}
	if ((fs = open(argv[0], 0)) < 0) {
		fprintf(stderr, "Can't open %s\n", argv[0]);
		exit(1);
	}
	if ((i = read(vm, (char *)&x, sizeof x)) != sizeof x) {
		fprintf(stderr, "Can't read %s\n", argv[1]);
		exit(1);
	}
#ifdef  mips
	mdsize = ((int)nmlst[0].n_value); 
	mdsize &= ~0x80000000;
	/* Kernel VA's offset by something other than 0 */
	ts = x.ex_o.text_start & ~0x80000000;
	mdsize +=  N_TXTOFF(x.ex_f, x.ex_o) - ts;
#endif  /* mips */
#if defined  __alpha || defined __mips64
	/*
	 * For Alpha we did not artificially set any bits in the low
	 * 32 bits of the address space (kernel starts at ffffffff 00000000)
	 */
	mdsize = ((int)nmlst[0].n_value); 
	ts = x.ex_o.text_start;
	mdsize +=  N_TXTOFF(x.ex_f, x.ex_o) - ts;
#endif  /* alpha */
#ifdef  vax
	mdsize = ((int)nmlst[0].n_value); 
	mdsize &= ~0x80000000;
	mdsize += sizeof x;
	if (x.a_text % 1024)
		mdsize -= (1024 - (x.a_text % 1024));
#endif  /* vax */
	if ((lseek(vm, (off_t)mdsize, 0)) < 0) {
		fprintf(stderr, "can't find mdsize\n");
		exit(1);
	}
	if ((read(vm, &mdsize, sizeof mdsize)) != sizeof mdsize) {
		fprintf(stderr, "can't read mdsize\n");
		exit(1);
	}
#if defined mips || defined __alpha || defined __mips64
	printf("real mdsize = %d\n", mdsize);
#endif  /* mips or alpha */
#if defined mips
	memdev = ((int)nmlst[1].n_value); 
	memdev &= ~0x80000000;
	memdev +=  N_TXTOFF(x.ex_f, x.ex_o) - ts;
#endif  /* mips or alpha */
#if defined __alpha || defined __mips64
	/*
	 * For Alpha we did not artificially set any bits in the low
	 * 32 bits of the address space (kernel starts at ffffffff 00000000)
	 */
	memdev = ((int)nmlst[1].n_value); 
	memdev +=  N_TXTOFF(x.ex_f, x.ex_o) - ts;
#endif  /* mips or alpha */
#ifdef  vax
	memdev = ((int)nmlst[1].n_value); 
	memdev &= ~0x80000000;
	memdev += sizeof x;
	if (x.a_text % 1024)
		memdev -= (1024 - (x.a_text % 1024));
#endif
	lseek(vm, memdev, 0);
	fprintf(stdout, "Writing %d blocks of %s into %s\n",
		mdsize, argv[0], argv[1]);
	for (i = 0 ; i < mdsize; i ++) {
		if ((read (fs, buf, BUFSZ)) != BUFSZ) {
			fprintf(stderr, "%s: read failed\n", argv[0]);
			exit(1);
		}
		if ((write (vm, buf, BUFSZ)) != BUFSZ) {
			fprintf(stderr, "%s: write failed\n", argv[1]);
			exit(1);
		}
	}
	close(vm);
	close(fs);
}
