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
static char	*sccsid = "@(#)$RCSfile: commands.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:36:37 $";
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
#include "mda.h"
#include "cpus.h"
#include "lock_stats.h"
#include "slock_stats.h"
#include "mach_ltracks.h"
#include "uni_compat.h"
#include "unix_uni.h"
#include <stdio.h>
#include <strings.h>
#include <kern/thread.h>
#include <sys/param.h>
#include <machine/pcb.h>
#include <mmax/psl.h>
#include <kern/processor.h>

extern char *nxtarg();

int dump_fd = -1;		/* File Descriptor for Dump File */
int  dump_file_size;		/* Size of dump image      */
unsigned int mask_val;		/* defined in mda.c, used in find_cmd */

cpu_cmd(arglist)
char *arglist;
{
	char *p;
	int cpu, result;

	dohist("cpu", arglist);
	p = arglist;
	cpu = intarg(&p, 0, "Current cpu: ", 0, NCPUS, master_cpu);
	current_cpu = cpu;
	printf("mda: Current processor is now cpu %d\n", cpu);
}


char *display_commands[] = {
#define D_ACTIVE_THREADS 0
	"active-threads",
#define D_ADDRESS_SPACE (D_ACTIVE_THREADS+1)
	"address-space",
#define D_ALL_THREADS 	(D_ADDRESS_SPACE+1)
	"all-threads",
#define D_BUFFER	(D_ALL_THREADS+1)
	"buffer"
#define D_BFREE_LIST	(D_BUFFER+1)
	"bfreelist"
#define D_BUF_HASH	(D_BFREE_LIST+1)
	"buf-hash",
#define D_BVNODE_LIST	(D_BUF_HASH+1)
	"buf-vnode-list",
#define D_CALLOUTS	(D_BVNODE_LIST+1)
	"callouts",
#define D_CRQ		(D_CALLOUTS+1)
	"crq"
#define	D_CPU_STATE	(D_CRQ+1)
	"cpu-state",
#define	D_CRQ_MSGS	(D_CPU_STATE+1)
	"crq-messages"
#define D_DIR_HASH_TAB	(D_CRQ_MSGS+1)
	"dir-hash-table",
#define D_FILE 		(D_DIR_HASH_TAB+1)
	"file",
#define D_FS 		(D_FILE+1)
	"fs",
#define D_INODE 	(D_FS+1)
	"inode",
#define D_INODE_HASH 	(D_INODE+1)
	"inode-hash",
#define	D_INPCB		(D_INODE_HASH+1)
	"inpcb",
#define	D_LOCK		(D_INPCB+1)
	"lock",
#define D_MESSAGES	(D_LOCK+1)
	"messages",
#define D_PCB		(D_MESSAGES+1)
	"pcb",
#define	D_PROCESSOR	(D_PCB+1)
	"processor",
#define	D_PROC		(D_PROCESSOR+1)
	"proctable",
#define D_REGISTERS 	(D_PROC+1)
	"registers",
#define	D_SOCKET	(D_REGISTERS+1)
	"socket",
#define D_SYMBOL_CACHE 	(D_SOCKET+1)
	"symbol-cache",
#define D_TASK 		(D_SYMBOL_CACHE+1)
	"task",
#define	D_TCPCB		(D_TASK+1)
	"tcpcb",
#define	D_TCPCHAIN	(D_TCPCB+1)
	"tcp-chain",
#define D_THREAD 	(D_TCPCHAIN+1)
	"thread",
#define D_UTASK 	(D_THREAD+1)
	"utask",
#define	D_UTHREAD	(D_UTASK+1)
	"uthread",
#define D_VM_MAP	(D_UTHREAD+1)
	"vm-map",
#define D_VM_OBJECT	(D_VM_MAP+1)
	"vm-object",
#define D_ZONE		(D_VM_OBJECT+1)
	"zone",
#define D_VNODE 	(D_ZONE+1)
	"vnode",
#define D_VNODE_CLNLIST	(D_VNODE+1)
	"vnode-clean-list",
#define D_VNODE_DRTYLIST (D_VNODE_CLNLIST+1)
	"vnode-dirty-list",
#define D_VNODE_HASHCHAIN (D_VNODE_DRTYLIST+1)
	"vnode-hash-chain",
#define D_NFSNODE 	(D_VNODE_HASHCHAIN+1)
	"nfsnode",
#define D_NFSNODE_HASH 	(D_NFSNODE+1)
	"nfsnode-hash",
#define D_MOUNT 	(D_NFSNODE_HASH+1)
	"mount",
#define D_UFSMOUNT 	(D_MOUNT+1)
	"ufsmount",
#define D_NFSMOUNT 	(D_UFSMOUNT+1)
	"nfsmount",
#define D_MNT_VNODELIST	(D_NFSMOUNT+1)
	"mount-vnode-list",
#define	D_NFSREQ	(D_MNT_VNODELIST+1)
	"nfsreq",
#define	D_NFSREQ_LIST	(D_NFSREQ+1)
	"nfsreq-list",
#define	D_NFSHOST	(D_NFSREQ_LIST+1)
	"nfshost",
#define	D_NFSHOST_LIST	(D_NFSHOST+1)
	"nfshost-list",
#define	D_MBUF		(D_NFSHOST_LIST+1)
	"mbuf",
#define D_XPR_BUFFER	(D_MBUF+1)
	"xpr-buffer",
#define	D_Z256		(D_XPR_BUFFER+1)
	"z256",
	0
};



