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
static char *rcsid = "@(#)$ $ (DEC) $";
#endif

#include <vm/vm_kern.h>
#include <machine/machparam.h>
#include <io/dec/fbus/fbusreg.h>
#include <hal/cpuconf.h>
#include <io/common/devdriver.h>

extern struct fbus_option Fbus_option[];
extern struct fbus_node Fbus_node[];
extern	struct controller controller_list[];

/*
 * Define all routines vectored via the loadable framework
 * extension.
 */

char *	fbus_handler_add();
int		fbus_handler_del();
int		fbus_handler_enable();
int		fbus_handler_disable();

struct bus_framework Fbus_loadable_framework = {
	0, 0, 0, 0, 
	fbus_handler_add, fbus_handler_del,
	fbus_handler_enable, fbus_handler_disable,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0 };

#define FBUS_DEBUG
#ifdef FBUS_DEBUG
int fbusdebug = 0;
#define Dprintf(x) if(fbusdebug){printf x;DELAY(500000); }
#define Cprintf(x) if(fbusdebug == 1){printf x;DELAY(500000);} 
static void dumpnode_tbl_ent( /* bus *, int */ );
#else /* FBUS_DEBUG */
#define Dprintf 
#define Cprintf 
#endif /* FBUS_DEBUG */

static void alloc_fbus_node_tbl( /* struct bus * */ );
static void read_rom( /* struct bus *, int */ );
static boolean_t is_profile_b( /* struct bus *, int */ );
static boolean_t fbus_match_dev( /* struct fbus_node * */ );
static boolean_t fbus_config_cont( /* caddr_t, caddr_t, struct controller * */ );

/*
 * Monarch duties are handled by the console.  The console will size 
 * the fbus+, and init any module it finds following the rules in the 
 * "System Configuration Software" section of the DEC Futurebus+ handbook.
 * The registers and their values are from IEEE Std 896.2:
 *
 * BUS_PROPOGATION_DELAY, SPLIT_TIMEOUT, BUSY_RETRY_DELAY, 
 * ERROR_RETRY_DELAY, LOGICAL_MODULE_CONTROL, NODE_IDS, 
 * TRANSACTION_TIMEOUT, BUSY_RETRY_COUNTER, ERROR_RETRY_COUNTER
 *
 */
extern int cpu;
int fbus_cobra_err_rtn();
struct cobra_io_csr {
	u_long iocsr;	u_long fill_00[3]; /* I/O Control/Status */
	u_long cerr1;	u_long fill_02[3]; /* Cbus Error Register */
	u_long cerr2;	u_long fill_04[3]; /* Cbus Cmd/Adr latch <63:0> */
	u_long cerr3;	u_long fill_06[3]; /* Cbus Cmd/Adr latch <127:64> */
	u_long lmbpr;	u_long fill_08[3]; /* Lbus Mbox Ptr (bits <31:6>) */
	u_long fmbpr;	u_long fill_0a[3]; /* Fbus Mbox Ptr (bits <31:6>) */
	u_long diagcsr;	u_long fill_0c[3]; /* Diagnostic CSR */
	u_long fivect;	u_long fill_0e[3]; /* FBus Interrupt Vector (FIFO) */
	u_long fhvect;	u_long fill_10[3]; /* FBus Halt Vector (FIFO 0=empty)*/
	u_long ferr1;	u_long fill_12[3]; /* FBus Error Register */
	u_long ferr2;	u_long fill_14[3]; /* FBus Error Address Even/Odd */
	u_long io_lint;	u_long fill_16[3]; /* Local Interrupt Register */

	u_long lerr1;	u_long fill_18[3]; /* LBus Error Register */
	u_long lerr2;	u_long fill_1a[3]; /* LBus Error Address Even/Odd */
};
#define IOCSR_FRL  (1UL << 7)		   /* Assert FBus reset (RE_L) */
#define IOCSR_FRH  (1UL << 39)		   /* Clear Odd FBus intfc logic */
#define IOCSR_FR   (IOCSR_FRL|IOCSR_FRH)   /* Assert FBus reset */
#define IOCSR_FMRL (1UL << 15)		   /* FBus MBX Ptr Reset Low (even) */
#define IOCSR_FMRH (1UL << 47)		   /* FBus MBX Ptr Reset High (odd) */
#define IOCSR_FMR  (IOCSR_FMRL|IOCSR_FMRH) /* FBus MBX Ptr (FMBPR) Reset */
#define CERR1_FME (1UL << 15)	/* FBus MBX Error */

