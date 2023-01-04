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
static char *rcsid = "@(#)$RCSfile: fdcam.c,v $ $Revision: 1.1.10.5 $ (DEC) $Date: 1993/12/09 20:25:43 $";
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
 * File:        fdcam.c
 * Date:        07-Aug-1991
 * Author:      Roger Morris
 *
 * Note:        For complete documentation, see header files fdi.h, fdcam.h,
 *              fdcamreg.h, and fdcamidc.h, and DITROFF files fd_fs.h
 *              and fdcam_fs.h.  Redundant documentation is kept to a minimum.
 *
 * Description:
 *      FDCAM --  Floppy Driver Common Access Method
 *
 * Modification History:
 *    Version     Date          Who     Reason
 *
 *      1.00    12-May-91       rsm     Creation date.  Just to start the
 *                                      process.
 *
 *      1.01    04-Sep-91       rsm     Update comments.
 *
 *      1.02    08-Oct-91       rsm     Fix track-80 problem and LUN-2 problem.
 *
 *              10-Oct-91       rsm     comment update.
 *
 *              10-Oct-91       rsm     removed some data redundancies from
 *                                      the code.
 *
 *              10-Oct-91       rsm     moved some things from fdcam.h to
 *                                      fdcamidc.h to make cleaner, and now
 *                                      include fdcamidc.h here.
 *
 *      4       15-Oct-91       rsm     Added fdcam_motor_off().  Changed motor
 *                                      timer state machine to allow for a
 *                                      sooner motor-off after close.
 *
 *              15-Oct-91       rsm     improved error reporting.
 *
 *              14-Nov-91       rsm     removed loading of slot register so
 *                                      any hardware fix will work.
 *
 *              25-Nov-91       rsm     protected accessing of ssr with spl calls.
 *
 */

int fdcam_ver = 5;

#define EXAGERATE_DELAY 0              /* Increase motor-on and motor-off    */
                                       /*  timing to track down timer state  */
                                       /*  machine errors.                   */

#define CODE_DMA 1                     /* 1 == Include code for using DMA    */
                                       /*  engine for sector-data transfers. */
#define CODE_POLL 0                    /* 1 == Include code for using polled */
                                       /*  data transfers for sector data.   */

/* #define TESTING_DEALLOC 1	       /* Testing dma's zfree code 	     */

#include <vm/vm_kern.h>
#include <sys/signal.h>                /* Required by user.h, which is       */
                                       /*  included by proc.h.               */
#include <sys/proc.h>
#include <sys/param.h>
#include <kern/kalloc.h>
#include <sys/kernel.h>
#include <vm/pmap.h>
#include <io/common/devdriver.h>

#include <io/dec/fdi/fdcamidc.h>
#include <io/dec/fdi/fdcam.h>
#include <io/dec/fdi/fdcamreg.h>

extern int cold;			/* set to 1 if system is booting */

typedef void (*sb_l_cb) (struct fdcam_status_block*, long);
typedef void (*fc_l_cb) (struct fdcam_class*, long);

#define FDCAM_READ(base, offset)   (READ_BUS_D8((vm_offset_t)(base + offset)))

#define FDCAM_WRITE(base, offset, value)\
			(WRITE_BUS_D8((vm_offset_t)(base + offset), (long)value))
#define FDCAM_SYNC()	(mb())

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/*                                                           PRIVATE CODE    */


/* Convert micro seconds to clock ticks, rounding up or down.                */
#define FD6_TO_TICKS_RD(A) ((((A)/1000)*hz)/1000)
#define FD6_TO_TICKS_RU(A) ((((((A)+999)/1000)+1000/hz-1)*hz)/1000)
                                       /* a little overflow prevention there.*/

#define FSBCK_MAGIC 1137               /* Placed in the fsb as a debugging   */
                                       /*  aid.                              */

/* The following routine performs no operation, it simply returns.  It is    */
/*  defined in another file and thus can be used to prevent optimization.    */
extern void fdcam_nothing();

/*****************************************************************************/
/*                                                              print_stat() */
/*****************************************************************************/

#if FDCAM_DEBUG
/* The following list is indexed by type ``enum fdcam_status''.              */
static char* ps_list[] = {
    "FDCAM_NORMAL","FDCAM_FAILURE","FDCAM_ABORT","FDCAM_MEDIA_TAMPER",
    "FDCAM_PROGRAMMING_ERR","FDCAM_INT_PROG_ERR","FDCAM_INVALID_SPEC",
    "FDCAM_NO_MEDIA","FDCAM_WRITE_PROTECT","FDCAM_READ_ERROR",
    "FDCAM_FOREIGN_FORMAT","FDCAM_HARDWARE_ERR","STAT-OOR" 
};

/* The following returns a static string which describes the passed status   */
/*  code.  The status code is type ``enum fdcam_status'', cast to an ``int''.*/

static char* 
print_stat(int stat)
{
    if (stat < 0 || stat > 11)
        return "STAT-OOR";
    else
        return ps_list[stat];
}
#endif

/*****************************************************************************/
/* The following is used if ``avail_fms'' is not provided.                   */

struct fdcam_media_specs* end_of_fms_list = 0;


/*****************************************************************************/
struct fdcam_media_specs fms_default = {
    1,                                 /* char supported;                    */
    0,                                 /* char surf;                         */
    0,
    2,                                 /* unsigned short n_surfaces;         */
    18,                                /* unsigned short sectors_p_track; */
    80,                                /* unsigned short cylinders_p_disk;   */
    SECTOR_SIZE,                       /* unsigned short bytes_p_sector;     */
    FDCAM_XFER_500,                    /* enum fdcam_xfer_rate xfer_rate;    */
    0,                                 /* char media_id_must_match;          */
    0,                                 /* FDCAM_PIN media_id;                */
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
    "Undefined-Media"                  /* char* diag_name;                   */
};
/*****************************************************************************/

struct intr_result {                   /* This structure is used to store    */
                                       /*  the results of an interrupt.      */
    int rbc;                           /* Number of result bytes read after  */
                                       /*  interrupt.  Valid after iwait has */
                                       /*  been set to 4.                    */
#define RB_LIMIT 12
    unsigned char rb[RB_LIMIT];        /* The result bytes.  The first       */
                                       /*  ``rbc'' bytes are loaded at the   */
                                       /*  time of the interrupt.            */
    unsigned char intr_st0;            /* The ST0 result byte from the       */
                                       /*  sense-interrupt-status command.   */
                                       /*  Unmodified if command not issued. */
    unsigned char intr_pcn;            /* The PCN result byte from the       */
                                       /*  sense-interrupt-status command.   */
                                       /*  Unmodified if command not issued. */
    long data_leftover;                /* If a polled transfer, the number   */
                                       /*  of bytes not transfered.          */
};


typedef void (*ihp) (struct fdcam_class*); 

typedef void (*icp) (struct fdcam_class*,void*);


typedef void (*ildc_cb_type) (void*,struct fdcam_class*, long);

/*****************************************************************************/

#if FDCAM_DEBUG

static void drop_dead(int error_code);

static void
safe_fc_l_cb(struct fdcam_class* a, long c)
{
    IEPRINTF("(SAFE_fc_l_cb)",0);
    drop_dead(189);
}

static void
safe_sb_l_cb(struct fdcam_status_block* a, long c)
{
    IEPRINTF("(SAFE_sb_l_cb)",0);
    drop_dead(188);
}

static void safe_f_cb(FDCAM_CB_TAG a, enum fdcam_status b, 
		      struct fdcam_status_block* c)
{
    IEPRINTF("(SAFE_F_CB)",0);
    drop_dead(124);
}

#endif


/*****************************************************************************/
/* The following is used to designate transfer type when a common routine is */
/*  used for both read and write.                                            */

enum enum_op { OP_READ, OP_WRITE, OP_VERIFY };

/*****************************************************************************/
/* The following is used by the interrupt handler to indicate what action    */
/*  to perform at interrupt time.                                            */

enum ildc_mode {
    IM_NO_INT,                         /* No Do not wait for interrupt, enter*/
                                       /*  result phase immediately          */
    IM_INT,                            /* Perform result phase at interrupt  */
                                       /*  level.                            */
    IM_INT_SIS,                        /* Perform result phase at interrupt  */
                                       /*  level, followed by a sense-intr-  */
                                       /*  status command.                   */
    IM_RESET,                          /* Perform chip reset, then at intr   */
                                       /*  level, perform four sense-intr-   */
                                       /*  status commands.                  */
    IM_RESET_EIM,                      /* Same as IM_RESET, but that         */
                                       /*  interrupts are enabled (via irm)  */
                                       /*  during the reset pulse.           */
    IM_RECOVERY,                       /* Timeout occured from IM_*INT* mode,*/
                                       /*  now expecting interrupt from      */
                                       /*  reset signal generated to recover */
                                       /*  from this timeout.                */
    IM_IWS_INT,                        /* Same as IM_INT, but spins while    */
                                       /*  waiting for interrupt.            */
    IM_IWS_INT_SIS,                    /* Same as IM_INT_SIS, but spins while*/
                                       /*  waiting for interrupt.            */
    IM_IWS_RESET,                      /* Same as IM_RESET, but spins while  */
                                       /*  waiting for interrupt.            */
    IM_IWS_RESET_EIM                   /* Same as IM_RESET_EIM, but spins    */
                                       /*  while waiting for interrupt.      */
};


/*****************************************************************************/
/* The following is used to form a linked list of processes waiting to use   */
/*  the FDI hardware.                                                        */

struct waiting_spec {
    FDCAM_ACQUIRE_CB cb;
    void* tag1;
    struct fdcam_status_block* fsb;
    struct waiting_spec* next;
};

/*****************************************************************************/
/* The following is the base class from which all static information is      */
/*  stored or pointed to.                                                    */

struct fdcam_class {
    /* private: */
    volatile char* r;                  /* Pointer to 82077A chip.  This is   */
                                       /*  the first element of this struct  */
                                       /*  for possibility of increased      */
                                       /*  performance.                      */
    int up_to_date;                    /* 1 == Register copies up to date    */
                                       /* 0 == not up to date.               */

    struct fdcam_82077aa rc;           /* A place to keep copies of regs.    */

    UNS8 specify_cmd[CBN_SPECIFY];     /* A copy of the last such cmd issued.*/
    UNS8 config_cmd[CBN_CONFIGURE];    /*  "                                 */
    UNS8 perpen_cmd[CBN_PERPEND_MODE]; /*  "                                 */

    UNS8 select_specify_1;             /* Used by fdcam_select().            */
    UNS8 select_specify_2;             /*  "                                 */
    UNS8 select_configure_2;           /*  "                                 */
    UNS8 select_configure_3;           /*  "                                 */
    UNS8 select_perpen_1;              /*  "                                 */
    UNS8 select_dsr;                   /*  "                                 */
    UNS8 select_dor;                   /*  "                                 */
    UNS8 select_ccr;                   /*  "                                 */
    UNS8 select_buf[8];                /*  "                                 */
    int  select_motor_req;             /*  "                                 */
    sb_l_cb select_cb;                 /*  "                                 */
    void* select_tag;                  /*  "                                 */

    char llu_tampered;                 /* Temporary variable used by         */
                                       /*  ll_update().                      */
    char llu_autoprobe;                /*  "                                 */
    sb_l_cb llu_cb;                    /*  "                                 */
    void* llu_cb_tag;


    long motor_on_delay;               /* Motor-on delay, in ticks,          */
    long motor_off_delay;

    fc_l_cb mo_cb;
    void* mo_tag;
    int mo_state;                      /* 0  == off                          */
                                       /*  1 == spinning up, next state is 4 */
                                       /*  8 == up to speed, next state is 4 */
                                       /*  2 == spinning up                  */
                                       /*  3 == up to speed                  */
                                       /*  4 == Timeout to spin-down.        */


    sb_l_cb seek_cb;
    void* seek_tag;
    long seek_cyl;
    int seek_wh;

    sb_l_cb ll_recal_cb;
    void* ll_recal_tag;
    int ll_recal_n_attempt;

    struct	sglist *sglp;		/* Used to communicate to the generic*/
                                        /* DMA functions		     */
    unsigned char* std_buf;		/* buffer used for initial read of   */
                                        /* disk for density detection.  Also */
                                        /* used for formatting a track.	     */

    enum enum_op at_op;
    long at_start_psn;
    int  at_pass;
    int  at_sw_intl; 
    int  at_first;
    long at_psn;
    long at_nsec;
    unsigned short at_prev_cyl;
    unsigned short at_prev_head;
    unsigned char* at_buffer;
    unsigned char* at_start_buf;
    struct proc* at_process;
    char at_did_timo;

    int fft_cylinder;
    int fft_surface;
    int fft_xsize;
    int fft_dma_retry;
    int fft_index;
    unsigned long fft_idma_addr;
    struct fdcam_format_spec fft_ffs;
    int fft_ffs_ok;

    unsigned long rw_idma_addr;
    enum enum_op rw_op;
    char rw_use_dma;                   /* Non zero to use dma hardware.      */
    char rw_do_seek;
    int rw_retry;
    int rw_max_retries;			/* max number to retry operation */
    int rw_dma_retry;
    int rw_surface;
    int rw_cylinder;
    int rw_sector;
    int rw_count;
    unsigned char* rw_buf;
    struct proc* rw_proc;
    sb_l_cb rw_cb;
    void* rw_tag;
    unsigned char rw_cmd[CBN_RW_DATA];

    void* interface_number;            /* Tags passed in from fd level.      */
    void* fd_tag_1;                    /*  "                                 */
    void* fd_tag_2;                    /*  "                                 */

    struct waiting_spec* waiting;
    struct waiting_spec* free;

    struct caller_spec cs;

    long seek_c;
    int seek_w;

    unsigned long dead_code;

    enum gs_enum {
        GS_NO_INIT = 0,                /* Everything is unitialized.  Any    */
                                       /*  unexpected interrupts should be   */
                                       /*  cleared, masked out, and ignored. */
        GS_NEED_CTOR,                  /* We are almost running.             */
        GS_INIT,                       /* This structure is itialized.  Here */
                                       /*  we can assume that the pointers   */
                                       /*  hereinbefore declared have been   */
                                       /*  initialized to point to hardware  */
                                       /*  registers.                        */
        GS_DEAD                        /* Something very bad has happened.   */
                                       /*  Let's just give it up.  Clear,    */
                                       /*  mask, and ignore any interrupts.  */
                                       /*  Assume pointers are still init.   */
    } great_state;	               /* This should be initialized to 0.   */

    char busy;                         /* 1 if driver busy, 0 otherwise.     */

/* ************************************************************************* */
/*                                        INTERRUPT-HANDLER STUFF            */
/* The following three, ``iwait'', ``ir'', and ``unexp_intr'', are only for  */
/*  use by fdcam_intr() and by wait_for_intr().                              */

    ihp intr_hdlr;                     /* The actual interrupt handler.      */
                                       /*  The function pointed to by this   */
                                       /*  will always be called as the      */
                                       /*  interrupt handler if great_state==*/
                                       /*  INIT.  A typical value for this   */
                                       /*  would be the address of int1().   */

    char unxp_intr;                    /* 0x01 bit set if unexpected result- */
                                       /*  phase interrupt occurred.         */
                                       /*  0x02 bit set if an execution-phase*/
                                       /*  interrupt occurred.               */
                                       /*  0x04 bit set if other error.      */

/* ************************************************************************* */
/*                                      ILDC / ILXC stuff...                 */
    int     ildc_rsize;
    sb_l_cb   ildc_cb;
    void*   ildc_cb_tag;
    ihp     ildc_old_hdlr;
    struct  intr_result ildc_ir;
    int     ildc_err;

#ifdef CODE_POLL
    unsigned char* ildc_u_buf;         /* Ilxc only...                       */
    struct proc*   ildc_u_proc;       
    long           ildc_ub_size;
    enum enum_op   ildc_dir;
    long           ildc_count_down;
    long           ildc_orig_cd;
    unsigned char* ildc_working_ptr;
#endif /* CODE_POLL */

    enum ildc_mode ildc_sis;           /* Ildc only...                       */


/* ************************************************************************* */

    int intr_err_count;

/* ************************************************************************* */

    struct controller *ctlr;           /* Place to store fdcam_probe params. */
};

struct fdcam_class fc_0;               /* This implementation only supports  */
                                       /*  one FDI interface, so lets define */
                                       /*  one object and put it here.       */


struct fdcam_status_block fdcam_gen_fsb = { &fc_0 };


#if FDCAM_DEBUG

#define PAUSE microdelay(20)

extern unsigned long fdcam_debug_mask;
extern unsigned long fd_trigger;
extern unsigned long fd_cbuf;
extern unsigned long fd_foo;

/* The following is used to log simple error codes for debugging.  The first */
/*  five numbers in the following are simply magic to help locate this array */
/*  within a dump.  The sixth element is the address of fc_0.                */
/*  The seventh is the address of the last fsb initialized.                  */
/*  The array is initialized to all ones, with a null term.                  */
unsigned long fdcam_diag[9+100+2] = {
    0x7938, 0x7328, 0x4893, 0x0238, 0x0001, (unsigned long)(&fc_0), 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0 
};                         /* Extra zeros just to make re-ent.   */

unsigned long* fdcam_diag_p = fdcam_diag+8;

int 
fdcam_diag_f(int err)
{
    if (*fdcam_diag_p)
        *fdcam_diag_p++ = err;

    if (fc_0.dead_code) {
        if (!(fc_0.dead_code & 0x0000FF00))
            fc_0.dead_code |= err<<8;
        else if (!(fc_0.dead_code & 0x00FF0000))
            fc_0.dead_code |= err<<16;
    }
}

/* The following may be treated as an expression as long as it does not      */
/*  appear more than once withing an expression.                             */
# define FDCAM_DIAG(A) (CPRINTF(FDCAM_TRACE," FDCAM_DIAG(%ld) ",A,0,0), \
        *fdcam_diag_p ? (*fdcam_diag_p++=(A)) : 0L)
#else /* FDCAM_DEBUG */
# define PAUSE /* */
# define FDCAM_DIAG(A) (0L)
#endif /* FDCAM_DEBUG */


/* The following initializes ``struct fdcam_class'' objects.                 */

static void 
fix_regp(struct fdcam_class** fcpp)
{
    if (!*fcpp)
        *fcpp = &fc_0;

    if ((*fcpp)->great_state < GS_INIT) 
	/* Hardwire base of 82077 to be 0x3f0 */
        /* and tack on the sysmap for the io_handle */
        (*fcpp)->r = (char *) ((u_long)(0x3f0)
		| ((u_long)(*fcpp)->interface_number & 0xffffffff00000000));
}

static void 
fdcam_class_ctor(struct fdcam_class* fcp, ihp default_intr_hdlr)
{
    vm_offset_t phys;

    fix_regp(&fcp);

    fcp->intr_hdlr = default_intr_hdlr;
    fcp->great_state = GS_INIT;
    fcp->dead_code = 0;

#if 0                                  /* The following will be used before  */
                                       /*  ctor is called, so don't init.    */
                                       /*  them here.                        */
    fcp->busy = 0;
    fcp->waiting = 0;
    fcp->free = 0;
#endif

    fcp->intr_err_count = 0;
    fcp->mo_state = 0;

#if FDCAM_DEBUG
    fcp->select_cb   = safe_sb_l_cb;
    fcp->mo_cb       = safe_fc_l_cb;
    fcp->seek_cb     = safe_sb_l_cb;
    fcp->rw_cb       = safe_sb_l_cb;
    fcp->ildc_cb     = safe_sb_l_cb;
    fcp->cs.cs_cb    = safe_f_cb;
    fcp->ll_recal_cb = safe_sb_l_cb;
#endif

    TPRINTF("((ALLOC-15",0);

    fcp->std_buf = (unsigned char*)kalloc(SECTOR_SIZE);
    svatophys(fcp->std_buf,&phys);
    fcp->std_buf = (unsigned char*)PHYS_TO_KSEG(phys);

    fcp->ildc_old_hdlr = 0;
    
    fcp->rc.sra = 0x00;                /* Status Register A                  */
    fcp->rc.srb = 0x00;                /* Status Register B                  */
    fcp->rc.dor = DOR_NORMAL;          /* Digital Output Register            */
    fcp->rc.tdr = 0x00;                /* Tape Drive Register                */
    fcp->rc.msr = 0x00;                /* Main status Register               */
    fcp->rc.dsr = DSR_NORMAL;          /* Data Rate Select Register          */
    fcp->rc.dat = 0x00;                /* Data (FIFO)                        */
    fcp->rc.res = 0x00;                /* Reserved                           */
    fcp->rc.dir = 0x00;                /* Digital Input Register             */
    fcp->rc.ccr = 0x00;                /* Configuration Control Register     */
    fcp->up_to_date = 0;               /* Neets updating.                    */
    fcp->fft_ffs_ok = 0;
#if EXAGERATE_DELAY
    fcp->motor_on_delay  = FD6_TO_TICKS_RU(2500000);
    fcp->motor_off_delay = FD6_TO_TICKS_RU(25000000);
#else
    fcp->motor_on_delay  = FD6_TO_TICKS_RU(250000);
    fcp->motor_off_delay = FD6_TO_TICKS_RU(5000000);
#endif
    fcp->config_cmd[0] = CB0_CONFIGURE;
    fcp->config_cmd[1] = CB1_CONFIGURE;
    fcp->specify_cmd[0] = CB0_SPECIFY;
    fcp->perpen_cmd[0] = CB0_PERPEND_MODE;
};

