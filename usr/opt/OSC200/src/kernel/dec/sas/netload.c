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
static char	*sccsid = "@(#)$RCSfile: netload.c,v $ $Revision: 4.3.4.13 $ (DEC) $Date: 93/02/01 10:33:15 $";
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
 * derived from netload.c	3.1	(ULTRIX/OSF)	2/28/91";
 */
/*
 * netload.c
 */

/*
 * Maintenance History
 *
 * 25-Feb-91 -- Don Dutile
 *	Merged v4.2 changes with osc.25 pool. Added back
 *	VAX code (untested).
 *
 * 09-Oct-90 J. Szczypek
 *	Added TURBOchannel ROM support.  Code has been added to build
 *	a complete ethernet packet (header added).
 *
 * 10-Nov-89 T.N. Cherng
 *	Get the netboot device from the prom_getenv("boot").
 *
 * 8-Feb-88 tresvik
 *	swap the definition of TERTIARY and SECONDARY so that Mop can
 *	be brought into spec.  Now TERTIARY = vmunix and SECONDARY =
 *	client specific paramater file.  Associated registration changes
 *	made to /etc/dms and /etc/ris in conjuction with this change.
 *	This turned out to more than anticipated.  A relatively minor
 * 	change to the mop_dlload code causes a significant restructuring 
 *	of this code.  It, in fact, represents the removal a fair amount
 *	of code which is no longer required.  Now that mop_dlload does the
 *	right things, it is easier to communicate with.
 */
/*
 * DEC OSF/1 netload.c
 */ 
#include <sys/param.h>
#include <sys/socket.h>
#ifdef mips
#include <mach/mips/vm_param.h>
#endif /* mips */
#ifdef __alpha
#include <mach/alpha/vm_param.h>
#endif /* __alpha */
#include <net/if.h>
#include <netinet/in.h>
#include <a.out.h>
#ifdef mips
#include "../../arch/mips/hal/entrypt.h"
#endif /* mips */
#ifdef __alpha
#include <machine/cpu.h>
#include <machine/entrypt.h>
#include <machine/rpb.h>
#include <alpha/load_image.h>
#endif /* __alpha */
/*DWS -- These includes need to be located in the build environment */
#include "mop.h"
#include <bootpd/bootp.h>

#ifdef mips
#define printf	_prom_printf
#define open(f,m)\
	((rex_base) ? rex_open((f), (m)) : _prom_open((f), (m))) 
#define	read	_prom_read
#define write	_prom_write
#define close	_prom_close
#define lseek	_prom_lseek
#define strcmp	prom_strcmp
#define strcat	prom_strcat
#define strlen	prom_strlen
#define strcpy	prom_strcpy
#define getenv	prom_getenv
#define setenv	prom_setenv
#define stop()	((rex_base) ? rex_rex('h') : _prom_restart())
#define bdev 	"boot"
#define mopnetboot ( (!strncmp(bootdevice, "mop", 3)) || \
		     (!strncmp(&bootdevice[2], "mop", 3)) )
#define bootpnetboot ( (!strncmp(bootdevice, "tftp", 4)) || \
		       (!strncmp(&bootdevice[2], "tftp", 4)) )
#endif /* mips */

#ifdef __alpha
#define open	prom_open
#define read	prom_read
#define write	prom_write
#define close	prom_close
#define getenv	prom_getenv
#define setenv	prom_setenv
#define bdev	"booted_dev"
#define mopnetboot   ( (!strncmp(bootdevice, "MOP", 3)) || \
			(!strncmp(bootdevice, "mop", 3)) )
#define bootpnetboot ( (!strncmp(bootdevice, "BOOTP", 5)) || \
			 (!strncmp(bootdevice, "bootp", 5)) ) 
#define MAXARG 15
#define INBUFSZ 256
char *local_argv[MAXARG];
char booted_osflags[128];
char booted_file[128];
char vmunix_name[] = "vmunix";
char *imagename;
int ub_argc;
char **ub_argv;
char **ub_envp;
char addrstring[28]; 
char *addrstringptr = addrstring;
long spfn;
struct rpb *rpb = (struct rpb *)HWRPB_ADDR; /* the rpb          */
static long ptbr;               /* pagetable base register      */
static long kernel_entry; 	/* osf startpc passed to kdebug */
/* Globals to pass the netblk physaddr to the kernel, and
 * to ensure that the netblk lives in a single page so it can be mapped by 
 * a single PTE.  On mips, the netblk location is hardcoded.
 */
static long netblkphysaddr; 	
#define NETBLKBUFSZ ((2*sizeof(struct netblk))+8)
char netblkbuf[NETBLKBUFSZ];
struct netblk *netblkbufptr;
#define NETBLK_LDADDR netblkbufptr
#endif /* __alpha */

extern char *index(char* s1, char);
#ifdef mips
extern struct execinfo coff;
extern int rex_base;
extern int rex_magicid;
#endif /* mips */
extern int sofar, spin, tot, check;	/* To print Percentage Loaded */

/* The Ultrix MOP server contains a bug.  Instead of sending the load address,
 * it sends the transfer address (which works on Ultrix because the load 
 * address and transfer address are the same).  This is not true for OSF,
 * so we need to hard-code the difference.  
 * SO, BE SURE TO CHANGE THESE OFFSETS IF THE KERNEL START ADDRESS CHANGES,
 * OR netload WILL BREAK!!
 */