fbusconfl1(connbus, fbus)
	struct bus *connbus, *fbus;
{
	int node;
	
	if(connbus == (struct bus *)0)
		panic("fbusconfl1: no connect bus");
	fbus->bus_type = BUS_FBUS;
	fbus->alive = ALV_ALIVE;
	conn_bus(connbus, fbus);
	printf("%s%d at %s%d\n", fbus->bus_name, fbus->bus_num, 
	       connbus->bus_name, connbus->bus_num);
		/* This would be better in kn430.c, but
		   it is done here to avoid any change to kn430.c at this time. */

	alloc_fbus_node_tbl(fbus);

	/*
	 * Provide support for loadable controllers (and sub-buses) by
	 * connecting the Fbus+ loadable framework extension to the
	 * current Fbus+ structure.
	 */
	if(fbus->framework == NULL)
		fbus->framework = &Fbus_loadable_framework;
	else
		printf("Warning: Fbus+ already has loadable framework extension.\n");

	for(node = 0; node < FBUS_MAX_NODES - 1; node++) {
		struct controller *ctlr;

		if(fbus_probe(fbus, node) == FALSE)
			continue;
		Dprintf(("fbus_probe TRUE for node %d\n",node));

			/* found a device, is it supported? */
		if(fbus_match_opt(fbus, node) == FALSE)
			continue;
		Dprintf(("fbus_match_opt TRUE for node %d\n",node));

		if((ctlr = get_ctlr(NODE_TBL_ENT(fbus, node).devname,
				    node, 
				    fbus->bus_name,
				    fbus->bus_num)) ||
		   (ctlr = get_ctlr(NODE_TBL_ENT(fbus, node).devname,
				    node, 
				    fbus->bus_name,
				    -99)) ||
		   (ctlr = get_ctlr(NODE_TBL_ENT(fbus, node).devname,
				    node,
				    "*",
				    -99)) ||
		   (ctlr = get_ctlr(NODE_TBL_ENT(fbus, node).devname,
				    -1,
				    fbus->bus_name,
				    fbus->bus_num)) ||
		   (ctlr = get_ctlr(NODE_TBL_ENT(fbus, node).devname,
				    -1,
				    fbus->bus_name,
				    -99)) ||
		   (ctlr = get_ctlr(NODE_TBL_ENT(fbus, node).devname,
				    -1,
				    "*",
				    -99)) ) {
			int savebus, saveslot;
			char *savebusname;
			u_int nxv, nxp;
			int (*cfg_cont)();

			if(ctlr->alive & ALV_ALIVE) {
				printf("%s%d at %s%d already alive\n", 
				       ctlr->ctlr_name, ctlr->ctlr_num,
				       fbus->bus_name, fbus->bus_num);
				continue;
			}

			savebus = ctlr->bus_num;
			savebusname = ctlr->bus_name;
			saveslot = ctlr->slot;
			ctlr->bus_name = fbus->bus_name;
			ctlr->bus_num = fbus->bus_num;
			ctlr->slot = node;
				/* For purpose of computing address of node CSRs
				     fbus nodes are accessed as being on the local
				     bus, which is FBUS_LOCAL_BUS (1023). */
			nxp = nxv = (u_int)FBUS_CORE_CSR_BASE(FBUS_LOCAL_BUS, node);

			MBOX_GET(fbus, ctlr);

			/* if this is an adapter */
			if(NODE_TBL_ENT(fbus, node).class == FBUS_ADPT) {
				if(NODE_TBL_ENT(fbus, node).adpt_config) {
					cfg_cont = NODE_TBL_ENT(fbus, node).adpt_config;
				} else {
					/* should this default to cntrlr? */
					printf("fbusconfl1: Warning: adapter %s has no config routine\n",
					       NODE_TBL_ENT(fbus, node).devname);
					continue;
				}
			} else {
				/* 
				 * its a controller. does it have any 
				 * port specific initialization 
				 */
				if(ctlr->port && ctlr->port->conf)
					cfg_cont = ctlr->port->conf;
				else 
					cfg_cont = fbus_config_cont;
			}

			conn_ctlr(fbus, ctlr);
			if(cfg_cont(nxv, nxp, ctlr) == 0) {
				ctlr->bus_name = savebusname;
				ctlr->bus_num = savebus;
				ctlr->slot = saveslot;
				continue;
			} 
		} else if(ctlr = (struct controller *)kalloc(sizeof(struct controller))){
			Dprintf(("fbusconfl1: no controller node %d\n", node));
			ctlr->ctlr_name = NODE_TBL_ENT(fbus, node).devname;
			ctlr->ctlr_num = -99;
			ctlr->bus_name = fbus->bus_name;
			ctlr->bus_num = fbus->bus_num;
			ctlr->slot = node;
			ctlr->alive |= ALV_PRES;
			MBOX_GET(fbus, ctlr);
			conn_ctlr(fbus, ctlr);
		}
		NODE_TBL_ENT(fbus, node).ctlr = ctlr;
#ifdef FBUS_DEBUG
		if(fbusdebug)
			dumpnode_tbl_ent(fbus, node);
#endif /* FBUS_DEBUG */

		/* now look for the controller */
	}
	return(1);
}