static void 
fdcam_class_dtor(struct fdcam_class* fcp)
{
    fcp->great_state = GS_NO_INIT;
};

/* ************************************************************************* */
/*                                              shut_it_down(), drop_dead()  */
/* ************************************************************************* */

/* The following causes the hardware to generate no more interrupts or DMA   */
/*  cycles.                                                                  */

static void 
shut_it_down(struct fdcam_class* fcp, int permanent, int error_code)
{
    int s = splhigh();
    fix_regp(&fcp);

    /* dealloc mem. resources on shutdown of fdi */
    if (fcp->rw_use_dma) {
	dma_map_dealloc(fcp->sglp);
    }

    if (error_code)
        FDCAM_DIAG(error_code);

    if (permanent) {
        if (fcp->dead_code == 0)
            fcp->dead_code = error_code;

	printf("<FDI: I've fallen and I can't get up, ERROR #%d ", error_code);
        printf(" sra:%08lx dor:%08lx msr:%08lx>\n",
	       FDCAM_READ(fcp->r, SRA),
	       FDCAM_READ(fcp->r, DOR),
	       FDCAM_READ(fcp->r, MSR));
    }

    fcp->up_to_date = 0;

    splx (s);
    
                                       /* Reset the 82077AA                  */
    FDCAM_WRITE(fcp->r, DOR, DOR_NORMAL & ~DOR_RESET_i);
    FDCAM_SYNC();
    microdelay(4);                     /* Wait for worst-case reset.         */
    PAUSE;
    fcp->rc.dor = DOR_NORMAL;
    FDCAM_WRITE(fcp->r, DOR, DOR_NORMAL);
    FDCAM_SYNC();
    microdelay(4);                     /* Wait for worst-case reset.         */
    PAUSE;
    
    if (permanent) {		       /* Put 82077A in power-down state.    */
	FDCAM_WRITE(fcp->r, DSR, DSR_POWER_DOWN);
	FDCAM_SYNC();
    }
}


/* The following is called from interrupt level if there is some big problem.*/
/*  To the extent possible, the FDI interface is shut down and prevented     */
/*  from generating interrupts or DMA cycles.  Various flags are set to      */
/*  indicate that everything has been shut down.                             */
static void 
drop_dead(int error_code)
{
    shut_it_down(&fc_0, 1, error_code);

    fc_0.great_state = GS_DEAD;        /* Indicates FDI dead.                */

    fc_0.unxp_intr |= 0x04;

}


/* ************************************************************************* */

int 
fdcam_port_is_free(struct fdcam_status_block* fsb)
{ 
    return !fsb->fcp->busy;
}

enum fdcam_status 
fdcam_acquire_port(struct fdcam_status_block* fsb, FDCAM_ACQUIRE_CB cb, 
		   void *tag1)
{
    struct fdcam_class* fcp = fsb->fcp;
    int s = splhigh();
    VPRINTF("(ap10)",0);
    if (!fcp->busy) {                /* This is by far the most likely     */
                                       /*  flow of control, so the "else"    */
                                       /*  code need not be that efficient.  */
        fcp->busy = 1;
        splx(s);
        if (cb)
            (*cb)(tag1,FDCAM_NORMAL,fsb);
        VPRINTF("(ap40)",0);
        return FDCAM_NORMAL;
    } else {
        struct waiting_spec* my_link;
        VPRINTF("(ap50)",0);

        if (! fcp->free) {
            VPRINTF("(ap51)",0);
	    fcp->free = (struct waiting_spec*)kalloc(
		     sizeof(struct waiting_spec));
            fcp->free->next = 0;
	}

        my_link = fcp->free;
        fcp->free = fcp->free->next;
        my_link->cb = cb;
        my_link->tag1 = tag1;
        my_link->fsb = fsb;

        if (fsb == &fdcam_gen_fsb) {
            struct waiting_spec** p;
            struct waiting_spec** x;
            for (x = p = &fcp->waiting ; *p ; p = &(*p)->next)
                if ((*p)->fsb == &fdcam_gen_fsb)
                    x = &(*p)->next;
            my_link->next = *x;
            *x = my_link;
	} else {
            struct waiting_spec** p;
            for (p = &fcp->waiting ; *p ; p = &(*p)->next)
                ;
            my_link->next = *p;
            *p = my_link;
	}

        VPRINTF("(ap55)",0);
        if (cb) {
            splx(s);
            return FDCAM_NORMAL;
	} else {
            while (fcp->busy) {
		sleep((caddr_t)my_link, PRIBIO + 1);
	    }
            fcp->busy = 1;
            VPRINTF("(ap60)",0);
            if (fcp->waiting == my_link) {
                fcp->waiting = fcp->waiting->next;
                my_link->next = fcp->free;
                fcp->free = my_link;
	    } else {
		splx(s);
                return FDCAM_DIAG(11), FDCAM_FAILURE;
	    }

            VPRINTF("(ap70)",0);
            splx(s);
            return FDCAM_NORMAL;
	}
    }
}

void 
fdcam_release_port(struct fdcam_status_block* fsb)
{
    struct fdcam_class* fcp = fsb->fcp;
    int s = splhigh();
    VPRINTF("(rp)",0);
    if (fcp->waiting) {
        struct waiting_spec* link = fcp->waiting;

        if (link->cb) {
            struct waiting_spec copy_of;
            copy_of = *link;

            fcp->waiting = fcp->waiting->next;
            link->next = fcp->free;
            fcp->free = link;
            splx(s);
            (*copy_of.cb)(copy_of.tag1,FDCAM_NORMAL,copy_of.fsb);
	} else {
            fcp->busy = 0;
            wakeup(link);
            splx(s);
	}
    } else {
        fcp->busy = 0;
        splx(s);
    }
}


/* ************************************************************************* */
/* The following makes entry and exit from FDCAM routines easier to code.    */
/* Upon entry, ``fdcam_user_init'' should be called.                         */
/* If the all operations are completed and return is immediate, exit should  */
/* comprise of `` return fdcam_user_return(fcp,error_status); ''             */
/* If operations are to be completed from an interrupt routine, then the     */
/* main routine should end with `` return fdcam_user_wait(fcp); '', and the  */
/* final interrupt routine should end with                                   */
/* ``fdcam_user_done(0,fcp,error_status);''                                  */

void 
fdcam_user_init(struct caller_spec* csp, struct fdcam_status_block* fsb,
		FDCAM_CB cb, FDCAM_CB_TAG cb_tag)
{
    CPRINTF(FDCAM_TRC1,"(ui)",0,0,0);
    csp->fsb = fsb;
#if FDCAM_DEBUG
    if (fdcam_port_is_free(fsb))
        IEPRINTF("(NO Port Reserve -- ui)",0);
    if (csp->cs_cb != safe_f_cb) {
        IEPRINTF("(DOUBLE UI, 0x%08lx)",csp->cs_cb);
        drop_dead(120);
    }
#endif
    csp->cs_cb = cb;
    csp->cs_cb_tag = cb ? cb_tag : 0;
}

enum fdcam_status 
fdcam_user_wait(struct caller_spec* csp)
{
    VPRINTF("(uw)",0);

    if (csp->cs_cb)
        return FDCAM_NORMAL;
    else {
        while (!csp->cs_cb_tag) {
	    /* if we are still booting just delay */
	    if (cold) {
		DELAY(20);
	    } else {
		sleep((caddr_t)(&csp->cs_cb), PRIBIO + 1);
	    }
	}
#if FDCAM_DEBUG
	csp->cs_cb = safe_f_cb;
#endif
        return csp->cs_results;
    }
}

enum fdcam_status 
fdcam_user_return(struct caller_spec* csp, enum fdcam_status results)
{
    FDCAM_CB x = csp->cs_cb;
#if FDCAM_DEBUG
      csp->cs_cb = safe_f_cb;
#endif
    VPRINTF("(ur)",0);

    if (x) {
        (*x)(csp->cs_cb_tag,results,csp->fsb);
        return FDCAM_NORMAL;
    } else
        return results;
}

void 
fdcam_user_done(struct caller_spec* csp, enum fdcam_status results)
{
    VPRINTF("(ud)",0);

    if (csp->cs_cb) {
        FDCAM_CB x = csp->cs_cb;
#if FDCAM_DEBUG
          csp->cs_cb = safe_f_cb;
#endif
        (*x)(csp->cs_cb_tag,results,csp->fsb);
    } else {
        csp->cs_results = results;
        csp->cs_cb_tag = (void*)4;
        wakeup(&csp->cs_cb);
    }
}


static void 
user_done(void* ignored, struct fdcam_class* fcp, enum fdcam_status results)
{
    fdcam_user_done(&fcp->cs,results);
}


static void 
f_s_user_done(struct fdcam_status_block* fsb, long results)
{
    fdcam_user_done(&fsb->fcp->cs,(enum fdcam_status)results);
}


/* ************************************************************************* */
/*                                                       MOTOR TIMER         */
/* ************************************************************************* */

static void 
motor_timo(struct fdcam_class* fcp)
{
    CPRINTF(FDCAM_TRC1, "<motor_timo,%d,\007>", fcp->mo_state, 0, 0);

    untimeout(motor_timo,fcp);

    if (fcp->mo_state == 1 || fcp->mo_state == 8) {
        fcp->mo_state = 4;
        timeout(motor_timo, fcp, fcp->motor_off_delay);
        if (fcp->mo_cb)
            (*fcp->mo_cb)(fcp->mo_tag,FDCAM_NORMAL);
        fcp->mo_cb = 0;
    } else if (fcp->mo_state == 2) {
        fcp->mo_state = 3;
        if (fcp->mo_cb)
            (*fcp->mo_cb)(fcp->mo_tag,FDCAM_NORMAL);
        fcp->mo_cb = 0;
    } else if (fcp->mo_state == 4) {
        fcp->mo_state = 0;
        fcp->rc.dor &= ~DOR_MOT_EN;
	FDCAM_WRITE(fcp->r, DOR, fcp->rc.dor);
	FDCAM_SYNC();
    } else
        FDCAM_DIAG(60);
}


/*****************************************************************************/
/* The following sets up the floppy motor as requested, and calls the call-  */
/*  back when done.                                                          */
/*                                                                           */
/* The following values may be retuned, possibly via callback.               */
/*    FDCAM_NORMAL            All went well, motor has reached the requested */
/*                             state.                                        */
/*    FDCAM_PROGRAMMING_ERR   One of these parameters out of range:          */
/*                             ``request''                                   */

static void
motor_req(struct fdcam_class* fcp, 
	  int request,	               /* 0  == don't care about motor.      */
                                       /* 1  == No action if motor already   */
                                       /*        spinning.  Otherwise,       */
                                       /*        Need motor spinning, don't  */
                                       /*        need it to be up to speed,  */
                                       /*        and once up to speed, go    */
                                       /*        directly to spin-down       */
                                       /*        sequence.                   */
                                       /* 2  == Need motor spinning, but     */
                                       /*        don't need it to be up to   */
                                       /*        speed.                      */
                                       /* 3  == Need motor up to speed.      */
                                       /* 7  == go to immediate spin-down    */
                                       /*        if ok to do so.             */
                                       /* 9  == Begin spin-down sequence.    */
                                       /* 6  == Error condition, cancel all  */
                                       /*        timers and shut down now.   */

	  fc_l_cb cb,	               /* May be zero if request == 1,7,8,9  */
	  void* tag)
{
    if (request == 0  &&  cb)
        (*cb)(tag,FDCAM_NORMAL);
    else {
        int s = splclock();
        int x = fcp->mo_state;
        CPRINTF(FDCAM_TRACE,"<motor_req:%d,s=%d>",request,x,0);
        if (request == 1) {
            if (x == 0) {
		fcp->rc.dor |= DOR_MOT_EN;
		FDCAM_WRITE(fcp->r, DOR, fcp->rc.dor);
		FDCAM_SYNC();
                fcp->mo_state = 1;
                fcp->mo_cb = 0;
                timeout(motor_timo, fcp, fcp->motor_on_delay);
	    } else if (x == 4) {
                untimeout(motor_timo,fcp);
                fcp->mo_state = 8;
                fcp->mo_cb = 0;
                timeout(motor_timo, fcp, fcp->motor_on_delay);
	    }
            splx(s);
            if (cb)
                (*cb)(tag, FDCAM_NORMAL);
	} else if (request == 2) {
            if (x == 0) {
                fcp->rc.dor |= DOR_MOT_EN;
		FDCAM_WRITE(fcp->r, DOR, fcp->rc.dor);
		FDCAM_SYNC();
                fcp->mo_state = 2;
                fcp->mo_cb = 0;
                timeout(motor_timo, fcp, fcp->motor_on_delay);
	    } else if (x == 1) {
                fcp->mo_state = 3;
                untimeout(motor_timo,fcp);
	    }
            splx(s);
            if (cb)
                (*cb)(tag, FDCAM_NORMAL);
	} else if (request == 3) {
            if (x == 0) {
                splx(s);
                fcp->rc.dor |= DOR_MOT_EN;
		FDCAM_WRITE(fcp->r, DOR, fcp->rc.dor);
		FDCAM_SYNC();
                fcp->mo_state = 2;
                if (fcp->mo_cb)
                    drop_dead(110);
                fcp->mo_cb = cb;
                fcp->mo_tag = tag;
                timeout(motor_timo, fcp, fcp->motor_on_delay);
	    } else if (x == 1 || x == 2) {
                fcp->mo_state = 2;
                if (fcp->mo_cb)
                    drop_dead(111);
                fcp->mo_cb = cb;
                fcp->mo_tag = tag;
                splx(s);
	    } else if (x == 4 || x == 8) {
                untimeout(motor_timo,fcp);
                splx(s);
                fcp->mo_state = 3;
                if (cb)
                    (*cb)(tag, FDCAM_NORMAL);
	    } else { /* (x == 3) */
                splx(s);
                if (cb)
                    (*cb)(tag, FDCAM_NORMAL);
	    }
	} else if (request == 9) {
            if (x == 1 || x == 2 || x == 3 || x == 8) {
                if (fcp->mo_cb) {
		    TPRINTF("(mo_cb should not be set, mo_cb = %lx)",
			   fcp->mo_cb);
                    drop_dead(112);
		}
                untimeout(motor_timo,fcp);
                fcp->mo_state = 4;
                timeout(motor_timo, fcp, fcp->motor_off_delay);
	    }
            splx(s);
            if (cb)
                (*cb)(tag, FDCAM_NORMAL);
	} else if (request == 7) {
            if (x == 1 || x == 4 || x == 8) {
                untimeout(motor_timo,fcp);
                fcp->mo_state = 0;
                splx(s);
                fcp->rc.dor &= ~DOR_MOT_EN;
		FDCAM_WRITE(fcp->r, DOR, fcp->rc.dor);
		FDCAM_SYNC();
                if (cb)
                    (*cb)(tag, FDCAM_NORMAL);
	    }
	} else if (request == 6) {
            untimeout(motor_timo,fcp);
            fcp->mo_state = 0;
            splx(s);
	    fcp->rc.dor &= ~DOR_MOT_EN;
	    FDCAM_WRITE(fcp->r, DOR, fcp->rc.dor);
	    FDCAM_SYNC();
            if (cb)
                (*cb)(tag, FDCAM_NORMAL);
	} else {
            FDCAM_DIAG(142);
            if (cb)
                (*cb)(tag, FDCAM_INVALID_SPEC);
	}
    }
}


/* ************************************************************************* */


static int 
make_precomp(long precomp_ns)
{
    if (precomp_ns < 0)
        return 0;                      /* use default.                       */
    else if (precomp_ns < 21)
        return 7;
    else if (precomp_ns > 250)
        return 6;
    else
        return (precomp_ns*3+63)/125;
}



/* Return -1 if error */
static int 
make_gap3(int fdcam_user_gap3, enum fdcam_xfer_rate drate, 
	  int is_format)
{
    if (fdcam_user_gap3 >= 0)
        return fdcam_user_gap3;
    else if (drate == FDCAM_XFER_250)
        return is_format ? 0x54 : 0x1B;
    else if (drate == FDCAM_XFER_500)
        return is_format ? 0x54 : 0x1B;
    else if (drate == FDCAM_XFER_1000)
        return is_format ? 0x53 : 0x1B;
    else
        return -1;
}


/* Return -1 if error */
int 
make_drate(int is_mfm, enum fdcam_xfer_rate drate)
{
    if (drate == FDCAM_XFER_1000)
        return is_mfm ? 3 : -1 ;
    else if (drate == FDCAM_XFER_500)
        return is_mfm ? 0 : -1 ;
    else if (drate == FDCAM_XFER_300)
        return is_mfm ? 1 : -1 ;
    else if (drate == FDCAM_XFER_250)
        return is_mfm ? 2 :  0 ;
    else if (drate == FDCAM_XFER_150)
        return is_mfm ? -1 : 1 ;
    else if (drate == FDCAM_XFER_125)
        return is_mfm ? -1 : 2 ;
    else
        return -1;
}

static unsigned char 
make_hut(FDCAM_6TIME duration, enum fdcam_xfer_rate drate)
{
    long k;

                                       /* Normalize to units of 0.1 ms for   */
                                       /*  data rate of 1Mb.                 */
    if (drate == FDCAM_XFER_250)
        k = (duration*25)/10000;
    else if (drate == FDCAM_XFER_300)
        k = (duration*3)/1000;
    else if (drate == FDCAM_XFER_500)
        k = (duration*5)/1000;
    else if (drate == FDCAM_XFER_1000)
        k = (duration*1)/100;
    else
        return 0;                      /* Out or range, return worst-case.   */

                                       /* Determing value for HUT register.  */
    if (k > 1200)
        return 0;
    else if (k <= 0)
        return 1;
    else
        return (k+79)/80;
}

static unsigned char 
make_srt(FDCAM_6TIME duration, enum fdcam_xfer_rate drate)
{
    long k;

                                       /* Normalize to units of 0.1 ms for   */
                                       /*  data rate of 1Mb.                 */
    if (drate == FDCAM_XFER_250)
        k = (duration*25)/10000;
    else if (drate == FDCAM_XFER_300)
        k = (duration*3)/1000;
    else if (drate == FDCAM_XFER_500)
        k = (duration*5)/1000;
    else if (drate == FDCAM_XFER_1000)
        k = (duration*1)/100;
    else
        return 0;                      /* Out or range, return worst-case.   */

                                       /* Determing value for SRT register.  */
    if (k > 80)
        return 0x00;
    else if (k < 5)
        return 0x0F;
    else
        return 0x10 - (k+4)/5;
}

unsigned char 
make_hlt(FDCAM_6TIME duration, enum fdcam_xfer_rate drate)
{
    long k;

                                       /* Normalize to units of 0.1 ms for   */
                                       /*  data rate of 1Mb.                 */
    if (drate == FDCAM_XFER_250)
        k = (duration*25)/10000;
    else if (drate == FDCAM_XFER_300)
        k = (duration*3)/1000;
    else if (drate == FDCAM_XFER_500)
        k = (duration*5)/1000;
    else if (drate == FDCAM_XFER_1000)
        k = (duration*1)/100;
    else
        return 0;                      /* Out or range, return worst-case.   */

                                       /* Determing value for HLT register.  */
    if (k > 1270)
        return 0x00;
    else if (k <= 10)
        return 0x01;
    else
        return (k+9)/10;
}



