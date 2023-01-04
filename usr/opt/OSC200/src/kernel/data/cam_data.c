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
static char *rcsid = "@(#)$RCSfile: cam_data.c,v $ $Revision: 1.1.37.12 $ (DEC) $Date: 1993/12/20 22:21:04 $";
#endif

/* ---------------------------------------------------------------------- */

/* cam_data.c			Version 1.25		Dec. 11, 1991 */

/* The CAM data file.  This file contains the system setable parameters and
static data arrays needed for the CAM subsystem.

Modification History

	Version	  Date		Who	Reason

	1.00	03/27/91	jag	Creation date, from xpt_data.c. 
	1.01	04/10/91	dallas	Added device descriptor tables
					and supporting structures
					Incrmented the CCB pool size defines.
	1.02	06/03/91	jag	Fixed the N_cam_conftbl calc.
	1.03	06/07/91	maria	Changed alll NCAM references to
					NASC.
					Added DTYPE_SHIFT in device
					descriptor table entries.
					Added the default_scsiid
					declaration.
	1.04	06/19/91	dallas  Added max record size to the dev desc.
					This gives the user/operator the
					ability to match the device.
					Fixed TZK10 density table to write
					1 file mark on close.
					Fixed comment line for tz07
					Added descriptions on how to add
					a device.
	1.05	07/03/91	rps	Added sii defines.

	1.06	07/09/91	janet	Fixed "sim.h" include to be "sii.h"

	1.07	07/17/91	maria	Added SZ_NO_DISC, SZ_TRY_SYNC bits
					and removed SCSI2 for device descriptor
					flags.

	1.08	07/31/91	dallas	Fixed density tables to correspond
					to old major/minor pairs.. For
					compat until OSF.

	1.09	08/02/91	rln	Added () to NCAM

	1.10	08/21/91	dallas	Added TZ85 and TLZ04 to the
					device tables.
					
	1.11	09/03/91	maria	Added unknown device descriptor
					entries for all types of devices.
					
	1.12	09/11/91	maria	Changed SZ_TRY_SYNC flag to
					SZ_NOSYNC in device descrptor
					table entries.  Synchronous
					will not be attempted on: RRD40,
					RRD42, RZ23, TK50, TZ30, TZ05.
					Added entry RWZ01.

	1.13	09/12/91	jag	Added the CDrv EDT scan INQUIRY
					retry count.

	1.14	09/13/91	dallas	Added compression for tlz06.

	1.15	09/16/91	maria	Added RWZ01 partition table.

	1.16	09/19/91	maria	Removed the prevent/allow media
					removal optional command for RRD40.

	1.17	10/24/91	maria 	Updated the RZ58 queue size to
					its full size of 32 and disabled
					tagged queueing for RZ58 (SZ_NO_TAGS).
	1.18	11/19/91	dallas  Added densities for tapes and new
					updated rz58 partition sizes..
					Fixed unknown tape entry to reflect
					default density. Added error log
					limits for device drivers..
	1.19	11/20/91	janet	Merged in sim_data.c and xpt_data.c
	1.20	11/20/91	dallas  Deleted the use of the tzk08_dens
					table for the exabyte. It seems 
					that the device does not like 
					anything other then 0's for the
					density setting..
	1.21	11/22/91	dallas  Added the sending of the vendor
					unique data on open for the TKZ08.
					This disables odd byte disconnects for
					the TKZ08 which fixes the 3min problem.
	1.22	12/04/91	dallas	Changed the tzk10_dens table for 
					TZK10's. The unit will reject the 
					QIC-24 density setting.. Now we 
					will send a default setting.
	1.23	12/09/91	maria	Added SZ_NO_TAG for UNKNOWN disk
					device descriptor entry to turn
					off tagged queueing.
	1.24	12/11/91	maria	Added common buf struture pool
	1.25	12/11/91	jag	Added the global EDT scanning wait
					variable for the CDrv.
					size variables.

	NOTES:

*/

/* ---------------------------------------------------------------------- */

#include <io/common/iotypes.h>
#include <sys/types.h>			/* system level types */
#include <sys/time.h>			/* system level time defs */
#include <sys/param.h>			/* system level parameter defs */

#include <io/cam/dec_cam.h>		/* DEC CAM include file */
#include <mach/vm_param.h>
#include <io/cam/cam_debug.h>		/* debug structures for the CAM code */

#include <io/cam/cam.h>			/* General CAM include file */
#include <io/common/devio.h>
#include <io/cam/scsi_all.h>		/* SCSI 2 defines (structs) */
#include <io/cam/scsi_direct.h>		/* SCSI 2 defines (structs) */
#include <io/cam/scsi_sequential.h>	/* SCSI 2 defines (structs) */
#include <io/cam/pdrv.h>		/* General PDrv include file */
#include <io/cam/cam_tape.h>
#include <io/cam/sim_target.h>		/* Target mode defines and macros */
#include <io/cam/sim_cirq.h>		/* Circular Q defines and macros */
#include <io/cam/dme.h>			/* DME include file */
#include <io/cam/sim.h>			/* SIMs include file */
#include <io/cam/sim_94.h>		/* SIM94 include file */
#include <io/cam/ccfg.h>		/* Configuration driver include file */
#include <io/cam/uagt.h>		/* UAgt include file */
#include <io/cam/xpt.h>			/* XPT include file */
#include <io/cam/tcds_adapt.h>		/* TCDS include file */

#include "asc.h"			/* Config generated defines file */
#include "sii.h"			/* Config generated defines file */
#include "tcds.h"			/* Config generated defines file */
#include "tza.h"                        /* Config generated defines file */

#ifdef __alpha
#include "skz.h"		/* Defines # of config'ed XZA SCSI busses */
#include "siop.h"
#include "aha.h"
#define NKZQ	0
#else
#include "kzq.h"
#define NSKZ 0
#define NSIOP 0
#define NAHA 0
#endif

#define NCAM	(NASC+NSII+NSKZ+NSIOP+NKZQ+NAHA+NTZA)

/* ---------------------------------------------------------------------- */

/*
 * The system's SCSI bus id.
 */
int default_scsiid = DEFAULT_SCSIID;

/* ---------------------------------------------------------------------- */

/* The CAM CCB pool parameters.  These settings define the initial size, 
hi/low water marks and increment values.  These settings are used by the
XPT module. */

U32 cam_ccb_pool_size = 200;
U32 cam_ccb_high_water = 1000;
U32 cam_ccb_low_water = 100;
U32 cam_ccb_increment = 50;

/* ---------------------------------------------------------------------- */
/* The CAM CDrv EDT scanning parameters.  These settings help to define
the scanning behavior when the CDrv is filling in or updating the EDT
data structures. */

U32 ccfg_inquiry_retry_limit = 3;
U32 ccfg_use_sdtr_inquiry = CAM_TRUE;
U32 ccfg_wait_delay_loop = CCFG_WAIT_DELAY_LOOP;

/* ---------------------------------------------------------------------- */

/* JAG: These two arrays need to reference the BUS count. */

EDT *edt_dir[ NCAM ]; 		/* ptrs for EDT grid per HBA */

unsigned long N_edt_dir = (sizeof(edt_dir)/sizeof(EDT *));

/* ---------------------------------------------------------------------- */

CAM_SIM_ENTRY *cam_conftbl[ NCAM ];

U32 N_cam_conftbl = (sizeof(cam_conftbl)/sizeof(CAM_SIM_ENTRY *));

/* ---------------------------------------------------------------------- */
/*
 * CAM SIM data.
 */
#ifdef CAMDEBUG
/*
 * Camdbg_flag and camdbg_id are only used during debug.  The shipped
 * source files will not use them.
 */
U32 camdbg_flag = (U32)0;
U32 camdbg_id = (U32)0;
#endif CAMDEBUG

/*
 * Used to identify a CAM controller structure.
 */
char cam_ctlr_string[] = { "CAM" };

/*
 * Configuration structures.
 */
int nNHBA94 = NASC;
int nNSII = NSII;
int nNKZQ = NKZQ;
int nNTZA = NTZA;
int nCAMBUS = NCAM;
struct device *camdinfo[NCAM*NDPS*NLPT];
struct controller *camminfo[NCAM];
SIM_SOFTC *softc_directory[NCAM];

#if NTCDS > 0
TCDS_BUS tcds_bus_table[NTCDS];

/*
 * sim94_fast_enable is only here to quickly and easily turn OFF all fast
 * buses.  The driver now examines the H/W state flags in the V3.0 console
 * eeprom, or in the PMAZC-AA option rom to determine which buses are fast.
 *
 * The tcds_inter_loop_limit value determines the number of scsi bus
 * interrupts that can be handled during 1 interrupt cycle before forcing
 * an exit.  The lower the value, the better the interrupt latency for
 * other devices (at the same priority).  The higher the value, the lower
 * the scsi interrupt overhead is.
 */
U32 sim94_fast_enable = 1;

#ifdef RT_PREEMPT
U32 tcds_inter_loop_limit = 1;
#else
U32 tcds_inter_loop_limit = 4;
#endif
#endif

#if NSKZ > 0
#include "xza.h"		/* defines # of XZA's, usually NKSZ/2 */
#if NXZA > 0
void* xza_softc_directory[NXZA];
#endif
#endif

/*
 * One state machine queue is kept for all controllers.
 */
U32 sm_queue_sz = SM_QUEUE_SZ * NCAM;
SIM_SM_DATA sm_data[SM_QUEUE_SZ * NCAM];
SIM_SM sim_sm;

/*
 * If scsi_bus_reset_at_boot is set to CAM_TRUE, a SCSI bus reset
 * will be performed at boot time.  This should not be set to
 * CAM_TRUE unless you are SURE that it is necessary for your device
 * to operate.
 */
I32 scsi_bus_reset_at_boot = CAM_FALSE;

/*
 * The SIM default value.  This value is initially SIM_DEFAULT_TIMEOUT
 * which is 5 seconds.  This shouldn't be changed unless you are SURE
 * that it is necessary for you device to operate.
 */
I32 sim_default_timeout = SIM_DEFAULT_TIMEOUT;

/*
 * The SIM will perform I/O reordering if the CCB's "cam_sort" value
 * is non-zero.  In a busy system it is possible that certain I/O's
 * will have to wait a while if reordering is allowing a lot of other
 * I/O to go first.  "sim_sort_age_time" is the maximum number of
 * microseconds that an I/O is allowed to sit and wait.
 *
 * The default is 2 sec (2000000 usec).
 */
U32 sim_sort_age_time = 2000000;
U32 sim_allow_io_sorting = 1;
U32 sim_allow_io_priority_sorting = 1;
U32 sim_min_sort_depth = 3;

/* ---------------------------------------------------------------------- */

/*
 * The prihperal driver unit table.
 */
PDRV_UNIT_ELEM pdrv_unit_table[(NCAM) * NDPS * NLPT];

/*
 * The following are the variables used for the buf structure pool.
 */
U32 ccmn_bp_pool_size = 50;
U32 ccmn_bp_high_water = 90;
U32 ccmn_bp_low_water = 10;
U32 ccmn_bp_increment = 50;


