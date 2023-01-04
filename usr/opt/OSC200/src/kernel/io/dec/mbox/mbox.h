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

#ifndef MBOX_H
#define MBOX_H

#include <sys/types.h>
#include <mach/boolean.h>
#include <io/common/devdriver.h>

#define LOCAL_CSR     (-1)

/* 
 * micro seconds to wait for a mailbox transaction to complete.
 * If exceeded, log an error message that we may be having a problem.
 */
#define MBOX_DELAY 40

#define RBUS_DEV_TYPE  0xffff      /* for who are you's */

/* mailbox bits */
#define MBOX_CMD    0x3fffffff     /* mailbox cmd bits <29:0> adpt specific */
#define MBOX_DON_BIT    0x00000001
#define MBOX_ERR_BIT    0x00000002
#define MBOX_MASK_BITS 0x000000ff
#define MBOX_AS_BITS    0x00000f00
#define MBOX_CMD_BITS   0xfffff000

struct mbox {
	/*** hardware portion ***/
	/* NB: 
	 * the hardware portion of the mailbox must be on a 64 byte boundary 
	 */
	unsigned int cmd;
	unsigned int mask:8,
                     rsv1:8,
                     hose:8,
                     rsv2:8; 
#ifdef __alpha
	unsigned long rbadr;
	unsigned long wdata;
#endif /* __alpha */
#ifdef __mips
#define rbadr rbaddr[0]
#define wdata wrtdata[0]
	unsigned int rbaddr[2];
	unsigned int wrtdata[2];
#endif /* __mips */
	unsigned int unpredictable0[2];
	unsigned int rdata;
	unsigned int unpredictable1;
	union {
#ifdef __alpha
#define mb_status status_un.stat
#define mb_don_bit status_un.status_64.don
#define mb_err_bit status_un.status_64.err
		struct status_64 {
			unsigned long don:1,
		                      err:1,
		                      stat0:62;
		} status_64;
		unsigned long stat;
#endif /* __alpha */
#ifdef __mips
#define mb_status status_un.stat[0]
#define mb_don_bit status_un.status_32.don
#define mb_err_bit status_un.status_32.err
		struct status_32 {
			unsigned int don:1,
                         	     err:1,
		                     stat0:30;
			unsigned int stat1;
		} status_32;
		unsigned int stat[2];
#endif /* __mips */
	} status_un;
	unsigned int unpredictable2[2];
	unsigned int unpredictable3[2];
	/*** software portion of the mailbox ***/
	unsigned int bus_timeout;  /* number of usec's to wait for comp */
	unsigned int mbox_inuse;   /* debug */
	vm_offset_t mbox_reg;      /* vaddr of mail box reg for this mbox */
	vm_offset_t mbox_pa;       /* mbox phys addr, set by mbox_pool_init */
	void (*mbox_cmd)();        /* adapter specific cmd field rtn */
	int (*err_rtn)();          /* adapter specific err rtn
				    * if != 0, then bus_ctlr_ptr must be
				    * filled in
				    */
	bus_ctlr_common_t bus_ctlr_ptr; /* used by err_rtn */
	struct mbox *next;		   /* used to link mbox structures when in a list */
#ifdef __alpha
#define PAD 8
#endif /* __alpha */
#ifdef __mips
#define PAD 32
#endif /* __mips */
	/* pad out to next 64 byte boundary */
	unsigned char pad[PAD];
};

typedef struct mbox * mbox_t;

/* ring buffer for debug */
/*
 * save the va for dumps, and 
 * the pa so we can look at this from the console
 */

struct mbox_debug {
	struct mb_debug_info {         /* some debug info */
		vm_offset_t pa;        /* phys addr of mbox */
		vm_offset_t va;        /* mbox va */
	} mbd[8];
};

boolean_t mbox_pool_init();
extern void dumpmbox(), null_mbox_cmd();
int mbox_error();

/* these are the type identifiers */
#define BYTE0    0x00000001
#define BYTE1    0x00000002
#define BYTE2    0x00000004
#define BYTE3    0x00000008
#define BYTE4    0x00000010
#define BYTE5    0x00000020
#define BYTE6    0x00000040
#define BYTE7    0x00000080

#define WORD0    (BYTE0|BYTE1)
#define WORD1    (BYTE2|BYTE3)
#define WORD2    (BYTE4|BYTE5)
#define WORD3    (BYTE6|BYTE7)

#define LONG0    (WORD0|WORD1)
#define LONG1    (WORD2|WORD3)

#define QUAD     (LONG0|LONG1)

/* address space identifiers */
#define A32 0x00000000
#define A64 0x00000100

/* temp back compat */
#define MBOX_AS32 A32
#define MBOX_AS64 A64

/* data space identifiers */
#define D32 0x00000000
#define D64 0x00001000

#define PARTIAL_DATUM 0x00010000

/* 
 * these bit definitions are used by drivers to access their csr's
 * These are the data size, address space and data type identifiers.
 * sizes: byte - 8 bits, word - 16, long - 32, quad - 64
 * bit definitions:
 * <16>  1 - partial (i.e. byte, word)
 * <12>  0 - 32 bit datum
 *       1 - 64 bit datum
 * <8>   0 - 32 bit address space
 *       1 - 64 bit address space
 * <7:0> byte mask - bit set is byte that will be written
 */