/* ************************************************************************* */
/*                                                   fdcam_select()          */
/* ************************************************************************* */
/* The following sets up any chip parameters that may not be up to date.     */
/* Those are the parameters specified in the SPECIFY, CONFIGURE, and PERPEND */
/*  commands, and those determined by the DSR, DOR, and CCR registers.       */
/*                                                                           */
/* The following values may be retuned (via callback):                       */
/*    FDCAM_NORMAL         All went well.                                    */
/*    FDCAM_INVALID_SPEC   One of these parameters out of range:             */
/*                          fsb->fms->xfer_rate                              */
/*    FDCAM_HARDWARE_ERR   Controller chip not responding.                   */


static void 
fdcam_select_3(struct fdcam_class* fcp, long stat)
{
    sb_l_cb cb = fcp->select_cb;
#if FDCAM_DEBUG
    fcp->select_cb = safe_sb_l_cb;
#endif

    VPRINTF("(fs3,%d)",stat);

    if (stat) {
        drop_dead(113);
	FDCAM_DIAG(54);
	if (cb)
	    (*cb)(fcp->select_tag,FDCAM_HARDWARE_ERR);
	return;
    } else {
	fcp->up_to_date = 1;
	(*cb)(fcp->select_tag,FDCAM_NORMAL);
    }

    VPRINTF("(end_fs3)",0);
}

static void 
fdcam_select_2(struct fdcam_class* fcp, long stat)
{
    sb_l_cb cb = fcp->select_cb;
    VPRINTF("(fs2,%d)",stat);

    if (stat) {
        drop_dead(113);
	FDCAM_DIAG(54);
#if FDCAM_DEBUG
        fcp->select_cb = safe_sb_l_cb;
#endif
	if (cb)
	    (*cb)(fcp->select_tag,FDCAM_HARDWARE_ERR);
	return;
    }

    if (!fcp->up_to_date
	    || fcp->select_specify_1 != fcp->specify_cmd[1]
	    || fcp->select_specify_2 != fcp->specify_cmd[2]) {
	fcp->specify_cmd[1] = fcp->select_specify_1;
	fcp->specify_cmd[2] = fcp->select_specify_2;
	CPRINTF(FDCAM_TRC2,
"(*** select specify_cmd[1] = 0x%02x, specify_cmd[2] = 0x%02x)",
		fcp->specify_cmd[1], fcp->specify_cmd[2], 0);
	if (ildc_no_int(fcp,CBN_SPECIFY,fcp->specify_cmd,RBN_SPECIFY)) {
	    drop_dead(113);
	    FDCAM_DIAG(54);
#if FDCAM_DEBUG
	    fcp->select_cb = safe_sb_l_cb;
#endif
	    if (cb)
		(*cb)(fcp->select_tag,FDCAM_HARDWARE_ERR);
	    return;
	}
    }


    if (!fcp->up_to_date 
	    || fcp->select_configure_2 != fcp->config_cmd[2]
	    || fcp->select_configure_3 != fcp->config_cmd[3]) {
	fcp->config_cmd[2] = fcp->select_configure_2;
	fcp->config_cmd[3] = fcp->select_configure_3;
	CPRINTF(FDCAM_TRC2,
"(*** select config_cmd[2] = 0x%02x, config_cmd[3] = 0x%02x)",
		fcp->config_cmd[2], fcp->config_cmd[3], 0);
	if (ildc_no_int(fcp,CBN_CONFIGURE,fcp->config_cmd,
		RBN_CONFIGURE)) {
	    drop_dead(113);
	    FDCAM_DIAG(54);
#if FDCAM_DEBUG
	    fcp->select_cb = safe_sb_l_cb;
#endif
	    if (cb)
		(*cb)(fcp->select_tag,FDCAM_HARDWARE_ERR);
	    return;
	}
    }


    if (!fcp->up_to_date
	    || fcp->select_perpen_1 != fcp->perpen_cmd[1]) {
	fcp->perpen_cmd[1] = fcp->select_perpen_1;
	CPRINTF(FDCAM_TRC2,"(*** select perpen_cmd[1] = 0x%02x)",
		fcp->perpen_cmd[1], 0, 0);
	if (ildc_no_int(fcp,CBN_PERPEND_MODE,fcp->perpen_cmd,
		RBN_PERPEND_MODE)) {
	    drop_dead(113);
	    FDCAM_DIAG(54);
#if FDCAM_DEBUG
	    fcp->select_cb = safe_sb_l_cb;
#endif
	    if (cb)
		(*cb)(fcp->select_tag,FDCAM_HARDWARE_ERR);
	    return;
	}
    }


    if (!fcp->up_to_date || fcp->select_dsr != fcp->rc.dsr) {
	fcp->rc.dsr = fcp->select_dsr;
	FDCAM_WRITE(fcp->r, DSR, fcp->rc.dsr);
	FDCAM_SYNC();
	CPRINTF(FDCAM_TRC2,"(*** select_dsr = 0x%02x)",
		fcp->select_dsr, 0, 0);
    }
#   define DOR_SEL_BITS (DOR_SPECIAL|DOR_DRIVE_SEL_MASK)
    if (!fcp->up_to_date || ((fcp->select_dor^fcp->rc.dor)&DOR_SEL_BITS)) {
	long foo = fcp->rc.dor;
	fcp->rc.dor = fcp->rc.dor&~DOR_SEL_BITS	| fcp->select_dor&DOR_SEL_BITS;
	FDCAM_WRITE(fcp->r, DOR, fcp->rc.dor);
	FDCAM_SYNC();
	CPRINTF(FDCAM_TRC2,
	    "(*** select_dor = 0x%02x, dor=0x%02x, was 0x%02x)",
		fcp->select_dor, fcp->rc.dor, foo);
    }
#   undef DOR_SEL_BITS
    if (!fcp->up_to_date || fcp->select_ccr != fcp->rc.ccr) {
	fcp->rc.ccr = fcp->select_ccr;
	FDCAM_WRITE(fcp->r, CCR, fcp->rc.ccr);
	FDCAM_SYNC();
	CPRINTF(FDCAM_TRC2,"(*** select_ccr = 0x%02x)",
		fcp->select_ccr, 0, 0);
    }

    VPRINTF("(MR)",0);
                               /* Do motor-req stuff last since the  */
			       /*  call-back might happen at clock   */
			       /*  level.                            */
    motor_req(fcp, fcp->select_motor_req, fdcam_select_3, fcp);
    VPRINTF("(end_fs2)",0);
}

static void 
fdcam_select(struct fdcam_status_block* fsb, 
	     int need_motor,   	  /* See motor_req() request param.     */
	     int nd,	          /* 0 == Use DMA                       */
	                          /* 1 == Don't use DMA (polled)        */
                                  /* -1 == don't care.                  */
	     sb_l_cb cb, void* cb_tag)
{
#define EIS 0
#define EFIFO 0
#define POLL 0
#define FIFOTHR 7
#define OW 1
    struct fdcam_class* fcp = fsb->fcp;
    char err = 0;
    unsigned char pretrk = fsb->fms->write_precomp_cyl;
    unsigned char precomp = make_precomp(fsb->fms->write_precomp_delay);
    unsigned char drive_num  = fsb->lun_id;
    unsigned char is_perpen  = fsb->fms->is_perpen;
    enum fdcam_xfer_rate drate = fsb->fms->xfer_rate;
    unsigned char wgate = is_perpen;
    unsigned char gap = is_perpen;
    int drate_sel = make_drate(1, drate);
    unsigned char hut = make_hut(fsb->fds->head_unload_time, drate);
    unsigned char hlt = make_hut(fsb->fds->head_load_time
            +fsb->fds->head_settle_time, drate);
    unsigned char srt = make_srt(fsb->fds->head_step_rate, drate);

    err |= drate_sel < 0;

    if (nd < 0)
        nd = GET_ND_FROM_CB2_SPECIFY(fcp->specify_cmd[2]);

#if FDCAM_DEBUG
    if (fcp->select_cb && fcp->select_cb != safe_sb_l_cb)
        IEPRINTF("(DOUBLE FDCAM_SELECT, 0x%08lx)",fcp->select_cb);
#endif
    fcp->select_cb = cb;
    fcp->select_tag = cb_tag;
    fcp->select_motor_req = need_motor;

    /* Determine what chip registers should contain... */
    fcp->select_specify_1 = CB1_SPECIFY(srt,hut);
    fcp->select_specify_2 = CB2_SPECIFY(hlt,nd);
    fcp->select_configure_2 = CB2_CONFIGURE(EIS,EFIFO,POLL,FIFOTHR);
    fcp->select_configure_3 = CB3_CONFIGURE(pretrk);
    fcp->select_perpen_1 = CB1_PERPEND_MODE(OW,
            GET_D_FROM_CB1_PERPEND_MODE(fcp->select_perpen_1)&~(1<<drive_num)
            | (is_perpen ? 1<<drive_num : 0), gap, wgate);
    fcp->select_dsr = DSR_VAL(0,0,precomp,drate_sel);
    
    fcp->select_dor = LUN_TO_DOR_DRIVE_SEL(drive_num);

    fcp->select_ccr = drate_sel;

    /* If chip registers do not contain expected value, set those registers. */

    if (err)
        (*cb)(cb_tag, FDCAM_INVALID_SPEC);
    else
        fdcam_select_2(fcp, FDCAM_NORMAL);
}




/* ************************************************************************* */
/*                                                     int fdcam_cmd_phase() */
/* ************************************************************************* */
/* The following issues the given command to the specified FDI interface.    */
/* It is expected that the 82077AA is ready to receive the specified command.*/
/* An error results if the 82077AA is not ready to recieve a command (that   */
/* is, not at "command phase"), or if it does not accept the specified       */
/* number of bytes as the command, or if there is a timeout waiting for the  */
/* 82077AA to be ready to accept the next byte.  After sending the last      */
/* byte of the command, the 82077AA is not checked to see if it wants yet    */
/* another byte (an error condition) because this would introduce a delay    */
/* into the device driver.                                                   */

int 
fdcam_cmd_phase(struct fdcam_class* fcp, int size, unsigned char* data)
	                               /* Return non-zero if error, zero     */
                                       /*  otherwise.                        */
{
    int byte_no = 0;
    int i;

    CPRINTF(FDCAM_TRC4,"<cmd_phase:s=%d, ",size,0,0);
    for (i = 0 ; i < size ; i++)
        CPRINTF(FDCAM_TRC4, "0x%02x,",data[i],0,0);
    CPRINTF(FDCAM_TRC4,">",size,0,0);

    fcp->rc.msr = FDCAM_READ(fcp->r, MSR);

    for (i = 10 ; MSR_RSLT_PHASE_R_RDY(fcp->rc.msr) && i>0 ; i--) {
        if (i == 1)
            return FDCAM_DIAG(148), 1;

        CPRINTF(0x200,"<abc: MSR=0x%02lx,data=0x%02x>\n", fcp->rc.msr,
                FDCAM_READ(fcp->r, DAT), 0);

        FDCAM_DIAG(119);

        fdcam_nothing();
        fcp->rc.msr = FDCAM_READ(fcp->r, MSR);
    }

                                       /* If the chip is ready to accept the */
                                       /*  command byte, the first byte is   */
                                       /*  written here outside the loop     */
                                       /*  to save an extra read of the MSR. */
    if (byte_no<size  &&  MSR_CMD_PHASE_W_RDY(fcp->rc.msr)) {
	FDCAM_WRITE(fcp->r, DAT, *data++);
	FDCAM_SYNC();
        byte_no++;
    }

    while (byte_no<size) {
        long die = 40000;
	/* Since there shouldn't be any wait, */
	/*  lets just spin.                   */
        while (MSR_CMD_PHASE_W_NRDY(fcp->rc.msr=FDCAM_READ(fcp->r, MSR))
	       && --die)
            fdcam_nothing();
	
#if FDCAM_DEBUG
	if (die < 40000-600)
	    CPRINTF(FDCAM_TRACE,"<SPIN1.%d@%ld>",byte_no,40000-die,0);
#endif
	
        if (MSR_CMD_PHASE_W_RDY(fcp->rc.msr)) {
	    FDCAM_WRITE(fcp->r, DAT, *data++);
	    FDCAM_SYNC();
            byte_no++;
	} else {
            CPRINTF(0x200,"<ER7,msr=0x%02x>\n", (int)fcp->rc.msr, 0, 0);
            if (MSR_RSLT_PHASE_R_RDY(fcp->rc.msr)) {
                CPRINTF(0x200,"<ER7b,dat=0x%02x>\n", 
			FDCAM_READ(fcp->r,DAT),0,0);
                FDCAM_DIAG(46);
                return 1;
	    } else {
                FDCAM_DIAG(12);
                return 1;
	    }
	}
    }

    /* Note, don't check for the error condition of the chip possibly        */
    /*  wanting another byte since certain commands might not have a result  */
    /*  phase and thuse would immediately go into another command phase.     */

    return 0;
}



/* ************************************************************************* */
/*                                                    int fdcam_rslt_phase() */
/* ************************************************************************* */
/* The following spins, waiting for the result phase (timeout about 5 sec).  */
/* The result bytes are then read into the indicated buffer.                 */

/*	Return:	0	No Error.					     */
/*		1-19	Size to big by that much			     */
/*		20	Hardware Error					     */
/*		21	Size too small					     */

int 
fdcam_rslt_phase(struct fdcam_class* fcp, int size, unsigned char* data, 
		 UNS8 msr_val)
{
#if FDCAM_DEBUG
    unsigned char* dp = data;
    int j = size;
    int i;

    CPRINTF(FDCAM_TRC4,"<rslt_phase:s=%d, ",size,0,0);
#endif

    if (MSR_RSLT_PHASE_R_RDY(msr_val) && size>0) {
        *data++ = FDCAM_READ(fcp->r, DAT);
        size--;
    }

    while (size > 0) {
        long die = 40000;
        while (MSR_RSLT_PHASE_R_NRDY(fcp->rc.msr=FDCAM_READ(fcp->r, MSR)) 
	       && --die)
            fdcam_nothing();
	
#if FDCAM_DEBUG
	if (die < 40000-200)
	    CPRINTF(FDCAM_TRACE,"<SPIN2.%d@%ld>",j-size,40000-die,0);
#endif
	
        if (MSR_RSLT_PHASE_R_RDY(fcp->rc.msr)) {
	    *data++ = FDCAM_READ(fcp->r, DAT);
            size--;
	} else if (!MSR_RSLT_PHASE(fcp->rc.msr)) {
#if FDCAM_DEBUG
            CPRINTF(0x200,"<MRP TOO BIG BY %d, msr=0x%08lx,>\n",
                    size, fcp->rc.msr, 0);
#endif
            return size;               /* Size was too big.                  */
	} else {
            FDCAM_DIAG(13);            /* Chip is stuck.                     */
            return 20;
	}
    }

    
#if FDCAM_DEBUG
/* Check for any additional result bytes available.                          */
    fdcam_nothing();
    if (MSR_RSLT_PHASE_R_RDY(fcp->rc.msr=FDCAM_READ(fcp->r, MSR))) {
        int data;
	data = FDCAM_READ(fcp->r, DAT);
        CPRINTF(FDCAM_TRACE,"<ExtraResult=0x%02x>",data&0xFF,0,0);
        FDCAM_DIAG(14);                /* Size was too small                 */
        return 21;
    }

    for (i = 0 ; i < j ; i++)
        CPRINTF(FDCAM_TRC4, "0x%02x,",dp[i], 0, 0);
    CPRINTF(FDCAM_TRC4,">",size,0,0);
#endif

    return 0;
}

/* ************************************************************************* */
/* Issue the sense-interrupt-status command and place results in the         */
/*  pointed-to variables.                                                    */
/*  If there is no pending interrupt, this command will return a 0x80 in     */
/*  *st0p and garbage in *pcn and return zero (success).                     */

/* Return 0 if all is well.      */
static int 
sense_i_s(struct fdcam_class* fcp, unsigned char* st0p, unsigned char* pcn)
{
    int t;
    unsigned char cb[CBN_SENSE_INTR_STAT];
    unsigned char rb[RBN_SENSE_INTR_STAT];
    VPRINTF("(sis)",0);

    cb[0] = CB0_SENSE_INTR_STAT;

    if (fdcam_cmd_phase(fcp, CBN_SENSE_INTR_STAT, cb)) {
        drop_dead(114);
        return 1;
    }

    t = fdcam_rslt_phase(fcp, RBN_SENSE_INTR_STAT, rb, 0);

    if (t == 0  ||  t == 1 && rb[0] == 0x80) {
        *st0p = rb[0];
        *pcn = rb[1];
        return 0;
    } else {
        CPRINTF(0x200,"<ER1,t=%d,rb[0]=0x%02x>\n",t,rb[0],0);
        drop_dead(115);
        return 2;
    }
}

static int 
sense_drive_status(fsb, st3p)          /* Return 0 if all is well.      */
struct fdcam_status_block* fsb;
unsigned char* st3p;
{
    int t;
    unsigned char cb[CBN_SENSE_DRV_STAT];
    unsigned char rb[RBN_SENSE_DRV_STAT];
    struct fdcam_class* fcp = fsb->fcp;

    VPRINTF("(sds)",0);

    cb[0] = CB0_SENSE_DRV_STAT;
    cb[1] = CB1_SENSE_DRV_STAT(0, fsb->lun_id);

    if (fdcam_cmd_phase(fcp, CBN_SENSE_DRV_STAT, cb)) {
        drop_dead(114);
        return 1;
    }

    t = fdcam_rslt_phase(fcp, RBN_SENSE_DRV_STAT, rb, 0);

    if (t == 0) {
        *st3p = rb[0];
        return 0;
    } else {
        CPRINTF(0x200,"<ER1,t=%d,rb[0]=0x%02x>\n",t,rb[0],0);
        drop_dead(115);
        return 2;
    }
}

#if FDCAM_DEBUG
static int 
dumpreg(struct fdcam_class* fcp)           /* Return 0 if all is well.      */
{
    int t;
    unsigned char cb[CBN_DUMPREG];
    unsigned char rb[RBN_DUMPREG];
    VPRINTF("(dumpreg)",0);

    cb[0] = CB0_DUMPREG;

    if (fdcam_cmd_phase(fcp, CBN_DUMPREG, cb)) {
        drop_dead(114);
        return 1;
    }

    t = fdcam_rslt_phase(fcp, RBN_DUMPREG, rb, 0);

    if (t == 0) {
	CPRINTF(FDCAM_TRACE, "(pcn0=%x, pcn1=%x, pcn2=%x,", 
		rb[0]&0xff, rb[1]&0xff, rb[2]&0xff);
	CPRINTF(FDCAM_TRACE, " pcn3=%x, srt=%x, hut=%x",
		rb[3]&0xff, (rb[4]&0xf0)>>4, (rb[4]&0xf));
	CPRINTF(FDCAM_TRACE, " hlt=%x, nd=%x, sc/eot=%x,",
		(rb[5]&0xff)>>1, (rb[5]&0x1), rb[6]&0xff);
	CPRINTF(FDCAM_TRACE, " rb[7]=%x, rb[8]=%x, rb[9]=%x)",
		rb[7], rb[8], rb[9]);
    } else
	TPRINTF("Error in dumpreg", 0);

    return 0;
}
#endif


/* ************************************************************************* */
static void 
clear_fdi_intr(struct fdcam_class* fcp)
{
    int foo;
    VPRINTF("(cfi)",0);
    if (!((fcp->rc.sra=FDCAM_READ(fcp->r, SRA)) & SRA_INT_PENDING)) {
        FDCAM_DIAG(40);
    } else if (MSR_EXEC_PHASE(fcp->rc.msr = FDCAM_READ(fcp->r, MSR))) {
        if (MSR_EXEC_PHASE_R_RDY(fcp->rc.msr)) {
            fcp->rc.dat = FDCAM_READ(fcp->r, DAT);
	} else if (MSR_EXEC_PHASE_W_RDY(fcp->rc.msr)) {
            fcp->rc.dat = 0;
	    FDCAM_WRITE(fcp->r, DAT, fcp->rc.dat);
	    FDCAM_SYNC();
	}
    } else if (MSR_RSLT_PHASE(fcp->rc.msr)) {
        UNS8 buf[RB_LIMIT];
        if (fdcam_rslt_phase(fcp, RB_LIMIT, buf, 0) == 20) {
            drop_dead(121);
            return;
	}
    } else if (MSR_CMD_PHASE_W1_RDY(fcp->rc.msr)) {
        UNS8 dummy;
        if (sense_i_s(fcp, &dummy, &dummy)) {
            FDCAM_DIAG(25);
            return;
	}
    } else {
        FDCAM_DIAG(41);
    }

}