display_cmd(arglist)
char *arglist;
{

	int	item_selected;
	unsigned int  vth, pth, vtask, ptask, vinode, pinode, vfile, pfile;
	unsigned int  vutask, putask, vfs, pfs, vuthread, puthread;
	unsigned int  vaddr, mapv, mapp, vproc, vzone, paddr, vobj;
	unsigned int  vvnode, pvnode, vnfsnode, pnfsnode, vmount, pmount;
	char	buf[256];
	char	*arg;

	dohist("display", arglist);
	if(dump_fd == -1) {
		printf("No dump file read yet\n");
		return(FAILED);
	}

	item_selected = stabarg(&arglist, 0, "item to display",
				&display_commands[0],
				"registers");

	/*  THESE ITEMS ARE IN ALPHABETICAL ORDER  */
	switch(item_selected) {	
	case D_ACTIVE_THREADS: 
		display_active_threads();
		break;

	case D_ALL_THREADS: 
		display_all_threads();
		break;

	case D_ADDRESS_SPACE: {
			struct task *ptask;

			vtask = hexarg(&arglist, 0, "Address of task: ", 
					0, 0xffffffff, 0);

			if(phys(vtask, &ptask, ptb0) != SUCCESS)
				return(FAILED);
			ptask = (struct task *)(MAPPED(ptask));
			mapv = (unsigned int)(ptask->map);
			display_address_space(mapv);
			break;
		}

	case D_BUFFER:
		{
		static char *bufoff[] = {
			"lock",
			"event",
			0
		};

		vaddr = hexarg(&arglist, 0, "Address of buffer: ", 0,
				0xffffffff, 0);
		arg = nxtarg(&arglist, 0);
		item_selected = 0;
		if(*arg && ((item_selected = stablk(arg, bufoff, 0)) >= 0))
			item_selected++;

		if(phys(vaddr, &paddr, ptb0) == SUCCESS)
			display_buffer(vaddr, MAPPED(paddr), item_selected);
		break;
		}

	case D_BFREE_LIST:
		vaddr = hexarg(&arglist, 0, "Address of buffer", 0,
				0xffffffff, 0);

		display_bfreelist((struct buf *)vaddr);
		break;

	case D_BUF_HASH:
		vaddr = hexarg(&arglist, 0, "Address of buffer", 0,
				0xffffffff, 0);

		display_bhash_chain((struct buf *)vaddr);
		break;

	case D_BVNODE_LIST:
		vaddr = hexarg(&arglist, 0, "Address of buffer", 0,
				0xffffffff, 0);

		display_bvnode_list((struct buf *)vaddr);
		break;

	case D_CALLOUTS:
		{
		  display_callouts();
		  break;
		}

	case D_CPU_STATE:
		display_cpu_state();
		break;

	case D_CRQ:
		mapv = hexarg(&arglist, 0, "Address of crq: ", 0,
				0xffffffff, 0);
		if(phys(mapv, &mapv, ptb0) == SUCCESS)
			display_crq(MAPPED(mapv));
		break;

	case D_CRQ_MSGS:
		vaddr = hexarg(&arglist, 0, "Address of crq: ", 0,
				0xffffffff, 0);
		if(phys(vaddr, &paddr, ptb0) == SUCCESS)
			display_crq_msgs(vaddr, paddr);
		break;

	case D_DIR_HASH_TAB:
		/* This command is no longer supported
		display_dir_hash_table();
		 */
		printf("This command is no longer supported\n");
		break;

	case D_FILE: 
		vfile = hexarg(&arglist, 0, "Address of file: ", 0,
			       0xffffffff, 0);
		if(phys(vfile, &pfile, ptb0) == SUCCESS)
			display_file(MAPPED(pfile));
		break;

	case D_FS: 
		vfs = hexarg(&arglist, 0, "Address of fs struct: ", 
				0, 0xffffffff, 0);

		if(phys(vfs, &pfs, ptb0) == SUCCESS)
			display_fs(MAPPED(pfs));
		break;

	case D_INODE: 
		{
		static char *bufoff[] = {
			"data-lock",
			"io-lock",
			"event",
			0
		};

		vinode = hexarg(&arglist, 0, "Address of inode: ", 0,
				0xffffffff, 0);
#if	MMAX_MP
		arg = nxtarg(&arglist, 0);
		if(*arg) {
			if((item_selected = stablk(arg, bufoff, 0)) < 0)
				break;
			else
				item_selected++;
		} else
			item_selected = 0;
#else	MMAX_MP
			item_selected = 0;
#endif	MMAX_MP
		if(phys(vinode, &pinode, ptb0) == SUCCESS)
			display_inode(vinode, MAPPED(pinode), item_selected);
		break;
		}

	case D_INODE_HASH:
		vaddr = hexarg(&arglist, 0, "Address of inode: ", 0,
				0xffffffff, 0);
		display_ihash_chain((struct inode *)vaddr);
		break;


	case D_INPCB:
		vaddr = hexarg(&arglist, 0, "Address of inpcb: ", 0,
				0xffffffff, 0);
		if(phys(vaddr, &vaddr, ptb0) == SUCCESS)
			display_inpcb(MAPPED(vaddr));
		break;

	case D_LOCK:
		vaddr = hexarg(&arglist, 0, "Address of lock: ", 0,
				0xffffffff, 0);
		if(phys(vaddr, &vaddr, ptb0) == SUCCESS)
			display_lock(MAPPED(vaddr));
		break;

        case D_MESSAGES:
		{
		  if (*arglist == NULL)
		    {
		      display_messages(FALSE);
		      break;
		    }
		  else
		    {
		      (void) strarg(&arglist, 0, "pending", "pending", buf);
		      if (strcmp("pending",buf)==0)
			display_messages(TRUE);
		      break;
		    }
		}

	case D_PCB:
		vaddr = hexarg(&arglist, 0, "Address of pcb: ", 0,
				0xffffffff, 0);
		if(phys(vaddr, &vaddr, ptb0) == SUCCESS)
			display_pcb(MAPPED(vaddr));
		break;

	case D_PROC:
		vproc = hexarg(&arglist, 0, "Address of Process: ",
				0, 0xffffffff, 0);
		display_proc(vproc);
		break;

	case D_PROCESSOR:
		vproc = hexarg(&arglist, 0, "Address of Processor: ",
				0, 0xffffffff, 0);
		display_processor(vproc);
		break;

        case D_REGISTERS: 
		display_registers();
		break;

	case D_NFSNODE: 
		vnfsnode = hexarg(&arglist, 0, "Address of nfsnode: ", 0,
				0xffffffff, 0);
		if(phys(vnfsnode, &pnfsnode, ptb0) == SUCCESS)
			display_nfsnode(MAPPED(pnfsnode));
		break;

	case D_NFSNODE_HASH:
		vaddr = hexarg(&arglist, 0, "Address of nfsnode: ", 0,
				0xffffffff, 0);
		display_nfshash_chain((struct inode *)vaddr);
		break;

	case D_MOUNT: 
		vmount = hexarg(&arglist, 0, "Address of mount: ", 0,
				0xffffffff, 0);
		if(phys(vmount, &pmount, ptb0) == SUCCESS)
			display_mount(MAPPED(pmount));
		break;

	case D_UFSMOUNT: 
		vmount = hexarg(&arglist, 0, "Address of ufsmount: ", 0,
				0xffffffff, 0);
		if(phys(vmount, &pmount, ptb0) == SUCCESS)
			display_ufsmount(MAPPED(pmount));
		break;

	case D_NFSMOUNT: 
		vmount = hexarg(&arglist, 0, "Address of nfsmount: ", 0,
				0xffffffff, 0);
		if(phys(vmount, &pmount, ptb0) == SUCCESS)
			display_nfsmount(MAPPED(pmount));
		break;

	case D_MNT_VNODELIST: 
		vmount = hexarg(&arglist, 0, "Address of mount: ", 0,
				0xffffffff, 0);
		if(phys(vmount, &pmount, ptb0) == SUCCESS)
			display_mnt_vnodelist(MAPPED(pmount));
		break;

	case D_NFSREQ: 
		vaddr = hexarg(&arglist, 0, "Address of nfsreq: ", 0,
				0xffffffff, 0);
		if(phys(vaddr, &paddr, ptb0) == SUCCESS)
			display_nfsreq(MAPPED(paddr));
		break;

	case D_NFSREQ_LIST: 
		vaddr = hexarg(&arglist, 0, "Address of nfsreq: ", 0,
				0xffffffff, 0);
		display_nfsreq_list(vaddr);
		break;

	case D_NFSHOST: 
		vaddr = hexarg(&arglist, 0, "Address of nfshost: ", 0,
				0xffffffff, 0);
		if(phys(vaddr, &paddr, ptb0) == SUCCESS)
			display_nfshost(MAPPED(paddr));
		break;

	case D_NFSHOST_LIST: 
		vaddr = hexarg(&arglist, 0, "Address of nfshost: ", 0,
				0xffffffff, 0);
		display_nfshost_list(vaddr);
		break;

	case D_SOCKET:
		vaddr = hexarg(&arglist, 0, "Address of socket: ", 0,
				0xffffffff, 0);
		display_socket(vaddr);
		break;

	case D_SYMBOL_CACHE: 
		display_symbol_cache();
		break;

	case D_TASK: 
		vtask = hexarg(&arglist, 0, "Address of task: ", 
				0, 0xffffffff, 0);

		if(phys(vtask, &ptask, ptb0) == SUCCESS)
			display_task(MAPPED(ptask));
		break;

	case D_TCPCB:
		vaddr = hexarg(&arglist, 0, "Address of tcpcb: ", 0,
				0xffffffff, 0);
		if(phys(vaddr, &vaddr, ptb0) == SUCCESS)
			display_tcpcb(MAPPED(vaddr));
		break;

	case D_TCPCHAIN:
		display_tcp_chain();
		break;

	case D_THREAD: 
		vth = hexarg(&arglist, 0, "Address of thread: ", 0,
			0xffffffff, 0);
		if(phys(vth, &pth, ptb0) == SUCCESS)
			display_thread(MAPPED(pth));
		break;

	case D_UTASK: 
		vutask = hexarg(&arglist, 0, "Address of utask structure: ",
				0, 0xffffffff, 0);
		display_utask(vutask);
		break;

	case D_UTHREAD:
	       vuthread = hexarg(&arglist, 0, "Address of uthread structure: ",
				0, 0xffffffff, 0);

		if(phys(vuthread, &puthread, ptb0) == SUCCESS)
			display_uthread(MAPPED(puthread));
		break;

	case D_VM_MAP: 
		mapv = hexarg(&arglist,0, "Address of map: ", 0, 0xffffffff,0);
		display_vm_map(mapv);
		break;

	case D_VM_OBJECT: 
		vobj = hexarg(&arglist,0, "Address of object: ", 0, 0xffffffff,0);
		display_vm_object(vobj);
		break;

	case D_VNODE: 
		vvnode = hexarg(&arglist, 0, "Address of vnode: ", 0,
				0xffffffff, 0);
		if(phys(vvnode, &pvnode, ptb0) == SUCCESS)
			display_vnode(MAPPED(pvnode), vvnode);
		break;

	case D_VNODE_CLNLIST: 
		vvnode = hexarg(&arglist, 0, "Address of vnode: ", 0,
				0xffffffff, 0);
		if(phys(vvnode, &pvnode, ptb0) == SUCCESS)
			display_vnode_clnlist(MAPPED(pvnode));
		break;

	case D_VNODE_DRTYLIST: 
		vvnode = hexarg(&arglist, 0, "Address of vnode: ", 0,
				0xffffffff, 0);
		if(phys(vvnode, &pvnode, ptb0) == SUCCESS)
			display_vnode_drtylist(MAPPED(pvnode));
		break;

	case D_VNODE_HASHCHAIN: 
		vvnode = hexarg(&arglist, 0, "Address of vnode: ", 0,
				0xffffffff, 0);
		if(phys(vvnode, &pvnode, ptb0) == SUCCESS)
			display_vnode_hashchain(MAPPED(pvnode));
		break;

        case D_ZONE:
		vzone = hexarg(&arglist, 0, "Address of zone: ",
			       0, 0xffffffff,0);
		display_zone(vzone);
		break;

	case D_MBUF: 
		vaddr = hexarg(&arglist, 0, "Address of mbuf: ", 0,
				0xffffffff, 0);
		if(phys(vaddr, &paddr, ptb0) == SUCCESS)
			display_mbuf(MAPPED(paddr));
		break;

	case D_XPR_BUFFER:
		display_xpr_buffer();
		break;

	case D_Z256:
		display_z256();
		break;

	default:
		printf("mda: Internal error - item_selected = %d\n",
			item_selected);
		break;
	}
}



