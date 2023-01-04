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
 *
 *  File created by Larry Scott
 *
 */

#ifndef __AUDIT__
#define __AUDIT__ 1

#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/syscall.h>
#include <sys/systm.h>
#include <sys/habitat.h>

/* audcntl options */
#define GET_SYS_AMASK           0
#define SET_SYS_AMASK           1
#define GET_TRUSTED_AMASK       2
#define SET_TRUSTED_AMASK       3
#define GET_PROC_AMASK          4
#define SET_PROC_AMASK          5
#define GET_PROC_ACNTL          6
#define SET_PROC_ACNTL          7
#define GET_AUDSWITCH           8
#define SET_AUDSWITCH           9
#define GETPAID                10
#define SETPAID                11
#define GET_AUDSTYLE           12
#define SET_AUDSTYLE           13
#define GET_SITEMASK           14
#define SET_SITEMASK           15
#define GET_OBJAUDBIT          16
#define SET_OBJAUDBIT          17
#define UPDEVENTS              18
#define FLUSH_AUD_BUF          19
#define GET_HABITAT_EVENT      20
#define SET_HABITAT_EVENT      21
#define GET_NSITEVENTS         22
#define GET_AUDSIZE            23

/* audcntl flags */
#define AUDIT_OR             0x00
#define AUDIT_AND            0x01
#define AUDIT_OFF            0x02
#define AUDIT_USR            0x04

/* audcntl UPDEVENTS flags */
#define UPD_CNTL             0x01
#define UPD_MASK             0x02

/* audstyle flags */
#define AUD_EXEC_ARGP        0x01
#define AUD_EXEC_ENVP        0x02
#define AUD_LOGIN_UNAME      0x04

/* audioctl commands and structure */
#define AUDIOCGETN           _IOR('C', 0, struct audiocp)
#define AUDIOCSETN           _IOW('C', 1, int)
struct audiocp {
    u_int size;              /* size of audit buffer      */
    u_int offset;            /* offset into buffer        */
    u_int nbytes;            /* # bytes of data available */
};

/* misc values */
#define AUID_INVAL          -1
#define AUDITD_RECSZ         (MAXPATHLEN*2)
#define AUD_BUF_SIZ          4096
#define AUD_MAXEVENT_LEN     64
#define N_SYSCALLS           ((LAST&~0x3)+(NBBY>>1))
#define MAXHABSYSCALL_LEN    128
#define AUD_VNOMAX           2      /* must be at least 2 */
#define N_AUDTUPLES          128


/* audit version number
    also distinguish between 64-bit and 32-bit longword
*/
#define AUD_VERSION          0x0001
#define AUD_VERS_LONG        0x8000     /* 64-bit longword */


/* token encodings:
    bit 0x80 (TP_* tokens) indicates private token; cannot be given to audgen(2)
    0x00-0x1f, 0x80-0x8f are reserved for ptr token types
*/
#define A_TOKEN_PRIV(x)      ((x)&0x80 ? 1 : 0)
#define A_TOKEN_PTR(x)       (((x)&0x7f) >= 0x01 && ((x)&0x7f) <= 0x1f ? 1 : 0)

/* -- public tokens -- */
#define T_CHARP              0x01
#define T_SOCK               0x03
#define T_LOGIN              0x04
#define T_HOMEDIR            0x05
#define T_SHELL              0x06
#define T_DEVNAME            0x07
#define T_SERVICE            0x08
#define T_HOSTNAME           0x09

#define T_AUID               0x20
#define T_RUID               0x21
#define T_UID                0x22
#define T_PID                0x23
#define T_PPID               0x24
#define T_GID                0x25
#define T_EVENT              0x26
#define T_SUBEVENT           0x27

#define T_DEV                0x28
#define T_ERRNO              0x29
#define T_RESULT             0x2a
#define T_MODE               0x2b
#define T_HOSTADDR           0x2c
#define T_INT                0x2d
#define T_DESCRIP            0x2e
#define T_HOSTID             0x2f

#define T_X_ATOM             0x30
#define T_X_CLIENT           0x31
#define T_X_PROPERTY         0x32
#define T_X_RES_CLASS        0x33
#define T_X_RES_TYPE         0x34
#define T_X_RES_ID           0x35


/* -- private tokens -- */
#define TP_ACCRGHT           0x81
#define TP_MSGHDR            0x82
#define TP_EVENTP            0x83
#define TP_HABITAT           0x84
#define TP_ADDRVEC           0x85
#define TP_INTP              0x86