/*
 * Error limits for the disk and tape drivers... Once the limits
 * are reached on a per device basis no more errlog entries will
 * be seen. A reboot clears...
 */

U32 cam_harderr_limit = 1000;
U32 cam_softerr_limit = 1000;

/*
 * The following allows changing the base timeout values for tape's
 * PLEASE NOTE ALL TIMES ARE IN SECONDS
 *
 * ctape_io_base_timo is the base timeout value for reads and write's.
 * For every 10000 bytes request a second is added to the base for the
 * timeout value.
 *
 * ctape_move_timo is the timeout value for a tape movement operations
 * that does not involve reads or writes.
 *
 * ctape_wfm_base_timo is the base value for writing filemarks. For
 * each filemark to be written 1 second is added.
 */
u_long ctape_io_base_timo = 300; 	/* 5 minutes			*/
u_long ctape_move_timo = 3600; 		/* 60 minutes			*/
u_long ctape_wfm_base_timo = 300;	/* 5 minutes Buffer flushes	*/

/*
 * For machines that don't issue a bus reset on boot issue a BDR to 
 * every tape drive seen. This is needed so when the machine boots
 * we can do our setups of the device. This will change once policy 
 * is determined for multi-initiator tapes.
 */
u_long ctape_boot_bdr = 1;		/* Issue a Bus device reset (tapes) */

/* 
 * Changable disk driver timeouts. cdisk_to_def for non read/write
 * commands to the disks (test unit ready, mode select etc.)
 * cdisk_io_def Hard fixed disk timeout value for i/o, and
 * cdisk_io_rmb for removable media disks since they are slow.
 */
u_long cdisk_to_def = 5;		/* 5 seconds			*/
u_long cdisk_io_def = 60;		/* 60 seconds (Tagged commands)	*/
u_long cdisk_io_rmb = 120;		/* 120 seconds (slow removables */

/*
 * (CAM_log_label_info):
 * Setting this flag to a non-zero value will cause informational
 * messages about disk labels to be logged to the syslog when an open
 * occurs for a SCSI/CAM disk.  This flag can be used to help debug
 * problems moving from ULTRIX disks to OSF/1 labelled disks.
 *
 * NOTE: When enabled, informational messages are always logged to
 * syslog (see /usr/adm/syslog.dated/<date>/kern.log for any messages).
 */
int CAM_log_label_info = 0;


