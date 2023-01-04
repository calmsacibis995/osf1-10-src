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
 * @(#)$RCSfile: config.h,v $ $Revision: 4.3.18.5 $ (DEC) $Date: 1993/10/29 21:11:44 $
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)config.h	5.3 (Berkeley) 4/18/86
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */


/*		Change History						*
 *									*
 * 27-Oct-91	Fred Canter						*
 *		Add maxssiz, dflssiz, and dfldsiz for configurable	*
 *		data and stack size limits.				*
 *		Make System V IPC definitions configurable		*
 *									*
 * 3-20-91	robin-							*
 *		Made changes to support new device data structures	*
 *		use to describe the system.				*
 *									*
 * 18-Oct-91	afd
 *		Add Alpha support.
 *
 * 6-June-1991	Brian Stevens
 *		Added new config file options maxuprc, bufcache,
 *		maxcallouts, and maxthreads (per task).	
 */

#ifndef _CONFIG_CONFIG_H_
#define _CONFIG_CONFIG_H_


#include <stdio.h>
#include <sys/types.h>
#include <mach/machine/vm_types.h>


#if !defined(GETUNIT)

/*
 * BOH - Beginning Of Hack 
 *
 * Here we are testing to see if the types.h on the native system 
 * (under /usr/include/sys) contains new dev_t annotation macros and types.  
 * An OSF system can be built on an ULTRIX or OSF system and here we make 
 * sure we have have new dev_t information available if we are building 
 * under ULTRIX.  Config is one of the few programs that needs to run 
 * under the native build system and thus has to use native include files.
 */

/*
 * Types used by dev_t annotation macros (see below)
 */
typedef unsigned int    uint_t;
typedef uint_t  major_t;      /* major device number   */
typedef uint_t  minor_t;      /* minor device number   */
typedef uint_t  devs_t;       /* device-specific info  */
typedef uint_t  unit_t;       /* unit number of device */


/*
 * Basic system types and major/minor device constructing/busting macros.
 */
#undef major
#undef minor
#undef makedev
#define major(x)        ((major_t)  (((uint_t)(x)>>20)&07777))
#define minor(x)        ((minor_t)  ((uint_t)(x)&03777777))
#define makedev(x,y)    ((uint_t)    (((major_t)(x)<<20) | (minor_t)(y)))

/*
 * Disk/Tape (SCSI/CAM - DSA) specific dev_t annotations macros.
 */
#define MAKEMINOR(u,d)  ((minor_t)  (((unit_t)(u)<<6) |(devs_t)(d)))
#define GETUNIT(dev)    ((unit_t)   (minor(dev)>>6)&037777)
#define GETDEVS(dev)    ((devs_t)   (minor(dev))&077)
#define MAKECAMMINOR(u,d) ((minor_t) MAKEMINOR((((u&030)<<5)|((u&07)<<4)),d))
#define GETCAMUNIT(x)   ((unit_t) (((GETUNIT(x))>>5)&030)|((GETUNIT(x)>>4)&07))
#define GETCAMTARG(x)   ((unit_t) ((x >> 3)&07))
 
/* EOH - End Of Hack */
#endif

/*
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY
 * on OSF systems.
 */
#undef NODEV
#define NODEV	((uint_t)-1)
#define	NNEEDS	10

struct file_list {
	struct	file_list *f_next;	
	char	*f_fn;			/* the name */
	int	f_flags;		/* see below */
	u_char	f_type;			/* see below */
	short	f_special;		/* requires special make rule */
	char	*f_needs[NNEEDS];
	char	*f_extra;		/* stuff to add to make line */
	char    *f_syscall;             /* tag used to make loadable syscall files */	
	char    *f_directory;		/* Directory name of root location of 3ed party stuff */
	/*
	 * Random values:
	 *	swap space parameters for swap areas
	 *	root device, etc. for system specifications
	 */
	union {
		struct {		/* when swap specification */

/*
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY
 * on OSF systems.
 */
			uint_t	fuw_swapdev;
			int	fuw_swapsize;
		} fuw;
		struct {		/* when system specification */
/*
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY
 * on OSF systems.
 */
			uint_t	fus_rootdev;
			uint_t	fus_argdev;
			uint_t	fus_dumpdev;
		} fus;
	} fun;
#define f_swapdev	fun.fuw.fuw_swapdev
#define f_swapsize	fun.fuw.fuw_swapsize
#define f_rootdev	fun.fus.fus_rootdev
#define f_argdev	fun.fus.fus_argdev
#define f_dumpdev	fun.fus.fus_dumpdev
};

