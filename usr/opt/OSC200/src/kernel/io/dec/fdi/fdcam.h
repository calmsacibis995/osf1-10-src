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
/*
static char *rcsid = "@(#)$RCSfile: fdcam.h,v $ $Revision: 1.1.9.3 $ (DEC) $Date: 1993/07/30 18:33:42 $";
*/
#endif


/************************************************************************
 *									*
 *			Copyright (c) 1991 by				*
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/************************************************************************
 *
 * File:        fdcam.h
 * Date:        9-Oct-1991
 * Author:      Roger Morris
 *
 * Description:
 *      FDCAM --  Floppy Driver Common Access Method
 *
 * Modification History:
 *    Version     Date          Who     Reason
 *
 *      1.00    02-May-91       rsm     Creation date.  Just to start the
 *                                      process.
 *
 *              10-Oct-91       rsm     Comment update.
 *
 *      3       10-Oct-91       rsm     Moved some items to fdcamidc.h
 *                                      where they belonged.
 */

#ifndef FDCAM_INCLUDE
#define FDCAM_INCLUDE   1


#define FDCAM_H_VER 5

#define SECTOR_SIZE 512                /* The sector size assumed by this    */
                                       /*  code.                             */
                                       
/* ************************************************************************* */
/* This file describes the FDCAM calling interface.                          */
/*                                                                           */
/* An overview of fdcam can be found in "fdcam_fs.dit"                       */
/*                                                                           */
/* The names of all symbols defined or declared here begin with the string   */
/* "fdcam_".  In addition, all external symbols resulting from the           */
/* fdcam code also begin with "fdcam_".                                      */
/* ************************************************************************* */



/* The following enables inclusion of debugging code.                        */
#define FDCAM_DEBUG 0
/* #define FDCAM_DEBUG 1 */


#if FDCAM_DEBUG
# define CPRINTF(L,A,B,C,D) (fdcam_log_msg((L),(A),(long)(B),(long)(C),\
	(long)(D)))
#else
# define CPRINTF(L,A,B,C,D) (0)
#endif


/* ************************************************************************* */
/*                                                          fdcam_log_msg()  */
/* ************************************************************************* */
/* The following function is to be provided by higher-level code.            */
/* The passed message (format string and three data elements) is to be delt  */
/* with according to it's nature, which is indicated by the mask.  Some of   */
/* the mask bits are defined below.  Others are free to be defined in any    */
/* way.  Typically, messages of type FDCAM_IERR and FDCAM_EERR should be     */
/* logged and reported to the user.                                          */

#define FDCAM_IERR  0x0040            /* Mask for error internal to driver.  */
#define FDCAM_EERR  0x0080            /* Mask for error external to driver.  */
#define FDCAM_TRACE 0x0100            /* Generally-desireable to see tracing.*/
#define FDCAM_TRC1  0x0001            /* Flow-control tracing.               */
#define FDCAM_TRC2  0x0002            /* Setup tracing.                      */
#define FDCAM_TRC4  0x0004            /* command-phase/result-phase tracing. */
#define FDCAM_TRCRW 0x0400            /* read/write tracing.                 */

void fdcam_log_msg( long mask, char* fmt, long o1, long o2, long o3 );

# define EEPRINTF(A,B) (fdcam_log_msg(FDCAM_EERR,(A),(long)(B),0,0))
# define IEPRINTF(A,B) (fdcam_log_msg(FDCAM_IERR,(A),(long)(B),0,0))
# define EECPRINTF(A,B,C,D) (fdcam_log_msg(FDCAM_EERR,(A),(long)(B),(long)(C),\
	(long)(D)))
# define IECPRINTF(A,B,C,D) (fdcam_log_msg(FDCAM_IERR,(A),(long)(B),(long)(C),\
	(long)(D)))

#if FDCAM_DEBUG
# define VPRINTF(A,B)  (fdcam_log_msg(FDCAM_TRC1,(A),(long)(B),0,0))
# define TPRINTF(A,B)  (fdcam_log_msg(FDCAM_TRACE,(A),(long)(B),0,0))
#else
# define VPRINTF(A,B) (0)
# define TPRINTF(A,B) (0)
#endif


/* ************************************************************************* */
/*                                                        struct fdcam_class */
/* ************************************************************************* */
/* The following structure is used to define an FDCAM interface.             */
/*  A pointer to this structure is required as the first parameter by most   */
/*  FDCAM calls.  The entire contents of this structure are private, and     */
/*  may change from implementation to implementation.                        */
struct fdcam_class;

/* ************************************************************************* */
/*                                                           status codes    */
/* ************************************************************************* */
/* The programmer may assume that, when promoted to an ``int'',              */
/*  FDCAM_NORMAL has the value 0, and FDCAM_FAILURE has the value 1, and     */
/*  that all others have a non-zero value.                                   */
/*  Note that there is a static array, ps_listp[], in fdcam.c that is        */
/*  indexed by a value of this type;  If the following is modified, that     */
/*  array should also be modified.                                           */

