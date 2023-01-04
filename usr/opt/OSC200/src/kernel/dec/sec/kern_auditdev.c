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
auditopen(), auditclose(), auditread(), auditioctl(), auditsel(), auditmmap(),
audlock_init(), initaud(), kernaudwrite()

auditopen():    setup, check for exclusive use
auditclose():   nop, return(0);

auditread():    perform a uiomove of up to # of chars specified,
                adjust /dev/audit memory buffers/ptrs

auditioctl():   support routines for mmap

auditsel():     set return val so select() works properly

auditmmap():    mmap audit buffer - for use by auditd

audlock_init(): initialize audit locks; called from global_lock_initialization

initaud():      initialize audit log buffer and pointers

kernaudwrite(): kernel write into audit buffers
*/

#include <sys/types.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/select.h>
#include <kern/kalloc.h>
#include <sys/systm.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/poll.h>
#include <sys/vmmac.h>
#include <sys/ioctl.h>

struct aud_log {                      /* pointers into local audit buffer  */
    caddr_t base;   /* base */
    caddr_t head;   /* head */
    caddr_t last;   /* last entered (aud_log highwater mark)    */
    caddr_t tail;   /* tail */
    caddr_t save;   /* used for recovery iff /dev/audit "read"  */
} aud_log;

static struct queue_entry aud_selq;
static int aud_queued = 0;            /* 1: thread enqueued for select op  */

#ifdef DEBUG
struct {                              /* statistics for tuning             */
    u_int n_read;
    u_int n_getn;
    u_int n_sel;
    u_int n_kernwrite;
    u_int n_writesleep;
} aud_stats;
#endif /* DEBUG */

static int aud_inuse = 0;             /* to make /dev/audit exclusive use  */
static int aud_setup = 1;             /* need to initialize                */
static unsigned int size;             /* /dev/audit buffer size            */
static int thrshld;                   /* threshold before auditd wakeup    */
static int flush = 0;                 /* audit data flush trigger          */

pid_t  auditd_pid;                    /* pid of proc which open'ed device  */

/* -- locks -- */
udecl_simple_lock_data(,audlog_lock)
#define	AUDLOG_LOCK()       usimple_lock(&audlog_lock)
#define	AUDLOG_UNLOCK()     usimple_unlock(&audlog_lock)
#define	AUDLOG_LOCK_INIT()  usimple_lock_init(&audlog_lock)

udecl_simple_lock_data(,audselq_lock)
#define	AUDSELQ_LOCK()       usimple_lock(&audselq_lock)
#define	AUDSELQ_UNLOCK()     usimple_unlock(&audselq_lock)
#define	AUDSELQ_LOCK_INIT()  usimple_lock_init(&audselq_lock)

udecl_simple_lock_data(extern,audbuf_lock)
#define	AUDBUF_LOCK_INIT()  usimple_lock_init(&audbuf_lock)

udecl_simple_lock_data(extern,audmask_lock)
#define	AUDMASK_LOCK_INIT()  usimple_lock_init(&audmask_lock)


/* close the audit device */
auditclose()
{
    AUDLOG_LOCK();
    if ( aud_inuse == 0 ) {
        AUDLOG_UNLOCK();
        return(0);
    }
    aud_inuse = 0;
    AUDLOG_UNLOCK();

    AUDSELQ_LOCK();
    select_wakeup ( &aud_selq );
    select_dequeue_all ( &aud_selq );
    AUDSELQ_UNLOCK();
    auditd_pid = 0;
    return(0);
}


/* miscellaneous support routines for mmap */
auditioctl ( dev, cmd, data, flag )
dev_t dev;
int cmd;
caddr_t data;
int flag;
{
    struct audiocp audiocp;
    u_int len = 0;
    int error = 0;

    AUDLOG_LOCK();

    /* calculate # bytes of data available */
    if ( aud_log.head > aud_log.tail ) 
        len = aud_log.head - aud_log.tail;
    else if ( aud_log.head < aud_log.tail ) {
        if ( aud_log.tail >= aud_log.last ) {
            /* safe transition */
            aud_log.tail = aud_log.base;
            aud_log.last = aud_log.head;
        }
        len = aud_log.last - aud_log.tail;
    }


    switch ( cmd ) {

    /* get # bytes audit data available */
    case AUDIOCGETN:
        audiocp.size = size;
        audiocp.offset = aud_log.tail - aud_log.base;
        audiocp.nbytes = len;
        *(struct audiocp *)data = audiocp;
#ifdef DEBUG
        aud_stats.n_getn++; /* stats */
#endif /* DEBUG */
        break;

    /* set ptr's as if data just read */
    case AUDIOCSETN:
        aud_log.tail += *(u_int *)data < len ? *(u_int *)data : len;
        aud_log.save = aud_log.tail;
        /* wakeup any process waiting for space to write data */
        wakeup ( (caddr_t)&aud_log );
        break;

    default:
        error = EINVAL;

    }

    AUDLOG_UNLOCK();
    return ( error );
}


