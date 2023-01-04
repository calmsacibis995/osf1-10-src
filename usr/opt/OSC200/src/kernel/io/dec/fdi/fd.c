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
static char *rcsid = "@(#)$RCSfile: fd.c,v $ $Revision: 1.1.10.3 $ (DEC) $Date: 1993/07/30 18:33:14 $";
#endif

/************************************************************************
 *                                                                      *
 *                      Copyright (c) 1991 by                           *
 *              Digital Equipment Corporation, Maynard, MA              *
 *                      All rights reserved.                            *
 *                                                                      *
 *   This software is furnished under a license and may be used and     *
 *   copied  only  in accordance with the terms of such license and     *
 *   with the  inclusion  of  the  above  copyright  notice.   This     *
 *   software  or  any  other copies thereof may not be provided or     *
 *   otherwise made available to any other person.  No title to and     *
 *   ownership of the software is hereby transferred.                   *
 *                                                                      *
 *   The information in this software is subject to change  without     *
 *   notice  and should not be construed as a commitment by Digital     *
 *   Equipment Corporation.                                             *
 *                                                                      *
 *   Digital assumes no responsibility for the use  or  reliability     *
 *   of its software on equipment which is not supplied by Digital.     *
 *                                                                      *
 ************************************************************************/

/************************************************************************
 *
 * File:        fd.c
 * Date:        07-Oct-1991
 * Author:      Roger Morris
 *
 * Note:        For complete documentation, see header files fdi.h, fdcam.h,
 *              fdcamreg.h, and fdcamidc.h, and DITROFF files fd_fs.h
 *              and fdcam_fs.h.  Redundant documentation is kept to a minimum.
 *
 * Modification History:
 *    Version     Date          Who     Reason
 *
 *      1.00    12-May-91       rsm     Creation date.  Just to start the
 *                                      process.
 *
 *      1.01    04-Sep-91       rsm     Caused DEVIOCGET call to reset
 *                                       the 'tamper' bit, thus allowing
 *                                       SoftPC to work when floppy is
 *                                       changed.
 *
 *      1.02    18-Sep-91       rsm     Comment changes.
 *
 *      1.03    08-Oct-91       rsm     Fix track-80 problem and LUN-2 problem.
 *
 *              10-Oct-91       rsm     comment update.
 *
 *              10-Oct-91       rsm     Fixed error condition that corrupted
 *                                      memory when device opened with no
 *                                      media in the drive.
 *
 *      2       10-Oct-91       rsm     moved some things from fdcam.h to
 *                                      fdcamidc.h to make cleaner, and now
 *                                      include fdcamidc.h here.
 *
 *      4       15-Oct-91       rsm     Improved efficiency for tiny read
 *                                      requests.
 *
 *      5       05-Feb-91       rsm     Working on OSF port.
 */

#define FD_VER 10

#ifdef _NO_PROTO                       /* ++++++                             */
# undef _NO_PROTO
#endif

/************************* Include Files ********************************/

#include <sys/signal.h>               /* Required by user.h, which is       */
                                       /*  included by proc.h.               */
#include <vm/vm_kern.h>
#include <sys/ioctl.h>
#include <sys/disklabel.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/buf.h>
#include <sys/proc.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <io/common/devdriver.h>
#include <io/common/pt.h>
#include <vm/pmap.h>
#include <kern/kalloc.h>

#include <io/dec/fdi/fdcamidc.h>
#include <io/dec/fdi/fdcam.h>
#include <io/dec/fdi/fdi.h>

#if FDCAM_DEBUG
#include "fdioctst.h"
#endif

/************************* Local Defines ********************************/

#define OFFSET_MASK (SECTOR_SIZE-1)    /* Sector-size mask.                  */

                                       /* Evaluates to non-zero if specified */
                                       /*  dev is for a 'very raw' device.   */
#define DEV_TO_VERY_RAW(D) ((minor(D)&0xFC)==0x40?1:0)

#define NFDI 1			       /* Number of interfaces supported     */
#define N_LUN 2                        /* Number of drives supported.        */

                                       /* Derives drive number from dev.     */
#define DEV_TO_LUN(D) (DEV_TO_VERY_RAW(D) ? minor(D)&0x03 : (minor(D)>>3&0x03))

#define VERY_RAW_LUN_TO_MINOR(L) (((L)&0x03)<<4)

#define N_PRT 8                        /* Number of partitions supported,    */
                                       /*  not including our internal pseudo */
                                       /*  partition ``VERY_RAW_PRT'', which */
                                       /*  means don't use partitions.       */

#define VERY_RAW_PRT N_PRT             /* Partition number of our internal   */
                                       /*  pseudo partition.                 */

#define N_PRT_TOTAL (N_PRT+1)          /* Number of partitions supported,    */
                                       /*  including the internal pseudo     */
                                       /*  partition ``VERY_RAW_PRT''.       */

                                       /* Derive partition number from dev.  */
#define DEV_TO_PRT(D) (DEV_TO_VERY_RAW(D) ? VERY_RAW_PRT : minor(D)&0x07)

/* Opening the following devs will enable/disable debugging cprintf()s.      */
#ifdef DEV_SPECIAL
# undef DEV_SPECIAL
#endif

#if FDCAM_DEBUG
                                       /* Returns non-zero if the specified  */
                                       /*  dev is a special device, the      */
                                       /*  opening of which enables all      */
                                       /*  debugging cprintf()s.             */
# define DEV_SPECIAL_ON(D) (minor(D)==0x44)
                                       /* Returns non-zero if the specified  */
                                       /*  dev is a special device, the      */
                                       /*  opening of which enables selected */
                                       /*  debugging cprintf()s.  Also,      */
                                       /*  reading from this devices will    */
                                       /*  return the entries in the fdcam   */
                                       /*  diag log.                         */
# define DEV_SPECIAL_TRACE(D) (minor(D)==0x45)
                                       /* Returns non-zero if the specified  */
                                       /*  dev is a special device, the      */
                                       /*  opening of which disables all     */
                                       /*  debugging cprintf()s.             */
# define DEV_SPECIAL_OFF(D) (minor(D)==0x46)
                                       /* Returns non-zero if the specified  */
                                       /*  dev is one of the above special   */
                                       /*  devices.                          */
# define DEV_SPECIAL(D) ((minor(D)&0xFC)==0x44)

# define DEV_SPECIAL_RST(D) (minor(D)==0x47)

#else

# define DEV_SPECIAL_ON(D)    (0)
# define DEV_SPECIAL_TRACE(D) (0)
# define DEV_SPECIAL_OFF(D)   (0)
# define DEV_SPECIAL(D)       (0)
# define DEV_SPECIAL_RST(D)   (0)

#endif


#ifndef PROBE_FAILURE
# define PROBE_FAILURE 0
# define PROBE_SUCCESS 1
#endif

extern int atintr_level;	/* used by tcheck_ba() to see if we are in 
				   the interrupt level */

/*
 * Default partition tables to be used of the
 * disk does not contain a parition table.
 */
struct pt fd_rx26_sizes = {
    0,
    0,
    { { -1, 0 },	/* A=blk 0 thru end (5759 for extra density) */
      {  0, 0 },
      { -1, 0 },	/* C=blk 0 thru end (5759 for extra density) */
      {  0, 0 },
      {  0, 0 },
      {  0, 0 },
      {  0, 0 },
      {  0, 0 }, }
};

#define DO_DISKLABEL 1


static struct buf thomas;
#define VALID_CHECK av_forw
#define T_OFFSET(bp) (*(long*)&((bp)->av_back))

#define SINTL_ENA 1                    /* 0 == Disable software interleave.  */
                                       /* 1 == Enable software interleave if */
                                       /*  media is not interleaved.         */

#define RBUF_ENA 2                     /* 0 == Disable read buffering        */
                                       /* 1 == Enable read buffering         */
                                       /* 2 == Disable read buffering if it  */
                                       /*  appears that the media's interl.  */
                                       /*  is greater than 1.                */
#define RBUF_CLOSE 0                   /* 1 == Free buffer space when device */
                                       /*  is closed.  0 == Once acquired,   */
                                       /*  don't free it unless necessary to */
                                       /*  change its size.                  */
#define RBUF_CYL 0                     /* 1 == buffer cylinder, 0 == buffer  */
                                       /*  track.                            */

/* ************************************************************************* */

#if FDCAM_DEBUG

# define FDCAM_TRC_ENABLE 0
/* #define FDCAM_TRC_ENABLE 1 */

# if FDCAM_TRC_ENABLE
static char * 
x_sprintf(char* dest, char* fmt, long o1, long o2, long o3)
{
    long* op = &o1;
    long foo = 0;
    char stack[20];
    char* stack_end = stack+20;
    char* sp = stack;

    while (*fmt) {
        if (*fmt == '%') {
            long l = 0;
            long lead_zero = *++fmt == '0';
            while (*fmt >= '0' && *fmt <= '9')
                l = l * 10 + *fmt++ - '0';
            if (*fmt == 'l' || *fmt == 'L')
                fmt++;
            if (*fmt == 'd' || *fmt == 'D' || *fmt == 'x' || *fmt == 'X' ||
		*fmt == 'u' || *fmt == 'U') {
                int minus = (*fmt == 'd' || *fmt == 'D') && *op < 0;
                unsigned long* up = (unsigned long*)op;
                if (minus)
                    *op *= -1;

                if (*fmt == 'x' || *fmt == 'X') {
                    do {
                        *sp++ = *up % 16 + '0';
                        if (sp[-1] > '9')
                            sp[-1] += 'a' - ('9'+1);
                        *up /= 16;
		    } while (*up != 0);
		} else {
                    do {
                        *sp++ = *up % 10 + '0';
                        *up /= 10;
		    } while (*up != 0);
		}
                while (sp-stack < l && sp < stack_end)
                    *sp++ = lead_zero ? '0' : ' ';
                if (minus)
                    *dest++ = '-';
                while (sp > stack)
                    *dest++ = *--sp;
                if (op == &o1)
                    op = &o2;
                else if (op == &o2)
                    op = &o3;
                else
                    op = &foo;
	    } else if (*fmt == 's') {
                char* xp = (char*)*op;
                while (*xp)
                    *dest++ = *xp++;
                if (op == &o1)
                    op = &o2;
                else if (op == &o2)
                    op = &o3;
                else
                    op = &foo;
	    } else
                *dest++ = *fmt++;
            fmt++;
	} else
            *dest++ = *fmt++;
    }
    *dest = 0;
    return dest;
}

# define FDCAM_TRC_SIZE 6000
/* #define FDCAM_TRC_SIZE 200 */

static char fdcam_trcb[FDCAM_TRC_SIZE];
static char* fdcam_trcp = fdcam_trcb;
static char* fdcam_trce = fdcam_trcb+FDCAM_TRC_SIZE;
static char* fdcam_trcpx = fdcam_trcb;

static void 
fdcam_trc(int mask, char* fmt, long o1, long o2, long o3)
{
    int s;
    char* ep;
    char* bp;

    s = splhigh();

    if (!fmt) {
        fdcam_trcp = fdcam_trcb;
        fdcam_trcpx = fdcam_trcb;
        *fdcam_trcp = 0;
        splx(s);
        return;
    }

    ep = x_sprintf(fdcam_trcp, fmt, o1, o2, o3);
    bp = fdcam_trcp;

    if (ep > fdcam_trce-150) {
        fdcam_trcp = fdcam_trcb;
        fdcam_trcpx = ep;
    } else
        fdcam_trcp = ep;

    splx(s);
}

# endif

#else
# define FDCAM_TRC_ENABLE 0
#endif

/* ************************************************************************* */

unsigned long fd_trigger = 0x0000;
unsigned long fd_cbuf    = 0x0000;
unsigned long fd_foo     = 0x0000;
/* unsigned long fdcam_debug_mask = 0x0000; */       /* +++RSM+++ */
/*unsigned long fdcam_debug_mask = 0x07c6;*/
unsigned long fdcam_debug_mask = 0x0;

void 
fdcam_log_msg(long mask, char* fmt, long o1, long o2, long o3)
{
    if (mask & (FDCAM_IERR|FDCAM_EERR)) {
        if (!fmt || strlen(fmt) > 100) {
            printf("fd: internal fmt error\n");
	} else {
            char buf[150], *bufptr;
	    char *strcpy();

            buf[0] = 0;

	    /* strcpy will return pointer to next location of dst */
	    bufptr = buf;
            if (FDCAM_IERR & mask)
                bufptr = strcpy(buf, " fd internal driver error: ");
            if (FDCAM_EERR & mask)
                bufptr = strcpy(buf, " fd failure: ");
            bufptr = strcpy(bufptr,fmt);
            strcpy(bufptr,"\n");

            printf(buf, o1, o2, o3);
	}
    } else if (fmt && (mask & fdcam_debug_mask)) {
         dprintf(fmt,o1,o2,o3);
         dprintf(" ");
     }

#if FDCAM_TRC_ENABLE
    fdcam_trc(mask, fmt, o1, o2, o3);
#endif
}

/* ************************************************************************* */

void 
fdintr(int foo)
{ 
    VPRINTF("(fdintr)", 0);
    fdcam_intr();
}


caddr_t		fd_csr[] = { 0 };
struct controller *fd_info[NFDI];
struct device *fd_dinfo[NFDI * N_LUN];

/* ************************************************************************* */
/*                                     DEVICE-DESCRIPTION TABLES             */
/* ************************************************************************* */


static struct fdcam_drive_specs fds_rx26 = {
    FDCAM_DRIVE_350,                   /* enum fdcam_drive_type fdt;         */
    2,                                 /* unsigned short n_surfaces;         */
    80,                                /* unsigned short max_cylinders;      */
    3500,                              /* FDCAM_6TIME head_step_rate;        */
    0, /* can't alter duty cycle */    /* FDCAM_6TIME head_step_pw;          */
    0, /* inc. into load time.   */    /* FDCAM_6TIME head_settle_time;      */
    29000,                             /* FDCAM_6TIME head_load_time;        */
    112000,                            /* FDCAM_6TIME head_unload_time;      */
    1000000,                           /* FDCAM_6TIME motor_on_delay;        */
    4000000,                           /* FDCAM_6TIME motor_off_delay;       */
    200000,                            /* FDCAM_6TIME revolution_period;     */

    FDCAM_PIN_2|FDCAM_PIN_6,           /* FDCAM_PIN density_select_mask;     */
    FDCAM_PIN_17|FDCAM_PIN_27,         /* FDCAM_PIN media_id_mask;           */
    FDCAM_PIN_INVALID, /* Normal */    /* FDCAM_PIN drive_select_mask;       */
    FDCAM_PIN_NONE,                    /* FDCAM_PIN static_control_mask;     */
    FDCAM_PIN_NONE,                    /* FDCAM_PIN static_control_val;      */
    2,                                 /* char can_recal;                    */
    0,                                 /* char spare1;                       */
    0,                                 /* char spare2;                       */
    0                                  /* int spare3;                        */
};

static struct fdcam_media_specs fms_350dd = {
    1,                                 /* char supported;                    */
    0,                                 /* char surf;                         */
    FD_MT_350DD,
    2,                                 /* unsigned short n_surfaces;         */
    9,                                 /* unsigned short sectors_p_track; */
    80,                                /* unsigned short cylinders_p_disk;   */
    SECTOR_SIZE,                       /* unsigned short bytes_p_sector;     */
    FDCAM_XFER_250,                    /* enum fdcam_xfer_rate xfer_rate;    */
    1,                                 /* char media_id_must_match;          */
    (FDCAM_PIN_17|FDCAM_PIN_27),       /* FDCAM_PIN media_id;                */
    0,                                 /* FDCAM_PIN density_select;          */
    1,                                 /* char is_mfm;                       */
    0,                                 /* char is_perpen;                    */
    -1,                                /* int gap_3_read;                    */
    -1,                                /* int gap_3_write;                   */
    -1,                                /* int gap_3_format;                  */
    1,                                 /* int steps_per_cylinder;            */
    0,                                 /* int cyl_0_pos;                     */
    1,                                 /* int sector_offset;                 */
    0,                                 /* int write_precomp_cyl;             */
    -1,                                /* FDCAM_6TIME write_precomp_delay;   */
    0,                                 /* unsigned short reduced_w_c_cyl;    */
    0,                                 /* int reduced_w_c_current;           */
    "350dd",                           /* char* diag_name;                   */
};

