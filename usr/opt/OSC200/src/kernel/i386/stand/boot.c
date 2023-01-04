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
static char	*sccsid = "@(#)$RCSfile: boot.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:14:47 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */

/* 
 * Mach Operating System
 * Copyright (c) 1990 Carnegie-Mellon University
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
 
/*
 * 			INTEL CORPORATION PROPRIETARY INFORMATION
 *
 *	This software is supplied under the terms of a license  agreement or 
 *	nondisclosure agreement with Intel Corporation and may not be copied 
 *	nor disclosed except in accordance with the terms of that agreement.
 *
 *	Copyright 1988 Intel Corporation
 * Copyright 1988, 1989 by Intel Corporation
 */

#include "small.h"
#include "boot.h"

#include "coff.h"
#include <a.out.h>
#include "saio.h"

#include <sys/reboot.h>

#ifndef	AOUT
#define	AOUT=1
#endif	AOUT
#ifndef	COFF
#define	COFF=1
#endif	COFF

#define	KERNEL_BOOT_ADDR	0x100000	/* load at 1M */

#ifdef	SMALL
#define	Gets(buf,to)	gets(buf)
#endif	SMALL

union {
	struct headers {
		struct filehdr mainhdr ;
		struct aouthdr secondhdr ;
	} c_h ;

	struct exec a_h;
} head;

struct scnhdr  section;

int	cnvmem;		/* size of the convetional memory in KB */
int	extmem;		/* size of the extended memory in KB */


char	buf[8192];
int	bufsize = 8192;

/* Path name of kernel. */
#define NAME_LENGTH	100
char	name[NAME_LENGTH];
char	*default_names[] = {
	"/mach",
#ifndef	SMALL
	"/RFS/.LOCALROOT/mach",
	"/vmunix",
	"/RFS/.LOCALROOT/vmunix",
	"/RFS/.LOCALROOT/mach.old"
#endif	SMALL
};
#define NUM_DEFAULT_NAMES	(sizeof(default_names)/sizeof(char *))
int	current_default = 0;

typedef unsigned long	entry_t;
int	argv[10];

int	retry;
int	esym;
extern int openfirst;	/* zero out file table flag */
extern int debug;

entry_t loadprog(), 
	loadaout(), 
	loadcoff();

#define	Slice(x)	(((x)>>B_PARTITIONSHIFT)&B_PARTITIONMASK)
#define	Dev(x)		(((x)>>B_TYPESHIFT)&B_TYPEMASK)

boot(bootdev)
	int		bootdev;
{
	entry_t		entry;
	int		howto;
	extern int	unit_part_dev;
	int		fd;
	extern char 	end, edata;

	retry = 0; esym = 0;
	bzero(&edata, &end-&edata);


	/* get the size of the extended memory */
	extmem = memsize(1);
	/* get the size of the conventional memory */
	cnvmem = memsize(0);

#ifndef	SMALL
	printf("\n>> Mach i386 boot\n");
	printf("\n>> %d/%d k of memory\n", 
		cnvmem, extmem);
#endif	SMALL

	/* turn on gate A20 to be able to access memory above 1 MB */
	setA20();

	/* bootdev <= slice<<8 | major */
	unit_part_dev = bootdev;

	/*  parse args */
	for (;;) {
		howto = KERNEL_BOOT_ADDR;	/* kernel load locn */

		/* 
		 * If the user hasn't given a path name, try one of 
		 * the defaults.  If we reach the end of the default 
		 * list, go back to the start.  (Might be better to 
		 * just wait for user to type in a working name?)
		 * 
		 * Assume that none of the default names is too long.
		 */
		strcpy(name, default_names[current_default]);
#ifndef	SMALL
		if (++current_default == NUM_DEFAULT_NAMES)
			current_default = 0;
#endif	SMALL

		while ( parse_args(&howto,Dev(unit_part_dev)) == -1 )
			/* be persistant */;

		if ((fd = open(name, 0)) == -1) {
			printf("boot: can't find %s\n", name);
		} else {
			if (iob[fd-3].i_ino.i_mode&IEXEC)
				howto |= RB_KDB;

			/* perform the actual load */
			entry = loadprog(howto, unit_part_dev, fd);
			close(fd);
			if ( entry == (entry_t)-1)
				continue;

			if (Dev(unit_part_dev) == DEV_FLOPPY) {
				puts("\n\nInsert file system \n");
				getchar();
			}

#if	DEBUG
			printf("launching entry=%x\n", entry);
			sleep(3);
#endif	DEBUG
			startprog(entry, argv);

		}
	}

}