/* return ptr to audit device data buffer */
auditmmap ( dev, off, prot )
dev_t dev;
off_t off;
int prot;
{
    caddr_t physaddr;

    /* map sys virt to phys */
    if ( off > size ) return(-1);
    if ( svatophys ( aud_log.base+off, &physaddr ) != KERN_SUCCESS ) return(-1);

    /* return byte to page value */
    return ( btop(physaddr) );
}


/* open (initialize if necessary) audit device */
auditopen ( dev, mode )
dev_t dev;
int mode;
{
    AUDLOG_LOCK();
    /* exclusive use device */
    if ( aud_inuse ) {
        AUDLOG_UNLOCK();
        return ( EACCES );
    }
    aud_inuse = 1;

    /* one-time initialization */
    if ( aud_setup ) {
        initaud();
        aud_setup = 0;
    }

    aud_log.save = aud_log.tail;
    AUDLOG_UNLOCK();

    auditd_pid = u.u_procp->p_pid;
    return(0);
}


/* pass buffer data to user (obsolete; use mmap interface) */
auditread ( dev, uio )
dev_t dev;
struct uio *uio;
{
    struct iovec *iov = uio->uio_iov;
    int rval = 0;
    int amt = 0;

#ifdef DEBUG
    aud_stats.n_read++;
#endif /* DEBUG */


    /* move as much data as possible from audit buffer to user */
    if ( iov->iov_len <= 0 || aud_log.head == aud_log.tail ) return(0);
    AUDLOG_LOCK();
    if ( aud_log.head > aud_log.tail )
        amt = iov->iov_len < aud_log.head-aud_log.tail ? iov->iov_len : aud_log.head-aud_log.tail;
    else if ( aud_log.head < aud_log.tail ) {
        if ( aud_log.tail >= aud_log.last ) {
            /* safe transition */
            aud_log.tail = aud_log.base;
            aud_log.last = aud_log.head;
        }
        amt = iov->iov_len < aud_log.last-aud_log.tail ? iov->iov_len : aud_log.last-aud_log.tail;
    }
    rval = uiomove ( aud_log.tail, amt, uio );
#ifdef notdef
    /* used when spec_read() disallowed neg offsets for char devices.
       not needed with off_t typedef'ed to ulong_t.
    */
    uio->uio_offset = 0;
#endif /* notdef */
    aud_log.save = aud_log.tail;
    aud_log.tail += amt;
    AUDLOG_UNLOCK();


    /* wakeup anyone who got sleep'ed for auditor */
    wakeup ( (caddr_t)&aud_log );

    return ( rval );
}


/* set return val so select() works properly */
auditsel ( dev, events, revents, scanning )
dev_t dev;
short *events, *revents;
int scanning;
{
    if ( scanning ) {
        if ( *events & POLLNORM ) {

            /* pass only when arbitrary threshold exceeded */
            AUDLOG_LOCK();
            if ( flush 
              ||
            ( aud_log.head > aud_log.tail &&
            aud_log.tail-aud_log.base < thrshld &&
            aud_log.base+size-aud_log.head < thrshld )
              ||
            ( aud_log.head < aud_log.tail && 
            aud_log.tail-aud_log.head < thrshld ) ) {
                flush = 0;
                *revents |= POLLNORM;
            }

            else {
                AUDSELQ_LOCK();
                select_enqueue ( &aud_selq );
                AUDSELQ_UNLOCK();
                aud_queued++;
            }

            AUDLOG_UNLOCK();
#ifdef DEBUG
            aud_stats.n_sel++;
#endif /* DEBUG */
        }
    }

    else {
        AUDSELQ_LOCK();
        select_dequeue ( &aud_selq );
        AUDSELQ_UNLOCK();
    }

    return(0);
}


