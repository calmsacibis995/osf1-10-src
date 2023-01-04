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
static char	*sccsid = "@(#)$RCSfile: kdb.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:18:33 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

#include <sys/param.h>
#include <sys/reboot.h>
#include <sys/systm.h>
#include <sys/vmmac.h>

#include <i386/pcb.h>
#include <i386/psl.h>
#include <i386/reg.h>
#include <i386/trap.h>

#include <mach/vm_param.h>
#include <vm/vm_map.h>
#include <vm/vm_kern.h>
#include <sys/user.h>		/* for u.u_procp  */
#include <sys/proc.h>
#include <kern/task.h>
#include <kern/thread.h>

extern char *kdbsbrk();

int   play_back_size = 0x10000;
char *play_back_buf = (char *) 0;
char *play_back_ptr = (char *) 0;

/*
 *  kdb_init - initialize kernel debugger:
 *	needs kmem_alloc_pageable() -> wait till after
 *	vm_mem_init() in startup()
 */
kdb_init()
{
#ifdef	wheeze
#else	wheeze
	extern char *esym;
	extern int end;

	if (esym > (char *)&end) {
	    int len = end;
	    char *ss = (char *)&end+sizeof(int);
	    char *es = ss+len;
	    int tablen = *((int *)(es));
	    extern char *esym;

	    printf("end %x, sym %x(%x) str = %x(%x)\n",
		   &end, ss, len, es, tablen);

	    if ((es+((tablen+sizeof(int)-1)&~(sizeof(int)-1))) == esym) {
		printf("[ preserving %d bytes of symbol table ]\n",
		   esym-ss);
		kdbsetsym(ss, es, es); 
	    } else
		printf("[ no valid symbol table present ]\n");
	}

	play_back_buf = (char *)kmem_alloc_pageable(kernel_map, play_back_size);
	play_back_ptr = play_back_buf;
	
	pmap_map(play_back_buf,
	 	     mem_size,
		     round_page(mem_size + play_back_size),
		     VM_PROT_READ | VM_PROT_WRITE);

#endif	wheeze
}

/*
 *  kdbsbrk - extended debugger dynamic memory
 */
static char kdbbuf[1024];
char *kdbend = kdbbuf; 

char *
kdbsbrk(n)
unsigned n;
{
    char *old = kdbend;

    if ((kdbend+n) >= &kdbbuf[sizeof(kdbbuf)])
    {
	return((char *)-1);
    }
    kdbend += n;
    return(old);
}

/*
 *	Reads or writes a longword to/from the specified address
 *	in the specified map.  The map is forced to be the kernel
 *	map if the address is in the kernel address space.
 *
 *	Returns 0 if read/write OK, -1 if not.
 */

int kdbreadwrite(map, addr, value, rd)
	vm_map_t	map;
	vm_offset_t	addr;
	long		*value;	/* IN/OUT */
	boolean_t	rd;
{
	int ret;

	if (rd)
		ret = kdbrlong(addr, value);
	else
		ret = kdbwlong(addr, value);
	return (!ret ? ret : -1);
}


extern	short	kdbecho;
short	kdbpagelength = 25;	/* 25 rows on atconsole */
short	kdbpagewidth = 80;	/* 80 columns on atconsole */
static	short	colcount, linecount;
static	short	kdbflusho;


/*
 * Save typed characters for later playback
 */
play_record(c)
{
	if (play_back_size) {
		if (play_back_ptr >= &play_back_buf[play_back_size])
			play_back_ptr = play_back_buf;
		*play_back_ptr++ = c;
	}
}

/*
 *  kdbread - read character from input queue
 */
kdbread(x, cp, len)
register char *cp;
{
	register int c;

	kdbflusho = 0;
        do {
                c = cngetc();
        } while (c == -1);

        if (c == '\r')
                c = '\n';

	play_record(c);

	*cp = c;

	colcount = linecount = 1;
	if (kdbecho)
		kdbwrite(x, cp, 1);
        
        return(*cp != 04);
}

/*
 *  kdbwrite - send characters to terminal
 */
kdbwrite(x, cp, len)
register char *cp;
register int len;
{
#ifdef	__STDC__
   const
#endif
   static char pagemodemsg[] =
          "\r[PAGE MODE: Press any key to continue, 'q' to flush output] "; 

    while (len--) {
	if (!kdbflusho) {
		if (*cp == '\n' ||	/* count lines, including wrap */
		    (kdbpagewidth && ++colcount > kdbpagewidth)) {
			colcount = 1;
			linecount++;
		}
                play_record(*cp);
		cnputc(*cp++);
	}
	else
		cp++;
	if (kdbpagelength && (linecount >= kdbpagelength)) {
		int c;
		int count = 0;
		while (count < sizeof(pagemodemsg))
			cnputc(pagemodemsg[count++]);
		do c = cngetc(); while (c == 0); /* swallow 1 character. */
		if ( c == '\f') {
			cnputc(c);
		} else {
			count = 0;
			cnputc('\r');
			while (count++ < sizeof(pagemodemsg))
				cnputc(' ');
			cnputc('\r');
		}
		colcount = 0;
		switch (c) {
		case 'q': case 'Q':
			kdbflusho = 1;
			/* fall through */
		case ' ': case 'f': case 'F': case ('F' & 0x1f):
			linecount = 1;
			break;
		case '\r': case 'j': case 'J': case '\n':
			linecount--;
			break;
		default:
			linecount /= 2;
			break;
		}
	}
    }
}


/*
 * Playback saved charaters
 */
pb(a1, a2)
{
register char *pp = play_back_buf;
register int c;

	if (a1 == -1) {
		play_back_ptr = play_back_buf;
		return;
	}

	while (c = *pp++) {
		if (pp >= &play_back_buf[play_back_size])
			pp = play_back_buf;
		if (c != '\n')
		 	cnputc(c);
		else {
			c = cngetc();
			if (c == ' ')
				cnputc('\n');
			else if (c == '\n')
				/* overwrite */;
			else
				return;
		}
	}
}
