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
static char *rcsid = "@(#)$RCSfile: lsbinit.c,v $ $Revision: 1.2.13.4 $ (DEC) $Date: 1993/09/24 15:55:13 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)lsbinit.c	9.5	(ULTRIX/OSF)	11/21/91";
#endif 

#include <sys/types.h>
#include <mach/mach_types.h>		/* added to get task_t */
#include <io/dec/lsb/lsbreg.h>
#include <io/common/devdriver.h>
#include <machine/machparam.h>		/* has NBPG defined...		*/
#include <mach/kern_return.h>		/* has KERN_SUCCESS defined	*/
#include <hal/cpuconf.h>		/* has cpusw definition		*/
#include <sys/presto.h>
#include <mach/vm_prot.h>		/* has VM_PROT_x defined...	*/
#include <machine/pmap.h>
#include <machine/rpb.h>

/* #define PRESTO_DEBUG */
#define CFLUSH_WORKS		/* this also needs to be set in kn7aa.c	*/
/* #define EEPROM_DEBUG */

extern int nNXMI;
extern int vecbi;
extern int cpu;			/* Ultrix internal System type */

extern struct cpusw *cpup;	/* pointer to cpusw entry */
extern struct config_adpt  *ni_port_adpt;

int	numlsb;

/*
 * Initialize nvram parameters for presto config code
*/
int	lsb_nvram_present = 0 ;
int	lsb_nvram_size = 0;
int	lsb_eeprom_inuse = 0;	/* flag used to sanity check eeprom access */
caddr_t	lsb_nvram_start_adr ;
volatile struct lsb_reg *lsb_nvram_nxv ;


 /* The lsb_crd_chk_mem structure was created for use by */
 /* lsb_chk_crd_thread to find each memory that must be polled for CRD */
 /* errors. 'lsbconf_conf' initializes and fills this array, as it */
 /* sizes the LSB, with the address of each memory module's LDEV */
 /* register. lsb_crd_count is an array of counters to keep track of */
 /* CRDs for memory modules in particular LSB node ids. */

struct lsb_lma_reg *lsb_crd_chk_mem[MAX_LSB_NODE];
long   lsb_crd_count[MAX_LSB_NODE];

 /* define the interval at which memory is polled for CRD errors */
#define CRD_TIMEOUT 1L*60	/* 1 minutes as per PFMS */
/* #define CRD_TIMEOUT 1*15L	/* 60 seconds, for testing */

/* #define PRM_DEBUG 1 */

 /* define the scaling factor. (1*hz) = (1 second) */
extern int hz;                  /* used to setup times. ticks/sec */

 /* for use in creating memory CRD polling thread as a part of the */
 /* original system task */
extern task_t first_task;	

 /* define for forward reference */
extern int lsb_crd_chk_thread();
extern int lsb_crd_create_thread();

pwr_sys_timeout();
pwr_sys_thread();




/*
 * Null confl2 routine. I don't think this is needed anywhere...
 */
lsbconfl2()
{
	return(1);
}

/*
* This routine initializes the SCB vector for the bus error interrupt vector
*/

lsbsetvec(lsbbus,param)
struct bus *lsbbus ;
long param ;
{
	int i;
	struct lsbdata *lsbdata;

	/* get the data structure that defines this lsb			*/
	lsbdata = get_lsb(lsbbus->bus_num);

}

/*
 * Based on a passed node number, get_lsb, just like the old get_xmi, will
 * return a pointer to the structure lsbdata that has information such as
 * node number, virtual/physical address of the base address, etc.
*/
struct lsbdata *get_lsb(lsbnum)
int lsbnum;
{
	register struct lsbdata *lsbdata ; 
	extern struct lsbdata *head_lsbdata;

	lsbdata = head_lsbdata;
	while(lsbdata) {
		if(lsbdata->lsbnum == lsbnum) {
			return(lsbdata);
		}
		lsbdata = lsbdata->next;
	}
	panic("no bus data");
	/*NOTREACHED*/
}

extern struct lsb_reg lsb_start[];
extern struct lsbsw lsbsw[9];

	/* 
	lsbconf1 structured after xmiconf - it has three basic routines.
	A reset, a wait and a conf. This makes it easy to reset all the
	modules at once and let them run self test in parallell...
 	*/
lsbconfl1(dum1,dum2,lsbbus)
struct bus *lsbbus;
struct bus *dum1;
struct bus *dum2;
{
	int s;

        lsbbus->bus_type = BUS_LSB;
        lsbbus->alive = 1;	/* ALV_ALIVE or ALU_ALIVE not found	*/

        lsbconf_reset(lsbbus);		/* Reset all nodes on the lsb	*/
        lsbconf_wait(lsbbus);		/* Wait for all init's to complete */
        lsbconf_conf(lsbbus);		/* Configure each node		*/
}

lsbconf_reset(lsbbus)
struct bus *lsbbus;
{
	int lsbnum = lsbbus->bus_num;
	volatile struct lsb_reg	*nxv, *nxp; /* virtual pointer to LSB node */
	register int lsbnode;
	register struct lsbsw *plsbsw;
	register struct lsbdata *lsbdata;
	volatile struct lsb_reg *cpunode=0;
	register int i, found;
	extern int stray();
	extern struct cpusw *cpup;

	lsbdata = get_lsb(lsbnum);
	nxp = lsbdata->lsbphys;

/*
 * map the LSB node space here
 */

	lsbdata->lsbnodes_alive =0;

	for(lsbnode = 0; lsbnode < MAX_LSB_NODE; lsbnode++) {
	    
	    /* Check to see if the lsb node responds to read */
	    nxv = lsbdata->lsbvirt[lsbnode];
	    if (BADADDR((int *) nxv,sizeof(int), lsbbus)) continue;
/*
 * There is a module in this slot, see what to do with it
*/
	    found = FALSE;
	    for (plsbsw = lsbsw ; plsbsw->lsb_ldev ; plsbsw++) {	
		if (plsbsw->lsb_ldev == (nxv->lsb_ldev & LSBLDEV_TYPE)) {
		    found = TRUE;
	    	    lsbdata->lsberr[lsbnode].plsbsw= plsbsw;
		    lsbdata->lsbnodes_alive |= (1 << lsbnode);
                    if (plsbsw->lsb_flags&LSBF_SST) /* Do a reset on the module if lsbsw says to */
			nxv->lsb_lcnr = nxv->lsb_lcnr | LSB_NRST;
                    break;
		}
	    }
	    if (!found)
		printf ("lsb %d node %d, unsupported device type 0x%x\n",
			 lsbnum,lsbnode, (unsigned short) nxv->lsb_ldev);

	}
	DELAY(10000);	/* need to give time for LSB bad line to be set */
}
	/* Wait for reset of lsb nodes to take effect. */

lsbconf_wait(lsbbus)
struct bus *lsbbus;
{
	int lsbnum = lsbbus->bus_num;
	volatile struct lsb_reg	*nxv, *xnp; /* virtual-phys pnter to LSB node*/
	register int lsbnode;
	register struct lsbsw *plsbsw;
	register struct lsbdata *lsbdata;
	volatile struct lsb_reg *cpunode=0;
	register int i;
	int broke;
	int alive;
	int totaldelay;

/* 
 * Wait up to a cumulative 20 seconds.
 * For extra safety, this is double the spec value.
*/
	totaldelay = 2000;
	lsbdata = get_lsb(lsbnum);

	for(lsbnode = 0; lsbnode < MAX_LSB_NODE; lsbnode++) {

	    /* Initialize each possible LSB node's entry in */
	    /* lsb_crd_chk_mem array to zero. */ 
	    lsb_crd_chk_mem[lsbnode] = 0;
	    lsb_crd_count[lsbnode] = 0;

	    if( !(lsbdata->lsbnodes_alive & (1 << lsbnode)))
		continue;
	    plsbsw = lsbdata->lsberr[lsbnode].plsbsw;
/*
 * wait here for up to remaining count time or until device is reset.
*/		   
            if (plsbsw->lsb_flags&LSBF_SST) {
	    	cpunode = lsbdata->cpu_lsb_addr;
	        nxv = lsbdata->lsbvirt[lsbnode];
		while((cpunode->lsb_lber & LSB_E) && (totaldelay-- > 0))
			DELAY(10000);
		while((nxv->lsb_lcnr&LSB_STF) && (totaldelay-- > 0))
			DELAY(10000);
		nxv->lsb_lcnr = nxv->lsb_lcnr & ~( LSB_NRST);
	    }
	}

}

	/* Do config of nodes. */

lsbconf_conf(lsbbus)
struct bus *lsbbus;
{
	int lsbnum=lsbbus->bus_num;
	volatile struct lsb_reg	*nxv, *nxp; /* virt/physl pnter to LSB node */
	register int lsbnode;
	register struct lsbsw *plsbsw;
	register struct lsbdata *lsbdata;
	volatile struct lsb_reg *cpunode=0;
	extern int npresto_configured ;
	struct bus *iop;
	register int i;
	int broke;
	int alive;
	int totaldelay;