enum fdcam_status {
    FDCAM_NORMAL = 0,                  /* Successful completion.             */
                                       /*  This status is always returned by */
                                       /*  functions to which a call-back    */
                                       /*  address has been passed; the      */
                                       /*  actual status code will be given  */
				       /*  to the call-back routine.         */
    FDCAM_FAILURE = 1,                 /* General failure.  Exact meaning    */
                                       /*  is situation dependent.           */
    FDCAM_ABORT,                       /* This command was aborted via       */
                                       /*  fdcam_abort() call.               */
    FDCAM_MEDIA_TAMPER,                /* The drive door was opened or the   */
                                       /*  media has been removed or swapped */
                                       /*  since the probe_media() call.     */
                                       /*  Most fdcam calls relating to this */
                                       /*  drive will return this status     */
                                       /*  code until cleared, even if the   */
				       /*  presents of media is not required */
				       /*  for the requested operation.  To  */
				       /*  clear this code, a call to        */
				       /*  fdcam_update().                   */
    FDCAM_PROGRAMMING_ERR,             /* This status indicates a programming*/
                                       /*  error.  Something appears to be   */
                                       /*  uninitialized, probably due to    */
                                       /*  incorrect calling sequence.       */
				       /*  This status is also returned if   */
				       /*  some parameter is out of the      */
                                       /*  specified range.                  */
    FDCAM_INT_PROG_ERR,                /* This indicates that an internal    */
                                       /*  programming error has occured in  */
                                       /*  the FDCAM code itself.            */
    FDCAM_INVALID_SPEC,                /* Some parameter is out of range of  */
                                       /*  the current FDCAM implementation, */
                                       /*  or the current call is not        */
                                       /*  supported in this implementation. */
    FDCAM_NO_MEDIA,                    /* The requested operation could not  */
                                       /*  be performed because there is no  */
                                       /*  media in the drive.  Note that,   */
                                       /*  typically, FDCAM_MEDIA_TAMPER will*/
                                       /*  be returned in such an event.     */
                                       /*  Should the caller reset that flag */
                                       /*  via fdcam_update(), then this     */
                                       /*  code will be returned if media is */
                                       /*  required for the requested op.    */
    FDCAM_WRITE_PROTECT,               /* The requested operation could      */
                                       /*  not be performed because the      */
                                       /*  write-protect flag is set.        */
    FDCAM_READ_ERROR,                  /* A read or verify was unsuccessful  */
                                       /*  due to CRC error.                 */
    FDCAM_FOREIGN_FORMAT,              /* Something relating to the format   */
                                       /*  of the current track was not      */
                                       /*  correct, such as wrong track,     */
                                       /*  sector, sector size, or head      */
                                       /*  number found in the sector-ID     */
                                       /*  field.                            */
    FDCAM_HARDWARE_ERR                 /* Some hardware error has been       */
                                       /*  detected.                         */
};


/* ************************************************************************* */
/* The following specifies some of the more-flexible pin                     */
/* assignments on the FDI cable.  These allow easy description               */
/* of things like media-ID bit assignment.                                   */
/* ************************************************************************* */
#define FDCAM_PIN_NONE  0x0000
#define FDCAM_PIN_1  0x0001
#define FDCAM_PIN_2  0x0002            /* typ:  DENSEL                       */
#define FDCAM_PIN_3  0x0004
#define FDCAM_PIN_4  0x0008
#define FDCAM_PIN_6  0x0010
#define FDCAM_PIN_10 0x0020
#define FDCAM_PIN_12 0x0040
#define FDCAM_PIN_14 0x0080
#define FDCAM_PIN_17 0x0100
#define FDCAM_PIN_27 0x0200
#define FDCAM_PIN_29 0x0400
#define FDCAM_PIN_33 0x0800
#define FDCAM_PIN_34 0x1000
#define FDCAM_PIN_SPARE_A 0x2000
#define FDCAM_PIN_INVALID 0xFFFF
/* ************************************************************************* */
typedef unsigned short FDCAM_PIN;      /* A value composed of the above-     */
                                       /*  defined pins, corresponding to    */
                                       /*  a particular state of some subset */
                                       /*  of these pins.  Normally, this    */
                                       /*  is used in conjunction with a     */
                                       /*  "mask", which would be of the     */
                                       /*  same type.                        */


/* ************************************************************************* */
/*                                                 ***  enum FDCAM_PIN_type  */
/* ************************************************************************* */
/* The following is used to describe the capabilities of the FDCAM           */
/*  implementation to set or sense the state of the pins listed above.       */

enum FDCAM_PIN_type {
    fdcam_PT_READ_ONLY,                /* Pin is readable.                   */
    fdcam_PT_TP_WRITABLE,              /* Pin is writable, driving totem-pole*/
                                       /*  outputs.                          */
    fdcam_PT_OCO_WRITABLE,             /* Pin is writable, driving open-     */
                                       /*  collector outputs.                */
    fdcam_PT_OCO_R_W,                  /* Pin is writable, driving open-     */
                                       /*  collector outputs.  Pin is also   */
                                       /*  readable (when output is driven   */
                                       /*  high, of course).                 */
    fdcam_PT_GND,                      /* Pin is always grounded.            */
    fdcam_PT_VCC,                      /* Pin is tied to +5Vdc               */
    fdcam_PT_FLOATING,                 /* Pin not connected.                 */
    fdcam_PT_UNDEFINED                 /* Nothing can be assumed.            */
};


enum fdcam_xfer_rate {
    FDCAM_XFER_125,
    FDCAM_XFER_150,
    FDCAM_XFER_250,                    /* typ: 3.50DD or 5.25DD              */
    FDCAM_XFER_300,                    /* typ: 5.25DD in a 5.25HD drive      */
    FDCAM_XFER_500,                    /* typ: 3.50HD or 5.25HD              */
    FDCAM_XFER_1000,                   /* typ: 3.50ED                        */
    FDCAM_XFER_2000,
    FDCAM_XFER_5000
};

typedef unsigned long FDCAM_6TIME;     /* Specifies duration in units of     */
                                       /*  microseconds (0.000001 seconds.)  */

/* ************************************************************************* */
/*                                          ***  struct:  fdcam_media_specs  */
/* ************************************************************************* */
/* The following structure is read only.  It provides information specific   */
/* to a particular media of a particular format.                             */