/* initialize audit locks; called from global_lock_initialization */
audlock_init()
{
    AUDSELQ_LOCK_INIT();
    AUDLOG_LOCK_INIT();
    AUDBUF_LOCK_INIT();
    AUDMASK_LOCK_INIT();
}


/* initializations; expect to be called under AUDLOG_LOCK */
initaud()
{
    /* smp TODO: size and thrshld should be f(#cpu's, memsize) */
    size = audsize*1024 < AUD_BUF_SIZ<<2 ? AUD_BUF_SIZ<<4 : audsize*1024<<2;
    thrshld = size>>2;
    aud_log.base = kalloc ( size );
    if ( aud_log.base == NULL ) panic ( "auditdev" );

    aud_log.head = aud_log.base;
    aud_log.tail = aud_log.base;
    aud_log.last = aud_log.base + size;
    aud_log.save = aud_log.base;

#ifdef DEBUG
    aud_stats.n_read        = 0;
    aud_stats.n_getn        = 0;
    aud_stats.n_sel         = 0;
    aud_stats.n_kernwrite   = 0;
    aud_stats.n_writesleep  = 0;
#endif /* DEBUG */

    queue_init ( &aud_selq );
}


/* copy data from kern_audit.c buffer to local buffer */
kernaudwrite ( buf, len, flag )
char *buf;
int len;
int flag;
{
    caddr_t buf_l;

#ifdef DEBUG
    aud_stats.n_kernwrite++;
#endif /* DEBUG */

    /* move as much data as possible into audit buffer */
    if ( len <= 0 ) {
        flush = flag;
        return(0);
    }

    AUDLOG_LOCK();
    /* one-time initialization */
    if ( aud_setup ) {
        initaud();
        aud_setup = 0;
    }

    /* test for buffer overflow; sleep offending proc */
    for ( ; ( aud_log.head < aud_log.save && len >= aud_log.save-aud_log.head )
      ||
    ( aud_log.head > aud_log.save && len >= aud_log.save-aud_log.base &&
    len >= aud_log.base+size-aud_log.head )
      ||
    ( aud_log.head == aud_log.save && aud_log.save != aud_log.tail ); ) {
#ifdef DEBUG
        aud_stats.n_writesleep++;
#endif /* DEBUG */
        flush++;
        aud_queued = 0;
        AUDSELQ_LOCK();
        select_wakeup ( &aud_selq );
        AUDSELQ_UNLOCK();
        mpsleep ( (caddr_t)&aud_log, PZERO, "audit driver", 0, 
            simple_lock_addr(audlog_lock), MS_LOCK_SIMPLE );
    }

    /* copy the data over */
    if ( aud_log.head >= aud_log.save ) {
        if ( len < aud_log.base+size-aud_log.head ) {
            bcopy ( buf, aud_log.head, len );
            aud_log.head += len;
            aud_log.last = aud_log.head;
        }
        else if ( len < aud_log.save - aud_log.base ) {
            buf_l = aud_log.base;
            bcopy ( buf, buf_l, len );
            aud_log.last = aud_log.head;
            aud_log.head = aud_log.base + len;
        }
#ifdef DEBUG
        else printf ( "kern_auditdev: audit buffer overflow A: %d %lx %lx %lx %lx\n",
        len, aud_log.base, aud_log.tail, aud_log.head, aud_log.save );
#endif /* DEBUG */
    }
    else {
        if ( len < aud_log.save - aud_log.head ) {
            bcopy ( buf, aud_log.head, len );
            aud_log.head += len;
        }
#ifdef DEBUG
        else printf ( "kern_auditdev: audit buffer overflow B: %d %lx %lx %lx %lx\n",
        len, aud_log.base, aud_log.tail, aud_log.head, aud_log.save );
#endif /* DEBUG */
    }
    
    /* wakeup processes sitting in select on /dev/audit device */
    /* don't need to lock aud_queued */
    flush = flag;
    if ( aud_queued ) {
        if ( flag
        ||
        ( aud_log.head > aud_log.save &&
        aud_log.save-aud_log.base < thrshld &&
        aud_log.base+size-aud_log.head < thrshld )
        ||
        ( aud_log.head < aud_log.save && 
        aud_log.save-aud_log.head < thrshld ) ) {
            aud_queued = 0;
            AUDSELQ_LOCK();
            select_wakeup ( &aud_selq );
            AUDSELQ_UNLOCK();
        }
    }

    AUDLOG_UNLOCK();
    return ( len );
}