find_cmd(arglist)
char *arglist;
{
#define dump_buf_size 1024	/* size of dump_val_buf */

	int result, i, y;
	int x = 0;
	int dump_val_buf[dump_buf_size];
	int stop_address;
	char *start_val;
	char *stop_val;
	char *val_to_find;
	int dump_value;

	dohist("find", arglist);
	val_to_find = (char *)hexarg(&arglist, 0, "Hex Data to search for: ",
		0, 0xffffffff, 0);

	if(*arglist == '\0') {
		(int *)start_val = 0;
		(int *)stop_val = (int *)dump_file_size;
	} else {
		start_val = (char *)hexarg(&arglist, 0, " ", 0xffffffff, 0);
		stop_val = (char *)hexarg(&arglist, 0, " ", 0xffffffff, 0);
	}

	printf("\nValue of mask is currently 0x%x", mask_val);
	printf("\nSearching for 0x%x", val_to_find);
	fflush(stdout);

	dump_value = (int)dump_file_address + (int)start_val;
	stop_address = (int)dump_file_address + (int)stop_val;

	while( (dump_value < stop_address) && (x < dump_buf_size) ) {
		if((*(int *)dump_value) == (int)val_to_find)
			dump_val_buf[x++] = (dump_value - dump_file_address)
					    & mask_val;

		if( (dump_value-(int)dump_file_address) % (1024*1024) == 0) {
			printf(".");
			fflush(stdout);
		}

		dump_value +=4;
	}

	if(x != 0) {
		printf("\nmda: Found %x at physical address", val_to_find);

		for (y = 0; y < x; y++) {
			if( (y%6) == 0)
				printf("\n");

			printf("0x%-8x   ", dump_val_buf[y]);
		}
	} else
		printf("\mda: 0x%x not found", val_to_find);

	printf("\n");
	return(SUCCESS);
}