#define TP_AUID              0xa1
#define TP_RUID              0xa2
#define TP_UID               0xa3
#define TP_PID               0xa4
#define TP_PPID              0xa5
#define TP_HOSTADDR          0xa6
#define TP_EVENT             0xa7

#define TP_SUBEVENT          0xa8
#define TP_NCPU              0xa9
#define TP_DEV               0xaa
#define TP_LENGTH            0xab
#define TP_IPC_GID           0xac
#define TP_IPC_MODE          0xad
#define TP_IPC_UID           0xae
#define TP_TV_SEC            0xaf

#define TP_TV_USEC           0xb0
#define TP_SHORT             0xb1
#define TP_LONG              0xb2
#define TP_VNODE_DEV         0xb3
#define TP_VNODE_ID          0xb4
#define TP_VNODE_MODE        0xb5
#define TP_VERSION           0xb6
#define TP_SET_UIDS          0xb7


/* trusted (application) audit events */
#define MIN_TRUSTED_EVENT    512
#define AUDIT_START          512
#define AUDIT_STOP           513
#define AUDIT_SETUP          514
#define AUDIT_SUSPEND        515
#define AUDIT_LOG_CHANGE     516
#define AUDIT_LOG_CREAT      517
#define AUDIT_XMIT_FAIL      518
#define AUDIT_REBOOT         519
#define AUDIT_LOG_OVERWRITE  520
#define AUDIT_DAEMON_EXIT    521
#define LOGIN                522
#define LOGOUT               523
#define AUTH_EVENT           524
#define AUDGEN8              525
#define SW_COMPATIBILITY     526

#define X_SERVER_STARTUP     540
#define X_SERVER_SHUTDOWN    541
#define X_CLIENT_STARTUP     542
#define X_CLIENT_SHUTDOWN    543
#define X_SERVER_DAC         544
#define X_CLIENT_IPC         545
#define X_OBJECT_CREATE      546
#define X_OBJECT_RENAME      547
#define X_OBJECT_DESTROY     548
#define X_OBJECT_DAC         549
#define X_OBJECT_READ        550
#define X_OBJECT_WRITE       551

#define MAX_TRUSTED_EVENT    (MIN_TRUSTED_EVENT+N_TRUSTED_EVENTS-1)

/* trusted, site event info */
#define N_TRUSTED_EVENTS     64
#define MIN_SITE_EVENT       2048


/* macros used for auditmask lengths
    kernel space: used as int-mask
    user space: used as char-mask
*/
#define AUDMASK_LEN(x)       (x > 0 ? ((x)-1)/(NBBY>>1)+1 : 0)
#define SYSCALL_MASK_LEN     AUDMASK_LEN(N_SYSCALLS)
#define TRUSTED_MASK_LEN     AUDMASK_LEN(N_TRUSTED_EVENTS)
#define AUDIT_MASK_LEN       (SYSCALL_MASK_LEN+TRUSTED_MASK_LEN)
#define AUDINTMASK_LEN(x)    (x > 0 ? ((x)-1)/(WORD_BIT>>1)+1 : 0)
#define SYSCALL_INTMASK_LEN  AUDINTMASK_LEN(N_SYSCALLS)
#define TRUSTED_INTMASK_LEN  AUDINTMASK_LEN(N_TRUSTED_EVENTS)
#define AUDIT_INTMASK_LEN    (SYSCALL_INTMASK_LEN+TRUSTED_INTMASK_LEN)


/* macros used to adjust various auditmasks */
#define BYTE_IN_MASK(event)         ((event)>>2)
#define BITS_IN_BYTE(status,event)  ( (status) << ( (~(event) & ((NBBY>>1)-1)) << 1) )
#define WORD_IN_MASK(event)         ((event)>>4)
#define BITS_IN_WORD(status,event)  ( (status) << EVENT_SHIFT(event) )
#define EVENT_SHIFT(event)          \
    (((event)&((WORD_BIT>>1)-1) | 0x3) - ((event)&0x3) << 1)


/* adjust buf for system audit_mask by setting event succeed/fail bits */
#define A_SYSMASK_SET(buf,event,succeed,fail) \
    { if ( (event) >= MIN_TRUSTED_EVENT ) { \
        buf[BYTE_IN_MASK((event)-MIN_TRUSTED_EVENT)] &= ~BITS_IN_BYTE(0x3,(event)-MIN_TRUSTED_EVENT); \
        buf[BYTE_IN_MASK((event)-MIN_TRUSTED_EVENT)] |= \
            BITS_IN_BYTE(( ((succeed)<<1)|(fail) ),(event)-MIN_TRUSTED_EVENT); \
      } \
      else { \
        buf[BYTE_IN_MASK(event)] &= ~BITS_IN_BYTE(0x3,event); \
        buf[BYTE_IN_MASK(event)] |= BITS_IN_BYTE(( ((succeed)<<1)|(fail) ),event); \
      } \
    }

