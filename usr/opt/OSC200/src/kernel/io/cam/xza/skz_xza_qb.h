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
#if !defined(SKZ_XZA_QB_INCLUDE)
#define SKZ_XZA_QB_INCLUDE

/************************************************************************
 *									*
 * File:	skz_xza_q.h						*
 * Date:	November 6, 1991					*
 * Author:	Ron Hegli                                               *
 *									*
 * Description:								*
 *	This file contains the XZA's Queue Buffer structure definitions *
 *									*
 ************************************************************************/

/*
**
**
** SCSI Command Definitions
**
**
*/
typedef struct qb_status {
	unsigned short		fail	: 1;	/* fail bit */
	unsigned short		type	: 7;	/* status type */
	unsigned short		scsi_status : 8;/* scsi status byte */
} QB_STATUS;

typedef struct qb_flags {
	unsigned short		r	: 1;	/* response enable bit */
	unsigned short		i	: 1;	/* interrupt suppress bit */
	unsigned short		ca	: 1;	/* count all events */
	unsigned short		unused	: 1;
	unsigned short		sw	: 4;	/* reserved for software */
	unsigned short		unused1	: 6;	
	unsigned short		nike	: 1;	/* next in kommand exec */
	unsigned short		unused2	: 1;
} QB_FLAGS;

typedef struct scsi_flags {
	unsigned char		rd	: 1;	/* read data transfer bit */
	unsigned char		tq	: 1;	/* tagged queue command */
	unsigned char		as	: 1;	/* autosense enable bit */
	unsigned char		sdtr	: 1;	/* synch data transfer */
	unsigned char		disc	: 1;	/* disconnect priv */
	unsigned char		tiop	: 1;	/* terminate I/O process */
	unsigned char		disrty	: 1;	/* disable retry */
	unsigned char		sbz	: 1;	/* should be zero */
} SCSI_FLAGS;

typedef struct scsi_id {
	unsigned char		targid;		/* target # */
	unsigned char		lun;		/* target logical unti # */
} SCSI_ID;

/*
** The XZA's Queue Buffer 
*/
typedef struct np_qb_header {
	long		token;		/* address token */
	unsigned char	opc;		/* qb opcode */
	unsigned char	chnl_idx;	/* channel index */
	QB_FLAGS	flags;		/* qb flags */
	QB_STATUS	status;		/* qb status */
	unsigned char	scsi_opc;	/* scsi opcode */
	SCSI_FLAGS	scsi_flags;	/* scsi flags */
	SCSI_ID		dst_xport;	/* destination id */
	SCSI_ID		src_xport;	/* source id */
	unsigned int	reserved;	/* reserved */
} NP_QB_HEADER;


/*
** The XZA's Queue Buffer for SCSI Commands
*/
typedef struct skz_qb_header {
	long		token;		/* address token */
	unsigned char	opc;		/* qb opcode */
	unsigned char	chnl_idx;	/* channel index */
	QB_FLAGS	flags;		/* qb flags */
	QB_STATUS	status;		/* qb status */
	unsigned char	scsi_opc;	/* scsi opcode */
	SCSI_FLAGS	scsi_flags;	/* scsi flags */
	SCSI_ID		dst_xport;	/* destination id */
	SCSI_ID		src_xport;	/* source id */
	unsigned int	reserved;	/* reserved */
} SCSI_QB_HEADER;


/*
** Buffer Pointer Types
*/
typedef struct buf_ptr {
	unsigned long		typ	: 2;	/* pointer type */
	unsigned long		mbz	: 3;	/* must be zero */
	unsigned long		pa	: 27;	/* phys addr of data or array */
	unsigned long		pax	: 16;	/* phys addr of data or array */
	unsigned long		count	: 16;	/* # entries in SG array */
} BUF_PTR;

#define TYP0	0
#define TYP1	1
#define TYP2	2
#define TYP3	3