static struct fdcam_media_specs fms_350hd = {
    1,                                 /* char supported;                    */
    0,                                 /* char surf;                         */
    FD_MT_350HD,
    2,                                 /* unsigned short n_surfaces;         */
    18,                                /* unsigned short sectors_p_track;    */
    80,                                /* unsigned short cylinders_p_disk;   */
    SECTOR_SIZE,                       /* unsigned short bytes_p_sector;     */
    FDCAM_XFER_500,                    /* enum fdcam_xfer_rate xfer_rate;    */
    1,                                 /* char media_id_must_match;          */
    FDCAM_PIN_17,                      /* FDCAM_PIN media_id;                */
    FDCAM_PIN_6,                       /* FDCAM_PIN density_select;          */
    1,                                 /* char is_mfm;                       */
    0,                                 /* char is_perpen;                    */
    -1,                                /* int gap_3_read;                    */
    -1,                                /* int gap_3_write;                   */
    -1,                                /* int gap_3_format;                  */
    1,                                 /* int steps_per_cylinder;            */
    0,                                 /* int cyl_0_pos;                     */
    1,                                 /* int sector_offset;                 */
    0,                                 /* int write_precomp_cyl;             */
    -1,                                /* FDCAM_6TIME write_precomp_delay;   */
    0,                                 /* unsigned short reduced_w_c_cyl;    */
    0,                                 /* int reduced_w_c_current;           */
    "350hd",                           /* char* diag_name;                   */
};

#if !FDCAM_DEBUG
static
#endif
struct fdcam_media_specs fms_350ed = {
    1,                                 /* char supported;                    */
    0,                                 /* char surf;                         */
    FD_MT_350ED,
    2,                                 /* unsigned short n_surfaces;         */
    36,                                /* unsigned short sectors_p_track; */
    80,                                /* unsigned short cylinders_p_disk;   */
    SECTOR_SIZE,                       /* unsigned short bytes_p_sector;     */
    FDCAM_XFER_1000,                   /* enum fdcam_xfer_rate xfer_rate;    */
    1,                                 /* char media_id_must_match;          */
    FDCAM_PIN_27,                      /* FDCAM_PIN media_id;                */
    FDCAM_PIN_2,                       /* FDCAM_PIN density_select;          */
    1,                                 /* char is_mfm;                       */
    1,                                 /* char is_perpen;                    */
    -1,                                /* int gap_3_read;                    */
    -1,                                /* int gap_3_write;                   */
    -1,                                /* int gap_3_format;                  */
    1,                                 /* int steps_per_cylinder;            */
    0,                                 /* int cyl_0_pos;                     */
    1,                                 /* int sector_offset;                 */
    0,                                 /* int write_precomp_cyl;             */
    146,                               /* FDCAM_6TIME write_precomp_delay;   */
    0,                                 /* unsigned short reduced_w_c_cyl;    */
    0,                                 /* int reduced_w_c_current;           */
    "350ed",                           /* char* diag_name;                   */
};


static struct fdcam_media_specs* avail_fms[] = {
    &fms_350ed,
    &fms_350hd,
    &fms_350dd,
    0
};

/* ************************************************************************* */

/* The following structure is used to keep track of the current state of a   */
/*  partition.  One instance exists for each partition.  One instatance also */
/*  exists for each drive for the ``VERY_RAW_PRT''.                          */

struct fdp_class {
    char plock;                        /* Lock access to partition.          */
                                       /*  0 == no lock, 1 = temporary lock, */
                                       /*  (until next I/O operation),       */
                                       /*  2 == lock until closeed or        */
                                       /*  specifically unlocked.            */

    struct fd_seek special_address;    /* special_address.mode has the value */
                                       /*  FD_SEEK_NOT_USED if "normal"      */
                                       /*  addressing is to be used.         */

    int p_nopen[2];                    /* [0] used for raw device, [1] for   */
                                       /*  block.                            */
};

/* ************************************************************************* */


enum open_state {
    OS_DISABLED,
    OS_CLOSED,
    OS_OPENED,                         /* opened, but device never accessed. */
    OS_NO_MEDIA,
    OS_TAMPERED,
    OS_MEDIA,
    OS_PROBED,                         /* Media has been probed, fsb->fms    */
                                       /*  initialized, fdp->n_blocks        */
                                       /*  initialized, and current and      */
                                       /*  default partition tables          */
                                       /*  initialized, and lbl initialized  */
                                       /*  from default partition table.     */
    OS_PTBL_READ
};

#if DO_DISKLABEL
# if MAXPARTITIONS<8 || N_PRT<8
#  if MAXPARTITIONS<N_PRT
#   define N_LABEL_PRT (MAXPARTITIONS)
#  else
#   define N_LABEL_PRT (N_PRT)
#  endif
# else
#  define N_LABEL_PRT (8)
# endif
#else
# define N_LABEL_PRT (0)
#endif

/* The following structure is used to keep track of the current state of a   */
/*  drive.  One instance exists for each drive.                             */
struct fd_class {
    struct fdp_class fdp[N_PRT_TOTAL]; /* One for each partition, including  */
                                       /*  our internal "very raw" partition.*/
                                       /*  Invalid if fndelay                */
#if DO_DISKLABEL
    struct disklabel lbl;              /* The OSF disk label.                */
    int wlabel;                        /* non-zero == disklabel write enable.*/
#endif

    struct pt def_pt;                  /* Invalid if fndelay                 */
    struct pt cur_pt;                  /* Invalid if fndelay                 */
                                       /*  All internal computations are     */
                                       /*  based on contents of ``cur_pt'',  */
                                       /*  not on contents of ``lbl'' or     */
                                       /*  ``def_pt''.  If information is    */
                                       /*  obtained from an OSF disk label,  */
                                       /*  ``cur_pt'' must be updated        */
                                       /*  accordingly.                      */
    long n_blocks;                     /* Invalid if fndelay                 */
    struct fdcam_status_block fsb;     /* FDCAM status block for this device.*/
    char dlock;                        /* Lock access to drive.              */
                                       /*  0 == no lock, 1 = temporary lock  */
                                       /*  (until format complete),          */
                                       /*  2 == lock until closeed or        */
                                       /*  specifically unlocked.            */

    char prev_tamper;                  /*                                    */

    enum open_state os;

    int new_media;                     /* Non-zero if media changed since    */
                                       /*  previous drive-open time.         */
                                       /*  If < 0, information not known.    */

    long change_count;                 /* Number of media changes detected   */

    unsigned char* raw_buffer;         /* If opened, points to tmp buffer.   */

    long raw_buffer_block;             /* If non-negative, indicates that    */
                                       /*  ``raw_buffer'' contains up-to-date*/
                                       /*  copy of this block.               */

#if RBUF_ENA
#define CYL_BUF_DISABLED (-99999)
#define CYL_BUF_INVALID  (-99998)
    long cyl_buf_block;                /* If non-negative, indicates that    */
                                       /*  ''cyl_buf'' contains up-to-date   */
                                       /*  copy of the cylinder starting     */
                                       /*  at this block.                    */
                                       /*  If ==CYL_BUF_DISABLED, then cyl   */
                                       /*  caching is disabled.  If          */
                                       /*  == CYL_BUF_INVALID, then the      */
                                       /*  buffer contents are invalid.      */
    long cyl_buf_start;                /* Valid only if cyl_buf_block is     */
                                       /*  non negative.  This is the psn    */
                                       /*  of the first sector within the    */
                                       /*  buffer that is valid.             */
    long cyl_size;                     /* Cylinder size in number of sectors */
                                       /*  (or track size, if !RBUF_CYL).    */
    unsigned char* cyl_buf;            /* If non-zero, points to cyl buffer. */
                                       /*  This space is alloced at probe    */
                                       /*  time, possibly re-alloced at that */
                                       /*  time if a cylinder size changes.  */
                                       /*  This space is freed at close time */
                                       /*  if all partitions for current     */
                                       /*  drive have been closed.           */
                                       /*  Under OSF, the cylinder buf is    */
                                       /*  not re-allocated as the size      */
                                       /*  changes.  Rather, a buffer is     */
                                       /*  allocated at open time that is    */
                                       /*  large enough to contain any       */
                                       /*  cylinder size.                    */

    FDCAM_CB bio_cb;
    FDCAM_CB_TAG bio_cb_tag;
    long bio_psn;
    long bio_nsec;
    long bio_len;
    struct proc* bio_process;
    unsigned char* bio_buffer;

    long bio_bads;                     /* Cylinder caching disabled for PSN  */
                                       /*  within this range.                */
    long bio_bade;
#endif


    int nopen;                         /* Total number of opens.  Note, since*/
                                       /*  fd_close is only called at the    */
                                       /*  time of the last close, this      */
                                       /*  variable can only contain the     */
                                       /*  upper limit on how many times the */
                                       /*  drive might have been opened.     */

    int vr_nopen;                      /* Number of "very raw" opens.        */


    int tc_new_media_ok;               /* Used by tcheck.  Use synchronized  */
                                       /*  by calls to acquire_port and      */
                                       /*  release_port.                     */
    int tc_update;                     /*  "                                 */
    enum open_state tc_need;           /*  "                                 */
    dev_t tc_dev;                      /*  "                                 */
    FDCAM_CB tc_cb;                    /*  "                                 */
    FDCAM_CB_TAG tc_cb_tag;            /*  "                                 */

    struct strategy_class *sc_list;    /* Pointer to a linked-list of free   */
                                       /*  structures.                       */
    int sc_count;                      /* Number in sc_list or in current    */
                                       /*  use.  Just for debugging.         */
};

static unsigned long 
get_partmask(struct fd_class* fd,
	     int is_block)             /* 0 == raw device                    */
 			               /* 1 == block device                  */
{
    int i;
    unsigned long mask = 0;
    for (i = 0 ; i < N_PRT ; i++)
	if (fd->fdp[i].p_nopen[is_block])
	    mask |= 1L << i;
    return mask;
}


static unsigned long 
get_bpartmask(struct fd_class* fd)
{
}

/* ************************************************************************* */

/* The following structure is used to keep track of the current state of the */
/*  interface.  One instatance exists for each floppy interface.  Currently, */
/*  and probably forever, the MAXine system only supports one floppy         */
/*  interface.                                                               */

struct fdi_class {
    struct fd_class fd[N_LUN];         /* One for each drive.                */

    struct fdcam_class* fcp;           /* Pointer to main FDCAM struct.      */

    caddr_t reg;                       /* As provided by fd_probe call.      */
    struct controller* ctlr;           /*  "                                 */

    long max_cyl_size;                 /* The number of sectors per cylinder */
                                       /*  (or per track, if !RBUF_CYL)      */
                                       /*  in the largest supported media.   */
                                       /*  This value calculated at probe    */
                                       /*  time.                             */

    int did_init;                      /* non-zero if successful init.       */

    char st_last_in_error;             /* Non-zero if last call to strategy  */
                                       /*  resulted in an error.             */
                                       /*  This is only used between calls   */
                                       /*  to fdcam_acquire_port() and       */
                                       /*  fdcam_release_port().             */

    char st_already_acquired;          /* Non-zero if strategy should NOT    */
                                       /*  acquire/release port (i.e., nested*/
                                       /*  strategy call via rsblk()).       */
                                       /*  If >= 10, then allow multiple,    */
                                       /*  consecutive (not sub nested)      */
                                       /*  strategy calls, otherwise only    */
                                       /*  only one nested strategy call.    */

    char st_force_very_raw;            /* Force use of very-raw partition    */
                                       /*  regardless of minor number by     */
                                       /*  strategy routine.                 */
};

/* We need one of these structures per interface.  Since we only support one */
/*  interface (in fact, only one device), lets define that structure here.   */
/*  Note that, other than for debugging, this is the only global data used   */
/*  by the code in this file.                                                */

static struct fdi_class fdi0;

/************************************************************************/

/* Following must be called between calls to fdcam_acquire_port() and        */
/*  fdcam_release_port().                                                    */
/* Return non-zero if error.                                                 */
/*   error #13 = INISR error, try call again.                                 */


void fd_bstrategy(struct buf* bp);

static void 
tcheck_ba(FDCAM_CB_TAG tag, enum fdcam_status istat, 
	  struct fdcam_status_block* fsb)
{
    struct fd_class* fdp = tag;
    long rstat;
    long tmp;
    char* msg;
    extern char* readdisklabel();      /* I can't find a prototype for this. */

    /*  We know fdp->os >= OS_MEDIA and fdp->tc_need > OS_MEDIA.  */

    VPRINTF("(tcheck_ba,%d)",(int)istat);

    if (istat || !fsb->fms || !fsb->media_present) {
        fdp->os = OS_NO_MEDIA;
        VPRINTF("(TC err 13, os=%d)",fdp->os);
        (*fdp->tc_cb)(fdp->tc_cb_tag,1,fsb);
        return;
    }

    if (fdp->os < OS_PROBED) {
        int i;

        fdp->n_blocks = fsb->fms->n_surfaces*fsb->fms->sectors_p_track
                * fsb->fms->cylinders_p_disk;

#if DO_DISKLABEL
	fdp->lbl.d_magic = 0;          /* Set to a bad magic number to       */
                                       /*  indicate that these numbers were  */
                                       /*  not read off the disk.            */
	fdp->lbl.d_secperunit = fdp->n_blocks;
	fdp->lbl.d_secpercyl = fsb->fms->n_surfaces*fsb->fms->sectors_p_track;
	fdp->lbl.d_nsectors = fsb->fms->sectors_p_track;
	fdp->lbl.d_npartitions = N_LABEL_PRT;

/* Note, MAXPARTITIONS is size of disklabel::d_partitions[]                  */
        for (i = 0 ; i < MAXPARTITIONS ; i++) {
	    if (i < N_PRT) {
		fdp->lbl.d_partitions[i].p_size =
                    fd_rx26_sizes.pt_part[i].pi_nblocks < 0
                    ?fdp->n_blocks-fd_rx26_sizes.pt_part[i].pi_blkoff
                    :fd_rx26_sizes.pt_part[i].pi_nblocks;
		fdp->lbl.d_partitions[i].p_offset =
                    fd_rx26_sizes.pt_part[i].pi_blkoff;
	    } else {
		fdp->lbl.d_partitions[i].p_size = 0;
		fdp->lbl.d_partitions[i].p_offset = 0;
	    }
	}
#endif

        for (i = 0 ; i < N_PRT ; i++) {
            fdp->cur_pt.pt_part[i].pi_nblocks =
                    fdp->def_pt.pt_part[i].pi_nblocks =
                    fd_rx26_sizes.pt_part[i].pi_nblocks < 0
                    ?fdp->n_blocks-fd_rx26_sizes.pt_part[i].pi_blkoff
                    :fd_rx26_sizes.pt_part[i].pi_nblocks;
            fdp->cur_pt.pt_part[i].pi_blkoff =
                    fdp->def_pt.pt_part[i].pi_blkoff =
                    fd_rx26_sizes.pt_part[i].pi_blkoff;
	}
	fdp->cur_pt.pt_magic = 0;      /* Set to a bad magic number to       */
                                       /*  indicate that these numbers were  */
                                       /*  not read off the disk.            */
	fdp->cur_pt.pt_valid = PT_VALID;

        fdp->os = OS_PROBED;
    }

#if RBUF_ENA
# if RBUF_CYL  /* If buffering cylinders...    */
     fdp->cyl_size = fsb->fms->sectors_p_track*fsb->fms->n_surfaces;
# else         /* Else, if buffering tracks... */
     fdp->cyl_size = fsb->fms->sectors_p_track;
# endif
#endif
        
    if (fdp->os >= fdp->tc_need) {
        VPRINTF("(TC-3 OK, os=%d)",fdp->os);
        (*fdp->tc_cb)(fdp->tc_cb_tag,0,fsb);
        return;
    }

    /*  We know fdp->os == OS_PROBED and fdp->tc_need == OS_PTBL_READ.  */

    if (atintr_level) {
        TPRINTF("(tc read_ptbl:INISR)", 0);
        (*fdp->tc_cb)(fdp->tc_cb_tag,13,fsb);
        return;
    }