/* adjust buf for user audit_mask by setting event succeed/fail bits */
#define A_PROCMASK_SET(buf,event,succeed,fail) \
    { if ( (event) >= MIN_TRUSTED_EVENT ) { \
        buf[BYTE_IN_MASK((event)-MIN_TRUSTED_EVENT+N_SYSCALLS)] &= \
            ~BITS_IN_BYTE(0x3,(event)-MIN_TRUSTED_EVENT+N_SYSCALLS); \
        buf[BYTE_IN_MASK((event)-MIN_TRUSTED_EVENT+N_SYSCALLS)] |= \
            BITS_IN_BYTE(( ((succeed)<<1)|(fail) ),(event)-MIN_TRUSTED_EVENT+N_SYSCALLS); \
      } \
      else { \
        buf[BYTE_IN_MASK(event)] &= ~BITS_IN_BYTE(0x3,event); \
        buf[BYTE_IN_MASK(event)] |= BITS_IN_BYTE(( ((succeed)<<1)|(fail) ),event); \
      } \
    }

/* adjust buf for siteauditmask by setting event succeed/fail bits */
#define A_SITEMASK_SET(buf,event,succeed,fail) \
    { if ( (event) >= MIN_SITE_EVENT ) { \
        buf[BYTE_IN_MASK((event)-MIN_SITE_EVENT)] &= \
            ~BITS_IN_BYTE(0x3,(event)-MIN_SITE_EVENT); \
        buf[BYTE_IN_MASK((event)-MIN_SITE_EVENT)] |= \
            BITS_IN_BYTE(( ((succeed)<<1)|(fail) ),(event)-MIN_SITE_EVENT); \
      } \
    }


#ifdef KERNEL
#include "bin_compat.h"

/* macros to check auditability of event according to auditmask; 2 bits/event */
#define EVENT_BITS(mask,event)  ((mask[WORD_IN_MASK(event)] >> EVENT_SHIFT(event) ) & 0x3)
#define AUDIT_EVENT_K(event)    syscallauditmask[event]
#define AUDIT_EVENT_U(event)    EVENT_BITS(u.u_auditmask,event)
#define AUDIT_EVENT_T(event)    EVENT_BITS(trustedauditmask,event)
#define AUDIT_EVENT_S(event)    EVENT_BITS(siteauditmask,event)

#define DO_AUD(event) \
    (u.u_audit_cntl & AUDIT_OFF) ? 0 : \
    ((u.u_audit_cntl & AUDIT_AND) ? \
        AUDIT_EVENT_K(event) & AUDIT_EVENT_U(event) : \
    ((u.u_audit_cntl & AUDIT_USR) ? \
        AUDIT_EVENT_U(event) : \
        AUDIT_EVENT_K(event) | AUDIT_EVENT_U(event) ))

#define DO_TRUSTED_AUD(event) \
    (u.u_audit_cntl & AUDIT_OFF) ? 0 : \
    ((u.u_audit_cntl & AUDIT_AND) ? \
        AUDIT_EVENT_T((event)-MIN_TRUSTED_EVENT) & AUDIT_EVENT_U((event)-MIN_TRUSTED_EVENT+N_SYSCALLS) : \
    ((u.u_audit_cntl & AUDIT_USR) ? \
        AUDIT_EVENT_U((event)-MIN_TRUSTED_EVENT+N_SYSCALLS) : \
        AUDIT_EVENT_T((event)-MIN_TRUSTED_EVENT) | AUDIT_EVENT_U((event)-MIN_TRUSTED_EVENT+N_SYSCALLS) ))


#if BIN_COMPAT
#define DO_HABITAT_AUD(habitat,event) \
    (u.u_audit_cntl & (AUDIT_OFF|AUDIT_USR)) || habitats[habitat]->cm_auditmask == NULL ? \
    0 : EVENT_BITS(habitats[habitat]->cm_auditmask,(event))

#define DO_AUDIT(event,error) \
    ( audswitch == 1 && \
    (hbval(event) ? \
        ( (((error) == 0) && (DO_HABITAT_AUD(hbval(event),(event)&~HABITAT_MASK)&0x02)) || \
        ((error) && (DO_HABITAT_AUD(hbval(event),(event)&~HABITAT_MASK)&0x01)) ) \
        : \
        ( (((error) == 0) && (DO_AUD(event)&0x02)) || \
        ((error) && (DO_AUD(event)&0x01)) )) )