	lsbdata = get_lsb(lsbnum);

	/* Register this routine to be called from init_main after threads have been setup */
	kd_thread_register(lsb_crd_create_thread,0);

	/* Setup timeout to start the Power System Monitor thread */
	/* timeout(pwr_sys_timeout, 0, hz); */

	/* do config of devices, adapters, controllers, etc.	*/
	for(lsbnode = 0; lsbnode < MAX_LSB_NODE; lsbnode++) {
	    if( !(lsbdata->lsbnodes_alive & (1 << lsbnode)))
		continue;
	    plsbsw = lsbdata->lsberr[lsbnode].plsbsw;
	    nxp = (struct lsb_reg *) cpup->nexaddr(lsbnode);
	    nxv = lsbdata->lsbvirt[lsbnode];
	    broke = (nxv->lsb_lcnr & LSB_STF) ;	/* broke = 0 means module OK */
						/* except for IOP!	     */
	    alive = 0;			/* alive = 1 means 'config' complete */
	    switch(nxv->lsb_ldev & LSBLDEV_TYPE) {
		  case LSB_IOP_LDEV:
		    if(!broke) {
			printf("lsbconf_conf: IOP node %d selftest failed\n", lsbnode);
			break;
		    }
		    if((iop = get_bus("iop", 0, lsbbus->bus_name, lsbbus->bus_num)) ||
		       (iop = get_bus("iop", 0, lsbbus->bus_name, -1)) ||
		       (iop = get_bus("iop", -1, lsbbus->bus_name, lsbbus->bus_num)) ||
		       (iop = get_bus("iop", -1, lsbbus->bus_name, -1))) {
			    
			    iop->connect_bus = lsbbus->bus_name;
			    iop->connect_num = lsbbus->bus_num;
			    if(!(*iop->confl1)(lsbbus, iop, lsbnode))
				    panic("lsbconf_conf: IOP confl1");
			    if(!(*iop->confl2)(lsbbus, iop, lsbnode))
				    panic("lsbconf_conf: IOP confl2");
		    } else 
			    panic("lsbconf_conf: cannot find IOP bus");
		    break;
		  case LSB_BBMEM:
		    if(broke) {
			printf("lsbconf_conf: ms7bb node %d selftest failed.\n", lsbnode);
			break;
		    }
			 lsb_nvram_config(nxv,lsbnode);
			 break;

		  case LSB_MEM:
		    if(broke) {
			printf("lsbconf_conf: ms7aa node %d selftest failed.\n", lsbnode);
			break;
		    }

		    printf("ms7aa at node %d\n", lsbnode);
		    /* Set the associated lsb_crd_chk_mem		*/
		    /* array entry to the address of the node's LDEV	*/
		    lsb_crd_chk_mem[lsbnode] = (struct lsb_lma_reg *)nxv;
		    break;

		  case LSB_LEP:
		    if(broke) {
			printf("lsbconf_conf: kn7aa node %d selftest failed.\n", lsbnode);
			break;
		    }
		    printf("kn7aa at node %d\n", lsbnode);
		    break;
		  default:
		    printf("lsbconf_conf: unknown device: 0x%x\n", nxv->lsb_ldev);
		    break;
		  }
	}
	/*
	*  The following loop cleans up any errors that might
	*  have gotten set when we probed the LSB
	*/
	for(lsbnode = 0; lsbnode < MAX_LSB_NODE; lsbnode++) {
		if (lsbdata->lsbnodes_alive & (1<<lsbnode)) {
/* should  call a CPU sw here to clear errors. The LEP needs its LMERR
reg cleared... */
			nxv = lsbdata->lsbvirt[lsbnode];
			nxv->lsb_lber = nxv->lsb_lber & ~LSB_E;
		}	

	}
}


/*
 * hardware dependent NVRAM status/config/etc.
*/

lsb_nvram_config(nxv,lsbnode)
	volatile struct lsb_reg	*nxv ;
	register int lsbnode;
{
	struct rpb *rpb ;
	struct rpb_mdt *memdsc ;
	struct rpb_cluster *clusterdsc ;
	u_long lsb_presto_phystart ;
	caddr_t nvram_bitmap ;
	int i ;
	extern int lsb_nvram_size ;
	extern vm_offset_t hwrpb_addr;
	extern caddr_t lsb_nvram_start_adr ;
	extern volatile struct lsb_reg *lsb_nvram_nxv ;

	printf("ms7bb at node %d\n", lsbnode);
	lsb_nvram_nxv = nxv ;
	nvram_batteries0.nv_nbatteries = 1;	   /* one battery	*/
	nvram_batteries0.nv_minimum_ok = 1;	   /* it must be good	*/
	nvram_batteries0.nv_primary_mandatory = 1; /* primary must be OK */
	nvram_batteries0.nv_test_retries = 1;	   /* One test will do it */

	/*
	 * Find the physical address of the NVram and assign
	 * its corresponding KSEG address
	*/
	rpb = (struct rpb *)hwrpb_addr;
	memdsc = (struct rpb_mdt *)((char *)rpb + rpb->rpb_mdt_off);
#ifdef PRESTO_DEBUG
printf("lsb_nvram_config: rpb = %l016x, memdsc=%l016x, memdsc->rpb_numcl=%l016x\n",memdsc, memdsc->rpb_numcl);
#endif /* PRESTO_DEBUG */

/*
 * Find the memory descriptor that defines the location, size, and selftest
 * results for the NVRAM
*/
	for (i = 0; i < (int)memdsc->rpb_numcl; i++) {
	  clusterdsc = (struct rpb_cluster *)((char *)(memdsc->rpb_cluster) +
		(i * sizeof(struct rpb_cluster))) ;
#ifdef PRESTO_DEBUG
printf("lsb_nvram_config: clusterdsc->rpb_usage = %l016x\n",clusterdsc->rpb_usage);
#endif /* PRESTO_DEBUG */
	  if (clusterdsc->rpb_usage == CLUSTER_USAGE_NVRAM) {
		lsb_presto_phystart=(clusterdsc->rpb_pfn * rpb->rpb_pagesize);
		break;
	  }
	}
	if ( i == memdsc->rpb_numcl) {
	  printf("lsb_nvram_config: No NVRAM memory descriptor present.\n");
	  return(0);
	}
	/*
	 * Check how much of the NVRAM  is good 
	*/
	if (clusterdsc->rpb_pa == 0L) {
		printf("lsb_nvram_config: NVRAM selftest did not run, presto not configured.\n");
		return(0);
		}
	nvram_bitmap = (caddr_t)PHYS_TO_KSEG(clusterdsc->rpb_pa);
	for (i=0 ; (char)*nvram_bitmap == (char)0xff; nvram_bitmap++, i++){
	}
	if ((i*8*rpb->rpb_pagesize) == MS700_NVRAM_SIZE) {
	  lsb_nvram_size = (i*8*rpb->rpb_pagesize) ;
	}
	else {
	  printf("lsb_nvram_config: %d NVRAM tested good, Maximum supported size = %d\n",
	  (i*8*rpb->rpb_pagesize), MS700_NVRAM_SIZE);
	  lsb_nvram_size = ((i*8*rpb->rpb_pagesize) < MS700_NVRAM_SIZE)
			? (i*8*rpb->rpb_pagesize) : MS700_NVRAM_SIZE ;
	}
#ifndef CFLUSH_WORKS
	/* If using the 'by hand eviction' method, there needs to be
	 * at least twice the cache size of good presto memory.
	 * If not, just bail out...
	*/
	if(lsb_nvram_size <= (2*RUBY_CACHE_SIZE)){
		printf("lsb_nvram_config: Not enough NVRAM tested good. Just returning!\n");
		return(0);
	}
#endif /* CFLUSH_WORKS */

	lsb_nvram_start_adr = (caddr_t)PHYS_TO_KSEG(lsb_presto_phystart);
#ifdef PRESTO_DEBUG
printf("lsb_nvram_config: lsb_presto_phystart = %l016x, lsb_nvram_start_adr = %l016x \n", lsb_presto_phystart, lsb_nvram_start_adr);
#endif /* PRESTO_DEBUG */
	lsb_nvram_present = 1 ;
	return(0);
      }

/*
 * Provide presto with status of diagnostics run on nvram
 * hence, let presto know what to do - recover, etc...
*/
int lsb_nvram_status()
{
/*
 *  We use the bit map to figure out if and how much of the memory is good
*/
	return(NVRAM_RDWR);
}

