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
static char	*sccsid = "@(#)$RCSfile: arp.c,v $ $Revision: 4.2.7.4 $ (DEC) $Date: 1993/12/20 15:11:24 $";
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
 * Copyright (c) 1984 Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Sun Microsystems, Inc.
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
char copyright[] =
"@(#) Copyright (c) 1984 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

/*
 * arp - display, set, and delete arp table entries
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>

extern priv_t *privvec();
static privvec_t saveprivs;
#endif

#if	!MACH
#include <machine/pte.h>

#include <sys/param.h>
#include <sys/vmmac.h>
#else
#include <sys/param.h>
#endif
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/if_ether.h>

#include <errno.h>
#include <nlist.h>
#include <stdio.h>
#include <paths.h>

extern int errno;
static int kflag;
static int canonical = 1;

void haddr_convert();

main(argc, argv)
	int argc;
	char *argv[];
{
	int ch;
	int nomatch = 1;

#if SEC_BASE
	set_auth_parameters(argc, argv);
	initprivs();
#endif

	if (argc < 2){
                usage();
	}

	while ((ch = getopt(argc, argv, "adsfu")) != EOF)
		switch((char)ch) {
		case 'a': {
			char *mem = NULL;
			u_short n = 0;
			
		        nomatch = 0;
			if (canonical) {
			    ch = getopt(argc, argv, "u");
			    if (ch == 'u') {
			        canonical = 0;
				n = 1;
			    }
			 }

			 if (argc > 4 + n)
			     usage();
			 if (argc == (4 + n)) {
			     kflag = 1;
			     mem = argv[3 + n];
			 } else
			     mem = _PATH_KMEM;

			if (canonical)
		            dump((argc >= 3) ? argv[2] : _PATH_UNIX, mem);
			else 
		            dump((argc >= 4) ? argv[3] : _PATH_UNIX, mem);
			break;
		}
		case 'd':
			if (getuid())
			    privilege();
			nomatch = 0;
			if (argc != 3)
				usage();
			delete(argv[2]);
		        break;
		case 's':
			if (getuid())
			    privilege();
			nomatch = 0;
			if (argc < 4 || argc > 8)
				usage();
			if (canonical) {
			    ch = getopt(argc, argv, "u");
			    if (ch == 'u') {
			       canonical = 0;
			       set(argc-3, &argv[3]) ;
		               break;
			    }
			} 
			set(argc-2, &argv[2]) ;
			break;
		case 'f':
			if (getuid())
			    privilege();
			nomatch = 0;
			if (argc != 3)
				usage();
			file(argv[2]) ;
			break;
		case 'u':
			canonical = 0;
			continue;
		case '?':
		default:
			nomatch = 0;
			usage();
		}
	if (nomatch) {
            if (argc == 2)
	       get(argv[1]);
	    else if (!canonical)
	        if (argc == 3)
	           get(argv[2]);
	}
	exit(0);
}

/*
 * Process a file to set standard arp entries
 */
file(name)
	char *name;
{
	FILE *fp;
	int i, retval;
	char line[100], arg[5][50], *args[5];

	if ((fp = fopen(name, "r")) == NULL) {
		fprintf(stderr, "arp: cannot open %s\n", name);
		exit(1);
	}
	args[0] = &arg[0][0];
	args[1] = &arg[1][0];
	args[2] = &arg[2][0];
	args[3] = &arg[3][0];
	args[4] = &arg[4][0];
	retval = 0;
	while(fgets(line, 100, fp) != NULL) {
		i = sscanf(line, "%s %s %s %s %s", arg[0], arg[1], arg[2],
		    arg[3], arg[4]);
		if (i < 2) {
			fprintf(stderr, "arp: bad line: %s\n", line);
			retval = 1;
			continue;
		}
		if (set(i, args))
			retval = 1;
	}
	fclose(fp);
	return (retval);
}

/*
 * Set an individual arp entry 
 */