/*
** SNDSCSI Command Section
*/
typedef struct sndscsi_command {
	unsigned long		xct_id;		/* command transaction id */
	unsigned int		xfer_len;	/* trans length, in bytes */
	unsigned int		xfer_offset;	/* data buff byte offset */
	unsigned int		ret_len;	/* actual trans length */
	unsigned short		cdb_len;	/* CDB size */
	unsigned short		sbz1;		/* should be zero */
	BUF_PTR			buf_root_ptr_0; /* root buf ptr */
	BUF_PTR			buf_root_ptr_1;
	unsigned char		cdb[14];	/* CB byes */
	unsigned short		tag_msg;	/* tag msg code byte */

	unsigned char		autosense_ptr[6]; /* as buff ptr */
	unsigned short		as_len;		/* as buff length */

	unsigned long		cmd_tmo;	/* command timeout */
} SNDSCSI_CMD;

/*
** SNDVNDR Command Section
*/
typedef struct sndvndr_command {
	unsigned long		xct_id;		/* command transaction id */
	unsigned int		xfer_len;	/* trans length, in bytes */
	unsigned int		xfer_offset;	/* data buff byte offset */
	unsigned int		ret_len;	/* actual trans length */
	unsigned short		cdb_len;	/* CDB size */
	unsigned short		cdb_offset;	/* should be zero */
	BUF_PTR			buf_root_ptr_0; /* root buf ptr */
	BUF_PTR			buf_root_ptr_1;
	unsigned long		cdb_ptr_0;	/* CDB byes */
	unsigned char		cdb_ptr_1[6];	
	unsigned short		tag_msg;	/* tag msg code byte */
	unsigned char		autosense_ptr[6];	/* as buffer ptr */
	unsigned short		as_len;		/* as buffer size */
	unsigned long		cmd_tmo;	/* command timeout */
} SNDVNDR_CMD;


/*
** ABRTCMD Command Section
*/
typedef struct abrtcmd_command {
	unsigned long		xct_id;		/* command transaction id */
} ABRTCMD_CMD;

/*
** MRKABRT Command Section
*/
typedef struct mrkabrt_command {
	unsigned long		xct_id;		/* command transaction id */
} MRKABRT_CMD;

/*
** BDRST Command Section
*/
typedef struct bdrst_command {
	unsigned long		xct_id;
} BDRST_CMD;

/*
**
**
** N_Port Adapter Commands
**
**
*/

/*
** RETQE Command
*/
typedef struct retqe_command {
	/* as per n_port spec */
	unsigned short	qe_req;
	unsigned short	qe_ret;
	unsigned int	reserved;
} RETQE_CMD;

/*
** PURGQ Command
*/
typedef struct purgq_command {
	unsigned char		lunq_mask;	/* logical unit queue mask */
} PURGQ_CMD;

/*
** RDCNT Command
*/
typedef struct rdcnt_command {
	unsigned short		opt_cntrs_len;
	unsigned short		impl_cntrs_len;
	unsigned int		unimplemented_counters[21];

	unsigned int		sndscsi_cmd_count;
	unsigned int		sndvndr_cmd_count;
	unsigned int		abrtcmd_cmd_count;
	unsigned int		mrkabrt_cmd_count;
	unsigned int		bdrst_cmd_count;
	unsigned int		purgq_cmd_count;
	unsigned int		setnex_cmd_count;
	unsigned int		rdcnt_cmd_count;
	unsigned int		setdiag_cmd_count;
	unsigned int		retqe_cmd_count;

	unsigned int		direct_class_cmd_count;
	unsigned int		read_class_cmd_count;
	unsigned int		write_class_cmd_count;

	unsigned int		gcqir_write_count;
	unsigned int		qir_write_count;
	unsigned int		inq_rec_count;
	unsigned int		aborted_count;
	unsigned int		autosense_count;
	
	unsigned long		node_0_read_byte_cnt;
	unsigned long		node_1_read_byte_cnt;
	unsigned long		node_2_read_byte_cnt;
	unsigned long		node_3_read_byte_cnt;
	unsigned long		node_4_read_byte_cnt;
	unsigned long		node_5_read_byte_cnt;
	unsigned long		node_6_read_byte_cnt;
	unsigned long		node_7_read_byte_cnt;

	unsigned long		node_0_write_byte_cnt;
	unsigned long		node_1_write_byte_cnt;
	unsigned long		node_2_write_byte_cnt;
	unsigned long		node_3_write_byte_cnt;
	unsigned long		node_4_write_byte_cnt;
	unsigned long		node_5_write_byte_cnt;
	unsigned long		node_6_write_byte_cnt;
	unsigned long		node_7_write_byte_cnt;

	unsigned int		bus_parity_err_cnt;
	unsigned int		unsol_resel_cnt;
	unsigned int		ill_phase_err_cnt;
	unsigned int		cmd_timeout_cnt;
	unsigned int		select_timeout_cnt;
	unsigned int		unexp_discon_cnt;
	unsigned int		bus_rst_sent_cnt;
	unsigned int		bus_rst_recv_cnt;
	unsigned int		bus_dev_rst_sent_cnt;
	unsigned int		bus_dev_rst_recv_cnt;
} RDCNT_CMD;