/*
 * lsb_nvram_battery_status - update the global battery information
 * structure for Prestoserve
 *
 * As of Jan 93 the BBM status register is defined as: (note asserted means
 * ZERO!!!)
 * 
 * Bit
 * 0 = RFU
 * 1 = BATTERY OK ; when asserted, indicate both battery strings are OK
 * 2 = CHARGING ; When asserted, inidats either or both battery strings
 * 	are recharging
 * 3 = VBB RESET ; When asserted, indicates that modules was not in battery
 * 	backup mode previous to latest powerup. Also indicates memory
 * 	contents are undefined
 * 4 = INIT ; For the first 10 mins after a power cycle, this bit is
 * 	asserted, indicateing that the batteries doing their "selftest"
 * 	When asserted, IGNORE BATTERY OK and CHARGING!
 * 5 = BBM ; Always asserted
 *
 * Apr 93 defined bit 6
 * 6 = SWITCH ON ; When asserted, means the batteries are connected.
 *	i.e. the manually operated battery switch is on.
 * 7 = RFU
*/
int lsb_nvram_battery_status()
{
	extern volatile struct lsb_reg *lsb_nvram_nxv ;
	int status ;
	status = lsb_read_eeprom(lsb_nvram_nxv, LSB_EEPROM_PRESTO) ;
#ifdef PRESTO_DEBUG
printf("lsb_nvram_battery_status: - returned status = %x\n", status);
#endif /* PRESTO_DEBUG */

	if((status & LSB_PRESTO_BAT_CONN)) {
	/*
	 * The batteries are not connected (the manual switch is off).
	*/
		nvram_batteries0.nv_status[0] = BATT_NONE ;
		return(0);
	}

	if(!(status & LSB_PRESTO_INIT)) {
	/*
	 * The batteries are still charging. We don't want to presto to
	 * be used yet, so we need to report ENABLED status... I understand
	 * this to mean that presto will not try and enable excelleration,
	 * but will look at the cookie and will recover data if the cookie 
	 * indicates to do so.
	*/
		nvram_batteries0.nv_status[0] = BATT_ENABLED  | BATT_SELFTEST ;
		return(0);
	}

	if(!(status & LSB_PRESTO_CHARGING)) {
	/* Batteries are still charging, cann't assess if they are OK or not.*/
		nvram_batteries0.nv_status[0] = BATT_ENABLED | BATT_CHARGING ;
		return(0);
	}

	if(!(status & LSB_PRESTO_BAT_OK))
		nvram_batteries0.nv_status[0] = BATT_OK ;
		else {
			/* The batteries are really broken	*/
			nvram_batteries0.nv_status[0] = BATT_NONE;
		}
	return(0);
}

/*
 * Performs disable battery kill function.
 * There is no way to turn on/off rubys NVRAM batteries in software.
*/
int lsb_nvram_battery_enable()
{
}

/*
 * Performs enable battery kill function.
 * There is no way to turn off/on rubys NVRAM batteries in software.
*/
int
lsb_nvram_battery_disable()
{
 }
/*
 * These routines read/write the LSB architected EEPROM's on LSB modules.
 * The EEPROM is 8 bits wide and is accessed serially, the clocking
 * of address and data is done via software.
*/

int
lsb_read_eeprom(nxv,adr)
volatile struct lsb_reg	*nxv;
int adr;
{
	extern int lsb_eeprom_inuse ;
	int i ;
	u_int data ;

	if (lsb_eeprom_inuse) {
		printf("lsb_read_eeprom: Conflicting access to ibr %x",
							nxv->lsb_eepr_cdr);
		return(0);
	}
#ifdef EEPROM_DEBUG
printf("lsb_read_eeprom: nxv = %l016x , adr = %x\n", nxv, adr);
#endif /* EEPROM_DEBUG */
	lsb_eeprom_inuse = 1;
	lsb_eeprom_stop(nxv);
	lsb_eeprom_start(nxv);

	/*
	 * Wiggle out the slave address, with a WRITE
	*/ 
	lsb_eeprom_slave(nxv,adr,LSB_EEPROM_WRITE);

	/*
	 * Wiggle out the word address
	*/ 
#ifdef EEPROM_DEBUG
printf("lsb_read_eeprom: word address = ");
#endif /* EEPROM_DEBUG */
	for (i=7 ; i >= 0 ; i--) {

#ifdef EEPROM_DEBUG
printf(" %x", (((adr & 0x0ff)  >> i) & 1) );
#endif /* EEPROM_DEBUG */
	  lsb_eeprom_wbit(nxv, (((adr & 0x0ff)  >> i) & 1));
	}
#ifdef EEPROM_DEBUG
printf(" \n");
#endif /* EEPROM_DEBUG */
	lsb_wait_ack(nxv);
	lsb_eeprom_start(nxv);
	/*
	 * Wiggle out the slave address
	*/ 
	lsb_eeprom_slave(nxv,adr,LSB_EEPROM_READ);	
	/*
	 * Read the data out
	*/
	nxv->lsb_eepr_cdr = LSB_XMT_SDAT ;
	mb();
	DELAY(LSB_EEPROM_SBITWAIT);		/* console had 1 */
#ifdef EEPROM_DEBUG
printf("lsb_read_eeprom: clocked data = ");
#endif /* EEPROM_DEBUG */	
	data = 0 ;
	for (i=7 ; i >= 0 ; i--) {
		nxv->lsb_eepr_cdr = LSB_SCLK | LSB_XMT_SDAT ;
		mb();
		DELAY(LSB_EEPROM_BITWAIT);	/* console has 5 */
		data = data | ((nxv->lsb_eepr_cdr & 1) << i ) ;
#ifdef EEPROM_DEBUG
printf(" %x", (nxv->lsb_eepr_cdr & 1));
#endif /* EEPROM_DEBUG */	
		nxv->lsb_eepr_cdr = LSB_XMT_SDAT ;
		mb();
		DELAY(LSB_EEPROM_BITWAIT);	/* console has 5 */
	}
#ifdef EEPROM_DEBUG
printf("\n");
#endif /* EEPROM_DEBUG */	

	/* Send a NACK to the EEPROM				*/
	nxv->lsb_eepr_cdr = LSB_SCLK | LSB_XMT_SDAT ;
	mb();
	DELAY(LSB_EEPROM_BITWAIT);		/* console has 5 */
	nxv->lsb_eepr_cdr = LSB_XMT_SDAT ;	
	mb();
	DELAY(LSB_EEPROM_BITWAIT);		/* console has 5 */

	lsb_eeprom_stop(nxv);
	lsb_eeprom_inuse = 0;
#ifdef EEPROM_DEBUG
printf("lsb_read_eeprom: data = %x\n", data);
#endif /* EEPROM_DEBUG */	
	return(data);
}

lsb_eeprom_slave(nxv,adr,action)
volatile struct lsb_reg	*nxv;
int adr, action ;
{
	int i, status;

	lsb_eeprom_wbit(nxv,1);
	lsb_eeprom_wbit(nxv,0);
	lsb_eeprom_wbit(nxv,1);
	lsb_eeprom_wbit(nxv,0);
#ifdef EEPROM_DEBUG
printf("lsb_eeprom_slave: address = ");
#endif /* EEPROM_DEBUG */	
	for (i=10 ; i >= 8 ; i--) {
#ifdef EEPROM_DEBUG
		printf(" %x", (((adr & 0x0700) | LSB_XMT_SDAT) >> i) & 0x01);
#endif /* EEPROM_DEBUG */	

		lsb_eeprom_wbit(nxv,
			((((adr & 0x0700) | LSB_EEPROM_SREAD) >> i) & 0x01));
	}
#ifdef EEPROM_DEBUG
printf(" \n");
#endif /* EEPROM_DEBUG */	

	lsb_eeprom_wbit(nxv,action);
	status = lsb_wait_ack(nxv) ;
	if (status != 0)
		printf("lsb_eeprom_slave: Noack received from nxv = %l016x\n",
								nxv);
	return(status);
}

int
lsb_wait_ack(nxv)
volatile struct lsb_reg	*nxv;
{
	int status ;

	DELAY(LSB_EEPROM_BITWAIT);	/* just in case		*/
	nxv->lsb_eepr_cdr = LSB_XMT_SDAT ;
	mb();
	DELAY(LSB_EEPROM_BITWAIT);	/* console has 4 */
	nxv->lsb_eepr_cdr =  LSB_SCLK | LSB_XMT_SDAT ;
	mb();
	DELAY(LSB_EEPROM_BITWAIT);	/* console has 5 */
#ifdef EEPROM_DEBUG
printf("lsb_wait_ack: nxv->lsb_eepr_cdr = %x \n", nxv->lsb_eepr_cdr);
#endif /* EEPROM_DEBUG */

	status = (nxv->lsb_eepr_cdr & LSB_RCV_SDAT) ;
	nxv->lsb_eepr_cdr = LSB_XMT_SDAT ;
	mb();
	DELAY(LSB_EEPROM_SBITWAIT);	/* console had 1 */
	nxv->lsb_eepr_cdr = 0;
	mb();
	DELAY(LSB_EEPROM_BITWAIT);	/* console had 4 */

	return(status);
}