struct fdcam_media_specs {	       /* media-specific */
    char supported;                    /* This entry ignored if zero.        */
    char surf;                         /* This indicates PSN to Cylinder/    */
                                       /*  Head/Sector mapping order.        */
                                       /*  If zero,head number is incremented*/
                                       /*  before cylinder number with       */
                                       /*  increasing PSN number.  If one,   */
                                       /*  all of one surface is mapped prior*/
                                       /*  to starting the next surface.  In */
                                       /*  either case, all sectors on a     */
				       /*  given track are mapped in before  */
                                       /*  proceeding to the next track.     */
    long user_1;                       /* Use for what-ever.                 */
    unsigned short n_surfaces;         /* typ: 2                             */
    unsigned short sectors_p_track;    /* typ: 8, 9, 15, 18, or 36           */
    unsigned short cylinders_p_disk;   /* typ: 40 or 80                      */
    unsigned short bytes_p_sector;     /* typ: 512                           */
    enum fdcam_xfer_rate xfer_rate;    /* typ: FDCAM_XFER_500                */
    char media_id_must_match;          /* If zero, the media-ID bits are     */
                                       /*  ignored when considering this     */
                                       /*  fdcam_media_specs entry for a     */
                                       /*  match.                            */
    FDCAM_PIN media_id;                /* Combination of FDCAM_PIN_??? bits  */
                                       /*  indicating media ID expected for  */
                                       /*  this media type.                  */
                                       /*  These must be a subset of the     */
                                       /*  bits indicated by fdcam_drive_spec*/
                                       /*s::density_select_mask.             */
    FDCAM_PIN density_select;          /* Combination of FDCAM_PIN_??? bits  */
                                       /*  indicating how to set density-    */
                                       /*  select bits for this media type.  */
                                       /*  These must be a subset of the     */
                                       /*  bits indicated by fdcam_drive_spec*/
                                       /*s::density_select_mask.             */
                                       /*  Also, the FDI hardware must be    */
                                       /*  capable of setting these pins     */
                                       /*  to the indicated levels.          */
    char is_mfm;                       /* Non-zero for MFM, zero for FM      */
    char is_perpen;                    /* Non-zero for perpendicular         */
                                       /*  recording format.                 */
    int gap_3_read;                    /* Space between sectors.  -1 for def.*/
    int gap_3_write;                   /* Space between sectors.  -1 for def.*/
    int gap_3_format;                  /* Space between sectors.  -1 for def.*/
    int steps_per_cylinder;            /* Typ: 1 for 80-trk, 2 for 40-trk.   */
    int cyl_0_pos;                     /* This is the position of cylinder 0 */
                                       /*  from physical cylinder 0 (as      */
                                       /*  indicated by the track-00 signal  */
                                       /*  from the drive) in number of step */
                                       /*  pulses.                           */
                                       /*  Typ: 0                            */
    int sector_offset;                 /* The number with which sector       */
                                       /*  numbering begins, as recorded in  */
                                       /*  the SID, within each track.       */
                                       /*  Typ: 0 or 1                       */
    int write_precomp_cyl;             /* -1 to disable.                     */
    long write_precomp_delay;          /* -1 for use default, 0 to disable.  */
                                       /*  This is delay in ns.              */
    unsigned short reduced_w_c_cyl;    /* Cylinder to begin reduced wc on.   */
    int reduced_w_c_current;           /* -1 for use default.                */
    char* diag_name;                   /* Name of media, for diagnostics.    */
};

     
enum fdcam_drive_type {
    FDCAM_DRIVE_TAPE,
    FDCAM_DRIVE_200,
    FDCAM_DRIVE_350,
    FDCAM_DRIVE_525,
    FDCAM_DRIVE_800
};


/* ************************************************************************* */
/*                                            ***  struct fdcam_drive_specs  */
/* ************************************************************************* */
/* The following structure is read-only.  It provides information specific   */
/* to a particular drive.                                                    */

struct fdcam_drive_specs {
    enum fdcam_drive_type fdt;         /* The size of the drive.             */
    unsigned short n_surfaces;
    unsigned short max_cylinders;
    FDCAM_6TIME head_step_rate;        /* This is the rate at which the      */
                                       /*  head is to be stepped.  Note that */
                                       /*  a "step" means the smallest unit  */
                                       /*  that the head can move, which is  */
                                       /*  not necessarily one cylinder.     */
                                       /*  If this field is zero, the default*/
                                       /*  value is used.                    */
    FDCAM_6TIME head_step_pw;          /* The head-step pulse width.  If     */
                                       /*  this field is zero, the default   */
                                       /*  is used.                          */
    FDCAM_6TIME head_settle_time;      /* Zero indicates to use default.     */
    FDCAM_6TIME head_load_time;        /* Zero indicates to use default.     */
    FDCAM_6TIME head_unload_time;      /* Zero indicates to use default.     */
    FDCAM_6TIME motor_on_delay;        /* Zero indicates to use default.     */
    FDCAM_6TIME motor_off_delay;       /*                                    */
    FDCAM_6TIME revolution_period;     /* 60000000/rpm.                      */
    FDCAM_PIN density_select_mask;     /* Combination of FDCAM_PIN_??? bits  */
                                       /*  indicating which are the density- */
                                       /*  select pins readable by the drive.*/
                                       /*  Typ:FDCAM_PIN_2 or                */
                                       /*  FDCAM_PIN_2|FDCAM_PIN_6           */
    FDCAM_PIN media_id_mask;           /* Combination of FDCAM_PIN_??? bits  */
                                       /*  indicating which are the media-ID */
                                       /*  pins writable by the drive.       */
    FDCAM_PIN drive_select_mask;       /* Combination of FDCAM_PIN_??? bits  */
                                       /*  indicating which pins are used    */
                                       /*  to control drive-select.          */
                                       /*  Typ:FDCAM_PIN_10 | FDCAM_PIN_12 | */
                                       /*  FDCAM_PIN_14                      */
                                       /*  NOTE:  If this has the value      */
                                       /*  FDCAM_PIN_INVALID (which an       */
                                       /*  implementation may require), then */
                                       /*  "normal" drive selection is used, */
                                       /*  and the drive-select specified in */
                                       /*  the fsb is the drive NUMBER to    */
                                       /*  select, not the bit pattern.      */
    FDCAM_PIN static_control_mask;     /* Combination of FDCAM_PIN_??? bits  */
                                       /*  indicating which pins must be     */
                                       /*  held at a specific state.         */
    FDCAM_PIN static_control_val;      /* Combination of FDCAM_PIN_??? bits  */
                                       /*  indicating the desired state of   */
                                       /*  those pins included in the        */
                                       /*  ``static_control_mask'' field.    */
                                       /*  These pins are asserted by the    */
                                       /*  call to ``fdcam_probe_drive()''.  */
    char can_recal;                    /* 0 == cannot recal drive.           */
                                       /*  1 == can recal drive only if      */
                                       /*        media is present.           */
                                       /*  2 == can always recal drive.      */
    char spare1;
    char spare2;
    int spare3;
};