/* ---------------------------------------------------------------------- */
/*
 * How to add "your own" SCSI (disk/tape/cdrom) device:
 *
 *    For those who would like to connect their own (non-DEC) devices to the
 *    SCSI bus the ULTRIX CAM driver allows the addition and definition of new
 *    devices in the "cam_data.c" file supplied with every binary or
 *    source license.
 *
 *    A good starting place would be to review the table before reading
 *    this description.
 *    A new entry should be added to cam_devdesc_tab describing the new device.  
 *    An example disk and tape entry might look like the following:
 *
 *    DEV_DESC  cam_devdesc_tab[] = {
 *
 *    {"DEC     RZ55", 12, DEV_RZ55, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
 *    (struct pt_info *)ccmn_rz55_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB, 
 *    SZ_BBR, 
 *    NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL ,
 *    36, 64
 *    },
 *
 *    {"DEC     TZK10", 13, DEV_TZK10, (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_QIC_CLASS,
 *    (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, DEC_MAX_REC, &tzk10_dens, &tzk10_mod,
 *    SZ_NO_FLAGS,
 *    NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
 *    36, 64
 *    },
 *
 *    { 0 },
 *
 *    };
 *
 *    The first field is the vendor returned string identifying the drive.
 *    This string is composed of the vendor id (first 8 chars) followed by
 *    the product id.
 *
 *    The second field is the length of the string, the total string returned 
 *    by the unit is what we match on.
 *
 *    The third field is the ULTRIX name for the device (DEV_RZ55 for this
 *    disk, DEV_TZK10 for this tape. For disk devices that are not from DIGITAL
 *    a generic name of DEV_RZxx should be used. For tapes a couple of generic
 *    categories are supplied. DEV_TZQIC (Quarter inch cartridge tape unit), 
 *    DEV_TZ9TRK (9 Track tape unit), DEV_TZ8MM ( 8 millimeter tape units),
 *    DEV_TZRDAT ( RDAT tape units), DEV_TZ3480 ( 3480 tape units), and
 *    for tape devices that do not fit into any of the above generic 
 *    classifications DEV_TZxx should be used.
 *
 *
 *    The fourth field is the class of SCSI device and sub_class. The upper
 *    4 bits are the class of the device. Please refer to 
 *    /usr/sys/h/scsi_all.h for a complete list of defines. The lower 28
 *    bits are the sub class. 
 *    Currently for disks the sub classes are:
 *    SZ_HARD_DISK	Normal disk unit.
 *    SZ_FLOP_3		Floppy disk 3.5"
 *    SZ_FLOP_5		Floppy disk 5.25"
 *
 *    For tapes the sub classes are:
 *    SZ_TK_CLASS	TK50 and TK30 Class tape.
 *    SZ_RDATE_CLASS	TLZ04, RDAT Class tape.
 *    SZ_9TRK_CLASS	9 Track Class tape.
 *    SZ_QIC_CLASS	Quarter inch Class tape.
 *    SZ_8MM_CLASS	8 Millimeter Class tape.
 *    SZ_3480_CLASS	3480 Cartridge Class tape.
 *    SZ_LOADER		Is there a loader tied to the unit that does not
 *			have a separate id. (Not implemented as of yet ).
 *    These defines can be found in /usr/sys/h/pdrv.h
 *    
 *    The fifth field is the partition table entry.  Tape units should use
 *    sz_null_sizes.  Disk devices may use sz_rzxx_sizes.  The sz_rzxx_sizes
 *    assumes that the disk is at least 48Mb.  DO NOT MODIFY sz_rzxx_sizes!
 *    To create your own partition table, you must make an entry in the
 *    "size" struct below.
 *
 *    The sixth field is the block/sector size of the unit. This field is
 *    used for disks and cdroms. The field is the number of bytes in a
 *    sector/block that the unit uses. Refer to the technical manual of 
 *    your device to get that number.
 *    
 *    The sixth field is the block/sector size of the unit is now being phased
 *    out. The cam_disk driver will get the block size from the device now.
 *    The field is being reused for RAID boxes. Since each SCSI RAID box
 *    is different the lower byte is used to tell the driver which page
 *    contains the RAID level for this LUN, the next byte is the offset
 *    within the page that contains the RAID level. The upper 2 bytes 
 *    the size of the page to request.
 *
 *    The seventh field is the maximun record size the unit can do in a single
 *    transfer. This field is used for raw transfers only and not nbuf/io.
 *    The field allows for single raw transfers to be allowed up the number
 *    of bytes defined by the field. Primarily used for tapes that contain
 *    seimic data which can have hugh record sizes. Please note that if your
 *    machine does not have the physical memory or the unit can not do the size
 *    transfer requested, errors will be a result.
 *    
 *    The eighth field is used for tapes only. The entry is a pointer to the
 *    tape units density structure. Please refer to the density structure 
 *    write up below.
 *
 *    The ninth field is a pointer to a mode select table. If the field
 *    is filled in the disk/tape/cdrom drivers will do mode selects
 *    for each page defined in the table on initial opens of the device.
 *    this field is optional and is Not recommended to be set up unless
 *    it is truely needed. Please refer to the write up for the mode select
 *    table write up below.
 *
 *    The tenth field are flags which direct the disk/tape/cdrom drivers
 *    on how to treat the device. Current flags are:
 *    SZ_NO_FLAGS	No flags defined.
 *    SZ_BBR		Device allows bad block replacement (disks only).
 *    SZ_NOSYNC		Device does not allow synchronous data transfer.
 *    SZ_NO_DISC	Device does not allow disconnects.
 *    SZ_NO_TAG		Do not use tagged queueing even though device
 *			supports it.
 *    SZ_REORDER	Allow the SIM to reorder specific CCB's.
 *    SZ_DISPERSE_QUE   Allow the queue depth to be spread across all
 *			LUNS connected to the device.
 *
 *    The eleventh field are optional commands. Some devices support a
 *    larger command set then just the mandatory ones defined for the
 *    device defined by the SCSI specs. Currently the optional command
 *    that are recognized by the disk/tape/cdrom drivers are:
 *    NO_OPT_CMDS	Device does not support optional commands
 *    SZ_RW10 		Device supports 10 byte CDB's for read and write.
 *    SZ_PREV_ALLOW     Device supports prevent/allow media removal commnad
 *    SZ_EXT_RESRV	Device supports extended reserve/release command.
 *
 *    The twelfth field is maximun ready time is seconds. For disks this
 *    field represents powerup/spinup time combined. For tape it represents
 *    powerup/load/rewind from End of Tape. This is the maximun amount of
 *    time in seconds the disk/tape/cdrom drivers will wait for a unit to
 *    become ready.
 *
 *    The thirteenth field is tag queue depth. If the device supports tag
 *    queuing this field represents the depth of the queue the device 
 *    supports. Refer to your device manual to see if your device supports
 *    tag queuing and the depth of the queue.
 *
 *    The fourteenth field are valid flags for the inquirey data lenght 
 *    and request sense data lenght. Indicates which fields are valid.
 *    Defines are:
 *    DD_REQSNS_VAL	Request Sense lenght is valid.
 *    DD_INQ_VAL	Inquirey lenght is valid.
 *
 *    The fifteenth is the inquirey data lenght for the device.
 *    Must be used in conjunction with the DD_INQ_VAL flag.
 *
 *    The sixteenth field is the request sense data lenght for 
 *    the device. Must be used in conjuction with DD_REQSNS_VAL flag.
 *
 *    Below is the structure definition for the struct dev_desc 
 * 
 *
 *    Device Descriptor Structure
 *    There will be a device descriptor structure entry for each
 *    specific device supported by DEC.  A user may supply a new
 *    entry to the device descriptor table by adding it to TBD and
 *    relinking the kernel or by sending it via an ioctl() call
 *    (TBD).  The following describes an entry in the device
 *    descriptor table:
 *
 *    typedef struct dev_desc {
 *	   u_char	dd_pv_name[IDSTRING_SIZE];  
 *	   u_char  	dd_length;
 *	   u_char	dd_dev_name[DEV_NAME_SIZE];	
 *	   u_long  	dd_device_type;
 *	   struct  	pt_info *dd_def_partition;
 *	   u_long  	dd_block_size;
 *	   u_long	dd_max_record;
 *	   DENSITY_TBL *dd_density_tbl;
 *	   MODESEL_TBL *dd_modesel_tbl;
 *	   u_long  dd_flags;
 *	   u_long  dd_scsi_optcmds;
 *	   u_long  dd_ready_time;
 *	   u_short dd_que_depth;
 *	   u_char  dd_valid;
 *	   u_char  dd_inq_len; 
 *	   u_char  dd_req_sense_len;
 *    }DEV_DESC;
 *
 *
 *    The density table allows the definition of eight densities
 *    for each type of tape unit. The current method of writing
 *    a particular density is by selection of the units minor 
 *    numbers lower 3 bits. Please refer to the units technical
 *    manual for the density/compression codes and blocking factors
 *    for each density. Each density is made up of 
 *    the following:
 *    	The first field are valid flags which signify which 
 *	of the following fileds for this density are valid.
 *	Flags are
 *	DENS_VALID              Is the structure valid 
 *	ONE_FM                  Write 1 filemark on close QIC
 *	DENS_SPEED_VALID        Speed setting valid. Multi speed
 *	DENS_BUF_VALID		Run in buffered mode (cache)
 *	DENS_COMPRESS_VALID 	Compression code if supported
 *
 *	The second field is the scsi density code for this density.
 *
 *	The third field is the scsi compression code for this density
 *	if the unit supports compression.
 *
 *	The fourth field is the setting speeding for this density.
 *	Some units are varible speed for certain densities.
 *
 *	The fifth field buffer control setting. 
 *
 *	The sixth field is the blocking factor for this density
 *	Certain units (QIC) will only write data in fixed lenght
 *	blocks to the tape. This field is the number of bytes in
 *	it's blocking factor. The field is NULL if this density
 *	blocking factor is variable.
 *
 *	Below is the TZK10's density table. Please note that this
 *	unit supports both fix and variable records.
 *
 *   DENSITY_TBL 
 *   tzk10_dens = {
 *       { Minor 00 or rmtXl 
 *
 *       Flags 
 *       DENS_VALID | DENS_BUF_VALID |ONE_FM ,
 *
 *       Density code	Compression code	Speed setting
 *       SEQ_QIC120,		NULL,			NULL,
 *
 *       Buffered setting	Blocking		
 *       1,			512
 *       },
 *       { Minor 01 or rmtXh 
 *
 *       Flags 
 *       DENS_VALID | DENS_BUF_VALID |ONE_FM ,
 *
 *       Density code	Compression code	Speed setting	
 *       SEQ_QIC320,		NULL,			NULL,
 *
 *       Buffered setting	Blocking				
 *       1,			512
 *       },
 *       { Minor 02 or rmtXm 
 *
 *       Flags 
 *       DENS_VALID | DENS_BUF_VALID |ONE_FM ,
 *
 *       Density code	Compression code	Speed setting	
 *       SEQ_QIC150,		NULL,			NULL,
 *
 *       Buffered setting	Blocking				
 *       1,			512
 *       },
 *       { Minor 03 or rmtXa 
 *
 *       Flags 
 *       DENS_VALID | DENS_BUF_VALID  |ONE_FM,
 *
 *       Density code	Compression code	Speed setting	
 *       SEQ_8000R_BPI,		NULL,			NULL,
 *
 *       Buffered setting	Blocking				
 *       1,			512
 *       },
 *       { Minor 04 
 *
 *       Flags 
 *       DENS_VALID | DENS_BUF_VALID |ONE_FM ,
 *
 *       Density code	Compression code	Speed setting	
 *       SEQ_QIC150,		NULL,			NULL,
 *
 *       Buffered setting	Blocking				
 *       1,			512
 *       },
 *       { Minor 05 
 *
 *       Flags 
 *       DENS_VALID | DENS_BUF_VALID |ONE_FM ,
 *
 *       Density code	Compression code	Speed setting	
 *       SEQ_QIC320,		NULL,			NULL,
 *
 *       Buffered setting	Blocking				
 *       1,			1024
 *       },
 *       { Minor 06 
 *
 *       Flags 
 *       DENS_VALID | DENS_BUF_VALID |ONE_FM ,
 *
 *       Density code	Compression code	Speed setting	
 *       SEQ_QIC320,		NULL,			NULL,
 *
 *       Buffered setting	Blocking				
 *       1,			1024
 *       },
 *       { Minor 07 
 *
 *       Flags 
 *       DENS_VALID | DENS_BUF_VALID |ONE_FM ,
 *
 *       Density code	Compression code	Speed setting	
 *       SEQ_QIC320,		NULL,			NULL,
 *
 *       Buffered setting	Blocking				
 *       1,			NULL
 *       }
 *   };	end of tzk10_dens 
 *
 *    Below is the structure definition for the density_tbl
 *
 *    typedef struct density_tbl {
 *    	struct density{
 *    	   u_char    den_flags;		
 *    	   u_char    den_density_code;
 *    	   u_char    den_compress_code;
 *    	   u_char    den_speed_setting;
 *    	   u_char    den_buffered_setting; 
 *    	   u_long    den_blocking;	
 *    	}density[MAX_TAPE_DENSITY]; MAX_TAPE_DENSITY defined as 8 
 *    }DENSITY_TBL;
 *
 *    The mode select table allows up to 8 mode selects to be
 *    done for a device type on initial open of a unit.
 *    The mode select table for a device consists of 8 entries.
 *    Each entry has the following format:
 *    
 *    The first field is the SCSI page number for that device
 *    type. If the device is a tape and you want to set the
 *    device configuration page for tapes, the page number
 *    would be a hex 10.
 *
 *    The second field is the pointer to the mode select data.
 *    You must set up the page data and place the address
 *    of the page structure in this field.
 *
 *    The third field is the lenght of the page. This is the
 *    number of bytes of the page size you want sent to the 
 *    device.
 *
 *    The fourth field are flags for the mode select cdb the
 *    driver formats. Bit 0 a zero don't save page , a one
 *    save the page. Bit 1 a zero SCSI 1 device, a one 
 *    means a SCSI 2 device.
 *
 *    Below you will find the the following:
 *    1. Mode select table struct definition
 *    2. An example filled out mode select table for a TZK10 
 *    3. An filled out page definition for page hex 10 for
 *       the TZK10.
 *    
 *
 *     Mode Select Table Structure Definition:
 *
 *     typedef struct modesel_tbl {
 *     	   struct ms_entry{
 *     	       u_char  ms_page;	
 *     	       u_char  *ms_data;
 *     	       u_char  ms_data_len;
 *     	       u_char  ms_ent_sp_pf;
 *     	   }ms_entry[MAX_OPEN_SELS]; MAX_OPEN_SELS defined as 8
 *     }MODESEL_TBL;
 *
 *    MODESEL_TBL
 *    tzk10_mod = {
 *    	{ MODE PAGE ENTRY 1	
 *
 *    	Page number		The data pointer		
 *    	0x02,			(u_char *)&tzk10_page2,
 *
 *    	Data len		SCSI2??				
 *    	   28,			0x2
 *    	},
 *    	{ MODE PAGE ENTRY 2	
 *
 *    	Page number		The data pointer		
 *    	0x10,			(u_char *)&tzk10_page10,
 *
 *    	Data len		SCSI2??				
 *    	   28,			0x2
 *    	},
 *    	{ MODE PAGE ENTRY 3	
 *
 *    	Page number		The data pointer		
 *    	NULL,			(u_char *)NULL,
 *
 *    	Data len		SCSI2??				
 *    	NULL,			NULL
 *    	},
 *    	{ MODE PAGE ENTRY 4	
 *
 *    	Page number		The data pointer		
 *    	NULL,			(u_char *)NULL,
 *
 *    	Data len		SCSI2??				
 *    	NULL,			NULL
 *    	},
 *    	{ MODE PAGE ENTRY 5	
 *
 *    	Page number		The data pointer		
 *    	NULL,			(u_char *)NULL,
 *
 *    	Data len		SCSI2??				
 *    	NULL,			NULL
 *    	},
 *    	{ MODE PAGE ENTRY 6	
 *
 *    	Page number		The data pointer		
 *    	NULL,			(u_char *)NULL,
 *
 *    	Data len		SCSI2??				
 *    	NULL,			NULL
 *    	},
 *    	{ MODE PAGE ENTRY 7	
 *
 *    	Page number		The data pointer		
 *    	NULL,			(u_char *)NULL,
 *
 *    	Data len		SCSI2??				
 *    	NULL,			NULL
 *    	},
 *    	{ MODE PAGE ENTRY 8	
 *
 *    	Page number		The data pointer		
 *    	NULL,			(u_char *)NULL,
 *
 *    	Data len		SCSI2??				
 *    	NULL,			NULL
 *    	},
 *    };
 *
 *    SEQ_MODE_DATA6
 *    tzk10_page10 = {
 *    
 *    	{ Parameter header 
 *
 *    	mode_len	medium type	speed	
 *    	NULL,		NULL,		NULL,
 *
 *    	Buf_mode	wp		blk_desc_len	
 *    	0x01,		NULL,		sizeof(SEQ_MODE_DESC)
 *    	},
 *    	{ Mode descriptor 
 *
 *    	Density	num_blks2	num_blks1	
 *    	NULL,		NULL,		NULL,
 *
 *    	num_blks0	reserved	blk_len2	
 *    	NULL,				NULL,
 *
 *    	blk_len1	blk_len0			
 *    	NULL,		NULL
 *    	},
 *    	{
 *    	Page data for page 0x2 			
 *
 *    	PAGE header 
 *    	byte0	byte1				
 *    	   0x10,	0x0e,
 *
 *    	byte2    byte3    byte4    byte5    byte6	
 *    	   0x00,    0x00,    40,      40,      NULL,
 *
 *    	byte7    byte8    byte9    byte10   byte11	
 *    	   NULL,    0xe0,    NULL,    0x38,    NULL,
 *
 *    	byte12   byte13   byte14   byte15   		
 *    	   NULL,    NULL,    NULL,    NULL
 *    	}
 *     };
 *
 *********************************************************************/

/*
 * The follow table is used for tapes and the DEVIOCGET ioctl.
 * The category_stat member of the structure returned has in bits
 * 4 - 31 a bit setting of BPI that the tape runs at. We use
 * the density the drive returns to use as an index into the table
 * for the correct defined bit setting. This should really change
 * A null means that there is no associated define.
 * 
 * Define the tape density table.
 */