lsb_eeprom_wbit(nxv,data)
volatile struct lsb_reg	*nxv;
u_int data;
{
	DELAY(LSB_EEPROM_BITWAIT);	/* just in case		*/
	/* data is assume to be the single low-order bit	*/
	nxv->lsb_eepr_cdr = ((data & 1) << 1) |  (data & 1) ;
	mb();
	DELAY(LSB_EEPROM_SBITWAIT);		/* console had 1 */
	nxv->lsb_eepr_cdr = LSB_SCLK | ((data & 1) << 1) | (data & 1) ;
	mb();
	DELAY(LSB_EEPROM_BITWAIT);		/* console had 5 */
	nxv->lsb_eepr_cdr = 0 ;
	mb();
	DELAY(LSB_EEPROM_BITWAIT);		/* console had 5 */
}

/*
 * These are the LSB EEPROM start/stop bit sequences
 * The DELAYs and values come the console code, they conflict with the
 * laser EEPROM spec...
*/

lsb_eeprom_start(nxv)
volatile struct lsb_reg *nxv ;
{
	DELAY(LSB_EEPROM_BITWAIT);	/* just in case			*/
	nxv->lsb_eepr_cdr = 0x02 ; 
	mb();		/* Why? Because the console code does...	*/
	DELAY(LSB_EEPROM_SBITWAIT); 	/* console had 1		*/
	nxv->lsb_eepr_cdr = 0x06 ;
	mb();
	DELAY(LSB_EEPROM_BITWAIT); 	/* console had 5		*/
	nxv->lsb_eepr_cdr = 0x04 ; 
	mb();
	DELAY(LSB_EEPROM_BITWAIT); 	/* console had 4		*/
	nxv->lsb_eepr_cdr = 0x00 ; 
	mb();
	DELAY(LSB_EEPROM_BITWAIT);	/* console had 5		*/
}

lsb_eeprom_stop(nxv)
volatile struct lsb_reg *nxv ;
{
	DELAY(LSB_EEPROM_BITWAIT);	/* just in case			*/
	nxv->lsb_eepr_cdr = 0 ;
	mb();
	DELAY(LSB_EEPROM_SBITWAIT);	/* console had 1		*/
	nxv->lsb_eepr_cdr = 4 ;
	mb();
	DELAY(LSB_EEPROM_BITWAIT);	/* console had 5		*/
	nxv->lsb_eepr_cdr = 6 ;
	mb();
	DELAY(LSB_EEPROM_BITWAIT);	/* console had 5		*/
	nxv->lsb_eepr_cdr = 2 ;
	mb();
	DELAY(LSB_EEPROM_SBITWAIT);	/* console had 1		*/
	nxv->lsb_eepr_cdr = 0 ;
	mb();
	DELAY(LSB_EEPROM_BITWAIT);	/* console had 4		*/
	return(0);
}

int
lsb_eeprom_ack(nxv)
volatile struct lsb_reg	*nxv;
{
	int status ;

	DELAY(LSB_EEPROM_BITWAIT);	/* just in case			*/
	nxv->lsb_eepr_cdr = LSB_XMT_SDAT | LSB_RCV_SDAT ;
	DELAY(LSB_EEPROM_BITWAIT);	/* console had 4		*/
	nxv->lsb_eepr_cdr = LSB_SCLK | LSB_XMT_SDAT | LSB_RCV_SDAT ;
	DELAY(LSB_EEPROM_BITWAIT);	/* console had 5		*/
	status = nxv->lsb_eepr_cdr & LSB_RCV_SDAT ;
	nxv->lsb_eepr_cdr = LSB_XMT_SDAT ;
	DELAY(LSB_EEPROM_SBITWAIT);	/* console had 1		*/
	nxv->lsb_eepr_cdr = 0 ;
	DELAY(LSB_EEPROM_BITWAIT);	/* console had 4		*/

	if (status)
		printf("lsb_eeprom_ack: eeprom command NOACK'ed\n");
	return(status) ;
}

int
lsb_write_eeprom(nxv,adr,data)
volatile struct lsb_reg	*nxv;
	int	adr,data;
{
printf("lsb_write_eeprom: Not currently implemented\n");
return(0);
}

lsbsst(nxv)
volatile struct	lsb_reg *nxv;
{
	int totaldelay;
	int ret = 0;
	int s;
	register struct lsbdata *lsbdata;
	volatile struct lsb_reg *cpunode=0;

        s= splbio();
	nxv->lsb_lcnr = nxv->lsb_lcnr & LSB_NRST;

	/* need to give time for LSB bad line to be set */
	DELAY(10000);
	
	lsbdata = head_lsbdata;

	while(lsbdata) {
		if ((lsbdata->lsbnum != -1) &&
		    (nxv >= lsbdata->lsbvirt[0]) && 
		    (nxv <= lsbdata->lsbvirt[MAX_LSB_NODE - 1]))
		    	cpunode = lsbdata->cpu_lsb_addr;
		lsbdata = lsbdata->next;
	}

	if (!cpunode) panic("invalid lsb address");
	
	/* wait for LSB_XBAD line to be deasserted.  or 10 seconds.*/
	totaldelay = 1000;
	while((cpunode->lsb_lber & LSB_E) && (totaldelay-- > 0)) {
		DELAY(10000);
	}
/*
 * Wait for the self tests to finish (10 seconds)
 */
	totaldelay = 1000;
	while ((nxv->lsb_lcnr & LSB_STF)
	     &&(totaldelay > 0)) {
		--totaldelay;
		DELAY(10000);
	}
	nxv->lsb_lcnr = (nxv->lsb_lcnr & ~LSB_NRST);
	if (totaldelay > 0)
		ret = 1;

	splx(s);
	return (ret);
}



#include <hal/kn7aa.h> 	/* required for mctl structure setup */
#include <dec/binlog/errlog.h> 	/* required for packet allocation size */

/* must make se_packet_ptr available as global, so system shutdown can */
/* cause packet to be written */
struct el_soft_error_data *se_packet_ptr;

/*
 * lsb_crd_footprint:
 *   given the node number of an LMA and an address of a
 *   elr_crd_footprint structure, as defined in errlog.h this routine
 *   fills the crd footprint. These footprints are used to uniquely
 *   identify CRD errors, so the same error can be logged once with a
 *   count instead of many times.
 */
int lsb_crd_footprint(lsbnode, crd_footprint_ptr)
	int lsbnode;
	struct elr_crd_footprint *crd_footprint_ptr;
{
	
	/* setup a pointer to the node */
	struct lsb_lma_reg *lsb_node_ptr = lsb_crd_chk_mem[lsbnode];
	
	/* NSES is set when any of MERA bits 4,5,9,10,or */
	/* 11 are set. NSES is a read only bit and must be */
	/* clear by clearing MERA. */ 
	
	/* count the error for this memory module. */
	lsb_crd_count[lsbnode]++; /* increment CRD count for */
                        	  /* node */ 
	crd_footprint_ptr->mera = lsb_node_ptr->lma_mera;
	crd_footprint_ptr->merb = lsb_node_ptr->lma_merb;
	crd_footprint_ptr->amr = lsb_node_ptr->lma_amr;
	
	crd_footprint_ptr->flags = 0x1; /* Footprint is valid */
	
	/* ECC Syndrome<32:0> - this is an 8 bit field. In the */
	/* rare event of an error from both banks */
	if ((crd_footprint_ptr->mera & 0x10) && 
		(crd_footprint_ptr->mera & 0x20)) /* CERA and CERB */
	{
		crd_footprint_ptr->ecc_syndrome =
			((lsb_node_ptr->lma_msyndb & 0xFF) << 16) |
				(lsb_node_ptr->lma_msynda & 0xFF);
	}
	else if (crd_footprint_ptr->mera & 0x10)
	{
		crd_footprint_ptr->ecc_syndrome =
			(lsb_node_ptr->lma_msynda & 0xFF);
	}
	else if (crd_footprint_ptr->mera & 0x20)
	{
		crd_footprint_ptr->ecc_syndrome =
			(lsb_node_ptr->lma_msyndb & 0xFF);
	}
	else crd_footprint_ptr->ecc_syndrome = -1;
	
	crd_footprint_ptr->fru_id = lsbnode;
	
	crd_footprint_ptr->type.dram_type = lsb_node_ptr->lma_mcr & 0x1;
	crd_footprint_ptr->type.strings = 
		((lsb_node_ptr->lma_mcr >> 2) & 0x3); /* type<2:1> = mcr<3:2> */
	
	if((lsb_node_ptr->lma_ldev & LSBLDEV_TYPE) == LSB_BBMEM)
		crd_footprint_ptr->type.bb_ram = 0x1; 
	else crd_footprint_ptr->type.bb_ram = 0;
	crd_footprint_ptr->low_addr = lsb_node_ptr->lma_fadr;/* ?? */
	crd_footprint_ptr->high_addr = lsb_node_ptr->lma_fadr;/* ?? */
	crd_footprint_ptr->fadr = lsb_node_ptr->lma_fadr;
	crd_footprint_ptr->crd_count = lsb_crd_count[lsbnode];
}




