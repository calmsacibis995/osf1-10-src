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
static char	*sccsid = "@(#)$RCSfile: mips_symtab.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:12:12 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from mips_symtab.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

#include <hal/kdb/defs.h>
#include <syms.h>

extern char end[];

HDRR	*symtab = 0;
char	*esymtab = end;
int	ld_screwed_up = 0;

typedef struct {
	struct {
		struct filehdr	f;
		AOUTHDR		a;
	}	 hdr;
	HDRR	 sym_hdr;
	char	 syms[1];	/* VARSIZE */
} loader_info;


read_mips_symtab(start)
loader_info *start;
{
	int stsize, st_hdrsize;
	unsigned st_filptr;

	/* get symbol table header */

	st_hdrsize = start->hdr.f.f_nsyms;
	st_filptr  = start->hdr.f.f_symptr;
	if (st_filptr == 0) {
		dprintf("[ no valid symbol table present ]\n");
		goto nosymt;
	}

	/* find out how much stuff is there */
	stsize = (start->sym_hdr.cbExtOffset - (st_filptr + st_hdrsize))
		 + start->sym_hdr.iextMax * cbEXTR;
	dprintf("[ preserving %d bytes of symbol table ]\n", stsize);

	symtab = &start->sym_hdr;
	esymtab = (char*)symtab + stsize + st_hdrsize;

	/* Change all file pointers into memory pointers */
	fixup_symtab( symtab, (char*)symtab + st_hdrsize,
		      st_filptr + st_hdrsize);
nosymt:
	return;
}


/*
 * Fixup the symbol table to hold in-memory pointers, as opposed
 * to file displacements
 */
fixup_symtab( hdr, data, f_ptr)
HDRR *hdr;
char *data;
{
	int             f_idx, s_idx;
	FDR            *fh;
	SYMR	       *sh;
	OPTR	       *op;
	PDR	       *pr;
	EXTR	       *esh;
	static SYMR	static_procedure_symbol =
			{ (long)"<static procedure>", 0, stStatic, scText, 0, 0};

#define FIX(off) \
	if (hdr->off) hdr->off = (unsigned int)data + (hdr->off - f_ptr);

	FIX(cbLineOffset);
	FIX(cbDnOffset);
	FIX(cbPdOffset);
	FIX(cbSymOffset);
	FIX(cbOptOffset);
	FIX(cbAuxOffset);
	FIX(cbSsOffset);
	FIX(cbSsExtOffset);
	FIX(cbFdOffset);
	FIX(cbRfdOffset);
	FIX(cbExtOffset);
#undef FIX
	/*
	 * Now go on and fixup all the indexes within the symtab itself.
	 * We must work on a file-basis, hence the mess. 
	 */
	for (f_idx = 0; f_idx < hdr->ifdMax; f_idx++) {
		fh = (FDR *) (hdr->cbFdOffset + f_idx * sizeof(FDR));
		/* fix file descriptor itself */
		fh->issBase += hdr->cbSsOffset;
		fh->isymBase = hdr->cbSymOffset + fh->isymBase * sizeof(SYMR);
		fh->ioptBase = hdr->cbOptOffset + fh->ioptBase * sizeof(OPTR);
		/* cannot fix fh->ipdFirst since it is a short */
#define IPDFIRST	((long)hdr->cbPdOffset + fh->ipdFirst * sizeof(PDR))
		fh->iauxBase = hdr->cbAuxOffset + fh->iauxBase * sizeof(AUXU);
		fh->rfdBase = hdr->cbRfdOffset + fh->rfdBase * sizeof(FDR);
		fh->cbLineOffset += hdr->cbLineOffset;
		fh->rss = (long)fh->rss + fh->issBase;


		/* fixup local symbols */
		for (s_idx = 0; s_idx < fh->csym; s_idx++) {
			sh = (SYMR*)(fh->isymBase + s_idx * sizeof(SYMR));
			sh->iss = (long) sh->iss + fh->issBase;

			/* fixup the index */
			switch (sh->st) {
			case stNil:
			case stFile:
			case stLabel:
			case stBlock:
			case stEnd:
				break;
			default:
				if (sh->index != indexNil) {
					/* this is only 20 bits wide */
					sh->index = (fh->iauxBase - (unsigned)data) +
						sh->index * sizeof(AUXU);
				}
			}
		}
		/* forget about line numbers */
		/* forget about opt symbols */
		/* fixup procedure symbols */
		if (fh->isymBase == 0) { /* cover up for a linker bug */
			ld_screwed_up++;
			fh->cpd = 0;
		}
		for (s_idx = 0; s_idx < fh->cpd; s_idx++) {
			pr = (PDR*)(IPDFIRST + s_idx * sizeof(PDR));
			pr->isym = fh->isymBase + pr->isym * sizeof(SYMR);
			pr->cbLineOffset += fh->cbLineOffset;
		}		
		/* forget about everything else */
	}

	/* fixup external symbols */
	for (s_idx = 0; s_idx < hdr->iextMax; s_idx++) {
		esh = (EXTR*)(hdr->cbExtOffset + s_idx * sizeof(EXTR));
		esh->ifd = (hdr->cbFdOffset - (unsigned)data) + esh->ifd * sizeof(FDR);
#define IFD(x) ((FDR*)((x)->ifd + data))
		esh->asym.iss = esh->asym.iss + hdr->cbSsExtOffset;

		/* fixup the index */
		switch (esh->asym.st) {
		case stNil:
		case stFile:
		case stLabel:
		case stBlock:
		case stEnd:
			break;
		default:
			if (esh->asym.index != indexNil)
				esh->asym.index = (IFD(esh)->iauxBase - (unsigned)data) +
					esh->asym.index * sizeof(AUXU);
		}
	}

	/* fixup procedure symbols for files linked with "-x" */
	if (ld_screwed_up && (hdr->ifdMax == 1) && fh->rss == -1) {
		for (s_idx = 0; s_idx < hdr->ipdMax; s_idx++) {
			pr = (PDR*)(hdr->cbPdOffset + s_idx * sizeof(PDR));

			if (pr->isym == -1) {/* static procedure */
				/* HACK, FIXME!! */
#if 0
				sh = (SYMR*)malloc(sizeof(SYMR));
				*sh = static_procedure_symbol;
#else
				sh = (SYMR*)&static_procedure_symbol;
#endif
				pr->isym = (long)sh;
				sh->value = pr->adr;
			} else {
				pr->isym = hdr->cbExtOffset + pr->isym * sizeof(EXTR);
				pr->isym = (long) &(((EXTR*)pr->isym)->asym);
			}

			pr->cbLineOffset += hdr->cbLineOffset;
		}
	}
}
