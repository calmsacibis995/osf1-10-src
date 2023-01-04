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
 * @(#)$RCSfile: scsipages.h,v $ $Revision: 1.1.7.3 $ (DEC) $Date: 1993/12/15 20:56:51 $
 */
/*
 * Definitions for CCS/SCSI-2 Devices.
 */

/*
 * Mode Sense/Mode Select Definitions:
 */
		/* Values for Page Code field. */
#define MODE_PARAMETERS_PAGE	0x00	/* Used for mode parameters.	*/
#define ERROR_RECOVERY_PAGE	0x01	/* Error recovery page.		*/
#define DISCO_RECO_PAGE		0x02	/* Disconnect/reconnect page.	*/
#define DIRECT_ACCESS_PAGE	0x03	/* Direct access format	page.	*/
#define DISK_GEOMETRY_PAGE	0x04	/* Disk geometry page.		*/
#define FLEXIBLE_DISK_PAGE	0x05	/* Flexible disk page.		*/
#define VERIFY_RECOVERY_PAGE	0x07	/* Verify error recovery page.	*/
#define CACHE_CONTROL_PAGE	0x08	/* Cache control page.		*/
#define CDROM_DEVICE_PAGE	0x0D	/* CD-ROM device page.		*/
#define AUDIO_CONTROL_PAGE	0x0E	/* Audio control page.		*/
#define DEVICE_CONFIG_PAGE	0x10	/* Device configuration page.	*/
#define MEDIUM_PART_PAGE1	0x11	/* Medium parition page 1.	*/
#define DEC_SPECIFIC_PAGE	0x25	/* DEC specific page.		*/
#define READAHEAD_CONTROL_PAGE	0x38	/* Read-ahead control page.	*/
#define ALL_MODE_PAGES		0x3f	/* All mode pages.		*/
#define MODE_PAGE_MASK		0x3f	/* Mask to isolate page code.	*/

		/* Values for Page Control field. */
#define PCF_CURRENT		0x00	/* Return current mode select parms */
#define PCF_CHANGEABLE		0x40	/*   ""  changeable ""   ""    "" */
#define PCF_DEFAULT		0x80	/*   ""   default   ""   ""    "" */
#define PCF_SAVED		0xc0	/*   ""   saved     ""   ""    "" */
#define PCF_SHIFT		6	/* The PCF shift value.		*/

#define MS_PAGE_FORMAT		0x10	/* MS data complies w Page Format */
#define MS_SAVE_MODE_PARMS	1	/* Save mode parameters.	*/
#define MS_DONT_SAVE_PARMS	0	/* Don't save mode parameters.	*/

#define SCSI_NDEFECT		256	/* Maximum defect list length.	*/
#define SCSI_BLOCK_LENGTH	512	/* Desired block length.	*/

/*
 * Define different sparring methods.
 */
#define CYLINDER_SPARRING	0	/* Cylinder oriented mode.	*/
#define TRACK_SPARRING		1	/* Track oriented mode.		*/
#define DRIVE_SPARRING		2	/* Drive oriented mode.		*/
#define HOST_SPARRING		3	/* Host oriented mode.		*/

/*
 * Define Error Recovery bits:
 */
#define ER_DCR		0x01		/* Disable correction.		*/
#define ER_DTE		0x02		/* Disable transfer on error.	*/
#define ER_PER		0x04		/* Post recovable error.	*/
#define ER_EEC		0x08		/* Enable early correction.	*/
#define ER_RC		0x10		/* Read continuous.		*/
#define ER_TB		0x20		/* Transfer block in error.	*/
#define ER_ARRE		0x40		/* Auto read realloc on error.	*/
#define ER_AWRE		0x80		/* Auto write realloc on error.	*/

/*
 * Default values originally hardcoded in the source.
 */
#define DEF_ALT_SECTORS_ZONE	1	/* Set for			*/
#define DEF_ALT_TRACKS_VOLUME	0 /* nhead /*	track oriented		*/
#define DEF_TRACKS_PER_ZONE	1	/*		sparring...	*/
#define DEF_ALT_TRACKS_ZONE	0	/* MUST be set to this value.	*/
#define DEF_ERROR_RECOVERY	(ER_PER | ER_TB)	/* See above.	*/
#define DEF_RETRY_COUNT		16	/* Error retry count.		*/
#define DEF_DATA_SECTOR		512	/* Bytes per physical sector.	*/
#define DEF_INTERLEAVE		1	/* Interleave factor.		*/
#define DEF_BUS_INACTIVITY_LIMIT 40	/* Bus inactivity limit.	*/
#define DEF_UNIX_ALT_CYLS	2	/* Default Unix alternate cyls.	*/
#define DEF_RESERVED_CYLS	3	/* Default reserved cylinders.	*/

/*
 * Medium Type Definitions:
 */
#define MT_DEFAULT		0x00	/* Default medium type.		*/
#define MT_FLEXIBLE_SS		0x01	/* Single sided flexible disk.	*/
#define MT_FLEXIBLE_DD		0x02	/* Double sided flexible disk.	*/

/*
 * Optical Memory Media-Type Code Definitions:
 */
enum optical_medium {
	MT_OP_READ_ONLY = 1,		/* Optical Read only medium.	*/
	MT_OP_WRITE_ONCE,		/* Optical Write once medium.	*/
	MT_OP_ERASABLE,			/* Reversible or erasable.	*/
	MT_OP_RONLY_WONCE,		/* Read only & Write once medium */
	MT_OP_RONLY_ERASABLE,		/* Read only & reverse/erasable. */
	MT_OP_WONCE_ERASABLE,		/* Write once & reverse/erasable.*/
	MT_OP_UNKNOWN			/* Reserved or vendor specific.	*/
};

/*
 * DEC Vendor Specific Medium Type Definitions:
 */
#define MT_DD_DISKETTE		0x80	/* Double density diskette (1MB)*/
#define MT_HD_DISKETTE		0x81	/* High density diskette.  (2MB)*/
#define MT_ED_DISKETTE		0x82	/* Extra density diskette. (4MB)*/

/*
 * Device Specific Parameter Definitions:
 */
#define DS_WRITE_PROTECT	0x80	/* Medium is write protected.	*/

/*
 * Sequential Access Device Specific Definitions:
 */
typedef struct sequential_device_specific {
	u_char	sds_speed	: 4,	/* The sequential tape speed.	*/
		sds_buffer_mode	: 3,	/* The tape buffered mode.	*/
		sds_wp		: 1;	/* Write protected (1 = yes).	*/
} SEQUENTIAL_DEVICE_SPECIFIC;

#define SPEED_DEFAULT		0x00	/* Use default device speed.	*/
#define SPEED_LOWEST		0x01	/* Use device's lowest speed.	*/

struct mode_parameter_header {
	u_char	mph_data_length;	/* Mode data length.		[0] */
	u_char	mph_medium_type;	/* Medium type.			[1] */
	union {
	    u_char device_specific;	/* Device specific information.	[2] */
	    struct sequential_device_specific tp;
	} un;
#define mph_device_specific un.device_specific
#define mph_speed un.tp.sds_speed
#define mph_buffer_mode un.tp.sds_buffer_mode
#define mph_write_protect un.tp.sds_wp
	u_char	mph_block_desc_length;	/* Block descriptor length.	[3] */
};

#define mph_reserved_0	mph_data_length	/* Reserved for mode select.	*/
#define mph_reserved_1	mph_medium_type /*       "   "   "     "	*/
#define mph_reserved_2	mph_device_specific /*   "   "   "     "	*/

/*
 * Define The Mode Parameter Block Descriptor:
 */
struct mode_block_descriptor {
	u_char	mbd_density_code;	/* Density code.		[0] */
	u_char	mbd_num_blocks_2;	/* Number of blocks (MSB).	[1] */
	u_char	mbd_num_blocks_1;	/*   "    "    " 		[2] */
	u_char	mbd_num_blocks_0;	/* Number of blocks (LSB).	[3] */
	u_char	mbd_reserved_0;		/* Reserved.			[4] */
	u_char	mbd_block_length_2;	/* Block length (MSB).		[5] */
	u_char	mbd_block_length_1;	/*   "     "			[6] */
	u_char	mbd_block_length_0;	/* Block length (LSB).		[7] */
};

/*
 * Define the Mode Page Header:
 */
struct mode_page_header {
	struct mode_parameter_header parameter_header;
	struct mode_block_descriptor block_descriptor;
}; 

/*
 * Page Header
 */
struct scsi_page_header {
	u_char	page_code	: 6;	/* define page function */
	u_char	reserved	: 1;
	u_char	ps		: 1;	/* Page savable. */
	u_char	page_length;		/* length of current page */
};

/*
 * Page One - Error Recovery Parameters:
 */
struct error_bits {
	u_char	dcr		: 1;	/* disable correction */
	u_char	dte		: 1;	/* disable transfer on error */
	u_char	per		: 1;	/* post error */
	u_char	eec		: 1;	/* enable early correction */
	u_char 	rc		: 1;	/* read continuous */
	u_char	tb		: 1;	/* transfer block */
	u_char	arre		: 1;	/* auto read realloc enabled */
	u_char	awre		: 1;	/* auto write realloc enabled */
};

struct scsi_error_recovery {
	struct	scsi_page_header page_header;
	union {
		u_char control_bits;
		struct error_bits eb;
	} un;
#define er_dcr	un.eb.dcr
#define er_dte	un.eb.dte
#define er_per	un.eb.per
#define er_eec	un.eb.eec
#define er_rc	un.eb.rc
#define er_tb	un.eb.tb
#define er_arre	un.eb.arre
#define er_awre	un.eb.awre
#define er_control un.control_bits
	u_char	er_read_retry_count;
	u_char	er_correction_span;
	u_char	er_head_offset_count;
	u_char	er_strobe_offset_count;
	u_char	er_reserved_0;
	u_char	er_write_retry_count;
	u_char	er_reserved_1;
	u_char	er_recovery_time_limit_1;
	u_char	er_recovery_time_limit_0;
};

/*
 * Defines for CCS compliant devices:
 */
#define er_retry_count		er_read_retry_count
#define er_recovery_time_limit	er_reserved_0
#define ccs_error_page_length	sizeof(struct scsi_error_recovery)-4

/*
 * Page Two - Disconnect / Reconnect Control Parameters:
 */
struct scsi_disco_reco {
	struct	scsi_page_header page_header;
	u_char	dr_buffer_full_ratio;
	u_char	dr_buffer_empty_ratio;
	u_char	dr_bus_inactivity_limit_1;
	u_char	dr_bus_inactivity_limit_0;
	u_char	dr_disconnect_time_limit_1;
	u_char	dr_disconnect_time_limit_0;
	u_char	dr_connect_time_limit_1;
	u_char	dr_connect_time_limit_0;
	u_char	dr_maximum_burst_size_1;
	u_char	dr_maximum_burst_size_0;
	u_char	dr_dtdc		: 2,	/* Data transfer disconnect control.*/
				: 6;	/* Reserved.			    */
	u_char	dr_reserved[3];
};

/*
 * Page Three - Direct Access Device Format Parameters:
 */
struct scsi_direct_access {
	struct	scsi_page_header page_header;
	u_char	da_tracks_per_zone_1;
	u_char	da_tracks_per_zone_0;
	u_char	da_alt_sectors_zone_1;
	u_char	da_alt_sectors_zone_0;
	u_char	da_alt_tracks_zone_1;
	u_char	da_alt_tracks_zone_0;
	u_char	da_alt_tracks_vol_1;
	u_char	da_alt_tracks_vol_0;
	u_char	da_sectors_track_1;
	u_char	da_sectors_track_0;
	u_char	da_data_sector_1;
	u_char	da_data_sector_0;
	u_char	da_interleave_1;
	u_char	da_interleave_0;
	u_char	da_track_skew_1;
	u_char	da_track_skew_0;
	u_char	da_cylinder_skew_1;
	u_char	da_cylinder_skew_0;
	u_char			: 4,
		da_surf		: 1,
		da_rmb		: 1,
		da_hsec		: 1,
		da_ssec		: 1;
	u_char	da_reserved[3];
};

/*
 * Page Four - Rigid Disk Geometry Parameters:
 */
struct scsi_disk_geometry {
	struct	scsi_page_header page_header;
	u_char	dg_cylinders_2;
	u_char	dg_cylinders_1;
	u_char	dg_cylinders_0;
	u_char	dg_heads;
	u_char	dg_precomp_cyl_2;
	u_char	dg_precomp_cyl_1;
	u_char	dg_precomp_cyl_0;
	u_char	dg_current_cyl_2;
	u_char	dg_current_cyl_1;
	u_char	dg_current_cyl_0;
	u_char	dg_step_rate_1;
	u_char	dg_step_rate_0;
	u_char	dg_landing_cyl_2;
	u_char	dg_landing_cyl_1;
	u_char	dg_landing_cyl_0;
	u_char	dg_rpl		: 2,
				: 6;
	u_char	dg_rotational_offset;
	u_char			: 8;
	u_char	dg_rpm_1;
	u_char	dg_rpm_0;
	u_char			: 8;
	u_char			: 8;
};

/*
 * Defines for CCS compliant devices:
 */
#define ccs_geometry_page_length	sizeof(struct scsi_disk_geometry)-4

/*
 * Page Five - Flexible Disk Parameters:
 */
struct scsi_flexible_disk {
	struct	scsi_page_header page_header;
	u_char	fd_transfer_rate_1;	/* Transfer rate.	  (MSB)	*/
	u_char	fd_transfer_rate_0;	/*    "      "		  (LSB)	*/
	u_char	fd_heads;		/* Number of heads.		*/
	u_char	fd_sectors_track;	/* Sectors per track.		*/
	u_char	fd_data_sector_1;	/* Data bytes per sector. (MSB)	*/
	u_char	fd_data_sector_0;	/*  "    "     "    "	  (LSB)	*/
	u_char	fd_cylinders_1;		/* Number of cylinders.	  (MSB)	*/
	u_char	fd_cylinders_0;		/*   "    "     "	  (LSB)	*/
	u_char	fd_precomp_cyl_1;	/* Write precompensation. (MSB)	*/
	u_char	fd_precomp_cyl_0;	/*   "      "      "	  (LSB)	*/
	u_char	fd_current_cyl_1;	/* Reduced write current. (MSB)	*/
	u_char	fd_current_cyl_0;	/*    "      "     "	  (LSB)	*/
	u_char	fd_step_rate_1;		/* Drive step rate.	  (MSB)	*/
	u_char	fd_step_rate_0;		/*   "    "    "	  (LSB)	*/
	u_char	fd_step_pulse_width;	/* Drive step pulse width.	*/
	u_char	fd_head_settle_delay_1;	/* Head settle delay.	  (MSB)	*/
	u_char	fd_head_settle_delay_0;	/*  "     "      "	  (LSB)	*/
	u_char	fd_motor_on_delay;	/* Motor on delay.		*/
	u_char	fd_motor_off_delay;	/* Motot off delay.		*/
	u_char			: 5,	/* Reserved.			*/
		fd_mo		: 1,	/* Motor on (pin 16 asserted).	*/
		fd_ssn		: 1,	/* Starting sector number one.	*/
		fd_trdy		: 1;	/* True ready indicator.	*/
	u_char	fd_spc		: 4,	/* Addl' step pulses/cylinder.	*/
				: 4;	/* Reserved.			*/
	u_char	fd_write_compensation;	/* Write compensation.		*/
	u_char	fd_head_load_delay;	/* Head load delay.		*/
	u_char	fd_head_unload_delay;	/* Head unload delay.		*/
	/*
	 * The use of the following pins varies between each vendor,
	 * therefore the bit definitions are not broken out.
	 */
	u_char	fd_pin_2	: 4,	/* Pin 2 definition.		*/
		fd_pin_34	: 4;	/* Pin 34 definition.		*/
	u_char	fd_pin_1	: 4,	/* Pin 1 definition.		*/
		fd_pin_4	: 4;	/* Pin 4 definition.		*/
	u_char	fd_rotate_rate_1;	/* Medium rotation rate.  (MSB)	*/
	u_char	fd_rotate_rate_0;	/*   "      "  "    "	  (LSB)	*/
	u_char	fd_reserved[2];		/* Reserved.			*/
};

/*
 * Page 7 - Verify Error Recovery Parameters:
 */
struct scsi_verify_recovery {
	struct	scsi_page_header page_header;
	union {
		u_char control_bits;
		struct error_bits eb;
	} un;
#define vr_dcr	un.eb.dcr
#define vr_dte	un.eb.dte
#define vr_per	un.eb.per
#define vr_eec	un.eb.eec
#define vr_rc	un.eb.rc
#define vr_tb	un.eb.tb
#define vr_arre	un.eb.arre
#define vr_awre	un.eb.awre
#define vr_control un.control_bits
	u_char	vr_retry_count;		/* Verify retry count.		*/
	u_char	vf_reserved[4];		/* Reserved.			*/
};

/*
 * Page 8 - Cache Control Parameters:
 */
struct scsi_cache_control {
	struct	scsi_page_header page_header;
	u_char	ca_rcd		: 1,	/* Read cache disable.		*/
		ca_ms		: 1,	/* Multiple selection enable.	*/
		ca_wce		: 1,	/* Write cache enable.		*/
				: 5;	/* Reserved.			*/
	u_char	ca_write_reten_pri : 4,	/* Write retention priority.	*/
		ca_read_reten_pri  : 4;	/* Demand read retention prior.	*/
	u_char	ca_dis_prefetch_xfer_1;	/* Disable prefetch		*/
	u_char	ca_dis_prefetch_xfer_0;	/*	      transfer length.	*/
	u_char	ca_min_prefetch_1;	/* Minimum prefetch.		*/
	u_char	ca_min_prefetch_0;	/*    "       "			*/
	u_char	ca_max_prefetch_1;	/* Maximum prefetch.		*/
	u_char	ca_max_prefetch_0;	/*    "       "			*/
	u_char	ca_max_prefetch_ceiling_1; /* Maximum prefetch ceiling.	*/
	u_char	ca_max_prefetch_ceiling_0; /*    "       "       "	*/
};

/*
 * Page D - CD-ROM Device Parameters:
 */
struct scsi_cdrom_device {
	struct	scsi_page_header page_header;
	u_char			: 8;	/* Reserved.			*/
	u_char	cd_inactive_timer : 4,	/* Inactivity timer multiplier.	*/
				: 4;	/* Reserved.			*/
	u_char	cd_num_su_per_mu_1;	/* Number S-units M-units (MSB)	*/
	u_char	cd_num_su_per_mu_0;	/*    "      "      "     (LSB)	*/
	u_char	cd_num_fu_per_su_1;	/* Number F-units S-units (MSB)	*/
	u_char	cd_num_fu_per_su_0;	/*    "      "      "     (LSB)	*/
};

/*
 * Page E - CD-ROM Audio Control Parameters:
 */
struct scsi_audio_control {
	struct	scsi_page_header page_header;
	u_char			: 1,	/* Reserved.			*/
		ac_sotc		: 1,	/* Stop on track crossing.	*/
		ac_immed	: 1,	/* Immediate bit.		*/
				: 5;	/* Reserved.			*/
	u_char	ac_reserved[5];		/* Reserved.			*/
	u_char	ac_chan0_select	: 4,	/* Channel 0 selection code.	*/
				: 4;	/* Reserved.			*/
	u_char	ac_chan0_volume;	/* Channel 0 volume level.	*/
	u_char	ac_chan1_select	: 4,	/* Channel 1 selection code.	*/
				: 4;	/* Reserved.			*/
	u_char	ac_chan1_volume;	/* Channel 1 volume level.	*/
	u_char	ac_chan2_select	: 4,	/* Channel 2 selection code.	*/
				: 4;	/* Reserved.			*/
	u_char	ac_chan2_volume;	/* Channel 2 volume level.	*/
	u_char	ac_chan3_select	: 4,	/* Channel 3 selection code.	*/
				: 4;	/* Reserved.			*/
	u_char	ac_chan3_volume;	/* Channel 3 volume level.	*/
};

/*
 * Page 10 - Device Configuration Parameters
 */
struct scsi_device_config {
	struct	scsi_page_header page_header;
	u_char	dc_act_fmt	: 5,	/* Active format.		*/
		dc_ch_act_fmt	: 1,	/* Change active format. 	*/
		dc_ch_act_part	: 1,	/* Change active partition.	*/
				: 1;	/* Reserved.			*/
	u_char	dc_act_part;		/* Active partition.		*/
	u_char	dc_wt_buff_full;	/* Write buffer full ratio.	*/
	u_char	dc_rd_buff_empty;	/* Read buffer empty ratio.	*/
	u_char	dc_wt_delay_time_1;	/* Write delay time. (MSB)	*/
	u_char	dc_wt_delay_time_0;	/*   "     ""    "   (LSB)	*/
	u_char	dc_rew		: 1,	/* Report early warning.	*/
	        dc_rbo		: 1,	/* Recover buffer order.	*/
	 	dc_socf		: 2,	/* Stop on consecutive FM's.	*/
		dc_avc		: 1,	/* Automatic velocity control.	*/
		dc_rsmk		: 1,	/* Report setmarks.		*/
		dc_bis		: 1,	/* Block identifiers supported.	*/
		dc_dbr		: 1;	/* Data buffer recovery.	*/
	u_char	dc_gap_size;		/* Gap size.			*/
	u_char			: 3,	/* Reserved.			*/
		dc_sew		: 1,	/* Sync at early warning.	*/
		dc_eeg		: 1,	/* Enable EOD generation.	*/
		dc_eod_defined	: 3;	/* EOD defined field.		*/
	u_char	dc_buff_sz_at_ew_2;	/* Buff size at early warn (MSB)*/
	u_char	dc_buff_sz_at_ew_1;	/*  "    "   "    "    "   (   )*/
	u_char	dc_buff_sz_at_ew_0;	/*  "    "   "    "    "   (LSB)*/
	u_char	dc_sel_data_comp;	/* Select data compression.	*/
	u_char	dc_reserved;		/* Reserved.			*/
};

/*
 * Page 11 - Medium Partition Parameters
 */

/*
 * Partition Size Descriptor
 */

struct mp_ps_desc {
	u_char	mp_psd1;
	u_char	mp_psd0;
};

#define MEDIUM_PART_BASE_SIZE 6

struct scsi_medium_part {
	struct	scsi_page_header page_header;
	u_char	mp_max_parts;		/* Max. Additional Partitions	*/
	u_char	mp_add_parts;		/* Additional Partitions.	*/
	u_char			: 3,	/* Reserved.			*/
		mp_psum		: 2,	/* Partition Size Unit of Meas.	*/
		mp_idp		: 1,	/* Initiator Defined Partions.	*/
		mp_sdp		: 1,	/* Select Data Partitions.	*/
		mp_fdp		: 1;	/* Fixed Data Partitions.	*/
	u_char	mp_med_fmt;		/* Medium Format Recognition.	*/
	u_char	mp_reserved[2];		/* Reserved.			*/
	struct	mp_ps_desc mp_psdesc[64]; /* Partition size descriptors.*/
};

/*
 * Page 25 - DEC Specific Parameters:
 */
struct scsi_dec_specific {
	struct	scsi_page_header page_header;
	u_char	ds_spd		: 1,	/* Spin-up disable.		*/
				: 7;	/* Reserved.			*/
	u_char	ds_reserved[23];	/* Reserved.			*/
};

/*
 * Page 38 - Read-Ahead Control Parameters:
 */
#define RAC_TABLE_SIZE		0x0f	/* Cache table size.		*/
#define RAC_CACHE_ENABLE	0x10	/* Read-ahead enable bit.	*/
#define RAC_RAMD		0x20	/* RA with mechanical delay.	*/

struct scsi_readahead_control {
	struct	scsi_page_header page_header;
	u_char	ra_table_size	: 4,	/* Cache table size.		*/
		ra_cache_enable	: 1,	/* Read-ahead enable bit.	*/
		ra_ramd		: 1,	/* RA with mechanical delay.	*/
				: 2;	/* Reserved.			*/
	u_char	ra_threshold;		/* Prefetch threshold.		*/
	u_char	ra_max_prefetch;	/* Max. prefetch.		*/
	u_char	ra_max_multiplier;	/* Max. prefetch multiplier.	*/
	u_char	ra_min_prefetch;	/* Min. prefetch.		*/
	u_char	ra_min_multiplier;	/* Min. prefetch multiplier.	*/
	u_char	ra_reserved[8];		/* Reserved.			*/
};

#define MAX_MODE_PAGES_SIZE		255	/* Max mode pages buffer. */
#define PAGES_BUFFER_LENGTH \
	(MAX_MODE_PAGES_SIZE - sizeof(struct mode_page_header))

/*
 * Define Generic Page Structure:
 */
struct scsi_page_params {
	struct mode_page_header mode_header;
	union {
		struct scsi_error_recovery error_page;	/* Page 1 */
		struct scsi_disco_reco reco_page;	/* Page 2 */
		struct scsi_direct_access direct_page;	/* Page 3 */
		struct scsi_disk_geometry geometry_page;/* Page 4 */
		struct scsi_verify_recovery verify_page;/* Page 7 */
		struct scsi_cache_control cache_page;	/* Page 8 */
		struct scsi_cdrom_device device_page;	/* Page D */
		struct scsi_audio_control audio_page;	/* Page E */
		struct scsi_device_config config_page;	/* Page 10 */
		struct scsi_medium_part part_page;	/* Page 11 */
		struct scsi_dec_specific dec_page;	/* Page 25 */
		struct scsi_readahead_control rac_page;	/* Page 38 */
		u_char scsi_mode_pages[PAGES_BUFFER_LENGTH]; /* All Pages */
	} un;
};

/*
 * Defect list returned by READ_DEFECT_LIST command.
 */

struct scsi_defect_hdr {		/* defect list header */
	u_short	reserved;
	u_short	length;
};

struct scsi_bfi_defect {		/* defect in bytes from index format */
	unsigned cyl  : 24;
	unsigned head : 8;
	long	bytes_from_index;
};

struct scsi_bfi_defect_list {		/* defect list, BFI format */
	struct scsi_defect_hdr	header;
	struct scsi_bfi_defect	defect[SCSI_NDEFECT];
};

/*
 * Defect list expected by REASSIGN_BLOCK command (logical block format).
 * This defect list is limited to 1 defect, as that is the only way
 * we use it.
 */
struct scsi_rab_defect_list {		/* defect list for reassign block */ 
	struct	scsi_defect_hdr header;
	u_int 	lba;			/* defect, in logical blk format */
};
