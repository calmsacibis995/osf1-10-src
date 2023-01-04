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

#ifndef FBUSREG_H
#define FBUSREG_H

#include <io/dec/mbox/mbox.h>

void fbus_mbox_cmd( /* mbox_t mbp, u_int rwflag, u_int mask_as */ );
boolean_t fbus_probe( /* struct bus *, int node */ );

/* 
 * The fbus node space is 4kb in size and broken up into 4 pieces
 *
 *          initial node space             offset (decimal)
 *          ------------------------------
 *          | Core csrs (p896.2)         | 0x0    (0)
 *          | 512 bytes                  |
 *          -----------------------------
 *          | Fbus+ dependent (896.2)    | 0x200  (512)
 *          | 512 bytes                  |
 *          -----------------------------
 *          | Rom space (p1212)          | 0x400  (1024)
 *          | 1024 bytes                 |
 *          -----------------------------
 *          | Initial units space        | 0x800  (2048)
 *          | (vendor defined)           |
 *          | 2048 bytes                 |
 *          ------------------------------
 *
 * Bus addressing (P1212)
 *
 * bits
 * <31:28>  fbus base address = 0xf
 * <27:18>  1k bus address space 
 * <17:12>  64 nodes
 * <11:0>   4k csr address space
 *          the 4k csr space is broken up as follows (byte addr)
 *          0-511 core csr's
 *          512-1023 fbus dependent csr's
 *          1024-2047 rom window's
 *          2048-4097 initial units space
 */

#define FBUS_BASE_VA           0xF0000000
#define FBUS_MAX_BUS           1024
#define FBUS_MAX_NODES         64
#define FBUS_MAX_PROF_B_NODES  (14*2) /* 14 slots * 2 nodes per slot */
#define FBUS_LOCAL_BUS         1023

#define FBUS_CORE_CSR_OFF     0
#define FBUS_BUS_DEP_OFF      0x200
#define FBUS_ROM_WIN_OFF      0x400
#define FBUS_INIT_UNIT_OFF    0x800

#define FBUS_BUS_OFF(busnum)       (((busnum) & 0x3ff) << 18)
#define FBUS_BUS_VA(busnum)        (FBUS_BASE_VA + FBUS_BUS_OFF(busnum))
#define FBUS_NODE_OFF(node)        (((node) & 0x3f) << 12)
#define FBUS_NODE_VA(busnum, node) (FBUS_BUS_VA(busnum) + FBUS_NODE_OFF(node))

#define FBUS_CORE_CSR_BASE(busnum, node)  ((core_regs_pt)FBUS_NODE_VA(busnum, node))
#define FBUS_BUS_DEP_BASE(busnum, node)   (FBUS_NODE_VA(busnum, node) + FBUS_BUS_DEP_OFF)
#define FBUS_ROM_WIN_BASE(busnum, node)   (FBUS_NODE_VA(busnum, node) + FBUS_ROM_WIN_OFF)
#define FBUS_INIT_UNIT_BASE(busnum, node) (FBUS_NODE_VA(busnum, node) + FBUS_INIT_UNIT_OFF)

/*
 * Core regs (P896.2)
 * Starts at address 0
 * offsets are in decimal
 */
struct core_regs {
	/*  name                         offset  comments                   */
	unsigned int state_clear;      /* 0  state and control information  */
	unsigned int state_set;        /* 4  sets STATE_SET bits            */
	unsigned int node_ids;         /* 8 REQUIRED                        */
	unsigned int reset_start;      /* 12 REQUIRED                       */
	unsigned int ind_addr;         /* 16 large ROM ( > 1kb)             */
	unsigned int ind_data;         /* 20 "                              */
	unsigned int splt_timeo_hi;    /* 24 split requestor (long timeout) */
	unsigned int splt_timeo_lo;    /* 28 split requestor (all timeouts) */
	unsigned int arg_hi;           /* 32   extended tests (64-address)  */
	unsigned int arg_lo;           /* 36   diagnostic test interface    */
	unsigned int test_start;       /* 40  "                             */
	unsigned int test_status;      /* 44  "                             */
	unsigned int units_base_hi;    /* 48  extended units space (64-addr)*/
	unsigned int units_base_lo;    /* 52  extended units space          */
	unsigned int units_bound_hi;   /* 56  extended units space (64-addr)*/
	unsigned int units_bound_lo;   /* 60  extended units space (64-addr)*/
	unsigned int mem_base_hi;      /* 64 extended memory space (64-addr)*/
	unsigned int mem_base_lo;      /* 68 extended memory space          */
	unsigned int mem_bound_hi;     /* 72 extended memory space (64-addr)*/
	unsigned int mem_bound_lo;     /* 76 extended memory space (64-addr)*/
	unsigned int int_targ;         /* 80 broadcast/nodecast interrupt   */
	unsigned int int_mask;         /* 84 broadcast/nodecast interrupt   */
	unsigned int clk_val_hi;       /* 88 remote clock_unit read/write   */
	unsigned int clk_val_mid;      /* 92   "                            */
	unsigned int clk_tk_per_mid;   /* 96 remote clock_unit calibration  */
	unsigned int clk_tk_per_lo;    /* 100  "                            */
	unsigned int clk_strb_arv_hi;  /* 104  "                            */
	unsigned int clk_strb_arv_mid; /* 108  "                            */
	unsigned int clk_info0;        /* 112  bus-dependent clock_unit uses*/
	unsigned int clk_info1;        /* 116  "                            */
	unsigned int clk_info2;        /* 120  "                            */
	unsigned int clk_info3;        /* 124  "                            */
	unsigned int msg_rqst[16];     /* 128  target address for messages  */
	unsigned int msg_rspn[16];     /* 192  "                            */
	unsigned int rsvd[32];         /* 256  future P1212 definitions     */
	unsigned int err_log_buff[32]; /* 384  bus dependent error log      */
};