/* The following clears a pending FDI error.  The first one is ignored, the  */
/*  second instance of such generates an error.                              */

static void 
fdi_intr_err(struct fdcam_class* fcp)
{
    int i;
    CPRINTF(0x200,"\n<fdi_intr_err,",0, 0,0);
    CPRINTF(0x200,"sra=0x%02x,srb=0x%02x,msr=0x%02x>\n",
            FDCAM_READ(fcp->r, SRA), FDCAM_READ(fcp->r, SRB),
	    FDCAM_READ(fcp->r, MSR));
    for (i = 10 ; MSR_RSLT_PHASE_R_RDY(FDCAM_READ(fcp->r, MSR)) && i-->0 ;)
        CPRINTF(0x200,",dat=0x%02x\n", FDCAM_READ(fcp->r, DAT),0,0);
    CPRINTF(0x200,",DOR=0x%02x>",FDCAM_READ(fcp->r, DOR),0,0);

    clear_fdi_intr(fcp);

    if (++fcp->intr_err_count > 2) {
        drop_dead(122);
        return;
    }
}

/* ************************************************************************* */
/*                                               The FDCAM interrupt handler */
/* ************************************************************************* */
/* This routine communicates with the rest of the code via the fc_0          */
/*  structure.  It is expected that, when this routine has been entered,     */
/*  that wait_for_intr() has been called and is sleeping on &fc0.iwait.      */

/* void fdcam_intr(struct fdcam_class* fcp) */

