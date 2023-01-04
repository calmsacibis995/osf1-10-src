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
static char	*sccsid = "@(#)$RCSfile: fsdb_sec.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:05:42 $";
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
 * Copyright (c) 1988 SecureWare, Inc.
 * All Rights Reserved.
 */



#ifdef STANDALONE
#define FsTYPE	2
#endif

#include	"sys/secdefines.h"
#if SEC_FSCHANGE
#include	"sys/types.h"
#include	"sys/param.h"
#include	"signal.h"
#include	"stdio.h"
#include	"setjmp.h"

#include	"sys/security.h"

#ifdef AUX
#include	"sys/time.h"
#include	"sys/vnode.h"
#include	"svfs/inode.h"
#include	"svfs/fsdir.h"
#include	"svfs/filsys.h"
#endif

#ifdef _OSF_SOURCE
#include	"ufs/inode.h"
#include	"ufs/dinode.h"
#include	"ufs/fs.h"
#endif

#ifdef SYSV
#include	"sys/fs/s5inode.h"
#include	"sys/inode.h"
#include	"sys/dir.h"
#include	"sys/ino.h"
#ifdef SYSV_3
#include	"sys/fs/s5filsys.h"
#else
#include	"sys/filsys.h"
#endif
#include	"sys/audit.h"
#include	"prot.h"
#include	"protcmd.h"
#endif


/*
 * Definitions for offset values of new fields in the dinode structure.
 */
#if defined(AUX) || defined(SYSV)
#define	TYP	124
#define	PV	64
#define	N_PV	4
#define	T0	80
#define	N_T	10
#define	PAR	120
#endif

/*
 * Reference implementation for BSD FFS (not OSF)
 */
#ifdef NOTDEF_BSD_FFS
#define PV	128
#define N_PV	4
#define T0	144
#define N_T	7
#define PAR	172
#define TYP	176
#ifdef hpux
#define OBJ_BYTE	1	/* Read/put a byte [8 bits] */
#define OBJ_WORD	2	/*  a word [16 bits] */
#define OBJ_LONG	3	/*  a long [32 bits] */
#define OBJ_DIR		4	/*  a directory entry [variable > 12] */
#define OBJ_INO		5	/*  an inode entry [sizeof(struct dinode)] */
#endif
#endif


extern long cur_ino;
extern long addr;
extern short objsz;
extern short prt_flag;
extern short error;
#ifdef AUX
extern struct filsys fs;
#endif


extern long get();
extern long getnumb();



#ifdef SYSV /*{*/
/*
 * For those cases where the superblock is not read in (the file system
 * is assumed to be a particular type, we instead have to read it in
 * because we need to know if the file system is secure or not.
 */
void
fsdb_preset_type(disk_fd, fs, offset, arguments)
	int disk_fd;
	struct filsys *fs;
	long offset;
	char *arguments[];
{
	if ((lseek (disk_fd, offset, 0) < 0) ||
	    (read (disk_fd, fs, sizeof(*fs)) != sizeof(*fs)))  {
		audit_subsystem(
		     "read superblock to find if file system is secure or not",
		     "cannot read superblock", ET_SUBSYSTEM);
		printf("%s: cannot read superblock\n", arguments[0]);
		exit(1);
	}
}


/*
 * Determine the file system type.  Report the type in English and
 * also report when the type is unknown.  Once we know, we set up
 * the disk parameters based on the type.
 */
void
fsdb_determine_type(fs, arguments, disk_fd)
	struct filsys *fs;
	char *arguments[];
	int disk_fd;
{
	switch (fs->s_type)  {
		case Fs1b:
		case Fs2b:
			break;

		case Fs1b | FsSW:
			printf("%s(%s): 512 byte Block Secure File System\n",
			       arguments[1], fs->s_fname);
			close(disk_fd);
			if (execvp("/etc/fsdb1b", arguments) < 0) {
				/*printf("%s: cannot exec /etc/fsdb1b\n",
					 arguments[0]);*/
				perror(arguments[0]);
				exit(1);
			}
			break;

		case Fs2b | FsSW:
			printf("%s(%s): 1K byte Block Secure File System\n",
			       arguments[1], fs->s_fname);
			break;

		default:
			audit_subsystem("determine file system type",
				"type unknown, fsdb abort", ET_SUBSYSTEM);
			printf("%s: Invalid File System Type\n", fs->s_fname);
			exit(1);
			break;
	}

	disk_set_file_system(fs, BSIZE);
}
#endif /*}*/


#if defined(AUX) || defined(SYSV) /*{*/
/*
 * Understand the different inode sizes in determining resetting error
 * conditions.
 */
int
fsdb_large_size_error_cleanup()
{
	int known_size = 0;

	if (objsz == disk_dinode_size()) {
		fprnt('i', 1);
		cur_ino = addr;
		prt_flag = 0;
		known_size = 1;
	}

	return known_size;
}


/*
 * Understand the different inode sizes in reading disk information.
 */
int
fsdb_large_size_read(object_size, pvtemp, bptr)
	short object_size;
	long *pvtemp;
	char *bptr;
{
	int known_size = 0;

	if (object_size == disk_dinode_size()) {
		loword(*pvtemp) = *(short *)bptr;
		known_size = 1;
	}

	return known_size;
}


/*
 * Understand the different inode sizes in writing disk information.
 */
int
fsdb_large_size_write(object_size, bptr, item)
	short object_size;
	char *bptr;
	long item;
{
	int code;

	if (object_size == disk_dinode_size()) {
		if (item & ~0177777L)
			code = 2;
		else  {
			*(short *)bptr = item;
			code = 0;
		}
	}
	else
		code = 1;

	return code;
}