set(argc, argv)
	int argc;
	char **argv;
{
	struct arpreq ar;
	struct hostent *hp;
	struct sockaddr_in *sin;
	u_char *ea;
	int s;
	char *host = argv[0], *eaddr = argv[1];

	argc -= 2;
	argv += 2;
	bzero((caddr_t)&ar, sizeof ar);
	sin = (struct sockaddr_in *)&ar.arp_pa;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(host);
	if (sin->sin_addr.s_addr == -1) {
		if (!(hp = gethostbyname(host))) {
			fprintf(stderr, "arp: %s: ", host);
			herror((char *)NULL);
			return (1);
		}
		bcopy((char *)hp->h_addr, (char *)&sin->sin_addr,
		    sizeof sin->sin_addr);
	}
	ea = (u_char *)ar.arp_ha.sa_data;
	if (ether_aton(eaddr, ea))
		return (1);
	ar.arp_flags = ATF_PERM;
	while (argc-- > 0) {
		if (strncmp(argv[0], "temp", 4) == 0)
			ar.arp_flags &= ~ATF_PERM;
		else if (strncmp(argv[0], "pub", 3) == 0)
			ar.arp_flags |= ATF_PUBL;
		else if (strncmp(argv[0], "trail", 5) == 0)
			ar.arp_flags |= ATF_USETRAILERS;
		argv++;
	}
	
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("arp: socket");
		exit(1);
	}
#if SEC_BASE
	raiseprivs(saveprivs);
#endif
	if (ioctl(s, SIOCSARP, (caddr_t)&ar) < 0) {
		perror(host);
		exit(1);
	}
#if SEC_BASE
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
	close(s);
	return (0);
}

/*
 * Display an individual arp entry
 */
get(host)
	char *host;
{
	struct arpreq ar;
	struct hostent *hp;
	struct sockaddr_in *sin;
	u_char *ea;
	int s;
	char *inet_ntoa();

	bzero((caddr_t)&ar, sizeof ar);
	ar.arp_pa.sa_family = AF_INET;
	sin = (struct sockaddr_in *)&ar.arp_pa;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(host);
	if (sin->sin_addr.s_addr == -1) {
		if (!(hp = gethostbyname(host))) {
			fprintf(stderr, "arp: %s: ", host);
			herror((char *)NULL);
			exit(1);
		}
		bcopy((char *)hp->h_addr, (char *)&sin->sin_addr,
		    sizeof sin->sin_addr);
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("arp: socket");
		exit(1);
	}
	if (ioctl(s, SIOCGARP, (caddr_t)&ar) < 0) {
		if (errno == ENXIO)
			printf("%s (%s) -- no entry\n",
			    host, inet_ntoa(sin->sin_addr));
		else
			perror("SIOCGARP");
		exit(1);
	}
	close(s);
	ea = (u_char *)ar.arp_ha.sa_data;
	printf("%s (%s) at ", host, inet_ntoa(sin->sin_addr));
	if (ar.arp_flags & ATF_COM)
		ether_print(ea);
	else
		printf("(incomplete)");
	if (ar.arp_flags & ATF_PERM)
		printf(" permanent");
	if (ar.arp_flags & ATF_PUBL)
		printf(" published");
	if (ar.arp_flags & ATF_USETRAILERS)
		printf(" trailers");
#ifdef	ATF_STALE
	if (ar.arp_flags & ATF_USE802)	/* not used */
		printf(" 802.3");
	if (ar.arp_flags & ATF_STALE)
		printf(" stale");
	if (ar.arp_flags & ATF_DEAD)
		printf(" not responding");
#endif
	printf("\n");
}

/*
 * Delete an arp entry 
 */
delete(host)
	char *host;
{
	struct arpreq ar;
	struct hostent *hp;
	struct sockaddr_in *sin;
	int s;

	bzero((caddr_t)&ar, sizeof ar);
	ar.arp_pa.sa_family = AF_INET;
	sin = (struct sockaddr_in *)&ar.arp_pa;
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = inet_addr(host);
	if (sin->sin_addr.s_addr == -1) {
		if (!(hp = gethostbyname(host))) {
			fprintf(stderr, "arp: %s: ", host);
			herror((char *)NULL);
			exit(1);
		}
		bcopy((char *)hp->h_addr, (char *)&sin->sin_addr,
		    sizeof sin->sin_addr);
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("arp: socket");
		exit(1);
	}
#if SEC_BASE
	raiseprivs(saveprivs);
#endif
	if (ioctl(s, SIOCDARP, (caddr_t)&ar) < 0) {
		if (errno == ENXIO)
			printf("%s (%s) -- no entry\n",
			    host, inet_ntoa(sin->sin_addr));
		else
			perror("SIOCDARP");
		exit(1);
	}
#if SEC_BASE
	seteffprivs(saveprivs, (priv_t *) 0);
#endif
	close(s);
	printf("%s (%s) deleted\n", host, inet_ntoa(sin->sin_addr));
}