/*
** Nexus Descriptor Table Entry
*/
typedef struct nexus_entry {
	unsigned int		qfroz	: 1;	/* queue frozen flag */
	unsigned int		qfull	: 1;	/* queue full flag */
	unsigned int		mbz	: 2;	/* must be zero */
	unsigned int		sync	: 1;	/* synch transfer enable */
	unsigned int		bdr	: 1;	/* device was reset */
	unsigned int		tmo	: 1;	/* SCSI command timeout */
	unsigned int		mbz2	: 22;	/* must be zero */
	unsigned int		qall	: 1;	/* lun queue allocated */
	unsigned int		qdea	: 1;	/* lun queue deallocated */
	unsigned int		tgt	: 1;	/* target wide function */
	unsigned int		ph_chg_tmo 	/* phase change timeout */
} NEXUS_ENTRY;
/*
** SETNEX Command
*/
typedef struct setnex_command {
	NEXUS_ENTRY	mask;
	NEXUS_ENTRY	m_value;
	NEXUS_ENTRY	in_nex_0;
	unsigned short	next_res;
	unsigned short	reserved[3];
	NEXUS_ENTRY	in_nex_1;
	NEXUS_ENTRY	in_nex_2;
	NEXUS_ENTRY	in_nex_3;
	NEXUS_ENTRY	in_nex_4;
	NEXUS_ENTRY	in_nex_5;
	NEXUS_ENTRY	in_nex_6;
	NEXUS_ENTRY	in_nex_7;
} SETNEX_CMD;


/*
**
**
** SETDIAG Queue Buffer Command Sections
**
**
*/


/*
** The SETDIAG Q_Buffer Header Structure
*/
typedef struct setdiag_qb_hdr {
	long		token;		/* address token */
	unsigned char	opc;		/* qb opcode */
	unsigned char	chnl_idx;	/* channel index */
	QB_FLAGS	flags;		/* qb flags */
	QB_STATUS	status;		/* qb status */
	unsigned char	diag_opc;	/* diagnostic opcode */
	char		sbz;		/* should be zero */
	SCSI_ID		dst_xport;	/* destination id */
	unsigned short	src_xport;	/* source id */
	unsigned int	reserved;	/* reserved */
} SETDIAG_QB_HEADER;

/*
**
*/
typedef struct eeprom_fcn {
	unsigned int	lce	: 1;	/* local console enabled */
	unsigned int	lste	: 1;	/* log self-test errors */
	unsigned int	lncre	: 1;	/* log NCR RBD errors */
	unsigned int	lxmie	: 1;	/* log XMI RBD errors */
	unsigned int	lxzae	: 1;	/* log XZA RBD errors */
	unsigned int	disrbd	: 1;	/* disable RBD error logging */
	unsigned int	hefo	: 1;	/* hard error frame overflow */
	unsigned int	sefo	: 1;	/* soft error frame overflow */
	unsigned int	refo	: 1;	/* RBD error frame overflow */
	unsigned int	fwufo	: 1;	/* Firmware update frame overflow */
	unsigned int	unused	: 4;	/* NOT USED */
	unsigned int	uc0id	: 1;	/* Update Chan 0 id in EEPROM */
	unsigned int	uc1id	: 1;	/* Update Chan 1 id in EEPROM */
} EEPROM_FCN;