    fdp->os = OS_PTBL_READ;            /* Mark partition table as already    */
                                       /*  having been read even though it   */
                                       /*  hasn't been read yet.  This is    */
                                       /*  to prevent recursive infinite     */
                                       /*  loop via rsblk().                 */
#   if FDCAM_DEBUG
    if (fdi0.st_already_acquired)
        IEPRINTF("<RSBLK LOOP fail-%d>",fdi0.st_already_acquired);
#   endif
    fdi0.st_already_acquired = 1;
    fdi0.st_last_in_error = 0;


#if DO_DISKLABEL
    fdp->lbl.d_magic = 0;
    fdi0.st_force_very_raw = 1;
    fdp->fdp[VERY_RAW_PRT].p_nopen[0]++; /* Mark VR partition as open,       */
                                         /*  since readdisklabel() sill      */
                                         /*  check it.                       */
    VPRINTF("<BEGIN READDISKLABEL...>",0);
    msg = readdisklabel(fdp->tc_dev, fd_bstrategy, &fdp->lbl);
    fdp->fdp[VERY_RAW_PRT].p_nopen[0]--;
    fdi0.st_force_very_raw = 0;

    if (msg) {
	VPRINTF("<...END READDISKLABEL,ERR=%s>\n",msg);

	fdp->lbl.d_magic = 0;          /* Make note that we couldn't read    */
                                       /*  the disk label.                   */

	/* ++++++ log(LOG_ERR, "rz%d: %s\n", unit, msg); */
#endif
	fdi0.st_already_acquired = 1;  /* To check for recursive looping,    */
                                       /*  fd_bstrategy (as called by        */
                                       /*  readdisklabel()) will increment   */
                                       /*  this.  We must reset it back to   */
                                       /*  1 for the next call to b_strategy */
                                       /*  from rsblk().                     */

        fdp->fdp[0].p_nopen[0]++;      /* Mark partition 0 as open, since it */
                                       /*  will be read by rsblk().          */
	VPRINTF("<BEGIN RSBLK...>",0);
	rstat = rsblk(fd_bstrategy, fdp->tc_dev, &fdp->cur_pt);
	VPRINTF("<...END RSBLK>",0);

	fdp->fdp[0].p_nopen[0]--;

#if DO_DISKLABEL

        if (!rstat) {
	    /* Let's update our disk label with   */
	    /*  what was read from the ULTRIX     */
	    /*  partition info from the disk.     */
	    int i;
	    fdp->lbl.d_npartitions = N_LABEL_PRT;
	    for (i = 0 ; i < N_LABEL_PRT ; i++) {
		fdp->lbl.d_partitions[i].p_size =
                    fdp->cur_pt.pt_part[i].pi_nblocks =
                    fdp->cur_pt.pt_part[i].pi_nblocks < 0
                    ?fdp->n_blocks-fdp->cur_pt.pt_part[i].pi_blkoff
                    :fdp->cur_pt.pt_part[i].pi_nblocks;
		fdp->lbl.d_partitions[i].p_offset =
                    fdp->cur_pt.pt_part[i].pi_blkoff;
	    }
	}
    } else {
	int i;
	VPRINTF("<...END READDISKLABEL,NO-ERROR>\n",0);
	rstat = 0;

	for (i = 0 ; i < fdp->lbl.d_npartitions && i < N_PRT ; i++) {
            fdp->cur_pt.pt_part[i].pi_nblocks =
		fdp->lbl.d_partitions[i].p_size;
            fdp->cur_pt.pt_part[i].pi_blkoff =
		fdp->lbl.d_partitions[i].p_offset;
	}
	for (; i < N_PRT ; i++) {
            fdp->cur_pt.pt_part[i].pi_nblocks = 0;
            fdp->cur_pt.pt_part[i].pi_blkoff = 0;
	}
	fdp->cur_pt.pt_magic = 0;      /* Set to a bad magic number to       */
                                       /*  indicate that these numbers were  */
                                       /*  not read off the disk.            */
	fdp->cur_pt.pt_valid = PT_VALID;
    }
#endif


    fdi0.st_already_acquired = 0;

    if (rstat && fdi0.st_last_in_error) {
        fdp->os = OS_PROBED;           /* Back off a step since we had io    */
                                       /*  problem while trying to read the  */
                                       /*  table.                            */
        VPRINTF("(TC err 8, os=%d)",fdp->os);
        (*fdp->tc_cb)(fdp->tc_cb_tag,1,fsb);
        fdi0.st_last_in_error = 0;
    } else {
        VPRINTF("(TC OK.b, os=%d)",fdp->os);
        (*fdp->tc_cb)(fdp->tc_cb_tag,0,fsb);
    }
}

static void 
tcheck_aa(FDCAM_CB_TAG tag, enum fdcam_status stat, 
	  struct fdcam_status_block* fsb)
{
    struct fd_class* fdp = tag;
    fdp->prev_tamper = stat;

    if (stat) {
        fdp->raw_buffer_block = -1;
#if RBUF_ENA
        fdp->bio_bads = fdp->bio_bade = 0;
        if (fdp->cyl_buf_block != CYL_BUF_DISABLED)
            fdp->cyl_buf_block = CYL_BUF_INVALID;
#endif
    }

    if (fdp->os < OS_OPENED || fdp->tc_need <= OS_OPENED) {
        IEPRINTF("bad open sync.",fdp->os);
        (*fdp->tc_cb)(fdp->tc_cb_tag,1,fsb);
        return;
    } else if (fdp->os == OS_OPENED || fdp->new_media < 0) {
        fdp->change_count = 0;
        fdp->new_media = stat;
        if (fdp->os == OS_OPENED)
            fdp->os = stat==FDCAM_NO_MEDIA ? OS_NO_MEDIA : OS_MEDIA;
    }

    
    /*  We know fdp->os > OS_OPENED and fdp->tc_need > OS_OPENED.  */

    if (stat == FDCAM_NO_MEDIA) {
        fdp->os = OS_NO_MEDIA;
    } else if (stat == FDCAM_MEDIA_TAMPER) {
        fdp->os = fdp->tc_new_media_ok ? OS_MEDIA : OS_TAMPERED;
        fdp->change_count++;
    } else if (stat != FDCAM_NORMAL) {
        IEPRINTF("bad tamper check.",0);
        fdp->os = OS_TAMPERED;
        (*fdp->tc_cb)(fdp->tc_cb_tag,1,fsb);
        return;
    } else if (fdp->os == OS_NO_MEDIA || fdp->os == OS_TAMPERED) {
        fdp->os = fdp->tc_new_media_ok ? OS_MEDIA : OS_TAMPERED;
    }

    if (fdp->os >= fdp->tc_need) {
        VPRINTF("(TC OK, os=%d)",fdp->os);
        (*fdp->tc_cb)(fdp->tc_cb_tag,0,fsb);
        return;
    }

    if (fdp->os < OS_MEDIA) {
        VPRINTF("(TC-2 FAIL)",0);
        (*fdp->tc_cb)(fdp->tc_cb_tag,1,fsb);
        return;
    }

    if (atintr_level) {
        TPRINTF("(tc before probe_media:INISR)", 0);
        (*fdp->tc_cb)(fdp->tc_cb_tag,13,fsb);
        return;
    }
    

    /*  We know fdp->os >= OS_MEDIA and fdp->tc_need > OS_MEDIA.  */

    if (fdp->os < OS_PROBED)
        fdcam_probe_media(fsb, tcheck_ba, fdp);
    else
        tcheck_ba(fdp, FDCAM_NORMAL, &fdp->fsb);
}

static void 
tcheck(struct fd_class* fdp, enum open_state os_need, int new_media_ok, 
       int update, dev_t dev, FDCAM_CB cb, FDCAM_CB_TAG t)
{
    CPRINTF(FDCAM_TRACE, "\n(tcheck,need=%d,cur=%d,new_ok=%d...)",
            os_need, fdp->os, new_media_ok);

    if (fdi0.st_already_acquired) {
        if (fdi0.st_already_acquired == 2 || fdi0.st_already_acquired >= 10) {
            VPRINTF("(TC LOOP OK, os=%d)",fdp->os);
            (*cb)(t,0,&fdp->fsb);
	} else {
            IEPRINTF("(TC LOOP BAD, os=%d)",fdp->os);
            (*cb)(t,1,&fdp->fsb);
	}
        return;
    }

    if (fdcam_port_is_free(&fdp->fsb))
        IEPRINTF("(TCHECK ERR #1)",0);

    fdp->tc_need = os_need;
    fdp->tc_new_media_ok = new_media_ok || fdp->os==OS_OPENED 
            || fdp->new_media < 0 ;
    fdp->tc_update = update;
    fdp->tc_dev = dev;
    fdp->tc_cb = cb;
    fdp->tc_cb_tag = t;

    fdcam_chk_no_media_tamper(&fdp->fsb, 0, tcheck_aa, fdp);
}

static int tfoo = 3;                   /* 3 == free-for-use                  */
                                       /* 2 == waiting                       */
                                       /* else == returned status.           */

static void 
t2scb(void* ignored, enum fdcam_status stat, struct fdcam_status_block* fsb)
{
#if FDCAM_DEBUG
    if (tfoo == 3)
        IEPRINTF("(TCHECK ERR #2)",0);
#endif
    tfoo = (stat==2 || stat==3) ? 1 : stat;
    wakeup(&tfoo);
}

static int 
tcheck_sync(struct fd_class* fdp, enum open_state os_need, 
	    int new_media_ok, int update, dev_t dev)
{
    int result;

    VPRINTF("(TCS..)",0);

#if FDCAM_DEBUG
    if (fdcam_port_is_free(&fdp->fsb))
        IEPRINTF("(TCHECK ERR #1)",0);
#endif

#if 0  /* This would speed opening a bit, but fdcam_quick_tamper_check()     */
       /*  appears to sometimes return 0 when it should return 1.            */
    if (os_need<=fdp->os && !update && !fdcam_quick_tamper_check(&fdp->fsb))
        return 0;
#endif

#if FDCAM_DEBUG
    if (tfoo != 3)
        IEPRINTF("FD tFOO ERROR\n",0);
#endif
    tfoo = 2;
    tcheck(fdp,os_need,new_media_ok,update,dev,t2scb,&tfoo);

    while (tfoo == 2)
       	sleep((caddr_t)&tfoo, PRIBIO + 1);
    result = tfoo;
    if (result == 13) {                /* INISR ERROR, try again.            */
        tfoo = 2;
        tcheck(fdp,os_need,new_media_ok,update,dev,t2scb,&tfoo);
        while (tfoo == 2)
            sleep((caddr_t)&tfoo, PRIBIO + 1);
        result = tfoo;
    }
    VPRINTF("(..TCS,%d)",result);
    tfoo = 3;
    return result;
}


/************************************************************************/

/* ************************************************************************* */
/*                                                        INITIALIZATION     */
/* ************************************************************************* */


/************************************************************************
 *
 *  ROUTINE NAME:  fd_probe()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This function handles the probe call for the disk driver.
 *
 *      Here, the structure fdi0 is initialized and the fdcam layer is
 *      initialized.
 *
 *  FORMAL PARAMETERS:
 *      reg     - Specifies the SVA control/status registers for the device.
 *      ctlr    - Specifies a pointer to a controller structure.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *      0 - Failure.
 *      1 - Success.
 *
 *  SIDE EFFECTS:
 *      After a successful call to this routine, other fd_xxx routines may
 *       be called.
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

/* A brief note on comment style.  The above comment block is structured as  */
/*  it is at DEC's request.  Use of this style, however, causes redundancy   */
/*  in that the type and names of the function's parameters and of the       */
/*  function itself are duplicated in the comments and at the start of the   */
/*  function.  Below is an examples of a method that might be better.        */

int 
fd_probe(caddr_t reg, struct controller* ctlr)
	                               /* Return:  0 - Failure               */
                                       /*          1 - Success               */
{
    int lun;
    enum fdcam_status stat;
    struct fdcam_media_specs** fmsp;

    fdi0.did_init = 0;
    fdi0.st_already_acquired = 0;
    fdi0.st_force_very_raw = 0;
    fdi0.st_last_in_error = 0;
    fdi0.reg = reg;
    fdi0.ctlr = ctlr;

    fdi0.max_cyl_size = 0;
    for (fmsp = avail_fms ; *fmsp ; fmsp++) {
#       if RBUF_CYL  /* If buffering cylinders...    */
         long tmp = (*fmsp)->sectors_p_track*fsb->fms->n_surfaces;
#       else         /* Else, if buffering tracks... */
         long tmp = (*fmsp)->sectors_p_track;
#       endif
	if (tmp > fdi0.max_cyl_size)
	    fdi0.max_cyl_size = tmp;
    }

    TPRINTF("FDI PROBE TIME\n",0);

                                       /* Initialize fdcam layer.            */
    stat = fdcam_initialize(&fdi0.fcp, reg, &fdi0, ctlr);

    if (stat) {
        IEPRINTF("FDI PROBE FAIL (A,%d).\n",stat);
        return PROBE_FAILURE;
    }

    return PROBE_SUCCESS;
}


/************************************************************************
 *
 *  ROUTINE NAME:  fd_slave()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This function handles the slave call for the disk driver.
 *
 *      No action is performed here.
 *
 *  FORMAL PARAMETERS:
 *      ui      - Specifies the ui device struct.
 *      reg     - Specifies the SVA control/status registers for the device.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *      1 - Success.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/


int 
fd_slave(struct device* ui, caddr_t reg)
{
    int lun = ui->unit;
    struct fd_class* fd = fdi0.fd + lun;
    int prt;
    enum fdcam_status stat;

    if (strcmp(ui->dev_name, "fd") != 0)
	return PROBE_FAILURE;

    CPRINTF(FDCAM_TRACE, "FDI SLAVE TIME, ui=0x%08lx, reg=0x%08lx\n", 
	    (long)ui,(long)reg,0);

    TPRINTF("(fd_slave, unit = %d)", lun);

    /* Initialize each element of fdi0.fd[lun] */
    fd->os = OS_CLOSED;
    
#if DO_DISKLABEL
    fd->lbl.d_magic = 0;           /* For debugging.                     */
    fd->lbl.d_secsize = SECTOR_SIZE;
    fd->wlabel = 0;                /* Disable disklabel writing.         */
#endif
    
    fd->change_count = 0;
    fd->dlock = 0;
    fd->raw_buffer = 0;
    fd->raw_buffer_block = -1;
    fd->sc_list = 0;
    fd->sc_count = 0;
#if RBUF_ENA
    fd->cyl_buf = 0;
    fd->bio_bads = fd->bio_bade = 0;
    fd->cyl_size = 0;
    fd->cyl_buf_block = CYL_BUF_INVALID;
#endif
    fd->nopen = 0;
    for (prt = 0 ; prt < N_PRT_TOTAL ; prt++) {
	fd->fdp[prt].p_nopen[0] = 0;
	fd->fdp[prt].p_nopen[1] = 0;
	fd->fdp[prt].plock = 0;
    }
    
    stat = fdcam_init_fsb
	(
	 &fd->fsb,                  /* struct fdcam_status_block* fsbp,   */
	 fdi0.fcp,                  /* struct fdcam_class* g_fcp,         */
	 &fds_rx26,                 /* struct fdcam_drive_specs* fdsp,    */
	 avail_fms,                 /* struct fdcam_media_specs** avail_fm*/
	 lun,                       /* int drive_no,                      */
	 lun,                       /* drive_select,  (used as LUN since  */
	                            /*  drive_select_mask is INVALID).    */
	 1                          /* char configured_present,           */
	 );
    
    if (stat) {
	fd->os = OS_DISABLED;
	return PROBE_FAILURE;
    }
    
    if (! (stat = fdcam_acquire_port(&fd->fsb,0,0))) {
	stat = fdcam_probe_drive(&fd->fsb,0,0);
	fdcam_release_port(&fd->fsb);
    }
    
    CPRINTF(FDCAM_TRACE, "<slave#%d,stat=%d,is_present=%d>",
	    lun, stat, fd->fsb.is_present);
    
    if (stat || !fd->fsb.is_present) {
	fd->os = OS_DISABLED;
	return PROBE_FAILURE;
    }
    
    fdi0.did_init++;               /* Success if at least one drive      */
                                   /*  initializes.                      */
    
    
    return PROBE_SUCCESS;
}


/************************************************************************
 *
 *  ROUTINE NAME:  fd_attach()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This function handles the attach call for the disk driver.
 *
 *      No action is performed here.
 *
 *  FORMAL PARAMETERS:
 *      ctlr      - Specifies the controller struct.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *      1 - Success.  This should always be returned.
 *      0 - Failure.  This is returned if the fd_probe was unsuccessful, in
 *           which case fd_attach should not have even been called.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

int 
fd_attach(struct controller* ctlr)
{
    CPRINTF(FDCAM_TRACE, "FDI ATTACH TIME, ctlr=0x%08lx\n", (long)ctlr, 0, 0);

    return PROBE_SUCCESS;
}


/* ************************************************************************* */