/* ************************************************************************* */
/*                                           ***  struct fdcam_status_block  */
/* ************************************************************************* */
/* The following structure contains "current" information about the drive.   */
/*  It must be initialized by calling ``fdcam_init_fsb()''.  A separate one  */
/*  of these structures is required for each drive present.                  */

struct fdcam_status_block {
/* PRIVATE: */
    struct fdcam_class* fcp;           /* Pointer to actual FDI interface.   */
                                       /*  Used primarily if multiple FDI    */
                                       /*  interfaces are implemented.       */
    long fdcam_fsbck;

    FDCAM_STATUS_BLOCK_PRIVATE         /* See fdcamidc.h for details.        */

/* PUBLIC, READ-ONLY: */
    FDCAM_PIN drive_select;            /* Initialized in ``fdcam_init_fsb()''*/
                                       /*  with parameter of same name.      */

    char is_present;                   /* In ``fdcam_init_fsb()'', this      */
                                       /*  field is initialized to 0 if      */
                                       /*  ``configured_present'' is 0,      */
                                       /*  otherwise it is set to 1.  If     */
                                       /*  ``configured_present'' is 1       */
                                       /*  (maybe present), then this field  */
                                       /*  may later be set to zero in the   */
                                       /*  fdcam_probe() call.*/

    char configured_present;           /* Initialized in ``fdcam_init_fsb()''*/
                                       /*  with parameter of same name.      */
    struct fdcam_drive_specs* fds;     /* Initialized in ``fdcam_init_fsb()''*/
                                       /*  with ``fdsp'' parameter.          */
    struct fdcam_media_specs**         /* Initialized in ``fdcam_init_fsb()''*/
            avail_fms;                 /*  with parameter of same name.      */
    struct fdcam_media_specs* fms;     /* Initialized in ``fdcam_init_fsb()''*/
                                       /*  to zero, unless ``avail_fms''     */
                                       /*  entry only contains one entry,    */
                                       /*  then ``fms'' is initialized       */
                                       /*  to that one entry.                */
    long read_retry_count;             /*                                    */

    long additional_status;            /* This may contain supplementary     */
				       /*  information following an error    */
                                       /*  condition.                        */

    long next_format_position;         /* This is only used during a format  */
                                       /*  track operation.  Stored here by  */
                                       /*  that operation is the physical    */
                                       /*  position of the first sector      */
                                       /*  to be formatted on the NEXT track */
                                       /*  so as to continue the interleave  */
                                       /*  sequence across track boundaries. */

/** DRIVE STATUS INFO **/
    FDCAM_PIN media_id_pins;           /* State of those pins specified in   */
                                       /*  fdcam_drive_specs::density_select */
				       /*  _mask.                            */
                                       /*                                    */
    char media_present;                /* Zero if media might not be present.*/
    char door_open;                    /*                                    */
    char media_changed;                /*                                    */
    char write_protect;                /*                                    */
    char motor_on;                     /* Zero if motor off or spinning down.*/
                                       /*  ==1 if motor spinning up.         */
                                       /*  ==2 if motor up to speed.         */
    int head_position;                 /* Current track, -1 if unknown.      */

    int interleave;                    /* The interleave factor, as read     */
                                       /*  from the media at media_probe     */
                                       /*  time, or as written during        */
                                       /*  format.  Set to 0 if unknown.     */


/* PUBLIC, USER-WRITABLE: */
    int drive_no;                      /* Initialized in ``fdcam_init_fsb()''*/
                                       /*  with parameter of same name.      */
    void* user_tag;                    /* This can be used for anything.     */
};

/* ************************************************************************* */
/*                                             ***  struct fdcam_format_spec */
/* ************************************************************************* */
/* This structure, along with ``fdcam_media_specs'' and ``fdcam_drive_specs''*/
/*  specifies everything necessary to format media.                          */
/*                                                                           */
/* Note, the following items are specified in a structure of type            */
/*  ``fdcam_media_specs'':                                                   */
/*                                                                           */

struct fdcam_format_spec {
    int gpl;                           /* Gap length -- space between        */
                                       /*  sectors.  -1 to use value from    */
                                       /*  fms struct.                       */
    int filler;                        /* Single byte to fill sector with    */
                                       /*  during formatting. -1 to use      */
                                       /*  default (0xF6).                   */
    int interleave_factor;             /* If <= 0, no interleave is          */
                                       /*  specified (i.e., factor=1).  Else,*/
                                       /*  this specifies the minimum        */
                                       /*  distance, in sectors, between     */
                                       /*  consecutively-numbered sectors.   */
    int first_sector_location;         /* The physical position on the disk  */
                                       /*  where the first sector should be  */
                                       /*  placed.  <= 0 indicates that      */
                                       /*  first sector goes in first        */
                                       /*  physical position.  This number   */
                                       /*  must be less than the number of   */
                                       /*  sectors on the disk.              */
};