void 
fdcam_intr()
{
    if (fc_0.great_state != GS_INIT) {
        if (fc_0.great_state == GS_NO_INIT) {
            shut_it_down(&fc_0, 0, 100);
            return;
	} else {
            drop_dead(123);
            return;
	}
    } else {
	(*fc_0.intr_hdlr)(&fc_0);
    }
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

#define ILDC_TIMO_DELAY (8*hz)

static void 
ildc_timo(struct fdcam_class* fcp)
{
    untimeout(ildc_timo,fcp);
    CPRINTF(0x200,"<ildc_timo, sra=0x%02x,", FDCAM_READ(fcp->r, SRA), 0, 0);
    CPRINTF(0x200,"srb=0x%02x,msr=0x%02x,dor=0x%02x>\n",
            FDCAM_READ(fcp->r, SRB), FDCAM_READ(fcp->r, MSR),
	    FDCAM_READ(fcp->r, DOR));

    if (fcp->great_state != GS_INIT) {
        fcp->ildc_err = 45;
        FDCAM_DIAG(45);
        return;
    } else if (!fcp->ildc_old_hdlr) {
        FDCAM_DIAG(46);
        return;
    }
    else if (fcp->ildc_err == 50)
        fcp->ildc_err = 51;
    else if (fcp->ildc_err == 51)
        FDCAM_DIAG(121);
    else
        FDCAM_DIAG(49);

    if (fcp->ildc_sis == IM_INT || fcp->ildc_sis == IM_INT_SIS) {
        motor_req(fcp,6,0,0);
        fcp->up_to_date = 0;
        fcp->rc.dor = DOR_NORMAL;
	FDCAM_WRITE(fcp->r, DOR, fcp->rc.dor);
        fcp->ildc_sis = IM_RECOVERY;
        FDCAM_WRITE(fcp->r, DSR, (fcp->rc.dsr=DSR_NORMAL) | DSR_SW_RESET);
	FDCAM_SYNC();
    } else {
        fcp->intr_hdlr = fcp->ildc_old_hdlr;
        fcp->ildc_old_hdlr = 0;

        if (fcp->ildc_cb)
            (*fcp->ildc_cb)(fcp->ildc_cb_tag,fcp->ildc_err);
        else
            wakeup(fcp->ildc_cb_tag);
    }

}


static void 
ildc_intr(struct fdcam_class* fcp)
{
    struct intr_result* irp = &fcp->ildc_ir;
    VPRINTF("<ildc_Intr>",0);

    untimeout(ildc_timo, fcp);

    if (!fcp->ildc_old_hdlr) {
        return;  /* Timeout just occurred. */
    } else if (fcp->great_state != GS_INIT) {
        fcp->ildc_err = 44;
        FDCAM_DIAG(44);
    } else {
        fcp->intr_hdlr = fcp->ildc_old_hdlr;
        fcp->ildc_old_hdlr = 0;

        fcp->rc.msr = FDCAM_READ(fcp->r, MSR);

        /* If execution phase, which shouldn't happen... */
        if (MSR_EXEC_PHASE_RDY(fcp->rc.msr)) {
            fcp->unxp_intr |= 0x02;

            if (MSR_EXEC_PHASE_R_RDY(fcp->rc.msr))
                fcp->rc.dat = FDCAM_READ(fcp->r, DAT);
            else
                {
                fcp->rc.dat = 0;
		FDCAM_WRITE(fcp->r, DAT, fcp->rc.dat);
		FDCAM_SYNC();
                }

            drop_dead(174);
            fcp->ildc_err = 35;
	} else {
            if (fcp->ildc_sis == IM_RESET || fcp->ildc_sis == IM_RESET_EIM
                    || fcp->ildc_sis == IM_RECOVERY) {
                int i;
                for (i = 0 ; i < 8 ; i += 2)
                    if (sense_i_s(fcp, irp->rb+i, irp->rb+i+1)) {
                        drop_dead(125);
                        fcp->ildc_err = 36;
                        break;
		    }
                irp->rbc = 8;
	    } else {
                int i;
                if (i = fdcam_rslt_phase(fcp, fcp->ildc_rsize, irp->rb, 0)) {
                    drop_dead(126);
                    fcp->ildc_err = i;
		} else {
                    irp->rbc = fcp->ildc_rsize;

                    if (fcp->ildc_sis == IM_INT_SIS) {
                        if (sense_i_s(fcp, &irp->intr_st0, &irp->intr_pcn)) {
                            drop_dead(127);
                            fcp->ildc_err = 37;
			}
		    }
		}
	    }
	}
    }

    if (fcp->ildc_err == 50)
        fcp->ildc_err = 0;

    if (fcp->ildc_cb)
        (*fcp->ildc_cb)(fcp->ildc_cb_tag,fcp->ildc_err);
    else
        wakeup(fcp->ildc_cb_tag);
}


/*                                                                           */
/* The following may be called from interrupt level or non-interrupt level.  */
/*  It may call (*cb)() before or after returning, and (*cb)() might or      */
/*  might not be called at interrupt level.                                  */
/*                                                                           */
/*  If ``cb'' is zero, then (*cb)() is not called, but rather, wakeup(cb_tag)*/
/*  is called.  ``fcp->ildc_err'' is initially set to 50 by ildc(), and is   */
/*  set to some other value before wakeup() is called, so that makes a nice  */
/*  address to sleep on.  The caller is free to set fcp->ildc_err to 50      */
/*  before calling ildc().                                                   */
/*                                                                           */
/*  The third parameter passed to (*cb)() is a status, which will be non-zero*/
/*  if there was a hardware error and the interrupt results might not be     */
/*  valid.  The results of the interrupt are stored in the fcp->ildc_ir      */
/*  structure, and a status is stored in fcp->ildc_err.                      */


/* The following is like ``ildc'', but doesn't wait for interrupt and doesn't*/
/*  support a call-back.  Zero returned if all went well.                    */
static int 
ildc_no_int(struct fdcam_class* fcp, int csize, unsigned char* cdata, 
	    int rsize)
{
    fcp->ildc_ir.data_leftover = 0;
    if (fcp->intr_hdlr == ildc_intr) {
        drop_dead(128);
        fcp->ildc_err = 67;
        return 1;
    } else {
        if (fdcam_cmd_phase(fcp,csize,cdata)) {
            FDCAM_DIAG(68);
            CPRINTF(0x200,"ER8,csize=%d,cdata=0x%02x,0x%02x,...>\n",
                    csize,cdata[0],cdata[1]);
            fcp->ildc_err = 68;
            return 1;
	}

        if (fdcam_rslt_phase(fcp, rsize, fcp->ildc_ir.rb, 0)) {
            FDCAM_DIAG(69);
            fcp->ildc_err = 69;
            return 1;
	}

        fcp->ildc_ir.rbc = rsize;
        fcp->ildc_err = 0;
        return 0;
    }
}


static void 
ildc(struct fdcam_class* fcp, 
     int csize,                     /* Size of command.                   */
     unsigned char* cdata,          /* The command bytes.                 */
     int rsize,                     /* Size of result.                    */
     enum ildc_mode expect_int,     /* See enum ildc_mode comments.       */
     sb_l_cb cb,                    /* The call-back.  If ``cb'' is zero, */
                                    /*  then the results are stored in    */
                                    /*  the fcp->ir structure and         */
                                    /*  wakeup(cb_tag) is called.         */
     void* cb_tag)                  /* The call-back tag.                 */
{
    int s = splbio();
    fcp->ildc_ir.data_leftover = 0;
    fcp->ildc_err = 50;
    if (fcp->intr_hdlr == ildc_intr) {
        splx (s);
        drop_dead(129);
        fcp->ildc_err = 40;
    } else if (expect_int == IM_NO_INT) {
        splx (s);
        if (fdcam_cmd_phase(fcp,csize,cdata)) {
            FDCAM_DIAG(34);
            CPRINTF(0x200,"ER8,csize=%d,cdata=0x%02x,0x%02x,...>\n",
                    csize,cdata[0],cdata[1]);
            fcp->ildc_err = 42;
	}

        if (fdcam_rslt_phase(fcp, rsize, fcp->ildc_ir.rb, 0)) {
            FDCAM_DIAG(35);
            fcp->ildc_err = 43;
	}

        fcp->ildc_ir.rbc = rsize;
    } else if (fcp->ildc_old_hdlr) {
        FDCAM_DIAG(79);
        fcp->ildc_err = 79;
    } else {
        fcp->ildc_rsize = rsize;
        fcp->ildc_sis = expect_int;
        fcp->ildc_old_hdlr = fcp->intr_hdlr;
        fcp->intr_hdlr = ildc_intr;
        fcp->ildc_cb_tag = cb_tag;
        fcp->ildc_cb = cb;

        if (expect_int == IM_RESET || expect_int == IM_RESET_EIM) {
            fcp->up_to_date = 0;
	    /* force chip into reset state */
	    fcp->rc.dor = DOR_NORMAL & ~DOR_RESET_i;
            FDCAM_WRITE(fcp->r, DOR, fcp->rc.dor);
            FDCAM_SYNC();
            microdelay(2);

            if (expect_int == IM_RESET_EIM) {
		/* enable our interrupt */
		eisa_enable_option(fcp->ctlr);
	    }

	    /* take chip out of reset state */
	    fcp->rc.dor = DOR_NORMAL;
	    FDCAM_WRITE(fcp->r, DOR, fcp->rc.dor);
	    FDCAM_SYNC();
            timeout(ildc_timo, fcp, ILDC_TIMO_DELAY);
            splx (s);
            return;                    /* (*cb)() called by interrupt hdlr.  */
	} else if (fdcam_cmd_phase(fcp,csize,cdata)) {
            fcp->ildc_cb = 0;
            FDCAM_DIAG(33);
            fcp->ildc_err = 41;
	} else {
            timeout(ildc_timo, fcp, ILDC_TIMO_DELAY);
            splx (s);
            return;                    /* (*cb)() called by interrupt hdlr.  */
	}
    }

    if (fcp->ildc_err == 50)
        fcp->ildc_err = 0;

    if (cb)
        (*cb)(cb_tag, fcp->ildc_err);
    else
        wakeup(fcp->ildc_cb_tag);

}


/* ************************************************************************* */
/* ************************************************************************* */

#if CODE_POLL

#define ILXC_TIMO_DELAY (8*hz)

static void 
ilxc_timo(struct fdcam_class* fcp)
{
    untimeout(ilxc_timo,fcp);
    CPRINTF(0x200,"<ilxc_timo, sra=0x%02x,", FDCAM_READ(fcp->r, SRA), 0, 0);
    CPRINTF(0x200,"srb=0x%02x,msr=0x%02x,dor=0x%02x>\n",
            FDCAM_READ(fcp->r, SRB), FDCAM_READ(fcp->r, MSR),
	    FDCAM_READ(fcp->r, DOR));

    if (fcp->great_state != GS_INIT) {
        fcp->ildc_err = 45;
        FDCAM_DIAG(208);
        return;
    } else if (!fcp->ildc_old_hdlr) {
        FDCAM_DIAG(207);
        return;
    } else if (fcp->ildc_err == 50)
        fcp->ildc_err = 51;
    else if (fcp->ildc_err == 51)
        FDCAM_DIAG(206);
    else
        FDCAM_DIAG(205);

    TPRINTF("(UH OH)",0);
    (*fcp->ildc_cb)(fcp->ildc_cb_tag,fcp->ildc_err);
}

/* The following to be called initially and when ildc_count_down reaches     */
/*  zero.                                                                    */
static void 
ilxc_setup(struct fdcam_class* fcp)
{
    if (fcp->ildc_u_proc) {
	fcp->ildc_working_ptr = (unsigned char*)PHYS_TO_KSEG(
		pmap_extract(fcp->ildc_u_proc->task->map->vm_pmap,
		fcp->ildc_u_buf));
        fcp->ildc_count_down = NBPG-((unsigned long)fcp->ildc_working_ptr
                & PGOFSET);
        if (fcp->ildc_count_down > fcp->ildc_ub_size)
            fcp->ildc_count_down = fcp->ildc_ub_size;
    } else {
        fcp->ildc_count_down = fcp->ildc_ub_size;
        fcp->ildc_working_ptr = fcp->ildc_u_buf;
    }
    fcp->ildc_orig_cd = fcp->ildc_count_down;
}

static void 
ilxc_intr(struct fdcam_class* fcp)
struct fdcam_class* fcp;
{
    struct intr_result* irp = &fcp->ildc_ir;
    VPRINTF("<XI>",0);

    untimeout(ilxc_timo, fcp);

    if (!fcp->ildc_old_hdlr) {
        return;  /* Timeout just occurred. */
    } else if (fcp->great_state != GS_INIT) {
        fcp->ildc_err = 44;
        FDCAM_DIAG(204);
    } else {
        int quit = 0;
        int extra_data = 0;
        long die;

	/*
	 * Do not allow clock interrupts.  The timing during the while
	 * loop is very critical.  If a clock interrupt was serviced, the
	 * floppy would get an underrun error
	 */
	int s = splclock();
        while (fcp->ildc_count_down) {
	    if (fcp->ildc_dir == OP_READ) {
		die = 4000000;

		fcp->rc.msr = FDCAM_READ(fcp->r, MSR);
		while (!MSR_EXEC_PHASE_R_RDY(fcp->rc.msr) && --die) {
		    if (MSR_RSLT_PHASE_R_RDY(fcp->rc.msr)) {
			CPRINTF(FDCAM_TRACE,"<QUITx1.%ld,0x%02x>",
				4000000-die, fcp->rc.msr, 0);
			quit = 1;
			break;
		    }
		    fdcam_nothing();
		    fcp->rc.msr = FDCAM_READ(fcp->r, MSR);
		}
		
#if FDCAM_DEBUG
		if (quit)
		    break;
		if (die < 4000000-50)
		    CPRINTF(FDCAM_TRACE,"<SPINxR.%ld>", 4000000-die,0,0);
#endif
		if (MSR_EXEC_PHASE_R_RDY(fcp->rc.msr)) {
		    fcp->ildc_count_down--;
		    *fcp->ildc_working_ptr++ = FDCAM_READ(fcp->r, DAT);
		} else {
		    CPRINTF(FDCAM_TRACE,"<QUITx2,0x%02x>",fcp->rc.msr,0,0);
		    quit = 1;
		    break;
		}
	    } else {
                while (fcp->ildc_count_down) {
                    die = 4000000;
		    
		    fcp->rc.msr = FDCAM_READ(fcp->r, MSR);
                    while (!MSR_EXEC_PHASE_W_RDY(fcp->rc.msr) && --die) {
                        if (MSR_RSLT_PHASE_R_RDY(fcp->rc.msr)) {
                            CPRINTF(FDCAM_TRACE,"<QUITx3.%ld>",4000000-die,
                                    0,0);
                            quit = 1;
                            break;
			}
                        fdcam_nothing();
			fcp->rc.msr = FDCAM_READ(fcp->r, MSR);
		    }
		    
#if FDCAM_DEBUG
		    if (die < 4000000-50000)
			CPRINTF(FDCAM_TRACE,"<SPINxW.%ld>", 4000000-die,0,0);
#endif

		    fcp->rc.msr = FDCAM_READ(fcp->r, MSR);
                    if (MSR_EXEC_PHASE_W_RDY(fcp->rc.msr)) {
                        fcp->ildc_count_down--;
			FDCAM_WRITE(fcp->r, DAT, *fcp->ildc_working_ptr++);
			FDCAM_SYNC();
		    } else {
                        CPRINTF(FDCAM_TRACE,"<QUITx4,0x%02x>",fcp->rc.msr,0,0);
                        quit = 1;
                        break;
		    }	
		}
	    }
	    
            fcp->ildc_u_buf += fcp->ildc_orig_cd-fcp->ildc_count_down;
            fcp->ildc_ub_size -= fcp->ildc_orig_cd-fcp->ildc_count_down;
	    
            if (quit)
                break;
	    
            ilxc_setup(fcp);
	}

	splx(s);

        fcp->intr_hdlr = fcp->ildc_old_hdlr;
        fcp->ildc_old_hdlr = 0;
        
        irp->data_leftover = fcp->ildc_ub_size;
        TPRINTF(quit?"(XI-QUIT,LO=%ld)":"(XI,LO=%ld)",irp->data_leftover);

        die = 40000;
        extra_data = 0;
        for (fcp->rc.msr=FDCAM_READ(fcp->r, MSR) ;
                extra_data < 16 && !MSR_RSLT_PHASE_R_RDY(fcp->rc.msr) ;
                fcp->rc.msr=FDCAM_READ(fcp->r, MSR)) {
            if (MSR_EXEC_PHASE_RDY(fcp->rc.msr)) {
                if (MSR_EXEC_PHASE_R_RDY(fcp->rc.msr))
                    fcp->rc.dat = FDCAM_READ(fcp->r, DAT);
                else {
                    fcp->rc.dat = 0;
                    FDCAM_WRITE(fcp->r, DAT, fcp->rc.dat);
		    FDCAM_SYNC();
		}
                die=40000;
	    } else if (--die <= 0)
                break;
	}

        if (extra_data > 0)
            TPRINTF("(TMNDD,%d)", extra_data);

        if (MSR_EXEC_PHASE_RDY(fcp->rc.msr)) {
            drop_dead(210);
            fcp->unxp_intr |= 0x02;
            fcp->ildc_err = 35;
	} else {
            int i;
            if (i = fdcam_rslt_phase(fcp, fcp->ildc_rsize, irp->rb, 0)) {
                drop_dead(211);
                fcp->ildc_err = i;
	    } else {
                irp->rbc = fcp->ildc_rsize;
	    }
	}
    }

    if (fcp->ildc_err == 50)
        fcp->ildc_err = 0;

    (*fcp->ildc_cb)(fcp->ildc_cb_tag,fcp->ildc_err);
}

/* The following is like ildc, but also does a polled data transfer while    */
/*  waiting for the final interrupt.                                         */

static void 
ilxc(struct fdcam_class* fcp, 
     int csize,                     /* Size of command.                   */
     unsigned char* cdata,          /* The command bytes.                 */
     int rsize,                     /* Size of result.                    */
     sb_l_cb cb,                    /* The call-back.  If ``cb'' is zero, */
                                    /*  then the results are stored in    */
                                    /*  the fcp->ir structure and         */
                                    /*  wakeup(cb_tag) is called.         */
     void* cb_tag,                  /* The call-back tag.                 */
     unsigned char* u_buf,
     struct proc* u_proc,
     long ub_size,
     enum enum_op dir)               /* Transfer direction...              */
                                     /*  OP_WRITE == read from memory      */
                                     /*   and write to controller chip.    */
                                     /*  OP_READ == read from controller   */
                                     /*   chip and write to memory.        */
{
    int s = splbio();

    TPRINTF(dir==OP_READ?"<ILXC-R:size=%ld>":"<ILXC-W:size=%ld>",ub_size);

    fcp->ildc_err = 50;
    if (fcp->intr_hdlr == ilxc_intr || !cb) {
        splx (s);
        drop_dead(212);
        fcp->ildc_err = 40;
    } else if (fcp->ildc_old_hdlr) {
        splx (s);
        FDCAM_DIAG(203);
        fcp->ildc_err = 79;
    } else {
        fcp->ildc_rsize = rsize;
        fcp->ildc_old_hdlr = fcp->intr_hdlr;
        fcp->intr_hdlr = ilxc_intr;
        fcp->ildc_cb_tag = cb_tag;
        fcp->ildc_cb = cb;
        fcp->ildc_u_buf = u_buf;
        fcp->ildc_u_proc = u_proc;
        fcp->ildc_ub_size = ub_size;
        fcp->ildc_dir = dir;
        ilxc_setup(fcp);

        TPRINTF("<I:cd=%d>",fcp->ildc_count_down);

        if (fdcam_cmd_phase(fcp,csize,cdata)) {
            fcp->ildc_cb = 0;
            FDCAM_DIAG(202);
            fcp->ildc_err = 41;
            splx (s);
	} else {
            timeout(ilxc_timo, fcp, ILXC_TIMO_DELAY);
            splx (s);
            return;                    /* (*cb)() called by interrupt hdlr.  */
	}
    }

    if (fcp->ildc_err == 50)
        fcp->ildc_err = 0;

    if (fcp->ildc_err)
        TPRINTF("<IXE:%d>",fcp->ildc_err);

    (*cb)(cb_tag, fcp->ildc_err);
}

#endif


/* ************************************************************************* */
/* ************************************************************************* */

/* Return:  1=bad error, 0=ok         */ 
static int 
tl_do_command(struct fdcam_class* fcp, 
	      int csize,                     /* Size of command.             */
	      unsigned char* cdata,          /* The command bytes.           */
	      int rsize,                     /* Size of result.              */
	      unsigned char* rdata,          /* The result bytes.            */
	      enum ildc_mode expect_int,     /* See comments for enum 	     */
                                             /* ildc_mode for description.   */
	unsigned char *sis_rb)         /* Place to store result bytes of the */
	                               /*  sense-interrupt-status command.   */
                                       /*  These bytes will be unmodified if */
                                       /*  expect_int != 2 or if there was   */
                                       /*  an error (non-zero return).       */
{
    /* struct tldc_dat dat; */

#if FDCAM_DEBUG
    if (!sis_rb && expect_int == 2)
        return FDCAM_DIAG(36), 1;
#endif

    VPRINTF("<tl_do_command>",0);

    fcp->ildc_err = 50;

    if (expect_int >= IM_IWS_INT && expect_int <= IM_IWS_RESET_EIM) {
        long die = 2000000;            /* Timeout at least 2 seconds.        */

        expect_int -= IM_IWS_INT-IM_INT;

        ildc(fcp, csize, cdata, rsize, expect_int, 0, &fcp->ildc_err);

        while (fcp->ildc_err == 50 && die-- > 0) {
            microdelay(1);
            fdcam_nothing();
	}

	if (fcp->ildc_err == 50) {
	    dprintf("forcing interrupt in tl_do_command\n");
	    fdcam_intr();
	}

    } else {
        ildc(fcp, csize, cdata, rsize, expect_int, 0, &fcp->ildc_err);

        while (fcp->ildc_err == 50)
	    sleep((caddr_t)(&fcp->ildc_err), PRIBIO + 1);
    }

    if (!fcp->ildc_err) {
        if (sis_rb) {
            sis_rb[0] = fcp->ildc_ir.intr_st0;
            sis_rb[1] = fcp->ildc_ir.intr_pcn;
	}
        if (rdata) {
            int a;
            for (a = 0 ; a < fcp->ildc_ir.rbc ; a++)
                rdata[a] = fcp->ildc_ir.rb[a];
	}
    }

    if (fcp->ildc_err == 50)
        fcp->ildc_err = 51;

    return fcp->ildc_err;             /* Note: 51 returned if timeout.      */
}




/* Return 1 if bad error, 0 if all OK.                                       */
static int 
flush_pending_intr(struct fdcam_class* fcp)
{
    unsigned char d1, d2;
    int i = 10;
    while((FDCAM_READ(fcp->r, SRA) & SRA_INT_PENDING) && i-- > 0)
        sense_i_s(fcp, &d1, &d2);

    if (i <= 0)
        return FDCAM_DIAG(38), 1;
    else
        return 0;
}

/* ************************************************************************* */
/* ************************************************************************* */


/* ************************************************************************* */
/*                                                                ll_recal() */
/* ************************************************************************* */
/* Seek to specified cylinder.                                               */
/* Return values one of (via callback):                                      */
/*   FDCAM_HARDWARE_ERR                Sufficient step pulses were issued    */
/*                                     to device but track 0 was not reached.*/
/*  Other return values include those that might be generated by any of the  */
/*  following functions:                                                     */
/*     ildc()                                                                */


static void 
ll_recal_b(struct fdcam_status_block* fsb, long stat)
{
    struct fdcam_class* fcp = fsb->fcp;
    VPRINTF("(llrb,%ld)",stat);
    if (stat)
        (*fcp->ll_recal_cb)(fcp->ll_recal_tag,stat);
    else if ((fcp->ildc_ir.intr_st0&(ST0_IC_MASK|ST0_SE))
            != (ST0_IC_NORMAL|ST0_SE)) {
        FDCAM_DIAG(172);
        fsb->head_position = -1;
        (*fcp->ll_recal_cb)(fcp->ll_recal_tag,FDCAM_HARDWARE_ERR);
    } else {
        fsb->head_position = 0;
        (*fcp->ll_recal_cb)(fcp->ll_recal_tag,FDCAM_NORMAL);
    }
}


static void ll_recal_a0();

static void 
ll_recal_aa(struct fdcam_status_block* fsb, long stat)
{
    struct fdcam_class* fcp = fsb->fcp;
    VPRINTF("(llraa,%ld)",stat);
    if (stat)
        ll_recal_a0(fsb,stat);
    else {
        unsigned char cmd[CBN_RECALIBRATE];
#if RBN_RECALIBRATE!=0 || CBN_RECALIBRATE!=2
**** #      error "I thought RBN_RECALIBRATE was zero!"
#endif

        cmd[0] = CB0_RECALIBRATE;
        cmd[1] = CB1_RECALIBRATE(fsb->lun_id);
        VPRINTF("(lllr,lun_id=%d)",fsb->lun_id);

        /* Let's grab this info before it goes away... */
        fsb->media_changed |= FDCAM_READ(fcp->r, DIR)&DIR_DSK_CHG;

        ildc (fcp, CBN_RECALIBRATE, cmd, RBN_RECALIBRATE, IM_INT_SIS,
                ll_recal_a0, fsb);
    }
}

/* Rcalibrate the indicated drive, then position the head to physical        */
/* cylinder ``cyl_0_pos''.                                                   */
static void 
ll_recal_a0(struct fdcam_status_block* fsb, long stat)
{
#define MAX_RECAL_RETRIES 3
    struct fdcam_class* fcp = fsb->fcp;
    VPRINTF("(llra,%ld)",stat);
    if (stat) {
        FDCAM_DIAG(74);
        fsb->head_position = -1;
        (*fcp->ll_recal_cb)(fcp->ll_recal_tag,stat);
    } else if (fcp->ll_recal_n_attempt<MAX_RECAL_RETRIES
            && (fcp->ildc_ir.intr_st0&(ST0_IC_MASK|ST0_EC))
             == (ST0_IC_ABNORMAL|ST0_EC)) { /* Jar head loose with forward step.  */
        unsigned char cb[CBN_RELATIVE_SEEK];
#if RBN_RELATIVE_SEEK!=0
**** #      error "I thought RBN_RECALIBRATE was zero!"
#endif
    
        cb[0] = CB0_RELATIVE_SEEK(1);
        cb[1] = CB1_RELATIVE_SEEK(0,fsb->lun_id);
        cb[2] = CB2_RELATIVE_SEEK(1);

        fcp->ll_recal_n_attempt++;
        ildc (fcp,CBN_RELATIVE_SEEK,cb,RBN_RELATIVE_SEEK,IM_INT_SIS,
                ll_recal_aa,fsb);
    } else if ((fcp->ildc_ir.intr_st0&(ST0_IC_MASK|ST0_SE))
             != (ST0_IC_NORMAL|ST0_SE)) {
        FDCAM_DIAG(73);
        fsb->head_position = -1;
        (*fcp->ll_recal_cb)(fcp->ll_recal_tag,FDCAM_HARDWARE_ERR);
    } else {
        if (! fsb->fms || fsb->fms->cyl_0_pos == 0) {
            fsb->head_position = 0;
            (*fcp->ll_recal_cb)(fcp->ll_recal_tag,FDCAM_NORMAL);
	} else {
            unsigned char cb[CBN_RELATIVE_SEEK];
#if RBN_RELATIVE_SEEK!=0
**** #          error "I thought RBN_RECALIBRATE was zero!"
#endif
        
            cb[0] = CB0_RELATIVE_SEEK(1);
            cb[1] = CB1_RELATIVE_SEEK(0,fsb->lun_id);
            cb[2] = CB2_RELATIVE_SEEK(fsb->fms->cyl_0_pos);

            ildc (fcp,CBN_RELATIVE_SEEK,cb,RBN_RELATIVE_SEEK,IM_INT_SIS,
                    ll_recal_b,fsb);
	}
    }
}

static void 
ll_recal(struct fdcam_status_block* fsb, sb_l_cb cb, void* cb_tag)
{
    VPRINTF("(llr)",0);
    fsb->fcp->ll_recal_tag = cb_tag;
    fsb->fcp->ll_recal_cb = cb;
    fsb->fcp->ll_recal_n_attempt = 0;
    ll_recal_aa(fsb,0);
}


/* ************************************************************************* */
/*                                                                ll_seek()  */
/* ************************************************************************* */
/* Seek to specified cylinder.                                               */
/* Return values one of (via callback):                                      */
/*   FDCAM_PROGRAMMING_ERR             Specified cylinder out of range.      */
/*                                     Fms not specified.                    */
/*   FDCAM_HARDWARE_ERR                Unexpected status from controller chip*/
/*   FDCAM_INT_PROG_ERR                If I knew why this would occurr, I    */
/*                                      would have fixed it.                 */
/*  Other return values include those that might be generated by any of the  */
/*  following functions:                                                     */
/*     ll_recal()                                                            */
/*     ildc()                                                                */


static void ll_seek_a();

static void 
ll_seek_b(struct fdcam_status_block* fsb, long stat)
{
    struct fdcam_class* fcp = fsb->fcp;
    VPRINTF("(llsb,%ld)",stat);
    fsb->head_position = fcp->seek_cyl;
    if ((fcp->ildc_ir.intr_st0&(ST0_IC_MASK|ST0_SE)) != (ST0_IC_NORMAL|ST0_SE)) {
	/*
	 * NOTE:  In this case we return as if successful, but we do
	 * 	  not update the head_position.  The chip seems to be
	 *	  in an odd state.  We return to retry the read/write operation
	 *	  which will probably fail.  The error recovery in ll_rwv_d()
	 *	  will cause a recalibrate to occur.
	 */
        sb_l_cb x = fcp->seek_cb;
        FDCAM_DIAG(72);
        fsb->head_position = fcp->ildc_ir.intr_pcn;

#if FDCAM_DEBUG
          fcp->seek_cb = safe_sb_l_cb;
#endif
        (*x)(fcp->seek_tag,FDCAM_NORMAL);
    } else if (fcp->seek_wh) {
        fcp->seek_cyl += fcp->seek_wh==1 ? -1 : 1;
        fcp->seek_wh = 0;
        ll_seek_a(fsb, 0);
    } else {
        sb_l_cb x = fcp->seek_cb;
#if FDCAM_DEBUG
          fcp->seek_cb = safe_sb_l_cb;
#endif
        (*x)(fcp->seek_tag,FDCAM_NORMAL);
    }
}

static void 
ll_seek_a(struct fdcam_status_block* fsb, long stat) 
{
    unsigned char cb[CBN_RELATIVE_SEEK];

#if RBN_RELATIVE_SEEK!=0
**** #error "I thought RBN_RECALIBRATE was zero!"
#endif

    unsigned char sisrb[RBN_SENSE_INTR_STAT];
    
    VPRINTF("(llsa,%ld)",stat);

    if (stat || fsb->head_position < 0) {
        sb_l_cb x = fsb->fcp->seek_cb;
#if FDCAM_DEBUG
          fsb->fcp->seek_cb = safe_sb_l_cb;
#endif
        if (stat)
            (*x)(fsb->fcp->seek_tag,  /* recal error                        */
                    stat);
        else
            (*x)(fsb->fcp->seek_tag, 
                    FDCAM_INT_PROG_ERR);
    } else if (fsb->fcp->seek_cyl == fsb->head_position) {
        sb_l_cb x = fsb->fcp->seek_cb;
#if FDCAM_DEBUG
          fsb->fcp->seek_cb = safe_sb_l_cb;
#endif
        (*x)(fsb->fcp->seek_tag,FDCAM_NORMAL);
    } else {
        int sps;
        if (fsb->fms && fsb->fms != &fms_default)
            sps = fsb->fms->steps_per_cylinder ;
        else
            sps = 1;

        if (fsb->fcp->seek_cyl > fsb->head_position) {
            cb[0] = CB0_RELATIVE_SEEK(1);
            cb[1] = CB1_RELATIVE_SEEK(0,fsb->lun_id);
            cb[2] = CB2_RELATIVE_SEEK((fsb->fcp->seek_cyl
                    - fsb->head_position) * sps);
	} else {
            cb[0] = CB0_RELATIVE_SEEK(0);
            cb[1] = CB1_RELATIVE_SEEK(0,fsb->lun_id);
            cb[2] = CB2_RELATIVE_SEEK((fsb->head_position
                    - fsb->fcp->seek_cyl) * sps);
	}

/* Note:  If fsb->fms->steps_per_cylinder != 1, then the 82077AA's PCN   */
/*   register will not properly reflect the current cylinder number.     */

        /* Let's grab this info before it goes away... */
        fsb->media_changed |= FDCAM_READ(fsb->fcp->r, DIR)&DIR_DSK_CHG;

        ildc(fsb->fcp,CBN_RELATIVE_SEEK,cb,RBN_RELATIVE_SEEK,IM_INT_SIS,
                ll_seek_b,fsb);
    }
}

static void 
ll_seek_sb(struct fdcam_status_block* fsb, long stat)
{
    struct fdcam_class* fcp = fsb->fcp;
    sb_l_cb x = fcp->seek_cb;
    VPRINTF("(llssb,%ld)",stat);
    fsb->head_position = -1;

#if FDCAM_DEBUG
    fcp->seek_cb = safe_sb_l_cb;
#endif

    if (stat) {
        (*x)(fcp->seek_tag,stat);
    } else if ((fcp->ildc_ir.intr_st0&(ST0_IC_MASK|ST0_SE))
            != (ST0_IC_NORMAL|ST0_SE)) {
        FDCAM_DIAG(173);
        (*x)(fcp->seek_tag,FDCAM_HARDWARE_ERR);
    } else {
        (*x)(fcp->seek_tag,FDCAM_NORMAL);
    }
}


static void 
ll_seek_s(struct fdcam_status_block* fsb, long stat)
{
    unsigned char cb[CBN_RELATIVE_SEEK], status;
    struct fdcam_class* fcp = fsb->fcp;

#if RBN_RELATIVE_SEEK!=0
**** #error "I thought RBN_RELATIVE_SEEK was zero!"
#endif

    VPRINTF("(llss,%ld)",stat);

    if (stat) {
        sb_l_cb x = fcp->seek_cb;
#if FDCAM_DEBUG
          fcp->seek_cb = safe_sb_l_cb;
#endif
        (*x)(fcp->seek_tag,stat);
    } else {
	if (sense_drive_status(fsb, &status)) {
	    sb_l_cb x = fcp->seek_cb;
#if FDCAM_DEBUG
	    fcp->seek_cb = safe_sb_l_cb;
#endif
	    (*x)(fcp->seek_tag, FDCAM_HARDWARE_ERR);
	} else {

	    cb[0] = CB0_RELATIVE_SEEK((status & ST3_T0) ? 1 : 0);
	    cb[1] = CB1_RELATIVE_SEEK(0,fsb->lun_id);
	    cb[2] = CB2_RELATIVE_SEEK(1);  /* one step */
	    
	    /* Let's grab this info before it goes away... */
	    fsb->media_changed |= FDCAM_READ(fcp->r, DIR)&DIR_DSK_CHG;
	    VPRINTF("(llss, med_chg=%x)", fsb->media_changed);
	    
	    ildc(fcp,CBN_RELATIVE_SEEK,cb,RBN_RELATIVE_SEEK,IM_INT_SIS,
		 ll_seek_sb,fsb);
	}
    }
}


static void 
ll_seek(struct fdcam_status_block* fsb,
	long cylinder,                 /* >= 0  destination cylinder.        */
                                       /* -1    seek to current cylinder.    */
	int whence,                    /* 0 = from current position.         */
                                       /* 1 = inner-to-outer direction.      */
	                               /* 2 = outer-to-inner direction.      */
                                       /* 3 = recalibrate first, then inner- */
                                       /*      to-outer direction.           */
                                       /* 4 = recalibrate first, then outer- */
                                       /*      to-inner direction.           */
                                       /* -1= Just issue a step pulse to the */
                                       /*      drive.  ``cylinder'' is       */
                                       /*      ignored.  Multiple pulses     */
                                       /*      might be issued.              */
	sb_l_cb cb,
	void* tag)
{
    struct fdcam_class* fcp = fsb->fcp;
    VPRINTF("(lls,cyl=%ld)",cylinder);

    if (whence == -1) {
        if (fsb->head_position >= 0 && fsb->media_present
                && fsb->fms  && fsb->fms != &fms_default) {
            whence = 0;
            cylinder = fsb->head_position>0
                    ? fsb->head_position-1 : 1;
	} else {
#if FDCAM_DEBUG
            if (fsb->fcp->seek_cb != safe_sb_l_cb) {
                IEPRINTF("(DOUBLE LLS1, 0x%08lx)",fsb->fcp->seek_cb);
                drop_dead(121);
	    }
#endif
            fsb->fcp->seek_cb = cb;
            fsb->fcp->seek_tag = tag;
            fsb->fcp->seek_cyl = cylinder;
            fsb->fcp->seek_wh = whence;

            ll_seek_s(fsb, 0);
            return;
	}
    }

    if (cylinder < 0)
        cylinder = fsb->head_position;

    if (fsb->fms == &fms_default)
        (*cb)(tag,FDCAM_PROGRAMMING_ERR);

    if (whence != 0) {
        if (whence > 2) {
            whence -= 2;
            fsb->head_position = -1;
	}

        if(whence == 1 && cylinder+1 >= fsb->fms->cylinders_p_disk)
            whence = 0;
        else if (whence == 2 && cylinder-1 < 0)
            whence = 0;

        if (whence == 1)
            cylinder++;
        else if (whence == 2)
            cylinder--;
    }

#if FDCAM_DEBUG
    if (fsb->fcp->seek_cb != safe_sb_l_cb) {
        IEPRINTF("(DOUBLE LLS2, 0x%08lx)",fsb->fcp->seek_cb);
        drop_dead(122);
    }
#endif

    fsb->fcp->seek_cb = cb;
    fsb->fcp->seek_tag = tag;
    fsb->fcp->seek_cyl = cylinder;
    fsb->fcp->seek_wh = whence;

    if (cylinder < 0 || cylinder >= fsb->fms->cylinders_p_disk)
        (*cb)(tag,FDCAM_PROGRAMMING_ERR);
    else if (fsb->head_position < 0)
        ll_recal(fsb, ll_seek_a, fsb);
    else
        ll_seek_a(fsb, 0);
}


/* ************************************************************************* */
/*                                                                  ll_rwv() */
/* ************************************************************************* */
/* This is the lowest-level routine that is specific to reading, writing, or */
/*  verifying the media.  The main entry point is ll_rwv().  The possible    */
/*  return values are (via callback):                                        */
/*     FDCAM_NORMAL         All completed OK                                 */
/*     FDCAM_HARDWARE_ERR   Too many retries while waiting for DMA to run.   */
/*     FDCAM_READ_ERR       Unable to read media during read, or missing     */
/*                           address mark during read or write.              */
/*     FDCAM_INVALID_SPEC   One of the following in error:                   */
/*                               fsb->fms->xfer_rate                         */
/*                               fsb->fms->gap_3_write                       */
/*                               fsb->fms->gap_3_read                        */
/*     FDCAM_WRITE_PROTECT  Media write protected during write.              */
/*  Other return values include those that might be generated by any of the  */
/*  following functions:                                                     */
/*     ll_seek()                                                             */
/*     ll_recal()                                                            */
/*     ll_select()                                                           */
/*     ildc()                                                                */

#if 7!=RBN_RW_DATA
**** #  error "I thought read/write/verify results were 7 !"
#endif
#if 9!=CBN_RW_DATA
**** #  error "I thought read/write/verify commands were 9 !"
#endif


static void ll_rwv_c(struct fdcam_status_block* fsb, long stat);
static void ll_rwv_d(struct fdcam_status_block* fsb, long stat);

#if CODE_DMA

static void 
ll_rw_timo(struct fdcam_status_block* fsb)
{
    struct fdcam_class* fcp = fsb->fcp;
    int		flags;

    untimeout(ll_rw_timo, fsb);

    flags = (fcp->rw_op == OP_READ) ? DMA_IN : DMA_OUT;
    
    if (dma_map_load(fcp->rw_count * SECTOR_SIZE, (vm_offset_t)fcp->rw_buf, 
		     fcp->rw_proc, fcp->ctlr, &fcp->sglp, 0, flags) == 0) {
	CPRINTF(FDCAM_TRACE, "dma_map_load returned 0\n", 0, 0, 0);
	drop_dead(999);
    }

    ildc (fcp,CBN_RW_DATA,fcp->rw_cmd,RBN_RW_DATA,IM_INT,ll_rwv_d,fsb);
}

#endif /* end of CODE_DMA */

static void 
ll_rwv_d(struct fdcam_status_block* fsb, long stat)
{
    struct fdcam_class* fcp = fsb->fcp;

#if CODE_DMA
    if (fcp->rw_use_dma) {
	dma_map_unload(0, fcp->sglp);
    }
#endif

    VPRINTF("(llrvwd,%d)",stat);

    if (stat) {
        motor_req(fcp,9,0,0);
        (*fcp->rw_cb)(fcp->rw_tag,stat);
        return;
    }


    if ((fcp->ildc_ir.rb[0]&ST0_IC_MASK) == ST0_IC_ABNORMAL
            && fcp->ildc_ir.rb[1]&(ST1_DE|ST1_ND|ST1_MA)) {

        fcp->rw_retry++;
        fsb->rwv_pause = 1;
        CPRINTF(FDCAM_TRACE,"\n\007--- RETRY #%d, st1=0x%02x, st2=0x%02x, ",
                fcp->rw_retry,0xFF&fcp->ildc_ir.rb[1],0xFF&fcp->ildc_ir.rb[2]);
        CPRINTF(FDCAM_TRACE, "CHS=%d/%d/%d ---\n", fcp->rw_cylinder,
                fcp->rw_surface, fcp->rw_sector);
        CPRINTF(FDCAM_TRACE,"(ccr@0x%02x,dor@0x%02x,dsr@0x%02x)\n",
                fcp->rc.ccr,fcp->rc.dor,fcp->rc.dsr);
#if FDCAM_DEBUG
	dumpreg(fcp);
#endif
	if (fcp->rw_retry > fcp->rw_max_retries) {
            motor_req(fcp,9,0,0);
            (*fcp->rw_cb)(fcp->rw_tag,FDCAM_READ_ERROR);
	} else if (fcp->rw_retry == 1)
            ll_rwv_c(fsb, 0);
        else if (fcp->rw_retry == 2)
            ll_seek(fsb, -1, 1, ll_rwv_c, fsb);
        else if (fcp->rw_retry == 3)
            ll_seek(fsb, -1, 2, ll_rwv_c, fsb);
        else if (fcp->rw_retry == 4)
            ll_seek(fsb, -1, 3, ll_rwv_c, fsb);
        else if (fcp->rw_retry == 5)
            ll_seek(fsb, -1, 4, ll_rwv_c, fsb);
        else if (fcp->rw_retry == 6)
            ll_rwv_c(fsb, 0);
        else if (fcp->rw_retry == 7)
            ll_seek(fsb, -1, 1, ll_rwv_c, fsb);
        else if (fcp->rw_retry == 8)
            ll_seek(fsb, -1, 2, ll_rwv_c, fsb);
        else if (fcp->rw_retry == 9)
            ll_seek(fsb, -1, 3, ll_rwv_c, fsb);
        else if (fcp->rw_retry == 10)
            ll_seek(fsb, -1, 4, ll_rwv_c, fsb);
        else {
            motor_req(fcp,9,0,0);
            (*fcp->rw_cb)(fcp->rw_tag,FDCAM_READ_ERROR);
	}
        return;
    } else if ((fcp->ildc_ir.rb[0]&ST0_IC_MASK) == ST0_IC_ABNORMAL
            &&  (fcp->ildc_ir.rb[1]&ST1_NW)  &&  fcp->rw_op == OP_WRITE) {
        (*fcp->rw_cb)(fcp->rw_tag,FDCAM_WRITE_PROTECT);
        return;
    }
#if CODE_DMA
    else if (fcp->rw_use_dma && (fcp->ildc_ir.rb[1]&ST1_OR)) {
        fsb->rwv_pause = 1;

        if (fcp->rw_dma_retry++ > 10) {
            EEPRINTF("\n****** GIVE UP ON DMA RETRY AT TRY #%d !\n",
                fcp->rw_dma_retry);
            motor_req(fcp,9,0,0);
            (*fcp->rw_cb)(fcp->rw_tag,FDCAM_HARDWARE_ERR);
	} else {
#if FDCAM_DEBUG
	    dumpreg(fcp);
#endif	   
                                       /* Wait at most 0.8 disk revolution.  */
            if (fsb->dma_retry_wait) {
                timeout(ll_rw_timo, fsb, fsb->dma_retry_wait);
                CPRINTF(0x1000|FDCAM_TRACE, "\n**DMARTY #%d,t=%d..\n",
                        fcp->rw_dma_retry, fsb->dma_retry_wait, 0);
                CPRINTF(0x1000|FDCAM_TRACE, " R:dma:0x%08lx",
                        fcp->rw_idma_addr, 0, 0);
                CPRINTF(0x1000|FDCAM_TRACE,"dir=%c, ",
                        fcp->rw_op == OP_READ ? 'R' : 'W', 0, 0);

                CPRINTF(0x1000|FDCAM_TRACE,
                        "st0=0x%02x, st1=0x%02x,  st2=0x%02x,",
                        fcp->ildc_ir.rb[0]&0xFF,
                        fcp->ildc_ir.rb[1]&0xFF,
                        fcp->ildc_ir.rb[2]&0xFF);

                CPRINTF(0x1000|FDCAM_TRACE,
                        "dor=0x%02x, sra=0x%02x, srb=0x%02x)\n",
                        FDCAM_READ(fcp->r, DOR)&0xFF, 
                        FDCAM_READ(fcp->r, SRA)&0xFF,
                        FDCAM_READ(fcp->r, SRB)&0xFF);
	    } else {
                CPRINTF(0x1000|FDCAM_TRACE, "\n**DMA RETRY #%d..\n",
                        fcp->rw_dma_retry, 0, 0);
                ll_rw_timo(fsb);
	    }
	}
        return;
    }
#endif
#if CODE_POLL
    else if (!fcp->rw_use_dma && fcp->ildc_ir.rb[1]&ST1_OR) {
        fsb->rwv_pause = 1;
        if (fcp->rw_dma_retry++ > 10) {
            TPRINTF("\n****** GIVE UP ON IO RETRY AT TRY #%d !\n",
                fcp->rw_dma_retry);
            motor_req(fcp,9,0,0);
            (*fcp->rw_cb)(fcp->rw_tag,FDCAM_READ_ERROR);
	} else {
            TPRINTF("\n**DMA RETRY #%d..  *** TBD *** \n",
                    fcp->rw_dma_retry);
	    {
		UNS8 dummy;

		sense_i_s(fcp, &dummy, &dummy);
	    }
	    ll_rwv_c(fsb, 0);
	    return;
	}
    }
#endif


    VPRINTF("\n(llrvwd le)",0);

    motor_req(fcp,9,0,0);
    (*fcp->rw_cb)(fcp->rw_tag,FDCAM_NORMAL);
}


#define EC 0
#define DTL 0xFF
#if EC
#define DTL_SC ****ERROR***
#else
#define DTL_SC DTL
#endif
#define MT 0
#define SK 0

static void 
ll_rwv_c(struct fdcam_status_block* fsb, long stat)
{
    struct fdcam_class* fcp = fsb->fcp;
    struct fdcam_media_specs* fms = fsb->fms;
    enum fdcam_xfer_rate drate = fms->xfer_rate;
    int gap = make_gap3(fcp->rw_op==OP_WRITE?fms->gap_3_write:fms->gap_3_read,
            drate, 0);
    int		flags = 0;

    VPRINTF("(llrvwc,%d)",stat);

    if (stat || gap<0) {
        motor_req(fcp,9,0,0);
        (*fcp->rw_cb)(fcp->rw_tag,stat?stat:FDCAM_INVALID_SPEC);
        return;
    }

    fcp->rw_cmd[2] = CB2_READ_DATA(fcp->rw_cylinder);
    fcp->rw_cmd[3] = CB3_READ_DATA(fcp->rw_surface);
    fcp->rw_cmd[4] = CB4_READ_DATA(fcp->rw_sector);
    fcp->rw_cmd[5] = CB5_READ_DATA(2);
    fcp->rw_cmd[6] = CB6_READ_DATA(fcp->rw_sector+fcp->rw_count-1);
    fcp->rw_cmd[7] = CB7_READ_DATA(gap);

    if (fcp->rw_op == OP_READ) {
        fcp->rw_cmd[0] = CB0_READ_DATA(MT,fms->is_mfm,SK);
        fcp->rw_cmd[1] = CB1_READ_DATA(fcp->rw_surface,fsb->lun_id);
        fcp->rw_cmd[8] = CB8_READ_DATA(DTL);
    } else if (fcp->rw_op == OP_WRITE) {
        fcp->rw_cmd[0] = CB0_WRITE_DATA(MT,fms->is_mfm);
        fcp->rw_cmd[1] = CB1_WRITE_DATA(fcp->rw_surface,fsb->lun_id);
        fcp->rw_cmd[8] = CB8_WRITE_DATA(DTL);
    } else { /* fcp->rw_op == OP_VERIFY */
        fcp->rw_cmd[0] = CB0_VERIFY(MT,fms->is_mfm,SK);
        fcp->rw_cmd[1] = CB1_VERIFY(EC,fcp->rw_surface,fsb->lun_id);
        fcp->rw_cmd[8] = CB8_VERIFY(DTL_SC);
    }

#if CODE_DMA
    if (fcp->rw_use_dma) {
	flags = (fcp->rw_op == OP_READ) ? DMA_IN : DMA_OUT;

	if (dma_map_load(fcp->rw_count * SECTOR_SIZE, (vm_offset_t)fcp->rw_buf,
			 fcp->rw_proc, fcp->ctlr, &fcp->sglp, 0, flags) == 0) {
	    CPRINTF(FDCAM_TRACE, "rtnbc = 0 in ll_rwv_c",0, 0, 0);
	    drop_dead(998);
	}
	    
	ildc(fcp,CBN_RW_DATA,fcp->rw_cmd,RBN_RW_DATA,IM_INT,ll_rwv_d,fsb);
    }
#endif	/* CODE_DMA */
#if CODE_POLL
    if (!fcp->rw_use_dma)
        ilxc (fcp, CBN_RW_DATA, fcp->rw_cmd, RBN_RW_DATA, ll_rwv_d, fsb,
                fcp->rw_buf, fcp->rw_proc, fcp->rw_count*SECTOR_SIZE,
                fcp->rw_op);
#endif
}


static void 
ll_rwv_b(struct fdcam_status_block* fsb, long stat)
{
    struct fdcam_class* fcp = fsb->fcp;
    VPRINTF("(llrvwb,%d)",stat);
    if (stat) {
        motor_req(fcp,9,0,0);
        (*fcp->rw_cb)(fcp->rw_tag,stat);
    } else if (fcp->rw_do_seek)
        ll_seek(fsb, fcp->rw_cylinder, 0, ll_rwv_c, fsb);
    else 
	ll_rwv_c(fsb, 0);
}

static void 
ll_rwv(struct fdcam_status_block* fsb, enum enum_op op, int pre_select,
       int do_seek, int cylinder, int head, int sector, int count,
       unsigned char* buffer, struct proc* process, int max_retries,
       sb_l_cb cb, void* cb_tag)
{
    struct fdcam_class* fcp = fsb->fcp;
    CPRINTF(FDCAM_TRCRW,
            "\n------(llrwv,%s, count=%d, med=%s,",
            (op*7+"READ\0\0\0WRITE\0\0VERIFY\0"), count,
            fsb->fms->diag_name);

    CPRINTF(FDCAM_TRCRW, " c=%d, h=%d, s=%d)-----",
            cylinder, head, sector);

#if CODE_DMA && CODE_POLL
    fcp->rw_use_dma = fsb->lun_id==0;  /* Use dma for drive zero, for test.  */
#elif CODE_DMA
    fcp->rw_use_dma = 1;
#elif CODE_POLL
    fcp->rw_use_dma = 0;
#else
 #+++++ error, at least one of CODE_DMA or CODE_POLL must be true.
#endif

    fsb->rwv_pause = 0;
    fcp->rw_op = op;
    fcp->rw_surface = head;
    fcp->rw_cylinder = cylinder;
    fcp->rw_sector = sector;
    fcp->rw_count = count;
    fcp->rw_buf = buffer;
    fcp->rw_proc = process;
    fcp->rw_cb = cb;
    fcp->rw_tag = cb_tag;
    fcp->rw_do_seek = do_seek;
    fcp->rw_retry = 0;
    fcp->rw_dma_retry = 0;
    fcp->rw_max_retries = max_retries;


 /* if (count > 2)  ++++ */
    if (count > 1) {
        (*cb)(cb_tag,FDCAM_INVALID_SPEC);
        return;
    }
    
#if CODE_DMA
    if (fcp->rw_use_dma) {
        fdcam_select(fsb, 3, 0, ll_rwv_b, fsb);
    }
#endif
#if CODE_POLL
    if (!fcp->rw_use_dma)
        fdcam_select(fsb, 3, 1, ll_rwv_b, fsb);
#endif

}


/* ************************************************************************* */
/* ************************************************************************* */
/*                                                         UTILITY FUNCTIONS */
/* ************************************************************************* */
/* ************************************************************************* */


/*****************************************************************************/
/*                                                         fdcam_init_fsb()  */
/*****************************************************************************/
/* Initialize fsb block.                                                     */
/*                                                                           */
/* Return values:                                                            */
/*  FDCAM_PROGRAMMING_ERR       Fsbp, g_fcp, or fdsp null                    */
/*                              Unsupported drive_select_mask,               */
/*                               media_id_mask, density_select_mask,         */
/*                               static_control_mask, or lun_id.             */
/*  FDCAM_NORMAL                All is well.                                 */

enum fdcam_status 
fdcam_init_fsb(struct fdcam_status_block* fsbp, 
	       struct fdcam_class* g_fcp, 
	       struct fdcam_drive_specs* fdsp,
	       struct fdcam_media_specs** avail_fms,
	       int drive_no,
	       FDCAM_PIN drive_select,
	       char configured_present)
{
    struct fdcam_media_specs** tmp_fmsp;

    if (!fsbp || !g_fcp || !fdsp)
        return FDCAM_PROGRAMMING_ERR;

#if FDCAM_DEBUG
      fdcam_diag[6] = (unsigned long)fsbp;
#endif
        
    fsbp->is_present = configured_present != 0;
    fsbp->configured_present = configured_present;
    fsbp->fds = fdsp;

                                       /* ``avail_fms'' is the list of media-*/
                                       /*  specs that might be used on this  */
                                       /*  drive.  ``fms'' is the media-spec */
                                       /*  for the media currently in the    */
                                       /*  drive.  If there is one spec in   */
                                       /*  the list, then assign it to       */
                                       /*  ``fms''.                          */
    fsbp->avail_fms = avail_fms ? avail_fms : &end_of_fms_list;
    if (fsbp->avail_fms[0] && !fsbp->avail_fms[1])
        fsbp->fms = fsbp->avail_fms[0];
    else
        fsbp->fms = &fms_default;

    fsbp->read_retry_count = 0;

    fsbp->fdcam_fsbck = 0;
    fsbp->fcp = g_fcp;

    fsbp->interleave = 0;
    fsbp->door_open = 2;
    fsbp->media_changed = 1;
    fsbp->write_protect = 2;
    fsbp->head_position = -1;

    /* PUBLIC, USER-WRITABLE: */
    fsbp->drive_no = drive_no;

    /* Check for unsupported conifgurations.                                 */

    if (fdsp->drive_select_mask != FDCAM_PIN_INVALID)
        return FDCAM_PROGRAMMING_ERR;  /* Only normal drive addressing ok.   */
    
    if (fdsp->media_id_mask & ~FDCAM_PIN_17 & ~FDCAM_PIN_27)
        return FDCAM_PROGRAMMING_ERR;  /* Only subset of these allowed for id*/

    if (fdsp->density_select_mask & ~FDCAM_PIN_2 & ~FDCAM_PIN_6)
        return FDCAM_PROGRAMMING_ERR;  /* Only subset of these allowed for   */
                                       /*  density select.                   */

    if (FD6_TO_TICKS_RU(fdsp->motor_on_delay) > g_fcp->motor_on_delay)
        g_fcp->motor_on_delay = FD6_TO_TICKS_RU(fdsp->motor_on_delay);
    if (FD6_TO_TICKS_RU(fdsp->motor_off_delay) > g_fcp->motor_off_delay)
        g_fcp->motor_off_delay = FD6_TO_TICKS_RU(fdsp->motor_off_delay);

    if (fdsp->static_control_mask)
        return FDCAM_PROGRAMMING_ERR;  /* Not allowed.                       */

    fsbp->dma_retry_wait =
            FD6_TO_TICKS_RD(fdsp->revolution_period*8/10);
    fsbp->half_spin =
            FD6_TO_TICKS_RD(fdsp->revolution_period*5/10);

    fsbp->lun_id = drive_select;

    if (fsbp->lun_id == 0)
        fsbp->drive_select = FDCAM_PIN_10;
    else if (fsbp->lun_id == 1)
        fsbp->drive_select = FDCAM_PIN_12;
    else if (fsbp->lun_id == 2)
        fsbp->drive_select = FDCAM_PIN_14;
    else
        return FDCAM_PROGRAMMING_ERR;  /* Only normal drive addressing ok.   */


    for (tmp_fmsp = avail_fms ; *tmp_fmsp ; tmp_fmsp++) {
        if ((*tmp_fmsp)->steps_per_cylinder<1
                || (*tmp_fmsp)->steps_per_cylinder>4)
            return FDCAM_PROGRAMMING_ERR;

                                       /* The following restriction eases    */
                                       /*  programming of the 82077AA.  If   */
                                       /*  this restriction needs to be      */
                                       /*  lifted, that is easy to do.       */
        if ((*tmp_fmsp)->cylinders_p_disk * (*tmp_fmsp)->steps_per_cylinder
                >= 255)
            return FDCAM_PROGRAMMING_ERR;
    }

    fsbp->fdcam_fsbck = FSBCK_MAGIC;   /* If bad things start happening, we  */
                                       /*  can add a check for this magic    */
                                       /*  number.                           */
    return FDCAM_NORMAL;
}


/*****************************************************************************/
/*                                                       fdcam_psn_to_chs()  */
/*****************************************************************************/
/* Initialize fsb block.                                                     */
/*                                                                           */
/* Return values:                                                            */
/*  FDCAM_PROGRAMMING_ERR       fms not specified.                           */
/*  FDCAM_FAILURE               psn out of range.                            */
/*  FDCAM_NORMAL                All ok.                                      */

enum fdcam_status 
fdcam_psn_to_chs(struct fdcam_media_specs* fms,
		 unsigned long psn,
		 unsigned short* cylinder,
		 unsigned short* head,
		 unsigned short* sector,
		 unsigned short* remainder,
		 unsigned short* track)
{
    if (fms && fms != &fms_default) {
        enum fdcam_status err;
        *sector = psn%fms->sectors_p_track + fms->sector_offset;

        if (remainder)
            *remainder = fms->sectors_p_track-1+fms->sector_offset-*sector;

        psn /= fms->sectors_p_track;

        if (track)
            *track = psn;

        if (fms->surf) {
            *cylinder = psn%fms->cylinders_p_disk;
            psn /= fms->cylinders_p_disk;
            err = (*head = psn)>=fms->n_surfaces
                    ? FDCAM_FAILURE : FDCAM_NORMAL;
	} else {
            *head = psn%fms->n_surfaces;
            psn /= fms->n_surfaces;
            err = (*cylinder = psn)>=fms->cylinders_p_disk
                    ? FDCAM_FAILURE : FDCAM_NORMAL;
	}
        if (err) {
            IEPRINTF("[xxxxx ptc error,%d]",psn);
	}
        return err;
    } else {
        IEPRINTF("[xxxxx ptc pe]",0);
        return FDCAM_PROGRAMMING_ERR;
    }
}
        

/*****************************************************************************/
/*                                                       fdcam_chs_to_psn()  */
/*****************************************************************************/
/* Initialize fsb block.                                                     */
/*                                                                           */
/* Return values:                                                            */
/*  FDCAM_PROGRAMMING_ERR       fms not specified.                           */
/*  FDCAM_FAILURE               cylinder, head, or sector out of range.      */
/*  FDCAM_NORMAL                All ok.                                      */

enum fdcam_status 
fdcam_chs_to_psn(struct fdcam_media_specs* fms, unsigned short cylinder, 
		 unsigned short head, unsigned short sector, 
		 unsigned long* psn)
{
    sector -= fms->sector_offset;
    if (fms && fms != &fms_default) {
        if (fms->surf)
            *psn = (head*fms->cylinders_p_disk+cylinder)
                    *fms->sectors_p_track+sector;
        else
            *psn = (cylinder*fms->n_surfaces+head)
                    *fms->sectors_p_track+sector;
        return  cylinder >= fms->cylinders_p_disk
                ||  head >= fms->n_surfaces
                ||  sector >= fms->sectors_p_track 
                ? FDCAM_FAILURE : FDCAM_NORMAL;
    } else
        return FDCAM_PROGRAMMING_ERR;
}
        

/* ************************************************************************* */
/*                                    INITIALIZATION AND SHUT-DOWN FUNCTIONS */
/* ************************************************************************* */


/*****************************************************************************/
/*                                                       fdcam_initialize()  */
/*****************************************************************************/
/* Initialize fsb block.                                                     */
/*                                                                           */
/* Return values:                                                            */
/*  FDCAM_NORMAL                All ok.                                      */
/*  FDCAM_PROGRAMMING_ERR       Fcpp not specified.                          */
/*  FDCAM_PROGRAMMING_ERR       Called more than once.                       */
/*  FDCAM_FAILURE               No Mem -- kmalloc failure.                   */
/*  FDCAM_HARDWARE_ERR          ACVIO while accessing controller or ASIC     */
/*                               registers.                                  */
/*  FDCAM_HARDWARE_ERR          No interrupt or other hardware error from    */
/*                               controller chip.                            */

enum fdcam_status 
fdcam_initialize(struct fdcam_class** fcpp, void* interface_number, 
		 void* fd_tag_1, struct controller *ctlr)
{
    struct fdcam_class* fcp = &fc_0;
    unsigned char reset_results[8];
    int i;
    int s;

    if (!fcpp || *fcpp)
        return FDCAM_PROGRAMMING_ERR;

    *fcpp = fcp;

    fcp->interface_number = interface_number;

    fdcam_class_ctor(fcp, fdi_intr_err);

    fcp->fd_tag_1 = fd_tag_1;
    fcp->ctlr = ctlr;

#if CODE_DMA
    /*
     * Call alloc with DMA_SLEEP flag because allocator doesn't have
     * have any memory yet.  Allocator will not sleep but it will use
     * the correct VM allocator to get the memory at bootstrap.
     */
    if (dma_map_alloc(SECTOR_SIZE, fcp->ctlr, &fcp->sglp, DMA_SLEEP) == 0) {
	CPRINTF(FDCAM_TRACE, "dma_map_alloc got 0 rtnbc", 0, 0, 0);
	return FDCAM_HARDWARE_ERR;
    }

#if TESTING_DEALLOC
    printf("\n");
    printf("\n");
    printf("\n");
    printf("Calling dma_map_dealloc \n");
    if (dma_map_dealloc(fcp->sglp) ==0) {
	CPRINTF(FDCAM_TRACE, "dma_map_alloc got 0 rtnbc", 0, 0, 0);
	return FDCAM_HARDWARE_ERR;
    }
    printf("\n");
    printf("Calling dma_map_alloc again\n");
    printf("\n");
    printf("\n");

    if (dma_map_alloc(SECTOR_SIZE, fcp->ctlr, &fcp->sglp, DMA_SLEEP) == 0) {
        CPRINTF(FDCAM_TRACE, "dma_map_alloc got 0 rtnbc", 0, 0, 0);
        return FDCAM_HARDWARE_ERR;
    }
#endif /* TESTING DEALLOC */

#endif /* CODE_DMA */

    if (i = tl_do_command(fcp,0,0,8,reset_results,IM_IWS_RESET_EIM,0)) {
        FDCAM_DIAG(39);
        /* drop_dead(); ++++ */
        return FDCAM_HARDWARE_ERR;
    }

    return FDCAM_NORMAL;
}


int 
fdcam_check_status(struct fdcam_class* fcp)
{
    if (fcp != &fc_0)
        return 1;
    else if (fcp->great_state == GS_INIT)
        return 0;
    else if (fcp->dead_code > 4)
        return fcp->dead_code;
    else if (fcp->great_state == GS_NO_INIT)
        return 2;
    else if (fcp->great_state == GS_NO_INIT)
        return 3;
    else
        return 4;
}

enum fdcam_status 
fdcam_shutdn(struct fdcam_class* fcp, int force)
{
    if (!fcp || fcp->great_state != GS_INIT)
        return FDCAM_PROGRAMMING_ERR;
    if (!force)
        /* ++++  Handle ``force''*/ ;
    shut_it_down(fcp, 0, 101);
    fdcam_class_dtor(fcp);
    return FDCAM_NORMAL;
}

/* ************************************************************************* */
/*                                                PROBE AND SET-UP FUNCTIONS */
/* ************************************************************************* */

static void 
fpd_a(struct fdcam_status_block* fsb, long stat)
{
    if (stat)
        fsb->is_present = 0;
    
    motor_req(fsb->fcp, 9, 0, 0);
    f_s_user_done(fsb, FDCAM_NORMAL);
}

static void 
probe_drive_b(struct fdcam_status_block* fsb, long stat)
{
    if (stat) {
        fsb->is_present = 0;
	f_s_user_done(fsb, FDCAM_NORMAL);
    } else {
	ll_recal(fsb, fpd_a, fsb);
    }
}


enum fdcam_status 
fdcam_probe_drive(struct fdcam_status_block* fsb, FDCAM_CB cb, 
		  FDCAM_CB_TAG cb_tag)
{
    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);
    fsb->head_position = -1;
    if (fsb->configured_present == 0)
        fsb->is_present = 0;
    else if (fsb->configured_present == 1) {
        fsb->is_present = 1;

        if (fsb->fds->can_recal == 2) {
	    fdcam_select(fsb, 1, -1, probe_drive_b, fsb);
            return fdcam_user_wait(&fsb->fcp->cs);
	}
    } else if (fsb->configured_present == 2)
        fsb->is_present = 1;

    return fdcam_user_return(&fsb->fcp->cs, FDCAM_NORMAL);
}