/*
 * Types.
 */
#define DRIVER		1
#define NORMAL		2
#define INVISIBLE	3
#define PROFILING	4
#define SYSTEMSPEC	5
#define SWAPSPEC	6
#define SYSCALL		7
#define FLOAT		8


/*
 * MSCP/CAM-SCSI major numbers.
 */
#define MSCP_MAJ	23
#define SCSI_MAJ 	8	
#define MSCP_MAXDISK	255 
#define NONMSCP_MAXDISK 31

/*
 * Attributes (flags).
 */
#define CONFIGDEP	0x001
#define OPTIONSDEF	0x002	/* options definition entry */
#define ORDERED		0x004	/* don't list in OBJ's, keep "files" order */
#define SEDIT		0x008	/* run sed filter (SQT) */
#define DYNAMICF	0x010	/* dynamic module */
#define UNSUPPORTED	0x020	/* User software not supported by DEC */
#define	OBJS_ONLY	0x040	/* Object code only */
#define NOTBINARY	0x080	/* Cc it its marked Notbinary	*/
#define ISBINARY	0x100	/* File marked as Binary */
#define LAYERED		0x200	/* 3ed party supplied file */
#define NO_GLOBAL_PTR	0x400	/* no data access from global pointer */

/*
 * Maximum number of fields for variable device fields (SQT).
 */
#define NFIELDS		10

struct	idlst {
	char	*id;
	struct	idlst *id_next;
	int	id_vec;		/* Sun interrupt vector number */
};

struct device_entry {
	int	d_type;			/* CONTROLLER, DEVICE, UBA or MBA */
	char    *d_type_string;		/* Holds device name on "device" entries*/
	struct	device_entry *d_conn;		/* what it is connected to */
	char	*d_name;		/* name of device (e.g. rk11) */
	char	*d_dynamic;		/* name of dynamic target */
	struct	idlst *d_vec;		/* interrupt vectors */
	int	d_pri;			/* interrupt priority */
	int	d_addr;			/* address of csr */
	int     d_addr2;                /* address of csr2 */
 	int     d_ivnum;                /* first interrupt vector */ 
	int	d_slot;			/* slot or node number	*/
	int	d_unit;			/* unit number */
	int	d_port;			/* Port number */
	char	*d_port_name;		/* Port name string	*/
	int	d_drive;		/* drive number */
	int	d_slave;		/* slave number */
#define QUES	-1	/* -1 means '?' */
#define UNKNOWN -2	/* -2 means not set yet */
	int	d_rcntl;
	int	d_dk;			/* if init 1 set to number for iostat */
	int	d_flags;		/* flags for device init */
	int	d_adaptor;
	int	d_nexus;
	int	d_extranum;
	int	d_counted;		/* has header been written?    */
	struct	device_entry *d_next;		/* Next one in list */
        u_short d_mach;                 /* Sun - machine type (0 = all)*/
        u_short d_bus;                  /* Sun - bus type (0 = unknown) */
	u_long	d_fields[NFIELDS];	/* fields values (SQT) */
	int	d_bin;			/* interrupt bin (SQT) */
	int	d_addrmod;		/* address modifier (MIPS) */
	char	*d_wildcard;		/* marks entry connected to a wild card spec */
	int	d_disable;		/* Marks an entry as broken-don't touch */
};
#define TO_NEXUS	(struct device_entry *)-1
#define TO_SLOT		(struct device_entry *)-1

struct config {
	char	*c_dev;
	char	*s_sysname;
};

/* Callout to a shell structure (for config callouts used by 3ed party setups
 */