extern char	last_symbol_shown[];



lookup_cmd(arglist)
char *arglist;
{
	char	data_name[256];
	char	buf[256];
	int	result, value;

	dohist("lookup", arglist);
	if(dump_fd == -1) {
		printf("No dump file read yet\n");
		return(FAILED);
	}

	(void) strarg(&arglist, 0, "symbol to show", last_symbol_shown, buf);
	data_name[0] = '_';
	strcpy(&data_name[1], buf);
	result = get_address(data_name, &value);
	switch(result) {
	case NO_SUCH_SYMBOL:
		printf("mda: No such symbol (%s)\n", data_name);
		return(FAILED);

	case FAILED:
		printf("mda: get_address(%s) failed\n", data_name);
		return(FAILED);

	case SUCCESS:
		printf("%s at address %#x\n", buf, value);
		return(SUCCESS);

	default:
		printf("mda: Internal error in lookup_cmd\n");
		return(FAILED);

	}
}



quit_cmd(arglist)
char *arglist;
{
	exit(0);
}

char *start_text, *text_end;	/* Used to validate pc's   */
char *master_rett;		/* Special trap return adr */
caddr_t	panicstr_adr;		/* Address of panic string */
vm_offset_t	panicstr;	/* Pointer to panic string */
char *intstkbeg;		/* Beginning of int stacks */
char *intstkend;		/* End of interrupt stacks */
int	nsysent;		/* Highest syscall number  */
int	OSmodpsr;		/* OS mod psr */
processor_t	master_processor;	/* corresponds to master_cpu */