static int probe_timer;

static void
probe_media_b(struct fdcam_status_block* fsb, long stat)
{

    if (stat == FDCAM_NORMAL) {
	probe_timer = 1;
    } else {
	probe_timer = 2;
    }

    wakeup(&probe_timer);
}
    

static enum fdcam_status 
probe_media(struct fdcam_status_block* fsb)
{
    struct fdcam_media_specs** fmsp;

    fsb->fms = fsb->avail_fms[0];
    VPRINTF("<PROBE_MEDIA>",0);
    fsb->head_position = -1;
    fsb->interleave = 0;
    if (!fsb->avail_fms || !*fsb->avail_fms)
        return FDCAM_PROGRAMMING_ERR;
    
    fsb->fcp->rc.dor= (fsb->fcp->rc.dor&~DOR_DRIVE_SEL_MASK)
            | LUN_TO_DOR_DRIVE_SEL(fsb->lun_id);
    FDCAM_WRITE(fsb->fcp->r, DOR, fsb->fcp->rc.dor);
    FDCAM_SYNC();
    VPRINTF("(*B* dor=0x%08lx)",fsb->fcp->rc.dor);

    for (fmsp = fsb->avail_fms; *fmsp ; fmsp++) {
	/* set media type to try */
	fsb->fms = *fmsp;

	probe_timer = 99;
	ll_rwv(fsb, OP_READ, 1, 1, 0, 0, fsb->fms->sector_offset, 
	       1, fsb->fcp->std_buf, 0, 2, probe_media_b, fsb);

	while (probe_timer == 99) 
	    sleep(&probe_timer, PRIBIO + 1);

	if (probe_timer == 1)
	    break;

	/* reset media type to undefined */
	fsb->fms = &fms_default;
    }

    VPRINTF("<id:%s>",fsb->fms->diag_name);

    if (fsb->fms->bytes_p_sector != SECTOR_SIZE)
        return FDCAM_PROGRAMMING_ERR;
    
    return FDCAM_NORMAL;
}