struct nlist nl[] = {
#define	X_ARPTAB	0
	{ "_arptab" },
#define	X_ARPTAB_SIZE	1
	{ "_arptab_size" },
#if	!MACH
#define	N_SYSMAP	2
	{ "_Sysmap" },
#define	N_SYSSIZE	3
	{ "_Syssize" },
#endif
	{ "" },
};

#if	!MACH
static struct pte *Sysmap;
#endif

/*
 * Dump the entire arp table
 */
dump(kernel, mem)
	char *kernel, *mem;
{
	extern int h_errno;
	struct arptab *at;
	struct hostent *hp;
	int bynumber, mf, arptab_size, sz;
	char *host, *malloc();
	off_t lseek();
#if	MACH
	caddr_t arptabptr;
#endif

	if (nlist(kernel, nl) < 0 || nl[X_ARPTAB_SIZE].n_type == 0) {
		fprintf(stderr, "arp: %s: bad namelist\n", kernel);
		exit(1);
	}
	mf = open(mem, O_RDONLY);
	if (mf < 0) {
		fprintf(stderr, "arp: cannot open %s\n", mem);
		exit(1);
	}
#if	!MACH
	if (kflag) {
		off_t off;

		Sysmap = (struct pte *)
		   malloc((u_int)(nl[N_SYSSIZE].n_value * sizeof(struct pte)));
		if (!Sysmap) {
			fputs("arp: can't get memory for Sysmap.\n", stderr);
			exit(1);
		}
		off = nl[N_SYSMAP].n_value & ~KERNBASE;
		(void)lseek(mf, (off_t)off, L_SET);
		(void)read(mf, (char *)Sysmap,
		    (int)(nl[N_SYSSIZE].n_value * sizeof(struct pte)));
	}
#endif
	klseek(mf, (off_t)nl[X_ARPTAB_SIZE].n_value, L_SET);
	read(mf, &arptab_size, sizeof arptab_size);
#if	MACH
	if (arptab_size <= 0) {
#else
	if (arptab_size <= 0 || arptab_size > 1000) {
#endif
		fprintf(stderr, "arp: %s: namelist wrong\n", kernel);
		exit(1);
	}
	sz = arptab_size * sizeof (struct arptab);
	at = (struct arptab *)malloc((u_int)sz);
	if (at == NULL) {
		fputs("arp: can't get memory for arptab.\n", stderr);
		exit(1);
	}
	klseek(mf, (off_t)nl[X_ARPTAB].n_value, L_SET);
#if	MACH
	read(mf, &arptabptr, sizeof (arptabptr));
	klseek(mf, (off_t)arptabptr, L_SET);
#endif
	if (read(mf, (char *)at, sz) != sz) {
		perror("arp: error reading arptab");
		exit(1);
	}
	close(mf);
	for (bynumber = 0; arptab_size-- > 0; at++) {
		if (at->at_iaddr.s_addr == 0 || at->at_flags == 0)
			continue;
		if (bynumber == 0)
			hp = gethostbyaddr((caddr_t)&at->at_iaddr,
			    sizeof at->at_iaddr, AF_INET);
		else
			hp = 0;
		if (hp)
			host = hp->h_name;
		else {
			host = "?";
			if (h_errno == TRY_AGAIN)
				bynumber = 1;
		}
		printf("%s (%s) at ", host, inet_ntoa(at->at_iaddr));
		if (at->at_flags & ATF_COM)
			ether_print(at->at_enaddr);
		else
			printf("(incomplete)");
		if (at->at_flags & ATF_PERM)
			printf(" permanent");
		if (at->at_flags & ATF_PUBL)
			printf(" published");
		if (at->at_flags & ATF_USETRAILERS)
			printf(" trailers");
#ifdef	ATF_STALE
		if (at->at_flags & ATF_USE802)	/* not used */
			printf(" 802.3");
		if (at->at_flags & ATF_STALE)
			printf(" stale");
		if (at->at_flags & ATF_DEAD)
			printf(" not responding");
#endif
		printf("\n");
	}
}

/*
 * Seek into the kernel for a value.
 */
klseek(fd, base, off)
	int fd, off;
	off_t base;
{
	off_t lseek();

#if	!MACH
	if (kflag) {	/* get kernel pte */
		base &= ~KERNBASE;
		base = ctob(Sysmap[btop(base)].pg_pfnum) + (base & PGOFSET);
	}
#endif
	(void)lseek(fd, base, off);
}

ether_print(cp)
	u_char *cp;
{
	if (canonical)
	    printf("%02x-%02x-%02x-%02x-%02x-%02x", cp[0], cp[1], cp[2], cp[3], cp[4], cp[5]);
	else {
            haddr_convert(cp);
	    printf("%02x:%02x:%02x:%02x:%02x:%02x", cp[0], cp[1], cp[2], cp[3], cp[4], cp[5]);
	}
}

ether_aton(a, n)
	char *a;
	u_char *n;
{
	int i, o[6];
	
	i = sscanf(a, "%x:%x:%x:%x:%x:%x", &o[0], &o[1], &o[2],
					   &o[3], &o[4], &o[5]);
	if (i != 6) {
	
	   i = sscanf(a, "%x-%x-%x-%x-%x-%x", &o[0], &o[1], &o[2],
					   &o[3], &o[4], &o[5]);
	   if (i != 6) {
		fprintf(stderr, "arp: invalid Ethernet address '%s'\n", a);
		return (1);
	   } 
	}

	for (i=0; i<6; i++)
		n[i] = o[i];

	if (!canonical)
	    haddr_convert(n);

	return (0);
}

usage()
{
	printf("usage: arp [-u] hostname\n");
	printf("       arp -a [-u] [kernel] [kernel_memory]\n");
	printf("       arp -d hostname\n");
	printf("       arp -s [-u] hostname ether_addr [temp] [pub] [trail]\n");
	printf("       arp -f filename\n");
	exit(1);
}

#if SEC_BASE
/*
 * Check user's authorization to modify the ARP table, and raise the
 * required privilege to do it.
 */
raiseprivs(save)
	priv_t *save;
{
	static int	auth_checked = 0;
	privvec_t	needed;

	/*
	 * Only check authorization once to avoid multiple auditing
	 */
	if (!auth_checked) {
		auth_checked = 1;
		if (!authorized_user("sysadmin")) {
			fprintf(stderr, "arp: need sysadmin authorization\n");
			exit(1);
		}
		/* Initialize the needed privilege vector */
		setprivvec(needed, SEC_REMOTE, -1);
	}
	if (forceprivs(needed, save)) {
		fprintf(stderr, "arp: insufficient privileges\n");
		exit(1);
	}
	disablepriv(SEC_SUSPEND_AUDIT);
}
#endif
/*
 * haddr_convert()
 * --------------
 * 
 * Converts a backwards address to a canonical address and a canonical address
 * to a backwards address.
 *
 * Uses a sizeable translation table to do the conversion.  Longwords are used
 * in an attempt to make this routine fast since it will be called often.
 *
 * INPUTS:
 *  adr - pointer to six byte string to convert (unsigned char *)
 *
 * OUTPUTS:
 *  The string is updated to contain the converted address.
 *
 * CALLER:
 *  many
 *
 * USAGE OF OTHER ROUTINES:
 *
 */

unsigned long con_table[256] = {
0x00,   /* 0x00 */  0x80,   /* 0x01 */  0x40,   /* 0x02 */  0xc0,   /* 0x03 */
0x20,   /* 0x04 */  0xa0,   /* 0x05 */  0x60,   /* 0x06 */  0xe0,   /* 0x07 */
0x10,   /* 0x08 */  0x90,   /* 0x09 */  0x50,   /* 0x0a */  0xd0,   /* 0x0b */
0x30,   /* 0x0c */  0xb0,   /* 0x0d */  0x70,   /* 0x0e */  0xf0,   /* 0x0f */
0x08,   /* 0x10 */  0x88,   /* 0x11 */  0x48,   /* 0x12 */  0xc8,   /* 0x13 */
0x28,   /* 0x14 */  0xa8,   /* 0x15 */  0x68,   /* 0x16 */  0xe8,   /* 0x17 */
0x18,   /* 0x18 */  0x98,   /* 0x19 */  0x58,   /* 0x1a */  0xd8,   /* 0x1b */
0x38,   /* 0x1c */  0xb8,   /* 0x1d */  0x78,   /* 0x1e */  0xf8,   /* 0x1f */
0x04,   /* 0x20 */  0x84,   /* 0x21 */  0x44,   /* 0x22 */  0xc4,   /* 0x23 */
0x24,   /* 0x24 */  0xa4,   /* 0x25 */  0x64,   /* 0x26 */  0xe4,   /* 0x27 */
0x14,   /* 0x28 */  0x94,   /* 0x29 */  0x54,   /* 0x2a */  0xd4,   /* 0x2b */
0x34,   /* 0x2c */  0xb4,   /* 0x2d */  0x74,   /* 0x2e */  0xf4,   /* 0x2f */
0x0c,   /* 0x30 */  0x8c,   /* 0x31 */  0x4c,   /* 0x32 */  0xcc,   /* 0x33 */
0x2c,   /* 0x34 */  0xac,   /* 0x35 */  0x6c,   /* 0x36 */  0xec,   /* 0x37 */
0x1c,   /* 0x38 */  0x9c,   /* 0x39 */  0x5c,   /* 0x3a */  0xdc,   /* 0x3b */
0x3c,   /* 0x3c */  0xbc,   /* 0x3d */  0x7c,   /* 0x3e */  0xfc,   /* 0x3f */
0x02,   /* 0x40 */  0x82,   /* 0x41 */  0x42,   /* 0x42 */  0xc2,   /* 0x43 */
0x22,   /* 0x44 */  0xa2,   /* 0x45 */  0x62,   /* 0x46 */  0xe2,   /* 0x47 */
0x12,   /* 0x48 */  0x92,   /* 0x49 */  0x52,   /* 0x4a */  0xd2,   /* 0x4b */
0x32,   /* 0x4c */  0xb2,   /* 0x4d */  0x72,   /* 0x4e */  0xf2,   /* 0x4f */
0x0a,   /* 0x50 */  0x8a,   /* 0x51 */  0x4a,   /* 0x52 */  0xca,   /* 0x53 */
0x2a,   /* 0x54 */  0xaa,   /* 0x55 */  0x6a,   /* 0x56 */  0xea,   /* 0x57 */
0x1a,   /* 0x58 */  0x9a,   /* 0x59 */  0x5a,   /* 0x5a */  0xda,   /* 0x5b */
0x3a,   /* 0x5c */  0xba,   /* 0x5d */  0x7a,   /* 0x5e */  0xfa,   /* 0x5f */
0x06,   /* 0x60 */  0x86,   /* 0x61 */  0x46,   /* 0x62 */  0xc6,   /* 0x63 */
0x26,   /* 0x64 */  0xa6,   /* 0x65 */  0x66,   /* 0x66 */  0xe6,   /* 0x67 */
0x16,   /* 0x68 */  0x96,   /* 0x69 */  0x56,   /* 0x6a */  0xd6,   /* 0x6b */
0x36,   /* 0x6c */  0xb6,   /* 0x6d */  0x76,   /* 0x6e */  0xf6,   /* 0x6f */
0x0e,   /* 0x70 */  0x8e,   /* 0x71 */  0x4e,   /* 0x72 */  0xce,   /* 0x73 */
0x2e,   /* 0x74 */  0xae,   /* 0x75 */  0x6e,   /* 0x76 */  0xee,   /* 0x77 */
0x1e,   /* 0x78 */  0x9e,   /* 0x79 */  0x5e,   /* 0x7a */  0xde,   /* 0x7b */
0x3e,   /* 0x7c */  0xbe,   /* 0x7d */  0x7e,   /* 0x7e */  0xfe,   /* 0x7f */
0x01,   /* 0x80 */  0x81,   /* 0x81 */  0x41,   /* 0x82 */  0xc1,   /* 0x83 */
0x21,   /* 0x84 */  0xa1,   /* 0x85 */  0x61,   /* 0x86 */  0xe1,   /* 0x87 */
0x11,   /* 0x88 */  0x91,   /* 0x89 */  0x51,   /* 0x8a */  0xd1,   /* 0x8b */
0x31,   /* 0x8c */  0xb1,   /* 0x8d */  0x71,   /* 0x8e */  0xf1,   /* 0x8f */
0x09,   /* 0x90 */  0x89,   /* 0x91 */  0x49,   /* 0x92 */  0xc9,   /* 0x93 */
0x29,   /* 0x94 */  0xa9,   /* 0x95 */  0x69,   /* 0x96 */  0xe9,   /* 0x97 */
0x19,   /* 0x98 */  0x99,   /* 0x99 */  0x59,   /* 0x9a */  0xd9,   /* 0x9b */
0x39,   /* 0x9c */  0xb9,   /* 0x9d */  0x79,   /* 0x9e */  0xf9,   /* 0x9f */
0x05,   /* 0xa0 */  0x85,   /* 0xa1 */  0x45,   /* 0xa2 */  0xc5,   /* 0xa3 */
0x25,   /* 0xa4 */  0xa5,   /* 0xa5 */  0x65,   /* 0xa6 */  0xe5,   /* 0xa7 */
0x15,   /* 0xa8 */  0x95,   /* 0xa9 */  0x55,   /* 0xaa */  0xd5,   /* 0xab */
0x35,   /* 0xac */  0xb5,   /* 0xad */  0x75,   /* 0xae */  0xf5,   /* 0xaf */
0x0d,   /* 0xb0 */  0x8d,   /* 0xb1 */  0x4d,   /* 0xb2 */  0xcd,   /* 0xb3 */
0x2d,   /* 0xb4 */  0xad,   /* 0xb5 */  0x6d,   /* 0xb6 */  0xed,   /* 0xb7 */
0x1d,   /* 0xb8 */  0x9d,   /* 0xb9 */  0x5d,   /* 0xba */  0xdd,   /* 0xbb */
0x3d,   /* 0xbc */  0xbd,   /* 0xbd */  0x7d,   /* 0xbe */  0xfd,   /* 0xbf */
0x03,   /* 0xc0 */  0x83,   /* 0xc1 */  0x43,   /* 0xc2 */  0xc3,   /* 0xc3 */
0x23,   /* 0xc4 */  0xa3,   /* 0xc5 */  0x63,   /* 0xc6 */  0xe3,   /* 0xc7 */
0x13,   /* 0xc8 */  0x93,   /* 0xc9 */  0x53,   /* 0xca */  0xd3,   /* 0xcb */
0x33,   /* 0xcc */  0xb3,   /* 0xcd */  0x73,   /* 0xce */  0xf3,   /* 0xcf */
0x0b,   /* 0xd0 */  0x8b,   /* 0xd1 */  0x4b,   /* 0xd2 */  0xcb,   /* 0xd3 */
0x2b,   /* 0xd4 */  0xab,   /* 0xd5 */  0x6b,   /* 0xd6 */  0xeb,   /* 0xd7 */
0x1b,   /* 0xd8 */  0x9b,   /* 0xd9 */  0x5b,   /* 0xda */  0xdb,   /* 0xdb */
0x3b,   /* 0xdc */  0xbb,   /* 0xdd */  0x7b,   /* 0xde */  0xfb,   /* 0xdf */
0x07,   /* 0xe0 */  0x87,   /* 0xe1 */  0x47,   /* 0xe2 */  0xc7,   /* 0xe3 */
0x27,   /* 0xe4 */  0xa7,   /* 0xe5 */  0x67,   /* 0xe6 */  0xe7,   /* 0xe7 */
0x17,   /* 0xe8 */  0x97,   /* 0xe9 */  0x57,   /* 0xea */  0xd7,   /* 0xeb */
0x37,   /* 0xec */  0xb7,   /* 0xed */  0x77,   /* 0xee */  0xf7,   /* 0xef */
0x0f,   /* 0xf0 */  0x8f,   /* 0xf1 */  0x4f,   /* 0xf2 */  0xcf,   /* 0xf3 */
0x2f,   /* 0xf4 */  0xaf,   /* 0xf5 */  0x6f,   /* 0xf6 */  0xef,   /* 0xf7 */
0x1f,   /* 0xf8 */  0x9f,   /* 0xf9 */  0x5f,   /* 0xfa */  0xdf,   /* 0xfb */
0x3f,   /* 0xfc */  0xbf,   /* 0xfd */  0x7f,   /* 0xfe */  0xff,   /* 0xff */
} ;

void 
haddr_convert(addr)
u_char *addr;
{
  u_long i;

  for (i=0; i<6; i++) {
        *addr = con_table[*addr] & 0xFF;
        addr++;
  }; 
}

privilege()
{

	printf("arp : Super user privileges required for using -d, -f, -s flags.\n");
	exit(1);
}