lsb_se_crd_footprint( crd_footprint_ptr )
	struct elr_crd_footprint *crd_footprint_ptr;
	
{
	int packet_cnt;
	int copy_size;
	struct el_rec *elrec;
	
	/* just copy footprint for now... */
	
	/* get index to next se packet crd footprint */
	packet_cnt = (((sizeof(struct elr_crd_footprint) +
			se_packet_ptr->crd_buf_size) /
		       sizeof(struct elr_crd_footprint)) - 1);
	
	/* update packet information */
	copy_size = sizeof(struct elr_crd_footprint);
	bcopy( &crd_footprint_ptr->ecc_syndrome, &se_packet_ptr->crd_footprints[packet_cnt],
	      copy_size); 
	
	se_packet_ptr->crd_buf_size +=
		(sizeof(struct elr_crd_footprint)); /* add one packet */
	
	/* if the number of footprints is 16, or there is some other reason */
	/* to log the packets, this is the time to do it */
	
	/* for this pass, logging will occur for each and every packet */
	copy_size = (sizeof(struct el_soft_error_data) -sizeof(struct elr_crd_footprint)
		     +se_packet_ptr->crd_buf_size);
	
	/* allocate real error log packet */	      
	elrec = (struct el_rec *) binlog_alloc(copy_size, 4); /* severity informational */    
	
	if (elrec != (struct el_rec *)BINLOG_NOBUFAVAIL) {
		/* set identifiers in packet for bits to text translation */
		LSUBID(elrec, ELCT_MEM, ELMETYP_CRD, ELMACH_LASER, mfpr_whami(),
		       0, 0);
		
		/* copy error log data to real error log packet */
		bcopy(&se_packet_ptr->packet_revision, &elrec->el_body, copy_size);
	
		/* release the packet's buffer space */
	
		/* validate the error log packet */
		binlog_valid(elrec);
	}

	/* reset packet, until true multi-CRD support is implemented */
	se_packet_ptr->crd_buf_size = 0;
}



lma_clear_error(lsbnode)
     int lsbnode;
{

	struct lsb_lma_reg *lsb_node_ptr = lsb_crd_chk_mem[lsbnode];


	/* clear error state in memory. All writeable bits */
	/* are W1C in these registers*/

	lsb_node_ptr->lma_mera =	/* Clear error bits */
		lsb_node_ptr->lma_mera;	/* for correctable */
				        /* errors. CER | MULE */
				        /* | CERA | CERB */
      
#ifdef LMA_MAPPED_PG_3
	lsb_node_ptr->lma_merb =	/* Clear error bits */
		lsb_node_ptr->lma_merb;	/* for correctable */
				        /* errors. CER | MULE */
#endif
    
	mb();
}


/*
 *  routine: int lsb_crd_create_thread(param)
 *
 * This routine serves as a calling point, through init_main, which
 * guarentees execution will not occur before threads are available.
 * 
 * Polling for CRDs from Memory is a low priority task and so a thread
 * is used to perform this duty at the lowest priority (as initialized
 * through creation).
 *    
 */
int lsb_crd_create_thread(param)
	caddr_t param;
{
	(void) kernel_thread(first_task, lsb_crd_chk_thread);
}



int lsb_crd_chk_thread()
{ 
  int s;
  int lsbnode;
  struct elr_crd_footprint *crd_footprint_ptr;
  struct elr_crd_footprint temp_crd_footprint;
  
  /* allocate buffer in which to build error log packet. */
  /* Packet will be treated as though it were the error */
  /* log buffer, and later be copied into a real error log */
  /* buffer. */
  se_packet_ptr = (struct el_soft_error_data *)kalloc(NBPG);
  
  /* write packet revision to buffer */
  se_packet_ptr->packet_revision = 2;
  se_packet_ptr->crd_buf_size = 0;
  
  for (;;) /* infinite loop */
  {
	  /**/
	  /* lsb_crd_chk_mem is an array, initialized to 0 and */
	  /* then filled with the address of device registers */
	  /* for existing memory modules in array elements that */
	  /* reflect the position of the memory on the LSB. */
	  /*                                                */
	  /* This loop checks the array for existing memories */
	  /* and checks those memories for errors */
	  /**/
	  for(lsbnode = 0; lsbnode < MAX_LSB_NODE; lsbnode++)
	  { /* all lsb nodes */
		  
		  if (lsb_crd_chk_mem[lsbnode])
		  { /* node is a memory node */ 
			  
			  if (lsb_crd_chk_mem[lsbnode]->lma_lber & LSB_NSES)
			  { /* memory module has an error */ 
				  
				  /* call routine to fill in footprint */
				  lsb_crd_footprint(lsbnode, &temp_crd_footprint);
				  
				  /* clear lsb memory errors */
				  lma_clear_error(lsbnode);
				  
				  /* intergrate footprint with se packet */
				  lsb_se_crd_footprint(&temp_crd_footprint);
			  }
			  
		  } /* node is a memory node */
		  
	  } /* loop through lsb nodes */
	  
	  /* wait a while before polling again */
	  assert_wait((vm_offset_t)lsb_crd_chk_thread, FALSE);
	  thread_set_timeout(CRD_TIMEOUT*hz);
	  thread_block();
	  
  } /* infinite loop */
  
} /* lsb_crd_chk_thread */


/****************************************************************************
**
** Power System Monitoring
**
*****************************************************************************/

extern struct gbus_map gbus_map;
u_char* ps_uart_base;

#define PWR_SYS_MODULES	3
char pwr_sys_mod_ids[PWR_SYS_MODULES] = { 'A', 'B', 'C' };

int	pwr_sys_mon_delay = 300;

#define PWR_SYS_COM_SIZE	5
#define	PWR_SYS_BRIEF_SIZE	9
#define PWR_SYS_FULL_SIZE	54


#define PWR_SYS_TX_TIMEOUT	100
#define PWR_SYS_RX_TIMEOUT	100

/*
** RR0 bit masks
*/
#define TX_BUF_EMPTY	0x04
#define RX_CHAR_AVAIL	0x01


struct pwr_sys_cmd {
	u_char	header[2];
	u_char	command;
	u_char	id;
	u_char	term;
	u_char  fill[11];
};

struct pwr_sys_data_brief {
	union {
		u_char	data[9];
		struct {
			u_char	mod_id;
			u_char	batt_cap[2];
			u_char  heatsink_status;
			u_char  batt_pack_status;
			u_char  test_status;
			u_char  pwr_supp_status;
			u_char	checksum[2];
		} data_str;
	} data_un;
	u_char		fill[7]; /* fill struct to 16 bytes */
};

struct pwr_sys_data_full {
	union {
		u_char data[54];
		struct {
			u_char mod_id;
			u_char range;
			u_char rev[4];
			u_char peak_ac_voltage[4];
			u_char dc_bulk_voltage[4];
			u_char v48_dc_bus_voltage[4];
			u_char v48_dc_bus_current[4];
			u_char v48_batt_pack_voltage[4];
			u_char v24_batt_pack_voltage[4];
			u_char batt_pack_dis_current[4];
			u_char ambient_temp[4];
			u_char elapsed_run_time[4];
			u_char rem_batt_capacity[2];
			u_char batt_discharge_time[2];
			u_char spare[2];
			u_char heatsink_status;
			u_char batt_pack_status;
			u_char test_status;
			u_char pwr_supp_status;
			u_char checksum[2];
		} data_str;
	} data_un;

	u_char 		fill[10];	/* fill to 64 bytes */
};

struct pwr_sys_data_full_trans {
	u_char mod_id;
	u_char range;
	u_char rev[4];
	u_int peak_ac_voltage;
	u_int dc_bulk_voltage;
	u_int v48_dc_bus_voltage;
	u_int v48_dc_bus_current;
	u_int v48_batt_pack_voltage;
	u_int v24_batt_pack_voltage;
	u_int batt_pack_dis_current;
	u_int ambient_temp;
	u_int elapsed_run_time;
	u_short rem_batt_capacity;
	u_short batt_discharge_time;
	u_char spare[2];
	u_char heatsink_status;
	u_char batt_pack_status;
	u_char test_status;
	u_char pwr_supp_status;
	u_char checksum[2];
	u_char fill[2];
};
		
pwr_sys_check_status ( u_char );
pwr_sys_uart_read_reg ( u_int, u_char*, u_char* );
pwr_sys_uart_write_reg ( u_int, u_char*, u_char );

pwr_sys_timeout ( int param )

{
    /*
    ** Initialize
    */
    /* ps_uart_base = (u_char *) gbus_map.uart0 + GBUS_UART0A_PG_OFFSET; */
    ps_uart_base = (u_char *) gbus_map.uart1;

    timeout ( pwr_sys_thread, NULL, hz );


/*    kernel_thread(first_task, pwr_sys_thread); */
}

