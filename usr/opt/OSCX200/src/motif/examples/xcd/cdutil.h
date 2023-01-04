/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1993 DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  ALL RIGHTS RESERVED
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN DIGITAL
*  EQUIPMENT CORPORATION OR ITS THIRD PARTY SUPPLIERS  
*  
*  DIGITAL EQUIPMENT CORPORATION AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND DIGITAL EQUIPMENT CORPORATION EXPRESSLY DISCLAIMS ALL IMPLIED 
*  WARRANTIES, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF 
*  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*  
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/

/*
 * File:	cdutil.h
 * Date:	January 25, 1991
 *
 * Modified:	December 4, 1993
 *
 * Added definitions and structures necessary for Mode Select of the Audio
 * Control Page which is now used to set the volume.
 *
 * Modified: 	April 4, 1991
 *
 * Modified: 	November 19, 1991
 * 	Removed default volume level definition.  It's now in the UIL file.
 * 
 * Description:
 *	This file contains defines for the utility functions.
 */

/*
 * Common include file used by 'cdplayer' Program.
 */

#include <sys/types.h>
#ifdef __osf__
#include <io/cam/cdrom.h>
#else
#include <sys/cdrom.h>
#endif /* __osf__ */

/*
 * Local Defines:
 */
#define MAXTRACKS CDROM_MAX_TRACK
#define CD_SUCCESS 0
#define FAILURE -1
#define WARNING		1			/* Warning status code.	*/
#define TERMINATE	0
#define CD_FATAL_ERROR	255			/* Fatal error code.	*/
#define	equal(s1,s2)	(strcmp(s1, s2) == 0)	/* Compare two strings.	*/

#define BZERO(dest, size) memset(dest, '\0', size)

/* When a Table of Contents command is issued, the drive fills in both
 * a header structure and the actual TOC.  This structure matches
 * the data that the drive returns.
 */
struct cd_toc_head_and_entries {
	/* The read Table of Contents command prepends the header, so put the
	 * two structures together.
	 */
	struct cd_toc_header cdth;
	struct cd_toc_entry cdte[MAXTRACKS];
};

/*
 * External Declarations:
 */
extern int CdrFd; /* Contains the file descriptor for the device */

#ifdef _NO_PROTO
extern int DoIoctl ( );
extern int StopUnit ( );
extern int EjectUnit ( );
extern int PausePlay ( );
extern int ResumePlay ( );
extern int PreventRemoval ( );
extern int AllowRemoval ( );
extern int PlayTrack ( );
extern int PlayTime ( );
extern int SetAddrFormatMsf ( );
extern int GetTotalTracks ( );
extern int GetPlayPosition ( );
extern int GetTOCHeader ( );
extern int SetVolume ( );
extern int GetPlaybackStatus ( );
extern int GetTOC ( );
extern void set_fd ( );
#else /* _NO_PROTO */
extern int DoIoctl ( int cmd , caddr_t argp , caddr_t msgp );
extern int StopUnit ( void );
extern int EjectUnit ( void );
extern int PausePlay ( void );
extern int ResumePlay ( void );
extern int PreventRemoval ( void );
extern int AllowRemoval ( void );
extern int PlayTrack ( int starting_track , int ending_track );
extern int PlayTime ( union cd_address starting_time , union cd_address ending_time );
extern int GetTotalTracks ( void );
extern int GetPlayPosition ( struct cd_subc_information *sci );
extern int GetTOCHeader ( struct cd_toc_header *th );
extern int SetVolume ( int left_channel_volume , int right_channel_volume );
extern int GetTOC ( struct cd_toc_head_and_entries *toc_buffer );
extern void set_fd ( int fd );
#endif /* _NO_PROTO */

/*
 * Mode Sense/Mode Select Definitions:
 */
		/* Values for Page Code field. */
#define AUDIO_CONTROL_PAGE	0x0E	/* Audio control page.	*/

		/* Values for Page Control field. */
#define PCF_CURRENT		0x00	/* Return current mode select parms */
#define PCF_SHIFT		6	/* The PCF shift value.		*/

#define MS_PAGE_FORMAT		0x10	/* MS data complies w Page Format */
#define MS_SAVE_MODE_PARMS	1	/* Save mode parameters.	*/
#define MS_DONT_SAVE_PARMS	0	/* Don't save mode parameters.	*/

struct mode_parameter_header {
	u_char	mph_data_length;	/* Mode data length.		[0] */
	u_char	mph_medium_type;	/* Medium type.			[1] */
	u_char	mph_device_specific;	/* Device specific information.	[2] */
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

#define MAX_MODE_PAGES_SIZE		255	/* Max mode pages buffer. */
#define PAGES_BUFFER_LENGTH \
	(MAX_MODE_PAGES_SIZE - sizeof(struct mode_page_header))

/*
 * Define Generic Page Structure:
 */
struct scsi_page_params {
	struct mode_page_header mode_header;
	union {
		struct scsi_audio_control audio_page;	/* Page E */
		u_char scsi_mode_pages[PAGES_BUFFER_LENGTH]; /* All Pages */
	} un;
};