entry_t loadprog(howto, dev, fd)
	int		howto;
	int		dev;
	int		fd;
{

	/* get file header */
	read(fd, &head, sizeof(head));

#if	COFF
	/* check magic number */
	if (head.c_h.mainhdr.f_magic == I386MAGIC ) {
		return loadcoff(&head, howto, dev, fd);
	} else
#endif	COFF
#if	AOUT
	if (head.a_h.a_magic == ZMAGIC 
	|| head.a_h.a_magic == NMAGIC
	|| head.a_h.a_magic == OMAGIC) {
		return loadaout(&head, howto, dev, fd);
	} else
#endif	AOUT
	{
		puts("boot: invalid format!\n");
		return -1;
	}

}

int xread(fd, addr, size)
	int		fd;
	char		* addr;
	int		size;
{
	char		* orgaddr = addr;
	long		offset;
	unsigned	count, pos;
	long		max;

	/* align your read to increase speed */
	offset = tell(fd) & 4095;
	if ( offset != 0)
		max = 4096 - offset;
	else
		max = bufsize;

	while (size > 0) {

		if (size > max)
			count = max;
		else
			count = size;

		if ( read(fd, buf, count) != count)
			break;

		pcpy(buf, addr, count);
		size -= count;
		addr += count;

		max = bufsize;
	}

	return addr-orgaddr;
}

#if	COFF
entry_t loadcoff(head, howto, dev, fd)
	struct headers	* head;
	int		fd;
{
	int		nheader, flag;
	entry_t		entry;
	long		offset, addr, loc, size;
	int		hcnt = 0;

	/* get section headers and load image */
	offset = sizeof(*head);
	nheader = head->mainhdr.f_nscns;
	entry = head->secondhdr.entry & 0xfffffff;
	printf("%x ", entry);
	while (nheader-- > 0) {
		lseek(fd, offset, 0);
		read(fd, &section, sizeof(struct scnhdr));
		flag = section.s_flags & 0xff;
		/* load image */
		if ((flag == STYP_TEXT) || (flag == STYP_DATA)) {
			/* physical addr = virtual addr without top nibble */ 
			addr = section.s_vaddr & 0xfffffff;
			size = section.s_size;
			loc = section.s_scnptr;

			printf("%s%d", (hcnt++ == 0) ? "" : "+", size);
			lseek(fd, loc, 0);
			xread(fd, addr, size);
		}
		offset += sizeof(struct scnhdr);
	}
	printf("\n");

	{
		static int (*x_entry)() = 0;
		extern int argv[];

		argv[1] = howto;
		argv[2] = dev;
		argv[3] = 0;	/* ssym */
		argv[4] = 0;	/* esym */
		argv[5] = entry;
		argv[6] = (int)&x_entry;
		argv[7] = cnvmem;
		argv[8] = extmem;
		argv[0] = 8;
	}

	return entry;
}
#endif	COFF