DENS_TBL_ENTRY   density_table[] = {
	0,		NULL,		/* 0x00 - Default density.	*/
	DEV_800BPI,	NULL,		/* 0x01 - 800 BPI   (NRZI, R)	*/
	DEV_1600BPI,	NULL,		/* 0x02 - 1600 BPI  (PE, R)	*/
	DEV_6250BPI,	NULL,		/* 0x03 - 6250 BPI  (GCR, R)	*/
	DEV_8000_BPI,	NULL,		/* 0x04 - 8000 BPI  (GCR, C)	*/
	DEV_8000_BPI,	NULL,		/* 0x05 - 8000 BPI  (GCR, C)	*/
	0,		NULL,		/* 0x06 - 3200 BPI  (PE, R)	*/
	0,		NULL,		/* 0x07 - 6400 BPI  (IMFM, C)	*/
	DEV_8000_BPI,	NULL,		/* 0x08 - 8000 BPI  (GCR, CS)	*/
	DEV_38000BPI,	NULL,		/* 0x09 - 37871 BPI (GCR, C)	*/
	DEV_6666BPI,	NULL,		/* 0x0A - 6667 BPI  (MFM, C)	*/
	DEV_1600BPI,	NULL, 		/* 0x0B - 1600 BPI  (PE, C)	*/
	0,		NULL,		/* 0x0C - 12690 BPI (GCR, C)	*/
	DEV_10000_BPI,	512,		/* 0x0D - QIC-120 with ECC.	*/
	DEV_10000_BPI,	512,		/* 0x0E - QIC-150 with ECC.	*/
	DEV_10000_BPI,	512,		/* 0x0F - QIC-120   (GCR, C)	*/
	DEV_10000_BPI,	512,		/* 0x10 - QIC-150   (GCR, C)	*/
	DEV_16000_BPI,	NULL,		/* 0x11 - QIC-320   (GCR, C)	*/
	0,		NULL,		/* 0x12 - QIC-1350  (RLL, C)	*/
	DEV_61000_BPI,	NULL,		/* 0x13 - 4mm Tape  (DDS, CS)	*/
	DEV_54000_BPI,	NULL,		/* 0x14 - 8mm Tape  (???, CS)	*/
	DEV_45434_BPI,	NULL,		/* 0x15 - 8mm Tape  (RLL, CS)	*/
	DEV_10000_BPI,	NULL,		/* 0x16 - TK70 10000bpi mfm	*/
	DEV_42500_BPI,	NULL,		/* 0x17 - TZ85 42500bpi mfm	*/
	DEV_42500_BPI,	NULL,		/* 0x18 - TZ86 42500bpi mfm 56trk */
	DEV_62500_BPI, 	NULL,		/* 0x19 - TZ87 62500bpi mfm 64trk */
	0, 		NULL,	 	/* 0x1A -                       */
	0, 		NULL,	 	/* 0x1B -                       */
	0, 		NULL,	 	/* 0x1C -                       */
	0, 		NULL,	 	/* 0x1D -                       */
	DEV_36000_BPI,	NULL,	 	/* 0x1E - TZK11 - QIC-1GIG      */
	0, 		NULL,	 	/* 0x1F -                       */
	0, 		NULL,	 	/* 0x20 -                       */
	0, 		NULL,	 	/* 0x21 -                       */
	DEV_40640_BPI,	NULL,	 	/* 0x22 - TZK11 - QIC-2GIG      */
	0, 		NULL,	 	/* 0x23 -                       */
	DEV_61000_BPI,	NULL,	 	/* 0x24 -  TLZ07                */
	0, 		NULL,	 	/* 0x25 -                       */
	0, 		NULL,	 	/* 0x26 -                       */
	0, 		NULL,	 	/* 0x27 -                       */
	0, 		NULL,	 	/* 0x28 -                       */
	0, 		NULL	 	/* 0x29 -                       */

};
/*
 * Do not change the below line.
 */
int density_entrys = sizeof(density_table) / sizeof(struct dens_tbl_entry);


/*
 * The density structures for the known devices. You can
 * added your own at the bottom
 */