pwr_sys_thread ()

{
	/*
	** Get the status from the power supply
	*/
	/* pwr_sys_check_status ( 'B' ); /* Brief Power System Status */
	pwr_sys_check_status ( 'S' ); /* Full Power System Status */

	/*
	** Block for 1 minute, then check again
	*/
/*
	assert_wait((vm_offset_t)pwr_sys_thread, FALSE );
	thread_set_timeout ( pwr_sys_mon_delay*hz );
	thread_block ();
*/
	timeout ( pwr_sys_thread, NULL, pwr_sys_mon_delay*hz );

}

static u_int	log_data = 1;  /* log once at bootup.. */

pwr_sys_check_status ( u_char command )

{
    u_int	i,j;
    u_int	status;

    struct 	pwr_sys_cmd		cmd;

    struct 	pwr_sys_data_brief	brief_data;
    struct 	pwr_sys_data_full	full_data[PWR_SYS_MODULES];
    struct 	pwr_sys_data_full_trans	full_data_trans[PWR_SYS_MODULES];

    static 	u_int	saved_time[PWR_SYS_MODULES] = { 0, 0, 0 };

    /*
    ** Initialize the cmd packet.  It never changes except for the cmd.id,
    ** the power system module id ( either A, B, or C ), and the command
    ** itself, either 'S' (full status) or 'B' (brief status)
    */
    cmd.header[0] = 0x02;
    cmd.header[1] = 0x02;
    cmd.command = command;
    cmd.term = 0x0d;

    /*
    ** try to get a brief report from each power system module
    */
    for ( i = 0; i < PWR_SYS_MODULES; i++ )
    {
	cmd.id = pwr_sys_mod_ids[i];

	/*
	** transmit the command
	*/
	if ( !(pwr_sys_tx_cmd ( &cmd, PWR_SYS_COM_SIZE )) )
	{
 /* printf ( "\npwr_sys_get_brief_status: command transmit failed" ); */
	    return ( 0 );
	}

	/*
	** read the response
	*/
	if ( cmd.command == 'B' )
	{
            bzero ( &brief_data, PWR_SYS_BRIEF_SIZE );
	    status = pwr_sys_rx_data ( &brief_data, PWR_SYS_BRIEF_SIZE );
	}
	else if ( cmd.command == 'S' )
	{
            bzero ( &full_data[i], PWR_SYS_FULL_SIZE );
	    status = pwr_sys_rx_data ( &full_data[i], PWR_SYS_FULL_SIZE );
	}

        if ( !status )
	{
	    /*
	    ** Since we did not receive a reply, the module is not
	    ** present or is not connected to the UART interface
	    */
	    /*
	    printf ( "\nPOWER SYSTEM: Power Supply Module %c not responding",
			cmd.id );
	    */
	}
	else
	{
	    /*
	    ** If brief, report and return
	    */
	    if ( cmd.command == 'B' )
	    {
		pwr_sys_print_data_brief ( &brief_data );
		return ( 1 );
	    }

	    /*
	    ** First, translate the characters received into real numbers
	    */
	    bzero ( &full_data_trans[i], PWR_SYS_FULL_SIZE );
	    pwr_sys_trans_data_full ( &full_data[i], &full_data_trans[i] );

	    /*
	    ** Check for reportable conditions
	    */
	    if (
		/*
		** Battery is discharging, discharged, or failed
	 	*/
		( full_data_trans[i].batt_pack_status == '-' ) ||
		( full_data_trans[i].batt_pack_status == 'L' ) ||
		( full_data_trans[i].batt_pack_status == 'Z' ) ||
		/*
		** Battery pack test failed
		*/
		( full_data_trans[i].test_status == 'F' ) ||
		/*
		** Remaining battery capacity is low
		**
		*/
		( ( full_data_trans[i].rem_batt_capacity < 5 ) &&
		  ( full_data_trans[i].rem_batt_capacity > 0 ) ) ||
		/*
		** Power supply status is bad
		*/
		( full_data_trans[i].pwr_supp_status == '6' ) ||
		/*
		** Heatsink status is bad
		*/
		( full_data_trans[i].heatsink_status == 'F' ) ||
		( full_data_trans[i].heatsink_status == 'W' ) ||
		( full_data_trans[i].heatsink_status == 'B' ) 
	       )
	    {
		/*
		** then, Print the report
		*/
		pwr_sys_print_data_full ( &full_data_trans[i] );
		pwr_sys_mon_delay = 60;
		log_data = 1;

	    }

	    /*
	    ** Another ten hours has passed - log the current state
	    */
	    if ( full_data_trans[i].elapsed_run_time > saved_time[i] )
		log_data = 1;

	    /*
	    ** Update the saved time
	    */
	    saved_time[i] = full_data_trans[i].elapsed_run_time;
	}

    }

    /*
    ** if something happened, broke, or another ten hours passed, log
    */
    if ( log_data )
    {
	pwr_sys_log_data ( full_data );
	log_data = 0;
    }


    return ( 1 );

}


pwr_sys_trans_data_full ( struct pwr_sys_data_full* data,
			  struct pwr_sys_data_full_trans* data_trans )

{

    u_char	char_temp[5];
    u_int	int_temp;
    u_short	short_temp;

    /*
    ** Translate received data into actual values
    */
    char_temp[4] = 0;

    /* peak ac voltage */
    char_temp[0] = data -> data_un.data_str.peak_ac_voltage[0];
    char_temp[1] = data -> data_un.data_str.peak_ac_voltage[1];
    char_temp[2] = data -> data_un.data_str.peak_ac_voltage[2];
    char_temp[3] = data -> data_un.data_str.peak_ac_voltage[3];

    int_temp = atoi ( char_temp );

    if ( data->data_un.data_str.range == 'L' )
	data_trans -> peak_ac_voltage = (int_temp * 200) / 1024;
    else
	data_trans -> peak_ac_voltage = (int_temp * 400) / 1024;

    /* dc bulk voltage */
    char_temp[0] = data -> data_un.data_str.dc_bulk_voltage[0];
    char_temp[1] = data -> data_un.data_str.dc_bulk_voltage[1];
    char_temp[2] = data -> data_un.data_str.dc_bulk_voltage[2];
    char_temp[3] = data -> data_un.data_str.dc_bulk_voltage[3];

    int_temp = atoi ( char_temp );

    if ( data->data_un.data_str.range == 'L' )
	data_trans -> dc_bulk_voltage = ((int_temp * 90) / 1024) + 120;
    else
	data_trans -> dc_bulk_voltage = ((int_temp * 180) / 1024) + 240;

    /* 48 volt dc bus voltage */
    char_temp[0] = data -> data_un.data_str.v48_dc_bus_voltage[0];
    char_temp[1] = data -> data_un.data_str.v48_dc_bus_voltage[1];
    char_temp[2] = data -> data_un.data_str.v48_dc_bus_voltage[2];
    char_temp[3] = data -> data_un.data_str.v48_dc_bus_voltage[3];

    int_temp = atoi ( char_temp );

    data_trans -> v48_dc_bus_voltage = (int_temp * 50) / 1024;

    /* 48 volt dc bus current */
    char_temp[0] = data -> data_un.data_str.v48_dc_bus_current[0];
    char_temp[1] = data -> data_un.data_str.v48_dc_bus_current[1];
    char_temp[2] = data -> data_un.data_str.v48_dc_bus_current[2];
    char_temp[3] = data -> data_un.data_str.v48_dc_bus_current[3];

    int_temp = atoi ( char_temp );

    data_trans -> v48_dc_bus_current = (int_temp * 50) / 1024;

    /* 48 volt battery pack voltage */
    char_temp[0] = data -> data_un.data_str.v48_batt_pack_voltage[0];
    char_temp[1] = data -> data_un.data_str.v48_batt_pack_voltage[1];
    char_temp[2] = data -> data_un.data_str.v48_batt_pack_voltage[2];
    char_temp[3] = data -> data_un.data_str.v48_batt_pack_voltage[3];

    int_temp = atoi ( char_temp );

    data_trans -> v48_batt_pack_voltage = (int_temp * 60) / 1024;

    /* 24 volt battery pack voltage */
    char_temp[0] = data -> data_un.data_str.v24_batt_pack_voltage[0];
    char_temp[1] = data -> data_un.data_str.v24_batt_pack_voltage[1];
    char_temp[2] = data -> data_un.data_str.v24_batt_pack_voltage[2];
    char_temp[3] = data -> data_un.data_str.v24_batt_pack_voltage[3];

    int_temp = atoi ( char_temp );

    data_trans -> v24_batt_pack_voltage = (int_temp * 60) / 1024;