struct strategy_class {
    struct fdi_class* f;

    long st_blkno;                     /* Used by stratgegy, once port has   */
                                       /*  been acquired.                    */

    unsigned char* st_muaddr;
    struct proc* st_process;

    long cur_dev;
    int cur_lun;                       
    int cur_prt;                      
    struct fd_class *cur_fd;         
    struct fdp_class* cur_fdp;      
    int cur_vr;                    

    struct buf* cur_bp;

    int offset;

    struct strategy_class* next;

    long obcount;			/* Original byte count.  Used for    */
					/*  partial transfers due to end of  */
					/*  disk or partition.  In such cases*/
					/*  bp->bcount is reduced to legal   */
					/*  maximum at the beginning         */
					/*  of the transfer.  The requested  */
					/*  count is needed in case the xfer */
					/*  fails.			     */
};

struct strategy_class* 
new_strategy_class(struct fdi_class* f, dev_t dev,
		   struct buf* bp)      /* Use bp==0 if we only want to      */
                                        /*  initialize the cur_fd element.   */
{
    struct strategy_class* scp;
    int lun = DEV_TO_LUN(dev);
    int prt = DEV_TO_PRT(dev);
    int s;
    struct fd_class *fd = f->fd+lun;
    if (fdi0.st_force_very_raw)
	prt = VERY_RAW_PRT;

    VPRINTF("(NSC,%s)",fd->sc_list?"O":"N");

    if (!f->did_init || lun >= N_LUN || prt >= N_PRT_TOTAL || !fd) {
	CPRINTF(FDCAM_TRACE,"(NSC-FAIL, init=%d,lun=%d,prt=%d)",
		f->did_init,lun, prt);
	return 0;
    }

    s = splhigh();
    if (fd->sc_list) {
        scp = fd->sc_list;
	fd->sc_list = fd->sc_list->next;
	scp->next = 0;
	splx(s);
    } else {
	splx(s);
        VPRINTF("((ALLOC-8, count=%d",++fd->sc_count);
	scp = (struct strategy_class*)kalloc(sizeof(*scp));
        VPRINTF("%s))",scp?"":",*****NULL*****");
    }

    if (scp) {
	scp->cur_fd = fd;
	if (bp) {
	    scp->f = f;
	    scp->cur_dev = dev;
	    scp->cur_bp = bp;
	    scp->offset = 0;
	    scp->obcount= 0;

	    scp->cur_lun = lun;
	    scp->cur_prt = prt;
	    
	    scp->cur_fdp = scp->cur_fd->fdp+scp->cur_prt;
	    scp->cur_vr  = DEV_TO_VERY_RAW(dev) || fdi0.st_force_very_raw;

	    scp->st_muaddr = (unsigned char*)scp->cur_bp->b_un.b_addr;

	    if (! IS_SYS_VA(scp->st_muaddr)) {
		scp->st_process = scp->cur_bp->b_proc;
		if (scp->st_process == 0) {
		    int s = splhigh();
		    IEPRINTF("(bp->b_proc null)",0);
		    
		    scp->next = fd->sc_list;
		    fd->sc_list = scp;
		    
		    scp = 0;
		    splx(s);
		}
	    } else
		scp->st_process = 0;
	}
    }
    return scp;
}

void 
delete_strategy_class(scp)
struct strategy_class* scp;
{
    VPRINTF("<dsc,0x%08lx,",scp);
    VPRINTF("cur_fd=0x%08lx,",scp?scp->cur_fd:0);
    VPRINTF("cur_bp=0x%08lx>",scp?scp->cur_bp:0);
    if (scp && scp->cur_fd) {
	int s = splhigh();
	scp->next = scp->cur_fd->sc_list;
	scp->cur_fd->sc_list = scp;
	splx(s);
    }
    VPRINTF("<end-dsc>",0);
}

/* ************************************************************************* */

#if FDCAM_DEBUG
extern unsigned long fdcam_diag[];
extern unsigned long* fdcam_diag_p;
static unsigned char *diagp;

static char* tracep;
static int trcmode;
#endif


/************************************************************************
 *
 *  ROUTINE NAME:  ll_open()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This is the common block/character open routine.  It is called
 *       by either fd_open() or fd_bopen().
 *
 *      Control is passed to ll_open, the common block/character open
 *      routine.  See comments there for description.
 *
 *  FORMAL PARAMETERS:
 *      dev      - Major/minor device number.  The major number is ignored.
 *      flag     - FNDELAY bit is checked.
 *      is_block - Non-zero if called as block device, zero if character
 *                  device.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *      0        - Open successful
 *
 *      ENXIO    - A Fatal hardware or software problem has been detected
 *                  since probe time.
 *
 *               - A bad minor number was passed.
 *
 *               - Call to open_2_sync() failed.
 *
 *      EWOULDBLOCK - The device was already locked (e.g., currently 
 *                     formatting.)
 *
 *      EIO      - Call to read_ptbl() failed.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

static int 
ll_open(dev_t dev, int flag, int is_block)
{
    int lun = DEV_TO_LUN(dev);
    int prt = DEV_TO_PRT(dev);
    int vr = DEV_TO_VERY_RAW(dev);

    long stat = fdcam_check_status(fdi0.fcp);
#if FDCAM_DEBUG
    extern fdcam_diag_f();
    extern void fdcam_nothing();
    extern int fd_close(dev_t dev, int flag);
#endif

    if (stat) {
        EECPRINTF(
             "\nFDI was shut down, reason #%d-%d-%d.  Reboot to reactivate.\n",
                stat&0xFF, stat>>8&0xFF, stat>>16&0xff);
        fdi0.did_init = 0;
        if (!DEV_SPECIAL_ON(dev))
            return ENXIO;
    }

    CPRINTF(FDCAM_TRACE,"(fd_open,lun=%d,vr=%d,",lun,vr,0);
    CPRINTF(FDCAM_TRACE,"flag=0x%08lx,FNDELAY=0x%08lx)\n", flag,
            FNDELAY, 0);

#if FDCAM_DEBUG
    CPRINTF(FDCAM_TRACE,
	    "<fdcam_log_msg@0x%08lx,fd_close@0x%08lx,fdcam_nothing@0x%08lx,",
	    fdcam_log_msg,fd_close,fdcam_nothing);
    CPRINTF(FDCAM_TRACE,
	    "fdcam_diag_f@0x%08lx,fdcam_intr@0x%08lx,do_fdioctst@0x%08lx>\n",
	    fdcam_diag_f,fdcam_intr,do_fdioctst);
#endif

    if (DEV_SPECIAL(dev)) {
	dprintf("We are opening a floppy special device\n");
        if (DEV_SPECIAL_ON(dev)) {
/*            fdcam_debug_mask = 0xfff;*/
# if FDCAM_TRC_ENABLE
            trcmode = 2;
            tracep = fdcam_trcp + strlen(fdcam_trcp)+1;
            if (tracep >= fdcam_trcpx || tracep >= fdcam_trce)
                {
                trcmode = 1;
                tracep = fdcam_trcb;
                }
            return 0;
# endif
	} else if (DEV_SPECIAL_TRACE(dev)) {
            /* fdcam_debug_mask = 0x0B02; */
/*            fdcam_debug_mask = ~FDCAM_TRC4;*/
#if FDCAM_TRC_ENABLE
            diagp = (unsigned char*)fdcam_diag;
            return 0;
#endif
	} else if (DEV_SPECIAL_OFF(dev)) {
	    dprintf("Turning debugging off\n");
            fdcam_debug_mask = 0x0000;
	} else if (DEV_SPECIAL_RST(dev)) {
            CPRINTF(0, 0, 0, 0, 0);
            return 0;
	}

        return ENXIO;
    } else if (!fdi0.did_init || lun >= N_LUN || prt >= N_PRT_TOTAL) {
        IECPRINTF("(fd_open-FAIL-1,lun=%d,prt=%d)",lun,prt,0);
        return ENXIO;
    } else {
        struct fd_class *fd = fdi0.fd+lun;
        struct fdp_class* fdp = fd->fdp+prt;

        if (fd->os == OS_DISABLED) {
            IEPRINTF("(fd_open-FAIL-2)",0);
            return ENXIO;
	}

        if (fd->dlock || fdp->plock) {
            CPRINTF(FDCAM_TRACE, "(fd_open-DLOCK=%d,PLOCK=%d)",
                    fd->dlock,fdp->plock,0);
            return EWOULDBLOCK;       /* Yup, we don't check FNDELAY here.  */
	}


        if (fd->nopen == 0) {
            int i;
            fd->change_count = 0;
            fd->new_media = -1;
            fd->raw_buffer_block = -1;

#if RBUF_ENA
            if (!fd->cyl_buf) {
		fd->cyl_buf = (unsigned char*)kalloc(
				    SECTOR_SIZE*fdi0.max_cyl_size);
	    }
#endif

            if (fd->raw_buffer) {
                IEPRINTF("(open_2a-errB)",0);
                return EIO;
	    }

	    fd->raw_buffer = (unsigned char*)kalloc(SECTOR_SIZE);

            for (i = 0 ; i < N_PRT_TOTAL ; i++) {
                struct fdp_class* fdp = fd->fdp+i;
                fdp->special_address.mode = FD_SEEK_NOT_USED;
	    }
                                       /* Allocate structure.                */
	    delete_strategy_class(new_strategy_class(&fdi0,dev,0));
	}

        CPRINTF(FDCAM_TRACE, "(O:vr_nopen=%d,nopen=%d,p_nopen=%d)",
                fd->vr_nopen, fd->nopen, fdp->p_nopen[0]+fdp->p_nopen[1]);

        fd->vr_nopen += vr;
        fd->nopen++;
        fdp->p_nopen[is_block]++;

        if (fd->os < OS_OPENED)
            fd->os = OS_OPENED;

        if (flag & FNDELAY) {
            VPRINTF("(fd_open FNDELAY)",0);
            if (!is_block)
                return 0;
	}

        VPRINTF("(fd_open-B)",0);

        if (fdcam_acquire_port(&fd->fsb,0,0)) {
            IEPRINTF("(fd_open-aq,fail)",0);
            fd->vr_nopen -= vr;
            if (--fd->nopen <= 0) {
		kfree(fd->raw_buffer, SECTOR_SIZE);
                fd->raw_buffer = 0;
                fd->raw_buffer_block = -1;
#if RBUF_ENA
                if (fd->cyl_buf_block != CYL_BUF_DISABLED)
                        fd->cyl_buf_block = CYL_BUF_INVALID;
#endif
	    }
            fdp->p_nopen[is_block]--;
            fdi0.did_init = 0;
            return ENXIO;
	}

        VPRINTF("(OP/TC)",0);
        stat = tcheck_sync(fd, vr?OS_PROBED:OS_PTBL_READ, 1, 0, dev);

        fdcam_release_port(&fd->fsb);

        if (fd->os == OS_NO_MEDIA  || stat) {
            VPRINTF("(fd_open-xx,fail)",0);
            fd->vr_nopen -= vr;
            if (--fd->nopen <= 0) {
		kfree(fd->raw_buffer, SECTOR_SIZE);
                fd->raw_buffer = 0;
                fd->raw_buffer_block = -1;
#if RBUF_ENA
                if (fd->cyl_buf_block != CYL_BUF_DISABLED)
                        fd->cyl_buf_block = CYL_BUF_INVALID;
#endif
	    }
            fdp->p_nopen[is_block]--;
            if (fd->os == OS_NO_MEDIA)
                EEPRINTF("No media in drive.",0);
            else if (stat)
                EEPRINTF("Error %d reading ptbl at open.",stat);
            return EIO;
	}

        VPRINTF("(fd_open-PASS)",0);
        return 0;
    }	
}


/************************************************************************
 *
 *  ROUTINE NAME:  fd_bopen()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This function handles the open call for the block disk driver.
 *
 *      Control is passed to ll_open, the common block/character open
 *      routine.  See comments there for description.
 *
 *  FORMAL PARAMETERS:
 *      dev     - Major/minor device number.
 *      flag    - see ll_open().
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *      See ll_open().
 *
 *  SIDE EFFECTS:
 *      See ll_open().
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/


int 
fd_bopen(dev_t dev, int flag)
{
    int stat;

    CPRINTF(FDCAM_TRACE|FDCAM_TRC1,"(bopen...)",0,0,0);
    stat = ll_open(dev, flag, 1);
    CPRINTF(FDCAM_TRACE|FDCAM_TRC1,"(...bopen,s=%d)",stat,0,0);
    return stat;
}

/*****************************************************************************/
/************************************************************************
 *
 *  ROUTINE NAME:  fd_open()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This function handles the open call for the character disk driver.
 *
 *      Control is passed to ll_open, the common block/character open
 *      routine.  See comments there for description.
 *
 *  FORMAL PARAMETERS:
 *      dev     - Major/minor device number.
 *      flag    - see ll_open().
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *      See ll_open().
 *
 *  SIDE EFFECTS:
 *      See ll_open().
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/


int 
fd_open(dev_t dev, int flag)
{
    int stat;
    CPRINTF(FDCAM_TRACE|FDCAM_TRC1,"(copen...,dev=%x)",dev,0,0);
    stat = ll_open(dev, flag, 0);
    CPRINTF(FDCAM_TRACE|FDCAM_TRC1,"(...copen,s=%d)",stat,0,0);
    return stat;
}

static int 
ll_close(dev_t dev, int flag, int is_block)
{
    int lun = DEV_TO_LUN(dev);
    int prt = DEV_TO_PRT(dev);
    struct fd_class *fd = fdi0.fd+lun;
    struct fdp_class* fdp;

    CPRINTF(FDCAM_TRACE|FDCAM_TRC1,"(fd_close,prt=%d)",prt,0,0);

    if (DEV_SPECIAL(dev))
        return 0;

    if (!fdi0.did_init || lun >= N_LUN || prt >= N_PRT_TOTAL
            || fd->os <= OS_CLOSED) {
        IEPRINTF("fd_close,ERR-A,%d",fdi0.did_init);
        return ENXIO;
    }

    fdp = fd->fdp+prt;

    CPRINTF(FDCAM_TRACE, "(C:vr_nopen=%d,nopen=%d,p_nopen=%d)",
            fd->vr_nopen, fd->nopen, fdp->p_nopen[0]+fdp->p_nopen[1]);

    fd->vr_nopen -= DEV_TO_VERY_RAW(dev);

    fd->nopen -= fdp->p_nopen[is_block];
    fdp->p_nopen[is_block] = 0;

    if (fd->nopen < 0) {
        CPRINTF(FDCAM_TRACE, "[too many close]", 0, 0, 0);
        return ENXIO;
    }

    if (fdp->p_nopen[0]+fdp->p_nopen[1] <= 0)
        fdp->plock = 0;

    if (fd->nopen <= 0) {
        VPRINTF("(fdc-all_closed,lun=%d)",lun);

        CPRINTF(0x10, "\n(fdi0.fd[0].fsb@0x%08lx, ...lun_id=%d)",
                &fdi0.fd[0].fsb, fdi0.fd[0].fsb.lun_id, 0);
        CPRINTF(0x10, "\n(fdi0.fd[1].fsb@0x%08lx, ...lun_id=%d)",
                &fdi0.fd[1].fsb, fdi0.fd[1].fsb.lun_id, 0);

        fdcam_motor_off(&fd->fsb);

#if RBUF_ENA && RBUF_CLOSE
        if (fd->cyl_buf_block != CYL_BUF_DISABLED)
            fd->cyl_buf_block = CYL_BUF_INVALID;
        if (fd->cyl_buf) {
	    kfree(fd->cyl_buf, fdi0.max_cyl_size*SECTOR_SIZE);
            fd->cyl_buf = 0;
	}
#endif

        while (fd->sc_list) {
	    struct strategy_class* scp = fd->sc_list;
	    fd->sc_list = fd->sc_list->next;
	    kfree(scp, sizeof(*scp));
	    fd->sc_count--;
	}

	kfree(fd->raw_buffer, SECTOR_SIZE);
        fd->raw_buffer = 0;
        if (fd->os <= OS_TAMPERED)
            fd->os = OS_CLOSED;

        fd->dlock = 0;
        fd->change_count = 0;
#if DO_DISKLABEL
	fd->wlabel = 0;                /* Re-write-protect disklabel.        */
#endif
    }

    return 0;
}