#define MAX_EVENT_LEVEL 10 
#define EVENT_LEVEL0 0		/* (at_start) Before config does anything (except parse the config file)	*/
#define EVENT_LEVEL1 1		/* (at_exit) As config exits (regardless of success or fail)		*/
#define EVENT_LEVEL2 2		/* (at_success) As config exits (only on normal exit)			*/
#define EVENT_LEVEL3 3		/* (before_h) Before dynamic 'h' files are made.				*/
#define EVENT_LEVEL4 4		/* (after_h) After the dynamic 'h' files are made.			*/
#define EVENT_LEVEL5 5		/* (before_makefile) Before the Makefile is created.				*/
#define EVENT_LEVEL6 6		/* (after_makefile) After the Makefile is created.				*/
#define EVENT_LEVEL7 7		/* (before_c) Before any of the 'c' files are copied/made			*/
#define EVENT_LEVEL8 8		/* (after_c) After all the 'c' files are in place				*/
#define EVENT_LEVEL9 9		/* (before_conf) Before conf.c is changed				*/
#define EVENT_LEVEL10 10	/* (after_conf) After conf.c is updated				*/

struct callout_data {
	int	event_class;
	char	*unix_command;
	struct callout_data *next, *last;
        } *callout_hd, *callout_lst, *callout_p;

/*
 * Config has a global notion of which machine type is
 * being used.  It uses the name of the machine in choosing
 * files and directories.  Thus if the name of the machine is ``vax'',
 * it will build from ``Makefile.vax'' and use ``../vax/inline''
 * in the makerules, etc.
 */
int	machine;
char	*machinename;
#define MACHINE_VAX	1
#define MACHINE_DEC_RISC    2
#define MACHINE_ALPHA 3

/*
 * For each machine, a set of CPU's may be specified as supported.
 * These and the options (below) are put in the C flags in the makefile.
 */
struct cputype {
	char	*cpu_name;
	struct	cputype *cpu_next;
} *cputype;

/*
 * In order to configure and build outside the kernel source tree,
 * we may wish to specify where the source tree lives.
 */
char *config_directory;
char *object_directory;

FILE *VPATHopen();
char *get_VPATH();
#define VPATH	get_VPATH()

/*
 * A set of options may also be specified which are like CPU types,
 * but which may also specify values for the options.
 * A separate set of options may be defined for make-style options.
 */
struct opt {
	char	*op_name;
	char	*op_value;
	char	*op_dynamic;
	struct	opt *op_next;
}  *opt,
   *mkopt,
   *opt_tail,
   *mkopt_tail;

/*
 * The file sys structure is to verify that at least one of the file
 * systems listed in the filesystems file is specified.
 */
struct file_sys {
	char *fs_name;
	struct file_sys *fs_next;
      } *file_sys;

char	*ident;
char	*ns();
char	*tc();
char	*qu();
char	*get_word();
char	*path();
char	*raise_to_upper();

int	do_trace;

char	*index();
char	*rindex();
char	*malloc();
char	*strcpy();
char	*strcat();

#if	MACHINE_VAX
int	seen_mba, seen_uba;
#endif

int	seen_vme, seen_mbii;

struct	device_entry *connect();
struct	device_entry *dtab;
/*
 * This is to hack around the fact that config may be built on an
 * ULTRIX system with an old (16-bit) dev_t in types.h.  Yuck!
 * The "uint_t" should be changed to "dev_t" when we build ONLY
 * on OSF systems.
 */
uint_t	nametodev();
char	*devtoname();

char	errbuf[80];
int	yyline;

struct	file_list *ftab, *conf_list, **confp;
char	*PREFIX;

long	timezone;
int	hadtz;
int	dst;
int	profiling;

int	processors;	/* Max number of processors in the system */
int	maxcallouts;
int	maxuprc;
int	maxproc;
int	bufcache;
int	ubcminpercent;
int	ubcmaxpercent;
u_char	cowfaults;
u_short mapentries;
vm_offset_t maxvas;
vm_offset_t maxwire;
u_char	heappercent;
u_short	anonklshift;
u_short	anonklpages;
u_long	vpagemax;
u_char  segmentation;
u_short	ubcpagesteal;
u_char	ubcdirtypercent;
u_char	ubcseqstartpercent;
u_char	ubcseqpercent;
vm_size_t csubmapsize;
u_short ubcbuffers;
u_short	swapbuffers;
u_long	clustermap;
u_long	clustersize;
vm_size_t zone_size;
vm_size_t kentry_zone_size;
u_char	syswiredpercent;