/* ************************************************************************* */
/*                                                  ***  typedef:  FDCAM_CB  */
/* ************************************************************************* */
/* Calls to fdcam typically return almost immediately.  The requested        */
/*  command is issued to the FDI hardware or queued until the hardware is    */
/*  ready.  When the command finally completes, a call-back function is      */
/*  called.  This typedef defines the calling interface to this call-back    */
/*  function.                                                                */
/*                                                                           */
/* Calls to fdcam might not return immediately due to implementation         */
/*  limitations.  For example, an implementation may be incapable of         */
/*  queuing up several commands. In such cases, the call-back function       */
/*  will be called before returning.                                         */
/*                                                                           */
/* Whenever such a call-back function is required, a null pointer may be     */
/*  passed instead.  If this is done, then the requested operation is        */
/*  completed prior to returning and the ``stat'' call-back parameter is     */
/*  returned.  If a call-back is specified, the return value will always     */
/*  be FDCAM_NORMAL.  A null call-back pointer can only be specified if      */
/*  calling from non-interrupt level, since a sleep/wait may be required     */
/*  to achieve this.  Conversely, if a non-null call-back pointer is         */
/*  specified, the calling routine should, if necessary, return from         */
/*  interrupt level or sleep so that FDI interrupts may take place,          */
/*  eventually causing the call-back to be called.                           */
/*                                                                           */
/* Also, whenever such a call-back function is required, a user-tag is also  */
/*  asked for, of type ``void*''.  This user tag is ignored by fdcam, and    */
/*  is simply passed along to the call-back function as the first parameter. */
/*                                                                           */

typedef void* FDCAM_CB_TAG;

typedef void (*FDCAM_CB) 
	(
        FDCAM_CB_TAG tag,              /* User's tag returned to him.        */
        enum fdcam_status stat,        /* Zero if success, otherwise error.  */
        struct fdcam_status_block* fsb /* The status block.                  */
	);


struct caller_spec {
    struct fdcam_status_block* fsb;
    FDCAM_CB cs_cb;               
    FDCAM_CB_TAG cs_cb_tag;      
    enum fdcam_status cs_results;
};


/* ************************************************************************* */
/* THE FOLLOWING COMMENTS NEED TO BE UPDATED!!...                            */
/*                                                                           */
/* The following makes entry and exit from FDCAM routines easier to code.    */
/* Upon entry, ``user_init'' should be called.                               */
/* If the all operations are completed and return is immediate, exit should  */
/* comprise of `` return user_return(fcp,error_status); ''                   */
/* If operations are to be completed from an interrupt routine, then the     */
/* main routine should end with `` return user_wait(fcp); '', and the        */
/* final interrupt routine should end with ``user_done(0,fcp,error_status);''*/

void fdcam_user_init
	(
	struct caller_spec* csp,
	struct fdcam_status_block* fsb,
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
	);

enum fdcam_status fdcam_user_wait ( struct caller_spec* csp );

enum fdcam_status fdcam_user_return
	( struct caller_spec* csp, enum fdcam_status results);

void fdcam_user_done
	( struct caller_spec* csp, enum fdcam_status results);

/* ************************************************************************* */
/*                                          ***  function: fdcam_init_fsb()  */
/* ************************************************************************* */
/* The following initializes a structure of type fdcam_status_block.  It     */
/*  does not "install" this structure, access the FDI hardware, or           */
/*  interact with any other fdcam code.  It assumes that the structure       */
/*  contains garbage upon entry.  See above comments in fdcam_status_block   */
/*  declaration for additional information on how the structure is           */
/*  initialized.                                                             */
/*                                                                           */
/* All operations are completed before returning. There is no call-back      */
/*  from this function.  This may be called from interrupt level.            */
/*                                                                           */

enum fdcam_status fdcam_init_fsb(
        struct fdcam_status_block*     /* Structure to initialize.           */
		fsbp, 
	struct fdcam_class* g_fcp,     /* The FDI interface.                 */
        struct fdcam_drive_specs* fdsp,/* Read-only data-block that          */
                                       /*  describes the drive corresponding */
                                       /*  to this fsb.                      */
        struct fdcam_media_specs**
                avail_fms,             /* Null-terminated list of pointers   */
                                       /*  to structures, each describing    */
                                       /*  the media types and formats       */
                                       /*  for this drive.                   */
        int drive_no,                  /* Drive number.  This field is       */
                                       /*  provided for convenience of user. */
        FDCAM_PIN drive_select,        /* The required state of the drive-   */
                                       /*  select lines for selecting this   */
                                       /*  drive.  If drive_select_mask is   */
				       /*  fdam_PIN_INVALID, this            */
				       /*  indicates to use "normal"         */
				       /*  drive-selection mapping as follows*/
                                       /*    drive_select   pins used.       */
                                       /*    0              FDCAM_PIN_10     */
                                       /*    1              FDCAM_PIN_12     */
                                       /*    2              FDCAM_PIN_14     */
                                       /*   other           INVALID          */
                                       /*  Other combinations may be used,   */
                                       /*  however, if for example a         */
                                       /*  "strange" cable is used or an     */
                                       /*  external demultiplexer is used.   */
        char configured_present        /* Used to indicate if drive is       */
                                       /*  present:                          */
                                       /*   0 == drive not present.          */
                                       /*   1 == drive might be present,     */
                                       /*         call to probe necessary.   */
                                       /*   2 == drive is present.           */
);



/* ************************************************************************* */
/*                                       ***  function:  fdcam_psn_to_chs()  */
/* ************************************************************************* */
/* Convert the given Physical Sector Number to the corresponding Cylinder/   */
/*  Head/Sector numbers, or return error if out of range.                    */
/*                                                                           */
/* All operations are completed before returning. There is no call-back      */
/*  from this function.  This may be called from interrupt level.            */
/*                                                                           */
enum fdcam_status fdcam_psn_to_chs(
        struct fdcam_media_specs* fms, /* Designates the geometry.           */
	unsigned long psn,             /* The PSN to be converted.           */
	unsigned short* cylinder,      /* Place to store the resulting       */
                                       /*  cylinder number.                  */
	unsigned short* head,          /* Place to store the resulting       */
                                       /*  surface number.                   */
	unsigned short* sector,        /* Place to store the resulting       */
                                       /*  sector number.                    */
        unsigned short* remainder,     /* Place to store the number of       */
                                       /*  sectors remaining on the specified*/
                                       /*  track after the specified sector. */
                                       /*  Ignored if pointer is zero.       */
	unsigned short* track          /* Place to store logical track       */
                                       /*  number.  Ignored if  pointer is   */
                                       /*  zero.                             */
);
        