/************************************************************************
 *
 *  ROUTINE NAME:  fd_bclose()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This function handles the close call for the block disk driver.
 *
 *      Control is passed to ll_close(), the common block/character close
 *      routine.  See comments there for description.
 *
 *  FORMAL PARAMETERS:
 *      dev     - see ll_close().
 *      flag    - see ll_close().
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *      See ll_close().
 *
 *  SIDE EFFECTS:
 *      See ll_close().
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/


int 
fd_bclose(dev_t dev, int flag)
{
    return ll_close(dev, flag, 1);
}


/************************************************************************
 *
 *  ROUTINE NAME:  fd_close()
 *
 *  FUNCTIONAL DESCRIPTION:
 *      This function handles the close call for the character disk driver.
 *
 *      Control is passed to ll_close(), the common block/character close
 *      routine.  See comments there for description.
 *
 *  FORMAL PARAMETERS:
 *      dev     - see ll_close().
 *      flag    - see ll_close().
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *      See ll_close().
 *
 *  SIDE EFFECTS:
 *      See ll_close().
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

int fd_close(dev_t dev, int flag)
{
    return ll_close(dev, flag, 0);
}

static struct buf* 
get_bp()
{
    struct buf* bp;
    VPRINTF("((ALLOC-6",0);
    bp = (struct buf*)kalloc(sizeof(*bp));
    bzero(bp, sizeof(*bp));
    VPRINTF("))",0);
    if (!bp || bp->b_flags)
        bp=0, IEPRINTF("OOPS, get_bp() err.\n",0);
    return bp;
}

static void 
free_bp(struct buf* bp)
{
    VPRINTF("(fbp)",0);
    if (bp) {
        VPRINTF("((FREE-7",0);
	kfree(bp, sizeof(*bp));
        VPRINTF("))",0);
        bp = 0;
    }
}


/* The following must not be called at intr level because get_bp() and       */
/* free_bp() are called.                                                     */

static int 
fd_rw(dev_t dev, struct uio* g_uio, int rw)
{
    struct buf* bp;
    int rval;
    struct fd_class *fd = fdi0.fd+DEV_TO_LUN(dev);
    struct fdp_class* fdp;
    int vr = DEV_TO_VERY_RAW(dev);
    int stat;

    CPRINTF(FDCAM_TRCRW,"\n(fd_rw,0x%08lx,uio=(uio_offset=0x%08lx,",dev,
            g_uio->uio_offset,0);
    CPRINTF(FDCAM_TRCRW,"iovcnt=%d,iov_len=0x%08lx),%s)",g_uio->uio_iovcnt,
            g_uio->uio_iov->iov_len,rw==B_READ?"B_READ":"B_WRITE");

    fdp = fd->fdp+DEV_TO_PRT(dev);

    if (fdp->plock == 1)
        fdp->plock = 0;

    if (g_uio->uio_iovcnt != 1) {
        TPRINTF("<<<<iovcnt = %d>>>>", g_uio->uio_iovcnt);
    }

    if (fd->os < (vr?OS_PROBED:OS_PTBL_READ)) {
        if (fdcam_acquire_port(&fd->fsb,0,0)) {
            CPRINTF(FDCAM_TRACE,"(AF1)",0,0,0);
            return ENXIO;
	}
        VPRINTF("(RW/TC%s)",vr?"/VR":"");
        stat = tcheck_sync(fd, vr?OS_PROBED:OS_PTBL_READ, 0, 0, dev);
        fdcam_release_port(&fd->fsb);
        if (stat) {
            return EIO;
	}
    }
    /* else, tcheck will be called by strategy anyways. */


    bp = get_bp();

    if (!bp)
        return ENOMEM;

    /* Note: ................................................................*/
    /*       Thus, though not clean, the best way to handle an offset        */
    /*       presented to the raw interface is to re-define an element of    */
    /*       struct buf.  Here, the VALID_CHECK element is set to a unique   */
    /*       value to indicate that the offset has been stored in the        */
    /*       re-defined element (see #define T_OFFSET) of struct buf.        */
    /*       This is done to prevent extra buffer copying.                   */

    bp->VALID_CHECK = &thomas;
    T_OFFSET(bp) = g_uio->uio_offset & OFFSET_MASK;

    VPRINTF("(fd_rw-E)",0);

    rval = physio(fd_bstrategy, bp, dev, rw, minphys, g_uio);

    bp->VALID_CHECK = 0;

    VPRINTF("(fd_rw-F,%d)",rval);

    free_bp(bp);

    return rval;
}


int 
fd_read(dev_t dev, struct uio* g_uio)
{
    if (DEV_SPECIAL(dev)) {
#if FDCAM_TRC_ENABLE
        if (DEV_SPECIAL_ON(dev)) {
            if (*tracep && tracep < fdcam_trce)
                uiomove(tracep++, 1, g_uio);
            else if (trcmode == 2) {
                trcmode = 1;
                tracep = fdcam_trcb;
                if (*tracep)
                    uiomove(tracep++, 1, g_uio);
                else
                    return ENOSPC;
	    } else {
                tracep = fdcam_trce;
                return ENOSPC;
	    }
	}
        else if (DEV_SPECIAL_TRACE(dev)) {
            if (diagp >= (unsigned char*)fdcam_diag_p)
                return ENOSPC;
            uiomove(diagp++, 1, g_uio);
	}
        return 0;
#else
        return EIO;
#endif
    } else {
        return fd_rw(dev, g_uio, B_READ);
    }
}

int 
fd_write(dev_t dev, struct uio* g_uio)
{
    if (DEV_SPECIAL(dev)) {
        if (DEV_SPECIAL_RST(dev)) {
            char foo[2];
            uiomove(foo, 1, g_uio);
            foo[1] = 0;
            CPRINTF(0x0FFF, "%s", foo, 0, 0);
	} else {
            return ENOSPC;
	}
    } else {
        return fd_rw(dev, g_uio, B_WRITE);
    }
}


int 
fd_ioctl(dev_t dev, unsigned int cmd, caddr_t data, int flag)
{
    int err = 0;

    int lun = DEV_TO_LUN(dev);
    int prt = DEV_TO_PRT(dev);
    int vr = DEV_TO_VERY_RAW(dev);

    enum fdcam_status stat;

    struct fd_class *fd = fdi0.fd+lun;
    struct fdp_class* fdp;

    VPRINTF("\n<fd_ioctl>",0);

    if (!fdi0.did_init || lun >= N_LUN || prt >= N_PRT_TOTAL
            || fd->os == OS_DISABLED || DEV_SPECIAL_TRACE(dev)) {
        IEPRINTF("<fd_ioctl-err,did_init=%d>",fdi0.did_init);
        return ENXIO;
    }

    fdp = fd->fdp+prt;

    if (fdcam_acquire_port(&fd->fsb,0,0))
        return ENODEV;
    
    switch(cmd)
        {
        case FDIOSENSE:
        case FDIOTPRRST:   /* _IOR (FDIO_X,  6, struct fd_sense)    */
            {
            extern int fdcam_ver;
            struct fdcam_media_specs** p;
            struct fd_sense* x = (struct fd_sense*)data;

            if (cmd == FDIOTPRRST) {
                VPRINTF("<FDIOTPRRST>",0);
                tcheck_sync(fd, OS_NO_MEDIA, 1, 1, dev);
                x->tamper_possible = fd->prev_tamper;
                fdcam_motor_off(&fd->fsb);
	    } else {
                VPRINTF("<FDIOSENSE>",0);
                tcheck_sync(fd, OS_NO_MEDIA, 0, 1, dev);
                x->tamper_possible = fd->os < OS_MEDIA;
	    }

            x->new_media = fd->new_media;
            x->magic = FD_MAGIC;
            x->spare1 = 0;
            x->version = (FDCAM_H_VER<<24)|(FD_VER<<16)|(fdcam_ver<<8)
                    |(SINTL_ENA&0x01)|((RBUF_ENA<<1)&0x06)
                    |(FDCAM_DEBUG?0x08:0x00);
            x->drive_type = fd->fsb.is_present ? FD_DT_RX26:FD_DT_NO_DRIVE;
            x->write_protected = fd->fsb.write_protect;


            x->media_id = FD_MID_CANNOT_DETERMINE;
            if (fd->fsb.media_present) {
                VPRINTF("<mip=%0x>",0);
                for (p = fd->fsb.avail_fms ; *p ; p++)
                    if (fd->fsb.media_id_pins == (*p)->media_id)
                        x->media_id = (enum fd_media_id)(*p)->user_1;
	    } else {
                x->media_id = FD_MID_NO_MEDIA;
	    }
            x->drive_nopen = fd->nopen;
            } break;
        case FDIOGETMTYP:  /* _IOR (FDIO_X,  2, struct fd_mt)       */
            {
            struct fd_mt* x = (struct fd_mt*)data;
            VPRINTF("<FDIOGETMTYP>",0);

            x->spare1 = 0;
            x->partition_start=1;      /* Data not known.                    */
            x->partition_end = 0;      /*  "                                 */

            if (tcheck_sync(fd, vr?OS_PROBED:OS_PTBL_READ, 0, 0, dev)) {
                if (fd->os == OS_NO_MEDIA)
                    x->media = FD_MT_NO_MEDIA;
                else {
                    VPRINTF("(ioctl-CX)",0);
                    err = 1;
		}
	    } else {
                x->spare1 = (x->spare1&~3)
                        | (fd->fsb.interleave&~3?3:fd->fsb.interleave);

                if (fd->fsb.fms) {
                    x->n_sectors = fd->fsb.fms->sectors_p_track;
                    x->n_surfaces  = fd->fsb.fms->n_surfaces;
                    x->n_cylinders = fd->fsb.fms->cylinders_p_disk;
                    x->sector_size = fd->fsb.fms->bytes_p_sector;
                    x->media = (enum fd_media_type)fd->fsb.fms->user_1;

                    if (vr) {
                        x->partition_start=0;
                        x->partition_end = fd->n_blocks;
		    } else { /* assume  fd->fndelay == 0  */
                        x->partition_start = fd->cur_pt.pt_part[prt].pi_blkoff;
                        x->partition_end = x->partition_start
                                + fd->cur_pt.pt_part[prt].pi_nblocks;
		    }
		} else {
                    x->media = FD_MT_NO_MEDIA;
		}
	    }
            } break;
        case FDIOFMTDSK:   /* _IOWR(FDIO_X,  3, struct fd_fmt_spec) */
        case FDIOFFBSETUP: /* _IOWR(FDIO_X,  4, struct fd_fmt_spec) */
        case FDIOFMTTRK:   /* _IOWR(FDIO_X, 14, struct fd_fmt_spec) */
        case FDIOFFBNEXT:  /* _IOR (FDIO_X,  5, struct fd_fmt_spec) */
            {
            struct fd_fmt_spec* x = (struct fd_fmt_spec*)data;
            struct fdcam_media_specs** p;
            switch(cmd)
                {
                case FDIOFMTDSK:
                    VPRINTF("[FD]",0);
                    break;
                case FDIOFFBSETUP:
                    {
                    VPRINTF("[FS]",0);

                    if (x->disable_dlock == 0) {
                        if (fd->nopen != 1)
                            err = 1;
                        else if (fd->dlock == 0)
                            fd->dlock = 1;
		    }
                    } break;
                case FDIOFMTTRK:
                    VPRINTF("[FT]",0);
                    break;
                case FDIOFFBNEXT:
                    VPRINTF("[FN]",0);
                    break;
                }

            if (cmd != FDIOFFBSETUP) {
                if (fd->os > OS_PROBED)
                    fd->os = OS_PROBED;
                fd->raw_buffer_block = -1;
#if RBUF_ENA
                if (fd->cyl_buf_block != CYL_BUF_DISABLED)
                        fd->cyl_buf_block = CYL_BUF_INVALID;
#endif
	    }

            if (cmd != FDIOFFBNEXT && !err) {
                if (cmd != FDIOFMTTRK) {
                    x->next_cylinder = 0;
                    x->next_surface = 0;
                    x->start_position = 0;
		}
                x->percent_complete = 0;
                if (x->media == FD_MT_AUTOSENSE) {
		    extern struct fdcam_media_specs fms_default;
                    VPRINTF("(AS/TC)",0);
                    tcheck_sync(fd, OS_PROBED, 1, 1, dev);
                    VPRINTF("(MP=%d)",fd->fsb.media_present);
                    VPRINTF("(MID=%d)",fd->fsb.media_id_pins);
                    x->actual.media = FD_MT_NO_MEDIA;
		    /* if the format is still default, the disk has 	*/
		    /* has never been formatted.  Try to determine  	*/
		    /* the disk density by formatting a track and   	*/
		    /* then attempting to read it.  			*/
		    if (fd->fsb.fms == &fms_default) {
			for (p = fd->fsb.avail_fms; *p; p++) {
			    struct fdcam_format_spec fs;
			    
			    VPRINTF("(fmt track 0, med=%s)", (*p)->diag_name);
			    /* fill in defaults for test format */
			    fs.gpl = -1;
			    fs.filler = -1;
			    fs.interleave_factor = -1;
			    fs.first_sector_location = -1;
			    
			    fdcam_set_media(&fd->fsb, *p);

			    /* attempt to format head 0, cylinder 0 */
			    fdcam_format_track(&fd->fsb, 0, 0, &fs, 0, 0);

			    /* read media to see if format worked */
			    /* fake tcheck into probing media again */
			    fd->os = OS_MEDIA;
			    tcheck_sync(fd, OS_PROBED, 1, 1, dev);

			    /* if format worked, fsb.fms will be set */
			    if (fd->fsb.fms != &fms_default)
				break;
			}
		    }
		    x->actual.media = fd->fsb.fms->user_1;
                    VPRINTF("(ACTUAL=%d)",x->actual.media);
		} else {
                    x->actual.media = x->media;
		}
                
                if (x->actual.media == FD_MT_NO_MEDIA)
                    p = 0;
                else
                    for (p = fd->fsb.avail_fms ; *p ; p++)
                        if (x->actual.media == (*p)->user_1)
                            break;
                if (p && *p) {
                    fdcam_set_media(&fd->fsb, *p);
                    x->actual.n_sectors = fd->fsb.fms->sectors_p_track;
                    x->actual.sector_size = fd->fsb.fms->bytes_p_sector;
                    x->actual.n_surfaces  = fd->fsb.fms->n_surfaces;
                    x->actual.n_cylinders = fd->fsb.fms->cylinders_p_disk;
                    x->is_complete = 0;
                    x->media_error = 0;
                    x->actual.spare1 = 0;
                    if (vr) {
                        x->actual.partition_start=0;
                        x->actual.partition_end = fd->n_blocks;
		    } else { /* assume  fd->fndelay == 0  */
                        x->actual.partition_start
                                = fd->def_pt.pt_part[prt].pi_blkoff;
                        x->actual.partition_end = x->actual.partition_start
                                + fd->def_pt.pt_part[prt].pi_nblocks;
		    }
		} else {
                    x->media_error = 1;
                    x->is_complete = 1;
                    x->actual.n_cylinders = 0;
                    x->actual.n_surfaces  = 0;
                    x->actual.partition_start = 1;
                    x->actual.partition_end = 0;
                    x->actual.spare1 = 0;
                    err = 1;
                    if (fd->dlock == 1)
                        fd->dlock = 0;
		}	
	    }

            if (cmd != FDIOFFBSETUP && !err) {
                if (x->next_surface < x->actual.n_surfaces
                        &&  x->next_cylinder < x->actual.n_cylinders) {
                    while (x->next_surface < x->actual.n_surfaces
                            &&  x->next_cylinder < x->actual.n_cylinders) {
                        struct fdcam_format_spec fs;

                        VPRINTF("<FL,C=%d>",x->next_cylinder);

                        fs.gpl = -1;
                        fs.filler = x->fill;
                        fs.interleave_factor = x->interleave;
                        fs.first_sector_location = x->start_position;

                        if (stat = fdcam_format_track(&fd->fsb,
                                x->next_cylinder, x->next_surface, &fs, 0, 0)) {
                            if (fd->dlock == 1)
                                fd->dlock = 0;
                            x->media_error = 1;
                            EECPRINTF("Media error, drive %d, cyl %d head %d.",
                                    lun, x->next_cylinder, x->next_surface);
                            break;
			}
                        
                        x->start_position = fd->fsb.next_format_position;

                        x->percent_complete = (x->next_cylinder
                                *x->actual.n_surfaces+x->next_surface)*100
                                /(x->actual.n_surfaces*x->actual.n_cylinders);

                        if (cmd == FDIOFMTTRK)
                            break;
                        if (++x->next_surface >= x->actual.n_surfaces) {
                            x->next_surface = 0;
                            if (++x->next_cylinder >= x->actual.n_cylinders) {
                                x->is_complete = 1;
                                if (fd->dlock == 1)
                                    fd->dlock = 0;
			    }
			}
                        if (cmd == FDIOFFBNEXT)
                            break;
		    }
                } else {
                    VPRINTF("{OOPS2}",0);
                    x->media_error = 1;
                    if (fd->dlock == 1)
                        fd->dlock = 0;
		}
	    }
            } break;
        case FDIOCHKMED:   /* _IOR (FDIO_X,  7, long)               */
            {
            long* x = (long*)data;
            VPRINTF("<FDIOCHKMED>",0);
            /* +++ */
            } break;
        case FDIOSEEK:     /* _IOWR(FDIO_X,  8, struct fd_seek)     */
            {
            struct fd_seek* x = (struct fd_seek*)data;
            VPRINTF("<FDIOSEEK>",0);
            if (x->disable_plock == 0) {
		fdp->special_address = *x;
		if (fdp->plock == 0)
		    fdp->plock = 1;
	    } else {
		fdp->special_address = *x;
	    }
            } break;
        case FDIOMKCHS:    /* _IOWR(FDIO_X, 10, struct fd_chs_psn)  */
            {
            struct fd_chs_psn* x = (struct fd_chs_psn*)data;
            VPRINTF("<FDIOMKCHS>",0);
            if (fdcam_psn_to_chs(fd->fsb.fms, x->psn, &x->cylinder,
                    &x->head, &x->sector, 0, 0))
                err = 1;
            } break;
        case FDIOMKPSN:    /* _IOWR(FDIO_X, 11, struct fd_chs_psn)  */
            {
            struct fd_chs_psn* x = (struct fd_chs_psn*)data;
            VPRINTF("<FDIOMKPSN>",0);
            if (fdcam_chs_to_psn(fd->fsb.fms, x->cylinder,
                    x->head, x->sector, &x->psn))
                err = 1;
            } break;

        case DEVIOCGET:
            {
            struct devget* x = (struct devget*)data;

            VPRINTF("<DEVIOCGET>",0);
            tcheck_sync(fd, OS_PROBED, 1, 1, dev);

            bzero((caddr_t)x,sizeof(*x));
            x->category = DEV_DISK;
            x->bus = DEV_NB;

            bcopy("FDI",x->interface,4);
            bcopy("RX26",x->device,5);   /* ++++++ */
            bcopy("fd",x->dev_name,3);   /* ++++++ */
	    x->nexus_num = 0;
	    x->ctlr_num = fdi0.ctlr->ctlr_num;
            x->unit_num = lun;

            x->category_stat = prt & 0x07; /*DEV_DPMASK*/

            x->category_stat |= DEV_MC_COUNT;

            x->category_stat |= fd->change_count<<16;

            if (fd->os == OS_NO_MEDIA) {
                x->category_stat |= DEV_X_XXXX;
	    } else {
                if (fd->fsb.fms) {
                    if (fd->fsb.fms->user_1 == FD_MT_350DD)
                        x->category_stat |= DEV_3_DD2S;
                    else if (fd->fsb.fms->user_1 == FD_MT_350HD)
                        x->category_stat |= DEV_3_HD2S;
                    else if (fd->fsb.fms->user_1 == FD_MT_350ED)
                        x->category_stat |= DEV_3_ED2S;
                    else
                        x->category_stat |= DEV_X_XXXX;
		} else
                    x->category_stat |= DEV_X_XXXX;
	    }
            } break;

        case DEVGETGEOM:
            {
            DEVGEOMST* x = (DEVGEOMST*)data;
            VPRINTF("<DEVGETGEOM>",0);
            bzero((caddr_t)x,sizeof(*x));

                                       /* Floppy probably removable.         */
            x->geom_info.attributes |= DEVGEOM_REMOVE;
            x->geom_info.dev_size = fd->n_blocks;
            x->geom_info.nsectors = fd->fsb.fms->sectors_p_track;
            x->geom_info.ntracks  = fd->fsb.fms->n_surfaces;
            x->geom_info.ncylinders = fd->fsb.fms->cylinders_p_disk;
            /* ++++ TODO: check for more to fill in */
            } break;

#if FDCAM_DEBUG
        case FDIOCTST:
            {
            struct fd_ioctst_struct* xp = (struct fd_ioctst_struct*)data;
            VPRINTF("<FDIOCTST>",0);
            if (xp->cmd == ST1) {
                printf(">>>>>\n");
                xp->d1 = fdcam_seek(&fd->fsb, xp->d1, xp->d2, 0, 0);
                printf("<<<<<\n");
	    } else if (xp->cmd == ST2) {
                xp->d1 = fdcam_read_at_psn(&fd->fsb, xp->d1, 1L,
                            fd->raw_buffer, 0, 1, 0, 0);
	    } else if (xp->cmd == ST3) {
                xp->d1 = fdcam_write_at_psn(&fd->fsb, xp->d1, 1L,
                            fd->raw_buffer, 0, 1, 0, 0);
	    } else if (xp->cmd == ST4) {
                xp->d1 = fdcam_read_at(&fd->fsb,
                        0, /* cyl  */
                        0, /* head */
                        1, /* sec  */
                        fd->raw_buffer,0,
                        0,0,0);
	    } else {
                do_fdioctst(0,flag,xp);
	    }
            fd->raw_buffer_block = -1;
            fd->raw_buffer_block = -1;
            } break;
#endif

#if DO_DISKLABEL
	case DIOCGDINFO:
	    {
            struct disklabel* xp = (struct disklabel*)data;
            VPRINTF("<DIOCGDINFO,d_magic%s ok>",
		    fd->lbl.d_magic==DISKMAGIC?"":" NOT");

	    if (fd->lbl.d_magic != DISKMAGIC)
	        err = EINVAL;
	    else
		*xp = fd->lbl;

            } break;
	case DIOCGPART:
	    {
            struct partinfo* xp = (struct partinfo*)data;

	    if (fd->lbl.d_magic != DISKMAGIC || prt >= MAXPARTITIONS
		    || vr || !xp)
	        err = EINVAL;
	    else {
		xp->disklab = &fd->lbl;
		xp->part = &fd->lbl.d_partitions[prt];
	    }

            VPRINTF("<end DIOCGPART>",0);
            } break;
	case DIOCSDINFO:
	    {
            VPRINTF("<DIOCSDINFO>",0);

	    if (vr) {
		VPRINTF("<VR-error>",0);
	        err = EINVAL;
		break;
	    }

	    if (flag & FWRITE)
		err = setdisklabel(&fd->lbl, (struct disklabel *)data,
			get_partmask(fd,0)|get_partmask(fd,1));
	       /* Note, The scsi code uses a partition-mask of zero if       */
               /*  neither a diskalbel or an ultrix partition table was      */
               /*  actually found on the disk.                               */
	    else
		err = EBADF;
            } break;
	case DIOCWLABEL:
	    {
            VPRINTF("<DIOCWLABEL>",0);
	    fd->wlabel = *(int *)data;
            } break;
	case DIOCWDINFO:
	    /* The following code simply moved over from scsi_data.c...      */
	    {
            VPRINTF("<DIOCWDINFO>",0);

	    if (vr) {
		VPRINTF("<VR-error>",0);
	        err = EINVAL;
	    } else if ((flag & FWRITE) == 0) {
		err = EBADF;
	    } else if (0 == (err = setdisklabel(&fd->lbl,
		    (struct disklabel *)data,
		    get_partmask(fd,0)|get_partmask(fd,1)))) {
		/* Note, The scsi code uses a partition-mask of zero if      */
		/*  neither a diskalbel or an ultrix partition table was     */
		/*  actually found on the disk.                              */
		int wlab = fd->wlabel;

		fd->wlabel = 1;
	        fdi0.st_already_acquired = 10;
		fdi0.st_force_very_raw = 1;

		/* Mark VR partition as open,       */
		/*  since readdisklabel() will      */
		/*  check it.                       */
		fd->fdp[VERY_RAW_PRT].p_nopen[0]++; 
						     
		err = writedisklabel(dev, fd_bstrategy, &fd->lbl);
		fd->fdp[VERY_RAW_PRT].p_nopen[0]--;

		fdi0.st_force_very_raw = 0;
                fdi0.st_already_acquired = 0;
		fd->wlabel = wlab;
	    }

            } break;
#else
	case DIOCGDINFO:
	    {
            VPRINTF("<DIOCGDINFO--DISABLED>",0);
	    err = EINVAL;
	    } break;
	case DIOCGPART:
	    {
            VPRINTF("<DIOCGPART--DISABLED>",0);
	    err = EINVAL;
	    } break;
	case DIOCSDINFO:
	    {
            VPRINTF("<DIOCSDINFO--DISABLED>",0);
	    err = EINVAL;
	    } break;
	case DIOCWLABEL:
	    {
            VPRINTF("<DIOCWLABEL--DISABLED>",0);
	    err = EINVAL;
	    } break;
	case DIOCWDINFO:
	    {
            VPRINTF("<DIOCWDINFO--DISABLED>",0);
	    err = EINVAL;
	    } break;
#endif

        default:

#if FDCAM_DEBUG
             {
             long size = cmd>>16&0xFF;
             unsigned char* cp = (unsigned char*)data;

             CPRINTF(FDCAM_TRACE,"fd_ioctl:unknown command,0x%08lx=%s('%c',",
                        (long)cmd,
                        (cmd&0xe0000000)==0xc0000000?"_IOWR":
                        (cmd&0xe0000000)==0x40000000?"_IOR":
                        (cmd&0xe0000000)==0x80000000?"_IOW":
                        (cmd&0xe0000000)==0x20000000?"_IO":"_HUH?",
                        cmd>>8&0xFF);
             CPRINTF(FDCAM_TRACE,"%d,size=%d{", cmd&0xFF, cmd>>16&0xFF, 0);
             while (size-- > 0)
                CPRINTF(FDCAM_TRACE,"%02X",*cp++,0,0);

             CPRINTF(FDCAM_TRACE,"}).\n",0,0,0);
             }
#endif
            err = ENXIO;
        }
    fdcam_release_port(&fd->fsb);
    VPRINTF("<end fd_ioctl,err=%d>\n",err);
    return err;
}