/*
 * this routine probes the fbus+ 
 * we start by doing a BADADDR to a required (896.2) register.
 */
boolean_t
fbus_probe(fbus, node)
	struct bus *fbus;
	int node;
{
	int found;
	u_int test_status;
	core_regs_pt core_regs;
	
		/* For purpose of computing address of node CSRs
		     fbus nodes are accessed as being on the local
		     bus, which is FBUS_LOCAL_BUS (1023). */
	core_regs = FBUS_CORE_CSR_BASE(FBUS_LOCAL_BUS, node);
	
	/*
	 * to determine if a module is there, we do a BADADDR
	 * on a required (P896.3) register
	Dprintf(("fbus_probe: BADADDR(0x%x, %d, 0x%l016x)\n",
		 &core_regs->node_ids, sizeof(int), fbus));
	 */
	if(BADADDR(&core_regs->node_ids, sizeof(int), fbus))
		return(FALSE);
	
	/* found a node, now get the test status register */
	test_status = swp_endian((u_int)RDCSR(LONG_32, fbus, 
				&core_regs->test_status), sizeof(test_status));
	Dprintf(("fbus_probe: addr=0x%x test_status 0x%x\n", &core_regs->test_status, test_status));

	/* 
	 * turn off this if-test if you have a fbe with
	 * pre-hw rev b3 which have broken test_state registers 
	 */
	if(((test_status & TEST_STATE) != 0) &&
	   ((test_status & TEST_STATE) != TEST_STATE_IMP)) {
		/* 
		 * do we want to log that a device is here?
		 */
		printf(("fbus_probe: node %d test_status 0x%x not ready\n",
			 node, test_status));
		return(FALSE);
	}
	
	/* only profile 'b' allowed (for now) */
	if( ! is_profile_b(fbus, node))
		return(FALSE);

	read_rom(fbus, node);
	return(TRUE);
}

boolean_t
fbus_config_cont(nxv, nxp, ctlr)
	void *nxv;
	void *nxp;
	struct controller *ctlr;
{
	register struct driver *drp;
	register struct device *device;
	int savectlr;
	char *savectname;
	extern	struct device	device_list[];
	
	drp = ctlr->driver;
	if((*drp->probe)(nxv, ctlr) == 0)
	    return(FALSE);

	ctlr->alive |= ALV_ALIVE;
	ctlr->addr = (char *)nxv;
	/* mbox addrs are already physical */
	if(ctlr->ctlr_mbox == 0) 
		(void)svatophys(ctlr->addr, &ctlr->physaddr);
	else
		ctlr->physaddr = (char *)nxp;
		
	drp->ctlr_list[ctlr->ctlr_num] = ctlr;
	config_fillin(ctlr);
	printf("\n");
	if (drp->cattach)
		(*drp->cattach)(ctlr);