extern char kernel_file[];
extern char dump_file[];



read_cmd(arglist)
char *arglist;
{
	int	result;
	char arg[MAXPATHLEN];

	dohist("read", arglist);
	if(*arglist == NULL)
		strcpy(&arg[0], dump_file);
	else
		strcpy(&arg[0], arglist);

	if(dump_fd != -1) {
		printf("mda: dump file already read\n");
		return(FAILED);
	}

	result = map(&arg[0], &dump_fd, &dump_file_size, &dump_file_address);
	if(result == FAILED) {
		printf("mda: Could not map dump file %s\n", arg);
		return(FAILED);
	}
	setup_global_symbols();
	result = get_address("_etext", &text_end);
	if(result != SUCCESS) {
		printf("mda: Could not get address of 'text_end'\n");
		return(FAILED);
	}
	result = get_address("_paniccpu", &panic_cpu);
	if(result == FAILED) {	
		panic_cpu = -1;
		printf("mda: Could not determine panic cpu\n");
	} else
		panic_cpu = MAP(panic_cpu);
	result = get_address("_panicstr", &panicstr_adr);
	if(result == FAILED) {	
		printf("mda: Could not determine panic string\n");
	} else {	
		panicstr_adr += dump_file_address;
		panicstr = (*(vm_offset_t *)panicstr_adr + dump_file_address);
		if(*(int *)panicstr_adr == (int)0) {	
			printf("[mda: No panic string]\n");
		}
		else {   
			printf("[mda: Panic: ");
			if(panic_cpu != -1)
				printf("(Cpu %d): ", panic_cpu);
			printf("%s]\n", panicstr);
		}
	}
	result = get_address("_master_cpu", &master_cpu);
	if(result == FAILED) {	
		printf("mda: Could not determine master cpu\n");
		master_cpu = 0;
	}
	else {	
		master_cpu = MAP(master_cpu);
		if(panic_cpu != -1) {
			current_cpu = panic_cpu;
		}
		else {
			current_cpu = master_cpu;
		}
	}
	result = get_address("_master_processor", &master_processor);
	if(result == FAILED) {	
		printf("mda: Could not determine master processor\n");
		master_processor = 0;
	}
	else {	
		master_processor = (processor_t)MAP(master_processor);
	}
	result = get_address("_master_rett", &master_rett);
	if(result != SUCCESS) {
		printf("mda: Could not get address of 'master_rett'\n");
		return(FAILED);
	}
	initptbr();

	result = get_address("_intstack", &intstkbeg);
	if(result != SUCCESS) {
		printf("mda: Could not get address of 'intstack'\n");
		return(FAILED);
	}
	result = get_address("_eintstack", &intstkend);
	if(result != SUCCESS) {
		printf("mda: Could not get address of 'eintstack'\n");
		return(FAILED);
	}
	intstkbeg = (char *)(MAP(intstkbeg));
	intstkend = (char *)(MAP(intstkend));

	result = get_address("_nsysent", &nsysent);
	if(result != SUCCESS) {
		printf("mda: Could not get address of 'nsysent'\n");
		return(FAILED);
	}
	nsysent = MAP(nsysent);

	result = get_address("_OSmodpsr", &OSmodpsr);
	if(result != SUCCESS) {
		printf("mda: Could not get address of 'OSmodpsr'\n");
		return(FAILED);
	}
	OSmodpsr = MAP(OSmodpsr);

	result = get_address("_start_text", &start_text);
	if(result != SUCCESS) {
		printf("mda: Could not get address of 'start_text'\n");
		return(FAILED);
	}
	start_text = (char *) MAP(start_text);
}