/* ************************************************************************* */
/************************************************************************
 *
 *  ROUTINE NAME:  fd_psize()
 *
 *  FUNCTIONAL DESCRIPTION:
 *	This routine returns the size of a partition to the caller.
 *
 *  FORMAL PARAMETERS:
 *      dev      - Major/minor device number.  The major number is ignored.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  RETURN VALUE:
 *      psize    - Successful
 *
 *      ENXIO    - A bad minor number was passed.
 *		 - Parition table invalid.
 *
 *  SIDE EFFECTS:
 *
 *  ADDITIONAL INFORMATION:
 *
 ************************************************************************/

daddr_t 
fd_psize(dev_t dev)
{
   int lun = DEV_TO_LUN(dev);		/* Parse disk number */
   int prt = DEV_TO_PRT(dev);		/* Parse partition   */
   struct fd_class *fd = fdi0.fd+lun;	/* Load disk struct  */
   struct fdp_class* fdp;		/* Partition pointer */
   daddr_t psize;			/* Returned partition size */

   /* Sanity checks. */
   if (!fdi0.did_init || lun >= N_LUN || prt >= N_PRT_TOTAL
       || fd->os == OS_DISABLED || DEV_SPECIAL_TRACE(dev)) {
       IEPRINTF("<fd_psize-err,did_init=%d>",fdi0.did_init);
       return ENXIO;
   }
   
   fdp = fd->fdp+prt;			/* Load partition    */
   
#if DO_DISKLABEL
   
   psize = fd->lbl.d_partitions[prt].p_size;	/* Use OSF label info */
   
#else	/* DO_DISKLABEL */
   
   if (fd->cur_pt.pt_valid != PT_VALID) 	/* Make sure partition*/
       return ENXIO;                              /*  is valid.	      */
   
   /* -1 number of blocks says partition goes to end of disk.*/
   if (fd->cur_pt.pt_part[prt].pi_nblocks != -1)
       psize = fd->cur_pt.pt_part[prt].pi_nblocks;
   else
       psize = fd->n_blocks - fd->cur_pt.pt_part[part].pi_blkoff;
   
#endif	/* DO_DISKLABEL */
   
   return(psize);
}
                                                           

/* ************************************************************************* */

static int 
copy_sva_to_ubuf(unsigned char* u_buf, struct proc* u_proc, 
		 unsigned char* sva, long size)
{
    unsigned char* end = sva+size;
    CPRINTF(FDCAM_TRC1,
            "copy_sva_to_ubuf(u_buf=0x%08lx,u_proc=0x%08lx,,size=%ld)",
            u_buf,u_proc,size);
    if (u_proc) {
        while (sva < end) {
	    unsigned char* u_k0 = (unsigned char*)PHYS_TO_KSEG(
                    pmap_extract(u_proc->task->map->vm_pmap, u_buf));
            unsigned char* up = u_k0;
            int count = NBPG-((unsigned long)up & PGOFSET);
            unsigned char* inner_end;

            if (count > end-sva)
                count = end-sva;
            inner_end = sva + count;

            while (sva < inner_end)
                *up++ = (unsigned char)*sva++;

            u_buf += count;
	 }
    } else {
        while (sva < end)
            *u_buf++ = *sva++;
    }
    return 0;
}


/* return non-zero if failure. */

static int 
copy_ubuf_to_sva(unsigned char* sva, unsigned char* u_buf, 
		 struct proc* u_proc, long size)
{
    unsigned char* end = sva+size;
    CPRINTF(FDCAM_TRC1,
            "copy_ubuf_to_sva(,u_buf=0x%08lx,u_proc=0x%08lx,size=%ld)",
            u_buf, u_proc, size);

    if (u_proc) {
        while (sva < end) {
	    unsigned char* u_k0 = (unsigned char*)PHYS_TO_KSEG(
                    pmap_extract(u_proc->task->map->vm_pmap, u_buf));

            int count = NBPG-((unsigned long)u_k0 & PGOFSET);
            unsigned char* inner_end;

            if (count > end-sva)
                count = end-sva;
            inner_end = sva + count;

            while (sva < inner_end)
                *sva++ = *u_k0++;

            u_buf += count;
	 }
    } else {
        while (sva < end)
            *sva++ = *u_buf++;
    }
    return 0;
}

/* ************************************************************************* */

static void 
fd_strategy_done(struct strategy_class* scp, int error)
{
    VPRINTF("(FSD,%d)",error);
    if (error) {
        scp->cur_bp->b_flags |= B_ERROR;

	/* If original byte count field is non-zero, */
	/*  this is a partial transfer, reset bcount.*/
	if (scp->obcount != 0)		
	  scp->cur_bp->b_bcount = scp->obcount;	

	scp->cur_bp->b_resid = scp->cur_bp->b_bcount;
        scp->f->st_last_in_error = 1;
    } else {
	if (scp->obcount != 0) {/* If partial transfer, return byte not xfer */
	    scp->cur_bp->b_resid = scp->obcount - scp->cur_bp->b_bcount;
	    scp->cur_bp->b_bcount= scp->obcount;
	} else {
	    scp->cur_bp->b_resid = 0;	
	}
    }

    scp->cur_bp->b_error = error;

    if (!scp->f->st_already_acquired) {
        struct fdcam_status_block* tmp = &scp->cur_fd->fsb;
        biodone(scp->cur_bp);
        delete_strategy_class(scp);
        fdcam_release_port(tmp);
    } else {
        VPRINTF("<AA,%d>",fdi0.st_already_acquired);
	if (fdi0.st_already_acquired > 10)
	    fdi0.st_already_acquired--;
        VPRINTF("<BB,0x%08lx>",scp->cur_bp);
        biodone(scp->cur_bp);
        VPRINTF("<CC>",0);
        delete_strategy_class(scp);
        VPRINTF("<DD>",0);
    }
}

/* ************************************************************************* */

#if RBUF_ENA

static void bio_wr_1();