#if !defined(LDADDR_ADJUST)
#ifdef mips
#define LDADDR_ADJUST	3*(-4096)
#endif /* mips */
#ifdef __alpha
/* The downloaded image will contain the coff header, so the load and transfer
 * addresses will be gotten from it instead of hardcoding a value here.
 */
#define LDADDR_ADJUST	0
#endif /* __alpha */
#endif /*!defined(LDADDR_ADJUST)*/

/* netload for bootp is for the MIPs platform only.  On Alpha, the ROMS
 * should be robust enough to download the entire vmunix, which eliminates
 * the need for netload as an intermediate loader.  This code can be ported
 * later, if the need arises.
 */
#ifdef mips

unsigned char vm_rfc1048[4] = VM_RFC1048;
#define RFC1048_LOAD (			\
	vp[0] == vm_rfc1048[0] &&	\
	vp[1] == vm_rfc1048[1] &&	\
	vp[2] == vm_rfc1048[2] &&	\
	vp[3] == vm_rfc1048[3] 		\
)

/*
 * Bootinfo struct...holdover from old netblock style
 */
#define BI_MAGIC	0x77640a73
#define BI_LDADDR	0x8001fc00	/* 1k under the kernel */

#define TFTP_WSIZE	 512
#define TFTP_RSIZE	1024

/*
 * Boot information descriptor
 */
struct bootinfo {
		u_int	bootinfo_magic;
		/*
		 * This is the basic definition of the local
		 * machine.
		 */
		char	host_name[MAXDOMNAMELEN];
		u_int	host_ipaddr;
		u_int	host_brdcast;
		u_int	host_netmask;

		/*
		 * This define the host we booted from
		 */
		u_int	boot_type;
#define		BOOT_LOCAL	0x0001
#define		BOOT_MOP 	0x0002
#define		BOOT_BOOTP 	0x0004
#define		BOOT_TYPES	(BOOT_LOCAL|BOOT_MOP|BOOT_BOOTP)
		char	boot_device[10];
		char	bootsvr_name[MAXDOMNAMELEN];
		u_int	bootsvr_ipaddr;

		/*
		 * This defines the root filessystem and
		 * the host its serverd from.
		 */
		u_int	root_type;
#define		ROOT_NONE	0x0001
#define		ROOT_UFS	0x0002
#define		ROOT_NFS	0x0004
		char	rootsvr_name[MAXDOMNAMELEN];
		u_int	rootsvr_ipaddr;
		char	rootsvr_path[MAXPATHLEN];

		/*
		 * This defines the swap file and the host its serverd from.
		 */
		u_int	swap_type;
#define		SWAP_NONE	0x0001
#define		SWAP_UFS	0x0002
#define		SWAP_NFS	0x0004
#define		SWAP_TYPES	(SWAP_NONE|SWAP_UFS|SWAP_NFS)
		char	swapsvr_name[MAXDOMNAMELEN];
		u_int	swapsvr_ipaddr;
		char	swapsvr_path[MAXPATHLEN];
		u_int	swapsvr_fsiz[2];

		/*
		 * This defines the dump file and the host its serverd from.
		 */
		u_int	dump_type;
#define		DUMP_NONE	0x0001
#define		DUMP_UFS	0x0002
#define		DUMP_NFS	0x0004
#define		DUMP_MOP	0x0008
#define		DUMP_TFTP	0x0010
#define		DUMP_TYPES	(DUMP_NONE|DUMP_UFS|DUMP_MOP|DUMP_NFS|DUMP_TFTP)
		char	dumpsvr_name[MAXDOMNAMELEN];
		u_int	dumpsvr_ipaddr;
		char	dumpsvr_path[MAXPATHLEN];
		u_int	dumpsvr_fsiz[2];
};

extern struct bootinfo *bootinfop;

#endif /* mips */

#define NSIZE		256
#define BSIZE		(sizeof(struct bootinfo))
#define BADLOAD		1
#define PROG		1
#define BOOTINFO	2

char bootfile[NSIZE + 12] = "KERNELIMAGE:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
char svrname[NSIZE + 11] = "SERVERNAME:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";

int	io = 0; 		/* IO channel */
int	debug = 0;		/* Debug flag */
/*
 * Mop routines and data.
 */