vm_size_t writeio_kluster;
vm_size_t readio_kluster;
int	maxthreads;
int	threadmax;
int	taskmax;
int	maxusers;
int	nclist;
int	sys_v_mode;
int	maxdsiz;
int	dfldsiz;
int	maxssiz;
int	dflssiz;
/* System V IPC definitions */
int	msgmax, msgmnb, msgmni, msgtql;
int	semmni, semmns, semmsl, semopm, semume, semvmx, semaem;
int	shmmin, shmmax, shmmni, shmseg;
double	release; /* Ultrix release number */
int	version; /* Version of that release */
int	highuba; /* highest uba number seen during config file parsing */
int	extrauba; /* number of extra unibuses we need to define */
#ifdef	notused
int	smmin;	 /* minumum shared memory segment size */
int	smmax;	 /* maximum shared memory segment size */
int	smbrk;	 /* number of VAX pages between end of data segment and
		    beginning of first shared memory segment */
int	smseg;	/* number of shared memory segments per process */
int	smsmat; /* highest attachable shared memory address */
int	maxtsiz; /* max size of text segment */
int	maxdsiz; /* max size of data segment*/
int 	maxssiz; /* max size of stack segment */
#endif
int 	swapfrag; /* max swap fragment size (in clicks)*/
int	vasslop;  /* slop for recovering when we run out of swap space (in clicks*/
int	maxretry; /* maximum retry count to try KM_ALLOC in dynamic swap */
int	maxuva; /* max aggrate user page table size */
int	emulation_instr; /* true if cpu MVAX or VAX3600 defined in config file */
struct  _scs_system_id {        /* SCS system identification number          */
    u_long      lol;            /*  Low order long                           */
    u_short     hos;            /*  High order short                         */
    } scs_system_id; 
u_long	scsid_l; /* low order four bytes of scsid */
u_short	scsid_h; /* high order two bytes of scsid */

#define eq(a,b)	(!strcmp(a,b))

#if defined mips || defined __alpha
#define DEV_MASK 0xf
#define DEV_SHIFT  4
#else	/* mips || __alpha */
#define DEV_MASK 0x7
#define DEV_SHIFT  3
#endif	/* mips || __alpha */

struct bus_info {
int max_bus_num;
int cnt;
};

extern  int     kdebug;
extern	int	source;

struct bus_info vaxbi_bus;
struct bus_info xmi_bus;
struct bus_info ci_bus;
struct bus_info msi_bus;
struct bus_info uba_bus;
struct bus_info ibus_bus;
struct bus_info vba_bus;

struct unique_list {
	char *name;
	int   number;
	struct device_entry *dp;
	struct unique_list *next;
};

struct dynlist {
	char *target;
	char *objects;
	struct dynlist *next;
};


struct file_state {
	char *fs_fname;		/* filename we are reading */
	FILE *fs_fp;		/* open file pointer */
	char *fs_wd;		/* next word in the input stream */
	char *fs_this;		/* entry that we are now processing */
	char *fs_dynamic;	/* dynamic module to be linked into */
	char *fs_syscall;	/* syscall module tag used to create files */
	struct device_entry *fs_dev;	/* device when this is an option */
	char *fs_directory;	/* Directory name for root entry of 3ed party stuff */
	int fs_type;		/* type of entry to create */
	int fs_flags;		/* flags for entry */
	int fs_define_dynamic;	/* want _DYNAMIC option for device */
} file_state;


/* 
 * Definitions to support configuration of optional kernel libraries
 * in the machine architecture specific files file.
 */

#define DEPEND_MAX		32
#define LIB_MAX			32
#define SINGLE_CPU		 1
#define LERROR			-1
#define LIB_PREFIX 		"LIB/lib_"
#define LQUALFR     		".a"
#define TRUE			1
#define FALSE			0
#define SUCCESS			0

struct lib_req {
	struct lib_req *next;
	char depend[DEPEND_MAX];
}; 

struct cpu_lib{
	struct cpu_lib *next;
	struct lib_req *lib_req;
	char lib[LIB_MAX]; 
};  

struct cpu_lib *cpu_lib_ptr, *cpu_lib_head;

int do_set_libs();
int add_lib();
int add_lib_req();
void lib_root();

struct orphen_dev{
	struct orphen_dev *next;
	char *dev;
	int num;
	char *dev_to;
	int num_to;
	int type;
};

#endif 