struct timeval *time_adr = (struct timeval *)-1;
struct timeval *boottime_adr = (struct timeval *)-1;



times_cmd(arglist)
char *arglist;
{
	struct timeval *ptime_adr, *pbtime_adr;

	dohist("times", arglist);
	if(time_adr == (struct timeval *)-1) {
		if(get_address("_time", &time_adr) != SUCCESS)
			return(FAILED);
	}

	if(boottime_adr == (struct timeval *)-1) {
		if(get_address("_boottime", &boottime_adr) != SUCCESS)
			return(FAILED);
	}

	if(phys(time_adr, &ptime_adr, ptb0) != SUCCESS)
		return(FAILED);

	if(phys(boottime_adr, &pbtime_adr, ptb0) != SUCCESS)
		return(FAILED);

	ptime_adr = (struct timeval *)(MAPPED(ptime_adr));
	pbtime_adr = (struct timeval *)(MAPPED(pbtime_adr));

	printf("Booted at: %s", ctime(pbtime_adr));
	printf(" Crash at: %s", ctime(ptime_adr));

}



extern char *kernel_address;



char *trace_options[] = {
#define T_CPU 0
	"cpu",
#define T_THREAD (T_CPU+1)
	"thread",
#define T_PC (T_THREAD+1)
	"pc",
	0
};