char	mop_destination[6] = {0xab, 0,0,1,0,0};
char	broadcast[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
char	mopdl_multicast[6] = {0xab, 0,0,1,0,0};

char	boot[128] = {0};
int 	state = 0;
char	state_char[4] = {'-', '\\', '|', '/'};
int 	load_kdebug = 0;

extern long mop_upload();

#ifdef mips
main (argc, argv, envp, vector)
int argc;
char **argv;
char **envp;
char **vector;
#endif /* mips */
#ifdef __alpha
main (argc,argv,envp)
int argc;
char **argv, **envp;
#endif /* __alpha */
{
	extern char *version;	/* version string in version.c */
	extern char *strchr(); 
	int (*start)();
	int (*old_start)();
	char *bootdevice = &boot[0], *cp;
	long i = BADLOAD;

#ifdef __alpha
	argc = 1;		/* as a minimum, pass kernel name */
	argv = local_argv;
	strcpy(booted_osflags, getenv("booted_osflags")); 
	strcpy(booted_file, getenv("booted_file"));
	if (strchr(booted_osflags, 'V') || strchr(booted_osflags, 'v')) {
		debug = 1;
	}

	if (debug) {
		printf("netload: booted_osflags = <%s\>\n", booted_osflags);
		printf("netload: booted_file = <%s\>\n", booted_file);
	}
	if (strcmp(booted_file, "") == 0) {
		argv[0] = vmunix_name;
		if (debug) {
			printf("netload: hardcoding vmunix name to %s\n", vmunix_name);
		}
	} else {
		argv[0] = booted_file;
		if (debug) {
			printf("netload: setting vmunix name to %s\n", argv[0]);
		}
	}

	/* Align the netblk structure to be passed to the kernel.  
	 * Make sure it does not cross a page boundary, so that it
	 * can be mapped by a single PTE.  
	 */
	netblkbufptr = (struct netblk *)align_netblk(netblkbuf, 
			&netblkbuf[NETBLKBUFSZ], sizeof(struct netblk));
	netblkphysaddr = boot_vtop(netblkbufptr); /* find physical address */
	if (debug) {
		printf("netload: netblkbufptr = 0x%lx\n", netblkbufptr);
		printf("netload: netblkphysaddr = 0x%lx\n", netblkphysaddr);
	}

	/* Pass the kernel the physical address of the netblk in argv */
	strcpy(addrstringptr, "netblk=");
	itoa(netblkphysaddr, &addrstring[strlen(addrstring)]);
	argv[argc++] = addrstringptr;

	ub_envp = envp; /* save prom's args for kernel */
#endif /* alpha */

	ub_argc = argc; /* save prom's args for kernel */
	ub_argv = argv; /* save prom's args for kernel */

#ifdef mips
	if ((int)envp == REX_MAGIC) {
		rex_base = (int)vector;
		rex_magicid = REX_MAGIC;
		/* printf("\nREXmagic \n"); only for debug */
	}

	for (i = 0; i < argc; i++) {
	    if (strcmp(argv[i], "-kdebug") == 0) {
		load_kdebug = 1;
	    }
	}
#endif /* mips */

	printf("\nDEC OSF/1 Network Loader - %s\n\n", version);
#ifdef mips
	if ((cp = (char *)getenv("debug")) != NULL)
		debug = *cp - '0';
#endif /* mips */

	if (debug)
		printf("debug flag is ON.\n");

	/*
	 * Get the boot device name 
	 */
#ifdef mips
	if(rex_base) {
		for(i=1;i<ub_argc;i++) {
			if(ub_argv[i][0] != '-' && ub_argv[i][0] != NULL)
				if(ub_argv[i][1] == '/') {
	  				cp = argv[i];
					}
		}
	  	if(rex_bootinit() < 0) {
	    		printf("netload: no bootdevice set.\n");
	    		stop();
	  	}
	} else
#endif /* mips */
	if ((cp = (char *)getenv(bdev)) == NULL) {
		printf("netload: no bootdevice set.\n");
		stop();
	}
	if (debug) {
		printf ("netload: boot device is '%s'\n\n", cp);
	}

	strcpy(boot, cp); /* Save boot variable because tftp code destroys it */

	/*
	 * Load boot information based on boot type
	 */
	if (strlen(bootdevice) > 2
	&& mopnetboot) {
		struct netblk *netblk_ptr = (struct netblk *)NETBLK_LDADDR;
#ifdef mips
		if (rex_base) {
			bcopy(mopdl_multicast, mop_destination, 6);
			if (rex_bootinit()) {
				printf("netload: bootinit of network driver failed.\n");
				goto bad;
			}
		}
		else if ((io = open(bootdevice, 2)) < 1) {
			printf("netload: cannot open the boot device (%s).", bootdevice);
			goto bad;
		}
#endif /* mips */
#ifdef __alpha
		bcopy(mopdl_multicast, mop_destination, 6);
		if ((io = open(bootdevice, 0)) < 0) {
			printf("netload: cannot open the boot device (%s).\n", bootdevice);
			goto bad;
		}
#endif /* __alpha */
		if (debug)
			printf("netload: boot channel is %d\n", io);

		if (debug) {
			printf("`*' means that 64K bytes have been loaded\n");
			printf("`R' means that a read error occurred\n");
			printf("`W' means that a write error occurred\n");
			printf("`S' means that the packet rcvd was not the one asked for\n\n");
		}

		bzero((char *)netblk_ptr, sizeof(struct netblk));
		i = mop_upload(PGMTYP_SECONDARY, netblk_ptr, sizeof(struct netblk));
		if (i == BADLOAD) {
			printf("netload: network parameter file load failed.\n");
			printf("\tContinuing without network information.\n");
		} else {
			printf ("Host server is '%s'\n", netblk_ptr->srvname);
			if (debug) {
				printf ("Dumping netblk contents\n");
				printf ("netblk_ptr->srvname=<%s>\n", netblk_ptr->srvname);
				printf ("netblk_ptr->srvipaddr=<0x%x>\n", netblk_ptr->srvipadr);
				printf ("netblk_ptr->cliname=<%s>\n", netblk_ptr->cliname);
				printf ("netblk_ptr->cliipaddr=<0x%x>\n", netblk_ptr->cliipadr);
				printf ("netblk_ptr->brdcst=<0x%x>\n", netblk_ptr->brdcst);
				printf ("netblk_ptr->netmsk=<0x%x>\n", netblk_ptr->netmsk);
				printf ("netblk_ptr->swapfs=<0x%x>\n", netblk_ptr->swapfs);
				printf ("netblk_ptr->rootfs=<0x%x>\n", netblk_ptr->rootfs);
				printf ("netblk_ptr->swapsz=<0x%x>\n", netblk_ptr->swapsz);
				printf ("netblk_ptr->dmpflg=<0x%x>\n", netblk_ptr->dmpflg);
				printf ("netblk_ptr->rootdesc=<%s>\n", netblk_ptr->rootdesc);
				printf ("netblk_ptr->swapdesc=<%s>\n", netblk_ptr->swapdesc);
			}
		}

/*	Commenting out of mop code - doesn't look good.
 *		printf("Loading %s%s/vmunix@%s ... \n", 
 *			bootdevice, netblk_ptr->rootdesc, netblk_ptr->srvname);
 */

		/*
		 * Load addr KLUDGE to handle OSF boot stack...
		 *	This should be fixed in mop_fetchfile
		 */
		start = (int(*)()) mop_upload(PGMTYP_TERTIARY, LDADDR_ADJUST,					      RCV_BUF_SZ);
#ifdef mips
		if (rex_base == 0)
			close(io);
#endif /* mips */

#ifdef __alpha
		close(io);
#endif /* alpha */

		if ((int)start == BADLOAD) {
			printf("failed.\n");
			goto bad;
		}
		else
			printf("done.\n");
		
#ifdef SUPPORTKDEBUG 
		if (load_kdebug) {
		    old_start = start;
		    printf("\nLoading diagnostic image ...\n");
		    for (i = 0; i < 2000000; i++); 	/* Give the host a breather */
		    start = (int(*)()) mop_upload(PGMTYP_OPSYS, 0, RCV_BUF_SZ);
		    printf("\n");
		    if ((int) start == BADLOAD) {
			printf("Unable to load diagnostic image, starting operating system\n");
			start = old_start;
	    		}
	 	}
#endif /* SUPPORTKDEBUG */


	}
#ifdef mips
	else if (strlen(bootdevice) > 3
	&& bootpnetboot) {
		struct bootinfo *bootinfop = (struct bootinfo *)BI_LDADDR;
		unsigned char *vp = (unsigned char *)getenv("bootpvend");
		char bootpath[NSIZE];
		char loadfile[NSIZE*2], *file;
		char servername[NSIZE];

		if (debug) 
			printf("\tbootdevice = %s\n", bootdevice);
		bzero(bootinfop, sizeof(struct bootinfo));

		/*
		 * Try to load boot information from client area
		 *	- for rex_base bootpvend is the address of the
		 *        vendor information
		 *	- for the PMAX bootpvend is the vendor information
		 *        string
		 */
		if (rex_base)
			vp = rex_strtol(vp, 0 , 0);
		if (vp && RFC1048_LOAD) {
			char *rootpath, *hostname;

			parsevp(vp, &rootpath, &hostname);
		}
#ifdef notdef
		if (vp && RFC1048_LOAD) {
			char *rootpath, *hostname;

			parsevp(vp, &rootpath, &hostname);
			strcpy(loadfile, bootdevice);
			strcat(loadfile, rootpath);
			strcat(loadfile, "/etc/bootpinfo.o");
			 i = tftp_upload(BOOTINFO, loadfile, bootinfop, BSIZE); 

		}
		else
			i = BADLOAD;
		if (i == BADLOAD) {
			printf("netload: can't load vendor information.\n");
			goto bad;
		}
		if ((strcmp(bootinfop->rootsvr_name, "") == 0) || 
		    (strcmp(bootinfop->rootsvr_path, "") == 0)) {
			printf("netload: boot file path not defined.\n");
			goto bad;
		}
#endif

		/*
		 * Extract the bootpath and file from bootfile. 
		 */
		strcpy(bootpath, bootfile);
		if ((file=index(bootpath,':'))==NULL){
		 	printf("netload: kernelpath: bootfile path not defined.\n"); 
			goto bad;
		}
		else{
			*file++='\0';
			strcpy(bootpath,file);
		}

		/*
		 * Extract the severname from svrname. 
		 */
		strcpy(servername, svrname);
		if ((file=index(servername,':'))==NULL){
		 	printf("netload: kernelpath: bootfile path not defined.\n"); 
			goto bad;
		}
		else{
			*file++='\0';
			strcpy(servername,file);
		}


		/*
		 * Try to load vmunix
		 */
		strcpy(loadfile, bootpath);

                printf("Loading %s@%s ... \n", loadfile, servername);

#ifdef mips
		if (rex_base) {
			/* turbo-channel */
			if (rex_bootinit()) {
				printf("netload: bootinit of network driver failed.\n");
				goto bad;
			}
			start = tftp_boot(0, 0, loadfile); 

			/*start = tftp_upload_rex(0, 0);*/
		}
		else {
			/* PMAX */
			/*
			 * Need to pass bootdevice and bootpath to proms.
			 */
			strcpy(loadfile, bootdevice); 
			strcat(loadfile, bootpath);
			start = (int(*)())tftp_upload(PROG, loadfile, 0, 0);
		}
#endif /* mips */

		if ((int)start == BADLOAD) {
			printf("failed.\n");
			goto bad;
		}
		else
			printf("done.\n");
	}
#endif /* mips */
	else {
		printf("netload: unknown boot device type (%s).\n", bootdevice);
		goto bad;
	}

	printf("\nStarting at 0x%lx\n\n", start);
/*	setenv("boot", bootdevice);	/* Restore boot variable */
	if (debug <= 2) {
#ifdef mips
		(*start)(argc, argv, envp, vector);
#endif /* mips */
#ifdef __alpha
		rpb->rpb_software = 0;
		{
			extern prom_io;
			if (prom_io >= 0)
				prom_close (prom_io);
		}
		imb();
        	(*((int (*) ()) start))
			(++spfn,ptbr,ub_argc,ub_argv,ub_envp, kernel_entry);
#endif /* alpha */
	} else
		printf("\tnetload: Not starting file...in debug mode\n");

bad:
	{
		extern prom_io;
		if (prom_io >= 0)
			prom_close (prom_io);
	}
	setenv(bdev, bootdevice);	/* Restore boot variable */
	stop();
}


#ifdef mips
tftp_upload(type, loadfile, addr, bufsz)
int type;
char *loadfile;
int addr;
int bufsz;
{
	int io = 0, rval = 0, i;

	for (i = 0; i < 2000000; i++)
		;	/* back off */
	if (debug)
		printf("\ttftp_upload: loading %s of type(%s).\n",
			loadfile,
			(type == PROG) ? "PROG" : 
			(type == BOOTINFO) ? "BOOTINFO" : "UNK");
	if ((io = open(loadfile, 0)) < 0) {
		if (debug)
			printf("\ttftp_upload: can't open loadfile (%s)\n",
			loadfile);
		return(BADLOAD);
	}

	switch (type) {
	case PROG:
		rval = load_image(io);
		break;

	case BOOTINFO:
/*
		printf("BOOTINFO\n");
		rval = load_bootinfo(io, addr);
*/
		break;

	default:
		rval = BADLOAD;
		break;
	}

	if (rex_base == 0)
		close(io);
	return(rval);
}

#include <arpa/tftp.h>	/* tftp header */

rex_open(fp, mode)
char *fp;
int mode;
{
	int i;
	char buf[600];
	struct tftphdr *rrq = (struct tftphdr *)&buf[0];
	if (rex_bootinit()) {
		if (debug)
			printf("\trex_open: can't initialize network driver.\n");
		return(BADLOAD);
	}
printf("rex_open: after bootinit\n");

	bzero((caddr_t)rrq, sizeof(*rrq));
	rrq->th_opcode = nuxi_s(RRQ);
	bcopy(fp, (caddr_t)&rrq->tu_stuff[0], strlen(fp));
	bcopy("octet", (caddr_t)&rrq->tu_stuff[strlen(fp)+1], 5);
printf("rex_open: after bcopy\n");
	if ((rex_bootwrite(io, (caddr_t)rrq, 2+strlen(fp)+1+6+1)) < 0) {
		if (debug)
			printf("\ttftp_read: RRQ failed\n");
		return(BADLOAD);
	} 

printf("rex_open: after bootwrite\n");

	while ((i = rex_bootread(0, rrq, sizeof(struct tftphdr))) == 0)
		; 
	if ((i < 0) || (i < 4)) {
		printf("rex_open: bad TFTP packet\n");
		return(1);
	}
	return(0);
}

pmax_tftp_read(io, addr, size)
int io;
caddr_t addr;
int size;
{
	int i, tsize, offset, count = size;
	int endcount = 0;
	
	if (debug) {
		printf("tftp_read(%d, %x, %d)\n", io, addr, size);
	}

	while (count) {
		tsize = MIN(TFTP_RSIZE, count);
	        if (spin++ >= check) {
			/* do this each time: sofar += (check * tsize); */
			spin = 1;
			printf("\b\b\b\b%2d  ", (100*sofar/tot));
		}
		printf("%c\b", state_char[state++]);
		if (state >= 4)
			state = 0;
#ifdef notdef
		if (rex_base) {
			char buf[600];
			struct tftphdr *bp = (struct tftphdr *)&buf[0];

			bzero(bp, sizeof(*bp));
			while ((i = rex_bootread(io, bp, tsize)) == 0)
				; 

			if ((i < 0) || (i < 4)) {
				printf("tftp_read: bad TFTP packet\n");
				size = BADLOAD;
				goto done;
			}
			switch (nuxi_s(bp->th_code)) {
			case RRQ:
				if (debug)
					printf("\ttftp_read: RRQ %d\n", i);
				continue;

			case WRQ:
				if (debug)
					printf("\ttftp_read: WRQ %d\n", i);
				continue;

			case DATA:
				if (debug)
					printf("\ttftp_read: DATA %d\n", i);
				bcopy((caddr_t)&bp->th_data[0], addr, tsize);
				break;

			case ACK:
				if (debug)
					printf("\ttftp_read: ACK %d\n", i);
				continue;

			case ERROR:
				if (debug)
					printf("\ttftp_read: ERROR %d\n",
					nuxi_s(bp->th_code));
				size = BADLOAD;
				goto done;

			default:
				if (debug)
					printf("\ttftp_read: UNK(%d) %d\n",
					nuxi_s(bp->th_code), i);
				size = BADLOAD;
				goto done;
			}
		}
		else { 
		}
#endif
again:
			i = read(io, addr, tsize);
			if ((i < 0) || (i != tsize)) {
				/***
				size = BADLOAD;
				goto done;
				 ***/
				if (i != 0) {
					printf("tftp_read: short read (%d bytes)\n", i);
					goto done;
				} else {
					printf("tftp_read: zero read\n");
					if (endcount++ > 4)
						goto done;
					else
						goto again;
				}
			}
			endcount=0;
		count -= tsize;
		addr  += tsize;
		sofar += tsize;
	}
done:
	/* printf("tftp_read: %d\n", size); 	XXX debugging only */
	return(size-count);
}

#ifdef notdef
load_bootinfo(io, addr)
int io;
int addr;
{
	struct mipsexec {
		struct filehdr fh;
		AOUTHDR ah;
		struct scnhdr sh;
	} coff;
	int size, byteoffset, datasize;

	printf("load_bootinfo(%d, %x)\n", io, addr);

	size = tftp_read(io, (char *)&coff, sizeof(coff));
	if ((size < 0) || (size < sizeof(coff))) {
		printf("load_bootinfo: read failed (%d).\n", size);
		datasize = BADLOAD;
		goto done;
	}
	
	if (ISCOFF(coff.fh.f_magic)) {
		if (coff.ah.magic != OMAGIC) {
			printf("load_bootinfo: COFF bad magic number %o\n",
			coff.ah.magic);
			datasize = BADLOAD;
			goto done;
		} 
		byteoffset = coff.sh.s_scnptr;
		datasize = coff.ah.dsize;
		if (debug>1)
			printf("\tload_bootinfo: COFF, byteoffset(%d), datasize(%d)\n",
			byteoffset, datasize);
	}
	else {
		printf("load_bootinfo: unknown load file format.\n");
		datasize = BADLOAD;
		goto done;
	}

	lseek(io, byteoffset, 0);
	size = tftp_read(io, addr, datasize);
	if ((size < 0) || (size != datasize)) {
		printf("load_bootinfo: short read (%d).\n", size);
		datasize = BADLOAD;
		goto done;
	}
done:
	printf("load_bootinfo: %d\n", datasize);
	return(datasize);
}
#endif /* notdef */

parsevp(vp, dir, host)
unsigned char *vp;
char **dir;
char **host;
{
	unsigned char tag;
	int n;
	unsigned char *ep = vp + 64;
 
	vp = vp + 4;
	tag = *vp++;
	while (tag < TAG_END && vp < ep) {
		if (debug>1)
			printf("\tparsevp: tag = %d ", tag);
		switch (tag) {
		case TAG_PAD:
			tag = *vp;
			if (debug>1)
				printf("NULL\n");
			break;

		case TAG_SUBNET_MASK:
		case TAG_TIME_OFFSET:
			vp = vp + 4;
			if (debug>1)
				printf("FIXED %x\n", *((int *) (vp-4)));
			break; 

		case TAG_HOSTNAME:
			*host = (char *)&vp[1];
			vp = vp + *vp + 1;
			tag = *vp;
			*vp = 0;
			if (debug>1)
				printf("Hostname = %s\n", *host);
			break;

		case TAG_BOOTSIZE:
			vp = vp + 3;
			if (debug>1)
				printf("Boot Size %d \n,", *((int *) (vp-2)));
			break;

		case 128/*TAG_PATHNAME*/:
			*dir = (char *)&vp[1];
			vp = vp + *vp + 1;
			tag = *vp;
			*vp = 0;
			if (debug>1)
				printf("Top Dir = %s\n", *dir);
			break;

		default:
			n = (int)*vp;
			vp = vp + *vp + 1;
			tag = *vp;
			if (debug>1)
				printf("N = %d\n", n);
			break;
		}
		vp++;
	}
}

#endif /* mips */

long
mop_upload(prog, addr, bufsz)
int prog;
long addr;
int bufsz;
{
	union mop_packets mop_output;
	union mop_packets mop_input;
	union mop_packets *mop_i = &mop_input;
	union mop_packets *mop_o = &mop_output;
	unsigned char *dp, buf[17];
	int wrt_cnt, wrt_retry=5, rd_retry=5, seq_errs=0;
	int j=0, ldnum=1;
	int i, size;
	long entryaddr;
	int aoutadj=0;
	int bssize=0;
	long bssaddr;
	struct image_return *image;
	extern long map_mop_image();
	
	/*
	 * The following setup of ...code is needed to get things 
	 * looping properly below.
	 */
	for (i=0; i < 2000000; i++);	/* back off */
	mop_i->memload.code = NETLOAD_REQUEST; 
	for (;;) {
		int tmp;

		switch (mop_i->memload.code) {
		/*
		 * This is local variable use to kick start the network
		 * boot sequence.  It initiates program requests by
		 * falling out to write after creating the desired
		 * program request packet.
		 */
		case NETLOAD_REQUEST:
			mop_o->req_pgm.code = REQ_PROG_CODE;
			mop_o->req_pgm.devtype = NET_QNA;
			mop_o->req_pgm.mopver = MOP_VERSION;
			mop_o->req_pgm.pgmtyp = prog;
			if (prog == PGMTYP_OPSYS)
			    mop_o->req_pgm.swid_form = -2; /* force diag load */
			else
			    mop_o->req_pgm.swid_form = -1; /* force sys load */
			mop_o->req_pgm.proc = SYSTEMPROC;
			mop_o->req_pgm.rbufsz_param = XTRA_BUFSZ;
			mop_o->req_pgm.sz_field = 2;
			tmp = sizeof mop_i->memload;
			bcopy((char *)&tmp, mop_o->req_pgm.rcvbufsz,
				sizeof mop_o->req_pgm.rcvbufsz);
			wrt_cnt = sizeof mop_o->req_pgm;
			break;
		/*
		 * In response to a request for a multisegment tertiary
		 * load from the network  (except for the last segment)
		 */
		case VOLASS_CODE:
		/*
		 * Send the same packet out again, which is the original
		 * request
		 */
			mop_i->memload.code = NETLOAD_REQUEST;
			continue;
		case MEMLD_CODE:
			/*
			 * The load number of the packet received must
			 * equal the number requested.  If it doesn't, the
			 * host is again asked for the same packet by
			 * load sequence number
			 */
			if (mop_i->memload.loadnum != 
				(u_char)((ldnum - 1) & 0xff)) {
				if (debug) printf("S");
				if (++seq_errs == 5) {
					printf("\n\
Wrong packet number received from server - retries exceeded\n");
					goto error;
				}
				break;		/* fall out to ask again */
			}
			seq_errs=0;
			bcopy(mop_i->memload.loadaddr, (char *)&tmp,
				sizeof mop_i->memload.loadaddr);
#ifdef __alpha
			if ( (tmp == 0) && (prog == PGMTYP_TERTIARY) ) {
				/* This is the first packet, which should
				 * contain the Alpha a.out header, at least,
				 * since MEMLD_CODE packets are always 1400 
				 * bytes long. The a.out header is passed 
				 * to map_mop_image(), which will use the 
				 * info to map the image about to be downloaded.
				 */
				image = (struct image_return *) map_mop_image(mop_i->memload.data, &addr, &aoutadj, &bssaddr, &bssize);
				if ((long)image == -1) {
					goto error;
				}
				entryaddr = image->entry;
				spfn = image->spfn;
				ptbr = image->ptbr;

				/* Now, copy the tertiary image, but leave off
				 * the a.out header!!  Need to adjust load addr
				 * as if the a.out header were loaded, since
				 * subsequent MOP packets will provide offsets
				 * assuming that it *was* loaded.
				 */
				bcopy(mop_i->memload.data + 
				      aoutadj, addr, bufsz - aoutadj);
			        addr -= aoutadj; /* adjust load address so
						  * subsequent packets get
						  * loaded at the right offset
						  */
			} else 
#endif /* __alpha */
			{
				bcopy(mop_i->memload.data, addr+tmp, bufsz);
			}

			/* 
			 * Display a progress indicator about every 
			 * 64k bytes
			 */
			if (j++ == 47){
				printf("*");
				j=0;
			}
			/*
			 * Now, prepare the next request packet before 
			 * falling out to send it.
			 */
			mop_o->req_memload.code = REQ_LOAD_CODE;
			mop_o->req_memload.loadnum = ldnum++;
			if (ldnum > 255) ldnum =0;
			mop_o->req_memload.error = 0;
			wrt_cnt = sizeof mop_o->req_memload;
			break;
		/* 
	 	 * In response to SECONDARY load request and
	 	 * the last packet on a multisegment tertiary load
	 	 */
		case MEMLD_XFR_CODE:
			/*
			 * For SECONDARY requests the rcvmsg contains
			 * real data and we don't care about any other
			 * part of the packet.  There is only one program
			 * segment allowed.  This is the netblk piece of
			 * vmbinfo.
			 *
			 * If this code is received to indicate the end of
			 * a multisegment load (our request for TERTIARY),
			 * then there is no data and all we care about
			 * is returning the transfer address of (presumably) 
			 * the vmunix that was just loaded.
			 */

			if (prog == PGMTYP_SECONDARY) {
				bcopy(mop_i->memload_xfr.loadaddr,
					(char *)&tmp, 
					sizeof mop_i->memload_xfr.loadaddr);
				bcopy(mop_i->memload_xfr.type.data,
				    addr + tmp, bufsz);
				return(0);
			}
			else {
#ifdef mips
				bcopy(mop_i->memload_xfr.type.xfr_addr,
				      (char *)&tmp,
				       sizeof mop_i->memload_xfr.type.xfr_addr);
				return ((long)tmp);
#endif /* mips */
#ifdef __alpha
				/* initialize bss space */
				bzero(bssaddr, bssize);

				/* On Alpha, this is info from the Mop header
				 * that was wrapped around the Alpha vmunix
				 * a.out image, which is useless.  The right
				 * info was stored in 'entryaddr' when we
				 * analyzed the a.out header info in the first
				 * packet.
				 */
				return (entryaddr);
#endif /* __alpha */
			}
			break;
		case PARAM_CODE:
			i = 0;
			dp = mop_i->paramload_xfr.type.data;
			while (dp[i]) {
			    switch (dp[i++]) {
			      case PLOAD_TSNAME:
			      case PLOAD_TSADDR:
			      case PLOAD_HSNAME:
			      case PLOAD_HSADDR:
			      case PLOAD_HSTIME:
				size = dp[i++];
				bcopy(&dp[i], buf, size);
				buf[size] = '\0';
				i += size;
				break;

			      default:
				printf("unknown param type %x\n", dp[i-1]);
				size = dp[i++];	
				i += size;
				break;
			    }
			}
			i++;
			bcopy(&dp[i], (char *)&tmp, sizeof(tmp));

			return ((long)tmp);
			break;
       
		/*
		 * Other codes are unexpected.
		 * If we receive one, print a message, drop the packet
		 * and continue..
		 */
		default:
			printf("Unexpected MOP response code of %d, continuing..\n",
				mop_i->memload.code);
			goto readagain;
		}
		/*
		 * At least one write and one read occurs now before
		 * looping back up to evaluate the packet received.  Of
		 * course, too many read or write errors will cause
		 * failures to occur.
		 */
		while (wrt_retry--) {
			if (write_net(wrt_cnt,&mop_o->req_pgm))
				break;
			else {
				if (debug) printf("W");
				continue;
			}
		}
		if (wrt_retry <= 0 ) { 	/* if we ran out of retries */
			printf("write I/O error: retries exceeded\n");
			goto error;
		}
		wrt_retry=5;

readagain:
		if ((read_net((sizeof mop_i->memload),&mop_i->memload.code)) == 0) {
			if (debug) printf("R");
			if (rd_retry--)
				continue;	/* retry */
			printf("read I/O error: retries exceeded\n");
			goto error;
		}
		rd_retry=5;
	}
error:
	return ((long)BADLOAD);
}


write_net(size, addr)
int size;
char *addr;
{
	int status;

#ifdef mips
	if (rex_base) {
#endif /* mips */
		unsigned char buffer[1600];

		/*
		 * We are being called with only the data portion of
		 * the mop packet.  We must stick the ethernet header
		 * on and set the destination, the protocol and the
		 * length field.
		 */
		bcopy(addr,(void *)&buffer[16],size);
		bcopy(mop_destination,(void *)buffer,6);
		buffer[12]= 0x60;
		buffer[13]= 0x01;
		buffer[14]= size &0xff;
		buffer[15]= (size>>8) && 0xff;
#ifdef __alpha
		status = write(io, buffer, size+16, 0); 
#endif /* alpha */
#ifdef mips
		status = rex_bootwrite(0,buffer,size+16);
	}
	else
		status = write(io, addr, size);
#endif /* mips */
	return((status < 0) ? 0 : 1);
}


read_net(size, addr)
int size;
char *addr;
{
	int status;

#ifdef mips
	if (rex_base) {
#endif /* mips */
		while(1) {
			u_char buffer[1600];
			int len;

#ifdef mips
			while ((status = rex_bootread(0,buffer,1600)) == 0); 
#endif /* mips */
#ifdef __alpha
			while ((status = read(io, size, buffer, 0)) == 0); 
#endif /* alpha */
			if (status <= 0)
				return(0);
			/*
			 * If we got a message, check that it is not broadcast,
			 * that it is a mop message, and if we have "bound",
			 * that is from right source.  If it's ok, copy it to
			 * the callers buffer and return the proper length.
			 */
			if ((buffer[12]==0x60 && buffer[13]==0x1)
			&& !eaddr_match((char*)buffer,broadcast)) {
				len = buffer[14] | (buffer[15]<<8);
				if (mop_destination[0] & 1) {
					bcopy((void *)&buffer[6],
					mop_destination,6);
					bcopy((void *)&buffer[16],addr,len);
					return(1);
				}
				else {
					if (eaddr_match(mop_destination,
					(char*)&buffer[6])){
						bcopy((void *)&buffer[16],
						addr,len);
						return(1);
					}
				} 
			}
		}
#ifdef mips
	}
	else {
		status = read(io, addr, size);
		return((status < 0) ? 0 : 1);
	}
#endif /* mips */
}


eaddr_match(ea1, ea2)
char *ea1, *ea2;
{
	int i;

	for (i=0; i<6; i++)
		if (*ea1++ != *ea2++)
			return(0);
	return(1);
}


strncmp(s1, s2, n)
char *s1, *s2;
int n;
{
	while (--n >= 0 && *s1 == *s2++)
		if (*s1++ == '\0')
			return(0);
	return(n<0 ? 0 : *s1 - *--s2);
}

itoa(number, string)    /* integer to ascii routine */
unsigned long number;
char string[];
{
	int a, b, c;
	int i;
	i = 0;

	do {
		string[i++] = number % 10 + '0';
	} while ((number /= 10) > 0);
	string[i] = '\0';
	/* flip the string in place */
	for (b = 0, c = strlen(string)-1; b < c; b++, c--) {
		a = string[b];
		string[b] = string[c];
		string[c] = a;
	}
	return( strlen(string) );
}