static void 
bio_wr_0(struct fd_class* fd, long stat, struct fdcam_status_block* fsb)
{
    VPRINTF("(bw0-%ld)",(long)fd->bio_nsec);
    if (fd->bio_nsec <= 0)
        (*fd->bio_cb)(fd->bio_cb_tag,FDCAM_NORMAL,fsb);
    else {
        fd->bio_len = fd->cyl_size - fd->bio_psn%fd->cyl_size;
        if (fd->bio_len > fd->bio_nsec)
            fd->bio_len = fd->bio_nsec;
#if SINTL_ENA
        fdcam_write_at_psn(fsb, fd->bio_psn, fd->bio_len, fd->bio_buffer,
                fd->bio_process, fd->fsb.interleave==1?2:1, bio_wr_1, fd);
#else
        fdcam_write_at_psn(fsb, fd->bio_psn, fd->bio_len, fd->bio_buffer,
                fd->bio_process, 1,                         bio_wr_1, fd);
#endif
    }
    VPRINTF("(end-bw0)",0);
}

static void 
bio_wr_1(FDCAM_CB_TAG tag, enum fdcam_status istat, 
	 struct fdcam_status_block* fsb)
{
    struct fd_class* fd = tag;
    VPRINTF("(bw1)",0);
    if (fd->bio_psn >= fd->cyl_buf_block 
            && fd->bio_psn < fd->cyl_buf_block+fd->cyl_size) {
        if (istat)
            fd->cyl_buf_block = CYL_BUF_INVALID;
        else {
            /* We know that fd->bio_len will not go beyond the end of our    */
            /*  cylinder buffer.                                             */
            copy_ubuf_to_sva (fd->cyl_buf
                    +(fd->bio_psn-fd->cyl_buf_block)*SECTOR_SIZE,
                    fd->bio_buffer, fd->bio_process, fd->bio_len*SECTOR_SIZE);
            if (fd->bio_psn < fd->cyl_buf_start
                    && fd->bio_psn+fd->bio_len >= fd->cyl_buf_start)
                fd->cyl_buf_start = fd->bio_psn;
	}
    }

    if (istat)
        (*fd->bio_cb)(fd->bio_cb_tag,istat,fsb);
    else {
        fd->bio_psn += fd->bio_len;
        fd->bio_nsec -= fd->bio_len;
        fd->bio_buffer += fd->bio_len*SECTOR_SIZE;
        bio_wr_0(fd, 0, fsb);
    }
}


static void 
bio_rd_2(FDCAM_CB_TAG tag, enum fdcam_status istat, 
	 struct fdcam_status_block* fsb)
{
    struct fd_class* fd = tag;
    VPRINTF("(br2)",0);
                                       /* If there was a media error, maybe  */
                                       /*  that media error was in part of   */
                                       /*  the cylinder that was requested   */
                                       /*  for buffering purposes, but not   */
                                       /*  specifically requested by the     */
                                       /*  user.  Let's try again, asking for*/
                                       /*  just those sectors really needed  */
                                       /*  by the user.                      */
    if (istat == FDCAM_READ_ERROR) {
        unsigned short cyl, head, sec, rem;

        VPRINTF("(br2RT-%d)",fd->bio_nsec);
        fd->cyl_buf_block = CYL_BUF_INVALID;

        fdcam_psn_to_chs(fd->fsb.fms, fd->bio_psn, &cyl, &head, &sec, &rem, 0);

        fd->bio_bads = fd->bio_psn;
        fd->bio_bade = fd->bio_psn+rem;

#if SINTL_ENA
        fdcam_read_at_psn(fsb, fd->bio_psn, fd->bio_nsec, fd->bio_buffer,
                fd->bio_process, fd->fsb.interleave==1?2:1, fd->bio_cb,
                fd->bio_cb_tag);
#else
        fdcam_read_at_psn(fsb, fd->bio_psn, fd->bio_nsec, fd->bio_buffer,
                fd->bio_process, 1,                         fd->bio_cb,
                fd->bio_cb_tag);
#endif
    } else if (istat) {
        VPRINTF("(br2F)",0);
        fd->cyl_buf_block = CYL_BUF_INVALID;
        (*fd->bio_cb)(fd->bio_cb_tag,istat,fsb);
    } else {
        /* Note, if we are here, we can assume that fd->bio_nsec sectors     */
        /*  are available in the cylinder buffer for copying.                */

        copy_sva_to_ubuf(fd->bio_buffer, fd->bio_process,
                fd->cyl_buf+(fd->bio_psn-fd->cyl_buf_block)*SECTOR_SIZE,
                fd->bio_nsec*SECTOR_SIZE);
        (*fd->bio_cb)(fd->bio_cb_tag,FDCAM_NORMAL,fsb);
    }
}

static void bio_rd_1();

static void 
bio_rd_0(struct fd_class* fd, long stat, struct fdcam_status_block* fsb)
{
    CPRINTF(FDCAM_TRC1,"(br0,nsec=%ld,cyl_size=%ld,fd@0x%08lx)",
	    fd->bio_nsec, fd->cyl_size, fd);
    if (fd->bio_nsec <= 0) {
	VPRINTF("(br0-@)",0);
        (*fd->bio_cb)(fd->bio_cb_tag,FDCAM_NORMAL,fsb);
    } else {
	/* fd->bio_len is number of sectors   */
	/*  requested from current cylinder   */
	/*  (or current track, if !RBUF_CYL). */
        fd->bio_len = fd->cyl_size - fd->bio_psn%fd->cyl_size;
        if (fd->bio_len > fd->bio_nsec)
            fd->bio_len = fd->bio_nsec;

	CPRINTF(FDCAM_TRC1,"(BR0-F,bio_len=%ld, bio_nsec=%ld,",
		fd->bio_len, fd->bio_nsec, 0);
	CPRINTF(FDCAM_TRC1,"bio_psn=%ld,cyl_buf_start=%ld,cyl_buf_block=%ld)",
		fd->bio_psn, fd->cyl_buf_start, fd->cyl_buf_block);

                                       /* If the current cylinder being      */
                                       /*  processed can be found in the     */
                                       /*  cylinder buffer.                  */
        if (fd->bio_psn>=fd->cyl_buf_start
                && fd->bio_psn<fd->cyl_buf_block+fd->cyl_size) {
	    VPRINTF("(br0-A)",0);
            copy_sva_to_ubuf(fd->bio_buffer, fd->bio_process,
                    fd->cyl_buf+(fd->bio_psn-fd->cyl_buf_block)*SECTOR_SIZE,
                    fd->bio_len*SECTOR_SIZE);
	    VPRINTF("(br0-B)",0);
            bio_rd_1(fd, 0, fsb);
	}
                                       /* If the current cylinder being      */
                                       /*  processed does not contain the    */
                                       /*  last sector requested.            */
        else if (fd->bio_len < fd->bio_nsec) {
	    VPRINTF("(br0-C)",0);
#if SINTL_ENA
            fdcam_read_at_psn(fsb, fd->bio_psn, fd->bio_len, fd->bio_buffer,
                    fd->bio_process, fd->fsb.interleave==1?2:1, bio_rd_1, fd);
#else
            fdcam_read_at_psn(fsb, fd->bio_psn, fd->bio_len, fd->bio_buffer,
                    fd->bio_process, 1,                         bio_rd_1, fd);
#endif
	}
                                       /* Else, the current cylinger being   */
                                       /*  processed contains the last sector*/
                                       /*  requested, so lets update the     */
                                       /*  cylinder buffer while we are here.*/
        else {
            int k = fd->bio_psn%fd->cyl_size;
	    VPRINTF("(br0-D)",0);
            if (fd->cyl_buf_block == fd->bio_psn - k) {
                /* It appears we are reading the sectors from the current    */
                /*  backwards, so lets just buffer the whole cylinder...     */
                int old_cbs = fd->cyl_buf_start;
                fd->cyl_buf_start = fd->cyl_buf_block;
                TPRINTF("<CYL_BUF*%ld>",fd->cyl_buf_block);
#if SINTL_ENA
                fdcam_read_at_psn(fsb, fd->cyl_buf_start,
                        old_cbs-fd->cyl_buf_start, fd->cyl_buf, 0,
                        fd->fsb.interleave==1?2:1, bio_rd_2, fd);
#else
                fdcam_read_at_psn(fsb, fd->cyl_buf_start,
                        old_cbs-fd->cyl_buf_start, fd->cyl_buf, 0,
                        1,                         bio_rd_2, fd);
#endif
	    } else {
                fd->cyl_buf_block = fd->bio_psn - k;
                fd->cyl_buf_start = fd->bio_psn;
                TPRINTF("<CYL_BUF@%ld>",fd->cyl_buf_block);
#if SINTL_ENA
                fdcam_read_at_psn(fsb, fd->cyl_buf_start,
                        fd->cyl_size-k, fd->cyl_buf+(k*SECTOR_SIZE), 0,
                        fd->fsb.interleave==1?2:1, bio_rd_2, fd);
#else
                fdcam_read_at_psn(fsb, fd->cyl_buf_start,
                        fd->cyl_size-k, fd->cyl_buf+(k*SECTOR_SIZE), 0,
                        1,                         bio_rd_2, fd);
#endif
	    }
	}
    }
    VPRINTF("(br0-X)",0);
}

static void 
bio_rd_1(FDCAM_CB_TAG tag, enum fdcam_status istat, 
	 struct fdcam_status_block* fsb)
{
    struct fd_class* fd = tag;
    VPRINTF("(br1)",0);
    if (istat)
        (*fd->bio_cb)(fd->bio_cb_tag,istat,fsb);
    else {
	fd->bio_psn += fd->bio_len;
	fd->bio_nsec -= fd->bio_len;
	fd->bio_buffer += fd->bio_len*SECTOR_SIZE;
	bio_rd_0(fd, 0, fsb);
    }
}

#endif

static enum fdcam_status 
bio_read(struct fd_class *fd, unsigned long psn, unsigned long nsec, 
	 unsigned char* buffer, struct proc* process, FDCAM_CB cb, 
	 FDCAM_CB_TAG cb_tag)
{
    VPRINTF("(br,psn=%ld)",psn);
#if RBUF_ENA
#if RBUF_ENA==2
    if (fd->fsb.interleave==1 && fd->cyl_buf_block==CYL_BUF_DISABLED)
        fd->cyl_buf_block = CYL_BUF_INVALID;
    if (fd->fsb.interleave>1 && fd->cyl_buf_block!=CYL_BUF_DISABLED)
        fd->cyl_buf_block = CYL_BUF_DISABLED;
#endif
    if (fd->cyl_buf_block == CYL_BUF_DISABLED || !fd->cyl_buf 
            || psn>=fd->bio_bads && psn+nsec<=fd->bio_bade)
#endif
#if SINTL_ENA
        return fdcam_read_at_psn(&fd->fsb, psn, nsec, buffer, process,
                fd->fsb.interleave==1?2:1, cb, cb_tag);
#else
        return fdcam_read_at_psn(&fd->fsb, psn, nsec, buffer, process,
                1,                         cb, cb_tag);
#endif

#if RBUF_ENA
    fd->bio_cb      = cb;
    fd->bio_cb_tag  = cb_tag;
    fd->bio_psn     = psn;
    fd->bio_nsec    = nsec;
    fd->bio_process = process;
    fd->bio_buffer  = buffer;

    bio_rd_0(fd, 0, &fd->fsb);
    return FDCAM_NORMAL;
#endif
}


static enum fdcam_status 
bio_write(struct fd_class *fd, unsigned long psn, unsigned long nsec, 
	  unsigned char* buffer, struct proc* process, FDCAM_CB cb, 
	  FDCAM_CB_TAG cb_tag)
{
#if RBUF_ENA
    VPRINTF("(bw,psn=%ld)",psn);
#if RBUF_ENA==2
    if (fd->fsb.interleave==1 && fd->cyl_buf_block==CYL_BUF_DISABLED)
        fd->cyl_buf_block = CYL_BUF_INVALID;
    if (fd->fsb.interleave>1 && fd->cyl_buf_block!=CYL_BUF_DISABLED)
        fd->cyl_buf_block = CYL_BUF_DISABLED;
#endif
    if (fd->cyl_buf_block == CYL_BUF_DISABLED || !fd->cyl_buf)
#endif
#if SINTL_ENA
        return fdcam_write_at_psn(&fd->fsb, psn, nsec, buffer, process,
                fd->fsb.interleave==1?2:1, cb, cb_tag);
#else
        return fdcam_write_at_psn(&fd->fsb, psn, nsec, buffer, process,
                1,                         cb, cb_tag);
#endif

#if RBUF_ENA
    fd->bio_cb      = cb;
    fd->bio_cb_tag  = cb_tag;
    fd->bio_psn     = psn;
    fd->bio_nsec    = nsec;
    fd->bio_process = process;
    fd->bio_buffer  = buffer;

    bio_wr_0(fd, 0, &fd->fsb);

    return FDCAM_NORMAL;
#endif
}

/*****************************************************************************/

static void 
fd_strategy_e(FDCAM_CB_TAG tag, enum fdcam_status istat, 
	      struct fdcam_status_block* fsb)
{
    struct strategy_class* scp = tag;
    VPRINTF("(SE)",0);

    if (istat) {
        scp->cur_fd->raw_buffer_block = -1;
        TPRINTF("(fds-fail-19)",0);  /* +++++++ */
        fd_strategy_done(scp,EIO);
    } else {
        scp->cur_fd->raw_buffer_block = scp->st_blkno;
        VPRINTF("(fds-pass-1e)",0);
        fd_strategy_done(scp,0);
    }
}


static void 
fd_strategy_d0(FDCAM_CB_TAG tag, enum fdcam_status istat, 
	      struct fdcam_status_block* fsb)
{
    struct strategy_class* scp = tag;
    VPRINTF("(SD)",0);

    if (istat) {
        scp->cur_fd->raw_buffer_block = -1;
        TPRINTF("(fds-fail-9)",0);  /* +++++++ */
        fd_strategy_done(scp,EIO);
        return;
    } else {
        scp->cur_fd->raw_buffer_block = scp->st_blkno;
    }

#   if FDCAM_DEBUG
      if (!scp->cur_fd->raw_buffer || scp->cur_bp->b_resid > SECTOR_SIZE)
          IEPRINTF("(bad-rb,1)",0);
#   endif

    if(scp->cur_bp->b_flags & B_READ) {
        copy_sva_to_ubuf(scp->st_muaddr, scp->st_process,
                scp->cur_fd->raw_buffer, scp->cur_bp->b_resid);
        fd_strategy_done(scp,0);
    } else {
        copy_ubuf_to_sva(scp->cur_fd->raw_buffer, scp->st_muaddr,
                scp->st_process, scp->cur_bp->b_resid);
        
        scp->cur_bp->b_resid = 0;

        bio_write(scp->cur_fd, scp->st_blkno, 1, scp->cur_fd->raw_buffer, 0,
                fd_strategy_e, scp);
    }
        
    VPRINTF("(fds-pass-2)",0);
}


static void 
fd_strategy_c0(FDCAM_CB_TAG tag, enum fdcam_status istat, 
	      struct fdcam_status_block* fsb)
{
    struct strategy_class* scp = tag;
    VPRINTF("(SC)",0);

#   if FDCAM_DEBUG
     if (!scp || !fsb || !scp->cur_bp || !scp->cur_fd)
          IEPRINTF("(bad-rb,8)",0);
#   endif

    if (istat) {
        TPRINTF("(fds-io_fail,s=%d)",(int)istat);   /*+++++++*/
        fd_strategy_done(scp,EIO);
    } else if (scp->cur_bp->b_resid <= 0) {
        VPRINTF("(fds-pass-1)",0);
        fd_strategy_done(scp,0);
    }
#if FDCAM_DEBUG
    else if (scp->cur_bp->b_resid >= SECTOR_SIZE) {
        IEPRINTF("(BAD123)", 0);
        fd_strategy_done(scp,EIO);
    }
#endif
    else {
#       if FDCAM_DEBUG
          if (!scp->cur_fd->raw_buffer)
              IEPRINTF("(bad-rb,2)",0);
#       endif
        if (scp->cur_fd->raw_buffer_block == scp->st_blkno) {
	    VPRINTF("(SC.2)",0);
            fd_strategy_d0(scp, FDCAM_NORMAL, fsb);
	} else {
	    VPRINTF("(SC.3)",0);
            bio_read(scp->cur_fd, scp->st_blkno, 1,
                    scp->cur_fd->raw_buffer, 0, fd_strategy_d0, scp);
	}
    }
}


static void 
fd_strategy_ac(FDCAM_CB_TAG tag, enum fdcam_status istat, 
	      struct fdcam_status_block* fsb)
{
    struct strategy_class* scp = tag;
                                       /* Total number of whole blocks to be */
                                       /*  read from the media.              */
    long blkcnt = (scp->cur_bp->b_resid)/SECTOR_SIZE;