	for (device = device_list; device->dev_name; device++) {
		if (((device->ctlr_num != ctlr->ctlr_num) &&
		     (device->ctlr_num !=-1) && (device->ctlr_num != -99)) ||
		    ((strcmp(device->ctlr_name, ctlr->ctlr_name)) &&
		     (strcmp(device->ctlr_name, "*"))) ||
		    (device->alive & ALV_ALIVE) ||
		    (device->alive & ALV_NOCNFG) ) {
			continue;
		}

		savectlr = device->ctlr_num;
		savectname = device->ctlr_name;
		device->ctlr_num = ctlr->ctlr_num;
		device->ctlr_name = ctlr->ctlr_name;
		
		if ((drp->slave) && (*drp->slave)(device, nxv)) {
			device->alive |= ALV_ALIVE;
			conn_device(ctlr, device);
			drp->dev_list[device->logunit] = device;
			
			printf("%s%d at %s%d", 
			       device->dev_name, device->logunit, 
			       drp->ctlr_name, ctlr->ctlr_num);

			if(device->unit >= 0) {
				/* print bus target lun info for SCSI devs */
				if((strncmp(device->dev_name, "rz", 2) == 0) |
				   (strncmp(device->dev_name, "tz", 2) == 0)) {
					printf(" bus %d target %d lun %d",
					       ((device->unit & 0xFC0) >> 6),
					       ((device->unit & 0x38) >> 3),
					       (device->unit & 0x7) );
				} else 
					printf(" unit %d", device->unit);
			} 

			if (drp->dattach)
				(*drp->dattach)(device);
			printf("\n");
		} else {
			device->ctlr_num = savectlr;
			device->ctlr_name = savectname;
		}
	}
	return(TRUE);
}

static boolean_t
fbus_match_opt(fbus, node)
	struct bus *fbus;
	int node;
{
	int i, found;

	/* search options table for driver */
	/*  Note that sw_vers match can be disabled by setting to 0. */
	for(i = 0; Fbus_option[i].drvname[0] != '\0'; i++) {

		Dprintf(("vend_id opt=%x  node=%x\n",
			Fbus_option[i].module_vend_id,
		    	NODE_TBL_ENT(fbus, node).module_vend_id));
		Dprintf(("hw_vers opt=%x  node=%x\n",
			Fbus_option[i].module_hw_version,
		    	NODE_TBL_ENT(fbus, node).module_hw_version));
		Dprintf(("sw_vers opt=%x  node=%x\n",
			Fbus_option[i].sw_vers,
		    	NODE_TBL_ENT(fbus, node).sw_vers));
		if(   (Fbus_option[i].module_vend_id == 
		       NODE_TBL_ENT(fbus, node).module_vend_id)
		   && (Fbus_option[i].module_hw_version == 
		       NODE_TBL_ENT(fbus, node).module_hw_version)
		   && ( (Fbus_option[i].sw_vers == 0) ||
		        (Fbus_option[i].sw_vers == NODE_TBL_ENT(fbus, node).sw_vers))
		  ) {
				/* match found */
			strcpy(NODE_TBL_ENT(fbus, node).devname, 
			       Fbus_option[i].drvname);
			strcpy(NODE_TBL_ENT(fbus, node).module_name, 
			       Fbus_option[i].module_name);
			if (Fbus_option[i].type == 'A') {
				NODE_TBL_ENT(fbus, node).adpt_config = 
					Fbus_option[i].adpt_config;
				if( Fbus_option[i].adpt_config == 0 )
					printf(
				"Warning: Fbus adapter %s has no config routine\n",
					Fbus_option[i].module_name);
				NODE_TBL_ENT(fbus, node).class = FBUS_ADPT;
			} else
				NODE_TBL_ENT(fbus, node).class = FBUS_CTLR;
			return(TRUE);
		}
	}
	/* Match failed.  Print out that device was not matched.
	   Print device values which were trying to be matched. */
	printf("Node %d (slot %d) not in Fbus_option data table.  Can't configure it.\n", 
		 node, node/2);
	printf("Module_Vendor_Id=%x, Module_Hw_Version=%x, Sw_Version=%x\n",
		NODE_TBL_ENT(fbus, node).module_vend_id,
		NODE_TBL_ENT(fbus, node).module_hw_version,
		NODE_TBL_ENT(fbus, node).sw_vers);
	return(FALSE);
}

/*
 * read the rom entries looking for the information to find the driver
 * for this device.  we'll fill in the fbus_node structure for this device.
 * the module vendor id and module spec id, and one of the module, node or 
 * unit sw version csrs are required csrs and what we use to match
 * with a driver
 */
static void
read_rom(fbus, node)
	struct bus *fbus;
	int node;
{
	binfo_blk_pt binfo_blk;
	int rootdir_len;
	u_int va;

	/* skip the bus info block and go directly to the root dir */
		/* For purpose of computing address of node CSRs
		     fbus nodes are accessed as being on the local
		     bus, which is FBUS_LOCAL_BUS (1023). */
	binfo_blk = (binfo_blk_pt)FBUS_ROM_WIN_BASE(FBUS_LOCAL_BUS, node);
	rootdir_len = swp_endian((u_int)RDCSR(LONG_32, fbus,
			       &binfo_blk->root_dir), sizeof(rootdir_len))>>16;
	Dprintf(("read_rom: rootdir_len %d\n", rootdir_len));
	va = ((u_int)&binfo_blk->root_dir + sizeof(struct dir_info));
	
	read_dir(fbus, node, va, rootdir_len);
}