    /* battery pack discharge current */
    char_temp[0] = data -> data_un.data_str.batt_pack_dis_current[0];
    char_temp[1] = data -> data_un.data_str.batt_pack_dis_current[1];
    char_temp[2] = data -> data_un.data_str.batt_pack_dis_current[2];
    char_temp[3] = data -> data_un.data_str.batt_pack_dis_current[3];

    int_temp = atoi ( char_temp );

    data_trans -> batt_pack_dis_current = (int_temp * 100) / 1024;

    /* ambient temperature */
    char_temp[0] = data -> data_un.data_str.ambient_temp[0];
    char_temp[1] = data -> data_un.data_str.ambient_temp[1];
    char_temp[2] = data -> data_un.data_str.ambient_temp[2];
    char_temp[3] = data -> data_un.data_str.ambient_temp[3];

    int_temp = atoi ( char_temp );

    data_trans -> ambient_temp = (int_temp * 50) / 1024;

    /* elapsed run time */
    char_temp[0] = data -> data_un.data_str.elapsed_run_time[0];
    char_temp[1] = data -> data_un.data_str.elapsed_run_time[1];
    char_temp[2] = data -> data_un.data_str.elapsed_run_time[2];
    char_temp[3] = data -> data_un.data_str.elapsed_run_time[3];

    int_temp = atoi ( char_temp );

    data_trans -> elapsed_run_time = int_temp * 10;

    /* battery discharge time */
    char_temp[0] = data -> data_un.data_str.batt_discharge_time[0];
    char_temp[1] = data -> data_un.data_str.batt_discharge_time[1];
    char_temp[2] = 0;

    short_temp = atoi ( char_temp );

    data_trans -> batt_discharge_time = short_temp;

    /* remaining battery capacity */
    char_temp[0] = data -> data_un.data_str.rem_batt_capacity[0];
    char_temp[1] = data -> data_un.data_str.rem_batt_capacity[1];
    char_temp[2] = 0;

    short_temp = atoi ( char_temp );

    data_trans -> rem_batt_capacity = short_temp;

    /*
    ** copy remaining fields directly
    */
    data_trans -> mod_id = data -> data_un.data_str.mod_id;
    data_trans -> range = data -> data_un.data_str.range;
    bcopy ( (data -> data_un.data_str.rev), 
	    (data_trans -> rev), 
	    sizeof ( data_trans -> rev ) );
    data_trans -> heatsink_status = data -> data_un.data_str.heatsink_status;
    data_trans -> batt_pack_status = data -> data_un.data_str.batt_pack_status;
    data_trans -> test_status = data -> data_un.data_str.test_status;
    data_trans -> pwr_supp_status = data -> data_un.data_str.pwr_supp_status;


    return ( 1 );

}



pwr_sys_print_data_brief ( struct pwr_sys_data_brief* data )

{
    u_int 	j;

    printf ( "\n\n            Power System Data - Brief Report" );
    printf ( "\n" );

    printf ( "\nModule ID: %c", data->data_un.data_str.mod_id );
    printf ( "\n" );

    printf ( "\nBattery:" );
    printf ( "\n" );

    printf ( "\nBattery Pack Status: \t" );
    switch ( (int) data->data_un.data_str.batt_pack_status ) {

	case '0':
		printf ( "Battery Pack Not Installed" );
		break;

	case 'E':
		printf ( "Failed" );
		break;

	case 'B':
		printf ( "UPS Inhibit" );
		break;

	case 'C':
		printf ( "Charger Inhibit" );
		break;

	case 'Z':
		printf ( "Battery At End-of-Life" );
		break;

	case 'L':
		printf ( "Discharged" );
		break;

	case '-':
		printf ( "Discharging" );
		break;

	case '+':
		printf ( "Charging" );
		break;

	case 'X':
		printf ( "Charge Mode Longer than 24 Hours" );
		break;

	case 'F':
		printf ( "Fully Charged" );
		break;

	default:
		printf ( "Unknown" );
		break;

    }

    printf ( "\nBattery Test Status: \t" ); 
    switch ( (int) data->data_un.data_str.test_status ) {

	case '0':
		printf ( "Battery Pack Not Installed" );
		break;

	case 'W':
		printf ( "Battery Pack Not Ready" );
		break;

	case 'A':
		printf ( "Test Aborted" );
		break;

	case 'T':
		printf ( "Test In Progress" );
		break;

	case 'F':
		printf ( "Test Failed" );
		break;

	case 'P':
		printf ( "Test Passed" );
		break;

	default:
		printf ( "Unknown" );
		break;

    }

    printf ( "\nRemaining Battery Capacity: \t%c%c minutes",
				      data->data_un.data_str.batt_cap[0],
				      data->data_un.data_str.batt_cap[1] );
    printf ( "\n" );

    printf ( "\nPower Supply:" );
    printf ( "\n" );

    printf ( "\nPower Supply Status: \t" );
    switch ( (int) data->data_un.data_str.pwr_supp_status ) {

	case '0':
		printf ( "Normal AC Operation" );
		break;

	case '1':
		printf ( "UPS Mode" );
		break;

	case '2':
		printf ( "Breaker Open" );
		break;

	case '3':
		printf ( "No AC Voltage" );
		break;

	case '4':
		printf ( "Key Switch Off" );
		break;

	case '5':
		printf ( "Non-Fatal Fault" );
		break;

	case '6':
		printf ( "Fatal Fault" );
		break;

	default:
		printf ( "Unknown" );
		break;
    }

    printf ( "\nHeatsink Status: \t" );
    switch ( (int) data -> data_un.data_str.heatsink_status ) {

	case 'B':
		printf ( "Broken" );
		break;

	case 'F':
		printf ( "Fault (Red Zone)" );
		break;

	case 'W':
		printf ( "Warning (Yellow Zone)" );
		break;

	case '0':
		printf ( "Normal Operation (Green Zone)" );
		break;

	default:
		printf ( "Unknown" );
		break;
    }

    printf ( "\n" );


    return ( 1 );
}


pwr_sys_print_data_full ( struct pwr_sys_data_full_trans* data )

{
    u_int 	j;
    u_int	temp;


    printf ( "\n\n              Power System Data - Full Report" );
    printf ( "\n" );

    printf ( "\nModule ID: %c", data->mod_id );
    printf ( "\n" );

    printf ( "\nPrimary Micro Firmware Revision: %c.%c",
		data->rev[0], data->rev[1] );
    printf ( "\nSecondary Micro Firmware Revision: %c.%c",
		data->rev[2], data->rev[3] );
    printf ( "\n" );

    printf ( "\nBattery Pack Information:" );
    printf ( "\n" );

    printf ( "\nBattery Pack Status: \t" );
    switch ( (int) data->batt_pack_status ) {

	case '0':
		printf ( "Battery Pack Not Installed" );
		break;

	case 'E':
		printf ( "Failed" );
		break;

	case 'B':
		printf ( "UPS Inhibit" );
		break;

	case 'C':
		printf ( "Charger Inhibit" );
		break;

	case 'Z':
		printf ( "Battery At End-of-Life" );
		break;

	case 'L':
		printf ( "Discharged" );
		break;

	case '-':
		printf ( "Discharging" );
		break;

	case '+':
		printf ( "Charging" );
		break;

	case 'X':
		printf ( "Charge Mode Longer than 24 Hours" );
		break;

	case 'F':
		printf ( "Fully Charged" );
		break;

	default:
		printf ( "Unknown" );
		break;

    }

    printf ( "\nBattery Test Status: \t" ); 
    switch ( (int) data->test_status ) {

	case '0':
		printf ( "Battery Pack Not Installed" );
		break;

	case 'W':
		printf ( "Battery Pack Not Ready" );
		break;

	case 'A':
		printf ( "Test Aborted" );
		break;

	case 'T':
		printf ( "Test In Progress" );
		break;

	case 'F':
		printf ( "Test Failed" );
		break;

	case 'P':
		printf ( "Test Passed" );
		break;

	default:
		printf ( "Unknown" );
		break;

    }

    printf ( "\nRemaining Battery Capacity: \t%d minutes",
		data -> rem_batt_capacity );

    printf ( "\nBattery Discharge Time: \t%d minutes",
		data -> batt_discharge_time );

    printf ( "\n48 Volt Battery Pack Voltage: \t%d volts",
		data -> v48_batt_pack_voltage );

    printf ( "\n24 Volt Battery Pack Voltage: \t%d volts",
		data -> v24_batt_pack_voltage );

    printf ( "\nBattery Pack Discharge Current: \t%d amperes",
		data -> batt_pack_dis_current );

    printf ( "\n" );

    printf ( "\nPower Supply Information:" );
    printf ( "\n" );

    printf ( "\nPower Supply Status: \t" );
    switch ( (int) data->pwr_supp_status ) {

	case '0':
		printf ( "Normal AC Operation" );
		break;

	case '1':
		printf ( "UPS Mode" );
		break;

	case '2':
		printf ( "Breaker Open" );
		break;

	case '3':
		printf ( "No AC Voltage" );
		break;

	case '4':
		printf ( "Key Switch Off" );
		break;

	case '5':
		printf ( "Non-Fatal Fault" );
		break;

	case '6':
		printf ( "Fatal Fault" );
		break;

	default:
		printf ( "Unknown" );
		break;
    }

    printf ( "\nHeatsink Status: \t" );
    switch ( (int) data -> heatsink_status ) {

	case 'B':
		printf ( "Broken" );
		break;

	case 'F':
		printf ( "Fault (Red Zone)" );
		break;

	case 'W':
		printf ( "Warning (Yellow Zone)" );
		break;

	case '0':
		printf ( "Normal Operation (Green Zone)" );
		break;

	default:
		printf ( "Unknown" );
		break;
    }

    printf ( "\nPeak AC Line Voltage: \t%d volts", 
		data -> peak_ac_voltage );

    printf ( "\nDC Bulk Voltage: \t%d volts", 
		data -> dc_bulk_voltage );

    printf ( "\n48 Volt DC Bus Voltage: \t%d volts",
		data -> v48_dc_bus_voltage );

    printf ( "\n48 Volt DC Bus Current: \t%d amperes",
		data -> v48_dc_bus_current );

    printf ( "\nAmbient Temperature: \t%d C",
		data -> ambient_temp );

    printf ( "\nElapsed Run Time: \t%d Hours",
		data -> elapsed_run_time );

    printf ( "\n" );


    return ( 1 );

}