enum fdcam_status 
fdcam_probe_media(struct fdcam_status_block* fsb, FDCAM_CB cb, 
		  FDCAM_CB_TAG cb_tag)
{
    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);
    return fdcam_user_return(&fsb->fcp->cs, probe_media(fsb));
}

enum fdcam_status 
fdcam_set_media(struct fdcam_status_block* fsb, struct fdcam_media_specs* x)
{
    VPRINTF("(F_SET_MEDIA)",0);
    fsb->fms = x;

    return FDCAM_NORMAL;
}

/* ************************************************************************* */
/*                                                  CONTROL ACTION FUNCTIONS */
/* ************************************************************************* */

/* +++ fdcam_abort();               +++ */

/* The following assumes that the drive is selected and the motor is         */
/*  running.                                                                 */
static void 
ll_update_chk(struct fdcam_status_block* fsb)
{
    unsigned char status;

    if (sense_drive_status(fsb, &status)) {
	(*fsb->fcp->llu_cb)(fsb->fcp->llu_cb_tag, FDCAM_HARDWARE_ERR);
	return;
    }
	
    fsb->write_protect = (status & ST3_WP) ? 1 : 0;

    VPRINTF("(ll_update_chk: wp = %x)", fsb->write_protect);

    if (FDCAM_READ(fsb->fcp->r, DIR) & DIR_DSK_CHG) {
        fsb->media_present = 0;
        fsb->door_open = 1;
        fsb->media_changed = 1;
    } else {
        fsb->media_present = 1;
        fsb->door_open = 0;
    }
}

static void 
ll_update_b(struct fdcam_status_block* fsb, long stat)
{
    struct fdcam_class* fcp = fsb->fcp;
    VPRINTF("(llub,%d)",stat);
    if (stat)
        FDCAM_DIAG(144);
    ll_update_chk(fsb);
    if (!fsb->media_present)
        motor_req(fsb->fcp,7,0,0);
    else if (fcp->llu_autoprobe)
        probe_media(fsb);
    (*fcp->llu_cb)(fcp->llu_cb_tag,0);
}

static void fdcam_seek_b(struct fdcam_status_block* fsb, long stat);

static void 
ll_update_a(struct fdcam_status_block* fsb, long stat)
{
    struct fdcam_class* fcp = fsb->fcp;
    VPRINTF("(llu_a)",0);

    if (stat) {
        drop_dead(159);
        (*fcp->llu_cb)(fcp->llu_cb_tag,159);
    }

    ll_update_chk(fsb);
    fcp->llu_tampered = fsb->media_changed;

    if (fsb->media_changed) {
        VPRINTF("(X1X,%d)",fsb->media_changed);
        ll_seek(fsb, 0, -1, ll_update_b, fsb);
    } else {
        if (fcp->llu_autoprobe == 2)
            probe_media(fsb);
        VPRINTF("(X2X)",0);
        (*fcp->llu_cb)(fcp->llu_cb_tag,0);
    }
}

static void 
ll_update(struct fdcam_status_block* fsb, int ap, sb_l_cb cb, void* cb_tag)
{
    VPRINTF("(llu)",0);
#if FDCAM_DEBUG
      if (!cb) {
          IEPRINTF("(llu-err-x)",0);
          return;
      }
#endif
    fsb->fcp->llu_cb = cb;
    fsb->fcp->llu_cb_tag = cb_tag;
    fsb->fcp->llu_autoprobe = ap;
    fdcam_select(fsb, 1, -1, ll_update_a, fsb);
}

static void 
fdcam_chk_b(struct fdcam_status_block* fsb, long stat)
{
    int result;
    VPRINTF("(fckb,%d)",stat);

    fsb->media_changed = !fsb->media_present;
    result = fsb->media_present ? fsb->fcp->llu_tampered ?
            FDCAM_MEDIA_TAMPER : FDCAM_NORMAL : FDCAM_NO_MEDIA;
    CPRINTF(FDCAM_TRACE,"<TAMPER:%s,present:%d,mo:%d>", print_stat(result),
            fsb->media_present, fsb->fcp->mo_state);
    f_s_user_done(fsb, result);
}

enum fdcam_status 
fdcam_chk_no_media_tamper(struct fdcam_status_block* fsb, int autoprobe, 
			  FDCAM_CB cb, FDCAM_CB_TAG cb_tag)
{
    VPRINTF("(TAMPER_CHECK)",0);
    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);
    ll_update(fsb, autoprobe, fdcam_chk_b, fsb);
    return fdcam_user_wait(&fsb->fcp->cs);
}


/* Return 1 if fdcam_chk_no_media_tamper or fdcam_update should be called    */
/* for more information.  Return 0 if no chance of media tamper.             */
int fdcam_quick_tamper_check(struct fdcam_status_block* fsb)
{
    struct fdcam_class* fcp = fsb->fcp;
    int i;
    VPRINTF("(QUICK_TPR_CHK:",0);
    if (!fcp->up_to_date || ((LUN_TO_DOR_DRIVE_SEL(fsb->lun_id)^
            fcp->rc.dor)&DOR_DRIVE_SEL_MASK)) {
        fcp->rc.dor = fcp->rc.dor&~DOR_DRIVE_SEL_MASK
                | LUN_TO_DOR_DRIVE_SEL(fsb->lun_id)&DOR_DRIVE_SEL_MASK;
	FDCAM_WRITE(fcp->r, DOR, fcp->rc.dor);
	FDCAM_SYNC();
    }
    motor_req(fcp, 1, 0, 0);
    i = FDCAM_READ(fcp->r, DIR)&DIR_DSK_CHG;
    fsb->media_changed |= i;

    VPRINTF("%d",fsb->media_changed);
    VPRINTF("/%d)",i);

    return fsb->media_changed;
}


enum fdcam_status 
fdcam_update(struct fdcam_status_block* fsb, int autoprobe, 
	     FDCAM_CB cb, FDCAM_CB_TAG cb_tag)
{
    VPRINTF("(update)",0);
    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);
    ll_update(fsb, autoprobe, f_s_user_done, fsb);
    return fdcam_user_wait(&fsb->fcp->cs);
}

enum fdcam_status 
fdcam_motor_off(struct fdcam_status_block* fsb)
{
    motor_req(fsb->fcp,7,0,0);
}

/* ************************************************************************* */
/*                                                   SIMPLE ACTION FUNCTIONS */
/* ************************************************************************* */


static void 
fdcam_seek_b(struct fdcam_status_block* fsb, long stat)
{
    struct fdcam_class* fcp = fsb->fcp;
    CPRINTF(FDCAM_TRC1,"(fsb0,stat=%d,seek_w=%d,seek_c=%d)",
            stat,fcp->seek_w,fcp->seek_c);

    ll_seek(fsb, fcp->seek_c, fcp->seek_w, f_s_user_done, fsb);
}


enum fdcam_status
fdcam_seek(struct fdcam_status_block* fsb, 
	   long cylinder,              /* Where to seek to.                  */
	                               /*  Specify -1 to re-seek to current  */
                                       /*  cylinder,                         */
	   int whence,                 /* 0 = from current position.         */
                                       /* 1 = inner-to-outer direction.      */
                                       /* 2 = outer-to-inner direction.      */
                                       /* 3 = recalibrate first, then inner- */
                                       /*      to-outer direction.           */
                                       /* 4 = recalibrate first, then outer- */
                                       /*      to-inner direction.           */
	   FDCAM_CB cb, FDCAM_CB_TAG cb_tag)
{
    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);

    fsb->fcp->seek_c = cylinder;
    fsb->fcp->seek_w = whence;
    fdcam_select(fsb, 2, -1, fdcam_seek_b, fsb);

    return fdcam_user_wait(&fsb->fcp->cs);
}


enum fdcam_status 
fdcam_read(struct fdcam_status_block* fsb,
	   unsigned short cylinder,
	   unsigned short head,
	   unsigned short sector,
	   unsigned char* buffer,
	   struct proc* process,
	   FDCAM_CB cb,
	   FDCAM_CB_TAG cb_tag)
{
    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);
    ll_rwv (fsb, OP_READ, 1, 0, cylinder, head, sector, 1, buffer,
            process, 10, f_s_user_done, fsb);
    return fdcam_user_wait(&fsb->fcp->cs);
}


enum fdcam_status 
fdcam_write(struct fdcam_status_block* fsb,
	    unsigned short cylinder,
	    unsigned short head,
	    unsigned short sector,
	    unsigned char* buffer,
	    struct proc* process,
	    FDCAM_CB cb,
	    FDCAM_CB_TAG cb_tag)
{
    long i;

    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);
    ll_rwv (fsb, OP_WRITE, 1, 0, cylinder, head, sector, 1, buffer,
            process, 10, f_s_user_done, fsb);
    return fdcam_user_wait(&fsb->fcp->cs);
}


enum fdcam_status 
fdcam_verify(struct fdcam_status_block* fsb,
	     unsigned short cylinder,
	     unsigned short head,
	     unsigned short sector,
	     unsigned char* buffer,
	     struct proc* process,
	     FDCAM_CB cb,
	     FDCAM_CB_TAG cb_tag)
{
    long i;

    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);
    ll_rwv (fsb, OP_VERIFY, 1, 0, cylinder, head, sector, 1, buffer,
            process, 10, f_s_user_done, fsb);
    return fdcam_user_wait(&fsb->fcp->cs);
}


enum fdcam_status 
fdcam_read_at(struct fdcam_status_block* fsb,
	      unsigned short cylinder,
	      unsigned short head,
	      unsigned short sector,
	      unsigned char* buffer,
	      struct proc* process,
	      int allow_mismatch,      /* If non-zero, the read will be      */
                                       /*  successful even if the cylinder   */
                                       /*  or head numbers do not match.     */
	      FDCAM_CB cb,
	      FDCAM_CB_TAG cb_tag)
{
    long i;
    /* ++++ do something with allow mismatch. */

    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);
    ll_rwv (fsb, OP_READ, 1, 1, cylinder, head, sector, 1, buffer,
            process, 10, f_s_user_done, fsb);
    return fdcam_user_wait(&fsb->fcp->cs);
}


enum fdcam_status 
fdcam_write_at(struct fdcam_status_block* fsb,
	       unsigned short cylinder,
	       unsigned short head,
	       unsigned short sector,
	       unsigned char* buffer,
	       struct proc* process,
	       FDCAM_CB cb,
	       FDCAM_CB_TAG cb_tag)
{
    long i;

    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);
    ll_rwv(fsb, OP_WRITE, 1, 1, cylinder, head, sector, 1, buffer,
            process, 10, f_s_user_done, fsb);
    return fdcam_user_wait(&fsb->fcp->cs);
}