#else /* BIN_COMPAT */
#define DO_AUDIT(event,error) \
    ( audswitch == 1 && \
    ( (((error) == 0) && (DO_AUD(event)&0x02)) || \
    ((error) && (DO_AUD(event)&0x01)) ) )
#endif /* BIN_COMPAT */


/* perform audit_rec_build if auditing on for specified event/proc */
#define AUDIT_CALL(event,error,args,retval,flags,param) \
{ \
    if ( DO_AUDIT((event),(error)) ) \
        audit_rec_build ( (event), (args), (error), (retval), (int *)0, (flags), (param) ); \
}


/* version of AUDIT_CALL which calls a function instead of inline expansion;
   specifically to decrease text space growth
*/
#define AUDIT_CALL2(event,error,args,retval,flags,param) \
{ \
    if ( audswitch == 1 ) \
        audit_call ( event, error, args, retval, flags, param ); \
}


/* audit_rec_build operations */
#define AUD_VNO              0x001
#define AUD_HDR              0x002
#define AUD_PRM              0x004
#define AUD_RES              0x008
#define AUD_HPR              (AUD_HDR|AUD_PRM|AUD_RES)
#define AUD_VHPR             (AUD_VNO|AUD_HDR|AUD_PRM|AUD_RES)
#define AUD_FLU              0x010
#define AUD_PTR              0x020
#define AUD_LCL              0x040
#define AUD_LCK              0x080
#define AUD_GEN              0x100


/* base size for audit data buffer */
#define AUDSIZE(x) \
    ( ((x)*1024) < (AUD_BUF_SIZ<<2) ? (AUD_BUF_SIZ<<2) : ((x)*1024) )


/* global data */
extern u_int syscallauditmask[];
extern u_int trustedauditmask[];
extern u_int siteauditmask[];
extern u_int n_sitevents;
extern u_int audswitch;
extern int   audsize;
extern u_int audstyle;
extern struct aud_log aud_log;


/*
  encodings for sysent[].aud_param
  (cap designation indicates security level must be recorded)
    0:   nothing
    1:   mode_t & ~umask
    2:   mode_t
    3:   long
    4:   *int
    5:   int[], length in prev param
    a/A: int
    b/B: *char (user-space)
    c/C: fd
    g/G: char[], length in next param
    h/H: struct sockaddr *addr, length in next param
    i/I: struct msghdr *msg (4.3)
    k/K: msgctl operations:
    l/L: uid_t
    m/M: gid_t
    n/N: semctl operations (similar to msgctl operations)
    o/O: shmctl operations (similar to msgctl operations)
    p/P: dev_t
    q/Q: pid_t
    r/R: *char (kernel-space)
    s/S: fd[2] (for socketpair)
    t/T: struct msghdr *msg (4.4)
    v/V: exportfs operation
    w/W: *char (user-space)
            with vnode info taken from retval (use only for object creat)
    x/X: *char (user-space)
            with add'l namei to get vnode info (use only for object creat)

  last field in sysent[].aud_param entry:
    1:   don't audit object access
    4:   perform audit from system call's code
    5:   don't audit object access, perform audit from system call's code
  sysent entries marked with values >= SELF_AUDIT indicate syscall 
  generates its own audit record
*/

#define SELF_AUDIT  '4'

#endif /* KERNEL */


#if !(defined(KERNEL) || defined(_KERNEL))

extern char *syscallnames[];
extern char *trustedevent[];

#ifdef _NO_PROTO

extern int audit_change_mask();
extern int audit_build_mask();
extern void audit_change_mask_done();
extern int aud_aliasent();
extern int aud_alias_event();
extern int aud_sitevent();
extern int aud_sitevent_num();

#else /* !_NO_PROTO */

#if defined(__STDC__) || defined(__cplusplus)
#ifdef __cplusplus
extern "C"
{
#endif

extern int audit_change_mask(char const * , char * , char * , char * , int , char * , int );
extern int audit_build_mask(char const * , char * , char * , int );
extern void audit_change_mask_done(void);
extern int aud_aliasent(int , char * , int );
extern int aud_alias_event(char const * , int , char * , int );
extern int aud_sitevent(int , int , char * , char * );
extern int aud_sitevent_num( char const * , char const * , int * , int * );

#ifdef __cplusplus
}
#endif
#endif /* __STDC__ || __cplusplus */

#endif /* !_NO_PROTO */

#endif /* !(defined(KERNEL) || defined(_KERNEL)) */
#endif /* __AUDIT__ */