/* ************************************************************************* */
/*                                       ***  function:  fdcam_chs_to_psn()  */
/* ************************************************************************* */
/* Convert the given Physical Sector Number to the corresponding Cylinder/   */
/*  Head/Sector numbers, or return error if out of range.                    */
/*                                                                           */
/* All operations are completed before returning. There is no call-back      */
/*  from this function.  This may be called from interrupt level.            */
/*                                                                           */
enum fdcam_status fdcam_chs_to_psn(
        struct fdcam_media_specs* fms, /* Designates the geometry.           */
	unsigned short cylinder,       /* Cylinder, Head, and Sector to be   */
                                       /*  converted.                        */
	unsigned short head,           /*  "                                 */
	unsigned short sector,         /*  "                                 */
	unsigned long* psn             /* Place to store the resulting PSN.  */
);


/* ************************************************************************* */
/*                                       ***  function:  fdcam_initialize()  */
/* ************************************************************************* */
/* Initialize fdcam hardware and software.                                   */
/*                                                                           */
/* This must be the first fdcam_function called.  If called more than once   */
/* for a given FDI interface, fdcam_PROGRAMMING_ERROR is returned.           */
/* The results of not calling this function are unpredictable.               */
/* Upon entry, it is expected that ``*fcpp'' is zero.                        */
/*                                                                           */
/* All operations are completed before returning. There is no call-back      */
/*  from this function.  THIS MUST BE CALLED FROM NON-INTERRUPT LEVEL.       */
/*                                                                           */
enum fdcam_status fdcam_initialize
	(
        struct fdcam_class** fcpp,     /* The address of a structure is      */
                                       /*  assigned to ``*fcpp''.  This      */
                                       /*  same address must be the first    */
                                       /*  parameter passed to all           */
                                       /*  subsequent fdcam calls.           */
	void* interface_number,        /* This parameter is used to specify  */
                                       /*  which FDI interface to initialize.*/
                                       /*  The exact meaning and type of     */
                                       /*  this parameter is implementation  */
                                       /*  specific.  If an implementation   */
                                       /*  only supports one FDI interface,  */
                                       /*  then this parameter is typically  */
                                       /*  ignored.                          */
        void* fd_tag_1,
	struct controller *ctlr	       /* This parameter is used to pass     */
	                               /* along bus information to the fdcam */
	                               /* layer.			     */
        );

/* ************************************************************************* */
/*                                                 int fdcam_check_status()  */
/* ************************************************************************* */
/* Zero is returned if this code has been initialized (via                   */
/* fdcam_initialize() ) and if there are no major hardware or software       */
/* errors.  An error code is returned otherwise.                             */
/*                                                                           */
        
int fdcam_check_status ( struct fdcam_class* fcp );
    

/* ************************************************************************* */
/*                                           ***  function:  fdcam_shutdn()  */
/* ************************************************************************* */
/* Shut down fdcam hardware and software.                                    */
/*                                                                           */
/* Issue shut-down command to controller, unload all heads, stop all spindle */
/*  motors.                                                                  */
enum fdcam_status fdcam_shutdn(
        struct fdcam_class* fcp,
        int force                      /* If non-zero, this command causes   */
                                       /*  an ``fdcam_abort'' on all drives. */
                                       /*  Otherwise, this command waits     */
                                       /*  until all pending operations have */
                                       /*  been completed before proceeding. */
);

/* ************************************************************************* */

typedef void (*FDCAM_ACQUIRE_CB)
	(
        void* tag1,
        /* void* tag2, */
        enum fdcam_status stat,
        struct fdcam_status_block* fsb
        );


/* OSF:  The following cannot be called from interrupt level since it calls  */
/*  xx_alloc() which is not supported at interrupt level.                    */
enum fdcam_status fdcam_acquire_port
	(
        struct fdcam_status_block* fsb,
	FDCAM_ACQUIRE_CB cb,
	void* tag1
	);

void fdcam_release_port
        (
        struct fdcam_status_block* fsb
	);

extern struct fdcam_status_block fdcam_gen_fsb;

/* ************************************************************************* */
/*                                      ***  function:  fdcam_probe_drive()  */
/* ************************************************************************* */
/* Test drive for presents, and perform any static initialization of drive.  */
/*                                                                           */
/*  Initialize ``fsb->is_present'' as per...                                 */
/*              Drive is present:       YES    NO    Cannot be determined.   */
/*                                  +-------------------------------------   */
/*    fsb->configured_present ==  0 |    0     0      0                      */
/*    fsb->configured_present ==  1 |    1     0      1                      */
/*    fsb->configured_present ==  2 |    1     1      1                      */
/*                                                                           */
/*  FDCAM_NORMAL is always returned.                                         */

enum fdcam_status fdcam_probe_drive(
        struct fdcam_status_block* fsb,
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);


        
/* ************************************************************************* */
/*                                      ***  function:  fdcam_probe_media()  */
/* ************************************************************************* */
/* Determine media type and perform any static initialization for that type. */
/*  Initialize ``fsb->fms'' field.  The algorithm used for this is as        */
/*  follows:                                                                 */
/*     Each entry in ``fsb->avail_fms'' is tested, one at a time,            */
/*     until a match is found.  A match is determined by passing the tests:  */
/*     1) If media_id_must_match is non-zero, then the media-ID bits are     */
/*        checked for a match.                                               */
/*     2) Read cylinder zero, side 0.  Check for ability to read the sectors,*/
/*        along with proper sector numbering, sector size, track numbering,  */
/*        and side numbering.                                                */
/*     3) Repeat step 2) for cylinder 2 (skipping cylinder 1).               */
/*     4) Repeat step 2) for each side of the media.                         */
/*     The entries of ``fsh->avail_fms'' are checked in order, unless        */
/*     ``media_id_must_match'' is zero.  In that case, the original ordering */
/*     is maintained except that those entries with matching media-ID bits   */
/*     are checked first, then those entries of lower density are checked in */
/*     order of decreasing density, then entries of higher density are       */
/*     checked.                                                              */