typedef struct core_regs * core_regs_pt;

/* register bit defs */

/* node_id */

/* test_status */
/* test_status: test_state subfields */
#define TEST_STATE              0x0000003f   /* all test state bits */
#define TEST_STATE_RSVD         0x00000020   /* reserved */
#define TEST_STATE_ACTV         0x00000010   /* active */
#define TEST_STATE_LOOPG        0x00000008   /* looping */
#define TEST_STATE_IMP          0x00000004   /* implemented */
#define TEST_STATE_TMO          0x00000002   /* time out */
#define TEST_STATE_FAILD        0x00000001   /* failed */

struct bus_info_blk_hdr {
	unsigned char len;              /* 0xf */
	unsigned char crc_len;          /* len protected by crc */
	unsigned short rom_crc_val;
};

/* 
 * the root dir information structure is the first thing after 
 * the bus info block in the rom window.
 * offset 1088
 */
struct dir_info {
	u_short dir_len;
	u_short dir_crc16l;
};

/* 
 * Rom window 
 * Starts at offset 0x400 (1204)
 * Bus specific rom entries are the 1st thing in the rom window.
 * After the bus info block, is the root directory structure (offset 1088)
 * This is made up of a series of immediate, offset, leaf or subdirectories
 * based upon the key type and key value (p896.2).
 *
 * -----------------------------------------------------------------------
 * |             key (8 bits)               | entry value (24 bits)      | 
 * -----------------------------------------------------------------------
 * | key_type (2 bits) | key_value (6 bits) | offset or immediate        |
 * -----------------------------------------------------------------------
 *
 */
struct bus_info_blk {                            /* offset 1024 */ 
	struct bus_info_blk_hdr binfo_hdr;
	unsigned int bus_id;           /* 0x30383936 */
	unsigned int profile_id[2];    /* at least 1 char must == 0x42 ('b') */
	unsigned int module_logical_cap;
	unsigned int node_cap_ext;
	unsigned int competition_internal_delay;
	unsigned int packet_speed;
	unsigned int msg_frame_sz;
	unsigned int bsy_retry_cntr_cap;
	unsigned int bsy_retry_dlay_cap;
	unsigned int err_retry_cntr_cap;
	unsigned int err_retry_dlay_cap;
	unsigned int bus_info_blk_rsvd[3];
	struct dir_info root_dir;                /* offset 1088 */
}; 

typedef struct bus_info_blk * binfo_blk_pt;

#define KEY_BITS         0xff000000
#define ENTRY_VALUE_BITS ~(KEY_BITS)
#define KEY_TYPE_BITS    0xc0
#define KEY_VALUE_BITS   0x3f
#define KEY_SHIFT        24
#define VALUE_SHIFT      8
#define MAKE_KEY(val)    ((val) >> KEY_SHIFT)
#define KEY_TYPE(key)    (((key) & KEY_TYPE_BITS) >> 6)
#define KEY_VALUE(key)   ((key) & KEY_VALUE_BITS)

/* key types */
#define IMMEDIATE_KEY    0x0
#define OFFSET_KEY       0x1
#define LEAF_KEY         0x2
#define DIRECTORY_KEY    0x3

/* key values */
enum { TEXT_DESC = 1, BUS_DEP_INFO, MOD_VEND_ID, MOD_HW_VERS, MOD_SPEC_ID, 
	       MOD_SW_VERS, MOD_DEP_INFO, NODE_VEND_ID, NODE_HW_VERS, 
	       NODE_SPEC_ID, NODE_SW_VERS, NODE_CAP, NODE_UNIQ_ID, 
	       NODE_UNITS_EXT, NODE_MEM_EXT, NODE_DEP_INFO, UNIT_DIR,
	       UNIT_SPEC_ID, UNIT_SW_VERS, UNIT_DEP_INFO, UNIT_LOC, 
	       UNIT_POLL_MASK };

/* supported devices */
struct fbus_option {
	/* required profile 'b' registers (896.2) */
	u_int module_vend_id;        /* module vendor id */
	u_int module_hw_version;     /* module hardware version */
	u_int sw_vers;               /* module, node or unit sw version */
	char module_name[9];         /* module name */
	char drvname[9];             /* device or ctlr name (config file) */
	char type;                   /* C = ctrlr, A = adpt */
	int (*adpt_config)();        /* adpater config routine to call */
};

/* whats on the bus */
struct fbus_node {
	u_int profiles[2];           /* profiles supported by this node */
	/* required profile 'b' registers (896.2) */
	u_int module_vend_id;        /* module vendor id */
	u_int module_hw_version;     /* module hardware version */
	u_int sw_vers;               /* module, node or unit sw version */
	/* from fbus_options */
	char devname[9];             /* device or ctlr name (config file) */
	char module_name[9];         /* module name */
	int class;                   /* ctlr or dev: to call right cnfg rtn */
	int (*adpt_config)();        /* adpater config routine to call */
	/* linkage */
	struct controller *ctlr;     /* controller for this node */
};

#define node_tbl private[7]
#define NODE_TBL_ENT(bus, node) ((struct fbus_node *)(bus)->node_tbl)[(node)]

enum { FBUS_ADPT, FBUS_CTLR };

#define FBUS_TIMEOUT       100000    /* mailbox timeout */

#endif /* FBUSREG_H */