/* 
 * these assume the type is in the low order of the data 
 */
#define	BYTE_32 (PARTIAL_DATUM|D32|A32|BYTE0)
#define	WORD_32 (PARTIAL_DATUM|D32|A32|WORD0)
#define	LONG_32 (D32|A32|LONG0)
#define	QUAD_32 (D64|A32|QUAD)

#define	BYTE_64 (PARTIAL_DATUM|D32|A64|BYTE0)
#define	WORD_64 (PARTIAL_DATUM|D32|A64|WORD0)
#define	LONG_64 (D32|A64|LONG0)
#define	QUAD_64 (D64|A64|QUAD)

/* mailbox commands/device select */
#define RD_CSR      0x00000000
#define WRT_CSR     0x80000000     /* w(rite) bit <31> 0- read, 1-write */
#define BRDG_CSR    0x40000000     /* b(ridge) bit <30> 1-cmd to remote adpt */
#define WHOAREYOU   BRDG_CSR       /* who's on the other end of this hose? */

/* keep this in sync with panicstr[] in mbox.c */
enum { MBOX_SUCCESS, MBOX_HUNGHOSE, MBOX_QFULL, MBOX_ERR };

/*
 * MACROS
 */
#ifdef MBOX_DEBUG

#define MBOX_WAIT(mbp, rtn) {                                       \
	u_int _retries = 0, _logged = 0;                            \
	extern int cold ;					    \
	while(((mbp)->mb_status & MBOX_DON_BIT) == 0) {             \
		if( ! _logged && _retries++ > MBOX_DELAY){          \
			_logged++;                                  \
			if (!cold) {			  	    \
				printf("%s: delayed > %d microseconds\n", \
			                    (rtn), MBOX_DELAY);     \
				dumpmbox(mbp);			    \
			}					    \
		}                                                   \
		if(_retries >= (mbp)->bus_timeout)                  \
			mbox_error((mbp), (rtn), MBOX_HUNGHOSE);    \
		DELAY(1);                                           \
	}                                                           \
	mb(); /* guarantee ordering */                              \
}

#else /* MBOX_DEBUG */

#define MBOX_WAIT(mbp, rtn) {                                       \
	u_int _retries = 0;                                         \
	extern int cold;					    \
	while(((mbp)->mb_status & MBOX_DON_BIT) == 0) {             \
		if(_retries >= (mbp)->bus_timeout)                  \
			mbox_error((mbp), (rtn), MBOX_HUNGHOSE);    \
		DELAY(1);                                           \
		_retries++;					    \
	}                                                           \
	mb(); /* guarantee ordering */                              \
}

#endif /* MBOX_DEBUG */

#define MBOX_GET(pptr, cptr)  {                                        \
	if(((bus_ctlr_common_t)(pptr))->mbox != 0) {                   \
	    if(((bus_ctlr_common_t)(cptr))->mbox == 0) {               \
                mbox_t _MBP;                                           \
                _MBP = MBOX_ALLOC();                                   \
	        _MBP->hose = ((mbox_t)((bus_ctlr_common_t)(pptr))->mbox)->hose;\
	        _MBP->bus_timeout = ((mbox_t)((bus_ctlr_common_t)(pptr))->mbox)->bus_timeout;\
	        _MBP->mbox_reg = ((mbox_t)((bus_ctlr_common_t)(pptr))->mbox)->mbox_reg;\
	        _MBP->mbox_cmd = ((mbox_t)((bus_ctlr_common_t)(pptr))->mbox)->mbox_cmd;\
	        _MBP->err_rtn = ((mbox_t)((bus_ctlr_common_t)(pptr))->mbox)->err_rtn;\
	        _MBP->bus_ctlr_ptr = ((bus_ctlr_common_t)(cptr));      \
	        ((bus_ctlr_common_t)(cptr))->mbox = (u_long *)_MBP;\
		}                                                      \
	}                                                              \
}

/* synchronous */
#define RDCSR(type_as, ptr, remaddr)  rdcsr((type_as), (ptr), (remaddr))
u_long rdcsr();

/* dump and run */
#define WRTCSR(type_as, ptr, remaddr, wrtdata)    \
	wrtcsr((type_as), (ptr), (remaddr), (wrtdata))
void wrtcsr();

/* synchronous */
#define WRTCSRS(type_as, ptr, remaddr, wrtdata)    \
	wrtcsrs((type_as), (ptr), (remaddr), (wrtdata))
void wrtcsrs();

/* dump and run */
#define RDCSRA(type_as, ptr, remaddr)  rdcsra((type_as), (ptr), (remaddr))
void rdcsra();

#define CHECK_MBOX check_mbox
boolean_t check_mbox();

#define MBOX_ALLOC mbox_alloc
mbox_t mbox_alloc();

#define MBOX_FREE mbox_free
void mbox_free();

#define MBOX_GO mbox_go
boolean_t mbox_go();

#endif /* MBOX_H */