enum fdcam_status fdcam_probe_media(
        struct fdcam_status_block* fsb,
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);

enum fdcam_status fdcam_motor_off(
        struct fdcam_status_block* fsb
);

/* ************************************************************************* */
/*                                        ***  function:  fdcam_set_media()  */
/* ************************************************************************* */
/* This is a "manual" version of ``fdcam_probe_media()''.  Via this function,*/
/*  fdcam is "told" of the current media type.                               */
enum fdcam_status fdcam_set_media(
        struct fdcam_status_block* fsb,/* Designates which drive.            */
        struct fdcam_media_specs* x    /* New media type.                    */
);


/* ************************************************************************* */
/*                                            ***  function:  fdcam_abort()  */
/* ************************************************************************* */
/* Abort the current operation as quickly as possible, and flush the queue   */
/*  of any pending operations.  The call-back is called after calling the    */
/*  call-backs of the aborted operations with a failure code.                */

enum fdcam_status fdcam_abort(
        struct fdcam_status_block* fsb 
);


/* ************************************************************************* */
/*                              ***  function:  fdcam_chk_no_media_tamper()  */
/* ************************************************************************* */
/* This routine does everything that fdcam_update() does, plus...            */
/*                                                                           */
/* Check that there was no possibility of someone changing the media since   */
/*  the last call to this routine.                                           */
/*  Return ``fdcam_NORMAL'' if true, otherwise return ``FDCAM_MEDIA_TAMPER'' */
/*  if there is new media in the drive or ``FDCAM_NO_MEDIA'' if the drive    */
/*  is empty.                                                                */
/*                                                                           */
/* The media_changed flag in the fsb is cleared if there is currently        */
/*  media present.                                                           */

enum fdcam_status fdcam_chk_no_media_tamper(
        struct fdcam_status_block* fsb,
	int autoprobe,                 /* 0 == ignored.                      */
                                       /* 1 == If media might have been      */
                                       /*  swapped, perform media-probe.     */
                                       /* 2 == Always perform media-probe if */
                                       /*  media is present.                 */
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);

int fdcam_quick_tamper_check
	(
        struct fdcam_status_block* fsb 
	);

/* ************************************************************************* */
/*                                           ***  function:  fdcam_update()  */
/* ************************************************************************* */
/* Update those fields in the "DRIVE STATUS INFO" section of the fsb.        */

enum fdcam_status fdcam_update(
        struct fdcam_status_block* fsb,
	int autoprobe,                 /* 0 == ignored.                      */
                                       /* 1 == If media might have been      */
                                       /*  swapped, perform media-probe.     */
                                       /* 2 == Always perform media-probe if */
                                       /*  media is present.                 */
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);

/* ************************************************************************* */
/*                                             ***  function:  fdcam_seek()  */
/* ************************************************************************* */
/* Seek heads to specified cylinder.                                         */

enum fdcam_status fdcam_seek( 
        struct fdcam_status_block* fsb,/* Designates which drive to seek.    */
        long cylinder,                 /* Where to seek to.                  */
                                       /*  Specify -1 to re-seek to current  */
                                       /*  cylinder, specify -2 to           */
                                       /*  recalibrate.                      */
        int whence,                    /* 0 = from current position.         */
                                       /* 1 = inner-to-outer direction.      */
                                       /* 2 = outer-to-inner direction.      */
                                       /* 3 = recalibrate first, then inner- */
                                       /*      to-outer direction.           */
                                       /* 4 = recalibrate first, then outer- */
                                       /*      to-inner direction.           */
        FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);
 
/* ************************************************************************* */
/*                                             ***  function:  fdcam_read()  */
/* ************************************************************************* */
/* Read specified sector to buffer.  Assume buffer is large enough to        */
/*  contain sector.  READ OCCURS ON CURRENT CYLINDER.                        */

enum fdcam_status fdcam_read( 
        struct fdcam_status_block* fsb,
	unsigned short cylinder,
        unsigned short head,
        unsigned short sector,
        unsigned char* buffer,
	struct proc* process,
        FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);
 
        
/* ************************************************************************* */
/*                                         ***  function:  fdcam_read_trk()  */
/* ************************************************************************* */
/* Read specified sector to buffer.  Assume buffer is large enough to        */
/*  contain sector.                                                          */

enum fdcam_status fdcam_read_trk( 
        struct fdcam_status_block* fsb,
        int surface,
        unsigned char* buffer,
	struct proc* process,
	long buffer_len,               /* Max size of buffer.                */
	long* actual_len,              /* The actual length read.  This value*/
                                       /*  will not exceed ``buffer_len''.   */
	long* sector_number_list,      /* Place to store the corresponding   */
                                       /*  sector numbers.  If not possible, */
                                       /*  the value ``-1'' is stored in the */
                                       /*  furst location.                   */
        long snl_len,                  /* The length of the above list       */
                                       /*  (number of entries).              */
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);
 
 
        
 
/* ************************************************************************* */
/*                                            ***  function:  fdcam_write()  */
/* ************************************************************************* */
/* Write specified sector to buffer.  Assume buffer is large enough to       */
/*  contain sector.  WRITE OCCURS ON CURRENT CYLINDER                        */

