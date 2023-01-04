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
 * @(#)$RCSfile: cam_special.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/26 16:45:13 $
 */
#if !defined(CAM_SPECIAL_INCLUDE)
#define CAM_SPECIAL_INCLUDE 1

/************************************************************************
 *									*
 * File:	cam_special.h						*
 * Date:	April 1, 1991						*
 * Author:	Robin T. Miller						*
 *									*
 * Description:								*
 *	Definitions for SCSI special command interface.			*
 *									*
 * Modification History:						*
 *									*
 * August 3, 1991 by Robin Miller.					*
 *	Remove head/tail flags previously used to specify where to add	*
 *	 command tables to list.  Always added to end of list now.	*
 *	Added field sa_sense_resid for saving sense residual count from	*
 *	 the SCSI I/O CCB.  Passed back on SCSI_SPECIAL I/O requests.	*
 *	 Also changed field sa_sense_length from int to u_char.		*
 *	Changed define DTYPE_ALL_DEVICES to ALL_DEVICE_TYPES which is	*
 *	 used in command table headers to specify all device types.	*
 *	Changed field spc_device_type from u_char to u_long.		*
 *									*
 * July 3, 1991 by Robin Miller.					*
 *	Added driver specific pointer to special argument structure so	*
 * peripheral drivers can use this mechanism for device dependant info.	*
 * This field can be used by the various routines invoked when doing	*
 * setup or making the CDB, to pass parameters such as block size,	*
 * fixed or variable length records, density, or whatever.		*
 *									*
 ************************************************************************/

/*
 * Pre-defined Special Command Tables:
 */
extern struct special_header cam_GenericCmdsHdr;
extern struct special_header cam_DirectCmdsHdr;
extern struct special_header cam_AudioCmdsHdr;
extern struct special_header cam_SequentialCmdsHdr;
extern struct special_header cam_MtCmdsHdr;

/*
 * Local Defines:
 */
#define BITMASK(v)   (1L << v)		/* Convert value to bit mask.	*/
#define ISSET(m,v)   (m & BITMASK(v))	/* Test bit set in bit mask.	*/
#define ISCLR(m,v)   ((m & BITMASK(v)) == 0) /* Test bit clear in bit mask. */
#define	LTOB(a,b)    ((a>>(b*8))&0xff)	/* Obtain byte from a long.	*/
#define ALL_DEVICE_TYPES	-1	/* Supported for all devices.	*/

/*
 * Define Masks for SCSI Group Codes.
 */
#define	SCSI_GROUP_0		0x00	/* SCSI Group Code 0.		*/
#define SCSI_GROUP_1		0x20	/* SCSI Group Code 1.		*/
#define SCSI_GROUP_2		0x40	/* SCSI Group Code 2.		*/
#define SCSI_GROUP_3		0x60	/* SCSI Group Code 3.		*/
#define SCSI_GROUP_4		0x80	/* SCSI Group Code 4.		*/
#define SCSI_GROUP_5		0xA0	/* SCSI Group Code 5.		*/
#define SCSI_GROUP_6		0xC0	/* SCSI Group Code 6.		*/
#define SCSI_GROUP_7		0xE0	/* SCSI Group Code 7.		*/
#define SCSI_GROUP_MASK		0xE0	/* SCSI Group Code mask.	*/

#if !defined(SUCCESS)

#define SUCCESS		0		/* Success status return code.	*/
#define FAILURE		-1		/* Failure status return code.	*/

#endif /* !defined(SUCCESS) */

#define RETRYABLE	-1		/* Retryable error condition.	*/
#define NOT_RETRYABLE	EIO		/* Non-retryable error occured.	*/

/*
 * Special Command Header Structure:
 */
#define SPH_SUB_COMMAND		0x01	/* Table contains sub-commands.	*/

typedef struct special_header {
    struct special_header *sph_flink;	/* Forward link to next table.	*/
    struct special_header *sph_blink;	/* Backward link to prev table.	*/
    struct special_cmd *sph_cmd_table;	/* Pointer to command table.	*/
    U32 sph_device_type;		/* The device types supported.	*/
    U32 sph_table_flags;		/* Flags to control cmd lookup.	*/
    caddr_t sph_table_name;		/* Name of this command table.	*/
} SPECIAL_HEADER;

/*
 * Special Command Flags:
 */
#define SPC_SUSER		0x01	/* Restricted to super-user.	*/
#define SPC_COPYIN		0x02	/* User buffer to copy in from.	*/
#define SPC_COPYOUT		0x04	/* User buffer to copy out to.	*/
#define SPC_NOINTR		0x10	/* Don't allow sleep interrupts	*/
#define SPC_DATA_IN		0x20	/* Data direction from device.	*/
#define SPC_DATA_OUT		0x40	/* Data direction is to device.	*/
#define SPC_DATA_NONE		0x00	/* No data movement for command.*/
#define SPC_SUB_COMMAND		0x80	/* Entry contains sub-command.	*/

#define SPC_INOUT	(SPC_COPYIN | SPC_COPYOUT) /* Copy in and out.	*/
#define SPC_DATA_INOUT	(SPC_DATA_IN | SPC_DATA_OUT) /* Ditto .....	*/
#define ALL_CLASSES	SZ_DEVMASK	/* Command for device classes.	*/

#define END_OF_CMD_TABLE	0	/* End of the command table.	*/

/*
 * Special Command Entry Structure:
 */
typedef struct special_cmd {
	u_int	spc_ioctl_cmd;		/* The I/O control command code	*/
	u_int	spc_sub_command;	/* The I/O control sub-command.	*/
	u_char	spc_cmd_flags;		/* The special command flags.	*/
	u_char	spc_cmd_code;		/* The special command code.	*/
	u_short	reserved;		/* Unused ... align next field.	*/
	U32	spc_device_type;	/* The device types supported.	*/
	U32	spc_cmd_parameter;	/* Command parameter (if any).	*/
	U32	spc_cam_flags;		/* The CAM flags field for CCB.	*/
	U32	spc_file_flags;		/* File control flags (fcntl).	*/
	int	spc_data_length;	/* Kernel data buffer length.	*/
	int	spc_timeout;		/* Timeout for this command.	*/
	int	(*spc_docmd)();		/* Function to do the command.	*/
	int	(*spc_mkcdb)();		/* Function to make SCSI CDB.	*/
	int	(*spc_setup)();		/* Setup parameters function.	*/
	caddr_t	spc_cdbp;		/* Pointer to prototype CDB.	*/
	caddr_t	spc_cmdp;		/* Pointer to the command name.	*/
} SPECIAL_CMD;

/*
 * Special Argument Control Flags (Also See 'h/scsi_special.h'):
 */
#define SA_NO_ERROR_RECOVERY	0x01	/* Don't perform error recovery	*/
#define SA_NO_ERROR_LOGGING	0x02	/* Don't log error messages.	*/
#define SA_NO_SLEEP_INTR	0x04	/* Don't allow sleep interrupts	*/
#define SA_NO_SIMQ_THAW		0x08	/* Leave SIMQ Frozen on errors.	*/
#define SA_NO_WAIT_FOR_IO	0x10	/* Don't wait for I/O complete.	*/
#define SA_USER_FLAGS_MASK	0x1f	/* Only flags settable by user.	*/

/*
 * Flags used by Special Interface Routines:
 */
#define SA_ALLOCATED_BP		0x100	/* Allocated a request buffer.	*/
#define SA_ALLOCATED_CCB	0x200	/* Allocated CAM control block.	*/
#define SA_ALLOCATED_IOP	0x400	/* Allocated I/O parameter buf.	*/
#define SA_ALLOCATED_DATA	0x800	/* Allocated a data buffer.	*/
#define SA_ALLOCATED_SENSE	0x1000	/* Allocated a sense buffer.	*/

#define SA_SENSE_LOCKED		0x2000	/* Sense buffer has been locked	*/
#define SA_USER_LOCKED		0x4000	/* Users' data buffer is locked	*/

/*
 * Set this flag when issuing special commands from within the kernel
 * so the sanity check for a users' buffer gets bypassed.
 */
#define SA_SYSTEM_REQUEST	0x80000000  /* System command request.	*/

/*
 * Special Command Argument Block Structure:
 */
typedef struct special_args {
	U32	sa_flags;		/* Flags to control command.	*/
	dev_t	sa_dev;			/* Device major/minor number.	*/
	u_char	sa_unit;		/* Device logical unit number.	*/
	u_char	sa_bus;			/* SCSI host adapter bus number	*/
	u_char	sa_target;		/* SCSI device target number.	*/
	u_char	sa_lun;			/* SCSI logical unit number.	*/
	u_int	sa_ioctl_cmd;		/* The I/O control command.	*/
	u_int	sa_ioctl_scmd;		/* The sub-command (if any).	*/
	caddr_t	sa_ioctl_data;		/* The command data pointer.	*/
	caddr_t	sa_device_name;		/* Pointer to the device name.	*/
	int	sa_device_type;		/* The peripheral device type.	*/
	int	sa_iop_length;		/* Parameters buffer length.	*/
	caddr_t	sa_iop_buffer;		/* Parameters buffer address.	*/
	int	sa_file_flags;		/* The file control flags.	*/
	u_char	sa_sense_length;	/* Sense data buffer length.	*/
	u_char	sa_sense_resid;		/* Sense data residual count.	*/
	caddr_t	sa_sense_buffer;	/* Sense data buffer address.	*/
	int	sa_user_length;		/* User data buffer length.	*/
	caddr_t	sa_user_buffer;		/* User data buffer address.	*/
#ifdef _KERNEL
	struct buf *sa_bp;		/* Kernel I/O request buffer.	*/
#endif
	CCB_SCSIIO *sa_ccb;		/* CAM control block buffer.	*/
	struct special_cmd *sa_spc;	/* Special command table entry.	*/
	struct special_header *sa_sph;	/* Special command table header	*/
	U32	sa_cmd_parameter;	/* Command parameter (if any).	*/
	int	(*sa_error)();		/* The error report function.	*/
	int	(*sa_start)();		/* The driver start function.	*/
	int	sa_data_length;		/* Kernel data buffer length.	*/
	caddr_t	sa_data_buffer;		/* Kernel data buffer address.	*/
	caddr_t	sa_cdb_pointer;		/* Pointer to the CDB buffer.	*/
	u_char	sa_cdb_length;		/* Length of this CDB buffer.	*/
	u_char	sa_cmd_flags;		/* The special command flags.	*/
	u_char	sa_retry_count;		/* The current retry count.	*/
	u_char	sa_retry_limit;		/* Times to retry this command.	*/
	int	sa_timeout;		/* Timeout for this command.	*/
	int	sa_xfer_resid;		/* Transfer residual count.	*/
	caddr_t	sa_specific;		/* Driver specific information.	*/
} SPECIAL_ARGS;

#endif /* !defined(CAM_SPECIAL_INCLUDE) */