read_dir(fbus, node, va, dirlen)
	struct bus *fbus;
	int node;
	u_int va;
	u_int dirlen;
{
	u_int len;

	Dprintf(("read_dir: fbus 0x%x va 0x%x dirlen %d\n", fbus, va, dirlen));
	for(len = 0; len < dirlen; len++) {
		u_char key;
		u_int value;

		value = swp_endian((u_int)RDCSR(LONG_32, fbus, va + (len<<2)), 
				   sizeof(value));
		Dprintf(("read_dir: value 0x%x\n", value));
		key = MAKE_KEY(value);
		value &= ENTRY_VALUE_BITS;
		if(KEY_TYPE(key) == IMMEDIATE_KEY) {
			switch(KEY_VALUE(key)) {
				/* required */
			      case MOD_VEND_ID:
				NODE_TBL_ENT(fbus, node).module_vend_id = value;
				break;
			      case MOD_HW_VERS:
				NODE_TBL_ENT(fbus, node).module_hw_version = value;
				break;
			      case MOD_SW_VERS:
			      case NODE_SW_VERS:
			      case UNIT_SW_VERS:
				/* 
				 * spec is unclear what todo if > 1 of 
				 * these exist
				 */
				NODE_TBL_ENT(fbus, node).sw_vers = value;
				break;
			}
		} else if(KEY_TYPE(key) == DIRECTORY_KEY) {
			/* read the len of the sub dir */
			u_int subdir_va = va + (value << 2);
			len = swp_endian((u_int)RDCSR(LONG_32, fbus, 
							subdir_va), 
					   sizeof(value)) >> 16;
			Dprintf(("read_dir: subdir va 0x%x len 0x%x\n", 
				 subdir_va, len));
			subdir_va += sizeof(struct dir_info);
			Dprintf(("read_dir: subdir va 0x%x len 0x%x\n", 
				 subdir_va, len));
			/* orig: read_dir(fbus, subdir_va, len);	*/
			read_dir(fbus, node, subdir_va, len); /* stuarth changed */
		} /* skip leafs (vendor dependent info) and csr offsets */
	}
}

/* 
 * verify that this is a profile 'b' module 
 * we don't care about endianism here 'cuz 
 * only we're looking for at least one byte == '0x42'
 */
boolean_t
is_profile_b(fbus, node)
	struct bus *fbus;
	int node;
{
	binfo_blk_pt binfo_blk;
	int prof_id, i, j;
#define B   ((int)0x00000042)
#define F   ((int)0x000000FF)

		/* For purpose of computing address of node CSRs
		     fbus nodes are accessed as being on the local
		     bus, which is FBUS_LOCAL_BUS (1023). */
	binfo_blk = (binfo_blk_pt)FBUS_ROM_WIN_BASE(FBUS_LOCAL_BUS, node);
	NODE_TBL_ENT(fbus, node).profiles[0] = (u_int)RDCSR(LONG_32, fbus, &binfo_blk->profile_id[0]);
	NODE_TBL_ENT(fbus, node).profiles[1] = (u_int)RDCSR(LONG_32, fbus, &binfo_blk->profile_id[1]);

	for(i = 0; i < 2; i++) {
		prof_id = NODE_TBL_ENT(fbus, node).profiles[i];
		for(j = 0; j < 4; j++) {
			if((prof_id & F) == B)
				return(TRUE);
			prof_id >>= 8;
		}
	}
	return(FALSE);
#undef B
#undef F
}

static void
alloc_fbus_node_tbl(fbus) 
	struct bus *fbus;
{
	if(fbus->node_tbl != 0)
		panic("node_tbl already allocated");

	fbus->node_tbl = (char *)kalloc(sizeof(struct fbus_node) * FBUS_MAX_NODES);
	if(fbus->node_tbl == 0)
		panic("cannot allocate fbus node table");

	bzero(fbus->node_tbl, sizeof(struct fbus_node) * FBUS_MAX_NODES);
}