enum fdcam_status fdcam_write( 
        struct fdcam_status_block* fsb,
	unsigned short cylinder,
        unsigned short head,
        unsigned short sector,
        unsigned char* buffer,
	struct proc* process,
        FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);
 
        
/* ************************************************************************* */
/*                                        ***  function:  fdcam_write_trk()  */
/* ************************************************************************* */
/* Write specified sector to buffer.  Assume buffer is large enough to       */
/*  contain sector.                                                          */
/* Note, the sectors are written in physical, not logical order, starting    */
/*  with the first physical sector on the track.  The corresponding logical  */
/*  order cannot be specified as that is determined at format time.          */

enum fdcam_status fdcam_write_trk( 
        struct fdcam_status_block* fsb,
        int surface,
        unsigned char* buffer,
	struct proc* process,
	long buffer_len,               /* Length of data to write.  This     */
                                       /*  value will be rounded down to a   */
                                       /*  whole number of sectors.          */
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);
 
 
/* ************************************************************************* */
/*                                         ***  function:  fdcam_read_sid()  */
/* ************************************************************************* */
/* Read the sector-ID field from the current track.                          */
        
enum fdcam_status fdcam_read_sid( 
        struct fdcam_status_block* fsb,
	unsigned short surface,
	unsigned short sector,         /* If -1, read any sector.            */
	unsigned short* r_sector,      /* Place to store sector number read. */
	unsigned short* r_cylinder,    /* Place to store cylinder number     */
                                       /*  read.                             */
	unsigned short* r_surface,     /* Place to stroe surface number read.*/
	unsigned short* r_sector_size, /* Place to store sector size read.   */
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);
                                                                             
 
/* ************************************************************************* */
/*                                     ***  function:  fdcam_format_track()  */
/* ************************************************************************* */
/* Format current specified surface of current cylinder.                     */
/*                                                                           */
/* Error codes returned:                                                     */
/*    FDCAM_NORMAL         All went well.                                    */
/*    FDCAM_INVALID_SPEC   One of these parameters out of range:             */
/*                          fsb->fms->xfer_rate                              */
/*    FDCAM_HARDWARE_ERR   Controller chip not responding.                   */
        
enum fdcam_status fdcam_format_track(
        struct fdcam_status_block* fsbp,
	unsigned short cylinder,       /* Cylinder to format.                */
	unsigned short surface,        /* Surface to format.                 */
	struct fdcam_format_spec* ffsp,/* Format specification.  Note, the   */
                                       /*  complete specification includes   */
                                       /*  this structure along with         */
                                       /*  ``*fsbp''.                        */
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);
 
        
/* ************************************************************************* */
/*                                                  ***  function:  eject()  */
/* ************************************************************************* */
/* Eject floppy from drive.                                                  */

enum fdcam_status fdcam_eject( 
        struct fdcam_status_block* fsb 
);
 
        
        
/* ************************************************************************* */
/*                                         ***  function:  fdcam_write_at()  */
/* ************************************************************************* */
/* Perform seek and then write specified sector to buffer.  Assume buffer    */
/* contains one sector.                                                      */

enum fdcam_status fdcam_write_at( 
        struct fdcam_status_block* fsb,
        unsigned short cylinder,
        unsigned short head,
        unsigned short sector,
        unsigned char* buffer,
	struct proc* process,
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);
 

/* ************************************************************************* */
/*                                          ***  function:  fdcam_read_at()  */
/* ************************************************************************* */
/* Perform seek and then read specified sector to buffer.  Assume buffer     */
/* is large enough to contain sector.                                        */

enum fdcam_status fdcam_read_at( 
        struct fdcam_status_block* fsb,
        unsigned short cylinder,
        unsigned short surface,
        unsigned short sector,
        unsigned char* buffer,
	struct proc* process,
	int allow_mismatch,            /* If non-zero, the read will be      */
                                       /*  successful even if the cylinder   */
                                       /*  or head numbers do not match.     */
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);
 
        

        
        
/* ************************************************************************* */
/*                                     ***  function:  fdcam_write_at_psn()  */
/* ************************************************************************* */
/* Perform seek and then write specified sector(s) to buffer.                */

enum fdcam_status fdcam_write_at_psn( 
        struct fdcam_status_block* fsb,
	unsigned long psn,             /* The sector to begin write at.      */
	unsigned long nsec,            /* The number of sectors to write.    */
        unsigned char* buffer,         /* The buffer to write.               */
	struct proc* process,
	int sw_intl,                   /* Software interleave.  typ=1.       */
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);
 

/* ************************************************************************* */
/*                                      ***  function:  fdcam_read_at_psn()  */
/* ************************************************************************* */
/* Perform seek and then read specified sector(s) to buffer.  Assume buffer  */
/* is large enough to contain sector.                                        */

enum fdcam_status fdcam_read_at_psn( 
        struct fdcam_status_block* fsb,
	unsigned long psn,             /* The sector to begin read at.       */
	unsigned long nsec,            /* The number of sectors to read.     */
        unsigned char* buffer,         /* The buffer to read.                */
	struct proc* process,
	int sw_intl,                   /* Software interleave.  typ=1.       */
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);
 

/* ************************************************************************* */
/*                                    ***  function:  fdcam_verify_at_psn()  */
/* ************************************************************************* */
/* Perform seek and then read specified sector(s) and compare with the       */
/*  buffer.  Return fdcam_NORMAL if everything checks.                       */

enum fdcam_status fdcam_verify_at_psn( 
        struct fdcam_status_block* fsb,
	unsigned long psn,             /* The sector to begin read at.       */
	unsigned long nsec,            /* The number of sectors to read.     */
        unsigned char* buffer,         /* The buffer to read.                */
	struct proc* process,
	int sw_intl,                   /* Software interleave.  typ=1.       */
	FDCAM_CB cb,
	FDCAM_CB_TAG cb_tag
);
 

        
void fdcam_intr ();

#endif /* FDCAM_INCLUDE */