int is_kernel_thread();



trace_cmd(arglist)
char *arglist;
{
	int result, item_selected, i, disp, last_routine_size;
	int symbol_size, cpu_save = current_cpu;
	char *pc, *psa, *symbol, *instr, *instr_adr, *psr;
	char *pstkp, *stkp, *last_routine, *call_adr;
	int value, diff, num_regs;
	boolean_t is_kernel_thread;

	dohist("trace", arglist);
	if(dump_fd == -1) {
		printf("No dump file read yet\n");
		return(FAILED);
	}

	if(*arglist != NULL) {
		item_selected = stabarg(&arglist, 0, "stack source",
		&trace_options[0], "cpu");
		switch(item_selected) {
		case T_CPU:
			{
				int cpu_num; 

				if (*arglist) {
					cpu_num = intarg(&arglist, 0,"Trace cpu: ", 0, NCPUS, current_cpu);
					current_cpu = cpu_num;
				}
				result = get_address("_psa", &psa);
				if(result != SUCCESS) {
					printf("mda: Cannot get address of ");
					printf("panic save area\n");
					current_cpu = cpu_save;
					return(SUCCESS);
				}
				cpu_data(psa, &pc, &psr, &stkp);
				stkp += 48;	/* skip nmi stack frame */
				/*   result = check_console_panic(&stkp);   */
				if(result == FAILED) {
					current_cpu = cpu_save;
					return(FAILED);
				}
				break;
			}

		case T_PC:
			{
				char *vpc, *vsp;

				pc = (char *)hexarg(&arglist, 0, 
					"Program Counter: ",

				0, 0xffffffff, 0);
				stkp = (char *)hexarg(&arglist, 0, 
					"Stack Pointer: ",

				0, 0xffffffff, 0);

				psr = (char *) OSmodpsr; /* Fake in kernel */
				break;
			}

		case T_THREAD:
			{
				thread_t vth, pth;
				struct pcb  *vpcb, *ppcb;

				vth = (thread_t)hexarg(&arglist, 0, 
					"Address of thread: ",
					0, 0xffffffff, 0);

				if(vth < text_end) {
					printf("mda: Invalid thread ");
					printf("address %x\n", vth);

					return(SUCCESS);
				}
				result = phys(vth, &pth, ptb0);
				if(result != SUCCESS) {
					printf("mda: Could not get physical ");
					printf("address of thread\n");

					return(FAILED);
				}
				pth = (thread_t)(MAPPED(pth));
				vpcb = pth->pcb;
				result = phys(vpcb, &ppcb, ptb0);
				if(result != SUCCESS) {
					printf("mda: Could not get physical ");
					printf("address of pcb\n");

					return(FAILED);
				}
				ppcb = (struct pcb *)(MAPPED(ppcb));
				psr = (char *)(ppcb->pcb_modpsr);
				stkp = (char *)(ppcb->pcb_ssp);
				PHYS(pstkp, stkp);
				pc = (char *)MAP(pstkp);
				if (pc == 0) {
				    printf("mda: can't trace, pc = 0\n");
				    return(FAILED);
				}
				break;
			}


		default:
				printf("mda: Internal error - ");
				printf("item_selected = %d\n",

				item_selected);
				break;
		}
	}

	if (((int)psr & PSL_U) == 0)
		trace(stkp, pc);
	else
		printf("CPU %d executing in user mode at pc %#x\n",
		       current_cpu, pc);

	current_cpu = cpu_save;
	return(SUCCESS);

}