#if	AOUT
/*ARGSUSED*/
entry_t loadaout(head, howto, dev, io)
	struct exec * head;
	register howto, dev, io;	/* howto=r11, devtype=r10 */
{
	struct exec x;
	int i;
	char *addr;
	register char *ssym = 0;
	char *base = (char *) (howto & 0xffff0000); /* high 16 bits */
	addr = base;

	x = *head;

	printf("%d", x.a_text);
#ifdef	vax
	if (x.a_magic == 0413 && lseek(io, 0x400, 0) == -1)
		goto shread;
	else
#endif	vax

	/* strip off header for OMAGIC files */
	if (x.a_magic == 0407 && lseek(io, 0x20, 0) == -1)
		goto shread;
	else
	if (x.a_magic == 0413 ) {
		lseek(io, 0L, 0);
		x.a_text += sizeof(struct exec);
	}

	xread(io, addr, x.a_text);
	addr += x.a_text;
	printf("+%d", x.a_data);
	if (xread(io, addr, x.a_data) != x.a_data)
		goto shread;
	addr += x.a_data;
	printf("+%d", x.a_bss);

	if ((howto&RB_KDB) && x.a_syms)
	{
		pzero(addr, x.a_bss);
		addr += x.a_bss;
		ssym = addr;
		pcpy(&x.a_syms, addr, sizeof(x.a_syms));
		addr += sizeof(x.a_syms);
		printf("[+%d", x.a_syms);
		if (xread(io, addr, x.a_syms) != x.a_syms)
			goto shread;
		addr += x.a_syms;
		if (read(io, &i, sizeof(int)) != sizeof(int))
			goto shread;
		pcpy(&i, addr, sizeof(int));
		i -= sizeof(int);
		addr += sizeof(int);
		printf("+%d]", i);
		if (xread(io, addr, i) != i)
			goto shread;
		addr += i;
		esym = ((int)(addr+sizeof(int)-1))&~(sizeof(int)-1);
		x.a_bss = 0;
	}
	else
	{
		howto &= ~RB_KDB;
	}

	printf("\n");


	x.a_entry &= 0xfffffff;

	io = esym;	/* XXX */
	/*
	 *  We now pass the various bootstrap parameters to the loaded
	 *  image via the argument list (as well as in the registers for
	 *  compatibility). 
	 *
	 *  arg1 = boot flags [R11]
	 *  arg2 = boot device [R10]
	 *  arg3 = start of symbol table (0 if not loaded)
	 *  arg4 = end of symbol table (0 if not loaded) [R9]
	 *  arg5 = transfer address from image (for indirect transfers)
	 *  arg6 = transfer address for next image pointer (a previously
	 *	   loaded program will use this to cause the transfer to the
	 *	   next loaded program to first indirect through it, such as
	 *	   when using the debugger).
	 */
	{
		static int (*x_entry)() = 0;
		extern int argv[];

		argv[1] = howto;
		argv[2] = dev;
		argv[3] = (int) ssym;
		argv[4] = esym;
		argv[5] = (int) x.a_entry;
		argv[6] = (int) &x_entry;
		argv[7] = cnvmem;
		argv[8] = extmem;
		argv[0] = 8;

		if (x_entry)
			return (int) x_entry;
		else
			return (int) x.a_entry;
	}
shread:
	printf("Short read\n");
	return -1;
}
#endif	AOUT

skipblank(cp) 
	char * * cp;
{
	/* skip leading space */
	while (**(cp) && (**(cp) == ' ' || **(cp) == '\t' || **(cp) == '\n'))
		++(*(cp));
}

parse_args(howto,dev)
	int		* howto;
	int		dev;
{
	char		line[NAME_LENGTH];
	char		* cp = line;
	char		* namep;

	puts("\nboot: ");
	/* If the user types too many characters, bad things may happen. */
	if ( Gets(line, 12) == NULL ) {
#ifndef	SMALL
		printf("\nBooting %s()%s\n", 
			devsw[dev].dv_name, name);
#endif	SMALL
		line[0] = '\0';
	}

	while (*cp) {
		skipblank(&cp);
		if (!*cp)
			return 0;
		else if (*cp == '-') {
			++cp;
			while (*cp) {
				switch (*cp) {
				    case 'a':
					*howto |= RB_ASKNAME;
					break;
				    case 's':
					*howto |= RB_SINGLE;
					break;
				    case 'd':
					*howto |= RB_KDB;
					break;
				    case 'D':
					debug++;
					break;
				}
				++cp;
			}
		} else {
			/* file name */
			namep = name;
			while (*cp && (*cp != ' ' && *cp != '\t' && *cp != '\n'))
				*namep++ = *cp++;
			*namep = '\0';
		}
	}

	return 0;
}