    if (istat) {
        TPRINTF("(fds-fail-9-%d)",(int)istat);
        scp->cur_fd->raw_buffer_block = -1;
        fd_strategy_done(scp,ENODEV);
        return;
    }

    scp->cur_bp->b_resid -= blkcnt*SECTOR_SIZE;

    CPRINTF(FDCAM_TRC1,"(SAC,blkcnt=%ld,resid=%ld,%ld)", blkcnt,
            (scp&&scp->cur_bp)?scp->cur_bp->b_resid:0, scp&&scp->cur_bp);

    if(scp->cur_bp->b_flags & B_READ) {
        long bn = scp->st_blkno;
        unsigned char* ad = scp->st_muaddr;
        scp->st_blkno += blkcnt;
        scp->st_muaddr += blkcnt*SECTOR_SIZE;
        VPRINTF("(SB1,bn=%d)",bn);
        bio_read(scp->cur_fd, bn, blkcnt,
                ad, scp->st_process, fd_strategy_c0, scp);
    } else {
        long bn = scp->st_blkno;
        unsigned char* ad = scp->st_muaddr;
        scp->st_blkno += blkcnt;
        scp->st_muaddr += blkcnt*SECTOR_SIZE;
        VPRINTF("(SB2,bn=%d)",bn);
        bio_write(scp->cur_fd, bn, blkcnt,
                ad, scp->st_process, fd_strategy_c0, scp);
    }
}


static void 
fd_strategy_x0(FDCAM_CB_TAG tag, enum fdcam_status istat, 
	      struct fdcam_status_block* fsb)
{
    struct strategy_class* scp = tag;
    long len;

    VPRINTF("(SX0,%ld)",scp&&scp->cur_bp);

    len = scp->cur_bp->b_resid;

    if (len > SECTOR_SIZE-scp->offset)
        len = SECTOR_SIZE-scp->offset;

    if (istat) {
        scp->cur_fd->raw_buffer_block = -1;
        TPRINTF("(fds-fail-x0)",0);  /* +++++++ */
        fd_strategy_done(scp,EIO);
        return;
    } else {
        scp->cur_fd->raw_buffer_block = scp->st_blkno;
    }

    if(scp->cur_bp->b_flags & B_READ) {
        copy_sva_to_ubuf(scp->st_muaddr, scp->st_process,
                scp->cur_fd->raw_buffer+scp->offset, len);

        scp->st_muaddr += len;
        scp->cur_bp->b_resid -= len;
        scp->st_blkno++;

        if (scp->cur_bp->b_resid <= 0)
            fd_strategy_done(scp,0);
        else
            fd_strategy_ac(scp, 0, fsb);
    } else {
        long bn = scp->st_blkno;

#       if FDCAM_DEBUG
          if (!scp->cur_fd->raw_buffer || len > SECTOR_SIZE)
              IEPRINTF("(bad-rb,83)",0);
#       endif

        copy_ubuf_to_sva(scp->cur_fd->raw_buffer+scp->offset, scp->st_muaddr,
                scp->st_process, len);

        scp->st_muaddr += len;
        scp->cur_bp->b_resid -= len;
        scp->st_blkno++;

        bio_write(scp->cur_fd, bn, 1, scp->cur_fd->raw_buffer, 0,
                fd_strategy_ac, scp);
    }
    
}


static void 
fd_strategy_ab(FDCAM_CB_TAG tag, enum fdcam_status istat, 
	      struct fdcam_status_block* fsb)
{
    struct strategy_class* scp = tag;
    long s;
    long limit;

    if (istat) {
/*        EEPRINTF("Media removed from device while open.",0);*/
        scp->cur_fd->raw_buffer_block = -1;
        fd_strategy_done(scp,EIO);
        return;
    }

    CPRINTF(FDCAM_TRC1,"(SAB,scp@0x%08lx,bp@0x%08lx,lun=%d,", scp, scp->cur_bp,
            scp->cur_lun);
#if RBUF_ENA
    CPRINTF(FDCAM_TRC1,"fsb@0x%08lx,fsb->lun_id=%d,scp->cur_fd->fsb@0x%08lx,",
            fsb, fsb->lun_id, &scp->cur_fd->fsb);
    CPRINTF(FDCAM_TRC1,"scp->cur_fd->fsb.interleave=%d,cs=%ld,cb=%ld)",
            scp->cur_fd->fsb.interleave,scp->cur_fd->cyl_buf_start,
            scp->cur_fd->cyl_buf_block);
#else
    CPRINTF(FDCAM_TRC1,"fsb@0x%08lx, fsb->lun_id=%d,scp->cur_fd->fsb@0x%08lx)",
            fsb, fsb->lun_id, &scp->cur_fd->fsb);
#endif

    if (scp->cur_fdp->special_address.mode == FD_SEEK_PSN
            && !fdi0.st_already_acquired) {
        scp->offset = 0;
        scp->cur_fdp->special_address.mode = FD_SEEK_NOT_USED;
                                       /* Number of first block that needs   */
                                       /*  read/writing.                     */
        scp->st_blkno = scp->cur_fdp->special_address.where.psn;
        VPRINTF("(SB-PSN-MODE)",scp->st_blkno);

                                       /* Last PSN avail on disk + 1         */
        limit = scp->cur_fd->n_blocks;
    } else if (scp->cur_fdp->special_address.mode != FD_SEEK_NOT_USED
            && !fdi0.st_already_acquired) {
        VPRINTF("(SB-BAD-MODE)",0);
        scp->cur_fdp->special_address.mode = FD_SEEK_NOT_USED;
        fd_strategy_done(scp,EIO);
        return;
    } else if (scp->cur_vr) {
        VPRINTF("(SAB-VR)",0);
        scp->st_blkno = scp->cur_bp->b_blkno;
        limit = scp->cur_fd->n_blocks;
    } else {
        VPRINTF("(SAB-N)",0);
        scp->st_blkno = scp->cur_bp->b_blkno
                + scp->cur_fd->cur_pt.pt_part[scp->cur_prt].pi_blkoff;
        limit = scp->cur_fd->cur_pt.pt_part[scp->cur_prt].pi_blkoff
                + scp->cur_fd->cur_pt.pt_part[scp->cur_prt].pi_nblocks;
    }

                                       /* Total number of blocks, including  */
                                       /*  whole and partial blocks, to be   */
                                       /*  read from the media.              */
    s = (scp->cur_bp->b_resid+scp->offset+SECTOR_SIZE-1)/SECTOR_SIZE;

    if (scp->st_blkno + s > limit) {
	/* This is not the optimal place to put boundary checks. But, since  */
	/* it has been put here during initial implementation, it is lower   */
	/* risk to localize additional boundary checks here.  bp->b_bcount   */
	/* and bp->b_resid has to be modified.				     */

	/* POSIX dictates the following :				     */
	/* Scenario		returned	errno		resid	     */
	/* --------		--------	-----		-----	     */
	/* Read  beyond		0		0		bcount	     */
	/* Write beyond		-1		ENOSPC		bcount	     */
	/* Partial r/w		# bytes xfer	0		#bytes !xfer */

	/* If this portion is executed, some boundary condition is exceeded. */
	/* Set up scp->obcount (original byte count now) since the posting   */
	/* routines depends on this field as a switch.			     */

	scp->obcount = scp->cur_bp->b_bcount;

        CPRINTF(FDCAM_TRACE,
		"(fds-fail-BLOCK_%ld_OUT_OF_RANGE,s=%ld,limit=%ld,",
                (long)scp->st_blkno+s-1, (long)s, (long)limit);
        CPRINTF(FDCAM_TRACE, "st_blkno=%ld,b_blkno=%ld,b_resid=%ld,",
                (long)scp->st_blkno,
                (long)scp->cur_bp->b_blkno, (long)scp->cur_bp->b_resid);
        CPRINTF(FDCAM_TRACE, "cur_prt=%d)", scp->cur_prt, 0, 0);

	/* First check for r/w at or beyond end of partition/media.*/
	if (scp->st_blkno >= limit) {
	    if (scp->cur_bp->b_flags & B_READ) {/* Read beyond boundary.*/
		/* Make it look like a successful 0 byte */
		/* partial transfer.  Post with no error. */		
		scp->cur_bp->b_bcount = 0;
		fd_strategy_done(scp,0);	
	    } else				/* Write beyond boundary.    */
		fd_strategy_done(scp,ENOSPC);	/* Post with enospc.   	     */
	    return;
	} else {
	    /* Must be partial r/w. Set up bp->bcount/resid */
	    /* and strategy_done will do the rest. */
	    
	    /* transfer up until limit */
	    scp->cur_bp->b_bcount = (((limit - scp->st_blkno) * SECTOR_SIZE)
				     - scp->offset);
	    scp->cur_bp->b_resid  = scp->cur_bp->b_bcount;
	}
    }

#if DO_DISKLABEL
# ifndef LABELSECTOR
#  ++++++ Error, LABELSECTOR undefined.
# endif
    if (scp->st_blkno <= LABELSECTOR
	    &&  scp->st_blkno+s > LABELSECTOR
            &&  (scp->cur_bp->b_flags & B_READ) == 0
                                       /* The following line was copied from */
                                       /*  scsi_disk.c, which says that if   */
                                       /*  we are writing a disklabel to the */
                                       /*  disk for the first time, we can   */
                                       /*  ignore the disklabel write-protect*/
                                       /*  indicator.  Is this correct? ++++ */
	    &&  scp->cur_fd->lbl.d_magic == DISKMAGIC
	    &&  scp->cur_fd->wlabel == 0
	    &&  scp->cur_vr == 0) {
        VPRINTF("(SAB-WLABEL-ERROR)",0);
	fd_strategy_done(scp,EROFS);
	return;
    }
#endif

#if RBUF_ENA
    VPRINTF("(SAB-X)",0);

    if ((scp->cur_bp->b_flags & B_READ)
            && scp->st_blkno>=scp->cur_fd->cyl_buf_start
            && scp->st_blkno<scp->cur_fd->cyl_buf_block+scp->cur_fd->cyl_size
            && scp->cur_fd->cyl_buf) {
	long len;
        TPRINTF("<QC>",0);

        len = (scp->cur_fd->cyl_buf_block + scp->cur_fd->cyl_size
                - scp->st_blkno) * SECTOR_SIZE - scp->offset;

        if (len > scp->cur_bp->b_resid)
            len = scp->cur_bp->b_resid;
            
        copy_sva_to_ubuf(scp->st_muaddr, scp->st_process,
                scp->cur_fd->cyl_buf+scp->offset
                +SECTOR_SIZE*(scp->st_blkno-scp->cur_fd->cyl_buf_block),
                len);

        scp->st_muaddr += len;
        scp->cur_bp->b_resid -= len;
        scp->st_blkno += (len+scp->offset)/SECTOR_SIZE;
        scp->offset = 0;

        if (scp->cur_bp->b_resid <= 0) {
            fd_strategy_done(scp,0);
            return;
	}
    }
#endif

    if (scp->offset == 0) {
        TPRINTF("<QD>",0);
        fd_strategy_ac(scp,0,fsb);
    } else if (scp->cur_fd->raw_buffer_block == scp->st_blkno) {
        TPRINTF("<QE>",0);
        fd_strategy_x0(scp, FDCAM_NORMAL, fsb);
    } else {
        TPRINTF("<QF>",0);
        bio_read(scp->cur_fd, scp->st_blkno, 1, scp->cur_fd->raw_buffer, 0,
                fd_strategy_x0, scp);
    }
    TPRINTF("<QX>",0);
}


static void 
fd_strategy_a0(FDCAM_CB_TAG tag, enum fdcam_status istat, 
	      struct fdcam_status_block* fsb)
{
    struct strategy_class* scp = tag;
    CPRINTF(FDCAM_TRC1,"\n(SA,resid=%ld,blkno=%ld,bcount=%ld",
            scp->cur_bp->b_resid,
            scp->cur_bp->b_blkno, scp->cur_bp->b_bcount);
    CPRINTF(FDCAM_TRC1,",dir=%s,lun=%d,prt=%d,",
            scp->cur_bp->b_flags&B_READ?"R":"W",
            scp->cur_lun, scp->cur_prt);
    CPRINTF(FDCAM_TRC1,"bp@0x%08lx,scp@0x%08lx,offset=%ld)", scp->cur_bp,
            scp, scp->offset);

    if (scp->cur_fdp->plock == 1)
        scp->cur_fdp->plock = 0;

    if (istat) {
        struct buf* tmp = scp->cur_bp;
        IEPRINTF("(fds-fail-3)",0);
        scp->cur_bp->b_flags |= B_ERROR;
        scp->f->st_last_in_error = 1;
        scp->cur_bp->b_error = ENODEV;
        delete_strategy_class(scp);
	if (fdi0.st_already_acquired > 10)
	    fdi0.st_already_acquired--;
        biodone(tmp);
        return;
    }

    tcheck(scp->cur_fd, scp->cur_vr?OS_PROBED:OS_PTBL_READ, 0, 0,
            scp->cur_dev, fd_strategy_ab, scp);
}

void 
fd_bstrategy(struct buf* bp)
{
    /* OSF:                                                                  */
    /* If strategy is called from an NFS interrupt routine, the following    */
    /* might cause trouble since it allocates memory via a routine not       */
    /* designed to be called from interrupt level.                           */
    struct strategy_class* scp;
    VPRINTF("(Strategy,bp@0x%08lx)",bp);
    scp = new_strategy_class(&fdi0, bp->b_dev, bp);

    if (!scp || scp->cur_fd->os == OS_DISABLED
            || scp->cur_fdp->p_nopen[0]+scp->cur_fdp->p_nopen[1] <= 0) {
        CPRINTF(FDCAM_TRACE,"(S0-ERROR,scp=%s,os=%d)",scp?"ok":"null",
                scp?scp->cur_fd->os:99,  0);

        if (scp && scp->cur_fdp->plock == 1)
            scp->cur_fdp->plock = 0;
        bp->b_flags |= B_ERROR;
        fdi0.st_last_in_error = 1;
        bp->b_resid = bp->b_bcount;
        bp->b_error = ENODEV;
        if (scp)
            delete_strategy_class(scp);
	if (fdi0.st_already_acquired > 10)
	    fdi0.st_already_acquired--;
        biodone(bp);
        return;
    }

    if (bp->VALID_CHECK == &thomas) {
        scp->offset = T_OFFSET(bp);
        if (scp->offset >= SECTOR_SIZE)
            IEPRINTF("(s0,734)",0);
        T_OFFSET(bp) = (scp->offset+bp->b_bcount) & OFFSET_MASK;
    } else {
        scp->offset = 0;
    }
    
    CPRINTF(FDCAM_TRC1,"\n(S0,resid=%ld,blkno=%ld,bcount=%ld", bp->b_resid,
            bp->b_blkno, bp->b_bcount);
    CPRINTF(FDCAM_TRC1,",dir=%s,lun=%d,prt=%d,", bp->b_flags&B_READ?"R":"W",
            scp->cur_lun, scp->cur_prt);
    CPRINTF(FDCAM_TRC1,"bp@0x%08lx,scp@0x%08lx,offset=%ld)", bp, scp,
            scp->offset);

                                       /* As we work our way through, b_resid*/
                                       /*  will keep track of number of bytes*/
                                       /*  remaining to transfer.            */
    bp->b_resid = bp->b_bcount;

    if (fdi0.st_already_acquired == 1 || fdi0.st_already_acquired == 10) {
        fdi0.st_already_acquired++;
        fd_strategy_a0(scp,0,&scp->cur_fd->fsb);
    } else {
        fdcam_acquire_port(&scp->cur_fd->fsb,fd_strategy_a0,scp);
    }

    VPRINTF("(End_Strategy,bp@0x%08lx,",bp);
    VPRINTF("ac=%d)",fdi0.st_already_acquired);
}


/* ************************************************************************* */

/* TODO:  CHECK OUT THE FOLLOWING... */
struct driver fdidriver = {
    fd_probe,
    fd_slave,			/* slave */
    fd_attach,			/* controller attach */
    (int(*)())0,		/* device attach */
    (int(*)())0,
    fd_csr,
    0,		                /* name of a device */
    fd_dinfo,                   /* backpointers to driver structs */
    "fdi",			/* name of controller */
    fd_info,             	/* backptrs to controller structs */
};

void 
fdcam_nothing()
{ /* do nothing */; }