/*
** EE$WriteParam Command
*/
typedef struct write_param_command {
	EEPROM_FCN	eeprm_fcn;	/* eeprom function */
	unsigned char	c0_id;		/* channel 0 scsi id */
	unsigned char	c1_id;		/* channel 1 scsi id */
	unsigned char	mod_ser_no[12];	/* module serial number */
} WRITEPARAM_CMD;

/*
** EE$ReadParam Command
*/
typedef struct read_param_command {
	EEPROM_FCN	eeprm_fcn;	/* eeprom function */
	unsigned char	c0_id;		/* channel 0 scsi id */
	unsigned char	c1_id;		/* channel 1 scsi id */
	unsigned char	mod_ser_no[12];	/* module serial number */
	unsigned char	fw_image_rev[4];	/* firmware image revision */
	unsigned char	fw_rev_date[12];	/* firmware rev date */
} READPARAM_CMD;

/*
** EE$ReadErrHrd Command
*/
typedef struct read_err_hrd_command {
	unsigned int	buf_offset;	/* byte offset in host memory buffer */
	unsigned int	buf_len;	/* host memory buffer length */
	BUF_PTR		ptr_0;		/* TYP0 buffer pointer */
	BUF_PTR		ptr_1;		/* TYP0 buffer pointer */
} READERRHRD_CMD;

/*
** EE$ReadErrSoft Command
*/
typedef struct read_err_soft_command {
	unsigned int	buf_offset;	/* byte offset in host memory buffer */
	unsigned int	buf_len;	/* host memory buffer length */
	BUF_PTR		ptr_0;		/* TYP0 buffer pointer */
	BUF_PTR		ptr_1;		/* TYP0 buffer pointer */
} READERRSOFT_CMD;

/*
** EE$WriteImage Command
*/
typedef struct write_image_command {
	unsigned int	buf_offset;		/* host memory buffer offset */
	unsigned int	buf_len;		/* host memory buffer length */
	BUF_PTR		buf_ptr;		/* TYP3 buffer pointer	*/	
} WRITEIMAGE_CMD;

#define XZA_FW_IMAGE_LEN	63 * 1024	/* 63K byte image */


/*
** EE$ReadImage Command
*/
typedef struct read_image_command {
	unsigned int	buf_offset;		/* host memory buffer offset */
	unsigned int	buf_len;		/* host memory buffer length */
	BUF_PTR		buf_ptr;		/* TYP3 buffer pointer	*/	
} READIMAGE_CMD;

/*
** EE$WriteEEPROM Command
*/
typedef struct write_eeprom_command {
	unsigned long		xct_id;
} WRITEEEPROM_CMD;

/*
** EE$ReadEEPROM Command
*/
typedef struct read_eeprom_command {
	unsigned int	buf_offset;		/* host memory buffer offset */
	unsigned int	buf_len;		/* host memory buffer length */
	BUF_PTR		buf_ptr;		/* TYP3 buffer pointer	*/	
} READEEPROM_CMD;