char *translate_options[] = {
#define TR_THREAD 0
	"thread",
#define TR_PTB (TR_THREAD+1)
	"ptb",
	0
};



translate_cmd(arglist)
char *arglist;
{
	char *paddr, *vaddr;
	int result;
	struct thread *vth, *pth;
	struct pcb *vpcb_ptr, *ppcb_ptr;
	int vptb, ptb, item_selected;

	dohist("translate", arglist);
	if(dump_fd == -1) {
		printf("No dump file read yet\n");
		return(FAILED);
	}
	vaddr = (char *)hexarg(&arglist, 0, "Address to translate: ", 
		0, 0xffffffff, 0);

	ptb = ptb0;			/* Initialize to kernel map */
	if(*arglist != NULL) {
		item_selected = stabarg(&arglist, 0, "ptb register",
		&translate_options[0], "thread");
		switch(item_selected) {
		case TR_THREAD:
				vth = (thread_t)hexarg(&arglist, 0, 
					"Address of thread: ",
					0, 0xffffffff, 0);

				result = phys(vth, &pth, ptb0);
				if(result != SUCCESS) {
					printf("mda: Could not translate ");
					printf("thread adr %#x\n", vth);

					return(FAILED);
				}
				pth = (thread_t)(MAPPED(pth));
				vpcb_ptr = pth->pcb;
				result = phys(vpcb_ptr, &ppcb_ptr, ptb0);
				if(result != SUCCESS) {
					printf("mda: Could not translate ");
					printf("pcb adr %#x\n", vpcb_ptr);

					return(FAILED);
				}
				ppcb_ptr = (struct pcb *)(MAPPED(ppcb_ptr));
				ptb = (int)(ppcb_ptr->pcb_ptbr);
				break;

		case TR_PTB:
				ptb = (int)hexarg(&arglist, 0, 
					"Address of ptb register: ",
					0, 0xffffffff, 0);

				break;

		default:
				printf("mda: Internal error, invalid ");
				printf("item (%d) selected\n", item_selected);

				return(FAILED);
		}
	}
	if(phys(vaddr, &paddr, ptb) != SUCCESS) {
		printf("mda: Could not translate virtual ");
		printf("address 0x%x\n", vaddr);

		return(FAILED);
	} else
		printf("%x : %x\n", vaddr, paddr);
	return(SUCCESS);
}