enum fdcam_status 
fdcam_verify_at(struct fdcam_status_block* fsb,
		unsigned short cylinder,
		unsigned short head,
		unsigned short sector,
		unsigned char* buffer,
		struct proc* process,
		FDCAM_CB cb,
		FDCAM_CB_TAG cb_tag)
{
    long i;

    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);
    ll_rwv(fsb, OP_VERIFY, 1, 1, cylinder, head, sector, 1, buffer,
            process, 10, f_s_user_done, fsb);
    return fdcam_user_wait(&fsb->fcp->cs);
}


        
/* ************************************************************************* */



/* ************************************************************************* */
/*                                                      fdcam_format_track() */
/* ************************************************************************* */
/* This is the lowest-level routine that is specific to formatting media.    */
/*  The main entry point is fdcam_format_track().  The possible              */
/*  return values are (via callback):                                        */
/*     FDCAM_NORMAL         All completed OK                                 */
/*     FDCAM_WRITE_PROTECT  Media write protected during format.             */
/*     FDCAM_HARDWARE_ERR   Too many retries while waiting for DMA to run.   */
/*     FDCAM_FAILURE        Bad media.                                       */
/*     FDCAM_INVALID_SPEC   One of the following in error:                   */
/*                               fsb->fms->xfer_rate                         */
/*                               fsb->fms->gap_3_format                      */
/*  Other return values include those that might be generated by any of the  */
/*  following functions:                                                     */
/*     ll_seek()                                                             */
/*     ildc()                                                                */
/*     ll_select()                                                           */



static void fft_c(struct fdcam_status_block* fsb, long stat);

static void 
fft_d(struct fdcam_status_block* fsb, long stat)
{
    vm_offset_t new_dma_addr;
    struct fdcam_class* fcp = fsb->fcp;

    dma_map_unload(0, fcp->sglp);

    VPRINTF("(fft_d,%d)",stat);
    if (stat) {
        f_s_user_done(fsb,stat);
        return;
    }

    if (fcp->ildc_ir.rb[1]&ST1_NW) {
        f_s_user_done(fsb,FDCAM_WRITE_PROTECT);
    } else if (fcp->ildc_ir.rb[1]&ST1_OR) {
        if (fcp->fft_dma_retry++ > 100) {
            CPRINTF(FDCAM_TRACE,
                    "\n***** GIVE UP ON DMA RETRY AT TRY #%d, FMT !\n",
                    fcp->fft_dma_retry, 0, 0);
            f_s_user_done(fsb,FDCAM_HARDWARE_ERR);
	} else {
            CPRINTF(FDCAM_TRACE,
                    "\n****** DMA ENGINE NO RUN, FMT, TRY #%d, TRY AGAIN...\n",
                    fcp->fft_dma_retry, 0, 0);
            fft_c(fsb, 0);
	}
        return;
    } else if ((fcp->ildc_ir.rb[0]&ST0_IC_MASK) != ST0_IC_NORMAL) {
        CPRINTF(FDCAM_TRACE, "(ABNORMAL rb0 after FFT, 0=0x%02x, 1=0x%02x)",
                fcp->ildc_ir.rb[0], fcp->ildc_ir.rb[1], 0);
        f_s_user_done(fsb,FDCAM_FAILURE);
        return;
    }

    VPRINTF("\n(fftc le)",0);

    f_s_user_done(fsb,FDCAM_NORMAL);
}

static void 
fft_c(struct fdcam_status_block* fsb, long stat)
{
    struct fdcam_class* fcp = fsb->fcp;
    unsigned char cmd[CBN_FORMAT_TRACK];
    struct fdcam_media_specs* fms = fsb->fms;
    enum fdcam_xfer_rate drate = fms->xfer_rate;
    int gap = make_gap3(fms->gap_3_format, drate, 1);
    int filler = 0xF6;
    int interleave = 1;
    int fsl = 0;
    unsigned char* lp;
    unsigned char* lp_end;
    int s;
    int i, j;

    if (fcp->fft_ffs_ok) {
        if (fcp->fft_ffs.gpl >= 0)
            gap = fcp->fft_ffs.gpl;

        if (fcp->fft_ffs.filler >= 0)
            filler = fcp->fft_ffs.filler;

        if (fcp->fft_ffs.interleave_factor > 0)
            interleave = fcp->fft_ffs.interleave_factor;

        if (fcp->fft_ffs.first_sector_location > 0)
            fsl = fcp->fft_ffs.first_sector_location;
    }

    VPRINTF("(fft_c,%d)",stat);
    if (stat || gap<0) {
        f_s_user_done(fsb,stat?stat:FDCAM_INVALID_SPEC);
        return;
    }

    cmd[0] = CB0_FORMAT_TRACK(fms->is_mfm);
    cmd[1] = CB1_FORMAT_TRACK(fcp->fft_surface,fsb->lun_id);
    cmd[2] = CB2_FORMAT_TRACK(2);
    cmd[3] = CB3_FORMAT_TRACK(fms->sectors_p_track);
    cmd[4] = CB4_FORMAT_TRACK(gap);
    cmd[5] = CB5_FORMAT_TRACK(filler);

    fcp->fft_xsize = fms->sectors_p_track * 4;

    /* check that the format will not overrun buffer */
    if (fcp->fft_xsize > SECTOR_SIZE) {
	VPRINTF("Format size larger than std_buf", 0);
	f_s_user_done(fsb, FDCAM_INVALID_SPEC);
	return;
    }

    for (i = 0 ; i < fms->sectors_p_track ; i++)
        fcp->std_buf[i*4] = fcp->fft_cylinder+1;

    lp_end = fcp->std_buf + fms->sectors_p_track*4;
    lp = fcp->std_buf + (fsl%fms->sectors_p_track)*4;

    for (i = 0 ; i < fms->sectors_p_track ; i++) {
        while (lp[0] == fcp->fft_cylinder)
            if ((lp+=4) >= lp_end)
                lp = fcp->std_buf;

        lp[0] = fcp->fft_cylinder;
        lp[1] = fcp->fft_surface;
        lp[2] = i + fms->sector_offset;
        lp[3] = 2;

        for (j = 0 ; j < interleave ; j++)
            if ((lp+=4) >= lp_end)
                lp = fcp->std_buf;
    }

#if FDCAM_DEBUG
    VPRINTF("(F-Order(i=%d):",interleave);
    for (i = 0 ; i < fms->sectors_p_track ; i++) {
        VPRINTF("%d,",fcp->std_buf[i*4+2]);
        if (fcp->std_buf[i*4+0] != fcp->fft_cylinder)
            IEPRINTF("(BAD_CYL_ERR)",0);
    }
    VPRINTF(")",0);
#endif
    
    fsb->next_format_position = (lp-fcp->std_buf)/4;

    if (dma_map_load(fcp->fft_xsize, (vm_offset_t)fcp->std_buf, (struct proc *)0, 
		     fcp->ctlr, &fcp->sglp, 0, DMA_OUT) == 0) {
	CPRINTF(FDCAM_TRACE, "rtnbc=0 in fft_c",0 ,0, 0);
	drop_dead(997);
    }
    ildc (fcp,CBN_FORMAT_TRACK,cmd,RBN_FORMAT_TRACK,IM_INT,fft_d,fsb);
}

static void 
fft_b(struct fdcam_status_block* fsb, long stat)
{
    struct fdcam_class* fcp = fsb->fcp;
    VPRINTF("(fft_b,%d)",stat);
    fcp->fft_dma_retry = 0;

    if (stat)
        f_s_user_done(fsb,stat);
    else
        ll_seek(fsb, fcp->fft_cylinder, 0, fft_c, fsb);
}


enum fdcam_status 
fdcam_format_track(struct fdcam_status_block* fsb, 
		   unsigned short cylinder,    /* Cylinder to format.        */
		   unsigned short surface,     /* Surface to format.         */
		   struct fdcam_format_spec* ffsp,
		                       /* Format specification.  Note, the   */
                                       /*  complete specification includes   */
                                       /*  this structure along with         */
                                       /*  ``*fsbp''.                        */
		   FDCAM_CB cb,
		   FDCAM_CB_TAG cb_tag)
{
    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);

    fsb->fcp->fft_cylinder = cylinder;
    fsb->fcp->fft_surface = surface;
    fsb->fcp->fft_index = 0;

    if (ffsp) {
        fsb->fcp->fft_ffs_ok = 1;
        fsb->fcp->fft_ffs = *ffsp;
        fsb->interleave = fsb->fcp->fft_ffs.interleave_factor;
    } else {
        fsb->fcp->fft_ffs_ok = 0;
        fsb->interleave = 1;
    }

    fdcam_select(fsb, 3, 0, fft_b, fsb);
    return fdcam_user_wait(&fsb->fcp->cs);
}






/* +++ fdcam_read_trk();            +++ */
/* +++ fdcam_write_trk();           +++ */
/* +++ fdcam_read_sid();            +++ */
/* +++ fdcam_eject();               +++ */


/* ************************************************************************* */
/*                                                 COMPOUND ACTION FUNCTIONS */
/* ************************************************************************* */


static void 
at_b_timo(struct fdcam_status_block* fsb)
{
    untimeout(at_b_timo, fsb);
    fsb->fcp->at_did_timo = 1;
}


static void 
fdcam_at_b(struct fdcam_status_block* fsb, long stat)
{
    struct fdcam_class* fcp = fsb->fcp;
    int did_back_off = 0;

    VPRINTF("(fdcam_at_b,%d)",stat);

    if (stat) {
        TPRINTF("(fdcam_at_b,fail,stat=%d)",stat);
        untimeout(at_b_timo, fsb);
        user_done(0,fcp,stat);
    } else {
        unsigned short surface;
        unsigned short cylinder;
        unsigned short sector;
        unsigned short remainder;
        enum fdcam_status stat;

        if (fcp->at_first) {
            fcp->at_first = 0;
            fcp->at_prev_cyl = 0xFFFF;
            fcp->at_prev_head = 0xFFFF;
            fcp->at_start_buf = fcp->at_buffer;
            fcp->at_start_psn = fcp->at_psn;
            fcp->at_pass = 0;
	} else {
            if (fcp->at_did_timo >= 0 && !fsb->rwv_pause) {
                untimeout(at_b_timo, fsb);
                if (fcp->at_sw_intl == 1)
                    fsb->interleave = fcp->at_did_timo ? 1 : 2;
	    }
            fcp->at_psn += fcp->at_sw_intl;
            fcp->at_buffer += fcp->at_sw_intl*SECTOR_SIZE;
	}

        VPRINTF("(FX)",0);
        if (fcp->at_psn >= fcp->at_start_psn+fcp->at_nsec) {
            if (++fcp->at_pass < fcp->at_sw_intl) {
                did_back_off = 1;
                fcp->at_psn = fcp->at_start_psn + fcp->at_pass;
                fcp->at_buffer = fcp->at_start_buf
                        + fcp->at_pass*SECTOR_SIZE;
	    }
            if (fcp->at_psn >= fcp->at_start_psn+fcp->at_nsec) {
                user_done(0,fcp,FDCAM_NORMAL);
                return;
	    }
	}

        CPRINTF(FDCAM_TRACE, "(fdcam_at_b,psn=%d,cnt=%d,%s,", fcp->at_psn,
            fcp->at_nsec, fcp->at_op==OP_READ?"OP_READ":"OP_WRITE");
        CPRINTF(FDCAM_TRACE, "lun=%d,pass=%d)", fsb->lun_id, fcp->at_pass, 0);

        if (stat = fdcam_psn_to_chs(fsb->fms, fcp->at_psn, &cylinder,
                &surface, &sector, &remainder, 0)) {
            user_done(0,fcp,FDCAM_FAILURE);
        } else {
            int sc = fcp->at_prev_cyl == cylinder;
            if (sc && surface == fcp->at_prev_head && !did_back_off) {
                fcp->at_did_timo = 0;
                timeout(at_b_timo, fsb, fsb->half_spin);
	    } else {
                fcp->at_did_timo = -1;
	    }
            fcp->at_prev_cyl = cylinder;
            fcp->at_prev_head = surface;
            ll_rwv(fsb,fcp->at_op,!sc,1,cylinder,surface,sector,1,
                    fcp->at_buffer,fcp->at_process,10,fdcam_at_b,fsb);
	}
    }
}


enum fdcam_status 
fdcam_read_at_psn(struct fdcam_status_block* fsb,
		  unsigned long psn,   /* The sector to begin read at.       */
		  unsigned long nsec,  /* The number of sectors to read.     */
		  unsigned char* buffer,/* The buffer to read.               */
		  struct proc* process,
		  int sw_intl,         /* Software interleave.  typ=1.       */
		  FDCAM_CB cb,
		  FDCAM_CB_TAG cb_tag)
{
    VPRINTF("(FRAP)",0);
    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);
    VPRINTF("(F-A)",0);
    fsb->fcp->at_psn = psn;
    fsb->fcp->at_nsec = nsec;
    fsb->fcp->at_buffer = buffer;
    fsb->fcp->at_process = process;
    fsb->fcp->at_op = OP_READ;
    fsb->fcp->at_first = 1;
    fsb->fcp->at_sw_intl = sw_intl>0 ? sw_intl : 1;
    fdcam_at_b(fsb,0);
    VPRINTF("(F-B)",0);

    return fdcam_user_wait(&fsb->fcp->cs);
}

enum fdcam_status 
fdcam_write_at_psn(struct fdcam_status_block* fsb,
		   unsigned long psn, /* The sector to begin write at.       */
		   unsigned long nsec,/* The number of sectors to write.     */
		   unsigned char* buffer,/* The buffer to write.             */
		   struct proc* process,
		   int sw_intl,        /* Software interleave.  typ=1.       */
		   FDCAM_CB cb,
		   FDCAM_CB_TAG cb_tag)
{
    VPRINTF("(FWAP)",0);
    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);
    fsb->fcp->at_psn = psn;
    fsb->fcp->at_nsec = nsec;
    fsb->fcp->at_buffer = buffer;
    fsb->fcp->at_process = process;
    fsb->fcp->at_op = OP_WRITE;
    fsb->fcp->at_first = 1;
    fsb->fcp->at_sw_intl = sw_intl>0 ? sw_intl : 1;
    fdcam_at_b(fsb,0);
    return fdcam_user_wait(&fsb->fcp->cs);
}


enum fdcam_status 
fdcam_verify_at_psn(struct fdcam_status_block* fsb,
		    unsigned long psn, /* The sector to begin read at.       */
		    unsigned long nsec,/* The number of sectors to read.     */
		    unsigned char* buffer,/* The buffer to read.             */
		    struct proc* process,
		    int sw_intl,       /* Software interleave.  typ=1.       */
		    FDCAM_CB cb,
		    FDCAM_CB_TAG cb_tag)
{
    fdcam_user_init(&fsb->fcp->cs, fsb, cb, cb_tag);
    fsb->fcp->at_psn = psn;
    fsb->fcp->at_nsec = nsec;
    fsb->fcp->at_buffer = buffer;
    fsb->fcp->at_process = process;
    fsb->fcp->at_op = OP_VERIFY;
    fsb->fcp->at_first = 1;
    fsb->fcp->at_sw_intl = sw_intl>0 ? sw_intl : 1;
    fdcam_at_b(fsb,0);

    return fdcam_user_wait(&fsb->fcp->cs);
}



/* ************************************************************************* */
/*                                                          #if FDCAM_DEBUG  */
/* ************************************************************************* */

#if FDCAM_DEBUG



/* ************************************************************************* */
/*                                                 SILLY JUST FOR FUN COMNDS */
/* ************************************************************************* */

# include <io/dec/fdi/fdi.h>
# include "fdioctst.h"

extern struct fdcam_media_specs fms_350ed;

void 
do_fdioctst(int minor_minor_num, int flag, struct fd_ioctst_struct* data)
{
    vm_offset_t phys;
    static unsigned long* drp = fdcam_diag+5;
    switch(data->cmd)
        {
        case SREG:
        case CREG:
        case WREG:
        case RREG:
#           define X(A) (data->cmd==WREG ? (*(A)=data->d2) : \
              data->cmd==SREG ? (*(A)|=data->d2) : \
              data->cmd==CREG ? (*(A)&=~data->d2) : (data->d2= *(A)))
            switch(data->d1)
                {
                case 0:  X(fc_0.r+SRA);        break;
                case 1:  X(fc_0.r+SRB);        break;
                case 2:  X(fc_0.r+DOR);        break;
                case 3:  X(fc_0.r+TDR);        break;
                case 4:  X(fc_0.r+MSR);        break;
                case 5:  X(fc_0.r+DAT);        break;
                case 6:  X(fc_0.r+RES);        break;
                case 7:  X(fc_0.r+DIR);        break;

                case 13: X(&fd_trigger);       break;
                case 14: X(&fdcam_debug_mask); break;
                case 15: X(&fd_cbuf);          break;
                case 16: X(&fd_foo);           break;

                case 20: X(&fms_350ed.write_precomp_delay); break;
                case 21: X(&fms_350ed.write_precomp_cyl);   break;
                case 22: X(&fms_350ed.gap_3_read);          break;
                case 23: X(&fms_350ed.gap_3_write);         break;
                case 24: X(&fms_350ed.gap_3_format);        break;
                case 25: X(&fms_350ed.is_perpen);           break;
                case 27: X(&fc_0.cs.fsb->dma_retry_wait);   break;

                default:
                    printf("EEEEE");
                }
            if (data->cmd==RREG && data->d1 <= 7)
                data->d2 &= 0xFF;
            break;
        case INIT_DIAG_REPORT:
            drp = fdcam_diag+5;
            break;
        case DIAG_REPORT:
            if (drp >= fdcam_diag_p)
                data->d1 = 0, data->d2 = 1;
            else
                data->d1 = *drp++, data->d2 = 0;
            break;
        case RESET_DIAG_LOG:
            fdcam_diag_p = fdcam_diag+8;
            drp = fdcam_diag+5;
            break;
        case REINIT:
            {
            struct fdcam_class* dummy = 0;
            if (fdcam_initialize(&dummy, fc_0.interface_number,
                    fc_0.fd_tag_1, fc_0.ctlr))
                printf("<reinit error>\n");
            else
                printf("<reinit ok>\n");
            break;
            }
            break;
        case WCHAR:
            *(unsigned char*)(PHYS_TO_K1(data->d1)) = (unsigned char)data->d2;
            break;
        case RCHAR:
            {
            unsigned char* cp = (unsigned char*)(PHYS_TO_K1(data->d1));
            if (BADADDR((caddr_t)cp, 1)) {
                data->d3 = 1;
                data->d2 = 0;
	    } else {
                data->d3 = 0;
                data->d2 = *cp;
	    }
            }
            break;
        case WLONG:
            *(unsigned long*)(PHYS_TO_K1(data->d1)) = (unsigned long)data->d2;
            break;
        case RLONG:
            {
            unsigned long* lp = (unsigned long*)(PHYS_TO_K1(data->d1));
            if (BADADDR((caddr_t)lp, 4)) {
                data->d3 = 1;
                data->d2 = 0;
	    } else {
                data->d3 = 0;
                data->d2 = *lp;
	    }
            }
            break;
        case RVLONG:
            {
            unsigned long* lp = (unsigned long*)(data->d1);
            if (BADADDR((caddr_t)lp, 4)) {
                data->d3 = 1;
                data->d2 = 0;
	    } else {
                data->d3 = 0;
                data->d2 = *lp;
	    }
            }
            break;
        case WVLONG:
            *(unsigned long*)(data->d1) = (unsigned long)data->d2;
            break;
        case RVTBADDR:
            data->d2 = (unsigned long)fc_0.std_buf;
            data->d3 = 0;
            break;
        case RPTBADDR:
	    svatophys(fc_0.std_buf,&phys);
	    data->d2 = (unsigned long)phys;
            data->d3 = 0;
            break;
        case TLDC:
            printf("<START TLDC...>");
            data->d1 = tl_do_command(&fc_0, data->d1, data->buf, data->d2, 
                    data->buf+2, data->d3, data->buf);
            printf("<...END TLDC>");
            break;
        case TEST1:
            /*
	    FDCAM_WRITE(fc_0.r, DAT, data->d1);
	    FDCAM_SYNC();
            data->d2 = FDCAM_READ(fc_0.r, MSR) & 0xFF;
            */
            break;
        case INT_ON:
            /* +++ */
        case INT_OFF:
            /* +++ */
        default:
            printf("do_fdioctst: No such command, %d.\n", data->cmd);
        }
    wbflush();
    fc_0.up_to_date = 0;
}

#endif  /* FDCAM_DEBUG */