DENSITY_TBL 
def_dens = {
    {
    { /* Minor 00  or rmtXl*/
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL 
    },
    { /* Minor 01 or rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 02 or rmtXm*/
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 03  or rmtXa*/
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
}, /* end of def_dens */

tk50_dens = {
    {
    { /* Minor 00  or rmtXl*/
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL 
    },
    { /* Minor 01 or rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 02 or rmtXm*/
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 03  or rmtXa*/
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
}, /* end of tk50_dens */

tz85_dens = {
    {
    { /* Minor 00  or rmtXl*/
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_42500_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL 
    },
    { /* Minor 01 or rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_42500_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 02 or rmtXm*/
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_42500_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 03  or rmtXa*/
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_42500_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_42500_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_42500_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_42500_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_42500_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
}, /* end of tz85_dens */

tz86_dens = {
    {
    { /* Minor 00  or rmtXl*/
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_42500_BPI,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 01 or rmtXh */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID,
    /* Density code     Compression code        Speed setting   */
    0x18,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 02 or rmtXm*/
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    0x18,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 03  or rmtXa*/
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_42500_BPI,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 04 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    0x18,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 05 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    0x18,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 06 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    0x18,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 07 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    0x18,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    }
    }
}, /* end of tz86_dens */

tz87_dens = {
    {
    { /* Minor 00  or rmtXl*/
    /* Flags */
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID ,
    /* Density code     Compression code        Speed setting   */
    0x18,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 01 or rmtXh */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code     Compression code        Speed setting   */
    SEQ_62500_BPI,              0x01,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 02 or rmtXm*/
    /* Flags */
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_62500_BPI,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 03  or rmtXa*/
    /* Flags */
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_42500_BPI,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 04 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_62500_BPI,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 05 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_62500_BPI,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 06 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_62500_BPI,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 07 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_62500_BPI,              NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    }
    }
}, /* end of tz87_dens */

tkz60c_dens = {
    {
    { /* Minor 00  or rmtXl*/
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_DEF_BPI,                NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 01 or rmtXh */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID,
    /* Density code     Compression code        Speed setting   */
    SEQ_DEF_BPI,                0x01,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 02 or rmtXm*/
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_DEF_BPI,                0x01,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 03  or rmtXa*/
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_DEF_BPI,                NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 04 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_DEF_BPI,                NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 05 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_DEF_BPI,                NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 06 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_DEF_BPI,                NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 07 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_DEF_BPI,                NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    }
    }
}, /* end of tkz60c_dens */

tkz60_dens = {
    {
    { /* Minor 00  or rmtXl*/
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL 
    },
    { /* Minor 01 or rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 02 or rmtXm*/
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 03  or rmtXa*/
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
}, /* end of tkz60_dens */

tk30_dens = {
    {
    { /* Minor 00 or rmtXl */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 01 or rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 02 or rmtXm */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 03 or rmtXa */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
},	/* end of tk30_dens */

tlz04_dens = {
    {
    { /* Minor 00 or rmtXl */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 01 or rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 02 or rmtXm */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 03 or rmtXa */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
},	/* end of tlz04_dens */

tlz06_dens = {
    {
    { /* Minor 00 or rmtXl */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 01 or rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,              0x01,                   NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 02 or rmtXm */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,              0x01,                   NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 03 or rmtXa */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_61000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
},	/* end of tlz06_dens */

tlz07_dens = {
    {
    { /* Minor 00 or rmtXl */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 01 or rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,              0x01,                   NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 02 or rmtXm */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,              0x01,                   NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 03 or rmtXa */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID | DENS_COMPRESS_VALID,
    /* Density code	Compression code	Speed setting	*/
    SEQ_DEF_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
},	/* end of tlz06_dens */

tz05_dens = {
    {
    { /* Minor 00 or rmtXl */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_1600R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 01 or rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_1600R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 02 or rmtXm */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_1600R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 03 or rmtXa */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_1600R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_1600R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_1600R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_1600R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_1600R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
},	/* end of tz05_dens */

tz07_dens = {
    {
    { /* Minor 00 or rmtXl */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_1600R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 01 or rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_6250R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 02 or rmtXm */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_1600R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 03 or rmtXa */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_6250R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_6250R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_6250R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_6250R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_6250R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
},	/* end of tz07_dens */

tzk10_dens = {
    {
    { /* Minor 00 rmtXl */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC120,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			512
    },
    { /* Minor 01 rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC320,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 02 rmtXm */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC150,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			512
    },
    { /* Minor 03 rmtXa */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID  |ONE_FM,
    /* Density code	Compression code	Speed setting	*/
    NULL,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			512
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC150,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			512
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC320,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			1024
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC320,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			1024
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC320,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
},	/* end of tzk10_dens */

tzk11_dens = {
    {
    { /* Minor 00 rmtXl */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code     Compression code        Speed setting   */
    SEQ_QIC120,         NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  512
    },
    { /* Minor 01 rmtXh */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    NULL, 	        NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    },
    { /* Minor 02 rmtXm */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code     Compression code        Speed setting   */
    SEQ_QIC150,	         NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  512
    },
    { /* Minor 03 rmtXa */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID  |ONE_FM,
    /* Density code     Compression code        Speed setting   */
    NULL,               NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  512
    },
    { /* Minor 04 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code     Compression code        Speed setting   */
    NULL,         	NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  512
    },
    { /* Minor 05 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code     Compression code        Speed setting   */
    NULL,         	NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  1024
    },
    { /* Minor 06 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code     Compression code        Speed setting   */
    NULL,      		   NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  1024
    },
    { /* Minor 07 */
    /* Flags */
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code     Compression code        Speed setting   */
    NULL,	         NULL,                   NULL,
    /* Buffered setting Blocking                                */
    1,                  NULL
    }
    }
},      /* end of tzk11_dens */

viper_2525_dens = {
    {
    { /* Minor 00 or rmtXl */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC120,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			512
    },
    { /* Minor 01 or rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC320,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			512
    },
    { /* Minor 02 or rmtXm */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC150,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			512
    },
    { /* Minor 03 or rmtXa */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID  |ONE_FM,
    /* Density code	Compression code	Speed setting	*/
    SEQ_8000R_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			512
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC320,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC320,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC320,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID |ONE_FM ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_QIC320,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
},	/* end of viper_25225_dens */

tzk08_dens = {
    {
    { /* Minor 00 or rmtXl */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_54000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 01 or rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_54000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 02 or rmtXm */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_54000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 03 or rmtXa */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_54000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_54000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_54000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_54000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_54000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
},
tkz09_dens = {
    {
    { /* Minor 00 or rmtXl */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_54000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 01 or rmtXh */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_45434_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 02 or rmtXm */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_45434_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 03 or rmtXa */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_54000_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 04 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_45434_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 05 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_45434_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 06 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_45434_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    },
    { /* Minor 07 */
    /* Flags */ 
    DENS_VALID | DENS_BUF_VALID ,
    /* Density code	Compression code	Speed setting	*/
    SEQ_45434_BPI,		NULL,			NULL,
    /* Buffered setting	Blocking				*/
    1,			NULL
    }
    }
};	/* end of tkz09_dens */
/* End density structures.. Please extend the table
 * for your own tape drives.. The table is designed
 * to allow for flexibility and customer selections
 */

/* 
 * Mode Select issued to CD-ROM devices to set block length to 512 bytes.
 */
DIR_MODE_DATA6
cdrom_page0 = {

	{ /* Parameter header */
	/* mode_len	medium type	dpofua */
	0,		0,		0,
	/* wp	blk_desc_len	*/
	0,	sizeof(DIR_MODE_DESC)
	},
	{ /* Mode descriptor */
	/* Density	num_blks2	num_blks1	*/
	0,		0,		0,
	/* num_blks0	reserved	blk_len2	*/
	0,				00,
	/* blk_len1	blk_len0			*/
	0x2,		00
	},
};

/* 
 * tzk08 page 0 SCSI 1
 */
SEQ_MODE_DATA6
tzk08_page0 = {

	{ /* Parameter header */
	/* mode_len	medium type	speed	*/
	NULL,		NULL,		NULL,
	/* Buf_mode	wp		blk_desc_len	*/
	0x01,		NULL,		sizeof(SEQ_MODE_DESC)
	},
	{ /* Mode descriptor */
	/* Density	num_blks2	num_blks1	*/
	NULL,		NULL,		NULL,
	/* num_blks0	reserved	blk_len2	*/
	NULL,		0,		NULL,
	/* blk_len1	blk_len0			*/
	NULL,		NULL
	},
	{
	/* Page data since scsi 1 vendor unique	*/
	/* byte0	byte1		byte2		*/
	0x06,		0x00,		0x80,		
	/* byte3	byte4				*/
	0x80,		0x07
	}
	
},
tkz09_page20 = {

	{ /* Parameter header */
	/* mode_len	medium type	speed	*/
	NULL,		NULL,		NULL,
	/* Buf_mode	wp		blk_desc_len	*/
	0x01,		NULL,		sizeof(SEQ_MODE_DESC)
	},
	{ /* Mode descriptor */
	/* Density	num_blks2	num_blks1	*/
	NULL,		NULL,		NULL,
	/* num_blks0	reserved	blk_len2	*/
	NULL,				NULL,
	/* blk_len1	blk_len0			*/
	NULL,		NULL
	},
	{
	/* Page data since scsi 1 vendor unique	*/
	/* byte0	byte1		byte2		*/
	0x20,		0x04,		0x06,		
	/* byte3	byte4		byte5		*/
	0x00,		0x80,		0x07
	}
	
},
tzk10_page2 = {

	{ /* Parameter header */
	/* mode_len	medium type	speed	*/
	NULL,		NULL,		NULL,
	/* Buf_mode	wp		blk_desc_len	*/
	0x01,		NULL,		sizeof(SEQ_MODE_DESC)
	},
	{ /* Mode descriptor */
	/* Density	num_blks2	num_blks1	*/
	NULL,		NULL,		NULL,
	/* num_blks0	reserved	blk_len2	*/
	NULL,		0,		NULL,
	/* blk_len1	blk_len0			*/
	NULL,		NULL
	},
	{
	/* Page data for page 0x2 			*/
	/* PAGE header */
	/* byte0	byte1				*/
	   0x02,	0x0e,
	/* byte2    byte3    byte4    byte5    byte6	*/
	   32,      32,      NULL,    NULL,    NULL,
	/* byte7    byte8    byte9    byte10   byte11	*/
	   NULL,    NULL,    NULL,    NULL,    NULL,
	/* byte12   byte13   byte14   byte15   		*/
	   NULL,    NULL,    NULL,    NULL
	}
},

tzk10_page10 = {

	{ /* Parameter header */
	/* mode_len	medium type	speed	*/
	NULL,		NULL,		NULL,
	/* Buf_mode	wp		blk_desc_len	*/
	0x01,		NULL,		sizeof(SEQ_MODE_DESC)
	},
	{ /* Mode descriptor */
	/* Density	num_blks2	num_blks1	*/
	NULL,		NULL,		NULL,
	/* num_blks0	reserved	blk_len2	*/
	NULL,		0,		NULL,
	/* blk_len1	blk_len0			*/
	NULL,		NULL
	},
	{
	/* Page data for page 0x10 			*/
	/* PAGE header */
	/* byte0	byte1				*/
	   0x10,	0x0e,
	/* byte2    byte3    byte4    byte5    byte6	*/
	   0x00,    0x00,    40,      40,      NULL,
	/* byte7    byte8    byte9    byte10   byte11	*/
	   NULL,    0xe0,    NULL,    0x38,    NULL,
	/* byte12   byte13   byte14   byte15   		*/
	   NULL,    NULL,    NULL,    NULL
	}
};


/* 
 * Mode select tables
 */
MODESEL_TBL
cdrom_mode_sel = {
	{
	{ /* MODE PAGE ENTRY 1	*/
	/* Page number		The data pointer		*/
	00,			(u_char *)&cdrom_page0,
	/* Data len		SCSI2??				*/
	12,			NULL
	},
	{ /* MODE PAGE ENTRY 2	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	}
	}
};
MODESEL_TBL
tzk08_mod = {
	{
	{ /* MODE PAGE ENTRY 1	*/
	/* Page number		The data pointer		*/
	00,			(u_char *)&tzk08_page0,
	/* Data len		SCSI2??				*/
	   17,			NULL
	},
	{ /* MODE PAGE ENTRY 2	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 3	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 4	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 5	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 6	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 7	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 8	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	}
	}
},
tkz09_mod = {
	{
	{ /* MODE PAGE ENTRY 1	*/
	/* Page number		The data pointer		*/
	00,			(u_char *)&tkz09_page20,
	/* Data len		SCSI2??				*/
	   18,			0x2
	},
	{ /* MODE PAGE ENTRY 2	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 3	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 4	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 5	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 6	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 7	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 8	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	}
	}
},
tzk10_mod = {
	{
	{ /* MODE PAGE ENTRY 1	*/
	/* Page number		The data pointer		*/
	0x02,			(u_char *)&tzk10_page2,
	/* Data len		SCSI2??				*/
	   28,			0x2
	},
	{ /* MODE PAGE ENTRY 2	*/
	/* Page number		The data pointer		*/
	0x10,			(u_char *)&tzk10_page10,
	/* Data len		SCSI2??				*/
	   28,			0x2
	},
	{ /* MODE PAGE ENTRY 3	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 4	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 5	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 6	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 7	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	},
	{ /* MODE PAGE ENTRY 8	*/
	/* Page number		The data pointer		*/
	NULL,			(u_char *)NULL,
	/* Data len		SCSI2??				*/
	NULL,			NULL
	}
	}
};







/*
 * Default partition tables to be used if the
 * disk does not contain a parition table.
 */
struct size {
	daddr_t	nblocks;
	int	blkoffs;
} ccmn_rz22_sizes[8] = {
	40960,	0,		/* A=blk 0 thru 40959 */
	-1,	40960,		/* B=blk 40960 thru end (102431)*/
	-1,	0,		/* C=blk 0 thru end (102431) */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
}, ccmn_rz23_sizes[8] = {
	40960,	0,		/* A=blk 0 thru 40959 */
	58498,	40960,		/* B=blk 40960 thru 99457 */
	-1,	0,		/* C=blk 0 thru end (204863) */
	35135,	99458,		/* D=blk 99458 thru 134592 */
	35135,	134593,		/* E=blk 134593 thru 169727 */
	-1,	169728,		/* F=blk 169728 thru end (204863) */
	-1,	99458,		/* G=blk 99458 thru end (204863) */
	-1,	134593,		/* H=blk 134593 thru end (204863) */
}, ccmn_rz23l_sizes[8] = {
	40960,	0,		/* A=blk 0 thru 40959 */
	58498,	40960,		/* B=blk 40960 thru 99457 */
	-1,	0,		/* C=blk 0 thru end (237588) */
	35135,	99458,		/* D=blk 99458 thru 134592 */
	35135,	134593,		/* E=blk 134593 thru 169727 */
	-1,	169728,		/* F=blk 169728 thru end (237588) */
	-1,	99458,		/* G=blk 99458 thru end (237588) */
	-1,	134593,		/* H=blk 134593 thru end (237587) */
}, ccmn_rz24_sizes[8] = {
	131072,  0,             /* A=blk 0 thru 131072 */
	262144, 131072,         /* B=blk 131072 thru 393215 */
	-1,	0,		/* C=blk 0 thru end (409791) */
	0,	0,         	/* D=zero for default */
	0,	0,         	/* E=zero for default */
	-1,	131072,         /* F=blk 131072 thru end (409791) */
	-1, 	393216,         /* G=blk 393216 thru end (409791) */
	0,	0,              /* H=zero for default */
}, ccmn_rz24l_sizes[8] = {
	40960,	0,		/* A=blk 0 thru 40959 */
	122880, 40960,          /* B=blk 40960 thru 163839 */
	-1,	0,		/* C=blk 0 thru end (479349) */
	105170,	163840,         /* D=blk 163840 thru 269009 */
	105170,	269010,         /* E=blk 269010 thru 374179 */
	-1,	374180,         /* F=blk 374180 thru end (479349) */
	-1, 	163840,         /* G=blk 163840 thru end (479349) */
	0,	0,              /* H=zero for default */
}, ccmn_rz25_sizes[8] = {
        131072, 0,              /* A=blk 0 thru 131071 */
        262144, 131072,         /* B=blk 131072 thru 393215 */
        -1,     0,              /* C=blk 0 thru end (832526) */
        146437, 393216,         /* D=blk 393216 thru 539652 */
        146437, 539653,         /* E=blk 539653 thru 686089 */
        -1,     686090,         /* F=blk 686090 thru end (832526) */
        -1,     393216,         /* G=blk 393216 thru end (832526) */
        0,      0,              /* H=empty */
}, ccmn_rz25l_sizes[8] = {
        131072, 0,              /* A=blk 0 thru 131071 */
        262144, 131072,         /* B=blk 131072 thru 393215 */
        -1,     0,              /* C=blk 0 thru end (1046205) */
        217663, 393216,         /* D=blk 393216 thru 610878 */
        217663, 610879,         /* E=blk 610879 thru 828541 */
        -1,     828542,         /* F=blk 828542 thru end (1046205) */
        -1,     393216,         /* G=blk 393216 thru end (1046205) */
        0,      0,              /* H=empty */
}, ccmn_rz35_sizes[8] = {
	131072,  0,             /* A=blk 0 thru 131071 (64 MB) */
	262144, 131072,         /* B=blk 131072 thru 393215 (128 MB) */
	-1,	0,		/* C=blk 0 thru 1664627 (832 MB) */
	423804,	393216,         /* D=blk 393216 thru 817019 (211 MB) */
	423804,	817020,         /* E=blk 817020 thru 1240823 (211 MB) */
	-1, 	1240824,        /* F=blk 1240824 thru 1662627 (211 MB) */
	819200, 393216,         /* G=blk 3932160 thru 1212415 (400 MB) */
	-1,     1212416,       	/* H=blk 1141140 thru 1664627 (261 MB) */
}, ccmn_rz26_sizes[8] = {
	131072, 0,              /* A=blk 0 thru 131071 */
	262144, 131072,         /* B=blk 131072 thru 393215 */
	-1,     0,              /* C=blk 0 thru end (2050859) */
	552548, 393216,         /* D=blk 393216 thru 945763 */
	552548, 945764,         /* E=blk 945764 thru 1498311 */
	-1,     1498312,        /* F=blk 1498312 thru end (2050859) */
	819200, 393216,         /* G=blk 393216 thru 1212415 */
	-1,     1212416,        /* H=blk 1212416 thru end (2050859) */
}, ccmn_rz27_sizes[8] = {
	131072, 0,              /* A=blk 0 thru 131071 */
	262144, 131072,         /* B=blk 131072 thru 393215 */
	-1,     0,              /* C=blk 0 thru end (3125407) */
	909312, 393216,         /* D=blk 393216 thru 1302527 */
	909312, 1302528,        /* E=blk 1302528 thru 2211839 */
	-1,     2211840,        /* F=blk 2211840 thru end (3125407) */
	1366016, 393216,        /* G=blk 393216 thru 1759231 */
	-1,     1759232,        /* H=blk 1759232 thru end (3125407) */
}, ccmn_rz28_sizes[8] = {
	131072, 0,              /* A=blk 0 thru 131071 */
	401408, 131072,         /* B=blk 131072 thru 532479 */
	-1,     0,              /* C=blk 0 thru end (4110480) */
	1191936, 532480,        /* D=blk 532480 thru 1724415 */
	1191936, 1724416,       /* E=blk 1724416 thru 2916351 */
	-1,     2916352,        /* F=blk 2916352 thru end (4110480) */
	1787904, 532480,        /* G=blk 532480 thru 2320383 */
	-1,     2320384,        /* H=blk 2320384 thru end (4110480) */
}, ccmn_rz74_sizes[8] = {
	131072, 0,              /* A=blk 0 thru 131071 */
	393216, 131072,         /* B=blk 131072 thru 524287 */
	-1,     0,              /* C=blk 0 thru end (6976374) */
	2150400, 524288,        /* D=blk 524288 thru 2674687 */
	2150400, 2674688,       /* E=blk 2674688 thru 4825087 */
	-1,     4825088,        /* F=blk 4825088 thru end (6976374) */
	3225600, 524288,        /* G=blk 524288 thru 3749887 */
	-1,     3749888,        /* H=blk 3749888 thru end (6976374) */
}, ccmn_rz55_sizes[8] = {
	131072,  0,             /* A=blk 0 thru 131071         (64 MB) */
	262144, 131072,         /* B=blk 131072 thru 393215   (128 MB) */
	-1,     0,              /* C=blk 0 thru end (649039)  (330 MB) */
	0,      0,              /* D-zero for default                  */
	0,      0,              /* E=zero for default                  */
	0,      0,              /* F=zero for default                  */
	-1,     393216,         /* G=blk 393216 thru 660959   (133 MB) */
	0,      0,              /* H=zero for default                  */
}, ccmn_rz56_sizes[8] = {
	131072, 0,              /* A=blk 0 thru 131071        ( 64 MB) */
	262144, 131072,         /* B=blk 131072 thru 393215   (128 MB) */
	-1,     0,              /* C=blk 0 thru end (1299173) (634 MB) */
	301986, 393216,         /* D=blk 393216 thru 702783   (147 MB) */
	301986, 695202,         /* E=blk 456370 thru 1012351  (147 MB) */
	-1,     997188,         /* F=blk 748900 thru end      (147 MB) */
	819200, 393216,         /* G=blk 393216 thru 1212415  (400 MB) */
	-1,     1212416,        /* H=blk 1212416 thru end     ( 42 MB) */
}, ccmn_rz57_sizes[8] = {
	131072,  0,             /* A=blk 0 thru 131071        ( 64 MB) */
	262144, 131072,         /* B=blk 131072 thru 393215   (128 MB) */
	-1,     0,              /* C=blk 0 thru end (1954049) (954 MB) */
	520278, 393216,         /* D=blk 393216 thru 913493   (254 MB) */
	520278, 913494,         /* E=blk 913494 thru 1433771  (254 MB) */
	-1,     1433772,        /* F=blk 1433772 thru end     (254 MB) */
	819200, 393216,         /* G=blk 393216 thru 1212415  (400 MB) */
	-1,     1212416,        /* H=blk 1212416 thru end     (362 MB) */
}, ccmn_rz58_sizes[8] = {
	131072,  0,             /* A=blk 0 thru 131071 (64 Mb) */
	262144, 131072,         /* B=blk 131072 thru 262144 (128 Mb) */
	-1,	0,		/* C=blk 0 thru 2698060 (1317.4Mb) */
	768281,	393216,         /* D=blk 393216 thru 1161496 (384Mb) */
 	768282,	1161497,        /* E=blk 1161497 thru 1853441 (384Mb) */
 	-1, 	1929779,        /* F=blk 1929779 thru 2698060 (384Mb) */
	819200, 393216,         /* G=blk 393216 thru 1212415 (400.0Mb) */
	-1,	1212416,       	/* H=blk 1212416 thru 2698060 (742Mb) */
}, ccmn_rz73_sizes[8] = {
	131072,  0,             /* A=blk 0 thru 131071 (64Mb) */
	262144,  131072,        /* B=blk 131072 thru 393215 (128Mb) */
	-1,	0,		/* C=blk 0 thru 3907910 (1907.43Mb) */
	1171565, 393216,        /* D=blk 393216 thru 1564780 (585Mb) */
 	1171565, 1564781,       /* E=blk 1564781 thru 2736345 (585Mb) */
 	-1,      2736346,       /* F=blk 2719584 thru 3907910 (585Mb) */
	819200,  393216,        /* G=blk 3932160 thru 1212415 (400Mb) */
	-1,      1212416,      	/* H=blk 1212416 thru 3907910 (1347Mb) */
}, ccmn_ez51_sizes[8] = {
	40960, 0,               /* A=blk 0 thru 40959 */
	40960, 40960,           /* B=blk 40960 thru 81919 */
	-1,     0,              /* C=blk 0 thru end (208799) */
	40960, 81920,           /* D=blk 81920 thru 122879 */
	40960, 122880,          /* E=blk 122880 thru 163839 */
	-1,     163840,         /* F=blk 163840 thru end (208799) */
	61440, 81920,           /* G=blk 81920 thru 143359 */
	-1,     143360,         /* H=blk 143360 thru end (208799) */
}, ccmn_ez54_sizes[8] = {
	131072, 0,              /* A=blk 0 thru 131071 */
	262144, 131072,         /* B=blk 131072 thru 393215 */
	-1,     0,              /* C=blk 0 thru end (835299) */
	147361, 393216,         /* D=blk 393216 thru 540576 */
	147361, 540577,         /* E=blk 540577 thru 687937 */
	-1,     687938,         /* F=blk 687938 thru end (835299) */
	-1,	393216,         /* G=blk 393216 thru end (835299) */
        0,      0,              /* H=empty */
}, ccmn_ez58_sizes[8] = {
        131072, 0,              /* A=blk 0 thru 131071 */
        262144, 131072,         /* B=blk 131072 thru 393215 */
        -1,     0,              /* C=blk 0 thru end (1670599) */
        425794, 393216,         /* D=blk 393216 thru 819009 */
        425794, 819010,         /* E=blk 819010 thru 1244803 */
        -1,     1244804,        /* F=blk 1244804 thru end (1670599) */
        -1,     393216,         /* G=blk 393216 thru end (1670599) */
        0,      0,              /* H=empty */
}, ccmn_hsx00_sizes[8] = {
	131072,  0,             /* A=blk 0      thru 131071   (64Mb)     */
	262144,  131072,        /* B=blk 131072 thru 393215   (128Mb)    */
	-1,	0,		/* C=blk 0      thru ??????   (???Mb/Gb) */
	 0,     0,              /* D=blk      - NOT DEFINED -            */
 	 0,     0,              /* E=blk      - NOT DEFINED -            */
 	 0,     0,              /* F=blk      - NOT DEFINED -            */
	-1,      393216,        /* G=blk 393216 thru ??????   (???Mb/Gb) */
	 0,     0,      	/* H=blk      - NOT DEFINED -            */
}, ccmn_hsx01_sizes[8] = {
	131072,  0,             /* A=blk 0      thru 131071   (64Mb)     */
	262144,  131072,        /* B=blk 131072 thru 393215   (128Mb)    */
	-1,	0,		/* C=blk 0      thru ??????   (???Mb/Gb) */
	 0,     0,              /* D=blk      - NOT DEFINED -            */
 	 0,     0,              /* E=blk      - NOT DEFINED -            */
 	 0,     0,              /* F=blk      - NOT DEFINED -            */
	-1,      393216,        /* G=blk 393216 thru ??????   (???Mb/Gb) */
	 0,     0,      	/* H=blk      - NOT DEFINED -            */
}, ccmn_hsz10_sizes[8] = {
	131072,  0,             /* A=blk 0      thru 131071   (64Mb)     */
	262144,  131072,        /* B=blk 131072 thru 393215   (128Mb)    */
	-1,	0,		/* C=blk 0      thru ??????   (???Mb/Gb) */
	 0,     0,              /* D=blk      - NOT DEFINED -            */
 	 0,     0,              /* E=blk      - NOT DEFINED -            */
 	 0,     0,              /* F=blk      - NOT DEFINED -            */
	-1,      393216,        /* G=blk 393216 thru ??????   (???Mb/Gb) */
	 0,     0,      	/* H=blk      - NOT DEFINED -            */
}, ccmn_hsz15_sizes[8] = {
	131072,  0,             /* A=blk 0      thru 131071   (64Mb)     */
	262144,  131072,        /* B=blk 131072 thru 393215   (128Mb)    */
	-1,	0,		/* C=blk 0      thru ??????   (???Mb/Gb) */
	 0,     0,              /* D=blk      - NOT DEFINED -            */
 	 0,     0,              /* E=blk      - NOT DEFINED -            */
 	 0,     0,              /* F=blk      - NOT DEFINED -            */
	-1,      393216,        /* G=blk 393216 thru ??????   (???Mb/Gb) */
	 0,     0,      	/* H=blk      - NOT DEFINED -            */
}, ccmn_hsz20_sizes[8] = {
	131072,  0,             /* A=blk 0      thru 131071   (64Mb)     */
	262144,  131072,        /* B=blk 131072 thru 393215   (128Mb)    */
	-1,	0,		/* C=blk 0      thru ??????   (???Mb/Gb) */
	 0,     0,              /* D=blk      - NOT DEFINED -            */
 	 0,     0,              /* E=blk      - NOT DEFINED -            */
 	 0,     0,              /* F=blk      - NOT DEFINED -            */
	-1,      393216,        /* G=blk 393216 thru ??????   (???Mb/Gb) */
	 0,     0,      	/* H=blk      - NOT DEFINED -            */
}, ccmn_hsz40_sizes[8] = {
	131072,  0,             /* A=blk 0      thru 131071   (64Mb)     */
	262144,  131072,        /* B=blk 131072 thru 393215   (128Mb)    */
	-1,	0,		/* C=blk 0      thru ??????   (???Mb/Gb) */
	 0,     0,              /* D=blk      - NOT DEFINED -            */
 	 0,     0,              /* E=blk      - NOT DEFINED -            */
 	 0,     0,              /* F=blk      - NOT DEFINED -            */
	-1,      393216,        /* G=blk 393216 thru ??????   (???Mb/Gb) */
	 0,     0,      	/* H=blk      - NOT DEFINED -            */
}, ccmn_rwz01_sizes[8] = {
	40960,	0,		/* A=blk 0 thru 40959 */
	131072, 32768,          /* B=blk 32768 thru 163839 */
	-1, 	0,	        /* C=blk 0 thru end (1299173) */
	0, 	0,         	/* D=blk 163840 thru 456369 */
	0, 	0,       	/* E=blk 456370 thru 748899 */
	0,	0,         	/* F=blk 748900 thru end (1299173) */
	413159,	163840,         /* G=blk 163840 thru 731505 */
	0, 	0,       	/* H=blk 731506 thru end (1299173) */
}, ccmn_rzxx_sizes[8] = {
	131072,	0,		/* A=blk 0 thru 131071        (64Mb)     */
	262144, 131072,         /* B=blk 131072 thru 393215   (128Mb)    */
	-1,	0,		/* C=blk 0      thru ??????   (???Mb/Gb) */
	0,	0,
	0,	0,
	0,	0,
	-1, 	393216,         /* G=blk 393216 thru end (?)             */
	0,	0,
}, ccmn_rrd40_sizes[8] = {
	-1,	0,              /* A=blk 0 thru end (CDROM size variable) */
	0,	0,
	-1,	0,		/* C=blk 0 thru end (CDROM size variable) */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
}, ccmn_rx23_sizes[8] = {
	-1,	0,		/* A=blk 0 thru end (2879 for hi density) */
	0,	0,
	-1,	0,		/* C=blk 0 thru end (2879 for hi density) */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
}, ccmn_rx33_sizes[8] = {
	-1,	0,		/* A=blk 0 thru end (2399 for hi density) */
	0,	0,
	-1,	0,		/* C=blk 0 thru end (2399 for hi density) */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
}, ccmn_rx26_sizes[8] = {
	-1,	0,		/* A=blk 0 thru end (5759 for extra density) */
	0,	0,
	-1,	0,		/* C=blk 0 thru end (5759 for extra density) */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
}, ccmn_null_sizes[8] = {		/* Dummy for tapes */
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
	0,	0,
};


/*
 * This is the unknown device table. If a device is not in the
 * the cam_devdesc_tab[] then we default it to this table based
 * on device type found in the inquiry data.
 * Do Not change this table or the order of the entries.
 */
DEV_DESC dev_desc_unknown[] = {

/* UNKNOWN DISK */
{"UNKNOWN", 7, DEV_RZxx, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
  (struct pt_info *)ccmn_rzxx_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  (SZ_BBR | SZ_NO_TAG | SZ_REORDER),
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},
/* UNKNOWN TAPE */
{"UNKNOWN", 7, DEV_TZxx, (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_9TRK_CLASS, 
  (struct pt_info *)ccmn_null_sizes, NULL, (DEC_MAX_REC - 1), &def_dens, 
  NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* UNKNOWN Printer */
{"\0", 0, "\0", 0, 0, 0, 0, 0, 	
 0, 0, 0, 0, 0, 0, 0, 0},	

/* UNKNOWN Processor */
{"\0", 0, "\0", 0, 0, 0, 0, 0, 	
 0, 0, 0, 0, 0, 0, 0, 0},	

/* UNKNOWN WORM */
{"\0", 0, "\0", 0, 0, 0, 0, 0, 	
 0, 0, 0, 0, 0, 0, 0, 0},	

/* UNKOWN cd-rom */
{"UNKNOWN", 7, DEV_RZxx, (ALL_DTYPE_RODIRECT << DTYPE_SHFT) , 
  (struct pt_info *)ccmn_rrd40_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  &cdrom_mode_sel, SZ_NO_FLAGS,
  SZ_PREV_ALLOW, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},
/* UNKNOWN Scanner */
{"\0", 0, "\0", 0, 0, 0, 0, 0, 	
 0, 0, 0, 0, 0, 0, 0, 0},	

/* UNKNOWN Optical */
{"\0", 0, "\0", 0, 0, 0, 0, 0, 	
 0, 0, 0, 0, 0, 0, 0, 0},	

/* UNKNOWN Medium Changer */
{"\0", 0, "\0", 0, 0, 0, 0, 0, 	
 0, 0, 0, 0, 0, 0, 0, 0},	

/* UNKNOWN Communication*/
{"\0", 0, "\0", 0, 0, 0, 0, 0, 	
 0, 0, 0, 0, 0, 0, 0, 0},	

};

/*
 * Number of valid unknown descriptors (size of table)
 */
I32 num_unknown_dev_desc = sizeof(dev_desc_unknown)/sizeof(DEV_DESC);

/*
 * CAM SCSI device descriptor information table.
 */
			/* DO NOT CHANGE MAX_DEVICE_DESC */
DEV_DESC cam_devdesc_tab[MAX_DEVICE_DESC] = {

/* DISKS: */

/* RZ55 */
{"DEC     RZ55", 12, DEV_RZ55, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
  (struct pt_info *)ccmn_rz55_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, (SZ_BBR | SZ_REORDER), 
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL ,
  36, 64
},

/* RZ56 */
{"DEC     RZ56", 12, DEV_RZ56, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
  (struct pt_info *)ccmn_rz56_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, (SZ_BBR | SZ_REORDER), NO_OPT_CMDS,
  SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ57 */
{"DEC     RZ57", 12, DEV_RZ57, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
  (struct pt_info *)ccmn_rz57_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, (SZ_BBR | SZ_REORDER), 
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ58 */
{"DEC     RZ58", 12, DEV_RZ58, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
  (struct pt_info *)ccmn_rz58_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, (SZ_BBR | SZ_REORDER | SZ_NO_TAG), 
  NO_OPT_CMDS, 60, 32, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ73 */
{"DEC     RZ73", 12, DEV_RZ73, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
  (struct pt_info *)ccmn_rz73_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, SZ_BBR , 
  NO_OPT_CMDS, SZ_READY_DEF, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64 
},

/* HSZ20 */
{"DEC     HSZ20", 13, DEV_HSZ20, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK
  | SZ_RAID , 
  (struct pt_info *)ccmn_hsx00_sizes, NULL, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, SZ_DISPERSE_QUE, 
  (SZ_DYNAMIC_GEOM), 90, 74, DD_REQSNS_VAL | 
  DD_INQ_VAL, 36, 160 
},

/* HSZ40 */
{"DEC     HSZ40", 13, DEV_HSZ40, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK
  | SZ_RAID , 
  (struct pt_info *)ccmn_hsx01_sizes, NULL, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, SZ_DISPERSE_QUE, 
  (SZ_DYNAMIC_GEOM), 90, 74, DD_REQSNS_VAL | 
  DD_INQ_VAL, 
  36, 160 
},

/* HSZ10 */
{"DEC     HSZ10", 13, DEV_HSZ10, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | 
( SZ_HARD_DISK | SZ_RAID ), 
  (struct pt_info *)ccmn_hsz10_sizes, 0x84032b, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, SZ_DISPERSE_QUE, 
  SZ_WR_VERI_PAR | SZ_DYNAMIC_GEOM, 90, 74, 
  DD_REQSNS_VAL | DD_INQ_VAL,
  36, 160 
},


/* HSZ15 */
{"DEC     HSZ15", 13, DEV_HSZ15, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | 
( SZ_HARD_DISK | SZ_RAID ), 
  (struct pt_info *)ccmn_hsz15_sizes, 0x82032b, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, SZ_DISPERSE_QUE, 
  SZ_WR_VERI_PAR | SZ_DYNAMIC_GEOM, 90, 74, 
  DD_REQSNS_VAL | DD_INQ_VAL,
  36, 160 
},

/* RWZ01 */
{"DEC     RWZ01", 13, DEV_RWZ01, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
  (struct pt_info *)ccmn_rwz01_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, (SZ_BBR | SZ_REORDER), 
  SZ_PREV_ALLOW, SZ_READY_DEF, 8, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ23L */
/*
 * Note -- This entry must appear before RZ23, or else the RZ23L will
 *         match on the RZ23.
 */
{"DEC     RZ23L", 13, DEV_RZ23L, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
  (struct pt_info *)ccmn_rz23l_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, (SZ_NOSYNC | SZ_BBR | SZ_REORDER), 
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ23 */
{"DEC     RZ23", 12, DEV_RZ23, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
  (struct pt_info *)ccmn_rz23_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, (SZ_BBR | SZ_REORDER), 
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ22 */
{"DEC     RZ22", 12, DEV_RZ22, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
  (struct pt_info *)ccmn_rz22_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, (SZ_BBR | SZ_REORDER), 
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RX23 */
{"DEC     RX23", 12, DEV_RX23, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_FLOP_3, 
  (struct pt_info *)ccmn_rx23_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, SZ_NO_FLAGS, 
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ24L */
/*
 * Note -- This entry must appear before RZ24, or else the RZ24L will
 *         match on the RZ24.
 */
{"DEC     RZ24L", 13, DEV_RZ24L,(ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_rz24l_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, (SZ_BBR | SZ_REORDER), 
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},
/* RZ24 */
{"DEC     RZ24", 12, DEV_RZ24, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
  (struct pt_info *)ccmn_rz24_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, (SZ_BBR | SZ_REORDER), 
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ25L */
/*
 * Note -- This entry must appear before RZ25, or else the RZ25L will
 *         match on the RZ25.
 */
{"DEC     RZ25L", 13, DEV_RZ25L, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_rz25l_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, SZ_BBR ,
  NO_OPT_CMDS, SZ_READY_DEF, 64, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ25 */
{"DEC     RZ25", 12, DEV_RZ25, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
  (struct pt_info *)ccmn_rz25_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, (SZ_BBR | SZ_REORDER), 
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ26B */
{"DEC     RZ26B", 13, DEV_RZ26B, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_rz26_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  SZ_BBR ,
  NO_OPT_CMDS, SZ_READY_DEF, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},


/* RZ26L */
{"DEC     RZ26L", 13, DEV_RZ26L, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_rz26_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  SZ_BBR ,
  NO_OPT_CMDS, SZ_READY_DEF, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},


/* RZ26M */
{"DEC     RZ26M", 13, DEV_RZ26M, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_rz26_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  SZ_BBR,
  NO_OPT_CMDS, SZ_READY_DEF, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ26 */
{"DEC     RZ26", 12, DEV_RZ26, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_rz26_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  SZ_BBR,
  NO_OPT_CMDS, SZ_READY_DEF, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ27 */
{"DEC     RZ27", 12, DEV_RZ27, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_rz27_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  SZ_BBR ,
  NO_OPT_CMDS, SZ_READY_DEF, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ28B */
{"DEC     RZ28B", 13, DEV_RZ28B, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_rz28_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  SZ_BBR,
  NO_OPT_CMDS, SZ_READY_DEF, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ28L */
{"DEC     RZ28L", 13, DEV_RZ28L, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_rz28_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  (SZ_BBR),
  NO_OPT_CMDS, SZ_READY_DEF, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ28M */
{"DEC     RZ28M", 13, DEV_RZ28M, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_rz28_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  (SZ_BBR | SZ_REORDER),
  NO_OPT_CMDS, SZ_READY_DEF, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ28 */
{"DEC     RZ28", 12, DEV_RZ28, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_rz28_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  SZ_BBR,
  NO_OPT_CMDS, SZ_READY_DEF, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},
/* RZ74 */
{"DEC     RZ74", 12, DEV_RZ74, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_rz74_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  SZ_BBR,
  NO_OPT_CMDS, SZ_READY_DEF, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RZ35 */
{"DEC     RZ35", 12, "RZ35", (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_rz35_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  (SZ_BBR | SZ_NO_TAG | SZ_REORDER),
  NO_OPT_CMDS, SZ_READY_DEF, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* The IBM classification */
{"IBM     0661467", 15, DEV_RZ25, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK, 
  (struct pt_info *)ccmn_rz25_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, SZ_BBR , 
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RX26 */ 
{"DEC     RX26", 12, DEV_RX26, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_FLOP_3,
  (struct pt_info *)ccmn_rx26_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, 
  NO_MODE_TAB, SZ_NO_FLAGS, 
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL, 
  36, 64
},

/* RX33 */
{"DEC     RX33", 12, DEV_RX33, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_FLOP_5,
  (struct pt_info *)ccmn_rx33_sizes, 512, DEC_MAX_REC, NO_DENS_TAB,
  NO_MODE_TAB, SZ_NO_FLAGS, 
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL, 
  36, 64
},

/* SOLID STATE DISK DRIVES: */

/* EZ51 */
{"DEC     EZ51", 12, DEV_EZ51, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_ez51_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  SZ_BBR ,
  NO_OPT_CMDS, 120, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* EZ54 */
{"DEC     EZ54", 12, DEV_EZ54, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_ez54_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  SZ_BBR,
  NO_OPT_CMDS, 120, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* EZ58 */
{"DEC     EZ58", 12, DEV_EZ58, (ALL_DTYPE_DIRECT << DTYPE_SHFT) | SZ_HARD_DISK,
  (struct pt_info *)ccmn_ez58_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  (SZ_BBR | SZ_NO_TAG | SZ_REORDER),
  NO_OPT_CMDS, 120, 74, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* CDROMS: */

/* RRD40 */
{"DEC     RRD40", 13, DEV_RRD40, (ALL_DTYPE_RODIRECT << DTYPE_SHFT) , 
  (struct pt_info *)ccmn_rrd40_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  (SZ_NOSYNC | SZ_REORDER),
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RRD42 */
{"DEC     RRD42", 13, DEV_RRD42, (ALL_DTYPE_RODIRECT << DTYPE_SHFT) , 
  (struct pt_info *)ccmn_rrd40_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  (SZ_NOSYNC | SZ_REORDER),
  SZ_PREV_ALLOW, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},
/* RRD43 */
{"DEC     RRD43", 13, DEV_RRD43, (ALL_DTYPE_RODIRECT << DTYPE_SHFT) , 
  (struct pt_info *)ccmn_rrd40_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  SZ_REORDER, SZ_PREV_ALLOW, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* RRD44 */
{"DEC     RRD44", 13, DEV_RRD44, (ALL_DTYPE_RODIRECT << DTYPE_SHFT) , 
  (struct pt_info *)ccmn_rrd40_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, NO_MODE_TAB,
  SZ_REORDER, SZ_PREV_ALLOW, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},
/* Sony cd-rom */
{"SONY    CD-ROM", 14, DEV_RRD42, (ALL_DTYPE_RODIRECT << DTYPE_SHFT) , 
  (struct pt_info *)ccmn_rrd40_sizes, 512, DEC_MAX_REC, NO_DENS_TAB, 
  &cdrom_mode_sel, SZ_NO_FLAGS,
  SZ_PREV_ALLOW, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TAPES: */
/* TZK50 */
{ "DEC     TZK50", 13, DEV_TK50, (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_TK_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 64512, &tk50_dens, NO_MODE_TAB,
  SZ_NOSYNC,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TZ30 */
{ "DEC     TZ30", 12, DEV_TZ30, (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_TK_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 64512, &tk30_dens, NO_MODE_TAB,
  SZ_NOSYNC,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TLZ04 - RDAT drive */
{"DEC     TLZ04", 13, DEV_TLZ04, (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_RDAT_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, (DEC_MAX_REC - 1), 
  &tlz04_dens, NO_MODE_TAB, SZ_NOSYNC,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TLZ06 - RDAT drive */
{"DEC     TLZ06", 13, DEV_TLZ06, (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_RDAT_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, (DEC_MAX_REC - 1), 
  &tlz06_dens, NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TLZ6 - Loader RDAT drive */
{"DEC     TLZ6", 12, DEV_TLZ6, (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_RDAT_CLASS | SZ_LOADER,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, (DEC_MAX_REC - 1), 
  &tlz06_dens, NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TLZ07 - RDAT drive */
{"DEC     TLZ07", 13, DEV_TLZ07, (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_RDAT_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, (DEC_MAX_REC - 1), 
  &tlz07_dens, NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TLZ7 - Loader RDAT drive */
{"DEC     TLZ7", 12, DEV_TLZ7, (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_RDAT_CLASS | SZ_LOADER,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, (DEC_MAX_REC - 1), 
  &tlz07_dens, NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TZ05 - alias TZX0 */ 
{"NCR H621", 8, DEV_TZ05, (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_9TRK_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 65536, 
  &tz05_dens, NO_MODE_TAB, SZ_NOSYNC,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TZ07 */
{"DEC     TSZ07", 13, DEV_TZ07, (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_9TRK_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 65536, 
  &tz07_dens, NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* The CIPHER look alike TZ07 - Dual density 100ips 9track */
{"CIPHER  M995", 12, DEV_TZ07, 
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_9TRK_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 65536, 
  &tz07_dens, NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},


/* TZK10 - DEC's QIC tape drive */ 
{"DEC     TZK10", 13, DEV_TZK10, 
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_QIC_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, (DEC_MAX_REC - 1), 
  &tzk10_dens, NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TZK11 - DEC's  2 GIG QIC tape drive */ 
{"DEC     TZK11", 13, DEV_TZK11, 
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_QIC_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, (DEC_MAX_REC - 1), 
  &tzk11_dens, NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TKZ60L LOADER */
{ "DEC     TKZ60L", 14, DEV_TKZ60L, 
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_3480_CLASS |SZ_LOADER,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 65536, &tkz60_dens,
  NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TKZ60C COMPRESSION */
{ "DEC     TKZ60C", 14, DEV_TKZ60C, 
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_3480_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 65536, &tkz60c_dens,
  NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TKZ60CL LOADER with COMPRESSION*/
{ "DEC     TKZ60CL", 15, DEV_TKZ60CL, 
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_3480_CLASS |SZ_LOADER,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 65536, &tkz60c_dens,
  NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},


/* TKZ60 */
{ "DEC     TKZ60", 13, DEV_TKZ60, 
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_3480_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 65536, &tkz60_dens,
  NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TZ857 */
{ "DEC     TZ857", 13, DEV_TZ857, 
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_TK_CLASS |SZ_LOADER,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 262144, &tz85_dens,
  NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TZ85 */
{ "DEC     TZ85", 12, DEV_TZ85, 
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_TK_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 262144, &tz85_dens,
  NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TZ867 */
{ "DEC     TZ867", 13, DEV_TZ867,
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_TK_CLASS | SZ_LOADER,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 262144, &tz86_dens,
  NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TZ86 */
{ "DEC     TZ86", 12, DEV_TZ86,
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_TK_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 262144, &tz86_dens,
  NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* TZ877 */
{ "DEC     TZ877", 13, DEV_TZ877,
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_TK_CLASS | SZ_LOADER,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, (DEC_MAX_REC - 1), &tz87_dens,
  NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},
                            
/* TZ87 */
{ "DEC     TZ87", 12, DEV_TZ87,
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_TK_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, (DEC_MAX_REC - 1), &tz87_dens,
  NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},
                 
{"TANDBERG TDC 3800", 17, DEV_TZQIC, 
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_QIC_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, (DEC_MAX_REC - 1), 
  &tzk10_dens, NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* Viper 2525 - ARCHIVES Viper 2525 QIC tape drive */ 
{"ARCHIVE VIPER", 13, DEV_TZQIC, 
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_QIC_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, (DEC_MAX_REC - 1), 
  &viper_2525_dens, NO_MODE_TAB, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

/* EXABYTE EXB-8200 */
{"EXABYTE EXB-8200", 16, DEV_TZK08, 
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_8MM_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, (DEC_MAX_REC - 1), 
  &def_dens, &tzk08_mod, SZ_NO_FLAGS,
  NO_OPT_CMDS, SZ_READY_DEF, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},
/* EXABYTE EXB-8500 */
{ "DEC     TKZ09", 13, DEV_TKZ09, 
  (ALL_DTYPE_SEQUENTIAL << DTYPE_SHFT) | SZ_8MM_CLASS,
  (struct pt_info *)ccmn_null_sizes, SZ_NO_BLK, 0x3c000, 
  &tkz09_dens, &tkz09_mod, SZ_NO_FLAGS,
  NO_OPT_CMDS, 240, SZ_NO_QUE, DD_REQSNS_VAL | DD_INQ_VAL,
  36, 64
},

{ 0 }

};


/*
 * Number of valid descriptors (size of table)
 */
I32 num_dev_desc = sizeof(cam_devdesc_tab)/sizeof(DEV_DESC);

#ifdef vax
/*
 * Disk milliseconds per word transfer rate for iostat.
 * The rate calculation is based on words per one disk revolution.
 * The RZxx entry is for new or user disks. The size of the table
 * is not fixed, more entries may be added as needed.
 * NOTE: the driver changes the rate for RX23 9 sector diskettes.
 */
float	sz_dk_mspw[] = {           /* 1.0 / (60 * sectors/track * 256) */
	0.0,		/* 0  - no iostat (tapes and unknown devices)	    */
	0.000002,	/* 1  - RZ22  (1.0 / (60 * 33 * 256))		    */
	0.000002,	/* 2  - RZ23  (1.0 / (60 * 33 * 256))		    */
	0.0000018,	/* 3  - RZ55  (1.0 / (60 * 36 * 256))		    */
	0.0000434,	/* 4  - RX23  (1.0 / (5 * 18 * 256))		    */
	0.0000113,	/* 5  - RRD40 (176.4 KB/SEC)			    */
	0.0000012,	/* 6  - RZ56  (1.0 / (60 * 54 * 256))		    */
	0.0000434,	/* 7  - RX33  (1.0 / (6 * 15 * 256))		    */
	0.0000017,      /* 8  - RZ24  (1.0 / (60 * 38 * 256))               */
	0.0000009,      /* 9  - RZ57  (1.0 / (60 * 71 * 256))               */
	0.0000017,      /* 10 - RZ23L (1.0 / (60 * 39 * 256))               */
	0.0000133,	/* 11 - RRD42 (150.0 KB/SEC)			    */
	0.0000217,	/* 12 - RX26  (1.0 / ((300 / 60) * 36 * 256))	    */
	0.0000013,	/* 13 - RZ25  (1.0 / (60 * 48 * 256))		    */
	0.0,		/* 14 - RZxx					    */
};
#endif vax


/* END OF FILE */
/* ---------------------------------------------------------------------- */