/*
 * Understand the different inode sizes in aligning data.
 */
int
fsdb_large_size_allign(ment, addr)
	short ment;
	long addr;
{
	int already_alligned = 0;

	if (ment == disk_dinode_size())
		already_alligned = !(addr & 01L);

	return already_alligned;
}


/*
 * Process SecureWare specific commands.
 *
 * Commands:
 *	P0-3	The privilege vectors of the inode.
 *	t0-12	The security policy tags.
 *	typ	The inode secure type flags word.
 */
void
fsdb_command(key, ino, addr, type)
	char key;
	long ino;
	long *addr;
	short *type;
{
	register long value;
	register char next_char;

	if (disk_secure_file_system())
	    switch (key)  {
		case 'P': /* Privileges and MLD parent */
			/* Privileges (P0-P3) or MLD parent inode (PAR) */
			next_char = getc(stdin);
			if (next_char == 'A') {
				if ((next_char = getc(stdin)) != 'R') {
					error++;
					break;
				}
				*addr = ino + PAR;
#ifdef _OSF_SOURCE
				value = get(sizeof(long));
#else
				value = get(sizeof(ino_tl));
#endif
				*type = PAR;
				break;
			}

			/* privileges */
			ungetc(next_char, stdin);
			value = getnumb();
			if (error || (value >= N_PV)) {
				error++;
				break;
			}
			*addr = ino + PV + (value * sizeof(long));
#ifdef hpux
			value = get(OBJ_LONG);
#else
			value = get(sizeof(priv_t));
#endif
			*type = PV;
			break;

		case 't': /* type flags (e.g., multilevel dir) */
			next_char = getc(stdin);
			if (next_char == 'y') {
				if((next_char = getc(stdin)) != 'p') {
					error++;
					break;
				}
				*addr = ino + TYP;
#ifdef hpux
				value = get(OBJ_WORD);
#else
				value = get(sizeof(ushort));
#endif
				*type = TYP;
				break;
			}

			/* security policy tags */
			ungetc(next_char, stdin);
			value = getnumb();
			if (error || (value >= N_T)) {
				error++;
				break;
			}
			*addr = ino + T0 + (value * sizeof(long));
#ifdef hpux
			value = get(OBJ_LONG);
#else
			value = get(sizeof(long));
#endif
			*type = T0;
			break;

		default:
			error++;
			break;

	    }
	    else
		error++;
}
#endif /*}*/


#ifdef SYSV /*{*/
/*
 * Based on the address and offset given, determine the place
 * where the inode resides and the inode number.  Also, get the
 * size of the inode.
 */
void
fsdb_inode_calc(ip, addr, offset, objsz, temp)
	struct dinode **ip;
	long addr;
	short offset;
	short *objsz;
	long *temp;
{
	int inode_size = disk_dinode_size();

	*objsz = inode_size;
	*temp = (addr - (BSIZE * 2)) / inode_size + 1;
	disk_inode_incr(ip, offset);
}
#endif /*}*/


#if defined(AUX) || defined(SYSV)
/*
 * Print the SecureWare specific portions of the inode, namely the
 * privilege vectors, the policy tags and the type word.
 */
void
fsdb_extended_inode_print(ip)
#if defined(AUX)
	register struct sec_dinode *ip;
#else
	register struct dinode *ip;
#endif
{
	register int i;

	if (disk_secure_file_system()) {
		printf("P0: %8lx(g)  P1: %8lx     P2: %8lx(p)  P3: %8lx\n",
			ip->di_gpriv[0], ip->di_fill1[0],
			ip->di_ppriv[0], ip->di_fill2[0]);
	
		for (i = 0; i < SEC_TAG_COUNT; i++)  {
			printf("t%d: %8lx     ", i, ip->di_tag[i]);
			if (i == 3)
				putc('\n', stdout);
		}
		putc('\n', stdout);
	
		for (i = 0; i < sizeof ip->di_fill3 / sizeof ip->di_fill3[0];
					++i) {
			printf("t%d: %8lx     ",
				i + SEC_TAG_COUNT, ip->di_fill3[i]);
		}
		putc('\n', stdout);

		printf("typ: %7d     ", ip->di_type_flags);
		printf("PAR: %7d     ", ip->di_parent);
		putc('\n', stdout) ;
	}
}
#endif /* AUX || SYSV */

#ifdef _OSF_SOURCE
/*
 * Print the SecureWare specific portions of the inode, namely the
 * privilege vectors, the policy tags and the type word.
 */
void
fsdb_extended_inode_print(ip)
	register struct sec_dinode *ip;
{
	register int i;
	register struct dinode_sec *dip;

	if (disk_secure_file_system()) {
		dip = &ip->di_sec;
		printf("gra0: %8lx  gra1: %8lx  pot0: %8lx  pot1: %8lx\n",
			dip->di_gpriv[0], dip->di_gpriv[1],
			dip->di_ppriv[0], dip->di_ppriv[1]);
	
		for (i = 0; i < SEC_TAG_COUNT; i++)  {
			printf("tag%d: %8lx  ", i, dip->di_tag[i]);
			if (i == 3)
				putc('\n', stdout);
		}
		putc('\n', stdout);
	
		printf("tflag: %7d  ", dip->di_type_flags);
		printf("parent: %6d  ", dip->di_parent);
		putc('\n', stdout) ;
	}
}
#endif /* _OSF_SOURCE */
#endif