/*
** NCR$ReadReg Command
*/
typedef struct read_reg_command {

	unsigned long		reserved;

	unsigned char		scntl0;
	unsigned char		scntl1;
	unsigned char		sdid;
	unsigned char		sien;
	unsigned char		scid;
	unsigned char		sxfer;
	unsigned char		sodl;
	unsigned char		socl;

	unsigned char		sfbr;
	unsigned char		sidl;
	unsigned char		sbdl;
	unsigned char		sbcl;
	unsigned char		dstat;
	unsigned char		sstat0;
	unsigned char		sstat1;
	unsigned char		sstat2;

	unsigned int		dsa;
	unsigned char		ctest0;
	unsigned char		ctest1;
	unsigned char		ctest2;
	unsigned char		ctest3;

	unsigned char		ctest4;
	unsigned char		ctest5;
	unsigned char		ctest6;
	unsigned char		ctest7;
	unsigned int		temp;

	unsigned char		dfifo;
	unsigned char		istat;
	unsigned char		ctest8;
	unsigned char		lcrc;
	unsigned int		dbc_dcmd;

	unsigned int		dnad;
	unsigned int		dsp;
	unsigned int		dsps;
	unsigned int		scratch;

	unsigned int		dmode;
	unsigned char		dien;
	unsigned char		dwt;
	unsigned char		dcntl;
	unsigned char		adder;

} READREG_CMD;

/*
** NCR$WriteReg Command
*/
typedef struct write_reg_command {

	unsigned long		ncr_regspace_byte_mask;

	unsigned char		scntl0;
	unsigned char		scntl1;
	unsigned char		sdid;
	unsigned char		sien;
	unsigned char		scid;
	unsigned char		sxfer;
	unsigned char		sodl;
	unsigned char		socl;

	unsigned char		sfbr;
	unsigned char		sidl;
	unsigned char		sbdl;
	unsigned char		sbcl;
	unsigned char		dstat;
	unsigned char		sstat0;
	unsigned char		sstat1;
	unsigned char		sstat2;

	unsigned int		dsa;
	unsigned char		ctest0;
	unsigned char		ctest1;
	unsigned char		ctest2;
	unsigned char		ctest3;

	unsigned char		ctest4;
	unsigned char		ctest5;
	unsigned char		ctest6;
	unsigned char		ctest7;
	unsigned int		temp;

	unsigned char		dfifo;
	unsigned char		istat;
	unsigned char		ctest8;
	unsigned char		lcrc;
	unsigned int		dbc_dcmd;

	unsigned int		dnad;
	unsigned int		dsp;
	unsigned int		dsps;
	unsigned int		scratch;

	unsigned int		dmode;
	unsigned char		dien;
	unsigned char		dwt;
	unsigned char		dcntl;
	unsigned char		adder;

} WRITEREG_CMD;

/*
** DME Descriptor Definitions
*/

/*
** DME Flags
*/
#define	SKZ_DME_SG_1_VALID	0x00000001
#define SKZ_DME_SG_2_VALID	0x00000002


/*
** Scatter-gather list descriptor
*/
typedef struct skz_dme_sg_dsc {
	u_long		token;
	vm_offset_t	sg_array_ptr;
	vm_offset_t	sg_phys_ptr;
	u_short		sg_elem_cnt;
} SKZ_DME_SG_DSC;

/*
** DME Descriptor
*/
typedef struct skz_dme_dsc {

	vm_offset_t		data_ptr;		/* user va */
	u_long			flags;			/* dme flags */
	SKZ_DME_SG_DSC*		sg1_dsc;		/* scat descriptor */
	SKZ_DME_SG_DSC*		sg2_dsc;		/* scat descriptor */
	vm_offset_t		car1_ptr;		/* car for sg1 */
	vm_offset_t		car2_ptr;		/* car for sg2 */
	u_int			cam_sg_count;		/* sg element count */
	u_int			xfer_len;		/* requested xfer len */
	struct buf*		bp;			/* buf ptr, from ccb */

	BUF_PTR			buf_root_ptr_0;
	BUF_PTR			buf_root_ptr_1;
	u_int			xfer_offset;

} SKZ_DME_DSC;



/*
**
** Generic Structure Definitions
**
*/

/*
** Generic Q_Buffer Header
*/
typedef union xza_qb_hdr {

    NP_QB_HEADER	np_qb_hdr;
    SCSI_QB_HEADER	scsi_qb_hdr;
    SETDIAG_QB_HEADER	setdiag_qb_hdr;

} QB_HEADER;