fbusconfl2(connbus, fbus)
	struct bus *connbus, *fbus;
{
	return(1);
}

/* TODO */
fbuserror(fbus)
	struct bus *fbus;
{
	printf("fbuserror\n");
}


/*
 * This routine builds the fbus+ specific command field 
 * in the software mailbox.
 */
void
fbus_mbox_cmd(mbp, rwflag, mask_as)
	struct mbox *mbp;          /* mailbox pointer */
	u_int rwflag;              /* rd/wrt flag */
	u_int mask_as;            /* data type, write mask and addr space */

{
	u_int cmd = 0;

	/*
	 * fbus mailbox command field 
	 * the fbus command is taken from the cobra spec
	 * these commands should be the same for all fbus implementations
	 * 
	 * bit
	 * <31:8>  7   6     5    4   <3:0>
	 * ---------------------------------
	 * | nu | aw | nu | dw | wr | trans |
	 * ---------------------------------
	 *
	 * nu    : not used. ignored on writes, read as zero
	 * aw    : address width. 0 = 32bit, 1 = 64bit
	 * dw    : data width. 0 = 32bit, 1 = 64bit
	 * wr    : write/read. 0 = read, 1 = write
	 * trans : 0 = unmasked, 2 = partial - byte mask is valid
	 */
#define FCMD_AW_64    0x80
#define FCMD_DW_64    0x20
#define FCMD_WR_W     0x10
#define FCMD_PARTIAL  0x02

	/* 
	 * check for A64 addreess width
	 * (shouldn't see this currently)
	 */
#ifdef notyet
	if(mask_as & A64)
		cmd = FCMD_AW_64;
	else
		/* force A32 only for now */
#endif /* notyet */
		(u_int)mbp->rbadr &= (u_int)0xffffffff;
	/* is this 64 bit datum? */
	if(mask_as & D64)
		cmd |= FCMD_DW_64;
	if(rwflag & WRT_CSR) 
		cmd |= FCMD_WR_W;
	if(mask_as & PARTIAL_DATUM)
		cmd |= FCMD_PARTIAL;

	mbp->cmd = cmd;

#ifdef FBUS_MBOX_DEBUG
	dumpmbox(mbp);
#endif /* FBUS_MBOX_DEBUG */
}

swp_endian(n, size)
	u_int n;
	u_int size;  /* in bytes */
{
	u_short s;
	u_int i;

	if(size == sizeof(short)) {
		s = (n & 0xff00) >> 8;
		s |= (n & 0x00ff) << 8;
		return((u_short)s);
	} else if(size == sizeof(int)) {
		i = (n & 0xff000000) >> 24;
		i |= (n & 0x00ff0000) >> 8;
		i |= (n & 0x0000ff00) << 8;
		i |= (n & 0x000000ff) << 24;
		return((u_int)i);
	} else /* nothing to do with chars, longs not supported (yet) */
		return;
}

#ifdef FBUS_DEBUG
static void
dumpnode_tbl_ent(fbus, node)
	struct bus *fbus;
	int node;
{
	if(fbus->node_tbl == 0) {
		printf("dump node table: no table\n");
		return;
	}

	Dprintf(("dumpnode_tbl_ent: fbus 0x%x node %d\n", 
		 fbus, node));
	Dprintf(("\tprofile_id[0] 0x%x profile_id[1] 0x%x\n",
		 NODE_TBL_ENT(fbus, node).profiles[0],
		 NODE_TBL_ENT(fbus, node).profiles[1]));
	Dprintf(("\tmodule_vend_id 0x%x module_hw_version 0x%x sw_vers 0x%x\n",
		NODE_TBL_ENT(fbus, node).module_vend_id, 
		 NODE_TBL_ENT(fbus, node).module_hw_version, 
		 NODE_TBL_ENT(fbus, node).sw_vers));
	Dprintf(("\tdevname %s class %c *ctlr 0x%x\n",
		 NODE_TBL_ENT(fbus, node).devname, NODE_TBL_ENT(fbus, node).class,
		 NODE_TBL_ENT(fbus, node).ctlr));
}
#endif /* FBUS_DEBUG */

      /*
	dump cerr1, ferr1, ferr2, mbox status.
	Then,
		reset only the fmbpr
		  write 8e00 00008e00 to iocsr
		  write 0e00 00000e00 to iocsr
		try to read known fbus addr (e.g. fffc0000)
		if read times out then
		  (bus is hung and must be reset)
		  write e80 00000e80 to iocsr
		  wait between 4 and 12 msec [we will wait 12ms]
		  write e00 00000e00 to iocsr
		  dump error regs from fbus devices to err log
		else [read did not time out]
		  (bus is not hung; err regs may
		   be interrogated without generating reset)
		  clear NXM status
		    write 8e00 00008e00 to iocsr
		    write 0e00 00000e00 to iocsr
		    write 8000 to CERR1
		  dump error regs from fbus devices to errlog
       */