pwr_sys_log_data ( struct pwr_sys_data_full data[] )

{

    struct el_rec*			elp;
    u_int				el_size;

    struct el_laser_ps_data*		body_ptr;

    u_int				reg_data_idx;
    u_int				i, logger_running;

    /*
    ** Calculate the errlog buffer size that's required
    */
    el_size = sizeof ( struct el_laser_ps_data );

    /*
    ** Allocate an errlog buffer
    */
    elp = (struct el_rec *) binlog_alloc ( el_size, EL_PRISEVERE );
    if ( elp == (struct el_rec *) BINLOG_NOBUFAVAIL )
    {
	/*
	** Either the error logger is not running or there
	** are no buffers available..  Allocate enough memory
	** or the event-specific data, and fill in anyway.. then
	** printf the information to the screen.
	*/
	logger_running = 0;
	elp = (struct el_rec *) kalloc ( el_size + EL_MISCSIZE );
	if ( elp == NULL )
	    printf ( "\npwr_sys_log_data: unable to allocate error buffer\n" );
	    return ( 0 );
    }
    else
	logger_running = 1;

    /*
    ** Fill in the 'subid' section of the errlog record
    */
    elp -> elsubid.subid_class = ELCT_STATE;
    elp -> elsubid.subid_ctldevtyp = ELMACH_DEC7000;
    elp -> elsubid.subid_type = ELST_POWER_FAIL;
    elp -> elsubid.subid_errcode = EL_UNDEF;
    elp -> elsubid.subid_num = EL_UNDEF;
    elp -> elsubid.subid_unitnum = EL_UNDEF;


    /*
    ** Copy the power system data into the error record
    */
    body_ptr = (struct el_laser_ps_data *) &(elp -> el_body);

    /*
    ** Fill in the power supply data header information
    */
    body_ptr -> record_subtype = PS_ST_HISTORY_PACKET;


    for ( i = 0; i < PWR_SYS_MODULES; i++ )
    {
	/*
	** Set regulator summary bit
	*/
	if ( data[i].data_un.data[0] == pwr_sys_mod_ids[i] )
	    body_ptr -> reg_summary |= 1 << i;

	/*
	** Copy the power supply full report data
	*/
	reg_data_idx = i * PS_HIST_DATA_SIZE;

	bzero ( &(body_ptr -> reg_data[reg_data_idx]), PS_HIST_DATA_SIZE );
	bcopy (	data[i].data_un.data, 
		&(body_ptr -> reg_data[reg_data_idx]), PWR_SYS_FULL_SIZE );
    }

    /*
    ** Log the error
    */
    if ( logger_running )
	binlog_valid ( elp );


    return ( 1 );

}

/****************************************************************************
**
** Power System Monitoring - UART I/O routines
**
** These routines access the power supply UART registers
** They could be integrated into the console driver routines
** found in kernel/arch/alpha/hal/dec7000_cons.c if we wanted
** to change the routines there.
**
*****************************************************************************/

/*
** Routine: pwr_sys_tx_cmd
**
** Abstract: This routine transmits a command packet to 
**           the power system UART
*/
pwr_sys_tx_cmd ( u_char* data, u_int size )

{
    u_int	i = 0, delay = 0;
    u_char	rr0 = 0;


    for ( i=0; i<size; i++ )
    {
	delay = 0;

	/*
	** Initial read of RR0 to test for tx_buf_empty
	*/
	pwr_sys_uart_read_reg ( RR0, ps_uart_base, &rr0 );

	/*
	** wait for the transmit buffer to be empty
	*/
	while ( !(rr0 & TX_BUF_EMPTY) && (delay < PWR_SYS_TX_TIMEOUT) )
	{
	    DELAY ( 1000 );
	    delay++;

	    pwr_sys_uart_read_reg ( RR0, ps_uart_base, &rr0 );
	}

	/*
	** If the transmit buffer is empty, write the next character into it
	*/
	if ( rr0 & TX_BUF_EMPTY )
	{
 	    /*
	    ** write the next character
	    */
	    pwr_sys_uart_write_reg ( WR8, ps_uart_base, *data );
	    data++; /* increment to point to next character to transmit */
	}
	else
	    return ( 0 ); /* device isn't swallowing characters */
    }

    return ( 1 );  /* all characters successfully written */

}


/*
** Routine: pwr_sys_rx_data
**
** Abstract: This routine receives data of an expected length
**           from the power supply UART
*/
pwr_sys_rx_data ( u_char* buf, u_int size )

{
    u_int	i = 0, delay = 0;
    u_char	rr0;


    for ( i=0; i<size; i++ )
    {
	delay = 0;

 	/*
	** Initial read of RR0 to check for characters available
	*/
	pwr_sys_uart_read_reg ( RR0, ps_uart_base, &rr0 );

	/*
	** wait for a character to become available
	*/
	while ( !(rr0 & RX_CHAR_AVAIL) && (delay < PWR_SYS_RX_TIMEOUT) )
	{
	    DELAY ( 1000 );
	    delay++;

	    pwr_sys_uart_read_reg ( RR0, ps_uart_base, &rr0 );
	}


	/*
	** If there's a character available, read it.  Otherwise,
	** Assume that the device just isn't going to respone
	*/
	if ( rr0 & RX_CHAR_AVAIL )
	{
	    pwr_sys_uart_read_reg ( RR8, ps_uart_base, buf );
	    buf++; /* increment to address of next character */
	}
	else
	{
	    return ( 0 );  /* read data failure */
	}

    }

    return ( 1 );  /* We've read all of the characters we expected to get */

}

/*
** Routine: pwr_sys_uart_read_reg
**
** Abstract: This is a generic UART register read routine
**
*/
pwr_sys_uart_read_reg ( u_int reg, u_char* uart_base, u_char* value )

{
    u_long	longword;


    mb();


    switch ( reg ) {

	case RR0:
		longword = *(u_long *)uart_base;
		*value = (u_char)longword;
		break;

	case RR1:
		pwr_sys_uart_write_reg ( WR0, uart_base, RR1 );
		pwr_sys_uart_read_reg ( RR0, uart_base, value );
		break;

	case RR2:
		pwr_sys_uart_write_reg ( WR0, uart_base, RR2 );
		pwr_sys_uart_read_reg ( RR0, uart_base, value );
		break;

	case RR3:
		pwr_sys_uart_write_reg ( WR0, uart_base, RR3 );
		pwr_sys_uart_read_reg ( RR0, uart_base, value );
		break;

	case RR8:
		longword = *(u_long *)(uart_base + RR8_OFFSET);
		*value = (u_char)longword;
		break;

	case RR15:
		pwr_sys_uart_write_reg ( WR0, uart_base, RR15 );
		pwr_sys_uart_read_reg ( RR0, uart_base, value );
		break;

	default:
		printf ( "\nillegal register" );

    }

    mb();

    return ( 1 );
}

/*
** Routine: pwr_sys_uart_write_reg
**
** Abstract: This is a generic UART register write routine
**
*/
pwr_sys_uart_write_reg ( u_int reg, u_char* uart_base, u_char value )

{
    u_long	quadword;
    u_long* 	quadaddr;


    quadword = (value);
    quadaddr = (u_long *)uart_base;


    switch ( reg ) {

	case WR0:
		*quadaddr = quadword;
		break;

	case WR8:
		*(quadaddr + RR8_OFFSET) = quadword;
		break;

	default:
		printf ( "\nwrite_reg: illegal register" );
    }

    mb();

    return ( 1 );

}