/*
** Generic Q_Buffer Command Body
*/
typedef union xza_qb_body {
    /* SCSI Commands */
    SNDSCSI_CMD		send_scsi_cmd;
    SNDVNDR_CMD		send_vendor_cmd;
    ABRTCMD_CMD		abort_cmd_cmd;
    BDRST_CMD		bus_device_reset_cmd;
    /* N_Port Adapter Commands */
    RETQE_CMD		return_que_entry_cmd;
    PURGQ_CMD		purge_que_cmd;
    RDCNT_CMD		read_count_cmd;
    SETNEX_CMD		set_nexus_cmd;
    /* Diagnostic Commands */
    WRITEPARAM_CMD	write_param_cmd;
    READPARAM_CMD	read_param_cmd;
    READERRHRD_CMD	read_error_hard_cmd;
    READERRSOFT_CMD	read_error_soft_cmd;
    WRITEIMAGE_CMD	write_image_cmd;
    READIMAGE_CMD	read_image_cmd;
    WRITEEEPROM_CMD	write_eeprom_cmd;
    READEEPROM_CMD	read_eeprom_cmd;
    READREG_CMD		read_register_cmd;
    WRITEREG_CMD	write_register_cmd;
} QB_BODY;


typedef struct xza_qb_driver {

    /*
    ** Channel Pointer
    */
    vm_offset_t		xza_chan_ptr;

    /*
    ** SIM Working Set
    */
    vm_offset_t		sws;

    /*
    ** data transfer information
    */
    SKZ_DME_DSC		dme_dsc;

} QB_DRIVER;

#define SKZ_XZA_QE_LEN	512
#define SKZ_XZA_QB_BODY_LEN SKZ_XZA_QE_LEN - ( sizeof ( QB_HEADER ) + \
					       sizeof ( QB_DRIVER ) )

/*
** Generic Q_Buffer Definition
*/

typedef struct qb {

    /*
    ** Q_Buffer Header
    */
    QB_HEADER	qb_header;

    /*
    ** Q_Buffer Command Body
    */
    char	qb_body[SKZ_XZA_QB_BODY_LEN];
    /*
    ** Q_Buffer Driver Section
    */
    QB_DRIVER	qb_driver;

} QB;

/*
** Queue Buffer Opcodes
*/
#define SNDPM   0
#define PMSNT   0

#define PMREC   1

#define RETQE   2
#define QERET   2

#define PURGQ   3
#define QPURG   3

#define RDCNT   5
#define CNTRD   5

#define SETDIAG 8
#define DIAGSET 8

#define SETNEX  9
#define NEXSET  9

/*
** SCSI Opcode Values, when Queue Buffer opcode = SNDPM/PMREC
*/
#define SNDSCSI	64
#define SCSISNT 64

#define SNDVNDR	65
#define VNDRSNT	65

#define ABRTCMD	66
#define CMDABRT 66

#define MRKABRT	67
#define ABRTMRK	67

#define BDRST	68
#define RSTBD	68

/*
** SETDIAG Opcode Values, when Queue Buffer opcode = SETDIAG
*/
#define EE$WriteParam	0x80
#define EE$ReadParam	0x81
#define EE$ReadErrHrd	0x82
#define EE$ReadErrSoft	0x83
#define EE$WriteImage	0x84
#define EE$ReadImage	0x85
#define EE$WriteEEPROM	0x86
#define EE$ReadEEPROM	0x87
#define NCR$ReadReg	0x88
#define NCR$WriteReg	0x89

/* Defines used by DIAG commands */
#define opcode cam_sim_priv[0]
#define diag_opcode cam_sim_priv[1]
#define xza_scsi_id cam_sim_priv[2]


/*
** Tag Message Codes
*/
#define SIMPLE_QUEUE_TAG	0x20
#define HEAD_OF_QUEUE_TAG	0x21
#define ORDERED_QUEUE_TAG	0x22 


#endif /* SKZ_XZA_QB_INCLUDE */