int fbus_cobra_err_rtn(mbp, rtn, err)
register mbox_t mbp;
register char *rtn;
register int err;
{
	int stat, timeout;
	struct controller *ctlr;
	u_int error_hi, error_lo;
	u_int retries = 0;
	struct cobra_io_csr *Io_regs;

	Io_regs = (struct cobra_io_csr *)PHYS_TO_KSEG(0x210000000L);
		/* Dump cerr1, ferr1, ferr2, mbox status.  */
	printf("CERR=%lx FERR1=%lx FERR2=%lx mbox_status=%lx\n",
	    Io_regs->cerr1, Io_regs->ferr1, Io_regs->ferr2, mbp->mb_status);
	mbp->mb_status = 1L; /* clear mbox error status; set simply to DONE  */
	Io_regs->iocsr |= IOCSR_FMR;	/* reset the fmbpr only */
	mb();
	Io_regs->iocsr &= ~IOCSR_FMR;
	Io_regs->cerr1 &= CERR1_FME;	/* clear fbus err bit in cerr1 */
	mb();              

#ifdef notyet /* NEEDS MORE TESTING */
		/* Try to read known fbus addr (e.g. fffc0000) */
		/* Or, get known device fbus addr. */
	stat = mbox_setup(RD_CSR, LONG_32, mbp, 0xfffc0000, 0);
	if(stat != MBOX_SUCCESS) {
		/* Unable to even setup mbox */
		/* no further error recovery is attempted */
		panic("fbus_cobra_err_rtn mbox_setup failure");
	}
	timeout = 0;
	while((mbp->mb_status & MBOX_DON_BIT) == 0) {
		if(retries >= (mbp)->bus_timeout) {
			timeout = 1;
			break;
		}
		DELAY(1);
		retries++;
	}
	mb();
	if(timeout){
		printf("mbox hung - resetting fbus\n");
		Io_regs->iocsr |= IOCSR_FR; /* bus is hung and must be reset */
		mb();
		DELAY(12000);	/* wait 12 msec */
		Io_regs->iocsr &= ~IOCSR_FR;
		Io_regs->cerr1 &= CERR1_FME;	/* clear fbus err bit in cerr1 */
		mb();
	} else {
		printf("mbox not hung - clearing fmbpr\n");
	  					/* clear NXM status */
		Io_regs->iocsr |= IOCSR_FMR;	/* reset fbus mbox ptr reg */
		mb();
		Io_regs->iocsr &= ~IOCSR_FMR;
		mb();              
		Io_regs->cerr1 &= CERR1_FME;	/* clear fbus err bit in cerr1 */
	}
	printf("Dumping fbus device error registers...\n");
		/* dump error regs from fbus devices to errlog */
	for (ctlr = controller_list; ctlr->driver != 0; ctlr++) {
		if(!(ctlr->alive&ALV_ALIVE) || strcmp(ctlr->bus_name,"fbus"))
				/* no need to check bus_num since cobra has one fbus */
			continue;
		printf("%s%d: \n", ctlr->ctlr_name, ctlr->ctlr_num);
		printf("args= %x %x %x\n",
			LONG_32,
			mbp->bus_ctlr_ptr,
			&((core_regs_pt)(ctlr->addr))->err_log_buff[0]);
		error_hi = swp_endian((u_int)
			  RDCSR(LONG_32, mbp->bus_ctlr_ptr, &((core_regs_pt)(ctlr->addr))->err_log_buff[0]),
			  sizeof(u_int));
		error_lo = swp_endian((u_int)
			  RDCSR(LONG_32, mbp->bus_ctlr_ptr, &((core_regs_pt)(ctlr->addr))->err_log_buff[1]),
			  sizeof(u_int));
		printf("ERROR_HI=%x  ERROR_LO=%x\n", error_hi, error_lo);
	}
	printf("Done dumping registers\n");
#endif
	panic("fbus fatal error");
}

