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
 *	@(#)$RCSfile: devdriver.h,v $ $Revision: 1.2.17.18 $ (DEC) $Date: 1993/12/21 22:41:57 $
 */ 
/*
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * New in OSF/1
 */

/*
 * Abstract:
 *	This module contains the definitions for the various data structures
 *	used by device drivers and configuration code.
 *
 * Revision History
 *
 *	25-Jan-1991	Mark Parenti (map)
 *		Original Version
 */

#ifndef DEVDRIVER_HDR
#define DEVDRIVER_HDR

#include <sys/types.h>

struct bus_ctlr_common {
	u_long *mbox;
	struct bus *bus_hd;
        int (*badaddr)();
};

typedef struct bus_ctlr_common * bus_ctlr_common_t;

#ifdef KERNEL
#include <io/common/handler.h>	
#include <sys/proc.h>
#include <mach/machine/vm_types.h>
#endif /* KERNEL */

#ifdef KERNEL
extern struct bus       *system_bus;    /* Pointer to nexus bus structure */
#endif /* KERNEL */
/*
 * Define the MP lock macros for the hardware topology tree created
 * with the structures defined in the header file.  This lock is
 * intended to protect the entire topology tree since access to the
 * tree will most likely be very seldom.  Also, static initialization
 * of the tree need not get the lock since we are known to be single
 * threaded (not multi-user).
 */
#ifdef KERNEL
extern lock_data_t topology_tree_lock;
#define LOCK_TOPOLOGY_TREE	lock_write(&topology_tree_lock)
#define UNLOCK_TOPOLOGY_TREE	lock_done(&topology_tree_lock)
#endif /* KERNEL */

/* bus structure - 
 *
 *	The bus structure is used to describe a bus entity. A bus is an entity
 *	(real or imagined) to which other buses or controllers are logically
 *	attached. All systems have at least one bus, the system bus.
 */

struct	bus {
	u_long		*bus_mbox;
	struct bus	*nxt_bus;	/* next bus			*/
	struct controller *ctlr_list;	/* controllers connected to this bus */
	struct bus	*bus_hd;	/* pointer to bus this bus connected to */
	struct bus	*bus_list;	/* buses connected to this bus */
	int		bus_type;	/* bus type			*/
	char		*bus_name;	/* bus name 			*/
	int		bus_num;	/* bus number			*/
	int		slot;		/* node or slot number		*/
	char		*connect_bus;	/* bus connected to		*/
	int		connect_num;	/* bus num connected to		*/
	int		(*confl1)();	/* Level 1 configuration routine */
	int		(*confl2)();	/* Level 2 configuration routine */
	char		*pname;		/* port name, if needed		*/
	struct port	*port;		/* pointer to port structure	*/
	int		(**intr)();	/* interrupt routine(s) for this bus */
	int		alive;		/* See bit definitions below	*/
	struct bus_framework *framework; /* Bus rtns for loadable drivers*/
	char            *driver_name;   /* Name of controlling driver */
	void		*private[8];	/* Reserved for this bus use	*/
	void		*conn_priv[8];	/* Reserved for connected bus use */
        void            *bus_bridge_dma;/* Field used to signify Hardware DMA */  
	void		*rsvd[7];	/* Reserved for future expansion */
};

typedef	unsigned long	io_handle_t;

/* bus info structure
 *
 *	The bus info structure is used to pass certain information between
 *	a bus and another bus or an adapter connected in an hierarchical
 *	manner. The flow of information is from parent to child. 
 *	Typically the bus info structure is parent bus specific, i.e. it
 *	contains information the parent bus knows about and wants to pass
 *	on to its children. It does not contain information specific to
 *	the child. Since the information contained in the structure is
 *	specific to the parent bus its format is defined by that bus
 *	and will most likely vary from bus to bus. However there is a
 *	common set of data that all busses will know and will want to
 *	pass to there children. This information is contained in the
 *	common_bus_info structure defined below. The first element of
 *	every bus info structure should be a pointer to this
 *	common_bus_info structure.
 *	
 *	+----------------------------------------+
 *	|  Pointer to common info block  	 |
 *	|  (this is a struct common_bus_info *)  |
 *	+----------------------------------------|
 *	|  Bus specific data block elements      |
 *	|		    :			 |
 *	|		    :			 |
 *	+----------------------------------------+
 *
 *	Those busses that do not have any bus specific data can use
 *	the bus_info_hdr structure defined below as their bus info
 *	structure.
 */

struct	common_bus_info 
        {
	int		(**intr)();	/* Intr. routine for the child bus */
					/* adapter. 			   */
	io_handle_t	sparse_io_base; /* Base address of child's bus-io */
					/* space.  			  */
	io_handle_t	sparse_mem_base;/* Base address of child's */
					/* bus-memory space.       */ 
	vm_offset_t	dense_mem_base;	/* Base address of child's */
					/* bus-memory  space.      */
	};

struct	bus_info_hdr
   {
   struct  common_bus_info	*common_infop;
   };


/* bus function table
 *
 *	Table of bus specific functions used by device drivers. This table
 *	is loaded by the bus configuration code and is then attached to the
 *	bus' bus structure at private[0]. The wrapper functions (in
 *	driver_support.c), which all have a device's controller function as
 *	an argument, follow the bus_hd pointer to get to the bus structure
 *	and then to the correct function for the bus they are attached to.
 */

struct	bus_funcs
   {
   void	(*do_config)();		/* Function to initialize option. */
   int	(*get_config)();	/* Function to get config info for option */
   void	(*enable_option)();	/* Function for enabling option interrupts */
   void	(*disable_option)();	/* Function for disabling option interrupts */
   io_handle_t (*busphys_to_iohandle)(); /* Function to generate an io handle
					     from a bus physical address */
   };

/*
 *	Location in ctlr.conn_priv[] where we plug in the pointer to the
 *	bus function table.
 */
#define	busfuncs	private[0]


/* controller structure - 
 *
 *	The controller structure is used to describe a controller entity. A
 *	controller is an entity which connects logically to a bus. A controller
 *	may control devices which are directly connected, such as disks or
 *	tapes, or may perform some other controlling operation such as 
 *	network interface.
 */

struct	controller {
	u_long		*ctlr_mbox;
	struct controller *nxt_ctlr;	/* pointer to next ctlr on this bus */
	struct device	*dev_list;	/* devices connected to this ctlr */
	struct bus	*bus_hd;	/* pointer to bus for this ctlr   */
	struct driver	*driver;	/* pointer to driver structure for */
					/* this controller 		   */
	int		ctlr_type;	/* controller type		*/
	char		*ctlr_name;	/* controller name		*/
	int		ctlr_num;	/* controller number		*/
	char		*bus_name;	/* bus name			*/
	int		bus_num;	/* bus number connected to 	*/
	int		rctlr;		/* remote controller number	*/
					/* e.g. ci node or scsi id	*/
	int		slot;		/* node or slot number		*/
	int		alive;		/* See bit definitions below	*/
	char		*pname;		/* port name			*/
	struct port	*port;		/* port structure		*/
	int		(**intr)();	/* interrupt routine(s) for this ctlr */
	caddr_t		addr;		/* virtual address of controller */
	caddr_t		addr2;		/* virtual address of second ctlr */
					/* register space		  */
	int		flags;		/* flags from config line	*/
	int		bus_priority;	/* bus priority from from config */
	int		ivnum;		/* interrupt vector number	*/
	int		priority;	/* system ipl level		*/
	int		cmd;		/* cmd for go routine		*/
	caddr_t		physaddr;	/* physical address of addr	*/
	caddr_t		physaddr2;	/* physical address of addr2	*/
	void		*private[8];	/* Reserved for ctlr use	*/
	void		*conn_priv[8];	/* Reserved for connected bus use */
	void		*rsvd[8];	/* reserved for future expansion */
};

/* device structure -
 *
 *	The device structure is used to describe a device entity. A device
 *	is an entity that connects to, and is controlled by, a controller.
 */

struct	device {
	struct device	*nxt_dev;/* pointer to next dev on this ctlr */
	struct controller *ctlr_hd;	/* pointer to ctlr for this device */
	char		*dev_type;	/* device type			*/
	char		*dev_name;	/* device name			*/
	int		logunit;	/* logical unit	number		*/
	int		unit;		/* physical unit number		*/
	char		*ctlr_name;	/* controller name connected to */
	int		ctlr_num;	/* controller number for this device */
	int		alive;		/* See bit definitions below	*/
	void		*private[8];	/* reserved for device use	*/
	void		*conn_priv[8];	/* Reserved for connected controller use */
	void		*rsvd[8];	/* reserved for future expansion */
};

/* Defines for device private structures */
#define perf private[1]

/* port structure - 
 *
 *	The port structure is used to contain information about a port.  A port
 *	is ??
 */

struct	port {
	int	(*conf)();		/* config routine for this port */
};

/* driver structure - 
 *
 *	The driver structure contains information about driver entry points
 *	and other driver-specific information.
 */

struct	driver {
	int	(*probe)();		/* see if a driver is really there */
	int	(*slave)();		/* see if a slave is there */
	int	(*cattach)();		/* controller attach routine */
	int	(*dattach)();		/* device attach routine */
	int	(*go)();		/* fill csr/ba to start transfer */
	caddr_t	*addr_list;		/* device csr addresses */
	char	*dev_name;		/* name of device which connects to */
					/* this controller		    */
	struct	device **dev_list;	/* backpointers to driver structs */
					/* indexed with device logunit    */
	char	*ctlr_name;		/* name of controller */
	struct	controller **ctlr_list;	/* backptrs to controller structs */
					/* indexed with controller number */
	short	xclu;			/* want exclusive use of bdp's */
	int	addr1_size;		/* size of first csr area */
        int	addr1_atype;	 	/* address space of first csr area */
	int	addr2_size;		/* size of second csr area */
        int	addr2_atype;	 	/* address space of second csr area */
	int	(*ctlr_unattach)();	/* controller unattach routine */
	int	(*dev_unattach)();	/* device unattach routine */
};

struct dump_request {
	u_int		page_count;			/* Number of blocks we are going to dump */
	u_int		blk_offset;			/* Offset into the partition we are going to start dumping */
	u_int		unit;				/* Unit specifier for this device */
	dev_t		dump_dev;			/* Device we are doing the dump to */
	char		device_name[80];		/* String to pass to prom open */
	char		protocol[40];			/* What type of protocol, SCSI, MSCP, MOP ect */
	struct	device	*device;			/* Where we get all our device info */
};
/*
 * These are request for info calls through the cpusw function get_info.
 * This allows kernel code that needs system specfic code to request the 
 * info in a generic way without refering to cpu type.
 */
	
struct item_list {
	u_long		function;			/* Function code for this item; see below for supported codes */
	u_int		out_flags;    			/* Flags for output data */
	u_int		in_flags;			/* Flags for input data */
	u_long		rtn_status;			/* Status for the function code */
	struct item_list *next_function;		/* Pointer to next function request */
	u_long		input_data;			/* Pointer to input data */
	u_long		output_data;			/* Pointer to output data */
};	
/*
 * Type of info that can be requested 
 */
#define NOT_SUPPORTED   0x0000				/* This request type is not supported or getinfo call is not */
#define INFO_RETURNED   0x0001				/* This request type is supported and returned data */
#define IN_LINE		0x0001				/* The data is contained in the item list structure */

#define	TEGC_SWITCH	0x0001				/* return the address of the sw table for the TEGC driver */
#define	FBUS_INTREQ_REG	0x0002				/* return the address of the int request register for the machine */
#define MOP_SYSID	0x0003				/* MOP SYSID for a system */
#define GET_TC_SPEED	0x0004				/* Get the system Turbo-Channel speed */


#ifdef KERNEL
/*
 * These routines provide dynamic extensions to bus functionality.  They
 * are used to configure in loadable drivers and to register interrupt
 * handlers.
 */
struct bus_framework {
	int		(*ctlr_configure)();	/* Ctlr configure routine     */
	int		(*ctlr_unconfigure)();	/* Ctlr unconfigure routine   */
	int		(*adp_unattach)();	/* Bus remove adapter routine */
	int		(*config_resolver)();	/* Autoconfig bus resolver    */
        ihandler_id_t   (*adp_handler_add)();	/* Register interrupt	      */
        int             (*adp_handler_del)();   /* De-register interrupt      */
        int             (*adp_handler_enable)(); /* Enable interrupt handler  */
        int             (*adp_handler_disable)(); /* Disable interrupt handler*/
	caddr_t		rsvd[8];		/* Rsvd for future expansion  */
	caddr_t		private[8];		/* Reserved for driver use    */
};
#endif /* KERNEL */

/*
 * Loadable driver configuration data structure:
 *
 * After the stanza entry has been parsed to produce a bus, controller and
 * device lists, the elements of these lists are packaged up and passed down
 * to the kernel via setsysinfo.  In order to have a single form of setsysinfo
 * command the following structure is used to represent all of the different
 * types.  It consists of the structures themselves with an identifying type
 * and name fields.  The name field is needed so that you could have kernel
 * bus, ctlr, dev lists with these entries for different drivers.
 *
 * The bus, controller and device structures conatin pointers to names.  Such
 * as connecting bus name, connecting controller name, or port name.  In order
 * to make it easy to pass this structure to the kernel via setsysinfo, the
 * name fields will be copied into this structure and then the name pointers
 * within the bus/controller/device structures will be setup in the kernel
 * to point to these strings.
 */
#define MAX_NAME        100     	/* Maximum name length          */
struct config_entry {
	struct  config_entry *e_next;	/* Next entry in linked list    */
	char 	e_name[MAX_NAME];	/* Driver name 			*/
	int	e_type;			/* type = bus, ctlr, dev        */
	union {
		struct bus		e_bus;
		struct controller	e_controller;
		struct device		e_device;
	} e_str;
	char 	e_nm_1[MAX_NAME];	/* Name string			*/
	char 	e_nm_2[MAX_NAME];	/* Name string			*/
	char 	e_nm_3[MAX_NAME];	/* Name string			*/
	char 	e_nm_4[MAX_NAME];	/* Name string			*/
};

/* Bus type definitions	*/

#define	BUS_IBUS	1
#define	BUS_TC		2
#define	BUS_XMI		3
#define	BUS_BI		4
#define	BUS_UNIBUS	5
#define	BUS_QBUS	6
#define	BUS_VME	  	7
#define BUS_MSI		8
#define BUS_CI		9
#define BUS_LSB		10		/* laser system bus */
#define BUS_IOP		11		/* laser I/O adapter */
#define BUS_LAMB	12		/* laser to xmi adapter */
#define BUS_FLAG	13		/* laser to future bus adapter */
#define BUS_LBUS	14		/* cobra local I/O bus */
#define BUS_FBUS	15		/* future bus */
#define BUS_ISA		16		/* ISA bus. */
#define BUS_EISA	17		/* EISA bus. */
#define BUS_SCSI 		18	/* SCSI buses */

/* for the ADU */
#define BUS_TV          99


/* Access type definitions used for EISA and PCI bus */
#define	BUS_MEMORY	0
#define BUS_IO		1
#define DENSE_MEMORY    2

/* Alive field bit definitions */

#define	ALV_FREE	0x00000000	/* Device not yet processed  */
#define ALV_ALIVE	0x00000001	/* Device present and configured */
#define	ALV_PRES	0x00000002	/* Device present but no configed */
#define	ALV_NOCNFG	0x00000004	/* Device not to be configured	*/
#define	ALV_WONLY	0x00000008	/* Device is write-only		*/
#define	ALV_RONLY	0x00000010	/* Device is read-only		*/
#define ALV_LOADABLE	0x00000020	/* Device resolved by loadable */
#define ALV_NOSIZER	0x00000040	/* Sizer should ignore this item */

/* Define various constants used in configuring of loadable drivers */

#define	NEXUS_NUMBER		(-1)
#define LDBL_WILDNAME	"*"			/* Char field wildcard */
#define LDBL_WILDNUM		(-99)		/* Int field wildcard */
/*
 * Module types.
 */
#define BOGUS_TYPE	0x0		/* Invalid type			  */
#define BUS_TYPE	0x1		/* Bus type			  */
#define CONTROLLER_TYPE	0x2		/* Controller type		  */
#define DEVICE_TYPE	0x4		/* Device type			  */

/*----------------------------------------------------*/
/* Defines and function prototypes for bus functions. */
/*----------------------------------------------------*/
#define	RES_MEM		0
#define	RES_IRQ		1
#define	RES_DMA		2
#define	RES_PORT	3

extern	void  do_config (struct controller *ctlr_p);
extern	int   get_config(struct controller *ctlr_p, uint_t config_item,
			 char *func_type, void *data_p, int handle);
extern	void  enable_option (struct controller *ctlr_p);
extern	void  disable_option(struct controller *ctlr_p);
extern io_handle_t busphys_to_iohandle(u_long addr, int flags, struct controller *ctlr_p);


/**********************************************/
/* Function prototypes for io access routines */
/**********************************************/

#define HANDLE_BYTE	0x1
#define HANDLE_WORD	0x2
#define HANDLE_LONGWORD 0x4
#define HANDLE_QUADWORD 0x8
#define HANDLE_TRIBYTE  0x10
#define HANDLE_DENSE_SPACE	0x80000000
#define HANDLE_SPARSE_SPACE	0x40000000
#define HANDLE_BUSPHYS_ADDR     0x20000000  

extern int io_copyin(io_handle_t src, vm_offset_t dst, u_long length);
extern int io_copyout(vm_offset_t src, io_handle_t dst, u_long length);
extern int io_copyio(io_handle_t src, io_handle_t dst, u_long length);
extern int io_zero(io_handle_t dst, u_long length);
extern u_long iohandle_to_phys(io_handle_t io_handle, long flags);
extern io_handle_t busphys_to_iohandle(u_long addr, int flags, struct controller *ctlr_p);
extern long read_io_port(io_handle_t dev_addr, int width, int type);
extern void write_io_port(io_handle_t dev_addr, int width, int type, long data);
extern vm_offset_t get_io_handle();
extern int io_bcopy();

/**********************************************/
/* Function prototypes for read/write_io_port */
/**********************************************/
/* extern	unsigned long read_io_port(vm_offset_t dev_addr, int width, int type); */
extern	void write_io_port(vm_offset_t dev_addr, int width, int type, long data);

/*----------------------*/
/* Macros for io access */
/*----------------------*/

#define READ_BUSIO_D8(a)   ((unsigned char)(read_io_port(a, 1, BUS_IO)))
#define READ_BUSIO_D16(a)  ((unsigned short)(read_io_port(a, 2, BUS_IO)))
#define READ_BUSIO_D32(a)  ((unsigned int)(read_io_port(a, 4, BUS_IO)))
#define READ_BUSIO_D64(a)  ((unsigned long)(read_io_port(a, 8, BUS_IO)))
#define READ_BUSMEM_D8(a)  ((unsigned char)(read_io_port(a, 1, BUS_MEMORY)))
#define READ_BUSMEM_D16(a) ((unsigned short)(read_io_port(a, 2, BUS_MEMORY)))
#define READ_BUSMEM_D32(a) ((unsigned int)(read_io_port(a, 4, BUS_MEMORY)))
#define READ_BUSMEM_D64(a) ((unsigned long)(read_io_port(a, 8, BUS_MEMORY)))
#define WRITE_BUSIO_D8(a,d)   ((void)(write_io_port(a, 1, BUS_IO, d)))
#define WRITE_BUSIO_D16(a,d)  ((void)(write_io_port(a, 2, BUS_IO, d)))
#define WRITE_BUSIO_D32(a,d)  ((void)(write_io_port(a, 4, BUS_IO, d)))
#define WRITE_BUSIO_D64(a,d)  ((void)(write_io_port(a, 8, BUS_IO, d)))
#define WRITE_BUSMEM_D8(a,d)  ((void)(write_io_port(a, 1, BUS_MEMORY, d)))
#define WRITE_BUSMEM_D16(a,d) ((void)(write_io_port(a, 2, BUS_MEMORY, d)))
#define WRITE_BUSMEM_D32(a,d) ((void)(write_io_port(a, 4, BUS_MEMORY, d)))
#define WRITE_BUSMEM_D64(a,d) ((void)(write_io_port(a, 8, BUS_MEMORY, d)))

/* io_handle_t versions */
#define READ_BUS_D8(a)   ((unsigned char) (read_io_port(a, 1, 0)))
#define READ_BUS_D16(a)  ((unsigned short)(read_io_port(a, 2, 0)))
#define READ_BUS_D32(a)  ((unsigned int)  (read_io_port(a, 4, 0)))
#define READ_BUS_D64(a)  ((unsigned long) (read_io_port(a, 8, 0)))
#define WRITE_BUS_D8(a,d)   ((void)(write_io_port(a, 1, 0, d)))
#define WRITE_BUS_D16(a,d)  ((void)(write_io_port(a, 2, 0, d)))
#define WRITE_BUS_D32(a,d)  ((void)(write_io_port(a, 4, 0, d)))
#define WRITE_BUS_D64(a,d)  ((void)(write_io_port(a, 8, 0, d)))


/* DMA SUPPORT -- structures, typedefs, define's	*/

/* bus-address/byte-count pair structure		*/
/* io bus addr. can be up to 64-bits, and byte-specific */
/* the byte count is signed 64-bits 			*/
typedef	char	*bus_addr_t;

struct	sg_entry {
	bus_addr_t	ba;	/* bus address                          */
	u_long		bc;	/* byte count from ma that contiguous
				   addresses are valid on the bus 	*/
};

typedef	struct sg_entry	*sg_entry_t;

/*
 * The sglist structure contains the control structure to access a
 * page of bus_address-byte_count pairs. The pointer to this structure
 * is passed back to drivers from dma_map_alloc() & dma_map_load().
 */

 /*
  *	+----------------------------------+
  *	|               sgp		   |
  *	+----------------+-----------------+
  *	|   num_ents     |     val_ents    |   -> each line 64-bits
  *	+----------------+-----------------+   -> fits in 32-byte cache block
  *	|      flags     |      index      |   -> each line 64-bits
  *	+----------------+--------+--------+   -> fits in 32-byte cache block
  *	|		next		   |
  * .---+----------------------------------+------------.
  * |	|		cntrlrp		   |   		|
  * V	+----------------------------------+		V
  *	|		va		   |	this section only added to
  *	+----------------------------------+    first sglist of babc list.
  *	|		private		   |	*** a struct ovrhd{} ***
  *	+----------------------------------+
  *	|		procp		   |
  *	+----------------------------------+
  *
  * So, a page of sglist's (256 structs) with assoc. pages of sg_entry's 
  * (512 per page, 8KB per pair, min.) will be able to describe 1GB (min.).
  */

struct	sglist	{
	sg_entry_t	sgp;	  /* ptr. to ba-bc pair (struct of)	 */
	int		val_ents; /* number of ba-bc pairs in babc list  */
	int		num_ents; /* loaded/valid ba-bc pairs in list    */
	int		index;	  /* last ba-bc pair read		 */
	int		flags;	  /* copy of flags from alloc/load	 */
	struct	sglist	*next;	  /* ptr. to next struct in linked list	 */
};

struct ovrhd {
	struct  controller *cntrlrp; /* requesting controller            */
	vm_offset_t	    va;	     /* saved va on dma_map_load	 */
	u_long		    private; /* bc on dma_map_load		 */
	struct	proc	   *procp;   /* proc ptr on dma_map_load	 */
};

/* Typedef for return value defs */
typedef	struct	sglist	*sglist_t;
typedef	struct	sglist	*dma_handle_t;

#ifdef KERNEL
/*
 * The dma_callsw structure contains pointers to the platform-dependent
 * functions that support drivers for DMA devices. The structure
 * is initialized by the platform-specific init function during bootstrap
 * (e.g., kn121_init() on JENSEN).
 * The generic functions are defined in locore.s and uses this jump
 * table (for now) to get to the platform-specific interfaces for these
 * functions.
 */

struct	dma_callsw	{
	u_long	(*hal_dma_map_alloc)();	  /* ptr to HAL's dma_map_alloc    */
	u_long	(*hal_dma_map_load)();	  /* ptr to HAL's dma_map_load     */
	int	(*hal_dma_map_unload)();  /* ptr to HAL's dma_map_unload   */
	int	(*hal_dma_map_dealloc)(); /* ptr to HAL's dma_map_dealloc  */
	int	(*hal_dma_min_bound)();   /* ptr to HAL's dma_min_boundary */
};
/*
 * dma_callsw defined in driver_support.c;
 * need this declaration for cpu-specific init modules-functions 
 */
extern	struct	dma_callsw	dma_callsw;

/*
 * The bus_bridge_dma_callsw structure contains pointers to the
 * bus bridge dma hardware mapping functionality used to do bus
 * mapping.
 */
struct  bus_bridge_dma_callsw      {
   unsigned long    (*dma_alloc)();
   unsigned long    (*dma_load)();
   int              (*dma_unload)();
   int              (*dma_dealloc)();
};



#endif	/* KERNEL */

/*
 * values for "flags" field in function calls, structures 
 */
#define	DMA_SLEEP	0x0001	/* SLEEP on dma_alloc/load call 	 */
#define DMA_GUARD_UPPER 0x0010  /* Add GUARD page at end of sg list	 */
#define DMA_GUARD_LOWER 0x0020  /* Add GUARD page at beg. of sg list	 */
#define DMA_ALL		0x0040  /* Get all resources req. or rtn failure */
#define DMA_IN		0x0100	/* device to main/system/core data xfer	 */
#define DMA_OUT		0x0200	/* main/system/core mem. to device xfer  */
#define DMA_DEALLOC	0x1000	/* dealloc. resources on unload		 */
#define DMA_DIRECT_MAP  0x0080  /* bus bridge flag "sglist chain"        */
/*
 * Prototype defs for drivers
 */
/* the following are implemented in locore.s (for performance reasons) */
extern	u_long	dma_map_alloc(u_long bc, struct controller *cntrlrp, 
				dma_handle_t *dma_handle_p, int flags);
extern	u_long	dma_map_load(u_long bc, vm_offset_t va, struct proc *procp, 
	struct controller *cntrlrp, dma_handle_t *dma_handle_p, 
			              u_long max_bc, int flags);
extern	int	dma_map_unload(int flags, dma_handle_t dma_handle);
extern	int	dma_map_dealloc(dma_handle_t dma_handle);
extern	int	dma_min_boundary(struct controller *cntrlrp);
/* the following are in driver_support.c */
extern	sg_entry_t	dma_get_next_sgentry(dma_handle_t dma_handle);
extern	sg_entry_t	dma_get_curr_sgentry(dma_handle_t dma_handle);
extern	int		dma_put_prev_sgentry(dma_handle_t dma_handle, sg_entry_t sg_entryp);
extern	int		dma_put_curr_sgentry(dma_handle_t dma_handle, sg_entry_t sg_entryp);
extern	vm_offset_t	dma_kmap_buffer(dma_handle_t dma_handle, u_long offset);
extern	int		dma_get_private(dma_handle_t dma_handle, int index, 
						u_long *data);
extern	int		dma_put_private(dma_handle_t dma_handle, int index,
						u_long data);

extern	void		drvr_shutdown();
extern	void		drvr_register_shutdown(void (*callback)(), caddr_t param, 
					       int flags);

/*
 * DMA Map support macros
 */
#define	DMA_MAP_BUF(A,B,C,D,E,F,G)	((u_long)dma_map_load(A, B, C, D, E, F, G))
#define	DMA_UNMAP_BUF(A)		((int)dma_map_dealloc(A))

/* Structure for driver shutdown routine callback */
struct drvr_shut {
	struct drvr_shut	*next;
	void			(*callback)();	/* callback routine */
	caddr_t			param;
};
	
#define	DRVR_REGISTER	0x00000001	/* register a callback routine */
#define	DRVR_UNREGISTER	0x00000002	/* de-register a callback routine */


/* External routine definitions */

#ifdef KERNEL
extern struct bus 		*get_sys_bus();
extern struct bus 		*get_bus();
extern struct bus 		*bus_search();
extern struct controller	*get_ctlr();
extern struct controller	*ctlr_search();
extern struct device		*get_device();
extern void			conn_ctlr();
extern void			conn_device();
extern void	 		conn_bus();
/* in Alpha at least, these are macros */
#ifndef __alpha
extern int			BADADDR();
extern int			DELAY();
#endif /* __alpha */
/* DMA memory allocation support routines */
extern	vm_offset_t	dma_zalloc(unsigned int size, int flags);
extern	vm_offset_t	dma_zalloc_super_sglist(int flags);
extern	vm_offset_t	dma_zalloc_sglist(int flags);
extern	void		dma_zfree(unsigned int size, vm_offset_t addr);
extern	void		dma_zfree_super_sglist(vm_offset_t addr);
extern	void		dma_zfree_sglist(vm_offset_t addr);
extern	void		dma_zones_init();
#endif /* KERNEL */

/* WBFLUSH define. */
#ifdef __alpha
#      define WBFLUSH()  mb()
#      define MB()       mb()
#else
#  ifdef __mips
#      define WBFLUSH()  wbflush()
#      define MB()       ;
#  else
#      define WBFLUSH()  ;
#      define MB()       ;
#  endif /* __mips */
#endif /* __alpha */


#endif /* DEVDRIVER_HDR */
